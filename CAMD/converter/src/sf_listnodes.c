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
#include "support.h"
#include "SDI_compiler.h"

static const struct ColumnInfo instrumentColumns[] = {
// chars * 8   + 8 margin left / right
  { (  3 * 8 ) + 8, "B#", CIF_CENTER },
  { (  3 * 8 ) + 8, "P#", CIF_CENTER },
  { ( 23 * 8 ) + 8, "GM Name", CIF_CENTER },
  { ( 20 * 8 ) + 8, "Preset Name", CIF_CENTER },
  { ( 20 * 8 ) + 8, "Instrument Name", CIF_CENTER },
  { ( 20 * 8 ) + 8, "Sample Name", CIF_CENTER },
  { -1, (STRPTR)~0, -1 }
};

static const UBYTE * instrumentNames[] = {
  "  0", "Acoustic Grand Piano",
  "  1", "Bright Acoustic Piano",
  "  2", "Electric Grand Piano",
  "  3", "Honky-tonk Piano",
  "  4", "Electric Piano 1",
  "  5", "Electric Piano 2",
  "  6", "Harpsichord",
  "  7", "Clavinet",
  "  8", "Celesta",
  "  9", "Glockenspiel",
  " 10", "Music Box",
  " 11", "Vibraphone",
  " 12", "Marimba",
  " 13", "Xylophone",
  " 14", "Tubular Bells",
  " 15", "Dulcimer",
  " 16", "Drawbar Organ",
  " 17", "Percussive Organ",
  " 18", "Rock Organ",
  " 29", "Church Organ",
  " 20", "Reed Organ",
  " 21", "Accordion",
  " 22", "Harmonica",
  " 23", "Bandoneon",
  " 24", "Acoustic Guitar (nylon)",
  " 25", "Acoustic Guitar (steel)",
  " 26", "Electric Guitar (jazz)",
  " 27", "Electric Guitar",
  " 28", "Electric Guitar (muted)",
  " 29", "Overdriven Guitar",
  " 30", "Distortion Guitar",
  " 31", "Guitar Harmonics",
  " 32", "Acoustic Bass",
  " 33", "Electric Bass (finger)",
  " 34", "Electric Bass (picked)",
  " 35", "Fretless Bass",
  " 36", "Slap Bass 1",
  " 37", "Slap Bass 2",
  " 38", "Synth Bass 1",
  " 39", "Synth Bass 2",
  " 40", "Violin",
  " 41", "Viola",
  " 42", "Cello",
  " 43", "Contrabass",
  " 44", "Tremolo Strings",
  " 45", "Pizzicato Strings",
  " 46", "Orchestral Harp",
  " 47", "Timpani",
  " 48", "String Ensemble 1",
  " 49", "String Ensemble 2",
  " 50", "Synth Strings 1",
  " 51", "Synth Strings 2",
  " 52", "Choir Aahs",
  " 53", "Voice Oohs",
  " 54", "Synth Voice",
  " 55", "Orchestra Hit",
  " 56", "Trumpet",
  " 57", "Trombone",
  " 58", "Tuba",
  " 59", "Muted Trumpet",
  " 60", "French Horn",
  " 61", "Brass Section",
  " 62", "Synth Brass 1",
  " 63", "Synth Brass 2",
  " 64", "Soprano Sax",
  " 65", "Alto Sax",
  " 66", "Tenor Sax",
  " 67", "Baritone Sax",
  " 68", "Oboe",
  " 69", "English Horn",
  " 70", "Bassoon",
  " 71", "Clarinet",
  " 72", "Piccolo",
  " 73", "Flute",
  " 74", "Recorder",
  " 75", "Pan Flute",
  " 76", "Blown bottle",
  " 77", "Shakuhachi",
  " 78", "Whistle",
  " 79", "Ocarina",
  " 80", "Lead 1",
  " 81", "Lead 2",
  " 82", "Lead 3",
  " 83", "Lead 4",
  " 84", "Lead 5",
  " 85", "Lead 6",
  " 86", "Lead 7",
  " 87", "Lead 8",
  " 88", "Pad 1",
  " 89", "Pad 2",
  " 90", "Pad 3",
  " 91", "Pad 4",
  " 92", "Pad 5",
  " 93", "Pad 6",
  " 94", "Pad 7",
  " 95", "Pad 8",
  " 96", "FX 1",
  " 97", "FX 2",
  " 98", "FX 3",
  " 99", "FX 4",
  "100", "FX 5",
  "101", "FX 6",
  "102", "FX 7",
  "103", "FX 8",
  "104", "Sitar",
  "105", "Banjo",
  "106", "Shamisen",
  "107", "Koto",
  "108", "Kalimba",
  "109", "Bag pipe",
  "110", "Fiddle",
  "111", "Shanai",
  "112", "Tinkle Bell",
  "113", "Cowbell",
  "114", "Steel Drums",
  "115", "Woodblock",
  "116", "Taiko Drum",
  "117", "Melodic Tom",
  "118", "Synth Drum",
  "119", "Reverse Cymbal",
  "120", "Guitar Fret Noise",
  "121", "Breath Noise",
  "122", "Seashore",
  "123", "Bird Tweet",
  "124", "Telephone Ring",
  "125", "Helicopter",
  "126", "Applause",
  "127", "Gunshot",
  NULL
};

