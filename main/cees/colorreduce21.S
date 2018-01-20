*
* File	: colorreduce.s
* Uses	:
* Date	: 18 december 1993
* Author: ing. C. Lieshout
* Desc	: Reduce an input colormap to a smaller one
*	: Original C++ code from Bitmapped Graphics Programming in C++

	INCDIR	"include:"
	include	"exec/memory.i"
	INCDIR	"wp:inclibs/"
	include	"exec_lib.i"

	XDEF	_reduce_palette
	XDEF	_smooth_colormap
	XDEF	_contrast_colormap

TESTING = 0
	RSRESET
rgb_struct:	rs.w	0
rgb_red:	rs.b	1
rgb_green:	rs.b	1
rgb_blue:	rs.b	1
		rs.b	1
rgb_dist:	rs.l	1
rgb_SIZEOF:	rs.w	0

	IFNE	TESTING
	bra	test
	ENDC

*
* dist3( int d0,d2,d4 (RGB) int d1,d3,d5 (RGB ) )
*
dist3:
	lea	PythTable(pc),a0
	sub.w	d0,d1
	sub.w	d2,d3
	sub.w	d4,d5
	add.w	d1,d1
	add.w	d3,d3
	add.w	d5,d5
	move.w	0(a0,d1.w),d0		; muls d0,d0
	add.w	0(a0,d3.w),d0
	add.w	0(a0,d5.w),d0
	rts
*
* xchg_rgb( RGB * a0, int m d0, int n d1 );
* Exhange two RGB entries
*
xchg_rgb:
	move.l	temp,d2
	cmp.l	d0,d2
	blt	.error
	cmp.l	d1,d2
	blt	.error
	
	lsl.l	#3,d0
	lsl.l	#3,d1
	move.l	0(a0,d0.l),d2
	move.l	0(a0,d1.l),0(a0,d0.l)
	move.l	d2,0(a0,d1.l)
	move.l	4(a0,d0.l),d2
	move.l	4(a0,d1.l),4(a0,d0.l)
	move.l	d2,4(a0,d1.l)
	rts

.error:
	move.l	d6,-(a7)
	move.w	d1,d0
	move.w	#12000,d1
	moveq	#0,d6
.err:	move.w	d6,$dff180
	addq.w	#1,d6
	dbf	d1,.err
	move.l	(a7)+,d6
	rts	
*
* closest_rgb( RGB *x a0, RGB *pal a1, int npal d0 );
* find the closest match with RGB x
*
closest_rgb:		
	movem.l	d1-d7/a0-a4,-(a7)

	move.w	d0,d7

	moveq	#0,d0
	moveq	#0,d2
	moveq	#0,d4

	move.b	(a0),d0			; get source rgb
	move.b	rgb_green(a0),d2
	move.b	rgb_blue(a0),d4

	move.l	#100000,a2		; set minimum large

	moveq	#0,d6			; pen nr.
	move.l	d6,a4
	bra	.in_loop
.rep_clos:
	moveq	#0,d1
	moveq	#0,d3
	moveq	#0,d5
	move.b	(a1),d1
	move.b	1(a1),d3
	move.b	2(a1),d5
	move.w	d0,a3
	bsr	dist3
	cmp.l	d0,a2
	ble	.no_store
	move.l	d0,a2
	move.l	d6,a4
.no_store:
	addq.w	#1,d6
	move.w	a3,d0
	addq.l	#8,a1			; next color
.in_loop:
	dbf	d7,.rep_clos
	move.l	a4,d0

	movem.l	(a7)+,d1-d7/a0-a4
	rts

*
* maximal_rgb( RGB *pal a0, int mid d0, int max d1 )
*
* Find entry from large palette that is maximaly apart from to basis palette
* mid is the number of entries filled
* max is the total number of colors
*
maximal_rgb:
	movem.l	d1-d7/a0-a6,-(a7)
	lea	PythTable(pc),a3
	move.l	a0,a2		; the palette pointer to the found cols
	move.l	d0,d2
	lsl.l	#3,d2
	lea	0(a2,d2.l),a2	; point to the first col after the found cols
	
; calculate for each color the distance
	move.w	d1,d7		; number of colors in total palette
	sub.w	d0,d7		; number of colors stil in palette
	bmi	error
	
	move.l	#1000000,d5	; max number
	sub.l	a6,a6		; remember the most far color
	sub.l	a5,a5
	move.l	d0,a5

	bra	.in_loop2
