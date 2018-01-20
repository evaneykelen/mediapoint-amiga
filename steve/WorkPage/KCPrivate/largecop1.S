*
* For the Large effect
* in d7 the line skip
*
init_user_coplist_mod_test4:
	move.l	db_inactive_viewblok(a3),a5

	move.l	#ucl_SIZEOF,d0
	move.l	#MEMF_CLEAR,d1
	move.l	$4,a6
	jsr	_LVOAllocMem(a6)

	move.l	d0,db_ucop_pointer(a3)
	beq	no_ucop_mem

	move.l	#1024,d0		
	move.l	db_ucop_pointer(a3),a0
	move.l	db_graphbase(a3),a6
	jsr	_LVOUCopperListInit(a6)

	moveq	#0,d5
	move.w	vb_leny(a5),d5
	move.l	#0,d6

rep_make_large:
;	moveq	#0,d1
;	move.l	d6,d0
;	add.l	d0,d0

;	move.l	db_ucop_pointer(a3),a1
;	jsr	_LVOCWait(a6)
;	move.l	db_ucop_pointer(a3),a1
;	jsr	_LVOCBump(a6)
	move.w	#$180,d0
	move.w	#$f0f,d1
	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCMove(a6)
	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCBump(a6)

	move.l	#$108,d0
	move.l	vb_breedte_x(a5),d1
	sub.w	db_modulodisp(a3),d1
	ext.l	d1
	neg.l	d1
	add.l	vb_breedte_x(a5),d1
	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCMove(a6)
	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCBump(a6)

	move.l	#$10a,d0
	move.l	vb_breedte_x(a5),d1
	sub.w	db_modulodisp(a3),d1
	ext.l	d1
	neg.l	d1
	add.l	vb_breedte_x(a5),d1
	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCMove(a6)
	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCBump(a6)

	move.l	#0,d1
	move.l	d6,d0
;	add.l	d7,d0
	add.l	#8,d0
;	add.l	d0,d0

	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCWait(a6)
	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCBump(a6)

	move.l	#$108,d0
	moveq	#0,d1
	move.w	db_moda(a3),d1
	add.l	vb_breedte_x(a5),d1
	move.l	d1,d3
	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCMove(a6)
	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCBump(a6)

	move.w	#$180,d0
	move.w	#$fff,d1
	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCMove(a6)
	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCBump(a6)

	move.l	#$10a,d0
	moveq	#0,d1
	move.w	d3,d1
	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCMove(a6)
	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCBump(a6)

	moveq	#0,d1
	move.l	d6,d0
	add.l	#10,d0
;	add.l	d0,d0

	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCWait(a6)
	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCBump(a6)

;	add.l	d7,d6
	add.l	#10,d6
	cmp.w	vb_leny(a5),d6
	ble	rep_make_large

	move.l	#10000,d0
	move.l	#255,d1
	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCWait(a6)

	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCBump(a6)

	move.w	#267,db_terminate(a3)
	rts
