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

/**
 * Generator (effect) numbers (Soundfont 2.01 specifications section 8.1.3)
 */
enum SF2_Generator_Ids {
    GEN_STARTADDROFS,       // Sample start address offset (0-32767)
    GEN_ENDADDROFS,         // Sample end address offset (-32767-0)
    GEN_STARTLOOPADDROFS,   // Sample loop start address offset (-32767-32767)
    GEN_ENDLOOPADDROFS,     // Sample loop end address offset (-32767-32767)
    GEN_STARTADDRCOARSEOFS, // Sample start address coarse offset (X 32768)
    GEN_MODLFOTOPITCH,      // Modulation LFO to pitch
    GEN_VIBLFOTOPITCH,      // Vibrato LFO to pitch
    GEN_MODENVTOPITCH,      // Modulation envelope to pitch
    GEN_FILTERFC,           // Filter cutoff
    GEN_FILTERQ,            // Filter Q
    GEN_MODLFOTOFILTERFC,   // Modulation LFO to filter cutoff
    GEN_MODENVTOFILTERFC,   // Modulation envelope to filter cutoff
    GEN_ENDADDRCOARSEOFS,   // Sample end address coarse offset (X 32768)
    GEN_MODLFOTOVOL,        // Modulation LFO to volume
    GEN_UNUSED1,            // Unused
    GEN_CHORUSSEND,         // Chorus send amount
    GEN_REVERBSEND,         // Reverb send amount
    GEN_PAN,                // Stereo panning
    GEN_UNUSED2,            // Unused
    GEN_UNUSED3,            // Unused
    GEN_UNUSED4,            // Unused
    GEN_MODLFODELAY,        // Modulation LFO delay
    GEN_MODLFOFREQ,         // Modulation LFO frequency
    GEN_VIBLFODELAY,        // Vibrato LFO delay
    GEN_VIBLFOFREQ,         // Vibrato LFO frequency
    GEN_MODENVDELAY,        // Modulation envelope delay
    GEN_MODENVATTACK,       // Modulation envelope attack
    GEN_MODENVHOLD,         // Modulation envelope hold
    GEN_MODENVDECAY,        // Modulation envelope decay
    GEN_MODENVSUSTAIN,      // Modulation envelope sustain
    GEN_MODENVRELEASE,      // Modulation envelope release
    GEN_KEYTOMODENVHOLD,    // Key to modulation envelope hold
    GEN_KEYTOMODENVDECAY,   // Key to modulation envelope decay
    GEN_VOLENVDELAY,        // Volume envelope delay
    GEN_VOLENVATTACK,       // Volume envelope attack
    GEN_VOLENVHOLD,         // Volume envelope hold
    GEN_VOLENVDECAY,        // Volume envelope decay
    GEN_VOLENVSUSTAIN,      // Volume envelope sustain
    GEN_VOLENVRELEASE,      // Volume envelope release
    GEN_KEYTOVOLENVHOLD,    // Key to volume envelope hold
    GEN_KEYTOVOLENVDECAY,   // Key to volume envelope decay
    GEN_INSTRUMENT,         // Instrument ID (shouldn't be set by user)
    GEN_RESERVED1,          // Reserved
    GEN_KEYRANGE,           // MIDI note range
    GEN_VELRANGE,           // MIDI velocity range
    GEN_STARTLOOPCOARSE,    // Sample start loop address coarse offset (X 32768)
    GEN_KEYNUM,             // Fixed MIDI note number
    GEN_VELOCITY,           // Fixed MIDI velocity value
    GEN_ATTENUATION,        // Initial volume attenuation
    GEN_RESERVED2,          // Reserved
    GEN_ENDLOOPCOARSE,      // Sample end loop address coarse offset (X 32768)
    GEN_COARSETUNE,         // Coarse tuning
    GEN_FINETUNE,           // Fine tuning
    GEN_SAMPLEID,           // Sample ID (shouldn't be set by user)
    GEN_SAMPLEMODE,         // Sample mode flags
    GEN_RESERVED3,          // Reserved
    GEN_SCALETUNE,          // Scale tuning
    GEN_EXCLUSIVECLASS,     // Exclusive class number
    GEN_OVERRIDEROOTKEY,    // Sample root note override
    GEN_LAST                // Count of generators - not to be used or exceeded!
};

#define SF2_ARG_VALUE_FLAG_NEXT_NUMBER   0x00000001
#define SF2_ARG_VALUE_FLAG_LOW_NOTE      0x00000002
#define SF2_ARG_VALUE_FLAG_HIGH_NOTE     0x00000004
#define SF2_ARG_VALUE_FLAG_ALL_REQUIRED  0x00000001

// Used for the comparable part
struct SF2_ArgValues {

  LONG sf2v_NextNumber; // -1 means ignore me for ever!
  ULONG sf2v_LowNote;  // UBYTE - but need long for display.
  ULONG sf2v_HighNote; // UBYTE - but need long for display.
};

struct SF2_Args {

  struct MinNode       sf2a_Node;
  struct SF2_ArgValues sf2a_Values;
};

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
  LONG sf2c_Number; // UWORD - but need long for display.
  UBYTE sf2c_Name[21];
  UBYTE sf2c_Type;
  UWORD sf2c_Padding;
};

struct SF2_Preset {

  // From File:
  struct SF2_Common sf2p_Common;
  LONG sf2p_Bank; // UWORD - but need long for display.

  // From Flattening:
  struct MinList sf2p_Args;
};

struct SF2_Instrument {

  // From File:
  struct SF2_Common sf2i_Common;

  // From Flattening:
  struct MinList sf2i_Args;
};

struct SF2_Sample {

  struct MinNode sf2s_Node;
  ULONG sf2s_SampleStartOffset; // in WORDs
  ULONG sf2s_SampleEndOffset;   // in WORDs
  ULONG sf2s_LoopStartOffset;   // in WORDs
  ULONG sf2s_LoopEndOffset;     // in WORDs
  ULONG sf2s_SampleRate;
  LONG sf2s_Number; // UWORD - but need long for display.
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

  ULONG sf2_SampleCount;
  ULONG sf2_InstrumentCount;
  ULONG sf2_PresetCount;

  struct SF2_Sample ** sf2_SampleArray;         // Not OWN!
  struct SF2_Instrument ** sf2_InstrumentArray; // Not OWN
};

#endif /* SF2_H */
