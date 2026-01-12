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

#include <hardware/intbits.h>

#include <proto/expansion.h>
#include <proto/exec.h>

#include "amigus_hardware.h"
#include "amigus_private.h"
#include "amigus_zorro2.h"
#include "debug.h"
#include "errors.h"
#include "support.h"

#define ANY_PRODUCT_ID   -1

STRPTR AmiGUS_Zorro2_Name = "AmiGUS Zorro2";

/******************************************************************************
 * AmiGUS Zorro2 functions - public function definitions.
 *****************************************************************************/

VOID AmiGusZorro2_AddAll( struct List * cards ) {

  struct ConfigDev * cd_PCM = NULL;
  struct ConfigDev * cd_Wavetable = NULL;
  struct ConfigDev * cd_Codec = NULL;
  
  for ( ; ; ) {

    struct AmiGUS_Private * card_private;
    struct AmiGUS * card_public;
    ULONG serial_PCM;
    ULONG serial_Wavetable;
    ULONG serial_Codec;
    ULONG serial;

    cd_PCM = FindConfigDev( cd_PCM,
                            AMIGUS_MANUFACTURER_ID,
                            AMIGUS_MAIN_PRODUCT_ID );
    if ( !( cd_PCM )) {

      break;
    }
    cd_Wavetable = FindConfigDev( cd_Wavetable,
                                  AMIGUS_MANUFACTURER_ID,
                                  AMIGUS_HAGEN_PRODUCT_ID );
    if ( !( cd_Wavetable )) {

      break;
    }
    cd_Codec = FindConfigDev( cd_Codec,
                              AMIGUS_MANUFACTURER_ID,
                              AMIGUS_CODEC_PRODUCT_ID );
    if ( !( cd_Codec )) {

      break;
    }

    card_private = AllocMem( sizeof( struct AmiGUS_Private ), MEMF_ANY );

    card_private->agp_PCM.agp_OwnerPointer = &( cd_PCM->cd_Driver );
    card_private->agp_PCM.agp_IntHandler = NULL;
    card_private->agp_PCM.agp_IntData = NULL;

    card_private->agp_Wavetable.agp_OwnerPointer = &( cd_Wavetable->cd_Driver );
    card_private->agp_Wavetable.agp_IntHandler = NULL;
    card_private->agp_Wavetable.agp_IntData = NULL;

    card_private->agp_Codec.agp_OwnerPointer = &( cd_Codec->cd_Driver );
    card_private->agp_Codec.agp_IntHandler = NULL;
    card_private->agp_Codec.agp_IntData = NULL;

    card_public = &( card_private->agp_AmiGUS_Public );
    card_public->agus_PcmBase = cd_PCM->cd_BoardAddr;
    card_public->agus_WavetableBase = cd_Wavetable->cd_BoardAddr;
    card_public->agus_CodecBase = cd_Codec->cd_BoardAddr;
    card_public->agus_TypeId = AmiGUS_Zorro2;
    card_public->agus_TypeName = AmiGUS_Zorro2_Name;

    serial_PCM = cd_PCM->cd_Rom.er_SerialNumber;
    serial_Wavetable = cd_Wavetable->cd_Rom.er_SerialNumber;
    serial_Codec = cd_Codec->cd_Rom.er_SerialNumber;

    if (( serial_PCM != serial_Wavetable )
      || ( serial_PCM != serial_Codec )) {

      LOG_W(( "W: Versions 0x%08lx/0x%08lx/0x%08lx of AmiGUSs "
              "combined into 0x%08lx/0x%08lx do not match!\n",
              serial_PCM, serial_Wavetable, serial_Codec,
              card_private, card_public ));
    }

    serial = MIN( serial_PCM, MIN( serial_Wavetable, serial_Codec ));
    card_public->agus_FirmwareRev = serial;
    LOG_V(("I: AmiGUS firmware 0x%08lx\n", serial ));

    card_public->agus_Minute = ( UBYTE )(( serial & 0x0000003Ful )       );
    card_public->agus_Hour   = ( UBYTE )(( serial & 0x000007C0ul ) >>  6 );
    card_public->agus_Day    = ( UBYTE )(( serial & 0x0000F800ul ) >> 11 );
    card_public->agus_Month  = ( UBYTE )(( serial & 0x000F0000ul ) >> 16 );
    card_public->agus_Year   = ( UWORD )(( serial & 0xFFF00000ul ) >> 20 );
    LOG_I(( "I: AmiGUS firmware date %04ld-%02ld-%02ld, %02ld:%02ld\n",
            card_public->agus_Year, 
            card_public->agus_Month,
            card_public->agus_Day,
            card_public->agus_Hour,
            card_public->agus_Minute ));

    AddTail( cards, &( card_private->agp_Node ));
  }
  return;
}

VOID AmiGusZorro2_InstallInterrupt( VOID ) {

  Disable();
  if ( AmiGUS_Base->agb_Flags & AMIGUS_BASE_F_ZORRO2_INT_SET ) {

    Enable();
    LOG_I(( "I: Interrupt already installed.\n" ));
    return;
  }
  AmiGUS_Base->agb_Flags |= AMIGUS_BASE_F_ZORRO2_INT_SET;
  AddIntServer( INTB_PORTS, &( AmiGUS_Base->agb_Interrupt ));

  Enable();
  LOG_I(( "I: Interrupt successfully installed.\n" ));
  return;
}

VOID AmiGusZorro2_RemoveInterrupt( VOID ) {

  Disable();
  if ( !( AmiGUS_Base->agb_Flags & AMIGUS_BASE_F_ZORRO2_INT_SET )) {

    Enable();
    LOG_I(( "I: Interrupt not installed.\n" ));
    return;
  }
  AmiGUS_Base->agb_Flags &= ~AMIGUS_BASE_F_ZORRO2_INT_SET;
  RemIntServer( INTB_PORTS, &( AmiGUS_Base->agb_Interrupt ));

  Enable();
  LOG_I(( "I: Interrupt successfully removed.\n" ));
  return;
}
