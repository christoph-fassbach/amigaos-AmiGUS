/*
 * This file is part of the amigus.library.
 *
 * amigus.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * amigus.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with amigus.library.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

// vc +kick13 -I../include -I../header amigus_lib_check.c -o amigus_lib_check

#include <stdio.h>
#include <string.h>

#ifdef NULL
#undef NULL
#endif

#include <proto/amigus.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/timer.h>

#include <stddef.h>

#include "amigus_private.h"
#include "errors.h"

#define SASC_MAXIMUM_DEFINE_LENGTH_IS_32 12345678

/*
 * AmiGUS interrupt registers:
#define	AMIGUS_CODEC_INT_CONTROL         0x00
#define	AMIGUS_CODEC_INT_ENABLE          0x02

#define AMIGUS_PCM_INT_CONTROL           0x00
#define AMIGUS_PCM_INT_ENABLE            0x02

#define AMIGUS_WT_INT_CONTROL_0          0x00
#define AMIGUS_WT_INT_ENABLE_0           0x08
 * AmiGUS interrupt flags:
#define AMIGUS_CODEC_INT_F_TIMER         0x4000
#define AMIGUS_PCM_INT_F_TIMER           0x4000
#define AMIGUS_WT_INT_F_TIMER            0x4000
 * results in:
 */
#define AMIGUS_INT_CONTROL               0x00
#define	AMIGUS_CODEC_PCM_INT_ENABLE      0x02
#define AMIGUS_WT_INT_ENABLE_0           0x08

#define AMIGUS_INT_F_TIMER               0x4000

#define AMIGUS_INT_F_SET                 0x8000
#define AMIGUS_INT_F_CLEAR               0x0000

/*
 * Because all three AmiGUS parts' timers work the same,
 * registers:
#define AMIGUS_CODEC_TIMER_CONTROL       0xf0
#define AMIGUS_CODEC_TIMER_RELOAD        0xf2

#define AMIGUS_PCM_TIMER_CONTROL         0xf0
#define AMIGUS_PCM_TIMER_RELOAD          0xf2

#define AMIGUS_WT_TIMER_CONTROL          0xf0
#define AMIGUS_WT_TIMER_RELOAD           0xf2
 * flags:
#define AMIGUS_TIMER_START               0x8000 // to be used with once or confinous
#define AMIGUS_TIMER_STOP                0x0000
#define AMIGUS_TIMER_ONCE                0x0001
#define AMIGUS_TIMER_CONTINOUS           0x0000
 * we just use:
 */
#define AMIGUS_TIMER_CONTROL             0xf0
#define AMIGUS_TIMER_RELOAD              0xf2

#define AMIGUS_TIMER_CLOCK               24576000  /* Hz = 1/s = quarz clock */

#define AMIGUS_TIMER_START_CONTINOUS     0x8000
#define AMIGUS_TIMER_START_ONCE          0x8001
#define AMIGUS_TIMER_STOP                0x0000

struct AmiGUS_Base * AmiGUS_Base;
struct TimerBase * TimerBase;
ULONG interruptCounter;

/******************************************************************************
 * Low-Level hardware access functions - public function definitions.
 *****************************************************************************/

UWORD ReadReg16( APTR card, ULONG offset ) {

  return *(( UWORD * )(( ULONG ) card + offset ));
}

ULONG ReadReg32( APTR card, ULONG offset ) {

  return *(( ULONG * )(( ULONG ) card + offset ));
}

VOID WriteReg16( APTR card, ULONG offset, UWORD value ) {

  *(( UWORD * )(( ULONG ) card + offset )) = value;
}

VOID WriteReg32( APTR card, ULONG offset, ULONG value ) {

  *(( ULONG * )(( ULONG ) card + offset )) = value;
}

/******************************************************************************
 * Interrupt handler
 *****************************************************************************/

LONG __ASM__ countingIntHandler( __REG__( a0, APTR cardbase )) {

  // interruptCounter = ( ULONG ) cardbase;

  UWORD status = ReadReg16( cardbase, AMIGUS_INT_CONTROL );
  if ( AMIGUS_INT_F_TIMER & status ) {

    WriteReg16( cardbase,
                AMIGUS_INT_CONTROL,
                AMIGUS_INT_F_CLEAR | AMIGUS_INT_F_TIMER );
    ++interruptCounter;
    return 1;
  }
  return 0;
}

/******************************************************************************
 * Actual test functions
 *****************************************************************************/

