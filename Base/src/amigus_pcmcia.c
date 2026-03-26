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

#include <resources/card.h>

#include <proto/cardres.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include <amigus/amigus.h>

#include "amigus_hardware.h"
#include "amigus_pcmcia.h"
#include "amigus_private.h"
#include "debug.h"
#include "errors.h"
#include "support.h"
#include "SDI_amigus_protos.h"

// https://github.com/torvalds/linux/blob/v6.19/arch/m68k/include/asm/amigahw.h
// https://github.com/torvalds/linux/blob/v6.19/arch/m68k/include/asm/amigayle.h
// https://github.com/torvalds/linux/blob/v6.19/arch/m68k/include/asm/amipcmcia.h
// https://github.com/torvalds/linux/blob/v4.0/arch/m68k/amiga/pcmcia.c

/*
 * defines are limited to 32 chars due to a SAS/C insufficiency !!!
 *
 * So define below is just kind of a ruler...
 */
#define SASC_MAXIMUM_DEFINE_LENGTH_IS_32 12345678
#define GAYLE_CONFIG_BASE_ADDRESS        0x00DA8000
#define GAYLE_CONFIG_MAX_ADDRESS         0x00DAFFFF

#define GAYLE_REG_ASK_HEDLEY             0x1000

#define GAYLE_PCMCIA_ASK_HEDLEY_RESET    0x0300


#define CHAR_TO_ULONG( a, b, c, d )      ((( a ) << 24 ) | (( b ) << 16 ) | (( c ) << 8 ) | ( d ))
#define AMIGUS_MINI_CARD_ID_LOW          CHAR_TO_ULONG( 'A', 'M' ,'I', 'G' )
#define AMIGUS_MINI_CARD_ID_HIGH         CHAR_TO_ULONG( 'U', 'S' ,'M', 'N' )

extern const char LibName[];

STRPTR AmiGUS_Mini_Name = "AmiGUS mini";

static VOID ResetPcmcia( VOID ) {

  UWORD value;
  
  LOG_I(( "I: Triggering PCMCIA reset.\n" ));

  value = ReadReg16(( APTR ) GAYLE_CONFIG_BASE_ADDRESS,
                             GAYLE_REG_ASK_HEDLEY );

  WriteReg16(( APTR ) GAYLE_CONFIG_BASE_ADDRESS,
              GAYLE_REG_ASK_HEDLEY,
              value | GAYLE_PCMCIA_ASK_HEDLEY_RESET );
  WriteReg16(( APTR ) GAYLE_CONFIG_BASE_ADDRESS,
              GAYLE_REG_ASK_HEDLEY,
              value );
}

ASM( LONG ) /* __entry for vbcc ? */ SAVEDS INTERRUPT HandleRemovedInterrupt (
  REG( a1, struct AmiGUS_Base * base )) {

  LOG_INT(( "INT: PCMCIA removed.\n" ));
//  struct CardHandle * handle = &( base->agb_CardHandle );
//  CardResetCard( handle );
//  ReleaseCard( handle, 0 );
  LOG_D(("1\n"));

  return 0;
}

ASM( LONG ) /* __entry for vbcc ? */ SAVEDS INTERRUPT HandleInsertedInterrupt (
  REG( a1, struct AmiGUS_Base * base )) {

  LOG_INT(( "INT: PCMCIA inserted.\n" ));
  // Test if Amigus 
  LOG_D(("2\n"));

  return 0;
}

ASM( ULONG ) /* __entry for vbcc ? */ SAVEDS INTERRUPT HandleStatusInterrupt (
  REG( d0, ULONG status ),
  REG( a1, struct AmiGUS_Base * base )) {

  LOG_INT(( "INT: PCMCIA status change, now %ld.\n", status ));
  if ( CARD_STATUSF_IRQ & status ) {

    HandleInterrupt( base );
  }
  LOG_D(("3\n"));
  return status;
}

