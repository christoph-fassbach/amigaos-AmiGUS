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

#include <amigus/amigus.h>
#include <intuition/intuitionbase.h>
#include <libraries/expansionbase.h>
#include <proto/amigus.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include <midi/camddevices.h>

#include "amigus_camd.h"
#include "amigus_wavetable.h"
#include "amisf.h"
#include "compiler_extras.h"
#include "debug.h"
#include "errors.h"
#include "progress_dialog.h"
#include "support.h"
#include "worker.h"

// Libraries as declared in amigus_camd.h:
struct ExecBase          * SysBase           = NULL;
struct DosLibrary        * DOSBase           = NULL;
struct IntuitionBase     * IntuitionBase     = NULL;
struct Library           * AmiGUS_Base       = NULL;
struct AmiGUS_CAMD       * AmiGUS_CAMD_Base  = NULL;
// And for ProgressDialogs in ReAction:
struct ClassLibrary      * FuelGaugeBase     = NULL;
struct ClassLibrary      * LabelBase         = NULL;
struct ClassLibrary      * LayoutBase        = NULL;
struct ClassLibrary      * WindowBase        = NULL;

STRPTR _LibVersionString = "$VER: " LIBRARY_IDSTRING "\r\n";

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
BOOL __ASM__ __SAVE_DS__ AmiGUS_Init(
  __REG__( a6, struct ExecBase * sysBase )
);

/**
 * This is the de-allocation routine for the AmiGUS CAMD MIDI driver.
 *
 * Unfortunately, it is not called if the initialization has failed,
 * so as a result it has to be called manually in case of an initialization
 * error.
 */
VOID __ASM__ __SAVE_DS__ AmiGUS_Expunge( VOID );

/**
 * Opens a single AmiGUS MIDI driver port and
 * tries allocating / locking the wavetable hardware.
 *
 * @param data Pointer to the struct MidiDeviceData of the driver.
 * @param portnum Number of the port to open.
 * @param transmitHandler Needs to be remembered and called whenever the
 *                        MIDI-2-AmiGUS interface is ready to digest another
 *                        byte coming from CAMD library to be played back by
 *                        the AmiGUS.
 * @param receiveHandler Needs to be remembered and called whenever a new
 *                       byte of data is received from the hardware and
 *                       shall be posted into the CAMD library for further
 *                       handling.
 *                       AmiGUS does not receive MIDI commands for the time
 *                       being.
 * @param userdata Kind of a client identifier for the CAMD library.
 *                 Needs to be remembered and used to "authenticate" calls
 *                 to transmitHandler / receiveHandler.
 *
 * @return Static AmiGUS_MidiPortData as per below if successful,
 *         NULL otherwise.
 */
APTR __ASM__ __SAVE_DS__ AmiGUS_OpenPort (
  __REG__( a3, struct MidiDeviceData * data ),
  __REG__( d0, LONG portnum ),
  __REG__( a0, TransmitFunctionType transmitHandler ), /* Transmit function  */
  __REG__( a1, ReceiveFunctionType receiveHandler ),   /* Receive function   */
  __REG__( a2, APTR userdata )
);

/**
 * Closes a single AmiGUS MIDI driver port and
 * releases the wavetable hardware.
 *
 * @param data Pointer to the struct MidiDeviceData of the driver.
 * @param portnum Number of the port to close.
 */
VOID __ASM__ __SAVE_DS__ AmiGUS_ClosePort (
  __REG__( a3, struct MidiDeviceData * data ),
  __REG__( d0, LONG portnum )
);

/**
 * Function to activate transmitter interrupt when idle.
 *
 * May also be called when new data arrives even though the "last byte" flag
 * (d1 in transmitHandler's return values) was never set.
 *
 * @param portnum Number of the port to re-activate.
 * @param userdata userdata as passed to OpenPort().
 * TODO: maybe wrong!!! drivers.doc says a0, examples say A2. maybe something in a0 too?
 */
