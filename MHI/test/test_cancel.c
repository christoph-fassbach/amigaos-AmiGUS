/*
 * This file is part of the mhiamigus.library.
 *
 * mhiamigus.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiamigus.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiamigus.library.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/types.h>

#include "amigus_mhi.h"
#include "amigus_vs1063.h"

/******************************************************************************
 * Mocked functions and stubbed external symbols below:
 *****************************************************************************/

struct AmiGUS_MHI        * AmiGUS_MHI_Base   = NULL;

/* From amigus_hardware.h */

#define AMIGUS_HARDWARE_H

/* All AmiGUS timers behave the same */

#define AMIGUS_TIMER_CLOCK               24576000  /* Hz = 1/s = quarz clock */

/* General time properties - Constants calculated out by C's preprocessor :) */

#define MILLIS_PER_SECOND                1000
#define MICROS_PER_SECOND                1000000

#define	AMIGUS_CODEC_INT_CONTROL         0x00
#define	AMIGUS_CODEC_FIFO_WRITE          0x0a
#define	AMIGUS_CODEC_FIFO_USAGE          0x0e

#define AMIGUS_CODEC_SPI_STATUS	         0x2a

#define	AMIGUS_CODEC_FIFO_CONTROL        0x04

#define AMIGUS_INT_F_CLEAR               0x0000

/* AmiGUS Codec Interrupt Flags */
#define AMIGUS_CODEC_INT_F_VS1063_DRQ    0x0010

/* AmiGUS Codec FIFO Steering Flags */
/* FIFO DMA */
#define AMIGUS_CODEC_FIFO_F_DMA_ENABLE   0x8000
#define AMIGUS_CODEC_FIFO_F_DMA_DISABLE  0x0000

/* AmiGUS Codec SPI Steering Flags */
#define AMIGUS_CODEC_SPI_F_DREQ          0x4000

// VS1063 codec's SCIs - Serial Control Interface SPI ports, overview page 43
#define VS1063_CODEC_SCI_MODE           0x0000 // page 44
#define VS1063_CODEC_SCI_CLOCKF         0x0003 // page 48
#define VS1063_CODEC_SCI_HDAT0          0x0008 // page 50
#define VS1063_CODEC_SCI_HDAT1          0x0009 // page 50
#define VS1063_CODEC_SCI_VOL            0x000B // page 53

// VS1063 codec's addresses of memory mapped registers
// all parameter memory 0x1E00-0x1E3F is mapped to 0xC0C0-0xC0FF - page 49
#define VS1063_CODEC_ADDRESS_END_FILL   0xC0C6 // 0x1E06 p.70 + 57
#define VS1063_CODEC_ADDRESS_PLAY_MODE  0xC0C9 // 0x1E09 p.77
#define VS1063_CODEC_ADDRESS_EQ5_LEVEL1 0xC0D3 // 0x1E13 p.77 (-32 - +32)*0.5dB
#define VS1063_CODEC_ADDRESS_EQ5_FREQ1  0xC0D4 // 0x1E14 p.77      20 -   150Hz
#define VS1063_CODEC_ADDRESS_EQ5_LEVEL2 0xC0D5 // 0x1E15 p.77 (-32 - +32)*0.5dB
#define VS1063_CODEC_ADDRESS_EQ5_FREQ2  0xC0D6 // 0x1E16 p.77 -    50 -  1000Hz
#define VS1063_CODEC_ADDRESS_EQ5_LEVEL3 0xC0D7 // 0x1E17 p.77 (-32 - +32)*0.5dB
#define VS1063_CODEC_ADDRESS_EQ5_FREQ3  0xC0D8 // 0x1E18 p.77 -  1000 - 15000Hz
#define VS1063_CODEC_ADDRESS_EQ5_LEVEL4 0xC0D9 // 0x1E19 p.77 (-32 - +32)*0.5dB
#define VS1063_CODEC_ADDRESS_EQ5_FREQ4  0xC0DA // 0x1E1A p.77 -  2000 - 15000Hz
#define VS1063_CODEC_ADDRESS_EQ5_LEVEL5 0xC0DB // 0x1E1B p.77 (-32 - +32)*0.5dB
#define VS1063_CODEC_ADDRESS_EQ5_UPDATE 0xC0DC // 0x1E1C p.77 strobe for update

