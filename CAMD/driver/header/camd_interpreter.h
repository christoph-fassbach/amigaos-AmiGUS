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

#include "amisf.h"

struct MidiMessage {

  UBYTE mm_Status;
  UBYTE mm_Channel;
  UBYTE mm_Data0;
  UBYTE mm_Data1;
};

/**
 * Tokenizes a stream of MIDI bytes into messages.
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
BOOL TokenizeMidiByte( UBYTE midiByte, struct MidiMessage * midiMessage );

/**
 * Interprets a MIDI message.
 *
 * Tracks the MIDI state in the MidiChannelState array 
 * and feeds the resulting output straight into AmiGUS for playback.
 *
 * @param midiMessage MIDI message to handle.
 *
 * @return TRUE if the MIDI message was interpretable,
 *         FALSE otherwise.
 */
BOOL InterpretMidiMessage( struct MidiMessage * midiMessage );

ULONG GetPlaybackRate( struct AmiSF * amisf,
                       struct AmiSF_Note * sfNote,
                       UBYTE note );

UWORD GetPlaybackVolume( UBYTE velocity );

#endif /* CAMD_INTERPRETER_H */
