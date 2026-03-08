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
#include <midi/camd.h>

#include <proto/camd.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include "camd_keyboard.h"
#include "debug.h"
#include "errors.h"
#include "support.h"
#include "worker.h"

/*__entry for vbcc*/ SAVEDS VOID WorkerProcess( VOID ) {

  struct CAMD_Keyboard * base = CAMD_Keyboard_Base;
  ULONG signals = TRUE;

  LOG_D(( "D: Worker for CAMD_Keyboard_Base @ %08lx starting...\n",
          (LONG) base ));

  base->ck_WorkerWorkSignal = AllocSignal( -1 );
  base->ck_WorkerStopSignal = AllocSignal( -1 );
  
  if ( ( -1 != base->ck_WorkerWorkSignal )
    && ( -1 != base->ck_WorkerStopSignal )) {

    /* Tell master worker is alive */
    Signal(
      (struct Task *) base->ck_MainProcess,
      1 << base->ck_MainSignal
    );
    // SetSignal(0, 1 << ck_WorkerStopSignal );
    while ( signals ) {

      MidiMsg message;

      while ( GetMidi( base->ck_MidiNode, &message )) {

        LOG_INT(( "WORKER: Received message=0x %08lx %08lx\n",
                  message.l[ 0 ], message.l[ 1 ] ));

      }
      LOG_INT(( "WORKER: Going to sleep...\n" ));

      base->ck_WorkerReady = TRUE;
      signals = Wait(
          SIGBREAKF_CTRL_C
        | ( 1 << base->ck_WorkerWorkSignal )
        | ( 1 << base->ck_WorkerStopSignal )
      );
      /* 
       All signals break the wait, 
       but only "work" continues the playback loop, 
       so the others are masked away.
       */
       signals &= ( 1 << base->ck_WorkerWorkSignal );
    }
  } else {
    /* Well... */
    DisplayError( EWorkerProcessSignalsFailed );
  }
  LOG_D(( "D: Worker for CAMD_Keyboard_Base @ %08lx ending...\n",
          (LONG) base ));

  FreeSignal( base->ck_WorkerWorkSignal );
  base->ck_WorkerWorkSignal = -1;
  FreeSignal( base->ck_WorkerStopSignal );
  base->ck_WorkerStopSignal = -1;

  /* Stop multitasking here - master will resume it TODO: ??? */
  //  Forbid();

  Signal(
      (struct Task *) base->ck_MainProcess,
      1 << base->ck_MainSignal
  );
  base->ck_WorkerProcess = NULL;
  base->ck_WorkerReady = FALSE;
  LOG_D(( "D: Worker for CAMD_Keyboard_Base @ %08lx ended.\n",
          (LONG) base ));
}

/*
 * TRUE = failure
 */
BOOL CreateWorkerProcess( VOID ) {
  
  struct CAMD_Keyboard * base = CAMD_Keyboard_Base;

  /* Prevent worker from waking up until we are ready. */
  Forbid();
  if ( base->ck_WorkerProcess ) {

    Permit();
    LOG_D(("D: Worker already exists!\n"));
    return FALSE;
  }
// TODO: Use CreateProc for 1.3
  base->ck_WorkerProcess =
      CreateNewProcTags( NP_Entry, (ULONG) &WorkerProcess,
                         NP_Name, (ULONG) ( STR( APP_NAME ) " CAMD" ),
                         NP_Priority, (ULONG) 127,
                         TAG_DONE, 0 
                       );
  if ( base->ck_WorkerProcess ) {

    base->ck_WorkerProcess->pr_Task.tc_UserData = base;

  } /* Potential error handling later */

  /* Well... Worker can only wake up if we ... */
  Permit();

  if ( base->ck_WorkerProcess ) {
    /* Wait for worker signalling main */
    Wait( 1 << base->ck_MainSignal );
    /* Check if worker is alive or dead */
    if ( !base->ck_WorkerProcess ) {
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

  struct CAMD_Keyboard * base = CAMD_Keyboard_Base;

  if ( !base->ck_WorkerProcess ) {

    LOG_D(("D: No worker process to destroy!\n"));
    return;
  }

  LOG_D(("D: Destroying worker process\n"));

  if ( -1 != base->ck_WorkerStopSignal ) {

    /* Kill the playback worker to stop the playback */
    Signal(( struct Task * ) base->ck_WorkerProcess,
          1 << base->ck_WorkerStopSignal );

    /* Wait for the worker to die and imply Permit() */
    Wait( 1 << base->ck_MainSignal );
  }
  if ( !base->ck_WorkerProcess ) {
    LOG_D(("D: Destroyed worker process\n"));
  }
}
