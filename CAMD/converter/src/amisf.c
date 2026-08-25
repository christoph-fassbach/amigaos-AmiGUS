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

#include <proto/dos.h>
#include <proto/exec.h>

#include "amisf.h"
#include "converter.h"
#include "debug.h"
#include "errors.h"
#include "progress_dialog.h"
#include "sf2.h"
#include "sf2_conversion.h"
#include "sf2_tools.h"
#include "support.h"

static ULONG GetPlaybackRateOffset( struct AmiSF * amisf, ULONG sampleRate ) {

  ULONG i;
  for ( i = 0; i < amisf->asf_SampleRateCount; ++i ) {
        
    if ( sampleRate ==  amisf->asf_SampleRate[ i ]) {
          
      ULONG result = amisf->asf_PlaybackRateOffset[ i ];
      LOG_V(( "V: Resolved %ldHz to index %ld\n", sampleRate, result ));
      return result;
    }
  }
  LOG_E(( "E: No playback rate offset for sample rate &ldHz, crashing now.\n",
          sampleRate ));
  return 0xFF000000;
}

static BOOL FillPresetNotes(
  struct SF2 * sf2,
  struct ConversionInfo * info,
  struct AmiSF * amisf,
  struct ProgressDialog * dialog,
  ULONG * currentProgress,
  ULONG * maxProgress
) {

  UBYTE lastBank = 255;
  UBYTE lastPreset = 255;
  ULONG nextNoteIndex = 0;

  UWORD i;
  UWORD j;

  struct SF2_Preset * sf2Preset;

  // Mark all presets as invalid!
  for ( i = 0; i < 129; ++i ) {
    for ( j = 0; j < 129; ++j ) {

      struct AmiSF_Preset * asf_Preset = &( amisf->asf_Preset[ i ][ j ]);
      asf_Preset->asfp_Bank = 255;
      asf_Preset->asfp_Preset = 255;
    }
  }

  /* Begin iteration over all presets - instruments - samples */
  FOR_LIST( &( sf2->sf2_Presets ),
            sf2Preset,
            struct SF2_Preset * ) {

    // ADSR combination logic is in 
    // "SoundFont Technical Specification Version 2.01 July 23, 1998"
    // aka SFSpec21.pdf, p.57
    LONG effectiveAttack  = INSTRUMENT_DEFAULT_ATTACK_VALUE;
    LONG effectiveDecay   = INSTRUMENT_DEFAULT_DECAY_VALUE;
    LONG effectiveSustain = INSTRUMENT_DEFAULT_SUSTAIN_VALUE;
    LONG effectiveRelease = INSTRUMENT_DEFAULT_RELEASE_VALUE;

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
        struct AmiSF_Preset * asf_Preset =
          &( amisf->asf_Preset[ bank ][ preset ]);
        struct AmiSF_Note * asf_Note;
        BOOL abort;

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
        // Fill the new preset with information as soon as we reach a new one
        if (( lastBank != bank ) || ( lastPreset != preset )) {

          asf_Preset->asfp_Bank = bank;
          asf_Preset->asfp_Preset = preset;
          asf_Preset->asfp_NoteStart = nextNoteIndex; // Absolute start index
          // asf_Preset->asfp_NoteCount = 0; done by AllocMem( ... MEMF_CLEAR );
          lastBank = bank;
          lastPreset = preset;
        }

        // Remember: all the notes are "pooled" in a single array addressed by
        // only indices, hence here it is enough to just get the current note...
        asf_Note = &( amisf->asf_Note[ nextNoteIndex ]);
        // and advance the note index for getting the next the note next time.
        ++nextNoteIndex;              // Counts the absolute note index.
        ++asf_Preset->asfp_NoteCount; // Counts the preset relative note index.

        // Fill the current note
        asf_Note->asfn_Volume = 0x4001;
        asf_Note->asfn_MaxNote = MIN( instrumentMax, sampleMax );
        asf_Note->asfn_BaseNote = tempSample->sf2s_SampleNote;
        asf_Note->asfn_BasePlaybackIndex = GetPlaybackRateOffset(
          amisf,
          tempSample->sf2s_SampleRate );
        asf_Note->asfn_Attack = GetTargetADR( effectiveAttack );
        asf_Note->asfn_Decay = GetTargetADR( effectiveDecay );
        asf_Note->asfn_Sustain = GetTargetS( effectiveSustain );
        asf_Note->asfn_Release = GetTargetADR( effectiveRelease );
        asf_Note->asfn_SampleIndex = info->ci_SampleMapping[ sampleIndex ];

/* TODO: Wohin soll das ADSR zeugs?
        effectiveAttack = argsI->sf2a_Values.sf2v_Attack;
        effectiveDecay = argsI->sf2a_Values.sf2v_Decay;
        effectiveSustain = argsI->sf2a_Values.sf2v_Sustain;
        effectiveRelease = argsI->sf2a_Values.sf2v_Release;

        
        if ( PRESET_DEFAULT_ATTACK_VALUE != argsP->sf2a_Values.sf2v_Attack ) {

          effectiveAttack += argsP->sf2a_Values.sf2v_Attack;
        }
        if ( PRESET_DEFAULT_DECAY_VALUE != argsP->sf2a_Values.sf2v_Decay ) {

          effectiveDecay += argsP->sf2a_Values.sf2v_Decay;
        }
        if ( PRESET_DEFAULT_SUSTAIN_VALUE != argsP->sf2a_Values.sf2v_Sustain ) {

          effectiveSustain += argsP->sf2a_Values.sf2v_Sustain;
        }
        if ( PRESET_DEFAULT_RELEASE_VALUE != argsP->sf2a_Values.sf2v_Release ) {

          effectiveRelease += argsP->sf2a_Values.sf2v_Release;
        }
        LOG_D(( "V: bank %ld preset %ld note %ld-%ld -> "
                "%ld A in Inst., %ld A in Pres., %ld A eff.\n",
                bank,
                preset,
                MAX( instrumentMin, sampleMin ), 
                asf_Note->asfn_MaxNote,
                argsI->sf2a_Values.sf2v_Attack,
                argsP->sf2a_Values.sf2v_Attack,
                effectiveAttack ));
*/

        /* Complete iteration over all presets - instruments - samples */
        LOG_V(( "V: bank %ld, preset %ld now has "
                "%ld notes "
                "starting at %ld.\n",
                bank, preset,
                asf_Preset->asfp_NoteCount,
                asf_Preset->asfp_NoteStart ));

        *currentProgress += 1;
        abort = HandleProgressDialogTick( dialog,
                                          *currentProgress,
                                          *maxProgress );
        if ( abort ) {

          return TRUE;
        }
      }
    }
  }

  LOG_I(( "I: Preset and note conversion %s.\n",
          ( nextNoteIndex == amisf->asf_NoteCount )
            ? "successful" : "failed" ));
  return FALSE;
}

