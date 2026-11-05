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
#include "debug.h"
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

  // Increment next channel ...
  ++base->agb_NextAmiGusChannelUsageId;
  // ... and handle next channel ID overflow back to ... well, close to 0
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

  // Anyway, produce debug output and return result
  LOG_D(( "D: Allocated %ld for channel %ld note %ld velocity %ld\n",
          selected, channel, note, velocity ));

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

    if ( channelState->acs_IsActive ) {
      if ( channelState->acs_UsageId < leastRecentlyUsedId ) {
        if ( channelState->acs_MidiChannel == channel ) {
          if ( channelState->acs_MidiNote == note ) {
            if (( 0 == velocity )
              || ( channelState->acs_MidiVelocity == velocity )) {

              leastRecentlyUsedId = channelState->acs_UsageId;
              selected = i;
              LOG_D(( "V: AmiGUS channel %ld selected\n", i ));

            } else {

              LOG_D(( "V: AmiGUS channel %ld: Midi velocity %ld != %ld\n",
                      i, channelState->acs_MidiVelocity, velocity ));
            }
          } else {

            LOG_D(( "V: AmiGUS channel %ld: Midi note %ld != %ld\n",
                    i, channelState->acs_MidiNote, note ));
          }
        } else {

          LOG_D(( "V: AmiGUS channel %ld: Midi channel %ld != %ld\n",
                  i, channelState->acs_MidiChannel, channel ));
        }
      } else {

        LOG_D(( "V: AmiGUS channel %ld: LRU ID %ld >= %ld\n",
                i, channelState->acs_UsageId, leastRecentlyUsedId ));
      }
    } else {

      LOG_D(( "V: AmiGUS channel %ld: IsActive %ld\n",
              i, channelState->acs_IsActive ));
    }
  }

  return selected;
}
