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
#define ALIB_STDIO

#include <proto/alib.h>
#include <proto/exec.h>
#include <proto/listbrowser.h>

#include "sf_listnodes.h"

#include "converter.h"
#include "debug.h"
#include "sf_listnode_data.h"
#include "sf2_tools.h"
#include "support.h"

struct SfListNodeData {

  LONG noteIndex;
  LONG attack;
  LONG decay;
  LONG sustain;
  LONG release;
  LONG minSample;
  LONG maxSample;
  LONG sampleIndex;
};

static struct Node * CreateAmiSfListBrowserNode( UWORD bank,
                                                 UWORD preset,
                                                 CONST_STRPTR name,
                                                 UBYTE minNote,
                                                 UBYTE maxNote,
                                                 UWORD noteIndex,
                                                 UWORD attack,
                                                 UWORD decay,
                                                 UWORD sustain,
                                                 UWORD release,
                                                 LONG sampleNumber ) {

  LONG columns = GetAmiSfColumnCount();
  struct SfListNodeData * data = AllocMem( sizeof( struct SfListNodeData ),
                                           MEMF_ANY );

  data->noteIndex = noteIndex;
  data->attack = GetMillisFromPeriods( attack );
  data->decay = GetMillisFromPeriods( decay );
  data->sustain = GetCentibelsFromFactorShift16( sustain );
  data->release = GetMillisFromPeriods( release );
  data->sampleIndex = sampleNumber;

  LOG_V(( "V: Creating label b:%ld p:%ld >n:%ld <=n:%ld n#:%ld "
          "a:%ld->%ld d:%ld->%ld "
          "s:%ld->%ld r:%ld->%ld "
          "s#:%ld\n",
          bank, preset, minNote, maxNote, noteIndex,
          attack, data->attack, decay, data->decay,
          sustain, data->sustain, release, data->release,
          sampleNumber ));

  return AllocListBrowserNode( columns,
                               LBNA_UserData, data,
                               LBNA_Column, 0,
                                 LBNCA_Integer, GetListNumber( bank ),
                                 LBNCA_Justification, LCJ_RIGHT,
                               LBNA_Column, 1,
                                 LBNCA_Integer, GetListNumber( preset ),
                                 LBNCA_Justification, LCJ_RIGHT,
                               LBNA_Column, 2,
                                 LBNCA_CopyText, FALSE,
                                 LBNCA_Text, name,
                               LBNA_Column, 3,
                                 LBNCA_Integer, GetListNumber( minNote ),
                                 LBNCA_Justification, LCJ_RIGHT,
                               LBNA_Column, 4,
                                 LBNCA_Integer, GetListNumber( maxNote ),
                                 LBNCA_Justification, LCJ_RIGHT,
                               LBNA_Column, 5,
                                 LBNCA_Integer, &( data->noteIndex ),
                                 LBNCA_Justification, LCJ_RIGHT,
                               LBNA_Column, 6,
                                 LBNCA_Integer, &( data->attack ),
                                 LBNCA_Justification, LCJ_RIGHT,
                               LBNA_Column, 7,
                                 LBNCA_Integer, &( data->decay ),
                                 LBNCA_Justification, LCJ_RIGHT,
                               LBNA_Column, 8,
                                 LBNCA_Integer, &( data->sustain ),
                                 LBNCA_Justification, LCJ_RIGHT,
                               LBNA_Column, 9,
                                 LBNCA_Integer, &( data->release ),
                                 LBNCA_Justification, LCJ_RIGHT,
                               LBNA_Column, 10,
                                 LBNCA_Integer, &( data->sampleIndex ),
                                 LBNCA_Justification, LCJ_RIGHT,
                               TAG_DONE );
}