#define VS1063_CODEC_ADDRESS_GPIO_DDR   0xC017 // page 86
#define VS1063_CODEC_ADDRESS_I2S_CONFIG 0xC040 // page 86

// VS1063 codec's magic flag values according to datasheet
#define VS1063_CODEC_F_SM_LAYER12       0x0002 // page 44
#define VS1063_CODEC_F_SM_RESET         0x0004 // page 44
#define VS1063_CODEC_F_SM_CANCEL        0x0008 // page 44
#define VS1063_CODEC_F_SM_SDINEW        0x0800 // page 44
#define VS1063_CODEC_F_SM_CLK_RANGE     0x4000 // page 44

#define VS1063_CODEC_F_SC_MULT_5_0X     0xE000 // page 48

#define VS1063_CODEC_F_PL_MO_EQ5_ENABLE 0x20   // page 77
#define VS1063_CODEC_F_EQ5_UPD_STROBE   0x01   // page 77

#define VS1063_CODEC_F_GPIO_DDR_192k    0xD0   // page 86

#define VS1063_CODEC_F_I2S_CONFIG_RESET 0x00   // page 86
#define VS1063_CODEC_F_I2S_CONFIG_192k  0x06   // page 86

#define VS1063_CODEC_RESET_DELAY_MICROS 2      // page 56
#define VS1063_CODEC_RESET_DELAY_TICKS  (( AMIGUS_TIMER_CLOCK \
                                          * VS1063_CODEC_RESET_DELAY_MICROS ) \
                                            / MICROS_PER_SECOND )

/* From amigus_hardware.c */

const WORD AmiGUSDefaultEqualizer[ 9 ] = {
  0, /* +/- 0dB */   125, /* Hz */
  0, /* +/- 0dB */   500, /* Hz */
  0, /* +/- 0dB */  2000, /* Hz */
  0, /* +/- 0dB */  8000, /* Hz */
  0  /* +/- 0dB */
};

const UBYTE AmiGUSVolumeMapping[ 104 ] = {
  0xFE /*  0% */, 
  72 /*  1% */, 66 /*  2% */, 60 /*  3% */, 54 /*  4% */, 51 /*   5% */,
  48 /*  6% */, 45 /*  7% */, 42 /*  8% */, 41 /*  9% */, 39 /*  10% */,
  37 /* 11% */, 36 /* 12% */, 34 /* 13% */, 33 /* 14% */, 32 /*  15% */,
  31 /* 16% */, 30 /* 17% */, 30 /* 18% */, 29 /* 19% */, 28 /*  20% */,
  27 /* 21% */, 26 /* 22% */, 26 /* 23% */, 25 /* 24% */, 24 /*  25% */,
  23 /* 26% */, 23 /* 27% */, 22 /* 28% */, 21 /* 29% */, 21 /*  30% */,
  20 /* 31% */, 20 /* 32% */, 19 /* 33% */, 19 /* 34% */, 18 /*  35% */,
  18 /* 36% */, 17 /* 37% */, 17 /* 38% */, 16 /* 39% */, 16 /*  40% */,
  15 /* 41% */, 15 /* 42% */, 15 /* 43% */, 14 /* 44% */, 14 /*  45% */,
  13 /* 46% */, 13 /* 47% */, 12 /* 48% */, 12 /* 49% */, 12 /*  50% */,
  11 /* 51% */, 11 /* 52% */, 11 /* 53% */, 10 /* 54% */, 10 /*  55% */,
  10 /* 56% */,  9 /* 57% */,  9 /* 58% */,  8 /* 59% */,  8 /*  60% */,
   8 /* 61% */,  7 /* 62% */,  7 /* 63% */,  7 /* 64% */,  7 /*  65% */,
   6 /* 66% */,  6 /* 67% */,  6 /* 68% */,  6 /* 69% */,  6 /*  70% */,
   5 /* 71% */,  5 /* 72% */,  5 /* 73% */,  5 /* 74% */,  5 /*  75% */,
   4 /* 76% */,  4 /* 77% */,  4 /* 78% */,  4 /* 79% */,  4 /*  80% */,
   3 /* 81% */,  3 /* 82% */,  3 /* 83% */,  3 /* 84% */,  3 /*  85% */,
   2 /* 86% */,  2 /* 87% */,  2 /* 88% */,  2 /* 89% */,  2 /*  90% */,
   1 /* 91% */,  1 /* 92% */,  1 /* 93% */,  1 /* 94% */,  1 /*  95% */,
   0 /* 96% */,  0 /* 97% */,  0 /* 98% */,  0 /* 99% */,  0 /* 100% */,
  99          , 99          , 99 // padding back to LONGs
};

