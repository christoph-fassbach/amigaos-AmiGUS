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
#include "test_window.h"

struct TextAttr topaz8 = { ( STRPTR ) "topaz.font", 8, 0, 1 };

enum GadgetIds {

  GADGET_CONTEXT = 0,
  GADGET_HEADER_LABEL,
  GADGET_SELECT_BUTTON,
  GADGET_INFO_BORDER_LABEL,
  GADGET_INFO_PATH_LABEL,
  GADGET_INFO_INSTRUMENTS_LABEL,
  GADGET_INFO_INSTRUMENT_SAMPLES_LABEL,
  GADGET_INFO_PERCUSSIONS_LABEL,
  GADGET_INFO_PERCUSSIONS_SAMPLES_LABEL,
  GADGET_INFO_SAMPLE_SIZE_LABEL,
  GADGET_INFO_SAMPLE_RAM_LABEL,
  GADGET_INFO_SAMPLE_MAX_LABEL,
  GADGET_SAMPLE_STRATEGY_CYCLE,
  GADGET_SAMPLE_PROGRESS_CHECKBOX,
  GADGET_TEST_BUTTON,
  GADGET_SAVE_BUTTON,
  GADGET_USE_BUTTON,
  GADGET_CANCEL_BUTTON
};

ULONG GadgetKind1 = TEXT_KIND;
struct NewGadget GadgetNew1 = {
  5, 15, 270, 13, "AmiGUS CAMD/MIDI sound font file:", &topaz8, GADGET_HEADER_LABEL, PLACETEXT_IN, NULL, NULL };
ULONG GadgetTags1[] = { (TAG_DONE) };

ULONG GadgetKind2 = BUTTON_KIND;
struct NewGadget GadgetNew2 = {
    280, 14,  70, 15, "Select", &topaz8, GADGET_SELECT_BUTTON, PLACETEXT_IN, NULL, NULL };
ULONG GadgetTags2[] = { (TAG_DONE) };

ULONG GadgetKind3 = TEXT_KIND;
struct NewGadget GadgetNew3 = {
  9, 35, 342, 86, NULL, &topaz8, GADGET_INFO_BORDER_LABEL, PLACETEXT_IN, NULL, NULL };
ULONG GadgetTags3[] = { GTTX_Border, TRUE, ( TAG_DONE ) };

ULONG GadgetKind4 = TEXT_KIND;
struct NewGadget GadgetNew4 = {
  69, 40, 200, 15, "Path:", &topaz8, GADGET_INFO_PATH_LABEL, PLACETEXT_LEFT, NULL, NULL };
ULONG GadgetTags4[] = { GTTX_Text, ( ULONG ) "Sys:Expansion/AmiGUS/2MB.sf2", ( TAG_DONE ) };

ULONG GadgetKind5 = TEXT_KIND;
struct NewGadget GadgetNew5 = {
  125, 55, 50, 15, "Instruments:", &topaz8, GADGET_INFO_INSTRUMENTS_LABEL, PLACETEXT_LEFT, NULL, NULL };
ULONG GadgetTags5[] = { GTTX_Text, ( ULONG ) "128", ( TAG_DONE ) };

ULONG GadgetKind6 = TEXT_KIND;
struct NewGadget GadgetNew6 = {
  250, 55, 50, 15, "Samples:", &topaz8, GADGET_INFO_INSTRUMENT_SAMPLES_LABEL, PLACETEXT_LEFT, NULL, NULL };
ULONG GadgetTags6[] = { GTTX_Text, ( ULONG ) "1234", ( TAG_DONE ) };

ULONG GadgetKind7 = TEXT_KIND;
struct NewGadget GadgetNew7 = {
  125, 70, 50, 15, "Percussions:", &topaz8, GADGET_INFO_PERCUSSIONS_LABEL, PLACETEXT_LEFT, NULL, NULL };
ULONG GadgetTags7[] = { GTTX_Text, ( ULONG ) "85", ( TAG_DONE ) };

