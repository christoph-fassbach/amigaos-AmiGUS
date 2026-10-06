/*
 * This file is part of the AmiGUS.audio driver.
 *
 * AmiGUS.audio driver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS.audio driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with AmiGUS.audio driver.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <proto/dos.h>

#include "amigus_ahi_modes.h"
#include "amigus_ahi_sub.h"
#include "amigus_hardware.h"
#include "amigus_pcm.h"
#include "debug.h"
#include "errors.h"

VOID StartAmiGusPcmPlayback( VOID ) {

  LONG i;
#ifdef MEM_LOG
  LONG prefillSize = 1200; /* in LONGs */
#else
  LONG prefillSize = 12; /* in LONGs */
#endif
  STRPTR environmentVariableKey = "AmiGUS-AHI-PREFILL-SIZE";
  UBYTE buffer[ 4 ];
  UWORD modeOffset = AmiGUS_AHI_Base->agb_AhiModeOffset;
  APTR amiGUS = AmiGUS_AHI_Base->agb_CardBase;

  LOG_D(("D: Init & start AmiGUS PCM playback @ 0x%08lx\n", amiGUS));

  i = GetVar( environmentVariableKey, buffer, sizeof( buffer ), 0 );
  if ( 0 < i ) {

    LONG j = StrToLong( buffer, ( LONG * ) &prefillSize );
    WORD temp = ( WORD ) prefillSize;

    LOG_W(( "W: %s tries setting PCM prefill size to %ld LONGs.\n",
            environmentVariableKey, temp )); 

    // Yes, in integer math, this DOES something! So do not dare touching it!
    temp /= 12; // Prefill size ALWAYS is a multiple of sample size, so...
    temp *= 12; // 6=2*3 (24bit Stereo) or 4=2*2 (16bit Stereo) and so on

    if (( 0 > j )
      || ( AMIGUS_PCM_PLAY_FIFO_LONGS < temp )
      || ( 0 > temp )) {

      temp = AMIGUS_PCM_PLAY_FIFO_LONGS;
    }
    prefillSize = temp;
  }
  LOG_I(( "I: Effective PCM prefill size is %ld LONGs.\n", prefillSize ));

  WriteReg16( amiGUS,
              AMIGUS_PCM_PLAY_SAMPLE_RATE,
              AMIGUS_PCM_SAMPLE_F_DISABLE );
  WriteReg16( amiGUS,
              AMIGUS_PCM_PLAY_FIFO_RESET,
              AMIGUS_PCM_FIFO_RESET_STROBE );
  // Idea: Watermark is here 6 words aka 3 longs,
  //       2 24bit samples, ... 
  // Cool, always a fit for all sample widths.
  WriteReg16( amiGUS,
              AMIGUS_PCM_PLAY_FIFO_WATERMARK,
              // Watermark is WORDs, so using LONG value means half
              prefillSize );
  WriteReg16( amiGUS, 
              AMIGUS_PCM_INT_CONTROL, 
              /* Clear interrupt flag bits */
              AMIGUS_INT_F_CLEAR
            | AMIGUS_PCM_INT_F_PLAY_FIFO_EMPTY
            | AMIGUS_PCM_INT_F_PLAY_FIFO_FULL
            | AMIGUS_PCM_INT_F_PLAY_FIFO_WTRMK );
  WriteReg16( amiGUS,
              AMIGUS_PCM_INT_ENABLE,
              /* Clear interrupt mask bits */
              AMIGUS_INT_F_CLEAR
            | AMIGUS_PCM_INT_F_PLAY_FIFO_EMPTY
            | AMIGUS_PCM_INT_F_PLAY_FIFO_FULL
            | AMIGUS_PCM_INT_F_PLAY_FIFO_WTRMK );
  WriteReg16( amiGUS,
              AMIGUS_PCM_INT_ENABLE,
              /* Now set some! */
              AMIGUS_INT_F_SET
            | AMIGUS_PCM_INT_F_PLAY_FIFO_EMPTY
            | AMIGUS_PCM_INT_F_PLAY_FIFO_WTRMK );

  // Now write twice the amount of data into FIFO to kick off playback
  for ( i = prefillSize; 0 < i; --i ) {

    WriteReg32( amiGUS,
                AMIGUS_PCM_PLAY_FIFO_WRITE,
                0L );
  }
  // Use correct sample settings, prefill is selected to match all
  WriteReg16( amiGUS,
              AMIGUS_PCM_PLAY_SAMPLE_FORMAT,
              PlaybackPropertiesById[ modeOffset ].pp_HwFormatId );

  // Start playback finally
  AmiGUS_AHI_Base->agb_StateFlags &= AMIGUS_AHI_F_PLAY_STOP_MASK;
  AmiGUS_AHI_Base->agb_StateFlags |= AMIGUS_AHI_F_PLAY_STARTED;
  WriteReg16( amiGUS,
              AMIGUS_PCM_PLAY_SAMPLE_RATE,
              AmiGUS_AHI_Base->agb_HwSampleRateId
            | AMIGUS_PCM_S_PLAY_F_INTERPOLATE
            | AMIGUS_PCM_SAMPLE_F_ENABLE );
}

