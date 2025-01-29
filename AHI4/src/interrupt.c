/*
 * This file is part of the AmiGUS.audio driver.
 *
 * AmiGUS.audio driver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS.audio driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with AmiGUS.audio driver.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <exec/interrupts.h>
#include <hardware/intbits.h>

#include <proto/exec.h>

#include "amigus_ahi_sub.h"
#include "amigus_hardware.h"
#include "debug.h"
#include "interrupt.h"

INLINE VOID HandlePlayback( struct AmiGUSAhiDriverData * driverData ) {

  struct AmiGUSPcmPlayback * playback = &( driverData->agdd_Playback );
  const APTR cardBase = driverData->agdd_Card->agpc_CardBase;

  ULONG *current = &( playback->agpp_CurrentBuffer );
  BOOL canSwap = TRUE;
  LONG copied = 0;        /* Sum of BYTEs actually filled into FIFO this run */
  const ULONG watermark = playback->agpp_Watermark;
  const LONG reminder =         /* Read-back remaining FIFO samples in BYTES */
    ReadReg16( cardBase, AMIGUS_PCM_PLAY_FIFO_USAGE ) << 1;
  const LONG minHwSampleSize =  /* Size of a single (mono / stereo) sample in BYTEs*/
    AmiGUSPlaybackSampleSizes[ playback->agpp_HwSampleFormatId ];

  /* Target amount of BYTEs to fill into FIFO during this interrupt run,     */
  const LONG target =                               /* taken from watermark, */
    ( watermark << 2 )      /* converted <<1 to BYTEs, want <<1 2x watermark */
    - reminder                                      /* deduct remaining FIFO */
    - minHwSampleSize;         /* and provide headroom for ALL sample sizes! */

  LOG_INT(( "INT: Playback pre bufs i0 %5ld m0 %5ld i1 %5ld m1 %5ld\n",
            playback->agpp_BufferIndex[ 0 ],
            playback->agpp_BufferMax[ 0 ],
            playback->agpp_BufferIndex[ 1 ],
            playback->agpp_BufferMax[ 1 ] ));

  while ( copied < target ) {

    if ( playback->agpp_BufferIndex[ *current ] 
        < playback->agpp_BufferMax[ *current ] ) {

      copied += (* playback->agpp_CopyFunction )(
        cardBase,
        playback->agpp_Buffer[ *current ],
        &( playback->agpp_BufferIndex[ *current ] ));

    } else if ( canSwap ) {

      *current ^= 0x00000001;
      canSwap = FALSE;

    } else {

      // Playback buffers empty, but FIFO could take more. - Not so good, Al...
      break;
    }
  }
  WriteReg16( cardBase, AMIGUS_PCM_PLAY_FIFO_WATERMARK, watermark );
  LOG_INT(( "INT: Playback t %4ld c %4ld r %4ld wm %4ld wr %ld\n",
            target,
            copied,
            reminder,
            watermark,
            AmiGUSBase->agb_WorkerReady ));
}

INLINE VOID HandleRecording( struct AmiGUSAhiDriverData * driverData ) {

  struct AmiGUSPcmRecording * recording = &( driverData->agdd_Recording );
  APTR cardBase = driverData->agdd_Card->agpc_CardBase;

  ULONG *current = &( recording->agpr_CurrentBuffer );
  BOOL canSwap = TRUE;
  LONG copied = 0;        /* Sum of BYTEs actually filled into FIFO this run */
  /* Read-back remaining FIFO samples in BYTES */
  LONG target = ReadReg16( cardBase, AMIGUS_PCM_REC_FIFO_USAGE ) << 1;              

  while ( copied < target ) {

// TODO: may write over buffer end!!!! Somehow subtract AHI sample output inc and take care worker can handle
    if ( recording->agpr_BufferIndex[ *current ]
        < recording->agpr_BufferMax[ *current ] ) {

      copied += (* recording->agpr_CopyFunction )(
        cardBase,
        recording->agpr_Buffer[ *current ],
        &( recording->agpr_BufferIndex[ *current ] ));

    } else if ( canSwap ) {

      *current ^= 0x00000001;
      canSwap = FALSE;

    } else {

      // Recording buffers full, but FIFO has had more. - Not so good, Al...
      break;
    }
  }

  LOG_INT((
    "INT: Recording t %4ld c %4ld wm %4ld wr %ld b%ld-i %ld\n",
    target,
    copied,
    ReadReg16( cardBase, AMIGUS_PCM_REC_FIFO_WATERMARK ),
    AmiGUSBase->agb_WorkerReady,
    *current,
    recording->agpr_BufferIndex[ *current ] ));
}

