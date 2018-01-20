init_copper_blend:
	move.l	db_inactive_viewblok(a3),a4
	move.l	db_horizontalmask(a3),a0
	move.w	vb_colors(a4),d7		; fade naar nul kleur inactive
	moveq	#31,d2
rep_i_c1:
	move.w	d7,(a0)+
	dbf	d2,rep_i_c1

	lea	db_cop_colorsLOF(a3),a1
	move.l	db_horizontalmask(a3),a0
	bsr	calc_it2			; reken delta's uit
						; voor even lijnen
	move.l	db_waarcolors2(a3),a6
	move.l	db_waardeltacol2(a3),a2
	lea	db_cop_colorsLOF(a3),a1
	move.l	db_horizontalmask(a3),a0
	bsr	calc_it3			; reken delta's uit
	rts

*
* blend alleen het LOF beeld van het interlace plaatje
*
do_even_copper_blend:
	moveq	#15,d0
rep_do_blend1:
	move.l	d0,-(a7)
	move.l	db_waarcolors1(a3),a6
	move.l	db_waardeltacol1(a3),a2
	lea	db_cop_colorsLOF(a3),a0
	moveq	#32,d0
	bsr	add_up_delta_cols
	move.l	db_active_viewblok(a3),a5
	move.l	vb_vieww(a5),a0
	move.l	v_SHFCprList(a0),a0
	move.w	crl_MaxCount(a0),d2		; aantal elementen
	move.l	crl_start(a0),a0		; eigenlijke copperlist
	lea	db_cop_colorsLOF(a3),a1
	bsr	put_copper_cols			; SHF 2 * zo snel ????
	bsr	wacht_tijd2
	bne	exit_b101
	bsr	reshowpicture
	move.l	(a7)+,d0
	dbf	d0,rep_do_blend1

; active nu oneven beeldlijne zwart en even beeldlijnen al op de helft van 
; de kleuren cycle
; reken nu de kleuren uit om van kleur nul inactive te komen naar 
; de inactive kleuren ( eigenlijk moet ook van het even beeld de kleur nul al
; klaar zijn ????????????? )

	move.l	db_horizontalmask(a3),a1		; staat nog nul kleur inactive
	move.l	db_inactive_viewblok(a3),a4
	move.l	vb_vieww(a4),a0
	move.l	v_SHFCprList(a0),a0
	move.w	crl_MaxCount(a0),d2		; aantal elementen
	move.l	crl_start(a0),a0		; eigenlijke copperlist
	move.l	db_horizontalmask(a3),a1		; maak SHF zwart
	bsr	put_copper_cols

	lea	vb_colors(a4),a0
	move.l	db_horizontalmask(a3),a1
	bsr	calc_it2			; reken delta's uit

	move.l	db_inactive_viewblok(a3),a5
	bsr	showinactive2

	moveq	#15,d0
rep_i_c2:
	move.l	d0,-(a7)
	move.l	db_waarcolors2(a3),a6			;ga verder met faden old active
	move.l	db_waardeltacol2(a3),a2
	lea	db_cop_colorsLOF(a3),a0
	moveq	#32,d0
	bsr	add_up_delta_cols
	move.l	db_active_viewblok(a3),a5
	move.l	vb_vieww(a5),a0
	move.l	v_LOFCprList(a0),a0
	move.w	crl_MaxCount(a0),d2		; aantal elementen
	move.l	crl_start(a0),a0		; eigenlijke copperlist
	lea	db_cop_colorsLOF(a3),a1
	bsr	put_copper_cols

	move.l	db_waarcolors1(a3),a6			;ga verder met faden old active
	move.l	db_waardeltacol1(a3),a2
	lea	db_cop_colorsLOF(a3),a0
	moveq	#32,d0
	bsr	add_up_delta_cols

	move.l	db_active_viewblok(a3),a5
	move.l	vb_vieww(a5),a0
	move.l	v_SHFCprList(a0),a0
	move.w	crl_MaxCount(a0),d2		; aantal elementen
	move.l	crl_start(a0),a0		; eigenlijke copperlist
	lea	db_cop_colorsLOF(a3),a1
	bsr	put_copper_cols

	bsr	wacht_tijd2
	bne	exit_b101

	bsr	reshowpicture
	move.l	(a7)+,d0
	dbf	d0,rep_i_c2

	bsr	proces_file

	move.l	db_inactive_viewblok(a3),a4
	move.l	db_horizontalmask(a3),a1		; maak oneven zwart
	move.w	vb_colors(a4),d7		; fade vanaf nul kleur inactive
	moveq	#31,d2
rep_i_c3:
	move.w	d7,(a1)+
	dbf	d2,rep_i_c3

	move.l	db_inactive_viewblok(a3),a4
	move.l	vb_vieww(a4),a0
	move.l	v_SHFCprList(a0),a0
	move.w	crl_MaxCount(a0),d2		; aantal elementen
	move.l	crl_start(a0),a0		; eigenlijke copperlist
	move.l	db_horizontalmask(a3),a1		; maak oneven zwart
	bsr	put_copper_cols
	lea	vb_colors(a4),a0
	move.l	db_horizontalmask(a3),a1

	bsr	calc_it2			; reken delta's uit

	move.l	db_inactive_viewblok(a3),a5
	bsr	showinactive2

	bsr	reshowpicture
	moveq	#15,d0
rep_i_c5:
	move.l	d0,-(a7)
	move.l	db_waarcolors1(a3),a6			;ga verder met faden old active
	move.l	db_waardeltacol1(a3),a2
	lea	db_cop_colorsLOF(a3),a0
	moveq	#32,d0
	bsr	add_up_delta_cols

	move.l	db_active_viewblok(a3),a5
	move.l	vb_vieww(a5),a0
	move.l	v_SHFCprList(a0),a0
	move.w	crl_MaxCount(a0),d2		; aantal elementen
	move.l	crl_start(a0),a0		; eigenlijke copperlist
	lea	db_cop_colorsLOF(a3),a1
	bsr	put_copper_cols
	bsr	reshowpicture
	bsr	wacht_tijd2
	bne	exit_b101
	move.l	(a7)+,d0
	dbf	d0,rep_i_c5

exit_b100:
	rts

exit_b101:
	move.l	(a7)+,d0
	rts
