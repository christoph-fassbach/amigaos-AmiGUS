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

#include <intuition/intuitionbase.h>
#include <reaction/reaction_macros.h>
#include <gadgets/getfile.h>

#include <proto/alib.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/getfile.h>
#include <proto/label.h>
#include <proto/layout.h>
#include <proto/listbrowser.h>
#include <proto/window.h>

#include "converter.h"

#include "amigus_utils.h"
#include "camd_utils.h"
#include "debug.h"
#include "errors.h"
#include "progress_dialog.h"
#include "sf_listnodes.h"
#include "sf2_optimizer.h"
#include "sf2_reader.h"
#include "sf2_tools.h"
#include "support.h"

/* Globals defined somewhere - here ;) */
struct SF_Converter      * SF_Converter_Base;   // Main app struct
// System libraries:
struct Library           * CamdBase;
struct IntuitionBase     * IntuitionBase;
struct UtilityBase       * UtilityBase;
struct Library           * MathIeeeDoubBasBase;
struct Library           * MathIeeeDoubTransBase;
// and some more owned by the linker libraries:
// struct DosLibrary     * DOSBase;
// struct ExecBase       * SysBase;
// And for ReAction:
struct ClassLibrary      * ButtonBase;
struct ClassLibrary      * FuelGaugeBase;
struct ClassLibrary      * GetFileBase;
struct ClassLibrary      * LabelBase;
struct ClassLibrary      * LayoutBase;
struct ClassLibrary      * ListBrowserBase;
struct ClassLibrary      * WindowBase;

/* Locals also defined here! */

enum GadgetIds {

