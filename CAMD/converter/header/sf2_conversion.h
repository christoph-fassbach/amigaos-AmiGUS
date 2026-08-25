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

#ifndef SF2_CONVERSION_H
#define SF2_CONVERSION_H

#include <exec/lists.h>
#include <exec/types.h>

#ifndef SF2_H
struct SF2;
#endif

struct ConversionRates {

  struct Node cpr_Node;

  ULONG cpr_SampleRate;
  UBYTE cpr_SampleNote;
  UBYTE cpr_Padding0;
  UWORD cpr_Padding1;
};

struct ConversionInfo {

  ULONG ci_NoteCount;
  ULONG ci_SampleCount;
  UWORD ci_SampleMapping[ 65536 ];
  // translates SF2 sample as (index) to (AmiSF sample index + 1)
  // =0 in mapping: empty
  // >1 in mapping: sf2 index mapped to that new index

  UWORD ci_PlaybackRateCount;
  struct List ci_PlaybackRatesNeeded;
};

struct ConversionInfo * AllocConversionInfo( struct SF2 * sf2 );
VOID FreeConversionInfo( struct ConversionInfo * info );


#endif /* SF2_CONVERSION_H */
