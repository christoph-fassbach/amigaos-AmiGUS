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
#include "amisf.h"

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
#define  BAG_CHUNK_SIZE_MULTIPLE (     4 )
#define  MOD_CHUNK_SIZE_MULTIPLE (    10 )
#define  GEN_CHUNK_SIZE_MULTIPLE (     4 )
#define IHDR_CHUNK_SIZE_MULTIPLE (    22 )
#define SHDR_CHUNK_SIZE_MULTIPLE (    46 )
#define CHUNK_SIZE_LIMIT_8bit    (   256 )
#define CHUNK_SIZE_LIMIT_16bit   ( 65536 )

/**
 * Generator (effect) numbers (Soundfont 2.01 specifications section 8.1.3)
 */
enum SF2_Generator_Ids {
    GEN_STARTADDROFS,       // Sample start address offset (0-32767)
    GEN_ENDADDROFS,         // Sample end address offset (-32767-0)
    GEN_STARTLOOPADDROFS,   // Sample loop start address offset (-32767-32767)
    GEN_ENDLOOPADDROFS,     // Sample loop end address offset (-32767-32767)
    GEN_STARTADDRCOARSEOFS, // Sample start address coarse offset (X 32768)
    GEN_MODLFOTOPITCH,      // Modulation LFO to pitch
    GEN_VIBLFOTOPITCH,      // Vibrato LFO to pitch
    GEN_MODENVTOPITCH,      // Modulation envelope to pitch
    GEN_FILTERFC,           // Filter cutoff
    GEN_FILTERQ,            // Filter Q
    GEN_MODLFOTOFILTERFC,   // Modulation LFO to filter cutoff
    GEN_MODENVTOFILTERFC,   // Modulation envelope to filter cutoff
    GEN_ENDADDRCOARSEOFS,   // Sample end address coarse offset (X 32768)
    GEN_MODLFOTOVOL,        // Modulation LFO to volume
    GEN_UNUSED1,            // Unused
    GEN_CHORUSSEND,         // Chorus send amount
    GEN_REVERBSEND,         // Reverb send amount
    GEN_PAN,                // Stereo panning
    GEN_UNUSED2,            // Unused
    GEN_UNUSED3,            // Unused
    GEN_UNUSED4,            // Unused
    GEN_MODLFODELAY,        // Modulation LFO delay
    GEN_MODLFOFREQ,         // Modulation LFO frequency
    GEN_VIBLFODELAY,        // Vibrato LFO delay
    GEN_VIBLFOFREQ,         // Vibrato LFO frequency
    GEN_MODENVDELAY,        // Modulation envelope delay
    GEN_MODENVATTACK,       // Modulation envelope attack
    GEN_MODENVHOLD,         // Modulation envelope hold
    GEN_MODENVDECAY,        // Modulation envelope decay
    GEN_MODENVSUSTAIN,      // Modulation envelope sustain
    GEN_MODENVRELEASE,      // Modulation envelope release
    GEN_KEYTOMODENVHOLD,    // Key to modulation envelope hold
    GEN_KEYTOMODENVDECAY,   // Key to modulation envelope decay
    GEN_VOLENVDELAY,        // Volume envelope delay
    GEN_VOLENVATTACK,       // Volume envelope attack
    GEN_VOLENVHOLD,         // Volume envelope hold
    GEN_VOLENVDECAY,        // Volume envelope decay
    GEN_VOLENVSUSTAIN,      // Volume envelope sustain
    GEN_VOLENVRELEASE,      // Volume envelope release
    GEN_KEYTOVOLENVHOLD,    // Key to volume envelope hold
    GEN_KEYTOVOLENVDECAY,   // Key to volume envelope decay
    GEN_INSTRUMENT,         // Instrument ID (shouldn't be set by user)
    GEN_RESERVED1,          // Reserved
    GEN_KEYRANGE,           // MIDI note range
    GEN_VELRANGE,           // MIDI velocity range
    GEN_STARTLOOPCOARSE,    // Sample start loop address coarse offset (X 32768)
    GEN_KEYNUM,             // Fixed MIDI note number
    GEN_VELOCITY,           // Fixed MIDI velocity value
    GEN_ATTENUATION,        // Initial volume attenuation
    GEN_RESERVED2,          // Reserved
    GEN_ENDLOOPCOARSE,      // Sample end loop address coarse offset (X 32768)
    GEN_COARSETUNE,         // Coarse tuning
    GEN_FINETUNE,           // Fine tuning
    GEN_SAMPLEID,           // Sample ID (shouldn't be set by user)
    GEN_SAMPLEMODE,         // Sample mode flags
    GEN_RESERVED3,          // Reserved
    GEN_SCALETUNE,          // Scale tuning
    GEN_EXCLUSIVECLASS,     // Exclusive class number
    GEN_OVERRIDEROOTKEY,    // Sample root note override
    GEN_LAST                // Count of generators - not to be used or exceeded!
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

static LONG ReadUBYTE( BPTR fileHandle, UBYTE * target ) {

  return FRead( fileHandle, target, sizeof( UBYTE ), 1 );
}

static LONG ReadString( BPTR fileHandle, UBYTE * target ) {

  return FRead( fileHandle, target, sizeof( UBYTE ), 20 );
}

static WORD GetBankFromCommon( struct SF2_Common * common ) {

  if ( SF2_COMMON_PRESET_TYPE != common->sf2c_Type ) {

    return -1;
  }

  return ( WORD )(( struct SF2_Preset * ) common )->sf2p_Bank;
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

    struct SF2_Preset * preset = NULL;
    if ( 0 < i ) {

      preset = AllocVec( sizeof( struct SF2_Preset ), MEMF_ANY | MEMF_CLEAR );
      preset->sf2p_Common.sf2c_Type = SF2_COMMON_PRESET_TYPE;
      NEW_LIST( &( preset->sf2p_Common.sf2c_Zones ));
      ADD_TAIL( target, &( preset->sf2p_Common.sf2c_Node ));
      ReadString( fileHandle, preset->sf2p_Common.sf2c_Name );
      ReadUWORD( fileHandle, &( temp ));
      preset->sf2p_Common.sf2c_Number = Swap16( temp );
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
          ADD_TAIL( &( previousPreset->sf2p_Common.sf2c_Zones ), zone );
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

static LONG ReadBags( BPTR fileHandle,
                      LONG size,
                      struct MinList * target ) {

  LONG result;
  struct SF2_Common * common;
  struct SF2_Zone * previousZone = NULL;
  UWORD previousGeneratorIndex = 0;
  UWORD previousModulatorIndex = 0;

  FOR_LIST( target, common, struct SF2_Common * ) {

    struct SF2_Zone * zone;
    FOR_LIST( &( common->sf2c_Zones ), zone, struct SF2_Zone * ) {

      size -= BAG_CHUNK_SIZE_MULTIPLE;

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
  if ( BAG_CHUNK_SIZE_MULTIPLE != size ) {

    return EInvalidBags;
  }
  result = ReadZone( fileHandle,
                      &previousGeneratorIndex,
                      &previousModulatorIndex,
                      previousZone );
  return result;
}

static LONG ReadModulators( BPTR fileHandle,
                            LONG size,
                            struct MinList * target ) {

  LONG i = 0;
  struct SF2_Common * common;

  FOR_LIST( target, common, struct SF2_Common * ) {

    struct SF2_Zone * zone;
    FOR_LIST( &( common->sf2c_Zones ), zone, struct SF2_Zone * ) {

      struct SF2_Modulator * modulator;
      FOR_LIST( &( zone->sfz2_Modulators ),
                modulator,
                struct SF2_Modulator * ) {

        UWORD temp;

        size -= MOD_CHUNK_SIZE_MULTIPLE;
        if ( 0 > size ) {

          return EInvalidModulators;
        }
        ReadUWORD( fileHandle, &temp );
        modulator->sf2m_Source = Swap16( temp );
        ReadUWORD( fileHandle, &temp );
        modulator->sf2m_Target = Swap16( temp );
        ReadUWORD( fileHandle, &temp );
        modulator->sf2m_Amount = Swap16( temp );
        ReadUWORD( fileHandle, &temp );
        modulator->sf2m_AmountSource = Swap16( temp );
        ReadUWORD( fileHandle, &temp );
        modulator->sf2m_Transform = Swap16( temp );

        ++i;
      }      
    }
  }
  LOG_D(( "D: Read %ld modulators.\n", i ));

  if ( size ) {

    // This should be default!
    Seek( fileHandle, MOD_CHUNK_SIZE_MULTIPLE, OFFSET_CURRENT );
    size -= MOD_CHUNK_SIZE_MULTIPLE;
  }

  if ( size ) {

    return EInvalidModulators;
  }

  return ENoError;
}

static BOOL IsValidInstrumentGeneratorId( UWORD id ) {

  if ( GEN_OVERRIDEROOTKEY < id ) {

    return FALSE;
  }
  switch ( id ) {
    case GEN_UNUSED1:
    case GEN_UNUSED2:
    case GEN_UNUSED3:
    case GEN_UNUSED4:
    case GEN_RESERVED1:
    case GEN_RESERVED2:
    case GEN_RESERVED3:
    case GEN_INSTRUMENT: {

      return FALSE;
    }
    default: {

      return TRUE;
    }
  }
}

static BOOL IsValidPresetGeneratorId( UWORD id ) {

  if ( !( IsValidInstrumentGeneratorId( id ))) {

    return FALSE;
  }
  switch ( id ) {
    case GEN_STARTADDROFS:
    case GEN_ENDADDROFS:
    case GEN_STARTLOOPADDROFS:
    case GEN_ENDLOOPADDROFS:
    case GEN_STARTADDRCOARSEOFS:
    case GEN_ENDADDRCOARSEOFS:
    case GEN_STARTLOOPCOARSE:
    case GEN_KEYNUM:
    case GEN_VELOCITY:
    case GEN_ENDLOOPCOARSE:
    case GEN_SAMPLEMODE:
    case GEN_EXCLUSIVECLASS:
    case GEN_OVERRIDEROOTKEY:
    case GEN_SAMPLEID: {

      return FALSE;
    }
    default: {

      return TRUE;
    }
  }
}

static struct SF2_Generator * FindGeneratorById( struct MinList * generators,
                                                 const UWORD id ) {

  struct SF2_Generator * generator;

  FOR_LIST( generators, generator, struct SF2_Generator * ) {

    if ( id == generator->sf2g_Id ) {

      return generator;
    }
  }

  return NULL;
}

static LONG ReadGenerators( BPTR fileHandle,
                            LONG size,
                            struct MinList * target ) {

  ULONG generatorCount = 0;
  UWORD presetIndex = 0;
  struct SF2_Common * common;

  FOR_LIST( target, common, struct SF2_Common * ) {

    UWORD zoneIndex = 0;
    struct SF2_Zone * zone;

    LOG_V(( "V: Next Common\n" ));
    FOR_LIST( &( common->sf2c_Zones ), zone, struct SF2_Zone * ) {

      UWORD level = 0;
      struct SF2_Generator * generator;

      LOG_V(( "V: New Zone\n" ));
      FOR_LIST( &( zone->sfz2_Generators ),
                generator,
                struct SF2_Generator * ) {

        UWORD temp;

        size -= GEN_CHUNK_SIZE_MULTIPLE;
        if ( 0 > size ) {

          return EInvalidGenerators;
        }

        ReadUWORD( fileHandle, &temp );
        generator->sf2g_Id = Swap16( temp );
        LOG_V(( "V: New generator with ID 0x%04lx - swapped 0x%04lx = %ld\n",
                temp, generator->sf2g_Id, generator->sf2g_Id ));
        ReadUWORD( fileHandle, &temp );
        generator->sf2g_Amount = temp;

        ++generatorCount;

        switch ( generator->sf2g_Id ) {
          case GEN_KEYRANGE: {

            LOG_V(( "V: Handling key range generator.\n" ));

            if ( 0 == level ) {

              level = 1;

            } else {

              LOG_I(( "I: Discarding out of order KeyRange "
                      "in Common %ld-%ld / %s of zone %d\n",
                      GetBankFromCommon( common ),
                      common->sf2c_Number,
                      common->sf2c_Name,
                      zoneIndex ));
              Remove(( struct Node * ) generator );
              // TODO: fix memory leak here and at all Removes in this function!
            }
            break;
          }
          case GEN_VELRANGE: {

            LOG_V(( "V: Handling velocity range generator.\n" ));
            if ( 1 >= level ) {

              level = 2;
            
            } else {

              LOG_I(( "I: Discarding out of order VelRange "
                      "in Common %ld-%ld / %s of zone %d\n",
                      GetBankFromCommon( common ),
                      common->sf2c_Number,
                      common->sf2c_Name,
                      zoneIndex ));
              Remove(( struct Node * ) generator );
              // TODO: fix memory leak here and at all Removes in this function!
            }
            break;
          }
          case GEN_INSTRUMENT: {

            LOG_V(( "V: Handling instrument generator.\n" ));
            if ( SF2_COMMON_PRESET_TYPE == common->sf2c_Type ) {

              level = 3;

            } else {

              LOG_W(( "W: Only Presets shall have instruments!\n" ));
              LOG_I(( "I: Discarding generator %ld in %ld-%ld / %s of "
                      "zone %d for unusable ID\n",
                      generator->sf2g_Id,
                      GetBankFromCommon( common ),
                      common->sf2c_Number,
                      common->sf2c_Name,
                      zoneIndex ));
              Remove(( struct Node * ) generator );
            }
            break;
          }
          case GEN_SAMPLEID: {

            LOG_V(( "V: Handling sample generator.\n" ));
            if ( SF2_COMMON_INSTRUMENT_TYPE == common->sf2c_Type ) {

              level = 3;

            } else {

              LOG_W(( "W: Only Instruments shall have samples!\n" ));
              LOG_I(( "I: Discarding generator %ld in %ld-%ld / %s of "
                      "zone %d for unusable ID\n",
                      generator->sf2g_Id,
                      GetBankFromCommon( common ),
                      common->sf2c_Number,
                      common->sf2c_Name,
                      zoneIndex ));
              Remove(( struct Node * ) generator );
            }
            break;
          }
          default: {

            LOG_V(( "V: Handling other generator.\n" ));
            if ((( SF2_COMMON_PRESET_TYPE == common->sf2c_Type ) &&
                 ( IsValidPresetGeneratorId( generator->sf2g_Id )))
              || (( SF2_COMMON_INSTRUMENT_TYPE == common->sf2c_Type ) &&
                  ( IsValidInstrumentGeneratorId( generator->sf2g_Id )))) {

              struct SF2_Generator * duplicate = FindGeneratorById(
                &( zone->sfz2_Generators ),
                generator->sf2g_Id );
              if ( duplicate != generator ) {

                LOG_I(( "I: Duplicate generator 0x%08lx-%ld overwriting "
                        "0x%08lx-%ld in preset %ld-%ld / %s of zone %d, "
                        "only latest kept\n",
                        duplicate, 
                        duplicate->sf2g_Id,
                        generator,
                        generator->sf2g_Id,
                        GetBankFromCommon( common ),
                        common->sf2c_Number,
                        common->sf2c_Name,
                        zoneIndex ));
                duplicate->sf2g_Id = generator->sf2g_Id;
                duplicate->sf2g_Amount = generator->sf2g_Amount;
                Remove(( struct Node * ) generator );
                // TODO: fix memory leak here and at all Removes in this function!
              }

            } else {

              LOG_I(( "I: Discarding generator %ld in preset %ld-%ld / %s of "
                      "zone %d for unusable ID\n",
                      generator->sf2g_Id,
                      GetBankFromCommon( common ),
                      common->sf2c_Number,
                      common->sf2c_Name,
                      zoneIndex ));
              Remove(( struct Node * ) generator );
              // TODO: fix memory leak here and at all Removes in this function!
            }
            break;
          }
        }

        if ( 3 == level ) {

          generator = ( struct SF2_Generator * ) generator->sf2g_Node.mln_Succ;
          if ( generator->sf2g_Node.mln_Succ ) {

            LOG_W(( "W: Last level reached, some remaining, "
                    "maybe an issue, will clean up later.\n" ));
          }
          // End parsing generators for this zone,
          // remaining will be scrapped below!
          break;
        }
      }

      if (( 3 > level) &&
          ( zone != ( struct SF2_Zone * ) common->sf2c_Zones.mlh_Head )) {

        LOG_W(( "W: Discarding zone as global zone not appearing first!\n" ));
        // TODO: fix memory leak here and at all Removes in this function!
        Remove(( struct Node * ) zone );
        continue;
      }

      while ( generator->sf2g_Node.mln_Succ ) {

        Remove(( struct Node * ) generator );
        // TODO: fix memory leak here and at all Removes in this function!

        size -= GEN_CHUNK_SIZE_MULTIPLE;
        if ( 0 > size ) {

          return EInvalidGenerators;
        }
        Seek( fileHandle, GEN_CHUNK_SIZE_MULTIPLE, OFFSET_CURRENT );

        if ( generator->sf2g_Id ) {

          LOG_W(( "W: Discarding generator %ld in Common %ld-%ld / %s of "
                  "zone %d after sample ID\n",
                  generator->sf2g_Id,
                  GetBankFromCommon( common ),
                  common->sf2c_Number,
                  common->sf2c_Name,
                  zoneIndex ));

        } else {

          LOG_D(( "V: Discarding empty generator %ld in Common %ld-%ld / %s of "
                  "zone %d after sample ID\n",
                  generator->sf2g_Id,
                  GetBankFromCommon( common ),
                  common->sf2c_Number,
                  common->sf2c_Name,
                  zoneIndex ));
        }

        generator = ( struct SF2_Generator * ) generator->sf2g_Node.mln_Succ;
      }

      ++zoneIndex;
    }

    ++presetIndex;
  }
  if ( !size ) {
  
    return ENoError;
  }
  size -= GEN_CHUNK_SIZE_MULTIPLE;
  if ( size ) {

    return EInvalidGenerators;
  }
  Seek( fileHandle, GEN_CHUNK_SIZE_MULTIPLE, OFFSET_CURRENT );

  LOG_D(( "D: Read %ld generators.\n", generatorCount ));

  return ENoError;
}

static LONG ReadSamples( BPTR fileHandle,
                         LONG size,
                         struct MinList * target ) {

  LONG i;

  if (( size % SHDR_CHUNK_SIZE_MULTIPLE ) || ( !size )) {

    return EInvalidSampleSize;
  }

  size = ( size / SHDR_CHUNK_SIZE_MULTIPLE ) - 1;
  if ( !size ) {

    LOG_W(( "W: No samples available!\n" ));
    Seek( fileHandle, SHDR_CHUNK_SIZE_MULTIPLE, OFFSET_CURRENT );
    return ENoError;
  }

  for ( i = 0; i < size; ++i ) {

    union {
      ULONG l;
      UWORD w;
    } temp;
    struct SF2_Sample * sample = AllocVec( sizeof( struct SF2_Sample ),
                                           MEMF_ANY | MEMF_CLEAR );
    ADD_HEAD( target, sample );
    sample->sf2s_Number = i;
    ReadString( fileHandle, sample->sf2s_Name );
    ReadULONG( fileHandle, &temp.l );
    sample->sf2s_SampleStartOffset = Swap32( temp.l );
    ReadULONG( fileHandle, &temp.l );
    sample->sf2s_SampleEndOffset = Swap32( temp.l );
    ReadULONG( fileHandle, &temp.l );
    sample->sf2s_LoopStartOffset = Swap32( temp.l );
    ReadULONG( fileHandle, &temp.l );
    sample->sf2s_LoopEndOffset = Swap32( temp.l );
    ReadULONG( fileHandle, &temp.l );
    sample->sf2s_SampleRate = Swap32( temp.l );
    ReadUBYTE( fileHandle, &( sample->sf2s_SampleNote ));
    Seek( fileHandle, 3 /* pitch adjust + link */, OFFSET_CURRENT );
    ReadUWORD( fileHandle, &temp.w );
    sample->sf2s_SampleType = Swap16( temp.w );
  }
  Seek( fileHandle, SHDR_CHUNK_SIZE_MULTIPLE, OFFSET_CURRENT );

  return ENoError;
}

static LONG ReadInstrumentHeaders( BPTR fileHandle,
                                   LONG size,
                                   struct MinList * target ) {

  LONG i;
  UWORD temp;
  UWORD index;
  UWORD previousIndex = 0;
  struct SF2_Instrument * previousInstrument = NULL;

  if (( !( size )) || ( size % IHDR_CHUNK_SIZE_MULTIPLE )) {

    return EInvalidInstrumentDataSize;
  }

  i = ( size / IHDR_CHUNK_SIZE_MULTIPLE ) - 1;
  if ( 0 >= i ) {

    return ENoInstruments;
  }
  LOG_D(( "D: Found %ld ionstruments in %ld.\n", i + 1, size ));

  while ( 0 <= i ) {

    struct SF2_Instrument * instrument = NULL;
    if ( 0 < i ) {

      instrument = AllocVec( sizeof( struct SF2_Instrument ),
                             MEMF_ANY | MEMF_CLEAR );
      instrument->sf2i_Common.sf2c_Type = SF2_COMMON_INSTRUMENT_TYPE;
      NEW_LIST( &( instrument->sf2i_Common.sf2c_Zones ));
      ADD_TAIL( target, &( instrument->sf2i_Common.sf2c_Node ));
      ReadString( fileHandle, instrument->sf2i_Common.sf2c_Name );

    } else {

      Seek( fileHandle, IHDR_CHUNK_SIZE_MULTIPLE - 2, OFFSET_CURRENT );
    }
    ReadUWORD( fileHandle, &( temp ));
    index = Swap16( temp );
    
    if ( previousInstrument ) {

      // So >1st instrument
      if ( index > previousIndex ) {

        LONG h = index - previousIndex;
        LOG_V(( "V: Adding %ld zones to instrument %ld\n",
                h, ( size / IHDR_CHUNK_SIZE_MULTIPLE ) - 1 - i ));
        while ( h ) {

          struct SF2_Zone * zone = AllocVec( sizeof( struct SF2_Zone ),
                                             MEMF_ANY | MEMF_CLEAR );
          NEW_LIST( &( zone->sfz2_Generators ));
          NEW_LIST( &( zone->sfz2_Modulators ));
          ADD_TAIL( &( previousInstrument->sf2i_Common.sf2c_Zones ), zone );
          previousInstrument->sf2i_Common.sf2c_Number = previousIndex;
          --h;
        }
      } else {

        return EInvalidInstrumentIndex;
      }
    } else if ( index > 0 ) {
      LOG_W(( "W: %ld instrument zones unused!\n" ));
    }

    previousInstrument = instrument;
    previousIndex = index;
    --i;
  }

  return ENoError;
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
  LOG_D(( "D: After preset headers, preset size is %ld\n", size ));

  // Preset Bags
  result = ReadPresetSubChunk( sf2->sf2_FileHandle,
                               &( chunk ),
                               PBAG_CHUNK_ID,
                               BAG_CHUNK_SIZE_MULTIPLE,
                               &( size ));
  if ( result ) {

    return result;
  }
  result = ReadBags( sf2->sf2_FileHandle,
                     chunk.size,
                     &( sf2->sf2_Presets ));
  if ( result ) {

    return result;
  }
  LOG_D(( "D: After preset bags, preset size is %ld\n", size ));

  // Preset Modulators
  result = ReadPresetSubChunk( sf2->sf2_FileHandle,
                               &( chunk ),
                               PMOD_CHUNK_ID,
                               MOD_CHUNK_SIZE_MULTIPLE,
                               &( size ));
  if ( result ) {

    return result;
  }
  result = ReadModulators( sf2->sf2_FileHandle,
                           chunk.size,
                           &( sf2->sf2_Presets ));
  if ( result ) {

    return result;
  }
  LOG_D(( "D: After preset modulators, preset size is %ld\n", size ));

  // Preset Generators
  result = ReadPresetSubChunk( sf2->sf2_FileHandle,
                               &( chunk ),
                               PGEN_CHUNK_ID,
                               GEN_CHUNK_SIZE_MULTIPLE,
                               &( size ));
  if ( result ) {

    return result;
  }
  result = ReadGenerators( sf2->sf2_FileHandle,
                           chunk.size,
                           &( sf2->sf2_Presets ));
  if ( result ) {

    return result;
  }
  LOG_D(( "D: After preset generators, preset size is %ld\n", size ));

  // Instrument Headers
  result = ReadPresetSubChunk( sf2->sf2_FileHandle,
                               &( chunk ),
                               IHDR_CHUNK_ID,
                               IHDR_CHUNK_SIZE_MULTIPLE,
                               &( size ));
  if ( result ) {

    return result;
  }
  result = ReadInstrumentHeaders( sf2->sf2_FileHandle,
                                  chunk.size,
                                  &( sf2->sf2_Instruments ));
  if ( result ) {

    return result;
  }
  LOG_D(( "D: After instrument headers, preset size is %ld\n", size ));

  // Instrument Bags
  result = ReadPresetSubChunk( sf2->sf2_FileHandle,
                               &( chunk ),
                               IBAG_CHUNK_ID,
                               BAG_CHUNK_SIZE_MULTIPLE,
                               &( size ));
  if ( result ) {

    return result;
  }
  result = ReadBags( sf2->sf2_FileHandle,
                     chunk.size,
                     &( sf2->sf2_Instruments ));
  if ( result ) {

    return result;
  }
  LOG_D(( "D: After instrument bags, preset size is %ld\n", size ));

  // Instrument Modulators
  result = ReadPresetSubChunk( sf2->sf2_FileHandle,
                               &( chunk ),
                               IMOD_CHUNK_ID,
                               MOD_CHUNK_SIZE_MULTIPLE,
                               &( size ));
  if ( result ) {

    return result;
  }
  result = ReadModulators( sf2->sf2_FileHandle,
                           chunk.size,
                           &( sf2->sf2_Instruments ));
  if ( result ) {

    return result;
  }
  LOG_D(( "D: After instrument modulators, preset size is %ld\n", size ));

  // Instrument Generators
  result = ReadPresetSubChunk( sf2->sf2_FileHandle,
                               &( chunk ),
                               IGEN_CHUNK_ID,
                               GEN_CHUNK_SIZE_MULTIPLE,
                               &( size ));
  if ( result ) {

    return result;
  }
  result = ReadGenerators( sf2->sf2_FileHandle,
                           chunk.size,
                           &( sf2->sf2_Instruments ));
  if ( result ) {

    return result;
  }
  LOG_D(( "D: After instrument generators, preset size is %ld\n", size ));

  // Instrument Generators
  result = ReadPresetSubChunk( sf2->sf2_FileHandle,
                               &( chunk ),
                               SHDR_CHUNK_ID,
                               SHDR_CHUNK_SIZE_MULTIPLE,
                               &( size ));
  if ( result ) {

    return result;
  }
  result = ReadSamples( sf2->sf2_FileHandle,
                        chunk.size,
                        &( sf2->sf2_Samples ));
  if ( result ) {

    return result;
  }
  LOG_D(( "D: After sample headers, preset size is %ld\n", size ));

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
/*
  LOG_D(( "D: %ld is das gross\n",
sizeof( struct AmiSF_Data ) + ( 127 * ( sizeof( struct AmiSF_Note ) << 7 ))

));
*/
  Close( sf2->sf2_FileHandle );
  return NULL;
}

VOID FreeSf2( struct SF2_Parsed * soundFont ) {

}
