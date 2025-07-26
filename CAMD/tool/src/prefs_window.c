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

/* Data for gadget structures * /
struct NewGadget Gadgets[ GADGET_COUNT ] = {
  {   5, 15, 270, 13, "AmiGUS CAMD/MIDI sound font file:", &topaz8, 1, PLACETEXT_IN, NULL, NULL },
  { 280, 14,  70, 15, "Select",                            &topaz8, 2, PLACETEXT_IN, NULL, NULL },
  {  10, 20, 230, 100, "Path: Sys:Expansion/AmiGUS/2MB.sf2\n"
                       "Instruments: 127/127 Samples: 300 \n"
                       "Percussions:   81/81 Samples:  81 \n"
                       "Sample size: 1234444/33554432 bytes", &topaz8, 3, PLACETEXT_IN, NULL,    NULL }
};

ULONG GadgetKinds[ GADGET_COUNT ] = {
  TEXT_KIND,
  BUTTON_KIND,
  TEXT_KIND
};


/* Extra information for gadgets using Tags * /
ULONG GadgetTags[] = {
  (GTST_MaxChars), 256, (TAG_DONE),
  (GTNM_Border), TRUE, (TAG_DONE),
  (TAG_DONE)
};
*/

ULONG GadgetKind1 = TEXT_KIND;
struct NewGadget GadgetNew1 = {
  5, 15, 270, 13, "AmiGUS CAMD/MIDI sound font file:", &topaz8, 1, PLACETEXT_IN, NULL, NULL };
ULONG GadgetTags1[] = { (TAG_DONE) };

ULONG GadgetKind2 = BUTTON_KIND;
struct NewGadget GadgetNew2 = {
    280, 14,  70, 15, "Select", &topaz8, 2, PLACETEXT_IN, NULL, NULL };
ULONG GadgetTags2[] = { (TAG_DONE) };

ULONG GadgetKind3 = TEXT_KIND;
struct NewGadget GadgetNew3 = {
  9, 35, 342, 86, NULL, &topaz8, 3, PLACETEXT_IN, NULL, NULL };
ULONG GadgetTags3[] = { GTTX_Border, TRUE, ( TAG_DONE ) };

ULONG GadgetKind4 = TEXT_KIND;
struct NewGadget GadgetNew4 = {
  69, 40, 200, 15, "Path:", &topaz8, 4, PLACETEXT_LEFT, NULL, NULL };
ULONG GadgetTags4[] = { GTTX_Text, ( ULONG ) "Sys:Expansion/AmiGUS/2MB.sf2", ( TAG_DONE ) };

ULONG GadgetKind5 = TEXT_KIND;
struct NewGadget GadgetNew5 = {
  125, 55, 50, 15, "Instruments:", &topaz8, 4, PLACETEXT_LEFT, NULL, NULL };
ULONG GadgetTags5[] = { GTTX_Text, ( ULONG ) "128", ( TAG_DONE ) };

ULONG GadgetKind6 = TEXT_KIND;
struct NewGadget GadgetNew6 = {
  250, 55, 50, 15, "Samples:", &topaz8, 6, PLACETEXT_LEFT, NULL, NULL };
ULONG GadgetTags6[] = { GTTX_Text, ( ULONG ) "1234", ( TAG_DONE ) };

ULONG GadgetKind7 = TEXT_KIND;
struct NewGadget GadgetNew7 = {
  125, 70, 50, 15, "Percussions:", &topaz8, 7, PLACETEXT_LEFT, NULL, NULL };
ULONG GadgetTags7[] = { GTTX_Text, ( ULONG ) "85", ( TAG_DONE ) };

ULONG GadgetKind8 = TEXT_KIND;
struct NewGadget GadgetNew8 = {
  250, 70, 50, 15, "Samples:", &topaz8, 8, PLACETEXT_LEFT, NULL, NULL };
ULONG GadgetTags8[] = { GTTX_Text, ( ULONG ) "19", ( TAG_DONE ) };

ULONG GadgetKind9 = TEXT_KIND;
struct NewGadget GadgetNew9 = {
  125, 85, 50, 15, "Sample size:", &topaz8, 9, PLACETEXT_LEFT, NULL, NULL };
ULONG GadgetTags9[] = { GTTX_Text, ( ULONG ) "33554432", ( TAG_DONE ) };

