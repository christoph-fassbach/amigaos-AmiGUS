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
#include <proto/dos.h>

#include "camd_prefs.h"
#include "debug.h"
#include "errors.h"
#include "prefs_window.h"
#include "support.h"

STRPTR _AppVersionString = "$VER: " APP_IDSTRING "\r\n";

struct AmiGUS_CAMD_Tool  * AmiGUS_CAMD_Tool_Base = NULL;
struct Library           * GadToolsBase          = NULL;
struct IntuitionBase     * IntuitionBase         = NULL;

struct Library           * LabelBase             = NULL;
struct Library           * LayoutBase            = NULL;
struct Library           * ListBrowserBase       = NULL;
struct Library           * VirtualBase           = NULL;
struct Library           * WindowBase            = NULL;

LONG Startup( VOID ) {

  if ( !AmiGUS_CAMD_Tool_Base ) {

    AmiGUS_CAMD_Tool_Base = ( struct AmiGUS_CAMD_Tool * )
      AllocMem( sizeof( struct AmiGUS_CAMD_Tool ), MEMF_CLEAR );
  }
  if ( !AmiGUS_CAMD_Tool_Base ) {

    DisplayError( EAllocateAmiGUSCAMDToolBase );
    return EAllocateAmiGUSCAMDToolBase;
  }
  if ( !DOSBase ) {

    DOSBase = ( struct DosLibrary * )
      OpenLibrary( "dos.library", 36 );
  }
  if ( !DOSBase ) {

    DisplayError( EOpenDosBase );
    return EOpenDosBase;
  }
  if ( !IntuitionBase ) {

    IntuitionBase = ( struct IntuitionBase * )
      OpenLibrary( "intuition.library", 36 );
  }
  if ( !IntuitionBase ) {

    DisplayError( EOpenIntuitionBase );
    return EOpenIntuitionBase;
  }
  if ( !GadToolsBase ) {

    GadToolsBase = OpenLibrary( "gadtools.library", 36 );
  }
  if ( !GadToolsBase ) {

    DisplayError( EOpenGadToolsBase );
    return EOpenGadToolsBase;
  }

  if ( !LabelBase ) {

    LabelBase = OpenLibrary( "images/label.image", 41 );
  }
  if ( !LabelBase ) {

    AmiGUS_CAMD_Tool_Base->agt_Flags |= CAMD_TOOL_FLAG_REACTION_FAILED;
  }
  if ( !LayoutBase ) {

    LayoutBase = OpenLibrary( "gadgets/layout.gadget", 41 );
  }
  if ( !LayoutBase ) {

    AmiGUS_CAMD_Tool_Base->agt_Flags |= CAMD_TOOL_FLAG_REACTION_FAILED;
  }
  if ( !ListBrowserBase ) {

    ListBrowserBase = OpenLibrary( "gadgets/listbrowser.gadget", 41 );
  }
  if ( !ListBrowserBase ) {

    AmiGUS_CAMD_Tool_Base->agt_Flags |= CAMD_TOOL_FLAG_REACTION_FAILED;
  }
  if ( !VirtualBase ) {

    VirtualBase = OpenLibrary( "gadgets/virtual.gadget", 41 );
  }
  if ( !VirtualBase ) {

    AmiGUS_CAMD_Tool_Base->agt_Flags |= CAMD_TOOL_FLAG_REACTION_FAILED;
  }
  if ( !WindowBase ) {

    WindowBase = OpenLibrary( "window.class", 41 );
  }
  if ( !WindowBase ) {

    AmiGUS_CAMD_Tool_Base->agt_Flags |= CAMD_TOOL_FLAG_REACTION_FAILED;
  }
  if ( AmiGUS_CAMD_Tool_Base->agt_Flags & CAMD_TOOL_FLAG_REACTION_FAILED ) {

    LOG_W(( "W: Could not open ReAction - Test button hidden!\n" ));
  }

  LOG_I(( "I: " STR( APP_NAME ) " startup complete.\n" ));
  return ENoError;
}

VOID Cleanup( VOID ) {

  LOG_I(( "I: " STR( APP_NAME ) " cleanup starting.\n" ));

  if ( WindowBase ) {

    CloseLibrary( WindowBase );
    WindowBase = NULL;
  }
  if ( VirtualBase ) {

    CloseLibrary( VirtualBase );
    VirtualBase = NULL;
  }
  if ( ListBrowserBase ) {

    CloseLibrary( ListBrowserBase );
    ListBrowserBase = NULL;
  }
  if ( LayoutBase ) {

    CloseLibrary( LayoutBase );
    LayoutBase = NULL;
  }
  if ( LabelBase ) {

    CloseLibrary( LabelBase );
    LabelBase = NULL;
  }

  if ( GadToolsBase ) {

    CloseLibrary( GadToolsBase );
    GadToolsBase = NULL;
  }
  if ( IntuitionBase ) {

    CloseLibrary(( struct Library *) IntuitionBase );
    IntuitionBase = NULL;
  }
  if ( AmiGUS_CAMD_Tool_Base ) {

    if ( AmiGUS_CAMD_Tool_Base->agt_LogFile ) {

      Close( AmiGUS_CAMD_Tool_Base->agt_LogFile );
    }
    /* Free'ing agt_LogMem deliberately not happening here! */

    FreeMem( AmiGUS_CAMD_Tool_Base, sizeof( struct AmiGUS_CAMD_Tool ));
    AmiGUS_CAMD_Tool_Base = NULL;
  }
  if ( DOSBase ) {

    CloseLibrary(( struct Library *) DOSBase );
    DOSBase = NULL;
  }
}

int main( VOID ) {

  LONG error;

  error = Startup();
  if ( error ) {

    Cleanup();
    return -10;
  }

  error = CreatePrefsUi( AmiGUS_CAMD_Tool_Base );
  if ( error ) {
    
    CleanupPrefsUi( AmiGUS_CAMD_Tool_Base );
    Cleanup();
    return -5;
  }

  HandlePrefsUiEvents( AmiGUS_CAMD_Tool_Base );

  CleanupPrefsUi( AmiGUS_CAMD_Tool_Base );
  Cleanup();

  return ENoError;
}
