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

#ifndef TEST_WINDOW_H
#define TEST_WINDOW_H

#include "camd_prefs.h"

LONG CreateTestUi( struct AmiGUS_CAMD_Tool * base );
VOID CleanupTestUi( struct AmiGUS_CAMD_Tool * base );
BOOL HandleTestUiEvents( struct AmiGUS_CAMD_Tool * base );

#endif /* TEST_WINDOW_H */