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

#include <limits.h>

#include <proto/exec.h>

#include "amisf.h"
#include "debug.h"
#include "sf2.h"
#include "sf2_tools.h"
#include "support.h"

struct ConversionRates {

  struct Node cpr_Node;

  ULONG cpr_SampleRate;
  UBYTE cpr_SampleNote;
  UBYTE cpr_Padding0;
  UWORD cpr_Padding1;
};

struct ConversionInfo {

  ULONG ci_NoteCount;
  ULONG ci_SampleCount;
  UWORD ci_SampleMapping[ 65536 ];
  // 0 in mapping: empty
  // 1 in mapping: sf2 index mapped to that new index

  UWORD ci_PlaybackRateCount;
  struct List ci_PlaybackRatesNeeded;
};

static LONG CompareRates( struct Node * a, struct Node * b ) {

  struct ConversionRates * aa = ( struct ConversionRates * ) a;
  struct ConversionRates * bb = ( struct ConversionRates * ) b;
  LONG rateDifference = aa->cpr_SampleRate - bb->cpr_SampleRate;

  if ( rateDifference ) {

    return rateDifference;
  }

  return aa->cpr_SampleNote - bb->cpr_SampleNote;
}

static struct ConversionInfo * CreateConversionInfo( struct SF2 * sf2 ) {

  UWORD count = 1;
  struct ConversionInfo * info = AllocMem( sizeof( struct ConversionInfo ),
                                           MEMF_ANY | MEMF_CLEAR );
  struct SF2_Preset * sf2Preset;

  NEW_LIST( &( info->ci_PlaybackRatesNeeded ));

  /* Begin iteration over all presets - instruments - samples */
  FOR_LIST( &( sf2->sf2_Presets ),
            sf2Preset,
            struct SF2_Preset * ) {

    struct SF2_Args * argsP;

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

        if ( 0 > sampleIndex ) {

          // Skip over the de-duplicated samples
          continue;
        }

        if (( sampleMin > instrumentMax )
          || ( instrumentMin > sampleMax)) {

          // Skip over unreachable samples
          continue;
        }

        tempSample = sf2->sf2_SampleArray[ sampleIndex ];
        /* Iteration payload below */
        if ( tempSample->sf2s_Number != sampleIndex ) {

          LOG_E(( "E: SampleIndex %ld != Sample's number %ld\n", 
                  sampleIndex, tempSample->sf2s_Number ));

        } else if ( !( info->ci_SampleMapping[ sampleIndex ] )) {

          info->ci_SampleMapping[ sampleIndex ] = count;
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
        }

        /* Complete iteration over all presets - instruments - samples */
      }
    }
  }
  --count;
  info->ci_SampleCount = count;
  LOG_I(( "I: AmiSF will have %ld notes, "
          "contain %ld of %ld samples in SF2.\n",
          info->ci_NoteCount,
          info->ci_SampleCount, sf2->sf2_SampleCount ));
  return info;
}

static VOID FreeConversionInfo( struct ConversionInfo * info ) {

  struct Node * node;

  if ( !( info )) {

    return;
  }

  while ( node = RemHead( &( info->ci_PlaybackRatesNeeded ))) {

    FreeMem( node, sizeof( struct ConversionRates ));
  }

  FreeMem( info, sizeof( struct ConversionInfo ));
}