VOID StopAmiGusPcmPlayback( VOID ) {

  APTR amiGUS = AmiGUS_AHI_Base->agb_CardBase;
  LOG_D(("D: Stop AmiGUS PCM playback @ 0x%08lx\n", amiGUS));

  WriteReg16( amiGUS, 
              AMIGUS_PCM_PLAY_SAMPLE_RATE, 
              AMIGUS_PCM_SAMPLE_F_DISABLE );
  WriteReg16( amiGUS, 
              AMIGUS_PCM_INT_CONTROL, 
              /* Clear interrupt flag bits */
              AMIGUS_PCM_INT_F_PLAY_FIFO_EMPTY
            | AMIGUS_PCM_INT_F_PLAY_FIFO_FULL
            | AMIGUS_PCM_INT_F_PLAY_FIFO_WTRMK );
  WriteReg16( amiGUS,
              AMIGUS_PCM_INT_ENABLE,
              /* Clear interrupt mask bits */
              AMIGUS_PCM_INT_F_PLAY_FIFO_EMPTY
            | AMIGUS_PCM_INT_F_PLAY_FIFO_FULL
            | AMIGUS_PCM_INT_F_PLAY_FIFO_WTRMK );
  WriteReg16( amiGUS,
              AMIGUS_PCM_PLAY_FIFO_RESET,
              AMIGUS_PCM_FIFO_RESET_STROBE );
  AmiGUS_AHI_Base->agb_StateFlags &= AMIGUS_AHI_F_PLAY_STOP_MASK;
}

