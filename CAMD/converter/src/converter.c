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
#include <libraries/gadtools.h>
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
#include "debug.h"
#include "errors.h"
#include "support.h"

/* Globals defined somewhere - here ;) */
struct SF_Converter      * SF_Converter_Base;   // Main app struct
// System libraries:
struct IntuitionBase     * IntuitionBase;
struct UtilityBase       * UtilityBase;
// and some more owned by the linker libraries:
// struct DosLibrary     * DOSBase;
// struct ExecBase       * SysBase;
// And for ReAction:
struct ClassLibrary      * ButtonBase;
struct ClassLibrary      * GetFileBase;
struct ClassLibrary      * LabelBase;
struct ClassLibrary      * LayoutBase;
struct ClassLibrary      * ListBrowserBase;
struct ClassLibrary      * WindowBase;

/* Locals also defined here! */

static const struct ColumnInfo instrumentColumns[] = {

  { 30, " # ", CIF_CENTER },
  { 190, "Name", CIF_CENTER },
  { -1, (STRPTR)~0, -1 }
};

static const UBYTE * instrumentNames[] = {
  "  0", "Acoustic Grand Piano",
  "  1", "Bright Acoustic Piano",
  "  2", "Electric Grand Piano",
  "  3", "Honky-tonk Piano",
  "  4", "Electric Piano 1",
  "  5", "Electric Piano 2",
  "  6", "Harpsichord",
  "  7", "Clavinet",
  "  8", "Celesta",
  "  9", "Glockenspiel",
  " 10", "Music Box",
  " 11", "Vibraphone",
  " 12", "Marimba",
  " 13", "Xylophone",
  " 14", "Tubular Bells",
  " 15", "Dulcimer",
  " 16", "Drawbar Organ",
  " 17", "Percussive Organ",
  " 18", "Rock Organ",
  " 29", "Church Organ",
  " 20", "Reed Organ",
  " 21", "Accordion",
  " 22", "Harmonica",
  " 23", "Bandoneon",
  " 24", "Acoustic Guitar (nylon)",
  " 25", "Acoustic Guitar (steel)",
  " 26", "Electric Guitar (jazz)",
  " 27", "Electric Guitar",
  " 28", "Electric Guitar (muted)",
  " 29", "Overdriven Guitar",
  " 30", "Distortion Guitar",
  " 31", "Guitar Harmonics",
  " 32", "Acoustic Bass",
  " 33", "Electric Bass (finger)",
  " 34", "Electric Bass (picked)",
  " 35", "Fretless Bass",
  " 36", "Slap Bass 1",
  " 37", "Slap Bass 2",
  " 38", "Synth Bass 1",
  " 39", "Synth Bass 2",
  " 40", "Violin",
  " 41", "Viola",
  " 42", "Cello",
  " 43", "Contrabass",
  " 44", "Tremolo Strings",
  " 45", "Pizzicato Strings",
  " 46", "Orchestral Harp",
  " 47", "Timpani",
  " 48", "String Ensemble 1",
  " 49", "String Ensemble 2",
  " 50", "Synth Strings 1",
  " 51", "Synth Strings 2",
  " 52", "Choir Aahs",
  " 53", "Voice Oohs",
  " 54", "Synth Voice",
  " 55", "Orchestra Hit",
  " 56", "Trumpet",
  " 57", "Trombone",
  " 58", "Tuba",
  " 59", "Muted Trumpet",
  " 60", "French Horn",
  " 61", "Brass Section",
  " 62", "Synth Brass 1",
  " 63", "Synth Brass 2",
  " 64", "Soprano Sax",
  " 65", "Alto Sax",
  " 66", "Tenor Sax",
  " 67", "Baritone Sax",
  " 68", "Oboe",
  " 69", "English Horn",
  " 70", "Bassoon",
  " 71", "Clarinet",
  " 72", "Piccolo",
  " 73", "Flute",
  " 74", "Recorder",
  " 75", "Pan Flute",
  " 76", "Blown bottle",
  " 77", "Shakuhachi",
  " 78", "Whistle",
  " 79", "Ocarina",
  " 80", "Lead 1",
  " 81", "Lead 2",
  " 82", "Lead 3",
  " 83", "Lead 4",
  " 84", "Lead 5",
  " 85", "Lead 6",
  " 86", "Lead 7",
  " 87", "Lead 8",
  " 88", "Pad 1",
  " 89", "Pad 2",
  " 90", "Pad 3",
  " 91", "Pad 4",
  " 92", "Pad 5",
  " 93", "Pad 6",
  " 94", "Pad 7",
  " 95", "Pad 8",
  " 96", "FX 1",
  " 97", "FX 2",
  " 98", "FX 3",
  " 99", "FX 4",
  "100", "FX 5",
  "101", "FX 6",
  "102", "FX 7",
  "103", "FX 8",
  "104", "Sitar",
  "105", "Banjo",
  "106", "Shamisen",
  "107", "Koto",
  "108", "Kalimba",
  "109", "Bag pipe",
  "110", "Fiddle",
  "111", "Shanai",
  "112", "Tinkle Bell",
  "113", "Cowbell",
  "114", "Steel Drums",
  "115", "Woodblock",
  "116", "Taiko Drum",
  "117", "Melodic Tom",
  "118", "Synth Drum",
  "119", "Reverse Cymbal",
  "120 ", "Guitar Fret Noise",
  "121 ", "Breath Noise",
  "122 ", "Seashore",
  "123 ", "Bird Tweet",
  "124 ", "Telephone Ring",
  "125 ", "Helicopter",
  "126 ", "Applause",
  "127 ", "Gunshot",
  NULL
};