.rep2:

	move.w	d0,d6
	move.l	a0,a1			; the palette pointer
	bra	.in_loop1
.rep1:
	moveq	#0,d1
	moveq	#0,d2
	move.b	(a2),d1
	move.b	(a1),d2
	sub.w	d1,d2
	add.w	d2,d2
	move.w	0(a3,d2.w),d3
	moveq	#0,d1
	moveq	#0,d2
	move.b	rgb_green(a2),d1
	move.b	rgb_green(a1),d2
	sub.w	d1,d2
	add.w	d2,d2
	add.w	0(a3,d2.w),d3
	moveq	#0,d1
	moveq	#0,d2
	move.b	rgb_blue(a2),d1
	move.b	rgb_blue(a1),d2
	sub.w	d1,d2
	add.w	d2,d2
	add.w	0(a3,d2.w),d3
	lsr.w	#1,d3				; no negative numbers
	moveq	#0,d4
	move.w	#$7000,d4
	tst.w	d3
	beq	.zero
	divu	d3,d4	
.zero:
	ext.l	d4
	add.l	d4,rgb_dist(a2)
	addq.l	#8,a1
.in_loop1:
	dbf	d6,.rep1
	cmp.l	rgb_dist(a2),d5
	blt	.no_min
	move.l	a5,a6			; remember color
	move.l	rgb_dist(a2),d5		; set new minimum
.no_min:
	addq.l	#1,a5			; next color
	addq.l	#8,a2
.in_loop2:
	dbf	d7,.rep2
	move.l	a6,d0	
	movem.l	(a7)+,d1-d7/a0-a6
	rts
*
* maximal_rgb( RGB *pal a0, int mid d0, int max d1 )
*
maximal_rgb_opt:
	movem.l	d1-d7/a0-a6,-(a7)
	lea	PythTable(pc),a3
	move.l	temp,d2
	cmp.l	d0,d2
	blt	error
	
	move.l	d0,d2
	lsl.l	#3,d2
	lea	0(a0,d2.l),a2	; point to the first col after the found cols

	move.w	d0,d2
	subq.w	#1,d2

	lsl.l	#3,d2
	lea	0(a0,d2.l),a0	; this is the last added color
		
; calculate for each color the distance
; to this last found one and substract that from the total distance

	move.w	d1,d7		; number of colors in total palette
	sub.w	d0,d7		; number of colors stil in palette
	bmi	error
	
	move.l	#1000000,d5	; max number
	sub.l	a6,a6		; remember the most far color
	sub.l	a5,a5
	move.l	d0,a5

	bra	.in_loop2
.rep2:

	move.l	a0,a1			; the palette pointer
.rep1:
	moveq	#0,d1
	moveq	#0,d2
	move.b	(a2),d1
	move.b	(a1),d2
	sub.w	d1,d2
	add.w	d2,d2
	move.w	0(a3,d2.w),d3
	moveq	#0,d1
	moveq	#0,d2
	move.b	rgb_green(a2),d1
	move.b	rgb_green(a1),d2
	sub.w	d1,d2
	add.w	d2,d2
	add.w	0(a3,d2.w),d3
	moveq	#0,d1
	moveq	#0,d2
	move.b	rgb_blue(a2),d1
	move.b	rgb_blue(a1),d2
	sub.w	d1,d2
	add.w	d2,d2
	add.w	0(a3,d2.w),d3
	lsr.w	#1,d3				; no negative numbers
	moveq	#0,d4
	move.w	#$7000,d4
	tst.w	d3
	beq	.zero
	divu	d3,d4	
.zero:
	ext.l	d4
	add.l	d4,rgb_dist(a2)
	cmp.l	rgb_dist(a2),d5
	blt	.no_min
	move.l	a5,a6			; remember color
	move.l	rgb_dist(a2),d5		; set new minimum
.no_min:
	addq.l	#1,a5			; next color
	addq.l	#8,a2
.in_loop2:
	dbf	d7,.rep2
	move.l	a6,d0	
	movem.l	(a7)+,d1-d7/a0-a6
	rts

error:
	move.w	d1,d0
	move.w	#12000,d1
	moveq	#0,d6
.err
	move.w	d6,$dff180
	addq.w	#1,d6
	dbf	d1,.err
	movem.l	(a7)+,d1-d7/a0-a6
	rts

temp:	dc.l	0

