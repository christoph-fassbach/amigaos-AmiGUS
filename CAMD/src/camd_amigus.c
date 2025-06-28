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

#include <intuition/intuitionbase.h>
#include <libraries/expansionbase.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include <midi/camddevices.h>

#include "amigus_camd.h"
#include "amigus_wavetable.h"
#include "debug.h"
#include "errors.h"
#include "support.h"
#include "SDI_compiler.h"

// As declared in amigus_camd.h
struct ExecBase          * SysBase           = NULL;
struct DosLibrary        * DOSBase           = NULL;
struct IntuitionBase     * IntuitionBase     = NULL;
struct Library           * UtilityBase       = NULL;
struct Library           * ExpansionBase     = NULL;
struct AmiGUS_CAMD       * AmiGUS_CAMD_Base  = NULL;

STRPTR _LibVersionString = "$VER: " LIBRARY_IDSTRING "\r\n";
STRPTR AmiGUS_IDString = LIBRARY_IDSTRING "\0";
STRPTR AmiGUS_Name = LIBRARY_NAME;

/******************************************************************************
 * CAMD MIDI driver replacement forward declarations
 *****************************************************************************/

 /**
 * CAMD internal function,
 * to be called when hardware can send another byte of data.
 * See driver.doc, line 137.
 * TODO
 */
typedef ULONG ( ASM( * ) TransmitFunctionType )(
  REG( a2, APTR userdata ));

/**
 * CAMD internal function,
 * to be called when hardware received a new byte of data.
 * See driver.doc, line 129.
 * TODO
 */
typedef VOID ( ASM( * ) ReceiveFunctionType )(
  REG( d0, UWORD input ),
  REG( a2, APTR userdata ));

/******************************************************************************
 * CAMD MIDI driver's "public" function forward declarations.
 *****************************************************************************/

/**
 * This is the initialization routine for the AmiGUS CAMD MIDI driver.
 *
 * It initializes any global data needed by the driver and
 * finds the address of the AmiGUS wavetable hardware.
 * For multi-card support, it could now determine if multiple boards 
 * are installed and patch the number of ports field to indicate the
 * number of ports available.
 * 
 * Documentation says it takes no parameters, but as CAMD loads the 
 * drivers via Exec's LoadSeg() ( *SIGH!!!* ), it is...
 *
 * @param sysBase Pointer to Exec / SysBase.
 *
 * @return TRUE if loaded everything loaded / openened / found successful,
 *         FALSE in case of error.
 */
ASM( BOOL ) SAVEDS AmiGUS_Init( REG( a6, struct ExecBase * sysBase ));

ASM( VOID ) SAVEDS AmiGUS_Expunge( VOID );

ASM( APTR ) SAVEDS AmiGUS_OpenPort (
  REG( a3, struct MidiDeviceData * data ),
  REG( d0, LONG portnum ),
  REG( a0, TransmitFunctionType transmitHandler ), /* Transmit function       */
  REG( a1, ReceiveFunctionType receiveHandler ),   /* Receive function        */
  REG( a2, APTR userdata ));

ASM( VOID ) SAVEDS AmiGUS_ClosePort (
  REG( a3, struct MidiDeviceData * data ),
  REG( d0, LONG portnum ));

ASM( VOID ) SAVEDS AmiGUS_ActivateXmit( VOID );

/******************************************************************************
 * CAMD MIDI driver's global data definitions.
 *****************************************************************************/

struct MidiPortData AmiGUS_MidiPortData = { AmiGUS_ActivateXmit };

/******************************************************************************
 * CAMD MIDI driver's "public" function definitions.
 *****************************************************************************/