ULONG * NextWriteBuffer = NULL;
ULONG * WriteBuffer = NULL;

ULONG * NextReadBuffer = NULL;

UWORD ReadReg16( APTR amiGUS, ULONG offset ) {

  printf( "ReadReg16, expected 0x%08lx, is 0x%08lx\n",
          *NextReadBuffer, offset );
  assert( offset == *NextReadBuffer );

  ++NextReadBuffer;
  return ( UWORD ) *NextReadBuffer++;
}

ULONG ReadReg32( APTR card, ULONG offset ) {

  printf( "ReadReg32, expected 0x%08lx, is 0x%08lx\n",
          *NextReadBuffer, offset );
  assert( offset == *NextReadBuffer );

  ++NextReadBuffer;
  return *NextReadBuffer++;
}

UWORD ReadCodecSPI( APTR card, UWORD SPIregister ) {

  printf( "ReadCodecSPI, expected 0x%08lx, is 0x%08lx\n",
          ( ULONG ) *NextReadBuffer, ( ULONG ) SPIregister );
  assert( SPIregister == ( UWORD ) *NextReadBuffer );

  ++NextReadBuffer;
  return ( UWORD ) *NextReadBuffer++;
}

UWORD ReadVS1063Mem( APTR amiGUS, UWORD address ) {

  printf( "ReadVS1063Mem, expected 0x%08lx, is 0x%08lx\n",
          ( ULONG ) *NextReadBuffer, ( ULONG ) address );
  assert( address == ( UWORD ) *NextReadBuffer );

  ++NextReadBuffer;
  return ( UWORD ) *NextReadBuffer++;
}

#define WriteReg16_MARKER     0x10000000
#define WriteReg32_MARKER     0x20000000
#define WriteCodecSPI_MARKER  0x30000000
#define WriteVS1063Mem_MARKER 0x40000000

VOID WriteReg16( APTR card, ULONG offset, UWORD value ) {

  *NextWriteBuffer = WriteReg16_MARKER | offset;
  ++NextWriteBuffer;
  *NextWriteBuffer = 0x77770000 | value;
  ++NextWriteBuffer;
}

VOID WriteReg32( APTR card, ULONG offset, ULONG value ) {

  *NextWriteBuffer = WriteReg32_MARKER | offset;
  ++NextWriteBuffer;
  *NextWriteBuffer = value;
  ++NextWriteBuffer;
}

VOID WriteCodecSPI( APTR card, UWORD SPIregister, UWORD SPIvalue ) {

  *NextWriteBuffer = WriteCodecSPI_MARKER | SPIregister;
  ++NextWriteBuffer;
  *NextWriteBuffer = 0x77770000 | SPIvalue;
  ++NextWriteBuffer;
}

VOID WriteVS1063Mem( APTR amiGUS, UWORD address, UWORD value ) {

  *NextWriteBuffer = WriteVS1063Mem_MARKER | address;
  ++NextWriteBuffer;
  *NextWriteBuffer = 0x77770000 | value;
  ++NextWriteBuffer;
}

VOID SleepCodecTicks( APTR amiGUS, ULONG ticks ) {}

/* From amigus_vs1063.c */