*
* reduce_palette( RGB *pal a0, int npal d0, int nsub d1 )
*
_reduce_palette:
	link	a5,#0

	movem.l	d1-d7/a0-a6,-(a7)
	move.l	8(a5),a0
	move.l	12(a5),d0
	move.l	16(a5),d1

	move.l	d0,temp

	move.l	d0,d7

	move.l	d1,d5			; nsub
	moveq	#0,d6			; imin
	move.l	a0,a1
	lea	black(pc),a0
	bsr	closest_rgb
	move.l	a1,a0

	move.l	d6,d1
	bsr	xchg_rgb
	addq.l	#1,d6			; imin++
	move.l	d7,d1
	move.l	d6,d0
	bsr	maximal_rgb
	cmp.l	d6,d5
	blt	.exit

.rep:
	move.l	d6,d1
	bsr	xchg_rgb
	addq.l	#1,d6			; imin++
	move.l	d7,d1
	move.l	d6,d0
	bsr	maximal_rgb_opt
	cmp.l	d6,d5
	bgt	.rep

.exit:
	movem.l	(a7)+,d1-d7/a0-a6
	unlk	a5
	rts

	RSRESET
csm_struct:	rs.w	0
csm_ccnt:	rs.l	1
csm_rsum:	rs.l	1
csm_gsum:	rs.l	1
csm_bsum:	rs.l	1
csm_ccnt_mem:	rs.l	1
csm_rsum_mem:	rs.l	1
csm_img:	rs.l	1
csm_nimg:	rs.l	1
csm_dev:	rs.l	1
csm_ndev:	rs.l	1
csm_SIZEOF:	rs.w	0

*
* color_map( RGB *img a0, int nimg d0, RGB *dev a1, int ndev d1 )
* Smooth out the colormap
*
_smooth_colormap:
	link	a5,#-csm_SIZEOF
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a0
	move.l	12(a5),d0
	move.l	16(a5),a1
	move.l	20(a5),d1
	sub.l	#csm_SIZEOF,a5
	move.l	a0,csm_img(a5)
	move.l	d0,csm_nimg(a5)
	move.l	a1,csm_dev(a5)
	move.l	d1,csm_ndev(a5)
	clr.l	csm_ccnt(a5)
	clr.l	csm_rsum(a5)
	move.l	$4.w,a6
	move.l	d1,d0
	lsl.l	#2,d0
	move.l	d0,csm_ccnt_mem(a5)
	move.l	#MEMF_PUBLIC,d1
	jsr	_LVOAllocMem(a6)
	move.l	d0,csm_ccnt(a5)
	beq	.mem_err1

	move.l	csm_ndev(a5),d0
	lsl.l	#2,d0

	move.l	d0,d1
	add.l	d0,d0
	add.l	d1,d0
	move.l	d0,csm_rsum_mem(a5)
	move.l	#MEMF_PUBLIC,d1
	jsr	_LVOAllocMem(a6)
	move.l	d0,csm_rsum(a5)
	beq	.mem_err2

	move.l	csm_ndev(a5),d1
	lsl.l	#2,d1				; longs
;	add.l	d1,d1
	add.l	d1,d0
	move.l	d0,csm_gsum(a5)
	add.l	d1,d0
	move.l	d0,csm_bsum(a5)
	
; memory allocation oke now do the smoothing
; start with initializing the weighted sums
; every color in the found colormap is given a bigger weight
; to ensure there appearance in the final map

	move.l	csm_rsum(a5),a1
	move.l	csm_gsum(a5),a2
	move.l	csm_bsum(a5),a3
	move.l	csm_ccnt(a5),a4
	move.l	csm_ndev(a5),d7
	move.l	csm_dev(a5),a0
	bra	.in_ws

.rep_ws:
	moveq	#0,d0
	moveq	#0,d1
	moveq	#0,d2
	move.b	(a0),d0
	move.b	rgb_green(a0),d1
	move.b	rgb_blue(a0),d2
	lsl.w	#3,d0				; weigth now 2^3 = 8
	lsl.w	#3,d1
	lsl.w	#3,d2
	move.l	d0,(a1)+
	move.l	d1,(a2)+
	move.l	d2,(a3)+
	move.l	#8,(a4)+
	addq.l	#8,a0
.in_ws:
	dbf	d7,.rep_ws

;
; compute the colormap
; avarage all the entries from the big table with the best match
; in the smaller table
;

	move.l	csm_nimg(a5),d7
	move.l	csm_img(a5),a0			; point to large cmap
	move.l	csm_dev(a5),a1			; point to reduced cmap
	move.l	csm_ccnt(a5),a3
	bra	.in_clos
