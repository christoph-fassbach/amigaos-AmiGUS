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

#ifndef CAMD_HELPER_H
#define CAMD_HELPER_H

#include <exec/lists.h>
#include <intuition/classes.h>

struct CAMD_Device_Node {

    struct MinNode cdn_Node;

    STRPTR cdn_Name;
    STRPTR cdn_Location;
};

LONG ExtractCamdOutputDevices( struct List * devices, struct List * labels );
VOID FreeCamdOutputDevices( struct List * devices, struct List * labels );

extern struct Library           * CamdBase;

extern struct ClassLibrary      * ChooserBase;

#endif /* CAMD_HELPER_H */