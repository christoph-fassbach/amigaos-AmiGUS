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
#include "sf2_optimizer.h"
#include "support.h"

/******************************************************************************
 * SF2 optimizer - private functions for InsertSorted.
 *****************************************************************************/

LONG PresetNodeCompare( struct Node * a, struct Node * b ) {

  struct SF2_Preset * aa = ( struct SF2_Preset * ) a;
  struct SF2_Preset * bb = ( struct SF2_Preset * ) b;
  LONG major = bb->sf2p_Bank - aa->sf2p_Bank;
  LONG minor = bb->sf2p_Common.sf2c_Number - aa->sf2p_Common.sf2c_Number;
  LONG result = ( major ) ? major : minor;

  LOG_V(( "V: Comparing 0x%08lx with bank %ld preset %ld "
          "and 0x%08lx with bank %ld preset %ld "
          "is %ld (%ld vs %ld)\n",
          aa, aa->sf2p_Bank, aa->sf2p_Common.sf2c_Number,
          bb, bb->sf2p_Bank, bb->sf2p_Common.sf2c_Number,
          result, major, minor ));
  return result;
}

LONG PresetArgsNodeCompare( struct Node * a, struct Node * b ) {

  struct SF2_Args * aa = ( struct SF2_Args * ) a;
  struct SF2_Args * bb = ( struct SF2_Args * ) b;
  LONG result = bb->sf2a_Values.sf2v_LowNote - aa->sf2a_Values.sf2v_LowNote;

  return result;
}

/******************************************************************************
 * SF2 optimizer - private functions offloading public work.
 *****************************************************************************/

// (# Presets + # Instruments + #Samples) >> 2 progress
// BOOL True = abort
BOOL PrepareIndex( struct SF2 * sf2,
                   struct ProgressDialog * dialog,
                   ULONG * currentProgress,
                   ULONG maxProgress ) {

  BOOL abort = FALSE;
  struct SF2_Sample * sample;
  struct SF2_Instrument * instrument;

  sf2->sf2_SampleArray =
    AllocMem( sf2->sf2_SampleCount * sizeof( struct SF2_Sample * ),
              MEMF_ANY | MEMF_CLEAR );
  FOR_LIST( &( sf2->sf2_Samples ), sample, struct SF2_Sample * ) {

    LONG i = sample->sf2s_Number;
    if ( !( sf2->sf2_SampleArray[ i ] )) {

      sf2->sf2_SampleArray[ i ] = sample;

    } else {

      LOG_E(( "E: Cannot create sample index!\n" ));
    }
  }
  *currentProgress += ( sf2->sf2_SampleCount >> 2 );
  abort |= HandleProgressDialogTick( dialog,
                                     *currentProgress,
                                     maxProgress );
  if ( abort ) {

    return abort;
  }

  sf2->sf2_InstrumentArray =
    AllocMem( sf2->sf2_InstrumentCount * sizeof( struct SF2_Instrument * ),
              MEMF_ANY | MEMF_CLEAR );
  FOR_LIST( &( sf2->sf2_Instruments ), instrument, struct SF2_Instrument * ) {

    LONG i = instrument->sf2i_Common.sf2c_Number;
    if ( !( sf2->sf2_InstrumentArray[ i ] )) {

      sf2->sf2_InstrumentArray[ i ] = instrument;

    } else {

      LOG_E(( "E: Cannot create instrument index!\n" ));
    }
  }
  *currentProgress += ( sf2->sf2_InstrumentCount >> 2 );
  abort |= HandleProgressDialogTick( dialog,
                                     *currentProgress,
                                     maxProgress );
  if ( abort ) {

    return abort;
  }

  InsertionSort(( struct List * ) &( sf2->sf2_Presets ),
                  &PresetNodeCompare );
  *currentProgress += ( sf2->sf2_PresetCount >> 2 );
  abort |= HandleProgressDialogTick( dialog,
                                     *currentProgress,
                                     maxProgress );
  if ( abort ) {

    return abort;
  }

  LOG_I(( "I: Indices created.\n" ));
  return FALSE;
}

