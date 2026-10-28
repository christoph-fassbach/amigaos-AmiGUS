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

#ifndef AMIGUS_CHANNELS_H
#define AMIGUS_CHANNELS_H

#include <exec/types.h>

struct MidiChannelState {

  UBYTE mcs_AmigusChannelUsed;
  UBYTE mcs_Padding0;
  UBYTE mcs_Padding1;
  UBYTE mcs_Padding2;
};

UBYTE AllocateNextAmiGusChannel( UBYTE midiChannel );
UBYTE GetUsedAmiGusChannel( UBYTE midiChannel );

#endif /* AMIGUS_CHANNELS_H */