static BOOL FillRateCounts(
  struct ConversionInfo * info,
  struct AmiSF * amisf,
  struct ProgressDialog * dialog,
  ULONG * currentProgress,
  ULONG * maxProgress
) {

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
    BOOL abort;

    while ( sampleRate == end->cpr_SampleRate ) {

      LOG_V(( "V: Checking for sample rate %ldHz, max note is %ld, current note is %ld\n",
              sampleRate, maxSampleNote, end->cpr_SampleNote ));

      minSampleNote = end->cpr_SampleNote;
      end = ( struct ConversionRates * ) end->cpr_Node.ln_Succ;
    }
    amisf->asf_PlaybackRateCount += maxSampleNote + 128 - minSampleNote;
    LOG_V(( "V: For sample rate %ldHz, "
            "min note is %ld, max note is %ld, "
            "so far needs %ld\n",
            sampleRate,
            minSampleNote, maxSampleNote,
            amisf->asf_PlaybackRateCount ));
    
    ++amisf->asf_SampleRateCount;
 
    // Skipping over the same rate - different notes we have handled already now.
    rate = ( struct ConversionRates * ) end->cpr_Node.ln_Pred;

    *currentProgress += 1;
    abort = HandleProgressDialogTick( dialog, *currentProgress, *maxProgress );
    if ( abort ) {

      return TRUE;
    }
  }
  *maxProgress += 
    ( amisf->asf_SampleRateCount << 1 ) - ( info->ci_PlaybackRateCount  << 1 );
  HandleProgressDialogTick( dialog, *currentProgress, *maxProgress );
  LOG_D(( "D: Found %ld distinct sample rates and %ld playback rates\n",
          amisf->asf_SampleRateCount,
          amisf->asf_PlaybackRateCount ));
  return FALSE;
}

