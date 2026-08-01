#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proto/exec.h>

#include "sf2_tools.h"

#if defined (__VBCC__)
/* Don't care about ugly type issues with format strings! */
#pragma dontwarn 214
#endif

UWORD GetTargetADSR( LONG timecents );

/******************************************************************************
 * Mocked functions and stubbed external symbols below:
 *****************************************************************************/

struct Library           * MathIeeeDoubBasBase = 0;
struct Library           * MathIeeeDoubTransBase = 0;

/******************************************************************************
 * Test functions:
 *****************************************************************************/

BOOL testGetTargetADSR( VOID ) {

  BOOL failed = FALSE;
  GetTargetADSR( -12000 );
  GetTargetADSR( -9600 );
  GetTargetADSR( -7973 );
  GetTargetADSR( 0 );
  GetTargetADSR( 3600 );
  GetTargetADSR( 8000 );

  return failed;
}

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main( int argc, char const *argv[] ) {

  BOOL failed = FALSE;

  MathIeeeDoubBasBase = OpenLibrary( "mathieeedoubbas.library", 36 );
  MathIeeeDoubTransBase = OpenLibrary( "mathieeedoubtrans.library", 36 );

  failed |= testGetTargetADSR();

  CloseLibrary(( struct Library * )MathIeeeDoubTransBase );
  CloseLibrary(( struct Library * )MathIeeeDoubBasBase );

  return ( failed ) ? 15 : 0;
}
