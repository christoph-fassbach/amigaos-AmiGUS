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

#include <proto/expansion.h>

#include "amigus_camd.h"
#include "amigus_hardware.h"
#include "amigus_wavetable.h"
#include "debug.h"
#include "errors.h"
#include "support.h"

/******************************************************************************
 * Wavetable convenience functions - public function definitions.
 *****************************************************************************/

LONG FindAmiGusWavetable( struct ConfigDev ** device ) {

  LONG result = EAmiGUSNotFound;
  struct ConfigDev * configDevice = NULL;

  ( * device ) = NULL;
  while ( !( * device )) {

    ULONG serial;

    configDevice = FindConfigDev( configDevice,
                                  AMIGUS_MANUFACTURER_ID,
                                  AMIGUS_HAGEN_PRODUCT_ID );
    if ( !configDevice ) {

      LOG_E(( "E: No more AmiGUS' found\n" ));
      break;
    }
    if (( AMIGUS_MANUFACTURER_ID != configDevice->cd_Rom.er_Manufacturer )
      || ( AMIGUS_HAGEN_PRODUCT_ID != configDevice->cd_Rom.er_Product )) {

      LOG_E(( "E: AmiGUS detection failed\n" ));
      return EAmiGUSDetectError;
    }
    if ( configDevice->cd_Driver ) {

      LOG_E(( "E: AmiGUS at 0x%08lx in use\n", ( * device )->cd_BoardAddr ));
      result = EDriverInUse;
      continue;
    }
    serial = configDevice->cd_Rom.er_SerialNumber;
    if ( AMIGUS_CAMD_FIRMWARE_MINIMUM > serial ) {

      LOG_E(( "E: AmiGUS firmware expected %08lx, actual %08lx at 0x%08lx\n",
              AMIGUS_CAMD_FIRMWARE_MINIMUM,
              serial,
              ( * device )->cd_BoardAddr ));
      result = EAmiGUSFirmwareOutdated;
      continue;
    }
    LOG_V(( "V: AmiGUS firmware %08lx\n", serial ));
    LOG_I(( "I: AmiGUS firmware date %04ld-%02ld-%02ld, %02ld:%02ld\n",
            ( UWORD )(( serial & 0xFFF00000ul ) >> 20 ) /* year     */,
            ( UBYTE )(( serial & 0x000F0000ul ) >> 16 ) /* month    */,
            ( UBYTE )(( serial & 0x0000F800ul ) >> 11 ) /* day      */,
            ( UBYTE )(( serial & 0x000007C0ul ) >>  6 ) /* hour     */,
            ( UBYTE )(( serial & 0x0000003Ful )       ) /* minute   */  ));
    
    result = ENoError;
    ( * device ) = configDevice;

    LOG_I(( "I: AmiGUS found at 0x%08lx\n", ( * device )->cd_BoardAddr ));
    LOG_V(( "V: AmiGUS address stored at 0x%08lx\n", device ));
  }

  return result;
}
