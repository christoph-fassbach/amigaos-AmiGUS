/*
 * This file is part of the SoundFontConverter.
 *
 * SoundFontConverter is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3 of the License only.
 *
 * SoundFontConverter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SoundFontConverter.
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
#include "errors.h"
#include "support.h"

extern struct ExecBase          * SysBase;

LONG ExtractCamdDevices( struct List * devices, BOOL input ) {

  LONG found = -1;
  APTR lock = LockCAMD( CD_Linkages );
  STRPTR typeString = "NO";

  LOG_V(( "V: CAMD locked @ 0x%08lx\n", ( ULONG ) lock ));
  if ( NULL != lock ) {

    struct MidiCluster * cluster;
    found = 0;

    for ( cluster = NextCluster( NULL );
          cluster;
          cluster = NextCluster( cluster )) {

      struct MidiLink * next;
      struct List * midiLinkList;

      if ( input ) {

        midiLinkList = &cluster->mcl_Senders;
        typeString = "in";

      } else {

        midiLinkList = &cluster->mcl_Receivers;
        typeString = "out";
      }
      LOG_D(( "D: Checking CAMD cluster 0x%08lx -> %s for %sputs\n",
              ( ULONG ) cluster, cluster->mcl_Node.ln_Name, typeString ));
      FOR_LIST( midiLinkList, next, struct MidiLink * ) {

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

  LOG_D(( "D: Found %ld CAMD %sputs\n", found, typeString ));
  return found;
}

VOID FreeCamdDevices( struct List * devices ) {

  struct CAMD_Device_Node * node;

  while ( node = ( struct CAMD_Device_Node * ) RemHead( devices )) {

    FreeVec( node->cdn_Name );
    FreeMem( node, sizeof( struct CAMD_Device_Node ));
  }
  LOG_D(( "V: Devices is empty = %ld\n", IS_EMPTY_LIST( devices )));
}

struct MidiNode * OpenMidi( CONST_STRPTR name ) {

  struct MidiNode * node = CreateMidi( MIDI_Name, name,
                                       MIDI_MsgQueue, 100L,
                                       MIDI_SysExSize, 8L,
                                       MIDI_ErrFilter, CMEF_All,
                                       TAG_END );

  LOG_D(( "D: Got MIDI node 0x%08lx for %s.\n", node, name ));
  return node;
}

VOID CloseMidi( struct MidiNode ** node ) {

  if ( *node ) {

    DeleteMidi( *node );
    LOG_D(( "D: Closed MIDI node 0x%08lx.\n", *node ));
    *node = NULL;
  }
}

struct MidiLink * OpenMidiInput( struct MidiNode * node,
                                    CONST_STRPTR location,
                                    CONST_STRPTR name ) {

  struct MidiLink * link = AddMidiLink( node,
                                        MLTYPE_Receiver,
                                        MLINK_Comment, name,
                                        MLINK_Name, "In",
                                        MLINK_EventMask, CMF_Note,
                                        //MLINK_Parse, TRUE, // TODO: needed?
                                        MLINK_Location, location,
                                        TAG_END );

  LOG_D(( "D: Got input link 0x%08lx for %s at %s.\n", link, name, location ));

  return link;
}

struct MidiLink * OpenMidiOutput( struct MidiNode * node,
                                    CONST_STRPTR location,
                                    CONST_STRPTR name ) {

  struct MidiLink * link = AddMidiLink( node,
                                        MLTYPE_Sender,
                                        MLINK_Comment, name,
                                        MLINK_Name, "Out",
                                        MLINK_EventMask, CMF_Note,
                                        //MLINK_Parse, TRUE, // TODO: needed?
                                        MLINK_Location, location,
                                        TAG_END );

  LOG_D(( "D: Got output link 0x%08lx for %s at %s.\n", link, name, location ));

  return link;
}

VOID CloseMidiInOutput( struct MidiLink ** link ) {

  if ( *link ) {

    RemoveMidiLink( *link );
    LOG_D(( "D: Closed in/output link 0x%08lx.\n", *link ));
    *link = NULL;
  }
}