  GadgetId_Start = 100,
  GadgetId_GetInputFile,
  GadgetId_GetOutputFile,
  GadgetId_ReadButton,
  GadgetId_WriteButton,
  GadgetId_Instruments,
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

STRPTR RequestFileName( struct Window * window, struct Gadget * gadget ) {

  STRPTR path = NULL;
  gfRequestFile(( Object * ) gadget, window );
  GetAttr( GETFILE_File, gadget, ( ULONG * ) &( path ));
  if (( !( path )) || ( 0 >= C_strlen( path ))) {

    LOG_D(( "D: No file selected\n" ));
    return NULL;
  }
  GetAttr( GETFILE_FullFile, gadget, ( ULONG * ) &( path ));
  LOG_D(( "D: Selected file %s \n", path ));
  return path;
}

ULONG Startup( VOID ) {

  struct LoadSoundFontMessage * message;

  if ( !SF_Converter_Base ) {

    SF_Converter_Base = AllocMem( sizeof( struct SF_Converter ), MEMF_ANY | MEMF_CLEAR );
  }
  if ( !SF_Converter_Base ) {

    DisplayError( EAllocateAmiGUSCAMDToolBase );
    return EAllocateAmiGUSCAMDToolBase;
  }

  // TODO: error handling of all the below!
  OpenLib(( struct Library ** )&CamdBase, "camd.library", 37, EOpenCamdBase );
  OpenLib(( struct Library ** )&IntuitionBase, "intuition.library", 36, EOpenIntuitionBase );
  OpenLib(( struct Library ** )&MathIeeeDoubBasBase, "mathieeedoubbas.library", 36, EOpenDoubBasBase );
  OpenLib(( struct Library ** )&MathIeeeDoubTransBase, "mathieeedoubtrans.library", 36, EOpenDoubTransBase );
  OpenLib(( struct Library ** )&UtilityBase, "utility.library", 36, EOpenUtilityBase );

  OpenLib(( struct Library ** )&ButtonBase, "gadgets/button.gadget", 0, EOpenButtonBase );
  OpenLib(( struct Library ** )&FuelGaugeBase, "gadgets/fuelgauge.gadget", 0, EOpenFuelGaugeBase );
  OpenLib(( struct Library ** )&GetFileBase, "gadgets/getfile.gadget", 0, EOpenGetFileBase );
  OpenLib(( struct Library ** )&LabelBase, "images/label.image", 0, EOpenLabelBase );
  OpenLib(( struct Library ** )&LayoutBase, "gadgets/layout.gadget", 0, EOpenLayoutBase );
  OpenLib(( struct Library ** )&ListBrowserBase, "gadgets/listbrowser.gadget", 0, EOpenListBrowserBase );
  OpenLib(( struct Library ** )&WindowBase, "window.class", 0, EOpenWindowBase );

  NewList( &( SF_Converter_Base->sfc_InstrumentLabels ));
  CreateEmptyListLabels( &SF_Converter_Base->sfc_InstrumentLabels );

  SF_Converter_Base->sfc_MainProcess = ( struct Process * ) FindTask( NULL );

  SF_Converter_Base->sfc_MidiReplyPort = CreateMsgPort();

  message = CreateAmigusLoadSoundFontMessage(
    SF_Converter_Base->sfc_MidiReplyPort );
  SendAmigusMessage(( struct Message *) message );

  LOG_I(( "I: " STR( APP_NAME ) " startup complete.\n" ));

  return ENoError;
}

VOID Cleanup( VOID ) {

  struct SF_Converter * base = SF_Converter_Base;

  FreeSf2( base->sfc_Sf2 );
  DeleteMsgPort( base->sfc_MidiReplyPort );

  CloseMidiInOutput( &( base->sfc_MidiLink ));
  CloseMidi( &( base->sfc_MidiNode ));

  FreeListLabels( &( base->sfc_InstrumentLabels ));

  CloseLib(( struct Library ** )&WindowBase );
  CloseLib(( struct Library ** )&ListBrowserBase );
  CloseLib(( struct Library ** )&LayoutBase );
  CloseLib(( struct Library ** )&LabelBase );
  CloseLib(( struct Library ** )&GetFileBase );
  CloseLib(( struct Library ** )&FuelGaugeBase );
  CloseLib(( struct Library ** )&ButtonBase );

  CloseLib(( struct Library ** )&UtilityBase );
  CloseLib(( struct Library ** )&MathIeeeDoubTransBase );
  CloseLib(( struct Library ** )&MathIeeeDoubBasBase );
  CloseLib(( struct Library ** )&IntuitionBase );
  CloseLib(( struct Library ** )&CamdBase );
  LOG_I(( "I: " STR( APP_NAME ) " cleanup starting.\n" ));
  if ( base ) {

    if ( base->sfc_LogFile ) {

      LOG_I(( "I: Attempting SF_Converter_Base->sfc_LogFile.\n" ));
      Close( base->sfc_LogFile );
    }
    /* Free'ing agt_LogMem deliberately not happening here! */

    LOG_I(( "I: Attempting SF_Converter_Base.\n" ));
    FreeMem( base, sizeof( struct SF_Converter ));
    SF_Converter_Base = NULL;
  }
  // No logging after this anymore!
}

VOID OpenWin( VOID ) { // TODO: enable error handling and return values

  struct SF_Converter * base = SF_Converter_Base;

  base->sfc_Screen = LockPubScreen( NULL );

  if ( !base->sfc_Screen ) {

    return;
  }

  base->sfc_ListBrowser =
    ListBrowserObject,
      LISTBROWSER_Labels, &( base->sfc_InstrumentLabels ),
      LISTBROWSER_ColumnInfo, GetSoundFontColumnInfos(),
      LISTBROWSER_Selected, 0,
      LISTBROWSER_ColumnTitles, TRUE,
      LISTBROWSER_ShowSelected, TRUE,
      LISTBROWSER_Editable, FALSE,
      LISTBROWSER_Hierarchical, FALSE,
      LISTBROWSER_MultiSelect, FALSE,
      LISTBROWSER_VirtualWidth, GetSoundFontColumnsWidth(),
      LISTBROWSER_HorizontalProp, TRUE,
      GA_Text, "Instruments",
      GA_ID, GadgetId_Instruments,
      GA_RelVerify, TRUE,
    ListBrowserEnd;

  base->sfc_MainWindowContent = WindowObject,
    WA_PubScreen, base->sfc_Screen,
    WA_ScreenTitle, APP_IDSTRING,
    WA_Title, "SoundFontConverter",
    WA_Activate, TRUE,
    WA_DepthGadget, TRUE,
    WA_DragBar, TRUE,
    WA_CloseGadget, TRUE,
    WA_SizeGadget, TRUE,
    WA_Width, 620,
    WA_Height, 200,
    WA_SmartRefresh, TRUE,
    WA_Flags, WFLG_CLOSEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_SIZEBBOTTOM | WFLG_SIZEGADGET | WFLG_ACTIVATE,
    WA_IDCMP, IDCMP_VANILLAKEY | IDCMP_NEWSIZE,
    WINDOW_Position, WPOS_TOPLEFT,
    WINDOW_IconifyGadget, FALSE, // TODO: TRUE,

    WINDOW_ParentGroup, VLayoutObject,

      LAYOUT_AddChild, HLayoutObject,
        LAYOUT_VertAlignment, LALIGN_CENTER,

        LAYOUT_AddChild, base->sfc_InputGetFile = GetFileObject,
          GA_ID, GadgetId_GetInputFile,
          GA_RelVerify, TRUE,
          GETFILE_TitleText, "Select a .sf2 / .AmiSF file",
          GETFILE_ReadOnly, TRUE,
          GETFILE_DoSaveMode, FALSE,
          GETFILE_DoPatterns, TRUE,
          GETFILE_RejectIcons, TRUE,
          GETFILE_FullFileExpand, TRUE,
          GETFILE_Pattern, "#?.(sf2|AmiSF)",
        GetFileEnd,
        CHILD_WeightedHeight, 0,
        CHILD_Label, LabelObject,
          LABEL_Text ,"Source: ",
        LabelEnd,

        LAYOUT_AddChild, base->sfc_ReadButton = ButtonObject,
          GA_Text, " Read  ",
          GA_ID, GadgetId_ReadButton,
          GA_RelVerify, TRUE,
          GA_Disabled, TRUE,
        LayoutEnd,
        CHILD_WeightedHeight, 0,
        CHILD_WeightedWidth, 0,

      LayoutEnd,
      CHILD_WeightedHeight, 0,

      LAYOUT_AddImage, LabelObject,
        LABEL_Text, "Instrument definitions:",
        LABEL_Justification, LABEL_CENTER,
      LabelEnd,
      LAYOUT_AddChild, base->sfc_ListBrowser,
      CHILD_WeightedHeight, 1000,

      LAYOUT_AddChild, HLayoutObject,
        LAYOUT_VertAlignment, LALIGN_CENTER,

        LAYOUT_AddChild, base->sfc_OutputGetFile = GetFileObject,
          GA_ID, GadgetId_GetOutputFile,
          GA_RelVerify, TRUE,
          GETFILE_TitleText, "Select target .AmiSF file",
          GETFILE_ReadOnly, FALSE,
          GETFILE_DoSaveMode, TRUE,
          GETFILE_DoPatterns, TRUE,
          GETFILE_RejectIcons, TRUE,
          GETFILE_FullFileExpand, TRUE,
          GETFILE_Pattern, "#?.AmiSF",
        GetFileEnd,
        CHILD_WeightedHeight, 0,
        CHILD_Label, LabelObject,
          LABEL_Text ,"Target: ",
        LabelEnd,

        LAYOUT_AddChild, base->sfc_WriteButton = ButtonObject,
          GA_Text, " Write ",
          GA_ID, GadgetId_WriteButton,
          GA_RelVerify, TRUE,
          GA_Disabled, TRUE,
        LayoutEnd,
        CHILD_WeightedHeight, 0,
        CHILD_WeightedWidth, 0,
      LayoutEnd,
      CHILD_WeightedHeight, 0,
    LayoutEnd,
  EndWindow;

  if ( !base->sfc_MainWindowContent ) {
    return;
  }

  // On creation by window open, clavier needs to tell scroller the size!

  base->sfc_MainWindow = ( struct Window * )
    RA_OpenWindow( base->sfc_MainWindowContent );

  if ( !base->sfc_MainWindow ) {
    return;
  }

  GetAttr( WINDOW_SigMask,
           base->sfc_MainWindowContent, 
           &( base->sfc_MainWindowSignal ));

  return;
}

VOID HandleReadButton( VOID ) {

  struct SF_Converter * base = SF_Converter_Base;
  BOOL abort = FALSE;
  ULONG currentProgress = 0;
  ULONG maxProgress = 100;
  
  base->sfc_ProgressDialog =
    CreateProgressDialog( base->sfc_MainWindow,
                          "Please wait...",
                          "Loading and parsing SoundFont ...",
                          "Cancel" );
  ShowProgressDialog( base->sfc_ProgressDialog );

  LOG_D(( "D: Reading %s.\n", base->sfc_SourceFileName ));

  if ( !( abort )) {

    FreeSf2( base->sfc_Sf2 );
    abort = HandleProgressDialogTick( base->sfc_ProgressDialog,
                                      1,
                                      maxProgress );
  }
  if ( !( abort )) {

    FreeListLabels( &( base->sfc_InstrumentLabels ));
    abort = HandleProgressDialogTick( base->sfc_ProgressDialog,
                                      2,
                                      maxProgress );
  }
  if ( !( abort )) {

    base->sfc_Sf2 = AllocSf2FromFile( base->sfc_SourceFileName );    
    maxProgress = base->sfc_Sf2->sf2_PresetCount; // CreateSf2ListLabels
    abort = HandleProgressDialogTick( base->sfc_ProgressDialog,
                                      5,
                                      maxProgress );
    LOG_D(( "D: SF2 will be at 0x%08lx\n", base->sfc_Sf2 ));
  }
  if ( !( abort )) {

    abort = OptimizeSF2( base->sfc_Sf2,
                         base->sfc_ProgressDialog,
                         &currentProgress,
                         &maxProgress );
  }
  if ( !( abort )) {

    abort = CreateSf2ListLabels( &( base->sfc_InstrumentLabels ),
                                 base->sfc_Sf2,
                                 base->sfc_ProgressDialog,
                                 &currentProgress,
                                 maxProgress );
  }

  if ( abort ) {

    LOG_D(( "D: Handling cancelled load.\n" ));
    FreeListLabels( &( base->sfc_InstrumentLabels ));
  }
  SetGadgetAttrs( base->sfc_ListBrowser,
                  base->sfc_MainWindow,
                  NULL,
                  LISTBROWSER_Labels, &( base->sfc_InstrumentLabels ),
                  TAG_DONE );

  CloseProgressDialog( base->sfc_ProgressDialog );
  FreeProgressDialog( base->sfc_ProgressDialog );
  base->sfc_ProgressDialog = NULL;
}

VOID HandleListElement( ULONG index ) {

  struct SF_Converter * base = SF_Converter_Base;
  struct SF2 * sf2 = base->sfc_Sf2;
  struct SF2_Preset * sf2Preset;
  struct SF2_Instrument * sf2Instrument;
  struct SF2_Sample * sf2Sample;
  struct PlaySampleMessage * message;
  struct AmiSF_Note * note;
  struct AmiSF_Sample * sample;
  APTR data;

  if ( !( sf2 )) {

    LOG_I(( "I: List element %ld pushed, but no SoundFont loaded!\n", index ));
    return;
  }
  if ( !( GetSf2InformationForIndex( &sf2Preset,
                                     &sf2Instrument,
                                     &sf2Sample,
                                     sf2,
                                     index ))) {

    LOG_E(( "E: Could not get information for index %ld\n", index ));
    return;
  }

  LOG_D(( "V: Playing SF2 sample %s for index %ld, "
          "overall start %ld = 0, end %ld, "
          "loop start %ld, end %ld\n",
          sf2Sample->sf2s_Name, index,
          sf2Sample->sf2s_SampleStartOffset,
          sf2Sample->sf2s_SampleEndOffset - sf2Sample->sf2s_SampleStartOffset,
          sf2Sample->sf2s_LoopStartOffset - sf2Sample->sf2s_SampleStartOffset,
          sf2Sample->sf2s_LoopEndOffset - sf2Sample->sf2s_SampleStartOffset ));
  note = CreateAmiSF_Note( sf2Preset,
                           sf2Instrument,
                           sf2Sample,
                           sf2Sample->sf2s_SampleNote );
  sample = CreateAmiSF_Sample( sf2Preset,
                               sf2Instrument,
                               sf2Sample,
                               0 );
  LOG_D(( "V: Playing AmiSF start %ld loop %ld end %ld rate 0x%08lx\n",
          sample->amisfs_StartOffset,
          sample->amisfs_LoopOffset - sample->amisfs_StartOffset,
          sample->amisfs_EndOffset - sample->amisfs_StartOffset,
          note->amisfn_PlaybackRate ));
  data = GetSF2SampleData( sf2, sf2Sample );
  message = CreateAmigusPlaySampleMessage(
    SF_Converter_Base->sfc_MidiReplyPort,
    note,
    sample,
    data );
  SendAmigusMessage(( struct Message * ) message );
  LOG_D(( "V: Got note at 0x%08lx\n", note ));
}

VOID HandleEvents( VOID ) {

  BOOL stop = FALSE;
  struct SF_Converter * base = SF_Converter_Base;

  const ULONG windowSignal = base->sfc_MainWindowSignal;
  const ULONG replyPortSignal =
    1 << SF_Converter_Base->sfc_MidiReplyPort->mp_SigBit;

  while ( !( stop )) {

    ULONG signals = Wait( windowSignal
                          | replyPortSignal
                          | SIGBREAKF_CTRL_C );
    if ( SIGBREAKF_CTRL_C & signals) {

      stop = TRUE;
    }

    while ( replyPortSignal & signals ) {

      struct Message * message = GetMsg( SF_Converter_Base->sfc_MidiReplyPort );
      if ( !message ) {

        break;
      }
      Remove(( struct Node * ) message );
      DeleteAmigusMessage( message );
    }
    while ( windowSignal & signals ) {

      WORD windowMessageCode;
      ULONG windowMessage = DoMethod( base->sfc_MainWindowContent,
                                      WM_HANDLEINPUT, 
                                      &windowMessageCode );

      if ( WMHI_LASTMSG == windowMessage ) {

        // All messages handled => break for-loop!
        break;
      }

      switch ( WMHI_CLASSMASK & windowMessage ) {
        case WMHI_GADGETUP: {
          switch ( WMHI_GADGETMASK & windowMessage ) {
            case GadgetId_GetInputFile: {

              BOOL disabled;
              base->sfc_SourceFileName = 
                RequestFileName( base->sfc_MainWindow,
                                 base->sfc_InputGetFile );
              disabled = ( NULL == base->sfc_SourceFileName );
              SetGadgetAttrs( base->sfc_ReadButton,
                              base->sfc_MainWindow,
                              NULL,
                              GA_Disabled, disabled,
                              TAG_END );
              break;
            }
            case GadgetId_GetOutputFile: {

              BOOL disabled;
              base->sfc_TargetFileName = 
                RequestFileName( base->sfc_MainWindow,
                                 base->sfc_OutputGetFile );
              disabled = ( NULL == base->sfc_TargetFileName );
              SetGadgetAttrs( base->sfc_WriteButton,
                              base->sfc_MainWindow,
                              NULL,
                              GA_Disabled, disabled,
                              TAG_END );
              break;
            }
            case GadgetId_ReadButton: {

              HandleReadButton();
              break;
            }
            case GadgetId_WriteButton: {

              LOG_D(( "D: Writing %s.\n", base->sfc_TargetFileName ));
              break;
            }
            case GadgetId_Instruments: {

              HandleListElement( windowMessageCode );
              break;
            }
            default: {
              LOG_I(( "I: Unknown Gadget\n" ));
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
          LOG_I(( "I: WMHI_NEWSIZE\n"));
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

  if ( !SF_Converter_Base ) {

    return;
  }
  if ( SF_Converter_Base->sfc_MainWindowContent ) {

    DisposeObject( SF_Converter_Base->sfc_MainWindowContent );
    SF_Converter_Base->sfc_MainWindowContent = NULL;
  }
  if ( SF_Converter_Base->sfc_Screen ) {

    UnlockPubScreen( NULL, SF_Converter_Base->sfc_Screen );
    SF_Converter_Base->sfc_Screen = NULL;
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
