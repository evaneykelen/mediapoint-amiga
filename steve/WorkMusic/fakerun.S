	incdir	"wp:inclibs/"
	include	"exec_lib.i"

_ExecBase = 4
_LVOAddICRVector	equ	-$6
_LVORemICRVector	equ	-$c

PALTimerSpeed		equ	14187		; 14187.58
NTSCTimerSpeed		equ	14318		; 14318.18
PALAccur		equ	244
NTSCAccur		equ	795

	xdef   _FakeRun	; Added 10-12-1993 KC.

_FakeRun:
	movem.l	d0-d7/a0-a6,-(a7)
	lea	fakemod(pc),a0
	move.l	a0,_CurrentMod
	bsr	OpenTimer
	bsr	StartTimer
	bsr	StopTimer
	movem.l	(a7)+,d0-d7/a0-a6
	moveq	#0,d0
	rts

; 1 on the 244 times add one by the time value
; NTSC 795 

_Interrupt	movem.l	d1-d7/a0-a6,-(sp)
		move.w	CIATimerSpeed(pc),d0
		addq.l	#1,_OffsetAdjust
		move.l	_AccurValue(pc),d1
		move.l	_OffsetAdjust(pc),d2
		cmp.l	d1,d2			; PAL 244
		blt	no_add1
		addq.l	#1,d1
		cmp.l	d1,d2			; PAL 245
		blt	no_add2

		move.l	#1,_OffsetAdjust
		bra	do_add1
no_add2:
		sub.w	#100,d0
;		move.w	#$f00,$dff180
do_add1:
		move.l	_wTimer(pc),a2
		move.l	4(a2),a0
		move.b	d0,(a0)
		move.l	8(a2),a0
		lsr.w	#8,d0
		move.b	d0,(a0)
		
no_add1:	move.l	_CurrentMod,a0
		move.l  12(a0),a0
;		move.w	#$f0f,$dff180
		jsr	(a0)
;		move.w	#$000,$dff180
		movem.l	(sp)+,d1-d7/a0-a6
		moveq	#0,d0
		rts

*
* Fake interrupt function
*
fake_init:
	move.l	#0,d0
	rts	

_CurrentMod	dc.l	0
_OffsetAdjust:	dc.l	0
_AccurValue:	dc.l	0

KillAudio	movem.l	a6,-(sp)
		lea	IOAudio(pc),a1
		move.l	_ExecBase,a6
		jsr	_LVOCloseDevice(a6)
		movem.l	(sp)+,a6
		rts

IOAudio
ioa_Request
io_Message
mn_Node
ln_Succ		dc.l	0
ln_Pred		dc.l	0
ln_Type		dc.b    0
ln_Pri		dc.b    $7f
ln_Name		dc.l	0
mn_ReplyPort	dc.l	0
mn_Length	dc.w	0
io_Device	dc.l	0
io_Unit		dc.l	0
io_Command	dc.w	0
io_Flags	dc.b	0
io_Error	dc.b	0
ioa_AllocKey	dc.w	0
ioa_Data	dc.l    TempDat
ioa_Length	dc.l	1
ioa_Period	dc.w	0
ioa_Volume	dc.w	0
ioa_Cycles	dc.w	0
ioa_WriteMsg	ds.l    5
TempDat		dc.w	$f00

;----------------------------------------------------------------------------
OpenTimer       movem.l	a6,-(sp)		;returns 0 on failure
		move.l	_ExecBase,a6
		lea	CIABRsrc,a1
		clr.l	d0
		jsr	_LVOOpenResource(a6)
		move.l	d0,CIABBase
		movem.l	(sp)+,a6
		rts

StartTimer	movem.l	a2/a6,-(sp)		;returns 0 on success
		move.l	_ExecBase,a6
		cmpi.b	#60,530(a6)
		beq.s   NTSC
		move.w	#PALTimerSpeed,CIATimerSpeed
		move.l	#PALAccur,_AccurValue
		bra.s	GTcont
NTSC		move.w	#NTSCTimerSpeed,CIATimerSpeed
		move.l	#NTSCAccur,_AccurValue
GTcont		lea	TimerRegisters,a2

ww1:
		moveq	#0,d0
		move.b	$dff006,d0
		and.b	#$ff,d0
		cmp.b	#100,d0
		ble	ww1
		cmp.b	#120,d0
		bge	ww1

		move.l	CIABBase,a6
		clr.l	d0
		move.l	d0,TimerTemp
		lea	TimerInterrupt,a1
		jsr	_LVOAddICRVector(a6)
		tst.l	d0
		beq.s	SetTimer

ww2:
		moveq	#0,d0
		move.b	$dff006,d0
		and.b	#$ff,d0
		cmp.b	#100,d0
		ble	ww2
		cmp.b	#120,d0
		bge	ww2
		
		lea	12(a2),a2
		moveq	#1,d0
		move.l	d0,TimerTemp
		lea	TimerInterrupt,a1
		jsr	_LVOAddICRVector(a6)
		tst.l	d0
		beq.s	SetTimer
		movem.l	(sp)+,a2/a6
		rts

SetTimer	move.l	a2,_wTimer
		move.l	(a2),a0
		move.b	#%10001,(a0)
		move.w	CIATimerSpeed,d1
		move.l	4(a2),a0
		move.b	d1,(a0)
		move.l	8(a2),a0
		lsr.w	#8,d1
		move.b	d1,(a0)
		clr.l	d0
		movem.l	(sp)+,a2/a6
		rts

StopTimer	movem.l	a6,-(sp)		;no return value
		move.l	CIABBase,a6
		move.l	TimerTemp,d0
		lea	TimerInterrupt,a1
		jsr	_LVORemICRVector(a6)
		movem.l	(sp)+,a6
		rts


_wTimer		dc.l	0
CIATimerSpeed	dc.w	0
CIABBase	dc.l	0
CIABRsrc	dc.b	'ciab.resource',0
InterruptName	dc.b	'MLP_interrupt',0
  even

TimerInterrupt	dc.l	0
		dc.l	0
		dc.b	4
		dc.b	0
		dc.l	InterruptName
		dc.w	0
		dc.w	0
		dc.l	_Interrupt

TimerTemp	dc.l	0

TimerRegisters	dc.l	$bfde00,$bfd400,$bfd500
			dc.l	$bfdf00,$bfd600,$bfd700

fakemod:	dc.b	'    '
	dc.l	fake_init
	dc.l	fake_init
	dc.l	fake_init
	dc.b	' '
	even
