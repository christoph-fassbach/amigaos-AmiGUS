/*
 * This file is part of the SoundFontConverter.
 *
 * SoundFontConverter is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3 of the License only.
 *
 * SoundFontConverter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SoundFontConverter.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <midi/camd.h>

#include <proto/exec.h>
#include <proto/intuition.h>

#include "amigus_utils.h"

#include "camd_utils.h"
#include "converter.h"
#include "debug.h"
#include "errors.h"
#include "support.h"

/******************************************************************************
 * MIDI driver helper variables - private variables.
 *****************************************************************************/

LONG PlayStandaloneSampleMessageName = PLAY_STANDALONE_MESSAGE_NAME;
LONG PlayAmiSfSampleMessageName      = PLAY_AMISF_SAMPLE_MESSAGE_NAME;
LONG StopSampleMessageName           = STOP_SAMPLE_MESSAGE_NAME;
LONG LoadSoundFontMessageName        = LOAD_SOUNDFONT_NESSAGE_NAME;
LONG ReloadSettingsMessageName       = RELOAD_SETTINGS_MESSAGE_NAME;

/******************************************************************************
 * MIDI driver helper functions - private functions.
 *****************************************************************************/

APTR CreateAmigusMessage( struct MsgPort * replyPort,
                          ULONG messageSize,
                          LONG * messageName ) {

  // This is clearly larger than a message,
  // but we access only the common part here.
  struct Message * message = AllocMem( messageSize, MEMF_ANY | MEMF_CLEAR );

  message->mn_Node.ln_Name = ( STRPTR ) messageName;
  message->mn_ReplyPort = replyPort;

  return message;
}

/******************************************************************************
 * MIDI driver helper functions - public function definitions.
 *****************************************************************************/

ULONG OpenAmigusPort( VOID ) {

  struct List devices;
  struct CAMD_Device_Node * device;

  NEW_LIST( &devices );
  ExtractCamdDevices( &devices, FALSE );
  FOR_LIST( &devices, device, struct CAMD_Device_Node * ) {

    if ( C_strcmp( device->cdn_Name, "amigus" )) {

      LOG_D(( "D: Found %s %s, opening...\n",
              device->cdn_Name, device->cdn_Location ));

      SF_Converter_Base->sfc_MidiNode = OpenMidi( STR( APP_NAME ));
      if ( !( SF_Converter_Base->sfc_MidiNode )) {

        return ECreateMidi;
      }
      SF_Converter_Base->sfc_MidiLink = OpenMidiOutput(
        SF_Converter_Base->sfc_MidiNode,
        device->cdn_Location,
        STR( APP_NAME )" Link" );
      if ( !( SF_Converter_Base->sfc_MidiLink )) {

        return EAddMidiLink;
      }

      return ENoError;
    }
  }
  return ENoAmigusFound;
}

LONG SendAmigusMessage( struct Message * message ) {

  CONST_STRPTR portName = "AmiGUS CAMD Port";
  struct MsgPort * port;

  Forbid();
  port = FindPort( portName );
  if ( !( port )) {

    LONG result;
    struct EasyStruct request;

    Permit();

    request.es_StructSize = sizeof( struct EasyStruct );
    request.es_Flags = 0;
    request.es_Title = "Error";
    request.es_TextFormat = "Could not reach AmiGUS CAMD Port.\n"
                            "Shall I retry and open AmiGUS CAMD for that?\n"
                            "This will prevent using another applications,\n"
                            "e.g. CAMDkeyboard, for testing the outcomes...";
    request.es_GadgetFormat = "Retry|Cancel";

    result = EasyRequest( SF_Converter_Base->sfc_MainWindow, &request, NULL );
    if ( !result ) {

      LOG_I(( "I: Not retrying...\n" ));
      DeleteAmigusMessage( message );
      return ENoAmigusNoRetry;
    }

    result = OpenAmigusPort();
    if ( result ) {

      LOG_I(( "I: Failed opening AmiGUS port...\n" ));
      DeleteAmigusMessage( message );
      DisplayError( result );
      return result;
    }

    Forbid();
    port = FindPort( portName );
  }
  if ( !( port )) {

    Permit();
    LOG_I(( "I: AmiGUS port not found...\n" ));
    DeleteAmigusMessage( message );
    DisplayError( ENoAmigusRetry );
    return ENoAmigusRetry;
  }

  LOG_D(( "D: Found AmiGUS CAMD Port at 0x%08lx\n", port ));
  PutMsg( port, message );
  Permit();
  return ENoError;
}

struct PlayStandaloneSampleMessage * CreateAmigusPlayStandaloneSampleMessage(
  struct MsgPort * replyPort,
  UWORD attack,
  UWORD decay,
  UWORD sustain,
  UWORD release,
  ULONG playbackRate,
  UWORD playbackVolume,
  UWORD playbackFlags,
  ULONG loopOffset,
  ULONG endOffset,
  APTR data ) {

  struct PlayStandaloneSampleMessage * message =
    CreateAmigusMessage( replyPort,
                         sizeof( struct PlayStandaloneSampleMessage ),
                         &( PlayStandaloneSampleMessageName ));

  message->pssm_Attack = attack;
  message->pssm_Decay = decay;
  message->pssm_Sustain = sustain;
  message->pssm_Release = release;
  message->pssm_PlaybackRate = playbackRate;
  message->pssm_PlaybackVolume = playbackVolume;
  message->pssm_PlaybackFlags = playbackFlags;
  message->pssm_LoopOffset = loopOffset;
  message->pssm_EndOffset = endOffset;
  message->pssm_SampleData = data;

  return message;
}

