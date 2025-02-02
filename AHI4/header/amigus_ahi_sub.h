/*
 * This file is part of the AmiGUS.audio driver.
 *
 * AmiGUS.audio driver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License only.
 *
 * AmiGUS.audio driver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU LesserGeneral Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with AmiGUS.audio driver.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AMIGUS_AHI_H
#define AMIGUS_AHI_H

/* 
* AHI driver library header.
*
* To be used only internally - but there in all .c files!
* If you are using some of the library base addresses from
* BASE_REDEFINE directly, e.g. in libinit.c,
* do a #define NO_BASE_REDEFINE before including this file.
*/

/* Activate / De-activate this define to toggle lib base mode! */
#define BASE_GLOBAL /**/

#ifndef BASE_GLOBAL 
#ifndef NO_BASE_REDEFINE
/* 
    either this is active for everything except libinit.c
    or BASE_GLOBAL is active everywhere
*/
#define BASE_REDEFINE
#endif
#endif

#include "copies.h"
#include "library.h"
#include "SDI_ahi_sub.h"

#define AMIGUS_AHI_AUTHOR           "Christoph `Chritoph` Fassbach"
#define AMIGUS_AHI_COPYRIGHT        "(c) 2024 Christoph Fassbach / LGPL3"
#define AMIGUS_AHI_ANNOTATION       "Thanks to: Oliver Achten (AmiGUS), " \
                                    "Frank Wille (vbcc), Martin Blom (AHI)"
#define AMIGUS_AHI_VERSION          LIBRARY_IDSTRING

#define AMIGUS_AHI_FIRMWARE_MINIMUM ( ( 2024 << 20 ) /* year   */ \
                                    + (   12 << 16 ) /* month  */ \
                                    + (    8 << 11 ) /* day    */ \
                                    + (   22 <<  6 ) /* hour   */ \
                                    + (   38 <<  0 ) /* minute */ )

#define AHIDB_AmiGUS_PlayCopyFunction   ( AHIDB_UserBase +  0 )
#define AHIDB_AmiGUS_PlayHwSampleId     ( AHIDB_UserBase +  1 )
#define AHIDB_AmiGUS_PlaySampleShift    ( AHIDB_UserBase +  2 )
#define AHIDB_AmiGUS_RecCopyFunction    ( AHIDB_UserBase +  8 )
#define AHIDB_AmiGUS_RecHwSampleId      ( AHIDB_UserBase +  9 )
#define AHIDB_AmiGUS_RecSampleShift     ( AHIDB_UserBase + 10 )

#define AMIGUS_MEM_LOG_MARKER        "********************************"   \
                                     " AmiGUS "                           \
                                     "********************************\n"

/******************************************************************************
 * Library base structure components
 *****************************************************************************/

/** A single AmiGUS Board representation - owned by the AmiGUSBase driver    */
struct AmiGUSPcmCard {
  struct MinNode                agpc_Node;
  APTR                          agpc_CardBase;      /* Card's base address   */
  struct AHIAudioCtrlDrv      * agpc_PlaybackCtrl;  /* Attached play client  */
  struct AHIAudioCtrlDrv      * agpc_RecordingCtrl; /* Attached rec client   */
  ULONG                         agpc_StateFlags;    /* AmiGUS state as below */
};

/** Playback sub-structure of the driver data                                */
struct AmiGUSPcmPlayback {
  struct AmiGUSPcmCard        * agpp_Card;           /* Attached card        */
  /* Mixing/playback double-buffers to be copied to FIFO alternatingly       */
  ULONG                       * agpp_Buffer[2];      /* Fully LONG aligned!  */
  ULONG                         agpp_BufferIndex[2]; /* Next LONG index each */
  ULONG                         agpp_BufferMax[2];   /* LONGs high mark each */
  ULONG                         agpp_CurrentBuffer;  /* Current playing buf. */

  ULONG                         agpp_Watermark;      /* Counting in WORDs!   */

  CopyFunctionType              agpp_CopyFunction;   /* Magic AHI<->AmiGUS.. */
  ULONG                         agpp_CopyFunctionId; /* ID of CopyFunction   */

  UWORD                         agpp_HwSampleFormatId;   /* Sample format ID */
  UBYTE                         agpp_AhiSampleShift; /* Sample <> Byte shift */
  UBYTE                         agpp_Reserved0;      /* for alignment        */
};

