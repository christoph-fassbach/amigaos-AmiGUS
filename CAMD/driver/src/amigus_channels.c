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
#include "support.h"

UBYTE AllocateNextAmiGusChannel( UBYTE channel, UBYTE note, UBYTE velocity ) {

  struct AmiGUS_CAMD * base = AmiGUS_CAMD_Base;
  struct AmiGusChannelState * channelState;
  UBYTE leastRecentlyUsedIndex = 0xFF;
  ULONG leastRecentlyUsedId = 0xFFffFFff;
  UBYTE i;
  UBYTE selected;

  for ( i = 0; 32 > i; ++i ) {

    channelState = &( base->agb_AmiGusChannelState[ i ]);

    if ( channelState->acs_UsageId < leastRecentlyUsedId ) {

      leastRecentlyUsedId = channelState->acs_UsageId;
      leastRecentlyUsedIndex = i;
    }
    if ( !( channelState->acs_IsActive )) {

      selected = i;
      break;
    }
  }
  if ( 32 == i ) {

    channelState = &( base->agb_AmiGusChannelState[ leastRecentlyUsedIndex ]);
    selected = leastRecentlyUsedIndex;
  }

  channelState->acs_IsActive = TRUE;
  channelState->acs_UsageId = base->agb_NextAmiGusChannelUsageId;
  channelState->acs_MidiChannel = channel;
  channelState->acs_MidiNote = note;
  channelState->acs_MidiVelocity = velocity;

  ++base->agb_NextAmiGusChannelUsageId;
  if ( 0xFFffFFff == base->agb_NextAmiGusChannelUsageId ) {

    ULONG min = 0xFFffFFff;
    for ( i = 0; 32 > i; ++i ) {

      min = MIN( min, base->agb_AmiGusChannelState[ i ].acs_UsageId );
    }      
    for ( i = 0; 32 > i; ++i ) {

      base->agb_AmiGusChannelState[ i ].acs_UsageId -= min;
    }
    base->agb_NextAmiGusChannelUsageId -= min;
  }

  return selected;
}

UBYTE FreeAmiGusChannel( UBYTE channel, UBYTE note, UBYTE velocity ) {

  struct AmiGUS_CAMD * base = AmiGUS_CAMD_Base;
  struct AmiGusChannelState * channelState;
  UBYTE leastRecentlyUsedIndex =
    GetUsedAmiGusChannel( channel, note, velocity );

  if ( 0xFF > leastRecentlyUsedIndex ) {

    channelState = &( base->agb_AmiGusChannelState[ leastRecentlyUsedIndex ]);
    channelState->acs_IsActive = FALSE;
  }

  return leastRecentlyUsedIndex; 
}

UBYTE GetUsedAmiGusChannel( UBYTE channel, UBYTE note, UBYTE velocity ) {

  struct AmiGUS_CAMD * base = AmiGUS_CAMD_Base;
  struct AmiGusChannelState * channelState;
  ULONG leastRecentlyUsedId = 0xFFffFFff;
  UBYTE i;
  UBYTE selected = 0xFF;

  for ( i = 0; 32 > i; ++i ) {

    channelState = &( base->agb_AmiGusChannelState[ i ]);

    if (( channelState->acs_IsActive )
      && ( channelState->acs_UsageId < leastRecentlyUsedId )
      && ( channelState->acs_MidiChannel == channel )
      && ( channelState->acs_MidiNote == note )
      && ( channelState->acs_MidiVelocity == velocity )) {

      leastRecentlyUsedId = channelState->acs_UsageId;
      selected = i;
    }
  }

  return selected;
}
