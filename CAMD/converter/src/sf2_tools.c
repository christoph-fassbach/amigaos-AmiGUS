
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

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/mathieeedoubbas.h>
#include <proto/mathieeedoubtrans.h>

#include "sf2_tools.h"

#include "converter.h"
#include "debug.h"
#include "support.h"

struct AmiSF_Note * CreateAmiSF_Note(
  struct SF2_Preset * preset,
  struct SF2_ArgValues * instrumentArgValues,
  struct SF2_Instrument * instrument,
  struct SF2_ArgValues * sampleArgValues,
  struct SF2_Sample * sample,
  ULONG targetNote,
  ULONG targetStartAddress ) {

  const LONG maxRate = 192000;
  const LONG maxValue = 0x40000000;
  LONG sourceNote = sample->sf2s_SampleNote;
  LONG sourceRate = sample->sf2s_SampleRate;
  // targetRate = sourceRate * 2 ^ (( targetNote - sourceNote ) / 12 )
  // targetRate = sourceRate * 2 ^ (             a              /  b )
  // targetRate = sourceRate * d ^ (                  c              )
  // targetRate = sourceRate * e
  // targetRate = f          * e
  // g          = f          * e
  double a = IEEEDPFlt( targetNote - sourceNote );
  double b = IEEEDPFlt( 12 );
  double c = IEEEDPDiv( a, b );
  double d = IEEEDPFlt( 2 );
  double e = IEEEDPPow( c, d );
  double f = IEEEDPFlt( sourceRate );
  double g = IEEEDPMul( f, e );
  LONG targetRate = IEEEDPFix( g );
  // targetRegisterValue = ( sampleRate / maxRate ) * maxValue
  // targetRegisterValue = ( g          /       h ) * maxValue
  // targetRegisterValue =              i           * maxValue
  // targetRegisterValue =              i           * j
  // k                   =              i           * j
  double h = IEEEDPFlt( maxRate );
  double i = IEEEDPDiv( g, h );
  double j = IEEEDPFlt( maxValue );
  double k = IEEEDPMul( i, j );
  LONG targetRegisterValue = IEEEDPFix( k );

  struct AmiSF_Note * note = AllocMem( sizeof( struct AmiSF_Note ),
                                       MEMF_ANY | MEMF_CLEAR );

  note->amisf_NoteFlags =
      AMISF_NOTE_RESOLUTION_16BIT // SF2 only knows 16 or 24bit
    | AMISF_NOTE_LOOPED_MASK
    | AMISF_NOTE_IN_FILE
    | AMISF_NOTE_NOT_IN_RAM
    | AMISF_NOTE_NOT_IN_CARD;
  note->amisf_StartOffset = targetStartAddress;                          // BYTE
  note->amisf_LoopOffset = targetStartAddress                            // BYTE
    + (( sample->sf2s_LoopStartOffset - sample->sf2s_SampleStartOffset ) // WORD
    << 1 );                                                              // BYTE
  note->amisf_EndOffset =  targetStartAddress                            // BYTE
    + (( sample->sf2s_LoopEndOffset - sample->sf2s_SampleStartOffset )   // WORD
    << 1 );                                                              // BYTE
  note->amisf_PlaybackRate = targetRegisterValue;
  LOG_V(( "V: a=%ld b=%ld c=%ld d=%ld e=%ld f=%ld "
          "g=%ld h=%ld i=%ld j=%ld k=%ld=0x%08lx\n",
          IEEEDPFix( a ),
          IEEEDPFix( b ),
          IEEEDPFix( c ),
          IEEEDPFix( d ),
          IEEEDPFix( e ),
          IEEEDPFix( f ),
          IEEEDPFix( g ),
          IEEEDPFix( h ),
          IEEEDPFix( i ),
          IEEEDPFix( j ),
          IEEEDPFix( k ),
          IEEEDPFix( k )));
  LOG_D(( "D: Source: Note %ld, sample rate %ld\n", sourceNote, sourceRate ));
  LOG_D(( "D: Target: Note %ld, sample rate %ld, register value 0x%08lx\n",
          targetNote, targetRate, targetRegisterValue ));

  return note;
}

struct AmiSF_Note * GetNoteAtIndex( struct SF2 * sf2, const ULONG index ) {

  struct SF2_Preset * preset;
  ULONG current = 0;

  FOR_LIST( &( sf2->sf2_Presets ),
            preset,
            struct SF2_Preset * ) {

    struct SF2_Args * argsP;

    FOR_LIST( &( preset->sf2p_Args ),
              argsP,
              struct SF2_Args * ) {

      struct SF2_Instrument * instrument = 
        sf2->sf2_InstrumentArray[ argsP->sf2a_Values.sf2v_NextNumber ];
      struct SF2_ArgValues * instrumentArgValues = &( argsP->sf2a_Values );
      struct SF2_Args * argsI;

      FOR_LIST( &( instrument->sf2i_Args ),
                argsI,
                struct SF2_Args * ) {

        struct SF2_Sample * sample =
          sf2->sf2_SampleArray[ argsI->sf2a_Values.sf2v_NextNumber ];
        struct SF2_ArgValues * sampleArgValues = &( argsI->sf2a_Values );
        if ( index == current ) {

          return CreateAmiSF_Note( preset,
                                   instrumentArgValues,
                                   instrument,
                                   sampleArgValues,
                                   sample,
                                   sample->sf2s_SampleNote,
                                   0 );
        }
        ++current;
      }
    }
  }
  return NULL;
}

APTR GetSampleForNote( struct SF2 * sf2, struct AmiSF_Note * note ) {

  ULONG size = note->amisf_EndOffset - note->amisf_StartOffset;
  APTR samples = AllocMem( size, MEMF_ANY | MEMF_CLEAR );
  UWORD * sample = samples;

  LOG_D(( "V: Samples @ 0x%08lx, start %ld - end %ld = size %ld\n",
          samples,
          note->amisf_StartOffset, note->amisf_EndOffset, size ));
  Seek( sf2->sf2_FileHandle, 
        sf2->sf2_16bitSamplePosition + note->amisf_StartOffset,
        OFFSET_BEGINNING );
  Read( sf2->sf2_FileHandle, samples, size );

  // Now counting size in 16bit samples!
  size >>= 1;
  while ( size ) {

    sample[ size ] = Swap16( sample[ size ]);
    size--;
  }

  note->amisf_NoteFlags |= AMISF_NOTE_IN_RAM;

  return samples;
}
