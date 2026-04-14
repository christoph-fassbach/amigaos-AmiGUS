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

#ifndef SF2_H
#define SF2_H

#include <dos/dos.h>
#include <exec/lists.h>

/*
 * defines are limited to 32 chars due to a SAS/C insufficiency !!!
 *
 * So define below is just kind of a ruler...
 */
#define SASC_MAXIMUM_DEFINE_LENGTH_IS_32 12345678

#define SF2_COMMON_NO_TYPE               0
#define SF2_COMMON_PRESET_TYPE           1
#define SF2_COMMON_INSTRUMENT_TYPE       2

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
  UWORD sf2c_Number;
  UBYTE sf2c_Name[21];
  UBYTE sf2c_Type;
};

struct SF2_Preset {

  struct SF2_Common sf2p_Common;
  UWORD sf2p_Bank;
  UWORD sf2p_Padding0;
};

struct SF2_Instrument {

  struct SF2_Common sf2i_Common;
};

struct SF2_Sample {

  struct MinNode sf2s_Node;
  ULONG sf2s_SampleStartOffset;
  ULONG sf2s_SampleEndOffset;
  ULONG sf2s_LoopStartOffset;
  ULONG sf2s_LoopEndOffset;
  ULONG sf2s_SampleRate;
  UWORD sf2s_Number;
  UWORD sf2s_SampleType;
  UBYTE sf2s_SampleNote;
  UBYTE sf2s_Name[21];
};

struct SF2 {

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
  struct MinList sf2_Instruments;
  struct MinList sf2_Presets;
};

#endif /* SF2_H */