static VOID FillPresetNotes( struct SF2 * sf2,
                             struct ConversionInfo * info,
                             struct AmiSF * amisf ) {

  UBYTE lastBank = 0;
  UBYTE lastPreset = 0;
  ULONG nextNodeIndex = 0;
  struct SF2_Preset * sf2Preset;

  /* Begin iteration over all presets - instruments - samples */
  FOR_LIST( &( sf2->sf2_Presets ),
            sf2Preset,
            struct SF2_Preset * ) {

    struct SF2_Args * argsP;
    const LONG bank = sf2Preset->sf2p_Bank;
    const LONG preset = sf2Preset->sf2p_Common.sf2c_Number;

    FOR_LIST( &( sf2Preset->sf2p_Args ),
              argsP,
              struct SF2_Args * ) {

      const LONG instrumentIndex = argsP->sf2a_Values.sf2v_NextNumber;

      struct SF2_Instrument * tempInstrument;
      struct SF2_Args * argsI;
      UBYTE minNote = 0;

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
        struct AmiSF_Preset * asf_Preset;
        struct AmiSF_Note * asf_Note;
        UWORD i;

        if ( 0 > sampleIndex ) {

          // Skip over the de-duplicated samples
          continue;
        }

        if (( sampleMin > instrumentMax )
          || ( instrumentMin > sampleMax)) {

          // Skip over unreachable samples
          continue;
        }

        tempSample = sf2->sf2_SampleArray[ sampleIndex ];

        /* Iteration payload below */
        // Update preset in case needed
        if (( lastBank != bank ) || ( lastPreset != preset )) {

          struct AmiSF_Preset * lastPresetPointer =
            &( amisf->asf_Preset[ lastBank ][ lastPreset ]);
          LONG lastNoteIndex =
            lastPresetPointer->asfp_NoteStart 
            + lastPresetPointer->asfp_NoteCount
            - 1;
          struct AmiSF_Note * lastNote = &( amisf->asf_Note[ lastNoteIndex ]);

          LOG_D(( "V: last ended %lu\n", lastNote->asfn_MaxNote ));
          nextNodeIndex += lastPresetPointer->asfp_NoteCount;

          lastBank = bank;
          lastPreset = preset;
        }

        asf_Preset =  &( amisf->asf_Preset[ bank ][ preset ]);
        asf_Preset->asfp_Bank = bank;
        asf_Preset->asfp_Preset = preset;
        asf_Preset->asfp_NoteStart = nextNodeIndex;

        // Remember the current note and advance to the next
        asf_Note = &( amisf->asf_Note[ 
          asf_Preset->asfp_NoteStart + asf_Preset->asfp_NoteCount ]);
        ++asf_Preset->asfp_NoteCount;

        // Update the current note
        asf_Note->asfn_MaxNote = MIN( instrumentMax, sampleMax );
        for ( i = 0; i < amisf->asf_SampleRateCount; ++i ) {
        
          if ( tempSample->sf2s_SampleRate ==  amisf->asf_SampleRate[ i ]) {
          
            asf_Note->asfn_BasePlaybackIndex =
              amisf->asf_PlaybackRateOffset[ i ];
            LOG_D(( "V: Resolved %ldHz to index %ld\n",
                    tempSample->sf2s_SampleRate,
                    asf_Note->asfn_BasePlaybackIndex ));
            break;
          }
        }

        // TODO: Create all that rates here!

        asf_Note->asfn_Volume = 0x4001;
        asf_Note->asfn_Attack = 1;
        asf_Note->asfn_Decay = 2;
        asf_Note->asfn_Sustain = 3;
        asf_Note->asfn_Release = 4;

        asf_Note->asfn_SampleIndex = info->ci_SampleMapping[ sampleIndex ];

        /* Complete iteration over all presets - instruments - samples */
        LOG_D(( "V: bank %ld, preset %ld now has "
                "%ld notes "
                "starting at %ld.\n",
                bank, preset,
                asf_Preset->asfp_NoteCount,
                asf_Preset->asfp_NoteStart ));
      }
    }
  }
}

