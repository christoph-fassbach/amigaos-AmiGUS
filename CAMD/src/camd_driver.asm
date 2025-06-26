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


Ports	 equ 1

	section driver,data

* C references used here:
    *xref _AmiGUS_Name
    xref _AmiGUS_IDString

	xref _AmiGUS_Init
	xref _AmiGUS_Expunge
	xref _AmiGUS_OpenPort
	xref _AmiGUS_ClosePort
	xref _AmiGUS_ActivateXmit

* ASM exports:
	section driver,code

	xdef	_main

* Local definitions:
MDD_Magic	equ 'MDEV'

****************************************************************
*
*   Standard MIDI Device driver header
*
****************************************************************

; code at start of file in case anyone tries to execute us as a program

;	entry	FalseStart
_main:
FalseStart
	moveq	#-1,d0
	rts

MDD ; struct MidiDeviceData
	dc.l	MDD_Magic	; mdd_Magic
	dc.l	Name		; mdd_Name
	dc.l	_AmiGUS_IDString	; mdd_IDString
	dc.w	LIB_VERSION 	; mdd_Version
	dc.w	LIB_REVISION	; mdd_Revision
	dc.l	_AmiGUS_Init		; mdd_Init
	dc.l	_AmiGUS_Expunge 	; mdd_Expunge
	dc.l	_AmiGUS_OpenPort	; mdd_OpenPort
	dc.l	_AmiGUS_ClosePort	; mdd_ClosePort
	dc.b	Ports		; mdd_NPorts
	dc.b	1		; mdd_Flags

Name		dc.b    'amigus',0
*IDString	dc.b    'Idamigus',0

*
*	section driver,data	    ; data
*
*MPD0 ; struct MidiPortData
*	    dc.l    ActivateXmit
*
*Init
*	moveq	#1,d0			    ; return TRUE
*	rts
*
*Expunge
*	rts
*
*OpenPort
*	lea	MPD0,a0
*	move.l	a0,d0			    ; return pointer to MidiPortData
*	rts
*
*ClosePort
*	rts
*
*ActivateXmit
*	rts

	end