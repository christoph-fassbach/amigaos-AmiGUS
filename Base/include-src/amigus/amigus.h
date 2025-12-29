/*
 * This file is part of the amigus.library.
 *
 * amigus.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * amigus.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with amigus.library.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AMIGUS_H
#define AMIGUS_H

#include <exec/types.h>

enum AmiGUS_TypeIds {

  AmiGUS_Zorro2 = 0,
  AmiGUS_mini
};

enum AmiGUS_Errors {

  AmiGUS_NoError = 0,
  AmiGUS_NotFound = 404,
  AmiGUS_InUse,
  AmiGUS_NotYours
};

#define AMIGUS_FLAG_PCM         0x0001
#define AMIGUS_FLAG_CODEC       0x0002
#define AMIGUS_FLAG_WAVETABLE   0x0004

struct AmiGUS {

  APTR      agus_PcmBase;
  APTR      agus_CodecBase;
  APTR      agus_WavetableBase;

  UBYTE     agus_FpgaId[ 8 ];
  
  ULONG     agus_HardwareRev;
  ULONG     agus_FirmwareRev;

  STRPTR    agus_TypeName;
  UWORD     agus_TypeId;

  UWORD     agus_Year;
  UBYTE     agus_Month;
  UBYTE     agus_Day;
  UBYTE     agus_Hour;
  UBYTE     agus_Minute;
};

typedef VOID (*AmiGUS_Interrupt)( APTR data );

#endif /* AMIGUS_H */
