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

#include <exec/types.h>
#include <libraries/configvars.h>

#include <proto/exec.h>

#include "SDI_compiler.h"

#if defined(__VBCC__)
  #define AMIGA_INTERRUPT __amigainterrupt
#elif defined(__SASC)
  #define AMIGA_INTERRUPT __interrupt
#endif

ASM( APTR ) SAVEDS LIB_MHIAllocDecoder(
  REG( a0, struct Task * task ),
  REG( d0, ULONG signal ),
  REG( a6, struct Library * AmiGUSmhiBase )
) {

  return NULL;
}

ASM( VOID ) SAVEDS LIB_MHIFreeDecoder(
  REG( a3, APTR handle ),
  REG( a6, struct Library * AmiGUSmhiBase )
) {

  return;
}

ASM( BOOL ) SAVEDS LIB_MHIQueueBuffer(
  REG( a3, APTR handle ),
  REG( a0, APTR buffer ),
  REG( d0, ULONG size),
  REG( a6, struct Library * AmiGUSmhiBase )
) {

  return TRUE;
}

ASM( APTR ) SAVEDS LIB_MHIGetEmpty(
  REG( a3, APTR handle ),
  REG( a6, struct Library * AmiGUSmhiBase )
) {

  return NULL;
}

ASM( UBYTE ) SAVEDS LIB_MHIGetStatus(
  REG( a3, APTR handle ),
  REG( a6, struct Library * AmiGUSmhiBase )
) {

  return ( UBYTE ) 17;
}

ASM( VOID ) SAVEDS LIB_MHIPlay(
  REG( a3, APTR handle ),
  REG( a6, struct Library * AmiGUSmhiBase )
) {

  return;
}

ASM( VOID ) SAVEDS LIB_MHIStop(
  REG( a3, APTR handle ),
  REG( a6, struct Library * AmiGUSmhiBase )
) {

  return;
}

ASM( VOID ) SAVEDS LIB_MHIPause(
  REG( a3, APTR handle ),
  REG( a6, struct Library * AmiGUSmhiBase )
) {

  return;
}

ULONG __near call_count_near = 0L;
ULONG __far call_count_far = 0L;
static ULONG call_count_static = 0L;

ASM( ULONG ) SAVEDS LIB_MHIQuery(
  REG( d1, ULONG query ),
  REG( a6, struct Library * AmiGUSmhiBase )
) {

  ULONG result = 0;
  static call_count = 0;

  if ( 0x80000000 & query ) {

    query &= 0x7fffffff;
    switch ( query ) {
      case 1: {
        result = ++call_count;
        break;
      }
      case 2: {
        result = ++call_count_near;
        break;
      }
      case 3: {
        result = ++call_count_far;
        break;
      }
      case 4: {
        result = ++call_count_static;
        break;
      }
      default: {
        break;
      }
    }

  } else {

    result = query + query;
  }

  return result;
}

ASM( VOID ) SAVEDS LIB_MHISetParam(
  REG( a3, APTR handle ),
  REG( d0, UWORD param ),
  REG( d1, ULONG value ),
  REG( a6, struct Library * AmiGUSmhiBase )
) {

  return;
}
