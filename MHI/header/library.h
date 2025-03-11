/*
 * This file is part of the mhiAmiGUS.library.
 *
 * mhiAmiGUS.library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * mhiAmiGUS.library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with mhiAmiGUS.library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBRARY_H
#define LIBRARY_H

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/libraries.h>

#include <libraries/dos.h>

/* Need 2 staged helpers to concat version strings and ints together... :/   */
#define GSTR_HELPER( x ) #x
#define GSTR( x )        GSTR_HELPER( x )

/******************************************************************************
 * Define your library's public functions here,
 * will be used in library.c.
 *****************************************************************************/

#define LIB_FUNCTIONS     ( APTR ) MHIAllocDecoder, \
                          ( APTR ) MHIFreeDecoder, \
                          ( APTR ) MHIQueueBuffer, \
                          ( APTR ) MHIGetEmpty, \
                          ( APTR ) MHIGetStatus, \
                          ( APTR ) MHIPlay, \
                          ( APTR ) MHIStop, \
                          ( APTR ) MHIPause, \
                          ( APTR ) MHIQuery, \
                          ( APTR ) MHISetParam

/******************************************************************************
 * Define your library's properties here,
 * will be used in library.c.
 *****************************************************************************/
#define LIB_NAME          "mhiAmiGUS.library"
#define LIB_VERSION       1
#define LIB_REVISION      2
#define LIB_DATETXT       __AMIGADATE__
#define LIB_VERSTXT       GSTR( LIB_VERSION ) ".00" GSTR( LIB_REVISION )

#if defined( _M68060 )
  #define LIB_CPUTXT      " 060"
#elif defined( _M68040 )
  #define LIB_CPUTXT      " 040"
#elif defined( _M68030 )
  #define LIB_CPUTXT      " 030"
#elif defined( _M68020 )
  #define LIB_CPUTXT      " 020"
#elif defined( __MORPHOS__ )
  #define LIB_CPUTXT      " MorphOS"
#else
  #define LIB_CPUTXT      " 000"
#endif

#if defined( __VBCC__ )
  #define LIB_COMPILERTXT " vbcc"
#elif defined( __SASC )
  #define LIB_COMPILERTXT " SAS/C"
#endif

#ifdef CROSS_TOOLCHAIN
  #define LIB_HOSTTXT     " cross"
#else
  #define LIB_HOSTTXT     " native"
#endif

#define LIB_IDSTRING \
  LIB_NAME " " LIB_VERSTXT " " LIB_DATETXT \
  LIB_CPUTXT LIB_COMPILERTXT LIB_HOSTTXT

/******************************************************************************
 * SegList pointer definition
 *****************************************************************************/

#if defined(_AROS)
  typedef struct SegList * SEGLISTPTR;
#elif defined(__VBCC__)
  typedef APTR SEGLISTPTR;
#else
  typedef BPTR SEGLISTPTR;
#endif

/******************************************************************************
 * Library base structure - ALWAYS make it the first element (!!!)
 * in whatever your custom library's structure looks like.
 *****************************************************************************/
struct BaseLibrary {

  struct Library                LibNode;
  UWORD                         Unused0;                 /* better alignment */
  SEGLISTPTR                    SegList;
};

/******************************************************************************
 * Define your library's base type here, will be used in library.c.
 *****************************************************************************/
#define LIBRARY_TYPE      struct AmiGUSBase

/******************************************************************************
 * Your library's own base structure shall have its own include,
 * maybe together with your library specific functions.
 * Include it here!
 *****************************************************************************/
#include "amigus_mhi.h"

/******************************************************************************
 * Now go ahead and implement these functions in your library adapter code!
 *****************************************************************************/
LONG CustomLibInit( struct BaseLibrary * base, struct ExecBase * sysBase );
VOID CustomLibClose( struct BaseLibrary * base );

#endif /* LIBRARY_H */
