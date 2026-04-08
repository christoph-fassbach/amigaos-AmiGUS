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

#ifndef SF2_READER_H
#define SF2_READER_H

#include <exec/types.h>
#include <exec/lists.h>

struct SF2_Zone {

  struct MinNode sf2z_Node;
  UWORD sfz2_GeneratorIndex;
  UWORD sfz2_ModulatorIndex;
};

struct SF2_Preset {

  struct MinNode sf2p_Node;
  UWORD sf2p_Bank;
  UWORD sf2p_Number;
  UBYTE sf2p_Name[21];
  struct MinList sf2p_Zones;
};

struct SF2_Parsed {

  STRPTR sf2_FilePath;
  BPTR sf2_FileHandle;
  ULONG sf2_FileSize;
  UWORD sf2_MajorVersion;
  UWORD sf2_MinorVersion;
  LONG sf2_16bitSamplePosition;
  ULONG sf2_16bitSampleSize;
  LONG sf2_24bitSamplePosition;
  ULONG sf2_24bitSampleSize;
  LONG sf2_PresetsPosition;
  ULONG sf2_PresetsSize;

  struct MinList sf2_Samples;
  struct MinList sf2_Modulators;
  struct MinList sf2_Instruments;
  struct MinList sf2_Presets;
};

struct SF2_Parsed * AllocSf2FromFile( STRPTR filePath );
VOID FreeSf2( struct SF2_Parsed * soundFont );

#endif /* SF2_READER_H */