BOOL testContinousTimerInterrupt( struct AmiGUS * card,
                                  LONG which, 
                                  APTR base,
                                  APTR owner,
                                  const ULONG expect ) {

  ULONG installed;

  interruptCounter = 0;

  printf( "Using card 0x%08lx part 0x%04lx base 0x%08lx for owner 0x%08lx\n",
          card, which, base, owner );

  installed = AmiGUS_InstallInterrupt( card,
                                       which,
                                       owner,
                                       &( countingIntHandler ),
                                       base );
  if ( AmiGUS_NoError != installed ) {

    return TRUE;
  }

  // Deactivate all interrupts for this card
  if ( AMIGUS_FLAG_WAVETABLE & which ) {
    WriteReg16( base,
                AMIGUS_WT_INT_ENABLE_0,
                AMIGUS_INT_F_CLEAR | AMIGUS_INT_F_TIMER | 0x3FFF);
  } else {
    WriteReg16( base,
                AMIGUS_CODEC_PCM_INT_ENABLE,
                AMIGUS_INT_F_CLEAR | AMIGUS_INT_F_TIMER | 0x3FFF);
  }
  // Stop the timer
  WriteReg16( base,
              AMIGUS_TIMER_CONTROL,
              AMIGUS_TIMER_STOP );
  // Clear left over timer interrupt status flags
  WriteReg16( base,
              AMIGUS_INT_CONTROL,
              AMIGUS_INT_F_CLEAR | AMIGUS_INT_F_TIMER );
  // Continoues mode - set reload, then start timer
  WriteReg32( base,
              AMIGUS_TIMER_RELOAD,
              AMIGUS_TIMER_CLOCK / expect );
  WriteReg16( base,
              AMIGUS_TIMER_CONTROL,
              AMIGUS_TIMER_START_CONTINOUS );
  // Enable interrupt for timer again
  if ( AMIGUS_FLAG_WAVETABLE & which ) {
    WriteReg16( base,
                AMIGUS_WT_INT_ENABLE_0,
                AMIGUS_INT_F_SET | AMIGUS_INT_F_TIMER );
  } else {
    WriteReg16( base,
                AMIGUS_CODEC_PCM_INT_ENABLE,
                AMIGUS_INT_F_SET | AMIGUS_INT_F_TIMER );
  }

  Delay( 50 ); // wait 1s

  // Stop the timer
  WriteReg16( base,
              AMIGUS_TIMER_CONTROL,
              AMIGUS_TIMER_STOP );
  // Deactivate timer interrupt for this card
  if ( AMIGUS_FLAG_WAVETABLE & which ) {
    WriteReg16( base,
                AMIGUS_WT_INT_ENABLE_0,
                AMIGUS_INT_F_CLEAR | AMIGUS_INT_F_TIMER );
  } else {
    WriteReg16( base,
                AMIGUS_CODEC_PCM_INT_ENABLE,
                AMIGUS_INT_F_CLEAR | AMIGUS_INT_F_TIMER );
  }
  // Clear left over timer interrupt status flags
  WriteReg16( base,
              AMIGUS_INT_CONTROL,
              AMIGUS_INT_F_CLEAR | AMIGUS_INT_F_TIMER );

  AmiGUS_RemoveInterrupt( card,
                          which,
                          owner );

  printf( "Interrupt was called %ld times out of expected %ld - ",
          interruptCounter, expect );

  if ((( expect - ( expect / 100 )) <= interruptCounter ) &&
    (( expect + ( expect / 100 )) >= interruptCounter )) {

    printf( "OK\n" );
    return FALSE;
  }
  printf( "Failed!\n" );
  return TRUE;
}