.rep_cl:
	move.l	csm_ndev(a5),d0
	bsr	closest_rgb
	lsl.l	#2,d0				; long words pointer	
	addq.l	#1,0(a3,d0.l)

	move.l	d0,d4
	add.l	d4,d4				; point to index

	moveq	#0,d1
	moveq	#0,d2
	moveq	#0,d3

	move.b	(a0),d1				; add the colors to the
	move.b	rgb_green(a0),d2		; found color
	move.b	rgb_blue(a0),d3

	move.l	csm_rsum(a5),a4
	add.l	d1,0(a4,d0.l)

	move.l	csm_gsum(a5),a4
	add.l	d2,0(a4,d0.l)

	move.l	csm_bsum(a5),a4
	add.l	d3,0(a4,d0.l)

	addq.l	#8,a0
.in_clos:
	dbf	d7,.rep_cl

;
; all the sums are calculated not calculate the actual colors
;

	move.l	csm_rsum(a5),a1
	move.l	csm_gsum(a5),a2
	move.l	csm_bsum(a5),a3
	move.l	csm_ccnt(a5),a4
	move.l	csm_dev(a5),a0
	
	moveq	#0,d6

	move.l	csm_ndev(a5),d7
	bra	.in_cc
.rep_cc:
	move.l	(a4)+,d1			; get count for this entry
	add.l	d1,d6

	move.l	(a1)+,d0			; sum red
	divu	d1,d0
	move.b	d0,(a0)

	move.l	(a2)+,d0
	divu	d1,d0
	move.b	d0,rgb_green(a0)

	move.l	(a3)+,d0
	divu	d1,d0
	move.b	d0,rgb_blue(a0)
	addq.l	#8,a0
.in_cc:
	dbf	d7,.rep_cc

.free_mem:
	move.l	csm_rsum(a5),a1
	move.l	csm_rsum_mem(a5),d0
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)	

.mem_err2:
	move.l	csm_ccnt(a5),a1
	move.l	csm_ccnt_mem(a5),d0
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)	

.mem_err1:
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	rts
*
* Create a conversion table which enhances the contrast of the colors
* in d0 the level
*
create_contrast_table:
	tst.w	d0
	beq	linear_table
	
	move.l	a1,a0
	cmp.w	#120,d0
	ble	.lev_oke
	moveq	#120,d0
.lev_oke:
	move.l	#256,d1
	sub.l	d0,d1		; the first and the last level entries
	move.l	d1,d5		; d5 hold the x value where the incr. stops
	sub.l	d0,d1		; are mapped to the low and high values
	move.l	d1,d2
	move.l	#256,d2
	lsl.l	#4,d2
	divu	d1,d2		; d2 holds the increment value * 16
	moveq	#0,d3		; d3 holds the level
	moveq	#0,d4		; d4 holds the x coordinate

	move.w	#254,d6
.contr:
	cmp.w	d4,d0
	bgt	.fixlow

	cmp.w	d4,d5
	blt	.fixhigh

	add.w	d2,d3
	move.w	d3,d1
	lsr.w	#4,d1
	move.b	d1,(a0)+
	bra	.con
.fixlow:
	move.b	#0,(a0)+
	bra	.con
.fixhigh:
	move.b	#255,(a0)+
.con:
	add.w	#1,d4
	dbf	d6,.contr	
	rts

linear_table:
	move.l	a1,a0
	moveq	#0,d1
	move.l	#254,d0
.rp:	move.b	d1,(a0)+
	addq.w	#1,d1
	dbf	d0,.rp
	rts
*
* enlarge contrast colormap
* contrast_colormap( RGB *pal a0, int entries d0, int value d1 );
*
_contrast_colormap:
	movem.l	d0-d7/a0-a6,-(a7)

	move.l	a0,a3

	move.l	d0,d7
	move.l	d1,d6

	move.l	#256,d0
	move.l	#MEMF_PUBLIC,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
	tst.l	d0
	beq	.contr_fail	

	move.l	d0,a1
	move.l	d6,d0

	movem.l	d0-d7/a0-a6,-(a7)
	bsr	create_contrast_table
	movem.l	(a7)+,d0-d7/a0-a6

	moveq	#0,d0
	moveq	#0,d1
	moveq	#0,d2
	bra	.in_cr
