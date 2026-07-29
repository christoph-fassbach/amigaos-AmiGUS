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

#include <stdio.h>
#include <string.h>

#ifdef NULL
#undef NULL
#endif

#include <proto/exec.h>
#include <proto/amigus.h>
#include <stddef.h>

// Gayle ID address
#define GAYLE_ID_ADDRESS                 0x00DE1000

// Other chip set addresses
#define CUSTOM_BLTDDAT_ADDRESS           0x00DFf000 // Random noise verification

#define LOG_INT( x )
#define LOG_V( x ) printf x
#define LOG_D( x ) printf x
#define LOG_I( x ) printf x
#define LOG_W( x ) printf x
#define LOG_E( x ) printf x

UBYTE ReadReg8( APTR reg, ULONG offset ) {

  return *(( UBYTE * )(( ULONG ) reg + offset ));
}

VOID WriteReg8( APTR reg, ULONG offset, UBYTE value ) {

  *(( UBYTE * )(( ULONG ) reg + offset )) = value;
}

static BOOL TestForGayle( VOID ) {

  UBYTE finalGayleVersion = 0;

  WORD i;
  WORD j;

  for ( i = 0; i < 4; ++i ) {

    // Similar to https://github.com/reinauer/xSysInfo/blob/main/src/hardware.c#L1274
    
    UBYTE tempGayleVersion = 0;
    
    UBYTE a;
    UBYTE b = ReadReg8( NULL, CUSTOM_BLTDDAT_ADDRESS );

    WriteReg8( NULL, GAYLE_ID_ADDRESS, 0 );
    for ( j = 0; j < 8; ++j ) {

      a = ReadReg8( NULL, GAYLE_ID_ADDRESS );
      if (( 0 == j )     // On first run only,
        && ( a == b )) { // test for garbage on the bus

        LOG_I(( "V: Found bus garbage!\n" ));
        return FALSE;
      }

      LOG_V(( "V: Raw Gayle id is %08lx.\n", a ));

      a &= 0x80;
      tempGayleVersion |= a >> j;
    }
    if ( 0 == i ) {

      finalGayleVersion = tempGayleVersion;

    } else if ( finalGayleVersion != tempGayleVersion ) {

      LOG_I(( "V: Found unstable Gayle version!\n" ));
      return FALSE;
    }
  }

  LOG_I(( "I: Found Gayle Version %ld.%ld.\n",
            ( finalGayleVersion >> 4 ),
            ( finalGayleVersion & 0x0F )));

  if (( finalGayleVersion >> 4 ) == 13 ) {

    LOG_I(( "I: Found %s Gayle.\n",
            (( finalGayleVersion & 0x0F ) == 1 ) ? "A1200" : "A600" ));

    return TRUE;
  }
  return FALSE;
}

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main(int argc, char const *argv[]) {

  BOOL failed = FALSE;

  printf( "+------------------------------------------------------+\n"
          "| Gayle direct PCMCIA development tool for AmiGUS mini |\n"
          "|     ... 'cause limits are there to be broken ...     |\n"
          "+------------------------------------------------------+\n" );

  failed |= !TestForGayle();

  if ( failed ) {

    printf( "Failed. :(\n" );

  } else {

    printf( "Looks good.\n" );
  }

  return ( failed ) ? 15 : 0;
}
