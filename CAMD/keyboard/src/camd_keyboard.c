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

#include <clib/alib_protos.h>
#include <graphics/gfxbase.h>
#include <intuition/intuitionbase.h>
#include <reaction/reaction_macros.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/layout.h>
#include <proto/scroller.h>
#include <proto/virtual.h>
#include <proto/window.h>

#include "camd_keyboard.h"
#include "clavier_gadgetclass.h"
#include "debug.h"
#include "errors.h"
#include "support.h"

/* Globals defined somewhere - here ;) */
struct CAMD_Keyboard     * CAMD_Keyboard_Base;   // Main app struct
// System libraries:
struct GfxBase           * GfxBase;
struct IntuitionBase     * IntuitionBase;
// and some more owned by the linker libraries:
// struct ExecBase       * SysBase;
// struct DosLibrary     * DOSBase;
// And for ReAction:
struct ClassLibrary      * ButtonBase;
struct ClassLibrary      * LayoutBase;
struct ClassLibrary      * ScrollerBase;
struct ClassLibrary      * VirtualBase;
struct ClassLibrary      * WindowBase;

Class                    * ClavierGadgetClass;

enum GadgetIds {

  GadgetId_Start = 100,
  GadgetId_ButtonOK,
  GadgetId_ButtonCancel,
  GadgetId_ClavierButton,
  GadgetId_ClavierScrollPane,
  GadgetId_Clavier,
  GadgetId_End
};

ULONG OpenLib( struct Library ** library, STRPTR name, ULONG version, ULONG error ) {

  *library = OpenLibrary( name, version );
  if ( *library ) {

    return ENoError;
  }

  DisplayError( error );
  return error;
}

VOID CloseLib( struct Library ** library ) {

  if (( library ) && ( *library )) {

    CloseLibrary( *library );
    *library = NULL;
  }
}

ULONG Startup( VOID ) {

  if ( !CAMD_Keyboard_Base ) {

    CAMD_Keyboard_Base = AllocMem( sizeof( struct CAMD_Keyboard ), MEMF_ANY | MEMF_CLEAR );
  }
  if ( !CAMD_Keyboard_Base ) {

    DisplayError( EAllocateAmiGUSCAMDToolBase );
    return EAllocateAmiGUSCAMDToolBase;
  }
  OpenLib(( struct Library ** )&IntuitionBase, "intuition.library", 36, EOpenIntuitionBase );
  OpenLib(( struct Library ** )&GfxBase, "graphics.library", 36, EOpenGfxBase );

  OpenLib(( struct Library ** )&ButtonBase, "gadgets/button.gadget", 0, EOpenButtonBase );
  OpenLib(( struct Library ** )&LayoutBase, "gadgets/layout.gadget", 0, EOpenLayoutBase );
  OpenLib(( struct Library ** )&ScrollerBase, "gadgets/scroller.gadget", 0, EOpenScrollerBase );
  OpenLib(( struct Library ** )&VirtualBase, "gadgets/virtual.gadget", 45, EOpenVirtualBase );
  OpenLib(( struct Library ** )&WindowBase, "window.class", 0, EOpenWindowBase );

  ClavierGadgetClass = InitClavierGadgetClass();
  if ( !( ClavierGadgetClass )) {

    DisplayError( EInitClavierGadgetClass );
  }

  LOG_I(( "I: " STR( APP_NAME ) " startup complete.\n" ));
}

VOID Cleanup( VOID ) {

  CloseLib(( struct Library ** )&WindowBase );
  CloseLib(( struct Library ** )&VirtualBase );
  CloseLib(( struct Library ** )&ScrollerBase );
  CloseLib(( struct Library ** )&LayoutBase );
  CloseLib(( struct Library ** )&ButtonBase );

  CloseLib(( struct Library ** )&GfxBase );
  CloseLib(( struct Library ** )&IntuitionBase );
  LOG_I(( "I: " STR( APP_NAME ) " cleanup starting.\n" ));
  if ( CAMD_Keyboard_Base ) {

    if ( CAMD_Keyboard_Base->ck_LogFile ) {

      LOG_I(( "I: Attempting CAMD_Keyboard_Base->ck_LogFile.\n" ));
      Close( CAMD_Keyboard_Base->ck_LogFile );
    }
    /* Free'ing agt_LogMem deliberately not happening here! */

    LOG_I(( "I: Attempting CAMD_Keyboard_Base.\n" ));
    FreeMem( CAMD_Keyboard_Base, sizeof( struct CAMD_Keyboard ));
    CAMD_Keyboard_Base = NULL;
  }
  // No logging after this anymore!
}

