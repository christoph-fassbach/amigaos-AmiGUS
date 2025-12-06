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


#define MAX( a, b ) (( a < b ) ? b : a )
#define MIN( a, b ) (( a > b ) ? b : a )

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

static WORD getClavierWhiteKeyWidthByHeight( WORD height ) {

  WORD result = (( WHITE_KEY_WIDTH_PIXEL * height )
                / WHITE_KEY_HEIGHT_PIXEL )
                + KEY_GAP_WIDTH_PIXEL;
  return result;
}

static WORD getClavierWhiteKeyWidth( struct Gadget * gadget ) {

  WORD result = getClavierWhiteKeyWidthByHeight( gadget->Height );
  return result;
}

static WORD getClavierWhiteKeyHeight( struct Gadget * gadget ) {

  WORD result = gadget->Height - 1;
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

static VOID getClavierKeyProperties( struct Clavier_Key * key,
                                     Class * class,
                                     struct Gadget * gadget,
                                     BYTE midiNote ) {

  const WORD note = midiNote % KEYS_PER_OCTAVE;
  const WORD octave = ( midiNote / KEYS_PER_OCTAVE ) - 1;

  const struct Clavier_Gadget_Data * data = INST_DATA( class, gadget );
  const WORD offset = data->cgd_OffsetX;

  const WORD whiteKeyWidth = getClavierWhiteKeyWidth( gadget );
  const WORD blackKeyWidth = getClavierBlackKeyWidth( gadget );
  const WORD blackKeyWidth_2 = blackKeyWidth >> 1;

  WORD nameIndex = 0;
  Key_Visual_Type keyType;
  WORD keyBase;

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
  key->ck_name[ nameIndex ] = 0;

  keyBase += ( 1 + octave ) * MAIN_KEYS_PER_OCTAVE;

  key->ck_TopY = 0;
  key->ck_SplitY = getClavierBlackKeyHeight( gadget );
  key->ck_BottomY = getClavierWhiteKeyHeight( gadget );
  key->ck_TopLeftX = ( whiteKeyWidth * keyBase ) - offset;
  key->ck_TopRightX = key->ck_TopLeftX
                      + whiteKeyWidth
                      - KEY_GAP_WIDTH_PIXEL;
  key->ck_BottomLeftX = key->ck_TopLeftX;
  key->ck_BottomRightX = key->ck_TopRightX;
  
  switch ( keyType ) {
    case WhiteLeft: {
      key->ck_SplitY += KEY_GAP_HEIGHT_PIXEL;
      key->ck_TopRightX -= blackKeyWidth_2;
      key->ck_pen = WHITE_KEY_PEN;
      break;
    }
    case WhiteMid: {
      key->ck_SplitY += KEY_GAP_HEIGHT_PIXEL;
      key->ck_TopLeftX += blackKeyWidth_2;
      key->ck_TopRightX -= blackKeyWidth_2;
      key->ck_pen = WHITE_KEY_PEN;
      break;
    }
    case WhiteRight: {
      key->ck_SplitY += KEY_GAP_HEIGHT_PIXEL;
      key->ck_TopLeftX += blackKeyWidth_2;
      key->ck_pen = WHITE_KEY_PEN;
      break;
    }
    case Black: {
      key->ck_BottomY = -1;
      key->ck_TopLeftX -=  blackKeyWidth_2;
      key->ck_TopRightX = key->ck_TopLeftX
                           + blackKeyWidth
                           - KEY_GAP_WIDTH_PIXEL;
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
    data->cgd_OffsetX = 50;
    data->cgd_NoteHit = -1;
  }

  return ( ULONG ) gadget;
}

static ULONG Handle_GM_HITTEST( Class * class,
                                struct Gadget * gadget,
                                struct gpHitTest * message ) {

  const UWORD hitX = message->gpht_Mouse.X;
  const UWORD hitY = message->gpht_Mouse.Y;
  const WORD keyWidth = getClavierWhiteKeyWidth( gadget );
#define LIMIT_HIT_TEST
#ifdef LIMIT_HIT_TEST

  struct Clavier_Gadget_Data * data = INST_DATA( class, gadget );
  const WORD offset = data->cgd_OffsetX;
  const WORD candidate = (( hitX + offset ) * KEYS_PER_OCTAVE ) / 
                         ( keyWidth * MAIN_KEYS_PER_OCTAVE );

#endif
  WORD i;
  struct Clavier_Key key;

#ifdef LIMIT_HIT_TEST

  // Not checking all, cause of speed
  for ( i = candidate - 2; i <= candidate + 2; ++i ) {

#else

  for ( i = 0; i < 128; ++i ) {

#endif

    getClavierKeyProperties( &key, class, gadget, i );

    if ((( key.ck_BottomLeftX <= hitX ) &&
         ( key.ck_BottomRightX >= hitX ) &&
         ( key.ck_SplitY <= hitY ) && 
         ( key.ck_BottomY >= hitY )) ||
        (( key.ck_TopLeftX <= hitX ) &&
         ( key.ck_TopRightX >= hitX ) &&
         ( key.ck_TopY <= hitY ) && 
         ( key.ck_SplitY >= hitY ))
       ) {

      data->cgd_NoteHit = i;
      return GMR_GADGETHIT;
    }
  }
#if 0
  data->cgd_NoteHit = 1000 + candidate;
  return GMR_GADGETHIT; 
#endif
  data->cgd_NoteHit = -1;  
  return GMR_GADGETNOTHIT;
}

static ULONG Handle_GM_RENDER( Class * class,
                               struct Gadget * gadget,
                               struct gpRender * message ) {

  struct Clavier_Gadget_Data * data = INST_DATA( class, gadget );
  struct RastPort * rastPort = message->gpr_RPort;
  WORD midiNote;
  BOOL done = FALSE;

  for ( midiNote = 0; midiNote < 128; ++midiNote ) {

    struct Clavier_Key key;
    ULONG pen;
    const WORD top = gadget->TopEdge;
    const WORD left = gadget->LeftEdge;
    const WORD width = gadget->Width - 1;

    getClavierKeyProperties( &key, class, gadget, midiNote );

    if (( midiNote == data->cgd_NoteHit ) &&
        ( GFLG_SELECTED & gadget->Flags )) {

      pen = 3;

    } else {

      pen = key.ck_pen;
    }
    SetAPen( rastPort, pen );

    if ( width < key.ck_TopLeftX ) {

      // Left border of key is outside window - abort!!!
      break;
    }
    if (( 0 > key.ck_BottomRightX ) && ( 0 > key.ck_TopRightX )) {

      // Right border ok key is outside window - skip this one!
      continue;
    }
    RectFill( rastPort,
              left + MAX( key.ck_TopLeftX, 0 ),
              top + key.ck_TopY,
              left + MIN( key.ck_TopRightX, width ),
              top + key.ck_SplitY);

    if ( 0 < key.ck_BottomY ) {

      RectFill( rastPort,
                left + MAX( key.ck_BottomLeftX, 0 ),
                top + key.ck_SplitY,
                left + MIN( key.ck_BottomRightX, width ),
                top + key.ck_BottomY );
    }
  }
  return 0;
}

static ULONG Handle_GM_GOACTIVE( Class * class,
                                 struct Gadget * gadget,
                                 struct gpInput * message ) {

  struct Clavier_Gadget_Data * data = INST_DATA( class, gadget );
  if ( data->cgd_NoteHit >= 0 ) {

    return GMR_MEACTIVE;
  }
  data->cgd_NoteHit = -1;
  return GMR_REUSE;
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

  struct Clavier_Gadget_Data * data = INST_DATA( class, gadget );
  data->cgd_NoteHit = -1;
  return 0;
}

static ULONG Handle_GM_LAYOUT( Class * class,
                               struct Gadget * gadget,
                               struct gpLayout * message ) {

  //Printf("vor h %ld w %ld \n", gadget->Height, gadget->Width );
  gadget->Width = getClavierWhiteKeyWidth( gadget )
                  * MAIN_KEYS_PER_OCTAVE 
                  * KEYS_PER_OCTAVE;
  gadget->Flags &= GFLG_RELHEIGHT & GFLG_RELWIDTH;
  SetAttrs( gadget, GA_Width, gadget->Width, TAG_END );
  //Printf("nach h %ld w %ld \n", gadget->Height, gadget->Width );
  return 1;
}

static ULONG Handle_GM_Domain( Class * class,
                               struct gpDomain * message ) {

  WORD heightIn = 43;
  if ( message->gpd_GInfo ) {
      heightIn = message->gpd_GInfo->gi_Domain.Height;
      Printf("ja %ld\n", heightIn);
  } else Printf("nein %ld \n", message->gpd_Which);

  switch ( message->gpd_Which ) {
/*
    case GDOMAIN_MINIMUM: {
      message->gpd_Domain.Left = 0;
      message->gpd_Domain.Top = 0;
      message->gpd_Domain.Width = WHITE_KEY_WIDTH_PIXEL * MAIN_KEYS_PER_OCTAVE;
      message->gpd_Domain.Height = WHITE_KEY_HEIGHT_PIXEL;
      return 1;
    }
*/
    case GDOMAIN_MINIMUM:
    case GDOMAIN_NOMINAL: {

      message->gpd_Domain.Left = 0;
      message->gpd_Domain.Top = 0;
      message->gpd_Domain.Width = getClavierWhiteKeyWidthByHeight( 43 )
                                  * MAIN_KEYS_PER_OCTAVE;
      message->gpd_Domain.Height = 43;
      return 1;
    }
    case GDOMAIN_MAXIMUM: {

      message->gpd_Domain.Left = 0;
      message->gpd_Domain.Top = 0;
      message->gpd_Domain.Width = getClavierWhiteKeyWidthByHeight( 2000 )
                                  * MAIN_KEYS_PER_OCTAVE
                                  * MAX_MIDI_OCTAVES;
      message->gpd_Domain.Height = 2000;
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
/*
    case GM_LAYOUT: {
      result = Handle_GM_LAYOUT( class,
                                 gadget,
                                 ( struct gpLayout * ) message );
      break;
    }
*/
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