VOID __ASM__ __SAVE_DS__ AmiGUS_ActivateXmit(
  __REG__( d0, LONG portnum ),
  __REG__( a2, APTR userdata )
);

/**
 * Special AmiGUS extension function to
 * force the CAMD driver to reload the settings and the SoundFont.
 */
VOID __ASM__ __SAVE_DS__ AmiGUS_ReloadSoundFont(
  VOID );

/**
 * Special AmiGUS extension function to
 * play a note set in the currently loaded SoundFont.
 */
VOID __ASM__ __SAVE_DS__ AmiGUS_PlayNote(
  VOID );

/**
 * Special AmiGUS extension function to
 * play a sound as provided completely in parameters to the function,
 * independently from all the SoundFont whatsoever.
 */
VOID __ASM__ __SAVE_DS__ AmiGUS_PlaySound(
  VOID );

/******************************************************************************
 * CAMD MIDI driver's global data definitions.
 *****************************************************************************/

struct MidiPortData AmiGUS_MidiPortData = { AmiGUS_ActivateXmit };

/******************************************************************************
 * CAMD MIDI driver's private functions.
 *****************************************************************************/

ULONG OpenLib( struct Library ** library,
               STRPTR name,
               ULONG version,
               ULONG error,
               BOOL fatal ) {

  *library = OpenLibrary( name, version );
  if ( *library ) {

    return ENoError;
  }

  DisplayError( error );

  if ( fatal ) {

    AmiGUS_Expunge();
  }

  return error;
}

VOID CloseLib( struct Library ** library ) {

  if (( library ) && ( *library )) {

    CloseLibrary( *library );
    *library = NULL;
  }
}

/******************************************************************************
 * CAMD MIDI driver's "public" function definitions.
 *****************************************************************************/

BOOL __ASM__ __SAVE_DS__ AmiGUS_Init(
  __REG__( a6, struct ExecBase * sysBase )
) {

  LONG error;
  struct AmiGUS_CAMD * base;

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

  base = AllocMem( sizeof( struct AmiGUS_CAMD ), MEMF_PUBLIC | MEMF_CLEAR );
  if ( !base ) {

    DisplayError( EAllocateAmiGUSCAMDBase );
    AmiGUS_Expunge();
    return FALSE;
  }
  AmiGUS_CAMD_Base = base;

  OpenLib(( APTR )&DOSBase, "dos.library", 34, EOpenDosBase, TRUE );
  if ( !( DOSBase )) {

    return FALSE;
  }
  OpenLib(( APTR )&IntuitionBase, "intuition.library", 34, EOpenDosBase, TRUE );
  if ( !( IntuitionBase )) {

    return FALSE;
  }
  OpenLib(( APTR )&AmiGUS_Base, "amigus.library", 1, EOpenAmiGusBase, TRUE );
  if ( !( AmiGUS_Base )) {

    return FALSE;
  }
  OpenLib(( struct Library ** ) &FuelGaugeBase, "gadgets/fuelgauge.gadget", 0, EOpenFuelBase, FALSE );
  OpenLib(( struct Library ** ) &LabelBase, "images/label.image", 0, EOpenLabelBase, FALSE );
  OpenLib(( struct Library ** ) &LayoutBase, "gadgets/layout.gadget", 0, EOpenLayoutBase, FALSE );
  OpenLib(( struct Library ** ) &WindowBase, "window.class", 0, EOpenWindowBase, FALSE );

#if 1

  Forbid();
  do {

    base->agb_AmiGUS = AmiGUS_FindCard( base->agb_AmiGUS );
    if ( !( base->agb_AmiGUS )) {

      error = EAmiGUSNotFound;
      continue;
    }
    if ( AMIGUS_CAMD_FIRMWARE_MINIMUM >
      base->agb_AmiGUS->agus_FirmwareRev ) {

      error = EAmiGUSFirmwareOutdated;
      continue;
    }
    error = AmiGUS_ReserveCard( base->agb_AmiGUS,
                                AMIGUS_FLAG_WAVETABLE,
                                base );

  } while (( base->agb_AmiGUS ) && ( error ));
  Permit();
#else

  error = ENoError;

#endif

  if ( error ) {

    DisplayError( error );
    AmiGUS_Expunge();
    return FALSE;
  }
  if ( base->agb_AmiGUS ) {

    base->agb_CardBase = base->agb_AmiGUS->agus_WavetableBase;

    base->agb_ProgressDialog =
      CreateProgressDialog( NULL,
                            "Please wait...",
                            "Loading AmiSF ...",
                            "Cancel" );

    LOG_D(( "D: AmiGUS_CAMD using Wavetable @ 0x%08lx on %s\n",
            base->agb_CardBase,
            base->agb_AmiGUS->agus_TypeName ));
    InitAmiGus();
  }

  LOG_D(( "D: AmiGUS_CAMD_Base ready @ 0x%08lx\n", base ));
  return TRUE;
}

