/*
 * This file is part of the mhiAmiGUS.library.
 *
 * mhiAmiGUS.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiAmiGUS.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiAmiGUS.library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/timer.h>
#include <proto/utility.h>

#include "amigus_mhi.h"
#include "debug.h"
#include "support.h"

// Only allowed for math routines that way!
struct UtilityBase * UtilityBase = 0;

VOID LogTicks( UBYTE bitmask ) {
  struct EClockVal ecv;
  ULONG ef = ReadEClock( &ecv );

  switch (bitmask) {
    case 0x00:
      break;
    case 0x01:
      LOG_I(("I: Tick frequency %ld\n", ef));
      break;
    case 0x02:
      LOG_I(("I: Tick low %lu\n", ecv.ev_lo));
      break;
    case 0x04:
      LOG_I(("I: Tick high %lu\n", ecv.ev_hi));
      break;
    case 0x03:
      LOG_I(("I: Tick freq %ld low %ld\n", ef, ecv.ev_lo));
      break;
    case 0x07:
      LOG_I(("I: Tick freq %ld low %ld high %ld\n", ef, ecv.ev_lo, ecv.ev_hi));
      break;
  }
}

VOID ShowError( STRPTR title, STRPTR message, STRPTR button ) {

  struct EasyStruct req;

  req.es_StructSize = sizeof( struct EasyStruct );
  req.es_Flags = 0;
  req.es_Title = title;
  req.es_TextFormat = "Error %ld : %s";
  req.es_GadgetFormat = button;

  EasyRequest( NULL, &req, NULL, 0, message );
}

VOID ShowAlert( ULONG alertNum ) {

  Alert( alertNum );
}

VOID Sleep( ULONG seconds, ULONG micros ) {

  struct EClockVal ecv;
  ULONG sec_ticks = ReadEClock( &ecv );
  ULONG micro_ticks = UDivMod32( sec_ticks, 1000000 );
  ULONG needed_ticks = UMult32( micros, micro_ticks )
                       + UMult32( seconds, sec_ticks );
  /*
   * Real seconds would be:
   * ULONG milli_ticks = UDivMod32( sec_ticks, 1000 );
   * ULONG needed_ticks = UMult32( milli_ticks, millis );
   *
   * Higher precision, but 32bit not wide enough:
   * ULONG second_ticks = UMult32( sec_ticks, millis );
   * ULONG needed_ticks = UDivMod32( second_ticks, 1000 );
  */
  const ULONG current_lo = ecv.ev_lo;
  const ULONG current_hi = ecv.ev_hi;
  const ULONG target_lo = current_lo + needed_ticks;
  const ULONG target_hi = ( current_lo > target_lo )
                          ? ( current_hi + 1 )
                          : ( current_hi );

  LOG_V(( "V: Tick freq %ld current: %ld %ld needed: %ld target: %ld %ld\n",
          sec_ticks,
          current_hi,
          current_lo,
          needed_ticks,
          target_hi,
          target_lo ));
  while (( target_lo > ecv.ev_lo ) || ( target_hi > ecv.ev_hi )) {

    ReadEClock( &ecv );
  }
  LOG_V(( "V: Slept until %ld %ld\n", ecv.ev_hi, ecv.ev_lo ));
}