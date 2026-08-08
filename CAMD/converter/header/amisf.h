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

#ifndef AMISF_H
#define AMISF_H

#include <exec/types.h>

// TODO: Most need to be sample-local!
#define AMISF_NOTE_RESOLUTION_MASK       0x0001
#define AMISF_NOTE_RESOLUTION_8BIT       0x0000
#define AMISF_NOTE_RESOLUTION_16BIT      0x0001

#define AMISF_NOTE_LOOPED_MASK           0x0002
#define AMISF_NOTE_INTERPOLATION_MASK    0x0004
//efine AMISF_NOTE_ENDIANESS_MASK        0x0008 // No, we do not do that here!
#define AMISF_NOTE_ENVELOPE_MASK         0x0020

#define AMISF_NOTE_OTHER_NOTE_MASK       0x1000
#define AMISF_NOTE_NOT_OTHER_NOTE_MASK   0x0000
#define AMISF_NOTE_OTHER_NOTE_MASK       0x1000

#define AMISF_NOTE_IN_FILE_MASK          0x2000
#define AMISF_NOTE_NOT_IN_FILE           0x0000
#define AMISF_NOTE_IN_FILE               0x2000

#define AMISF_NOTE_IN_RAM_MASK           0x4000
#define AMISF_NOTE_NOT_IN_RAM            0x0000
#define AMISF_NOTE_IN_RAM                0x4000

#define AMISF_NOTE_IN_CARD_MASK          0x8000
#define AMISF_NOTE_NOT_IN_CARD           0x0000
#define AMISF_NOTE_IN_CARD               0x8000

struct SF2;

struct AmiSF_Sample {

  ULONG asfs_Flags;
  ULONG asfs_StartOffset; // relative to binary start in disk, RAM, AmiGUS
  ULONG asfs_EndOffset;
  ULONG asfs_LoopOffset;
};

struct AmiSF_Note {
  UWORD asfn_Volume;
  UBYTE asfn_MaxNote;
  UBYTE asfn_BaseNote;

  ULONG asfn_BasePlaybackIndex;    // Version 01: AmiGUS register format - Version 11: real sample rate

  UWORD asfn_Attack;
  UWORD asfn_Decay;
  UWORD asfn_Sustain;
  UWORD asfn_Release;

  UWORD asfn_SampleIndex;
};

struct AmiSF_Preset {

  UBYTE asfp_Bank;
  UBYTE asfp_Preset;
  UWORD asfp_NoteCount;

  ULONG asfp_NoteStart;
};

struct AmiSF {

  struct AmiSF_Preset asf_Preset[ 129 ][ 129 ];

  ULONG asf_NoteCount;
  struct AmiSF_Note * asf_Note;

  ULONG asf_SampleCount;
  struct AmiSF_Sample * asf_SampleMetadata;

  ULONG asf_SampleRateCount;
  ULONG * asf_SampleRate;
  ULONG * asf_PlaybackRateOffset;

  ULONG asf_PlaybackRateCount;
  ULONG * asf_PlaybackRate;

  union {
    
    APTR asf_SampleData;
    ULONG asf_SampleBaseOffset;
  };
};

struct AmiSF * AllocAmiSFfromSF2( struct SF2 * sf2 );
VOID FreeAmiSF( struct AmiSF * amisf );

#endif /* AMISF2_H */
