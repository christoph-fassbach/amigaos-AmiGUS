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

#include <amigus/amigus.h>

#include <proto/expansion.h>
#include <proto/exec.h>
#include <limits.h>

#include "amigus_hardware.h"
#include "amigus_private.h"
#include "debug.h"
#include "errors.h"
#include "support.h"

#define ANY_PRODUCT_ID   -1

STRPTR AmiGUS_Zorro2_Name = "AmiGUS Zorro2";

LONG HandleProduct(
  struct AmiGUS * card,
  ULONG * which,
  ULONG * own,
  struct ConfigDev * configDevice,
  const LONG flag ) {

  Forbid();
  if ( flag & *( own )) {
    if ( configDevice->cd_Driver ) {

      LOG_W(( "W: AmiGUS product at 0x%08lx in use\n",
              configDevice->cd_BoardAddr ));
      Permit();
      return ( AmiGUS_DetectError | flag );
    }
    *( own ) &= ~flag;
    configDevice->cd_Driver = card;
    // TODO: reserve in internal structs here!
    LOG_I(( "I: AmiGUS product at 0x%08lx reserved\n",
            configDevice->cd_BoardAddr ));
  }
  Permit();
  if ( flag & *( which )) {

    *( which ) &= ~flag;
    switch ( flag ) {
      case AMIGUS_FLAG_PCM: {

        card->agus_PcmBase = configDevice->cd_BoardAddr;
        break;
      }
      case AMIGUS_FLAG_WAVETABLE: {

        card->agus_WavetableBase = configDevice->cd_BoardAddr;
        break;
      }
      case AMIGUS_FLAG_CODEC: {

        card->agus_CodecBase = configDevice->cd_BoardAddr;
        break;
      }
      default: {

        LOG_E(( "E: Flag 0x%04lx not understood!\n", flag ));
        break;
      }
    }
    card->agus_TypeId = AmiGUS_Zorro2;
    card->agus_TypeName = AmiGUS_Zorro2_Name;
    card->agus_FirmwareRev = MIN( card->agus_FirmwareRev,
                                  configDevice->cd_Rom.er_SerialNumber );
    LOG_I(( "I: AmiGUS product info at 0x%08lx retrieved\n",
            configDevice->cd_BoardAddr ));
  }
  return AmiGUS_NoError;
}

/******************************************************************************
 * AmiGUS Zorro2 functions - public function definitions.
 *****************************************************************************/

LONG AmiGusZorro2_Alloc( struct AmiGUS * card, ULONG which, ULONG own ) {

  LONG result = AmiGUS_NoError;
  struct ConfigDev * configDevice = NULL;
  ULONG remainingWhich = which;
  ULONG remainingOwn = own;

  card->agus_FirmwareRev = ULONG_MAX;
  while ( remainingOwn | remainingWhich ) {

    configDevice = FindConfigDev( configDevice,
                                  AMIGUS_MANUFACTURER_ID,
                                  ANY_PRODUCT_ID );
    if ( !configDevice ) {

      LOG_I(( "I: No more AmiGUS' found\n" ));
      return MAX( result, AmiGUS_NotFound );
    }
    if ( AMIGUS_MANUFACTURER_ID != configDevice->cd_Rom.er_Manufacturer ) {

      LOG_E(( "E: AmiGUS detection failed\n" ));
      return EAmiGUSDetectError;
    }
    switch ( configDevice->cd_Rom.er_Product ) {

      case AMIGUS_MAIN_PRODUCT_ID: {

        LOG_I(( "I: AmiGUS PCM at 0x%08lx found\n",
                configDevice->cd_BoardAddr ));
        result |= HandleProduct(
          card,
          &remainingWhich,
          &remainingOwn,
          configDevice,
          AMIGUS_FLAG_PCM );
        break;
      }
      case AMIGUS_HAGEN_PRODUCT_ID: {

        LOG_I(( "I: AmiGUS WaveTable at 0x%08lx found\n",
                configDevice->cd_BoardAddr ));
        result |= HandleProduct(
          card,
          &remainingWhich,
          &remainingOwn,
          configDevice,
          AMIGUS_FLAG_WAVETABLE );
        break;
      }
      case AMIGUS_CODEC_PRODUCT_ID: {

        LOG_I(( "I: AmiGUS Codec at 0x%08lx found\n",
                configDevice->cd_BoardAddr ));
        result |= HandleProduct(
          card,
          &remainingWhich,
          &remainingOwn,
          configDevice,
          AMIGUS_FLAG_CODEC );
        break;
      }
      default: {

        LOG_D(( "D: Other 0x0ADE hardware found, product 0x02lx\n",
                configDevice->cd_Rom.er_Product ));
        break;
      }
    }
  }
  LOG_V(("I: AmiGUS firmware 0x%08lx\n", card->agus_FirmwareRev));
  if ( ULONG_MAX > card->agus_FirmwareRev ) {

    const ULONG serial = card->agus_FirmwareRev;
    card->agus_Minute = ( UBYTE )(( serial & 0x0000003Ful )       );
    card->agus_Hour   = ( UBYTE )(( serial & 0x000007C0ul ) >>  6 );
    card->agus_Day    = ( UBYTE )(( serial & 0x0000F800ul ) >> 11 );
    card->agus_Month  = ( UBYTE )(( serial & 0x000F0000ul ) >> 16 );
    card->agus_Year   = ( UWORD )(( serial & 0xFFF00000ul ) >> 20 );
  }
  LOG_I(( "I: AmiGUS firmware date %04ld-%02ld-%02ld, %02ld:%02ld\n",
          card->agus_Year,
          card->agus_Month,
          card->agus_Day,
          card->agus_Hour,
          card->agus_Minute ));
  return result;
}