VOID __ASM__ __SAVE_DS__ AmiGUS_Expunge( VOID ) {

  LOG_D(( "D: AmiGUS_CAMD_Base @ 0x%08lx leaving the building...\n",
          AmiGUS_CAMD_Base ));

  // Even if reserve failed, would not matter at all...
  AmiGUS_FreeCard( AmiGUS_CAMD_Base->agb_AmiGUS,
                   AMIGUS_FLAG_WAVETABLE,
                   AmiGUS_CAMD_Base );

  FreeProgressDialog( AmiGUS_CAMD_Base->agb_ProgressDialog );

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
  CloseLib(( struct Library ** ) &WindowBase );
  CloseLib(( struct Library ** ) &LayoutBase );
  CloseLib(( struct Library ** ) &LabelBase );
  CloseLib(( struct Library ** ) &FuelGaugeBase );

  CloseLib(( struct Library ** ) &AmiGUS_Base );
  CloseLib(( struct Library ** ) &IntuitionBase );
  CloseLib(( struct Library ** ) &DOSBase );


  if ( AmiGUS_CAMD_Base ) {

    FreeMem( AmiGUS_CAMD_Base, sizeof( struct AmiGUS_CAMD ));
  }
}

APTR __ASM__ __SAVE_DS__ AmiGUS_OpenPort (
  __REG__( a3, struct MidiDeviceData * data ),
  __REG__( d0, LONG portnum ),
  __REG__( a0, TransmitFunctionType transmitHandler ), /* Transmit function  */
  __REG__( a1, ReceiveFunctionType receiveHandler ),   /* Receive function   */
  __REG__( a2, APTR userdata )
) {

  struct AmiGUS_CAMD * base = AmiGUS_CAMD_Base;
  LONG amiSfPathVarLength;
  LONG amiSfModeVarLength;


  LOG_D(( "D: OpenPort called for "
          "data=0x%08lx port=%ld txh=0x%08lx rxh=0x%08lx user=0x%08lx\n",
          data, portnum, transmitHandler, receiveHandler, userdata ));

  // TODO: Allocate / reserve card
  // TODO: check port num
  
  base->agb_TransmitFunction = transmitHandler;
  base->agb_ReceiveFunction = receiveHandler;
  base->agb_CAMD_userdata = userdata;

  /*
   * ------------------------------------------------------
   * Part X: Prepare worker task communication. <- TODO
   * ------------------------------------------------------
   */
  base->agb_WorkerWorkSignal = -1;
  base->agb_WorkerStopSignal = -1;
  base->agb_MainProcess = ( struct Process * ) FindTask( NULL );
  base->agb_MainSignal = AllocSignal( -1 );
  if ( -1 == base->agb_MainSignal ) {

    DisplayError( EMainProcessSignalsFailed );
    return NULL;
  }
  base->agb_WorkerReady = FALSE;

  /*
   * ------------------------------------------------------
   * Part X: Create worker task. <- TODO
   * ------------------------------------------------------
   */
  LOG_D(( "D: Creating worker process for AmiGUS_CAMD_Base @ %08lx\n",
          (LONG) base ));
  if ( CreateWorkerProcess() ) {

    LOG_D(( "D: No worker, failed.\n" ));
    return NULL;
  }

  /*
   * ------------------------------------------------------
   * Part X: Load Settings
   * ------------------------------------------------------
   */

  amiSfPathVarLength = GetVar( "AmiGUS/CAMD-AmiSF-PATH",
                               base->agb_AmiSfPath,
                               sizeof( base->agb_AmiSfPath ),
                               0 );
  amiSfModeVarLength = GetVar( "AmiGUS/CAMD-AmiSF-MODE",
                               ( STRPTR ) &( base->agb_AmiSfMode ),
                               sizeof( base->agb_AmiSfMode ),
                               0 );

  LOG_D(( "D: AmiSF path var length %ld, is '%s'.\n",
          amiSfPathVarLength, base->agb_AmiSfPath ));
  LOG_D(( "D: AmiSF mode var length %ld, is '%s'.\n",
          amiSfModeVarLength, ( STRPTR ) &( base->agb_AmiSfMode )));
  if ( 0 < amiSfPathVarLength ) {

    ShowProgressDialog( base->agb_ProgressDialog );

    base->agb_AmiSf = AllocAmiSFfromFile( base->agb_AmiSfPath,
                                          base->agb_ProgressDialog );
    PrepareSamples( base->agb_AmiSf,
                    base->agb_AmiSfMode,
                    base->agb_SampleBuffer,
                    base->agb_ProgressDialog );

    CloseProgressDialog( base->agb_ProgressDialog );
  }

  LOG_D(( "D: OpenPort done, returning 0x%lx.\n",
          &AmiGUS_MidiPortData ));

  return &AmiGUS_MidiPortData;
}

