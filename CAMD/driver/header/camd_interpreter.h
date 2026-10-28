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

#ifndef CAMD_INTERPRETER_H
#define CAMD_INTERPRETER_H

#include <exec/types.h>

/**
 * Parses a stream of MIDI bytes into messages.
 *
 * Needs to be called per byte and sets 
 * the midiMessage to a valid value when completed.
 *
 * @param[in] midiByte Next MIDI byte received.
 * @param[out] midiMessage Completed MIDI message to be handled.
 *
 * @return TRUE if the MIDI message is complete,
 *         FALSE otherwise.
 */
BOOL ParseMidiByte( UBYTE midiByte );

#endif /* CAMD_INTERPRETER_H */