static BOOL FillRates(
  struct ConversionInfo * info,
  struct AmiSF * amisf,
  struct ProgressDialog * dialog,
  ULONG * currentProgress,
  ULONG * maxProgress
) {

  LONG sampleRateCount = 0;
  LONG playbackRateCount = 0;

  struct ConversionRates * rate;

  // Walk all the playback rates collected again,
  // calculate the first batch of playback rates,
  // set the sample rate information,
  // calculate the second batch of playback.
  FOR_LIST( &( info->ci_PlaybackRatesNeeded ),
            rate,
            struct ConversionRates * ) {

    struct ConversionRates * end = rate;

    const ULONG sampleRate = rate->cpr_SampleRate;
    ULONG minSampleNote = rate->cpr_SampleNote;
    const ULONG maxSampleNote = rate->cpr_SampleNote;
    BOOL abort;
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

    *currentProgress += 1;
    abort = HandleProgressDialogTick( dialog, *currentProgress, *maxProgress );
    if ( abort ) {

      return TRUE;
    }
  }

  LOG_D(( "D: Calc'd %ld distinct sample rates and %ld playback rates\n",
          sampleRateCount,
          playbackRateCount ));
  LOG_I(( "I: Sample rate conversion %s.\n",
          (( sampleRateCount == amisf->asf_SampleRateCount )
            && ( playbackRateCount == amisf->asf_PlaybackRateCount ))
            ? "successful" : "failed" ));
  return FALSE;
}

static BOOL FillSampleData(
  struct SF2 * sf2,
  struct ConversionInfo * info,
  struct AmiSF * amisf,
  struct ProgressDialog * dialog,
  ULONG * currentProgress,
  ULONG * maxProgress
) {

  ULONG targetCount = 0;
  ULONG sourceIndex;
  ULONG targetOffset = 0;

  for ( sourceIndex = 0; sourceIndex < 65536; ++sourceIndex ) {

    UWORD targetIndex = info->ci_SampleMapping[ sourceIndex ];

    // Value 0 means not used!
    if ( targetIndex ) {

      struct SF2_Sample * sourceSample;
      struct AmiSF_Sample * targetSample;
      BOOL abort;

      // Value 1 means using sample index 0, correcting here.
      --targetIndex;

      sourceSample = sf2->sf2_SampleArray[ sourceIndex ];
      targetSample = &( amisf->asf_SampleMetadata[ targetIndex ]);

      LOG_V(( "V: Found mapping %s = %ld = %ld -> %ld\n",
              sourceSample->sf2s_Name,
              sourceSample->sf2s_Number,
              sourceIndex,
              targetIndex ));
      targetSample->asfs_PlaybackFlags = AMISF_PLAY_RESOLUTION_16BIT
                                       | AMISF_PLAY_LOOPED
                                       | AMISF_PLAY_INTERPOLATION
                                       | AMISF_PLAY_ENVELOPE_MODULATION
                                       | AMISF_PLAY_ENVELOPE_KEY_ON
                                       | AMISF_PLAY_SAMPLE_PLAYBACK_ON;
      targetSample->asfs_StatusFlags = AMISF_STATUS_LOCATION_OTHER_FILE;
      targetSample->asfs_StartOffset = targetOffset;
      targetSample->asfs_LoopOffset = targetOffset
        + (( sourceSample->sf2s_LoopStartOffset
          - sourceSample->sf2s_SampleStartOffset ) << 1 );
      targetOffset +=
        (( sourceSample->sf2s_LoopEndOffset 
          - sourceSample->sf2s_SampleStartOffset ) << 1 ) - 1;
      targetSample->asfs_EndOffset = targetOffset;
      ++targetOffset;

      LOG_V(( "V: Sample mapped from s: 0x%08lx l: 0x%08lx e: 0x%08lx "
              "to s: 0x%08lx l: 0x%08lx e: 0x%08lx\n",
              sourceSample->sf2s_SampleStartOffset,
              sourceSample->sf2s_LoopStartOffset,
              sourceSample->sf2s_LoopEndOffset,
              targetSample->asfs_StartOffset,
              targetSample->asfs_LoopOffset,
              targetSample->asfs_EndOffset ));
      ++targetCount;

      *currentProgress += 1;
      abort = HandleProgressDialogTick( dialog,
                                         *currentProgress,
                                         *maxProgress );
      if ( abort ) {

        return TRUE;
      }
    } else {
      
      LOG_V(( "V: Found mapping %ld -> %ld (empty)\n",
              sourceIndex, targetIndex ));
    }
  }
  LOG_I(( "I: Samples expected %ld, actual %ld, %s\n", 
    amisf->asf_SampleCount,
    targetCount,
    ( amisf->asf_SampleCount == targetCount ) ? "OK." : "failed!" ));
  return FALSE;
}