struct Node * CreateListBrowserNode( CONST_STRPTR string0,
                                     CONST_STRPTR string1,
                                     CONST_STRPTR string2,
                                     CONST_STRPTR string3,
                                     CONST_STRPTR string4,
                                     CONST_STRPTR string5,
                                     CONST_STRPTR string6 ) {

  LONG columns = sizeof( instrumentColumns ) / sizeof( struct ColumnInfo );
  return AllocListBrowserNode( columns,
                               LBNA_Column, 0,
                                 LBNCA_CopyText, TRUE,
                                 LBNCA_Text, string0,
                                 LBNCA_Justification, LCJ_CENTER,
                               LBNA_Column, 1,
                                 LBNCA_CopyText, TRUE,
                                 LBNCA_Text, string1,
                                 LBNCA_Justification, LCJ_CENTER,
                               LBNA_Column, 2,
                                 LBNCA_CopyText, FALSE,
                                 LBNCA_Text, string2,
                               LBNA_Column, 3,
                                 LBNCA_CopyText, FALSE,
                                 LBNCA_Text, string3,
                               LBNA_Column, 4,
                                 LBNCA_CopyText, FALSE,
                                 LBNCA_Text, string4,
                               LBNA_Column, 5,
                                 LBNCA_CopyText, FALSE,
                                 LBNCA_Text, string5,
                               LBNA_Column, 6,
                                 LBNCA_CopyText, FALSE,
                                 LBNCA_Text, string6,
                               TAG_DONE );
}

VOID OrderedInsertListBrowserNode( struct List * list,
                                   struct Node * node,
                                   STRPTR bank,
                                   STRPTR number ) {

  struct Node * next;
  struct Node * previous = NULL;

  STRPTR nextBank = NULL;
  STRPTR nextNumber = NULL;
  BOOL added = FALSE;

  FOR_LIST ( list, next, struct Node * ) {
    GetListBrowserNodeAttrs( next,
                             LBNA_Column, 0,
                               LBNCA_Text, &nextBank,
                             LBNA_Column, 1,
                               LBNCA_Text, &nextNumber,
                             TAG_END );
    if (( 0 <= C_strcmp( nextBank, bank )) && 
        ( 0 < C_strcmp( nextNumber, number ))) {
      if ( previous ) {
        Insert( list, node, previous );
        
      } else {
        AddHead( list, node );
      }
      added = TRUE;
      break;
    }
    previous = next;
  }
  if ( !added ) {
    AddTail( list, node );
  }
}

const struct ColumnInfo * GetSoundFontColumnInfos( VOID ) {

  return instrumentColumns;
}

const ULONG GetSoundFontColumnsWidth( VOID ) {

  LONG i = 0;
  ULONG result = 0;

  while( 0 < instrumentColumns[ i ].ci_Width ) {

    result += instrumentColumns[ i ].ci_Width;
    ++i;
  }
  LOG_V(( "V: Width over all columns is %ld\n", result ));
  return result;
}

