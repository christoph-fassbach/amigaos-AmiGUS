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

#include <libraries/dos.h>

#include <proto/dos.h>

#include "sf2_reader.h"

#include "converter.h"
#include "debug.h"
#include "errors.h"
#include "support.h"

#define CHAR_TO_ULONG( a, b, c, d ) ((( a ) << 24 ) | (( b ) << 16 ) | (( c ) << 8 ) | ( d ))

#define RIFF_CHUNK_ID CHAR_TO_ULONG( 'R', 'I', 'F', 'F' )
#define LIST_CHUNK_ID CHAR_TO_ULONG( 'L', 'I', 'S', 'T' )
#define SFBK_CHUNK_ID CHAR_TO_ULONG( 's', 'f', 'b', 'k' )
#define INFO_CHUNK_ID CHAR_TO_ULONG( 'I', 'N', 'F', 'O' )
#define SDTA_CHUNK_ID CHAR_TO_ULONG( 's', 'd', 't', 'a' )
#define PDTA_CHUNK_ID CHAR_TO_ULONG( 'p', 'd', 't', 'a' )
#define IFIL_CHUNK_ID CHAR_TO_ULONG( 'i', 'f', 'i', 'l' )
#define ISNG_CHUNK_ID CHAR_TO_ULONG( 'i', 's', 'n', 'g' )
#define INAM_CHUNK_ID CHAR_TO_ULONG( 'I', 'N', 'A', 'M' )
#define IROM_CHUNK_ID CHAR_TO_ULONG( 'i', 'r', 'o', 'm' )
#define IVER_CHUNK_ID CHAR_TO_ULONG( 'i', 'v', 'e', 'r' )
#define ICRD_CHUNK_ID CHAR_TO_ULONG( 'I', 'C', 'R', 'D' )
#define IENG_CHUNK_ID CHAR_TO_ULONG( 'I', 'E', 'N', 'G' )
#define IPRD_CHUNK_ID CHAR_TO_ULONG( 'I', 'P', 'R', 'D' )
#define ICOP_CHUNK_ID CHAR_TO_ULONG( 'I', 'C', 'O', 'P' )
#define ICMT_CHUNK_ID CHAR_TO_ULONG( 'I', 'C', 'M', 'T' )
#define ISFT_CHUNK_ID CHAR_TO_ULONG( 'I', 'S', 'F', 'T' )
#define DMOD_CHUNK_ID CHAR_TO_ULONG( 'D', 'M', 'O', 'D' )
#define SNAM_CHUNK_ID CHAR_TO_ULONG( 's', 'n', 'a', 'm' )
#define SMPL_CHUNK_ID CHAR_TO_ULONG( 's', 'm', 'p', 'l' )
#define PHDR_CHUNK_ID CHAR_TO_ULONG( 'p', 'h', 'd', 'r' )
#define PBAG_CHUNK_ID CHAR_TO_ULONG( 'p', 'b', 'a', 'g' )
#define PMOD_CHUNK_ID CHAR_TO_ULONG( 'p', 'm', 'o', 'd' )
#define PGEN_CHUNK_ID CHAR_TO_ULONG( 'p', 'g', 'e', 'n' )
#define IHDR_CHUNK_ID CHAR_TO_ULONG( 'i', 'n', 's', 't' )
#define IBAG_CHUNK_ID CHAR_TO_ULONG( 'i', 'b', 'a', 'g' )
#define IMOD_CHUNK_ID CHAR_TO_ULONG( 'i', 'm', 'o', 'd' )
#define IGEN_CHUNK_ID CHAR_TO_ULONG( 'i', 'g', 'e', 'n' )
#define SHDR_CHUNK_ID CHAR_TO_ULONG( 's', 'h', 'd', 'r' )
#define SM24_CHUNK_ID CHAR_TO_ULONG( 's', 'm', '2', '4' )
#define DLS_CHUNK_ID  CHAR_TO_ULONG( 'D', 'L', 'S', ' ' )

static const ULONG chunkIds[] = {

  RIFF_CHUNK_ID,
  LIST_CHUNK_ID,
  SFBK_CHUNK_ID,
  INFO_CHUNK_ID,
  SDTA_CHUNK_ID,
  PDTA_CHUNK_ID,
  IFIL_CHUNK_ID,
  ISNG_CHUNK_ID,
  INAM_CHUNK_ID,
  IROM_CHUNK_ID,
  IVER_CHUNK_ID,
  ICRD_CHUNK_ID,
  IENG_CHUNK_ID,
  IPRD_CHUNK_ID,
  ICOP_CHUNK_ID,
  ICMT_CHUNK_ID,
  ISFT_CHUNK_ID,
  DMOD_CHUNK_ID,
  SNAM_CHUNK_ID,
  SMPL_CHUNK_ID,
  PHDR_CHUNK_ID,
  PBAG_CHUNK_ID,
  PMOD_CHUNK_ID,
  PGEN_CHUNK_ID,
  IHDR_CHUNK_ID,
  IBAG_CHUNK_ID,
  IMOD_CHUNK_ID,
  IGEN_CHUNK_ID,
  SHDR_CHUNK_ID,
  SM24_CHUNK_ID,
  DLS_CHUNK_ID,
  0
};

