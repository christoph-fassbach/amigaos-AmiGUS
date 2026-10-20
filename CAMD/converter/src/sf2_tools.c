
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

/*******************************************************************************
 * Private functions.
 ******************************************************************************/

/**
 * Converts a "timecents" value storing AD_R time values in SF2 format
 * to seconds.
 *
 * @param timecents Time value for ADSR envelopes in SF2 format.
 *
 * @return Time value to seconds.
 */
static DOUBLE GetSecondsFromTimecents( WORD timecents ) {

  // 1200 * ln(seconds)/ln(2) = timecents
  // => seconds = e^( timecents * ( ln( 2 ) / 1200 ) )
  //              | |     |       | |   | |    |   | |
  //              | |     |       | |   a |    |   | |
  //              | |     |       | +--b--+    c   | |
  //              | |     e       +---------d------+ |
  //              | +-----------f--------------------+
  //              +-------------g--------------------+
  DOUBLE a = IEEEDPFlt( 2 );
  DOUBLE b = IEEEDPLog( a );
  DOUBLE c = IEEEDPFlt( 1200 );
  DOUBLE d = IEEEDPDiv( b, c );
  DOUBLE e = IEEEDPFlt( timecents );
  DOUBLE f = IEEEDPMul( e, d );
  DOUBLE seconds = IEEEDPExp( f );

  return seconds;
}

/*******************************************************************************
 * Public functions.
 ******************************************************************************/

LONG GetTargetSampleRate( LONG sourceNote, LONG sourceRate, LONG targetNote ) {

  const LONG maxRate = 192000;      // TODO: move to AmiGUS HW constants?
  const LONG maxValue = 0x40000000; // TODO: move to AmiGUS HW constants?
  // targetRate = sourceRate * 2 ^ (( targetNote - sourceNote ) / 12 )
  // targetRate = sourceRate * 2 ^ (             a              /  b )
  // targetRate = sourceRate * d ^ (                  c              )
  // targetRate = sourceRate * e
  // targetRate = f          * e
  // g          = f          * e
  DOUBLE a = IEEEDPFlt( targetNote - sourceNote );
  DOUBLE b = IEEEDPFlt( 12 );
  DOUBLE c = IEEEDPDiv( a, b );
  DOUBLE d = IEEEDPFlt( 2 );
  DOUBLE e = IEEEDPPow( c, d );
  DOUBLE f = IEEEDPFlt( sourceRate );
  DOUBLE g = IEEEDPMul( f, e );
  // LONG targetRate = IEEEDPFix( g );
  // targetRegisterValue = ( sampleRate / maxRate ) * maxValue
  // targetRegisterValue = ( g          /       h ) * maxValue
  // targetRegisterValue =              i           * maxValue
  // targetRegisterValue =              i           * j
  // k                   =              i           * j
  DOUBLE h = IEEEDPFlt( maxRate );
  DOUBLE i = IEEEDPDiv( g, h );
  DOUBLE j = IEEEDPFlt( maxValue );
  DOUBLE k = IEEEDPMul( i, j );
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
  LOG_V(( "V: Converted note %ld + rate %ld -> "
          "note %ld + rate %ld = register 0x%08lx\n",
          sourceNote, sourceRate,
          targetNote, IEEEDPFix( g ), targetRegisterValue ));

  return targetRegisterValue;
}