VOID OpenWin( VOID ) { // TODO: enable error handling and return values
#if 0
  APTR scrollPane = 
    ScrollerObject,
      SCROLLER_Orientation, SCROLLER_HORIZONTAL,
      SCROLLER_Total, 100,
      SCROLLER_Visible, 10,
    ScrollerEnd;
#endif

  CAMD_Keyboard_Base->ck_Screen = LockPubScreen( NULL );

  if ( !CAMD_Keyboard_Base->ck_Screen ) {

    return;
  }

  CAMD_Keyboard_Base->ck_MainWindowContent = WindowObject,
    WA_PubScreen, CAMD_Keyboard_Base->ck_Screen,
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
//    WA_Flags, WFLG_CLOSEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_SIZEBBOTTOM | WFLG_SIZEGADGET | WFLG_ACTIVATE,
//    WA_IDCMP, IDCMP_GADGETDOWN | IDCMP_GADGETUP | IDCMP_IDCMPUPDATE | IDCMP_VANILLAKEY,
				WA_Flags,WFLG_CLOSEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_SIZEBBOTTOM | WFLG_SIZEGADGET | WFLG_ACTIVATE,
				WA_IDCMP,IDCMP_VANILLAKEY | IDCMP_NEWSIZE,
    WINDOW_Position, WPOS_TOPLEFT,
    WINDOW_IconifyGadget, FALSE,
    WINDOW_ParentGroup, VLayoutObject,
      LAYOUT_AddChild, HLayoutObject,
        LAYOUT_AddChild, ButtonObject,
          GA_Text, "Ok",
          GA_ID, GadgetId_ButtonOK,
          GA_RelVerify, TRUE,
        ButtonEnd,

        LAYOUT_AddChild, ButtonObject,
          GA_Text, "Cancel",
          GA_ID, GadgetId_ButtonCancel,
          GA_RelVerify, TRUE,
        ButtonEnd,
      LayoutEnd,

      LAYOUT_AddChild, ButtonObject,
        GA_Text, "Clavier",
        GA_ID, GadgetId_ClavierButton,
        GA_RelVerify, TRUE,
      ButtonEnd,
//      LAYOUT_AddChild, scrollPane,
      LAYOUT_AddChild, CAMD_Keyboard_Base->ck_ScrollPane = VirtualObject,
        GA_ID, GadgetId_ClavierScrollPane,
        GA_RelVerify, TRUE,
//        VIRTUALA_ScrollerX, scrollPane,
//        VIRTUALA_Contents, HLayoutObject,
#if 0
          LAYOUT_AddChild, ButtonObject,
            GA_Text, "Hallo lieber bernt",
            GA_ID, GadgetId_ClavierButton + 10,
            GA_RelVerify, TRUE,
          ButtonEnd,
#endif
//          LAYOUT_AddChild, NewObject( ClavierGadgetClass, NULL,
          VIRTUALA_Contents, CAMD_Keyboard_Base->ck_Clavier = NewObject( ClavierGadgetClass, NULL,
            GA_ID, GadgetId_Clavier,
            //GA_RelSpecial, TRUE,
//            CHILD_MinHeight, 60,
  //          CHILD_MinWidth, 1260,
            GA_RelVerify, TRUE,

          TAG_END ),
#if 0
          LAYOUT_AddChild, ButtonObject,
            GA_Text, "ja das muss sein",
            GA_ID, GadgetId_ClavierButton + 10,
            GA_RelVerify, TRUE,
          ButtonEnd,
#endif
//        LayoutEnd,
      VirtualEnd,


#if 0
      LAYOUT_AddChild, ButtonObject,
        GA_Text, "Clavier",
        GA_ID, GadgetId_ClavierButton,
        GA_RelVerify, TRUE,
      ButtonEnd,
#endif
    LayoutEnd,
  EndWindow;

  if ( !CAMD_Keyboard_Base->ck_MainWindowContent ) {
    return;
  }
#if 0
  RethinkVirtualSize(
    scrollPane, 
    CAMD_Keyboard_Base->ck_MainWindowContent,
    NULL,
    CAMD_Keyboard_Base->ck_Screen,
    NULL );
#endif
  CAMD_Keyboard_Base->ck_MainWindow = ( struct Window * )
    RA_OpenWindow( CAMD_Keyboard_Base->ck_MainWindowContent );

  if ( !CAMD_Keyboard_Base->ck_MainWindow ) {
    return;
  }

  GetAttr( WINDOW_SigMask,
           CAMD_Keyboard_Base->ck_MainWindowContent, 
           &( CAMD_Keyboard_Base->ck_MainWindowSignal ));
// RethinkLayout((struct Gadget *)CAMD_Keyboard_Base->ck_MainWindowContent, CAMD_Keyboard_Base->ck_MainWindow, NULL, TRUE);

  return;
}

