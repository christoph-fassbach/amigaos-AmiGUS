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
#include <proto/exec.h>

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

/* SF2 well known chunk size multiples and sizes */
#define PHDR_CHUNK_SIZE_MULTIPLE (    38 )
#define PBAG_CHUNK_SIZE_MULTIPLE (     4 )
#define PMOD_CHUNK_SIZE_MULTIPLE (    10 )
#define PGEN_CHUNK_SIZE_MULTIPLE (     4 )
#define IHDR_CHUNK_SIZE_MULTIPLE (    22 )
#define SHDR_CHUNK_SIZE_MULTIPLE (    46 )
#define CHUNK_SIZE_LIMIT_8bit    (   256 )
#define CHUNK_SIZE_LIMIT_16bit   ( 65536 )

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

static LONG ReadString( BPTR fileHandle, UBYTE * target ) {

  return FRead( fileHandle, target, sizeof( UBYTE ), 20 );
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
  if (( 1 != result ) || ( LIST_CHUNK_ID != target->id )) {

    return result;
  }
  target->size -= 4;
  result = ReadULONG( fileHandle, &( target->id ));
  return result;
}

static LONG ReadPresetSubChunk( BPTR fileHandle,
                                struct SF2_Chunk * target,
                                const LONG expectedId,
                                const LONG expectedSizeMultiple,
                                ULONG * remainingSize ) {

  union { ULONG id;
          BYTE idAsString[ 8 ];
        } chunkHelper[ 2 ];
  LONG result = ReadChunk( fileHandle, target );

  *( remainingSize ) -= 8;
  chunkHelper[ 0 ].id = expectedId;
  chunkHelper[ 0 ].idAsString[ 4 ] = 0;

  if ( 1 != result ) {

    LOG_E(( "E: Preset sub chunk '%s' expected, but read %ld.\n",
            chunkHelper[ 0 ].idAsString, result ));
    return EInvalidPresetSubChunk;
  }
  if ( expectedId != target->id ) {

    chunkHelper[ 1 ].id = target->id;
    chunkHelper[ 1 ].idAsString[ 4 ] = 0;

    LOG_E(( "E: Preset sub chunk expected '%s', actual '%s'\n",
            chunkHelper[ 0 ].idAsString, chunkHelper[ 1 ].idAsString ));
    return EInvalidPresetSubChunk;
  }
  if ( target->size % expectedSizeMultiple ) {

    LOG_E(( "E: Preset sub chunk '%s' size multiple of expected %ld, "
            "actual %ld.\n",
            chunkHelper[ 0 ].idAsString,
            expectedSizeMultiple,
            target->size ));
    return EInvalidPresetSubChunk;
  }
  if ( *( remainingSize ) < target->size ) {

    LOG_E(( "E: Preset sub chunk '%s' size %ld exceeds remaining size %ld.\n",
            chunkHelper[ 0 ].idAsString,
            target->size,
            remainingSize ));
  }
  *( remainingSize ) -= target->size;
  return ENoError;
}

