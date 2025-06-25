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

#ifndef SDI_CAMD_PROTOS_EXT_H
#define SDI_CAMD_PROTOS_EXT_H

// TO NEVER BE USED INSIDE THE LIBRARY CODE !!!

#include <exec/types.h>
#include <exec/tasks.h>

#include "SDI_compiler.h"

/******************************************************************************
 * CAMD MIDI driver interface functions rewritten into SDI_compiler macros,
 * making them compiler agnostic.
 * To be used when just using a random CAMD MIDI driver library.
 *
 * Detailed explanation see
 * TODO: enter source -> location
 *****************************************************************************/

#if 0
ASM( APTR ) SAVEDS MHIAllocDecoder( REG( a0, struct Task * task ),
                                    REG( d0, ULONG signal ));

ASM( VOID ) SAVEDS MHIFreeDecoder( REG( a3, APTR handle ));

ASM( BOOL ) SAVEDS MHIQueueBuffer( REG( a3, APTR handle ),
                                   REG( a0, APTR buffer ),
                                   REG( d0, ULONG size ));

ASM( APTR ) SAVEDS MHIGetEmpty( REG( a3, APTR handle ));

ASM( UBYTE ) SAVEDS MHIGetStatus( REG( a3, APTR handle ));

ASM( VOID ) SAVEDS MHIPlay( REG( a3, APTR handle ));

ASM( VOID ) SAVEDS MHIStop( REG( a3, APTR handle ));

ASM( VOID ) SAVEDS MHIPause( REG( a3, APTR handle ));

ASM( ULONG ) SAVEDS MHIQuery( REG( d1, ULONG query ));

ASM( VOID ) SAVEDS MHISetParam( REG( a3, APTR handle ),
                                REG( d0, UWORD param ),
                                REG( d1, ULONG value ));
#endif

#endif /* SDI_CAMD_PROTOS_EXT_H */
