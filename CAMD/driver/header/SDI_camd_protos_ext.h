/*
 * This file is part of the AmiGUS CAMD MIDI driver.
 *
 * AmiGUS CAMD MIDI driver is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS CAMD MIDI driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with AmiGUS CAMD MIDI driver.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SDI_CAMD_PROTOS_EXT_H
#define SDI_CAMD_PROTOS_EXT_H

// TO NEVER BE USED INSIDE THE LIBRARY CODE !!!

#include <exec/types.h>
#include <exec/tasks.h>
#include <utility/tagitem.h>

#include <midi/camd.h>

#include "SDI_compiler.h"

/******************************************************************************
 * CAMD MIDI driver interface functions rewritten into SDI_compiler macros,
 * making them compiler agnostic.
 * To be used when just using a random CAMD MIDI driver library.
 *
 * Detailed explanation see
 * TODO: enter source -> location
 *****************************************************************************/

/* CAMD Locks */
ASM( APTR ) SAVEDS LockCAMD(
  REG( d0, ULONG locknum ));
ASM( VOID ) SAVEDS UnlockCAMD(
  REG( a0, APTR lock ));

/* CAMD MidiNode */
ASM( struct MidiNode * ) SAVEDS CreateMidiA(
  REG( a0, struct TagItem * tags ));
// ASM( struct MidiNode * ) SAVEDS CreateMidi( Tag name, ... );
ASM( VOID ) SAVEDS DeleteMidi(
  REG( a0, struct MidiNode * mi ));
ASM( BOOL ) SAVEDS SetMidiAttrsA(
  REG( a0, struct MidiNode * mi ),
  REG( a1, struct TagItem * tags ));
// ASM( BOOL ) SAVEDS SetMidiAttrs( struct MidiNode *, Tag tag, ... );
ASM( ULONG ) SAVEDS GetMidiAttrsA(
  REG( a0, struct MidiNode * mi ),
  REG( a1, struct TagItem * tags ));
// ASM( ULONG ) SAVEDS GetMidiAttrs( struct MidiNode *, Tag tag, ... );
ASM( struct MidiNode * ) SAVEDS NextMidi(
  REG( a0, struct MidiNode * mi ));
ASM( struct MidiNode * ) SAVEDS FindMidi(
  REG( a1, STRPTR name ));
ASM( VOID ) SAVEDS FlushMidi(
  REG( a0, struct MidiNode * mi ));

/* CAMD MidiLink */
ASM( struct MidiLink * ) SAVEDS AddMidiLinkA(
  REG( a0, struct MidiNode * mi),
  REG( d0, LONG type ),
  REG( a1, struct TagItem * tags ));
//ASM( struct MidiLink * ) SAVEDS AddMidiLink( struct MidiNode *, LONG, Tag, ... );
ASM( VOID ) SAVEDS RemoveMidiLink(
  REG( a0, struct MidiLink * ml ));
ASM( BOOL ) SAVEDS SetMidiLinkAttrsA(
  REG( a0, struct MidiLink * ml ),
  REG( a1, struct TagItem * tags ));
// ASM( BOOL ) SAVEDS SetMidiLinkAttrs( struct MidiLink *, Tag tag, ... );
ASM( ULONG ) SAVEDS GetMidiLinkAttrsA(
  REG( a0, struct MidiLink * ml ),
  REG( a1, struct TagItem * tags ));
// ASM( ULONG ) SAVEDS GetMidiLinkAttrs( struct MidiLink *, Tag tag, ... );
ASM( struct MidiLink * ) SAVEDS NextClusterLink(
  REG( a0, struct MidiCluster * mc ),
  REG( a1, struct MidiLink * ml ),
  REG( d0, LONG type ));
ASM( struct MidiLink * ) SAVEDS NextMidiLink(
  REG( a0, struct MidiNode * mi ),
  REG( a1, struct MidiLink * ml ),
  REG( d0, LONG type ));
ASM( BOOL ) SAVEDS MidiLinkConnected(
  REG( a0, struct MidiLink * ml ));

/* CAMD MidiCluster */
ASM( struct MidiCluster * ) SAVEDS NextCluster(
  REG( a0, struct MidiCluster * mc ));
ASM( struct MidiCluster * ) SAVEDS FindCluster(
  REG( a0, STRPTR name ));

/* CAMD Message */
ASM( VOID ) SAVEDS PutMidi(
  REG( a0, struct MidiLink * ml ),
  REG( d0, LONG msgdata ));
ASM( BOOL ) SAVEDS GetMidi(
  REG( a0, struct MidiNode * mi ),
  REG( a1, MidiMsg * msg ));
ASM( BOOL ) SAVEDS WaitMidi(
  REG( a0, struct MidiNode * mi ),
  REG( a1, MidiMsg * msg ));
ASM( VOID ) SAVEDS PutSysEx(
  REG( a0, struct MidiLink * ml ),
  REG( a1, UBYTE * buffer ));
ASM( ULONG ) SAVEDS GetSysEx(
  REG( a0, struct MidiNode * mi ),
  REG( a1, UBYTE * buffer ),
  REG( d0, ULONG len ));
ASM( ULONG ) SAVEDS QuerySysEx(
  REG( a0, struct MidiNode * mi ));
ASM( VOID ) SAVEDS SkipSysEx(
  REG( a0, struct MidiNode * mi ));
ASM( UBYTE ) SAVEDS GetMidiErr(
  REG( a0, struct MidiNode * mi ));
ASM( WORD ) SAVEDS MidiMsgType(
  REG( a0, MidiMsg * msg ));
ASM( WORD ) SAVEDS MidiMsgLen(
  REG( d0, ULONG status ));
ASM( VOID ) SAVEDS ParseMidi(
  REG( a0, struct MidiLink * ml ),
  REG( a1, UBYTE * buffer ),
  REG( d0, ULONG Length ));

/* CAMD Device */
ASM( struct MidiDeviceData * ) SAVEDS OpenMidiDevice(
  REG( a0, UBYTE * name ));
ASM( VOID ) SAVEDS CloseMidiDevice(
  REG( a0, struct MidiDeviceData * mdd ));

/* CAMD External Functions */
ASM( int ) SAVEDS RethinkCAMD( VOID );
ASM( VOID ) SAVEDS StartClusterNotify(
    REG( a0, struct ClusterNotifyNode * node ));
ASM( VOID ) SAVEDS EndClusterNotify(
    REG( a0, struct ClusterNotifyNode * node ));

/* private*/
// ASM( ? ) SAVEDS PutMidiNoWait( REG( a0, ml ), REG( a1, msg ))

#endif /* SDI_CAMD_PROTOS_EXT_H */