VOID AmiGusPcmcia_AddAll( struct List * cards ) {

  struct AmiGUS_Base * base = AmiGUS_Base;
  struct CardHandle * handle = &( base->agb_CardHandle );
  struct AmiGUS_Private * card_private;
  struct AmiGUS * card_public;
  struct CardMemoryMap * cardMap;
  ULONG cardBaseLong;
  APTR cardBasePtr;
  union { ULONG idLong[ 2 ];
          UBYTE idString[ 9 ];
        } id;
  ULONG interface;
  UBYTE status;
  APTR own;
  BOOL reset;
  ULONG serial;

  if ( !( base->agb_CardResource )) {

    LOG_I(( "I: No card.resource, not looking further on PCMCIA.\n" ));
    return;
  }
  LOG_I(( "I: Found card.resource %ld.%ld.\n",
          base->agb_CardResource->lib_Version,
          base->agb_CardResource->lib_Revision ));

  interface = CardInterface();
  if ( CARD_INTERFACE_AMIGA_0	!= interface ) {

    LOG_I(( "I: Unknown PCMCIA interface, not looking further.\n" ));
    return;
  }

  ResetPcmcia();

  status = ReadCardStatus();
  if ( !( CARD_STATUSF_CCDET & status )) {

    LOG_I(( "I: No card inserted.\n" ));
    return;
  }

  handle->cah_CardNode.ln_Pri = 20;
  handle->cah_CardNode.ln_Name = ( char * ) LibName;
  handle->cah_CardRemoved->is_Data = ( APTR ) base;
  handle->cah_CardRemoved->is_Code = ( VOID ( * )( )) HandleRemovedInterrupt;
  handle->cah_CardInserted->is_Data = ( APTR ) base;
  handle->cah_CardInserted->is_Code = ( VOID ( * )( )) HandleInsertedInterrupt;
  handle->cah_CardStatus->is_Data = ( APTR ) base;
  handle->cah_CardStatus->is_Code = ( VOID ( * )( )) HandleStatusInterrupt;
  handle->cah_CardFlags = CARDF_RESETREMOVE;

  LOG_D(( "D: Requesting PCMCIA card ownership for 0x%08lx.\n", handle ));

  own = OwnCard( handle );
  if ( NULL == own ) {

    LOG_D(( "D: Success, owning card!\n" ));
  }
  /*
  DO NOT TRY THIS AT HOME - CRASHES ON RELEASE!
  resultHandle = OwnCard( handle );
  if ( !( 0 > ( LONG ) resultHandle )) {

    LOG_D(( "D: PCMCIA card owned by 0x%08lx, owner %s.\n",
            resultHandle, resultHandle->cah_CardNode.ln_Name ));
  }
  if ( resultHandle != handle ) {

    LOG_E(( "E: Not owning card!\n" ));
    return;
  }
    */
  
  reset = CardResetCard( handle );
  if ( !( reset )) {

    LOG_E(( "E: Card reset failed!\n" ));
    ReleaseCard( handle, 0 );
    return;
  }
  Delay( 3 );
// TODO: Only if nothing is mapped in mem area!
/*
  x = CardAccessSpeed( handle, 100 );
	if ( !( x )) {	

    LOG_E(( "E: Card access speed failed!\n" ));
    ReleaseCard( handle, 0 );
    return;
	}
  Delay( 3 );
*/
  // TODO: Only if nothing is mapped in mem area!
//begincardaccess / endcardaccess

  cardMap = GetCardMap(); // <- has addresses and sizes and shit
  LOG_V(( "V: Addresses: Mem 0x%08lx Attr 0x%08lx IO 0x%08lx\n",
          cardMap->cmm_CommonMemory,
          cardMap->cmm_AttributeMemory,
          cardMap->cmm_IOMemory ));

  if ( 39 <= base->agb_CardResource->lib_Version ) {

    LOG_V(( "V: Sizes:     Mem 0x%08lx Attr 0x%08lx IO 0x%08lx\n",
            cardMap->cmm_CommonMemSize,
            cardMap->cmm_AttributeMemSize,
            cardMap->cmm_IOMemSize ));
  }
  
// todo: only if something is mapped in mem area
  cardBasePtr = cardMap->cmm_AttributeMemory;
  cardBaseLong = ( ULONG ) cardBasePtr;
  id.idLong[ 0 ] = ReadReg32( cardBasePtr, 0x80 );
  id.idLong[ 1 ] = ReadReg32( cardBasePtr, 0x84 );
  id.idString[ 8 ] = 0;
  LOG_I(( "I: Found card %s.\n", id.idString ));
  if (( AMIGUS_MINI_CARD_ID_LOW  != id.idLong[ 0 ] ) ||
      ( AMIGUS_MINI_CARD_ID_HIGH != id.idLong[ 1 ] )) {

    LOG_E(( "E: No %s found!\n", AmiGUS_Mini_Name ));
    ReleaseCard( handle, CARDB_REMOVEHANDLE );
    return;
  }

  card_private = AllocMem( sizeof( struct AmiGUS_Private ), MEMF_ANY );

  card_private->agp_PCM.agp_OwnerPointer =
    &( card_private->agp_PCM.agp_MaybeOwnerData );
  card_private->agp_PCM.agp_MaybeOwnerData = NULL;
  card_private->agp_PCM.agp_IntHandler = NULL;
  card_private->agp_PCM.agp_IntData = NULL;

  card_private->agp_Wavetable.agp_OwnerPointer =
    &( card_private->agp_Wavetable.agp_MaybeOwnerData );
  card_private->agp_Wavetable.agp_MaybeOwnerData = NULL;
  card_private->agp_Wavetable.agp_IntHandler = NULL;
  card_private->agp_Wavetable.agp_IntData = NULL;

  card_private->agp_Codec.agp_OwnerPointer =
    &( card_private->agp_Codec.agp_MaybeOwnerData );
  card_private->agp_Codec.agp_MaybeOwnerData = NULL;
  card_private->agp_Codec.agp_IntHandler = NULL;
  card_private->agp_Codec.agp_IntData = NULL;

  card_public = &( card_private->agp_AmiGUS_Public );
  card_public->agus_PcmBase = ( APTR )( cardBaseLong + 0x00000100 );
  card_public->agus_WavetableBase = ( APTR )( cardBaseLong + 0x00000200 );
  card_public->agus_CodecBase = ( APTR )( cardBaseLong + 0x00000300 );
  card_public->agus_TypeId = AmiGUS_mini;
  card_public->agus_TypeName = AmiGUS_Mini_Name;

  serial = ReadReg32( cardBasePtr, 0x88 );
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

  return;
}

