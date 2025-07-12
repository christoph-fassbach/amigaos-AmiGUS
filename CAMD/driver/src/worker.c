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

#include <dos/dostags.h>

#include <proto/dos.h>
#include <proto/exec.h>

#include "amigus_camd.h"
#include "amigus_hardware.h"
#include "debug.h"
#include "errors.h"
#include "support.h"
#include "worker.h"

/*__entry for vbcc*/ SAVEDS VOID WorkerProcess( VOID ) {

  ULONG signals = TRUE;

  LOG_D(( "D: Worker for AmiGUS_CAMD_Base @ %08lx starting...\n",
          (LONG) AmiGUS_CAMD_Base ));

  AmiGUS_CAMD_Base->agb_WorkerWorkSignal = AllocSignal( -1 );
  AmiGUS_CAMD_Base->agb_WorkerStopSignal = AllocSignal( -1 );
  
  if ( ( -1 != AmiGUS_CAMD_Base->agb_WorkerWorkSignal )
    && ( -1 != AmiGUS_CAMD_Base->agb_WorkerStopSignal )) {

    /* Tell master worker is alive */
    Signal(
      (struct Task *) AmiGUS_CAMD_Base->agb_MainProcess,
      1 << AmiGUS_CAMD_Base->agb_MainSignal
    );
    // SetSignal(0, 1 << agb_WorkerStopSignal );
    while ( signals ) {

      ULONG bufferEmpty = FALSE;
      ULONG midiData;
      while ( !bufferEmpty ) {

        midiData = AmiGUS_CAMD_Base->agb_TransmitFunction(
          AmiGUS_CAMD_Base->agb_CAMD_userdata );
        bufferEmpty = GET_REG( REG_D1 );

        LOG_INT(( "WORKER: Received data=0x%02lx empty=0x%02lx\n",
                  midiData, bufferEmpty ));

// TODO: Translate MIDI information to AmiGUS actions and execute them!
      }
      LOG_INT(( "WORKER: Going to sleep...\n" ));

      AmiGUS_CAMD_Base->agb_WorkerReady = TRUE;
      signals = Wait(
          SIGBREAKF_CTRL_C
        | ( 1 << AmiGUS_CAMD_Base->agb_WorkerWorkSignal )
        | ( 1 << AmiGUS_CAMD_Base->agb_WorkerStopSignal )
      );
      /* 
       All signals break the wait, 
       but only "work" continues the playback loop, 
       so the others are masked away.
       */
       signals &= ( 1 << AmiGUS_CAMD_Base->agb_WorkerWorkSignal );
    }
  } else {
    /* Well... */
    DisplayError( EWorkerProcessSignalsFailed );
  }
  LOG_D(( "D: Worker for AmiGUS_CAMD_Base @ %08lx ending...\n",
          (LONG) AmiGUS_CAMD_Base ));

  FreeSignal( AmiGUS_CAMD_Base->agb_WorkerWorkSignal );
  AmiGUS_CAMD_Base->agb_WorkerWorkSignal = -1;
  FreeSignal( AmiGUS_CAMD_Base->agb_WorkerStopSignal );
  AmiGUS_CAMD_Base->agb_WorkerStopSignal = -1;

  /* Stop multitasking here - master will resume it TODO: ??? */
  //  Forbid();

  Signal(
      (struct Task *) AmiGUS_CAMD_Base->agb_MainProcess,
      1 << AmiGUS_CAMD_Base->agb_MainSignal
  );
  AmiGUS_CAMD_Base->agb_WorkerProcess = NULL;
  AmiGUS_CAMD_Base->agb_WorkerReady = FALSE;
  LOG_D(( "D: Worker for AmiGUS_CAMD_Base @ %08lx ended.\n",
          (LONG) AmiGUS_CAMD_Base ));
}

/*
 * TRUE = failure
 */
BOOL CreateWorkerProcess( VOID ) {

  /* Prevent worker from waking up until we are ready. */
  Forbid();
  if ( AmiGUS_CAMD_Base->agb_WorkerProcess ) {

    Permit();
    LOG_D(("D: Worker already exists!\n"));
    return FALSE;
  }
// TODO: Use CreateProc for 1.3
  AmiGUS_CAMD_Base->agb_WorkerProcess =
      CreateNewProcTags( NP_Entry, (ULONG) &WorkerProcess,
                         NP_Name, (ULONG) ( STR( LIB_FILE ) " CAMD" ),
                         NP_Priority, (ULONG) 127,
                         TAG_DONE, 0 
                       );
  if ( AmiGUS_CAMD_Base->agb_WorkerProcess ) {

    AmiGUS_CAMD_Base->agb_WorkerProcess->pr_Task.tc_UserData = AmiGUS_CAMD_Base;

  } /* Potential error handling later */

  /* Well... Worker can only wake up if we ... */
  Permit();

  if ( AmiGUS_CAMD_Base->agb_WorkerProcess ) {
    /* Wait for worker signalling main */
    Wait( 1 << AmiGUS_CAMD_Base->agb_MainSignal );
    /* Check if worker is alive or dead */
    if ( !AmiGUS_CAMD_Base->agb_WorkerProcess ) {
      /* Worker died meanwhile */
      DisplayError( EWorkerProcessDied );
      return TRUE;
    }
  } else {
    /* Handling worker never really been created. */
    DisplayError( EWorkerProcessCreationFailed );
    return TRUE;
  }
}

VOID DestroyWorkerProcess(VOID) {

  if ( !AmiGUS_CAMD_Base->agb_WorkerProcess ) {

    LOG_D(("D: No worker process to destroy!\n"));
    return;
  }

  LOG_D(("D: Destroying worker process\n"));

  if ( -1 != AmiGUS_CAMD_Base->agb_WorkerStopSignal ) {

    /* Kill the playback worker to stop the playback */
    Signal(( struct Task * ) AmiGUS_CAMD_Base->agb_WorkerProcess,
          1 << AmiGUS_CAMD_Base->agb_WorkerStopSignal );

    /* Wait for the worker to die and imply Permit() */
    Wait( 1 << AmiGUS_CAMD_Base->agb_MainSignal );
  }
  if ( !AmiGUS_CAMD_Base->agb_WorkerProcess ) {
    LOG_D(("D: Destroyed worker process\n"));
  }
}