#include "../src/amigus_vs1063.c"

/******************************************************************************
 * Private functions / fields under test:
 *****************************************************************************/

/******************************************************************************
 * Test functions:
 *****************************************************************************/

BOOL CheckWrites( ULONG * expectedWrites ) {

  ULONG * check = WriteBuffer;
  int i, j;

  printf( "Checking written values...\n");
  while ( check < NextWriteBuffer ) {

    ULONG count = *expectedWrites++;
    printf( "Checking %8lu writes of type 0x%08lx\n",
            count, ( LONG ) ( *expectedWrites & 0xf0000000 ));

    while ( count ) {
      for ( i = 0; i < 2 ; ++i ) {
        if ( *check != *expectedWrites ) {

          printf( "Written check, position: %ld, "
                  "actual: 0x%08lx, expected: 0x%08lx\n",
                  (( LONG ) check - ( LONG ) WriteBuffer ) >> 3,
                  *check,
                  *expectedWrites );
          printf( "Area around:\n");
          check -= 40;
          for ( j = 0; j < 80; ++j ) {

            printf( "0x%08lx ", *check++ );
            if ( 7 == j % 8 ) {

                printf( "\n" );
            }
          }
          return TRUE;
        }

        ++check;
        ++expectedWrites;
      }
      --count;
      if ( count ) {

        expectedWrites -= 2;
      }
    }
  }
  for ( i = 0; i < 2 ; ++i ) {
    if ( 0xffFFffFF != *expectedWrites++ ) {

      printf( "Less bytes written than expected!!!\n" );
      return TRUE;
    }
  }
  printf( "OK\n" );
  return FALSE;
}

BOOL testCancelPlaybackVanilla( VOID ) {

  BOOL result = FALSE;
  
  ULONG ReadBuffer[] = {
    VS1063_CODEC_SCI_HDAT1, 0x4d50,
    VS1063_CODEC_ADDRESS_END_FILL, 0x1234,
    AMIGUS_CODEC_FIFO_USAGE, 17,
    AMIGUS_CODEC_FIFO_USAGE, 0,
    VS1063_CODEC_SCI_MODE, 0,
    VS1063_CODEC_SCI_MODE, 0,
#if defined (MEM_LOG)
#if defined(__SASC)
    VS1063_CODEC_SCI_HDAT0, 0x4242,
    VS1063_CODEC_SCI_HDAT1, 0x4242,
#elif defined(__VBCC__)
    VS1063_CODEC_SCI_HDAT1, 0x4242,
    VS1063_CODEC_SCI_HDAT0, 0x4242,
#endif
#endif
    0xffFFffFF, 0xffFFffFF
  };
  ULONG expectedWrites[] = {
    1,      WriteReg16_MARKER | AMIGUS_CODEC_FIFO_CONTROL, 0x77770000 | AMIGUS_CODEC_FIFO_F_DMA_ENABLE,
    768,    WriteReg32_MARKER | AMIGUS_CODEC_FIFO_WRITE,   0x34343434,
    1,      WriteCodecSPI_MARKER | VS1063_CODEC_SCI_MODE,  0x77770000 | VS1063_CODEC_F_SM_CANCEL,
    8,      WriteReg32_MARKER | AMIGUS_CODEC_FIFO_WRITE,   0x34343434,
    1,      WriteReg16_MARKER | AMIGUS_CODEC_FIFO_CONTROL, 0x77770000 | AMIGUS_CODEC_FIFO_F_DMA_DISABLE,
    0xffFFffFF, 0xffFFffFF
  };

  NextReadBuffer = ReadBuffer;
  NextWriteBuffer = WriteBuffer;
  memset( WriteBuffer, 0, 1024 * 1024 );

  printf( "\nTesting CancelVS1063Playback - vanilla...\n" );
  CancelVS1063Playback( NULL );

  // Were all ReadBuffer elements consumed as expected?
  printf( "Checking read values...\n");
  result |= ( 0xffFFffFF != *NextReadBuffer++ );
  result |= ( 0xffFFffFF != *NextReadBuffer++ );
  if ( result ) {

    printf( "Read error - not all elements consumed!\n" );
    return result;
  }
  printf( "OK\n\n" );

  // Were all addresses and stuff written as expected?
  result |= CheckWrites( expectedWrites );

  return result;
}