VOID HandleEvents( VOID ) {

  BOOL stop = FALSE;

  while ( !( stop )) {

    ULONG signals = Wait( CAMD_Keyboard_Base->ck_MainWindowSignal
                          | SIGBREAKF_CTRL_C );
    if ( SIGBREAKF_CTRL_C & signals) {
      stop = TRUE;
    }

    for ( ; ; ) {

      WORD windowMessageCode;
      ULONG windowMessage = DoMethod( CAMD_Keyboard_Base->ck_MainWindowContent,
                                      WM_HANDLEINPUT, 
                                      &windowMessageCode );

      if ( WMHI_LASTMSG == windowMessage ) {

        // All messages handled => break for-loop!
        break;
      }

      switch ( WMHI_CLASSMASK & windowMessage ) {
        case WMHI_GADGETUP: {
          switch ( WMHI_GADGETMASK & windowMessage ) {
            case GadgetId_ButtonOK:
            case GadgetId_ButtonCancel: {

              stop = TRUE;
              break;
            }
            case GadgetId_ClavierButton: {
  //            Printf( "%ld\n", windowMessageCode );
  ULONG a, b, c, d, e, f, g, h;
  GetAttr( VIRTUALA_TotalX, CAMD_Keyboard_Base->ck_ScrollPane, &a );
  GetAttr( VIRTUALA_TotalY, CAMD_Keyboard_Base->ck_ScrollPane, &b );
  GetAttr( VIRTUALA_TopX, CAMD_Keyboard_Base->ck_ScrollPane, &c );
  GetAttr( VIRTUALA_TopY, CAMD_Keyboard_Base->ck_ScrollPane, &d );
  GetAttr( VIRTUALA_VisibleX, CAMD_Keyboard_Base->ck_ScrollPane, &e );
  GetAttr( VIRTUALA_VisibleY, CAMD_Keyboard_Base->ck_ScrollPane, &f );
  GetAttr( GA_Width, CAMD_Keyboard_Base->ck_ScrollPane, &g );
  GetAttr( GA_Height, CAMD_Keyboard_Base->ck_ScrollPane, &h );
  Printf("TotalX %ld TotalY %ld TopX %ld TopY %ld VisX %ld VisY %ld w %ld h %ld\n", a, b, c, d, e, f, g, h );

              break;
            }
            case GadgetId_Clavier: {
              Printf( "echtes clavier %ld\n", windowMessageCode );
              break;
            }
          }
          break;
        }
        case WMHI_VANILLAKEY: {
          if (( WMHI_KEYMASK & windowMessage ) == 0x1b) {
            stop = TRUE;
          }
          break;
        }
        case WMHI_CLOSEWINDOW: {
          stop = TRUE;
          break;
        }
        case WMHI_NEWSIZE: {
          Printf("asdf\n");
          /*
          SetAttrs( CAMD_Keyboard_Base->ck_ScrollPane,
                    VIRTUALA_Contents, CAMD_Keyboard_Base->ck_Clavier = NewObject( ClavierGadgetClass, NULL,
            GA_ID, GadgetId_Clavier,
            //GA_RelSpecial, TRUE,
//            CHILD_MinHeight, 60,
  //          CHILD_MinWidth, 1260,
            GA_RelVerify, TRUE,

          TAG_END ),
        TAG_END);
        */
          RethinkVirtualSize(
            CAMD_Keyboard_Base->ck_ScrollPane,
            CAMD_Keyboard_Base->ck_MainWindowContent,
            NULL,
            CAMD_Keyboard_Base->ck_Screen,
            NULL
          );
          RethinkLayout(
            (struct Gadget *)CAMD_Keyboard_Base->ck_MainWindowContent,
            CAMD_Keyboard_Base->ck_MainWindow,
            NULL,
            TRUE );
          RefreshVirtualGadget(
            CAMD_Keyboard_Base->ck_Clavier,
            CAMD_Keyboard_Base->ck_ScrollPane,
            CAMD_Keyboard_Base->ck_MainWindow,
            NULL
          );
          break;
        }
        default: {
          break;
        }
      }
    }
  }
}

VOID CloseWin( VOID ) {

  if ( !CAMD_Keyboard_Base ) {

    return;
  }
  if ( CAMD_Keyboard_Base->ck_MainWindowContent ) {

    DisposeObject( CAMD_Keyboard_Base->ck_MainWindowContent );
    CAMD_Keyboard_Base->ck_MainWindowContent = NULL;
  }
  if ( CAMD_Keyboard_Base->ck_Screen ) {

    UnlockPubScreen( NULL, CAMD_Keyboard_Base->ck_Screen );
    CAMD_Keyboard_Base->ck_Screen = NULL;
  }
}

int main( VOID ) {

  Startup();
  OpenWin();
  HandleEvents();
  CloseWin();
  Cleanup();

  return ENoError;
}