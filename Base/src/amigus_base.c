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

#include "amigus_private.h"
#include "amigus_zorro2.h"
#include "debug.h"
#include "SDI_compiler.h"

ASM( ULONG ) SAVEDS AmiGUS_Alloc(
  REG( a0, struct AmiGUS * card ),
  REG( d0, ULONG which ),
  REG( d1, ULONG own ),
  REG( a6, struct AmiGUS_Base * base )) {

  ULONG result = AmiGusZorro2_Alloc( card, which, own );
  return result;
}

ASM( VOID ) SAVEDS AmiGUS_Free(
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
  REG( a6, struct AmiGUS_Base * base )) {

  LOG_W(( "W: Not implemented!\n" ));
  return;
}
