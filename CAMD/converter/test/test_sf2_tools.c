#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proto/exec.h>

#include "sf2_tools.h"

#if defined (__VBCC__)
/* Don't care about ugly type issues with format strings! */
#pragma dontwarn 214
#endif

UWORD GetTargetADR( LONG timecents );
UWORD GetTargetS( WORD centibels );

/******************************************************************************
 * Mocked functions and stubbed external symbols below:
 *****************************************************************************/

struct Library           * MathIeeeDoubBasBase = 0;
struct Library           * MathIeeeDoubTransBase = 0;

/******************************************************************************
 * Test functions:
 *****************************************************************************/

BOOL testGetTargetADR( VOID ) {

  BOOL failed = FALSE;

#if 0

  WORD i;

  // Valid range: -15061..17339
  // even smaller: lost precision
  // even bigger: crash
  // but: SF2 only allows -12000..8000
  for ( i = -15061; i <= 17339; ++i ) {
    
    failed |= ( 0 == GetTargetADR( i ));
  }

#else

  failed |= ( 0 == GetTargetADR( -12000 ));
  failed |= ( 0 == GetTargetADR( -9600 ));
  failed |= ( 0 == GetTargetADR( -7973 ));
  failed |= ( 0 == GetTargetADR( -7000 ));
  failed |= ( 0 == GetTargetADR( -6000 ));
  failed |= ( 0 == GetTargetADR( -5000 ));
  failed |= ( 0 == GetTargetADR( -4000 ));
  failed |= ( 0 == GetTargetADR( -2000 ));
  failed |= ( 0 == GetTargetADR( -1000 ));
  failed |= ( 0 == GetTargetADR( -500 ));
  failed |= ( 0 == GetTargetADR( 0 ));
  failed |= ( 0 == GetTargetADR( 500 ));
  failed |= ( 0 == GetTargetADR( 1200 ));
  failed |= ( 0 == GetTargetADR( 2500 ));
  failed |= ( 0 == GetTargetADR( 3000 ));
  failed |= ( 0 == GetTargetADR( 3600 ));
  failed |= ( 0 == GetTargetADR( 8000 ));

#endif

  return failed;
}

BOOL testGetTargetS( VOID ) {

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

      failed |= ( 0 != GetTargetS( i ));
    }
  }
#else
  failed |= ( 0 == GetTargetS( 0 ));
  failed |= ( 0 == GetTargetS( 100 ));
  failed |= ( 0 == GetTargetS( 200 ));
  failed |= ( 0 == GetTargetS( 400 ));
  failed |= ( 0 == GetTargetS( 800 ));
  failed |= ( 0 != GetTargetS( 1000 ));
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

  failed |= testGetTargetADR();
  failed |= testGetTargetS();

  CloseLibrary(( struct Library * )MathIeeeDoubTransBase );
  CloseLibrary(( struct Library * )MathIeeeDoubBasBase );

  return ( failed ) ? 15 : 0;
}
