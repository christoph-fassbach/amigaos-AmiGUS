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

#include <proto/dos.h>
#include <proto/exec.h>

#include <amigus/amigus.h>

#include <hardware/intbits.h>

#include "amigus_gayle.h"
#include "amigus_hardware.h"
#include "amigus_private.h"
#include "debug.h"
#include "errors.h"
#include "support.h"
#include "SDI_amigus_protos.h"

/*
 * defines are limited to 32 chars due to a SAS/C insufficiency !!!
 *
 * So define below is just kind of a ruler...
 */
#define SASC_MAXIMUM_DEFINE_LENGTH_IS_32 12345678
#define GAYLE_STATUS_BASE_ADDRESS        0x00DA8000
#define GAYLE_CHANGE_BASE_ADDRESS        0x00DA9000
#define GAYLE_INTERRUPT_BASE_ADDRESS     0x00DAA000
#define GAYLE_CONTROL_BASE_ADDRESS       0x00DAB000
#define GAYLE_HEDLEY_ADDRESS             0x00A40000

// Gayle well defined register contents - for CHANGE
#define GAYLE_CHANGE_IDE_INT             0x0080
#define GAYLE_CHANGE_CCDET               0x0040
#define GAYLE_CHANGE_BVD1                0x0020
#define GAYLE_CHANGE_SC                  0x0020
#define GAYLE_CHANGE_BVD2                0x0010
#define GAYLE_CHANGE_DA                  0x0010
#define GAYLE_CHANGE_WR                  0x0008
#define GAYLE_CHANGE_BSYIRQ              0x0004
#define GAYLE_CHANGE_DETRESET            0x0002
#define GAYLE_CHANGE_DETBERR             0x0001

#define GAYLE_PCMCIA_ASK_HEDLEY_RESET    0x0300 // for CHANGE

// Gayle ID address
#define GAYLE_ID_ADDRESS                 0x00DE1000

// Other chip set addresses
#define CIA_A_BASE_ADDRESS               0x00BFE001
#define CIA_B_BASE_ADDRESS               0x00BFD000
#define CUSTOM_BLTDDAT_ADDRESS           0x00DFf000 // Random noise verification

#define CIA_TIMER_A_HIGH_OFFSET          500
#define CIA_TIMER_A_LOW_OFFSET           400
#define CIA_TIMER_B_HIGH_OFFSET          700
#define CIA_TIMER_B_LOW_OFFSET           600

#define CHAR_TO_ULONG( a, b, c, d )      ((( a ) << 24 ) | (( b ) << 16 ) | (( c ) << 8 ) | ( d ))
#define AMIGUS_MINI_CARD_ID_LOW          CHAR_TO_ULONG( 'A', 'M' ,'I', 'G' )
#define AMIGUS_MINI_CARD_ID_HIGH         CHAR_TO_ULONG( 'U', 'S' ,'M', 'N' )
#define AMIGUS_MINI_ID_LOW_OFFSET        0x80
#define AMIGUS_MINI_ID_HIGH_OFFSET       0x84
#define AMIGUS_MINI_SERIAL_OFFSET        0x88
#define AMIGUS_MINI_HARDWARE_ID          0x8C
#define AMIGUS_MINI_PCM_OFFSET           0x00000100
#define AMIGUS_MINI_CODEC_OFFSET         0x00000200
#define AMIGUS_MINI_WAVETABLE_OFFSET     0x00000300

extern const char LibName[];
extern STRPTR AmiGUS_Mini_Name;

//extern VOID ResetPcmcia( VOID );

UBYTE ReadReg8( APTR reg, ULONG offset ) {

  return *(( UBYTE * )(( ULONG ) reg + offset ));
}

VOID WriteReg8( APTR reg, ULONG offset, UBYTE value ) {

  *(( UBYTE * )(( ULONG ) reg + offset )) = value;
}


/*
Attempt to copy resource.asm / ll. 2016
static VOID ResetPcmcia( VOID ) {

  UWORD i;
  UWORD value = 0;
  
  LOG_I(( "I: Triggering PCMCIA reset.\n" ));

  WriteReg8( NULL, GAYLE_HEDLEY_ADDRESS, 0 );

  for ( i = 0; i < 10; ++i ) {

    value += ReadReg8(( APTR ) CIA_A_BASE_ADDRESS, CIA_TIMER_A_HIGH_OFFSET );
  }
  value += ReadReg8( NULL, GAYLE_HEDLEY_ADDRESS );

  LOG_I(( "I: Read some %ld.\n", value ));
}
*/

static VOID ResetPcmcia( VOID ) {

  UWORD value;
  
  LOG_I(( "I: Triggering PCMCIA reset.\n" ));

  value = ReadReg16(( APTR ) GAYLE_CHANGE_BASE_ADDRESS, 0 );

  WriteReg16(( APTR ) GAYLE_CHANGE_BASE_ADDRESS,
              0,
              value | GAYLE_PCMCIA_ASK_HEDLEY_RESET );
  WriteReg16(( APTR ) GAYLE_CHANGE_BASE_ADDRESS,
              0,
              value );
}

UWORD irqCounter = 0;

ASM( LONG ) /* __entry for vbcc ? */ SAVEDS INTERRUPT HandleGayleInterrupt (
  REG( a1, struct AmiGUS_Base * base )) {

  LONG result = 0;

  UWORD flag = ReadReg16(( APTR ) GAYLE_CHANGE_BASE_ADDRESS, 0 );
  if ( GAYLE_CHANGE_CCDET & flag ) {

    WriteReg16(( APTR ) GAYLE_CHANGE_BASE_ADDRESS,
                0,
                GAYLE_CHANGE_IDE_INT |
                GAYLE_CHANGE_BVD1 |
                GAYLE_CHANGE_BVD2 |
                GAYLE_CHANGE_WR |
                GAYLE_CHANGE_BSYIRQ );
    irqCounter += 1;

    result = 1;
  }
  return result;
}

