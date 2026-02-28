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

#include <graphics/gfxbase.h>
#include <intuition/icclass.h>
#include <intuition/intuitionbase.h>
#include <libraries/gadtools.h>
#include <reaction/reaction_macros.h>

#include <proto/alib.h>
#include <proto/chooser.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/integer.h>
#include <proto/intuition.h>
#include <proto/label.h>
#include <proto/layout.h>
#include <proto/listbrowser.h>
#include <proto/scroller.h>
#include <proto/slider.h>
#include <proto/window.h>

#include "proto/camd.h"

#include "midi/camd.h"
#include "midi/mididefs.h"

#include "camd_keyboard.h"
#include "camd_utils.h"
#include "clavier_gadgetclass.h"
#include "debug.h"
#include "errors.h"
#include "support.h"

#define PERCUSSION_CHANNEL 9

/* Globals defined somewhere - here ;) */
struct CAMD_Keyboard     * CAMD_Keyboard_Base;   // Main app struct
// System libraries:
struct Library           * CamdBase;
struct Library           * DiskfontBase;
struct GfxBase           * GfxBase;
struct IntuitionBase     * IntuitionBase;
struct UtilityBase       * UtilityBase;
// and some more owned by the linker libraries:
// struct ExecBase       * SysBase;
// struct DosLibrary     * DOSBase;
// And for ReAction:
struct ClassLibrary      * ButtonBase;
struct ClassLibrary      * ChooserBase;
struct ClassLibrary      * IntegerBase;
struct ClassLibrary      * LabelBase;
struct ClassLibrary      * LayoutBase;
struct ClassLibrary      * ListBrowserBase;
struct ClassLibrary      * ScrollerBase;
struct ClassLibrary      * SliderBase;
struct ClassLibrary      * WindowBase;

Class                    * ClavierGadgetClass;

/* Locals also defined here! */

static const struct TagItem clavier2scroller[] = {

  { CG_VIRTUAL_WIDTH, SCROLLER_Total },
  { CG_VISUAL_WIDTH, SCROLLER_Visible },
  { GA_ID, 0 }, // Smart: the sender destroys the receivers ID otherwise...
  { TAG_END, 0 }
};

static const struct TagItem scroller2clavier[] = {

  { SCROLLER_Top, CG_OFFSET_X },
  { GA_ID, 0 }, // Smart: the sender destroys the receivers ID otherwise...
  { TAG_END, 0 }
};

static const struct ColumnInfo instrumentColumns[] = {

  { 30, " # ", CIF_CENTER },
  { 190, "Name", CIF_CENTER },
  { -1, (STRPTR)~0, -1 }
};

