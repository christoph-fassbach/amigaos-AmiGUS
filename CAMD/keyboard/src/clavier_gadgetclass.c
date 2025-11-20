/*
 * This file is part of the CAMD keyboard.
 *
 * CAMD keyboard is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3 of the License only.
 *
 * CAMD keyboard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CAMD keyboard.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <limits.h>

#include <clib/alib_protos.h>
#include <intuition/gadgetclass.h>

#include <proto/graphics.h>
#include <proto/intuition.h>

#include "clavier_gadgetclass.h"

// Key sizes in mm
#define BLACK_KEY_PEN            1
#define BLACK_KEY_LENGTH_MM    900  //originally:  900
#define BLACK_KEY_WIDTH_MM     200  //originally:  137
#define WHITE_KEY_PEN            2
#define WHITE_KEY_LENGTH_MM   1500  //originally: 1500
#define WHITE_KEY_WIDTH_MM     300  //originally:  235

// Conversion quotient to pixel
#define MM_TO_PIXEL_QUOTIENT    50

// Key sizes in pixel
#define BLACK_KEY_HEIGHT_PIXEL  ( BLACK_KEY_LENGTH_MM / MM_TO_PIXEL_QUOTIENT )
#define BLACK_KEY_WIDTH_PIXEL   ( BLACK_KEY_WIDTH_MM / MM_TO_PIXEL_QUOTIENT )
#define WHITE_KEY_HEIGHT_PIXEL  ( WHITE_KEY_LENGTH_MM / MM_TO_PIXEL_QUOTIENT )
#define WHITE_KEY_WIDTH_PIXEL   ( WHITE_KEY_WIDTH_MM / MM_TO_PIXEL_QUOTIENT )
#define KEY_GAP_HEIGHT_PIXEL    3
#define KEY_GAP_WIDTH_PIXEL     3


#define MAIN_KEYS_PER_OCTAVE     7
#define KEYS_PER_OCTAVE         12
#define MAX_MIDI_OCTAVES        11

typedef enum {
  WhiteLeft = 1,
  WhiteMid,
  WhiteRight,
  Black
} Key_Visual_Type;

struct Clavier_Key {

  LONG ck_TopY;
  LONG ck_SplitY;
  LONG ck_BottomY;
  LONG ck_TopLeftX;
  LONG ck_TopRightX;
  LONG ck_BottomLeftX;
  LONG ck_BottomRightX;

  ULONG ck_pen;
  BYTE ck_name[8];
};

static VOID getClavierKeyProperties( struct Clavier_Key * key,
                                     BYTE midiNote,
                                     struct Gadget * gadget ) {

  WORD note = midiNote % KEYS_PER_OCTAVE;
  WORD octave = ( midiNote / KEYS_PER_OCTAVE ) - 1;
  WORD nameIndex = 0;
  Key_Visual_Type keyType;
  WORD keyBase;

  WORD topY = 0;
  WORD splitY = ( BLACK_KEY_HEIGHT_PIXEL * gadget->Height )
                / WHITE_KEY_HEIGHT_PIXEL;
  WORD bottomY = gadget->Height;
  WORD whiteKeyWidth = (( WHITE_KEY_WIDTH_PIXEL * gadget->Height )
                       / WHITE_KEY_HEIGHT_PIXEL )
                       + 2;
  WORD blackKeyWidth = ( BLACK_KEY_WIDTH_PIXEL * gadget->Height )
                       / WHITE_KEY_HEIGHT_PIXEL;
  WORD blackKeyWidth_2 = blackKeyWidth >> 1;

//  Printf("whiteKeyWidth = %ld\n", whiteKeyWidth);
//  Printf("blackKeyWidth = %ld\n", blackKeyWidth);

  switch ( note ) {
    case 0: {
      keyType = WhiteLeft;
      keyBase = 0;
      key->ck_name[ nameIndex++ ] = 'C';
      break;
    }
    case 1: {
      keyType = Black;
      keyBase = 1;
      key->ck_name[ nameIndex++ ] = 'C';
      key->ck_name[ nameIndex++ ] = '#';
      break;
    }
    case 2: {
      keyType = WhiteMid;
      keyBase = 1;
      key->ck_name[ nameIndex++ ] = 'D';
      break;
    }
    case 3: {
      keyType = Black;
      keyBase = 2;
      key->ck_name[ nameIndex++ ] = 'D';
      key->ck_name[ nameIndex++ ] = '#';
      break;
    }
    case 4: {
      keyType = WhiteRight;
      keyBase = 2;
      key->ck_name[ nameIndex++ ] = 'E';
      break;
    }
    case 5: {
      keyType = WhiteLeft;
      keyBase = 3;
      key->ck_name[ nameIndex++ ] = 'F';
      break;
    }
    case 6: {
      keyType = Black;
      keyBase = 4;
      key->ck_name[ nameIndex++ ] = 'F';
      key->ck_name[ nameIndex++ ] = '#';
      break;
    }
    case 7: {
      keyType = WhiteMid;
      keyBase = 4;
      key->ck_name[ nameIndex++ ] = 'G';
      break;
    }
    case 8: {
      keyType = Black;
      keyBase = 5;
      key->ck_name[ nameIndex++ ] = 'G';
      key->ck_name[ nameIndex++ ] = '#';
      break;
    }
    case 9: {
      keyType = WhiteMid;
      keyBase = 5;
      key->ck_name[ nameIndex++ ] = 'A';
      break;
    }
    case 10: {
      keyType = Black;
      keyBase = 6;
      key->ck_name[ nameIndex++ ] = 'A';
      key->ck_name[ nameIndex++ ] = '#';
      break;
    }
    case 11:
    default: {
      keyBase = 6;
      keyType = WhiteRight;
      key->ck_name[ nameIndex++ ] = 'B';
      break;
    }
  }

  if ( octave < 0 ) {

    key->ck_name[ nameIndex++ ] = '-';
    key->ck_name[ nameIndex++ ] = '1';

  } else {

    key->ck_name[ nameIndex++ ] = '0' + octave;
  }
  key->ck_name[ nameIndex++ ] = 0;

  keyBase += ( 1 + octave ) * MAIN_KEYS_PER_OCTAVE;

  switch ( keyType ) {
    case WhiteLeft: {
      key->ck_TopY = topY;
      key->ck_SplitY = splitY + KEY_GAP_HEIGHT_PIXEL;
      key->ck_BottomY = bottomY;
      key->ck_TopLeftX = whiteKeyWidth * keyBase;
      key->ck_BottomLeftX = key->ck_TopLeftX;
      key->ck_BottomRightX = key->ck_BottomLeftX + whiteKeyWidth - KEY_GAP_WIDTH_PIXEL;
      key->ck_TopRightX = key->ck_BottomRightX - blackKeyWidth_2;
      key->ck_pen = WHITE_KEY_PEN;
      break;
    }
    case WhiteMid: {
      key->ck_TopY = topY;
      key->ck_SplitY = splitY + KEY_GAP_HEIGHT_PIXEL;
      key->ck_BottomY = bottomY;
      key->ck_BottomLeftX = whiteKeyWidth * keyBase;
      key->ck_TopLeftX = key->ck_BottomLeftX + blackKeyWidth_2;
      key->ck_BottomRightX = key->ck_BottomLeftX + whiteKeyWidth - KEY_GAP_WIDTH_PIXEL;
      key->ck_TopRightX = key->ck_BottomRightX - blackKeyWidth_2;
      key->ck_pen = WHITE_KEY_PEN;
      break;
    }
    case WhiteRight: {
      key->ck_TopY = topY;
      key->ck_SplitY = splitY + KEY_GAP_HEIGHT_PIXEL;
      key->ck_BottomY = bottomY;
      key->ck_BottomLeftX = whiteKeyWidth * keyBase;
      key->ck_TopLeftX = key->ck_BottomLeftX + blackKeyWidth_2;
      key->ck_BottomRightX = key->ck_BottomLeftX + whiteKeyWidth - KEY_GAP_WIDTH_PIXEL;
      key->ck_TopRightX = key->ck_BottomRightX;
      key->ck_pen = WHITE_KEY_PEN;
      break;
    }
    case Black: {
      key->ck_TopY = topY;
      key->ck_SplitY = splitY;
      key->ck_BottomY = -1;
      key->ck_TopLeftX = ( whiteKeyWidth * keyBase ) - blackKeyWidth_2;
      key->ck_TopRightX = key->ck_TopLeftX + blackKeyWidth - KEY_GAP_WIDTH_PIXEL;
      key->ck_BottomLeftX = -1;
      key->ck_BottomRightX = -1;
      key->ck_pen = BLACK_KEY_PEN;
      break;
    }
    default: {
      key->ck_TopY = -1;
      key->ck_SplitY = -1;
      key->ck_BottomY = -1;
      key->ck_TopLeftX = -1;
      key->ck_TopRightX = -1;
      key->ck_BottomLeftX = -1;
      key->ck_BottomRightX = -1;
      key->ck_pen = 0;
      break;
    }
  }
}

static ULONG Handle_OM_NEW( Class * class,
                            struct Gadget * gadget,
                            struct opSet * message ) {

  gadget = ( struct Gadget * ) DoSuperMethodA( class,
                                               ( Object * ) gadget,
                                               ( Msg ) message );
  if ( gadget ) {

    struct Clavier_Gadget_Data * data = INST_DATA( class, gadget );
    // TODO: init data fields here!
  }

  return ( ULONG ) gadget;
}

static ULONG Handle_GM_HITTEST( Class * class,
                                struct Gadget * gadget,
                                struct gpHitTest *message ) {

  return GMR_GADGETHIT;
  // return GMR_GADGETNOTHIT;
}

static ULONG Handle_GM_RENDER( Class * class,
                               struct Gadget * gadget,
                               struct gpRender * message ) {

  struct RastPort * rastPort = message->gpr_RPort;
  WORD midiNote;

  for ( midiNote = 0; midiNote < 12; ++midiNote ) {
    struct Clavier_Key key;
    getClavierKeyProperties( &key, midiNote, gadget );
    /*
    Printf("%ld top (%ld, %ld) -> (%ld, %ld)\n",
      midiNote,
       key.ck_TopLeftX,
              key.ck_TopY,
              key.ck_TopRightX,
              key.ck_SplitY);*/
    SetAPen( rastPort, key.ck_pen );
    RectFill( rastPort,
              gadget->LeftEdge + key.ck_TopLeftX,
              gadget->TopEdge + key.ck_TopY,
              gadget->LeftEdge + key.ck_TopRightX,
              gadget->TopEdge + key.ck_SplitY);
    if ( 0 < key.ck_BottomY ) {
      /*
          Printf("%ld bottom (%ld, %ld) -> (%ld, %ld)\n",
            midiNote,
             key.ck_BottomLeftX,
                key.ck_SplitY,
                key.ck_BottomRightX,
                key.ck_BottomY);
*/
      RectFill( rastPort,
                gadget->LeftEdge + key.ck_BottomLeftX,
                gadget->TopEdge + key.ck_SplitY,
                gadget->LeftEdge + key.ck_BottomRightX,
                gadget->TopEdge + key.ck_BottomY );
    }
  }
  return 0;
}

