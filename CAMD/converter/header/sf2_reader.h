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

#include <exec/lists.h>

struct SF2_Modulator {

  struct MinNode sf2m_Node;
  UWORD sf2m_Source;       // Source modulator
  UWORD sf2m_Target;       // Target generator 
  WORD sf2m_Amount;        // Degree of modulation
  UWORD sf2m_AmountSource; // Second source controlling amount of first source
  UWORD sf2m_Transform;    // Transform applied to first source
};

struct SF2_Generator {

  struct MinNode sf2g_Node;
  UWORD sf2g_Id;
  UWORD sf2g_Amount;
};

struct SF2_Zone {

  struct MinNode sf2z_Node;
  struct MinList sfz2_Generators;
  struct MinList sfz2_Modulators;
};

struct SF2_Common {

  struct MinNode sf2c_Node;
  struct MinList sf2c_Zones;
};

struct SF2_Preset {

  struct SF2_Common sf2p_Common;
  UWORD sf2p_Bank;
  UWORD sf2p_Number;
  UBYTE sf2p_Name[21];
  UBYTE sf2p_Padding0;
  UWORD sf2p_Padding1;
};

struct SF2_Instrument {

  struct SF2_Common sf2i_Common;
  UWORD sf2i_Number;
  UBYTE sf2i_Name[21];
  UBYTE sf2i_Padding0;
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