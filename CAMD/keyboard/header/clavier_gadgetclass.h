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
#include <gadgets/scroller.h>

#define CLAVIERGCLASS           "claviergclass"

#define GMR_GADGETNOTHIT        0

#define CG_OFFSET_X             ( TAG_USER + 0x10000001 )
#define CG_VIRTUAL_WIDTH        ( TAG_USER + 0x10000002 )
#define CG_VISUAL_WIDTH         ( TAG_USER + 0x10000003 )

struct Clavier_Gadget_Data {
  BYTE cgd_Flags;
  BYTE cgd_NoteHit;
  WORD cgd_OffsetX;
  WORD cgd_VirtualWidth;
  WORD cgd_VisualWidth;
};

/*
 * defines are limited to 32 chars due to a SAS/C insufficiency !!!
 *
 * So define below is just kind of a ruler...
 */
#define SASC_MAXIMUM_DEFINE_LENGTH_IS_32 12345678
#define CG_FLAG_REDRAW_BACKGROUND        0x01

Class * InitClavierGadgetClass( VOID );

extern struct GfxBase       * GfxBase;
extern struct IntuitionBase * IntuitionBase;
extern struct UtilityBase   * UtilityBase;

#endif /* CLAVIER_GADGETCLASS_H */