static VOID FillRates( struct ConversionInfo * info, struct AmiSF * amisf ) {

  LONG sampleRateCount = 0;
  LONG playbackRateCount = 0;
  LONG sampleRateCheck = 0;
  LONG playbackRateCheck = 0;
  struct ConversionRates * rate;

  // Walk all the playback rates collected,
  // find the max and min notes for each rate and
  // calculate how many sample rates we need for playback here.
  FOR_LIST( &( info->ci_PlaybackRatesNeeded ),
            rate,
            struct ConversionRates * ) {

    struct ConversionRates * end = rate;

    const ULONG sampleRate = rate->cpr_SampleRate;
    UBYTE minSampleNote = rate->cpr_SampleNote;
    const UBYTE maxSampleNote = rate->cpr_SampleNote;

    while ( sampleRate == end->cpr_SampleRate ) {

      LOG_V(( "V: Checking for sample rate %ldHz, max note is %ld, current note is %ld\n",
              sampleRate, maxSampleNote, end->cpr_SampleNote ));

      minSampleNote = end->cpr_SampleNote;
      end = ( struct ConversionRates * ) end->cpr_Node.ln_Succ;
    }
    playbackRateCount += maxSampleNote + 128 - minSampleNote;
    LOG_V(( "V: For sample rate %ldHz, "
            "min note is %ld, max note is %ld, "
            "so far needs %ld\n",
            sampleRate,
            minSampleNote, maxSampleNote,
            playbackRateCount ));
    
    ++sampleRateCount;
 
    // Skipping over the same rate - different notes we have handled already now.
    rate = ( struct ConversionRates * ) end->cpr_Node.ln_Pred;
  }
  LOG_D(( "D: Found %ld distinct sample rates and %ld playback rates\n",
          sampleRateCount,
          playbackRateCount ));

  amisf->asf_SampleRateCount = sampleRateCount;
  amisf->asf_SampleRate = AllocMem( sampleRateCount * sizeof( ULONG ),
                                    MEMF_ANY | MEMF_CLEAR );
  amisf->asf_PlaybackRateOffset = AllocMem( sampleRateCount * sizeof( ULONG ),
                                            MEMF_ANY | MEMF_CLEAR );

  amisf->asf_PlaybackRateCount = playbackRateCount;
  amisf->asf_PlaybackRate = AllocMem( playbackRateCount * sizeof( ULONG ),
                                      MEMF_ANY | MEMF_CLEAR );

  sampleRateCheck = sampleRateCount;
  playbackRateCheck = playbackRateCount;
  sampleRateCount = 0;
  playbackRateCount = 0;

  // 
  FOR_LIST( &( info->ci_PlaybackRatesNeeded ),
            rate,
            struct ConversionRates * ) {

    struct ConversionRates * end = rate;

    const ULONG sampleRate = rate->cpr_SampleRate;
    ULONG minSampleNote = rate->cpr_SampleNote;
    const ULONG maxSampleNote = rate->cpr_SampleNote;
    ULONG i;

    while ( sampleRate == end->cpr_SampleRate ) {

      minSampleNote = end->cpr_SampleNote;
      end = ( struct ConversionRates * ) end->cpr_Node.ln_Succ;
    }

    // Calculate all up until maxSampleNote
    for ( i = 0; i < maxSampleNote ; ++i ) {

      amisf->asf_PlaybackRate[ playbackRateCount + i ] =
        GetTargetSampleRate( maxSampleNote, sampleRate, i );
    }

    // Remember the sample rate, and where it will be found
    amisf->asf_SampleRate[ sampleRateCount ] =
      rate->cpr_SampleRate;
    amisf->asf_PlaybackRateOffset[ sampleRateCount ] =
      i + playbackRateCount;
    ++sampleRateCount;

    // Calculate maxSampleNote and all up until 127
    for ( i = maxSampleNote;
          i < ( maxSampleNote + 128 - minSampleNote );
          ++i ) {

      amisf->asf_PlaybackRate[ playbackRateCount + i ] =
        GetTargetSampleRate( minSampleNote,
                             sampleRate,
                             i - maxSampleNote + minSampleNote );
    }
    playbackRateCount += i;

    // Skipping over the same rate - different notes we have handled already now.
    rate = ( struct ConversionRates * ) end->cpr_Node.ln_Pred;
  }
  LOG_D(( "D: Calc'd %ld distinct sample rates and %ld playback rates\n",
          sampleRateCount,
          playbackRateCount ));
  LOG_I(( "I: Sample rate conversion %s.\n",
          (( sampleRateCount == sampleRateCheck )
            && ( playbackRateCount == playbackRateCount ))
            ? "successful" : "failed" ));
}

struct AmiSF * AllocAmiSFfromSF2( struct SF2 * sf2 ) {

  ULONG allocatedSize = 0;
  struct SF2_Preset * sf2Preset;

  struct ConversionInfo * info = CreateConversionInfo( sf2 );

  struct AmiSF * amisf = AllocMem( sizeof( struct AmiSF ),
                                   MEMF_ANY | MEMF_CLEAR );
  allocatedSize += sizeof( struct AmiSF );

  amisf->asf_SampleCount = info->ci_SampleCount;
  amisf->asf_SampleMetadata = AllocMem(
    sizeof( struct AmiSF_Sample ) * amisf->asf_SampleCount,
    MEMF_ANY | MEMF_CLEAR );
  allocatedSize += sizeof( struct AmiSF_Sample ) * amisf->asf_SampleCount;

  amisf->asf_NoteCount = info->ci_NoteCount;
  amisf->asf_Note = AllocMem(
    sizeof( struct AmiSF_Note ) * amisf->asf_NoteCount,
    MEMF_ANY | MEMF_CLEAR );
  allocatedSize += sizeof( struct AmiSF_Note ) * amisf->asf_NoteCount;

  FillRates( info, amisf );
  FillPresetNotes( sf2, info, amisf );

  FreeConversionInfo( info );
  LOG_D(( "Allocated size for AmiSF is %ld\n", allocatedSize ));
  return amisf;
}


VOID FreeAmiSF( struct AmiSF * amisf ) {

  if ( !amisf ) {

    return;
  }
  if ( amisf->asf_SampleMetadata ) {

    FreeMem( amisf->asf_SampleMetadata,
             sizeof( struct AmiSF_Sample ) * amisf->asf_SampleCount );
    amisf->asf_SampleMetadata = NULL;
  }

  FreeMem( amisf, sizeof( struct AmiSF ));
}