BOOL testCancelPlaybackFLAC( VOID ) {

  BOOL result = FALSE;
  
  ULONG ReadBuffer[] = {
    VS1063_CODEC_SCI_HDAT1, 0x664C,
    VS1063_CODEC_ADDRESS_END_FILL, 0x1234,
    AMIGUS_CODEC_FIFO_USAGE, 17,
    AMIGUS_CODEC_FIFO_USAGE, 0,
    AMIGUS_CODEC_FIFO_USAGE, 0,
    AMIGUS_CODEC_FIFO_USAGE, 0,
    AMIGUS_CODEC_FIFO_USAGE, 44,
    AMIGUS_CODEC_FIFO_USAGE, 0,
    VS1063_CODEC_SCI_MODE, 0,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL, // 3 blocks of ignoring
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL, // 3 blocks of ignoring
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL, // 3 blocks of ignoring
    VS1063_CODEC_SCI_MODE, 0,
#if defined (MEM_LOG)
#if defined(__SASC)
    VS1063_CODEC_SCI_HDAT0, 0x4242,
    VS1063_CODEC_SCI_HDAT1, 0x4242,
#elif defined(__VBCC__)
    VS1063_CODEC_SCI_HDAT1, 0x4242,
    VS1063_CODEC_SCI_HDAT0, 0x4242,
#endif
#endif
    0xffFFffFF, 0xffFFffFF
  };
  ULONG expectedWrites[] = {
    1,          WriteReg16_MARKER | AMIGUS_CODEC_FIFO_CONTROL, 0x77770000 | AMIGUS_CODEC_FIFO_F_DMA_ENABLE,
    4 * 768,    WriteReg32_MARKER | AMIGUS_CODEC_FIFO_WRITE,   0x34343434,
    1,          WriteCodecSPI_MARKER | VS1063_CODEC_SCI_MODE,  0x77770000 | VS1063_CODEC_F_SM_CANCEL,
    8,          WriteReg32_MARKER | AMIGUS_CODEC_FIFO_WRITE,   0x34343434, // 3 blocks of ignoring
    8,          WriteReg32_MARKER | AMIGUS_CODEC_FIFO_WRITE,   0x34343434, // 3 blocks of ignoring
    8,          WriteReg32_MARKER | AMIGUS_CODEC_FIFO_WRITE,   0x34343434, // 3 blocks of ignoring
    8,          WriteReg32_MARKER | AMIGUS_CODEC_FIFO_WRITE,   0x34343434,
    1,          WriteReg16_MARKER | AMIGUS_CODEC_FIFO_CONTROL, 0x77770000 | AMIGUS_CODEC_FIFO_F_DMA_DISABLE,
    0xffFFffFF, 0xffFFffFF
  };

  NextReadBuffer = ReadBuffer;
  NextWriteBuffer = WriteBuffer;
  memset( WriteBuffer, 0, 1024 * 1024 );

  printf( "\nTesting CancelVS1063Playback - FLAC + 3 ignores...\n" );
  CancelVS1063Playback( NULL );

  // Were all ReadBuffer elements consumed as expected?
  printf( "Checking read values...\n");
  result |= ( 0xffFFffFF != *NextReadBuffer++ );
  result |= ( 0xffFFffFF != *NextReadBuffer++ );
  if ( result ) {

    printf( "Read error - not all elements consumed!\n" );
    return result;
  }
  printf( "OK\n\n" );

  // Were all addresses and stuff written as expected?
  result |= CheckWrites( expectedWrites );

  return result;
}

