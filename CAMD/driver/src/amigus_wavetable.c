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

#include "amigus_camd.h"
#include "amigus_hardware.h"
#include "amigus_wavetable.h"
#include "debug.h"
#include "errors.h"
#include "support.h"

/******************************************************************************
 * Wavetable convenience functions - public function definitions.
 *****************************************************************************/

VOID InitAmiGus( VOID ) {

  struct AmiGUS_CAMD * base = AmiGUS_CAMD_Base;
  APTR card = base->agb_AmiGUS->agus_WavetableBase;
  WORD i;

  WriteReg16( card, AMIGUS_WT_RESET, AMIGUS_WT_F_RESET_STROBE );
  for ( i = 0; i < 32; ++i ) {

    WriteReg16( card, AMIGUS_WT_CHANNEL_NUMBER, i );
    WriteReg16( card, AMIGUS_WT_CHANNEL_CONTROL, 0x0000 );
    WriteReg32( card, AMIGUS_WT_CHANNEL_START_32BIT, 0x00000000 );
    WriteReg32( card, AMIGUS_WT_CHANNEL_LOOP_32BIT, 0x00000000 );
    WriteReg32( card, AMIGUS_WT_CHANNEL_END_32BIT, 0x00000000 );
    WriteReg16( card, AMIGUS_WT_CHANNEL_VOLUME_LEFT, 0x8000 );
    WriteReg16( card, AMIGUS_WT_CHANNEL_VOLUME_RIGHT, 0x8000 );
  }
  // Set master volume
  WriteReg16( card, AMIGUS_WT_MASTER_VOLUME_LEFT, 0xffff );
  WriteReg16( card, AMIGUS_WT_MASTER_VOLUME_RIGHT, 0xffff );
  LOG_V(( "V: Wavetable @ 0x%08lx init'ed\n", card ));
}

VOID LoadAmiGusWavetableSample( ULONG * source, ULONG target, ULONG size ) {

  struct AmiGUS_CAMD * base = AmiGUS_CAMD_Base;
  APTR card = base->agb_AmiGUS->agus_WavetableBase;
  ULONG i;

  if ( 0x00000003 & size ) {

    LOG_E(( "E: Loading sample will go sideways... "
            "size is %ld needs to be ULONGs!\n",
            size ));
  }
  size >>= 2;
  if ( 0xFF000003 & target ) {

    LOG_E(( "E: Loading sample will go sideways... "
            "target is 0x%08lx needs to be UWORDs!\n",
            target ));
    target &= 0x00FFFFFC;
  }

  WriteReg32( card, AMIGUS_WT_ADDRESS_32BIT, target );
  for ( i = 0; i < size; ++i ) {

    ULONG data = source[ i ];
    WriteReg32( card, AMIGUS_WT_DATA_32BIT, data );
  }
  LOG_D(( "D: Copied %ld LONGs to 0x%08lx\n", i, target ));
}

VOID StartAmiGusWavetablePlayback( struct AmiSF_Note * note,
                                   struct AmiSF_Sample * sample ) {

  struct AmiGUS_CAMD * base = AmiGUS_CAMD_Base;
  APTR card = base->agb_AmiGUS->agus_WavetableBase;

  WriteReg16( card, AMIGUS_WT_CHANNEL_NUMBER, 0x0000 );
  WriteReg16( card, AMIGUS_WT_CHANNEL_CONTROL, 0x0000 );

  LOG_D(( "D: Playing from %lx to 0x%08lx\n", 
          sample->amisfs_StartOffset, sample->amisfs_EndOffset ));
  WriteReg32( card, AMIGUS_WT_CHANNEL_START_32BIT, sample->amisfs_StartOffset );
  WriteReg32( card, AMIGUS_WT_CHANNEL_LOOP_32BIT, sample->amisfs_LoopOffset );
  WriteReg32( card, AMIGUS_WT_CHANNEL_END_32BIT, sample->amisfs_EndOffset );
  WriteReg32( card, AMIGUS_WT_CHANNEL_RATE_32BIT, note->amisfn_PlaybackRate );

  WriteReg16( card, AMIGUS_WT_CHANNEL_VOLUME_LEFT, 50000 );
  WriteReg16( card, AMIGUS_WT_CHANNEL_VOLUME_RIGHT, 50000 );

  WriteReg16( card, AMIGUS_WT_CHANNEL_ATTACK, note->amisfn_Attack );
  WriteReg16( card, AMIGUS_WT_CHANNEL_DECAY, note->amisfn_Decay );
  WriteReg16( card, AMIGUS_WT_CHANNEL_SUSTAIN, note->amisfn_Sustain );
  WriteReg16( card, AMIGUS_WT_CHANNEL_RELEASE, note->amisfn_Release );

  WriteReg16( card,
              AMIGUS_WT_CHANNEL_CONTROL,
              AMIGUS_WT_F_CONTROL_START
              | AMIGUS_WT_F_CONTROL_INTERPOLATE
              | ( sample->amisfs_Flags 
                & ( AMISF_NOTE_RESOLUTION_16BIT 
                  | AMISF_NOTE_LOOPED_MASK )));
}