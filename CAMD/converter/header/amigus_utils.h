/*
 * This file is part of the SoundFontConverter.
 *
 * SoundFontConverter is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3 of the License only.
 *
 * SoundFontConverter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SoundFontConverter.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AMIGUS_UTILS_H
#define AMIGUS_UTILS_H

#include <amigus/amigus_ports.h>
#include <amigus/amisf.h>

ULONG OpenAmigusPort( VOID );
LONG SendAmigusMessage( struct Message * message );

struct PlaySampleMessage * CreateAmigusPlaySampleMessage(
  struct MsgPort * replyPort,
  struct AmiSF_Note * note,
  APTR sample );

struct PlayNoteMessage * CreateAmigusPlayNoteMessage(
  struct MsgPort * replyPort );

struct PlayInstrumentMessage * CreateAmigusPlayInstrumentMessage(
  struct MsgPort * replyPort );

struct LoadSoundFontMessage * CreateAmigusLoadSoundFontMessage(
  struct MsgPort * replyPort );

struct ReloadSettingsMessage * CreateAmigusReloadSettingsMessage(
  struct MsgPort * replyPort );

VOID DeleteAmigusMessage( APTR message );

#endif /* AMIGUS_UTILS_H */