static struct Node * CreateSf2ListBrowserNode( UWORD bank,
                                               UWORD preset,
                                               CONST_STRPTR generalMidiName,
                                               CONST_STRPTR presetName,
                                               UBYTE instrumentMinNote,
                                               UBYTE instrumentMaxNote,
                                               UWORD instrumentNumber,
                                               CONST_STRPTR instrumentName,
                                               WORD attack,
                                               WORD decay,
                                               WORD sustain,
                                               WORD release,
                                               UBYTE sampleMinNote,
                                               UBYTE sampleMaxNote,
                                               UWORD sampleNumber,
                                               CONST_STRPTR sampleName ) {

  LONG columns = GetSf2ColumnCount();
  struct SfListNodeData * data = AllocMem( sizeof( struct SfListNodeData ),
                                           MEMF_ANY );

  data->noteIndex = instrumentNumber;
  data->attack = GetMillisFromTimecents( attack );
  data->decay = GetMillisFromTimecents( decay );
  data->sustain = sustain;
  data->release = GetMillisFromTimecents( release );
  data->minSample = sampleMinNote;
  data->maxSample = sampleMaxNote;
  data->sampleIndex = sampleNumber;

  LOG_V(( "V: Creating label b:%ld p:%ld >n:%ld <=n:%ld n#:%ld "
          "a:%ld->%ld d:%ld-%ld "
          "s:%ld r:%ld->%ld "
          ">s:%ld <=s:%ld s#:%ld\n",
          bank, preset, instrumentMinNote, instrumentMaxNote, instrumentNumber,
          attack, data->attack, decay, data->decay,
          sustain, release, data->release,
          sampleMinNote, sampleMaxNote, sampleNumber ));

  return AllocListBrowserNode( columns,
                               LBNA_UserData, data,
                               LBNA_Column, 0,
                                 LBNCA_Integer, GetListNumber( bank ),
                                 LBNCA_Justification, LCJ_RIGHT,
                               LBNA_Column, 1,
                                 LBNCA_Integer, GetListNumber( preset ),
                                 LBNCA_Justification, LCJ_RIGHT,
                               LBNA_Column, 2,
                                 LBNCA_CopyText, FALSE,
                                 LBNCA_Text, generalMidiName,
                               LBNA_Column, 3,
                                 LBNCA_CopyText, FALSE,
                                 LBNCA_Text, presetName,
                               LBNA_Column, 4,
                                 LBNCA_Integer, GetListNumber( instrumentMinNote ),
                                 LBNCA_Justification, LCJ_RIGHT,
                               LBNA_Column, 5,
                                 LBNCA_Integer, GetListNumber( instrumentMaxNote ),
                                 LBNCA_Justification, LCJ_RIGHT,
                               LBNA_Column, 6,
                                 LBNCA_Integer, &( data->noteIndex ),
                                 LBNCA_Justification, LCJ_RIGHT,
                               LBNA_Column, 7,
                                 LBNCA_CopyText, FALSE,
                                 LBNCA_Text, instrumentName,
                               LBNA_Column, 8,
                                 LBNCA_Integer, &( data->attack ),
                                 LBNCA_Justification, LCJ_RIGHT,
                               LBNA_Column, 9,
                                 LBNCA_Integer, &( data->decay ),
                                 LBNCA_Justification, LCJ_RIGHT,
                               LBNA_Column, 10,
                                 LBNCA_Integer, &( data->sustain ),
                                 LBNCA_Justification, LCJ_RIGHT,
                               LBNA_Column, 11,
                                 LBNCA_Integer, &( data->release ),
                                 LBNCA_Justification, LCJ_RIGHT,
                               LBNA_Column, 12,
                                 LBNCA_Integer, &( data->minSample ),
                                 LBNCA_Justification, LCJ_RIGHT,
                               LBNA_Column, 13,
                                 LBNCA_Integer, &( data->maxSample ),
                                 LBNCA_Justification, LCJ_RIGHT,
                               LBNA_Column, 14,
                                 LBNCA_Integer, &( data->sampleIndex ),
                                 LBNCA_Justification, LCJ_RIGHT,
                               LBNA_Column, 15,
                                 LBNCA_CopyText, FALSE,
                                 LBNCA_Text, sampleName,
                               TAG_DONE );
}