.rep_cr:
	move.b	(a3),d0
	move.b	rgb_green(a3),d1
	move.b	rgb_blue(a3),d2
	
	move.b	0(a1,d0.w),(a3)	
	move.b	0(a1,d1.w),rgb_green(a3)
	move.b	0(a1,d2.w),rgb_blue(a3)	
	addq.l	#8,a3
.in_cr:
	dbf	d7,.rep_cr

	move.l	#256,d0
	jsr	_LVOFreeMem(a6)
.contr_fail:

	movem.l	(a7)+,d0-d7/a0-a6
	rts

	IFNE	TESTING	
test:	
	bsr	fill
	lea	testp(pc),a0
	move.l	#255,d0
	move.l	#16,d1
	move.l	d1,-(a7)
	move.l	d0,-(a7)
	move.l	a0,-(a7)
	bsr	_reduce_palette
	add.l	#12,a7
	lea	testp2(pc),a0
	lea	testp(pc),a1
	move.l	#255,d0
	move.l	#16,d1
	move.l	d1,-(a7)
	move.l	a1,-(a7)
	move.l	d0,-(a7)
	move.l	a0,-(a7)
	bsr	_smooth_colormap
	add.l	#16,a7
	move.l	#16,d0
	lea	testp(pc),a0
	moveq	#20,d1
;	bsr	_contrast_colormap
	rts

fill:
	move.w	#255,d0
	lea	testp(pc),a0
	lea	testp2(pc),a1
	moveq	#0,d1
.rep:
	move.b	d1,(a0)
	move.b	d1,1(a0)
	move.b	d1,2(a0)

	move.b	d1,(a1)
	move.b	d1,1(a1)
	move.b	d1,2(a1)

	addq.l	#8,a0
	addq.l	#8,a1
	addq.w	#1,d1
	dbf	d0,.rep
	rts
	ENDC

black:	dc.b	0,0,0,0
	dc.l	0

	dc.w	$0000,$3f80,$3f01,$3e82,$3e04,$3d86,$3d09,$3c8c
	dc.w	$3c10,$3b94,$3b19,$3a9e,$3a24,$39aa,$3931,$38b8
	dc.w	$3840,$37c8,$3751,$36da,$3664,$35ee,$3579,$3504
	dc.w	$3490,$341c,$33a9,$3336,$32c4,$3252,$31e1,$3170
	dc.w	$3100,$3090,$3021,$2fb2,$2f44,$2ed6,$2e69,$2dfc
	dc.w	$2d90,$2d24,$2cb9,$2c4e,$2be4,$2b7a,$2b11,$2aa8
	dc.w	$2a40,$29d8,$2971,$290a,$28a4,$283e,$27d9,$2774
	dc.w	$2710,$26ac,$2649,$25e6,$2584,$2522,$24c1,$2460
	dc.w	$2400,$23a0,$2341,$22e2,$2284,$2226,$21c9,$216c
	dc.w	$2110,$20b4,$2059,$1ffe,$1fa4,$1f4a,$1ef1,$1e98
	dc.w	$1e40,$1de8,$1d91,$1d3a,$1ce4,$1c8e,$1c39,$1be4
	dc.w	$1b90,$1b3c,$1ae9,$1a96,$1a44,$19f2,$19a1,$1950
	dc.w	$1900,$18b0,$1861,$1812,$17c4,$1776,$1729,$16dc
	dc.w	$1690,$1644,$15f9,$15ae,$1564,$151a,$14d1,$1488
	dc.w	$1440,$13f8,$13b1,$136a,$1324,$12de,$1299,$1254
	dc.w	$1210,$11cc,$1189,$1146,$1104,$10c2,$1081,$1040
	dc.w	$1000,$0fc0,$0f81,$0f42,$0f04,$0ec6,$0e89,$0e4c
	dc.w	$0e10,$0dd4,$0d99,$0d5e,$0d24,$0cea,$0cb1,$0c78
	dc.w	$0c40,$0c08,$0bd1,$0b9a,$0b64,$0b2e,$0af9,$0ac4
	dc.w	$0a90,$0a5c,$0a29,$09f6,$09c4,$0992,$0961,$0930
	dc.w	$0900,$08d0,$08a1,$0872,$0844,$0816,$07e9,$07bc
	dc.w	$0790,$0764,$0739,$070e,$06e4,$06ba,$0691,$0668
	dc.w	$0640,$0618,$05f1,$05ca,$05a4,$057e,$0559,$0534
	dc.w	$0510,$04ec,$04c9,$04a6,$0484,$0462,$0441,$0420
	dc.w	$0400,$03e0,$03c1,$03a2,$0384,$0366,$0349,$032c
	dc.w	$0310,$02f4,$02d9,$02be,$02a4,$028a,$0271,$0258
	dc.w	$0240,$0228,$0211,$01fa,$01e4,$01ce,$01b9,$01a4
	dc.w	$0190,$017c,$0169,$0156,$0144,$0132,$0121,$0110
	dc.w	$0100,$00f0,$00e1,$00d2,$00c4,$00b6,$00a9,$009c
	dc.w	$0090,$0084,$0079,$006e,$0064,$005a,$0051,$0048
	dc.w	$0040,$0038,$0031,$002a,$0024,$001e,$0019,$0014
	dc.w	$0010,$000c,$0009,$0006,$0004,$0002,$0001,$0000
