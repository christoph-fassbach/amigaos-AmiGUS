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

#include <proto/exec.h>
#include <proto/gadtools.h>
#include <proto/intuition.h>

#include "camd_prefs.h"
#include "debug.h"
#include "errors.h"
#include "prefs_window.h"
#include "support.h"

#define GADGET_COUNT 3

CONST_STRPTR headerLabelText  = "AmiGUS CAMD/MIDI sound font file:";
CONST_STRPTR selectButtonText = "Select";
CONST_STRPTR infoLabelText = "Path: Sys:Expansion/AmiGUS/2MB.sf2\n"
                       "Instruments: 127/127 Samples: 300 \n"
                       "Percussions:   81/81 Samples:  81 \n"
                       "Sample size: 1234444/33554432 bytes";
STRPTR strategyLabelText  = "Loading strategy:";
STRPTR strategyUpfrontText  = "All upfront";
STRPTR strategyRamText  = "RAM cache";
STRPTR strategyDemandText  = "On demand";
STRPTR progressLabelText  = "Show progress bar:";
STRPTR testButtonText = "Test";
STRPTR applyButtonText = "Use";
STRPTR saveButtonText = "Save";
STRPTR cancelButtonText = "Cancel";

struct TextAttr topaz8 = { ( STRPTR ) "topaz.font", 8, 0, 1 };

/* Data for gadget structures */
struct NewGadget Gadgetdata[ GADGET_COUNT ] = {
  3, 15, 1, 13, "AmiGUS CAMD/MIDI sound font file:", &topaz8, 1, PLACETEXT_RIGHT, NULL, NULL,
  280, 14, 56, 15, "Select", &topaz8, 2, PLACETEXT_IN, NULL, NULL,
  10, 20, 230, 100, "Path: Sys:Expansion/AmiGUS/2MB.sf2\n"
                       "Instruments: 127/127 Samples: 300 \n"
                       "Percussions:   81/81 Samples:  81 \n"
                       "Sample size: 1234444/33554432 bytes", &topaz8, 3, PLACETEXT_IN, NULL,    NULL 
};

ULONG GadgetKinds[ GADGET_COUNT ] = {
  TEXT_KIND,
  BUTTON_KIND,
  TEXT_KIND
};


/* Extra information for gadgets using Tags */
ULONG GadgetTags[] = {
  (GTST_MaxChars), 256, (TAG_DONE),
  (GTNM_Border), TRUE, (TAG_DONE),
  (TAG_DONE)
};

LONG CreatePrefsUi( struct AmiGUS_CAMD_Tool * base ) {

  struct Gadget * gadget;
  LONG i;

  base->agt_Screen = LockPubScreen( NULL );
  if ( !( base->agt_Screen )) {

    DisplayError( ELockPubScreen );
    return ELockPubScreen;
  }

  base->agt_VisualInfo = GetVisualInfo( base->agt_Screen, TAG_DONE );
  if ( !( base->agt_VisualInfo )) {

    DisplayError( EGadToolsGetVisualInfo );
    return EGadToolsGetVisualInfo;
  }

  gadget = CreateContext( &( base->agt_GadgetList ));
  if ( !( gadget )) {

    DisplayError( EGadToolsCreateContext );
    return EGadToolsCreateContext;
  }

  // Create all gadgets from static table above:
  for ( i = 0; i < GADGET_COUNT; ++i ) {

    Gadgetdata[i].ng_VisualInfo = base->agt_VisualInfo;
    gadget = CreateGadgetA(
      GadgetKinds[i], 
      gadget, 
      &Gadgetdata[i], 
      ( struct TagItem * ) &GadgetTags[ i ] );

      if ( gadget ) {

      LOG_D(( "D: Gadget %ld created.\n", i ));

    } else {

      LOG_W(( "W: Failed to create gadget %ld.\n", i ));
    }
  }

  // Open window specifying the gadget list:
  base->agt_Window = OpenWindowTags(
    NULL,
    WA_Left, 10,
    WA_Top, 15,
    WA_Width, 380,
    WA_Height, 180,
    WA_IDCMP, IDCMP_CLOSEWINDOW |
              IDCMP_GADGETUP,
    WA_Flags, WFLG_DRAGBAR |
              WFLG_DEPTHGADGET |
              WFLG_CLOSEGADGET |
              WFLG_ACTIVATE |
              WFLG_SMART_REFRESH,
    WA_Gadgets, base->agt_GadgetList,
    WA_Title, "AmiGUS CAMD/MIDI preferences",
    WA_PubScreenName, "Workbench",
    TAG_DONE );
  GT_RefreshWindow( base->agt_Window, NULL );

  return ENoError;
}

VOID CleanupPrefsUi( struct AmiGUS_CAMD_Tool * base ) {

  if ( base->agt_Window ) {

    CloseWindow( base->agt_Window );
    base->agt_Window = NULL;
  }

  if ( base->agt_GadgetList ) {

    FreeGadgets( base->agt_GadgetList );
    base->agt_GadgetList = NULL;
  }

  if ( base->agt_VisualInfo ) {

    FreeVisualInfo( base->agt_VisualInfo );
    base->agt_VisualInfo = NULL;
  }
  if ( base->agt_Screen ) {

    UnlockPubScreen( NULL, base->agt_Screen );
    base->agt_Screen = NULL;
  }
}

VOID HandlePrefsUiEvents( struct AmiGUS_CAMD_Tool * base ) {

  while ( TRUE ) {

    struct IntuiMessage * message;
    ULONG messageClass;

    Wait( 1L << base->agt_Window->UserPort->mp_SigBit );
    message = GT_GetIMsg( base->agt_Window->UserPort );
    messageClass = message->Class;
    GT_ReplyIMsg( message );

    if ( IDCMP_CLOSEWINDOW == messageClass ) {

      return;
    }
  }
}
