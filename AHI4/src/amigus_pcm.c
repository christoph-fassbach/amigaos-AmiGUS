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

#include <proto/alib.h>
#include <proto/exec.h>
#include <proto/expansion.h>

#include "amigus_ahi_sub.h"
#include "amigus_hardware.h"
#include "amigus_pcm.h"
#include "debug.h"
#include "errors.h"
#include "support.h"

LONG FindAmiGusPcm( struct AmiGUSBase * amiGUSBase ) {

  struct ConfigDev *configDevice = NULL;

  NonConflictingNewMinList( &( amiGUSBase->agb_CardList ) );

  while ( configDevice = FindConfigDev( configDevice,
                                        AMIGUS_MANUFACTURER_ID,
                                        AMIGUS_MAIN_PRODUCT_ID )) {

    struct AmiGUSPcmCard * card;
    ULONG serial;

    if ( ( AMIGUS_MANUFACTURER_ID != configDevice->cd_Rom.er_Manufacturer )
      || ( AMIGUS_MAIN_PRODUCT_ID != configDevice->cd_Rom.er_Product ) ) {

      LOG_E(("E: AmiGUS detection failed\n"));
      return EAmiGUSDetectError;
    }

    serial = configDevice->cd_Rom.er_SerialNumber;
    if ( AMIGUS_AHI_FIRMWARE_MINIMUM > serial ) {

      LOG_E(( "E: AmiGUS firmware expected %08lx, actual %08lx\n",
              AMIGUS_AHI_FIRMWARE_MINIMUM,
              serial ));
      return EAmiGUSFirmwareOutdated;
    }

    LOG_V(( "V: AmiGUS firmware %08lx\n", serial ));
    LOG_I(( "I: AmiGUS firmware date %04ld-%02ld-%02ld, %02ld:%02ld\n",
            ( UWORD ) (( serial & 0xFFF00000ul ) >> 20 ) /* year */,
            ( UBYTE ) (( serial & 0x000F0000ul ) >> 16 ) /* month */,
            ( UBYTE ) (( serial & 0x0000F800ul ) >> 11 ) /* day */,
            ( UBYTE ) (( serial & 0x000007C0ul ) >>  6 ) /* hour */,
            ( UBYTE ) (( serial & 0x0000003Ful )       ) /* minute */ ));

    card = ( struct AmiGUSPcmCard * ) AllocVec( sizeof( struct AmiGUSPcmCard ),
                                                MEMF_FAST | MEMF_CLEAR ); // TODO: where to free!?!
    card->agpc_CardBase = configDevice->cd_BoardAddr;
    LOG_I(( "I: AmiGUS found at 0x%08lx\n", card->agpc_CardBase ));

    AddHead( ( struct List * ) &( amiGUSBase->agb_CardList ),
             ( struct Node * ) card );
  }
  if ( IsListEmpty( ( struct List * ) &( amiGUSBase->agb_CardList ) ) ) {

    LOG_E(("E: AmiGUS not found\n"));
    return EAmiGUSNotFound;
  }

  return ENoError;
}

