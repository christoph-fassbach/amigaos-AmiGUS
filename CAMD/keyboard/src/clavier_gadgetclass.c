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
#include <proto/utility.h>

#include "clavier_gadgetclass.h"
#include "camd_keyboard.h"

#define LIMIT_ACTIVE_AREA // If defined, only hit and draw active area

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
      key->ck_name[ nameIndex++ ] = 0;
      break;
    }
    case 1: {
      keyType = Black;
      keyBase = 1;
      key->ck_name[ nameIndex++ ] = 'C';
      key->ck_name[ nameIndex++ ] = 0;
      key->ck_name[ nameIndex++ ] = '#';
      key->ck_name[ nameIndex++ ] = 0;
      break;
    }
    case 2: {
      keyType = WhiteMid;
      keyBase = 1;
      key->ck_name[ nameIndex++ ] = 'D';
      key->ck_name[ nameIndex++ ] = 0;
      break;
    }
    case 3: {
      keyType = Black;
      keyBase = 2;
      key->ck_name[ nameIndex++ ] = 'D';
      key->ck_name[ nameIndex++ ] = 0;
      key->ck_name[ nameIndex++ ] = '#';
      key->ck_name[ nameIndex++ ] = 0;
      break;
    }
    case 4: {
      keyType = WhiteRight;
      keyBase = 2;
      key->ck_name[ nameIndex++ ] = 'E';
      key->ck_name[ nameIndex++ ] = 0;
      break;
    }
    case 5: {
      keyType = WhiteLeft;
      keyBase = 3;
      key->ck_name[ nameIndex++ ] = 'F';
      key->ck_name[ nameIndex++ ] = 0;
      break;
    }
    case 6: {
      keyType = Black;
      keyBase = 4;
      key->ck_name[ nameIndex++ ] = 'F';
      key->ck_name[ nameIndex++ ] = 0;
      key->ck_name[ nameIndex++ ] = '#';
      key->ck_name[ nameIndex++ ] = 0;
      break;
    }
    case 7: {
      keyType = ( 127 == midiNote ) ? WhiteRight : WhiteMid;
      keyBase = 4;
      key->ck_name[ nameIndex++ ] = 'G';
      key->ck_name[ nameIndex++ ] = 0;
      break;
    }
    case 8: {
      keyType = Black;
      keyBase = 5;
      key->ck_name[ nameIndex++ ] = 'G';
      key->ck_name[ nameIndex++ ] = 0;
      key->ck_name[ nameIndex++ ] = '#';
      key->ck_name[ nameIndex++ ] = 0;
      break;
    }
    case 9: {
      keyType = WhiteMid;
      keyBase = 5;
      key->ck_name[ nameIndex++ ] = 'A';
      key->ck_name[ nameIndex++ ] = 0;
      break;
    }
    case 10: {
      keyType = Black;
      keyBase = 6;
      key->ck_name[ nameIndex++ ] = 'A';
      key->ck_name[ nameIndex++ ] = 0;
      key->ck_name[ nameIndex++ ] = '#';
      key->ck_name[ nameIndex++ ] = 0;
      break;
    }
    case 11:
    default: {
      keyBase = 6;
      keyType = WhiteRight;
      key->ck_name[ nameIndex++ ] = 'B';
      key->ck_name[ nameIndex++ ] = 0;
      break;
    }
  }

  if ( octave < 0 ) {

    key->ck_name[ nameIndex++ ] = '-';

  } else if ( octave > 10 ) {

    key->ck_name[ nameIndex++ ] = '+';

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
    data->cgd_Flags = CG_FLAG_REDRAW_BACKGROUND;
    data->cgd_NoteHit = -1;
    data->cgd_OffsetX = 0;
    data->cgd_VirtualWidth = 1000;
  }

  return ( ULONG ) gadget;
}

