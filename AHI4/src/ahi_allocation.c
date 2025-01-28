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

#include <exec/libraries.h>
#include <proto/exec.h>
#include <proto/utility.h>

#include "amigus_ahi_sub.h"
#include "amigus_hardware.h"
#include "debug.h"
#include "errors.h"
#include "samplerate.h"
#include "support.h"
#include "SDI_ahi_sub.h"

/* Basic functions - Allocation */

ASM(ULONG) SAVEDS AHIsub_AllocAudio(
  REG(a1, struct TagItem* aTagList),
  REG(a2, struct AHIAudioCtrlDrv* aAudioCtrl)
) {

  struct AmiGUSAhiDriverData *driverData;
  struct AmiGUSPcmPlayback *playback;
  struct AmiGUSPcmRecording *recording;
  struct TagItem *stateTag = 0;
  struct TagItem *tag;
  ULONG sampleRateId;
  ULONG sampleRate;
  WORD playHwSampleFormatId = -1;
  WORD recHwSampleFormatId = -1;
  BYTE playbackCopyFunctionId = -1;
  BYTE recordingCopyFunctionId = -1;
  UBYTE isStereo = FALSE;
  UBYTE isHifi = FALSE;
  UBYTE isRealtime = FALSE;
  UBYTE canRecord = FALSE;
  UBYTE bitsPerAmiGusSample = 0;
  BYTE playSampleBytesShift = -1;
  BYTE recSampleBytesShift = -1;
  struct AmiGUSPcmCard *card =
    ( struct AmiGUSPcmCard * ) AmiGUSBase->agb_CardList.mlh_Head;

  /* 
   * Will rely on AHI provided mixing and timing,
   * trying out AHIsub_AllocAudio -> 1.
   */
  ULONG result = AHISF_MIXING | AHISF_TIMING;

  LOG_D(( "D: AHIsub_AllocAudio start\n" ));
  LogTicks( 0x03 );

  /*
   * ------------------------------------------------------
   * Part 1: Allocate this AmiGUS to exactly this callee,
   *         re-entrance/interrupt/multitasking safe.
   * ------------------------------------------------------
   */
  Disable();
  while (( card->agpc_PlaybackCtrl ) && ( card->agpc_RecordingCtrl )) {

    if ( !( card->agpc_Node.mln_Succ ) ) {

      Enable();
      DisplayError( EDriverInUse );
      return AHISF_ERROR;
    }
    card = ( struct AmiGUSPcmCard * )card->agpc_Node.mln_Succ;
  }
  Enable();
  LOG_D(( "D: Alloc`ed AmiGUS AHI hardware\n" ));

  /*
   * ------------------------------------------------------
   * Part 2: Extract audio mode information
   * ------------------------------------------------------
   */
  /* Find nearest supported frequency and provide it back */
  sampleRateId = FindSampleRateIdForValue( aAudioCtrl->ahiac_MixFreq );
  sampleRate = FindSampleRateValueForId( sampleRateId );
  LOG_V(("D: Using %ldHz = 0x%02lx for requested %ldHz\n",
         sampleRate,
         sampleRateId,
         aAudioCtrl->ahiac_MixFreq));
  aAudioCtrl->ahiac_MixFreq = sampleRate;

  /* Parse aTagList */
  stateTag = aTagList;
  while ( tag = NextTagItem( &stateTag ) ) {

    switch ( tag->ti_Tag ) {
      case AHIDB_Bits: {
        bitsPerAmiGusSample = (UBYTE)tag->ti_Data;
        break;
      }
      case AHIDB_Stereo: {
        isStereo = (UBYTE)tag->ti_Data;
        if ( isStereo ) {

          result |= AHISF_KNOWSTEREO;
        }
        break;
      }
      case AHIDB_HiFi: {
        isHifi = ( UBYTE )tag->ti_Data;
        if ( isHifi ) {

          result |= AHISF_KNOWHIFI;
        }
        break;
      }
      case AHIDB_Realtime: {
        isRealtime = ( UBYTE )tag->ti_Data;
        break;
      }
      case AHIDB_AmiGUS_PlayCopyFunction: {
        playbackCopyFunctionId = ( BYTE )tag->ti_Data;
        break;
      }
      case AHIDB_AmiGUS_PlayHwSampleId: {
        playHwSampleFormatId = ( WORD )tag->ti_Data;
        break;
      }
      case AHIDB_AmiGUS_PlaySampleShift: {
        playSampleBytesShift = ( BYTE )tag->ti_Data;
        break;
      }
      case AHIDB_AmiGUS_RecCopyFunction: {
        recordingCopyFunctionId = ( BYTE )tag->ti_Data;
        break;
      }
      case AHIDB_AmiGUS_RecHwSampleId: {
        recHwSampleFormatId = ( WORD )tag->ti_Data;
        break;
      }
      case AHIDB_AmiGUS_RecSampleShift: {
        recSampleBytesShift = ( BYTE )tag->ti_Data;
        break;
      }
      case AHIDB_Record: {
        canRecord = ( UBYTE )tag->ti_Data;
        if ( canRecord ) {

          result |= AHISF_CANRECORD;

        } else {

          result &= ( ~ AHISF_CANRECORD );
        }
        break;
      }
      default: {
        break;
      }
    }
  }

  /*
   * ------------------------------------------------------
   * Part 3: Apply information to AmiGUS & driver.
   * ------------------------------------------------------
   */
  LOG_I(( "I: AmiGUS playback mode is format 0x%lx, %ldbit, %ld stereo, "
          "%ld HiFi, %ld Realtime, %ldHz, %ld Recording\n",
          playHwSampleFormatId,
          bitsPerAmiGusSample, isStereo,
          isHifi, isRealtime, sampleRate, canRecord ));
  LOG_I(( "I: AHI playback sample size %ld BYTEs, "
          "conversion AHI samples<->BYTEs by %ld shifts, "
          "AmiGUS sample size %ld BYTEs\n",
          ( 1 << playSampleBytesShift ),
          playSampleBytesShift,
          AmiGUSPlaybackSampleSizes[ playHwSampleFormatId  ] ));
  LOG_I(( "I: AmiGUS recording mode is format 0x%lx, %ldbit, %ld stereo, "
          "%ld HiFi, %ld Realtime, %ldHz, %ld Recording\n",
          recHwSampleFormatId,
          bitsPerAmiGusSample, isStereo,
          isHifi, isRealtime, sampleRate, canRecord ));
  LOG_I(( "I: AHI recording sample size %ld BYTEs, "
          "conversion AHI samples<->BYTEs by %ld shifts, "
          "AmiGUS sample size %ld BYTEs\n",
          ( 1 << recSampleBytesShift ),
          recSampleBytesShift,
          AmiGUSPlaybackSampleSizes[ recHwSampleFormatId  ] )); // TODO: wrong - IDs misaligned!!

  if (( 0 > playHwSampleFormatId ) || ( 0 > recHwSampleFormatId )) {

    DisplayError( ESampleFormatMissingFromMode );
    return AHISF_ERROR;
  }
  if (( 0 > playbackCopyFunctionId ) || ( 0 > recordingCopyFunctionId )) {

    DisplayError( ECopyFunctionMissingFromMode );
    return AHISF_ERROR;
  }
  if (( 0 > playSampleBytesShift ) || ( 0 > recSampleBytesShift )) {

    DisplayError( EShiftMissingFromMode );
    return AHISF_ERROR;
  }

  driverData = AllocVec( sizeof( struct AmiGUSAhiDriverData ), MEMF_FAST );
  if ( !driverData ) {

    DisplayError( EOutOfMemory );
    return AHISF_ERROR;
  }
  aAudioCtrl->ahiac_DriverData = driverData;
  recording = &( driverData->agdd_Recording );
  playback = &( driverData->agdd_Playback );

  driverData->agdd_HwSampleRateId = sampleRateId;

  playback->agpp_Buffer[ 0 ] = NULL;
  playback->agpp_Buffer[ 1 ] = NULL;
  playback->agpp_BufferIndex[ 0 ] = 0;
  playback->agpp_BufferIndex[ 1 ] = 0;
  playback->agpp_BufferMax[ 0 ] = 0;
  playback->agpp_BufferMax[ 1 ] = 0;
  playback->agpp_CurrentBuffer = 0;
  playback->agpp_Watermark = 0;
  playback->agpp_CopyFunctionId = playbackCopyFunctionId;
  playback->agpp_CopyFunction =
    PlaybackCopyFunctionById[ playbackCopyFunctionId ];
  playback->agpp_HwSampleFormatId = playHwSampleFormatId;
  playback->agpp_AhiSampleShift = playSampleBytesShift;

  recording->agpr_Buffer[ 0 ] = NULL;
  recording->agpr_Buffer[ 1 ] = NULL;
  recording->agpr_BufferIndex[ 0 ] = 0;
  recording->agpr_BufferIndex[ 1 ] = 0;
  recording->agpr_BufferMax[ 0 ] = 0;
  recording->agpr_BufferMax[ 1 ] = 0;
  recording->agpr_CurrentBuffer = 0;
  recording->agpr_CopyFunction = ( canRecord ) ?
      RecordingCopyFunctionById[ recordingCopyFunctionId ] :
      NULL;
  recording->agpr_CopyFunctionId = recordingCopyFunctionId;
  recording->agpr_RecordingMessage.ahirm_Type =
        RecordingSampleTypeById[ recordingCopyFunctionId ];
  recording->agpr_RecordingMessage.ahirm_Buffer = NULL;
  recording->agpr_RecordingMessage.ahirm_Length = 0;
  recording->agpr_HwSampleFormatId = recHwSampleFormatId;
  recording->agpr_AhiSampleShift = recSampleBytesShift;
  recording->agpr_HwSourceId = 0;

  /*
   * ------------------------------------------------------
   * Part 4: Prepare slave task communication.
   * ------------------------------------------------------
   */
  // TODO: only if needed! not twice!
  AmiGUSBase->agb_WorkerWorkSignal = -1;
  AmiGUSBase->agb_WorkerStopSignal = -1;
  AmiGUSBase->agb_MainProcess = ( struct Process * ) FindTask( NULL );
  AmiGUSBase->agb_MainSignal = AllocSignal( -1 );
  if ( -1 == AmiGUSBase->agb_MainSignal ) {

    DisplayError( EMainProcessSignalsFailed );
    return AHISF_ERROR;
  }
  AmiGUSBase->agb_WorkerReady = FALSE;

  LOG_D(( "D: AHIsub_AllocAudio done, returning 0x%lx.\n", result ));

  return result;
}

