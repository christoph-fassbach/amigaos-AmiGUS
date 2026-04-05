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

#ifndef CAMD_UTILS_H
#define CAMD_UTILS_H

#include <exec/lists.h>
#include <midi/camd.h>

struct CAMD_Device_Node {

    struct MinNode cdn_Node;

    STRPTR cdn_Name;
    STRPTR cdn_Location;
};

LONG ExtractCamdDevices( struct List * devices, BOOL input );
VOID FreeCamdDevices( struct List * devices );

struct MidiNode * OpenMidi( CONST_STRPTR name );
VOID CloseMidi( struct MidiNode ** node );
struct MidiLink * OpenMidiInput( struct MidiNode * node,
                                 CONST_STRPTR location,
                                 CONST_STRPTR name );
struct MidiLink * OpenMidiOutput( struct MidiNode * node,
                                  CONST_STRPTR location,
                                  CONST_STRPTR name );
VOID CloseMidiInOutput( struct MidiLink ** link );

extern struct Library           * CamdBase;

#endif /* CAMD_UTILS_H */