ULONG GadgetKind8 = TEXT_KIND;
struct NewGadget GadgetNew8 = {
  250, 70, 50, 15, "Samples:", &topaz8, GADGET_INFO_PERCUSSIONS_SAMPLES_LABEL, PLACETEXT_LEFT, NULL, NULL };
ULONG GadgetTags8[] = { GTTX_Text, ( ULONG ) "19", ( TAG_DONE ) };

ULONG GadgetKind9 = TEXT_KIND;
struct NewGadget GadgetNew9 = {
  125, 85, 50, 15, "Sample size:", &topaz8, GADGET_INFO_SAMPLE_SIZE_LABEL, PLACETEXT_LEFT, NULL, NULL };
ULONG GadgetTags9[] = { GTTX_Text, ( ULONG ) "33554432", ( TAG_DONE ) };

ULONG GadgetKind10 = TEXT_KIND;
struct NewGadget GadgetNew10 = {
  285, 85, 50, 15, "/ 33554432", &topaz8, GADGET_INFO_SAMPLE_MAX_LABEL, PLACETEXT_LEFT, NULL, NULL };
ULONG GadgetTags10[] = { ( TAG_DONE ) };

ULONG GadgetKind11 = TEXT_KIND;
struct NewGadget GadgetNew11 = {
  250, 100, 50, 15, "AmiGUS sample RAM used:", &topaz8, GADGET_SAMPLE_STRATEGY_CYCLE, PLACETEXT_LEFT, NULL, NULL };
ULONG GadgetTags11[] = { ( GTTX_Text ), ( ULONG ) "100%", ( TAG_DONE ) };

STRPTR GadgetOptions12[] = { "Upfront", "RAM", "On Demand", NULL };
ULONG GadgetKind12 = CYCLE_KIND;
struct NewGadget GadgetNew12 = {
  208, 125, 110, 15, "Sample loading strategy:", &topaz8, GADGET_SAMPLE_STRATEGY_CYCLE, PLACETEXT_LEFT, NULL, NULL };
ULONG GadgetTags12[] = { ( GTCY_Labels ), ( ULONG ) &GadgetOptions12, ( GTCY_Active ), 1, ( TAG_DONE ) };

ULONG GadgetKind13 = CHECKBOX_KIND;
struct NewGadget GadgetNew13 = {
  272, 143, 50, 15, "Show progress bar while loading:", &topaz8, GADGET_SAMPLE_PROGRESS_CHECKBOX, PLACETEXT_LEFT, NULL, NULL };
ULONG GadgetTags13[] = { ( GTCB_Checked ), TRUE, ( TAG_DONE ) };

ULONG GadgetKind14 = BUTTON_KIND;
struct NewGadget GadgetNew14 = {
    9, 160,  70, 15, "Test", &topaz8, GADGET_TEST_BUTTON, PLACETEXT_IN, NULL, NULL };
ULONG GadgetTags14[] = { (TAG_DONE) };

ULONG GadgetKind15 = BUTTON_KIND;
struct NewGadget GadgetNew15 = {
    99, 160,  70, 15, "Save", &topaz8, GADGET_SAVE_BUTTON, PLACETEXT_IN, NULL, NULL };
ULONG GadgetTags15[] = { (TAG_DONE) };

ULONG GadgetKind16 = BUTTON_KIND;
struct NewGadget GadgetNew16 = {
    189, 160,  70, 15, "Use", &topaz8, GADGET_USE_BUTTON, PLACETEXT_IN, NULL, NULL };
ULONG GadgetTags16[] = { (TAG_DONE) };

ULONG GadgetKind17 = BUTTON_KIND;
struct NewGadget GadgetNew17 = {
    279, 160,  70, 15, "Cancel", &topaz8, GADGET_CANCEL_BUTTON, PLACETEXT_IN, NULL, NULL };
ULONG GadgetTags17[] = { (TAG_DONE) };

struct GadgetParams {
  ULONG * kind;
  struct NewGadget * gadget;
  ULONG * tags;
};

