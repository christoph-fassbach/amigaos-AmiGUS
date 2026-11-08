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

#include "amigus_camd.h"
#include "amigus_channels.h"
#include "amigus_hardware.h"
#include "amigus_wavetable.h"
#include "camd_interpreter.h"
#include "debug.h"

/******************************************************************************
 * Private constants.
 *****************************************************************************/

// MIDI status messages types
#define MIDI_NOTE_OFF                   0x80
#define MIDI_NOTE_ON                    0x90
#define MIDI_POLY_PRESSURE              0xA0
#define MIDI_CONTROL_CHANGE             0xB0
#define MIDI_PROGRAM_CHANGE             0xC0
#define MIDI_CHANNEL_PRESSURE           0xD0
#define MIDI_PITCH_BEND                 0xE0
#define MIDI_SYSTEM_MSG                 0xF0

#define CAMD_INTERPRETER_INVALID        0x80

#define MIDI_PERCUSSIONS_CHANNEL        9
#define MIDI_PERCUSSIONS_BANK           128
#define MIDI_CHANNEL_COUNT              16

#define MIDI_VOLUME_MIN_VALUE           0
#define MIDI_VOLUME_MAX_VALUE           127
#define MIDI_VOLUME_VALUE_COUNT         128

#define MIDI_PANNING_VALUE_COUNT        128
#define MIDI_PANNING_ALL_LEFT_VALUE     0
#define MIDI_PANNING_MID_VALUE          64
#define MIDI_PANNING_ALL_RIGHT_VALUE    127

typedef enum {

  STATE_WAIT_STATUS,
  STATE_WAIT_DATA1,
  STATE_WAIT_DATA2_2,
  STATE_WAIT_DATA2_1

} MidiState;

/**
 * Volume lookup values for 0 to 127 volume values input from CAMD / MIDI
 * direction to be translated to AmiGUS compatible channel volume settings.
 *
 * MIDI in:    x = 0..127
 * AmiGUS out: y = 0xFFFF + 0xFFFF * (( 40 * log10( x / 127 )) / 100 )
 */
static const UWORD AmiGusMidiVolumeMapping[ MIDI_VOLUME_VALUE_COUNT ] = {

  0x0000,
  0x2892,
  0x4765,
  0x596D,
  0x6638,
  0x7025,
  0x7840,
  0x7F1B,
  0x850B,
  0x8A48,
  0x8EF8,
  0x9335,
  0x9714,
  0x9AA3,
  0x9DEE,
  0xA100,
  0xA3DF,
  0xA691,
  0xA91C,
  0xAB83,
  0xADCB,
  0xAFF7,
  0xB208,
  0xB402,
  0xB5E7,
  0xB7B7,
  0xB976,
  0xBB24,
  0xBCC2,
  0xBE51,
  0xBFD3,
  0xC148,
  0xC2B2,
  0xC410,
  0xC564,
  0xC6AE,
  0xC7EF,
  0xC927,
  0xCA56,
  0xCB7E,
  0xCC9E,
  0xCDB7,
  0xCECA,
  0xCFD6,
  0xD0DB,
  0xD1DB,
  0xD2D5,
  0xD3CA,
  0xD4BA,
  0xD5A5,
  0xD68B,
  0xD76C,
  0xD849,
  0xD922,
  0xD9F7,
  0xDAC8,
  0xDB95,
  0xDC5E,
  0xDD24,
  0xDDE7,
  0xDEA6,
  0xDF63,
  0xE01C,
  0xE0D2,
  0xE185,
  0xE236,
  0xE2E3,
  0xE38F,
  0xE437,
  0xE4DD,
  0xE581,
  0xE623,
  0xE6C2,
  0xE75F,
  0xE7FA,
  0xE893,
  0xE92A,
  0xE9BE,
  0xEA51,
  0xEAE2,
  0xEB71,
  0xEBFF,
  0xEC8B,
  0xED15,
  0xED9D,
  0xEE24,
  0xEEA9,
  0xEF2C,
  0xEFAF,
  0xF02F,
  0xF0AE,
  0xF12C,
  0xF1A9,
  0xF224,
  0xF29D,
  0xF316,
  0xF38D,
  0xF403,
  0xF478,
  0xF4EB,
  0xF55E,
  0xF5CF,
  0xF63F,
  0xF6AE,
  0xF71C,
  0xF789,
  0xF7F5,
  0xF860,
  0xF8CA,
  0xF933,
  0xF99B,
  0xFA02,
  0xFA68,
  0xFACD,
  0xFB32,
  0xFB95,
  0xFBF8,
  0xFC59,
  0xFCBA,
  0xFD1A,
  0xFD7A,
  0xFDD8,
  0xFE36,
  0xFE93,
  0xFEEF,
  0xFF4A,
  0xFFA5,
  0xFFFF
};

