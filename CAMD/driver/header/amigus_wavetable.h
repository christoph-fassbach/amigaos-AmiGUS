/*
 * This file is part of the AmiGUS CAMD MIDI driver.
 *
 * AmiGUS CAMD MIDI driver is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS CAMD MIDI driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with AmiGUS CAMD MIDI driver.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AMIGUS_WAVETABLE_H
#define AMIGUS_WAVETABLE_H

#include <exec/types.h>

#include "amisf.h"

/******************************************************************************
 * Wavetable convenience functions.
 *****************************************************************************/

VOID InitAmiGus( VOID );

// Size in byte!
// data needs to be long aligned unfortunately...
VOID LoadAmiGusWavetableSample( ULONG * source, ULONG target, ULONG size );

// Only plays from inside Wavetable memory!
VOID StartAmiGusWavetablePlayback( struct AmiSF_Note * note );

#endif /* AMIGUS_WAVETABLE_H */
