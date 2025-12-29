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

// vc +kick13 -I../include -I../header amigus_lib_check.c -o amigus_lib_check

#include <stdio.h>
#include <string.h>

#ifdef NULL
#undef NULL
#endif

#include <proto/exec.h>
#include <proto/amigus.h>
#include <amigus/amigus.h>

#include "errors.h"

struct Library * AmiGUS_Base;

struct AmiGUS card;

/******************************************************************************
 * Test functions:
 *****************************************************************************/

VOID testInterruptHandler( APTR data ) {

}

BOOL testAlloc( VOID ) {

  ULONG result = AmiGUS_Alloc(
    &card,
    AMIGUS_FLAG_PCM | AMIGUS_FLAG_CODEC | AMIGUS_FLAG_WAVETABLE,
    AMIGUS_FLAG_PCM | AMIGUS_FLAG_CODEC | AMIGUS_FLAG_WAVETABLE
  );

  return ( BOOL )( result != ENoError );
}

BOOL testInstallInterrupt( VOID ) {

  ULONG result = AmiGUS_InstallInterrupt(
    &card,
    AMIGUS_FLAG_PCM | AMIGUS_FLAG_CODEC | AMIGUS_FLAG_WAVETABLE,
    &testInterruptHandler
  );

  return ( BOOL )( result != ENoError );
}

BOOL testRemoveInterrupt( VOID ) {

  AmiGUS_RemoveInterrupt(
    &card,
    AMIGUS_FLAG_PCM | AMIGUS_FLAG_CODEC | AMIGUS_FLAG_WAVETABLE
  );
  return FALSE;
}

BOOL testFree( VOID ) {

  AmiGUS_Free( &card );

  return FALSE;
}

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main(int argc, char const *argv[]) {

  BOOL failed = FALSE;
  STRPTR libraryName = "amigus.library";

  AmiGUS_Base = OpenLibrary( libraryName, 0 );
  if ( !AmiGUS_Base ) {

    printf( "Opening %s failed!\n", libraryName );
    return 20;
  }
  printf( "%s opened\n", libraryName );

  failed |= testAlloc();
  failed |= testInstallInterrupt();
  failed |= testRemoveInterrupt();
  failed |= testFree();

  CloseLibrary( AmiGUS_Base );
  printf( "%s closed\n", libraryName );

  return ( failed ) ? 15 : 0;
}
