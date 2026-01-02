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

BOOL testFindCard( VOID ) {

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

    ++i;
  }

  return ( BOOL ) ( 0 == i );
}

BOOL testReservations( VOID ) {

  struct AmiGUS * card = AmiGUS_FindCard( NULL );
  ULONG returnValue;
  // Owner can be anything unique you own. :)
  APTR owner1 = FindTask( NULL );
  APTR owner2 = &( owner1 );
  APTR owner3 = ( APTR ) 1234567890;

  if ( NULL == card ) {

    printf( "No Card found.\n" );
    return TRUE;
  }
  printf( "Card 0x%08lx found.\n", card );
  printf( "Owners are 1: 0x%08lx 2: 0x%08lx 3: 0x%08lx.\n",
          owner1, owner2, owner3 );

  // Test 1: Reserve free card part:
  returnValue = AmiGUS_ReserveCard( card, AMIGUS_FLAG_PCM, owner1 );
  if ( AmiGUS_NoError != returnValue ) {

    printf( "Could not reserve PCM part for owner 0x%08lx, reason 0x%04lx.\n",
            owner1, returnValue );
    return TRUE;
  }

  // Test 2: Reserve used card part for same owner:
  returnValue = AmiGUS_ReserveCard( card, AMIGUS_FLAG_PCM, owner1 );
  if ( AmiGUS_NoError != returnValue ) {

    printf( "Could not reserve PCM part for owner 0x%08lx, reason 0x%04lx.\n",
            owner1, returnValue );
    return TRUE;
  }

  // Test 3: Reserve used card part for someone else:
  returnValue = AmiGUS_ReserveCard( card, AMIGUS_FLAG_PCM, owner2 );
  if ( AmiGUS_PcmInUse != returnValue ) {

    printf( "Could reserve PCM part for owner 0x%08lx, reason 0x%04lx.\n",
            owner2, returnValue );
    return TRUE;
  }

  // Test 4: Reserve remaining free card parts:
  returnValue = AmiGUS_ReserveCard( card,
                                    AMIGUS_FLAG_WAVETABLE | AMIGUS_FLAG_CODEC,
                                    owner2 );
  if ( AmiGUS_NoError != returnValue ) {

    printf( "Could not reserve wavetable and codec parts for owner 0x%08lx, "
            "reason 0x%04lx.\n",
            owner2, returnValue );
    return TRUE;
  }

  // Test5: Error codes
  returnValue = AmiGUS_ReserveCard( card,
                                    AMIGUS_FLAG_WAVETABLE,
                                    owner3 );
  if ( AmiGUS_WavetableInUse != returnValue ) {

    printf( "Wrong error code 0x%04lx for owner 0x%08lx, expected 0x%04lx.\n",
            owner3, returnValue, AmiGUS_WavetableInUse );
    return TRUE;
  }
  returnValue = AmiGUS_ReserveCard( card,
                                    AMIGUS_FLAG_CODEC,
                                    owner3 );
  if ( AmiGUS_CodecInUse != returnValue ) {

    printf( "Wrong error code 0x%04lx for owner 0x%08lx, expected 0x%04lx.\n",
            owner3, returnValue, AmiGUS_CodecInUse );
    return TRUE;
  }
  returnValue = AmiGUS_ReserveCard( card,
                                    AMIGUS_FLAG_PCM | AMIGUS_FLAG_WAVETABLE,
                                    owner3 );
  if ( AmiGUS_PcmWavetableInUse != returnValue ) {

    printf( "Wrong error code 0x%04lx for owner 0x%08lx, expected 0x%04lx.\n",
            owner3, returnValue, AmiGUS_PcmWavetableInUse );
    return TRUE;
  }
  returnValue = AmiGUS_ReserveCard( card,
                                    AMIGUS_FLAG_PCM | AMIGUS_FLAG_CODEC,
                                    owner3 );
  if ( AmiGUS_PcmCodecInUse != returnValue ) {

    printf( "Wrong error code 0x%04lx for owner 0x%08lx, expected 0x%04lx.\n",
            owner3, returnValue, AmiGUS_PcmCodecInUse );
    return TRUE;
  }
  returnValue = AmiGUS_ReserveCard( card,
                                    AMIGUS_FLAG_WAVETABLE | AMIGUS_FLAG_CODEC,
                                    owner3 );
  if ( AmiGUS_WavetableCodecInUse != returnValue ) {

    printf( "Wrong error code 0x%04lx for owner 0x%08lx, expected 0x%04lx.\n",
            owner3, returnValue, AmiGUS_WavetableCodecInUse );
    return TRUE;
  }
  returnValue = AmiGUS_ReserveCard(
    card,
    AMIGUS_FLAG_PCM | AMIGUS_FLAG_WAVETABLE | AMIGUS_FLAG_CODEC,
    owner3 );
  if ( AmiGUS_PcmWavetableCodecInUse != returnValue ) {

    printf( "Wrong error code 0x%04lx for owner 0x%08lx, expected 0x%04lx.\n",
            owner3, returnValue, AmiGUS_PcmWavetableCodecInUse );
    return TRUE;
  }
  return FALSE; // NOT failed :)
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
 return FALSE;
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

  failed |= testFindCard();
  failed |= testReservations();
  failed |= testInstallInterrupt();
  failed |= testRemoveInterrupt();
  failed |= testFree();

  CloseLibrary( AmiGUS_Base );
  printf( "%s closed\n", libraryName );

  if ( failed ) {

    printf( "Failed. :(\n" );

  } else {

    printf( "Looks good.\n" );
  }

  return ( failed ) ? 15 : 0;
}
