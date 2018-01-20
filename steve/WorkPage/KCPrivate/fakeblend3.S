*
* copieer de copper kleuren van lijst active naar lijst inactive
* alleen de LOF coplist
*
copy_copper_cols1:
	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_viewblok(a3),a4

	move.l	vb_vieww(a5),a0
	move.l		v_LOFCprList(a0),a0

	move.w		crl_MaxCount(a0),d2		; aantal elementen
	move.l		crl_start(a0),a0		; eigenlijke copperlist
	lea		db_cop_colorsLOF(a3),a1
	bsr		get_copper_cols

	move.l	vb_vieww(a4),a0
	move.l		v_LOFCprList(a0),a0

	move.w		crl_MaxCount(a0),d2		; aantal elementen
	move.l		crl_start(a0),a0		; eigenlijke copperlist
	lea		db_cop_colorsLOF(a3),a1
	bsr		put_copper_cols
	rts

*
* haal copper kleuren vanaf copper lijst a0 lengte d2
* store naar a1 ruimte voor 32 kleuren ?????????
*
get_copper_cols:
	subq.l		#1,d2
rep_copcop:
	move.w		(a0)+,d0
	cmp.w		#$ffff,d0
	beq		exit_copcopy

	cmp.w		#$180,d0
	blt		no_col0
	cmp.w		#$1be,d0
	bgt		no_col0
	sub.l		#$180,d0
	move.w		(a0),0(a1,d0)		; sla kleuren info op
no_col0:
	addq.l		#2,a0
	dbf		d2,rep_copcop

exit_copcopy:
	rts
*
* zet copper kleuren vanaf buffer a1
* naar copper a0 met lengte d2
*
put_copper_cols:
	subq.l		#1,d2
rep_putcop:
	move.w		(a0)+,d0
	cmp.w		#$ffff,d0
	beq		pexit_putcopy

	cmp.w		#$180,d0
	blt		no_putcol0
	cmp.w		#$1be,d0
	bgt		no_putcol0
	sub.l		#$180,d0
	move.w		0(a1,d0),(a0)		; store kleuren info in copper
no_putcol0:
	addq.l		#2,a0
	dbf		d2,rep_putcop
pexit_putcopy:
	rts