// # Presets progress
// BOOL True = abort
#define LOG_FPH LOG_V
BOOL FlattenPresetHierarchy( struct SF2 * sf2,
                             struct ProgressDialog * dialog,
                             ULONG * currentProgress,
                             ULONG maxProgress ) {

  struct SF2_Preset * preset;
  FOR_LIST( &( sf2->sf2_Presets ),
            preset,
            struct SF2_Preset * ) {

    BOOL abort;
    struct SF2_Zone * zoneP;

    FOR_LIST( &( preset->sf2p_Common.sf2c_Zones ),
              zoneP,
              struct SF2_Zone * ) {

      struct SF2_Generator * generatorP;
      ULONG argValueFlags = 0x00000000;
      struct SF2_ArgValues ArgValues;
      ArgValues.sf2v_LowNote = 0;
      ArgValues.sf2v_HighNote = 127;
      ArgValues.sf2v_Attack = -12000;
      ArgValues.sf2v_Decay = -12000;
      ArgValues.sf2v_Sustain = 0;
      ArgValues.sf2v_Release = -12000;

      LOG_FPH(( "V: Preset %ld-%ld\n",
                preset->sf2p_Bank,
                preset->sf2p_Common.sf2c_Number ));
      FOR_LIST( &( zoneP->sfz2_Generators ),
                generatorP,
                struct SF2_Generator * ) {

        switch ( generatorP->sf2g_Id ) {
          case GEN_KEYRANGE: {

            UBYTE low = ( 0x00FF & generatorP->sf2g_Amount );
            UBYTE high = (( 0xFF00 & generatorP->sf2g_Amount ) >> 8 );
            LOG_FPH(( "V:   +-> Has KeyRange %ld-%ld\n", low, high ));
            ArgValues.sf2v_LowNote = low;
            ArgValues.sf2v_HighNote = high;
            argValueFlags |= SF2_ARG_VALUE_FLAG_LOW_NOTE;
            argValueFlags |= SF2_ARG_VALUE_FLAG_HIGH_NOTE;
            break;
          }
          case GEN_VELRANGE: {

            UBYTE low = ( 0x00FF & generatorP->sf2g_Amount );
            UBYTE high = (( 0xFF00 & generatorP->sf2g_Amount ) >> 8 );
            LOG_FPH(( "V:   +-> Has VelRange %ld-%ld\n", low, high ));
            break;
          }
          case GEN_VOLENVATTACK: {

            WORD value = ( WORD ) generatorP->sf2g_Amount;
            LOG_FPH(( "V:   +-> Has Attack %ld\n", value ));
            ArgValues.sf2v_Attack = value;
            break;
          }
          case GEN_VOLENVDECAY: {

            WORD value = ( WORD ) generatorP->sf2g_Amount;
            LOG_FPH(( "V:   +-> Has Decay %ld\n", value ));
            ArgValues.sf2v_Decay = value;
            break;
          }
          case GEN_VOLENVSUSTAIN: {

            WORD value = ( WORD ) generatorP->sf2g_Amount;
            LOG_FPH(( "V:   +-> Has Sustain %ld\n", value ));
            ArgValues.sf2v_Sustain = value;
            break;
          }
          case GEN_VOLENVRELEASE: {

            WORD value = ( WORD ) generatorP->sf2g_Amount;
            LOG_FPH(( "V:   +-> Has Release %ld\n", value ));
            ArgValues.sf2v_Release = value;
            break;
          }
          case GEN_INSTRUMENT: {

            UWORD next = generatorP->sf2g_Amount;
            LOG_FPH(( "V:   +-> Has Instrument %ld\n", next ));
            ArgValues.sf2v_NextNumber = next;
            argValueFlags |= SF2_ARG_VALUE_FLAG_NEXT_NUMBER;
            break;
          }
          default: {
            LOG_FPH(( "V:   +-> Has id %ld and value %ld\n",
                    generatorP->sf2g_Id,
                    generatorP->sf2g_Amount ));
            break;
          }
        }
        if ( SF2_ARG_VALUE_FLAG_ALL_REQUIRED 
             == ( SF2_ARG_VALUE_FLAG_ALL_REQUIRED & argValueFlags )) {

          struct SF2_Args * args = AllocMem( sizeof( struct SF2_Args ),
                                             MEMF_ANY | MEMF_CLEAR );
          CopyMem( &ArgValues,
                   &( args->sf2a_Values ),
                   sizeof( struct SF2_ArgValues ));
          InsertSorted(( struct Node * ) &( args->sf2a_Node ),
                       ( struct List * ) &( preset->sf2p_Args ),
                       &PresetArgsNodeCompare );
        }
      }
    }
    abort = HandleProgressDialogTick( dialog,
                                      *currentProgress,
                                      maxProgress );
    if ( abort ) {

      return TRUE;
    }
    ++( *currentProgress );
  }
  return FALSE;
}