VOID CreateEmptyListLabels( struct List * labels ) {

  LONG columns = GetSf2ColumnCount();
  WORD i;

  for ( i = 0; 128 > i; ++i ) {

    CONST_STRPTR instrumentName = GetMidiName( 0, i, 0, 0, 0, 0 );
    const LONG * bankNumber = GetListNumber( 0 );
    const LONG * presetNumber = GetListNumber( i );

    struct Node * label = AllocListBrowserNode( columns,
                               LBNA_Column, 0,
                                 LBNCA_Integer, bankNumber,
                               LBNA_Column, 1,
                                 LBNCA_Integer, presetNumber,
                               LBNA_Column, 2,
                                 LBNCA_CopyText, FALSE,
                                 LBNCA_Text, instrumentName,
                               LBNA_Column, 3,
                                 LBNCA_CopyText, FALSE,
                                 LBNCA_Text, NULL,
                               LBNA_Column, 4,
                                 LBNCA_Integer, NULL,
                               LBNA_Column, 5,
                                 LBNCA_Integer, NULL,
                               LBNA_Column, 6,
                                 LBNCA_Integer, NULL,
                               LBNA_Column, 7,
                                 LBNCA_CopyText, FALSE,
                                 LBNCA_Text, NULL,
                               LBNA_Column, 8,
                                 LBNCA_Integer, NULL,
                               LBNA_Column, 9,
                                 LBNCA_Integer, NULL,
                               LBNA_Column, 10,
                                 LBNCA_Integer, NULL,
                               LBNA_Column, 11,
                                 LBNCA_Integer, NULL,
                               LBNA_Column, 12,
                                 LBNCA_Integer, NULL,
                               LBNA_Column, 13,
                                 LBNCA_Integer, NULL,
                               LBNA_Column, 14,
                                 LBNCA_Integer, NULL,
                               LBNA_Column, 15,
                                 LBNCA_CopyText, FALSE,
                                 LBNCA_Text, NULL,
                               TAG_DONE );
    AddTail( labels, label );
  }
  for ( i = 0; 47 > i; ++i ) {

    const LONG j = i + 35;
    CONST_STRPTR percussionName = GetMidiName( 128, 0, j, j, j, j );
    const LONG * bankNumber = GetListNumber( 128 );
    const LONG * presetNumber = GetListNumber( 0 );
    const LONG * sampleNumber = GetListNumber( j );

    struct Node * label = AllocListBrowserNode( columns,
                               LBNA_Column, 0,
                                 LBNCA_Integer, bankNumber,
                               LBNA_Column, 1,
                                 LBNCA_Integer, presetNumber,
                               LBNA_Column, 2,
                                 LBNCA_CopyText, FALSE,
                                 LBNCA_Text, percussionName,
                               LBNA_Column, 3,
                                 LBNCA_CopyText, FALSE,
                                 LBNCA_Text, NULL,
                               LBNA_Column, 4,
                                 LBNCA_Integer, sampleNumber,
                               LBNA_Column, 5,
                                 LBNCA_Integer, sampleNumber,
                               LBNA_Column, 6,
                                 LBNCA_Integer, NULL,
                               LBNA_Column, 7,
                                 LBNCA_CopyText, FALSE,
                                 LBNCA_Text, NULL,
                               LBNA_Column, 8,
                                 LBNCA_Integer, NULL,
                               LBNA_Column, 9,
                                 LBNCA_Integer, NULL,
                               LBNA_Column, 10,
                                 LBNCA_Integer, NULL,
                               LBNA_Column, 11,
                                 LBNCA_Integer, NULL,
                               LBNA_Column, 12,
                                 LBNCA_Integer, sampleNumber,
                               LBNA_Column, 13,
                                 LBNCA_Integer, sampleNumber,
                               LBNA_Column, 14,
                                 LBNCA_Integer, NULL,
                               LBNA_Column, 15,
                                 LBNCA_CopyText, FALSE,
                                 LBNCA_Text, NULL,
                               TAG_DONE );
    AddTail( labels, label );
  }
}

