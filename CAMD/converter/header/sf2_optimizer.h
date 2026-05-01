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

#ifndef SF2_OPTIMIZER_H
#define SF2_OPTIMIZER_H

#include <exec/types.h>

#include "progress_dialog.h"
#include "sf2.h"

BOOL OptimizeSF2( struct SF2 * sf2,
                  struct ProgressDialog * dialog,
                  ULONG * currentProgress,
                  ULONG * maxProgress );

#endif /* SF2_OPTIMIZER_H */
