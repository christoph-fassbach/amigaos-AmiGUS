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

#ifndef AMIGUS_GAYLE_H
#define AMIGUS_GAYLE_H

#include <amigus/amigus.h>
#include <exec/lists.h>

/**
 * Discovers all AmiGUS cards connected to a Gayle PCMCIA port and not
 * reachable via card.resource, but likely this will be 
 * - on kick below 2.0,
 * - maximum 1 and an AmiGUS mini. ;)
 *
 * @param cards List of cards to add the discovered AmiGUS cards to.
 */
VOID AmiGusGayle_AddAll( struct List * cards );

/**
 * Removes and frees all AmiGUS cards connected to Zorro2 slots.
 *
 * @param cards List of cards to free.
 */
VOID AmiGusGayle_RemoveAll( struct List * cards );

/**
 * Installs the interrupt handling and further dispatching by amigus.library
 * for Gayle PCMCIA without card.resource.
 *
 * @return AmiGUS_NoError if successful,
 *         error code from enum AmiGUS_Errors otherwise.
 */
LONG AmiGusGayle_InstallInterrupt( VOID );

/**
 * Removes the interrupt handling and further dispatching by amigus.library
 * for Gayle PCMCIA without card.resource.
 *
 * @return AmiGUS_NoError if successful,
 *         error code from enum AmiGUS_Errors otherwise.
 */
LONG AmiGusGayle_RemoveInterrupt( VOID );

#endif /* AMIGUS_GAYLE_H */