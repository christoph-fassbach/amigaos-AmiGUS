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

#ifndef AMIGUS_PORTS_H
#define AMIGUS_PORTS_H

#include <exec/ports.h>

#define CHAR_TO_ULONG( a, b, c, d ) ((( a ) << 24 ) | (( b ) << 16 ) | (( c ) << 8 ) | ( d ))

/*
 * defines are limited to 32 chars due to a SAS/C insufficiency !!!
 *
 * So define below is just kind of a ruler...
 */
#define SASC_MAXIMUM_DEFINE_LENGTH_IS_32 12345678

#define PLAY_STANDALONE_MESSAGE_NAME     (( LONG ) CHAR_TO_ULONG( 'P', 'S', 'M', 0 ))
#define PLAY_AMISF_SAMPLE_MESSAGE_NAME   (( LONG ) CHAR_TO_ULONG( 'P', 'A', 'M', 0 ))
#define STOP_SAMPLE_MESSAGE_NAME         (( LONG ) CHAR_TO_ULONG( 'S', 'S', 'M', 0 ))
#define LOAD_SOUNDFONT_NESSAGE_NAME      (( LONG ) CHAR_TO_ULONG( 'L', 'S', 'M', 0 ))
#define RELOAD_SETTINGS_MESSAGE_NAME     (( LONG ) CHAR_TO_ULONG( 'R', 'S', 'M', 0 ))

struct PlayStandaloneSampleMessage {
  struct Message pssm_Message;
  UWORD pssm_Attack;
  UWORD pssm_Decay;
  UWORD pssm_Sustain;
  UWORD pssm_Release;
  UWORD pssm_PlaybackFlags;
  UWORD pssm_Padding0;
  ULONG pssm_PlaybackRate;
  ULONG pssm_LoopOffset;
  ULONG pssm_EndOffset;
  APTR pssm_SampleData;
};

struct PlayAmiSfSampleMessage {
  struct Message passm_Message;
  struct AmiSF * passm_AmiSf;
  UBYTE passm_Bank;
  UBYTE passm_Preset;
  UBYTE passm_Note;
  UBYTE passm_Padding;
  APTR passm_SampleData;
};

struct StopSampleMessage {
  struct Message ssm_Message;
  APTR ssm_PlayMessage;
};

struct LoadSoundFontMessage {
  struct Message lsfm_Message;
};

struct ReloadSettingsMessage {
  struct Message rsm_Message;
};

#endif /* AMIGUS_PORTS_H */
