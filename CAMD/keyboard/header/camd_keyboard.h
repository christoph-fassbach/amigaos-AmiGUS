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

#ifndef CAMD_KEYBOARD_H
#define CAMD_KEYBOARD_H

#include <dos/dos.h>
#include <exec/execbase.h>
#include <intuition/classes.h>
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
struct CAMD_Keyboard {

  /* Tool specific member variables */
  struct Screen               * ck_Screen;
  struct Window               * ck_MainWindow;
  APTR                          ck_VisualInfo;
  struct Gadget               * ck_GadgetList;
  ULONG                         ck_Flags;         /** Bitmask as per below  */ 

  struct Window               * ck_TestWindow;
  struct MsgPort              * ck_TestWindowPort;
  ULONG /*temporary - wrong type */ * ck_TestWindowHook;
  ULONG                         ck_TestWindowSignal;
  ULONG                         ck_TestContentSignal;
  Object                      * ck_TestWindowContent;

  /* AmiGUS specific member variables */

  /* CAMD pointers */

  BPTR                          ck_LogFile;       // Debug log file handle
  APTR                          ck_LogMem;        // Debug log memory blob
};

#define CAMD_TOOL_FLAG_REACTION_FAILED   0x00000001

/*
 * All libraries' base pointers used by the CAMD MIDI driver tool.
 */
extern struct CAMD_Keyboard     * CAMD_Keyboard_Base;
extern struct DosLibrary        * DOSBase;
extern struct Library           * GadToolsBase;
extern struct GfxBase           * GfxBase;
extern struct IntuitionBase     * IntuitionBase;
extern struct ExecBase          * SysBase;
/* Optional, opened if available, if not, no test window ;) */
extern struct Library           * BevelBase;
extern struct Library           * LabelBase;
extern struct Library           * LayoutBase;
extern struct Library           * ListBrowserBase;
extern struct Library           * VirtualBase;
extern struct Library           * WindowBase;

#endif /* CAMD_KEYBOARD_H */