static LONG ReadPresetHeaders( BPTR fileHandle,
                               const LONG size,
                               struct MinList * target ) {

  LONG i;
  UWORD temp;
  UWORD index;
  UWORD previousIndex = 0;
  struct SF2_Preset * previousPreset = NULL;

  if (( !( size )) || ( size % PHDR_CHUNK_SIZE_MULTIPLE )) {

    return EInvalidPresetDataSize;
  }

  i = ( size / PHDR_CHUNK_SIZE_MULTIPLE ) - 1;
  if ( 0 >= i ) {

    return ENoPresets;
  }
  LOG_D(( "D: Found %ld presets in %ld.\n", i + 1, size ));

  while ( 0 <= i ) {

    struct SF2_Preset * preset;
    if ( 0 < i ) {

      preset = AllocVec( sizeof( struct SF2_Preset ), MEMF_ANY | MEMF_CLEAR );
      NEW_LIST( &( preset->sf2p_Zones ));
      ADD_TAIL( target, &( preset->sf2p_Node ));
      ReadString( fileHandle, preset->sf2p_Name );
      ReadUWORD( fileHandle, &( temp ));
      preset->sf2p_Number = Swap16( temp );
      ReadUWORD( fileHandle, &( temp ));
      preset->sf2p_Bank = Swap16( temp );

    } else {

      Seek( fileHandle, 24, OFFSET_CURRENT );
    }
    ReadUWORD( fileHandle, &( temp ));
    index = Swap16( temp );
    Seek( fileHandle, 12 /* library + genre + morphology */, OFFSET_CURRENT );

    if ( previousPreset ) {

      // So >1st preset
      if ( index > previousIndex ) {

        LONG h = index - previousIndex;
        LOG_V(( "V: Adding %ld zones to preset %ld\n",
                h, ( size / PHDR_CHUNK_SIZE_MULTIPLE ) - 1 - i ));
        while ( h ) {

          struct SF2_Zone * zone = AllocVec( sizeof( struct SF2_Zone ),
                                             MEMF_ANY | MEMF_CLEAR );
          NEW_LIST( &( zone->sfz2_Generators ));
          NEW_LIST( &( zone->sfz2_Modulators ));
          ADD_TAIL( &( previousPreset->sf2p_Zones ), zone );
          --h;
        }
      } else {

        return EInvalidPresetIndex;
      }
    } else if ( index > 0 ) {
      LOG_W(( "W: %ld preset zones unused!\n" ));
    }

    previousPreset = preset;
    previousIndex = index;
    --i;
  }

  return ENoError;
}

static LONG ReadZone( BPTR fileHandle,
                      UWORD * lastGeneratorIndex,
                      UWORD * lastModulatorIndex,
                      struct SF2_Zone * target ) {

  UWORD temp;
  UWORD generatorIndex;
  UWORD modulatorIndex;

  ReadUWORD( fileHandle, &( temp ));
  generatorIndex = Swap16( temp );
  ReadUWORD( fileHandle, &( temp ));
  modulatorIndex = Swap16( temp );

  if ( target ) {

    // So >1st zone
    if ( generatorIndex < *lastGeneratorIndex ) {

      return EInvalidGeneratorIndex;
    }
    if ( modulatorIndex < *lastModulatorIndex ) {

      return EInvalidModulatorIndex;
    }

    temp = generatorIndex - *lastGeneratorIndex;
    while ( temp ) {

      struct SF2_Generator * generator =
        AllocVec( sizeof( struct SF2_Generator ), MEMF_ANY | MEMF_CLEAR );
      ADD_TAIL( &( target->sfz2_Generators ), generator );
      --temp;
    }
    temp = modulatorIndex - *lastModulatorIndex;
    while ( temp ) {

      struct SF2_Modulator * modulator =
        AllocVec( sizeof( struct SF2_Modulator ), MEMF_ANY | MEMF_CLEAR );
      ADD_TAIL( &( target->sfz2_Modulators ), modulator );
      --temp;
    }
  }
  *( lastGeneratorIndex ) = generatorIndex;
  *( lastModulatorIndex ) = modulatorIndex;

  return ENoError;
}

static LONG ReadPresetBags( BPTR fileHandle,
                            LONG size,
                            struct MinList * target ) {

  LONG result;
  struct SF2_Preset * preset;
  struct SF2_Zone * previousZone = NULL;
  UWORD previousGeneratorIndex = 0;
  UWORD previousModulatorIndex = 0;

  FOR_LIST( target, preset, struct SF2_Preset * ) {

    struct SF2_Zone * zone;
    FOR_LIST( &( preset->sf2p_Zones ), zone, struct SF2_Zone * ) {

      
      size -= PBAG_CHUNK_SIZE_MULTIPLE;

      result = ReadZone( fileHandle,
                         &previousGeneratorIndex,
                         &previousModulatorIndex,
                         previousZone );

      if ( result ) {

        return result;
      }
      previousZone = zone;
    }
  }
  if ( PBAG_CHUNK_SIZE_MULTIPLE != size ) {

    return EInvalidPresetBags;
  }
  result = ReadZone( fileHandle,
                      &previousGeneratorIndex,
                      &previousModulatorIndex,
                      previousZone );
  return result;
}

