

#include    <exec/types.h>
#include    <exec/libraries.h>

#include    <clib/exec_protos.h>
#include    <pragmas/exec_pragmas.h>

#include    <stdio.h>

#include    "SDI_mhi_protos.h"
#include    "mhiamigus_pragmas.h"

extern struct ExecBase *SysBase;

struct Library *AmiGUSmhiBase;

VOID main( VOID ) {
  if ( AmiGUSmhiBase = OpenLibrary( "mhiamigus.library",0 )) {
    printf( "12 + 12 = %ld\n", MHIQuery( 12 ));
    printf( "1: called %ld time(s)\n", MHIQuery( 0x80000001 ));
    printf( "1: called %ld time(s)\n", MHIQuery( 0x80000001 ));
    printf( "2: called %ld time(s)\n", MHIQuery( 0x80000002 ));
    printf( "2: called %ld time(s)\n", MHIQuery( 0x80000002 ));
    printf( "3: called %ld time(s)\n", MHIQuery( 0x80000003 ));
    printf( "3: called %ld time(s)\n", MHIQuery( 0x80000003 ));
    printf( "4: called %ld time(s)\n", MHIQuery( 0x80000004 ));
    printf( "4: called %ld time(s)\n", MHIQuery( 0x80000004 ));
    /* We're done, so close the library... */
    CloseLibrary( AmiGUSmhiBase );

  } else {

    printf( "Couldn't open mhiamigus.library!\n" );
  }
}
