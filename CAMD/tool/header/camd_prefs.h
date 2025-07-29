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

#ifndef CAMD_PREFS_H
#define CAMD_PREFS_H

#include <dos/dos.h>
#include <exec/execbase.h>
#include <intuition/intuitionbase.h>

/******************************************************************************
 * Define the tool's properties here,
 * will be used in camd_amigus.c.
 *****************************************************************************/

#define STR_VALUE(x)      #x
#define STR(x)            STR_VALUE(x)

#define APP_IDSTRING      STR( APP_FILE )" "                         \
                          STR( APP_VERSION )".00"STR( APP_REVISION ) \
                          " "APP_DATE" "STR( APP_CPU )" "            \
                          STR( APP_COMPILER )" "STR( APP_HOST )

/*
 * If logging to memory is activated, this is used to mark the start
 * of the log memory. And as 1 pointer to this marker, 1 pointer to the
 * library file name and 1 more pointer to the marker are used, the full
 * start marker is not even in the library, no need to unload the library
 * so even with library in memory it should unique in memory.
 */
#define AMIGUS_MEM_LOG_BORDERS      "********************************"

/******************************************************************************
 * Tool base structure
 *****************************************************************************/
/**
 * Private AmiGUS CAMD MIDI library base structure.
 *
 * There is no public one, pointers to libraries opened, interrupts,
 * list of client handles, logs. Nothing to play around with.
 */
struct AmiGUS_CAMD_Tool {

  /* Tool specific member variables */
  struct Screen               * agt_Screen;
  struct Window               * agt_MainWindow;
  APTR                          agt_VisualInfo;
  struct Gadget               * agt_GadgetList;
  ULONG                         agt_Flags;         /** Bitmask as per below  */ 
  struct Window               * agt_TestWindow;

  /* AmiGUS specific member variables */

  /* CAMD pointers */

  BPTR                          agt_LogFile;       // Debug log file handle
  APTR                          agt_LogMem;        // Debug log memory blob
};

#define CAMD_TOOL_FLAG_REACTION_FAILED   0x00000001

/*
 * All libraries' base pointers used by the CAMD MIDI driver tool.
 */
extern struct AmiGUS_CAMD_Tool  * AmiGUS_CAMD_Tool_Base;
extern struct DosLibrary        * DOSBase;
extern struct Library           * GadToolsBase;
extern struct IntuitionBase     * IntuitionBase;
extern struct ExecBase          * SysBase;
/* Optional, opened if available, if not, no test window ;) */
extern struct Library           * LabelBase;
extern struct Library           * LayoutBase;
extern struct Library           * ListBrowserBase;
extern struct Library           * WindowBase;

#endif /* CAMD_PREFS_H */
