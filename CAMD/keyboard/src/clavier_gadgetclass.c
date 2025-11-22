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

struct Clavier_KeyBorders {

  LONG ckb_TopY;
  LONG ckb_SplitY;
  LONG ckb_BottomY;
  LONG ckb_TopLeftX;
  LONG ckb_TopRightX;
  LONG ckb_BottomLeftX;
  LONG ckb_BottomRightX;

  ULONG ckb_pen;
  BYTE ckb_name[8];
};

static WORD getClavierWhiteKeyWidth( struct Gadget * gadget ) {

  WORD result = (( WHITE_KEY_WIDTH_PIXEL * gadget->Height )
                / WHITE_KEY_HEIGHT_PIXEL )
                + 2;
  return result;
}

static WORD getClavierWhiteKeyHeight( struct Gadget * gadget ) {

  WORD result = gadget->Height;
  return result;
}

static WORD getClavierBlackKeyWidth( struct Gadget * gadget ) {

  WORD result = ( BLACK_KEY_WIDTH_PIXEL * gadget->Height )
                / WHITE_KEY_HEIGHT_PIXEL;
  return result;
}

static WORD getClavierBlackKeyHeight( struct Gadget * gadget ) {

  WORD result = ( BLACK_KEY_HEIGHT_PIXEL * gadget->Height )
                / WHITE_KEY_HEIGHT_PIXEL;
  return result;
}

