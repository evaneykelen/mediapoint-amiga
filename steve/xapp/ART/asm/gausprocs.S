*
* Try to aply a gaussian filter on a bitmap
* a1 points to the bitmap structure
*
scan_bitmap:
	movem.l	d0-d7/a0-a6,-(a7)
	lea	loc_gaus(pc),a5
	move.l	a1,ga_bitmap(a5)
	moveq	#0,d0
	move.w	bm_Rows(a1),d0
	move.l	d0,ga_height(a5)
	move.w	bm_BytesPerRow(a1),d0
	move.l	d0,ga_bpr(a5)
	move.l	bm_Planes+4(a1),d0
	sub.l	bm_Planes(a1),d0		; width in case of interleaved
	move.l	d0,ga_width_byte(a5)		; in bytes
	lsl.l	#3,d0
	move.l	d0,ga_width(a5)			; in pixels

	bsr	create_gaus_tab			; better is once in the init.

	bsr	get_pixels_bitmap
	movem.l	(a7)+,d0-d7/a0-a6
	rts

*
* Aply gaussian filter on a bitmap
* In this test version the first plane
* is used as a mask, where the routine gets its pixels from
* Therefor the first plane must contain data representing the picture
*
* To get the data for the conversion table some shifting is needen
* to store the top row and the bottom row in there place
*
get_pixels_bitmap:
	move.l	ga_bpr(a5),d5			; line offset
	move.l	d5,d4
	neg.l	d4
	move.l	ga_bitmap(a5),a1
	move.l	bm_Planes(a1),a0
	move.l	a0,a2
	lea	gaus_tab(pc),a1
	lea	color_disp(pc),a4
	move.l	ga_height(a5),d6
	subq.l	#3,d6				; stop one line down
	add.l	d5,a2				; start one line down
rep_height:
	move.l	ga_width(a5),d7
	subq.l	#2,d7
	moveq	#12,d1				; start one row from the left
	move.l	a2,a0
rep_width:
	moveq	#0,d3
	move.l	-2(a0),d0			; retrieve 4 5 6
	lsr.l	d1,d0
	and.w	#$7,d0
	btst	#1,d0
	bne	no_pix
	lsl.b	#2,d0
	move.b	d0,d3
	and.b	#$10,d3
	and.b	#$04,d0
	lsl.b	#1,d0
	or.b	d0,d3
	move.l	-2(a0,d4.w),d0			; retrieve 7 6 5
	lsr.l	d1,d0
	and.w	#$7,d0
	lsl.b	#6,d0
	or.b	d0,d3

	move.l	-2(a0,d5.w),d0			; retrieve 2 1 0
	lsr.l	d1,d0
	and.w	#$7,d0
	lsl.b	#6,d0
	or.b	d0,d3				; in d3 the value of this pixel
						; 7 6 5 4 3 2 1 0
	move.b	0(a1,d3.w),d3
	move.b	0(a4,d3.w),d3	
	beq	no_pix
	bsr	put_pixel
no_pix:
	subq.l	#1,d1
	bpl	no_addr_add
	addq.l	#2,a0
	moveq	#15,d1
no_addr_add:
	dbf	d7,rep_width	
	add.l	d5,a2
	dbf	d6,rep_height
	rts

put_pixel:
	movem.l	a1/d4/d6/d7/d1,-(a7)	
	moveq	#0,d7
;	move.b	d3,d7
;	add.l	d7,d7
;	add.l	d7,d7
;	lea	col_count(pc),a1
;	add.l	#1,0(a1,d7)

	move.l	ga_bitmap(a5),a1
	move.l	ga_width_byte(a5),d6

	moveq	#2,d4				; I skip the first plane
	moveq	#1,d0				; to leave the mask plane
	addq.w	#1,d1				; in tact
	lsl.l	d1,d0
	move.l	d0,d1
	eor.l	#$ffffffff,d1
	move.l	d6,d7
rep_pla:
	lsr.b	#1,d3
	btst	#0,d3
	beq	nopix
	or.l	d0,-2(a0,d7)
	bra	wpix
nopix:
	and.l	d1,-2(a0,d7)
wpix:	
	add.l	d6,d7
	dbf	d4,rep_pla
	movem.l	(a7)+,a1/d4/d6/d7/d1
	rts

col_count:
		blk.l	256,0
		
loc_gaus:	blk.l	20,0

	RSRESET
ga_struct:	rs.l	0		; should be on the stack
ga_bitmap:	rs.l	1
ga_width:	rs.l	1
ga_width_byte:	rs.l	1
ga_height:	rs.l	1
ga_bpr:		rs.l	1
ga_wplane:	rs.l	1
ga_SIZEOF	rs.l	0

gtab:	dc.b	1,2,1,2,2,1,2,1		; the gaussian filter except elem (1,1)
	even
*
* Create a bit displacement table for the gausian filter
* the bits are stored as follows
* 1 2 3
* 4 5 6
* 7 8 9
* ----- 8 bits -> 1-2-3-4-6-7-8-9
*
create_gaus_tab:
	lea	gtab(pc),a1
	move.l	#255,d7
	moveq	#0,d0
	lea	gaus_tab(pc),a0
rep_cr_g1:
	moveq	#0,d1
	btst	#0,d0
	beq	no_a1
	add.b	(a1),d1
no_a1:
	btst	#1,d0
	beq	no_a2
	add.b	1(a1),d1
no_a2:
	btst	#2,d0
	beq	no_a3
	add.b	2(a1),d1
no_a3:
	btst	#3,d0
	beq	no_a4
	add.b	3(a1),d1
no_a4:
	btst	#4,d0
	beq	no_a5
	add.b	4(a1),d1
no_a5:
	btst	#5,d0
	beq	no_a6
	add.b	5(a1),d1
no_a6:
	btst	#6,d0
	beq	no_a7
	add.b	6(a1),d1
no_a7:
	btst	#7,d0
	beq	no_a8
	add.b	7(a1),d1
no_a8:
	addq.b	#1,d0
	move.b	d1,(a0)+

	dbf	d7,rep_cr_g1
	rts


gaus_tab:	blk.b	256,0		; the results from the matrix mult.
					; are stored here
					; pref. it should be stored here
					; and not calculated every time
					
*
* This table gives the color number by the matrix result
* there should be 12 maximum for this version
* 8,9,10 are colors black-grey grey greyer ?? choose for this testversion
* maybe we can use for example the last three or four cols. for now
*
; result matrix		1,2,3,4,5, 6, 7, 8,9
color_disp:	dc.b	0,0,0,8,9,10,15,15,15,15,15,15,15,15,15,15
	even
	ENDC