BOOL CreateAmiSfListLabels( struct List * labels,
                            struct AmiSF * amisf,
                            struct ProgressDialog * dialog,
                            ULONG * currentProgress,
                            ULONG maxProgress ) {

  LONG bankIndex;
  LONG presetIndex;

  for ( bankIndex = 0; bankIndex <= 128; ++bankIndex ) {
    for ( presetIndex = 0; presetIndex <= 128; ++presetIndex ) {

      struct AmiSF_Preset * preset =
        &( amisf->asf_Preset[ bankIndex ][ presetIndex ]);

      if (( preset->asfp_Bank != 255 ) || ( preset->asfp_Preset != 255 )) {

        UWORD noteCount = preset->asfp_NoteStart + preset->asfp_NoteCount;
        LONG noteIndex = preset->asfp_NoteStart;
        LONG minNote = 0;

        for ( ; ( noteIndex < noteCount ); ++noteIndex ) {

          struct AmiSF_Note * note = &( amisf->asf_Note[ noteIndex ]);
          LONG maxNote = note->asfn_MaxNote;
          CONST_STRPTR gmName = GetMidiName( bankIndex,
                                             presetIndex,
                                             minNote,
                                             maxNote,
                                             minNote,
                                             maxNote );
          LONG sampleNumber = note->asfn_SampleIndex - 1;
          struct Node * label = CreateAmiSfListBrowserNode( bankIndex,
                                                            presetIndex,
                                                            gmName,
                                                            minNote,
                                                            maxNote,
                                                            noteIndex,
                                                            note->asfn_Attack,
                                                            note->asfn_Decay,
                                                            note->asfn_Sustain,
                                                            note->asfn_Release,
                                                            sampleNumber );

          LOG_D(( "D: Found b:%ld p:%ld %s n:%ld-%ld -> %ld s:%ld\n",
                   bankIndex, presetIndex,
                   gmName,
                   minNote, maxNote, noteIndex,
                   sampleNumber ));
          LOG_V(( "V: Inserting label\n" ));
          AddTail( labels, label );

          minNote = maxNote + 1;
        }        
      }
    }
  }
  return FALSE;
}

BOOL GetAmiSfIndicesForIndex( UBYTE * bankIndex,
                              UBYTE * presetIndex,
                              LONG * noteIndex,
                              struct AmiSF * amisf,
                              ULONG index ) {

  UBYTE tempBankIndex;
  UBYTE tempPresetIndex;
  UBYTE tempNoteIndex;

  for ( tempBankIndex = 0; tempBankIndex <= 128; ++tempBankIndex ) {
    for ( tempPresetIndex = 0; tempPresetIndex <= 128; ++tempPresetIndex ) {

      struct AmiSF_Preset * preset  =
        &( amisf->asf_Preset[ tempBankIndex ][ tempPresetIndex ]);
      UWORD noteCount = preset->asfp_NoteCount;

      for ( tempNoteIndex = 0; tempNoteIndex < noteCount; ++tempNoteIndex ) {

        if ( !index ) {

          *bankIndex = tempBankIndex;
          *presetIndex = tempPresetIndex;
          *noteIndex = preset->asfp_NoteStart + tempNoteIndex;

          return TRUE;
        }

        --index;
      }
    }
  }
  return FALSE;
}

