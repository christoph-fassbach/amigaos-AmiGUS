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
#include <intuition/intuitionbase.h>

#include <proto/dos.h>
#include <proto/exec.h>

#include "camd_keyboard.h"
#include "debug.h"
#include "errors.h"
#include "support.h"

/* Globals defined somewhere - here ;) */
struct CAMD_Keyboard     * CAMD_Keyboard_Base;
struct GfxBase           * GfxBase;
struct IntuitionBase     * IntuitionBase;

// And owned by linker libraries:
// struct ExecBase       * SysBase;
// struct DosLibrary     * DOSBase;

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

  LOG_I(( "I: " STR( APP_NAME ) " startup complete.\n" ));
}

VOID Cleanup( VOID ) {

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

int main( VOID ) {

  Startup();
  Cleanup();

  return ENoError;
}