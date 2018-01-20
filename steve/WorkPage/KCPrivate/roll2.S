hardblithorizontal3:
	move.l	db_effect_speed(a3),db_effect_speed_org(a3)
	move.l	d0,d7			; save d0 reg

	moveq	#0,d6
	move.w	vb_leny(a5),d6
	cmp.l	d6,d0
	bge	no_blith

	add.l	db_effect_speed(a3),d7
	cmp.w	d6,d7
	ble	no_horb_pro3
	sub.l	d6,d7
	sub.l	d7,db_effect_speed(a3)
no_horb_pro3:
	tst.l	d0
	bpl	no_probhor3
	moveq	#0,d0
no_probhor3:

	move.l	d1,d6
;	move.l	db_graphbase(a3),a6
;	jsr	_LVOOwnBlitter(a6)	; blitter multi-tasking ends here

	move.l	d7,d0
	move.l	#$dff000,a0		; hardware basis registers

	move.l	vb_breedte_x(a5),d7
	mulu	vb_planes(a5),d7
	mulu	d7,d0			; vertikale offset dest
	mulu	d7,d6			; vertikale offset source 

	move.l	vb_tempbuffer(a4),d1	; source pointer
	add.l	d6,d1

	move.l	vb_tempbuffer(a5),d2	; destination pointer
	add.l	d0,d2

;	bra	hardblithorizontal2

; FFFFFFFFFFFFFFFFFFFFFFF

	move.l	vb_totalwidth(a5),d7
	move.l	d1,a0
	move.l	d2,a1
.repp:
	move.l	(a0)+,(a1)+
	subq.l	#1,d7
	bne	.repp
	rts
	
; FFFFFFFFFFFFFFFFFFFFFFF

	bra	hardblithorizontal2
