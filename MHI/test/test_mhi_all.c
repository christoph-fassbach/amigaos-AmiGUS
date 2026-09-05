/*
 * This file is part of the mhiamigus.library.
 *
 * mhiamigus.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiamigus.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiamigus.library.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/types.h>

#include "amigus_mhi.h"
#include "mhi_protos_ext.h"

/******************************************************************************
 * MHI interface - private functions - declared for testing!
 *****************************************************************************/

VOID InitHandle( struct AmiGUS_MHI_Handle * handle );
VOID FlushAllBuffers( struct AmiGUS_MHI_Handle * handle );

/******************************************************************************
 * Mocked functions and stubbed external symbols below:
 *****************************************************************************/

struct AmiGUS_MHI        * AmiGUS_MHI_Base   = NULL;
struct Library           * AmiGUS_Base       = NULL;

/******************************************************************************
 * Test functions:
 *****************************************************************************/

BOOL testMHIGetEmpty( VOID ) {

  APTR probe;
  BOOL failed = FALSE;
  struct AmiGUS_MHI_Buffer * buffer = NULL;

  struct AmiGUS_MHI_Handle * handle =
    malloc( sizeof( struct AmiGUS_MHI_Handle ));

  /////////////////////////////////////////////////////////////////////////////
  printf( "Testing MHIGetEmpty / empty handle... \t\t" );

  memset( handle, 0, sizeof( struct AmiGUS_MHI_Handle ));
  InitHandle( handle );

  probe = MHIGetEmpty( handle );

  failed |= ( probe != NULL );
  printf( ( failed ) ? "Failed!\n" : "OK.\n" );
  /////////////////////////////////////////////////////////////////////////////
  printf( "Testing MHIGetEmpty / no used buffer... \t" );

  memset( handle, 0, sizeof( struct AmiGUS_MHI_Handle ));
  InitHandle( handle );

  failed |= !( MHIQueueBuffer( handle, ( APTR )  4, (  4 * sizeof( ULONG ))));
  failed |= !( MHIQueueBuffer( handle, ( APTR )  8, (  8 * sizeof( ULONG ))));
  failed |= !( MHIQueueBuffer( handle, ( APTR ) 12, ( 12 * sizeof( ULONG ))));

  probe = MHIGetEmpty( handle );

  failed |= ( probe != NULL );
  printf( ( failed ) ? "Failed!\n" : "OK.\n" );
  /////////////////////////////////////////////////////////////////////////////
  printf( "Testing MHIGetEmpty / used buffer current... \t" );

  // Re-using handle here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  buffer = handle->agch_CurrentBuffer;
  failed |= ( buffer->agmb_BufferMax != 4 );
  printf( ( failed ) ? "Failed..." : "OK...    " );

  buffer->agmb_BufferIndex = 0xFFFF;
  probe = MHIGetEmpty( handle );

  failed |= ( probe != NULL );
  printf( ( failed ) ? "Failed!\n" : "OK.\n" );
  /////////////////////////////////////////////////////////////////////////////
  printf( "Testing MHIGetEmpty / available buffer... \t" );

  // Re-using handle here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  buffer = ( APTR ) handle->agch_Buffers.mlh_Head->mln_Succ;
  failed |= ( buffer->agmb_BufferMax != 8 );
  printf( ( failed ) ? "Failed..." : "OK...    " );

  buffer->agmb_BufferIndex = 0xFFFF;
  probe = MHIGetEmpty( handle );

  failed |= ( probe != buffer->agmb_Buffer );
  printf( ( failed ) ? "Failed!\n" : "OK.\n" );
  /////////////////////////////////////////////////////////////////////////////
  printf( "Testing MHIGetEmpty / flushing buffers... \t" );
  FlushAllBuffers( handle );
  failed |= ( !( IsListEmpty(( struct List * ) &( handle->agch_Buffers ))));
  printf( ( failed ) ? "Failed!\n" : "OK.\n" );
  /////////////////////////////////////////////////////////////////////////////
  
  free( handle );

  return failed;
}

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main(int argc, char const *argv[]) {
 
  BOOL failed = FALSE;

  AmiGUS_MHI_Base = malloc( sizeof( struct AmiGUS_MHI ));
  memset( AmiGUS_MHI_Base, 0, sizeof( struct AmiGUS_MHI ));

  failed |= testMHIGetEmpty();

//  free( WriteBuffer );
  free( AmiGUS_MHI_Base );
 
  return ( failed ) ? 15 : 0;
}