VOID StartAmiGusPcmRecording( VOID ) {

  APTR amiGUS = AmiGUS_AHI_Base->agb_CardBase;
  struct AmiGUSPcmRecording *recording = &AmiGUS_AHI_Base->agb_Recording;
  UWORD flags16;
  ULONG flags32;

  LOG_D(("D: Init & start AmiGUS PCM recording @ 0x%08lx\n", amiGUS));

  WriteReg16( amiGUS,
              AMIGUS_PCM_REC_SAMPLE_RATE,
              AMIGUS_PCM_SAMPLE_F_DISABLE );
  WriteReg16( amiGUS,
              AMIGUS_PCM_REC_FIFO_RESET,
              AMIGUS_PCM_FIFO_RESET_STROBE );
  // Idea: Watermark is half the FIFO size, much smaller here anyway
  WriteReg16( amiGUS,
              AMIGUS_PCM_REC_FIFO_WATERMARK,
              // Watermark is WORDs, so using LONG value means half
              AMIGUS_PCM_REC_FIFO_LONGS );
  WriteReg16( amiGUS, 
              AMIGUS_PCM_INT_CONTROL, 
              /* Clear interrupt flag bits */
              AMIGUS_INT_F_CLEAR
            | AMIGUS_PCM_INT_F_REC_FIFO_EMPTY
            | AMIGUS_PCM_INT_F_REC_FIFO_FULL
            | AMIGUS_PCM_INT_F_REC_FIFO_WTRMRK );
  WriteReg16( amiGUS,
              AMIGUS_PCM_INT_ENABLE,
              /* Clear interrupt mask bits */
              AMIGUS_INT_F_CLEAR
            | AMIGUS_PCM_INT_F_REC_FIFO_EMPTY
            | AMIGUS_PCM_INT_F_REC_FIFO_FULL
            | AMIGUS_PCM_INT_F_REC_FIFO_WTRMRK );
  WriteReg16( amiGUS,
              AMIGUS_PCM_INT_ENABLE,
              /* Now set some! */
              AMIGUS_INT_F_SET
            | AMIGUS_PCM_INT_F_REC_FIFO_FULL
            | AMIGUS_PCM_INT_F_REC_FIFO_WTRMRK );

  flags32 = 0xffFFffFF; // TODO: Input Gain here!
  WriteReg32( amiGUS, AMIGUS_PCM_REC_VOLUME, flags32 ); 
  LOG_V(( "V: Set AMIGUS_PCM_REC_VOLUME = 0x%08lx\n", flags32 ));

  flags16 = AmiGUS_AHI_Base->agb_AhiModeOffset;
  flags16 = RecordingPropertiesById[ flags16 ].rp_HwFormatId;
  WriteReg16( amiGUS, AMIGUS_PCM_REC_SAMPLE_FORMAT, flags16 );
  LOG_V(( "V: Set AMIGUS_PCM_REC_SAMPLE_FORMAT = 0x%04lx\n", flags16 ));

  // Start recording finally
  AmiGUS_AHI_Base->agb_StateFlags &= AMIGUS_AHI_F_REC_STOP_MASK;
  AmiGUS_AHI_Base->agb_StateFlags |= AMIGUS_AHI_F_REC_STARTED;

  flags16 = AmiGUS_AHI_Base->agb_HwSampleRateId
          | AmiGUSInputFlags[ recording->agpr_HwSourceId ]
          | AMIGUS_PCM_SAMPLE_F_ENABLE;
  WriteReg16( amiGUS, AMIGUS_PCM_REC_SAMPLE_RATE, flags16 );
  LOG_V(( "V: Set AMIGUS_PCM_REC_SAMPLE_RATE = 0x%04lx\n", flags16 ));
}

VOID StopAmiGusPcmRecording( VOID ) {

  APTR amiGUS = AmiGUS_AHI_Base->agb_CardBase;
  LOG_D(("D: Stop AmiGUS PCM recording @ 0x%08lx\n", amiGUS));

  WriteReg16( amiGUS, 
              AMIGUS_PCM_REC_SAMPLE_RATE, 
              AMIGUS_PCM_SAMPLE_F_DISABLE );
  WriteReg16( amiGUS, 
              AMIGUS_PCM_INT_CONTROL, 
              /* Clear interrupt flag bits */
              AMIGUS_INT_F_CLEAR
            | AMIGUS_PCM_INT_F_REC_FIFO_EMPTY
            | AMIGUS_PCM_INT_F_REC_FIFO_FULL
            | AMIGUS_PCM_INT_F_REC_FIFO_WTRMRK );
  WriteReg16( amiGUS,
              AMIGUS_PCM_INT_ENABLE,
              /* Clear interrupt mask bits */
              AMIGUS_INT_F_CLEAR
            | AMIGUS_PCM_INT_F_REC_FIFO_EMPTY
            | AMIGUS_PCM_INT_F_REC_FIFO_FULL
            | AMIGUS_PCM_INT_F_REC_FIFO_WTRMRK );
  WriteReg16( amiGUS,
              AMIGUS_PCM_REC_FIFO_RESET,
              AMIGUS_PCM_FIFO_RESET_STROBE );
  AmiGUS_AHI_Base->agb_StateFlags &= AMIGUS_AHI_F_REC_STOP_MASK;
}
