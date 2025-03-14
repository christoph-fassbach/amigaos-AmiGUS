/*
 * This file is part of the mhiAmiGUS.library.
 *
 * mhiAmiGUS.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiAmiGUS.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiAmiGUS.library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <exec/types.h>
#include <libraries/mhi.h>
#include <libraries/configvars.h>

#include <proto/exec.h>

#include "amigus_codec.h"
#include "amigus_hardware.h"
#include "amigus_mhi.h"
#include "debug.h"
#include "errors.h"
#include "interrupt.h"
#include "support.h"
#include "SDI_compiler.h"

#if defined(__VBCC__)
  #define AMIGA_INTERRUPT __amigainterrupt
#elif defined(__SASC)
  #define AMIGA_INTERRUPT __interrupt
#endif

VOID FlushAllBuffers( struct AmiGUSClientHandle * clientHandle ) {

  struct List * buffers = ( struct List * )&clientHandle->agch_Buffers;
  struct Node * buffer;

  while ( buffer = RemHead( buffers )) {

    LOG_V(( "V: Removing / free'ing MHI buffer 0x%08lx, "
      "size %ld LONGs, index %ld\n",
      buffer,
      (( struct AmiGUSMhiBuffer * ) buffer)->agmb_BufferMax,
      (( struct AmiGUSMhiBuffer * ) buffer)->agmb_BufferIndex ));
    FreeMem( buffer, sizeof( struct AmiGUSMhiBuffer ) );
  }
  clientHandle->agch_CurrentBuffer = NULL;
  LOG_D(( "D: All buffers flushed.\n" ));
}

VOID UpdateEqualizer( UWORD bandLevel, UWORD percent ) {

  APTR amiGUScard = AmiGUSmhiBase->agb_CardBase;
  // -32 .. +32 = ((( 0 .. 100 ) * 2655 ) / 4096 ) - 32
  // gain = percent * 2655 / 4096 - 32
  LONG intermediate = (( percent * 2655 ) >> 12 ) - 32;
  WORD gain = ( WORD ) intermediate;
  LOG_D(( "D: Calculated gain %ld = %ld for %ld%\n",
          intermediate, gain, percent ));

  UpdateVS1063Equalizer( amiGUScard, bandLevel, gain );
}

ASM( APTR ) SAVEDS LIB_MHIAllocDecoder(
  REG( a0, struct Task * task ),
  REG( d0, ULONG signal ),
  REG( a6, struct AmiGUSmhi * base )
) {

  if ( base != AmiGUSmhiBase ) {

    DisplayError( ELibraryBaseInconsistency );
  }
  
  return NULL;
}

ASM( VOID ) SAVEDS LIB_MHIFreeDecoder(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUSmhi * base )
) {

  return;
}

ASM( BOOL ) SAVEDS LIB_MHIQueueBuffer(
  REG( a3, APTR handle ),
  REG( a0, APTR buffer ),
  REG( d0, ULONG size),
  REG( a6, struct AmiGUSmhi * base )
) {

  return TRUE;
}

ASM( APTR ) SAVEDS LIB_MHIGetEmpty(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUSmhi * base )
) {

  return NULL;
}

ASM( UBYTE ) SAVEDS LIB_MHIGetStatus(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUSmhi * base )
) {

  return ( UBYTE ) 17;
}

ASM( VOID ) SAVEDS LIB_MHIPlay(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUSmhi * base )
) {

  return;
}

ASM( VOID ) SAVEDS LIB_MHIStop(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUSmhi * base )
) {

  return;
}

ASM( VOID ) SAVEDS LIB_MHIPause(
  REG( a3, APTR handle ),
  REG( a6, struct AmiGUSmhi * base )
) {

  return;
}

ULONG __near call_count_near = 0L;
ULONG __far call_count_far = 0L;
static ULONG call_count_static = 0L;

ASM( ULONG ) SAVEDS LIB_MHIQuery(
  REG( d1, ULONG query ),
  REG( a6, struct AmiGUSmhi * base )
) {

  ULONG result = MHIF_UNSUPPORTED;

  LOG_D(( "D: MHIQuery start\n" ));
  switch ( query ) {
    case MHIQ_DECODER_NAME: {

      result = ( ULONG ) AMIGUS_MHI_DECODER;
      break;
    }
    case MHIQ_DECODER_VERSION: {

      result = ( ULONG ) AMIGUS_MHI_VERSION;
      break;
    }
    case MHIQ_AUTHOR: {

      result = ( ULONG ) AMIGUS_MHI_AUTHOR    " \n"        \
                         AMIGUS_MHI_COPYRIGHT " \n"        \
                         AMIGUS_MHI_ANNOTATION;
      break;
    }
    case MHIQ_CAPABILITIES: {

      result = ( ULONG )                                   \
               "audio/mpeg{audio/mp2,audio/mp3},"          \
               "audio/ogg{audio/vorbis},"                  \
               "audio/mp4{audio/aac},audio/aac,"           \
               "audio/flac";
      break;
    }
    case MHIQ_IS_HARDWARE:
    case MHIQ_MPEG1:
    case MHIQ_MPEG2:
    case MHIQ_MPEG25:
    case MHIQ_MPEG4:
    case MHIQ_LAYER1:
    case MHIQ_LAYER2:
    case MHIQ_LAYER3:
    case MHIQ_VARIABLE_BITRATE:
    case MHIQ_JOINT_STEREO:
    case MHIQ_BASS_CONTROL:
    case MHIQ_TREBLE_CONTROL:
    case MHIQ_MID_CONTROL:
    case MHIQ_5_BAND_EQ: {

      result = MHIF_SUPPORTED;
      break;
    }
    case MHIQ_IS_68K:
    case MHIQ_IS_PPC:
    case MHIQ_PREFACTOR_CONTROL:
    case MHIQ_10_BAND_EQ:
    case MHIQ_VOLUME_CONTROL:  // TODO
    case MHIQ_PANNING_CONTROL: // TODO
    case MHIQ_CROSSMIXING:     // TODO
    default: {
      break;
    }
  }
  LOG_V(( "V: Query %ld, result %ld = 0x%08lx \n", query, result, result ));
  LOG_D(( "D: MHIQuery done\n" ));
  return result;
}

ASM( VOID ) SAVEDS LIB_MHISetParam(
  REG( a3, APTR handle ),
  REG( d0, UWORD param ),
  REG( d1, ULONG value ),
  REG( a6, struct AmiGUSmhi * base )
) {

  return;
}
