/*
 * This file is part of the amigus.library.
 *
 * amigus.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * amigus.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with amigus.library.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

// vc +kick13 -I../include -I../header amigus_lib_check.c -o amigus_lib_check

#include <stdio.h>
#include <string.h>

#ifdef NULL
#undef NULL
#endif

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/amigus.h>
#include <stddef.h>

#include "amigus_private.h"
#include "errors.h"

#define HAGEN_INTC0			    0x00
#define HAGEN_INTE0			    0x08

#define HAGEN_TIMER_CTRL    0xf0
#define HAGEN_TIMER_RELOADH	0xf2
#define HAGEN_TIMER_RELOADL	0xf4
#define HAGEN_TIMER_READH	  0xf6
#define HAGEN_TIMER_READL	  0xf8

#define HAGEN_TIMER_TIMEBASE 24576000

struct AmiGUS_Base * AmiGUS_Base;
ULONG interruptCounter;

/******************************************************************************
 * Low-Level hardware access functions - public function definitions.
 *****************************************************************************/

UWORD ReadReg16( APTR card, ULONG offset ) {

  return *(( UWORD * )(( ULONG ) card + offset ));
}

ULONG ReadReg32( APTR card, ULONG offset ) {

  return *(( ULONG * )(( ULONG ) card + offset ));
}

VOID WriteReg16( APTR card, ULONG offset, UWORD value ) {

  *(( UWORD * )(( ULONG ) card + offset )) = value;
}

VOID WriteReg32( APTR card, ULONG offset, ULONG value ) {

  *(( ULONG * )(( ULONG ) card + offset )) = value;
}

/******************************************************************************
 * Interrupt handler
 *****************************************************************************/

LONG __ASM__ countingIntHandler( __REG__( a0, APTR cardbase )) {

  // interruptCounter = ( ULONG ) cardbase;

  UWORD status = ReadReg16( cardbase, HAGEN_INTC0 );
  if ( 0x4000 & status ) {

    WriteReg16( cardbase,
                HAGEN_INTC0,
                0x4000 );
    ++interruptCounter;
    return 1;
  }
  return 0;
}

/******************************************************************************
 * Actual test functions
 *****************************************************************************/

BOOL testTestInterrupt( struct AmiGUS * card,
                        LONG which, 
                        APTR base,
                        APTR owner ) {

  ULONG returnValue;

  interruptCounter = 0;

  printf( "Using card 0x%08lx part 0x%04lx base 0x%08lx for owner 0x%08lx\n",
          card, which, base, owner );
  printf( "Here is 0x%08lx, INT is 0x%08lx\n",
          &( testTestInterrupt ), &( countingIntHandler ));

  returnValue = AmiGUS_InstallInterrupt( card,
                                         which,
                                         owner,
                                         &( countingIntHandler ),
                                         base );
  if ( AmiGUS_NoError != returnValue ) {

    return TRUE;
  }

  WriteReg16( base, HAGEN_TIMER_CTRL,    0x0000 );
  WriteReg32( base, HAGEN_TIMER_RELOADH, HAGEN_TIMER_TIMEBASE / 50 );
  WriteReg16( base, HAGEN_TIMER_CTRL,    0x8000 );
  Delay( 50 ); // wait 1s
  WriteReg16( base, HAGEN_TIMER_CTRL,    0x0000 );
  WriteReg16( base, HAGEN_INTE0,         0x4000 );
  WriteReg16( base, HAGEN_INTC0,         0x4000 );

  AmiGUS_RemoveInterrupt( card,
                          which,
                          owner );

  printf( "Interrupt was called %ld times.\n", interruptCounter );
  return FALSE;
}

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main(int argc, char const *argv[]) {

  BOOL failed = FALSE;
  STRPTR libraryName = "amigus.library";
  struct AmiGUS * card;
  APTR owner;
  ULONG reserve;

  AmiGUS_Base = ( struct AmiGUS_Base * ) OpenLibrary( libraryName, 0 );
  if ( !AmiGUS_Base ) {

    printf( "Opening %s failed!\n", libraryName );
    return 20;
  }
  printf( "%s opened\n", libraryName );

  card = AmiGUS_FindCard( NULL );

  // Owner can be anything unique you own. :)
  owner = FindTask( NULL );
  if ( NULL == card ) {

    printf( "No Card found.\n" );
    return 20;
  }

  reserve = AmiGUS_ReserveCard( card,
                                AMIGUS_FLAG_PCM 
                                | AMIGUS_FLAG_WAVETABLE
                                | AMIGUS_FLAG_CODEC,
                                owner );
  if ( AmiGUS_NoError != reserve ) {

    printf( "Could not reserve card for owner 0x%08lx, reason 0x%04lx.\n",
            owner, reserve );
    return 20;
  }

  failed |= testTestInterrupt( card, AMIGUS_FLAG_WAVETABLE, card->agus_WavetableBase, owner );

  AmiGUS_FreeCard( card,
                   AMIGUS_FLAG_PCM 
                   | AMIGUS_FLAG_WAVETABLE
                   | AMIGUS_FLAG_CODEC,
                   owner );

  CloseLibrary(( struct Library * ) AmiGUS_Base );
  printf( "%s closed\n", libraryName );

  if ( failed ) {

    printf( "Failed. :(\n" );

  } else {

    printf( "Looks good.\n" );
  }

  return ( failed ) ? 15 : 0;
}
