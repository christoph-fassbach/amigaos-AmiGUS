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
