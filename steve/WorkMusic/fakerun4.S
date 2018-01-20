_LVOAddICRVector	equ	-$6
_LVORemICRVector	equ	-$c

PALTimerSpeed		equ	14187		; 14187.58
PALAccur		equ	244

start:
	movem.l	d0-d7/a0-a6,-(a7)
	bsr	OpenTimer
	tst.l	CIABBase
	beq	.nores
	bsr	StartTimer
	bsr	StopTimer
.nores:
	movem.l	(a7)+,d0-d7/a0-a6
	moveq	#0,d0
	rts

_Interrupt	moveq	#0,d0
		rts
;----------------------------------------------------------------------------
OpenTimer       movem.l	a6,-(sp)		;returns 0 on failure
		move.l	4.w,a6
		lea	CIABRsrc,a1
		clr.l	d0
		jsr	-498(a6)		;_LVOOpenResource(a6)
		move.l	d0,CIABBase
		movem.l	(sp)+,a6
		rts

StartTimer	movem.l	a2/a6,-(sp)		;returns 0 on success

		move.w	#PALTimerSpeed,CIATimerSpeed

		lea	TimerRegisters,a2

		move.l	CIABBase,a6
		clr.l	d0
		move.l	d0,TimerTemp
		lea	TimerInterrupt,a1
		jsr	_LVOAddICRVector(a6)
		tst.l	d0
		beq.s	SetTimer

		lea	12(a2),a2
		moveq	#1,d0
		move.l	d0,TimerTemp
		lea	TimerInterrupt,a1
		jsr	_LVOAddICRVector(a6)
		tst.l	d0
		beq.s	SetTimer
		movem.l	(sp)+,a2/a6
		rts
SetTimer
		move.l	a2,_wTimer
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

TimerRegisters	dc.l	$bfde00,$bfd400,$bfd500
		dc.l	$bfdf00,$bfd600,$bfd700

TimerTemp:	dc.l	0

