#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proto/exec.h>

#include "sf_math.h"

#if defined (__VBCC__)
/* Don't care about ugly type issues with format strings! */
#pragma dontwarn 214
#endif

UWORD GetAmiGusPeriodsFromTimecents( LONG timecents );
UWORD GetAmiGusFactorShift16FromCentibels( WORD centibels );

/******************************************************************************
 * Mocked functions and stubbed external symbols below:
 *****************************************************************************/

struct Library           * MathIeeeDoubBasBase = 0;
struct Library           * MathIeeeDoubTransBase = 0;

/******************************************************************************
 * Test functions:
 *****************************************************************************/

BOOL testGetAmiGusPeriodsFromTimecents( VOID ) {

  BOOL failed = FALSE;

#if 0

  WORD i;

  // Valid range: -15061..17339
  // even smaller: lost precision
  // even bigger: crash
  // but: SF2 only allows -12000..8000
  for ( i = -15061; i <= 17339; ++i ) {
    
    failed |= ( 0 == GetAmiGusPeriodsFromTimecents( i ));
  }

#else

  failed |= ( 0 == GetAmiGusPeriodsFromTimecents( -12000 ));
  failed |= ( 0 == GetAmiGusPeriodsFromTimecents( -9600 ));
  failed |= ( 0 == GetAmiGusPeriodsFromTimecents( -7973 ));
  failed |= ( 0 == GetAmiGusPeriodsFromTimecents( -7000 ));
  failed |= ( 0 == GetAmiGusPeriodsFromTimecents( -6000 ));
  failed |= ( 0 == GetAmiGusPeriodsFromTimecents( -5000 ));
  failed |= ( 0 == GetAmiGusPeriodsFromTimecents( -4000 ));
  failed |= ( 0 == GetAmiGusPeriodsFromTimecents( -2000 ));
  failed |= ( 0 == GetAmiGusPeriodsFromTimecents( -1000 ));
  failed |= ( 0 == GetAmiGusPeriodsFromTimecents( -500 ));
  failed |= ( 0 == GetAmiGusPeriodsFromTimecents( 0 ));
  failed |= ( 0 == GetAmiGusPeriodsFromTimecents( 500 ));
  failed |= ( 0 == GetAmiGusPeriodsFromTimecents( 1200 ));
  failed |= ( 0 == GetAmiGusPeriodsFromTimecents( 2500 ));
  failed |= ( 0 == GetAmiGusPeriodsFromTimecents( 3000 ));
  failed |= ( 0 == GetAmiGusPeriodsFromTimecents( 3600 ));
  failed |= ( 0 == GetAmiGusPeriodsFromTimecents( 8000 ));

#endif

  return failed;
}

BOOL testGetAmiGusFactorShift16FromCentibels( VOID ) {

  BOOL failed = FALSE;

#if 0
  WORD i;
  // Valid range: -a..b
  // even smaller: ?
  // even bigger: ?
  // but: SF2 only allows 0..1440
  for ( i = 0; i <= 1440; ++i ) {

    if ( 964 > i ) {

      failed |= ( 0 == GetTargetS( i ));

    } else {

      failed |= ( 0 != GetAmiGusFactorShift16FromCentibels( i ));
    }
  }
#else
  failed |= ( 0 == GetAmiGusFactorShift16FromCentibels( 0 ));
  failed |= ( 0 == GetAmiGusFactorShift16FromCentibels( 100 ));
  failed |= ( 0 == GetAmiGusFactorShift16FromCentibels( 200 ));
  failed |= ( 0 == GetAmiGusFactorShift16FromCentibels( 400 ));
  failed |= ( 0 == GetAmiGusFactorShift16FromCentibels( 800 ));
  failed |= ( 0 != GetAmiGusFactorShift16FromCentibels( 1000 ));
#endif
  return failed;
}

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main( int argc, char const *argv[] ) {

  BOOL failed = FALSE;

  MathIeeeDoubBasBase = OpenLibrary( "mathieeedoubbas.library", 36 );
  MathIeeeDoubTransBase = OpenLibrary( "mathieeedoubtrans.library", 36 );

  failed |= testGetAmiGusPeriodsFromTimecents();
  failed |= testGetAmiGusFactorShift16FromCentibels();

  CloseLibrary(( struct Library * )MathIeeeDoubTransBase );
  CloseLibrary(( struct Library * )MathIeeeDoubBasBase );

  return ( failed ) ? 15 : 0;
}
