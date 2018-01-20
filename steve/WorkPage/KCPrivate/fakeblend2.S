
*
* copieer van twee lace pics van active naar inactive alleen de oneven lijnen
*
copy_blend:

	move.l	db_inactive_viewblok(a3),a4	; source
	move.l	db_active_viewblok(a3),a5		; destination

	move.l	vb_tempbuffer(a5),a0
	move.l	vb_tempbuffer(a4),a1

	moveq	#0,d2
	move.l	vb_breedte_x(a5),d1
	mulu	vb_planes(a5),d1		; d1 nu de regel lengte
						; in bytes
;	add.l	d1,a1				; eerste regel is active view

	move.l	d1,d2
	lsr.w	#1,d1				; in words

	move.w	vb_leny(a4),d0
	lsr.w	#1,d0
repblend:
	move.l	d1,d3
repblend2:
	move.w	(a0)+,(a1)+
	subq.w	#1,d3
	bne	repblend2
	add.l	d2,a1				; zijn beide interlace
	add.l	d2,a0				; sla een regel over	

	subq.l	#1,d0
	bne.b	repblend

	rts
