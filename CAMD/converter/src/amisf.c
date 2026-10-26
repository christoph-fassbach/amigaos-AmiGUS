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

#include <proto/dos.h>
#include <proto/exec.h>

#include "amisf.h"
#include "converter.h"
#include "debug.h"
#include "errors.h"
#include "support.h"

struct AmiSF * AllocAmiSFfromFile(
  STRPTR filePath,
  struct ProgressDialog * dialog
) {
// TODO: Use also for converter!
  LONG expected;
  LONG actual;
  UBYTE header[ 8 ];
  struct AmiSF * amisf = AllocMem( sizeof( struct AmiSF ),
                                   MEMF_ANY | MEMF_CLEAR );

  amisf->asf_SampleSourceFile = Open( filePath, MODE_OLDFILE );
  if ( !( amisf->asf_SampleSourceFile )) {

    FreeAmiSF( amisf );
    DisplayError( EOpenAmiSFreadFailed );
    return NULL;
  }


  expected = 8 * sizeof( UBYTE );
  actual = Read( amisf->asf_SampleSourceFile,
                 header,
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  if (( CHAR_TO_ULONG( 'A', 'm', 'i', 'S' ) != ( *(( LONG * ) &( header[ 0 ]))))
    || ( CHAR_TO_ULONG( 'F', 0, 0, 1 ) != ( *(( LONG * ) &( header[ 4 ]))))) {

    //TODO: handle error
    LOG_E(( "E: File handle shall now be at sample position %ld but is %ld!\n",
            expected, amisf->asf_SampleSourceOffset ));
    return NULL;
  }

  ///////////////////////////////////////////////////////////////////

  expected = 128 * 128 * sizeof( struct AmiSF_Preset );
  actual = Read( amisf->asf_SampleSourceFile,
                 &( amisf->asf_Preset ),
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  ///////////////////////////////////////////////////////////////////

  expected = 1 * sizeof( ULONG );
  actual = Read( amisf->asf_SampleSourceFile,
                 &( amisf->asf_NoteCount ),
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  expected = amisf->asf_NoteCount * sizeof( struct AmiSF_Note );
  amisf->asf_Note = AllocMem( expected,
                              MEMF_ANY | MEMF_CLEAR );
  if ( !( amisf->asf_Note )) {
    // TODO: handle error
  }
  actual = Read( amisf->asf_SampleSourceFile,
                 amisf->asf_Note,
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  ///////////////////////////////////////////////////////////////////

  expected = 1 * sizeof( ULONG );
  actual = Read( amisf->asf_SampleSourceFile,
                 &( amisf->asf_SampleCount ),
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  expected = amisf->asf_SampleCount * sizeof( struct AmiSF_Sample );
  amisf->asf_SampleMetadata = AllocMem( expected,
                                        MEMF_ANY | MEMF_CLEAR );
  if ( !( amisf->asf_SampleMetadata )) {
    // TODO: handle error
  }
  actual = Read( amisf->asf_SampleSourceFile,
                 amisf->asf_SampleMetadata,
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  ///////////////////////////////////////////////////////////////////

  expected = 1 * sizeof( ULONG );
  actual = Read( amisf->asf_SampleSourceFile,
                 &( amisf->asf_SampleRateCount ),
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  expected = amisf->asf_SampleRateCount * sizeof( ULONG );
  amisf->asf_SampleRate = AllocMem( expected,
                                    MEMF_ANY | MEMF_CLEAR );
  if ( !( amisf->asf_SampleRate )) {
    // TODO: handle error
  }
  actual = Read( amisf->asf_SampleSourceFile,
                 amisf->asf_SampleRate,
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  amisf->asf_PlaybackRateOffset = AllocMem( expected,
                                            MEMF_ANY | MEMF_CLEAR );
  if ( !( amisf->asf_PlaybackRateOffset )) {
    // TODO: handle error
  }
  actual = Read( amisf->asf_SampleSourceFile,
                 amisf->asf_PlaybackRateOffset,
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  ///////////////////////////////////////////////////////////////////

  expected = 1 * sizeof( ULONG );
  actual = Read( amisf->asf_SampleSourceFile,
                 &( amisf->asf_PlaybackRateCount ),
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  expected = amisf->asf_PlaybackRateCount * sizeof( ULONG );
  amisf->asf_PlaybackRate = AllocMem( expected,
                                    MEMF_ANY | MEMF_CLEAR );
  if ( !( amisf->asf_PlaybackRate )) {
    // TODO: handle error
  }
  actual = Read( amisf->asf_SampleSourceFile,
                 amisf->asf_PlaybackRate,
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  ///////////////////////////////////////////////////////////////////

  expected = 1 * sizeof( ULONG );

  actual = Read( amisf->asf_SampleSourceFile,
                 &( amisf->asf_SampleDataSize ),
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  actual = Read( amisf->asf_SampleSourceFile,
                 &( amisf->asf_SampleSourceOffset ),
                 expected );
  if ( actual != expected ) {
    // TODO: handle error
  }

  expected = Seek( amisf->asf_SampleSourceFile, OFFSET_CURRENT, 0 );
  if ( amisf->asf_SampleSourceOffset != expected ) {
    // TODO: handle error
    LOG_E(( "E: File handle shall now be at sample position %ld but is %ld!\n",
            expected, amisf->asf_SampleSourceOffset ));
    return NULL;
  }

  LOG_I(( "I: Read AmiSF with %ld notes, %ld samples, "
          "%ld sample and %ld playback rates, "
          "and %lu bytes of sample data starting at 0x%08lx.\n",
          amisf->asf_NoteCount,
          amisf->asf_SampleCount,
          amisf->asf_SampleRateCount,
          amisf->asf_PlaybackRateCount,
          amisf->asf_SampleDataSize,
          amisf->asf_SampleSourceOffset ));
  return amisf;
}

VOID FreeAmiSF( struct AmiSF * amisf ) {

  if ( !amisf ) {

    return;
  }
  if ( amisf->asf_SampleRate ) {

    FreeMem( amisf->asf_SampleRate,
             sizeof( ULONG ) * amisf->asf_SampleRateCount );
    amisf->asf_SampleRate = NULL;
  }
  if ( amisf->asf_PlaybackRateOffset ) {

    FreeMem( amisf->asf_PlaybackRateOffset,
             sizeof( ULONG ) * amisf->asf_SampleRateCount );
    amisf->asf_PlaybackRateOffset = NULL;
  }
  if ( amisf->asf_PlaybackRate ) {

    FreeMem( amisf->asf_PlaybackRate,
             sizeof( ULONG ) * amisf->asf_PlaybackRateCount );
    amisf->asf_PlaybackRate = NULL;
  }
  if ( amisf->asf_SampleMetadata ) {

    FreeMem( amisf->asf_SampleMetadata,
             sizeof( struct AmiSF_Sample ) * amisf->asf_SampleCount );
    amisf->asf_SampleMetadata = NULL;
  }
  if ( amisf->asf_Note ) {

    FreeMem( amisf->asf_Note,
             sizeof( struct AmiSF_Note ) * amisf->asf_NoteCount );
    amisf->asf_Note = NULL;
  }
  if ( amisf->asf_SampleSourceFile ) {

    Close( amisf->asf_SampleSourceFile );
    amisf->asf_SampleSourceFile = NULL;
  }

  FreeMem( amisf, sizeof( struct AmiSF ));
  LOG_D(( "D: Free'd AmiSF with %ld notes, %ld samples, %ld bytes sample data, "
          "%ld sample rates, and %ld playback rates.\n",
          amisf->asf_NoteCount,
          amisf->asf_SampleCount,
          -1,
          amisf->asf_SampleRateCount,
          amisf->asf_PlaybackRateCount ));
}