static LONG ReadInfo( struct SF2_Parsed * sf2, ULONG size ) {

  LONG result;
  struct SF2_Chunk chunk;
  union { ULONG id;
          BYTE idAsString[ 8 ];
        } chunkHelper;

  while ( 0 < size ) {

    result = ReadChunk( sf2->sf2_FileHandle, &( chunk ));
    size -= sizeof( struct SF2_Chunk );
    if ( 1 != result ) {

      return EInvalidInfoChunk;
    }

    if ( IFIL_CHUNK_ID == chunk.id ) {

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
    if ( IVER_CHUNK_ID == chunk.id ) {

      if ( 4 != chunk.size ) {

        return EInvalidVersionChunk;
      }

      // Assumption: We do not need the ROM version at all.

      Seek( sf2->sf2_FileHandle, chunk.size, OFFSET_CURRENT );
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
    if (((( ICOP_CHUNK_ID == chunk.id ) ||
          ( ICRD_CHUNK_ID == chunk.id ) ||    
          ( IENG_CHUNK_ID == chunk.id ) ||
          ( INAM_CHUNK_ID == chunk.id ) ||
          ( IPRD_CHUNK_ID == chunk.id ) ||
          ( IROM_CHUNK_ID == chunk.id ) ||
          ( ISFT_CHUNK_ID == chunk.id ) ||
          ( ISNG_CHUNK_ID == chunk.id )) &&
          ( CHUNK_SIZE_LIMIT_8bit < chunk.size )) ||
          (( ICMT_CHUNK_ID == chunk.id ) &&
           ( CHUNK_SIZE_LIMIT_16bit < chunk.size ))) {

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

static LONG ReadSampleInfo( struct SF2_Parsed * sf2, ULONG size ) {

  LONG result;
  struct SF2_Chunk chunk;

  if ( size < 8 ) {

    LOG_E(( "E: Sample chunk size %ld too small.\n", size ));
    return ENoSmplChunk;
  }

  result = ReadChunk( sf2->sf2_FileHandle, &( chunk ));
  if ( 1 != result ) {

    LOG_E(( "E: Could not read sample chunk.\n" ));
    return ENoSmplChunk;
  }
  size -= 8;

  if ( SMPL_CHUNK_ID == chunk.id ) {

    LONG position;

    if ( sf2->sf2_16bitSamplePosition ) {

      return EDuplicatedSmplChunk;
    }
    position = Seek( sf2->sf2_FileHandle, 0, OFFSET_CURRENT );
    sf2->sf2_16bitSamplePosition = position;
    sf2->sf2_16bitSampleSize = chunk.size;
    Seek( sf2->sf2_FileHandle, chunk.size, OFFSET_CURRENT );

    LOG_I(( "I: 16bit sample position %ld, size %ld\n",
            position, chunk.size ));
    
    size -= chunk.size;
  }
  if ( size >= 8 ) {

    result = ReadChunk( sf2->sf2_FileHandle, &( chunk ));
    size -= 8;
  }
  if ( 1 != result ) {

    LOG_E(( "E: Could not read sample chunk.\n" ));
    return ENoSmplChunk;
  }
  if (( 2 >= sf2->sf2_MajorVersion ) &&
      ( 4 >= sf2->sf2_MinorVersion ) &&
      ( SM24_CHUNK_ID == chunk.id )) {

    LONG position;
    if ( sf2->sf2_24bitSamplePosition ) {

      return EDuplicatedSm24Chunk;
    }
    position = Seek( sf2->sf2_FileHandle, 0, OFFSET_CURRENT );
    sf2->sf2_24bitSamplePosition = position;
    sf2->sf2_24bitSampleSize = chunk.size;
    Seek( sf2->sf2_FileHandle, chunk.size, OFFSET_CURRENT );

    LOG_I(( "I: 24bit sample position %ld, size %ld\n",
            position, chunk.size ));
    
    size -= chunk.size;
  }
  if ( size >= 8 ) {

    return ENoSmplChunk;
  }
  if ( 0 > size ) {

    Seek( sf2->sf2_FileHandle, size, OFFSET_CURRENT );
    LOG_W(( "W: Skipping over some sample data left-overs.\n" ));
  }
  return ENoError;
}

static LONG ReadPresetInfo( struct SF2_Parsed * sf2, ULONG size ) {

  LONG result;
  struct SF2_Chunk chunk;
  union { ULONG id;
          BYTE idAsString[ 8 ];
        } chunkHelper;

  sf2->sf2_PresetsPosition = Seek( sf2->sf2_FileHandle, 0, OFFSET_CURRENT );
  sf2->sf2_PresetsSize = size;
  LOG_D(( "D: Initial preset size is %ld\n", size ));

  // Preset Headers
  result = ReadPresetSubChunk( sf2->sf2_FileHandle,
                               &( chunk ),
                               PHDR_CHUNK_ID,
                               PHDR_CHUNK_SIZE_MULTIPLE,
                               &( size ));
  if ( result ) {

    return result;
  }
  result = ReadPresetHeaders( sf2->sf2_FileHandle,
                              chunk.size,
                              &( sf2->sf2_Presets ));
  if ( result ) {

    return result;
  }

  LOG_D(( "D: After headers, preset size is %ld\n", size ));
  // Preset Bags
  result = ReadPresetSubChunk( sf2->sf2_FileHandle,
                               &( chunk ),
                               PBAG_CHUNK_ID,
                               PBAG_CHUNK_SIZE_MULTIPLE,
                               &( size ));
  if ( result ) {

    return result;
  }
  result = ReadPresetBags( sf2->sf2_FileHandle,
                           chunk.size,
                           &( sf2->sf2_Presets ));
  if ( result ) {

    return result;
  }

  LOG_D(( "D: Remaining preset size is %ld\n", size ));
  Seek( sf2->sf2_FileHandle, size, OFFSET_CURRENT );
  return ENoError;
}

static LONG ReadHeader( struct SF2_Parsed * sf2 ) {

  LONG result;
  LONG expectedPosition;
  LONG actualPosition;
  struct SF2_Chunk chunk;

  result = ReadChunk( sf2->sf2_FileHandle, &( chunk ));
  if (( 1 != result ) || ( RIFF_CHUNK_ID != chunk.id )) {

    DisplayError( ENoRiffChunk );
    return ENoRiffChunk;
  }
  LOG_D(( "D: RIFF passed.\n" ));

  result = ReadULONG( sf2->sf2_FileHandle, &( chunk.id ));
  if (( 1 != result ) || ( SFBK_CHUNK_ID != chunk.id )) {

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
  if (( 1 != result ) || ( INFO_CHUNK_ID != chunk.id )) {

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
  if (( 1 != result ) || ( SDTA_CHUNK_ID != chunk.id )) {

    DisplayError( ENoSdtaChunk );
    return ENoSdtaChunk;
  }
  result = ReadSampleInfo( sf2, chunk.size );
  if ( ENoError != result ) {

    DisplayError( result );
    return result;
  }
  LOG_D(( "D: Sample info passed.\n" ));

  result = ReadListChunk( sf2->sf2_FileHandle, &( chunk ));
  if (( 1 != result ) || ( PDTA_CHUNK_ID != chunk.id )) {

    DisplayError( ENoPdtaChunk );
    return ENoPdtaChunk;
  }
  result = ReadPresetInfo( sf2, chunk.size );
  if ( ENoError != result ) {

    DisplayError( result );
    return result;
  }
  LOG_D(( "D: Preset data passed.\n" ));

  actualPosition = Seek( sf2->sf2_FileHandle, 0, OFFSET_END );
  expectedPosition = Seek( sf2->sf2_FileHandle, 0, OFFSET_CURRENT );
  LOG_D(( "D: Expected %ld, actual %ld\n", expectedPosition, actualPosition ));
  if ( actualPosition != expectedPosition ) {

    DisplayError( EParseFailed );
    return EParseFailed;
  }

  return ENoError;
}

struct SF2_Parsed * AllocSf2FromFile( STRPTR filePath ) {

  LONG result;
  struct SF2_Parsed X;
  struct SF2_Parsed * sf2 = &( X );

  sf2->sf2_16bitSamplePosition = 0;
  sf2->sf2_24bitSamplePosition = 0;

  NEW_LIST( &( sf2->sf2_Samples ));
  NEW_LIST( &( sf2->sf2_Modulators ));
  NEW_LIST( &( sf2->sf2_Instruments ));
  NEW_LIST( &( sf2->sf2_Presets ));

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
  if ( result ) {

    Close( sf2->sf2_FileHandle );
    return NULL;
  }

  Close( sf2->sf2_FileHandle );
  return NULL;
}

VOID FreeSf2( struct SF2_Parsed * soundFont ) {

}