/** Recording sub-structure of the driver data                               */
struct AmiGUSPcmRecording {
  struct AmiGUSPcmCard        * agpr_Card;           /* Attached card        */
  /* Recording double-buffers filled from FIFO/emptied by AHI alternatingly  */
  ULONG                       * agpr_Buffer[2];      /* Fully LONG aligned!  */
  ULONG                         agpr_BufferIndex[2]; /* Next LONG index each */
  ULONG                         agpr_BufferMax[2];   /* LONGs high mark each */
  ULONG                         agpr_CurrentBuffer;  /* Current recording b. */

  /* NULL if recording not supported !!! */
  CopyFunctionType              agpr_CopyFunction;   /* Magic AmiGUS<->AHI.. */
  ULONG                         agpr_CopyFunctionId; /* ID of CopyFunction   */

  struct AHIRecordMessage       agpr_RecordingMessage;

  UWORD                         agpr_HwSampleFormatId;   /* Sample format ID */
  UBYTE                         agpr_AhiSampleShift; /* Sample <> Byte shift */
  UBYTE                         agpr_HwSourceId;          /* Input source ID */
};

/** Driver data - owned by AHIAudioCtrlDrv->ahiac_DriverData                 */
struct AmiGUSAhiDriverData {
  struct AmiGUSPcmPlayback      agdd_Playback;       /* Playback vars group  */
  struct AmiGUSPcmRecording     agdd_Recording;      /* Recording vars group */
  UWORD                         agdd_HwSampleRateId; /* HW sample rate ID    */
  UWORD                         agdd_Reserved0;      /* for alignment        */
};

/******************************************************************************
 * Library base structure
 *****************************************************************************/

struct AmiGUSBase {
  /* Library base stuff */
  struct BaseLibrary            agb_BaseLibrary;

  /* Library dependencies */
  struct ExecBase             * agb_SysBase;
  struct DosLibrary           * agb_DOSBase;
  struct IntuitionBase        * agb_IntuitionBase;
  struct Library              * agb_UtilityBase;
  struct Library              * agb_ExpansionBase;

  /* List of driver compatible boards found in the system */
  struct MinList                agb_CardList;

  /* Interrupt, main/worker process and messaging members */
  struct Interrupt            * agb_Interrupt;        /*                     */
  struct Process              * agb_MainProcess;      /*                     */
  struct Process              * agb_WorkerProcess;    /*                     */
  LONG                          agb_WorkerReady;      /*                     */
  BYTE                          agb_MainSignal;       /*                     */
  BYTE                          agb_WorkerWorkSignal; /*                     */
  BYTE                          agb_WorkerStopSignal; /*                     */
  UBYTE                         agb_Reserved0;        /* for alignment       */

  /* Debugging / logging members */
  struct Device               * agb_TimerBase;
  struct IORequest            * agb_TimerRequest;
  BPTR                          agb_LogFile;        /* Debug log file handle */
  APTR                          agb_LogMem;         /* Debug log memory blob */
};

#if defined(BASE_GLOBAL)
  extern struct AmiGUSBase        * AmiGUSBase;
  extern struct DosLibrary        * DOSBase;
  extern struct Library           * ExpansionBase;
  extern struct IntuitionBase     * IntuitionBase;
  extern struct ExecBase          * SysBase;
  extern struct Device            * TimerBase;
  extern struct Library           * UtilityBase;
#elif defined(BASE_REDEFINE)
  #define AmiGUSBase                (amiGUSBase)
  #define DOSBase                   amiGUSBase->agb_DOSBase
  #define ExpansionBase             amiGUSBase->agb_ExpansionBase
  #define IntuitionBase             amiGUSBase->agb_IntuitionBase
  #define SysBase                   amiGUSBase->agb_SysBase
  #define TimerBase                 amiGUSBase->TimerBase
  #define UtilityBase               amiGUSBase->agb_UtilityBase
#endif

/******************************************************************************
 * Library flag definitions
 *****************************************************************************/

#define AMIGUS_AHI_F_PLAY_STARTED        0x01
#define AMIGUS_AHI_F_PLAY_UNDERRUN       0x02
#define AMIGUS_AHI_F_PLAY_0              0x04
#define AMIGUS_AHI_F_PLAY_1              0x08
#define AMIGUS_AHI_F_REC_STARTED         0x10
#define AMIGUS_AHI_F_REC_0               0x20
#define AMIGUS_AHI_F_REC_1               0x40
#define AMIGUS_AHI_F_REC_2               0x80

#define AMIGUS_AHI_F_PLAY_MASK           0x0F
#define AMIGUS_AHI_F_PLAY_STOP_MASK      0xF0
#define AMIGUS_AHI_F_REC_MASK            0xF0
#define AMIGUS_AHI_F_REC_STOP_MASK       0x0F

#endif /* AMIGUS_AHI_H */