/******************************************************************************
 * Low-Level MIDI functions - private functions.
 *****************************************************************************/

VOID AmiGusNoteOff( struct MidiMessage * midiMessage ) {
 
  struct AmiGUS_CAMD * base = AmiGUS_CAMD_Base;
  struct AmiSF * amisf = base->agb_AmiSf;
  UBYTE midiChannel = midiMessage->mm_Channel;
  UBYTE note = midiMessage->mm_Data0;
  UBYTE velocity = midiMessage->mm_Data1;
  UBYTE amigusChannelId = FreeAmiGusChannel( midiChannel, note, velocity );

  if ( 0xFF == amigusChannelId ) {
  
    LOG_D(( "D: Trying to stop channel %ld, but not active!\n", midiChannel ));

    return;
  }
  StopAmiGusWavetablePlayback( amigusChannelId );
}

VOID AmiGusNoteOn( struct MidiMessage * midiMessage ) {
 
  struct AmiGUS_CAMD * base = AmiGUS_CAMD_Base;
  struct AmiSF * amisf = base->agb_AmiSf;
  UBYTE midiChannelId = midiMessage->mm_Channel;
  struct MidiChannelState * midiChannel =
    &( base->agb_MidiChannelState[ midiChannelId ]);

  UBYTE bank = midiChannel->mcs_Bank;
  UBYTE preset = midiChannel->mcs_Preset;
  UBYTE note = midiMessage->mm_Data0;
  UBYTE velocity = midiMessage->mm_Data1;

  struct AmiSF_Preset * sfPreset = GetAmiSfPreset( amisf, bank, preset );
  LONG noteIndex = GetAmiSfNoteIndex( amisf, sfPreset, note );
  struct AmiSF_Note * sfNote = GetAmiSfNote( amisf, noteIndex );
  struct AmiSF_Sample * sfSample = GetAmiSfSample( amisf, sfNote );

  ULONG playbackRate = GetPlaybackRate( amisf, sfNote, note );
  UWORD playbackVolume = GetPlaybackVolume( velocity,
                                            midiChannel->mcs_Volume );

  UBYTE amigusChannelId = AllocateNextAmiGusChannel( midiChannelId,
                                                     note,
                                                     velocity );

  // TODO: Sample handling

  StartAmiGusWavetablePlayback( amigusChannelId,
                                sfSample->asfs_StartOffset,
                                sfSample->asfs_LoopOffset,
                                sfSample->asfs_EndOffset,
                                playbackRate,
                                playbackVolume,
                                sfSample->asfs_PlaybackFlags,
                                sfNote->asfn_Attack,
                                sfNote->asfn_Decay,
                                sfNote->asfn_Sustain,
                                sfNote->asfn_Release );
}

