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

static const LONG instrumentNumbers[] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
   10,
   11,
   12,
   13,
   14,
   15,
   16,
   17,
   18,
   19,
   20,
   21,
   22,
   23,
   24,
   25,
   26,
   27,
   28,
   29,
   30,
   31,
   32,
   33,
   34,
   35,
   36,
   37,
   38,
   39,
   40,
   41,
   42,
   43,
   44,
   45,
   46,
   47,
   48,
   49,
   50,
   51,
   52,
   53,
   54,
   55,
   56,
   57,
   58,
   59,
   60,
   61,
   62,
   63,
   64,
   65,
   66,
   67,
   68,
   69,
   70,
   71,
   72,
   73,
   74,
   75,
   76,
   77,
   78,
   79,
   80,
   81,
   82,
   83,
   84,
   85,
   86,
   87,
   88,
   89,
   90,
   91,
   92,
   93,
   94,
   95,
   96,
   97,
   98,
   99,
  100,
  101,
  102,
  103,
  104,
  105,
  106,
  107,
  108,
  109,
  110,
  111,
  112,
  113,
  114,
  115,
  116,
  117,
  118,
  119,
  120,
  121,
  122,
  123,
  124,
  125,
  126,
  127,
  -1
};

static const UBYTE * instrumentNames[] = {
  "Acoustic Grand Piano",
  "Bright Acoustic Piano",
  "Electric Grand Piano",
  "Honky-tonk Piano",
  "Electric Piano 1",
  "Electric Piano 2",
  "Harpsichord",
  "Clavinet",
  "Celesta",
  "Glockenspiel",
  "Music Box",
  "Vibraphone",
  "Marimba",
  "Xylophone",
  "Tubular Bells",
  "Dulcimer",
  "Drawbar Organ",
  "Percussive Organ",
  "Rock Organ",
  "Church Organ",
  "Reed Organ",
  "Accordion",
  "Harmonica",
  "Bandoneon",
  "Acoustic Guitar (nylon)",
  "Acoustic Guitar (steel)",
  "Electric Guitar (jazz)",
  "Electric Guitar",
  "Electric Guitar (muted)",
  "Overdriven Guitar",
  "Distortion Guitar",
  "Guitar Harmonics",
  "Acoustic Bass",
  "Electric Bass (finger)",
  "Electric Bass (picked)",
  "Fretless Bass",
  "Slap Bass 1",
  "Slap Bass 2",
  "Synth Bass 1",
  "Synth Bass 2",
  "Violin",
  "Viola",
  "Cello",
  "Contrabass",
  "Tremolo Strings",
  "Pizzicato Strings",
  "Orchestral Harp",
  "Timpani",
  "String Ensemble 1",
  "String Ensemble 2",
  "Synth Strings 1",
  "Synth Strings 2",
  "Choir Aahs",
  "Voice Oohs",
  "Synth Voice",
  "Orchestra Hit",
  "Trumpet",
  "Trombone",
  "Tuba",
  "Muted Trumpet",
  "French Horn",
  "Brass Section",
  "Synth Brass 1",
  "Synth Brass 2",
  "Soprano Sax",
  "Alto Sax",
  "Tenor Sax",
  "Baritone Sax",
  "Oboe",
  "English Horn",
  "Bassoon",
  "Clarinet",
  "Piccolo",
  "Flute",
  "Recorder",
  "Pan Flute",
  "Blown bottle",
  "Shakuhachi",
  "Whistle",
  "Ocarina",
  "Lead 1",
  "Lead 2",
  "Lead 3",
  "Lead 4",
  "Lead 5",
  "Lead 6",
  "Lead 7",
  "Lead 8",
  "Pad 1",
  "Pad 2",
  "Pad 3",
  "Pad 4",
  "Pad 5",
  "Pad 6",
  "Pad 7",
  "Pad 8",
  "FX 1",
  "FX 2",
  "FX 3",
  "FX 4",
  "FX 5",
  "FX 6",
  "FX 7",
  "FX 8",
  "Sitar",
  "Banjo",
  "Shamisen",
  "Koto",
  "Kalimba",
  "Bag pipe",
  "Fiddle",
  "Shanai",
  "Tinkle Bell",
  "Cowbell",
  "Steel Drums",
  "Woodblock",
  "Taiko Drum",
  "Melodic Tom",
  "Synth Drum",
  "Reverse Cymbal",
  "Guitar Fret Noise",
  "Breath Noise",
  "Seashore",
  "Bird Tweet",
  "Telephone Ring",
  "Helicopter",
  "Applause",
  "Gunshot",
  NULL
};