BOOL testOneShotTimerInterrupt( struct AmiGUS * card,
                                LONG which, 
                                APTR base,
                                APTR owner,
                                ULONG interval ) {

  ULONG installed;

  interruptCounter = 0;

  printf( "Using card 0x%08lx part 0x%04lx base 0x%08lx for owner 0x%08lx\n",
          card, which, base, owner );

  installed = AmiGUS_InstallInterrupt( card,
                                       which,
                                       owner,
                                       &( countingIntHandler ),
                                       base );
  if ( AmiGUS_NoError != installed ) {

    return TRUE;
  }

  // Deactivate all interrupts for this card
  if ( AMIGUS_FLAG_WAVETABLE & which ) {
    WriteReg16( base,
                AMIGUS_WT_INT_ENABLE_0,
                AMIGUS_INT_F_CLEAR | AMIGUS_INT_F_TIMER | 0x3FFF);
  } else {
    WriteReg16( base,
                AMIGUS_CODEC_PCM_INT_ENABLE,
                AMIGUS_INT_F_CLEAR | AMIGUS_INT_F_TIMER | 0x3FFF);
  }
  // Stop the timer
  WriteReg16( base,
              AMIGUS_TIMER_CONTROL,
              AMIGUS_TIMER_STOP );
  // Clear left over timer interrupt status flags
  WriteReg16( base,
              AMIGUS_INT_CONTROL,
              AMIGUS_INT_F_CLEAR | AMIGUS_INT_F_TIMER );
  // One shot mode - start timer, then set reload
  WriteReg16( base,
              AMIGUS_TIMER_CONTROL,
              AMIGUS_TIMER_START_ONCE );
  WriteReg32( base,
              AMIGUS_TIMER_RELOAD,
              AMIGUS_TIMER_CLOCK / interval );
  // Enable interrupt for timer again
  if ( AMIGUS_FLAG_WAVETABLE & which ) {
    WriteReg16( base,
                AMIGUS_WT_INT_ENABLE_0,
                AMIGUS_INT_F_SET | AMIGUS_INT_F_TIMER );
  } else {
    WriteReg16( base,
                AMIGUS_CODEC_PCM_INT_ENABLE,
                AMIGUS_INT_F_SET | AMIGUS_INT_F_TIMER );
  }

  Delay( 50 ); // wait 1s

  // Stop the timer
  WriteReg16( base,
              AMIGUS_TIMER_CONTROL,
              AMIGUS_TIMER_STOP );
  // Deactivate timer interrupt for this card
  if ( AMIGUS_FLAG_WAVETABLE & which ) {
    WriteReg16( base,
                AMIGUS_WT_INT_ENABLE_0,
                AMIGUS_INT_F_CLEAR | AMIGUS_INT_F_TIMER );
  } else {
    WriteReg16( base,
                AMIGUS_CODEC_PCM_INT_ENABLE,
                AMIGUS_INT_F_CLEAR | AMIGUS_INT_F_TIMER );
  }
  // Clear left over timer interrupt status flags
  WriteReg16( base,
              AMIGUS_INT_CONTROL,
              AMIGUS_INT_F_CLEAR | AMIGUS_INT_F_TIMER );

  AmiGUS_RemoveInterrupt( card,
                          which,
                          owner );

  printf( "Interrupt was called %ld times out of expected %ld - ",
          interruptCounter, 1 );

  if ( 1 == interruptCounter ) {

    printf( "OK\n" );
    return FALSE;
  }
  printf( "Failed!\n" );
  return TRUE;
}

VOID SleepCodecTicks( APTR amiGUS, ULONG ticks ) {

  LONG rounds = 0;

  //printf( "V: Sleeping for %ld ticks\n", ticks );
  WriteReg16( amiGUS, 
              AMIGUS_TIMER_CONTROL,
              AMIGUS_TIMER_STOP );
#if 0
  WriteReg16( amiGUS,
              AMIGUS_WT_INT_ENABLE_0,
              AMIGUS_INT_F_CLEAR | AMIGUS_INT_F_TIMER );
  WriteReg16( amiGUS,
              AMIGUS_CODEC_PCM_INT_ENABLE,
              AMIGUS_INT_F_CLEAR | AMIGUS_INT_F_TIMER );
#endif
  WriteReg16( amiGUS,
              AMIGUS_INT_CONTROL,
              AMIGUS_INT_F_CLEAR | AMIGUS_INT_F_TIMER );
  WriteReg16( amiGUS, 
              AMIGUS_TIMER_CONTROL,
              AMIGUS_TIMER_START_ONCE );
  WriteReg32( amiGUS, AMIGUS_TIMER_RELOAD, ticks );
  while ( !( AMIGUS_INT_F_TIMER 
          & ReadReg16( amiGUS, AMIGUS_INT_CONTROL ))) {

    ++rounds;
  }
  //printf( "V: Woke up after %ld rounds.\n", rounds );
}