static const UBYTE * percussionNames[] = {
  "B-1 ", "Ac./Low Bass Drum",
  "C-2 ", "Elec./High Bass Drum",
  "C#2 ", "Side Stick",
  "D-2 ", "Acoustic Snare",
  "D#2 ", "Hand Clap",
  "E-2 ", "Elec. Snare/Rimshot",
  "F-2 ", "Low Floor Tom",
  "F#2 ", "Closed Hi-hat",
  "G-2 ", "High Floor Tom",
  "G#2 ", "Pedal Hi-hat",
  "A-2 ", "Low Tom",
  "A#2 ", "Open Hi-hat",
  "B-2 ", "Low-Mid Tom",
  "C-3 ", "High-Mid Tom",
  "C#3 ", "Crash Cymbal 1",
  "D-3 ", "High Tom",
  "D#3 ", "Ride Cymbal 1",
  "E-3 ", "Chinese Cymbal",
  "F-3 ", "Ride Bell",
  "F#3 ", "Tambourine",
  "G-3 ", "Splash Cymbal",
  "G#3 ", "Cowbell",
  "A-3 ", "Crash Cymbal 2",
  "A#3 ", "Vibraslap",
  "B-3 ", "Ride Cymbal 2",
  "C-4 ", "High Bongo",
  "C#4 ", "Low Bongo",
  "D-4 ", "Mute High Conga",
  "D#4 ", "Open High Conga",
  "E-4 ", "Low Conga",
  "F-4 ", "High Timbale",
  "F#4 ", "Low Timbale",
  "G-4 ", "High Agogo",
  "G#4 ", "Low Agogo",
  "A-4 ", "Cabasa",
  "A#4 ", "Maracas",
  "B-4 ", "Short Whistle",
  "C-5 ", "Long Whistle",
  "C#5 ", "Short Guiro",
  "D-5 ", "Long Guiro",
  "D#5 ", "Claves",
  "E-5 ", "High Woodblock",
  "F-5 ", "Low Woodblock",
  "F#5 ", "Mute Cuica",
  "G-5 ", "Open Cuica",
  "G#5 ", "Mute Triangle",
  "A-5 ", "Open Triangle",
  NULL
};

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
VOID CreateListLabels( struct List * labels, const UBYTE ** strings ) {

  LONG i = 0;

  while ( NULL != strings[ i ] ) {

    struct Node * label = AllocListBrowserNode(
      sizeof( instrumentColumns ) / sizeof( struct ColumnInfo ),
      LBNA_Column, 0,
        LBNCA_CopyText, TRUE,
        LBNCA_Text, strings[ i++ ],
        LBNCA_Justification, LCJ_RIGHT,
      LBNA_Column, 1,
        LBNCA_CopyText, TRUE,
        LBNCA_Text, strings[ i++ ],
      TAG_DONE
    );
    AddTail( labels, label );
  }
}

VOID FreeListLabels( struct List * list ) {

  struct Node * label;
  FOR_LIST( list,
            label,
            struct Node * ) {

    FreeListBrowserNode( label );
  }
  LOG_D(( "V: List labels emptied.\n" ));
}