VOID StartAmiGusPcmPlayback( struct AmiGUSAhiDriverData * driverData ) {

  ULONG i;
#ifdef USE_MEM_LOGGING
  // Mem logging enables LOG_INT level, and that is slow as f*ck,
  // so we need more prefil to not time out before the INT handler is ready.
  ULONG prefillSize = 1200; /* in LONGs */
#else
  ULONG prefillSize = 12; /* in LONGs */
#endif
  struct AmiGUSPcmCard * card = driverData->agdd_Playback.agpp_Card;
  APTR cardBase = card->agpc_CardBase;
  LOG_D(( "D: Init & start AmiGUS PCM playback @ 0x%08lx with prefill %ld\n",
          cardBase, prefillSize ));

  WriteReg16( cardBase,
              AMIGUS_PCM_PLAY_SAMPLE_RATE,
              AMIGUS_PCM_SAMPLE_F_DISABLE );
  WriteReg16( cardBase,
              AMIGUS_PCM_PLAY_FIFO_RESET,
              AMIGUS_PCM_FIFO_RESET );
  // Idea: Watermark is here 6 words aka 3 longs,
  //       2 24bit samples, ... 
  // Cool, always a fit for all sample widths.
  WriteReg16( cardBase,
              AMIGUS_PCM_PLAY_FIFO_WATERMARK,
              // Watermark is WORDs, so using LONG value means half
              prefillSize );
  WriteReg16( cardBase, 
              AMIGUS_PCM_MAIN_INT_CONTROL, 
              /* Clear interrupt flag bits */
              AMIGUS_INT_F_CLEAR
            | AMIGUS_INT_F_PLAY_FIFO_EMPTY
            | AMIGUS_INT_F_PLAY_FIFO_FULL
            | AMIGUS_INT_F_PLAY_FIFO_WATERMARK );
  WriteReg16( cardBase,
              AMIGUS_PCM_MAIN_INT_ENABLE,
              /* Clear interrupt mask bits */
              AMIGUS_INT_F_CLEAR
            | AMIGUS_INT_F_PLAY_FIFO_EMPTY
            | AMIGUS_INT_F_PLAY_FIFO_FULL
            | AMIGUS_INT_F_PLAY_FIFO_WATERMARK );
  WriteReg16( cardBase,
              AMIGUS_PCM_MAIN_INT_ENABLE,
              /* Now set some! */
              AMIGUS_INT_F_SET
            | AMIGUS_INT_F_PLAY_FIFO_EMPTY
            | AMIGUS_INT_F_PLAY_FIFO_WATERMARK );

  // Now write twice the amount of data into FIFO to kick off playback
  for ( i = prefillSize; 0 < i; --i ) {

    WriteReg32( cardBase,
                AMIGUS_PCM_PLAY_FIFO_WRITE,
                0L );
  }
  // Use correct sample settings, prefill is selected to match all
  WriteReg16( cardBase,
              AMIGUS_PCM_PLAY_SAMPLE_FORMAT,
              driverData->agdd_Playback.agpp_HwSampleFormatId );

  // Start playback finally
  card->agpc_StateFlags &= AMIGUS_AHI_F_PLAY_STOP_MASK;
  card->agpc_StateFlags |= AMIGUS_AHI_F_PLAY_STARTED;
  WriteReg16( cardBase,
              AMIGUS_PCM_PLAY_SAMPLE_RATE,
              driverData->agdd_HwSampleRateId
            | AMIGUS_PCM_S_PLAY_F_INTERPOLATE
            | AMIGUS_PCM_SAMPLE_F_ENABLE );
}

VOID StopAmiGusPcmPlayback( struct AmiGUSAhiDriverData * driverData ) {

  struct AmiGUSPcmCard * card = driverData->agdd_Playback.agpp_Card;
  APTR cardBase = card->agpc_CardBase;
  LOG_D(( "D: Stop AmiGUS PCM playback @ 0x%08lx\n", cardBase ));

  WriteReg16( cardBase, 
              AMIGUS_PCM_PLAY_SAMPLE_RATE, 
              AMIGUS_PCM_SAMPLE_F_DISABLE );
  WriteReg16( cardBase, 
              AMIGUS_PCM_MAIN_INT_CONTROL, 
              /* Clear interrupt flag bits */
              AMIGUS_INT_F_PLAY_FIFO_EMPTY
            | AMIGUS_INT_F_PLAY_FIFO_FULL
            | AMIGUS_INT_F_PLAY_FIFO_WATERMARK );
  WriteReg16( cardBase,
              AMIGUS_PCM_MAIN_INT_ENABLE,
              /* Clear interrupt mask bits */
              AMIGUS_INT_F_PLAY_FIFO_EMPTY
            | AMIGUS_INT_F_PLAY_FIFO_FULL
            | AMIGUS_INT_F_PLAY_FIFO_WATERMARK );
  WriteReg16( cardBase,
              AMIGUS_PCM_PLAY_FIFO_RESET,
              AMIGUS_PCM_FIFO_RESET );
  card->agpc_StateFlags &= AMIGUS_AHI_F_PLAY_STOP_MASK;
}