static BOOL TestForGayle( VOID ) {

  UBYTE finalGayleVersion = 0;

  WORD i;
  WORD j;

  for ( i = 0; i < 4; ++i ) {

    // Similar to https://github.com/reinauer/xSysInfo/blob/main/src/hardware.c#L1274
    
    UBYTE tempGayleVersion = 0;
    
    UBYTE a;
    UBYTE b = ReadReg8( NULL, CUSTOM_BLTDDAT_ADDRESS );

    WriteReg8( NULL, GAYLE_ID_ADDRESS, 0 );
    for ( j = 0; j < 8; ++j ) {

      a = ReadReg8( NULL, GAYLE_ID_ADDRESS );
      if (( 0 == j )     // On first run only,
        && ( a == b )) { // test for garbage on the bus

        LOG_I(( "V: Found bus garbage!\n" ));
        return FALSE;
      }

      LOG_V(( "V: Raw Gayle id is %08lx.\n", a ));

      a &= 0x80;
      tempGayleVersion |= a >> j;
    }
    if ( 0 == i ) {

      finalGayleVersion = tempGayleVersion;

    } else if ( finalGayleVersion != tempGayleVersion ) {

      LOG_I(( "V: Found unstable Gayle version!\n" ));
      return FALSE;
    }
  }

  LOG_I(( "I: Found Gayle Version %ld.%ld.\n",
            ( finalGayleVersion >> 4 ),
            ( finalGayleVersion & 0x0F )));

  if (( finalGayleVersion >> 4 ) == 13 ) {

    LOG_I(( "I: Found %s Gayle.\n",
            (( finalGayleVersion & 0x0F ) == 1 ) ? "A1200" : "A600" ));

    return TRUE;
  }
  return FALSE;
}

static BOOL AddGayleInterrupt( struct Interrupt ** interrupt ) {

  *( interrupt ) = AllocMem( sizeof( struct Interrupt ),
                             MEMF_ANY | MEMF_CLEAR );
  if ( !( *( interrupt ))) {

    return FALSE;
  }

  ( *( interrupt ))->is_Node.ln_Pri = -127; // way after CIA interrupt handler
  ( *( interrupt ))->is_Node.ln_Name = "AmiGUS_Gayle_INT";
  ( *( interrupt ))->is_Data = ( APTR ) AmiGUS_Base;
  ( *( interrupt ))->is_Code = ( VOID ( * )( )) HandleGayleInterrupt;

  AddIntServer( INTB_EXTER, *( interrupt ) );

  return TRUE;
}

static BOOL TestForPCMCIA( VOID ) {

  BOOL success = FALSE;
  UWORD value;

  Disable();

  // Flush all pending interrupts except IDE
  WriteReg16( NULL, GAYLE_CHANGE_BASE_ADDRESS, GAYLE_CHANGE_IDE_INT );

  // Turn on PCMCIA detection interrupt
  value = ReadReg16( NULL, GAYLE_INTERRUPT_BASE_ADDRESS );
  value |= GAYLE_CHANGE_CCDET;
  WriteReg16( NULL, GAYLE_INTERRUPT_BASE_ADDRESS, value );

  // Read back PCMCIA detection status
  value = ReadReg16( NULL, GAYLE_STATUS_BASE_ADDRESS );
  if ( GAYLE_CHANGE_CCDET & value ) {

    success = TRUE;
    ResetPcmcia();

    // ll. 476
    value &= ~GAYLE_CHANGE_BVD2;
    value |= GAYLE_CHANGE_BVD1;
    value |= GAYLE_CHANGE_WR;
    value |= GAYLE_CHANGE_BSYIRQ;
    WriteReg16( NULL, GAYLE_INTERRUPT_BASE_ADDRESS, value );
  }

  Enable();

  return success;
}

VOID AmiGusGayle_AddAll( struct List * cards ) {

  struct AmiGUS_Base * base = AmiGUS_Base;
  BOOL success;

  base->agb_GayleInterrupt = NULL;
#if 0
  if ( 36 <= base->agb_SysBase->LibNode.lib_Version ) {

    LOG_I(( "I: Gayle direct PCMCIA only supported on exec < 36!\n" ));
    return;
  }
  if ( base->agb_CardResource ) {
 
    LOG_I(( "I: Oddly enough, you seem to have a card.resource!\n"
            "Please use it.\n" ));
    return;
  }
#endif
  LOG_I(( "I: Trying to find Gayle...\n" ));

  success = TestForGayle();
  if ( !success ) {

    LOG_I(( "I: No Gayle - no AmiGUS mini.\n" ));
    return;
  }
  LOG_I(( "I: Found Gayle .\nI: Adding interrupt...\n" ));

  success = AddGayleInterrupt( &( base->agb_GayleInterrupt ));
  if ( !success ) {

    LOG_I(( "I: No interrupts - no AmiGUS mini.\n" ));
    return;
  }
  LOG_I(( "I: Added interrupt.\nI: Checking PCMCIA status...\n" ));

  success = TestForPCMCIA();

  LOG_I(( "I: IRQ called %ld times\n", irqCounter ));
  //I: Resetting PCMCIA...\n" ));

}

VOID AmiGusGayle_RemoveAll( struct List * cards ) {
}

LONG AmiGusGayle_InstallInterrupt( VOID ) {

  LOG_I(( "I: Gayle ints not handled yet.\n" ));

  return AmiGUS_NoError;
}

LONG AmiGusGayle_RemoveInterrupt( VOID ) {

  LOG_I(( "I: Gayle ints not handled yet.\n" ));

  return AmiGUS_NoError;
}
