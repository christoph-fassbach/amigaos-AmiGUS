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
#include <string.h>

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
  UBYTE m = 0;
  UBYTE e;
  UBYTE c;

  memset( (& AmiGUS_CAMD_Base->agb_NextAmiGusChannelUsageId ),
          0,
          sizeof( ULONG )
          + ( sizeof( struct MidiChannelState ) * 16 )
          + ( sizeof( struct AmiGusChannelState ) * 32 ));

  printf( "\nSection 1:\n" );

  // Test normal overwrite cycle
  for ( i = 0; i < 64; ++i ) {

    m = ( i % 16 );
    e = ( i % 32);
    c = AllocateNextAmiGusChannel( m, i, i + 2 );
    
    printf( "AmiGUS #%ld allocated for MIDI #%ld\n", c, m );

    failed |= ( e != c );
    failed |= ( m != base->agb_AmiGusChannelState[ c ].acs_MidiChannel );
    failed |= ( i != base->agb_AmiGusChannelState[ c ].acs_MidiNote );
    failed |= ( i + 2 != base->agb_AmiGusChannelState[ c ].acs_MidiVelocity );
  }

  // Test finding free ones
  printf( "\nSection 2:\n" );

  base->agb_AmiGusChannelState[ 0 ].acs_IsActive = FALSE;
  base->agb_AmiGusChannelState[ 7 ].acs_IsActive = FALSE;
  base->agb_AmiGusChannelState[ 31 ].acs_IsActive = FALSE;

  e = 0;
  c = AllocateNextAmiGusChannel( m, i, i + 2 );
  printf( "AmiGUS #%ld allocated for MIDI #%ld\n", c, m );
  failed |= ( e != c );
  failed |= ( m != base->agb_AmiGusChannelState[ c ].acs_MidiChannel );
  failed |= ( i != base->agb_AmiGusChannelState[ c ].acs_MidiNote );
  failed |= ( i + 2 != base->agb_AmiGusChannelState[ c ].acs_MidiVelocity );


  e = 7;
  c = AllocateNextAmiGusChannel( m, i, i + 2 );
  printf( "AmiGUS #%ld allocated for MIDI #%ld\n", c, m );
  failed |= ( e != c );
  failed |= ( m != base->agb_AmiGusChannelState[ c ].acs_MidiChannel );
  failed |= ( i != base->agb_AmiGusChannelState[ c ].acs_MidiNote );
  failed |= ( i + 2 != base->agb_AmiGusChannelState[ c ].acs_MidiVelocity );

  e = 31;
  c = AllocateNextAmiGusChannel( m, i, i + 2 );
  printf( "AmiGUS #%ld allocated for MIDI #%ld\n", c, m );
  failed |= ( e != c );
  failed |= ( m != base->agb_AmiGusChannelState[ c ].acs_MidiChannel );
  failed |= ( i != base->agb_AmiGusChannelState[ c ].acs_MidiNote );
  failed |= ( i + 2 != base->agb_AmiGusChannelState[ c ].acs_MidiVelocity );

  // Test normal overwrite cycle
  printf( "\nSection 3:\n" );

  e = 0;
  for ( i = 0; i < 29; ++i ) {

    m = ( i % 16 );
    ++e;

    if (( 0 == e ) || ( 7 == e ) || ( 31 == e )) {

      e = ( e + 1 ) % 32;
    } 
    c = AllocateNextAmiGusChannel( m, i, i + 2 );
    
    printf( "AmiGUS #%ld == %ld allocated for MIDI #%ld\n", c, e, m );

    failed |= ( e != c );
    failed |= ( m != base->agb_AmiGusChannelState[ c ].acs_MidiChannel );
    failed |= ( i != base->agb_AmiGusChannelState[ c ].acs_MidiNote );
    failed |= ( i + 2 != base->agb_AmiGusChannelState[ c ].acs_MidiVelocity );
  }

  // Test skipped overwrite cycle
  printf( "\nSection 4:\n" );

  e = 0;
  c = AllocateNextAmiGusChannel( m, i, i + 2 );
  printf( "AmiGUS #%ld allocated for MIDI #%ld\n", c, m );
  failed |= ( e != c );
  failed |= ( m != base->agb_AmiGusChannelState[ c ].acs_MidiChannel );
  failed |= ( i != base->agb_AmiGusChannelState[ c ].acs_MidiNote );
  failed |= ( i + 2 != base->agb_AmiGusChannelState[ c ].acs_MidiVelocity );

  e = 7;
  c = AllocateNextAmiGusChannel( m, i, i + 2 );
  printf( "AmiGUS #%ld allocated for MIDI #%ld\n", c, m );
  failed |= ( e != c );
  failed |= ( m != base->agb_AmiGusChannelState[ c ].acs_MidiChannel );
  failed |= ( i != base->agb_AmiGusChannelState[ c ].acs_MidiNote );
  failed |= ( i + 2 != base->agb_AmiGusChannelState[ c ].acs_MidiVelocity );

  e = 31;
  c = AllocateNextAmiGusChannel( m, i, i + 2 );
  printf( "AmiGUS #%ld allocated for MIDI #%ld\n", c, m );
  failed |= ( e != c );
  failed |= ( m != base->agb_AmiGusChannelState[ c ].acs_MidiChannel );
  failed |= ( i != base->agb_AmiGusChannelState[ c ].acs_MidiNote );
  failed |= ( i + 2 != base->agb_AmiGusChannelState[ c ].acs_MidiVelocity );

  // Test rollover cycle
  printf( "\nSection 5:\n" );

  base->agb_NextAmiGusChannelUsageId = 0xFFffFFff - 32;
  for ( i = 0; i < 32; ++i ) {

    AllocateNextAmiGusChannel( m, i, i + 2 );
  }
  failed |= ( base->agb_NextAmiGusChannelUsageId != 32 );
  e = 0;
  for ( i = 0; i < 32; ++i ) {

    c = base->agb_AmiGusChannelState[ i ].acs_UsageId;
    switch ( i ) {
      case 0: {
        failed |= ( 29 != c );
        ++e;
        break;
      }
      case 7: {
        failed |= ( 30 != c );
        ++e;
        break;
      }
      case 31: {
        failed |= ( 31 != c );
        ++e;
        break;
      }
      default: {
        failed |= ( i - e != c );
        break;
      }
    }
    printf( "AmiGUS #%ld allocated for MIDI #%ld\n", c, m );
  }

  return failed;
}