enum SF2_Chunk_Indices {

  RIFF_CHUNK_INDEX = 0,
  LIST_CHUNK_INDEX,
  SFBK_CHUNK_INDEX,
  INFO_CHUNK_INDEX,
  SDTA_CHUNK_INDEX,
  PDTA_CHUNK_INDEX,
  IFIL_CHUNK_INDEX,
  ISNG_CHUNK_INDEX,
  INAM_CHUNK_INDEX,
  IROM_CHUNK_INDEX,
  IVER_CHUNK_INDEX,
  ICRD_CHUNK_INDEX,
  IENG_CHUNK_INDEX,
  IPRD_CHUNK_INDEX,
  ICOP_CHUNK_INDEX,
  ICMT_CHUNK_INDEX,
  ISFT_CHUNK_INDEX,
  DMOD_CHUNK_INDEX,
  SNAM_CHUNK_INDEX,
  SMPL_CHUNK_INDEX,
  PHDR_CHUNK_INDEX,
  PBAG_CHUNK_INDEX,
  PMOD_CHUNK_INDEX,
  PGEN_CHUNK_INDEX,
  IHDR_CHUNK_INDEX,
  IBAG_CHUNK_INDEX,
  IMOD_CHUNK_INDEX,
  IGEN_CHUNK_INDEX,
  SHDR_CHUNK_INDEX,
  SM24_CHUNK_INDEX,
  DLS_CHUNK_INDEX,
  MAX_CHUNK_INDEX
};

struct SF2_Chunk {

  ULONG id;
  ULONG size;
};

static UWORD Swap16( UWORD value ) {

  return ( UWORD )((( value & 0xFF00 ) >> 8 ) |
                   (( value & 0x00FF ) << 8 ));
}

static ULONG Swap32( ULONG value ) {

  return ((( value & 0xFF000000 ) >> 24 ) |
          (( value & 0x00FF0000 ) >>  8 ) |
          (( value & 0x0000FF00 ) <<  8 ) |
          (( value & 0x000000FF ) << 24 ));
}

static LONG ReadULONG( BPTR fileHandle, ULONG * target ) {

  return FRead( fileHandle, target, sizeof( ULONG ), 1 );
}

static LONG ReadUWORD( BPTR fileHandle, UWORD * target ) {

  return FRead( fileHandle, target, sizeof( UWORD ), 1 );
}

static LONG ReadChunk( BPTR fileHandle, struct SF2_Chunk * target ) {

  LONG result = FRead( fileHandle, target, sizeof( struct SF2_Chunk ), 1 );
  if ( 1 != result ) {

    return result;
  }
  target->size = Swap32( target->size );
  return result;
}

static LONG ReadListChunk( BPTR fileHandle, struct SF2_Chunk * target ) {

  LONG result = ReadChunk( fileHandle, target );
  if (( 1 != result ) || ( chunkIds[ LIST_CHUNK_INDEX ] != target->id )) {

    return result;
  }
  target->size -= 4;
  result = ReadULONG( fileHandle, &( target->id ));
  return result;
}

static LONG ReadInfo( struct SF2_Parsed * sf2, ULONG size ) {

  LONG result;
  struct SF2_Chunk chunk;

  union {

    ULONG id;
    char idAsString[ 8 ];
  } chunkHelper;

  while ( 0 < size ) {

    result = ReadChunk( sf2->sf2_FileHandle, &( chunk ));
    size -= sizeof( struct SF2_Chunk );
    if ( 1 != result ) {

      return EInvalidInfoChunk;
    }

    if ( chunkIds[ IFIL_CHUNK_INDEX ] == chunk.id ) {

      UWORD version;
      if ( 4 != chunk.size ) {

        return EInvalidVersionChunk;
      }

      ReadUWORD( sf2->sf2_FileHandle, &( version ));
      sf2->sf2_MajorVersion = Swap16( version );
      ReadUWORD( sf2->sf2_FileHandle, &( version ));
      sf2->sf2_MinorVersion = Swap16( version );

      LOG_I(( "I: Found SoundFont version %ld.%ld.\n",
              sf2->sf2_MajorVersion, sf2->sf2_MinorVersion ));
      if ( 2 != sf2->sf2_MajorVersion ) {

        LOG_D(( "D: exp 0002 is %04lx\n", sf2->sf2_MajorVersion ));
        return EIncompatibleVersion;
      }
      size -= chunk.size;
      continue;
    }
    if ( chunkIds[ IVER_CHUNK_INDEX ] == chunk.id ) {

      if ( 4 != chunk.size ) {

        return EInvalidVersionChunk;
      }

      // Assumption: We do not need the ROM version at all.

      size -= chunk.size;
      continue;
    }

    // Handling all "ignored" chunks now :)
    chunkHelper.id = chunk.id;
    chunkHelper.idAsString[ 4 ] = 0;

    if ( chunk.size & 0x00000001 ) {

      LOG_E(( "E: Chunk %s has invalid size %ld.\n",
              chunkHelper.idAsString, chunk.size ));
      return EInvalidInfoSubChunkSize;
    }
    if (((( chunkIds[ ICOP_CHUNK_INDEX ] == chunk.id ) ||
          ( chunkIds[ ICRD_CHUNK_INDEX ] == chunk.id ) ||    
          ( chunkIds[ IENG_CHUNK_INDEX ] == chunk.id ) ||
          ( chunkIds[ INAM_CHUNK_INDEX ] == chunk.id ) ||
          ( chunkIds[ IPRD_CHUNK_INDEX ] == chunk.id ) ||
          ( chunkIds[ IROM_CHUNK_INDEX ] == chunk.id ) ||
          ( chunkIds[ ISFT_CHUNK_INDEX ] == chunk.id ) ||
          ( chunkIds[ ISNG_CHUNK_INDEX ] == chunk.id )) &&
          ( 256 < chunk.size )) ||
          (( chunkIds[ ICMT_CHUNK_INDEX ] == chunk.id ) &&
           ( 65536 < chunk.size ))) {

      LOG_W(( "W: Chunk %s has invalid size %ld.\n",
              chunkHelper.idAsString, chunk.size ));
    }
    size -= chunk.size;
    Seek( sf2->sf2_FileHandle, chunk.size, OFFSET_CURRENT );
    LOG_I(( "I: Skipping chunk %s with size %ld, \tremaining in header %ld.\n",
            chunkHelper.idAsString, chunk.size, size ));
  }
  return ENoError;
}