static ULONG Handle_GM_GOACTIVE( Class * class,
                                 struct Gadget * gadget,
                                 struct gpInput * message ) {
  return GMR_MEACTIVE;
  // return GMR_REUSE;
}

static ULONG Handle_GM_HANDLEINPUT( Class * class,
                                 struct Gadget * gadget,
                                 struct gpInput * message ) {

	ULONG result = DoSuperMethodA( class,
                                 ( Object * ) gadget,
                                 ( Msg ) message );
  return result;
}

static ULONG Handle_GM_GOINACTIVE( Class * class,
                                 struct Gadget * gadget,
                                 struct gpGoInactive * message ) {

  return 0;
}

static ULONG Handle_GM_Domain( Class * class,
                               struct gpDomain * message ) {

  switch ( message->gpd_Which ) {

    case GDOMAIN_MINIMUM: {
      message->gpd_Domain.Left = 0;
      message->gpd_Domain.Top = 0;
      message->gpd_Domain.Width = WHITE_KEY_WIDTH_PIXEL * MAIN_KEYS_PER_OCTAVE;
      message->gpd_Domain.Height = WHITE_KEY_HEIGHT_PIXEL;
      return 1;
    }
    case GDOMAIN_NOMINAL: {

      message->gpd_Domain.Left = 0;
      message->gpd_Domain.Top = 0;
      message->gpd_Domain.Width = WHITE_KEY_WIDTH_PIXEL
                                  * MAIN_KEYS_PER_OCTAVE
                                  * MAX_MIDI_OCTAVES;
      message->gpd_Domain.Height = WHITE_KEY_HEIGHT_PIXEL;
      return 1;
    }
    case GDOMAIN_MAXIMUM: {

      message->gpd_Domain.Left = 0;
      message->gpd_Domain.Top = 0;
      message->gpd_Domain.Width = SHRT_MAX;
      message->gpd_Domain.Height = ( SHRT_MAX / (WHITE_KEY_WIDTH_PIXEL * MAIN_KEYS_PER_OCTAVE * MAX_MIDI_OCTAVES))
                                   * WHITE_KEY_HEIGHT_PIXEL;
      return 1;
    }
    default: {
      return 0;
    }
  }
}

