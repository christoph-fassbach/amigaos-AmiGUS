/*
 * This file is part of the amigus.library.
 *
 * amigus.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * amigus.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with amigus.library.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AMIGUS_ZORRO2_H
#define AMIGUS_ZORRO2_H

#include <amigus/amigus.h>
#include <exec/types.h>

VOID AmiGusZorro2_AddAll( struct List * cards );
VOID AmiGusZorro2_InstallInterrupt( VOID );
VOID AmiGusZorro2_RemoveInterrupt( VOID );

#endif /* AMIGUS_ZORRO2_H */