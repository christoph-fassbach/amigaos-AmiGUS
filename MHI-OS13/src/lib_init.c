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

#include <exec/types.h>
#include <intuition/intuitionbase.h>
#include <libraries/expansionbase.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include "amigus_codec.h"
#include "amigus_mhi.h"
#include "debug.h"
#include "errors.h"
#include "support.h"

#ifdef BASE_GLOBAL
struct ExecBase          * SysBase           = NULL;
struct DosLibrary        * DOSBase           = NULL;
struct IntuitionBase     * IntuitionBase     = NULL;
struct ExpansionBase     * ExpansionBase     = NULL;
struct Device            * TimerBase         = NULL;
struct AmiGUSmhi         * AmiGUSmhiBase     = NULL;
#endif

/* Closes all the libraries opened by LibInit() */
VOID CustomLibClose( struct AmiGUSmhi * amiGUSBase ) {

#ifndef BASE_GLOBAL
  struct ExecBase *SysBase = AmiGUSBase->agb_SysBase;
#endif

  if ( amiGUSBase->agb_TimerBase ) {

    CloseDevice( amiGUSBase->agb_TimerRequest );
  }
  if ( amiGUSBase->agb_TimerRequest ) {

    DeleteExtIO(( struct IORequest * ) amiGUSBase->agb_TimerRequest );
  }
  if ( amiGUSBase->agb_TimerPort ) {

    DeletePort( amiGUSBase->agb_TimerPort );
  }
  if ( amiGUSBase->agb_LogFile ) {

    Close( amiGUSBase->agb_LogFile );
  }
  /*
  Remember: memory cannot be overwritten if we do not return it. :)
  So... we leak it here... 
  if ( amiGUSBase->agb_LogMem ) {

    FreeMem( amiGUSBase->agb_LogMem, ... );
  }    
  */

  if ( amiGUSBase->agb_DOSBase ) {

    CloseLibrary(( struct Library * ) amiGUSBase->agb_DOSBase );
  }
  if ( amiGUSBase->agb_IntuitionBase ) {

    CloseLibrary(( struct Library * ) amiGUSBase->agb_IntuitionBase );
  }
  if ( amiGUSBase->agb_ExpansionBase ) {

    CloseLibrary(( struct Library * ) amiGUSBase->agb_ExpansionBase );
  }
}

LONG CustomLibInit( struct AmiGUSmhi * amiGUSBase, struct ExecBase * sysBase ) {

  LONG error;

#ifdef BASE_GLOBAL
  SysBase = sysBase;
#endif
  amiGUSBase->agb_SysBase = sysBase;

  LOG_I(( "1\n" ));
  /* Prevent use of customized library versions on CPUs not targetted. */
#ifdef _M68060
  if ( !( sysBase->AttnFlags & AFF_68060 )) {

    return EWrongDriverCPUVersion;
  }
#elif defined ( _M68040 )
  if ( !( sysBase->AttnFlags & AFF_68040 )) {

    return EWrongDriverCPUVersion;
  }
#elif defined ( _M68030 )
  if ( !( sysBase->AttnFlags & AFF_68030 )) {

    return EWrongDriverCPUVersion;
  }
#elif defined ( _M68020 )
  if ( !( sysBase->AttnFlags & AFF_68020 )) {

    return EWrongDriverCPUVersion;
  }
#endif

  amiGUSBase->agb_LogFile = NULL;
  amiGUSBase->agb_LogMem = NULL;
  LOG_I(( "2\n" ));
  amiGUSBase->agb_DOSBase =
    ( struct DosLibrary * ) OpenLibrary( "dos.library", 34 );
  if ( !( amiGUSBase->agb_DOSBase )) {

    return EOpenDosBase;
  }
  LOG_I(( "3\n" ));
  amiGUSBase->agb_IntuitionBase =
    ( struct IntuitionBase * ) OpenLibrary( "intuition.library", 34 );
  if ( !( amiGUSBase->agb_IntuitionBase )) {

    return EOpenIntuitionBase;
  }
  LOG_I(( "4\n" ));
  amiGUSBase->agb_ExpansionBase =
    ( struct ExpansionBase * ) OpenLibrary( "expansion.library", 34 );
  if ( !( amiGUSBase->agb_ExpansionBase )) {

    return EOpenExpansionBase;
  }
  LOG_I(( "5\n" ));
  amiGUSBase->agb_TimerPort = CreatePort( NULL, 0 );
  if ( !( amiGUSBase->agb_TimerPort )) {

    return EAllocateTimerPort;
  }
  LOG_I(( "5a\n" ));
  amiGUSBase->agb_TimerRequest = ( struct timerequest * )
    CreateExtIO( amiGUSBase->agb_TimerPort, sizeof( struct timerequest ));
  if ( !( amiGUSBase->agb_TimerRequest )) {

    return EAllocateTimerRequest;
  }
  LOG_I(( "6\n" ));
  error = OpenDevice( "timer.device", 0, amiGUSBase->agb_TimerRequest, 0 );
  if ( error ) {

    return EOpenTimerDevice;
  }
  amiGUSBase->agb_TimerBase = amiGUSBase->agb_TimerRequest->tr_node.io_Device;

#ifdef BASE_GLOBAL
  DOSBase       = amiGUSBase->agb_DOSBase;
  IntuitionBase = amiGUSBase->agb_IntuitionBase;
  ExpansionBase = amiGUSBase->agb_ExpansionBase;
  TimerBase     = amiGUSBase->agb_TimerBase;
  AmiGUSmhiBase = amiGUSBase;
#endif

  LOG_I(( "7\n" ));
  LOG_D(( "D: AmiGUS base ready @ 0x%08lx\n", amiGUSBase ));
  error = FindAmiGusCodec( amiGUSBase );
  if ( error ) {

    DisplayError( error );
  }

  LOG_I(( "8\n" ));
  return ENoError;
}
