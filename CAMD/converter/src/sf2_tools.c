
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

UWORD GetTargetADSR( LONG timecents ) {

  // 1200 * ln(seconds)/ln(2) = timecents
  // => seconds = e^( timecents * ( ln( 2 ) / 1200 ) )
  //              | |     |       | |   | |    |   | |
  //              | |     |       | |   a |    |   | |
  //              | |     |       | +--b--+    c   | |
  //              | |     e       +---------d------+ |
  //              | +-----------f--------------------+
  //              +-------------g--------------------+
  double a = IEEEDPFlt( 2 );
  double b = IEEEDPLog( a );
  double c = IEEEDPFlt( 1200 );
  double d = IEEEDPDiv( b, c );
  double e = IEEEDPFlt( timecents );
  double f = IEEEDPMul( e, d );
  double g = IEEEDPExp( f );

  // increment = ( maxULONG / ( seconds  * 192.000 ))
  //             |     |      |    |          |  ||
  //             |     |      |    g          h  ||
  //             |     j      +----------i-------+|
  //             +----------k---------------------+
  double h = IEEEDPFlt( 192000 );
  double i = IEEEDPMul( g, h );
  double j = IEEEDPMul( IEEEDPFlt( 0x7fFFffFF ), a );
  double k = IEEEDPDiv( j, i );

  // exponent = floor( ( ln( increment ) / ln( 2 )))
  //            |      | |       |     |   |   | |||
  //            |      | |       k     |   |   a |||
  //            |      | +------l------+   +--b--+||
  //            |      +-----------------m--------+|
  //            +-----------------n----------------+

  double l = IEEEDPLog( k );
  double m = IEEEDPDiv( l, b );
  double n = IEEEDPFloor( m );
  const LONG idealExponent = IEEEDPFix( n );
  const LONG exponent = MIN( idealExponent, 0x0000000F );

  // mantissa = ( increment / ( 2 ^ exponent ))
  // mantissa = ( increment / ( e ^ ( exponent * ln( 2 ))))
  //            |     |       |     |     |      |   | ||||
  //            |     |       |     |     |      |   a ||||
  //            |     |       |     |     o      +--b--+|||
  //            |     |       |     +----------p--------+||
  //            |     k       +---q----------------------+|
  //            +-----------r-----------------------------+
  double o = IEEEDPFlt( exponent );
  double p = IEEEDPMul( o, b );
  double q = IEEEDPExp( p );
  double r = IEEEDPDiv( k, q );
  const LONG idealMantissa = IEEEDPFix( r );
  const LONG mantissa = MIN( idealMantissa, 0x00000FFF );
  const UWORD adsr = ( exponent << 12 ) | mantissa;

  LOG_D(( "D: %ld timecents => %lu ms => increment %lu"
          " => %lu * 2 ^ %lu"
          " => %lu * 2 ^ %lu"
          " => %lu adsr => %lu increment => %lu ms\n",
          timecents,
          IEEEDPFix( IEEEDPMul( g, IEEEDPFlt( 1000 ))),
          IEEEDPFix( IEEEDPDiv( k, a )) << 1,
          IEEEDPFix( IEEEDPDiv( k, IEEEDPExp( IEEEDPMul( n, b )))),
          idealExponent,
          mantissa,
          exponent,
          adsr,
          mantissa << exponent,
          (((( ULONG ) 0xffFFffFF ) / 192 ) / mantissa ) >> exponent ));

  return 0;//adsr;
}

