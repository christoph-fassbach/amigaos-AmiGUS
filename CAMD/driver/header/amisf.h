/*
 * This file is part of the AmiGUS CAMD MIDI.
 *
 * AmiGUS CAMD MIDI is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS CAMD MIDI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with AmiGUS CAMD MIDI.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AMISF_H
#define AMISF_H

#include <dos/dos.h>
#include <exec/types.h>

/*
 * defines are limited to 32 chars due to a SAS/C insufficiency !!!
 *
 * So define below is just kind of a ruler...
 */
#define SASC_MAXIMUM_DEFINE_LENGTH_IS_32 12345678
#define CHAR_TO_ULONG( a, b, c, d )      ((( a ) << 24 ) | (( b ) << 16 ) | (( c ) << 8 ) | ( d ))
#define CHAR_TO_UWORD( a, b )            ((( a ) << 8 ) | ( b ))

#define AMISF_IDENTIFIER_0               CHAR_TO_ULONG( 'A', 'm', 'i', 'S' )
#define AMISF_IDENTIFIER_1               CHAR_TO_UWORD( 'F', 0 )

struct AmiSF_Note {

    UWORD amisf_NoteFlags;
    UBYTE amisf_OtherBank;   // not so sure...
    UBYTE amisf_OtherPreset; // not sure either...
    ULONG amisf_StartOffset;
    ULONG amisf_LoopOffset;
    ULONG amisf_EndOffset;
    ULONG amisf_PlaybackRate;    // Version 01: AmiGUS register format - Version 11: real sample rate
    UWORD amisf_Attack;
    UWORD amisf_Decay;
    UWORD amisf_Sustain;
    UWORD amisf_Release;
};

#define AMISF_NOTE_RESOLUTION_MASK       0x0001
#define AMISF_NOTE_RESOLUTION_8BIT       0x0000
#define AMISF_NOTE_RESOLUTION_16BIT      0x0001

#define AMISF_NOTE_LOOPED_MASK           0x0002
#define AMISF_NOTE_INTERPOLATION_MASK    0x0004
#define AMISF_NOTE_ENVELOPE_MASK         0x0008

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

struct AmiSF_Bank {

  struct AmiSF_Note amisf_Notes[ 128 ];
};

struct AmiSF_Data {

  // size for allocation: sizeof( struct AmiSF_Data ) + ( amisf_MaxBank * ( sizeof( struct AmiSF_Note ) << 7 ))

  BPTR              amisf_FileHandle;
  APTR              amisf_SampleBlob;
  // Check only: AMISF_IDENTIFIER_0
  // Check only: AMISF_IDENTIFIER_1
  UBYTE             amisf_Version;   // Version 1 -> AmiGUS only
  UBYTE             amisf_Revision;
  UBYTE             amisf_MaxBank;
  UBYTE             amisf_Padding;

  ULONG             amisf_SampleOffset; // Offset from start of file!
  ULONG             amisf_SampleSize;   // Offset from start of file!

  struct AmiSF_Note amisf_Notes[ 1 ][ 128 ];
  //                             |     |
  //                             |     +---- Preset
  //                             +---------- Bank
};

#endif /* AMISF_H */