UWORD GetTargetADR( LONG timecents ) {

  DOUBLE seconds = GetSecondsFromTimecents( timecents );

  DOUBLE a = IEEEDPFlt( 2 );
  DOUBLE b = IEEEDPLog( a );

  // increment = ( maxULONG / ( seconds  * 192.000 ))
  //             |     |      |    |          |  ||
  //             |     |      |    g          h  ||
  //             |     j      +----------i-------+|
  //             +----------k---------------------+
  DOUBLE h = IEEEDPFlt( 192000 );
  DOUBLE i = IEEEDPMul( seconds, h );
  DOUBLE j = IEEEDPMul( IEEEDPFlt( 0x7fFFffFF ), a );
  DOUBLE k = IEEEDPDiv( j, i );
  const LONG increment = IEEEDPFix( k );

  LONG idealExponent;
  LONG exponent;
  LONG idealMantissa;
  LONG mantissa;
  UWORD adsr;

  if ( increment < ( 1 << 12 )) {

    // Easy case: Mantissa does the job!

    idealExponent = 0;
    exponent = 0;
    mantissa = increment;

  } else {

    DOUBLE l;
    DOUBLE m;
    DOUBLE n;
    DOUBLE o;
    DOUBLE p;
    DOUBLE q;
    DOUBLE r;

    // The complicated case: we want to use full Mantissa precision available,
    // so we calculate the component and limit its length more than needed! :D

    // exponent = floor( ( ln( increment ) / ln( 2 )))
    //            |      | |       |     |   |   | |||
    //            |      | |       k     |   |   a |||
    //            |      | +------l------+   +--b--+||
    //            |      +-----------------m--------+|
    //            +-----------------n----------------+
    l = IEEEDPLog( k );
    m = IEEEDPDiv( l, b );
    n = IEEEDPFloor( m );
    idealExponent = IEEEDPFix( n );
    // A maximum of idealExponent = 24 arrives here
    // for -12000 <= timecents < 8000
    exponent = idealExponent - 11;
    // -11 nicely blends that into our value format range.
    // Just to be sure...
    exponent = MIN( exponent, 0x0000000F );

    // Finally, we calculate the mantissa!
    // mantissa = ( increment / ( 2 ^ exponent ))
    // mantissa = ( increment / ( e ^ ( exponent * ln( 2 ))))
    //            |     |       |     |     |      |   | ||||
    //            |     |       |     |     |      |   a ||||
    //            |     |       |     |     o      +--b--+|||
    //            |     |       |     +----------p--------+||
    //            |     k       +---q----------------------+|
    //            +-----------r-----------------------------+
    o = IEEEDPFlt( exponent );
    p = IEEEDPMul( o, b );
    q = IEEEDPExp( p );
    r = IEEEDPDiv( k, q );
    idealMantissa = IEEEDPFix( r );
    mantissa = MIN( idealMantissa, 0x00000FFF );

    if ( idealMantissa & 0xFFFFF000 ) {

      LOG_W(( "W: ADR miscalculation ahead!\n" ));
      return 0;
    }
  }

  adsr = ( exponent << 12 ) | mantissa;

  LOG_V(( "V: %ld timecents => %lu ms => increment %lu"
          " => %lu * 2 ^ %lu"
          " => %lu * 2 ^ %lu"
          " => %lu adsr => %lu increment => %lu ms\n",
          timecents,
          IEEEDPFix( IEEEDPMul( g, IEEEDPFlt( 1000 ))),
          increment,
          IEEEDPFix( IEEEDPDiv( k, IEEEDPExp( 
            IEEEDPMul( IEEEDPFlt( idealExponent ), b )))),
          idealExponent,
          mantissa,
          exponent,
          adsr,
          mantissa << exponent,
          (((( ULONG ) 0xffFFffFF ) / 192 ) / mantissa ) >> exponent ));

  return adsr;
}