ASM( BOOL ) SAVEDS AmiGUS_Init( REG( a6, struct ExecBase * sysBase )) {

  LONG error = ENoError;

  /* Prevent use of customized library versions on CPUs not targetted. */
#ifdef _M68060
  if( !(sysBase->AttnFlags & AFF_68060) ) {

    AmiGUS_Expunge();
    return FALSE;
  }
#elif defined (_M68040)
  if( !(sysBase->AttnFlags & AFF_68040) ) {

    AmiGUS_Expunge();
    return FALSE;
  }
#elif defined (_M68030)
  if( !(sysBase->AttnFlags & AFF_68030) ) {

    AmiGUS_Expunge();
    return FALSE;
  }
#elif defined (_M68020)
  if( !(sysBase->AttnFlags & AFF_68020) ) {

    AmiGUS_Expunge();
    return FALSE;
  }
#endif

  SysBase = sysBase;

  AmiGUS_CAMD_Base = AllocMem(
    sizeof( struct AmiGUS_CAMD ),
    MEMF_PUBLIC | MEMF_CLEAR );
  if ( !AmiGUS_CAMD_Base ) {

    DisplayError( EAllocateAmiGUSCAMDBase );
    AmiGUS_Expunge();
    return FALSE;
  }

  DOSBase =
    ( struct DosLibrary * ) OpenLibrary( "dos.library", 34 );
  if ( !( EOpenDosBase )) {

    DisplayError( EOpenDosBase );
    AmiGUS_Expunge();
    return FALSE;
  }
  IntuitionBase =
    ( struct IntuitionBase * ) OpenLibrary( "intuition.library", 34 );
  if ( !( IntuitionBase )) {

    DisplayError( EOpenIntuitionBase );
    AmiGUS_Expunge();
    return FALSE;
  }
  ExpansionBase =
    ( struct Library * ) OpenLibrary( "expansion.library", 34 );
  if ( !( ExpansionBase )) {

    DisplayError( EOpenExpansionBase );
    AmiGUS_Expunge();
    return FALSE;
  }

  error = FindAmiGusWavetable( &( AmiGUS_CAMD_Base->agb_ConfigDevice ));
  if ( error ) {

    DisplayError( error );
    AmiGUS_Expunge();
    return FALSE;
  }

  LOG_D(( "D: AmiGUS base ready @ 0x%08lx\n", AmiGUS_CAMD_Base ));
  return TRUE;
}

ASM( VOID ) SAVEDS AmiGUS_Expunge( VOID ) {

  LOG_D(( "D: AmiGUS base @ 0x%08lx leaving the building...\n",
          AmiGUS_CAMD_Base ));
  
  if ( AmiGUS_CAMD_Base->agb_LogFile ) {

    Close( AmiGUS_CAMD_Base->agb_LogFile );
  }
  /*
  Remember: memory cannot be overwritten if we do not return it. :)
  So... we leak it here... 
  if ( AmiGUS_CAMD_Base->agb_LogMem ) {

    FreeMem( AmiGUS_CAMD_Base->agb_LogMem, ... );
  }    
  */
  if ( DOSBase ) {

    CloseLibrary(( struct Library *) DOSBase );
  }
  if ( IntuitionBase ) {

    CloseLibrary(( struct Library * ) IntuitionBase );
  }
  if ( ExpansionBase ) {

    CloseLibrary(( struct Library * ) ExpansionBase );
  }
  if ( AmiGUS_CAMD_Base ) {

    FreeMem( AmiGUS_CAMD_Base, sizeof( struct AmiGUS_CAMD ));
  }
}

ASM( APTR ) SAVEDS AmiGUS_OpenPort (
  REG( a3, struct MidiDeviceData * data ),
  REG( d0, LONG portnum ),
  REG( a0, TransmitFunctionType transmitHandler ), /* Transmit function       */
  REG( a1, ReceiveFunctionType receiveHandler ),   /* Receive function        */
  REG( a2, APTR userdata )) {

  LOG_D(( "D: AmiGUS CAMD OpenPort called for "
          " data=0x%08lx port=%ld txh=0x%08lx rxh=0x%08lx user=0x%08lx\n",
          data, portnum, transmitHandler, receiveHandler, userdata ));

  return &AmiGUS_MidiPortData;
}

ASM( VOID ) SAVEDS AmiGUS_ClosePort (
  REG( a3, struct MidiDeviceData * data ),
  REG( d0, LONG portnum )) {

  LOG_D(( "D: AmiGUS CAMD ClosePort called for data=0x%08lx port=%ld\n",
          data, portnum ));
}

ASM( VOID ) SAVEDS AmiGUS_ActivateXmit( VOID ) {

  LOG_D(( "D: AmiGUS CAMD ActivateXmit called\n" ));
}