BOOL AmiGusControlChange( struct MidiMessage * midiMessage ) {

  struct AmiGUS_CAMD * base = AmiGUS_CAMD_Base;
  struct MidiChannelState * channel =
    &( base->agb_MidiChannelState[ midiMessage->mm_Channel ]);

  switch ( midiMessage->mm_Data0 ) {
    // See MIDI 1.0 Detailed Specification
    // T-3 p.75 - T-5 p.77
    case 0x00: {
      // See MIDI 1.0 Detailed Specification
      // T-3 p.75 - Bank Select MSB
      channel->mcs_Bank &= 0x7F;
      channel->mcs_Bank |= ((( UWORD ) midiMessage->mm_Data1 ) << 7 );

      return TRUE;
    }
    case 0x07: {
      // See MIDI 1.0 Detailed Specification
      // T-3 p.75 - Channel Volume MSB
      channel->mcs_Volume = midiMessage->mm_Data1;
      return TRUE;
    }
    // case 0x08: { balance }-T3
    case 0x0A: {

      // See MIDI 1.0 Detailed Specification
      // T-3 p.75 - Channel Panning MSB
      // and pp.13/18:
      // 00 = hard left, 64 (40H) = center, and 127 (7FH) = hard right

      channel->mcs_Panning = midiMessage->mm_Data1;

      //return TRUE;
      break;
    }
    case 0x20: {

      // See MIDI 1.0 Detailed Specification
      // T-3 p.75 - Bank Select LSB
      channel->mcs_Bank &= 0x80;
      channel->mcs_Bank |= midiMessage->mm_Data1;

      return TRUE;
    }
    // case 0x46: { sound variation }
    // case 0x47: { timbre / harmonic }
    // case 0x48: { release }
    // case 0x49: { attack }
    // case 0x4a: { brightness }
    default: {
      break;
    }
  }
  LOG_W(( "W: Did not understand Control Change 0x%02lx-%02lx-%02lx-%02lx\n",
          midiMessage->mm_Status,
          midiMessage->mm_Channel,
          midiMessage->mm_Data0,
          midiMessage->mm_Data1 ));
  return FALSE;
}

VOID AmiGusProgramChange( struct MidiMessage * midiMessage ) {
 
  struct AmiGUS_CAMD * base = AmiGUS_CAMD_Base;
  struct MidiChannelState * channel =
    &( base->agb_MidiChannelState[ midiMessage->mm_Channel ]);
        
  channel->mcs_Preset = midiMessage->mm_Data0;  
}

/******************************************************************************
 * High-Level MIDI functions - public functions.
 *****************************************************************************/

VOID InitMidiState( VOID ) {

  struct AmiGUS_CAMD * base = AmiGUS_CAMD_Base;

  ULONG i;

  base->agb_NextAmiGusChannelUsageId = 32;
  for ( i = 0; 16 > i ; ++i ) {

    struct MidiChannelState * state = &( base->agb_MidiChannelState[ i ]);
    if ( MIDI_PERCUSSIONS_CHANNEL != i ) {

      state->mcs_Bank = 0;

    } else {

      // Prepare percussions
      state->mcs_Bank = MIDI_PERCUSSIONS_BANK;
    }
    
    state->mcs_Preset = 0;
    state->mcs_Volume = MIDI_VOLUME_MAX_VALUE;
    state->mcs_Panning = MIDI_PANNING_MID_VALUE;
  }
  for ( i = 0; 32 > i ; ++i ) {

    struct AmiGusChannelState * state = &( base->agb_AmiGusChannelState[ i ]);

    state->acs_IsActive = 0;
    state->acs_UsageId = i;
  }
}

BOOL TokenizeMidiByte( UBYTE midiByte, struct MidiMessage * midiMessage ) {

  static MidiState state;

  // Handle realtime messages w/o changing state machine
  if ( 0xF8 <= midiByte ) {

    // TODO: Handle them ;)
    return FALSE;
  }

  // Handle status messages
  if ( 0x80 & midiByte ) {

    // TODO: Handle System Common / SysEx messages (0xF0 - 0xF7)
    if (( midiByte & 0xF0) == MIDI_SYSTEM_MSG ) {

      // Device Volume and master balance see 
      // MIDI 1.0 Detailed Specification, p.57
      state = STATE_WAIT_STATUS;
      return FALSE;
    }

    midiMessage->mm_Status = midiByte & 0xF0;
    midiMessage->mm_Channel = midiByte & 0x0F;

    switch ( midiMessage->mm_Status ) {
      case MIDI_PROGRAM_CHANGE:
      case MIDI_CHANNEL_PRESSURE: {
        state = STATE_WAIT_DATA1;
        break;
      }
      default: {
        state = STATE_WAIT_DATA2_2;
        break;
      }
    }
    return FALSE;
  }

  // Handle data bytes
  switch ( state ) {
    case STATE_WAIT_DATA1: {
      midiMessage->mm_Data0 = midiByte;
      midiMessage->mm_Data1 = 0;
      state = STATE_WAIT_STATUS;

      LOG_D(( "D: MIDI message completed, 0x%02lx-%02lx-%02lx-%02lx\n",
              midiMessage->mm_Status,
              midiMessage->mm_Channel,
              midiMessage->mm_Data0,
              midiMessage->mm_Data1 ));
      return TRUE;
    }
    case STATE_WAIT_DATA2_2: {
      midiMessage->mm_Data0 = midiByte;
      state = STATE_WAIT_DATA2_1;
      return FALSE;
    }
    case STATE_WAIT_DATA2_1: {
      midiMessage->mm_Data1 = midiByte;
      state = STATE_WAIT_STATUS;

      LOG_D(( "D: MIDI message completed, 0x%02lx-%02lx-%02lx-%02lx\n",
              midiMessage->mm_Status,
              midiMessage->mm_Channel,
              midiMessage->mm_Data0,
              midiMessage->mm_Data1 ));
      return TRUE;
    }
    case STATE_WAIT_STATUS:
    default: {
      break;
    }
  }
  return FALSE;
}

