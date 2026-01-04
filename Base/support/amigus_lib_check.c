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

VOID flushOwners(
  struct AmiGUS * card,
  APTR owner0,
  APTR owner1,
  APTR owner2 ) {

  AmiGUS_FreeCard(
    card,
    AMIGUS_FLAG_PCM | AMIGUS_FLAG_WAVETABLE | AMIGUS_FLAG_CODEC,
    owner0 );
  AmiGUS_FreeCard(
    card,
    AMIGUS_FLAG_PCM | AMIGUS_FLAG_WAVETABLE | AMIGUS_FLAG_CODEC,
    owner1 );
  AmiGUS_FreeCard(
    card,
    AMIGUS_FLAG_PCM | AMIGUS_FLAG_WAVETABLE | AMIGUS_FLAG_CODEC,
    owner2 );
}

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

BOOL testReserveCard( VOID ) {

  struct AmiGUS * card = AmiGUS_FindCard( NULL );
  ULONG returnValue;
  // Owner can be anything unique you own. :)
  APTR owner0 = FindTask( NULL );
  APTR owner1 = ( APTR ) 1234567890;
  APTR owner2 = ( APTR ) 1234567891;

  if ( NULL == card ) {

    printf( "No Card found.\n" );
    return TRUE;
  }
  printf( "Card 0x%08lx found.\n", card );
  printf( "Owners are 1: 0x%08lx 2: 0x%08lx 3: 0x%08lx.\n",
          owner0, owner1, owner2 );
  printf( "Attempting failure situation recovery, "
          "may work if no allocations happened in the meantime, "
          "so ALL tests may only be valid straight after reboot!\n" );
  flushOwners( card, owner0, owner1, owner2 );
  printf( "Testing AmiGUS_ReserveCard...\n");

  // Test 1: Reserve free card part:
  returnValue = AmiGUS_ReserveCard( card, AMIGUS_FLAG_PCM, owner0 );
  if ( AmiGUS_NoError != returnValue ) {

    printf( "Could not reserve PCM part for owner 0x%08lx, reason 0x%04lx.\n",
            owner0, returnValue );
    flushOwners( card, owner0, owner1, owner2 );
    return TRUE;
  }

  // Test 2: Reserve used card part for same owner:
  returnValue = AmiGUS_ReserveCard( card, AMIGUS_FLAG_PCM, owner0 );
  if ( AmiGUS_NoError != returnValue ) {

    printf( "Could not reserve PCM part for owner 0x%08lx, reason 0x%04lx.\n",
            owner0, returnValue );
    flushOwners( card, owner0, owner1, owner2 );
    return TRUE;
  }

  // Test 3: Reserve used card part for someone else:
  returnValue = AmiGUS_ReserveCard( card, AMIGUS_FLAG_PCM, owner1 );
  if ( AmiGUS_PcmInUse != returnValue ) {

    printf( "Could reserve PCM part for owner 0x%08lx, reason 0x%04lx.\n",
            owner1, returnValue );
    flushOwners( card, owner0, owner1, owner2 );
    return TRUE;
  }

  // Test 4: Reserve remaining free card parts:
  returnValue = AmiGUS_ReserveCard( card,
                                    AMIGUS_FLAG_WAVETABLE | AMIGUS_FLAG_CODEC,
                                    owner1 );
  if ( AmiGUS_NoError != returnValue ) {

    printf( "Could not reserve wavetable and codec parts for owner 0x%08lx, "
            "reason 0x%04lx.\n",
            owner1, returnValue );
    flushOwners( card, owner0, owner1, owner2 );
    return TRUE;
  }

  // Test5: Error codes
  returnValue = AmiGUS_ReserveCard( card,
                                    AMIGUS_FLAG_WAVETABLE,
                                    owner2 );
  if ( AmiGUS_WavetableInUse != returnValue ) {

    printf( "Wrong error code 0x%04lx for owner 0x%08lx, expected 0x%04lx.\n",
            owner2, returnValue, AmiGUS_WavetableInUse );
    flushOwners( card, owner0, owner1, owner2 );
    return TRUE;
  }
  returnValue = AmiGUS_ReserveCard( card,
                                    AMIGUS_FLAG_CODEC,
                                    owner2 );
  if ( AmiGUS_CodecInUse != returnValue ) {

    printf( "Wrong error code 0x%04lx for owner 0x%08lx, expected 0x%04lx.\n",
            owner2, returnValue, AmiGUS_CodecInUse );
    flushOwners( card, owner0, owner1, owner2 );
    return TRUE;
  }
  returnValue = AmiGUS_ReserveCard( card,
                                    AMIGUS_FLAG_PCM | AMIGUS_FLAG_WAVETABLE,
                                    owner2 );
  if ( AmiGUS_PcmWavetableInUse != returnValue ) {

    printf( "Wrong error code 0x%04lx for owner 0x%08lx, expected 0x%04lx.\n",
            owner2, returnValue, AmiGUS_PcmWavetableInUse );
    flushOwners( card, owner0, owner1, owner2 );
    return TRUE;
  }
  returnValue = AmiGUS_ReserveCard( card,
                                    AMIGUS_FLAG_PCM | AMIGUS_FLAG_CODEC,
                                    owner2 );
  if ( AmiGUS_PcmCodecInUse != returnValue ) {

    printf( "Wrong error code 0x%04lx for owner 0x%08lx, expected 0x%04lx.\n",
            owner2, returnValue, AmiGUS_PcmCodecInUse );
    flushOwners( card, owner0, owner1, owner2 );
    return TRUE;
  }
  returnValue = AmiGUS_ReserveCard( card,
                                    AMIGUS_FLAG_WAVETABLE | AMIGUS_FLAG_CODEC,
                                    owner2 );
  if ( AmiGUS_WavetableCodecInUse != returnValue ) {

    printf( "Wrong error code 0x%04lx for owner 0x%08lx, expected 0x%04lx.\n",
            owner2, returnValue, AmiGUS_WavetableCodecInUse );
    flushOwners( card, owner0, owner1, owner2 );
    return TRUE;
  }
  returnValue = AmiGUS_ReserveCard(
    card,
    AMIGUS_FLAG_PCM | AMIGUS_FLAG_WAVETABLE | AMIGUS_FLAG_CODEC,
    owner2 );
  if ( AmiGUS_PcmWavetableCodecInUse != returnValue ) {

    printf( "Wrong error code 0x%04lx for owner 0x%08lx, expected 0x%04lx.\n",
            owner2, returnValue, AmiGUS_PcmWavetableCodecInUse );
    flushOwners( card, owner0, owner1, owner2 );
    return TRUE;
  }

  printf( "Success.\n");
  // Made it to the end? Success!
  flushOwners( card, owner0, owner1, owner2 );
  return FALSE; // NOT failed :)
}

