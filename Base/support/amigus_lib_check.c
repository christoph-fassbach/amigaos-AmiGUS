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

/******************************************************************************
 * Test functions:
 *****************************************************************************/

VOID testInterruptHandler( APTR data ) {

}

BOOL testAlloc( VOID ) {

  struct AmiGUS * card = NULL;
  LONG i = 0;

  while ( card = AmiGUS_FindCard( card )) {

    printf( "AmiGUS_FindCard result is 0x%08lx\n", card );
    printf( "PCM Base is 0x%08lx\n",
            card->agus_PcmBase );
    printf( "WaveTable Base is 0x%08lx\n",
            card->agus_WavetableBase );
    printf( "Codec Base is 0x%08lx\n",
            card->agus_CodecBase );
    printf( "Card type is %ld / %s\n",
            card->agus_TypeId,
            card->agus_TypeName );
    printf( "Firmware rev is 0x%08lx, %04ld-%02ld-%02ld, %02ld:%02ld\n",
            card->agus_FirmwareRev,
            card->agus_Year,
            card->agus_Month,
            card->agus_Day,
            card->agus_Hour,
            card->agus_Minute );
    printf( "Hardware rev is ?\n");
    printf( "FPGA id is ?\n");
  }

  return ( BOOL ) ( 0 < i );
}

BOOL testInstallInterrupt( VOID ) {
/*
  ULONG result = AmiGUS_InstallInterrupt(
    &card,
    AMIGUS_FLAG_PCM | AMIGUS_FLAG_CODEC | AMIGUS_FLAG_WAVETABLE,
    &testInterruptHandler
  );

  return ( BOOL )( result != ENoError );
  */
}

BOOL testRemoveInterrupt( VOID ) {
/*
  AmiGUS_RemoveInterrupt(
    &card,
    AMIGUS_FLAG_PCM | AMIGUS_FLAG_CODEC | AMIGUS_FLAG_WAVETABLE
  );*/
  return FALSE;
}

BOOL testFree( VOID ) {
/*
  AmiGUS_Free( &card );
*/
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
