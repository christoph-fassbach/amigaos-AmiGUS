/*
 * This file is part of the mhiAmiGUS.library.
 *
 * mhiAmiGUS.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiAmiGUS.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiAmiGUS.library.  If not, see <http://www.gnu.org/licenses/>.
 */

// TO NEVER BE USED OUTSIDE THE LIBRARY CODE !!!

#ifndef SDI_MHI_PROTOS_H
#define SDI_MHI_PROTOS_H

#include "SDI_compiler.h"

#if defined(__VBCC__)
  #define AMIGA_INTERRUPT __amigainterrupt
#elif defined(__SASC)
  #define AMIGA_INTERRUPT __interrupt
#endif

ASM( APTR ) SAVEDS LIB_MHIAllocDecoder( REG( a0, struct Task * task ),
                                        REG( d0, ULONG signal ),
                                        REG( a6, struct AmiGUSmhi * base ));

ASM( VOID ) SAVEDS LIB_MHIFreeDecoder( REG( a3, APTR handle ),
                                       REG( a6, struct AmiGUSmhi * base ));

ASM( BOOL ) SAVEDS LIB_MHIQueueBuffer( REG( a3, APTR handle ),
                                      REG( a0, APTR buffer ),
                                      REG( d0, ULONG size ),
                                      REG( a6, struct AmiGUSmhi * base ));

ASM( APTR ) SAVEDS LIB_MHIGetEmpty( REG( a3, APTR handle ),
                                    REG( a6, struct AmiGUSmhi * base ));

ASM( UBYTE ) SAVEDS LIB_MHIGetStatus( REG( a3, APTR handle ),
                                      REG( a6, struct AmiGUSmhi * base ));

ASM( VOID ) SAVEDS LIB_MHIPlay( REG( a3, APTR handle ),
                                REG( a6, struct AmiGUSmhi * base ));

ASM( VOID ) SAVEDS LIB_MHIStop( REG( a3, APTR handle ),
                                REG( a6, struct AmiGUSmhi * base ));

ASM( VOID ) SAVEDS LIB_MHIPause( REG( a3, APTR handle ),
                                 REG( a6, struct AmiGUSmhi * base ));

ASM( ULONG ) SAVEDS LIB_MHIQuery( REG( d1, ULONG query ),
                                  REG( a6, struct AmiGUSmhi * base ));

ASM( VOID ) SAVEDS LIB_MHISetParam( REG( a3, APTR handle ),
                                    REG( d0, UWORD param ),
                                    REG( d1, ULONG value ),
                                    REG( a6, struct AmiGUSmhi * base ));

#endif /* SDI_MHI_PROTOS_H */