VOID CreateEmptyListLabels( struct List * labels ) {

  LONG i = 0;

  while ( NULL != instrumentNames[ i ] ) {

    struct Node * label =
      CreateListBrowserNode( "  0",
                             instrumentNames[ i++ ],
                             instrumentNames[ i++ ],
                             "",
                             "",
                             "",
                             "" );
    AddTail( labels, label );
  }
}

VOID AddSf2Label(
  struct SF2 * sf2,
  struct List * labels,
  struct SF2_Preset * preset,
  struct SF2_Instrument * instrument,
  struct SF2_Sample * sample ) {

  UBYTE presetBank = preset->sf2p_Bank;
  UBYTE presetNumber = preset->sf2p_Common.sf2c_Number;
  CONST_STRPTR presetName = preset->sf2p_Common.sf2c_Name;
  CONST_STRPTR gmName = instrumentNames[ ( presetNumber << 1 ) + 1 ];
  CONST_STRPTR instrumentName = instrument->sf2i_Common.sf2c_Name;
  CONST_STRPTR sampleName = sample->sf2s_Name;

  UBYTE bank[ 4 ];
  UBYTE number[ 4 ];

  struct Node * label;

  sprintf( bank, "%3ld\0", presetBank );
  sprintf( number, "%3ld\0", presetNumber );
  label = CreateListBrowserNode( bank,
                                 number,
                                 gmName,
                                 presetName,
                                 instrumentName,
                                 sampleName,
                                 "" );
  OrderedInsertListBrowserNode( labels, label, bank, number );
}

VOID CreateSf2ListLabels(
  struct List * labels,
  struct SF2 * sf2 ) {

  ULONG progress = 0;
  ULONG maxProgress = sf2->sf2_PresetCount;/*sf2->sf2_SampleCount 
                    * sf2->sf2_InstrumentCount
                    * sf2->sf2_PresetCount;*/
  LONG countP = 0;
  LONG countO = 0;
  struct SF2_Preset * preset;
  FOR_LIST( &( sf2->sf2_Presets ),
            preset,
            struct SF2_Preset * ) {

    struct SF2_Zone * zoneP;

    FOR_LIST( &( preset->sf2p_Common.sf2c_Zones ),
              zoneP,
              struct SF2_Zone * ) {

      struct SF2_Generator * generatorP;
      FOR_LIST( &( zoneP->sfz2_Generators ),
                generatorP,
                struct SF2_Generator * ) {

        if ( GEN_INSTRUMENT == generatorP->sf2g_Id ) {

          struct SF2_Instrument * instrument = 
            sf2->sf2_InstrumentArray[ generatorP->sf2g_Amount ];
          struct SF2_Zone * zoneI;

          LOG_V(( "V: Found instrument %ld\n",
                  instrument->sf2i_Common.sf2c_Number ));

          FOR_LIST( &( instrument->sf2i_Common.sf2c_Zones ),
                    zoneI,
                    struct SF2_Zone * ) {

            struct SF2_Generator * generatorI;
            FOR_LIST( &( zoneI->sfz2_Generators ),
                      generatorI,
                      struct SF2_Generator * ) {

              if ( GEN_SAMPLEID == generatorI->sf2g_Id ) {

                struct SF2_Sample * sample =
                  sf2->sf2_SampleArray[ generatorI->sf2g_Amount ];
                LOG_V(( "V: Preset %ld-%ld instrument %ld sample %ld\n",
                        preset->sf2p_Bank,
                        preset->sf2p_Common.sf2c_Number,
                        generatorP->sf2g_Amount,
                        generatorI->sf2g_Amount ));

                LOG_V(( "V: Found sample %ld\n",
                        sample->sf2s_Number ));
                AddSf2Label( sf2, labels, preset, instrument, sample );
                ++countO;
                break;
              }
            }
          }
        }
      }
    }
    ++countP;
    ++progress;
    LOG_D(("D: Progress %ld of %ld\n", progress, maxProgress ));
  }
  LOG_I(( "I: %ld presets created %ld labels\n",
          countP, countO ));
}

VOID FreeListLabels( struct List * list ) {

  struct Node * label;
  while ( label = RemHead( list )) {

    FreeListBrowserNode( label );
  }
  LOG_D(( "V: List labels emptied.\n" ));
}
