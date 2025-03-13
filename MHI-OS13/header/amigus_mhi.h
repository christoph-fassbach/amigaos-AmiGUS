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

#ifndef AMIGUS_MHI_H
#define AMIGUS_MHI_H

/******************************************************************************
 * Library base structure components
 *****************************************************************************/

struct AmiGUSMhiBuffer {

  struct MinNode                agmb_Node;

  ULONG                       * agmb_Buffer;
  ULONG                         agmb_BufferIndex;
  ULONG                         agmb_BufferMax;
  ULONG                         agmb_BufferExtraBytes; // Only 0-3 - :)
};

struct AmiGUSClientHandle {
  struct Task                 * agch_Task;
  ULONG                         agch_Signal;

  struct MinList                agch_Buffers;
  struct AmiGUSMhiBuffer *      agch_CurrentBuffer;

  ULONG                         agch_Status;
};

/******************************************************************************
 * Library base structure
 *****************************************************************************/

/* This is the private structure. The official one does not contain all
the private fields! */
struct AmiGUSmhi {
  /* Library base stuff */
  struct BaseLibrary            agb_BaseLibrary;

  struct ExecBase             * agb_SysBase;
  struct DosLibrary           * agb_DOSBase;
  struct IntuitionBase        * agb_IntuitionBase;
  struct Library              * agb_ExpansionBase;

  struct Device               * agb_TimerBase;
  struct IORequest            * agb_TimerRequest;
  /* AmiGUS specific member variables */
  struct ConfigDev            * agb_ConfigDevice;
  APTR                          agb_CardBase;
  struct Interrupt            * agb_Interrupt;

  /* Client info */
  struct AmiGUSClientHandle     agb_ClientHandle;

  /* Only 1 AmiGUS supported per machine currently, sorry */
  BYTE                          agb_UsageCounter;
  UBYTE                         agb_Reserved0;
  UWORD                         agb_Reserved1;

  BPTR                          agb_LogFile;       /* Debug log file handle  */
  APTR                          agb_LogMem;        /* Debug log memory blob  */
};

#endif /* AMIGUS_MHI_H */