BOOL testFreeAmiGusChannel( VOID ) {

  BOOL failed = FALSE;
  struct AmiGUS_CAMD * base = AmiGUS_CAMD_Base;
  UBYTE i;
  UBYTE m = 0;
  UBYTE e;
  UBYTE c;

  memset( (& AmiGUS_CAMD_Base->agb_NextAmiGusChannelUsageId ),
          0,
          sizeof( ULONG )
          + ( sizeof( struct MidiChannelState ) * 16 )
          + ( sizeof( struct AmiGusChannelState ) * 32 ));

  printf( "\n" );

  AllocateNextAmiGusChannel( 1, 1, 1 );
  AllocateNextAmiGusChannel( 2, 1, 1 );
  AllocateNextAmiGusChannel( 1, 2, 1 );
  AllocateNextAmiGusChannel( 1, 1, 2 );
  for ( i = 0; 32 > i; ++i )
    if ( 4 > i )
      failed |= ( !( base->agb_AmiGusChannelState[ i ].acs_IsActive ));
    else
      failed |= ( base->agb_AmiGusChannelState[ i ].acs_IsActive );
  printf( "Free test #0 %s\n", (failed) ? "failed" : "OK" );

  FreeAmiGusChannel( 2, 2, 2 );
  for ( i = 0; 32 > i; ++i )
    if ( 4 > i )
      failed |= ( !( base->agb_AmiGusChannelState[ i ].acs_IsActive ));
    else
      failed |= ( base->agb_AmiGusChannelState[ i ].acs_IsActive );
  printf( "Free test #1 %s\n", (failed) ? "failed" : "OK" );

  FreeAmiGusChannel( 1, 1, 2 );
  for ( i = 0; 32 > i; ++i )
    if ( 3 > i )
      failed |= ( !( base->agb_AmiGusChannelState[ i ].acs_IsActive ));
    else
      failed |= ( base->agb_AmiGusChannelState[ i ].acs_IsActive );
  printf( "Free test #2 %s\n", (failed) ? "failed" : "OK" );

  FreeAmiGusChannel( 1, 2, 1 );
  for ( i = 0; 32 > i; ++i )
    if ( 2 > i )
      failed |= ( !( base->agb_AmiGusChannelState[ i ].acs_IsActive ));
    else
      failed |= ( base->agb_AmiGusChannelState[ i ].acs_IsActive );
  printf( "Free test #3 %s\n", (failed) ? "failed" : "OK" );

  FreeAmiGusChannel( 2, 1, 1 );
  for ( i = 0; 32 > i; ++i )
    if ( 1 > i )
      failed |= ( !( base->agb_AmiGusChannelState[ i ].acs_IsActive ));
    else
      failed |= ( base->agb_AmiGusChannelState[ i ].acs_IsActive );
  printf( "Free test #4 %s\n", (failed) ? "failed" : "OK" );

  FreeAmiGusChannel( 1, 1, 1 );
  for ( i = 0; 32 > i; ++i )
    if ( 0 > i )
      failed |= ( !( base->agb_AmiGusChannelState[ i ].acs_IsActive ));
    else
      failed |= ( base->agb_AmiGusChannelState[ i ].acs_IsActive );
  printf( "Free test #5 %s\n", (failed) ? "failed" : "OK" );

  return failed;
}

BOOL testGetUsedAmiGusChannel( VOID ) {

  BOOL failed = FALSE;
  struct AmiGUS_CAMD * base = AmiGUS_CAMD_Base;
  UBYTE i;

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
  failed |= testFreeAmiGusChannel();
  failed |= testGetUsedAmiGusChannel();

  FreeMem( AmiGUS_CAMD_Base, sizeof( struct AmiGUS_CAMD ));

  printf( "\nDone, result %s.\n", 
          failed ? "FAIL!" : "OK." );

  return ( failed ) ? 15 : 0;
}