struct AmiSF_Note * CreateAmiSF_Note(
  struct SF2_Preset * preset,
  struct SF2_Instrument * instrument,
  struct SF2_Sample * sample,
  ULONG targetNote ) {

  // ADSR combination logic is in 
  // "SoundFont Technical Specification Version 2.01 July 23, 1998"
  // aka SFSpec21.pdf, p.57
  LONG effectiveAttack  = INSTRUMENT_DEFAULT_ATTACK_VALUE;
  LONG effectiveDecay   = INSTRUMENT_DEFAULT_DECAY_VALUE;
  LONG effectiveSustain = INSTRUMENT_DEFAULT_SUSTAIN_VALUE;
  LONG effectiveRelease = INSTRUMENT_DEFAULT_RELEASE_VALUE;

  struct SF2_Args * argsP;
  struct SF2_Args * argsI;

  struct AmiSF_Note * result = AllocMem( sizeof( struct AmiSF_Note ),
                                         MEMF_ANY | MEMF_CLEAR );

  result->amisfn_PlaybackRate = GetTargetSampleRate( sample->sf2s_SampleNote,
                                                     sample->sf2s_SampleRate,
                                                     targetNote );



  LOG_D(( "D: Found S-%ld\n", sample->sf2s_Number ));

  FOR_LIST( &( instrument->sf2i_Args ),
            argsI,
            struct SF2_Args * ) {

    const LONG sampleIndex = argsI->sf2a_Values.sf2v_NextNumber;
    const LONG sampleMin = argsI->sf2a_Values.sf2v_LowNote;
    const LONG sampleMax = argsI->sf2a_Values.sf2v_HighNote;

    if (( sampleMin <= targetNote )        // TODO: >= ? <= ?
      && ( sampleMax >= targetNote )
      && ( 0 <= sampleIndex )) {

      LOG_D(( "D: Found in I-%ld %ld < %ld < %ld with A:%ld D:%ld S:%ld R:%ld -> S: %ld\n",
        instrument->sf2i_Common.sf2c_Number,
        sampleMin,
        targetNote,
        sampleMax,
        argsI->sf2a_Values.sf2v_Attack,
        argsI->sf2a_Values.sf2v_Decay,
        argsI->sf2a_Values.sf2v_Sustain,
        argsI->sf2a_Values.sf2v_Release,
        sampleIndex ));

      effectiveAttack = argsI->sf2a_Values.sf2v_Attack;
      effectiveDecay = argsI->sf2a_Values.sf2v_Decay;
      effectiveSustain = argsI->sf2a_Values.sf2v_Sustain;
      effectiveRelease = argsI->sf2a_Values.sf2v_Release;
    }
  }

  FOR_LIST( &( preset->sf2p_Args ),
            argsP,
            struct SF2_Args * ) {

    const LONG instrumentMin = argsP->sf2a_Values.sf2v_LowNote;
    const LONG instrumentMax = argsP->sf2a_Values.sf2v_HighNote;
    const LONG instrumentIndex = argsP->sf2a_Values.sf2v_NextNumber;

    if (( instrumentMin <= targetNote )
      && ( instrumentMax >= targetNote )
      && ( 0 <= instrumentIndex )) {

      //struct SF2_Instrument * tempInstrument= sf2->sf2_InstrumentArray[ instrumentIndex ];
      

      LOG_D(( "D: Found in P-%ld %ld < %ld < %ld with A:%ld D:%ld S:%ld R:%ld -> I: %ld\n",
        preset->sf2p_Common.sf2c_Number,
        instrumentMin,
        targetNote,
        instrumentMax,
        argsP->sf2a_Values.sf2v_Attack,
        argsP->sf2a_Values.sf2v_Decay,
        argsP->sf2a_Values.sf2v_Sustain,
        argsP->sf2a_Values.sf2v_Release,
        instrumentIndex ));

      if ( PRESET_DEFAULT_ATTACK_VALUE == argsP->sf2a_Values.sf2v_Attack ) {

        effectiveAttack += argsP->sf2a_Values.sf2v_Attack;
      }
      if ( PRESET_DEFAULT_DECAY_VALUE == argsP->sf2a_Values.sf2v_Decay ) {

        effectiveDecay += argsP->sf2a_Values.sf2v_Decay;
      }
      if ( PRESET_DEFAULT_SUSTAIN_VALUE == argsP->sf2a_Values.sf2v_Sustain ) {

        effectiveSustain += argsP->sf2a_Values.sf2v_Sustain;
      }
      if ( PRESET_DEFAULT_RELEASE_VALUE == argsP->sf2a_Values.sf2v_Release ) {

        effectiveRelease += argsP->sf2a_Values.sf2v_Release;
      }
    }
  }

  LOG_D(( "V: Effective A: %lx D: %lx S: %lx R: %lx\n",
          effectiveAttack,
          effectiveDecay,
          effectiveSustain,
          effectiveRelease ));

  result->amisfn_Attack = GetTargetADSR( effectiveAttack );
  result->amisfn_Decay = GetTargetADSR( effectiveDecay );
  result->amisfn_Sustain = GetTargetADSR( effectiveSustain );
  result->amisfn_Release = GetTargetADSR( effectiveRelease );

  LOG_D(( "V: Final A: %lx D: %lx S: %lx R: %lx\n",
          result->amisfn_Attack,
          result->amisfn_Decay,
          result->amisfn_Sustain,
          result->amisfn_Release ));

  return result;
}

struct AmiSF_Sample * CreateAmiSF_Sample(
  struct SF2_Preset * preset,
  struct SF2_Instrument * instrument,
  struct SF2_Sample * sample,
  ULONG targetStartAddress ) {

  struct AmiSF_Sample * result = AllocMem( sizeof( struct AmiSF_Sample ),
                                           MEMF_ANY | MEMF_CLEAR );
  result->amisfs_Flags =
      AMISF_NOTE_RESOLUTION_16BIT // SF2 only knows 16 or 24bit
    | AMISF_NOTE_LOOPED_MASK
    | AMISF_NOTE_NOT_IN_FILE
    | AMISF_NOTE_IN_RAM
    | AMISF_NOTE_NOT_IN_CARD;
  result->amisfs_StartOffset = targetStartAddress;                       // BYTE
  result->amisfs_LoopOffset = targetStartAddress                         // BYTE
    + (( sample->sf2s_LoopStartOffset - sample->sf2s_SampleStartOffset ) // WORD
    << 1 );                                                              // BYTE
  result->amisfs_EndOffset =  targetStartAddress                         // BYTE
    + GetSF2SampleSize( sample );                                        // BYTE

  return result;
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

    LOG_E(( "E: Cannot make sense out of sample loop location, "
            "sample: start %ld end %ld, loop: start %ld end %ld\n",
            sample->sf2s_SampleStartOffset,
            sample->sf2s_SampleEndOffset,
            sample->sf2s_LoopStartOffset,
            sample->sf2s_LoopEndOffset ));
  }

  return size;
}

APTR GetSF2SampleData( struct SF2 * sf2, struct SF2_Sample * sample ) {

  LONG sampleStart = sample->sf2s_SampleStartOffset << 1; // in BYTE
  LONG loopEnd = sample->sf2s_LoopEndOffset << 1;         // in BYTE
  LONG diskSize = loopEnd - sampleStart;
  LONG memorySize = GetSF2SampleSize( sample );
  LONG i;
  UWORD * samples;

  if (( sampleStart > loopEnd )
    || ( 0 > sampleStart )
    || ( 0 > loopEnd )
    || ( 0 > diskSize )
    || ( 0 > memorySize )) {

    LOG_E(( "E: Implausible sample data, "
            "start %ld, end %ld memsize %ld, disksize %ld\n",
            sampleStart, loopEnd, memorySize, diskSize ));
    return NULL;
  }

  samples = AllocMem( memorySize, MEMF_ANY | MEMF_CLEAR );

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
