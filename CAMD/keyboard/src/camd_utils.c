/*
 * This file is part of the CAMD keyboard.
 *
 * CAMD keyboard is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3 of the License only.
 *
 * CAMD keyboard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CAMD keyboard.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#define ALIB_STDIO

#include <midi/camd.h>

#include <proto/alib.h>
#include <proto/camd.h>
#include <proto/exec.h>

#include "camd_utils.h"
#include "debug.h"
#include "support.h"

LONG ExtractCamdOutputDevices( struct List * devices ) {

  LONG found = -1;
  APTR lock = LockCAMD( CD_Linkages );

  LOG_V(( "V: CAMD locked @ 0x%08lx\n", ( ULONG ) lock ));
  if ( NULL != lock ) {

    struct MidiCluster * cluster;
    found = 0;

    for ( cluster = NextCluster( NULL );
          cluster;
          cluster = NextCluster( cluster )) {

      struct MidiLink * next;

      LOG_D(( "D: Checking CAMD cluster 0x%08lx -> %s\n",
             ( ULONG ) cluster, cluster->mcl_Node.ln_Name ));
      FOR_LIST( &cluster->mcl_Senders, next, struct MidiLink * ) {

        STRPTR location = NULL;
        STRPTR comment = NULL;
        STRPTR name;
        LONG length = 3;

        LOG_D(( "D: Checking link 0x%08lx\n", ( ULONG ) next ));
        GetMidiLinkAttrs( next,
                          MLINK_Location, &location,
                          MLINK_Comment, &comment,
                          TAG_END );
        found++;

        if (( comment ) && ( location )) {

          struct CAMD_Device_Node * node;

          LOG_V(( "V: Found comment %s\n", comment ));
          LOG_V(( "V: Found location %s\n", location ));

          length += C_strlen( comment );
          length += C_strlen( location );
          name = AllocVec( length, MEMF_ANY );
          
          sprintf( name, "%s, %s\0", comment, location );
          LOG_V(( "V: Created name %s\n", name ));

          node = AllocMem( sizeof( struct CAMD_Device_Node ), MEMF_ANY );
          node->cdn_Name = name;
          node->cdn_Location = location;
          AddTail( devices, ( struct Node * ) node );
        }
      }
    }
    UnlockCAMD( lock );
    LOG_V(( "V: CAMD unlocked @ 0x%08lx\n", ( ULONG ) lock ));
  }

  LOG_D(( "D: Found %ld CAMD outputs\n", found ));
  return found;
}

VOID FreeCamdOutputDevices( struct List * devices ) {

  struct CAMD_Device_Node * node;

  while ( node = ( struct CAMD_Device_Node * ) RemHead( devices )) {

    FreeVec( node->cdn_Name );
    FreeMem( node, sizeof( struct CAMD_Device_Node ));
  }
  LOG_D(( "V: Devices is empty = %ld\n", IS_EMPTY_LIST( devices )));
}