ASM(void) SAVEDS AHIsub_FreeAudio(
  REG(a2, struct AHIAudioCtrlDrv *aAudioCtrl)
) {

  struct AmiGUSAhiDriverData * driverData = 
    ( struct AmiGUSAhiDriverData * ) aAudioCtrl->ahiac_DriverData;
  struct AmiGUSPcmCard * card = driverData->agdd_Card;

  LOG_D(( "D: AHIsub_FreeAudio start\n" ));

  /*
   * ------------------------------------------------------
   * Part 4: Free slave task communication.
   * ------------------------------------------------------
   */
  /* Freeing a non-alloc`ed signal, i.e. -1, is harmless */
  FreeSignal( AmiGUSBase->agb_MainSignal );
  AmiGUSBase->agb_MainSignal = -1;
  LOG_D(( "D: Free`ed main signal\n" ));
  // TODO: nope, cannot do that here! Only for last one!

  /*
   * ------------------------------------------------------
   * Part 3: Free driver data.
   * ------------------------------------------------------
   */
  FreeVec( aAudioCtrl->ahiac_DriverData );
  aAudioCtrl->ahiac_DriverData = NULL;
  // TODO: should that be in stop only?
  if ( aAudioCtrl == card->agpc_PlaybackCtrl ) {

    card->agpc_PlaybackCtrl = NULL;
    card->agpc_StateFlags &= AMIGUS_AHI_F_PLAY_STOP_MASK;
  }
  if ( aAudioCtrl == card->agpc_RecordingCtrl ) {

    card->agpc_RecordingCtrl = NULL;
    card->agpc_StateFlags &= AMIGUS_AHI_F_REC_STOP_MASK;
  }

  /*
   * ------------------------------------------------------
   * Part 1: De-allocate this AmiGUS,
   *         re-entrance/interrupt/multitasking safe.
   * ------------------------------------------------------
   */
  Disable();
  driverData->agdd_Card = NULL;
  Enable();
  LOG_D(( "D: Free`ed AmiGUS AHI hardware\n" ));

  LOG_D(( "D: AHIsub_FreeAudio done\n" ));

  return;
}
