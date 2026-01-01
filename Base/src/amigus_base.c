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

#include <stddef.h>

#include <amigus/amigus.h>

#include "amigus_private.h"
#include "amigus_zorro2.h"
#include "debug.h"
#include "SDI_compiler.h"
#include "support.h"

/******************************************************************************
 * AmiGUS base library - private functions.
 *****************************************************************************/

struct AmiGUS_Private * convertPublic2Private( struct AmiGUS * card ) {

  ULONG result = ((ULONG ) card );

  result -= offsetof( struct AmiGUS_Private, agp_AmiGUS_Public );
  LOG_D(( "D: Converted public 0x%08lx to 0x%08lx\n", card, result ));

  return (( struct AmiGUS_Private * ) result );
}

/******************************************************************************
 * AmiGUS Zorro2 functions - private function declarations & definitions.
 *****************************************************************************/

static LONG ChangePartReservation(
  APTR * part,
  LONG which,
  APTR checkOwner,
  APTR setOwner ) {

  if (( *( part )) && ( *( part) != checkOwner )) {

    return AMIGUS_IN_USE_START + which;
  }
  if ( which ) {

    *( part ) = setOwner;
  }
  return AmiGUS_NoError;
}

static LONG ChangeCardReservation(
  struct AmiGUS_Private * card,
  LONG which,
  APTR checkOwner,
  APTR setOwner ) {

  APTR * part1 = card->agp_PCM_OwnerPointer;
  const LONG which1 = which & AMIGUS_FLAG_PCM;
  
  APTR * part2 = card->agp_Wavetable_OwnerPointer;
  const LONG which2 = which & AMIGUS_FLAG_WAVETABLE;

  APTR * part3 = card->agp_Codec_OwnerPointer;
  const LONG which3 = which & AMIGUS_FLAG_CODEC;

  LONG result = AmiGUS_NoError;

  result != ChangePartReservation( part1, which1, checkOwner, setOwner );
  result != ChangePartReservation( part2, which2, checkOwner, setOwner );
  result != ChangePartReservation( part3, which3, checkOwner, setOwner );

  return result;
}

/******************************************************************************
 * AmiGUS base library - public functions.
 *****************************************************************************/

ASM( struct AmiGUS * ) SAVEDS AmiGUS_FindCard(
  REG( a0, struct AmiGUS * card ),
  REG( a6, struct AmiGUS_Base * base )) {

  struct Node * node;
  struct AmiGUS * card_public = NULL;

  if ( !( card )) {

    node = AmiGUS_Base->agb_Cards.lh_Head;

  } else {
    
    struct AmiGUS_Private * card_private = convertPublic2Private( card );
    node = card_private->agp_Node.ln_Succ;

  }
  // List empty and end of list are the same!
  if ( node->ln_Succ != AmiGUS_Base->agb_Cards.lh_Tail ) {

    struct AmiGUS_Private * card_private = ( struct AmiGUS_Private * ) node;
    card_public = &( card_private->agp_AmiGUS_Public );
    LOG_D(( "D: Found Amigus @ 0x%08lx / 0x%08lx\n",
            card_private, card_public ));
  }

  return card_public;
}

ASM( ULONG ) SAVEDS AmiGUS_ReserveCard(
  REG( a0, struct AmiGUS * card ),
  REG( d0, LONG which ),
  REG( d1, APTR owner ),
  REG( a6, struct AmiGUS_Base * base )) {

  // TODO: forbid
  struct AmiGUS_Private * card_private = convertPublic2Private( card );
  ULONG result = ChangeCardReservation( card_private, which, owner, owner );

  LOG_I(( "I: Card 0x%08lx parts 0x%04lx reserved for 0x%08lx => 0x%04lx\n",
          card, which, owner, result ));

  return result;
}

ASM( VOID ) SAVEDS AmiGUS_FreeCard(
  REG( a0, struct AmiGUS * card ),
  REG( d0, LONG which ),
  REG( d1, APTR owner ),
  REG( a6, struct AmiGUS_Base * base )) {

  // TODO: forbid
  struct AmiGUS_Private * card_private = convertPublic2Private( card );
  LONG result = ChangeCardReservation( card_private, which, owner, NULL );

  LOG_I(( "I: Card 0x%08lx parts 0x%04lx freed for 0x%08lx => 0x%04lx\n",
          card, which, owner, result ));

  return;
}