static LONG ReadHeader( struct SF2_Parsed * sf2 ) {

  LONG result;
  struct SF2_Chunk chunk;

  result = ReadChunk( sf2->sf2_FileHandle, &( chunk ));
  if (( 1 != result ) || ( chunkIds[ RIFF_CHUNK_INDEX ] != chunk.id )) {

    DisplayError( ENoRiffChunk );
    return ENoRiffChunk;
  }
  LOG_D(( "D: RIFF passed.\n" ));

  result = ReadULONG( sf2->sf2_FileHandle, &( chunk.id ));
  if (( 1 != result ) || ( chunkIds[ SFBK_CHUNK_INDEX ] != chunk.id )) {

    DisplayError( ENoSfbkChunk );
    return ENoSfbkChunk;
  }
  LOG_D(( "D: SoundFont passed.\n" ));

  if ( chunk.size != ( sf2->sf2_FileSize - 8 )) {

    DisplayError( EInvalidFileSize );
    return EInvalidFileSize;
  }
  LOG_D(( "D: Size passed.\n" ));

  result = ReadListChunk( sf2->sf2_FileHandle, &( chunk ));
  if (( 1 != result ) || ( chunkIds[ INFO_CHUNK_INDEX ] != chunk.id )) {

    DisplayError( ENoInfoChunk );
    return ENoInfoChunk;
  }
  result = ReadInfo( sf2, chunk.size );
  if ( ENoError != result ) {

    DisplayError( result );
    return result;
  }
  LOG_D(( "D: Info passed.\n" ));

  result = ReadListChunk( sf2->sf2_FileHandle, &( chunk ));
  if (( 1 != result ) || ( chunkIds[ SDTA_CHUNK_INDEX ] != chunk.id )) {

    DisplayError( ENoSdtaChunk );
    return ENoSdtaChunk;
  }
  LOG_D(( "D: Sample data passed.\n" ));

  result = ReadListChunk( sf2->sf2_FileHandle, &( chunk ));
  if (( 1 != result ) || ( chunkIds[ PDTA_CHUNK_INDEX ] != chunk.id )) {

    DisplayError( ENoPdtaChunk );
    return ENoPdtaChunk;
  }
  LOG_D(( "D: Preset data passed.\n" ));
  // hydra pos + size here

  return ENoError;
}


struct SF2_Parsed * AllocSf2FromFile( STRPTR filePath ) {

  LONG result;
  struct SF2_Parsed X;
  struct SF2_Parsed * sf2 = &( X );
  
  sf2->sf2_FileHandle = Open( filePath, MODE_OLDFILE );
  if ( !( sf2->sf2_FileHandle )) {

    DisplayError( EOpenFileFailed );
    return NULL;
  }

  sf2->sf2_FilePath = C_strcpy_VD( filePath );

  // SFData https://github.com/FluidSynth/fluidsynth/blob/master/src/sfloader/fluid_sffile.h#L135
  // read https://github.com/FluidSynth/fluidsynth/blob/master/src/sfloader/fluid_sffile.c#L354

  Seek( sf2->sf2_FileHandle, 0, OFFSET_END );
  sf2->sf2_FileSize = Seek( sf2->sf2_FileHandle, 0, OFFSET_BEGINING );
  LOG_D(("D: SF2 file size %ld\n", sf2->sf2_FileSize ));
  if ( 0 >= sf2->sf2_FileSize ) {

    DisplayError( EInvalidFileSize );
    Close( sf2->sf2_FileHandle );
    return NULL;
  }

  result = ReadHeader( sf2 );

  Close( sf2->sf2_FileHandle );
  return NULL;
}

VOID FreeSf2( struct SF2_Parsed * soundFont ) {

}
