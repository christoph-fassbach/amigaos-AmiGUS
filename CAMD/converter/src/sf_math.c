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

#include <proto/mathieeedoubbas.h>
#include <proto/mathieeedoubtrans.h>

#include "converter.h"
#include "debug.h"
#include "sf_math.h"
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

LONG GetMillisFromPeriods( UWORD periods ) {

  UWORD mantissa = periods & 0x0FFF;
  UWORD exponent = ( periods & 0xF000 ) >> 12;

  // increment = ( mantissa * ( 2 ^ exponent ))
  //             |     |      | |      |     ||
  //             |     |      | a      b     ||
  //             |     d      +----c---------+|
  //             +----------e-----------------+
  double a = IEEEDPFlt( 2 );
  double b = IEEEDPFlt( exponent );
  double c = IEEEDPPow( b, a );
  double d = IEEEDPFlt( mantissa );
  double e = IEEEDPMul( d, c );

  // increment = ( maxULONG / ( seconds  * 192.000 ))
  // seconds * increment = maxULONG / 192.000
  // seconds = ( maxULONG / ( 192.000 * increment ))
  //           |     |      |    |          |  ||
  //           |     |      |    f          e  ||
  //           |     h      +----------g-------+|
  //           +----------i---------------------+
  double f = IEEEDPFlt( 192000 );
  double g = IEEEDPMul( f, e );
  double h = IEEEDPMul( IEEEDPFlt( 0x7fFFffFF ), a );
  double i = IEEEDPDiv( h, g );

  // seconds -> millis
  double j = IEEEDPFlt( 1000 );
  double k = IEEEDPMul( j, i );

  LONG millis = IEEEDPFix( k );

  return millis;
}

LONG GetCentibelsFromFactorShift16( UWORD factorShift16 ) {

  // Usage like:
  // SustainVolume = ( OriginalVolume x FactorShifted ) >> 16
  // FactorShifted = ( Factor << 16 )
  // Factor = ( FactorShifted >> 16 )
  //          |       |       |   | |
  //          |       a       +-b-+ |
  //          +-----------c---------+
  double a = IEEEDPFlt( factorShift16 );
  double b = IEEEDPFlt( 1 << 16 );
  double c = IEEEDPDiv( a, b );

  // Factor = ( 10 ^ (    centibels / -200  ))
  // log10( Factor ) = ( centibels / -200 )
  // log10( Factor ) * ( -200 ) = centibels
  // centibels = ( -200 * log10( Factor ))
  //             |   |    |         |   ||
  //             |   |    |         c   ||
  //             |   e    +----d--------+|
  //             +---------e-------------+
  double d = IEEEDPLog10( c );
  double e = IEEEDPFlt( -200 );
  double f = IEEEDPMul( e, d );

  LONG centibels = IEEEDPFix( f );

  if ( centibels > 1000 ) {

    centibels = 1000;
  }
  return centibels;
}

LONG GetMillisFromTimecents( WORD timecents ) {

  DOUBLE seconds = GetSecondsFromTimecents( timecents );

  // seconds => milliseconds
  DOUBLE h = IEEEDPFlt( 1000 );
  DOUBLE i = IEEEDPMul( seconds, h );

  LONG millis = IEEEDPFix( i );

  return millis;
}

LONG GetAmiGusSampleRate( LONG sourceNote, LONG sourceRate, LONG targetNote ) {

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

UWORD GetAmiGusPeriodsFromTimecents( LONG timecents ) {

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

UWORD GetAmiGusFactorShift16FromCentibels( WORD centibels ) {

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