VOID AmiGusPcmcia_RemoveAll( struct List * cards ) {

  struct AmiGUS_Base * base = AmiGUS_Base;
  struct CardHandle * handle = &( base->agb_CardHandle );
  struct AmiGUS_Private * card_private; 
  FOR_LIST( cards, card_private, struct AmiGUS_Private * ) {

    if ( AmiGUS_mini == card_private->agp_AmiGUS_Public.agus_TypeId ) {

      LOG_I(( "I: Resetting %s...\n", AmiGUS_Mini_Name ));
      CardResetCard( handle );
      Delay( 3 );
      LOG_I(( "I: done, releasing %s...\n", AmiGUS_Mini_Name ));
      Delay( 3 );
      ReleaseCard( handle, CARDB_REMOVEHANDLE );
      LOG_I(( "I: done, %s free'd!\n", AmiGUS_Mini_Name ));
    }
  }
}

LONG AmiGusPcmcia_InstallInterrupt( VOID ) {

  LOG_I(( "I: PCMCIA ints handled permanently, no install needed.\n" ));

  return AmiGUS_NoError;
}

LONG AmiGusPcmcia_RemoveInterrupt( VOID ) {

  LOG_I(( "I: PCMCIA ints handled permanently, no remove possible.\n" ));

  return AmiGUS_NoError;
}
