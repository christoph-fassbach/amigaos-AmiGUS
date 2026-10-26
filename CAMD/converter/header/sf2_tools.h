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

#ifndef SF2_TOOLS_H
#define SF2_TOOLS_H

#include "amisf_conversion.h"

#include "sf2.h"

LONG GetSf2SampleSize( struct SF2_Sample * sample );
APTR GetSf2SampleData( struct SF2 * sf2,
                       struct SF2_Sample * sample,
                       LONG memorySize );

/**
 * Retrieves the effective ADSR values from SF2 preset and insturment arguments.
 *
 * @param[in] presetArgs Preset arguments to evaluate.
 * @param[in] instrumentArgs Instrument arguments to evaluate.
 * @param[out] attack Attack value of ADSR envelope in SF2 timecents.
 * @param[out] decay Decay value of ADSR envelope in SF2 timecents.
 * @param[out] sustain Sustain level relative to original volume
 *                     in centibels aka 1/10th decibels.
 * @param[out] release Release value of ADSR envelope in SF2 timecents.
 */
VOID GetSf2ADSRs( struct SF2_Args * presetArgs,
                  struct SF2_Args * instrumentArgs,
                  WORD * attack,
                  WORD * decay,
                  WORD * sustain,
                  WORD * release );

#endif /* SF2_TOOLS_H */