struct GadgetParams Params[] = {
  {  &GadgetKind1,  &GadgetNew1,  GadgetTags1 },
  {  &GadgetKind2,  &GadgetNew2,  GadgetTags2 },
  {  &GadgetKind3,  &GadgetNew3,  GadgetTags3 },
  {  &GadgetKind4,  &GadgetNew4,  GadgetTags4 },
  {  &GadgetKind5,  &GadgetNew5,  GadgetTags5 },
  {  &GadgetKind6,  &GadgetNew6,  GadgetTags6 },
  {  &GadgetKind7,  &GadgetNew7,  GadgetTags7 },
  {  &GadgetKind8,  &GadgetNew8,  GadgetTags8 },
  {  &GadgetKind9,  &GadgetNew9,  GadgetTags9 },
  { &GadgetKind10, &GadgetNew10, GadgetTags10 },
  { &GadgetKind11, &GadgetNew11, GadgetTags11 },
  { &GadgetKind12, &GadgetNew12, GadgetTags12 },
  { &GadgetKind13, &GadgetNew13, GadgetTags13 },
  { &GadgetKind14, &GadgetNew14, GadgetTags14 },
  { &GadgetKind15, &GadgetNew15, GadgetTags15 },
  { &GadgetKind16, &GadgetNew16, GadgetTags16 },
  { &GadgetKind17, &GadgetNew17, GadgetTags17 },
};
#define GADGET_COUNT ( sizeof( Params ) / 12 )

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

    if (( AmiGUS_CAMD_Tool_Base->agt_Flags & CAMD_TOOL_FLAG_REACTION_FAILED )
      && ( GADGET_TEST_BUTTON == Params[i].gadget->ng_GadgetID )) {

      continue;
    }

    Params[i].gadget->ng_VisualInfo = base->agt_VisualInfo;
    gadget = CreateGadgetA(
      *( Params[i].kind ), 
      gadget, 
      Params[i].gadget,
      ( struct TagItem * ) Params[i].tags );

      if ( gadget ) {

      LOG_D(( "D: Gadget %ld created.\n", i ));

    } else {

      LOG_W(( "W: Failed to create gadget %ld.\n", i ));
    }
  }

  // Open window specifying the gadget list:
  base->agt_MainWindow = OpenWindowTags(
    NULL,
    WA_Left, 10,
    WA_Top, 15,
    WA_Width, 360,
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
  GT_RefreshWindow( base->agt_MainWindow, NULL );

  return ENoError;
}

VOID CleanupPrefsUi( struct AmiGUS_CAMD_Tool * base ) {

  if ( base->agt_MainWindow ) {

    CloseWindow( base->agt_MainWindow );
    base->agt_MainWindow = NULL;
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

    Wait( 1L << base->agt_MainWindow->UserPort->mp_SigBit );
    message = GT_GetIMsg( base->agt_MainWindow->UserPort );
    messageClass = message->Class;
    GT_ReplyIMsg( message );

    switch ( messageClass ) {
      case IDCMP_GADGETUP: {

        struct Gadget * gadget = message->IAddress;
        if ( !gadget ) {

          LOG_V(( "V: Gadget to handle is NULL\n" ));
          break;
        }
        switch ( gadget->GadgetID ) {
          case GADGET_TEST_BUTTON: {
            LOG_V(( "V: Handling test button\n" ));
            CreateTestUi( base );
            break;
          }
          case GADGET_CANCEL_BUTTON: {
            LOG_V(( "V: Handling cancel button\n" ));
            return;
          }
          case GADGET_SELECT_BUTTON:
          case GADGET_SAVE_BUTTON:
          case GADGET_USE_BUTTON:
          default: {

            LOG_V(( "V: GadgetID %ld not handled\n", gadget->GadgetID ));
            break;
          }
        }
        break;
      }
      case IDCMP_CLOSEWINDOW: {

        LOG_V(( "V: Handling window close event\n" ));
        return;
      }
      default: {

        LOG_V(( "V: Message class %ld not handled\n", messageClass ));
        break;
      }
    }
  }
}
