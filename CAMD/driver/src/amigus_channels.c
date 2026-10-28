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

#include "amigus_camd.h"
#include "amigus_channels.h"

UBYTE AllocateNextAmiGusChannel( UBYTE midiChannel ) {

  struct AmiGUS_CAMD * base = AmiGUS_CAMD_Base;
  struct MidiChannelState * currentChannelState =
    &( base->agb_MidiChannelState[ midiChannel ]);
  const UBYTE currentChannelIndex = base->agb_NextAmiGusChannel;

  currentChannelState->mcs_AmigusChannelUsed = currentChannelIndex;
  ++base->agb_NextAmiGusChannel;
  base->agb_NextAmiGusChannel &= 0x1F; // Poor man's mod 32 ;-)

  return currentChannelIndex;
}
