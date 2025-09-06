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

#include <classes/window.h>
#include <clib/alib_protos.h>
#include <dos/dos.h>
#include <gadgets/button.h>
#include <gadgets/layout.h>
#include <gadgets/virtual.h>
#include <intuition/classes.h>
#include <intuition/intuition.h>
#include <intuition/icclass.h>
#include <reaction/reaction_macros.h>

#include <proto/bevel.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/layout.h>
#include <proto/utility.h>
#include <proto/virtual.h>
#include <proto/window.h>

#include "debug.h"
#include "errors.h"
#include "keyboard_window.h"
#include "support.h"

LONG CreateKeyboardWindow( struct CAMD_Keyboard * base ) {

  if ( base->ck_Flags & CAMD_TOOL_FLAG_REACTION_FAILED ) {

    DisplayError( EOpenReactionBases );
    return EOpenReactionBases;
  }
  
  base->ck_TestWindowPort = CreateMsgPort();
  if ( !( base->ck_TestWindowPort )) {

    return ECreateMessagePort;
  }
#define SEPERATOR_PEN 0 // -1 background, 0 grey, 1 black, 2 white, 3 blue
  base->ck_TestWindowContent = 
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

        WA_SmartRefresh, TRUE,
//        WA_IDCMP, IDCMP_GADGETDOWN | IDCMP_GADGETUP | IDCMP_IDCMPUPDATE,
//        WINDOW_IDCMPHook, &( base->ck_TestWindowHook ),
//        WINDOW_IDCMPHookBits, IDCMP_IDCMPUPDATE,

        WINDOW_IconifyGadget, FALSE,
        WINDOW_AppPort, base->ck_TestWindowPort,
        WINDOW_Position, WPOS_TOPLEFT,
  
        WINDOW_ParentGroup, VirtualObject,
          GA_RelVerify, TRUE,
          GA_TabCycle, TRUE,
          VIRTUALA_Contents, HLayoutObject,
            LAYOUT_FillPen, SEPERATOR_PEN,
            LAYOUT_InnerSpacing, 0,
            LAYOUT_AddChild, VLayoutObject,
              LAYOUT_FillPen, SEPERATOR_PEN,
              LAYOUT_InnerSpacing, 0,
              LAYOUT_AddChild, HLayoutObject,
                LAYOUT_FillPen, SEPERATOR_PEN,
                LAYOUT_InnerSpacing, 0,
                LAYOUT_AddChild, ButtonObject,
                  GA_ID, 10060,
                  GA_Text, "C4",
                  GA_RelVerify, TRUE,
                  BUTTON_BackgroundPen, 2,
                  BUTTON_TextPen, 1,
                  BUTTON_BevelStyle, BVS_NONE,
                ButtonEnd,
                CHILD_WeightedWidth, 8,
                LAYOUT_AddChild, BevelObject,
                  BEVEL_Style, BVS_SBAR_VERT,
                BevelEnd,
                CHILD_MaxWidth, 2,
                LAYOUT_AddChild, ButtonObject,
                  GA_ID, 10061,
                  GA_Text, "cis",
                  GA_RelVerify, TRUE,
                  BUTTON_BackgroundPen, 1,
                  BUTTON_TextPen, 2,
                ButtonEnd,
                CHILD_WeightedWidth, 10,
                LAYOUT_AddChild, BevelObject,
                  BEVEL_Style, BVS_SBAR_VERT,
                BevelEnd,
                CHILD_MaxWidth, 2,
                LAYOUT_AddChild, ButtonObject,
                  GA_ID, 10062,
                  GA_Text, "D4",
                  GA_RelVerify, TRUE,
                  BUTTON_BackgroundPen, 2,
                  BUTTON_TextPen, 1,
                  BUTTON_BevelStyle, BVS_NONE,
                ButtonEnd,
                CHILD_WeightedWidth, 8,
                LAYOUT_AddChild, BevelObject,
                  BEVEL_Style, BVS_SBAR_VERT,
                BevelEnd,
                CHILD_MaxWidth, 2,
                LAYOUT_AddChild, ButtonObject,
                  GA_ID, 10063,
                  GA_Text, "dis",
                  GA_RelVerify, TRUE,
                  BUTTON_BackgroundPen, 1,
                  BUTTON_TextPen, 2,
                ButtonEnd,
                CHILD_WeightedWidth, 10,
                LAYOUT_AddChild, BevelObject,
                  BEVEL_Style, BVS_SBAR_VERT,
                BevelEnd,
                CHILD_MaxWidth, 2,
                LAYOUT_AddChild, ButtonObject,
                  GA_ID, 10064,
                  GA_Text, "E4",
                  GA_RelVerify, TRUE,
                  BUTTON_BackgroundPen, 2,
                  BUTTON_TextPen, 1,
                  BUTTON_BevelStyle, BVS_NONE,
                ButtonEnd,
                CHILD_WeightedWidth, 8,
                LAYOUT_AddChild, BevelObject,
                  BEVEL_Style, BVS_SBAR_VERT,
                BevelEnd,
                CHILD_MaxWidth, 2,
              EndHGroup,
              CHILD_WeightedHeight, 3,
              LAYOUT_AddChild, HLayoutObject,
                LAYOUT_FillPen, SEPERATOR_PEN,
                LAYOUT_InnerSpacing, 0,
                LAYOUT_AddChild, ButtonObject,
                  GA_ID, 10060,
                  GA_Text, "C4",
                  GA_RelVerify, TRUE,
                  BUTTON_BackgroundPen, 2,
                  BUTTON_TextPen, 1,
                  BUTTON_BevelStyle, BVS_NONE,
                ButtonEnd,
                CHILD_WeightedWidth, 10,
                LAYOUT_AddChild, BevelObject,
                  BEVEL_Style, BVS_SBAR_VERT,
                BevelEnd,
                CHILD_MaxWidth, 2,
                LAYOUT_AddChild, ButtonObject,
                  GA_ID, 10062,
                  GA_Text, "D4",
                  GA_RelVerify, TRUE,
                  BUTTON_BackgroundPen, 2,
                  BUTTON_TextPen, 1,
                  BUTTON_BevelStyle, BVS_NONE,
                ButtonEnd,
                CHILD_WeightedWidth, 10,
                LAYOUT_AddChild, BevelObject,
                  BEVEL_Style, BVS_SBAR_VERT,
                BevelEnd,
                CHILD_MaxWidth, 2,
                LAYOUT_AddChild, ButtonObject,
                  GA_ID, 10064,
                  GA_Text, "E4",
                  GA_RelVerify, TRUE,
                  BUTTON_BackgroundPen, 2,
                  BUTTON_TextPen, 1,
                  BUTTON_BevelStyle, BVS_NONE,
                ButtonEnd,
                CHILD_WeightedWidth, 10,
                LAYOUT_AddChild, BevelObject,
                  BEVEL_Style, BVS_SBAR_VERT,
                BevelEnd,
                CHILD_MaxWidth, 2,
              EndHGroup,
              CHILD_WeightedHeight, 2,
            EndVGroup,
            CHILD_WeightedWidth, 30,

            LAYOUT_AddChild, VLayoutObject,
              LAYOUT_FillPen, SEPERATOR_PEN,
              LAYOUT_InnerSpacing, 0,
              LAYOUT_AddChild, HLayoutObject,
                LAYOUT_FillPen, SEPERATOR_PEN,
                LAYOUT_InnerSpacing, 0,
                LAYOUT_AddChild, ButtonObject,
                  GA_ID, 10065,
                  GA_Text, "F4",
                  GA_RelVerify, TRUE,
                  BUTTON_BackgroundPen, 2,
                  BUTTON_TextPen, 1,
                  BUTTON_BevelStyle, BVS_NONE,
                ButtonEnd,
                CHILD_WeightedWidth, 8,
                LAYOUT_AddChild, BevelObject,
                  BEVEL_Style, BVS_SBAR_VERT,
                BevelEnd,
                CHILD_MaxWidth, 2,
                LAYOUT_AddChild, ButtonObject,
                  GA_ID, 10066,
                  GA_Text, "fis",
                  GA_RelVerify, TRUE,
                  BUTTON_BackgroundPen, 1,
                  BUTTON_TextPen, 2,
                ButtonEnd,
                CHILD_WeightedWidth, 10,
                LAYOUT_AddChild, BevelObject,
                  BEVEL_Style, BVS_SBAR_VERT,
                BevelEnd,
                CHILD_MaxWidth, 2,
                LAYOUT_AddChild, ButtonObject,
                  GA_ID, 10067,
                  GA_Text, "G4",
                  GA_RelVerify, TRUE,
                  BUTTON_BackgroundPen, 2,
                  BUTTON_TextPen, 1,
                  BUTTON_BevelStyle, BVS_NONE,
                ButtonEnd,
                CHILD_WeightedWidth, 8,
                LAYOUT_AddChild, BevelObject,
                  BEVEL_Style, BVS_SBAR_VERT,
                BevelEnd,
                CHILD_MaxWidth, 2,
                LAYOUT_AddChild, ButtonObject,
                  GA_ID, 10068,
                  GA_Text, "gis",
                  GA_RelVerify, TRUE,
                  BUTTON_BackgroundPen, 1,
                  BUTTON_TextPen, 2,
                ButtonEnd,
                CHILD_WeightedWidth, 10,
                LAYOUT_AddChild, BevelObject,
                  BEVEL_Style, BVS_SBAR_VERT,
                BevelEnd,
                CHILD_MaxWidth, 2,
                LAYOUT_AddChild, ButtonObject,
                  GA_ID, 10069,
                  GA_Text, "A4",
                  GA_RelVerify, TRUE,
                  BUTTON_BackgroundPen, 2,
                  BUTTON_TextPen, 1,
                  BUTTON_BevelStyle, BVS_NONE,
                ButtonEnd,
                CHILD_WeightedWidth, 8,
                LAYOUT_AddChild, BevelObject,
                  BEVEL_Style, BVS_SBAR_VERT,
                BevelEnd,
                CHILD_MaxWidth, 2,
                LAYOUT_AddChild, ButtonObject,
                  GA_ID, 10070,
                  GA_Text, "ais",
                  GA_RelVerify, TRUE,
                  BUTTON_BackgroundPen, 1,
                  BUTTON_TextPen, 2,
                ButtonEnd,
                CHILD_WeightedWidth, 10,
                LAYOUT_AddChild, BevelObject,
                  BEVEL_Style, BVS_SBAR_VERT,
                BevelEnd,
                CHILD_MaxWidth, 2,
                LAYOUT_AddChild, ButtonObject,
                  GA_ID, 10071,
                  GA_Text, "B4",
                  GA_RelVerify, TRUE,
                  BUTTON_BackgroundPen, 2,
                  BUTTON_TextPen, 1,
                  BUTTON_BevelStyle, BVS_NONE,
                ButtonEnd,
                CHILD_WeightedWidth, 8,
                LAYOUT_AddChild, BevelObject,
                  BEVEL_Style, BVS_SBAR_VERT,
                BevelEnd,
                CHILD_MaxWidth, 2,
              EndHGroup,
              CHILD_WeightedHeight, 3,
              LAYOUT_AddChild, HLayoutObject,
                LAYOUT_FillPen, SEPERATOR_PEN,
                LAYOUT_InnerSpacing, 0,
                LAYOUT_AddChild, ButtonObject,
                  GA_ID, 10065,
                  GA_Text, "F4",
                  GA_RelVerify, TRUE,
                  BUTTON_BackgroundPen, 2,
                  BUTTON_TextPen, 1,
                  BUTTON_BevelStyle, BVS_NONE,
                ButtonEnd,
                CHILD_WeightedWidth, 10,
                LAYOUT_AddChild, BevelObject,
                  BEVEL_Style, BVS_SBAR_VERT,
                BevelEnd,
                CHILD_MaxWidth, 2,
                LAYOUT_AddChild, ButtonObject,
                  GA_ID, 10067,
                  GA_Text, "G4",
                  GA_RelVerify, TRUE,
                  BUTTON_BackgroundPen, 2,
                  BUTTON_TextPen, 1,
                  BUTTON_BevelStyle, BVS_NONE,
                ButtonEnd,
                CHILD_WeightedWidth, 10,
                LAYOUT_AddChild, BevelObject,
                  BEVEL_Style, BVS_SBAR_VERT,
                BevelEnd,
                CHILD_MaxWidth, 2,
                LAYOUT_AddChild, ButtonObject,
                  GA_ID, 10069,
                  GA_Text, "A4",
                  GA_RelVerify, TRUE,
                  BUTTON_BackgroundPen, 2,
                  BUTTON_TextPen, 1,
                  BUTTON_BevelStyle, BVS_NONE,
                ButtonEnd,
                CHILD_WeightedWidth, 10,
                LAYOUT_AddChild, BevelObject,
                  BEVEL_Style, BVS_SBAR_VERT,
                BevelEnd,
                CHILD_MaxWidth, 2,
                LAYOUT_AddChild, ButtonObject,
                  GA_ID, 10071,
                  GA_Text, "B4",
                  GA_RelVerify, TRUE,
                  BUTTON_BackgroundPen, 2,
                  BUTTON_TextPen, 1,
                  BUTTON_BevelStyle, BVS_NONE,
                ButtonEnd,
                CHILD_WeightedWidth, 10,
                LAYOUT_AddChild, BevelObject,
                  BEVEL_Style, BVS_SBAR_VERT,
                BevelEnd,
                CHILD_MaxWidth, 2,
              EndHGroup,
              CHILD_WeightedHeight, 2,
            EndVGroup,
            CHILD_WeightedWidth, 40,
          EndHGroup,
        VirtualEnd,
      EndWindow;
    base->ck_TestWindow = ( struct Window * )
      RA_OpenWindow( base->ck_TestWindowContent );
  if ( !( base->ck_TestWindow )) {

    return EOpenTestWindow;
  }

  /* Store window signal */
  base->ck_TestWindowSignal = ( 1L << base->ck_TestWindowPort->mp_SigBit );
  /* Store content signal */
  GetAttr( WINDOW_SigMask,
           base->ck_TestWindowContent,
           &( base-> ck_TestContentSignal ));

#if 0

  while ( !( abort )) {

    ULONG signalled =
      Wait( base->ck_TestWindowSignal
          | base->ck_TestContentSignal // <- has all events so far
          | SIGBREAKF_CTRL_C );
    if ( SIGBREAKF_CTRL_C & signalled ) {

      abort = TRUE;

    } else {

      abort = HandleKeyboardWindowEvents( base );
    }
  }

  CleanupKeyboardWindow( base );
#endif
  return ENoError;
}

