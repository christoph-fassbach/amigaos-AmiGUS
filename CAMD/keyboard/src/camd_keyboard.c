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

#include <proto/exec.h>
#include <proto/dos.h>

#include "camd_keyboard.h"
#include "debug.h"
#include "errors.h"
#include "keyboard_window.h"
#include "support.h"

STRPTR _AppVersionString = "$VER: " APP_IDSTRING "\r\n";

struct CAMD_Keyboard     * CAMD_Keyboard_Base    = NULL;
struct Library           * GadToolsBase          = NULL;
struct IntuitionBase     * IntuitionBase         = NULL;
struct GfxBase           * GfxBase               = NULL;

struct Library           * BevelBase             = NULL;
struct Library           * LabelBase             = NULL;
struct Library           * LayoutBase            = NULL;
struct Library           * ListBrowserBase       = NULL;
struct Library           * VirtualBase           = NULL;
struct Library           * WindowBase            = NULL;

LONG Startup( VOID ) {

  if ( !CAMD_Keyboard_Base ) {

    CAMD_Keyboard_Base = ( struct CAMD_Keyboard * )
      AllocMem( sizeof( struct CAMD_Keyboard ), MEMF_CLEAR );
  }
  if ( !CAMD_Keyboard_Base ) {

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
  if ( !GfxBase ) {

    GfxBase = ( struct GfxBase * )
      OpenLibrary( "graphics.library", 36 );
  }
  if ( !GfxBase ) {

    DisplayError( EOpenGfxBase );
    return EOpenGfxBase;
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

  if ( !BevelBase ) {

    BevelBase = OpenLibrary( "images/bevel.image", 41 );
  }
  if ( !BevelBase ) {

    CAMD_Keyboard_Base->ck_Flags |= CAMD_TOOL_FLAG_REACTION_FAILED;
  }
  if ( !LabelBase ) {

    LabelBase = OpenLibrary( "images/label.image", 41 );
  }
  if ( !LabelBase ) {

    CAMD_Keyboard_Base->ck_Flags |= CAMD_TOOL_FLAG_REACTION_FAILED;
  }
  if ( !LayoutBase ) {

    LayoutBase = OpenLibrary( "gadgets/layout.gadget", 41 );
  }
  if ( !LayoutBase ) {

    CAMD_Keyboard_Base->ck_Flags |= CAMD_TOOL_FLAG_REACTION_FAILED;
  }
  if ( !ListBrowserBase ) {

    ListBrowserBase = OpenLibrary( "gadgets/listbrowser.gadget", 41 );
  }
  if ( !ListBrowserBase ) {

    CAMD_Keyboard_Base->ck_Flags |= CAMD_TOOL_FLAG_REACTION_FAILED;
  }
  if ( !VirtualBase ) {

    VirtualBase = OpenLibrary( "gadgets/virtual.gadget", 41 );
  }
  if ( !VirtualBase ) {

    CAMD_Keyboard_Base->ck_Flags |= CAMD_TOOL_FLAG_REACTION_FAILED;
  }
  if ( !WindowBase ) {

    WindowBase = OpenLibrary( "window.class", 41 );
  }
  if ( !WindowBase ) {

    CAMD_Keyboard_Base->ck_Flags |= CAMD_TOOL_FLAG_REACTION_FAILED;
  }
  if ( CAMD_Keyboard_Base->ck_Flags & CAMD_TOOL_FLAG_REACTION_FAILED ) {

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
  LOG_I(( "I: WindowBase gone.\n" ));
  if ( VirtualBase ) {

    CloseLibrary( VirtualBase );
    VirtualBase = NULL;
  }
  LOG_I(( "I: VirtualBase gone.\n" ));
  if ( ListBrowserBase ) {

    CloseLibrary( ListBrowserBase );
    ListBrowserBase = NULL;
  }
  LOG_I(( "I: ListBrowserBase gone.\n" ));
  if ( LayoutBase ) {

    CloseLibrary( LayoutBase );
    LayoutBase = NULL;
  }
  LOG_I(( "I: LayoutBase gone.\n" ));
  if ( LabelBase ) {

    CloseLibrary( LabelBase );
    LabelBase = NULL;
  }
  LOG_I(( "I: LabelBase gone.\n" ));
  if ( BevelBase ) {

    CloseLibrary( BevelBase );
    BevelBase = NULL;
  }
  LOG_I(( "I: BevelBase gone.\n" ));

  if ( GadToolsBase ) {

    CloseLibrary( GadToolsBase );
    GadToolsBase = NULL;
  }
  LOG_I(( "I: GadToolsBase gone.\n" ));
  if ( IntuitionBase ) {

    CloseLibrary(( struct Library *) IntuitionBase );
    IntuitionBase = NULL;
  }
  LOG_I(( "I: IntuitionBase gone.\n" ));
  if ( GfxBase ) {

    CloseLibrary(( struct Library *) GfxBase );
    GfxBase = NULL;
  }
  LOG_I(( "I: GfxBase gone.\n" ));
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
#if 0
  // Does not like to be cleaned up here!
  if ( DOSBase ) {

    CloseLibrary(( struct Library *) DOSBase );
    DOSBase = NULL;
  }
#endif
}

VOID HandleMainEventLoop( VOID ) {

  struct CAMD_Keyboard * base = CAMD_Keyboard_Base;
  BOOL abort = FALSE;

  while ( !( abort )) {

    ULONG signalled =
      Wait( base->ck_TestWindowSignal
          | base->ck_TestContentSignal // <- has all events so far
          | SIGBREAKF_CTRL_C );
    if ( SIGBREAKF_CTRL_C & signalled ) {

      abort = TRUE;

    } else {

      abort = HandleKeyboardWindowEvents( base );
    }
  }
  return;
}

int main( VOID ) {

  LONG error;

  error = Startup();
  if ( error ) {

    LOG_E(( "E: Startup failed.\n" ));
    Cleanup();
    return -10;
  }

  LOG_I(( "I: Startup complete.\n" ));
  error = CreateKeyboardWindow( CAMD_Keyboard_Base );
  if ( error ) {

    LOG_I(( "E: Keyboard window failed.\n" ));
    CleanupKeyboardWindow( CAMD_Keyboard_Base );
    Cleanup();
    return -5;
  }

  LOG_I(( "I: Keyboard window complete.\n" ));
  HandleMainEventLoop();

  LOG_I(( "I: Main event loop done.\n" ));
  CleanupKeyboardWindow( CAMD_Keyboard_Base );

  LOG_I(( "I: Keyboard window gone.\n" ));
  Cleanup();

  return ENoError;
}