static ULONG Handle_OM_SET_OR_UPDATE( Class * class,
                                      struct Gadget * gadget,
                                      struct opUpdate * message ) {

  ULONG result = DoSuperMethodA( class,
                                 ( Object * ) gadget,
                                 ( Msg ) message );

  struct Clavier_Gadget_Data * data = INST_DATA( class, gadget );
  struct TagItem * tagState = message->opu_AttrList;
  struct TagItem * tagItem;

  while ( tagItem = NextTagItem( &tagState )) {

    switch ( tagItem->ti_Tag ) {
      case CG_OFFSET_X: {

        data->cgd_OffsetX = tagItem->ti_Data;
        data->cgd_Flags |= CG_FLAG_REDRAW_BACKGROUND;
        RefreshGadgets(gadget, CAMD_Keyboard_Base->ck_MainWindow, NULL );
        result |= 1;
        break;
      }
      case CG_VIRTUAL_WIDTH: {

        data->cgd_VirtualWidth = tagItem->ti_Data;
        result |= 1;
        break;
      }
      case CG_VISUAL_WIDTH:

        data->cgd_VisualWidth = tagItem->ti_Data;
        result |= 1;
      default: {
        break;
      }
    }
  }

  return result;
}

static ULONG Handle_OM_GET( Class * class,
                            struct Gadget * gadget,
                            struct opGet * message ) {

  ULONG result = DoSuperMethodA( class,
                                 ( Object * ) gadget,
                                 ( Msg ) message );

  const ULONG attributeId = message->opg_AttrID;
  struct Clavier_Gadget_Data * data = INST_DATA( class, gadget );

  if ( result ) {

    return result;
  }

  switch ( attributeId ) {
    case CG_OFFSET_X: {

      *( message->opg_Storage ) = data->cgd_OffsetX;
      return 1;
    }
    case CG_VIRTUAL_WIDTH: {

      *( message->opg_Storage ) = data->cgd_VirtualWidth;
      return 1;
    }
    case CG_VISUAL_WIDTH: {

      *( message->opg_Storage ) = data->cgd_VisualWidth;
      return 1;
    }
    default: {

      Printf("Unknown %lx\n", attributeId );
      return 0;
    }
  }
}

static ULONG Handle_GM_HITTEST( Class * class,
                                struct Gadget * gadget,
                                struct gpHitTest * message ) {

  const UWORD hitX = message->gpht_Mouse.X;
  const UWORD hitY = message->gpht_Mouse.Y;
  const WORD keyWidth = getClavierWhiteKeyWidth( gadget );
  struct Clavier_Gadget_Data * data = INST_DATA( class, gadget );

#ifdef LIMIT_ACTIVE_AREA
  
  const WORD offset = data->cgd_OffsetX;
  const WORD candidate = (( hitX + offset ) * KEYS_PER_OCTAVE ) / 
                         ( keyWidth * MAIN_KEYS_PER_OCTAVE );

#endif

  WORD i;
  struct Clavier_Key key;

#ifdef LIMIT_ACTIVE_AREA

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

  data->cgd_NoteHit = -1;
  return GMR_GADGETNOTHIT;
}