struct PlayAmiSfSampleMessage * CreateAmigusPlayAmiSfSampleMessage(
  struct MsgPort * replyPort,
  struct AmiSF * amisf,
  UBYTE bank,
  UBYTE preset,
  UBYTE note,
  UBYTE velocity,
  APTR data ) {

  struct PlayAmiSfSampleMessage * message =
    CreateAmigusMessage( replyPort,
                         sizeof( struct PlayAmiSfSampleMessage ),
                         &( PlayAmiSfSampleMessageName ));

  message->passm_AmiSf = amisf;
  message->passm_Bank = bank;
  message->passm_Preset = preset;
  message->passm_Note = note;
  message->passm_Velocity = velocity;
  message->passm_SampleData = data;

  return message;
}

struct StopSampleMessage * CreateAmigusStopSampleMessage(
  struct MsgPort * replyPort,
  APTR playMessage ) {

  struct StopSampleMessage * message =
    CreateAmigusMessage( replyPort,
                         sizeof( struct StopSampleMessage ),
                         &( StopSampleMessageName ));
  // Only meant for finding WHAT to stop - never to be accessed!
  message->ssm_PlayMessage = playMessage;

  return message;
}

struct LoadSoundFontMessage * CreateAmigusLoadSoundFontMessage(
  struct MsgPort * replyPort ) {

  struct LoadSoundFontMessage * message =
    CreateAmigusMessage( replyPort,
                         sizeof( struct LoadSoundFontMessage ),
                         &( LoadSoundFontMessageName ));

  return message;
}

struct ReloadSettingsMessage * CreateAmigusReloadSettingsMessage(
  struct MsgPort * replyPort ) {

  struct ReloadSettingsMessage * message =
    CreateAmigusMessage( replyPort,
                         sizeof( struct ReloadSettingsMessage ),
                         &( ReloadSettingsMessageName ));

  return message;
}

VOID DeleteAmigusMessage( APTR message ) {

  struct Message * mess = (struct Message * ) message;

  switch ( *(( LONG * ) mess->mn_Node.ln_Name )) {
    case PLAY_STANDALONE_MESSAGE_NAME: {

      struct PlayStandaloneSampleMessage * sampleMessage =
        ( struct PlayStandaloneSampleMessage * ) message;
      ULONG size = sampleMessage->pssm_EndOffset;
      LOG_D(( "D: Deleting PlayStandaloneSampleMessage 0x%08lx...\n", mess ));
      FreeMem( sampleMessage->pssm_SampleData, size );
      FreeMem( sampleMessage, sizeof( struct PlayStandaloneSampleMessage ));
      break;
    }
    case PLAY_AMISF_SAMPLE_MESSAGE_NAME: {
      struct PlayAmiSfSampleMessage * sampleMessage =
        ( struct PlayAmiSfSampleMessage * ) message;

      const UBYTE bankIndex = sampleMessage->passm_Bank;
      const UBYTE presetIndex = sampleMessage->passm_Preset;

      struct AmiSF * amisf = sampleMessage->passm_AmiSf;
      struct AmiSF_Preset * preset = GetAmiSfPreset( amisf,
                                                     bankIndex,
                                                     presetIndex );
      const LONG noteIndex = GetAmiSfNoteIndex( amisf,
                                                preset,
                                                sampleMessage->passm_Note );
      struct AmiSF_Note * note = GetAmiSfNote( amisf, noteIndex );
      struct AmiSF_Sample * sample = GetAmiSfSample( amisf, note );

      ULONG size = sample->asfs_EndOffset - sample->asfs_StartOffset;

      LOG_D(( "D: Deleting PlayAmiSfSampleMessage 0x%08lx...\n", mess ));
      FreeMem( sampleMessage->passm_SampleData, size );
      FreeMem( sampleMessage, sizeof( struct PlayAmiSfSampleMessage ));
      break;
    }
    case STOP_SAMPLE_MESSAGE_NAME: {

      LOG_D(( "D: Deleting StopSampleMessage 0x%08lx...\n", mess ));
      FreeMem( message, sizeof( struct StopSampleMessage ));
      break;
    }
    case LOAD_SOUNDFONT_NESSAGE_NAME: {

      LOG_D(( "D: Deleting LoadSoundFontMessage 0x%08lx...\n", mess ));
      FreeMem( message, sizeof( struct LoadSoundFontMessage ));
      break;
    }
    case RELOAD_SETTINGS_MESSAGE_NAME: {

      LOG_D(( "D: Deleting ReloadSettingsMessage 0x%08lx...\n", mess ));
      FreeMem( message, sizeof( struct ReloadSettingsMessage ));
      break;
    }
    default: {
      LOG_E(( "E: Cannot delete message 0x%08lx of unknown type %s!\n",
              mess,
              mess->mn_Node.ln_Name ));
      break;
    }
  }
}
