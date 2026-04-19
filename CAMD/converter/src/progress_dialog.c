/*
 * This file is part of the SoundFontConverter.
 *
 * SoundFontConverter is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3 of the License only.
 *
 * SoundFontConverter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SoundFontConverter.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <reaction/reaction_macros.h>

#include <proto/alib.h>
#include <proto/exec.h>
#include <proto/fuelgauge.h>
#include <proto/intuition.h>
#include <proto/window.h>
#include <proto/label.h>
#include <proto/layout.h>
#include <proto/button.h>

#include "progress_dialog.h"

#include "converter.h"
#include "debug.h"
#include "errors.h"

struct ProgressDialog * CreateProgressDialog(
  struct Window * parentWindow,
  CONST_STRPTR title,
  CONST_STRPTR message,
  CONST_STRPTR button ) {

  WORD parentX = parentWindow->LeftEdge;
  WORD parentY = parentWindow->TopEdge;
  WORD parentWidth = parentWindow->Width;
  WORD parentHeight =  parentWindow->Height;

  struct ProgressDialog * dialog = AllocMem(
    sizeof( struct ProgressDialog ),
    MEMF_ANY );

  if ( !( dialog )) {

    return NULL;
  }
  LOG_V(( "V: x:%ld y:%ld w:%ld h:%ld\n",
          parentX, parentY, parentWidth, parentHeight ));

  dialog->pd_ParentWindow = parentWindow;
  InitRequester( &( dialog->pd_LockRequest ));
  dialog->pd_ProgressGauge =
    FuelGaugeObject,
      FUELGAUGE_Min, 0,
      FUELGAUGE_Max, 100,
      FUELGAUGE_Level, 0,
      FUELGAUGE_Orientation, FGORIENT_HORIZ,
      FUELGAUGE_Percent, TRUE,
      FUELGAUGE_Ticks, 10,
      FUELGAUGE_TickSize, 5,
      FUELGAUGE_ShortTicks, TRUE,
      FUELGAUGE_Justification, FGJ_CENTER,
    FuelGaugeEnd;
  if ( !( dialog->pd_ProgressGauge )) {

    return NULL;
  }

  dialog->pd_ProgressContent =
    WindowObject,
      WA_ScreenTitle, APP_IDSTRING,
      WA_Title, title,
      WA_Activate, TRUE,
      WA_DepthGadget, TRUE,
      WA_DragBar, TRUE,
      WA_CloseGadget, TRUE,
      WA_SizeGadget, TRUE,
      WA_Top, ( parentY + parentHeight - 60 ) >> 1,
      WA_Left, ( parentX + parentWidth - 300 ) >> 1,
      WA_Width, 300,
      //WA_Height, 60,
      WA_SmartRefresh, TRUE,
      //WINDOW_Parent,    parentWindow,
      //WINDOW_SharedPort, port,
      // WINDOW_Position, WPOS_CENTERWINDOW,
      // WINDOW_RefWindow, parentWindow,
      WINDOW_ParentGroup, VLayoutObject,

        LAYOUT_AddChild, VGroupObject,
          LAYOUT_SpaceOuter, TRUE,
          LAYOUT_VertAlignment, LALIGN_CENTER,
          LAYOUT_HorizAlignment, LALIGN_CENTER,
          LAYOUT_AddImage, LabelObject,
            LABEL_Text, message,
          LabelEnd,
        LayoutEnd,

        LAYOUT_AddChild, dialog->pd_ProgressGauge,
        CHILD_WeightedHeight, 0,

        LAYOUT_AddChild, ButtonObject,
          GA_Text, button,
          GA_RelVerify, TRUE,
        ButtonEnd,
        CHILD_WeightedHeight, 0,

      LayoutEnd,
    WindowEnd;
  if ( !( dialog->pd_ProgressContent )) {

    return NULL;
  }

  return dialog;
}

ULONG ShowProgressDialog( struct ProgressDialog * dialog ) {

  Request( &( dialog->pd_LockRequest ), dialog->pd_ParentWindow );
  SetWindowPointer( dialog->pd_ParentWindow,
                    WA_BusyPointer, TRUE,
                    TAG_DONE );
  
  dialog->pd_ProgressWindow = RA_OpenWindow( dialog->pd_ProgressContent );
  if ( dialog->pd_ProgressWindow ) {

    return EAllocateProgressDialog;
  }

  return ENoError;
}

BOOL HandleProgressDialogTick( struct ProgressDialog * dialog,
                               ULONG newValue,
                               ULONG newMax ) {

  BOOL result = FALSE;
  WORD windowMessageCode;
  ULONG windowMessage;

  SetGadgetAttrs( dialog->pd_ProgressGauge,
                  dialog->pd_ProgressWindow,
                  NULL,
                  FUELGAUGE_Max, newMax,
                  FUELGAUGE_Level, newValue,
                  TAG_END );
  while ( WMHI_LASTMSG 
    != ( windowMessage = DoMethod( dialog->pd_ProgressContent,
                                   WM_HANDLEINPUT, 
                                   &windowMessageCode ))) {

    if ( WMHI_LASTMSG == windowMessage ) {

      // All messages handled => break for-loop!
      LOG_D(( "a\n" ));
      break;
    }
    switch ( WMHI_CLASSMASK & windowMessage ) {
      case WMHI_GADGETUP:
      case WMHI_CLOSEWINDOW: {

        LOG_D(( "D: Aborting...\n" ));
        result = TRUE;
        break;
      }
      default: {
        LOG_D(( "D: Unknown event 0x%08lx-0x%04lx in progress dialog.\n",
                windowMessage, windowMessageCode ));
        break;
      }
    }
  }
  return result;
}

VOID CloseProgressDialog( struct ProgressDialog * dialog ) {

  DisposeObject( dialog->pd_ProgressContent );
  EndRequest( &( dialog->pd_LockRequest ), dialog->pd_ParentWindow );
  SetWindowPointer( dialog->pd_ParentWindow,
                    WA_BusyPointer, FALSE,
                    TAG_DONE );
}

VOID FreeProgressDialog( struct ProgressDialog * dialog ) {

  FreeMem( dialog, sizeof( struct ProgressDialog ));
}