BOOL testCancelPlaybackReset( VOID ) {

  BOOL result = FALSE;
  
  ULONG ReadBuffer[] = {
    VS1063_CODEC_SCI_HDAT1, 0x4d50,
    VS1063_CODEC_ADDRESS_END_FILL, 0x1234,
    AMIGUS_CODEC_FIFO_USAGE, 17,
    AMIGUS_CODEC_FIFO_USAGE, 0,
    VS1063_CODEC_SCI_MODE, 0,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    VS1063_CODEC_SCI_MODE, VS1063_CODEC_F_SM_CANCEL,
    AMIGUS_CODEC_SPI_STATUS, AMIGUS_CODEC_SPI_F_DREQ,
    AMIGUS_CODEC_SPI_STATUS, AMIGUS_CODEC_SPI_F_DREQ,
    AMIGUS_CODEC_SPI_STATUS, 0,
#if defined (MEM_LOG)
#if defined(__SASC)
    VS1063_CODEC_SCI_HDAT0, 0x4242,
    VS1063_CODEC_SCI_HDAT1, 0x4242,
#elif defined(__VBCC__)
    VS1063_CODEC_SCI_HDAT1, 0x4242,
    VS1063_CODEC_SCI_HDAT0, 0x4242,
#endif
#endif
    0xffFFffFF, 0xffFFffFF
  };
  ULONG expectedWrites[] = {
    1,      WriteReg16_MARKER | AMIGUS_CODEC_FIFO_CONTROL, 0x77770000 | AMIGUS_CODEC_FIFO_F_DMA_ENABLE,
    768,    WriteReg32_MARKER | AMIGUS_CODEC_FIFO_WRITE,   0x34343434,
    1,      WriteCodecSPI_MARKER | VS1063_CODEC_SCI_MODE,  0x77770000 | VS1063_CODEC_F_SM_CANCEL,
    64*8,   WriteReg32_MARKER | AMIGUS_CODEC_FIFO_WRITE,   0x34343434,
    1,      WriteReg16_MARKER | AMIGUS_CODEC_INT_CONTROL,  0x77770000 | AMIGUS_CODEC_INT_F_VS1063_DRQ | AMIGUS_INT_F_CLEAR,
    1,      WriteCodecSPI_MARKER | VS1063_CODEC_SCI_MODE,  0x77770000 | VS1063_CODEC_F_SM_CLK_RANGE | VS1063_CODEC_F_SM_SDINEW | VS1063_CODEC_F_SM_RESET | VS1063_CODEC_F_SM_LAYER12,
    1,      WriteReg16_MARKER | AMIGUS_CODEC_FIFO_CONTROL, 0x77770000 | AMIGUS_CODEC_FIFO_F_DMA_DISABLE,
    0xffFFffFF, 0xffFFffFF
  };

  NextReadBuffer = ReadBuffer;
  NextWriteBuffer = WriteBuffer;
  memset( WriteBuffer, 0, 1024 * 1024 );

  printf( "\nTesting CancelVS1063Playback - vanilla...\n" );
  CancelVS1063Playback( NULL );

  // Were all ReadBuffer elements consumed as expected?
  printf( "Checking read values...\n");
  result |= ( 0xffFFffFF != *NextReadBuffer++ );
  result |= ( 0xffFFffFF != *NextReadBuffer++ );
  if ( result ) {

    printf( "Read error - not all elements consumed!\n" );
    return result;
  }
  printf( "OK\n\n" );

  // Were all addresses and stuff written as expected?
  result |= CheckWrites( expectedWrites );

  return result;
}

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main(int argc, char const *argv[]) {
 
  BOOL failed = FALSE;
 
  AmiGUS_MHI_Base = malloc( sizeof( struct AmiGUS_MHI ));
  memset( AmiGUS_MHI_Base, 0, sizeof( struct AmiGUS_MHI ));

  WriteBuffer = malloc( 1024 * 1024 );
 
  if ( !AmiGUS_MHI_Base ) {
 
    printf( "Memory allocation failed!" );
    return 20;
  }
  failed |= testCancelPlaybackVanilla();
  failed |= testCancelPlaybackFLAC();
  failed |= testCancelPlaybackReset();

  free( WriteBuffer );
  free( AmiGUS_MHI_Base );
 
  return ( failed ) ? 15 : 0;
}
