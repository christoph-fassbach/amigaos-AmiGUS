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
#include <dos/dos.h>
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
#include <proto/virtual.h>
#include <proto/window.h>

#include "debug.h"
#include "errors.h"
#include "support.h"
#include "test_window.h"

LONG CreateTestUi( struct AmiGUS_CAMD_Tool * base ) {

  BOOL abort = FALSE;

  if ( base->agt_Flags & CAMD_TOOL_FLAG_REACTION_FAILED ) {

    DisplayError( EOpenReactionBases );
    return EOpenReactionBases;
  }
  
  base->agt_TestWindowPort = CreateMsgPort();
  if ( !( base->agt_TestWindowPort )) {

    return ECreateMessagePort;
  }

  base->agt_TestWindowContent = 
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
//        WINDOW_IDCMPHook, &( base->agt_TestWindowHook ),
//        WINDOW_IDCMPHookBits, IDCMP_IDCMPUPDATE,

        WINDOW_IconifyGadget, FALSE,
        WINDOW_AppPort, base->agt_TestWindowPort,
        WINDOW_Position, WPOS_TOPLEFT,
  
        WINDOW_ParentGroup, HLayoutObject,
          LAYOUT_AddChild, VLayoutObject,
            LAYOUT_AddChild, HLayoutObject,
              LAYOUT_AddChild, ButtonObject,
                GA_ID, 10060,
                GA_Text, "C4",
                GA_RelVerify, TRUE,
                BUTTON_BackgroundPen, 2,
                BUTTON_TextPen, 1,
              ButtonEnd,
              CHILD_WeightedWidth, 8,
              LAYOUT_AddChild, ButtonObject,
                GA_ID, 10061,
                GA_Text, "cis",
                GA_RelVerify, TRUE,
                BUTTON_BackgroundPen, 1,
                BUTTON_TextPen, 2,
              ButtonEnd,
              CHILD_WeightedWidth, 10,
              LAYOUT_AddChild, ButtonObject,
                GA_ID, 10062,
                GA_Text, "D4",
                GA_RelVerify, TRUE,
                BUTTON_BackgroundPen, 2,
                BUTTON_TextPen, 1,
              ButtonEnd,
              CHILD_WeightedWidth, 8,
              LAYOUT_AddChild, ButtonObject,
                GA_ID, 10063,
                GA_Text, "dis",
                GA_RelVerify, TRUE,
                BUTTON_BackgroundPen, 1,
                BUTTON_TextPen, 2,
              ButtonEnd,
              CHILD_WeightedWidth, 10,
              LAYOUT_AddChild, ButtonObject,
                GA_ID, 10064,
                GA_Text, "E4",
                GA_RelVerify, TRUE,
                BUTTON_BackgroundPen, 2,
                BUTTON_TextPen, 1,
              ButtonEnd,
              CHILD_WeightedWidth, 8,
            EndHGroup,
            LAYOUT_AddChild, HLayoutObject,
              LAYOUT_AddChild, ButtonObject,
                GA_ID, 10060,
                GA_Text, "C4",
                GA_RelVerify, TRUE,
                BUTTON_BackgroundPen, 2, // -1 background, 0 grey, 1 black, 2 white, 3 blue
                BUTTON_TextPen, 1,
              ButtonEnd,
              CHILD_WeightedWidth, 10,
              LAYOUT_AddChild, ButtonObject,
                GA_ID, 10062,
                GA_Text, "D4",
                GA_RelVerify, TRUE,
                BUTTON_BackgroundPen, 2,
                BUTTON_TextPen, 1,
              ButtonEnd,
              CHILD_WeightedWidth, 10,
              LAYOUT_AddChild, ButtonObject,
                GA_ID, 10064,
                GA_Text, "E4",
                GA_RelVerify, TRUE,
                BUTTON_BackgroundPen, 2, // white
                BUTTON_TextPen, 1,
//  BUTTON_BevelStyle, BVS_NONE,
  // BEVEL_HorizSize 0
  // BEVEL_VertSize 0
              ButtonEnd,
              CHILD_WeightedWidth, 10,
            EndHGroup,
          EndVGroup,
          CHILD_WeightedWidth, 30,

          LAYOUT_AddChild, VLayoutObject,
            LAYOUT_AddChild, HLayoutObject,
              LAYOUT_AddChild, ButtonObject,
                GA_ID, 10065,
                GA_Text, "F4",
                GA_RelVerify, TRUE,
                BUTTON_BackgroundPen, 2,
                BUTTON_TextPen, 1,
              ButtonEnd,
              CHILD_WeightedWidth, 8,
              LAYOUT_AddChild, ButtonObject,
                GA_ID, 10066,
                GA_Text, "fis",
                GA_RelVerify, TRUE,
                BUTTON_BackgroundPen, 1,
                BUTTON_TextPen, 2,
              ButtonEnd,
              CHILD_WeightedWidth, 10,
              LAYOUT_AddChild, ButtonObject,
                GA_ID, 10067,
                GA_Text, "G4",
                GA_RelVerify, TRUE,
                BUTTON_BackgroundPen, 2,
                BUTTON_TextPen, 1,
              ButtonEnd,
              CHILD_WeightedWidth, 8,
              LAYOUT_AddChild, ButtonObject,
                GA_ID, 10068,
                GA_Text, "gis",
                GA_RelVerify, TRUE,
                BUTTON_BackgroundPen, 1,
                BUTTON_TextPen, 2,
              ButtonEnd,
              CHILD_WeightedWidth, 10,
              LAYOUT_AddChild, ButtonObject,
                GA_ID, 10069,
                GA_Text, "A4",
                GA_RelVerify, TRUE,
                BUTTON_BackgroundPen, 2,
                BUTTON_TextPen, 1,
              ButtonEnd,
              CHILD_WeightedWidth, 8,
              LAYOUT_AddChild, ButtonObject,
                GA_ID, 10070,
                GA_Text, "ais",
                GA_RelVerify, TRUE,
                BUTTON_BackgroundPen, 1,
                BUTTON_TextPen, 2,
              ButtonEnd,
              CHILD_WeightedWidth, 10,
              LAYOUT_AddChild, ButtonObject,
                GA_ID, 10071,
                GA_Text, "B4",
                GA_RelVerify, TRUE,
                BUTTON_BackgroundPen, 2,
                BUTTON_TextPen, 1,
              ButtonEnd,
              CHILD_WeightedWidth, 8,
            EndHGroup,            
            LAYOUT_AddChild, HLayoutObject,
              LAYOUT_AddChild, ButtonObject,
                GA_ID, 10065,
                GA_Text, "F4",
                GA_RelVerify, TRUE,
                BUTTON_BackgroundPen, 2,
                BUTTON_TextPen, 1,
              ButtonEnd,
              CHILD_WeightedWidth, 10,
              LAYOUT_AddChild, ButtonObject,
                GA_ID, 10067,
                GA_Text, "G4",
                GA_RelVerify, TRUE,
                BUTTON_BackgroundPen, 2,
                BUTTON_TextPen, 1,
              ButtonEnd,
              CHILD_WeightedWidth, 10,
              LAYOUT_AddChild, ButtonObject,
                GA_ID, 10069,
                GA_Text, "A4",
                GA_RelVerify, TRUE,
                BUTTON_BackgroundPen, 2,
                BUTTON_TextPen, 1,
              ButtonEnd,
              CHILD_WeightedWidth, 10,
              LAYOUT_AddChild, ButtonObject,
                GA_ID, 10071,
                GA_Text, "B4",
                GA_RelVerify, TRUE,
                BUTTON_BackgroundPen, 2,
                BUTTON_TextPen, 1,
              ButtonEnd,
              CHILD_WeightedWidth, 10,
            EndHGroup,
          EndVGroup,
          CHILD_WeightedWidth, 40,
        EndHGroup,
      EndWindow;
  base->agt_TestWindow = ( struct Window * )
    RA_OpenWindow( base->agt_TestWindowContent );
  if ( !( base->agt_TestWindow )) {

    return EOpenTestWindow;
  }

  /* Store window signal */
  base->agt_TestWindowSignal = ( 1L << base->agt_TestWindowPort->mp_SigBit );
  /* Store content signal */
  GetAttr( WINDOW_SigMask,
           base->agt_TestWindowContent,
           &( base-> agt_TestContentSignal ));

  while ( !( abort )) {

    ULONG signalled =
      Wait( base->agt_TestWindowSignal
          | base->agt_TestContentSignal // <- has all events so far
          | SIGBREAKF_CTRL_C );
    if ( SIGBREAKF_CTRL_C & signalled ) {

      abort = TRUE;

    } else {

      abort = HandleTestUiEvents( base );
    }
  }

  CleanupTestUi( base );

  return ENoError;
}

VOID CleanupTestUi( struct AmiGUS_CAMD_Tool * base ) {

  if ( base->agt_TestWindowContent ) {

    DisposeObject( base->agt_TestWindowContent );
    base->agt_TestWindow = NULL;
    LOG_V(( "V: Cleaned up TestWindowContent\n" ));
  }
  if ( base->agt_TestWindowPort ) {

    DeleteMsgPort( base->agt_TestWindowPort );
    base->agt_TestWindowPort = NULL;
    LOG_V(( "V: Cleaned up TestWindow MsgPort\n" ));
  }
}

BOOL HandleTestUiEvents( struct AmiGUS_CAMD_Tool * base ) {
  ULONG code = 0;
  ULONG event;

  do {
    event = RA_HandleInput( base->agt_TestWindowContent, &( code ));

    switch ( event & WMHI_CLASSMASK ) {

      case WMHI_UNICONIFY: {

        RA_Uniconify( base->agt_TestWindowContent );
        break;
      }
      case WMHI_ICONIFY: {

        RA_Iconify(  base->agt_TestWindowContent );
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