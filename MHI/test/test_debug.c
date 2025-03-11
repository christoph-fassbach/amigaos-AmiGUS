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

/*
 * Building:
 * make USEVBCC=1 clean all
 * cd test
 * vc +kick13 test_debug.c -I../header ../build/debug.o -o test_debug
 * cp test_debug ~/Dokumente/FS-UAE/Shared/MHI
 */

 #include <stdio.h>
#ifdef NULL
#undef NULL
#endif /* NULL */

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>

#include <exec/types.h>

#include <libraries/dos.h>

#include "debug.h"

struct AmiGUSBase {
    // SIZE NEEDS TO MATCH WHAT debug.c SAW!!!!!!
    struct Library                agb_Reserved00;
    UWORD                         agb_Reserved01;
    APTR                          agb_Reserved02;

    struct ExecBase             * agb_Reserved03;
    struct DosLibrary           * agb_Reserved04;
    struct IntuitionBase        * agb_Reserved05;
    struct Library              * agb_Reserved06;
  
    struct Device               * agb_Reserved07;
    struct IORequest            * agb_Reserved08;
    struct ConfigDev            * agb_Reserved09;
    APTR                          agb_Reserved10;
    struct Interrupt            * agb_Reserved11;
  
    struct Task                 * agb_Reserved12;
    ULONG                         agb_Reserved13;
  
    struct MinList                agb_Reserved14;
    struct AmiGUSMhiBuffer      * agb_Reserved15;
  
    ULONG                         agb_Reserved16;
  
    BYTE                          agb_Reserved17;
    UBYTE                         agb_Reserved18;
    UWORD                         agb_Reserved19;
  
    BPTR                          agb_LogFile;
    APTR                          agb_LogMem;
};

struct Library * ExecBase = NULL;
//extern struct DOSLibrary * DOSBase;
struct IntuitionBase * IntuitionBase = NULL;

struct AmiGUSBase * AmiGUSBase = NULL;

VOID DisplayError( ULONG error ) {

  printf( "DisplayError %lu\n", error );
}

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main(int argc, char const *argv[]) {

  ExecBase = OpenLibrary( "exec.library", 34 );
  if ( !ExecBase ) {
    return 21;
  }
  
  IntuitionBase = (struct  IntuitionBase * ) OpenLibrary( "intuition.library", 34 );
  if ( !IntuitionBase ) {
    return 22;
  }

  DOSBase = (struct DosLibrary * ) OpenLibrary( "dos.library", 34 );
  if ( !DOSBase ) {
    return 23;
  }

  AmiGUSBase = AllocMem( sizeof( struct AmiGUSBase ), MEMF_PUBLIC | MEMF_CLEAR );
  AmiGUSBase->agb_LogFile = NULL;

  printf( "Libraries opened\n" );
  LOG_INT(( "INT: log\n" ));
  LOG_V(( "V: log\n" ));
  LOG_D(( "D: log\n" ));
  LOG_I(( "I: log\n" ));
  LOG_W(( "W: log\n" ));
  LOG_E(( "E: log\n" ));
  printf( "done.\n" );

  Close( AmiGUSBase->agb_LogFile );
  FreeMem(( APTR ) AmiGUSBase, sizeof( struct AmiGUSBase ));

  if ( !DOSBase ) {

    CloseLibrary( ( struct Library * ) DOSBase );
  }

  if ( !IntuitionBase ) {

    CloseLibrary( ( struct Library * ) IntuitionBase );
  }

  if ( !ExecBase ) {

    CloseLibrary( ExecBase );
  }

  return 0;
}