PythTable:
	dc.w	$0000,$0000,$0001,$0002,$0004,$0006,$0009,$000c
	dc.w	$0010,$0014,$0019,$001e,$0024,$002a,$0031,$0038
	dc.w	$0040,$0048,$0051,$005a,$0064,$006e,$0079,$0084
	dc.w	$0090,$009c,$00a9,$00b6,$00c4,$00d2,$00e1,$00f0
	dc.w	$0100,$0110,$0121,$0132,$0144,$0156,$0169,$017c
	dc.w	$0190,$01a4,$01b9,$01ce,$01e4,$01fa,$0211,$0228
	dc.w	$0240,$0258,$0271,$028a,$02a4,$02be,$02d9,$02f4
	dc.w	$0310,$032c,$0349,$0366,$0384,$03a2,$03c1,$03e0
	dc.w	$0400,$0420,$0441,$0462,$0484,$04a6,$04c9,$04ec
	dc.w	$0510,$0534,$0559,$057e,$05a4,$05ca,$05f1,$0618
	dc.w	$0640,$0668,$0691,$06ba,$06e4,$070e,$0739,$0764
	dc.w	$0790,$07bc,$07e9,$0816,$0844,$0872,$08a1,$08d0
	dc.w	$0900,$0930,$0961,$0992,$09c4,$09f6,$0a29,$0a5c
	dc.w	$0a90,$0ac4,$0af9,$0b2e,$0b64,$0b9a,$0bd1,$0c08
	dc.w	$0c40,$0c78,$0cb1,$0cea,$0d24,$0d5e,$0d99,$0dd4
	dc.w	$0e10,$0e4c,$0e89,$0ec6,$0f04,$0f42,$0f81,$0fc0
	dc.w	$1000,$1040,$1081,$10c2,$1104,$1146,$1189,$11cc
	dc.w	$1210,$1254,$1299,$12de,$1324,$136a,$13b1,$13f8
	dc.w	$1440,$1488,$14d1,$151a,$1564,$15ae,$15f9,$1644
	dc.w	$1690,$16dc,$1729,$1776,$17c4,$1812,$1861,$18b0
	dc.w	$1900,$1950,$19a1,$19f2,$1a44,$1a96,$1ae9,$1b3c
	dc.w	$1b90,$1be4,$1c39,$1c8e,$1ce4,$1d3a,$1d91,$1de8
	dc.w	$1e40,$1e98,$1ef1,$1f4a,$1fa4,$1ffe,$2059,$20b4
	dc.w	$2110,$216c,$21c9,$2226,$2284,$22e2,$2341,$23a0
	dc.w	$2400,$2460,$24c1,$2522,$2584,$25e6,$2649,$26ac
	dc.w	$2710,$2774,$27d9,$283e,$28a4,$290a,$2971,$29d8
	dc.w	$2a40,$2aa8,$2b11,$2b7a,$2be4,$2c4e,$2cb9,$2d24
	dc.w	$2d90,$2dfc,$2e69,$2ed6,$2f44,$2fb2,$3021,$3090
	dc.w	$3100,$3170,$31e1,$3252,$32c4,$3336,$33a9,$341c
	dc.w	$3490,$3504,$3579,$35ee,$3664,$36da,$3751,$37c8
	dc.w	$3840,$38b8,$3931,$39aa,$3a24,$3a9e,$3b19,$3b94
	dc.w	$3c10,$3c8c,$3d09,$3d86,$3e04,$3e82,$3f01,$3f80

	IFNE	TESTING	
testp:
	dc.w	0
	blk.l	4000,0			; space for 2000 entries
testp2:
	dc.w	0
	blk.l	4000,0			; space for 2000 entries
	ENDC
