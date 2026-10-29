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
#include <exec/ports.h>

#include <proto/dos.h>
#include <proto/exec.h>

#include "amigus_camd.h"
#include "amigus_hardware.h"
#include "amigus_ports.h"
#include "amigus_wavetable.h"
#include "amisf.h"
#include "camd_interpreter.h"
#include "debug.h"
#include "errors.h"
#include "support.h"
#include "worker.h"

#define MESSAGE_MIDI_CHANNEL             15
#define PLAY_STANDALONE_SAMPLE_START     0

BOOL HandleMidi( VOID ) {

  struct AmiGUS_CAMD * base = AmiGUS_CAMD_Base;
  ULONG bufferEmpty;
  ULONG midiByte;
  struct MidiMessage midiMessage;
  BOOL complete;

  do {

    midiByte = base->agb_TransmitFunction( base->agb_CAMD_userdata );
    bufferEmpty = GET_REG( REG_D1 );

    complete = TokenizeMidiByte( midiByte, &midiMessage );

  } while (( !( bufferEmpty )) && ( !( complete )));
  LOG_INT(( "WORKER: Received data=0x%02lx empty=0x%02lx\n",
            midiByte, bufferEmpty ));
  
  return ( BOOL ) ( !( bufferEmpty ));
}

VOID HandleMessage( struct Message * message ) {
  
  LOG_INT(( "WORKER: Got message type %ld name %s\n",
            message->mn_Node.ln_Type,
            message->mn_Node.ln_Name ));

  switch ( *(( LONG * ) message->mn_Node.ln_Name )) {
    case PLAY_STANDALONE_MESSAGE_NAME: {

      struct PlayStandaloneSampleMessage * sampleMessage =
        ( struct PlayStandaloneSampleMessage * ) message;
      UBYTE channel = AllocateNextAmiGusChannel( MESSAGE_MIDI_CHANNEL );

      LOG_D(( "D: Got sample @ 0x%08lx and size %ld BYTEs\n",
              sampleMessage->pssm_SampleData,
              sampleMessage->pssm_EndOffset ));

      LoadAmiGusWavetableSample( sampleMessage->pssm_SampleData,
                                 PLAY_STANDALONE_SAMPLE_START,
                                 sampleMessage->pssm_EndOffset );
      StartAmiGusWavetablePlayback( channel,
                                    PLAY_STANDALONE_SAMPLE_START,
                                    sampleMessage->pssm_LoopOffset,
                                    sampleMessage->pssm_EndOffset,
                                    sampleMessage->pssm_PlaybackRate,
                                    sampleMessage->pssm_PlaybackVolume,
                                    sampleMessage->pssm_PlaybackFlags,
                                    sampleMessage->pssm_Attack,
                                    sampleMessage->pssm_Decay,
                                    sampleMessage->pssm_Sustain,
                                    sampleMessage->pssm_Release );

      LOG_D(( "D: Replying PlayStandaloneSampleMessage 0x%08lx...\n",
              message ));
      ReplyMsg( message );
      break;
    }
    case PLAY_AMISF_SAMPLE_MESSAGE_NAME: {

      struct PlayAmiSfSampleMessage * sampleMessage =
        ( struct PlayAmiSfSampleMessage * ) message;
      UBYTE channel = AllocateNextAmiGusChannel( MESSAGE_MIDI_CHANNEL );

      const UBYTE bank = sampleMessage->passm_Bank;
      const UBYTE preset = sampleMessage->passm_Preset;
      const UBYTE note = sampleMessage->passm_Note;
      const UBYTE velocity = sampleMessage->passm_Velocity;

      struct AmiSF * amisf = sampleMessage->passm_AmiSf;
      struct AmiSF_Preset * sfPreset = GetAmiSfPreset( amisf, bank, preset );
      const LONG noteIndex = GetAmiSfNoteIndex( amisf, sfPreset, note );
      struct AmiSF_Note * sfNote = GetAmiSfNote( amisf, noteIndex );
      struct AmiSF_Sample * sfSample = GetAmiSfSample( amisf, sfNote );
      ULONG playbackRate = GetPlaybackRate( amisf, sfNote, note );
      ULONG sampleDataSize =
        sfSample->asfs_EndOffset - sfSample->asfs_StartOffset;
      // Automatically limited to 127 * 127 + 2 * 127 = 16383 = 0x3FFF :D
      UWORD playbackVolume = GetPlaybackVolume( velocity );

      LOG_D(( "D: Got PlayAmiSfSampleMessage @ 0x%08lx for %ld-%ld-%ld-%ld, "
              "preset 0x%08lx note 0x%08lx sample 0x%08lx, "
              "data 0x%08lx and size %ld BYTEs\n",
              message, bank, preset, note, velocity,
              sfPreset, sfNote, sfSample,
              sampleMessage->passm_SampleData, sampleDataSize ));

      LoadAmiGusWavetableSample( sampleMessage->passm_SampleData,
                                 sfSample->asfs_StartOffset,
                                 sampleDataSize );
      StartAmiGusWavetablePlayback( channel,
                                    sfSample->asfs_StartOffset,
                                    sfSample->asfs_LoopOffset,
                                    sfSample->asfs_EndOffset,
                                    playbackRate,
                                    playbackVolume,
                                    sfSample->asfs_PlaybackFlags,
                                    sfNote->asfn_Attack,
                                    sfNote->asfn_Decay,
                                    sfNote->asfn_Sustain,
                                    sfNote->asfn_Release );

      LOG_D(( "D: Replying PlayAmiSfSampleMessage 0x%08lx...\n",
              message ));
      ReplyMsg( message );
      break;
    }
    case STOP_SAMPLE_MESSAGE_NAME: {

      UBYTE channel = GetUsedAmiGusChannel( MESSAGE_MIDI_CHANNEL );
      StopAmiGusWavetablePlayback( channel );
      LOG_D(( "D: Replying StopSampleMessage 0x%08lx...\n", message ));
      ReplyMsg( message );
      break;
    }
    case LOAD_SOUNDFONT_NESSAGE_NAME: {

      LOG_D(( "D: Replying LoadSoundFontMessage 0x%08lx...\n", message ));
      ReplyMsg( message );
      break;
    }
    case RELOAD_SETTINGS_MESSAGE_NAME: {

      LOG_D(( "D: Replying ReloadSettingsMessage 0x%08lx...\n", message ));
      ReplyMsg( message );
      break;
    }
    default: {

      LOG_INT(( "WORKER: Unknown message...\n" ));
      break;
    }
  }
}