INLINE LONG HandleCard( struct AmiGUSPcmCard * card ) {

  const APTR cardBase = card->agpc_CardBase;
  const UWORD status = ReadReg16( cardBase, AMIGUS_PCM_MAIN_INT_CONTROL );
  if ( !( status & ( AMIGUS_INT_F_PLAY_FIFO_EMPTY
                   | AMIGUS_INT_F_PLAY_FIFO_WATERMARK
                   | AMIGUS_INT_F_REC_FIFO_FULL
                   | AMIGUS_INT_F_REC_FIFO_WATERMARK ) ) ) {

    return 0;
  }
  LOG_INT(( "INT: Handling card 0x%08lx status 0x%04lx flags 0x%02lx\n",
            cardBase, status, card->agpc_StateFlags ));
  if ( AMIGUS_AHI_F_PLAY_STARTED & card->agpc_StateFlags ) {

    HandlePlayback(( struct AmiGUSAhiDriverData * )
      card->agpc_PlaybackCtrl->ahiac_DriverData );

    if ( status & AMIGUS_INT_F_PLAY_FIFO_EMPTY ) {

      /*
       Recovery from buffer underruns is a bit tricky.
       DMA will stay disabled until worker task prepared some buffers and
       triggered a full playback init cycle to make us run again.
      */
      card->agpc_StateFlags |= AMIGUS_AHI_F_PLAY_UNDERRUN;
      LOG_INT(( "INT: Underrun - new flags 0x%02lx\n",
                card->agpc_StateFlags ));
    }
  }
  if ( AMIGUS_AHI_F_REC_STARTED & card->agpc_StateFlags ) {

    HandleRecording(( struct AmiGUSAhiDriverData * )
      card->agpc_RecordingCtrl->ahiac_DriverData );
  }

  /* Clear AmiGUS control flags here!!! */
  WriteReg16( cardBase,
              AMIGUS_PCM_MAIN_INT_CONTROL,
              AMIGUS_INT_F_CLEAR
            | AMIGUS_INT_F_PLAY_FIFO_EMPTY
            | AMIGUS_INT_F_PLAY_FIFO_WATERMARK
            | AMIGUS_INT_F_REC_FIFO_FULL
            | AMIGUS_INT_F_REC_FIFO_WATERMARK );
  /* Signal sub task */
  if ( AmiGUSBase->agb_WorkerReady ) {

    Signal( (struct Task *) AmiGUSBase->agb_WorkerProcess,
            1 << AmiGUSBase->agb_WorkerWorkSignal );

  } else {

    // TODO: How do we handle worker not ready here? Maybe crying?
  }

  return 1;
}

ASM(LONG) /* __entry for vbcc ? */ SAVEDS INTERRUPT handleInterrupt (
  REG(a1, struct AmiGUSBasePrivate * amiGUSBase)
) {

  struct AmiGUSPcmCard * card = ( struct AmiGUSPcmCard * )
    AmiGUSBase->agb_CardList.mlh_Head;
  LONG result = 0;
  while ( card->agpc_Node.mln_Succ ) {

    result |= HandleCard( card );
    card = ( struct AmiGUSPcmCard * ) card->agpc_Node.mln_Succ;
  }
  return result;
}

// TRUE = failure
BOOL CreateInterruptHandler( VOID ) {

  if (AmiGUSBase->agb_Interrupt) {

    LOG_D(("D: INT server in use!\n"));
    return FALSE;
  }

  LOG_D(("D: Creating INT server\n"));
  Disable();

  AmiGUSBase->agb_Interrupt = ( struct Interrupt * )
      AllocMem(
          sizeof( struct Interrupt ),
          MEMF_CLEAR | MEMF_PUBLIC
      );
  if ( AmiGUSBase->agb_Interrupt ) {

    AmiGUSBase->agb_Interrupt->is_Node.ln_Pri = 100;
    AmiGUSBase->agb_Interrupt->is_Node.ln_Name = "AMIGUS_AHI_INT";
    AmiGUSBase->agb_Interrupt->is_Data = AmiGUSBase;
    AmiGUSBase->agb_Interrupt->is_Code = ( VOID(*)() ) handleInterrupt;

    AddIntServer( INTB_PORTS, AmiGUSBase->agb_Interrupt );

    Enable();

    LOG_D(("D: Created INT server\n"));
    return FALSE;
  }

  Enable();
  LOG_D(("D: Failed creating INT server\n"));
  // TODO: Display error?
  return TRUE;
}

VOID DestroyInterruptHandler( VOID ) {

  if ( !AmiGUSBase->agb_Interrupt ) {

    LOG_D(("D: No INT server to destroy!\n"));
    return;
  }
  
  LOG_D(("D: Destroying INT server\n"));

  Disable();
  RemIntServer( INTB_PORTS, AmiGUSBase->agb_Interrupt );
  Enable();

  FreeMem( AmiGUSBase->agb_Interrupt, sizeof( struct Interrupt ));
  AmiGUSBase->agb_Interrupt = NULL;

  LOG_D(("D: Destroyed INT server\n"));
}
