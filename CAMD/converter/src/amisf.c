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
#include <proto/mathieeedoubbas.h>
#include <proto/mathieeedoubtrans.h>

#include "amisf_conversion.h"
#include "converter.h"
#include "debug.h"
#include "errors.h"
#include "progress_dialog.h"
#include "sf2.h"
#include "sf2_conversion.h"
#include "sf2_tools.h"
#include "support.h"

#define CHAR_TO_ULONG( a, b, c, d ) ((( a ) << 24 ) | (( b ) << 16 ) | (( c ) << 8 ) | ( d ))

STRPTR AmiSF_Suffix = ".AmiSF";

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
        WORD attack;
        WORD decay;
        WORD sustain;
        WORD release;

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
        GetSf2ADSRs( argsP, argsI, &attack, &decay, &sustain, &release );

        asf_Note->asfn_Volume = 0x4001;
        asf_Note->asfn_MaxNote = MIN( instrumentMax, sampleMax );
        asf_Note->asfn_BaseNote = tempSample->sf2s_SampleNote;
        asf_Note->asfn_BasePlaybackIndex = GetPlaybackRateOffset(
          amisf,
          tempSample->sf2s_SampleRate );
        asf_Note->asfn_Attack = GetTargetADR( attack );
        asf_Note->asfn_Decay = GetTargetADR( decay );
        asf_Note->asfn_Sustain = GetTargetS( sustain );
        asf_Note->asfn_Release = GetTargetADR( release );
        asf_Note->asfn_SampleIndex = info->ci_Sf2ToAmiSfSample[ sampleIndex ];

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

  struct SF2_Sample * sourceSample;
  struct AmiSF_Sample * targetSample;
  UWORD sourceIndex;
  UWORD targetIndex;
  ULONG targetCount = 0;
  ULONG targetOffset = 0;

  for ( targetIndex = 0; 
        targetIndex < amisf->asf_SampleCount;
        ++targetIndex ) {

    sourceIndex = info->ci_AmiSfToSf2Sample[ targetIndex ];

    if ( sourceIndex ) {

      BOOL abort;
      ULONG preLoopLength;
      ULONG sampleLength;
      
      --sourceIndex; // As 1 from array is 0 in as index as 0 is none

      sourceSample = sf2->sf2_SampleArray[ sourceIndex ];
      targetSample = &( amisf->asf_SampleMetadata[ targetIndex ]);

      preLoopLength = ( sourceSample->sf2s_LoopStartOffset
        - sourceSample->sf2s_SampleStartOffset ) << 1;
      sampleLength = ( sourceSample->sf2s_LoopEndOffset
        - sourceSample->sf2s_SampleStartOffset ) << 1;
      
      // sampleLength needs to be multiple of 4
      sampleLength += 3;
      sampleLength &= 0xFFFFFFFC;

      LOG_V(( "V: Found mapping %s = %ld = %ld -> %ld\n",
              sourceSample->sf2s_Name,
              sourceSample->sf2s_Number,
              j,
              i ));
      targetSample->asfs_PlaybackFlags = AMISF_PLAY_RESOLUTION_16BIT
                                       | AMISF_PLAY_LOOPED
                                       | AMISF_PLAY_INTERPOLATION
                                       | AMISF_PLAY_ENVELOPE_MODULATION
                                       | AMISF_PLAY_ENVELOPE_KEY_ON
                                       | AMISF_PLAY_SAMPLE_PLAYBACK_ON;
      targetSample->asfs_StatusFlags = AMISF_STATUS_LOCATION_OTHER_FILE;

      targetSample->asfs_StartOffset = targetOffset;
      targetSample->asfs_LoopOffset = targetOffset + preLoopLength;
      targetOffset += sampleLength;
      targetSample->asfs_EndOffset = targetOffset;

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

  LONG expected;
  LONG actual;
  UBYTE header[ 8 ];
  struct AmiSF * amisf = AllocMem( sizeof( struct AmiSF ),
                                   MEMF_ANY | MEMF_CLEAR );

  amisf->asf_SampleSourceFile = Open( filePath, MODE_OLDFILE );
  if ( !( amisf->asf_SampleSourceFile )) {

    FreeAmiSF( amisf );
    DisplayError( EOpenAmiSFreadFailed );
    return NULL;
  }


  expected = 8 * sizeof( UBYTE );
  actual = Read( amisf->asf_SampleSourceFile,
                 header,
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  if (( CHAR_TO_ULONG( 'A', 'm', 'i', 'S' ) != ( *(( LONG * ) &( header[ 0 ]))))
    || ( CHAR_TO_ULONG( 'F', 0, 0, 1 ) != ( *(( LONG * ) &( header[ 4 ]))))) {

    //TODO: handle error
    LOG_E(( "E: File handle shall now be at sample position %ld but is %ld!\n",
            expected, amisf->asf_SampleSourceOffset ));
    return NULL;
  }

  ///////////////////////////////////////////////////////////////////

  expected = 128 * 128 * sizeof( struct AmiSF_Preset );
  actual = Read( amisf->asf_SampleSourceFile,
                 &( amisf->asf_Preset ),
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  ///////////////////////////////////////////////////////////////////

  expected = 1 * sizeof( ULONG );
  actual = Read( amisf->asf_SampleSourceFile,
                 &( amisf->asf_NoteCount ),
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  expected = amisf->asf_NoteCount * sizeof( struct AmiSF_Note );
  amisf->asf_Note = AllocMem( expected,
                              MEMF_ANY | MEMF_CLEAR );
  if ( !( amisf->asf_Note )) {
    // TODO: handle error
  }
  actual = Read( amisf->asf_SampleSourceFile,
                 amisf->asf_Note,
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  ///////////////////////////////////////////////////////////////////

  expected = 1 * sizeof( ULONG );
  actual = Read( amisf->asf_SampleSourceFile,
                 &( amisf->asf_SampleCount ),
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  expected = amisf->asf_SampleCount * sizeof( struct AmiSF_Sample );
  amisf->asf_SampleMetadata = AllocMem( expected,
                                        MEMF_ANY | MEMF_CLEAR );
  if ( !( amisf->asf_SampleMetadata )) {
    // TODO: handle error
  }
  actual = Read( amisf->asf_SampleSourceFile,
                 amisf->asf_SampleMetadata,
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  ///////////////////////////////////////////////////////////////////

  expected = 1 * sizeof( ULONG );
  actual = Read( amisf->asf_SampleSourceFile,
                 &( amisf->asf_SampleRateCount ),
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  expected = amisf->asf_SampleRateCount * sizeof( ULONG );
  amisf->asf_SampleRate = AllocMem( expected,
                                    MEMF_ANY | MEMF_CLEAR );
  if ( !( amisf->asf_SampleRate )) {
    // TODO: handle error
  }
  actual = Read( amisf->asf_SampleSourceFile,
                 amisf->asf_SampleRate,
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  amisf->asf_PlaybackRateOffset = AllocMem( expected,
                                            MEMF_ANY | MEMF_CLEAR );
  if ( !( amisf->asf_PlaybackRateOffset )) {
    // TODO: handle error
  }
  actual = Read( amisf->asf_SampleSourceFile,
                 amisf->asf_PlaybackRateOffset,
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  ///////////////////////////////////////////////////////////////////

  expected = 1 * sizeof( ULONG );
  actual = Read( amisf->asf_SampleSourceFile,
                 &( amisf->asf_PlaybackRateCount ),
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  expected = amisf->asf_PlaybackRateCount * sizeof( ULONG );
  amisf->asf_PlaybackRate = AllocMem( expected,
                                    MEMF_ANY | MEMF_CLEAR );
  if ( !( amisf->asf_PlaybackRate )) {
    // TODO: handle error
  }
  actual = Read( amisf->asf_SampleSourceFile,
                 amisf->asf_PlaybackRate,
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  ///////////////////////////////////////////////////////////////////

  expected = 1 * sizeof( ULONG );

  actual = Read( amisf->asf_SampleSourceFile,
                 &( amisf->asf_SampleDataSize ),
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  actual = Read( amisf->asf_SampleSourceFile,
                 &( amisf->asf_SampleSourceOffset ),
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  expected = Seek( amisf->asf_SampleSourceFile, OFFSET_CURRENT, 0 );
  if ( amisf->asf_SampleSourceOffset != expected ) {
    // TODO: handle error
    LOG_E(( "E: File handle shall now be at sample position %ld but is %ld!\n",
            expected, amisf->asf_SampleSourceOffset ));
    return NULL;
  }

  LOG_I(( "I: Read AmiSF with %ld notes, %ld samples, "
          "%ld sample and %ld playback rates, "
          "and %lu bytes of sample data starting at 0x%08lx.\n",
          amisf->asf_NoteCount,
          amisf->asf_SampleCount,
          amisf->asf_SampleRateCount,
          amisf->asf_PlaybackRateCount,
          amisf->asf_SampleDataSize,
          amisf->asf_SampleSourceOffset ));
  return amisf;
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

    return EAmiSFwriteOpenFailed;
  }
  if ( abort ) {

    Close( fileHandle );
    return EAmiSFwriteAborted;
  }

  written = Write( fileHandle,
                   "AmiSF\0\0\1",
                   8 );
  if ( -1 == written ) {

    Close( fileHandle );
    return EAmiSFwriteError;
  }
  written = Write( fileHandle,
                   &( amisf->asf_Preset ),
                   128 * 128 * sizeof( struct AmiSF_Preset ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EAmiSFwriteError;
  }

  currentProgress += 128;
  abort = HandleProgressDialogTick( dialog, currentProgress, maxProgress );
  if ( abort ) {

    Close( fileHandle );
    return EAmiSFwriteAborted;
  }

  written = Write( fileHandle,
                   &( amisf->asf_NoteCount ),
                   sizeof( ULONG ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EAmiSFwriteError;
  }
  written = Write( fileHandle,
                    amisf->asf_Note,
                    amisf->asf_NoteCount * sizeof( struct AmiSF_Note ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EAmiSFwriteError;
  }

  currentProgress += amisf->asf_NoteCount;
  abort = HandleProgressDialogTick( dialog, currentProgress, maxProgress );
  if ( abort ) {

    Close( fileHandle );
    return EAmiSFwriteAborted;
  }

  written = Write( fileHandle,
                   &( amisf->asf_SampleCount ),
                   sizeof( ULONG ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EAmiSFwriteError;
  }
  written = Write( fileHandle,
                   amisf->asf_SampleMetadata,
                   amisf->asf_SampleCount * sizeof( struct AmiSF_Sample ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EAmiSFwriteError;
  }

  currentProgress += amisf->asf_SampleCount;
  abort = HandleProgressDialogTick( dialog, currentProgress, maxProgress );
  if ( abort ) {

    Close( fileHandle );
    return EAmiSFwriteAborted;
  }

  written = Write( fileHandle,
                   &( amisf->asf_SampleRateCount ),
                   sizeof( ULONG ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EAmiSFwriteError;
  }
  written = Write( fileHandle,
                   amisf->asf_SampleRate,
                   amisf->asf_SampleRateCount * sizeof( ULONG ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EAmiSFwriteError;
  }
  written = Write( fileHandle,
                   amisf->asf_PlaybackRateOffset,
                   amisf->asf_SampleRateCount * sizeof( ULONG ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EAmiSFwriteError;
  }

  currentProgress += amisf->asf_SampleRateCount;
  abort = HandleProgressDialogTick( dialog, currentProgress, maxProgress );
  if ( abort ) {

    Close( fileHandle );
    return EAmiSFwriteAborted;
  }

  written = Write( fileHandle,
                   &( amisf->asf_PlaybackRateCount ),
                   sizeof( ULONG ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EAmiSFwriteError;
  }
  written = Write( fileHandle,
                   amisf->asf_PlaybackRate,
                   amisf->asf_PlaybackRateCount * sizeof( ULONG ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EAmiSFwriteError;
  }

  currentProgress += amisf->asf_PlaybackRateCount;
  abort = HandleProgressDialogTick( dialog, currentProgress, maxProgress );
  if ( abort ) {

    Close( fileHandle );
    return EAmiSFwriteAborted;
  }

  written = Write( fileHandle,
                   &( amisf->asf_SampleDataSize ),
                   sizeof( ULONG ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EAmiSFwriteError;
  }

  if ( 0 == amisf->asf_SampleSourceOffset ) {

    amisf->asf_SampleSourceOffset = Seek( fileHandle, 0, OFFSET_CURRENT ) + 4;
  }

  written = Write( fileHandle,
                    &( amisf->asf_SampleSourceOffset ),
                    sizeof( ULONG ));
  if ( -1 == written ) {

    Close( fileHandle );
    return EAmiSFwriteError;
  }
  LOG_D(( "D: AmiSF sample offset is %08lx\n", amisf->asf_SampleSourceOffset ));

  if ( !( amisf->asf_SampleSourceFile )
    && ( sf2 ) 
    && ( info )) {

    UWORD targetIndex;
    UWORD sourceIndex;

    // Obviously, we came from a converted SF2 file.
    amisf->asf_SampleDataSize = 0;

    for ( targetIndex = 0; 
          targetIndex < amisf->asf_SampleCount;
          ++targetIndex ) {

      sourceIndex = info->ci_AmiSfToSf2Sample[ targetIndex ];

      if ( sourceIndex ) {

        struct SF2_Sample * sf2Sample;
        LONG sf2SampleSize;
        APTR sf2SampleData;

        --sourceIndex; // As 1 from array is 0 in as index as 0 is none

        sf2Sample = sf2->sf2_SampleArray[ sourceIndex ];
        sf2SampleSize = GetSf2SampleSize( sf2Sample );
        sf2SampleData = GetSf2SampleData( sf2, sf2Sample, sf2SampleSize );
        
        LOG_D(( "D: AmiSF sample %ld is mapped to SF2 sample %ld, "
                "size %ld @ 0x%08lx.\n",
                targetIndex, sourceIndex, sf2SampleSize, sf2SampleData ));

        written = Write( fileHandle, sf2SampleData, sf2SampleSize );
        if ( -1 == written ) {

          Close( fileHandle );
          return EAmiSFwriteError;
        }

        FreeMem( sf2SampleData, sf2SampleSize );
        amisf->asf_SampleDataSize += sf2SampleSize;
        currentProgress += ( 1 << 4 );
        abort = HandleProgressDialogTick( dialog,
                                          currentProgress,
                                          maxProgress );
        if ( abort ) {

          Close( fileHandle );
          return EAmiSFwriteAborted;
        }
      }
    }

    Seek( fileHandle, amisf->asf_SampleSourceOffset - 8, OFFSET_BEGINNING );
    written = Write( fileHandle,
                     &( amisf->asf_SampleDataSize ),
                     sizeof( ULONG ));
    if ( -1 == written ) {

      Close( fileHandle );
      return EAmiSFwriteError;
    }

  } else {

    APTR buffer = AllocMem( 4096, MEMF_ANY );
    ULONG copiedSize = 0;

    Seek( amisf->asf_SampleSourceFile,
          amisf->asf_SampleSourceOffset,
          OFFSET_BEGINNING );
    maxProgress = currentProgress + ( amisf->asf_SampleDataSize >> 12 );
    while ( copiedSize < amisf->asf_SampleDataSize ) {

      ULONG readSize = Read( amisf->asf_SampleSourceFile, buffer, 4096 );
      ULONG writeSize = Write( fileHandle, buffer, readSize );
      if (( 0 >= readSize )
        || ( readSize != writeSize )) {

        Close( fileHandle );
        return EAmiSFsampleWriteError;
      }
      copiedSize += writeSize;

      ++currentProgress;
      abort = HandleProgressDialogTick( dialog,
                                            currentProgress,
                                            maxProgress );
      if ( abort ) {

        Close( fileHandle );
        FreeMem( buffer, 4096 );
        return EAmiSFwriteAborted;
      }
    }
    FreeMem( buffer, 4096 );
  }

  LOG_I(( "I: Wrote AmiSF with %ld notes, %ld samples, "
          "%ld sample and %ld playback rates, "
          "and %lu bytes of sample data starting at 0x%08lx.\n",
          amisf->asf_NoteCount,
          amisf->asf_SampleCount,
          amisf->asf_SampleRateCount,
          amisf->asf_PlaybackRateCount,
          amisf->asf_SampleDataSize,
          amisf->asf_SampleSourceOffset ));
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
  if ( amisf->asf_SampleSourceFile ) {

    Close( amisf->asf_SampleSourceFile );
    amisf->asf_SampleSourceFile = NULL;
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

APTR GetAmiSfSampleData( struct AmiSF * amisf, struct AmiSF_Sample * sample ) {

  ULONG size = sample->asfs_EndOffset - sample->asfs_StartOffset;
  APTR result = AllocMem( size, MEMF_ANY );

  Seek( amisf->asf_SampleSourceFile,
        amisf->asf_SampleSourceOffset + sample->asfs_StartOffset,
        OFFSET_BEGINNING );
  Read( amisf->asf_SampleSourceFile, result, size );

  return result;
}

LONG GetMillisFromPeriods( UWORD periods ) {

  UWORD mantissa = periods & 0x0FFF;
  UWORD exponent = ( periods & 0xF000 ) >> 12;

  // increment = ( mantissa * ( 2 ^ exponent ))
  //             |     |      | |      |     ||
  //             |     |      | a      b     ||
  //             |     d      +----c---------+|
  //             +----------e-----------------+
  double a = IEEEDPFlt( 2 );
  double b = IEEEDPFlt( exponent );
  double c = IEEEDPPow( b, a );
  double d = IEEEDPFlt( mantissa );
  double e = IEEEDPMul( d, c );

  // increment = ( maxULONG / ( seconds  * 192.000 ))
  // seconds * increment = maxULONG / 192.000
  // seconds = ( maxULONG / ( 192.000 * increment ))
  //           |     |      |    |          |  ||
  //           |     |      |    f          e  ||
  //           |     h      +----------g-------+|
  //           +----------i---------------------+
  double f = IEEEDPFlt( 192000 );
  double g = IEEEDPMul( f, e );
  double h = IEEEDPMul( IEEEDPFlt( 0x7fFFffFF ), a );
  double i = IEEEDPDiv( h, g );

  // seconds -> millis
  double j = IEEEDPFlt( 1000 );
  double k = IEEEDPMul( j, i );

  LONG millis = IEEEDPFix( k );

  return millis;
}

LONG GetCentibelsFromFactorShift16( UWORD factorShift16 ) {

  // Usage like:
  // SustainVolume = ( OriginalVolume x FactorShifted ) >> 16
  // FactorShifted = ( Factor << 16 )
  // Factor = ( FactorShifted >> 16 )
  //          |       |       |   | |
  //          |       a       +-b-+ |
  //          +-----------c---------+
  double a = IEEEDPFlt( factorShift16 );
  double b = IEEEDPFlt( 1 << 16 );
  double c = IEEEDPDiv( a, b );

  // Factor = ( 10 ^ (    centibels / -200  ))
  // log10( Factor ) = ( centibels / -200 )
  // log10( Factor ) * ( -200 ) = centibels
  // centibels = ( -200 * log10( Factor ))
  //             |   |    |         |   ||
  //             |   |    |         c   ||
  //             |   e    +----d--------+|
  //             +---------e-------------+
  double d = IEEEDPLog10( c );
  double e = IEEEDPFlt( -200 );
  double f = IEEEDPMul( e, d );

  LONG centibels = IEEEDPFix( f );

  if ( centibels > 1000 ) {

    centibels = 1000;
  }
  return centibels;
}
