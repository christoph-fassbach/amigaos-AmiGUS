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
 * AmiGUS base library - public functions.
 *****************************************************************************/

ASM( struct AmiGUS * ) SAVEDS AmiGUS_FindCard(
  REG( a0, struct AmiGUS * card ),
  REG( a6, struct AmiGUS_Base * base )) {

  struct AmiGUS_Private * card_private;
  struct Node * node;
  struct AmiGUS * card_public;

  if ( !( card )) {

    node = AmiGUS_Base->agb_Cards.lh_Head;

  } else {
    
    card_private = convertPublic2Private( card );
    node = card_private->agp_Node.ln_Succ;
  }
  if ( node->ln_Succ == AmiGUS_Base->agb_Cards.lh_Tail ) {

    return NULL;
  }

  card_private = ( struct AmiGUS_Private * ) node;
  card_public = &( card_private->agp_AmiGUS_Public );
  LOG_D(( "D: Found Amigus @ 0x%08lx / 0x%08lx\n",
          card_private, card_public ));

  return card_public;
}

ASM( ULONG ) SAVEDS AmiGUS_ReserveCard(
  REG( a0, struct AmiGUS * card ),
  REG( d0, ULONG which ),
  REG( a6, struct AmiGUS_Base * base )) {

  LOG_W(( "W: Not implemented!\n" ));
  return 0;
}

ASM( VOID ) SAVEDS AmiGUS_FreeCard(
  REG( a0, struct AmiGUS * card ),
  REG( a6, struct AmiGUS_Base * base )) {

  LOG_W(( "W: Not implemented!\n" ));
  return;
}

ASM( ULONG ) SAVEDS AmiGUS_InstallInterrupt(
  REG( a0, struct AmiGUS * card ),
  REG( d0, ULONG which ),
  REG( d1, AmiGUS_Interrupt handler ),
  REG( a6, struct AmiGUS_Base * base )) {

  LOG_W(( "W: Not implemented!\n" ));
  return FALSE;
}

ASM( VOID ) SAVEDS AmiGUS_RemoveInterrupt(
  REG( a0, struct AmiGUS * card ),
  REG( d0, ULONG which ),
  REG( a6, struct AmiGUS_Base * base )) {

  LOG_W(( "W: Not implemented!\n" ));
  return;
}