ASM( ULONG ) SAVEDS AmiGUS_InstallInterrupt(
  REG( a0, struct AmiGUS * card ),
  REG( d0, LONG which ),
  REG( d1, APTR owner ),
  REG( d2, AmiGUS_Interrupt handler ),
  REG( d3, APTR data ),
  REG( a6, struct AmiGUS_Base * base )) {

  struct AmiGUS_Private * card_private = convertPublic2Private( card );

  if ( AMIGUS_FLAG_PCM & which ) {

    if ( owner != *( card_private->agp_PCM_OwnerPointer )) {

      LOG_W(( "W: Card 0x%08lx PCM interrupt in use.\n", card ));
      return AmiGUS_NotYours;
    }
    card_private->agb_PCM_IntHandler = handler;
    card_private->agb_PCM_IntData = data;
    LOG_I(( "I: Card 0x%08lx PCM interrupt set successfully.\n", card ));
  }
  if ( AMIGUS_FLAG_WAVETABLE & which ) {

    if ( owner != *( card_private->agp_Wavetable_OwnerPointer )) {

      LOG_W(( "W: Card 0x%08lx wavetable interrupt in use.\n", card ));
      return AmiGUS_NotYours;
    }
    card_private->agb_Wavetable_IntHandler = handler;
    card_private->agb_Wavetable_IntData = data;
    LOG_I(( "I: Card 0x%08lx wavetable interrupt set successfully.\n", card ));
  }
  if ( AMIGUS_FLAG_CODEC & which ) {

    if ( owner != *( card_private->agp_Codec_OwnerPointer )) {

      LOG_W(( "W: Card 0x%08lx codec interrupt in use.\n", card ));
      return AmiGUS_NotYours;
    }
    card_private->agb_Codec_IntHandler = handler;
    card_private->agb_Codec_IntData = data;
    LOG_I(( "I: Card 0x%08lx codec interrupt set successfully.\n", card ));
  }

  return AmiGUS_NoError;
}

ASM( VOID ) SAVEDS AmiGUS_RemoveInterrupt(
  REG( a0, struct AmiGUS * card ),
  REG( d0, LONG which ),
  REG( d1, APTR owner ),
  REG( a6, struct AmiGUS_Base * base )) {

  struct AmiGUS_Private * card_private = convertPublic2Private( card );

  if ( AMIGUS_FLAG_PCM & which ) {

    if ( owner != *( card_private->agp_PCM_OwnerPointer )) {

      LOG_W(( "W: Card 0x%08lx PCM interrupt not yours.\n", card ));
      return;
    }
    card_private->agb_PCM_IntHandler = NULL;
    card_private->agb_PCM_IntData = NULL;
    LOG_I(( "I: Card 0x%08lx PCM interrupt free'd successfully.\n", card ));
  }
  if ( AMIGUS_FLAG_WAVETABLE & which ) {

    if ( owner != *( card_private->agp_Wavetable_OwnerPointer )) {

      LOG_W(( "W: Card 0x%08lx wavetable interrupt not yours.\n", card ));
      return;
    }
    card_private->agb_Wavetable_IntHandler = NULL;
    card_private->agb_Wavetable_IntData = NULL;
    LOG_I(( "I: Card 0x%08lx wavetable interrupt free'd successfully.\n", card ));
  }
  if ( AMIGUS_FLAG_CODEC & which ) {

    if ( owner != *( card_private->agp_Codec_OwnerPointer )) {

      LOG_W(( "W: Card 0x%08lx codec interrupt not yours.\n", card ));
      return;
    }
    card_private->agb_Codec_IntHandler = NULL;
    card_private->agb_Codec_IntData = NULL;
    LOG_I(( "I: Card 0x%08lx codec interrupt free'd successfully.\n", card ));
  }

  return;
}