UWORD GetTargetS( WORD centibels ) {

  // Sustain logic is in 
  // "SoundFont Technical Specification Version 2.01 July 23, 1998"
  // aka SFSpec21.pdf, p.44

  // Factor is meant to be used like:
  // Sustain Volume = ((( Original Volume ) * ( Factor )) >> 16 )
  // Original Volume is not known here, so we only calculate factor!
  LONG factor;

  if ( 0 > centibels ) {

    centibels = 0;
  }
  if ( 0 == centibels ) {

    // Max factor is 65535 ( and not 65536 ),
    // hence the default volume shall be
    // ( 0x4001 * 0xFFff ) >> 16 = 0x4000
    // =:D
    factor = ( 1 << 16 ) - 1;

  } else {

    // Factor = ( 10 ^ ( -(  decibels /   20 )))
    // Factor = ( 10 ^ ( -( centibels /  200 )))
    // Factor = ( 10 ^ (    centibels / -200  ))
    //          |  |   |         |        |   ||
    //          |  |   |         a        b   ||
    //          |  d   +--------------c-------+|
    //          +----e-------------------------+
    DOUBLE a = IEEEDPFlt( centibels );
    DOUBLE b = IEEEDPFlt( -200 );
    DOUBLE c = IEEEDPDiv( a, b );
    DOUBLE d = IEEEDPFlt( 10 );
    DOUBLE e = IEEEDPPow( c, d );

    // so...
    // SustainVolume = MaxVolume x Factor
    // but as we do not know the OriginalVolume, we are happily storing
    // the factor here - and only the factor for multiplying it when the
    // maximum volume is known in fact!
    // But as the factor is likely < 1 - so we store it shifted left by 16bits,
    // like...
    // FactorShifted = ( Factor << 16 )
    //                 |    |   |   | |
    //                 |    e   +-f-+ |
    //                 +------g-------+
    // To be applied like 
    // SustainVolume = ( MaxVolume x FactorShifted ) >> 16
    DOUBLE f = IEEEDPFlt( 1 << 16 );
    DOUBLE g = IEEEDPMul( f, e );
  
    factor = IEEEDPFix( g );
  }

  if ( 0xFFff0000 & factor ) {

    LOG_W(( "W: S miscalculation ahead!\n" ));
    return 0;
  }

  LOG_D(( "V: %ld centibels => %ld factor\n", 
          centibels,
          factor ));

  return ( UWORD ) factor;
}

LONG GetSf2SampleSize( struct SF2_Sample * sample ) {

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

APTR GetSf2SampleData( struct SF2 * sf2,
                       struct SF2_Sample * sample,
                       LONG memorySize ) {

  LONG sampleStart = sample->sf2s_SampleStartOffset << 1; // in BYTE
  LONG loopEnd = sample->sf2s_LoopEndOffset << 1;         // in BYTE
  LONG diskSize = loopEnd - sampleStart;
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

  LOG_V(( "V: Samples @ 0x%08lx, start %ld, memsize %ld, disksize %ld\n",
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

  LOG_V(( "V: 16 bytes of sample data: %04lx %04lx %04lx %04lx\n",
          samples[ 0 ], samples[ 1 ], samples[ 2 ], samples[ 3 ] ));

  return samples;
}

VOID GetSf2ADSRs( struct SF2_Args * presetArgs,
                  struct SF2_Args * instrumentArgs,
                  WORD * attack,
                  WORD * decay,
                  WORD * sustain,
                  WORD * release ) {

  // ADSR combination logic is in 
  // "SoundFont Technical Specification Version 2.01 July 23, 1998"
  // aka SFSpec21.pdf, p.57
  const WORD attackModifier = presetArgs->sf2a_Values.sf2v_Attack;
  const WORD decayModifier = presetArgs->sf2a_Values.sf2v_Decay;
  const WORD sustainModifier = presetArgs->sf2a_Values.sf2v_Sustain;
  const WORD releaseModifier = presetArgs->sf2a_Values.sf2v_Release;

  *attack = instrumentArgs->sf2a_Values.sf2v_Attack;
  *decay = instrumentArgs->sf2a_Values.sf2v_Decay;
  *sustain = instrumentArgs->sf2a_Values.sf2v_Sustain;
  *release = instrumentArgs->sf2a_Values.sf2v_Release;
  if ( PRESET_DEFAULT_ATTACK_VALUE != attackModifier ) {

    *attack += attackModifier;
  }
  if ( PRESET_DEFAULT_DECAY_VALUE != decayModifier ) {

    *decay += decayModifier;
  }
  if ( PRESET_DEFAULT_SUSTAIN_VALUE != sustainModifier ) {

    *sustain += sustainModifier;
  }
  if ( PRESET_DEFAULT_RELEASE_VALUE != releaseModifier ) {

    *release += releaseModifier;
  }
}

LONG GetMillisFromTimecents( WORD timecents ) {

  DOUBLE seconds = GetSecondsFromTimecents( timecents );

  // seconds => milliseconds
  DOUBLE h = IEEEDPFlt( 1000 );
  DOUBLE i = IEEEDPMul( seconds, h );

  LONG millis = IEEEDPFix( i );

  return millis;
}