static ULONG Handle_GM_RENDER( Class * class,
                               struct Gadget * gadget,
                               struct gpRender * message ) {

  struct Clavier_Gadget_Data * data = INST_DATA( class, gadget );
  struct RastPort * rastPort = message->gpr_RPort;
  WORD midiNote;
  const WORD top = gadget->TopEdge;
  const WORD left = gadget->LeftEdge;
  const WORD width = gadget->Width - 1;
  const WORD height = gadget->Height - 1;

#ifdef LIMIT_ACTIVE_AREA

  const WORD keyWidth = getClavierWhiteKeyWidth( gadget );

  const WORD offset = data->cgd_OffsetX;
  const WORD start = (( offset * KEYS_PER_OCTAVE ) / 
                      ( keyWidth * MAIN_KEYS_PER_OCTAVE ));

#else

  const WORD start = 0;

#endif

  if ( CG_FLAG_REDRAW_BACKGROUND & data->cgd_Flags ) {

    // Only needed when scrolling, so do it only once!
    data->cgd_Flags &= ~CG_FLAG_REDRAW_BACKGROUND;

    // Flush drawable area with grey background
    SetAPen( rastPort, 0 );
    RectFill( rastPort, left, top, left + width, top + height );
  }

  for ( midiNote = start; midiNote < 128; ++midiNote ) {

    struct Clavier_Key key;
    ULONG pen;

    getClavierKeyProperties( &key, class, gadget, midiNote );

    if (( midiNote == data->cgd_NoteHit ) &&
        ( GFLG_SELECTED & gadget->Flags )) {

      pen = 3;

    } else {

      pen = key.ck_pen;
    }
    SetAPen( rastPort, pen );

    if (( width < key.ck_TopLeftX ) && ( width < key.ck_BottomLeftX )) {

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
      
      if (( 0 <= key.ck_BottomLeftX > 0 ) && ( width > key.ck_BottomRightX )) {

        struct IntuiText name = { 
          BLACK_KEY_PEN,
          WHITE_KEY_PEN,
          JAM1,
          2,
          2,
          NULL,
          NULL,
          NULL };
        struct IntuiText octave = { 
          BLACK_KEY_PEN,
          WHITE_KEY_PEN,
          JAM1,
          2,
          10,
          NULL,
          NULL,
          NULL };
        name.IText = &( key.ck_name[ 0 ] );
        octave.IText = &( key.ck_name[ 2 ] );
        name.NextText = &( octave );
        PrintIText( rastPort,
                    &name,
                    left + key.ck_BottomLeftX,
                    top + key.ck_SplitY );        
      }
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

  const WORD virtualWidth = getClavierWhiteKeyWidth( gadget ) *
                            (( MAIN_KEYS_PER_OCTAVE 
                            * MAX_MIDI_OCTAVES ) - 2 );
  const WORD visualWidth = gadget->Width;
  struct Clavier_Gadget_Data * data = INST_DATA( class, gadget );
  struct TagItem notifyTags[ 3 ];

  data->cgd_VirtualWidth = virtualWidth;
  data->cgd_VisualWidth = visualWidth;

  notifyTags[ 0 ].ti_Tag = CG_VIRTUAL_WIDTH;
  notifyTags[ 0 ].ti_Data = virtualWidth;
  notifyTags[ 1 ].ti_Tag = CG_VISUAL_WIDTH;
  notifyTags[ 1 ].ti_Data = visualWidth;
  notifyTags[ 2 ].ti_Tag = TAG_END;

  DoSuperMethod( class, ( Object * ) gadget, OM_NOTIFY, notifyTags, 0 );

  return 1;
}

static ULONG Handle_GM_Domain( Class * class,
                               struct gpDomain * message ) {

  if ( message->gpd_GInfo ) {

    Printf( "ja %ld\n", message->gpd_GInfo->gi_Domain.Height );

  } else {

    Printf( "nein %ld \n", message->gpd_Which );
  }

  switch ( message->gpd_Which ) {

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

  ULONG result;

  switch ( message->MethodID ) {
    case OM_NEW: {
      result = Handle_OM_NEW( class,
                              gadget,
                              ( struct opSet * ) message );
      break;
    }
    case OM_SET: {
      result = Handle_OM_SET_OR_UPDATE( class,
                                        gadget,
                                        ( struct opUpdate * ) message );
      break;
    }
    case OM_GET: {
      result = Handle_OM_GET( class,
                              gadget,
                              ( struct opGet * ) message );
      break;
    }
    case OM_UPDATE: {
      result = Handle_OM_SET_OR_UPDATE( class,
                                        gadget,
                                        ( struct opUpdate * ) message );
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
    case GM_LAYOUT: {
      result = Handle_GM_LAYOUT( class,
                                 gadget,
                                 ( struct gpLayout * ) message );
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
