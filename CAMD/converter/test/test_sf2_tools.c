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

  // Valid range: -15061 ... 17339
  // even smaller: lost precision
  // even bigger: crash
  // but: SF2 only allows -12000 ... 8000
  for ( i = -15061; i <= 17339; ++i ) {
    
    GetTargetADR( i );
  }

#else

  GetTargetADR( -12000 );
  GetTargetADR( -9600 );
  GetTargetADR( -7973 );
  GetTargetADR( -7000 );
  GetTargetADR( -6000 );
  GetTargetADR( -5000 );
  GetTargetADR( -4000 );
  GetTargetADR( -2000 );
  GetTargetADR( -1000 );
  GetTargetADR( -500 );
  GetTargetADR( 0 );
  GetTargetADR( 500 );
  GetTargetADR( 1200 );
  GetTargetADR( 2500 );
  GetTargetADR( 3000 );
  GetTargetADR( 3600 );
  GetTargetADR( 8000 );

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

  CloseLibrary(( struct Library * )MathIeeeDoubTransBase );
  CloseLibrary(( struct Library * )MathIeeeDoubBasBase );

  return ( failed ) ? 15 : 0;
}