struct Node * CreateListBrowserNode( const LONG * integer0,
                                     const LONG * integer1,
                                     CONST_STRPTR string2,
                                     CONST_STRPTR string3,
                                     CONST_STRPTR string4,
                                     CONST_STRPTR string5,
                                     CONST_STRPTR string6 ) {

  LONG columns = sizeof( instrumentColumns ) / sizeof( struct ColumnInfo );
  return AllocListBrowserNode( columns,
                               LBNA_Column, 0,
                                 LBNCA_Integer, integer0,
                                 LBNCA_Justification, LCJ_RIGHT,
                               LBNA_Column, 1,
                                 LBNCA_Integer, integer1,
                                 LBNCA_Justification, LCJ_RIGHT,
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
                                 LBNCA_CopyText, TRUE,
                                 LBNCA_Text, string6,
                               TAG_DONE );
}

VOID OrderedInsertListBrowserNode( struct List * list,
                                   struct Node * node,
                                   LONG bank,
                                   LONG number ) {

  struct Node * next;
  struct Node * previous = NULL;

  LONG * nextBank = NULL;
  LONG * nextNumber = NULL;
  BOOL added = FALSE;

  LOG_V(( "V: Inserting \n" ));
  FOR_LIST ( list, next, struct Node * ) {
    GetListBrowserNodeAttrs( next,
                             LBNA_Column, 0,
                               LBNCA_Integer, &nextBank,
                             LBNA_Column, 1,
                               LBNCA_Integer, &nextNumber,
                             TAG_END );
    if (( *nextBank > bank ) ||
        (( *nextBank == bank ) && ( *nextNumber > number ))) {

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
      CreateListBrowserNode( &( instrumentNumbers[ 0 ]),
                             &( instrumentNumbers[ i ]),
                             instrumentNames[ i ],
                             "",
                             "",
                             "",
                             "" );
    i++;
    AddTail( labels, label );
  }
}

VOID AddSf2Label(
  struct SF2 * sf2,
  struct List * labels,
  struct SF2_Preset * preset,
  struct SF2_Instrument * instrument,
  struct SF2_Sample * sample ) {

  LONG * bank = &( preset->sf2p_Bank );
  LONG * number = &( preset->sf2p_Common.sf2c_Number );
  CONST_STRPTR presetName = preset->sf2p_Common.sf2c_Name;
  CONST_STRPTR gmName = instrumentNames[ *number ];
  CONST_STRPTR instrumentName = instrument->sf2i_Common.sf2c_Name;
  CONST_STRPTR sampleName = sample->sf2s_Name;

  struct Node * label;
  LOG_V(( "V: Creating label %ld %ld %s %s %s %s\n",
          *bank, *number, presetName, gmName, instrumentName, sampleName ));
  label = CreateListBrowserNode( bank,
                                 number,
                                 gmName,
                                 presetName,
                                 instrumentName,
                                 sampleName,
                                 "" );
  LOG_V(( "V: Inserting label\n" ));
  OrderedInsertListBrowserNode( labels, label, *bank, *number );
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
      LOG_V(( "V: Preset %ld-%ld\n",
              preset->sf2p_Bank,
              preset->sf2p_Common.sf2c_Number ));
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