VOID __ASM__ __SAVE_DS__ AmiGUS_ClosePort (
  __REG__( a3, struct MidiDeviceData * data ),
  __REG__( d0, LONG portnum )
) {

  LOG_D(( "D: ClosePort called for data=0x%08lx port=%ld\n", data, portnum ));

  // TODO: Deallocate / free card
  // TODO: check port num

  DestroyWorkerProcess();
  FreeSignal( AmiGUS_CAMD_Base->agb_MainSignal );
}

VOID __ASM__ __SAVE_DS__ AmiGUS_ActivateXmit(
  __REG__( d0, LONG portnum ),
  __REG__( a2, APTR userdata )
) {

  LOG_D(( "D: ActivateXmit userdata=0x%08lx port=%ld\n", userdata, portnum ));

  /* Signal sub task */
  if ( AmiGUS_CAMD_Base->agb_WorkerReady ) {

    Signal( (struct Task *) AmiGUS_CAMD_Base->agb_WorkerProcess,
            1 << AmiGUS_CAMD_Base->agb_WorkerWorkSignal );

  } else {

    // TODO: How do we handle worker not ready? Maybe crying? Starting worker?
    LOG_E(( "E: Worker not ready!!!" ));
  }
}

/**
 * Special AmiGUS extension function to
 * force the CAMD driver to reload the settings and the SoundFont.
 */
VOID __ASM__ __SAVE_DS__ AmiGUS_ReloadSoundFont(
  VOID ) {

  //LOG_D(( "D: AmiGUS_ReloadSoundFont triggered\n" ));
}

/**
 * Special AmiGUS extension function to
 * play a note set in the currently loaded SoundFont.
 */
VOID __ASM__ __SAVE_DS__ AmiGUS_PlayNote(
  VOID ) {

  LOG_D(( "D: AmiGUS_PlayNote triggered\n" ));
}

VOID __ASM__ __SAVE_DS__ AmiGUS_PlaySound(
  VOID ) {

  LOG_D(( "D: AmiGUS_PlaySound triggered\n" ));
}
