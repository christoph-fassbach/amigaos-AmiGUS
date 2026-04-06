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

LONG PlayNoteMessageName       = PLAY_NOTE_MESSAGE_NAME;
LONG PlayInstrumentMessageName = PLAY_INSTRUMENT_MESSAGE_NAME;
LONG LoadSoundFontMessageName  = LOAD_SOUNDFONT_NESSAGE_NAME;
LONG ReloadSettingsMessageName = RELOAD_SETTINGS_MESSAGE_NAME;

/******************************************************************************
 * MIDI driver helper functions - private functions.
 *****************************************************************************/

APTR CreateAmigusMessage( struct MsgPort * replyPort,
                          ULONG messageSize,
                          LONG * messageName ) {

  // This is clearly larger than a message,
  // but we access only the common part here.
  struct Message * message = AllocVec( messageSize, MEMF_ANY | MEMF_CLEAR );

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
    request.es_TextFormat = "Could not reach AmiGUS CAMD Port.";
    request.es_GadgetFormat = "Retry|Cancel";

    result = EasyRequest( SF_Converter_Base->sfc_MainWindow, &request, NULL );
    if ( !result ) {

      return ENoAmigusRetry;
    }
    result = OpenAmigusPort();
    if ( result ) {

      DisplayError( result );
      return result;
    }

    Forbid();
    port = FindPort( portName );
  }
  if ( !( port )) {

    Permit();
    DisplayError( ENoAmigusRetry );
    return ENoAmigusRetry;
  }

  LOG_D(( "D: Found AmiGUS CAMD Port at 0x%08lx\n", port ));
  PutMsg( port, message );
  Permit();
  return ENoError;
}

struct PlayNoteMessage * CreateAmigusPlayNoteMessage(
  struct MsgPort * replyPort ) {

  struct PlayNoteMessage * message =
    CreateAmigusMessage( replyPort,
                         sizeof( struct PlayNoteMessage ),
                         &( PlayNoteMessageName ));

  return message;
}

struct PlayInstrumentMessage * CreateAmigusPlayInstrumentMessage(
  struct MsgPort * replyPort ) {

  struct PlayInstrumentMessage * message =
    CreateAmigusMessage( replyPort,
                         sizeof( struct PlayInstrumentMessage ),
                         &( PlayInstrumentMessageName ));

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
    case PLAY_NOTE_MESSAGE_NAME: {

      LOG_D(( "D: Deleting PlayNoteMessage 0x%08lx...\n", mess ));
      break;
    }
    case PLAY_INSTRUMENT_MESSAGE_NAME: {

      LOG_D(( "D: Deleting PlayInstrumentMessage 0x%08lx...\n", mess ));
      break;
    }
    case LOAD_SOUNDFONT_NESSAGE_NAME: {

      LOG_D(( "D: Deleting LoadSoundFontMessage 0x%08lx...\n", mess ));
      break;
    }
    case RELOAD_SETTINGS_MESSAGE_NAME: {

      LOG_D(( "D: Deleting ReloadSettingsMessage 0x%08lx...\n", mess ));
      break;
    }
    default: {
      break;
    }
  }
  FreeVec( message );
}