static const struct ColumnInfo percussionColumns[] = {

  { 70, " Key ", CIF_CENTER },
  { 250, "Name", CIF_CENTER },
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
  GadgetId_DeviceChooser,
  GadgetId_VelocityInteger,
  GadgetId_VelocitySlider,
  GadgetId_ChannelInteger,
  GadgetId_ChannelSlider,
  GadgetId_InfoButton,
  GadgetId_Scroller,
  GadgetId_Clavier,
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

VOID CreateChooserLabels( VOID ) {

  struct CAMD_Device_Node * device;
  FOR_LIST( &( CAMD_Keyboard_Base->ck_Devices ),
            device,
            struct CAMD_Device_Node * ) {

    STRPTR name = device->cdn_Name;
    struct Node * label = AllocChooserNode( CNA_Text, name, TAG_END );
    AddTail( &( CAMD_Keyboard_Base->ck_DeviceLabels ), label );
  }
}

VOID FreeChooserLabels( VOID ) {

  struct Node * node;

  while ( node = RemHead( &( CAMD_Keyboard_Base->ck_DeviceLabels ))) {

    FreeChooserNode( node );
  }
  LOG_D(( "V: Labels is empty = %ld\n",
          IS_EMPTY_LIST( &( CAMD_Keyboard_Base->ck_DeviceLabels ))));
}

VOID CreateLabels( struct List * labels, const UBYTE ** strings ) {

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

VOID FreeInstrumentLabels( VOID ) {

  struct Node * label;
  FOR_LIST( &( CAMD_Keyboard_Base->ck_InstrumentLabels ),
            label,
            struct Node * ) {

    FreeListBrowserNode( label );
  }
} 

LONG OpenMidi( ULONG index ) {

  STRPTR appName = STR( APP_FILE );
  STRPTR linkName = STR( APP_FILE )" Link";

  struct CAMD_Keyboard * base = CAMD_Keyboard_Base;
  struct CAMD_Device_Node * node = ( struct CAMD_Device_Node * )
    NodeAtIndex( &( base->ck_Devices ), index );

  if ( !node ) {

    return ENoMidiNodes;
  }

  // Keyboard only supports playback, hence does not need input buffers.
  base->ck_MidiNode = CreateMidi( MIDI_Name, appName,
                                  // MIDI_MsgQueue, 0L,
                                  // MIDI_SysExSize,0L,
                                  MIDI_ErrFilter, CMEF_All,
                                  TAG_END );
  if ( !( base->ck_MidiNode )) {

    return ECreateMidi;
  }
  LOG_D(( "D: Got output node 0x%08lx for %s.\n",
          base->ck_MidiNode, appName ));

  base->ck_MidiLink = AddMidiLink( base->ck_MidiNode,
                                   MLTYPE_Sender,
                                   MLINK_Comment, linkName,
                                   MLINK_Name, "Out",
                                   //MLINK_Parse, TRUE, // TODO: needed?
                                   MLINK_Location, node->cdn_Location,
                                   TAG_END );
  if ( !( base->ck_MidiLink )) {

    return EAddMidiLink;
  }
  LOG_D(( "D: Got output link 0x%08lx for %s at %s.\n",
          base->ck_MidiLink, linkName, node->cdn_Location ));

  return ENoError;
}

VOID CloseMidi( VOID ) {

  struct CAMD_Keyboard * base = CAMD_Keyboard_Base;
  if ( base->ck_MidiLink ) {

    RemoveMidiLink( base->ck_MidiLink );
    LOG_D(( "D: Closed link 0x%08lx.\n", base->ck_MidiLink ));
    base->ck_MidiLink = NULL;
  }
  if ( base->ck_MidiNode ) {

    DeleteMidi( base->ck_MidiNode );
    LOG_D(( "D: Closed node 0x%08lx.\n", base->ck_MidiNode ));
    base->ck_MidiNode = NULL;
  }
}

ULONG Startup( VOID ) {

  LONG result;
  if ( !CAMD_Keyboard_Base ) {

    CAMD_Keyboard_Base = AllocMem( sizeof( struct CAMD_Keyboard ), MEMF_ANY | MEMF_CLEAR );
  }
  if ( !CAMD_Keyboard_Base ) {

    DisplayError( EAllocateAmiGUSCAMDToolBase );
    return EAllocateAmiGUSCAMDToolBase;
  }

  CAMD_Keyboard_Base->ck_Velocity = 127;

  // TODO: error handling of all the below!
  OpenLib(( struct Library ** )&CamdBase, "camd.library", 37, EOpenCamdBase );
  OpenLib(( struct Library ** )&IntuitionBase, "intuition.library", 36, EOpenIntuitionBase );
  OpenLib(( struct Library ** )&GfxBase, "graphics.library", 36, EOpenGfxBase );
  OpenLib(( struct Library ** )&UtilityBase, "utility.library", 36, EOpenUtilityBase );
  OpenLib(( struct Library ** )&DiskfontBase, "diskfont.library", 36, EOpenDiskfontBase );

  OpenLib(( struct Library ** )&ButtonBase, "gadgets/button.gadget", 0, EOpenButtonBase );
  OpenLib(( struct Library ** )&ChooserBase, "gadgets/chooser.gadget", 0, EOpenChooserBase );
  OpenLib(( struct Library ** )&IntegerBase, "gadgets/integer.gadget", 0, EOpenIntegerBase );
  OpenLib(( struct Library ** )&LabelBase, "images/label.image", 0, EOpenLabelBase );
  OpenLib(( struct Library ** )&LayoutBase, "gadgets/layout.gadget", 0, EOpenLayoutBase );
  OpenLib(( struct Library ** )&ListBrowserBase, "gadgets/listbrowser.gadget", 0, EOpenListBrowserBase );
  OpenLib(( struct Library ** )&ScrollerBase, "gadgets/scroller.gadget", 0, EOpenScrollerBase );
  OpenLib(( struct Library ** )&SliderBase, "gadgets/slider.gadget", 0, EOpenSliderBase );
  OpenLib(( struct Library ** )&WindowBase, "window.class", 0, EOpenWindowBase );

  ClavierGadgetClass = InitClavierGadgetClass();
  if ( !( ClavierGadgetClass )) {

    DisplayError( EInitClavierGadgetClass );
  }

  NEW_LIST( &( CAMD_Keyboard_Base->ck_Devices ));
  NEW_LIST( &( CAMD_Keyboard_Base->ck_DeviceLabels ));

  if ( CamdBase->lib_Version > 37 ) {

    DisplayError( EInvalidCamdVersion );

    return EInvalidCamdVersion;
  }
  result = ExtractCamdOutputDevices( &( CAMD_Keyboard_Base->ck_Devices ));
  if ( result ) {

    CreateChooserLabels();
    result = OpenMidi( 0 );

    if ( result ) {

      DisplayError( result );
    }
  }

  NEW_LIST( &( CAMD_Keyboard_Base->ck_InstrumentLabels ));
  CreateLabels( &CAMD_Keyboard_Base->ck_InstrumentLabels, instrumentNames );
  NEW_LIST( &( CAMD_Keyboard_Base->ck_PercussionLabels ));
  CreateLabels( &CAMD_Keyboard_Base->ck_PercussionLabels, percussionNames );

  LOG_I(( "I: " STR( APP_NAME ) " startup complete.\n" ));
}

VOID Cleanup( VOID ) {

  FreeInstrumentLabels();

  CloseMidi();
  FreeCamdOutputDevices( &( CAMD_Keyboard_Base->ck_Devices ));
  FreeChooserLabels();

  CloseLib(( struct Library ** )&WindowBase );
  CloseLib(( struct Library ** )&ScrollerBase );
  CloseLib(( struct Library ** )&ListBrowserBase );
  CloseLib(( struct Library ** )&LayoutBase );
  CloseLib(( struct Library ** )&LabelBase );
  CloseLib(( struct Library ** )&IntegerBase );
  CloseLib(( struct Library ** )&ChooserBase );
  CloseLib(( struct Library ** )&ButtonBase );

  CloseLib(( struct Library ** )&DiskfontBase );
  CloseLib(( struct Library ** )&UtilityBase );
  CloseLib(( struct Library ** )&GfxBase );
  CloseLib(( struct Library ** )&IntuitionBase );
  CloseLib(( struct Library ** )&CamdBase );
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

  ULONG offsetX;
  ULONG visualWidth;
  ULONG virtualWidth;
  APTR velocityGadget;
  APTR channelGadget;

  CAMD_Keyboard_Base->ck_Screen = LockPubScreen( NULL );

  if ( !CAMD_Keyboard_Base->ck_Screen ) {

    return;
  }

  if ( 47 > SliderBase->cl_Lib.lib_Version ) {

    velocityGadget =
      IntegerObject,
        INTEGER_MaxChars, 3,
        INTEGER_Minimum, 0,
        INTEGER_Maximum, 127,
        INTEGER_Number, CAMD_Keyboard_Base->ck_Velocity,
        GA_ID, GadgetId_VelocityInteger,
        GA_RelVerify, TRUE,
      IntegerEnd;
    channelGadget =
      IntegerObject,
        INTEGER_MaxChars, 2,
        INTEGER_Minimum, 0,
        INTEGER_Maximum, 15,
        INTEGER_Number, CAMD_Keyboard_Base->ck_Channel,
        GA_ID, GadgetId_ChannelInteger,
        GA_RelVerify, TRUE,
      IntegerEnd;

  } else {

    velocityGadget =
      SliderObject,
        SLIDER_Orientation, SLIDER_HORIZONTAL,
        SLIDER_Min, 0,
        SLIDER_Max, 127,
        SLIDER_Level, CAMD_Keyboard_Base->ck_Velocity,
        SLIDER_Ticks, 8,
        SLIDER_LevelPlace, PLACETEXT_RIGHT,
        SLIDER_LevelFormat, "%3ld",
        SLIDER_LevelMaxLen, 3,
        GA_ID, GadgetId_VelocitySlider,
        GA_RelVerify, TRUE,
      SliderEnd;
    channelGadget =
      SliderObject,
        SLIDER_Orientation, SLIDER_HORIZONTAL,
        SLIDER_Min, 0,
        SLIDER_Max, 15,
        SLIDER_Level, CAMD_Keyboard_Base->ck_Channel,
        SLIDER_Ticks, 8,
        SLIDER_LevelPlace, PLACETEXT_RIGHT,
        SLIDER_LevelFormat, "%3ld",
        SLIDER_LevelMaxLen, 3,
        GA_ID, GadgetId_ChannelSlider,
        GA_RelVerify, TRUE,
      IntegerEnd;
  }

  CAMD_Keyboard_Base->ck_ListBrowser =
    ListBrowserObject,
      LISTBROWSER_Labels, &( CAMD_Keyboard_Base->ck_InstrumentLabels ),
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

  CAMD_Keyboard_Base->ck_MainWindowContent = WindowObject,
    WA_PubScreen, CAMD_Keyboard_Base->ck_Screen,
    WA_ScreenTitle, APP_IDSTRING,
    WA_Title, "CAMD Keyboard",
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

      LAYOUT_AddChild, ChooserObject,
        GA_ID, GadgetId_DeviceChooser,
        GA_RelVerify, TRUE,
        CHOOSER_Labels, &( CAMD_Keyboard_Base->ck_DeviceLabels ),
        CHOOSER_Selected, 0,
        CHOOSER_PopUp, TRUE,
        CHOOSER_AutoFit, TRUE,
      ChooserEnd,
      CHILD_NominalSize, TRUE,
      CHILD_Label, LabelObject,
        LABEL_Text ,"Target CAMD device: ",
      LabelEnd,

      LAYOUT_AddChild, HLayoutObject,
        LAYOUT_AddChild, VLayoutObject,

          LAYOUT_AddChild, velocityGadget,
          CHILD_Label, LabelObject,
            LABEL_Text ,"Velocity:",
          LabelEnd,

          LAYOUT_AddChild, channelGadget,
          CHILD_Label, LabelObject,
            LABEL_Text ,"Channel: ",
          LabelEnd,

          LAYOUT_AddChild, ButtonObject,
            GA_Text, "Print Info",
            GA_ID, GadgetId_InfoButton,
            GA_RelVerify, TRUE,
          ButtonEnd,
        LayoutEnd,

        LAYOUT_AddChild, VLayoutObject,
          LAYOUT_AddImage, LabelObject,
            LABEL_Text, "Selected instrument:",
            LABEL_Justification, LABEL_CENTER,
          LabelEnd,
          LAYOUT_AddChild, CAMD_Keyboard_Base->ck_ListBrowser,
        LayoutEnd,
      LayoutEnd,

      LAYOUT_AddChild, CAMD_Keyboard_Base->ck_Clavier = NewObject( ClavierGadgetClass, NULL,
        GA_ID, GadgetId_Clavier,
        GA_RelVerify, TRUE,
      TAG_END ),

      LAYOUT_AddChild, CAMD_Keyboard_Base->ck_Scroller = ScrollerObject,
        GA_ID, GadgetId_Scroller,
        // Do not need the events ATM: GA_RelVerify, TRUE,
        SCROLLER_Orientation, SCROLLER_HORIZONTAL,
      ScrollerEnd,
    LayoutEnd,
  EndWindow;

  if ( !CAMD_Keyboard_Base->ck_MainWindowContent ) {
    return;
  }

  // On creation by window open, clavier needs to tell scroller the size!
  SetGadgetAttrs(
    CAMD_Keyboard_Base->ck_Clavier,
    NULL, // Window does not yet exist
    NULL,
    ICA_TARGET, CAMD_Keyboard_Base->ck_Scroller,
    ICA_MAP, clavier2scroller,
    TAG_END );

  CAMD_Keyboard_Base->ck_MainWindow = ( struct Window * )
    RA_OpenWindow( CAMD_Keyboard_Base->ck_MainWindowContent );

  if ( !CAMD_Keyboard_Base->ck_MainWindow ) {
    return;
  }

  GetAttr( CG_VIRTUAL_WIDTH, CAMD_Keyboard_Base->ck_Clavier, &( virtualWidth ));
  GetAttr( CG_VISUAL_WIDTH, CAMD_Keyboard_Base->ck_Clavier, &( visualWidth ));
  offsetX = ( virtualWidth - visualWidth ) >> 1;
  // ICA_TARGET + ICA_MAP mechanisms see
  // - https://wiki.amigaos.net/wiki/BOOPSI_-_Object_Oriented_Intuition
  // - http://amigadev.elowar.com/read/ADCD_2.1/Libraries_Manual_guide/node04CA.html
  // - https://www.theflatnet.de/pub/cbm/amiga/AmigaDevDocs/lib_12.html
  // and remember: doing them on stack does not work,
  // keeps going invalid in between and won't work, but not crash either.
  SetGadgetAttrs(
    CAMD_Keyboard_Base->ck_Scroller,
    CAMD_Keyboard_Base->ck_MainWindow,
    NULL,
    ICA_TARGET, CAMD_Keyboard_Base->ck_Clavier,
    ICA_MAP, scroller2clavier,
    SCROLLER_Top, offsetX,
    TAG_END );
  SetGadgetAttrs(
    CAMD_Keyboard_Base->ck_Clavier,
    CAMD_Keyboard_Base->ck_MainWindow,
    NULL,
    CG_OFFSET_X, offsetX,
    TAG_END );

  GetAttr( WINDOW_SigMask,
           CAMD_Keyboard_Base->ck_MainWindowContent, 
           &( CAMD_Keyboard_Base->ck_MainWindowSignal ));

  return;
}

VOID PlayNote( BYTE channel, BYTE note, BYTE velocity ) {

  struct MidiLink * link = CAMD_Keyboard_Base->ck_MidiLink;
  MidiMsg message = { 0L, 0L };

  message.mm_Status = MS_NoteOn | channel;
  message.mm_Data1 = note;
  message.mm_Data2 = velocity;
              
  PutMidiMsg( link, &( message ));

  message.mm_Status = MS_NoteOn | channel;
  message.mm_Data1 = note;
  message.mm_Data2 = 0;
  PutMidiMsg( link, &( message ));
}

VOID SelectInstrument( BYTE channel ) {

  struct CAMD_Keyboard * base = CAMD_Keyboard_Base;
  struct MidiLink * link = base->ck_MidiLink;
  MidiMsg message = { 0L, 0L };

  if ( PERCUSSION_CHANNEL == channel ) {

    return;
  }

  message.mm_Status = MS_Prog | channel;
  message.mm_Data1 = base->ck_Instrument[ channel ];
  PutMidiMsg( link, &( message ));
}

VOID SelectChannel( BYTE channel ) {

  struct CAMD_Keyboard * base = CAMD_Keyboard_Base;
  const BYTE instrument = base->ck_Instrument[ channel ];
  const BOOL percussionsNext = ( PERCUSSION_CHANNEL == channel );
  const BOOL percussionsBefore = ( PERCUSSION_CHANNEL == base->ck_Channel );
  const BOOL switchedType = ( percussionsNext != percussionsBefore );

  if ( switchedType ) {
    if ( percussionsNext ) {

      SetGadgetAttrs( base->ck_ListBrowser,
                      base->ck_MainWindow,
                      NULL,
                      LISTBROWSER_ShowSelected, FALSE,
                      LISTBROWSER_Selected, -1,
                      LISTBROWSER_MakeVisible, instrument,
                      LISTBROWSER_Labels, &( base->ck_PercussionLabels ),
                      LISTBROWSER_ColumnInfo, percussionColumns,
                      TAG_END );

    } else {

      SetGadgetAttrs( base->ck_ListBrowser,
                      base->ck_MainWindow,
                      NULL,
                      LISTBROWSER_ShowSelected, TRUE,
                      LISTBROWSER_Selected, instrument,
                      LISTBROWSER_MakeVisible, instrument,
                      LISTBROWSER_Labels, &( base->ck_InstrumentLabels ),
                      LISTBROWSER_ColumnInfo, instrumentColumns,
                      TAG_END );

    }
  } else {

    SetGadgetAttrs( base->ck_ListBrowser,
                    base->ck_MainWindow,
                    NULL,
                    LISTBROWSER_Selected, instrument,
                    LISTBROWSER_MakeVisible, instrument,
                    TAG_END );
  }

  Printf( "New channel %ld with instrument %ld\n",
          channel, instrument );
  base->ck_Channel = ( BYTE ) channel;
}

VOID PrintInfos( VOID ) {
  ULONG u;
  ULONG v;
  ULONG w;
  ULONG x;
  ULONG y;
  ULONG z;
  u = GetAttr( CG_OFFSET_X, CAMD_Keyboard_Base->ck_Clavier, &v );
  w = GetAttr( CG_VIRTUAL_WIDTH, CAMD_Keyboard_Base->ck_Clavier, &x );
  y = GetAttr( CG_VISUAL_WIDTH, CAMD_Keyboard_Base->ck_Clavier, &z );
  SetGadgetAttrs(
    CAMD_Keyboard_Base->ck_Scroller,
    CAMD_Keyboard_Base->ck_MainWindow,
    NULL,
    SCROLLER_Visible, z,
    SCROLLER_Total, x,
    TAG_END
  );
  Printf( "From Clavier: %ld %ld %ld %ld %ld %ld\n", u, v, w, x, y, z );
  u = GetAttr( SCROLLER_Top, CAMD_Keyboard_Base->ck_Scroller, &v );
  SetGadgetAttrs(
    CAMD_Keyboard_Base->ck_Clavier,
    CAMD_Keyboard_Base->ck_MainWindow,
    NULL,
    CG_OFFSET_X, v,
    TAG_END
  );
  Printf( "From Scroller: %ld %ld\n", u, v );
  RefreshGadgets( CAMD_Keyboard_Base->ck_Clavier,
  CAMD_Keyboard_Base->ck_MainWindow, NULL);
  RefreshGadgets( CAMD_Keyboard_Base->ck_Scroller,
  CAMD_Keyboard_Base->ck_MainWindow, NULL);
  w = GetAttr( GA_ID, CAMD_Keyboard_Base->ck_Clavier, &x );
  y = GetAttr( GA_ID, CAMD_Keyboard_Base->ck_Scroller, &z );
  Printf( "Clar: %ld %ld Scr: %ld %ld\n", w, x, y, z );
}

VOID PrintMidiClusters( VOID ) {

  APTR lock = LockCAMD( CD_Linkages );

  if ( NULL != lock ) {

    struct MidiCluster * cluster;

    Printf( "CAMD locked @ 0x%08lx\n", (ULONG)lock );

    for ( cluster = NextCluster( NULL );
          cluster;
          cluster = NextCluster( cluster )) {

      struct MidiLink * next;

      Printf( "Cluster 0x%08lx\n", ( ULONG ) cluster );
      Printf( "+-> Name %s\n", cluster->mcl_Node.ln_Name );
      Printf( "+-> Participants %ld\n", cluster->mcl_Participants );
      Printf( "+-> PublicParticipants %ld\n", cluster->mcl_PublicParticipants );
      Printf( "+-> Flags 0x%04lx\n", cluster->mcl_Flags );
      
      Printf( "+-> Receivers:\n" );
      FOR_LIST( &cluster->mcl_Receivers, next, struct MidiLink * ) {
        STRPTR location = NULL;
        STRPTR name = NULL;
        STRPTR comment = NULL;
        Printf( "+---> Link 0x%08lx\n", ( ULONG ) next );
        GetMidiLinkAttrs( next,
                          MLINK_Location, &location,
                          MLINK_Name, &name,
                          MLINK_Comment, &comment,
                          TAG_END );
        if ( location ) {
          Printf( "+-----> Location %s \n", location );
        }
        if ( name ) {
          Printf( "+-----> Name %s \n", name );
        }
        if ( comment ) {
          Printf( "+-----> Comment %s \n", comment );
        }
      }
      Printf( "+-> Senders:\n" );
      FOR_LIST( &cluster->mcl_Senders, next, struct MidiLink * ) {
        STRPTR location = NULL;
        STRPTR name = NULL;
        STRPTR comment = NULL;
        Printf( "+---> Link 0x%08lx\n", ( ULONG ) next );
        GetMidiLinkAttrs( next,
                          MLINK_Location, &location,
                          MLINK_Name, &name,
                          MLINK_Comment, &comment,
                          TAG_END );
        if ( location ) {
          Printf( "+-----> Location %s \n", location );
        }
        if ( name ) {
          Printf( "+-----> Name %s \n", name );
        }
        if ( comment ) {
          Printf( "+-----> Comment %s \n", comment );
        }
      }
    }
    UnlockCAMD( lock );
  }
}

VOID PrintMidiNodes( VOID ) {

  APTR lock = LockCAMD( CD_Linkages );

  if ( NULL != lock ) {

    struct MidiNode * node;

    Printf( "CAMD locked @ 0x%08lx\n", (ULONG)lock );
    
    for ( node = NextMidi( NULL );
          node;
          node = NextMidi( node )) {

      struct MidiLink * link;
      STRPTR name = NULL;

      Printf( "Node 0x%08lx\n", ( ULONG ) node );
      Printf( "+-> ClientType 0x%08lx\n", node->mi_ClientType );
      GetMidiAttrs( node, MIDI_Name, &name, TAG_END );
      if ( name ) {
        Printf( "+-> Name %s \n", name );
      }

      link = NextMidiLink( node, NULL, 0xFFFF );
      while ( link ) {

        STRPTR location = NULL;
        STRPTR comment = NULL;
        Printf( "+-> Link 0x%08lx\n", ( ULONG ) link );
        GetMidiLinkAttrs( link,
                          MLINK_Location, &location,
                          MLINK_Name, &name,
                          MLINK_Comment, &comment,
                          TAG_END );
        if ( location ) {
          Printf( "+---> Location %s \n", location );
        }
        if ( name ) {
          Printf( "+---> Name %s \n", name );
        }
        if ( comment ) {
          Printf( "+---> Comment %s \n", comment );
        }
        link = NextMidiLink( node, link, 0xFFFF );
      }
    }
    UnlockCAMD( lock );
  }
}

VOID HandleEvents( VOID ) {

  BOOL stop = FALSE;
  struct CAMD_Keyboard * base = CAMD_Keyboard_Base;

  while ( !( stop )) {

    ULONG signals = Wait( base->ck_MainWindowSignal
                          | SIGBREAKF_CTRL_C );
    if ( SIGBREAKF_CTRL_C & signals) {
      stop = TRUE;
    }

    for ( ; ; ) {

      WORD windowMessageCode;
      ULONG windowMessage = DoMethod( base->ck_MainWindowContent,
                                      WM_HANDLEINPUT, 
                                      &windowMessageCode );

      if ( WMHI_LASTMSG == windowMessage ) {

        // All messages handled => break for-loop!
        break;
      }

      switch ( WMHI_CLASSMASK & windowMessage ) {
        case WMHI_GADGETUP: {
          switch ( WMHI_GADGETMASK & windowMessage ) {
            case GadgetId_DeviceChooser: {

              Printf( "Chooser picked item %ld.\n", windowMessageCode );
              CloseMidi();
              OpenMidi( windowMessageCode );
              break;
            }
            case GadgetId_InfoButton: {
              // PrintInfos();
              PrintMidiClusters();
              PrintMidiNodes();
              break;
            }
            case GadgetId_Clavier: {

              BYTE channel = base->ck_Channel;
              BYTE note = windowMessageCode;
              BYTE velocity = base->ck_Velocity;

              Printf( "Playing channel %ld, note %ld, velocity %ld\n",
                      channel, note, velocity );
              PlayNote( channel, note, velocity );
              break;
            }
            case GadgetId_VelocityInteger:
            case GadgetId_VelocitySlider: {

              Printf( "New velocity %ld\n", windowMessageCode );
              base->ck_Velocity = ( BYTE ) windowMessageCode;
              break;
            }
            case GadgetId_ChannelInteger:
            case GadgetId_ChannelSlider: {

              SelectChannel( windowMessageCode );
              break;
            }
            case GadgetId_Instruments: {

              WORD channel = base->ck_Channel;
              BYTE instrument = ( BYTE ) windowMessageCode;

              Printf( "New instrument %ld in channel %ld\n",
                        instrument, channel );
              base->ck_Instrument[ channel ] = instrument;
              SelectInstrument( channel );
              break;
            }
            default: {
              Printf( "Unknown Gadget\n" );
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
          Printf("WMHI_NEWSIZE\n");
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