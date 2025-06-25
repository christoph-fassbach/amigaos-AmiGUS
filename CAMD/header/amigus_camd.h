/*
 * This file is part of the AmiGUS CAMD MIDI driver.
 *
 * AmiGUS CAMD MIDI driver is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS CAMD MIDI driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with AmiGUS CAMD MIDI driver.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AMIGUS_CAMD_H
#define AMIGUS_CAMD_H

/* Activate / De-activate this define to toggle lib base mode! */
#define BASE_GLOBAL /**/

#ifndef BASE_GLOBAL 
#ifndef NO_BASE_REDEFINE
/* 
 * either this is active for everything except libinit.c
 * or BASE_GLOBAL is active everywhere
 */
#define BASE_REDEFINE
#endif
#endif

#include "library.h"

/*
 * Minimum firmware required to use this version of the CAMD MIDI driver,
 * e.g. the codec part's timer came late in the process to ditch
 * the AmigaOS 2.0/v36 requirement.
 */
#define AMIGUS_CAMD_FIRMWARE_MINIMUM ( ( 2025 << 20 ) /* year   */ \
                                     + (    3 << 16 ) /* month  */ \
                                     + (   24 << 11 ) /* day    */ \
                                     + (   21 <<  6 ) /* hour   */ \
                                     + (   38 <<  0 ) /* minute */ )

/*
 * If logging to memory is activated, this is used to mark the start
 * of the log memory. And as 1 pointer to this marker, 1 pointer to the
 * library file name and 1 more pointer to the marker are used, the full
 * start marker is not even in the library, no need to unload the library
 * so even with library in memory it should unique in memory.
 */
#define AMIGUS_MEM_LOG_BORDERS      "********************************"

/******************************************************************************
 * Library base structure
 *****************************************************************************/

/**
 * Private AmiGUS CAMD MIDI library base structure.
 *
 * There is no public one, pointers to libraries opened, interrupts,
 * list of client handles, logs. Nothing to play around with.
 */
struct AmiGUS_CAMD {
  /* Library base stuff */
  struct BaseLibrary            agb_BaseLibrary;   // Instance of library.h

  struct ExecBase             * agb_SysBase;       // Exec, allocations etc.
  struct DosLibrary           * agb_DOSBase;       // DOS, logs and so on
  struct IntuitionBase        * agb_IntuitionBase; // For error messages
  struct Library              * agb_ExpansionBase; // Finding devices

  /* AmiGUS specific member variables */

  BPTR                          agb_LogFile;       // Debug log file handle
  APTR                          agb_LogMem;        // Debug log memory blob
};

/*
 * All libraries' base pointers used by the CAMD MIDI driver library.
 * Also used to switch between relying on globals or not.
 */
#if defined(BASE_GLOBAL)
  extern struct AmiGUS_CAMD       * AmiGUS_CAMD_Base;
  extern struct DosLibrary        * DOSBase;
  extern struct Library           * ExpansionBase;
  extern struct IntuitionBase     * IntuitionBase;
  extern struct ExecBase          * SysBase;
#elif defined(BASE_REDEFINE)
  #define AmiGUS_CAMD_Base          (base)
  #define DOSBase                   base->agb_DOSBase
  #define ExpansionBase             base->agb_ExpansionBase
  #define IntuitionBase             base->agb_IntuitionBase
  #define SysBase                   base->agb_SysBase
#endif

#endif /* AMIGUS_CAMD_H */