BOOL InterpretMidiMessage( struct MidiMessage * midiMessage ) {

  switch ( midiMessage->mm_Status ) {
    case MIDI_NOTE_OFF: {

      LOG_D(( "D: Called NOTE_OFF for channel %ld note %ld velocity %ld\n",
              midiMessage->mm_Channel,
              midiMessage->mm_Data0,
              midiMessage->mm_Data1 ));

      AmiGusNoteOff( midiMessage );

      return TRUE;
    }
    case MIDI_NOTE_ON: {

      UBYTE velocity = midiMessage->mm_Data1;
      LOG_D(( "D: Called NOTE_ON for channel %ld note %ld velocity %ld\n",
              midiMessage->mm_Channel,
              midiMessage->mm_Data0,
              velocity ));

      if ( velocity ) {

        AmiGusNoteOn( midiMessage );

      } else {

        AmiGusNoteOff( midiMessage );
      }

      return TRUE;
    }
    case MIDI_POLY_PRESSURE: {
      return FALSE;
    }
    case MIDI_CONTROL_CHANGE: {

      LOG_D(( "D: Called CONTROL_CHANGE for channel %ld sub 0x%08lx data %ld\n",
              midiMessage->mm_Channel,
              midiMessage->mm_Data0,
              midiMessage->mm_Data1 ));

      AmiGusControlChange( midiMessage );

      return TRUE;
    }
    case MIDI_PROGRAM_CHANGE: {

      LOG_D(( "D: Called POGRAM_CHANGE for channel %ld preset %ld\n",
              midiMessage->mm_Channel,
              midiMessage->mm_Data0 ));

      AmiGusProgramChange( midiMessage );

      return TRUE;
    }
    case MIDI_CHANNEL_PRESSURE: {
      return FALSE;
    }
    case MIDI_PITCH_BEND: {
      return FALSE;
    }
    case MIDI_SYSTEM_MSG: {
      // See MIDI 1.0 Detailed Specification
      // T-6 p.78 - T-7 p.79
      return FALSE;
    }
    default: {

      LOG_W(( "W: Message %lx not understood.\n", midiMessage->mm_Status ));
      break;
    }
  }
  return FALSE;
}

ULONG GetPlaybackRate( struct AmiSF * amisf,
                       struct AmiSF_Note * sfNote,
                       UBYTE note ) {

  ULONG playbackRateIndex =
    note + sfNote->asfn_BasePlaybackIndex - sfNote->asfn_BaseNote;
  ULONG playbackRate = amisf->asf_PlaybackRate[ playbackRateIndex ];

  return playbackRate;
}

/******************************************************************************
 * Volume calculation functions - public functions.
 *****************************************************************************/

UWORD GetPlaybackVolume( UBYTE velocity, UBYTE channelVolume ) {

  UWORD velocityFactor = AmiGusMidiVolumeMapping[ velocity ];
  UWORD volumeFactor = AmiGusMidiVolumeMapping[ channelVolume ];
  ULONG volume = ( UWORD ) velocityFactor * ( UWORD ) volumeFactor;
  UWORD playbackVolume = volume >> 19;

  LOG_D(( "V: GetPlaybackVolume( %ld, %ld) -> "
          "%ld * %ld = 0x%08lx = 0x%04lx \n",
          velocity, channelVolume,
          AmiGusMidiVolumeMapping[ velocity ],
          AmiGusMidiVolumeMapping[ channelVolume ],
          volume, playbackVolume ));

  return playbackVolume;
}