VOID StartAmiGusPcmRecording( struct AmiGUSAhiDriverData * driverData ) {

  struct AmiGUSPcmCard * card = driverData->agdd_Recording.agpr_Card;
  APTR cardBase = card->agpc_CardBase;
  struct AmiGUSPcmRecording *recording = &driverData->agdd_Recording;
  UWORD flags16;
  ULONG flags32;

  LOG_D(( "D: Init & start AmiGUS PCM recording @ 0x%08lx\n", cardBase ));

  WriteReg16( cardBase,
              AMIGUS_PCM_REC_SAMPLE_RATE,
              AMIGUS_PCM_SAMPLE_F_DISABLE );
  WriteReg16( cardBase,
              AMIGUS_PCM_REC_FIFO_RESET,
              AMIGUS_PCM_FIFO_RESET );
  // Idea: Watermark is half the FIFO size, much smaller here anyway
  WriteReg16( cardBase,
              AMIGUS_PCM_REC_FIFO_WATERMARK,
              // Watermark is WORDs, so using LONG value means half
              AMIGUS_PCM_REC_FIFO_LONGS );
  WriteReg16( cardBase, 
              AMIGUS_PCM_MAIN_INT_CONTROL, 
              /* Clear interrupt flag bits */
              AMIGUS_INT_F_CLEAR
            | AMIGUS_INT_F_REC_FIFO_EMPTY
            | AMIGUS_INT_F_REC_FIFO_FULL
            | AMIGUS_INT_F_REC_FIFO_WATERMARK );
  WriteReg16( cardBase,
              AMIGUS_PCM_MAIN_INT_ENABLE,
              /* Clear interrupt mask bits */
              AMIGUS_INT_F_CLEAR
            | AMIGUS_INT_F_REC_FIFO_EMPTY
            | AMIGUS_INT_F_REC_FIFO_FULL
            | AMIGUS_INT_F_REC_FIFO_WATERMARK );
  WriteReg16( cardBase,
              AMIGUS_PCM_MAIN_INT_ENABLE,
              /* Now set some! */
              AMIGUS_INT_F_SET
            | AMIGUS_INT_F_REC_FIFO_FULL
            | AMIGUS_INT_F_REC_FIFO_WATERMARK );

  flags32 = 0xffFFffFF; // TODO: Input Gain here!
  WriteReg32( cardBase, AMIGUS_PCM_REC_VOLUME, flags32 ); 
  LOG_V(( "V: Set AMIGUS_PCM_REC_VOLUME = 0x%08lx\n", flags32 ));

  flags16 = driverData->agdd_Recording.agpr_HwSampleFormatId;
  WriteReg16( cardBase, AMIGUS_PCM_REC_SAMPLE_FORMAT, flags16 );
  LOG_V(( "V: Set AMIGUS_PCM_REC_SAMPLE_FORMAT = 0x%04lx\n", flags16 ));

  // Start recording finally
  card->agpc_StateFlags &= AMIGUS_AHI_F_REC_STOP_MASK;
  card->agpc_StateFlags |= AMIGUS_AHI_F_REC_STARTED;

  flags16 = driverData->agdd_HwSampleRateId
          | AmiGUSInputFlags[ recording->agpr_HwSourceId ]
          | AMIGUS_PCM_SAMPLE_F_ENABLE;
  WriteReg16( cardBase, AMIGUS_PCM_REC_SAMPLE_RATE, flags16 );
  LOG_V(( "V: Set AMIGUS_PCM_REC_SAMPLE_RATE = 0x%04lx\n", flags16 ));
}

VOID StopAmiGusPcmRecording( struct AmiGUSAhiDriverData * driverData ) {

  struct AmiGUSPcmCard * card = driverData->agdd_Recording.agpr_Card;
  APTR cardBase = card->agpc_CardBase;
  LOG_D(("D: Stop AmiGUS PCM recording @ 0x%08lx\n", cardBase));

  WriteReg16( cardBase, 
              AMIGUS_PCM_REC_SAMPLE_RATE, 
              AMIGUS_PCM_SAMPLE_F_DISABLE );
  WriteReg16( cardBase, 
              AMIGUS_PCM_MAIN_INT_CONTROL, 
              /* Clear interrupt flag bits */
              AMIGUS_INT_F_CLEAR
            | AMIGUS_INT_F_REC_FIFO_EMPTY
            | AMIGUS_INT_F_REC_FIFO_FULL
            | AMIGUS_INT_F_REC_FIFO_WATERMARK );
  WriteReg16( cardBase,
              AMIGUS_PCM_MAIN_INT_ENABLE,
              /* Clear interrupt mask bits */
              AMIGUS_INT_F_CLEAR
            | AMIGUS_INT_F_REC_FIFO_EMPTY
            | AMIGUS_INT_F_REC_FIFO_FULL
            | AMIGUS_INT_F_REC_FIFO_WATERMARK );
  WriteReg16( cardBase,
              AMIGUS_PCM_REC_FIFO_RESET,
              AMIGUS_PCM_FIFO_RESET );
  card->agpc_StateFlags &= AMIGUS_AHI_F_REC_STOP_MASK;
}
