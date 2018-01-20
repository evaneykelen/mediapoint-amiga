*
* probeer een lijn te trekken
*
draw:
	movem.l	d0-d7/a2/a3,-(a7)
	move.l	#$10001,d4
;	move.l	#$10001,d5

	bsr.L	fastplot

	sub.w	d0,d2
	bcc.s	x2tox1
	neg.w	d2
	neg.w	d4
x2tox1:
	swap	d4
	sub.w	d1,d3
	bcc.s	y2toy1
	neg.w	d3
	neg.w	d4
y2toy1:
	move.w	d2,d7
	or.w	d3,d7
	beq.s	enddraw

	move.l	d4,d5

	cmp.w	d2,d3
	bcs.s	xbig

	and.l	#$0000ffff,d5

	exg	d2,d3
	bra.s	ybig

xbig:	and.l	#$ffff0000,d5
ybig:	moveq	#0,d7
	move.w	d2,d7
	subq.w	#1,d7
	move.w	d2,d6
	lsr.w	#1,d6
	move.w	d6,a2
	move.w	d3,a3

next:	move.w	a2,d3
	add.w	a3,d3
	bcs.s	diagonal
	cmp.w	d2,d3
	bcs.s	horivert

diagonal:
	sub.w	d2,d3
	move.w	d3,a2

	add.w	d4,d1
	swap	d4
	add.w	d4,d0		; PUTpoint
	swap	d4

	movem.w	d0/d1,-(a7)
	move.w	d0,d6
	lsr.w	#3,d6
	add.w	d1,d1
	add.w	(a4,d1.w),d6
	and.w	#$7,d0
	move.b	settabel2(pc,d0.w),d0
	bchg.b	d0,(a1,d6.l)
	movem.w	(a7)+,d0/d1

	dbf	d7,next
enddraw:
	movem.l	(a7)+,d0-d7/a2/a3
	rts

horivert:
	move.w	d3,a2
	add.w	d5,d1
	swap	d5
	add.w	d5,d0		; PUTpoint
	swap	d5

	movem.w	d0/d1,-(a7)
	move.w	d0,d6
	lsr.w	#3,d6
	add.w	d1,d1
	add.w	(a4,d1.w),d6
	and.w	#$7,d0
	move.b	settabel2(pc,d0.w),d0
	bchg.b	d0,(a1,d6.l)
	movem.w	(a7)+,d0/d1

	dbf	d7,next
	movem.l	(a7)+,d0-d7/a2/a3
	rts
