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

#include "debug.h"
#include "lib_init.h"

// Since the file is meant only for SASC, let's assume we are in SASC!
#ifndef getreg

#define getreg __builtin_getreg
extern long getreg(int);

#define REG_D0       0
#define REG_D1       1
#define REG_D2       2
#define REG_D3       3
#define REG_D4       4
#define REG_D5       5
#define REG_D6       6
#define REG_D7       7
#define REG_A0       8
#define REG_A1       9
#define REG_A2      10
#define REG_A3      11
#define REG_A4      12
#define REG_A5      13
#define REG_A6      14
#define REG_A7      15
#define REG_FP0     16
#define REG_FP1     17
#define REG_FP2     18
#define REG_FP3     19
#define REG_FP4     20
#define REG_FP5     21
#define REG_FP6     22
#define REG_FP7     23

#endif

int __saveds __stdargs __UserLibInit( VOID ) {

  struct AmiGUSmhi * ownBase = ( struct AmiGUSmhi * ) getreg( REG_A6 );
  struct ExecBase * sysBase = *(( struct ExecBase ** ) 4 );
  LONG result = CustomLibInit( ownBase, sysBase );

  LOG_I(( "I: " LIB_FILE " initialized\n" ));
  return result;
}

VOID __saveds __stdargs __UserLibCleanup( VOID ) {

  LOG_I(( "I: " LIB_FILE " de-initializing...\n" ));
  CustomLibClose( AmiGUSmhiBase );
}