struct AmiSF * AllocAmiSFfromSF2(
  struct SF2 * sf2,
  struct ConversionInfo * info,
  struct ProgressDialog * dialog
) {

  ULONG allocatedSize = 0;
  ULONG currentProgress = 100;
  ULONG maxProgress = 100
                      + info->ci_SampleCount
                      + info->ci_NoteCount
                      + ( info->ci_PlaybackRateCount << 1 );

  BOOL abort = HandleProgressDialogTick( dialog, currentProgress, maxProgress );
  struct AmiSF * amisf = AllocMem( sizeof( struct AmiSF ),
                                   MEMF_ANY | MEMF_CLEAR );
  if ( !amisf ) {

    abort = TRUE;
  }
  if ( !abort ) {

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

    abort = FillRateCounts( info,
                            amisf,
                            dialog,
                            &currentProgress,
                            &maxProgress );
  }
  if ( !abort ) {

    amisf->asf_SampleRate = AllocMem(
      sizeof( ULONG ) * amisf->asf_SampleRateCount,
      MEMF_ANY | MEMF_CLEAR );
    amisf->asf_PlaybackRateOffset = AllocMem(
      sizeof( ULONG ) * amisf->asf_SampleRateCount,
      MEMF_ANY | MEMF_CLEAR );
    allocatedSize += sizeof( ULONG ) * amisf->asf_SampleRateCount * 2;

    amisf->asf_PlaybackRate = AllocMem(
      sizeof( ULONG ) * amisf->asf_PlaybackRateCount,
      MEMF_ANY | MEMF_CLEAR );
    allocatedSize += sizeof( ULONG ) * amisf->asf_PlaybackRateCount;

    abort = FillRates( info,
                       amisf,
                       dialog,
                       &currentProgress,
                       &maxProgress );
  }

  if ( !abort ) {

    abort = FillPresetNotes( sf2,
                             info,
                             amisf,
                             dialog,
                             &currentProgress,
                             &maxProgress );
  }
  if ( !abort ) {

    abort = FillSampleData( sf2,
                            info,
                            amisf,
                            dialog,
                            &currentProgress,
                            &maxProgress );
  }

  if ( abort ) {

    FreeAmiSF( amisf );
    amisf = NULL;

  } else {

    LOG_I(( "I: Conversion successful, allocated size for AmiSF is %ld\n",
            allocatedSize ));
  }
  return amisf;
}

struct AmiSF * AllocAmiSFfromFile(
  STRPTR filePath,
  struct ProgressDialog * dialog
) {

  // TODO!!!
  return NULL;
}

