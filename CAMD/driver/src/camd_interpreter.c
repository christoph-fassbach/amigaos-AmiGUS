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

#include "camd_interpreter.h"
#include "debug.h"

// MIDI status messages types
#define MIDI_NOTE_OFF         0x80
#define MIDI_NOTE_ON          0x90
#define MIDI_POLY_PRESSURE    0xA0
#define MIDI_CONTROL_CHANGE   0xB0
#define MIDI_PROGRAM_CHANGE   0xC0
#define MIDI_CHANNEL_PRESSURE 0xD0
#define MIDI_PITCH_BEND       0xE0
#define MIDI_SYSTEM_MSG       0xF0

typedef enum {

  STATE_WAIT_STATUS,
  STATE_WAIT_DATA1,
  STATE_WAIT_DATA2_2,
  STATE_WAIT_DATA2_1

} MidiState;

BOOL ParseMidiByte( UBYTE midiByte ) {

  static MidiState state;

  static UBYTE status;
  static UBYTE channel;
  static UBYTE date0;
  static UBYTE date1;

  // Handle realtime messages w/o changing state machine
  if ( 0xF8 <= midiByte ) {

    // TODO: Handle them ;)
    return FALSE;
  }

  // Handle status messages
  if ( 0x80 & midiByte ) {

    // TODO: Handle System Common / SysEx messages (0xF0 - 0xF7)
    if (( midiByte & 0xF0) == MIDI_SYSTEM_MSG ) {

      state = STATE_WAIT_STATUS;
      return FALSE;
    }

    status = midiByte & 0xF0;
    channel = midiByte & 0x0F;

    switch ( status ) {
      case MIDI_PROGRAM_CHANGE:
      case MIDI_CHANNEL_PRESSURE: {
        state = STATE_WAIT_DATA1;
        break;
      }
      default: {
        state = STATE_WAIT_DATA2_2;
        break;
      }
    }
    return FALSE;
  }

  // Handle data bytes
  switch ( state ) {
    case STATE_WAIT_DATA1: {
      date0 = midiByte;
      date1 = 0;
      state = STATE_WAIT_STATUS;
      //TODO: Handle the message!
      LOG_D(( "D: MIDI message completed, %02lx %02lx %02lx %02lx\n",
              status, channel, date0, date1 ));
      return TRUE;
    }
    case STATE_WAIT_DATA2_2: {
      date1 = midiByte;
      state = STATE_WAIT_DATA2_1;
      return FALSE;
    }
    case STATE_WAIT_DATA2_1: {
      date0 = midiByte;
      state = STATE_WAIT_STATUS;
      //TODO: Handle the message!
      LOG_D(( "D: MIDI message completed, %02lx %02lx %02lx %02lx\n",
              status, channel, date0, date1 ));
      return TRUE;
    }
    case STATE_WAIT_STATUS:
    default: {
      break;
    }
  }
  return FALSE;
}
