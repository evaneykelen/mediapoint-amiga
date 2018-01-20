*
* File	: findpal24.s
* Uses	:
* Date	: 24 january 1994 
* Author: ing. C. Lieshout
* Desc	: Find pallete from 24bits picture

	INCDIR	"include:"
	include	"exec/memory.i"
	incdir	"pascal:include/"
	include	"misctools.i"
	incdir	"wp:inclibs/"
	include	"exec_lib.i"

COLORSQ_SIZE = 16384

	RSRESET
im_block:	rs.w	0
im_width:	rs.l	1
im_height:	rs.l	1
im_depth:	rs.w	1
im_width_bytes:	rs.l	1
im_planar:	rs.l	1
im_planar_size:	rs.l	1
im_rgb:		rs.l	1
im_rgb_size:	rs.l	1
im_colorsquare:	rs.l	1
im_modulo:	rs.l	1
im_bm24:	rs.l	1
im_colsize:	rs.l	1
im_colmap:	rs.l	1
im_planes:	rs.l	24
im_SIZEOF:	rs.w	0

get_colsquare:
	move.l	#COLORSQ_SIZE,d0
	move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,im_colorsquare(a5)
	rts

get_rgb:
	move.l	im_width(a5),d0
	move.l	d0,d1
	add.l	d0,d0
	add.l	d1,d0					; mul by 3
	move.l	d0,im_rgb_size(a5)
	move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,im_rgb(a5)
	rts
*
* FindPal24( struct Bitmap24 *bm, UBYTE *cmap, long palsize )
* returns 0 when failed ( memory )
*
_FindPal24:
	link	a5,#-im_SIZEOF
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a0
	move.l	12(a5),a1
	move.l	16(a5),d0

	sub.l	#im_SIZEOF,a5
	move.l	a0,im_bm24(a5)
	move.l	a1,im_colmap(a5)
	move.l	d0,im_colsize(a5)
	moveq	#0,d1
	move.w	bm24_Modulo(a0),d1
	move.l	d1,im_width_bytes(a5)	
	lsl.l	#3,d1
	move.l	d1,im_width(a5)
	moveq	#0,d1
	move.w	bm24_Rows(a0),d1
	move.l	d1,im_height(a5)
	lea	bm24_Planes(a0),a2
	lea	im_planes(a5),a3
	moveq	#23,d0
.copy_planes:
	move.l	(a2)+,(a3)+
	dbf	d0,.copy_planes

	clr.l	im_modulo(a5)

	move.l	im_planes+4(a5),d0
	sub.l	im_planes(a5),d0

	cmp.l	im_width_bytes(a5),d0		; interleaved
	beq	.mod0
	
	move.l	im_width_bytes(a5),d0
	move.w	#24,d1
	subq.w	#1,d1
	mulu	d1,d0
	move.l	d0,im_modulo(a5)
.mod0:
	clr.l	im_rgb(a5)
	clr.l	im_colorsquare(a5)

	bsr	get_colsquare
	beq	.init_error
	bsr	get_rgb
	beq	.init_error

	move.l	im_rgb(a5),a6		; point to rgb data to be filled

	move.l	im_height(a5),d6
	subq.l	#1,d6
.rep_rgb1:
	move.l	a6,-(a7)
	lea	im_planes(a5),a0
	bsr	get_4planes		; get R data
	lea	im_planes+32(a5),a0
	bsr	get_4planes		; get G data
	lea	im_planes+64(a5),a0
	bsr	get_4planes		; get B data
	move.l	(a7)+,a4
	bsr	count_pens_line
	dbf	d6,.rep_rgb1

	bsr	find_best_cols2

	bsr	free_mem

	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#1,d0
	rts	

.init_error:
	bsr	free_mem
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts	
*
* a5 points to im_block
* a4 points to chunky rgb line
*
count_pens_line:
	move.l	d6,-(a7)
	move.l	im_colorsquare(a5),a2
	move.w	#$000e,d3
	move.l	im_width(a5),d6
	subq.l	#1,d6			; number off pixels
.rep_rgb:
	move.l	im_width(a5),d7
	moveq	#0,d0
	moveq	#0,d1
	moveq	#0,d2
	move.b	0(a4,d7),d1
	add.l	d7,d7
	move.b	0(a4,d7),d2
	move.b	(a4)+,d0
	and.w	d3,d0
	and.w	d3,d1
	and.w	d3,d2
	lsl.w	#7,d0
	lsl.w	#4,d1
	lsl.w	#1,d2
	or.w	d1,d0
	or.w	d2,d0
	add.l	#1,0(a2,d0)
	dbf	d6,.rep_rgb
	move.l	(a7)+,d6
	rts


find_best_cols2:
	move.l	im_colorsquare(a5),a0
	move.l	im_colmap(a5),a1
	moveq	#0,d7			; 16 colors to divide
.rep:
	bsr	find_high
	move.l	d0,d4
	move.w	d2,d0

	and.w	#$e0,d0
	move.w	d2,d1
	and.w	#$1c,d1
	lsl.w	#3,d1
	and.w	#$3,d2
	lsl.w	#6,d2

	add.w	#16,d0
	add.w	#16,d1
	add.w	#16,d2
	
	move.b	d0,(a1)+
	move.b	d1,(a1)+
	move.b	d2,(a1)+
	addq.w	#1,d7
	cmp.l	im_colsize(a5),d7
	blt	.rep
	rts
*
* Find the entry with the most colors 
*
find_high:
	moveq	#0,d0
	moveq	#0,d1
	moveq	#0,d2
	move.l	im_colorsquare(a5),a0
	move.l	#511,d6
.ff:
	cmp.l	(a0)+,d0
	bgt	.no_store
	move.l	-4(a0),d0
	move.w	d1,d2
.no_store:
	add.w	#1,d1
	dbf	d6,.ff
	move.l	d2,d1
	add.l	d1,d1
	add.l	d1,d1
	move.l	im_colorsquare(a5),a0
	clr.l	0(a0,d1.w)		; clear this entry
	rts

free_mem:
	tst.l	im_colorsquare(a5)
	beq	.noc
	move.l	$4.w,a6
	move.l	im_colorsquare(a5),a1
	move.l	#COLORSQ_SIZE,d0
	jsr	_LVOFreeMem(a6)
	clr.l	im_colorsquare(a5)
.noc:
	tst.l	im_rgb(a5)
	beq	.norgb

	move.l	$4.w,a6
	move.l	im_rgb(a5),a1
	move.l	im_rgb_size(a5),d0
	jsr	_LVOFreeMem(a6)
	clr.l	im_rgb(a5)
.norgb:
	rts
*
* a0 points to planes
* a6 points to chunky data
*
get_4planes:
	movem.l	a0/a3/a5,-(a7)

	lea.l	16(a0),a4
	move.l	(a4)+,a0		; use only the most significant bits
	move.l	(a4)+,a1
	move.l	(a4)+,a2
	move.l	(a4)+,a3

	move.l	im_width(a5),d7
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
	movem.l	(a7)+,a0/a3/a5

	move.l	im_modulo(a5),d0
	add.l	im_width_bytes(a5),d0
	add.l	d0,16(a0)			; pointer to next line
	add.l	d0,20(a0)
	add.l	d0,24(a0)
	add.l	d0,28(a0)
	rts

