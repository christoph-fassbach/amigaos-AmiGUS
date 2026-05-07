
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

LONG GetTargetSampleRate( LONG sourceNote, LONG sourceRate, LONG targetNote ) {

  const LONG maxRate = 192000;      // TODO: move to AmiGUS HW constants?
  const LONG maxValue = 0x40000000; // TODO: move to AmiGUS HW constants?
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
  // LONG targetRate = IEEEDPFix( g );
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
          targetRegisterValue,
          targetRegisterValue ));
  LOG_D(( "D: Converted note %ld + rate %ld -> "
          "note %ld + rate %ld = register 0x%08lx\n",
          sourceNote, sourceRate,
          targetNote, IEEEDPFix( g ), targetRegisterValue ));

  return targetRegisterValue;
}

struct AmiSF_Note * CreateAmiSF_NoteFULL(
  struct SF2_Preset * preset,
  struct SF2_Instrument * instrument,
  struct SF2_Sample * sample,
  ULONG targetNote,
  ULONG targetStartAddress ) {

  return NULL;
}

struct AmiSF_Note * CreateAmiSF_NoteAtIndex( struct SF2 * sf2, const ULONG index ) {

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
      struct SF2_Args * argsI;

      FOR_LIST( &( instrument->sf2i_Args ),
                argsI,
                struct SF2_Args * ) {

        struct SF2_Sample * sample =
          sf2->sf2_SampleArray[ argsI->sf2a_Values.sf2v_NextNumber ];
        if ( index == current ) {

          struct AmiSF_Note * note =
            CreateAmiSF_NoteFULL( preset,
                              instrument,
                              sample,
                              sample->sf2s_SampleNote,
                              0 );
          return note;
        }
        ++current;
      }
    }
  }
  return NULL;
}

struct SF2_Sample * GetSf2SampleAtIndex( struct SF2 * sf2, const ULONG index ) {

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
      struct SF2_Args * argsI;

      FOR_LIST( &( instrument->sf2i_Args ),
                argsI,
                struct SF2_Args * ) {

        struct SF2_Sample * sample =
          sf2->sf2_SampleArray[ argsI->sf2a_Values.sf2v_NextNumber ];
        if ( index == current ) {

          return sample;
        }
        ++current;
      }
    }
  }
  return NULL;
}

struct AmiSF_Note * CreateAmiSF_Note(
  struct SF2_Sample * sample,
  ULONG targetNote,
  ULONG targetStartAddress ) {

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
    + GetSF2SampleSize( sample );                                        // BYTE
  note->amisf_PlaybackRate = GetTargetSampleRate( sample->sf2s_SampleNote,
                                                  sample->sf2s_SampleRate,
                                                  targetNote );

  return note;
}

LONG GetSF2SampleSize( struct SF2_Sample * sample ) {

  LONG size = -1;

  if (( sample->sf2s_SampleEndOffset >= sample->sf2s_LoopEndOffset )
    && ( sample->sf2s_LoopEndOffset > sample->sf2s_LoopStartOffset )
    && ( sample->sf2s_LoopStartOffset >= sample->sf2s_SampleStartOffset )) {

    size = sample->sf2s_LoopEndOffset - sample->sf2s_SampleStartOffset;
    size <<= 1;
    size += 3;
    size &= 0xFFffFFfc;

  } else { 

    LOG_E(( "E: Cannot make sense out of sample loop location\n" ));
  }

  return size;
}

APTR GetSF2SampleData( struct SF2 * sf2, struct SF2_Sample * sample ) {

  LONG sampleStart = sample->sf2s_SampleStartOffset << 1; // in BYTE
  LONG loopEnd = sample->sf2s_LoopEndOffset << 1;         // in BYTE
  LONG diskSize = loopEnd - sampleStart;
  LONG memorySize = GetSF2SampleSize( sample );
  LONG i;
  UWORD * samples = AllocMem( memorySize, MEMF_ANY | MEMF_CLEAR );


  LOG_D(( "V: Samples @ 0x%08lx, start %ld, memsize %ld, disksize %ld\n",
          samples, sampleStart, memorySize, diskSize ));
  Seek( sf2->sf2_FileHandle, 
        sf2->sf2_16bitSamplePosition + sampleStart,
        OFFSET_BEGINNING );
  Read( sf2->sf2_FileHandle, samples, diskSize );

  // Now counting size in 16bit samples!
  for ( i = 0; ( i < diskSize >> 1 ); ++i ) {

    samples[ i ] = Swap16( samples[ i ]);
  }
  if ( diskSize < memorySize ) {

    samples[ i ] = samples[ i - 1 ];
    LOG_I(( "I: Padding sample size %ld to %ld\n", diskSize, memorySize ));
  }

  LOG_D(( "D: 16 bytes of sample data: %04lx %04lx %04lx %04lx\n",
          samples[ 0 ], samples[ 1 ], samples[ 2 ], samples[ 3 ] ));

  return samples;
}

APTR GetAmiSF_SampleData( struct SF2 * sf2, struct SF2_Sample * sample ) {

  /*
  APTR sf2Data = GetSF2SampleData( sf2, sample );
  ULONG size = sample->amisf_EndOffset - sample->amisf_StartOffset;

  APTR monoData;
  if ( !( 8 & sample->sf2s_SampleType )) {

    return sf2Data;
  }

  monoData = AllocMem( size >> 1, MEMF_ANY | MEMF_CLEAR );

*/
return NULL;

}