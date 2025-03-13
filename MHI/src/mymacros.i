        IFND MYMACROS_I
MYMACROS_I SET 1

        ifnd	NDEBUG
        XREF	_kprintf
        endc
        
        IFND _LVORawDoFmt
_LVORawDoFmt    	EQU     -522
        ENDC
        
_LVORawPutChar		EQU	-516	* Private function in Exec
_LVORawMayGetChar	EQU	-510
_AbsExecBase		EQU  	4
        
	XDEF _LVORawPutChar
	XDEF _LVORawDoFmt
	XDEF _LVORawMayGetChar
	XDEF _AbsExecBase
	
DEBUG	EQU	1

DBUG	macro
	ifnd	NDEBUG

	; save all regs
	movem.l	d0/d1/a0/a1,-(a7)
	IFGE	NARG-9
		move.l	\9,-(sp)		; stack arg8
	ENDC
	IFGE	NARG-8
		move.l	\8,-(sp)		; stack arg7
	ENDC
	IFGE	NARG-7
		move.l	\7,-(sp)		; stack arg6
	ENDC
	IFGE	NARG-6
		move.l	\6,-(sp)		; stack arg5
	ENDC
	IFGE	NARG-5
		move.l	\5,-(sp)		; stack arg4
	ENDC
	IFGE	NARG-4
		move.l	\4,-(sp)		; stack arg3
	ENDC
	IFGE	NARG-3
		move.l	\3,-(sp)		; stack arg2
	ENDC
	IFGE	NARG-2
		move.l	\2,-(sp)		; stack arg1
	ENDC

PULLSP	SET	(NARG)<<2		
	pea.l	.n1\@
	jsr		_debug_kvprintf
	lea.l	PULLSP(sp),sp			
	movem.l	(a7)+,d0/d1/a0/a1
	bra.s	.n2\@

.n1\@	dc.b	\1,0
	cnop	0,2
.n2\@
	endc
	endm

**********************************************************************************
*assorted low level assembly support routines used by the sample Library & Device*
**********************************************************************************
CLEAR   MACRO           ;quick way to clear a D register on 68000
        MOVEQ   #0,\1
        ENDM

LINKSYS MACRO           ; link to a library without having to see a _LVO
        MOVE.L  A6,-(SP)
        MOVE.L  \2,A6
        JSR     _LVO\1(A6)
        MOVE.L  (SP)+,A6
        ENDM

CALLSYS MACRO           ; call a library via A6 without having to see _LVO
        JSR     _LVO\1(A6)
        ENDM

XLIB    MACRO           ; define a library reference without the _LVO
**        XREF    _LVO\1
        ENDM

        ENDC ;MYMACROS_I
