/*
 * This file is part of the AmiGUS CAMD MIDI driver.
 *
 * AmiGUS CAMD MIDI driver is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS CAMD MIDI driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with AmiGUS CAMD MIDI driver.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <classes/window.h>
#include <clib/alib_protos.h>
#include <gadgets/button.h>
#include <gadgets/layout.h>
#include <intuition/classes.h>
#include <intuition/intuition.h>
#include <intuition/icclass.h>
#include <reaction/reaction_macros.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/layout.h>
#include <proto/utility.h>
#include <proto/window.h>

#include "errors.h"
#include "support.h"
#include "test_window.h"

LONG CreateTestUi( struct AmiGUS_CAMD_Tool * base ) {

  Object * objects =
    WindowObject,
      WA_ScreenTitle, "asdf",
      WA_Title, "test",
      WA_Activate, TRUE,
      WA_DepthGadget, TRUE,
      WA_DragBar, TRUE,
      WA_CloseGadget, TRUE,
      WA_SizeGadget, TRUE,
      WA_Width, 600,
      WA_Height, 200,

      WINDOW_IconifyGadget, FALSE,
      WINDOW_ParentGroup, VLayoutObject,
        LAYOUT_AddChild, ButtonObject,
          GA_ID, 1000,
          GA_Text, "Button",
          GA_RelVerify, TRUE,
        ButtonEnd,
      EndGroup,
    EndWindow;

  if ( base->agt_Flags & CAMD_TOOL_FLAG_REACTION_FAILED ) {

    DisplayError( EOpenReactionBases );
    return EOpenReactionBases;
  }
  
  base->agt_TestWindow = ( struct Window * )RA_OpenWindow( objects );

  return ENoError;
}

VOID CleanupTestUi( struct AmiGUS_CAMD_Tool * base ) {

}

VOID HandleTestUiEvents( struct AmiGUS_CAMD_Tool * base ) {

}