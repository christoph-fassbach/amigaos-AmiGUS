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

#ifndef SF_LISTNODES_H
#define SF_LISTNODES_H

#include <exec/lists.h>
#include <gadgets/listbrowser.h>

const struct ColumnInfo * GetSoundFontColumnInfos( VOID );
const ULONG GetSoundFontColumnsWidth( VOID );

VOID CreateEmptyListLabels( struct List * labels );

VOID FreeListLabels( struct List * list );

#endif /* SF_LISTNODES_H */
