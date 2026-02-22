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

#define ALIB_STDIO

#include <clib/alib_protos.h>
#include <graphics/gfxbase.h>
#include <intuition/icclass.h>
#include <intuition/intuitionbase.h>
#include <reaction/reaction_macros.h>

#include <proto/alib.h>
#include <proto/chooser.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/label.h>
#include <proto/layout.h>
#include <proto/scroller.h>
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
struct ClassLibrary      * LabelBase;
struct ClassLibrary      * LayoutBase;
struct ClassLibrary      * ScrollerBase;
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

enum GadgetIds {

  GadgetId_Start = 100,
  GadgetId_DeviceChooser,
  GadgetId_ButtonOK,
  GadgetId_ButtonCancel,
  GadgetId_ClavierButton,
  GadgetId_Scroller,
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

LONG OpenMidi( ULONG index ) {

  TEXT linkName[128];
  STRPTR name = STR( APP_FILE );

  struct CAMD_Keyboard * base = CAMD_Keyboard_Base;
  struct CAMD_Device_Node * node = ( struct CAMD_Device_Node * )
    NodeAtIndex( &( base->ck_Devices ), index );

  if ( !node ) {

    return ENoMidiNodes;
  }

  // Keyboard only supports playback, hence does not need input buffers.
  base->ck_MidiNode = CreateMidi( MIDI_Name, name,
                                  // MIDI_MsgQueue, 0L,
                                  // MIDI_SysExSize,0L,
                                  MIDI_ErrFilter, CMEF_All,
                                  TAG_END );
  if ( !( base->ck_MidiNode )) {

    return ECreateMidi;
  }
  LOG_D(( "D: Got output node 0x%08lx for %s.\n",
          base->ck_MidiNode, name ));

  sprintf( linkName, "%s Link", name );
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
  // TODO: error handling of all the below!
  OpenLib(( struct Library ** )&CamdBase, "camd.library", 37, EOpenCamdBase );
  OpenLib(( struct Library ** )&IntuitionBase, "intuition.library", 36, EOpenIntuitionBase );
  OpenLib(( struct Library ** )&GfxBase, "graphics.library", 36, EOpenGfxBase );
  OpenLib(( struct Library ** )&UtilityBase, "utility.library", 36, EOpenUtilityBase );
  OpenLib(( struct Library ** )&DiskfontBase, "diskfont.library", 36, EOpenDiskfontBase );

  OpenLib(( struct Library ** )&ButtonBase, "gadgets/button.gadget", 0, EOpenButtonBase );
  OpenLib(( struct Library ** )&ChooserBase, "gadgets/chooser.gadget", 0, EOpenChooserBase );
  OpenLib(( struct Library ** )&LabelBase, "images/label.image", 0, EOpenLabelBase );
  OpenLib(( struct Library ** )&LayoutBase, "gadgets/layout.gadget", 0, EOpenLayoutBase );
  OpenLib(( struct Library ** )&ScrollerBase, "gadgets/scroller.gadget", 0, EOpenScrollerBase );
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

  LOG_I(( "I: " STR( APP_NAME ) " startup complete.\n" ));
}

VOID Cleanup( VOID ) {

  CloseMidi();
  FreeCamdOutputDevices( &( CAMD_Keyboard_Base->ck_Devices ));
  FreeChooserLabels();

  CloseLib(( struct Library ** )&WindowBase );
  CloseLib(( struct Library ** )&ScrollerBase );
  CloseLib(( struct Library ** )&LayoutBase );
  CloseLib(( struct Library ** )&LabelBase );
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
    WA_Flags,WFLG_CLOSEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_SIZEBBOTTOM | WFLG_SIZEGADGET | WFLG_ACTIVATE,
    WA_IDCMP, IDCMP_VANILLAKEY | IDCMP_NEWSIZE,
    WINDOW_Position, WPOS_TOPLEFT,
    WINDOW_IconifyGadget, FALSE,

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

      LAYOUT_AddChild, CAMD_Keyboard_Base->ck_Scroller = ScrollerObject,
        GA_ID, GadgetId_Scroller,
        // Do not need the events ATM: GA_RelVerify, TRUE,
        SCROLLER_Orientation, SCROLLER_HORIZONTAL,
      ScrollerEnd,

      LAYOUT_AddChild, CAMD_Keyboard_Base->ck_Clavier = NewObject( ClavierGadgetClass, NULL,
        GA_ID, GadgetId_Clavier,
        GA_RelVerify, TRUE,
        ICA_TARGET, CAMD_Keyboard_Base->ck_Scroller,
        ICA_MAP, clavier2scroller,
      TAG_END ),
    LayoutEnd,
  EndWindow;

  if ( !CAMD_Keyboard_Base->ck_MainWindowContent ) {
    return;
  }

  CAMD_Keyboard_Base->ck_MainWindow = ( struct Window * )
    RA_OpenWindow( CAMD_Keyboard_Base->ck_MainWindowContent );

  if ( !CAMD_Keyboard_Base->ck_MainWindow ) {
    return;
  }

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
    TAG_END );

  GetAttr( WINDOW_SigMask,
           CAMD_Keyboard_Base->ck_MainWindowContent, 
           &( CAMD_Keyboard_Base->ck_MainWindowSignal ));

  return;
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
      ULONG result;

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
        result = GetMidiLinkAttrs( link,
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

              LOG_D(( "D: Chooser picked item %ld.\n", windowMessageCode ));
              CloseMidi();
              OpenMidi( windowMessageCode );
              break;
            }
            case GadgetId_ButtonOK:
            case GadgetId_ButtonCancel: {

              stop = TRUE;
              break;
            }
            case GadgetId_ClavierButton: {
              // PrintInfos();
              PrintMidiClusters();
              PrintMidiNodes();
              break;
            }
            case GadgetId_Clavier: {
              MidiMsg message;
              message.mm_Status = MS_NoteOn | 6; // 1 = current Channel
              message.mm_Data1 = windowMessageCode; // current Note
              message.mm_Data2 = 127; // current Velocity or 0 for note off
              Printf( "echtes clavier %ld\n", windowMessageCode );

              PutMidiMsg( base->ck_MidiLink, &( message ));
              Printf( "Ton an\n" );
              Printf( "Ton wieder aus\n" );

              message.mm_Status = MS_NoteOn | 6; // 1 = current Channel
              message.mm_Data1 = windowMessageCode; // current Note
              message.mm_Data2 = 0; // current Velocity or 0 for note off
              PutMidiMsg( base->ck_MidiLink, &( message ));

              break;
            }
            case GadgetId_Scroller: {
              Printf( "scroller %ld\n", windowMessageCode );
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