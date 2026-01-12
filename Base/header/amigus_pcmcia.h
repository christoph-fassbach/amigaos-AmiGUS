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

#ifndef AMIGUS_PCMCIA_H
#define AMIGUS_PCMCIA_H

#include <amigus/amigus.h>
#include <exec/types.h>

VOID AmiGusPcmcia_AddAll( struct List * cards );
VOID AmiGusPcmcia_InstallInterrupt( VOID );
VOID AmiGusPcmcia_RemoveInterrupt( VOID );

#endif /* AMIGUS_PCMCIA_H */