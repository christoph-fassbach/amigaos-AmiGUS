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
#include <libraries/configvars.h>

/******************************************************************************
 * Wavetable convenience functions.
 *****************************************************************************/

/**
 * Finds an available, matching AmiGUS wavetable device and 
 * fills its properties into the provided device.
 *
 * Does not prevent any conflicts due to concurrency,
 * but works fine in Permit()/Forbid().
 *
 * @param[out] device "struct ConfigDev *" provided by expansion.library
 *                    if a suitable AmiGUS wavetable device is available,
 *                    NULL otherwise.
 *
 * @return ENoError if an AmiGUS wavetable device was found,
 *         error value describing the reason otherwise.
 */
LONG FindAmiGusWavetable( struct ConfigDev ** device );

#endif /* AMIGUS_WAVETABLE_H */
