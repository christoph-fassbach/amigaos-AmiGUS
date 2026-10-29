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

#define DEBUG_H
#define LOG_INT(X) printf X
#define LOG_V(X)   printf X
#define LOG_D(X)   printf X
#define LOG_I(X)   printf X
#define LOG_W(X)   printf X
#define LOG_E(X)   printf X

#include "../src/camd_interpreter.c"

struct AmiGUS_CAMD * AmiGUS_CAMD_Base;

UBYTE AllocateNextAmiGusChannel( UBYTE midiChannel ) {

  return 7;
}

UBYTE GetUsedAmiGusChannel( UBYTE midiChannel ) {

  return 7;
}

VOID StartAmiGusWavetablePlayback( UBYTE amiGusChannel,
                                   ULONG startOffset,
                                   ULONG loopOffset,
                                   ULONG endOffset,
                                   ULONG playbackRate,
                                   UWORD playbackVolume,
                                   UWORD playbackFlags,
                                   UWORD attack,
                                   UWORD decay,
                                   UWORD sustain,
                                   UWORD release ) {
}

VOID StopAmiGusWavetablePlayback( UWORD channel ) {
}

/******************************************************************************
 * Test functions:
 *****************************************************************************/

BOOL testTokenizeMidiByte( VOID ) {

  struct MidiMessage midiMessage;
  BOOL failed = FALSE;

  printf("\n");

  // Channel 13 Note 60 Velocity 127 = on -> to be done
  failed |= ( TokenizeMidiByte( 0x9D, &midiMessage ) != FALSE ); // Note On, Channel 13
  failed |= ( TokenizeMidiByte( 0x3C, &midiMessage ) != FALSE ); // Note 60
  failed |= ( TokenizeMidiByte( 0x7F, &midiMessage ) != TRUE );  // Velocity 127
  failed |= ( *(( ULONG * ) &midiMessage ) != 0x900D3C7F );

  // Channel 13 Note 61 Velocity 127 = on -> to be ignored
  failed |= ( TokenizeMidiByte( 0x8D, &midiMessage ) != FALSE ); // Note Off, Channel 13
  failed |= ( TokenizeMidiByte( 0x40, &midiMessage ) != FALSE ); // Note 61
  failed |= ( TokenizeMidiByte( 0x7F, &midiMessage ) != TRUE );  // Velocity 127
  failed |= ( *(( ULONG * ) &midiMessage ) != 0x800D407F );

  // Channel 13 Note 60 Velocity 127 = off -> to be done
  failed |= ( TokenizeMidiByte( 0x8D, &midiMessage ) != FALSE ); // Note Off, Channel 13
  failed |= ( TokenizeMidiByte( 0x3C, &midiMessage ) != FALSE ); // Note 60
  failed |= ( TokenizeMidiByte( 0x7F, &midiMessage ) != TRUE );  // Velocity 127
  failed |= ( *(( ULONG * ) &midiMessage ) != 0x800D3C7F );

  // Select Bank 129
  failed |= ( TokenizeMidiByte( 0xB0, &midiMessage ) != FALSE ); // Control Change channel 0
  failed |= ( TokenizeMidiByte( 0x00, &midiMessage ) != FALSE ); // #0 (Bank MSB)
  failed |= ( TokenizeMidiByte( 0x01, &midiMessage ) != TRUE );  // MSB 1 * 128
  failed |= ( *(( ULONG * ) &midiMessage ) != 0xB0000001 );

  failed |= ( TokenizeMidiByte( 0xB0, &midiMessage ) != FALSE ); // Control Change channel 0
  failed |= ( TokenizeMidiByte( 0x20, &midiMessage ) != FALSE ); // #32 (Bank LSB)
  failed |= ( TokenizeMidiByte( 0x01, &midiMessage ) != TRUE );  // LSB 1
  failed |= ( *(( ULONG * ) &midiMessage ) != 0xB0002001 );

  // Select Preset 4
  failed |= ( TokenizeMidiByte( 0xC0, &midiMessage ) != FALSE ); // Program Change channel 0
  failed |= ( TokenizeMidiByte( 0x04, &midiMessage ) != TRUE );  // Preset 4
  failed |= ( *(( ULONG * ) &midiMessage ) != 0xC0000400 );

  return failed;
}

BOOL testInterpretMidiMessage( VOID ) {

  struct MidiMessage midiMessage;
  BOOL failed = FALSE;

  midiMessage.mm_Status = 0x90;
  midiMessage.mm_Channel = 0x04;
  midiMessage.mm_Data0 = 0x3C;
  midiMessage.mm_Data1 = 0x40;

  failed |= ( !( InterpretMidiMessage( &midiMessage )));

  midiMessage.mm_Status = 0x80;

  failed |= ( !( InterpretMidiMessage( &midiMessage )));

  return failed;
}

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main( int argc, char const *argv[] ) {

  BOOL failed = FALSE;
  struct AmiSF * amisf = AllocMem( sizeof( struct AmiSF ),
                                   MEMF_PUBLIC | MEMF_CLEAR );

  AmiGUS_CAMD_Base = AllocMem( sizeof( struct AmiGUS_CAMD ),
                               MEMF_PUBLIC | MEMF_CLEAR );
  AmiGUS_CAMD_Base->agb_AmiSf = amisf;
  amisf->asf_NoteCount = 100;
  amisf->asf_Note = AllocMem( sizeof( struct AmiSF_Note ) * 100,
                              MEMF_PUBLIC | MEMF_CLEAR );
  amisf->asf_PlaybackRateCount = 1000;
  amisf->asf_PlaybackRate = AllocMem( sizeof( ULONG ) * 1000,
                                      MEMF_PUBLIC | MEMF_CLEAR );
  amisf->asf_SampleMetadata = AllocMem( sizeof( struct AmiSF_Sample ) * 100,
                                        MEMF_PUBLIC | MEMF_CLEAR );

  amisf->asf_Preset[0][0].asfp_NoteCount = 3;
  amisf->asf_Note[1].asfn_MaxNote = 0x40;
  amisf->asf_Note[2].asfn_MaxNote = 0x7F;


  failed |= testTokenizeMidiByte();
  failed |= testInterpretMidiMessage();

  FreeMem( amisf->asf_SampleMetadata, sizeof( struct AmiSF_Sample ) * 100 );
  FreeMem( amisf->asf_PlaybackRate, sizeof( struct AmiSF_Note ) * 1000 );
  FreeMem( amisf->asf_Note, sizeof( struct AmiSF_Note ) * 100 );
  FreeMem( AmiGUS_CAMD_Base->agb_AmiSf, sizeof( struct AmiGUS_CAMD ));
  FreeMem( AmiGUS_CAMD_Base, sizeof( struct AmiSF ));

  printf( "\nDone, result %s.\n", 
          failed ? "FAIL!" : "OK." );

  return ( failed ) ? 15 : 0;
}