__saveds ULONG DispatchClavierGadgetClass(
  Class * class,
  struct Gadget * gadget,
  Msg message ) {

  ULONG result = 0;
  switch ( message->MethodID ) {
    case OM_NEW: {

      result = Handle_OM_NEW( class,
                              gadget,
                              ( struct opSet * ) message );
      break;
    }
    case GM_HITTEST: {

      result = Handle_GM_HITTEST( class,
                                  gadget,
                                  ( struct gpHitTest * ) message );
      break;
    }
    case GM_RENDER: {

      result = Handle_GM_RENDER( class,
                                 gadget,
                                 ( struct gpRender * ) message );
      break;
    }
    case GM_GOACTIVE: {

      result = Handle_GM_GOACTIVE( class,
                                   gadget,
                                   ( struct gpInput * ) message );
      break;
    }
    case GM_HANDLEINPUT: {

      result = Handle_GM_HANDLEINPUT( class,
                                      gadget,
                                      ( struct gpInput * ) message );
      break;
    }
    case GM_GOINACTIVE: {

      result = Handle_GM_GOINACTIVE( class,
                                     gadget,
                                     ( struct gpGoInactive * ) message );
      break;
    }
    case GM_DOMAIN: {

      result = Handle_GM_Domain( class,
                                 ( struct gpDomain * ) message );
      break;
    }
    default: {

      result = DoSuperMethodA( class, ( Object * ) gadget, message );
      break;
    }
  }
  return result;
}

Class * InitClavierGadgetClass( VOID ) {

  Class * class = MakeClass(
    CLAVIERGCLASS,
    BUTTONGCLASS,
    NULL, 
    sizeof( struct Clavier_Gadget_Data ),
    0 );
  if ( class ) {

    struct Hook * hook = &( class->cl_Dispatcher );
    hook->h_Entry = ( HOOKFUNC ) HookEntry;
	  hook->h_SubEntry = ( HOOKFUNC ) DispatchClavierGadgetClass;
  }

  return class;
}