	IFND LIBRARIES_AMIGUS_I
LIBRARIES_AMIGUS_I	SET	1

**
**	$VER: amigus.i 1.0 (10.08.2026)
**	:ts=8 (TAB SIZE: 8)
**
**	Structures and codes for amigus.library
**      based on amigus.h, converted by H.Richter
**
**	amigus.library is free software: you can redistribute it and/or modify
**      it under the terms of the GNU Lesser General Public License as published 
**      by the Free Software Foundation, version 3 of the License only.
**
**

*------------------------------------------------------------------------*

	IFND EXEC_TYPES_I
	INCLUDE 'exec/types.i'
	ENDC

*------------------------------------------------------------------------*

*** AmiGUS_TypeIds
AmiGUS_Zorro2   EQU     $7000   ;original Zorro-II card
AmiGUS_mini     EQU     $7001   ;PCMCIA card

*** Flags defining all functional blocks / parts of an AmiGUS card.
AMIGUS_FLAG_NONE        EQU     $0000   ;< No part ;)
AMIGUS_FLAG_PCM         EQU     $0001   ; PCM like main part, incl. mixer
AMIGUS_FLAG_WAVETABLE   EQU     $0002   ; Wavetable part
AMIGUS_FLAG_CODEC       EQU     $0004   ; Codec part, for MP3, FLAC, ...

*** amigus.library error codes as returned by library interface functions.
AMIGUS_IN_USE_START     EQU     $100

AmiGUS_NoError                EQU 0
AmiGUS_InUse                  EQU AMIGUS_IN_USE_START
AmiGUS_PcmInUse               EQU AMIGUS_IN_USE_START                                             | AMIGUS_FLAG_PCM
AmiGUS_WavetableInUse         EQU AMIGUS_IN_USE_START                     | AMIGUS_FLAG_WAVETABLE                  
AmiGUS_PcmWavetableInUse      EQU AMIGUS_IN_USE_START                     | AMIGUS_FLAG_WAVETABLE | AMIGUS_FLAG_PCM
AmiGUS_CodecInUse             EQU AMIGUS_IN_USE_START | AMIGUS_FLAG_CODEC                                          
AmiGUS_PcmCodecInUse          EQU AMIGUS_IN_USE_START | AMIGUS_FLAG_CODEC                         | AMIGUS_FLAG_PCM
AmiGUS_WavetableCodecInUse    EQU AMIGUS_IN_USE_START | AMIGUS_FLAG_CODEC | AMIGUS_FLAG_WAVETABLE                  
AmiGUS_PcmWavetableCodecInUse EQU AMIGUS_IN_USE_START | AMIGUS_FLAG_CODEC | AMIGUS_FLAG_WAVETABLE | AMIGUS_FLAG_PCM
AmiGUS_NotYours               EQU $200
AmiGUS_DetectError            EQU $401
AmiGUS_InterruptInstallFailed EQU $402
AmiGUS_InterruptRemoveFailed  EQU $403
AmiGUS_NotFound               EQU $404
AmiGUS_NotImplemented         EQU $500


*** STRUCTUES

**
** AmiGUS card description as returned by amigus.library/AmiGUS_FindCard().
**
** No need to free it, ownership stays with amigus.library.
** Consider ALL fields read-only, please.
**
	STRUCTURE AmiGUS,0
        APTR      agus_PcmBase          ;Base address of the PCM part of the card.
        APTR      agus_WavetableBase    ;Base address of the Wavetable part.
        APTR      agus_CodecBase        ;Base address of the codec part.
        STRUCT    agus_FpgaId,8         ;Hardware ID of the cards FPGA - in UBYTEs.
        ULONG     agus_HardwareRev      ;Hardware revision of the card.
        ULONG     agus_FirmwareRev      ;Firmware revision of the card.
        APTR      agus_TypeName         ;Human readable card type string.
        UWORD     agus_TypeId           ;Card type from enum AmiGUS_TypeIds.
        UWORD     agus_Year             ;Firmware date, year portion.
        UBYTE     agus_Month            ;Firmware date, month portion.
        UBYTE     agus_Day              ;Firmware date, day portion.
        UBYTE     agus_Hour             ;Firmware date, hour portion.
        UBYTE     agus_Minute           ;Firmware date, minute portion.
        LABEL     AmiGUS_SIZEOF

	ENDC ; LIBRARIES_AMIGUS_I
