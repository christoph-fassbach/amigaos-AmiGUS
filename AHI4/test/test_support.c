#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proto/alib.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/timer.h>

#include "amigus_ahi_sub.h"
#include "support.h"

#if defined (__VBCC__)
/* Don't care about ugly type issues with format strings! */
#pragma dontwarn 214
#endif

/******************************************************************************
 * Mocked functions and stubbed external symbols below:
 *****************************************************************************/
struct AmiGUSBase * AmiGUSBase;

/* Taken over from lib_amigus.c */
/*
 defined in LIB:c.o:
struct ExecBase          * SysBase           = 0;
struct DosLibrary        * DOSBase           = 0;
struct IntuitionBase     * IntuitionBase     = 0;
struct Library           * UtilityBase       = 0;
struct Library           * ExpansionBase     = 0;
 */
struct Device            * TimerBase         = 0;

/******************************************************************************
 * Private functions / fields under test:
 *****************************************************************************/

/******************************************************************************
 * Test functions:
 *****************************************************************************/

BOOL testSleep( VOID ) {

  BOOL failed = FALSE;
  int i;

#define NUM_CASES 5
  int cases[NUM_CASES][ 2 ] = {
    {   250, 2 },
    {  1500, 3 },
    {  5000, 4 },
    {  7500, 5 },
    { 10000, 6 }
  };

  for( i = 0; i < NUM_CASES; ++i ) {
    struct timeval before;
    struct timeval after;
    LONG delta_millis;
    BOOL tst;
    LONG millis_in = cases[i][0];
    LONG deviation = cases[i][1];

    printf( "Testing Sleep( %ld ) ...\n", millis_in );
    
    GetSysTime( &before );
    Sleep( millis_in );
    GetSysTime( &after );
    delta_millis = ((( after.tv_secs - before.tv_secs ) * 1000 )
                  + ( (LONG)after.tv_micro - (LONG)before.tv_micro ) / 1000 );
    tst = ( abs( millis_in - delta_millis ) <= deviation );
    printf( "Before %ld.%06ld - After %ld.%06ld - Delta %ldms \t %s\n",
            before.tv_secs,
            before.tv_micro,
            after.tv_secs,
            after.tv_micro,
            delta_millis,
            (tst) ? "passed" : "FAIL!!" );

    failed = !tst;
  }

  return failed;
}

BOOL testNewList( VOID ) {

  ULONG exp0;
  ULONG exp1;
  ULONG exp2;

  BOOL failed = FALSE;

  struct MinList myList;
  
  myList.mlh_Head = NULL;
  myList.mlh_Tail = NULL;
  myList.mlh_TailPred = NULL;

  NewList(( struct List * ) &myList );
  exp0 = ( ULONG ) myList.mlh_Head;
  exp1 = ( ULONG ) myList.mlh_Tail;
  exp2 = ( ULONG ) myList.mlh_TailPred;

  myList.mlh_Head = NULL;
  myList.mlh_Tail = NULL;
  myList.mlh_TailPred = NULL;

  NonConflictingNewMinList( &myList );

  failed |= (( ULONG ) myList.mlh_Head != exp0 );
  failed |= (( ULONG ) myList.mlh_Tail != exp1 );
  failed |= (( ULONG ) myList.mlh_TailPred != exp2 );
  
  printf( "NewList @ %08lx -                                             %s\n"
          "Head     expected %08lx actual %08lx \n"
          "Tail     expected %08lx actual %08lx \n"
          "TailPred expected %08lx actual %08lx \n",
          ( ULONG ) &myList, (!failed) ? "passed" : "FAIL!!",
          exp0, ( ULONG ) myList.mlh_Head,
          exp1, ( ULONG ) myList.mlh_Tail,
          exp2, ( ULONG ) myList.mlh_TailPred);

  return failed;
}

