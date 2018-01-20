
get_4planes:
	movem.l	a4/a5,-(a7)
	bsr	init_planes

	move.l	(a6)+,a0
	move.l	(a6)+,a1
	move.l	(a6)+,a2
	move.l	(a6)+,a3

	move.l	im_chunk(a4),a6

	move.l	im_height(a4),d6	
	subq.l	#1,d6
	move.l	im_width(a4),d7
	move.l	ch_mod(a5),a4
.rep_y:
	move.l	#0,a5
.rep_x:
	moveq	#0,d0
	moveq	#0,d1
	move.b	(a3)+,d1
	rol.w	#8,d1
	move.b	(a1)+,d1

; D1 = a3b3c3d3e3f3g3h3 a1b1c1d1e1f1g1h1

	move.b	(a2)+,d0	; plane 2
	rol.w	#8,d0	
	move.b	(a0)+,d0	; plane 0

; D0 = a2b2c2d2e2f2g2h2 a0b0c0d0e0f0g0h0

	move.w	d1,d2
	and.w	#$aaaa,d2
	and.w	#$5555,d1
	move.w	d0,d3
	and.w	#$aaaa,d0
	and.w	#$5555,d3
	lsr.w	#1,d0
	or.w	d2,d0
	add.w	d1,d1
	or.w	d3,d1
	move.w	d1,d2
	and.w	#$ff,d2
	and.w	#$ff00,d1
	move.w	d0,d3
	and.w	#$ff,d0
	and.w	#$ff00,d3
	lsr.w	#8,d3
	lsl.w	#8,d2
	or.w	d2,d0
	or.w	d3,d1
	move.w	d1,d2
	and.w	#$3333,d1
	and.w	#$cccc,d2
	move.w	d0,d3
	and.w	#$cccc,d0
	and.w	#$3333,d3
	lsl.w	#2,d1
	or.w	d3,d1
	lsr.w	#2,d0
	or.w	d2,d0
	swap	d0
	move.w	d1,d0
	swap	d0
	move.l	d0,d3
	and.l	#$0f0f0f0f,d3
	and.l	#$f0f0f0f0,d0
	lsr.l	#4,d0
	ror.w	#8,d0
	swap	d0
	ror.w	#8,d0
	ror.w	#8,d3
	swap	d3
	ror.w	#8,d3
	move.l	d0,(a6)+
	move.l	d3,(a6)+
	addq.w	#8,a5	
	cmp.w	d7,a5
	blt	.rep_x
	add.l	a4,a0			; pointer to next line
	add.l	a4,a1
	add.l	a4,a2
	add.l	a4,a3
	dbf	d6,.rep_y	
	movem.l	(a7)+,a4/a5
	rts
