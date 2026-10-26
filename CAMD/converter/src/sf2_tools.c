
/*
 * This file is part of the SoundFontConverter.
 *
 * SoundFontConverter is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3 of the License only.
 *
 * SoundFontConverter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SoundFontConverter.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/mathieeedoubbas.h>
#include <proto/mathieeedoubtrans.h>

#include "sf2_tools.h"

#include "converter.h"
#include "debug.h"
#include "support.h"

/*******************************************************************************
 * Private functions.
 ******************************************************************************/

/*******************************************************************************
 * Public functions.
 ******************************************************************************/

LONG GetSf2SampleSize( struct SF2_Sample * sample ) {

  LONG size = -1;

  if (( sample->sf2s_SampleEndOffset >= sample->sf2s_LoopEndOffset )
    && ( sample->sf2s_LoopEndOffset > sample->sf2s_LoopStartOffset )
    && ( sample->sf2s_LoopStartOffset >= sample->sf2s_SampleStartOffset )) {

    size = sample->sf2s_LoopEndOffset - sample->sf2s_SampleStartOffset;
    size <<= 1;
    size += 3;
    size &= 0xFFffFFfc;

  } else { 

    LOG_E(( "E: Cannot make sense out of sample loop location, "
            "sample: start %ld end %ld, loop: start %ld end %ld\n",
            sample->sf2s_SampleStartOffset,
            sample->sf2s_SampleEndOffset,
            sample->sf2s_LoopStartOffset,
            sample->sf2s_LoopEndOffset ));
  }

  return size;
}

APTR GetSf2SampleData( struct SF2 * sf2,
                       struct SF2_Sample * sample,
                       LONG memorySize ) {

  LONG sampleStart = sample->sf2s_SampleStartOffset << 1; // in BYTE
  LONG loopEnd = sample->sf2s_LoopEndOffset << 1;         // in BYTE
  LONG diskSize = loopEnd - sampleStart;
  LONG i;
  UWORD * samples;

  if (( sampleStart > loopEnd )
    || ( 0 > sampleStart )
    || ( 0 > loopEnd )
    || ( 0 > diskSize )
    || ( 0 > memorySize )) {

    LOG_E(( "E: Implausible sample data, "
            "start %ld, end %ld memsize %ld, disksize %ld\n",
            sampleStart, loopEnd, memorySize, diskSize ));
    return NULL;
  }

  samples = AllocMem( memorySize, MEMF_ANY | MEMF_CLEAR );

  LOG_V(( "V: Samples @ 0x%08lx, start %ld, memsize %ld, disksize %ld\n",
          samples, sampleStart, memorySize, diskSize ));
  Seek( sf2->sf2_FileHandle, 
        sf2->sf2_16bitSamplePosition + sampleStart,
        OFFSET_BEGINNING );
  Read( sf2->sf2_FileHandle, samples, diskSize );

  // Now counting size in 16bit samples!
  for ( i = 0; ( i < diskSize >> 1 ); ++i ) {

    samples[ i ] = Swap16( samples[ i ]);
  }
  if ( diskSize < memorySize ) {

    samples[ i ] = samples[ i - 1 ];
    LOG_I(( "I: Padding sample size %ld to %ld\n", diskSize, memorySize ));
  }

  LOG_V(( "V: 16 bytes of sample data: %04lx %04lx %04lx %04lx\n",
          samples[ 0 ], samples[ 1 ], samples[ 2 ], samples[ 3 ] ));

  return samples;
}

VOID GetSf2ADSRs( struct SF2_Args * presetArgs,
                  struct SF2_Args * instrumentArgs,
                  WORD * attack,
                  WORD * decay,
                  WORD * sustain,
                  WORD * release ) {

  // ADSR combination logic is in 
  // "SoundFont Technical Specification Version 2.01 July 23, 1998"
  // aka SFSpec21.pdf, p.57
  const WORD attackModifier = presetArgs->sf2a_Values.sf2v_Attack;
  const WORD decayModifier = presetArgs->sf2a_Values.sf2v_Decay;
  const WORD sustainModifier = presetArgs->sf2a_Values.sf2v_Sustain;
  const WORD releaseModifier = presetArgs->sf2a_Values.sf2v_Release;

  *attack = instrumentArgs->sf2a_Values.sf2v_Attack;
  *decay = instrumentArgs->sf2a_Values.sf2v_Decay;
  *sustain = instrumentArgs->sf2a_Values.sf2v_Sustain;
  *release = instrumentArgs->sf2a_Values.sf2v_Release;
  if ( PRESET_DEFAULT_ATTACK_VALUE != attackModifier ) {

    *attack += attackModifier;
  }
  if ( PRESET_DEFAULT_DECAY_VALUE != decayModifier ) {

    *decay += decayModifier;
  }
  if ( PRESET_DEFAULT_SUSTAIN_VALUE != sustainModifier ) {

    *sustain += sustainModifier;
  }
  if ( PRESET_DEFAULT_RELEASE_VALUE != releaseModifier ) {

    *release += releaseModifier;
  }
}
