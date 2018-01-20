
large_it:
	subq.w	#1,d5
	move.l	a0,a6
lar_rep_line:
	move.l	a6,a0
	move.w	vb_planes(a5),d6
	subq.w	#1,d6
	move.l	a0,a4
lar_rep_pl:
	move.l	a4,a0
	move.l	vb_breedte_x(a5),d7
	lsr.l	#1,d7
	bra	in_by
lar_rep_by
	moveq	#0,d1
	move.b	(a0)+,d1
	add.w	d1,d1
	move.w	0(a2,d1),(a1)+
in_by:
	dbf	d7,lar_rep_by
	add.l	vb_breedte_x(a5),a4
	dbf	d6,lar_rep_pl
	move.l	vb_breedte_x(a5),d0
	mulu	vb_planes(a5),d0
	add.l	d0,a6
	dbf	d5,lar_rep_line
	rts
	
large:
	bsr	init_drip1

	move.l	db_inactive_viewblok(a3),a5
	move.l	vb_breedte_x(a5),d0
	mulu	vb_planes(a5),d0

	mulu	vb_leny(a5),d0
	lsr.l	#1,d0				; longs ?
	move.l	vb_tempbuffer(a5),a0
	move.l	db_triple_mem(a3),a1
	move.l	db_wpatroon_tabel(a3),a2
	move.w	vb_leny(a5),d5

	moveq	#1,d7
rep_make_large2:
	move.l	a1,(a2)
	lsr.w	#1,d5
	movem.l	d5/d7/a2,-(a7)
	bsr	get_hi_tablea2
	bsr	large_it
	movem.l	(a7)+,a2/d7/d5
	move.l	a1,a4
	sub.l	db_triple_mem(a3),a4
	move.l	(a2)+,a0
	cmp.w	#1,d5
	bne	rep_make_large2
	move.l	#0,(a2)+

* create pointers to the data in the triple mem buffer

	move.l		db_inactive_viewblok(a3),a5
	move.l		vb_breedte_x(a5),d1
	move.l		vb_bitmapw(a5),a0

	move.l		db_triple_mem(a3),d0
	move.l	vb_tempbuffer(a5),d0
	move.l		d0,bm_Planes(a0)		; zet de bitplanes
	add.l		d1,d0
	move.l		d0,bm_Planes+4(a0)
	add.l		d1,d0
	move.l		d0,bm_Planes+8(a0)
	add.l		d1,d0
	move.l		d0,bm_Planes+12(a0)
	add.l		d1,d0
	move.l		d0,bm_Planes+16(a0)
	add.l		d1,d0
	move.l		d0,bm_Planes+20(a0)
	add.l		d1,d0
	move.l		d0,bm_Planes+24(a0)
	add.l		d1,d0
	move.l		d0,bm_Planes+28(a0)

	lea	teller(pc),a0
	move.l	#10,(a0)
	move.l	#10,4(a0)

	move.l	db_active_viewblok(a3),a5
	
; atviewp now copy of the active view

	move.l	db_wpatroon_tabel(a3),a2
	moveq	#2,d7
frep_cop_large:
	movem.l	d7/a2,-(a7)

	bsr	free_view25u
	bsr	init_user_coplist_mod_test4

	move.l	itviewp(pc),a1
	move.l	db_ucop_pointer(a3),vp_UCopIns(a1)

	move.l	itview(pc),a0			; first the active view
	move.l	itviewp(pc),a1
	move.w	vp_Modes(a1),v_Modes(a0)
	jsr	_LVOMakeVPort(a6)

	move.l	itview(pc),a1
	jsr	_LVOMrgCop(a6)

	move.l	itview(pc),a1
	jsr	_LVOLoadView(a6)

	bsr	switch_temp_views
	movem.l	(a7)+,a2/d7
	
	bsr	wacht_tijd2
	tst.b	d0
	bne	fexit_cop_large

	add.l	d7,d7
	tst.l	(a2)+
	beq	fexit_cop_large
	
	bra	frep_cop_large

fexit_cop_large:

	bsr	showpicture

	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)

	bsr	free_view_ucop
	bsr	switch_temp_views
	bsr	free_view_ucop
	rts
