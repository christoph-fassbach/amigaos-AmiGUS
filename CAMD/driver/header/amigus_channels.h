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
  UBYTE mcs_Bank;
  UBYTE mcs_Preset;
  UBYTE mcs_Padding0;
  UBYTE mcs_Padding1;
};

struct AmiGusChannelState {

  UBYTE acs_IsActive;
  UBYTE acs_MidiChannel;
  UBYTE acs_MidiNote;
  UBYTE acs_MidiVelocity;
  ULONG acs_UsageId;
};

/**
 * @params all midi values, midi channel,
 * @return AmiGUS channel used.
 */
UBYTE AllocateNextAmiGusChannel( UBYTE channel, UBYTE note, UBYTE velocity );
UBYTE FreeAmiGusChannel( UBYTE channel, UBYTE note, UBYTE velocity );
UBYTE GetUsedAmiGusChannel( UBYTE channel, UBYTE note, UBYTE velocity );

#endif /* AMIGUS_CHANNELS_H */