ULONG Startup( VOID ) {

  LONG result;
  if ( !SF_Converter_Base ) {

    SF_Converter_Base = AllocMem( sizeof( struct SF_Converter ), MEMF_ANY | MEMF_CLEAR );
  }
  if ( !SF_Converter_Base ) {

    DisplayError( EAllocateAmiGUSCAMDToolBase );
    return EAllocateAmiGUSCAMDToolBase;
  }

  // TODO: error handling of all the below!
  OpenLib(( struct Library ** )&IntuitionBase, "intuition.library", 36, EOpenIntuitionBase );
  OpenLib(( struct Library ** )&UtilityBase, "utility.library", 36, EOpenUtilityBase );

  OpenLib(( struct Library ** )&ButtonBase, "gadgets/button.gadget", 0, EOpenButtonBase );
  OpenLib(( struct Library ** )&GetFileBase, "gadgets/getfile.gadget", 0, EOpenGetFileBase );
  OpenLib(( struct Library ** )&LabelBase, "images/label.image", 0, EOpenLabelBase );
  OpenLib(( struct Library ** )&LayoutBase, "gadgets/layout.gadget", 0, EOpenLayoutBase );
  OpenLib(( struct Library ** )&ListBrowserBase, "gadgets/listbrowser.gadget", 0, EOpenListBrowserBase );
  OpenLib(( struct Library ** )&WindowBase, "window.class", 0, EOpenWindowBase );

  NEW_LIST( &( SF_Converter_Base->sfc_InstrumentLabels ));
  CreateListLabels( &SF_Converter_Base->sfc_InstrumentLabels, instrumentNames );

  SF_Converter_Base->sfc_MainProcess = ( struct Process * ) FindTask( NULL );

  LOG_I(( "I: " STR( APP_NAME ) " startup complete.\n" ));
}

VOID Cleanup( VOID ) {

  FreeListLabels( &( SF_Converter_Base->sfc_InstrumentLabels ));

  CloseLib(( struct Library ** )&WindowBase );
  CloseLib(( struct Library ** )&ListBrowserBase );
  CloseLib(( struct Library ** )&LayoutBase );
  CloseLib(( struct Library ** )&LabelBase );
  CloseLib(( struct Library ** )&GetFileBase );
  CloseLib(( struct Library ** )&ButtonBase );

  CloseLib(( struct Library ** )&UtilityBase );
  CloseLib(( struct Library ** )&IntuitionBase );
  LOG_I(( "I: " STR( APP_NAME ) " cleanup starting.\n" ));
  if ( SF_Converter_Base ) {

    if ( SF_Converter_Base->sfc_LogFile ) {

      LOG_I(( "I: Attempting SF_Converter_Base->sfc_LogFile.\n" ));
      Close( SF_Converter_Base->sfc_LogFile );
    }
    /* Free'ing agt_LogMem deliberately not happening here! */

    LOG_I(( "I: Attempting SF_Converter_Base.\n" ));
    FreeMem( SF_Converter_Base, sizeof( struct SF_Converter ));
    SF_Converter_Base = NULL;
  }
  // No logging after this anymore!
}

