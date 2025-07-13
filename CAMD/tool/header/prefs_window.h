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

#ifndef PREFS_WINDOW_H
#define PREFS_WINDOW_H

#include <exec/types.h>

#include "camd_prefs.h"

LONG CreatePrefsUi( struct AmiGUS_CAMD_Tool * base );
VOID CleanupPrefsUi( struct AmiGUS_CAMD_Tool * base );
VOID HandlePrefsUiEvents( struct AmiGUS_CAMD_Tool * base );

#endif /* PREFS_WINDOW_H */
