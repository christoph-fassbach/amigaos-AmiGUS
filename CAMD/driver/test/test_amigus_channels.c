/*
 * This file is part of the AmiGUS CAMD MIDI driver.
 *
 * AmiGUS CAMD MIDI driver is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS CAMD MIDI driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with AmiGUS CAMD MIDI driver.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>

#include <proto/exec.h>

#include "amigus_camd.h"

struct AmiGUS_CAMD * AmiGUS_CAMD_Base;

/******************************************************************************
 * Test functions:
 *****************************************************************************/

BOOL testAllocateNextAmiGusChannel( VOID ) {

  BOOL failed = FALSE;
  struct AmiGUS_CAMD * base = AmiGUS_CAMD_Base;
  UBYTE i;

  base->agb_NextAmiGusChannel = 0;

  printf("\n");

  for ( i = 0; 64 > i; ++i ) {

    UBYTE midiChannel = ( i % 16 );
    UBYTE actualChannelReturned = AllocateNextAmiGusChannel( midiChannel );
    UBYTE actualChannelState;
    UBYTE expectedChannel = ( i % 32 );

    printf( "Allocated AmiGUS #%ld (#%ld) to MIDI #%ld\n",
            actualChannelReturned, expectedChannel, midiChannel );

    actualChannelState =
      base->agb_MidiChannelState[ midiChannel ].mcs_AmigusChannelUsed;
    failed |= ( actualChannelState != expectedChannel );
    failed |= ( actualChannelReturned != expectedChannel );
  }
  failed |= ( 0 != base->agb_NextAmiGusChannel );

  return failed;
}

BOOL testGetUsedAmiGusChannel( VOID ) {

  BOOL failed = FALSE;
  struct AmiGUS_CAMD * base = AmiGUS_CAMD_Base;
  UBYTE i;

  base->agb_NextAmiGusChannel = 5;

  printf("\n");

  for ( i = 0; 32 > i; ++i ) {

    UBYTE midiChannel = ( i % 16 );
    AllocateNextAmiGusChannel( midiChannel );
  }

  for ( i = 0; 16 > i; ++i ) {

    UBYTE actualChannel = base->agb_MidiChannelState[ i ].mcs_AmigusChannelUsed;
    UBYTE expectedChannel = (( i + 5 + 16 ) % 32 );
    printf( "Found MIDI #%ld in AmiGUS #%ld (#%ld)\n",
            i, actualChannel, expectedChannel );

    failed |= ( actualChannel != expectedChannel );
  }
  failed |= ( 5 != base->agb_NextAmiGusChannel );

  return failed;
}

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main( int argc, char const *argv[] ) {

  BOOL failed = FALSE;

  AmiGUS_CAMD_Base = AllocMem( sizeof( struct AmiGUS_CAMD ),
                               MEMF_PUBLIC | MEMF_CLEAR );

  failed |= testAllocateNextAmiGusChannel();
  failed |= testGetUsedAmiGusChannel();

  FreeMem( AmiGUS_CAMD_Base, sizeof( struct AmiGUS_CAMD ));

  printf( "\nDone, result %s.\n", 
          failed ? "FAIL!" : "OK." );

  return ( failed ) ? 15 : 0;
}