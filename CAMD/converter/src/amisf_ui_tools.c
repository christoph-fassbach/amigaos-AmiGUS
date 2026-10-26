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