VOID CleanupKeyboardWindow( struct CAMD_Keyboard * base ) {

  if ( base->ck_TestWindowContent ) {

    DisposeObject( base->ck_TestWindowContent );
    base->ck_TestWindowContent = NULL;
    LOG_V(( "V: Cleaned up TestWindowContent\n" ));
  }
  if ( base->ck_TestWindowPort ) {

    DeleteMsgPort( base->ck_TestWindowPort );
    base->ck_TestWindowPort = NULL;
    LOG_V(( "V: Cleaned up TestWindow MsgPort\n" ));
  }
}

BOOL HandleKeyboardWindowEvents( struct CAMD_Keyboard * base ) {
  ULONG code = 0;
  ULONG event;

  do {
    event = RA_HandleInput( base->ck_TestWindowContent, &( code ));

    switch ( event & WMHI_CLASSMASK ) {

      case WMHI_UNICONIFY: {

        RA_Uniconify( base->ck_TestWindowContent );
        break;
      }
      case WMHI_ICONIFY: {

        RA_Iconify(  base->ck_TestWindowContent );
        break;
      }
      case WMHI_CLOSEWINDOW: {

        LOG_I(( "I: Closing TestWindow\n" ));
        return TRUE;
      }
      case WMHI_GADGETUP: {

        ULONG gadgetId = ( event & WMHI_GADGETMASK );
        LOG_D(( "D: Handling event for %ld\n", gadgetId ));
        break;
      }
      case WMHI_ACTIVE:     // Window got activated
      case WMHI_LASTMSG: {  // All messages processed

        // Deliberately ignored :)
        break;
      }
      default: {

        LOG_W(( "D: Handling unknown event %lx\n", event ));
        break;
      }
    }
  } while ( event != WMHI_LASTMSG );

  return FALSE;
}