BOOL testShortSleep( APTR base ) {

  BOOL failed = FALSE;

  // Create a message port for the device communication
  struct MsgPort * timerPort = CreateMsgPort();
  if ( timerPort ) {

    // Create the I/O Request structure
    struct TimeRequest * timerRequest =
      ( struct TimeRequest * ) CreateIORequest( timerPort,
                                                sizeof( struct TimeRequest ));
    if ( timerRequest ) {

      // Open timer.device 
      BYTE openError = OpenDevice( "timer.device",
                                   UNIT_MICROHZ,
                                   ( struct IORequest * ) timerRequest,
                                   0 );
      if ( !openError ) { 
        struct EClockVal start;
        struct EClockVal end;
        ULONG ticsPerSecond;
        ULONG elapsedTicks;
        double millis;
        ULONG roundedMillis;

        // Set up the library base pointer to call the function directly
        TimerBase = ( struct TimerBase * ) timerRequest->tr_node.io_Device;

        ticsPerSecond = ReadEClock( &( start ));

        // Delay( 50 / 8 );
        SleepCodecTicks( base, AMIGUS_TIMER_CLOCK >> 3);

        ReadEClock( &( end ));

        if ( end.ev_lo >= start.ev_lo ) {

          elapsedTicks = end.ev_lo - start.ev_lo;

        } else {

          elapsedTicks = (0xFFFFFFFF - start.ev_lo) + end.ev_lo + 1;
        }

        millis = (( double ) elapsedTicks * 1000.0 ) / ( double ) ticsPerSecond;
        roundedMillis = (( elapsedTicks * 1000 ) / ticsPerSecond );

#if 0
        printf( "E-Clock frequency: %uHz\n", ticsPerSecond);
        printf( "Start: Hi=%u, Lo=%u\n", start.ev_hi, start.ev_lo );
        printf( "End: Hi=%u, Lo=%u\n", end.ev_hi, end.ev_lo );
#endif
        printf( "Elapsed ticks: %u ticks or %fms (rounded %ldms) "
                "for 1/8th second - ",
                elapsedTicks, millis, roundedMillis );

        if (( 120 > roundedMillis ) || ( 130 < roundedMillis )) {

          printf( "Insufficient timer precision!\n" );
          failed |= TRUE;
          printf( "Failed!\n" );

        } else {

          printf( "OK\n" );
        }
        CloseDevice(( struct IORequest * ) timerRequest );

      } else {
    
        printf( "Could not open Timer Device\n" );
        failed |= TRUE;
      }

      DeleteIORequest(( struct IORequest * ) timerRequest );

    } else {
    
      printf( "Could not create Timer Request\n" );
      failed |= TRUE;
    }

    DeleteMsgPort( timerPort );

  } else {

    printf( "Could not create Message Port\n" );
    failed |= TRUE;
  }
  return failed;
}

/******************************************************************************
 * Finally, main triggering all tests:
 *****************************************************************************/
int main(int argc, char const *argv[]) {

  BOOL failed = FALSE;
  STRPTR libraryName = "amigus.library";
  struct AmiGUS * card;
  APTR owner;
  ULONG reserve;

  AmiGUS_Base = ( struct AmiGUS_Base * ) OpenLibrary( libraryName, 0 );
  if ( !AmiGUS_Base ) {

    printf( "Opening %s failed!\n", libraryName );
    return 20;
  }
  printf( "%s opened\n", libraryName );

  card = AmiGUS_FindCard( NULL );

  // Owner can be anything unique you own. :)
  owner = FindTask( NULL );
  if ( NULL == card ) {

    printf( "No Card found.\n" );
    return 20;
  }

  reserve = AmiGUS_ReserveCard( card,
                                AMIGUS_FLAG_PCM 
                                | AMIGUS_FLAG_WAVETABLE
                                | AMIGUS_FLAG_CODEC,
                                owner );
  if ( AmiGUS_NoError != reserve ) {

    printf( "Could not reserve card for owner 0x%08lx, reason 0x%04lx.\n",
            owner, reserve );
    return 20;
  }
#if 1
  failed |= testContinousTimerInterrupt( card,
                                         AMIGUS_FLAG_CODEC,
                                         card->agus_CodecBase,
                                         owner,
                                         100 );
  failed |= testContinousTimerInterrupt( card,
                                         AMIGUS_FLAG_PCM,
                                         card->agus_PcmBase,
                                         owner,
                                         1000 );
  failed |= testContinousTimerInterrupt( card,
                                         AMIGUS_FLAG_WAVETABLE,
                                         card->agus_WavetableBase,
                                         owner,
                                         10000 );
  failed |= testOneShotTimerInterrupt( card,
                                       AMIGUS_FLAG_CODEC,
                                       card->agus_CodecBase,
                                       owner,
                                       100 );
  failed |= testOneShotTimerInterrupt( card,
                                       AMIGUS_FLAG_PCM,
                                       card->agus_PcmBase,
                                       owner,
                                       1000 );
  failed |= testOneShotTimerInterrupt( card,
                                       AMIGUS_FLAG_WAVETABLE,
                                       card->agus_WavetableBase,
                                       owner,
                                       10000 );
#endif
  failed |= testShortSleep( card->agus_CodecBase );
  failed |= testShortSleep( card->agus_PcmBase );
  failed |= testShortSleep( card->agus_WavetableBase );
  AmiGUS_FreeCard( card,
                   AMIGUS_FLAG_PCM 
                   | AMIGUS_FLAG_WAVETABLE
                   | AMIGUS_FLAG_CODEC,
                   owner );

  CloseLibrary(( struct Library * ) AmiGUS_Base );
  printf( "%s closed\n", libraryName );

  if ( failed ) {

    printf( "Failed. :(\n" );

  } else {

    printf( "Looks good.\n" );
  }

  return ( failed ) ? 15 : 0;
}
