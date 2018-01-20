fake_blend:
	bra	no_blend

	
	tst.b	db_aa_present(a3)
	bne	no_blend

	cmp.l	#1,db_variation(a3)
	bne	no_blend

	tst.b	vb_interlace(a5)
	beq	no_conv2

no_conv1:
	bsr	clear_convert_bytes	; active is interlace dus geen probleem
	bsr	adaptfile
	tst.w	d0
	bne	no_blend
	bra	go_on_tb

no_conv2:
	tst.b	vb_interlace(a4)
	beq	do_conv2

	bra	no_conv1		; inactive forceert active naar lace

do_conv2:
	move.w	vb_leny(a4),d0
	add.w	d0,d0
	move.w	d0,vb_leny(a4)
	move.b	#$ff,vb_interlace(a4)
; forceer de active naar lace
	bsr	clear_convert_bytes
	bsr	adaptfile		; verander naar zelfde formaat
	tst.w	d0
	bne	no_blend
	move.l	db_inactive_fileblok(a3),a4
	move.w	vb_leny(a4),d0
	lsr.w	#1,d0
	move.w	d0,vb_leny(a4)
	move.b	#0,vb_interlace(a4)
	move.b	#$ff,db_wipe_in_to_lace(a3)
	move.b	#$ff,db_change(a3)

; de inactive wordt nu naar lace gepushed

;	bsr	convert_nolace_lace_black

go_on_tb:
	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_viewblok(a3),a4
	tst.b	db_change(a3)
	beq	no_change1

	bsr	adaptdecrunch
	tst.b	db_wipe_in_to_lowres(a3)
	bne	yes_low1
	tst.b	db_wipe_in_to_hires(a3)
	beq	setd1
	move.b	#$ff,vb_hires(a4)
	or.w	#$8000,vb_mode(a4)	; turn on hires
	bra	setd1

yes_low1:
	move.b	#$0,vb_hires(a4)
	and.w	#$7fff,vb_mode(a4)	; turn off hires
	bra	setd1

no_change1:
	bsr	proces_file

setd1:	
; in de inactive buffer nu het tweede plaatje nonlace
; copieer nu het plaatje naar de zwarte lijnen van het active screen
;
;	bsr	calc_fade_colors

	move.l	db_inactive_viewblok(a3),a4
	move.l	db_active_viewblok(a3),a5
	move.b	#$ff,vb_interlace(a4)		; switch naar lace
	move.w	vb_planes(a5),vb_planes(a4)
	move.w	vb_leny(a5),vb_leny(a4)
	move.w	vb_lenx(a5),vb_lenx(a4)
	move.l	vb_breedte_x(a5),vb_breedte_x(a4)
	or.w	#$4,vb_mode(a4)

	bsr	copy_blend

; alle twee views zijn nu klaar maak nu de oneven lijnen van het active view
; langzaam zwart en schakel als ze zwart zijn over naar het inactive view

	move.l	db_inactive_viewblok(a3),a5
	bsr	maak_viewport			; maak coppers aan etc
;	bsr	showinactive2

;	bsr	reshowpicture
	
	bsr	copy_copper_cols1		; copieer kleuren naar inac.cop

	bsr	init_copper_blend		; delta's gezet voor fade

	bsr	do_even_copper_blend
						; naar black

;	bsr	reshowpicture

no_can_do:

	bsr	proces_file			; show orgineel
	bsr	showpicture	

	rts
