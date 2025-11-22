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

#ifndef CLAVIER_GADGETCLASS_H
#define CLAVIER_GADGETCLASS_H

#include <intuition/intuitionbase.h>
#include <intuition/gadgetclass.h>

#define CLAVIERGCLASS           "claviergclass"

#define GMR_GADGETNOTHIT        0

struct Clavier_Gadget_Data {
    WORD cgd_NoteHit;
    WORD cgd_NoteActive;
};

Class * InitClavierGadgetClass( VOID );

extern struct GfxBase       * GfxBase;
extern struct IntuitionBase * IntuitionBase;

#endif /* CLAVIER_GADGETCLASS_H */