ULONG GadgetKind10 = TEXT_KIND;
struct NewGadget GadgetNew10 = {
  285, 85, 50, 15, "/ 33554432", &topaz8, 10, PLACETEXT_LEFT, NULL, NULL };
ULONG GadgetTags10[] = { ( TAG_DONE ) };

ULONG GadgetKind11 = TEXT_KIND;
struct NewGadget GadgetNew11 = {
  250, 100, 50, 15, "AmiGUS sample RAM used:", &topaz8, 11, PLACETEXT_LEFT, NULL, NULL };
ULONG GadgetTags11[] = { ( GTTX_Text ), ( ULONG ) "100%", ( TAG_DONE ) };

STRPTR GadgetOptions12[] = { "Upfront", "RAM", "On Demand", NULL };
ULONG GadgetKind12 = CYCLE_KIND;
struct NewGadget GadgetNew12 = {
  208, 125, 110, 15, "Sample loading strategy:", &topaz8, 12, PLACETEXT_LEFT, NULL, NULL };
ULONG GadgetTags12[] = { ( GTCY_Labels ), ( ULONG ) &GadgetOptions12, ( GTCY_Active ), 1, ( TAG_DONE ) };

ULONG GadgetKind13 = CHECKBOX_KIND;
struct NewGadget GadgetNew13 = {
  272, 143, 50, 15, "Show progress bar while loading:", &topaz8, 13, PLACETEXT_LEFT, NULL, NULL };
ULONG GadgetTags13[] = { ( GTCB_Checked ), TRUE, ( TAG_DONE ) };

ULONG GadgetKind14 = BUTTON_KIND;
struct NewGadget GadgetNew14 = {
    9, 160,  70, 15, "Test", &topaz8, 14, PLACETEXT_IN, NULL, NULL };
ULONG GadgetTags14[] = { (TAG_DONE) };

ULONG GadgetKind15 = BUTTON_KIND;
struct NewGadget GadgetNew15 = {
    99, 160,  70, 15, "Save", &topaz8, 15, PLACETEXT_IN, NULL, NULL };
ULONG GadgetTags15[] = { (TAG_DONE) };

ULONG GadgetKind16 = BUTTON_KIND;
struct NewGadget GadgetNew16 = {
    189, 160,  70, 15, "Use", &topaz8, 16, PLACETEXT_IN, NULL, NULL };
ULONG GadgetTags16[] = { (TAG_DONE) };

ULONG GadgetKind17 = BUTTON_KIND;
struct NewGadget GadgetNew17 = {
    279, 160,  70, 15, "Cancel", &topaz8, 17, PLACETEXT_IN, NULL, NULL };
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


#if 0
struct GadgetParams {
  ULONG kind;
  struct NewGadget gadget;
  CONST ULONG * tags;
};

struct GadgetParams Params[] = {
  { 
    TEXT_KIND, 
    { 5, 15, 270, 13, "AmiGUS CAMD/MIDI sound font file:", &topaz8, 1, PLACETEXT_IN, NULL, NULL }, 
    { (TAG_DONE) }
  }, {
    BUTTON_KIND,
    { 280, 14,  70, 15, "Select",                            &topaz8, 2, PLACETEXT_IN, NULL, NULL },
    { (TAG_DONE) }
  }, {
    TEXT_KIND,
    {  10, 20, 230, 100, "Path: Sys:Expansion/AmiGUS/2MB.sf2", &topaz8, 3, PLACETEXT_IN, NULL, NULL },
    { 1, ( TAG_DONE ) }
  },{
    TEXT_KIND,
    {  10, 20, 230, 100, "Path: Sys:Expansion/AmiGUS/2MB.sf2", &topaz8, 3, PLACETEXT_IN, NULL, NULL },
                       /*"Instruments: 127/127 Samples: 300 \n"
                       "Percussions:   81/81 Samples:  81 \n"
                       "Sample size: 1234444/33554432 bytes", &topaz8, 3, PLACETEXT_IN, NULL, NULL },*/
    { (TAG_DONE) }
  }
};
#endif 

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
    /*
  gadget = CreateGadget(
    TEXT_KIND, 
    gadget,
    (struct NewGadget){  5, 15, 270, 13, "AmiGUS CAMD/MIDI sound font file:", &topaz8, 1, PLACETEXT_IN, NULL, NULL },
    ( TAG_DONE )
  );
  */

  // Open window specifying the gadget list:
  base->agt_Window = OpenWindowTags(
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
