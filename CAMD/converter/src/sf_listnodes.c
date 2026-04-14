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

#include <proto/alib.h>
#include <proto/exec.h>
#include <proto/listbrowser.h>

#include "sf_listnodes.h"

#include "converter.h"
#include "debug.h"
#include "support.h"

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
  LONG bankNumber = 0;
  UBYTE bankText[ 4 ];
  LONG columns = sizeof( instrumentColumns ) / sizeof( struct ColumnInfo );

  bankText[ 0 ] = ' ';
  bankText[ 1 ] = ' ';
  bankText[ 2 ] = '0';
  bankText[ 3 ] = 0;

  while ( NULL != instrumentNames[ i ] ) {
    
    struct Node * label = AllocListBrowserNode(
      columns,
      LBNA_Column, 0,
        LBNCA_CopyText, TRUE,
        LBNCA_Text, bankText,
        LBNCA_Justification, LCJ_CENTER,
      LBNA_Column, 1,
        LBNCA_CopyText, TRUE,
        LBNCA_Text, instrumentNames[ i++ ],
        LBNCA_Justification, LCJ_CENTER,
      LBNA_Column, 2,
        LBNCA_CopyText, TRUE,
        LBNCA_Text, instrumentNames[ i++ ],
      LBNA_Column, 3,
        LBNCA_CopyText, TRUE,
        LBNCA_Text, "",
      LBNA_Column, 4,
        LBNCA_CopyText, TRUE,
        LBNCA_Text, "",
      LBNA_Column, 5,
        LBNCA_CopyText, TRUE,
        LBNCA_Text, "",
      LBNA_Column, 6,
        LBNCA_CopyText, TRUE,
        LBNCA_Text, "",
      TAG_DONE
    );
    AddTail( labels, label );
  }
  //++bank;
}

VOID FreeListLabels( struct List * list ) {

  struct Node * label;
  FOR_LIST( list,
            label,
            struct Node * ) {

    FreeListBrowserNode( label );
  }
  LOG_D(( "V: List labels emptied.\n" ));
}
