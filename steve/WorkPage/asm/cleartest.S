	INCDIR	"wp:inclibs/"
	include	"exec_lib.i"

start:
	move.l	$4.w,a6
	move.l	#1000000,d0
	move.l	#0,d1
	jsr	_LVOAllocMem(a6)
	move.l	d0,mem
	beq	.nomem

	moveq	#0,d2
	moveq	#0,d3
	moveq	#0,d4
	moveq	#0,d5
.repp1:
	move.l	#$100,d1
	move.l	mem(pc),a0
	move.l	#1000000,d0
;	add.l	d0,a0
.repp:
	clr.l	(a0)+
;	movem.l	d2/d3,-(a0)
	subq.l	#4,d0
	bpl	.repp
	dbf	d1,.repp1

	move.l	mem(pc),a1
	move.l	#1000000,d0
	jsr	_LVOFreeMem(a6)
.nomem:
	moveq	#0,d0
	rts

mem:	dc.l	0