// TODO: Do we need __entry for vbcc here?
__SAVE_DS__ VOID WorkerProcess( VOID ) {

  struct AmiGUS_CAMD * base = AmiGUS_CAMD_Base;

  LOG_D(( "D: Worker for AmiGUS_CAMD_Base @ %08lx starting...\n",
          (LONG) base ));

  base->agb_WorkerWorkSignal = AllocSignal( -1 );
  base->agb_WorkerStopSignal = AllocSignal( -1 );
  base->agb_WorkerPort = CreatePort( "AmiGUS CAMD Port", 0 );

  LOG_D(( "D: Worker work signal is 0x%08lx\n",
    1 << base->agb_WorkerWorkSignal ));
  LOG_D(( "D: Worker stop signal is 0x%08lx\n",
    1 << base->agb_WorkerStopSignal ));
  LOG_D(( "D: Worker message signal is 0x%08lx\n",
    1 << base->agb_WorkerPort->mp_SigBit ));

  if ( ( -1 != base->agb_WorkerWorkSignal )
    && ( -1 != base->agb_WorkerStopSignal )
    && ( base->agb_WorkerPort )) {

    const ULONG workerSignal = ( 1 << base->agb_WorkerWorkSignal );
    const ULONG messageSignal = ( 1 << base->agb_WorkerPort->mp_SigBit );
    const ULONG stopSignals = ( 1 << base->agb_WorkerStopSignal )
                            | SIGBREAKF_CTRL_C;
    const ULONG allSignals = workerSignal | messageSignal | stopSignals;

    ULONG signals = workerSignal;

    /* Tell master worker is alive */
    Signal(( struct Task * ) base->agb_MainProcess, 1 << base->agb_MainSignal );
    // SetSignal(0, 1 << agb_WorkerStopSignal );
    while ( TRUE ) {

      LOG_INT(( "WORKER: Beginning main loop\n" ));
      if ( workerSignal & signals ) {

        LOG_INT(( "WORKER: Beginning MIDI loop\n" ));
        while ( HandleMidi( ));
        LOG_INT(( "WORKER: Ending MIDI loop\n" ));
      }

      if ( messageSignal & signals ) {

        struct Message * message;

        LOG_INT(( "WORKER: Beginning message loop\n" ));
        while ( message = GetMsg( base->agb_WorkerPort )) {

          HandleMessage( message );
        }
        LOG_INT(( "WORKER: Ending message loop\n" ));
      }
      LOG_INT(( "WORKER: Going to sleep...\n" ));

      base->agb_WorkerReady = TRUE;
      signals = Wait( allSignals );

      // Activating the log line below breaks the driver... no clue why
      // LOG_INT(( "WORKER: Woke up, signals are 0x%08lx\n", signals ));
      /* 
       All signals break the wait,
       but "stop" and "CTRL-C" break the playback loop.
       */
      if ( stopSignals & signals ) {

        break;
      }
    }
  } else {
    /* Well... */
    DisplayError( EWorkerProcessSignalsFailed );
  }
  LOG_D(( "D: Worker for AmiGUS_CAMD_Base @ %08lx ending...\n",
          (LONG) base ));

  DeletePort( base->agb_WorkerPort );
  base->agb_WorkerPort = NULL;
  FreeSignal( base->agb_WorkerWorkSignal );
  base->agb_WorkerWorkSignal = -1;
  FreeSignal( base->agb_WorkerStopSignal );
  base->agb_WorkerStopSignal = -1;

  /* Stop multitasking here - master will resume it TODO: ??? */
  //  Forbid();

  Signal(( struct Task * ) base->agb_MainProcess, 1 << base->agb_MainSignal );
  base->agb_WorkerProcess = NULL;
  base->agb_WorkerReady = FALSE;
  LOG_D(( "D: Worker for AmiGUS_CAMD_Base @ %08lx ended.\n", ( LONG ) base ));
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
    CreateNewProcTags( NP_Entry, ( ULONG ) &WorkerProcess,
                       NP_Name, ( ULONG ) ( STR( LIB_FILE ) " CAMD" ),
                       NP_Priority, ( ULONG ) 127,
                       TAG_DONE, 0 );

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