BOOL testFreeCardInitialState(
  struct AmiGUS * card,
  APTR owner0,
  APTR owner1,
  APTR owner2 ) {

  ULONG returnValue;

  flushOwners( card, owner0, owner1, owner2 );

  // Reserve all, fail if not.
  returnValue = AmiGUS_ReserveCard(
    card,
    AMIGUS_FLAG_PCM | AMIGUS_FLAG_WAVETABLE | AMIGUS_FLAG_CODEC,
    owner0 );
  if ( AmiGUS_NoError != returnValue ) {

    printf( "Could not reserve all parts for owner 0x%08lx, reason 0x%04lx.\n",
            owner0, returnValue );
    flushOwners( card, owner0, owner1, owner2 );
    return TRUE;
  }
  return FALSE;
}

BOOL testFreeCard( VOID ) {

  struct AmiGUS * card = AmiGUS_FindCard( NULL );
  ULONG returnValue;
  // Owner can be anything unique you own. :)
  APTR owner0 = FindTask( NULL );
  APTR owner1 = ( APTR ) 1234567890;
  APTR owner2 = ( APTR ) 1234567891;

  if ( NULL == card ) {

    printf( "No Card found.\n" );
    return TRUE;
  }
  printf( "Card 0x%08lx found.\n", card );
  printf( "Owners are 1: 0x%08lx 2: 0x%08lx 3: 0x%08lx.\n",
          owner0, owner1, owner2 );
  printf( "Testing AmiGUS_FreeCard...\n");

  // Test 1: Test freeing PCM, fail if re-reserve reserves more or less
  returnValue = testFreeCardInitialState( card, owner0, owner1, owner2 );
  if ( returnValue ) {
    printf( "Cannot reach initial state for free'ing PCM.\n" );
    return TRUE;
  }
  AmiGUS_FreeCard( card, AMIGUS_FLAG_PCM, owner0 );
  returnValue = AmiGUS_ReserveCard(
    card,
    AMIGUS_FLAG_PCM | AMIGUS_FLAG_WAVETABLE | AMIGUS_FLAG_CODEC,
    owner1 );
  if ( AmiGUS_WavetableCodecInUse != returnValue ) {

    printf( "PCM wasn't free'd as expected." );
    flushOwners( card, owner0, owner1, owner2 );
    return TRUE;
  }

  // Test 2: Test freeing Wavetable, fail if re-reserve reserves more or less
  returnValue = testFreeCardInitialState( card, owner0, owner1, owner2 );
  if ( returnValue ) {
    printf( "Cannot reach initial state for free'ing Wavetable.\n" );
    return TRUE;
  }
  AmiGUS_FreeCard( card, AMIGUS_FLAG_WAVETABLE, owner0 );
  returnValue = AmiGUS_ReserveCard(
    card,
    AMIGUS_FLAG_PCM | AMIGUS_FLAG_WAVETABLE | AMIGUS_FLAG_CODEC,
    owner1 );
  if ( AmiGUS_PcmCodecInUse != returnValue ) {

    printf( "Wavetable wasn't free'd as expected." );
    flushOwners( card, owner0, owner1, owner2 );
    return TRUE;
  }

  // Test 3: Test freeing Codec, fail if re-reserve reserves more or less
  returnValue = testFreeCardInitialState( card, owner0, owner1, owner2 );
  if ( returnValue ) {
    printf( "Cannot reach initial state for free'ing Codec.\n" );
    return TRUE;
  }
  AmiGUS_FreeCard( card, AMIGUS_FLAG_CODEC, owner0 );
  returnValue = AmiGUS_ReserveCard(
    card,
    AMIGUS_FLAG_PCM | AMIGUS_FLAG_WAVETABLE | AMIGUS_FLAG_CODEC,
    owner1 );
  if ( AmiGUS_PcmWavetableInUse != returnValue ) {

    printf( "Codec wasn't free'd as expected." );
    flushOwners( card, owner0, owner1, owner2 );
    return TRUE;
  }

  // Test 4: Test freeing Codec from wrong owner
  returnValue = testFreeCardInitialState( card, owner0, owner1, owner2 );
  if ( returnValue ) {
    printf( "Cannot reach initial state for free'ing Codec.\n" );
    return TRUE;
  }
  AmiGUS_FreeCard(
    card,
    AMIGUS_FLAG_PCM,
    owner1 );
  returnValue = AmiGUS_ReserveCard(
    card,
    AMIGUS_FLAG_PCM | AMIGUS_FLAG_WAVETABLE | AMIGUS_FLAG_CODEC,
    owner1 );
  if ( AmiGUS_PcmWavetableCodecInUse != returnValue ) {

    printf( "Codec was free'd unexpectedly.\n" );
    flushOwners( card, owner0, owner1, owner2 );
    return TRUE;
  }
  AmiGUS_FreeCard(
    card,
    AMIGUS_FLAG_WAVETABLE,
    owner1 );
  returnValue = AmiGUS_ReserveCard(
    card,
    AMIGUS_FLAG_PCM | AMIGUS_FLAG_WAVETABLE | AMIGUS_FLAG_CODEC,
    owner1 );
  if ( AmiGUS_PcmWavetableCodecInUse != returnValue ) {

    printf( "Codec was free'd unexpectedly.\n" );
    flushOwners( card, owner0, owner1, owner2 );
    return TRUE;
  }
  AmiGUS_FreeCard(
    card,
    AMIGUS_FLAG_CODEC,
    owner1 );
  returnValue = AmiGUS_ReserveCard(
    card,
    AMIGUS_FLAG_PCM | AMIGUS_FLAG_WAVETABLE | AMIGUS_FLAG_CODEC,
    owner1 );
  if ( AmiGUS_PcmWavetableCodecInUse != returnValue ) {

    printf( "Codec was free'd unexpectedly.\n" );
    flushOwners( card, owner0, owner1, owner2 );
    return TRUE;
  }
  AmiGUS_FreeCard(
    card,
    AMIGUS_FLAG_PCM | AMIGUS_FLAG_WAVETABLE | AMIGUS_FLAG_CODEC,
    owner1 );
  returnValue = AmiGUS_ReserveCard(
    card,
    AMIGUS_FLAG_PCM | AMIGUS_FLAG_WAVETABLE | AMIGUS_FLAG_CODEC,
    owner1 );
  if ( AmiGUS_PcmWavetableCodecInUse != returnValue ) {

    printf( "Codec was free'd unexpectedly.\n" );
    flushOwners( card, owner0, owner1, owner2 );
    return TRUE;
  }

  printf( "Success.\n");
  // Made it to the end? Success!
  flushOwners( card, owner0, owner1, owner2 );
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
  failed |= testReserveCard();
  failed |= testFreeCard();
  failed |= testInstallInterrupt();
  failed |= testRemoveInterrupt();

  CloseLibrary( AmiGUS_Base );
  printf( "%s closed\n", libraryName );

  if ( failed ) {

    printf( "Failed. :(\n" );

  } else {

    printf( "Looks good.\n" );
  }

  return ( failed ) ? 15 : 0;
}
