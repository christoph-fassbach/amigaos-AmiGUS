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
#include <libraries/dos.h>

#define AMISF_PLAY_RESOLUTION_16BIT      0x0001 // Not set => 8bit
#define AMISF_PLAY_LOOPED                0x0002
#define AMISF_PLAY_INTERPOLATION         0x0004
//efine AMISF_PLAY_BIG_ENDIANESS         0x0008 // No, we do not do that here!
// unused                                0x0010
#define AMISF_PLAY_ENVELOPE_MODULATION   0x0020
// unused                                0x0040
// unused                                0x0080
// unused                                0x0100
// unused                                0x0200
// unused                                0x0400
// unused                                0x0800
// unused                                0x1000
// unused                                0x2000
#define AMISF_PLAY_ENVELOPE_KEY_ON       0x4000
#define AMISF_PLAY_SAMPLE_PLAYBACK_ON    0x8000

#define AMISF_STATUS_LOCATION_MASK       0x000F
#define AMISF_STATUS_LOCATION_OTHER_FILE 0x0001
#define AMISF_STATUS_LOCATION_AMISF_FILE 0x0002
#define AMISF_STATUS_LOCATION_SYS_RAM    0x0004
#define AMISF_STATUS_LOCATION_CARD_RAM   0x0008

extern STRPTR AmiSF_Suffix;

struct ConversionInfo;
struct SF2;
struct ProgressDialog;

struct AmiSF_Sample {

  UWORD asfs_PlaybackFlags;
  UWORD asfs_StatusFlags;
  ULONG asfs_StartOffset; // relative to binary start in disk, RAM, AmiGUS
  ULONG asfs_LoopOffset;
  ULONG asfs_EndOffset;
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

  BPTR asf_SampleSourceFile;
  ULONG asf_SampleSourceOffset;

  ULONG asf_SampleDataSize;
  APTR asf_SampleData;
};

struct AmiSF * AllocAmiSFfromSF2(
  struct SF2 * sf2,
  struct ConversionInfo * info,
  struct ProgressDialog * dialog
);

struct AmiSF * AllocAmiSFfromFile(
  STRPTR filePath,
  struct ProgressDialog * dialog
);

LONG WriteAmiSFtoFile(
  struct AmiSF * amisf,
  STRPTR filePath,
  struct SF2 * sf2,
  struct ConversionInfo * info,
  struct ProgressDialog * dialog
);

VOID FreeAmiSF( struct AmiSF * amisf );

#endif /* AMISF2_H */