// # Instruments progress
// BOOL True = abort
#define LOG_FIH LOG_D
BOOL FlattenInstrumentHierarchy( struct SF2 * sf2,
                                 struct ProgressDialog * dialog,
                                 ULONG * currentProgress,
                                 ULONG maxProgress ) {

  struct SF2_Instrument * instrument;
  FOR_LIST( &( sf2->sf2_Instruments ),
            instrument,
            struct SF2_Instrument * ) {

    BOOL abort;
    struct SF2_Zone * zoneI;

    FOR_LIST( &( instrument->sf2i_Common.sf2c_Zones ),
              zoneI,
              struct SF2_Zone * ) {

      struct SF2_Generator * generatorI;
      ULONG argValueFlags = 0x00000000;
      struct SF2_ArgValues ArgValues;
      ArgValues.sf2v_LowNote = 0;
      ArgValues.sf2v_HighNote = 127;
      ArgValues.sf2v_Attack = -12000;
      ArgValues.sf2v_Decay = -12000;
      ArgValues.sf2v_Sustain = 0;
      ArgValues.sf2v_Release = -12000;

      LOG_FIH(( "V: Instrument %ld - %s\n",
                instrument->sf2i_Common.sf2c_Number,
                instrument->sf2i_Common.sf2c_Name ));
      FOR_LIST( &( zoneI->sfz2_Generators ),
                generatorI,
                struct SF2_Generator * ) {

        switch ( generatorI->sf2g_Id ) {
          case GEN_KEYRANGE: {

            UBYTE low = ( 0x00FF & generatorI->sf2g_Amount );
            UBYTE high = (( 0xFF00 & generatorI->sf2g_Amount ) >> 8 );
            LOG_FIH(( "V:   +-> Has KeyRange %ld-%ld\n", low, high ));
            ArgValues.sf2v_LowNote = low;
            ArgValues.sf2v_HighNote = high;
            argValueFlags |= SF2_ARG_VALUE_FLAG_LOW_NOTE;
            argValueFlags |= SF2_ARG_VALUE_FLAG_HIGH_NOTE;
            break;
          }
          case GEN_VELRANGE: {
            UBYTE low = ( 0x00FF & generatorI->sf2g_Amount );
            UBYTE high = (( 0xFF00 & generatorI->sf2g_Amount ) >> 8 );
            LOG_FIH(( "V:   +-> Has VelRange %ld-%ld\n", low, high ));
            break;
          }
          case GEN_VOLENVATTACK: {

            WORD value = ( WORD ) generatorI->sf2g_Amount;
            LOG_FIH(( "V:   +-> Has Attack %ld\n", value ));
            ArgValues.sf2v_Attack = value;
            break;
          }
          case GEN_VOLENVDECAY: {

            WORD value = ( WORD ) generatorI->sf2g_Amount;
            LOG_FIH(( "V:   +-> Has Decay %ld\n", value ));
            ArgValues.sf2v_Decay = value;
            break;
          }
          case GEN_VOLENVSUSTAIN: {

            WORD value = ( WORD ) generatorI->sf2g_Amount;
            LOG_FIH(( "V:   +-> Has Sustain %ld\n", value ));
            ArgValues.sf2v_Sustain = value;
            break;
          }
          case GEN_VOLENVRELEASE: {

            WORD value = ( WORD ) generatorI->sf2g_Amount;
            LOG_FIH(( "V:   +-> Has Release %ld\n", value ));
            ArgValues.sf2v_Release = value;
            break;
          }
          case GEN_SAMPLEID: {

            UWORD next = generatorI->sf2g_Amount;
            LOG_FIH(( "V:   +-> Has Sample %ld\n", next ));
            ArgValues.sf2v_NextNumber = next;
            argValueFlags |= SF2_ARG_VALUE_FLAG_NEXT_NUMBER;
            break;
          }
          default: {
            LOG_FIH(( "V:   +-> Has id %ld and value %ld\n",
                      generatorI->sf2g_Id,
                      generatorI->sf2g_Amount ));
            break;
          }
        }
        if ( SF2_ARG_VALUE_FLAG_ALL_REQUIRED 
             == ( SF2_ARG_VALUE_FLAG_ALL_REQUIRED & argValueFlags )) {

          struct SF2_Args * args = AllocMem( sizeof( struct SF2_Args ),
                                             MEMF_ANY );
          CopyMem( &ArgValues,
                   &( args->sf2a_Values ),
                   sizeof( struct SF2_ArgValues ));
          InsertSorted(( struct Node * ) &( args->sf2a_Node ),
                       ( struct List * ) &( instrument->sf2i_Args ),
                       &PresetArgsNodeCompare );
        }
      }
    }
    abort = HandleProgressDialogTick( dialog,
                                      *currentProgress,
                                      maxProgress );
    if ( abort ) {

      return TRUE;
    }
    ++( *currentProgress );
  }
  return FALSE;
}

