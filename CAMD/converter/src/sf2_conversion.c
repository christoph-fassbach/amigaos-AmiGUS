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

#include <proto/exec.h>

#include "debug.h"
#include "sf2.h"
#include "sf2_conversion.h"
#include "support.h"

static LONG CompareRates( struct Node * a, struct Node * b ) {

  struct ConversionRates * aa = ( struct ConversionRates * ) a;
  struct ConversionRates * bb = ( struct ConversionRates * ) b;
  LONG rateDifference = aa->cpr_SampleRate - bb->cpr_SampleRate;

  if ( rateDifference ) {

    return rateDifference;
  }

  return aa->cpr_SampleNote - bb->cpr_SampleNote;
}

struct ConversionInfo * AllocConversionInfo( struct SF2 * sf2 ) {

  UWORD count = 0;
  struct ConversionInfo * info = AllocMem( sizeof( struct ConversionInfo ),
                                           MEMF_ANY | MEMF_CLEAR );
  struct SF2_Preset * sf2Preset;

  NEW_LIST( &( info->ci_PlaybackRatesNeeded ));

  /* Begin iteration over all presets - instruments - samples */
  FOR_LIST( &( sf2->sf2_Presets ),
            sf2Preset,
            struct SF2_Preset * ) {

    struct SF2_Args * argsP;
    const LONG bank = sf2Preset->sf2p_Bank;
    const LONG preset = sf2Preset->sf2p_Common.sf2c_Number;
    LONG currentPresetMaxNote = -1;

    FOR_LIST( &( sf2Preset->sf2p_Args ),
              argsP,
              struct SF2_Args * ) {

      const LONG instrumentIndex = argsP->sf2a_Values.sf2v_NextNumber;

      struct SF2_Instrument * tempInstrument;
      struct SF2_Args * argsI;

      if ( 0 > instrumentIndex ) {

        // Skip over the de-duplicated instruments
        continue;
      }

      tempInstrument = sf2->sf2_InstrumentArray[ instrumentIndex ];

      FOR_LIST( &( tempInstrument->sf2i_Args ),
                argsI,
                struct SF2_Args * ) {

        const LONG sampleIndex = argsI->sf2a_Values.sf2v_NextNumber;
        const LONG sampleMin = argsI->sf2a_Values.sf2v_LowNote;
        const LONG sampleMax = argsI->sf2a_Values.sf2v_HighNote;
        const LONG instrumentMin = argsP->sf2a_Values.sf2v_LowNote;
        const LONG instrumentMax = argsP->sf2a_Values.sf2v_HighNote;
        struct SF2_Sample * tempSample;
        struct ConversionRates * rate;
        const LONG noteMin =  MAX( instrumentMin, sampleMin );
        const LONG noteMax =  MIN( instrumentMax, sampleMax );

        if ( 0 > sampleIndex ) {

          // Skip over the de-duplicated samples
          continue;
        }

        if (( sampleMin > instrumentMax )
          || ( instrumentMin > sampleMax)) {

          // Skip over unreachable samples - TODO - use noteMin/Max?
          continue;
        }

        if ( noteMax < currentPresetMaxNote ) {

          // Skip over multiple samples to be played for
          // overlapping note ranges, write warning and pick only first.
          LOG_W(( "W: Found attempt to play multiple samples for "
                 "b:%ld, p:%ld, n:%ld-%ld, that will not sound as expected.\n",
                bank, preset, noteMin, noteMax ));
          continue;
        }
        currentPresetMaxNote = noteMax;

        tempSample = sf2->sf2_SampleArray[ sampleIndex ];
        /* Iteration payload below */
        if ( tempSample->sf2s_Number != sampleIndex ) {

          LOG_E(( "E: SampleIndex %ld != Sample's number %ld\n", 
                  sampleIndex, tempSample->sf2s_Number ));

        } else if ( !( info->ci_Sf2ToAmiSfSample[ sampleIndex ] )) {

          info->ci_Sf2ToAmiSfSample[ sampleIndex ] = count + 1;
          info->ci_AmiSfToSf2Sample[ count ] = sampleIndex + 1;
          ++count;
        }
        ++info->ci_NoteCount;

        rate = AllocMem( sizeof( struct ConversionRates ), MEMF_ANY );
        if ( rate ) {
          rate->cpr_SampleRate = tempSample->sf2s_SampleRate;
          rate->cpr_SampleNote = tempSample->sf2s_SampleNote;
          InsertSorted( ( struct Node * ) rate,
                        &( info->ci_PlaybackRatesNeeded ),
                        &( CompareRates ));
          ++info->ci_PlaybackRateCount;
        }

        /* Complete iteration over all presets - instruments - samples */
      }
    }
  }

  info->ci_SampleCount = count;
  LOG_I(( "I: AmiSF will have %ld notes, "
          "contain %ld of %ld samples in SF2.\n",
          info->ci_NoteCount,
          info->ci_SampleCount, sf2->sf2_SampleCount ));
  return info;
}

VOID FreeConversionInfo( struct ConversionInfo * info ) {

  struct Node * node;

  if ( !( info )) {

    LOG_D(( "D: Cannot free ConversionInfo at NULL.\n" ));
    return;
  }

  while ( node = RemHead( &( info->ci_PlaybackRatesNeeded ))) {

    FreeMem( node, sizeof( struct ConversionRates ));
  }

  FreeMem( info, sizeof( struct ConversionInfo ));
}