BOOL CreateSf2ListLabels(
  struct List * labels,
  struct SF2 * sf2,
  struct ProgressDialog * dialog,
  ULONG * currentProgress,
  ULONG maxProgress ) {

  BOOL abort = FALSE;

  /* Begin iteration over all presets - instruments - samples */
  struct SF2_Preset * preset;
  ULONG count = 0;

  FOR_LIST( &( sf2->sf2_Presets ),
            preset,
            struct SF2_Preset * ) {

    struct SF2_Args * argsP;

    FOR_LIST( &( preset->sf2p_Args ),
              argsP,
              struct SF2_Args * ) {

      const LONG instrumentIndex = argsP->sf2a_Values.sf2v_NextNumber;

      struct SF2_Instrument * instrument;
      struct SF2_Args * argsI;

      if ( 0 > instrumentIndex ) {

        // Skip over the de-duplicated instruments
        continue;
      }

      instrument = sf2->sf2_InstrumentArray[ instrumentIndex ];

      FOR_LIST( &( instrument->sf2i_Args ),
                argsI,
                struct SF2_Args * ) {

        const LONG sampleIndex = argsI->sf2a_Values.sf2v_NextNumber;
        const UBYTE sampleMin = argsI->sf2a_Values.sf2v_LowNote;
        const UBYTE sampleMax = argsI->sf2a_Values.sf2v_HighNote;
        const UBYTE instrumentMin = argsP->sf2a_Values.sf2v_LowNote;
        const UBYTE instrumentMax = argsP->sf2a_Values.sf2v_HighNote;

        if ( 0 > sampleIndex ) {

          // Skip over the de-duplicated samples
          continue;
        }

        if (( sampleMin <= instrumentMax )
          && ( instrumentMin <= sampleMax)) {

          // Skip over unreachable samples

          /* Iteration payload below */
          UWORD bank = preset->sf2p_Bank;
          UWORD presetNumber = preset->sf2p_Common.sf2c_Number;
          UWORD instrumentNumber = instrument->sf2i_Common.sf2c_Number;
          struct SF2_Sample * sample = sf2->sf2_SampleArray[ sampleIndex ];
          UWORD sampleNumber = sample->sf2s_Number;
          CONST_STRPTR presetName = preset->sf2p_Common.sf2c_Name;
          CONST_STRPTR gmName = GetMidiName( bank,
                                             presetNumber,
                                             instrumentMin,
                                             instrumentMax,
                                             sampleMin,
                                             sampleMax );
          CONST_STRPTR instrumentName = instrument->sf2i_Common.sf2c_Name;
          CONST_STRPTR sampleName = sample->sf2s_Name;

          WORD attack;
          WORD decay;
          WORD sustain;
          WORD release;

          struct Node * label;

          GetSf2ADSRs( argsP, argsI, &attack, &decay, &sustain, &release );

          label = CreateSf2ListBrowserNode( bank,
                                            presetNumber,
                                            gmName,
                                            presetName,
                                            instrumentMin,
                                            instrumentMax,
                                            instrumentNumber,
                                            instrumentName,
                                            attack,
                                            decay,
                                            sustain,
                                            release,
                                            sampleMin,
                                            sampleMax,
                                            sampleNumber,
                                            sampleName );
          LOG_V(( "V: Inserting label\n" ));
          AddTail( labels, label );
          ++count;
        }    
        /* Complete iteration over all presets - instruments - samples */
      }
    }
    ++( *currentProgress );
    abort |= HandleProgressDialogTick( dialog,
                                       *currentProgress,
                                       maxProgress );
    if ( abort ) {
  
      return TRUE;
    }
  }
  LOG_I(( "I: Created %ld labels, progress %ld/%ld\n",
          count, *currentProgress, maxProgress ));
  return FALSE;
}

BOOL GetSf2InformationForIndex(
  struct SF2_Preset ** preset,
  struct SF2_Args ** presetArgs,
  struct SF2_Instrument ** instrument,
  struct SF2_Args ** instrumentArgs,
  struct SF2_Sample ** sample,
  struct SF2 * sf2,
  const ULONG index ) {

  /* Begin iteration over all presets - instruments - samples */
  struct SF2_Preset * tempPreset;
  ULONG count = 0;

  FOR_LIST( &( sf2->sf2_Presets ),
            tempPreset,
            struct SF2_Preset * ) {

    struct SF2_Args * argsP;

    FOR_LIST( &( tempPreset->sf2p_Args ),
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

        if ( 0 > sampleIndex ) {

          // Skip over the de-duplicated samples
          continue;
        }

        if (( sampleMin > instrumentMax )
          || ( instrumentMin > sampleMax )) {

          // Skip over unreachable samples
          continue;
        }

        tempSample = sf2->sf2_SampleArray[ sampleIndex ];

        /* Iteration payload below */

        if ( index == count ) {

          LOG_D(( "V: Found bank %ld preset %ld instrument %ld sample %ld\n",
            tempPreset->sf2p_Bank,
            tempPreset->sf2p_Common.sf2c_Number,
            argsP->sf2a_Values.sf2v_NextNumber,
            argsI->sf2a_Values.sf2v_NextNumber ));

          *preset = tempPreset;
          *presetArgs = argsP;
          *instrument = tempInstrument;
          *instrumentArgs = argsI;
          *sample = tempSample;
          return TRUE;
        }

        /* Complete iteration over all presets - instruments - samples */

        ++count;
      }
    }
  }
  *preset = NULL;
  *instrument = NULL;
  *sample = NULL;
  return FALSE;
}

VOID FreeListLabels( struct List * list ) {

  struct Node * label;
  while ( label = RemHead( list )) {

    struct SfListNodeData * data = NULL;
    GetListBrowserNodeAttrs( label, LBNA_UserData, &data, TAG_DONE );
    if ( data ) {

      LOG_V(( "V: Deleting list node 0x%08lx's data 0x%08lx\n",
              label, data ));
      FreeMem( data, sizeof( struct SfListNodeData ));
    }
    FreeListBrowserNode( label );
  }
  LOG_D(( "D: List labels emptied.\n" ));
}
