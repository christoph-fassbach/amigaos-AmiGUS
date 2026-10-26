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

#ifndef SF_MATH_H
#define SF_MATH_H

#include <exec/types.h>

/**
 * Converts a "periods" value storing AD_R time values in AmiSF format
 * to milliseconds.
 *
 * Used for .AmiSF -> UI.
 *
 * @param timecents Time value for ADSR envelopes in AmiSF format.
 *
 * @return Time value to milliseconds, truncating everything below.
 */
LONG GetMillisFromPeriods( UWORD periods );

/**
 * Converts a 16bit-shifted fration factor understood by AmiGUS
 * for __S_ value as given in AmiSF format to centibels.
 *
 * Used for .AmiSF -> UI.
 *
 * @param factorShift16 AmiGUS sustain value.
 *
 * return Sustain in centibels. A centibel is 1/10th decibel, obviously.
 */
LONG GetCentibelsFromFactorShift16( UWORD factorShift16 );

/**
 * Converts a "timecents" value storing AD_R time values in SF2 format
 * to milliseconds.
 *
 * Used for .SF2 -> UI.
 *
 * @param timecents Time value for ADSR envelopes in SF2 format.
 *
 * @return Time value to milliseconds, truncating everything below.
 */
LONG GetMillisFromTimecents( WORD timecents );

LONG GetAmiGusSampleRate( LONG sourceNote, LONG sourceRate, LONG targetNote );
UWORD GetAmiGusPeriodsFromTimecents( LONG timecents );
UWORD GetAmiGusFactorShift16FromCentibels( WORD centibels );

#endif /* SF_MATH_H */