static VOID getClavierKeyProperties( struct Clavier_KeyBorders * key,
                                     BYTE midiNote,
                                     struct Gadget * gadget ) {

  WORD note = midiNote % KEYS_PER_OCTAVE;
  WORD octave = ( midiNote / KEYS_PER_OCTAVE ) - 1;
  WORD nameIndex = 0;
  Key_Visual_Type keyType;
  WORD keyBase;

  WORD whiteKeyWidth = getClavierWhiteKeyWidth( gadget );
  WORD blackKeyWidth = getClavierBlackKeyWidth( gadget );
  WORD blackKeyWidth_2 = blackKeyWidth >> 1;

  switch ( note ) {
    case 0: {
      keyType = WhiteLeft;
      keyBase = 0;
      key->ckb_name[ nameIndex++ ] = 'C';
      break;
    }
    case 1: {
      keyType = Black;
      keyBase = 1;
      key->ckb_name[ nameIndex++ ] = 'C';
      key->ckb_name[ nameIndex++ ] = '#';
      break;
    }
    case 2: {
      keyType = WhiteMid;
      keyBase = 1;
      key->ckb_name[ nameIndex++ ] = 'D';
      break;
    }
    case 3: {
      keyType = Black;
      keyBase = 2;
      key->ckb_name[ nameIndex++ ] = 'D';
      key->ckb_name[ nameIndex++ ] = '#';
      break;
    }
    case 4: {
      keyType = WhiteRight;
      keyBase = 2;
      key->ckb_name[ nameIndex++ ] = 'E';
      break;
    }
    case 5: {
      keyType = WhiteLeft;
      keyBase = 3;
      key->ckb_name[ nameIndex++ ] = 'F';
      break;
    }
    case 6: {
      keyType = Black;
      keyBase = 4;
      key->ckb_name[ nameIndex++ ] = 'F';
      key->ckb_name[ nameIndex++ ] = '#';
      break;
    }
    case 7: {
      keyType = WhiteMid;
      keyBase = 4;
      key->ckb_name[ nameIndex++ ] = 'G';
      break;
    }
    case 8: {
      keyType = Black;
      keyBase = 5;
      key->ckb_name[ nameIndex++ ] = 'G';
      key->ckb_name[ nameIndex++ ] = '#';
      break;
    }
    case 9: {
      keyType = WhiteMid;
      keyBase = 5;
      key->ckb_name[ nameIndex++ ] = 'A';
      break;
    }
    case 10: {
      keyType = Black;
      keyBase = 6;
      key->ckb_name[ nameIndex++ ] = 'A';
      key->ckb_name[ nameIndex++ ] = '#';
      break;
    }
    case 11:
    default: {
      keyBase = 6;
      keyType = WhiteRight;
      key->ckb_name[ nameIndex++ ] = 'B';
      break;
    }
  }

  if ( octave < 0 ) {

    key->ckb_name[ nameIndex++ ] = '-';
    key->ckb_name[ nameIndex++ ] = '1';

  } else {

    key->ckb_name[ nameIndex++ ] = '0' + octave;
  }
  key->ckb_name[ nameIndex ] = 0;

  keyBase += ( 1 + octave ) * MAIN_KEYS_PER_OCTAVE;

  key->ckb_TopY = 0;
  key->ckb_SplitY = getClavierBlackKeyHeight( gadget );
  key->ckb_BottomY = getClavierWhiteKeyHeight( gadget );
  key->ckb_TopLeftX = whiteKeyWidth * keyBase;
  key->ckb_TopRightX = key->ckb_TopLeftX
                       + whiteKeyWidth
                       - KEY_GAP_WIDTH_PIXEL;
  key->ckb_BottomLeftX = key->ckb_TopLeftX;
  key->ckb_BottomRightX = key->ckb_TopRightX;
  
  switch ( keyType ) {
    case WhiteLeft: {
      key->ckb_SplitY += KEY_GAP_HEIGHT_PIXEL;
      key->ckb_TopRightX -= blackKeyWidth_2;
      key->ckb_pen = WHITE_KEY_PEN;
      break;
    }
    case WhiteMid: {
      key->ckb_SplitY += KEY_GAP_HEIGHT_PIXEL;
      key->ckb_TopLeftX += blackKeyWidth_2;
      key->ckb_TopRightX -= blackKeyWidth_2;
      key->ckb_pen = WHITE_KEY_PEN;
      break;
    }
    case WhiteRight: {
      key->ckb_SplitY += KEY_GAP_HEIGHT_PIXEL;
      key->ckb_TopLeftX += blackKeyWidth_2;
      key->ckb_pen = WHITE_KEY_PEN;
      break;
    }
    case Black: {
      key->ckb_BottomY = -1;
      key->ckb_TopLeftX -=  blackKeyWidth_2;
      key->ckb_TopRightX = key->ckb_TopLeftX
                           + blackKeyWidth
                           - KEY_GAP_WIDTH_PIXEL;
      key->ckb_pen = BLACK_KEY_PEN;
      break;
    }
    default: {
      key->ckb_TopY = -1;
      key->ckb_SplitY = -1;
      key->ckb_BottomY = -1;
      key->ckb_TopLeftX = -1;
      key->ckb_TopRightX = -1;
      key->ckb_BottomLeftX = -1;
      key->ckb_BottomRightX = -1;
      key->ckb_pen = 0;
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
    data->cgd_NoteHit = -1;
    // TODO: init data fields here!
  }

  return ( ULONG ) gadget;
}

static ULONG Handle_GM_HITTEST( Class * class,
                                struct Gadget * gadget,
                                struct gpHitTest * message ) {

  const UWORD hitX = message->gpht_Mouse.X;
  const UWORD hitY = message->gpht_Mouse.Y;
  const WORD keyWidth = getClavierWhiteKeyWidth( gadget );
  const WORD candidate = ( hitX * KEYS_PER_OCTAVE ) / 
                         ( keyWidth * MAIN_KEYS_PER_OCTAVE );
  WORD i;
  struct Clavier_KeyBorders key;
  struct Clavier_Gadget_Data * data = INST_DATA( class, gadget );

  // Not checking all, cause of 
  for ( i = candidate - 2; i <= candidate + 2; ++i ) {
  //  for ( i = 0; i < 128; ++i ) {
    getClavierKeyProperties( &key, i, gadget );

    if ((( key.ckb_BottomLeftX <= hitX ) &&
         ( key.ckb_BottomRightX >= hitX ) &&
         ( key.ckb_SplitY <= hitY ) && 
         ( key.ckb_BottomY >= hitY )) ||
        (( key.ckb_TopLeftX <= hitX ) &&
         ( key.ckb_TopRightX >= hitX ) &&
         ( key.ckb_TopY <= hitY ) && 
         ( key.ckb_SplitY >= hitY ))
       ) {

      data->cgd_NoteHit = i;
      return GMR_GADGETHIT;
    }
  }

  data->cgd_NoteHit = 1000 + candidate; //-1;  
  return GMR_GADGETHIT; //GMR_GADGETNOTHIT;
}

static ULONG Handle_GM_RENDER( Class * class,
                               struct Gadget * gadget,
                               struct gpRender * message ) {

  struct RastPort * rastPort = message->gpr_RPort;
  WORD midiNote;

  for ( midiNote = 0; midiNote < 64; ++midiNote ) {
    struct Clavier_KeyBorders key;
    getClavierKeyProperties( &key, midiNote, gadget );
    /*
    Printf("%ld top (%ld, %ld) -> (%ld, %ld)\n",
      midiNote,
       key.ckb_TopLeftX,
              key.ckb_TopY,
              key.ckb_TopRightX,
              key.ckb_SplitY);*/
    SetAPen( rastPort, key.ckb_pen );
    RectFill( rastPort,
              gadget->LeftEdge + key.ckb_TopLeftX,
              gadget->TopEdge + key.ckb_TopY,
              gadget->LeftEdge + key.ckb_TopRightX,
              gadget->TopEdge + key.ckb_SplitY);
    if ( 0 < key.ckb_BottomY ) {
      /*
          Printf("%ld bottom (%ld, %ld) -> (%ld, %ld)\n",
            midiNote,
             key.ckb_BottomLeftX,
                key.ckb_SplitY,
                key.ckb_BottomRightX,
                key.ckb_BottomY);
*/
      RectFill( rastPort,
                gadget->LeftEdge + key.ckb_BottomLeftX,
                gadget->TopEdge + key.ckb_SplitY,
                gadget->LeftEdge + key.ckb_BottomRightX,
                gadget->TopEdge + key.ckb_BottomY );
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
  // Button-like clavier was hit,
  // so fetch which sub-key it was and put back to message for window.
  if ( GMR_VERIFY & result ) {

    struct Clavier_Gadget_Data * data = INST_DATA( class, gadget );
    *( message->gpi_Termination ) = data->cgd_NoteHit;
  }
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