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
#include "amigus_wavetable.h"
#include "camd_interpreter.h"
#include "debug.h"

// MIDI status messages types
#define MIDI_NOTE_OFF         0x80
#define MIDI_NOTE_ON          0x90
#define MIDI_POLY_PRESSURE    0xA0
#define MIDI_CONTROL_CHANGE   0xB0
#define MIDI_PROGRAM_CHANGE   0xC0
#define MIDI_CHANNEL_PRESSURE 0xD0
#define MIDI_PITCH_BEND       0xE0
#define MIDI_SYSTEM_MSG       0xF0

#define CAMD_INTERPRETER_INVALID 0x80

typedef enum {

  STATE_WAIT_STATUS,
  STATE_WAIT_DATA1,
  STATE_WAIT_DATA2_2,
  STATE_WAIT_DATA2_1

} MidiState;

VOID AmiGusNoteOff( struct MidiMessage * midiMessage ) {
 
  struct AmiGUS_CAMD * base = AmiGUS_CAMD_Base;
  struct AmiSF * amisf = base->agb_AmiSf;
  UBYTE midiChannel = midiMessage->mm_Channel;
  struct MidiChannelState * channel =
    &( base->agb_MidiChannelState[ midiChannel ]);

  UBYTE noteStoping = midiMessage->mm_Data0;
  UBYTE velocityStoping = midiMessage->mm_Data1;

  UBYTE notePlaying = channel->mcs_ActiveNote;
  UBYTE velocityPlaying = channel->mcs_ActiveVelocity;
  UBYTE amiGusChannel;

  if ( !channel->mcs_IsActive ) {
  
    LOG_D(( "D: Trying to stop channel %ld, but not active!\n", midiChannel ));

    return;
  }
  if (( noteStoping != notePlaying )
    || ( velocityStoping != velocityPlaying )) {

    LOG_D(( "D: Trying to stop channel %ld, "
            "but mismatching note %ld = %ld or velocity %ld = %ld!\n",
            midiChannel,
            noteStoping, notePlaying, velocityStoping, velocityPlaying ));

    return;
  }

  channel->mcs_IsActive = FALSE;
  channel->mcs_AmigusChannelUsed = CAMD_INTERPRETER_INVALID;
  channel->mcs_ActiveNote = CAMD_INTERPRETER_INVALID;
  channel->mcs_ActiveVelocity = CAMD_INTERPRETER_INVALID;

  amiGusChannel = GetUsedAmiGusChannel( midiChannel );
  StopAmiGusWavetablePlayback( amiGusChannel );
}

VOID AmiGusNoteOn( struct MidiMessage * midiMessage ) {
 
  struct AmiGUS_CAMD * base = AmiGUS_CAMD_Base;
  struct AmiSF * amisf = base->agb_AmiSf;
  UBYTE channelId = midiMessage->mm_Channel;
  struct MidiChannelState * channel =
    &( base->agb_MidiChannelState[ channelId ]);

  UBYTE bank = channel->mcs_Bank;
  UBYTE preset = channel->mcs_Preset;
  UBYTE note = midiMessage->mm_Data0;
  UBYTE velocity = midiMessage->mm_Data1;

  struct AmiSF_Preset * sfPreset = GetAmiSfPreset( amisf, bank, preset );
  LONG noteIndex = GetAmiSfNoteIndex( amisf, sfPreset, note );
  struct AmiSF_Note * sfNote = GetAmiSfNote( amisf, noteIndex );
  struct AmiSF_Sample * sfSample = GetAmiSfSample( amisf, sfNote );

  ULONG playbackRate = GetPlaybackRate( amisf, sfNote, note );
  UWORD playbackVolume = GetPlaybackVolume( velocity );

  if ( !( channel->mcs_IsActive )) {
  
    channel->mcs_AmigusChannelUsed = AllocateNextAmiGusChannel( channelId );
    channel->mcs_IsActive = TRUE;
  }

  channel->mcs_ActiveNote = note;
  channel->mcs_ActiveVelocity = velocity;

  // TODO: Sample handling

  StartAmiGusWavetablePlayback( channel->mcs_AmigusChannelUsed,
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
      //TODO: Handle the message!
      LOG_D(( "D: MIDI message completed, %02lx %02lx %02lx %02lx\n",
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
      //TODO: Handle the message!
      LOG_D(( "D: MIDI message completed, %02lx %02lx %02lx %02lx\n",
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

      LOG_D(( "D: Called NOTE_ON for channel %ld note %ld velocity %ld\n",
              midiMessage->mm_Channel,
              midiMessage->mm_Data0,
              midiMessage->mm_Data1 ));

      AmiGusNoteOn( midiMessage );

      return TRUE;
    }
    case MIDI_POLY_PRESSURE: {
      return FALSE;
    }
    case MIDI_CONTROL_CHANGE: {
      return FALSE;
    }
    case MIDI_PROGRAM_CHANGE: {
      return FALSE;
    }
    case MIDI_CHANNEL_PRESSURE: {
      return FALSE;
    }
    case MIDI_PITCH_BEND: {
      return FALSE;
    }
    case MIDI_SYSTEM_MSG: {
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

UWORD GetPlaybackVolume( UBYTE velocity ) {

  LONG volume = ( WORD ) velocity * ( WORD ) velocity;
  UWORD playbackVolume = ( UWORD )( volume ) + (( UWORD ) velocity << 1 );

  return playbackVolume;
}
