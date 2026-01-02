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
 * AmiGUS base library - private function declarations & definitions.
 *****************************************************************************/

static struct AmiGUS_Private * convertPublic2Private( struct AmiGUS * card ) {

  ULONG result = ((ULONG ) card );

  result -= offsetof( struct AmiGUS_Private, agp_AmiGUS_Public );
  LOG_D(( "D: Converted public 0x%08lx to 0x%08lx\n", card, result ));

  return (( struct AmiGUS_Private * ) result );
}

static LONG ChangePartReservation(
  APTR * ownerPointer,
  LONG which,
  APTR checkOwner,
  APTR setOwner ) {

  LOG_V(( "D: Old owner is 0x%08lx, check 0x%08lx, set 0x%08lx\n",
          *( ownerPointer ), checkOwner, setOwner ));
  if ( !( which )) {

    return AmiGUS_NoError;
  }
  if (( *( ownerPointer )) && ( *( ownerPointer) != checkOwner )) {

    return AMIGUS_IN_USE_START + which;
  }

  *( ownerPointer ) = setOwner;
  LOG_V(( "D: Now owner is 0x%08lx\n", *( ownerPointer )));

  return AmiGUS_NoError;
}

static LONG ChangeCardReservation(
  struct AmiGUS_Private * card,
  LONG which,
  APTR checkOwner,
  APTR setOwner ) {

  LONG result = AmiGUS_NoError;

  result |= ChangePartReservation( card->agp_PCM.agp_OwnerPointer,
                                   ( which & AMIGUS_FLAG_PCM ),
                                   checkOwner,
                                   setOwner );
  result |= ChangePartReservation( card->agp_Wavetable.agp_OwnerPointer,
                                   ( which & AMIGUS_FLAG_WAVETABLE ),
                                   checkOwner,
                                   setOwner );
  result |= ChangePartReservation( card->agp_Codec.agp_OwnerPointer,
                                   ( which & AMIGUS_FLAG_CODEC ),
                                   checkOwner,
                                   setOwner );

  return result;
}

static LONG ChangePartInterrupt(
  struct AmiGUS_Part * part,
  const LONG which,
  const LONG flag,
  const APTR owner,
  AmiGUS_Interrupt handler,
  APTR data ) {

  if ( !( flag & which )) {

    return AmiGUS_NoError;
  }
  if ( owner != *( part-> agp_OwnerPointer )) {

    LOG_W(( "W: Part 0x%08lx for 0x%04lx not yours\n",
            part, which ));
    return AmiGUS_NotYours;
  }

  part->agp_IntHandler = handler;
  part->agp_IntData = data;
  LOG_I(( "I: Set int for part 0x%08lx for 0x%04lx successfully\n",
          part, which ));

  return AmiGUS_NoError;
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
  // TODO: Install interrupt here!
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
  // TODO: Free interrupt here!
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
  ULONG result = AmiGUS_NoError;

  LOG_I(( "I: Setting interrupts for card 0x%08lx\n", card ));
  result |= ChangePartInterrupt( &( card_private->agp_PCM ),
                                 which,
                                 AMIGUS_FLAG_PCM,
                                 owner,
                                 handler,
                                 data );
  result |= ChangePartInterrupt( &( card_private->agp_Wavetable ),
                                 which,
                                 AMIGUS_FLAG_WAVETABLE,
                                 owner,
                                 handler,
                                 data );
  result |= ChangePartInterrupt( &( card_private->agp_Codec ),
                                 which,
                                 AMIGUS_FLAG_CODEC,
                                 owner,
                                 handler,
                                 data );
  LOG_I(( "I: Card 0x%08lx codec interrupts set, result 0x%04lx\n",
          card, result ));

  return result;
}

ASM( VOID ) SAVEDS AmiGUS_RemoveInterrupt(
  REG( a0, struct AmiGUS * card ),
  REG( d0, LONG which ),
  REG( d1, APTR owner ),
  REG( a6, struct AmiGUS_Base * base )) {

  struct AmiGUS_Private * card_private = convertPublic2Private( card );
  LONG result = AmiGUS_NoError;

  LOG_I(( "I: Freeing interrupts for card 0x%08lx\n", card ));
  result |= ChangePartInterrupt( &( card_private->agp_PCM ),
                                 which,
                                 AMIGUS_FLAG_PCM,
                                 owner,
                                 NULL,
                                 NULL );
  result |= ChangePartInterrupt( &( card_private->agp_Wavetable ),
                                 which,
                                 AMIGUS_FLAG_WAVETABLE,
                                 owner,
                                 NULL,
                                 NULL );
  result |= ChangePartInterrupt( &( card_private->agp_Codec ),
                                 which,
                                 AMIGUS_FLAG_CODEC,
                                 owner,
                                 NULL,
                                 NULL );
  LOG_I(( "I: Card 0x%08lx codec interrupts free'd, result 0x%04lx\n",
          card, result ));

  return;
}