VOID OpenWin( VOID ) { // TODO: enable error handling and return values

  SF_Converter_Base->sfc_Screen = LockPubScreen( NULL );

  if ( !SF_Converter_Base->sfc_Screen ) {

    return;
  }

  SF_Converter_Base->sfc_ListBrowser =
    ListBrowserObject,
      LISTBROWSER_Labels, &( SF_Converter_Base->sfc_InstrumentLabels ),
      LISTBROWSER_ColumnInfo, instrumentColumns,
      LISTBROWSER_Selected, 0,
      LISTBROWSER_ColumnTitles, TRUE,
      LISTBROWSER_ShowSelected, TRUE,
      LISTBROWSER_Editable, FALSE,
      LISTBROWSER_Hierarchical, FALSE,
      LISTBROWSER_MultiSelect, FALSE,
      LISTBROWSER_VirtualWidth, 220,
      LISTBROWSER_HorizontalProp, TRUE,
      GA_Text, "Instruments",
      GA_ID, GadgetId_Instruments,
      GA_RelVerify, TRUE,
    ListBrowserEnd;

  SF_Converter_Base->sfc_MainWindowContent = WindowObject,
    WA_PubScreen, SF_Converter_Base->sfc_Screen,
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

        LAYOUT_AddChild, SF_Converter_Base->sfc_InputGetFile = GetFileObject,
          GA_ID, GadgetId_GetInputFile,
          GA_RelVerify, TRUE,
          GETFILE_TitleText, "Select a .sf2 / .AmiSF file",
          GETFILE_ReadOnly, TRUE,
          GETFILE_DoSaveMode, FALSE,
          GETFILE_DoPatterns, TRUE,
          GETFILE_RejectIcons, TRUE,
          GETFILE_Pattern, "#?.(sf2|AmiSF)",
        GetFileEnd,
        CHILD_WeightedHeight, 0,
        CHILD_Label, LabelObject,
          LABEL_Text ,"Source: ",
        LabelEnd,

        LAYOUT_AddChild, ButtonObject,
          GA_Text, " Read  ",
          GA_ID, GadgetId_ReadButton,
          GA_RelVerify, TRUE,
        LayoutEnd,
        CHILD_WeightedHeight, 0,
        CHILD_WeightedWidth, 0,

      LayoutEnd,
      CHILD_WeightedHeight, 0,

      LAYOUT_AddImage, LabelObject,
        LABEL_Text, "Instrument definitions:",
        LABEL_Justification, LABEL_CENTER,
      LabelEnd,
      LAYOUT_AddChild, SF_Converter_Base->sfc_ListBrowser,
      CHILD_WeightedHeight, 1000,

      LAYOUT_AddChild, HLayoutObject,
        LAYOUT_VertAlignment, LALIGN_CENTER,

        LAYOUT_AddChild, SF_Converter_Base->sfc_OutputGetFile = GetFileObject,
          GA_ID, GadgetId_GetOutputFile,
          GA_RelVerify, TRUE,
          GETFILE_TitleText, "Select target .AmiSF file",
          GETFILE_ReadOnly, FALSE,
          GETFILE_DoSaveMode, TRUE,
          GETFILE_DoPatterns, TRUE,
          GETFILE_RejectIcons, TRUE,
          GETFILE_Pattern, "#?.AmiSF",
        GetFileEnd,
        CHILD_WeightedHeight, 0,
        CHILD_Label, LabelObject,
          LABEL_Text ,"Target: ",
        LabelEnd,

        LAYOUT_AddChild, ButtonObject,
          GA_Text, " Write ",
          GA_ID, GadgetId_WriteButton,
          GA_RelVerify, TRUE,
        LayoutEnd,
        CHILD_WeightedHeight, 0,
        CHILD_WeightedWidth, 0,
      LayoutEnd,
      CHILD_WeightedHeight, 0,
    LayoutEnd,
  EndWindow;

  if ( !SF_Converter_Base->sfc_MainWindowContent ) {
    return;
  }

  // On creation by window open, clavier needs to tell scroller the size!

  SF_Converter_Base->sfc_MainWindow = ( struct Window * )
    RA_OpenWindow( SF_Converter_Base->sfc_MainWindowContent );

  if ( !SF_Converter_Base->sfc_MainWindow ) {
    return;
  }

  GetAttr( WINDOW_SigMask,
           SF_Converter_Base->sfc_MainWindowContent, 
           &( SF_Converter_Base->sfc_MainWindowSignal ));

  return;
}

VOID HandleEvents( VOID ) {

  BOOL stop = FALSE;
  struct SF_Converter * base = SF_Converter_Base;

  const ULONG windowSignal = base->sfc_MainWindowSignal;

  while ( !( stop )) {

    ULONG signals = Wait( windowSignal
                          | SIGBREAKF_CTRL_C );
    if ( SIGBREAKF_CTRL_C & signals) {

      stop = TRUE;
    }

    for ( ; ; ) {

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

              gfRequestFile(( Object * ) base->sfc_InputGetFile,
                             base->sfc_MainWindow );
              break;
            }
            case GadgetId_GetOutputFile: {

              gfRequestFile(( Object * ) base->sfc_OutputGetFile,
                             base->sfc_MainWindow );
              break;
            }
            case GadgetId_ReadButton: {

              LOG_D(( "D: Read button pressed.\n" ));
              break;
            }
            case GadgetId_WriteButton: {

              LOG_D(( "D: Write button pressed.\n" ));
              break;
            }
            case GadgetId_Instruments: {

              LOG_D(( "D: List element pressed.\n" ));
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