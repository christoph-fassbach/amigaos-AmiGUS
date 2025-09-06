/*
 * This file is part of the CAMD keyboard.
 *
 * CAMD keyboard is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3 of the License only.
 *
 * CAMD keyboard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CAMD keyboard.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KEYBOARD_WINDOW_H
#define KEYBOARD_WINDOW_H

#include "camd_keyboard.h"

LONG CreateKeyboardWindow( struct CAMD_Keyboard * base );
VOID CleanupKeyboardWindow( struct CAMD_Keyboard * base );
BOOL HandleKeyboardWindowEvents( struct CAMD_Keyboard * base );

#endif /* KEYBOARD_WINDOW_H */