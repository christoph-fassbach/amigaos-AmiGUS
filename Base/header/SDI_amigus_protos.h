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

#ifndef SDI_AMIGUS_PROTOS_H
#define SDI_AMIGUS_PROTOS_H

// TO NEVER BE USED OUTSIDE THE LIBRARY CODE !!!

#include <amigus/amigus.h>
#include <exec/types.h>

#include "amigus_private.h"
#include "SDI_compiler.h"

// Fixes a bug in VBCC - somehow it would not compile the return value of
// AmiGUS_FindCard otherwise.
typedef struct AmiGUS * AmiGUS_PTR;

/******************************************************************************
 * AmiGUS base library interface functions written in SDI_compiler macros,
 * thereby making them compiler agnostic and 
 * adapting them for AmiGUS library internal usage.
 *****************************************************************************/

ASM( AmiGUS_PTR ) SAVEDS AmiGUS_FindCard(
  REG( a0, struct AmiGUS * card ),
  REG( a6, struct AmiGUS_Base * base ));

ASM( ULONG ) SAVEDS AmiGUS_ReserveCard(
  REG( a0, struct AmiGUS * card ),
  REG( d0, LONG which ),
  REG( d1, APTR owner ),
  REG( a6, struct AmiGUS_Base * base ));

ASM( VOID ) SAVEDS AmiGUS_FreeCard(
  REG( a0, struct AmiGUS * card ),
  REG( d0, LONG which ),
  REG( d1, APTR owner ),
  REG( a6, struct AmiGUS_Base * base ));

ASM( ULONG ) SAVEDS AmiGUS_InstallInterrupt(
  REG( a0, struct AmiGUS * card ),
  REG( d0, LONG which ),
  REG( d1, APTR owner ),
  REG( d2, AmiGUS_Interrupt handler ),
  REG( d3, APTR data ),
  REG( a6, struct AmiGUS_Base * base ));

ASM( VOID ) SAVEDS AmiGUS_RemoveInterrupt(
  REG( a0, struct AmiGUS * card ),
  REG( d0, LONG which ),
  REG( d1, APTR owner ),
  REG( a6, struct AmiGUS_Base * base ));

/******************************************************************************
 * AmiGUS base library interrupt function written in SDI_compiler macros,
 * compiler agnostic and for AmiGUS library internal usage.
 *****************************************************************************/

/**
 * Interrupt handler function, checking the status of all AmiGUS cards
 * found so far and filling their buffers accordingly.
 *
 * @param base Pointer to the driver library's base address.
 *
 * @return 1 if there was at least one card's interrupt pending that was handled,
 *         0 otherwise.
 */
ASM( LONG ) /* __entry for vbcc ? */ SAVEDS INTERRUPT HandleInterrupt (
  REG( a1, struct AmiGUS_Base * base ));

#endif /* SDI_AMIGUS_PROTOS_H */
