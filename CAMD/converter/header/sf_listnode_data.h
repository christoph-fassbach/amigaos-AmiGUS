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

#ifndef SF_LISTNODE_DATA_H
#define SF_LISTNODE_DATA_H

#include <gadgets/listbrowser.h>

CONST_STRPTR GetMidiName( LONG bank,
                          LONG preset,
                          LONG instrumentMin,
                          LONG instrumentMax,
                          LONG sampleMin,
                          LONG sampleMax );
const LONG * GetListNumber( LONG number );

const struct ColumnInfo * GetAmiSfColumnInfos( VOID );
const LONG GetAmiSfColumnCount( VOID );
const ULONG GetAmiSfColumnsWidth( VOID );

const struct ColumnInfo * GetSf2ColumnInfos( VOID );
const LONG GetSf2ColumnCount( VOID );
const ULONG GetSf2ColumnsWidth( VOID );

#endif