LONG WriteAmiSFtoFile(
  struct AmiSF * amisf,
  STRPTR filePath,
  struct SF2 * sf2,
  struct ConversionInfo * info,
  struct ProgressDialog * dialog
) {

  LONG written;
  ULONG i;
  ULONG j;
  ULONG currentProgress = 100;
  ULONG maxProgress = 100
                      + 128                      /* Banks */
                      + amisf->asf_NoteCount
                      + amisf->asf_SampleCount
                      + amisf->asf_SampleRateCount
                      + amisf->asf_PlaybackRateCount
                      + (( amisf->asf_SampleCount ) << 4 );

  BOOL abort = HandleProgressDialogTick( dialog, currentProgress, maxProgress );
  BPTR fileHandle = Open( filePath, MODE_NEWFILE );

  if ( !( fileHandle )) {

    return EOpenAmiSFwriteFailed;
  }
  if ( abort ) {

    Close( fileHandle );
    return EOpenAmiSFwriteAborted;
  }

  written = Write( fileHandle,
                   "AmiSF\0\0\1",
                   8 );
  if ( -1 == written ) {

    Close( fileHandle );
    return EOpenAmiSFwriteError;
  }
  written = Write( fileHandle,
                   &( amisf->asf_Preset ),
                   128 * 128 * sizeof( struct AmiSF_Preset ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EOpenAmiSFwriteError;
  }

  currentProgress += 128;
  abort = HandleProgressDialogTick( dialog, currentProgress, maxProgress );
  if ( abort ) {

    Close( fileHandle );
    return EOpenAmiSFwriteAborted;
  }

  written = Write( fileHandle,
                   &( amisf->asf_NoteCount ),
                   sizeof( ULONG ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EOpenAmiSFwriteError;
  }
  written = Write( fileHandle,
                    amisf->asf_Note,
                    amisf->asf_NoteCount * sizeof( struct AmiSF_Note ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EOpenAmiSFwriteError;
  }

  currentProgress += amisf->asf_NoteCount;
  abort = HandleProgressDialogTick( dialog, currentProgress, maxProgress );
  if ( abort ) {

    Close( fileHandle );
    return EOpenAmiSFwriteAborted;
  }

  written = Write( fileHandle,
                   &( amisf->asf_SampleCount ),
                   sizeof( ULONG ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EOpenAmiSFwriteError;
  }
  written = Write( fileHandle,
                   amisf->asf_SampleMetadata,
                   amisf->asf_SampleCount * sizeof( struct AmiSF_Sample ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EOpenAmiSFwriteError;
  }

  currentProgress += amisf->asf_SampleCount;
  abort = HandleProgressDialogTick( dialog, currentProgress, maxProgress );
  if ( abort ) {

    Close( fileHandle );
    return EOpenAmiSFwriteAborted;
  }

  written = Write( fileHandle,
                   &( amisf->asf_SampleRateCount ),
                   sizeof( ULONG ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EOpenAmiSFwriteError;
  }
  written = Write( fileHandle,
                   amisf->asf_SampleRate,
                   amisf->asf_SampleRateCount * sizeof( ULONG ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EOpenAmiSFwriteError;
  }
  written = Write( fileHandle,
                   amisf->asf_PlaybackRateOffset,
                   amisf->asf_SampleRateCount * sizeof( ULONG ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EOpenAmiSFwriteError;
  }

  currentProgress += amisf->asf_SampleRateCount;
  abort = HandleProgressDialogTick( dialog, currentProgress, maxProgress );
  if ( abort ) {

    Close( fileHandle );
    return EOpenAmiSFwriteAborted;
  }

  written = Write( fileHandle,
                   &( amisf->asf_PlaybackRateCount ),
                   sizeof( ULONG ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EOpenAmiSFwriteError;
  }
  written = Write( fileHandle,
                   amisf->asf_PlaybackRate,
                   amisf->asf_PlaybackRateCount * sizeof( ULONG ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EOpenAmiSFwriteError;
  }

  currentProgress += amisf->asf_PlaybackRateCount;
  abort = HandleProgressDialogTick( dialog, currentProgress, maxProgress );
  if ( abort ) {

    Close( fileHandle );
    return EOpenAmiSFwriteAborted;
  }

  written = Write( fileHandle,
                   &( amisf->asf_SampleDataSize ),
                   sizeof( ULONG ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EOpenAmiSFwriteError;
  }

  if ( !( amisf->asf_SampleSourceFile )
    && !( amisf->asf_SampleSourceOffset )
    && ( sf2 ) 
    && ( info )) {

    // Obviously, we came from a converted SF2 file.
    amisf->asf_SampleSourceOffset = Seek( fileHandle, OFFSET_CURRENT, 0 ) + 4;
    written = Write( fileHandle,
                     &( amisf->asf_SampleSourceOffset ),
                     sizeof( ULONG ));
    if ( -1 == written ) {

      Close( fileHandle );
      return EOpenAmiSFwriteError;
    }

    for ( i = 0; i < amisf->asf_SampleCount; ++i ) {

      for (j = 0; j < 65535; ++j ) {

        if (( info->ci_SampleMapping[ j ] )
          && (( info->ci_SampleMapping[ j ] - 1 ) == i )) {

          struct SF2_Sample * sf2Sample = sf2->sf2_SampleArray[ j ];
          LONG sf2SampleSize = GetSF2SampleSize( sf2Sample );
          APTR sf2SampleData = GetSF2SampleData( sf2, sf2Sample );
        
          LOG_D(( "D: AmiSF sample %ld is mapped to SF2 sample %ld, "
                  "size %ld @ 0x%08lx.\n",
                  i, j, sf2SampleSize, sf2SampleData ));

          written = Write( fileHandle, sf2SampleData, sf2SampleSize );
          if ( -1 == written ) {

            Close( fileHandle );
            return EOpenAmiSFwriteError;
          }

          FreeMem( sf2SampleData, sf2SampleSize );

          currentProgress += ( 1 << 4 );
          abort = HandleProgressDialogTick( dialog,
                                            currentProgress,
                                            maxProgress );
          if ( abort ) {

            Close( fileHandle );
            return EOpenAmiSFwriteAborted;
          }

          break;
        }
      }
    }
  } else {

    // TODO: How would we write back a loaded AmiSF?
  }

  LOG_D(( "D: Written, progress %ld of %ld.\n", currentProgress, maxProgress ));
  Close( fileHandle );

  return ENoError;
}

VOID FreeAmiSF( struct AmiSF * amisf ) {

  if ( !amisf ) {

    return;
  }
  if ( amisf->asf_SampleRate ) {

    FreeMem( amisf->asf_SampleRate,
             sizeof( ULONG ) * amisf->asf_SampleRateCount );
    amisf->asf_SampleRate = NULL;
  }
  if ( amisf->asf_PlaybackRateOffset ) {

    FreeMem( amisf->asf_PlaybackRateOffset,
             sizeof( ULONG ) * amisf->asf_SampleRateCount );
    amisf->asf_PlaybackRateOffset = NULL;
  }
  if ( amisf->asf_PlaybackRate ) {

    FreeMem( amisf->asf_PlaybackRate,
             sizeof( ULONG ) * amisf->asf_PlaybackRateCount );
    amisf->asf_PlaybackRate = NULL;
  }
  if ( amisf->asf_SampleMetadata ) {

    FreeMem( amisf->asf_SampleMetadata,
             sizeof( struct AmiSF_Sample ) * amisf->asf_SampleCount );
    amisf->asf_SampleMetadata = NULL;
  }
  if ( amisf->asf_Note ) {

    FreeMem( amisf->asf_Note,
             sizeof( struct AmiSF_Note ) * amisf->asf_NoteCount );
    amisf->asf_Note = NULL;
  }

  FreeMem( amisf, sizeof( struct AmiSF ));
  LOG_D(( "D: Free'd AmiSF with %ld notes, %ld samples, %ld bytes sample data, "
          "%ld sample rates, and %ld playback rates.\n",
          amisf->asf_NoteCount,
          amisf->asf_SampleCount,
          -1,
          amisf->asf_SampleRateCount,
          amisf->asf_PlaybackRateCount ));
}