// # Instruments progress
// BOOL True = abort
BOOL DeDuplicateInstruments( struct SF2 * sf2,
                             struct ProgressDialog * dialog,
                             ULONG * currentProgress,
                             ULONG maxProgress ) {

  BOOL abort = FALSE;
  ULONG count = 0;
  struct SF2_Preset * preset;
  FOR_LIST( &( sf2->sf2_Presets ),
            preset,
            struct SF2_Preset * ) {

    struct SF2_Args * argsPouter;

    FOR_LIST( &( preset->sf2p_Args ),
              argsPouter,
              struct SF2_Args * ) {

      LONG nextOuter = argsPouter->sf2a_Values.sf2v_NextNumber;
      ULONG lowOuter = argsPouter->sf2a_Values.sf2v_LowNote;
      ULONG highOuter = argsPouter->sf2a_Values.sf2v_HighNote;

      struct SF2_Args * argsPinner;

      if ( 0 > nextOuter ) {

        continue;
      }

      FOR_LIST( &( preset->sf2p_Args ),
                argsPinner,
                struct SF2_Args * ) {

        LONG nextInner = argsPinner->sf2a_Values.sf2v_NextNumber;
        ULONG lowInner = argsPinner->sf2a_Values.sf2v_LowNote;
        ULONG highInner = argsPinner->sf2a_Values.sf2v_HighNote;

        if ( 0 > nextInner ) {

          continue;
        }

        if (( argsPinner != argsPouter )
          &&( nextInner == nextOuter )
          && ( lowInner == lowOuter )
          && ( highInner == highOuter )) {

          LONG bank = preset->sf2p_Bank;
          LONG presetNumber = preset->sf2p_Common.sf2c_Number;

          LOG_V(( "V: Removing duplicate instrument p:%ld %ld "
                    "i:%ld >%ld <%ld\n",
                    bank, presetNumber,
                    nextOuter, lowOuter, highOuter ));

          argsPinner->sf2a_Values.sf2v_NextNumber = -1;
          ++count;
        }
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
  LOG_I(( "I: Removed %ld duplicate instruments, progress %ld/%ld\n",
          count, *currentProgress, maxProgress ));
  return FALSE;
}

// # Instruments progress
// BOOL True = abort
BOOL DeDuplicateSamples( struct SF2 * sf2,
                         struct ProgressDialog * dialog,
                         ULONG * currentProgress,
                         ULONG maxProgress ) {

  BOOL abort = FALSE;
  ULONG count = 0;
  struct SF2_Preset * preset;
  FOR_LIST( &( sf2->sf2_Presets ),
            preset,
            struct SF2_Preset * ) {

    struct SF2_Args * argsP;

    FOR_LIST( &( preset->sf2p_Args ),
              argsP,
              struct SF2_Args * ) {

      struct SF2_Args * argsIouter;
      struct SF2_Instrument * instrument;

      LONG nextI = argsP->sf2a_Values.sf2v_NextNumber;
      if ( 0 > nextI ) {

        continue;
      }

      instrument = sf2->sf2_InstrumentArray[ nextI ];

      FOR_LIST( &( instrument->sf2i_Args ),
                argsIouter,
                struct SF2_Args * ) {

        LONG nextOuter = argsIouter->sf2a_Values.sf2v_NextNumber;
        ULONG lowOuter = argsIouter->sf2a_Values.sf2v_LowNote;
        ULONG highOuter = argsIouter->sf2a_Values.sf2v_HighNote;
        struct SF2_Args * argsIinner;

        if ( 0 > nextOuter ) {

          continue;
        }

        FOR_LIST( &( instrument->sf2i_Args ),
                  argsIinner,
                  struct SF2_Args * ) {
          
          LONG nextInner = argsIinner->sf2a_Values.sf2v_NextNumber;
          ULONG lowInner = argsIinner->sf2a_Values.sf2v_LowNote;
          ULONG highInner = argsIinner->sf2a_Values.sf2v_HighNote;

          if ( 0 > nextInner ) {

            continue;
          }

          if (( argsIinner != argsIouter )
            && ( nextInner == nextOuter )
            && ( lowInner == lowOuter )
            && ( highInner == highOuter )) {

            LONG bank = preset->sf2p_Bank;
            LONG presetNumber = preset->sf2p_Common.sf2c_Number;
            LONG instrumentMin = argsP->sf2a_Values.sf2v_LowNote;
            LONG instrumentMax = argsP->sf2a_Values.sf2v_HighNote;
            LONG instrumentNumber = instrument->sf2i_Common.sf2c_Number;

            LOG_V(( "V: Removing duplicate sample for "
                    "p:%ld %ld i:%ld >%ld <%ld s:%ld >%ld <%ld\n",
                    bank, presetNumber,
                    instrumentNumber, instrumentMin, instrumentMax,
                    nextInner, lowInner, highInner ));

            argsIinner->sf2a_Values.sf2v_NextNumber = -1;
            ++count;
          }
        }
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
  LOG_I(( "I: Removed %ld duplicate samples, progress %ld/%ld\n",
          count, *currentProgress, maxProgress ));
  return FALSE;
}

/******************************************************************************
 * SF2 optimizer - public functions.
 *****************************************************************************/

BOOL OptimizeSF2( struct SF2 * sf2,
                  struct ProgressDialog * dialog,
                  ULONG * currentProgress,
                  ULONG * maxProgress ) {

  BOOL abort = FALSE;

  ULONG pCount = sf2->sf2_PresetCount;
  ULONG iCount = sf2->sf2_InstrumentCount;
  ULONG sCount = sf2->sf2_SampleCount;
    
  *maxProgress += (( pCount + iCount + sCount ) >> 2) // PrepareIndex
                 + pCount              // FlattenPresetHierarchy
                 + iCount              // FlattenInstrumentHierarchy
                 + pCount              // DeDuplicateSamples
                 + pCount              // DeDuplicateInstruments
                 + 0;

  if ( !( abort )) {

    abort = PrepareIndex( sf2,
                          dialog,
                          currentProgress,
                          *maxProgress );
  }
  if ( !( abort )) {

    abort = FlattenPresetHierarchy( sf2,
                                    dialog,
                                    currentProgress,
                                    *maxProgress );
  }
  if ( !( abort )) {

    abort = FlattenInstrumentHierarchy( sf2,
                                        dialog,
                                        currentProgress,
                                        *maxProgress );
  }
  if ( !( abort )) {

    abort = DeDuplicateInstruments( sf2,
                                    dialog,
                                    currentProgress,
                                    *maxProgress );
  }
  if ( !( abort )) {

    abort = DeDuplicateSamples( sf2,
                                dialog,
                                currentProgress,
                                *maxProgress );
  }

  return abort;
}