BOOL testUseList( VOID ) {

  BOOL failed = FALSE;

  struct MinList myList;

  struct MinNode * node;
  struct MinNode node0;
  struct MinNode node1;
  struct MinNode node2;

  LONG act0 = 0;
  LONG act1 = 0;
  BOOL tst0;
  BOOL tst1;

  NonConflictingNewMinList( &myList );
  AddHead(( struct List * ) &myList,
          ( struct Node * ) &node2 );
  AddHead(( struct List * ) &myList,
          ( struct Node * ) &node1 );
  AddHead(( struct List * ) &myList,
          ( struct Node * ) &node0 );

  printf( "List @ %08lx - head %08lx tail %08lx tailpre %08lx \nforward:\n",
          ( ULONG ) &myList,
          ( ULONG ) myList.mlh_Head,
          ( ULONG ) myList.mlh_Tail,
          ( ULONG ) myList.mlh_TailPred );

  // Iterate Forward
  node = myList.mlh_Head;
  while ( node->mln_Succ ) {

    printf( "Node @ %08lx - pred %08lx succ %08lx \n",
          ( ULONG ) node,
          ( ULONG ) node->mln_Pred,
          ( ULONG ) node->mln_Succ );
    ++act0;
    node = node->mln_Succ;
  }
  tst0 = ( 3 == act0 );
  failed |= !tst0;
  printf( "Iterate List @ %08lx forward expected %ld actual %ld -            %s\n"
          "backward:\n",
          ( ULONG ) &myList, 3, act0, (tst0) ? "passed" : "FAIL!!" );
  // Iterate Backward
  node = myList.mlh_TailPred;
  while ( node->mln_Pred ) {

    printf( "Node @ %08lx - pred %08lx succ %08lx \n",
          ( ULONG ) node,
          ( ULONG ) node->mln_Pred,
          ( ULONG ) node->mln_Succ );
    ++act1;
    node = node->mln_Pred;
  }
  tst1 = ( 3 == act1 );
  failed |= !tst1;
  printf( "Iterate List @ %08lx backward expected %ld actual %ld -           %s\n",
          ( ULONG ) &myList, 3, act1, (tst1) ? "passed" : "FAIL!!" );

  return failed;
}

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main(int argc, char const *argv[]) {

  LONG error;
  BOOL failed = FALSE;
  struct AmiGUSBase * amiGUSBase;

  AmiGUSBase = malloc( sizeof( struct AmiGUSBase ) );
  memset( AmiGUSBase, 0, sizeof( struct AmiGUSBase ) );
  amiGUSBase = AmiGUSBase;

  if ( !amiGUSBase ) {

    printf( "Memory allocation 0 failed!\n" );
    return 20;
  }
  amiGUSBase->agb_TimerRequest = malloc( sizeof( struct IORequest ));
  if( !(amiGUSBase->agb_TimerRequest) ) {

    printf( "Memory allocation 1 failed!\n" );
    return 21;
  }
  error = OpenDevice( "timer.device", 0, amiGUSBase->agb_TimerRequest, 0 );
  if ( error ) {

    printf( "Opening timer.device failed!\n" );
    return 22;
  }
  amiGUSBase->agb_TimerBase = amiGUSBase->agb_TimerRequest->io_Device;
  TimerBase = amiGUSBase->agb_TimerRequest->io_Device;

  amiGUSBase->agb_LogFile = (BPTR)NULL;
  amiGUSBase->agb_LogMem = NULL;

  amiGUSBase->agb_DOSBase =
    (struct DosLibrary *) OpenLibrary( "dos.library", 37 );
  if( !(amiGUSBase->agb_DOSBase) ) {

    printf( "Opening dos.library failed!\n" );
    return 23;
  }
  DOSBase = amiGUSBase->agb_DOSBase;

  failed |= testSleep();
  failed |= testNewList();
  failed |= testUseList();

  if ( amiGUSBase->agb_LogFile ) {

    Close( amiGUSBase->agb_LogFile );
  }
  if( amiGUSBase->agb_DOSBase ) {

    CloseLibrary( (struct Library *) amiGUSBase->agb_DOSBase );
  }
  if( amiGUSBase->agb_TimerBase ) {

    CloseDevice( amiGUSBase->agb_TimerRequest );
  }
  if( amiGUSBase->agb_TimerRequest ) {

    free( amiGUSBase->agb_TimerRequest );
  }
  free( amiGUSBase );

  return ( failed ) ? 15 : 0;
}
