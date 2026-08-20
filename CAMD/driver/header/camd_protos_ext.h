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

#ifndef CAMD_PROTOS_EXT_H
#define CAMD_PROTOS_EXT_H

// TO NEVER BE USED INSIDE THE LIBRARY CODE !!!

#include <clib/compiler-specific.h>

#include <exec/types.h>
#include <exec/tasks.h>

#include <midi/camd.h>

#include <utility/tagitem.h>

/******************************************************************************
 * CAMD MIDI driver interface functions rewritten into SDI_compiler macros,
 * making them compiler agnostic.
 * To be used when just using a random CAMD MIDI driver library.
 *
 * Detailed explanation see
 * TODO: enter source -> location
 *****************************************************************************/

/* CAMD Locks */
APTR __ASM__ __SAVE_DS__ LockCAMD(
  __REG__( d0, ULONG locknum )
);

VOID __ASM__ __SAVE_DS__ UnlockCAMD(
  __REG__( a0, APTR lock )
);

/* CAMD MidiNode */
struct MidiNode * __ASM__ __SAVE_DS__ CreateMidiA(
  __REG__( a0, struct TagItem * tags )
);

// struct MidiNode * __ASM__ __SAVE_DS__ CreateMidi( Tag name, ... );
VOID __ASM__ __SAVE_DS__ DeleteMidi(
  __REG__( a0, struct MidiNode * mi )
);
BOOL __ASM__ __SAVE_DS__ SetMidiAttrsA(
  __REG__( a0, struct MidiNode * mi ),
  __REG__( a1, struct TagItem * tags )
);
// BOOL __ASM__ __SAVE_DS__ SetMidiAttrs(
//   struct MidiNode *, Tag tag, ... );
ULONG __ASM__ __SAVE_DS__ GetMidiAttrsA(
  __REG__( a0, struct MidiNode * mi ),
  __REG__( a1, struct TagItem * tags )
);
// ULONG __ASM__ __SAVE_DS__ GetMidiAttrs(
//   struct MidiNode *, Tag tag, ... );
struct MidiNode * __ASM__ __SAVE_DS__ NextMidi(
  __REG__( a0, struct MidiNode * mi )
);
struct MidiNode * __ASM__ __SAVE_DS__ FindMidi(
  __REG__( a1, STRPTR name )
);
VOID __ASM__ __SAVE_DS__ FlushMidi(
  __REG__( a0, struct MidiNode * mi )
);

/* CAMD MidiLink */
struct MidiLink * __ASM__ __SAVE_DS__ AddMidiLinkA(
  __REG__( a0, struct MidiNode * mi),
  __REG__( d0, LONG type ),
  __REG__( a1, struct TagItem * tags )
);
// struct MidiLink * __ASM__ __SAVE_DS__ AddMidiLink(
//   struct MidiNode *, LONG, Tag, ... );
VOID __ASM__ __SAVE_DS__ RemoveMidiLink(
  __REG__( a0, struct MidiLink * ml )
);
BOOL __ASM__ __SAVE_DS__ SetMidiLinkAttrsA(
  __REG__( a0, struct MidiLink * ml ),
  __REG__( a1, struct TagItem * tags )
);
// BOOL __ASM__ __SAVE_DS__ SetMidiLinkAttrs(
//   struct MidiLink *, Tag tag, ... );
ULONG __ASM__ __SAVE_DS__ GetMidiLinkAttrsA(
  __REG__( a0, struct MidiLink * ml ),
  __REG__( a1, struct TagItem * tags )
);
// ULONG __ASM__ __SAVE_DS__ GetMidiLinkAttrs(
//   struct MidiLink *, Tag tag, ... );
struct MidiLink * __ASM__ __SAVE_DS__ NextClusterLink(
  __REG__( a0, struct MidiCluster * mc ),
  __REG__( a1, struct MidiLink * ml ),
  __REG__( d0, LONG type )
);
struct MidiLink * __ASM__ __SAVE_DS__ NextMidiLink(
  __REG__( a0, struct MidiNode * mi ),
  __REG__( a1, struct MidiLink * ml ),
  __REG__( d0, LONG type )
);
BOOL __ASM__ __SAVE_DS__ MidiLinkConnected(
  __REG__( a0, struct MidiLink * ml )
);

/* CAMD MidiCluster */
struct MidiCluster * __ASM__ __SAVE_DS__ NextCluster(
  __REG__( a0, struct MidiCluster * mc )
);
struct MidiCluster * __ASM__ __SAVE_DS__ FindCluster(
  __REG__( a0, STRPTR name )
);

/* CAMD Message */
VOID __ASM__ __SAVE_DS__ PutMidi(
  __REG__( a0, struct MidiLink * ml ),
  __REG__( d0, LONG msgdata )
);
BOOL __ASM__ __SAVE_DS__ GetMidi(
  __REG__( a0, struct MidiNode * mi ),
  __REG__( a1, MidiMsg * msg )
);
BOOL __ASM__ __SAVE_DS__ WaitMidi(
  __REG__( a0, struct MidiNode * mi ),
  __REG__( a1, MidiMsg * msg )
);
VOID __ASM__ __SAVE_DS__ PutSysEx(
  __REG__( a0, struct MidiLink * ml ),
  __REG__( a1, UBYTE * buffer )
);
ULONG __ASM__ __SAVE_DS__ GetSysEx(
  __REG__( a0, struct MidiNode * mi ),
  __REG__( a1, UBYTE * buffer ),
  __REG__( d0, ULONG len )
);
ULONG __ASM__ __SAVE_DS__ QuerySysEx(
  __REG__( a0, struct MidiNode * mi )
);
VOID __ASM__ __SAVE_DS__ SkipSysEx(
  __REG__( a0, struct MidiNode * mi )
);
UBYTE __ASM__ __SAVE_DS__ GetMidiErr(
  __REG__( a0, struct MidiNode * mi )
);
WORD __ASM__ __SAVE_DS__ MidiMsgType(
  __REG__( a0, MidiMsg * msg )
);
WORD __ASM__ __SAVE_DS__ MidiMsgLen(
  __REG__( d0, ULONG status )
);
VOID __ASM__ __SAVE_DS__ ParseMidi(
  __REG__( a0, struct MidiLink * ml ),
  __REG__( a1, UBYTE * buffer ),
  __REG__( d0, ULONG Length )
);

/* CAMD Device */
struct MidiDeviceData * __ASM__ __SAVE_DS__ OpenMidiDevice(
  __REG__( a0, UBYTE * name )
);
VOID __ASM__ __SAVE_DS__ CloseMidiDevice(
  __REG__( a0, struct MidiDeviceData * mdd )
);

/* CAMD External Functions */
int __ASM__ __SAVE_DS__ RethinkCAMD( VOID );
VOID __ASM__ __SAVE_DS__ StartClusterNotify(
    __REG__( a0, struct ClusterNotifyNode * node )
);
VOID __ASM__ __SAVE_DS__ EndClusterNotify(
    __REG__( a0, struct ClusterNotifyNode * node )
);

/* private*/
// ? __ASM__ __SAVE_DS__ PutMidiNoWait( __REG__( a0, ml ), __REG__( a1, msg ))

#endif /* CAMD_PROTOS_EXT_H */
