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

#include <limits.h>

#include <clib/alib_protos.h>
#include <intuition/gadgetclass.h>

#include <proto/graphics.h>
#include <proto/intuition.h>

#include "clavier_gadgetclass.h"

static ULONG Handle_OM_NEW( Class * class,
                            struct Gadget * gadget,
                            struct opSet * message ) {

  gadget = ( struct Gadget * ) DoSuperMethodA( class,
                                               ( Object * ) gadget,
                                               ( Msg ) message );
  if ( gadget ) {

    struct Clavier_Gadget_Data * data = INST_DATA( class, gadget );
    // TODO: init data fields here!
  }

  return ( ULONG ) gadget;
}

static ULONG Handle_GM_HITTEST( Class * class,
                                struct Gadget * gadget,
                                struct gpHitTest *message ) {

  return GMR_GADGETHIT;
  // return GMR_GADGETNOTHIT;
}

static ULONG Handle_GM_RENDER( Class * class,
                               struct Gadget * gadget,
                               struct gpRender * message ) {

  struct RastPort * rastPort = message->gpr_RPort;

  SetAPen( rastPort, 3 );
  RectFill( rastPort,
            gadget->LeftEdge,
            gadget->TopEdge,
            gadget->LeftEdge + 10,
            gadget->TopEdge + 10);
  return 0;
}

static ULONG Handle_GM_GOACTIVE( Class * class,
                                 struct Gadget * gadget,
                                 struct gpInput * message ) {
  return GMR_MEACTIVE;
  // return GMR_REUSE;
}

static ULONG Handle_GM_HANDLEINPUT( Class * class,
                                 struct Gadget * gadget,
                                 struct gpInput * message ) {

	ULONG result = DoSuperMethodA( class,
                                 ( Object * ) gadget,
                                 ( Msg ) message );
  return result;
}

static ULONG Handle_GM_GOINACTIVE( Class * class,
                                 struct Gadget * gadget,
                                 struct gpGoInactive * message ) {

  return 0;
}

static ULONG Handle_GM_Domain( Class * class,
                               struct gpDomain * message ) {

  switch ( message->gpd_Which ) {

    case GDOMAIN_MINIMUM: {

      message->gpd_Domain.Left = 0;
      message->gpd_Domain.Top = 0;
      message->gpd_Domain.Width = 10;
      message->gpd_Domain.Height = 10;
      return 1;
    }
    case GDOMAIN_NOMINAL: {

      message->gpd_Domain.Left = 0;
      message->gpd_Domain.Top = 0;
      message->gpd_Domain.Width = 20;
      message->gpd_Domain.Height = 20;
      return 1;
    }
    case GDOMAIN_MAXIMUM: {

      message->gpd_Domain.Left = 0;
      message->gpd_Domain.Top = 0;
      message->gpd_Domain.Width = SHRT_MAX;
      message->gpd_Domain.Height = SHRT_MAX;
      return 1;
    }
    default: {
      return 0;
    }
  }
}

__saveds ULONG DispatchClavierGadgetClass(
  Class * class,
  struct Gadget * gadget,
  Msg message ) {

  ULONG result = 0;
  switch ( message->MethodID ) {
    case OM_NEW: {

      result = Handle_OM_NEW( class,
                              gadget,
                              ( struct opSet * ) message );
      break;
    }
    case GM_HITTEST: {

      result = Handle_GM_HITTEST( class,
                                  gadget,
                                  ( struct gpHitTest * ) message );
      break;
    }
    case GM_RENDER: {

      result = Handle_GM_RENDER( class,
                                 gadget,
                                 ( struct gpRender * ) message );
      break;
    }
    case GM_GOACTIVE: {

      result = Handle_GM_GOACTIVE( class,
                                   gadget,
                                   ( struct gpInput * ) message );
      break;
    }
    case GM_HANDLEINPUT: {

      result = Handle_GM_HANDLEINPUT( class,
                                      gadget,
                                      ( struct gpInput * ) message );
      break;
    }
    case GM_GOINACTIVE: {

      result = Handle_GM_GOINACTIVE( class,
                                     gadget,
                                     ( struct gpGoInactive * ) message );
      break;
    }
    case GM_DOMAIN: {

      result = Handle_GM_Domain( class,
                                 ( struct gpDomain * ) message );
      break;
    }
    default: {

      result = DoSuperMethodA( class, ( Object * ) gadget, message );
      break;
    }
  }
  return result;
}

Class * InitClavierGadgetClass( VOID ) {

  Class * class = MakeClass(
    CLAVIERGCLASS,
    BUTTONGCLASS,
    NULL, 
    sizeof( struct Clavier_Gadget_Data ),
    0 );
  if ( class ) {

    struct Hook * hook = &( class->cl_Dispatcher );
    hook->h_Entry = ( HOOKFUNC ) HookEntry;
	  hook->h_SubEntry = ( HOOKFUNC ) DispatchClavierGadgetClass;
  }

  return class;
}