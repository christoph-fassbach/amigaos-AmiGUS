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

#ifndef SF2_READER_H
#define SF2_READER_H

#include "progress_dialog.h"
#include "sf2.h"

// 5% of 100%, then update to ...
struct SF2 * AllocSf2FromFile( STRPTR filePath );

// (# Presets + # Instruments + #Samples) >> 2 progress
// BOOL True = abort
BOOL PrepareIndex( struct SF2 * sf2 );

// # Presets progress
// BOOL True = abort
BOOL FlattenPresetHierarchy( struct SF2 * sf2,
                             struct ProgressDialog * dialog,
                             ULONG * currentProgress,
                             ULONG maxProgress );

// # Instruments progress
// BOOL True = abort
BOOL FlattenInstrumentHierarchy( struct SF2 * sf2,
                                 struct ProgressDialog * dialog,
                                 ULONG * currentProgress,
                                 ULONG maxProgress );

VOID FreeSf2( struct SF2 * sf2 );

#endif /* SF2_READER_H */