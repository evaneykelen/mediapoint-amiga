*
* File	: chunk_to_plane.s
* Uses	:
* Date	: 22 august 1993 
* Author: ing. C. Lieshout
* Desc	: convert from colorpen data to plane data

	incdir	"include:"
	include	"graphics/view.i"
	include	"exec/memory.i"
	include	"graphics/gfxbase.i"
	include	"hardware/custom.i"
	INCDIR	"wp:inclibs/"
	include	"graphics_libv39.i"
	include	"exec_lib.i"

; typedef vi_block
	RSRESET
vi_block:	rs.w	0
vi_view:	rs.b	v_SIZEOF
vi_viewport:	rs.b	vp_SIZEOF
vi_rasinfo:	rs.b	ri_SIZEOF
vi_bitmap:	rs.b	bm_SIZEOF
vi_SIZEOF:	rs.w	0

; typedef data block
	RSRESET
dt_block:	rs.w	0
dt_screenmem:	rs.l	1
dt_breedte_x:	rs.l	1
dt_leny:	rs.w	1
dt_lenx:	rs.w	1
dt_planes:	rs.w	1
dt_mode:	rs.w	1
dt_colors:	rs.w	32
dt_vi_block:	rs.b	vi_SIZEOF
dt_SIZEOF	rs.w	0

; typedef object struct
	RSRESET
os_block	rs.w	0
os_x		rs.w	1
os_y		rs.w	1
os_width	rs.w	1
os_heigth	rs.w	1
os_width_bytes	rs.l	1
os_modulo	rs.w	1
os_obj_ptr	rs.l	1
os_mask_ptr	rs.l	1
os_obj_size	rs.l	1
os_blt_size	rs.w	1
os_bitmap:	rs.b	bm_SIZEOF
os_SIZEOF	rs.w	0

MAXY		equ	256
MAXX		equ	320
MAXPLANES	equ	4
MYMODE		equ	0
MEMSIZE		equ	150*1024

start:	lea	screen_struct(pc),a3
	lea	dt_vi_block(a3),a5

	move.l	4,a6
	move.l	#MEMSIZE,d0
	move.l	#MEMF_CHIP+MEMF_CLEAR,d1
	jsr	_LVOAllocMem(a6)
	move.l	d0,screen_mem
	beq.w	no_mem

	lea	gfx_name(pc),a1
	moveq.l	#0,d0
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,gfx_base
	beq.w	no_gfx

	move.l	d0,a6
	move.l	gb_ActiView(a6),old_view

	move.w	#MAXX,d0
	move.w	#MAXY,d1
	move.w	#MAXPLANES,d2
	move.w	#MYMODE,d3
	move.l	screen_mem(pc),d4
	lea	ball(pc),a0
	bsr.w	init_dt_block

	bsr.w	create_viewport
	bsr.w	show_viewport

	bsr	init_timer
	bne	no_obj

	move.l	a5,-(a7)
	lea	secs(pc),a5
	bsr	get_time
	move.l	(a7)+,a5

	moveq	#20,d0
.ww:
	jsr	do_chunks
	bsr	re_show
	bsr	clear_screen
	dbf	d0,.ww

	bsr	re_show
.ww2:
;	btst	#6,$bfe001
;	bne	.ww2

	movem.l	a4/a5,-(a7)
	lea	secs2(pc),a5
	bsr	get_time
	lea	secs2(pc),a5
	lea	secs(pc),a4
	move.l	(a5),d0
	sub.l	(a4),d0
	and.l	#$ffff,d0
	swap	d0
	move.l	4(a5),d1
	and.l	#$ffff0000,d1
	swap	d1
	add.l	d1,d0
	move.l	4(a4),d1
	and.l	#$ffff0000,d1
	swap	d1
	sub.l	d1,d0	
	move.l	d0,tijd
	movem.l	(a7)+,a4/a5
	
	bsr	remove_timer

	move.l	gfx_base(pc),a6
	move.l	old_view(pc),a1
	jsr	_LVOLoadView(a6)
	bsr.w	free_view

no_obj:	move.l	4,a6
	move.l	gfx_base(pc),a1
	jsr	_LVOCloseLibrary(a6)

no_gfx:	move.l	4,a6
	move.l	screen_mem(pc),a1
	move.l	#MEMSIZE,d0
	jsr	_LVOFreeMem(a6)

no_mem:	rts
secs:	dc.l	0,0
secs2:	dc.l	0,0
tijd:	dc.l	0

show_viewport:
	move.l		gfx_base(pc),a6
	lea		vi_view(a5),a0
	lea		vi_viewport(a5),a1
	jsr		_LVOMakeVPort(a6)

	lea		vi_view(a5),a1
	jsr		_LVOMrgCop(a6)

re_show:
	move.l	d0,-(a7)
	move.l		gfx_base(pc),a6
	lea		vi_view(a5),a1
	jsr		_LVOLoadView(a6)
	move.l	(a7)+,d0
	rts

*
* a5 points to a vi_block with view viewport etc.
*		
create_viewport:
	move.l		gfx_base(pc),a6
	lea		vi_view(a5),a1
	jsr		_LVOInitView(a6)

	lea		vi_viewport(a5),a0
	jsr		_LVOInitVPort(a6)

	lea		vi_view(a5),a0
	lea		vi_viewport(a5),a1
	move.l		a1,v_ViewPort(a0)
	move.w		dt_mode(a3),v_Modes(a0)

	lea		vi_bitmap(a5),a0
	move.w		dt_planes(a3),d0		; depth

	move.l		dt_breedte_x(a3),d1		; breedte in bytes * 8
no_modu:
	lsl.l		#3,d1				; width
	mulu		dt_planes(a3),d1		; interleaved
	move.w		dt_leny(a3),d2			; height
	jsr		_LVOInitBitMap(a6)

	lea		vi_bitmap(a5),a0

	move.l		dt_screenmem(a3),d0
	move.l		dt_breedte_x(a3),d1
;	mulu		dt_leny(a3),d1			; non-interleaved
no_modu2:
	move.l		dt_screenmem(a3),d0

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

	lea		vi_rasinfo(a5),a0		; initialiseer rasinfo
	lea		vi_bitmap(a5),a1
	move.l		a1,ri_BitMap(a0)
	move.w		#0,ri_RxOffset(a0)
	move.w		#0,ri_RyOffset(a0)
	move.l		#0,ri_Next(a0)

	lea		vi_viewport(a5),a0
	lea		vi_rasinfo(a5),a1
	move.l		a1,vp_RasInfo(a0)
	move.w		dt_lenx(a3),vp_DWidth(a0)
	move.w		dt_leny(a3),vp_DHeight(a0)
	move.w		dt_mode(a3),vp_Modes(a0)

	moveq		#32,d0
	jsr		_LVOGetColorMap(a6)
	beq.b		no_colormap
	lea		vi_viewport(a5),a0
	move.l		d0,vp_ColorMap(a0)
	lea		vi_viewport(a5),a0
	lea		dt_colors(a3),a1
	moveq		#32,d0
	jsr		_LVOLoadRGB4(a6)
	rts
no_colormap:
	moveq.l	#-1,d0
	rts

free_view:
	move.l	gfx_base(pc),a6
	lea	vi_viewport(a5),a0
	tst.l	vp_ColorMap(a0)
	beq.s	noColor
	move.l	vp_ColorMap(a0),a0
	jsr	_LVOFreeColorMap(a6)
	lea	vi_viewport(a5),a0
	clr.l	vp_ColorMap(a0)
noColor:move.l	vi_viewport(a5),a0
	jsr	_LVOFreeVPortCopLists(a6)
	lea	vi_view(a5),a0
	tst.l	v_LOFCprList(a0)
	beq.s	noLOFmem
	move.l	v_LOFCprList(a0),a0
	jsr	_LVOFreeCprList(a6)
	lea	vi_view(a5),a0
	clr.l	v_LOFCprList(a0)
noLOFmem:
	lea	vi_view(a5),a0
	tst.l	v_SHFCprList(a0)
	beq.s	noSHFmem
	move.l	v_SHFCprList(a0),a0
	jsr	_LVOFreeCprList(a6)
	lea	vi_view(a5),a0
	clr.l	v_SHFCprList(a0)
noSHFmem:
	rts

* d0->lenx
* d1->leny
* d2->planes
* d3->mode
* d4->screenmem
* a0->colors
init_dt_block:
	move.w	d0,dt_lenx(a3)
	move.w	d1,dt_leny(a3)
	move.w	d2,dt_planes(a3)
	move.w	d3,dt_mode(a3)
	move.l	d4,dt_screenmem(a3)
	ext.l	d0
	add.l	#15,d0
	lsr.l	#4,d0
	add.l	d0,d0
	move.l	d0,dt_breedte_x(a3)

	moveq	#0,d1
	move.w	#31,d0
	lea	dt_colors(a3),a1
init_dt_loop:
	move.w	d1,(a1)+
	add.w	#$111,d1
	dbf	d0,init_dt_loop
	rts

	lea	dt_colors(a3),a1
	move.w	#$000,(a1)+
	move.w	#$555,(a1)+
	move.w	#$aaa,(a1)+
	move.w	#$fff,(a1)+
	move.w	#$0,(a1)+
	move.w	#$0,(a1)+
	move.w	#$0,(a1)+
	move.w	#$0,(a1)+
	move.w	#$0,(a1)+
	move.w	#$0,(a1)+
	move.w	#$0,(a1)+
	move.w	#$0,(a1)+
	move.w	#$0,(a1)+
	move.w	#$0,(a1)+
	move.w	#$0,(a1)+
	move.w	#$0,(a1)+
	rts

old_view:	dc.l	0

gfx_base:	dc.l	0
screen_mem:	dc.l	0
screen_struct:	blk.b	dt_SIZEOF,0

gfx_name:	dc.b	'graphics.library',0
	even

clear_screen:
	movem.l	a0/d0,-(a7)
	move.l	dt_breedte_x(a3),d0
	lsr.l	#2,d0				; longs
	mulu	dt_leny(a3),d0
	mulu	dt_planes(a3),d0
	move.l	dt_screenmem(a3),a0
	moveq	#0,d1
cc:	move.l	d1,(a0)+
	subq.l	#1,d0
	bne	cc
	movem.l	(a7)+,a0/d0
	rts
	
* don't touch a3 & a5

	INCDIR	"include:"
	INCLUDE	"devices/timer.i"

ciaapra =$bfe001
openlibrary= -30-522
execbase = 4

freemem=-210
allocmem=-198
forbid=-132
permit=-138
findtask=-294
addport=-354
remport=-360
openlib=-408
closelib=-414
opendev=-444
closedev=-450
doio=-456

init_timer:
	movem.l	d1-d7/a0-a6,-(a7)
	move.l	execbase,a6
	sub.l	a1,a1
	jsr	findtask(a6)
	lea.l	readreply(pc),a1
	move.l	d0,MP_SIGTASK(a1)

	move.l	execbase,a6
	jsr	addport(a6)
	
	lea	diskio(pc),a1
	move.l	#0,d0
	clr.l	d1
	lea	timerdevice(pc),a0
	jsr	opendev(a6)
	movem.l	(a7)+,d1-d7/a0-a6
	tst.l	d0
	rts

get_time:	
	movem.l	d0-d7/a0-a6,-(a7)
	lea	diskio(pc),a1
	lea	readreply(pc),a2
	move.l	a2,TIMEREQUEST+IO+MN_REPLYPORT(a1)
	move.b	#NT_MESSAGE,TIMEREQUEST+IO+MN+LN_TYPE(a1)
	move.b	#0,TIMEREQUEST+IO+MN+LN_PRI(a1)
	move.b	#0,TIMEREQUEST+IO+MN+LN_NAME(a1)
	move.w	#TR_GETSYSTIME,TIMEREQUEST+IO_COMMAND(a1)
	move.l	execbase,a6
	jsr	doio(a6)

	lea	diskio(pc),a1
;	move.l	IOTV_TIME+TV_SECS(a1),secs
;	move.l	IOTV_TIME+TV_MICRO(a1),mics
	move.l	IOTV_TIME+TV_SECS(a1),(a5)+
	move.l	IOTV_TIME+TV_MICRO(a1),(a5)+
	movem.l	(a7)+,d0-d7/a0-a6
	rts

remove_timer:
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	execbase,a6
	lea	readreply(pc),a1
	jsr	remport(a6)
	
	move.l	execbase,a6
	lea	diskio(pc),a1
	jsr	closedev(a6)
	movem.l	(a7)+,d0-d7/a0-a6
	rts

timerdevice:	dc.b	'timer.device',0
	even

diskio:		blk.b	IOTV_SIZE,0
readreply:	blk.b	MP_SIZE,0



	INCDIR	"ram:"
ball:	INCBIN	"test.uit"
bend:	dc.l	0
	even

*
* Try to dither a picture to less grey colors
* use the im_struct pointed at by a0
*
dither_pic:
	movem.l	d0-d7/a0-a6,-(a7)
	lea	burkes(pc),a3
	lea	burkes_rev(pc),a4
	lea	floyd(pc),a3
	lea	floyd_rev(pc),a4
;	bsr	contrast
	move.l	a0,a5
	move.l	im_chunk(a5),a0		; pointer to chunky data
	move.l	a0,a2
	move.l	im_width(a5),d5
	move.l	im_height(a5),d7
	subq.l	#1,d7
	bra	.in_dp_h
.rep_dp_h:
	move.l	im_width(a5),d6
	subq.w	#3,d6
	move.l	a2,a0
	add.l	d5,a2
.rep_dp_w:
	move.b	(a0),d0			; get pixel value
	move.b	d0,d1
	lsr.b	#2,d0			; scale down to 4 colors
	lsl.b	#2,d0
	sub.b	d0,d1			; difference with orginal color
;	bsr	floyd
	jsr	(a3)
	addq.l	#1,a0
.in_dp_w:
	subq.w	#1,d6
	bne	.rep_dp_w

	subq.w	#1,d7
	beq	.ex_dp_w
	move.l	im_width(a5),d6
	subq.w	#3,d6
	add.l	d5,a2
	move.l	a2,a0
	subq.l	#2,a0
.rep_dp_w2:
	subq.l	#1,a0
	move.b	(a0),d0			; get pixel value
	move.b	d0,d1
	lsr.b	#2,d0			; scale down to 4 colors
	lsl.b	#2,d0
	sub.b	d0,d1			; difference with orginal color
	jsr	(a4)
;	bsr	floyd_rev
	subq.w	#1,d6
	bne	.rep_dp_w2
.ex_dp_w:
.in_dp_h:
	subq.w	#1,d7
	bne	.rep_dp_h
	movem.l	(a7)+,d0-d7/a0-a6
	rts

*
* Apply Floyd-Streinberg on pixel pointed at by (a0)
* The new value is in d0.b
* The error is in d1.b
* The width is in d5
* Use the following error diffusion diagram
*
*		X	7
*	3	5	1
*
floyd:
	ext.w	d1
	ext.w	d0
	move.b	c_tab(pc,d0.w),(a0)
	move.w	d1,d0
	lsl.w	#3,d0
	sub.w	d1,d0			; err * 7
	lsr.w	#4,d0
	add.b	d0,1(a0)
	move.w	d1,d0
	add.w	d0,d0
	add.w	d1,d0			; err * 3
	lsr.w	#4,d0
	add.b	d0,-1(a0,d5.w)
	move.w	d1,d0
	add.w	d0,d0
	add.w	d0,d0
	add.w	d1,d0			; err * 5
	lsr.w	#4,d0
	add.b	d0,0(a0,d5.w)
	lsr.w	#4,d1
	add.b	d1,1(a0,d5.w)		; err * 1
	rts

floyd_rev:
	ext.w	d1
	ext.w	d0
	move.b	c_tab(pc,d0.w),(a0)
	move.w	d1,d0
	lsl.w	#3,d0
	sub.w	d1,d0			; err * 7
	lsr.w	#4,d0
	add.b	d0,-1(a0)
	move.w	d1,d0
	add.w	d0,d0
	add.w	d1,d0			; err * 3
	lsr.w	#4,d0
	add.b	d0,1(a0,d5.w)
	move.w	d1,d0
	add.w	d0,d0
	add.w	d0,d0
	add.w	d1,d0			; err * 5
	lsr.w	#4,d0
	add.b	d0,0(a0,d5.w)
	lsr.w	#4,d1
	add.b	d1,-1(a0,d5.w)		; err * 1
	rts

c_tab:
	dc.b	0,0,0,0
	dc.b	1,1,1,1
	dc.b	2,2,2,2
	dc.b	3,3,3,3
	dc.b	3,3,3,3
	dc.b	3,3,3,3
	dc.b	$a,$a,$a,$a
	dc.b	$f,$f,$f,$f
	dc.b	$f,$f,$f,$f
	dc.b	$f,$f,$f,$f
	dc.b	$f,$f,$f,$f
	dc.b	$f,$f,$f,$f
*
* Apply Burkes on pixel pointed at by (a0)
* The new value is in d0.b
* The error is in d1.b
* The width is in d5
* Use the following error diffusion diagram
*
*			X	8	4
*	2	4	8	4	2
*
burkes:
	ext.w	d1
	ext.w	d0
	move.b	c_tab(pc,d0.w),(a0)
	move.w	d1,d0
	lsr.w	#2,d0
	add.b	d0,1(a0)		; + 8 * err
	add.b	d0,0(a0,d5.w)		; + 8 * err
	move.w	d1,d0
	lsr.w	#4,d0
	add.b	d0,2(a0)		; + 4 * err
	add.b	d0,1(a0,d5.w)		; + 4 * err	
	add.b	d0,-1(a0,d5.w)		; + 4 * err	
	lsr.w	#4,d1
	add.b	d1,-2(a0,d5.w)		; + 2 * err
	add.b	d1,2(a0,d5.w)		; + 2 * err
	rts

burkes_rev:
	ext.w	d1
	ext.w	d0
	move.b	c_tab(pc,d0.w),(a0)
	move.w	d1,d0
	lsr.w	#2,d0
	add.b	d0,-1(a0)		; + 8 * err
	add.b	d0,0(a0,d5.w)		; + 8 * err
	move.w	d1,d0
	lsr.w	#4,d0
	add.b	d0,-2(a0)		; + 4 * err
	add.b	d0,-1(a0,d5.w)		; + 4 * err	
	add.b	d0,1(a0,d5.w)		; + 4 * err	
	lsr.w	#4,d1
	add.b	d1,-2(a0,d5.w)		; + 2 * err
	add.b	d1,2(a0,d5.w)		; + 2 * err
	rts

do_chunks:
	movem.l	d0-d7/a0-a6,-(a7)
	lea	imb(pc),a0
	lea	vi_bitmap(a5),a1
	move.l	a1,im_bitmap(a0)
	move.l	#MAXX,im_width(a0)
	move.l	#MAXY,im_height(a0)
	move.l	#MAXPLANES,im_depth(a0)
	lea	ball,a2
	move.l	a2,im_chunk(a0)
;	bsr	dither_pic
	move.l	a0,-(a7)
	bsr	_ChunkToPlane
	move.l	(a7)+,a0

	move.l	a0,-(a7)
	bsr	_PlaneToChunk
	move.l	(a7)+,a0
;pp:
	move.l	a0,-(a7)
	bsr	_ChunkToPlane
	move.l	(a7)+,a0


	movem.l	(a7)+,d0-d7/a0-a6
	rts

	INCDIR	"include:"
	INCLUDE	"graphics/gfx.i"

	RSRESET
im_block:	rs.w	0
im_width:	rs.l	1
im_height:	rs.l	1
im_depth:	rs.l	1
im_chunk:	rs.l	1
im_bitmap:	rs.l	1
im_SIZEOF:	rs.w	0
	RSRESET
ch_block:	rs.w	0
ch_wbytes:	rs.l	1
ch_mod:		rs.l	1
ch_im:		rs.l	1
ch_planes:	rs.l	24
ch_SIZEOF:	rs.w	0

	XDEF	_ChunkToPlane
*
* Usage : ChunkToPlane( struct image *im );
*
*
_ChunkToPlane:
	link a5,#-ch_SIZEOF
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a4		; pointer to image struct
	sub.l	#ch_SIZEOF,a5	
	tst.l	im_height(a4)
	beq	no_conv
	tst.l	im_width(a4)
	beq	no_conv

	move.l	a4,ch_im(a5)
	move.l	im_bitmap(a4),a3
	moveq	#0,d7
	move.b	bm_Depth(a3),d7
	beq	no_conv
	cmp.b	#4,d7
	bne	no_4planes
	bsr	do_4planes3
;	bsr	do_var_planes
	bra	no_conv
no_4planes:
	cmp.w	#8,d7
	bne	no_8planes

	bsr	do_4planes
;	bsr	do_s4planes
	
no_8planes:
no_conv:
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	rts

_PlaneToChunk:
	link a5,#-ch_SIZEOF
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a4		; pointer to image struct
	sub.l	#ch_SIZEOF,a5	
	tst.l	im_height(a4)
	beq	no_conv2
	tst.l	im_width(a4)
	beq	no_conv2

	move.l	a4,ch_im(a5)
	move.l	im_bitmap(a4),a3
	moveq	#0,d7
	move.b	bm_Depth(a3),d7
	beq	no_conv2
	cmp.b	#4,d7
	bne	no_4planes2
	bsr	get_4planes
	bra	no_conv
no_4planes2:
	cmp.w	#8,d7
	bne	no_8planes2
	bsr	get_4planes
no_8planes2:
no_conv2:
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	rts

init_planes:
	move.l	im_bitmap(a4),a3
	moveq	#0,d0
	move.w	bm_BytesPerRow(a3),d0
	move.l	d0,ch_wbytes(a5)

	move.l	im_width(a4),d7
	add.l	#15,d7
	lsr.l	#4,d7
	add.l	d7,d7				; width one line
	sub.l	d7,d0
	move.l	d0,ch_mod(a5)
	lea	bm_Planes(a3),a6
	lea	ch_planes(a5),a0
	moveq	#0,d0
	move.b	bm_Depth(a3),d0
	bra	.in
.rep_c:
	move.l	(a6)+,(a0)+
.in:	dbf	d0,.rep_c
	lea	bm_Planes(a3),a6
	rts

*
* Fill d0 and d1 with the planar data from plane 0,1,2,3
*
get_d0d1:
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

	rts

get_4planes:
	movem.l	a4/a5,-(a7)
	bsr	init_planes
	move.l	(a6)+,a0
	move.l	(a6)+,a1
	move.l	(a6)+,a2
	move.l	(a6)+,a3

	move.l	im_chunk(a4),a6

	move.l	im_height(a4),d6	
	subq.l	#1,d6
	move.l	im_width(a4),d7
	move.l	ch_mod(a5),a4
.rep_y:
	move.l	#0,a5
.rep_x:
	bsr	get_d0d1

; D0 = a2b2c2d2e2f2g2h2 a0b0c0d0e0f0g0h0
; D1 = a3b3c3d3e3f3g3h3 a1b1c1d1e1f1g1h1

	move.w	d1,d2
	and.w	#$aaaa,d2
	and.w	#$5555,d1

; D1 = --b3--d3--f3--h3 --b1--d1--f1--h1
; d2 = a3--c3--e3--g3-- a1--c1--e1--g1--

	move.w	d0,d3

	and.w	#$aaaa,d0
	and.w	#$5555,d3

; D0 = a2--c2--e2--g2-- a0--c0--e0--g0--
; D3 = --b2--d2--f2--h2 --b0--d0--f0--h0

	lsr.w	#1,d0

; D0 = --a2--c2--e2--g2 --a0--c0--e0--g0

	or.w	d2,d0

; D0 = a3a2c3c2e3e2g3g2 a1a0c1c0e1e0g1g0

	add.w	d1,d1

; D1 = b3--d3--f3--h3-- b1--d1--f1--h1--
	
	or.w	d3,d1

; D1 = b3b2d3d2f3f2h3h2 b1b0d1d0f1f0h1h0

	move.w	d1,d2
	and.w	#$ff,d2
	and.w	#$ff00,d1

; D1 = b3b2d3d2f3f2h3h2 ---------------
; D2 = ---------------- b1b0d1d0f1f0h1h0

	move.w	d0,d3
	and.w	#$ff,d0
	and.w	#$ff00,d3

; D0 = ---------------- a1a0c1c0e1e0g1g0
; D3 = a3a2c3c2e3e2g3g2 ----------------

	lsr.w	#8,d3

; D3 = ---------------- a3a2c3c2e3e2g3g2

	lsl.w	#8,d2
	
; D2 = b1b0d1d0f1f0h1h0 ----------------

	or.w	d2,d0
	or.w	d3,d1
	
; D0 = b1b0d1d0f1f0h1h0 a1a0c1c0e1e0g1g0
; D1 = b3b2d3d2f3f2h3h2 a3a2c3c2e3e2g3g2

	move.w	d1,d2
	and.w	#$3333,d1
	and.w	#$cccc,d2

; D1 = ----d3d2----h3h2 ----c3c2----g3g2
; D2 = b3b2----f3f2---- a3a2----e3e2----

	move.w	d0,d3
	and.w	#$cccc,d0
	and.w	#$3333,d3


; D0 = b1b0----f1f0---- a1a0----e1e0----
; D3 = ----d1d0----h1h0 ----c1c0----g1g0

	lsl.w	#2,d1

; D1 = d3d2----h3h2---- c3c2----g3g2----

	or.w	d3,d1
		
; D1 = d3d2d1d0h3h2h1h0 c3c2c1c0g3g2g1g0

	lsr.w	#2,d0

; D0 = ----b1b0----f1f0 ----a1a0----e1e0

	or.w	d2,d0
	
; D0 = b3b2b1b0f3f2f1f0 a3a2a1a0e3e2e1e0
; D1 = d3d2d1d0h3h2h1h0 c3c2c1c0g3g2g1g0

	swap	d0

; D0 = b3b2b1b0f3f2f1f0 a3a2a1a0e3e2e1e0 ---------------- ----------------

	move.w	d1,d0

; D0 = b3b2b1b0f3f2f1f0 a3a2a1a0e3e2e1e0 d3d2d1d0h3h2h1h0 c3c2c1c0g3g2g1g0

	swap	d0
	
; D0 = d3d2d1d0h3h2h1h0 c3c2c1c0g3g2g1g0 b3b2b1b0f3f2f1f0 a3a2a1a0e3e2e1e0

	move.l	d0,d3

	and.l	#$0f0f0f0f,d3
	and.l	#$f0f0f0f0,d0

; D0 = d3d2d1d0-------- c3c2c1c0-------- b3b2b1b0-------- a3a2a1a0--------
; D3 = --------h3h2h1h0 --------g3g2g1g0 --------f3f2f1f0 --------e3e2e1e0

	lsr.l	#4,d0

; D0 = --------d3d2d1d0 --------c3c2c1c0 --------b3b2b1b0 --------a3a2a1a0

	ror.w	#8,d0
	swap	d0
	ror.w	#8,d0

; D0 = abcd

	ror.w	#8,d3
	swap	d3
	ror.w	#8,d3

; d3 = efgh

	move.l	d0,(a6)+
	move.l	d3,(a6)+

	addq.w	#8,a5	
	cmp.w	d7,a5
	blt	.rep_x
	add.l	a4,a0			; pointer to next line
	add.l	a4,a1
	add.l	a4,a2
	add.l	a4,a3
	dbf	d6,.rep_y	
	movem.l	(a7)+,a4/a5
	rts

*
* Do a chunk to planar on 4 planes
*
do_4planes:
	movem.l	a4/a5,-(a7)
	bsr	init_planes
	move.l	(a6)+,a0
	move.l	(a6)+,a1
	move.l	(a6)+,a2
	move.l	(a6)+,a3

	move.l	im_chunk(a4),a6

	move.l	im_height(a4),d6	
	subq.l	#1,d6
	move.l	im_width(a4),d7
	move.l	ch_mod(a5),a4
.rep_y
	move.l	#0,a5
.rep_x:
	move.l	(a6)+,d0	; a b c d
	rol.w	#8,d0		; a b d c
	swap	d0
	rol.w	#8,d0		; d c b a
	move.l	(a6)+,d1	; a b c d
	rol.w	#8,d1		; a b d c
	swap	d1
	rol.w	#8,d1		; d c b a

	Move.L	#$f0f0f0f0,D4	; d4=$f0f0f0f0
	Move.L	D4,D5
	Not.L	D5

	Move.L	D0,D2
	And.L	D4,D2
	Move.L	D1,D3
	And.L	D5,D3
	LSL.L	#4,D0
	And.L	D4,D0
	Or.L	D3,D0
	LSR.L	#4,D1
	And.L	D5,D1
	Or.L	D2,D1
	Swap	D0
	Move.W	D0,D2
	Move.W	D1,D0
	Move.W	D2,D1
	Swap	D0
	Move.L	#$cccccccc,d4	; d4=$CCCCCCCC
	Move.L	D4,D5
	Not.L	D5
	Move.L	D0,D2
	And.L	D4,D2
	Move.L	D1,D3
	And.L	D5,D3
	LSL.L	#2,D0
	And.L	D4,D0
	Or.L	D3,D0
	LSR.L	#2,D1
	And.L	D5,D1
	Or.L	D2,D1
	Move.L	#$ff00ff00,d4	;d4=FF00FF00
	Move.L	D4,D5
	Not.L	D5
	Move.L	D0,D2
	And.L	D4,D2
	LSR.L	#8,D2
	Move.L	D1,D3
	And.L	D5,D3
	LSL.L	#8,D3
	And.L	D5,D0
	Or.L	D3,D0
	And.L	D4,D1
	Or.L	D2,D1
	Move.L	#$AAAAAAAA,D4
	Move.L	D4,D5
	Not.L	D5
	Move.L	D0,D2
	And.L	D4,D2
	Move.L	D1,D3
	And.L	D5,D3
	add.l	d0,d0
	And.L	D4,D0
	Or.L	D3,D0
; modify plane 0
	move.b	d0,(a0)+	; plane0
	Swap	D0
	LSR.L	#1,D1
	And.L	D5,D1
;modify_me_plane4:
;	move.b	d0,$1234(a1)
	RoR.L	#8,D0
;modify_me_plane6:
;	move.b	d0,$1234(a1)
	Or.L	D2,D1
	Swap	D0
;modify_me_plane2:
	move.b	d0,(a2)+	; plane 2
;modify_me_plane1:
	Move.B	D1,(a1)+	; plane 1
	Swap	D1
;modify_me_plane5:
;	move.b	d1,$1234(a1)
	RoR.L	#8,D1
;modify_me_plane7:
;	move.b	d1,$1234(a1)
	Swap	D1
;modify_me_plane3:
	Move.B	D1,(a3)+	; plane 3
	addq.w	#8,a5	
	cmp.w	d7,a5
	blt	.rep_x
	add.l	a4,a0			; pointer to next line
	add.l	a4,a1
	add.l	a4,a2
	add.l	a4,a3
	dbf	d6,.rep_y	
	movem.l	(a7)+,a4/a5
	rts

*
* Converted values to word to skip bits 7 - 4
*
do_4planes3:
	movem.l	a4/a5,-(a7)
	bsr	init_planes
	move.l	(a6)+,a0
	move.l	(a6)+,a1
	move.l	(a6)+,a2
	move.l	(a6)+,a3

	move.l	im_chunk(a4),a6

	move.l	im_height(a4),d6	
	subq.l	#1,d6
	move.l	im_width(a4),d7
	move.l	ch_mod(a5),a4
.rep_y
	move.l	#0,a5
.rep_x:
	move.l	(a6)+,d0	; a b c d
	rol.w	#8,d0		; a b d c
	swap	d0
	rol.w	#8,d0		; d c b a
	move.l	(a6)+,d1	; a b c d
	rol.w	#8,d1		; a b d c
	swap	d1
	rol.w	#8,d1		; d c b a

	Move.L	#$f0f0f0f0,D4	; d4=$f0f0f0f0
	Move.L	D4,D5
	Not.L	D5

	Move.L	D0,D2
	And.L	D4,D2
	Move.L	D1,D3
	And.L	D5,D3
	LSL.L	#4,D0
	And.L	D4,D0
	Or.L	D3,D0

;	LSR.L	#4,D1
;	And.L	D5,D1
;	Or.L	D2,D1

	moveq	#0,d1		; forget 7-4 bits

	Swap	D0
	Move.W	D0,D2
	Move.W	D1,D0
	Move.W	D2,D1
	Swap	D0

	Move.w	#$cccc,d4	; d4=$CCCCCCCC
	Move.w	D4,D5
	Not.w	D5
	Move.w	D0,D2
	And.w	D4,D2
	Move.w	D1,D3
	And.w	D5,D3
	LSL.w	#2,D0
	And.w	D4,D0
	Or.w	D3,D0
	LSR.w	#2,D1
	And.w	D5,D1
	Or.w	D2,D1
	Move.w	#$ff00,d4	;d4=FF00FF00
	Move.w	D4,D5
	Not.w	D5
	Move.w	D0,D2
	And.w	D4,D2
	LSR.w	#8,D2
	Move.w	D1,D3
	And.w	D5,D3
	LSL.w	#8,D3
	And.w	D5,D0
	Or.w	D3,D0
	And.w	D4,D1
	Or.w	D2,D1
	Move.w	#$AAAA,D4
	Move.w	D4,D5
	Not.w	D5
	Move.w	D0,D2
	And.w	D4,D2
	Move.w	D1,D3
	And.w	D5,D3
	add.w	d0,d0
	And.w	D4,D0
	Or.w	D3,D0
	move.b	d0,(a0)+	; plane0
	Swap	D0
	LSR.L	#1,D1
	And.L	D5,D1
	RoR.L	#8,D0
	Or.L	D2,D1
	Swap	D0
	move.b	d0,(a2)+	; plane 2
	Move.B	D1,(a1)+	; plane 1
	Swap	D1
	RoR.L	#8,D1
	Swap	D1
	Move.B	D1,(a3)+	; plane 3

	addq.w	#8,a5	
	cmp.w	d7,a5
	blt	.rep_x
	add.l	a4,a0			; pointer to next line
	add.l	a4,a1
	add.l	a4,a2
	add.l	a4,a3
	dbf	d6,.rep_y	
	movem.l	(a7)+,a4/a5
	rts
*
* Retrieve the pointer to the plane fill function
* Depends on the depth off the bitmap
*
get_plane_func:
	move.l	im_bitmap(a4),a3
	moveq	#0,d0
	move.b	bm_Depth(a3),d0
	add.w	d0,d0
	add.w	d0,d0
	lea	ftab(pc,d0.w),a1
	add.l	(a1),a1
	lea	planes_4(pc),a2
	move.l	a2,a1
	rts

ftab:	dc.l	planes_1-*,planes_1-*,planes_2-*,planes_3-*
	dc.l	planes_4-*,planes_5-*,planes_6-*,planes_7-*
	dc.l	planes_8-*

*
* Do a chunk to planar on depth planes
*
do_var_planes:
	movem.l	a4/a5,-(a7)
	bsr	init_planes
	bsr	get_plane_func
	lea	ch_planes(a5),a0	; points to plane data

	move.l	im_chunk(a4),a6
	move.l	im_height(a4),d6	
	subq.l	#1,d6
	move.l	im_width(a4),d7
	move.l	ch_mod(a5),a4
.rep_y
	move.l	#0,a5
.rep_x:
	move.l	(a6)+,d0	; a b c d
	rol.w	#8,d0		; a b d c
	swap	d0
	rol.w	#8,d0		; d c b a
	move.l	(a6)+,d1	; a b c d
	rol.w	#8,d1		; a b d c
	swap	d1
	rol.w	#8,d1		; d c b a

	Move.L	#$f0f0f0f0,D4	; d4=$f0f0f0f0
	Move.L	D4,D5
	Not.L	D5

	Move.L	D0,D2
	And.L	D4,D2
	Move.L	D1,D3
	And.L	D5,D3
	LSL.L	#4,D0
	And.L	D4,D0
	Or.L	D3,D0
	LSR.L	#4,D1
	And.L	D5,D1
	Or.L	D2,D1
	Swap	D0
	Move.W	D0,D2
	Move.W	D1,D0
	Move.W	D2,D1
	Swap	D0
	Move.L	#$cccccccc,d4	; d4=$CCCCCCCC
	Move.L	D4,D5
	Not.L	D5
	Move.L	D0,D2
	And.L	D4,D2
	Move.L	D1,D3
	And.L	D5,D3
	LSL.L	#2,D0
	And.L	D4,D0
	Or.L	D3,D0
	LSR.L	#2,D1
	And.L	D5,D1
	Or.L	D2,D1
	Move.L	#$ff00ff00,d4	;d4=FF00FF00
	Move.L	D4,D5
	Not.L	D5
	Move.L	D0,D2
	And.L	D4,D2
	LSR.L	#8,D2
	Move.L	D1,D3
	And.L	D5,D3
	LSL.L	#8,D3
	And.L	D5,D0
	Or.L	D3,D0
	And.L	D4,D1
	Or.L	D2,D1
	Move.L	#$AAAAAAAA,D4
	Move.L	D4,D5
	Not.L	D5
	Move.L	D0,D2
	And.L	D4,D2
	Move.L	D1,D3
	And.L	D5,D3
	add.l	d0,d0
	And.L	D4,D0
	Or.L	D3,D0
	jsr	(a1)		; fill the planes with the found data
	addq.w	#8,a5	
	cmp.w	d7,a5
	blt	.rep_x
	move.l	a4,d0
	add.l	d0,(a0)
	add.l	d0,4(a0)
	add.l	d0,8(a0)
	add.l	d0,12(a0)
	dbf	d6,.rep_y	
	movem.l	(a7)+,a4/a5
	rts

planes_1:
	move.l	(a0),a2		; get plane 0 pointer
	move.b	d0,(a2)+
	move.l	a2,(a0)		; restore pointer
	rts
planes_2:
; modify plane 0
	move.l	(a0),a2		; get plane 0 pointer
	move.b	d0,(a2)+
	move.l	a2,(a0)		; restore pointer
	Swap	D0
	LSR.L	#1,D1
	And.L	D5,D1
	RoR.L	#8,D0
	Or.L	D2,D1
	Swap	D0
	move.l	4(a0),a2
	Move.B	D1,(a2)+	; plane 1
	move.l	a2,4(a0)
	rts

planes_3:
; modify plane 0
	move.l	(a0),a2		; get plane 0 pointer
	move.b	d0,(a2)+
	move.l	a2,(a0)		; restore pointer
	Swap	D0
	LSR.L	#1,D1
	And.L	D5,D1
	RoR.L	#8,D0
	Or.L	D2,D1
	Swap	D0
	move.l	8(a0),a2
	move.b	d0,(a2)+	; plane 2
	move.l	a2,8(a0)
	move.l	4(a0),a2
	Move.B	D1,(a2)+	; plane 1
	move.l	a2,4(a0)
	rts

planes_4:	
; modify plane 0
	move.l	(a0),a2		; get plane 0 pointer
	move.b	d0,(a2)+
	move.l	a2,(a0)		; restore pointer
	Swap	D0
	LSR.L	#1,D1
	And.L	D5,D1
;modify_me_plane4:
;	move.b	d0,$1234(a1)
	RoR.L	#8,D0
;modify_me_plane6:
;	move.b	d0,$1234(a1)
	Or.L	D2,D1
	Swap	D0
;modify_me_plane2:
	move.l	8(a0),a2
	move.b	d0,(a2)+	; plane 2
	move.l	a2,8(a0)
;modify_me_plane1:
	move.l	4(a0),a2
	Move.B	D1,(a2)+	; plane 1
	move.l	a2,4(a0)
	Swap	D1
;modify_me_plane5:
;	move.b	d1,$1234(a1)
	RoR.L	#8,D1
;modify_me_plane7:
;	move.b	d1,$1234(a1)
	Swap	D1
;modify_me_plane3:
	move.l	12(a0),a2
	Move.B	D1,(a2)+	; plane 3
	move.l	a2,12(a0)
	rts

planes_5:
	move.l	(a0),a2		; get plane 0 pointer
	move.b	d0,(a2)+
	move.l	a2,(a0)		; restore pointer
	Swap	D0
	LSR.L	#1,D1
	And.L	D5,D1
	move.l	16(a0),a2
	move.b	d0,(a2)+
	move.l	a2,16(a0)
	RoR.L	#8,D0
	Or.L	D2,D1
	Swap	D0
	move.l	8(a0),a2
	move.b	d0,(a2)+	; plane 2
	move.l	a2,8(a0)
	move.l	4(a0),a2
	Move.B	D1,(a2)+	; plane 1
	move.l	a2,4(a0)
	Swap	D1
	RoR.L	#8,D1
	Swap	D1
	move.l	12(a0),a2
	Move.B	D1,(a2)+	; plane 3
	move.l	a2,12(a0)
	rts

planes_6:
	move.l	(a0),a2		; get plane 0 pointer
	move.b	d0,(a2)+
	move.l	a2,(a0)		; restore pointer
	Swap	D0
	LSR.L	#1,D1
	And.L	D5,D1
	move.l	16(a0),a2
	move.b	d0,(a2)+
	move.l	a2,16(a0)
	RoR.L	#8,D0
;	move.l	24(a0),a2
;	move.b	d0,(a2)+
;	move.l	a2,24(a0)
	Or.L	D2,D1
	Swap	D0
	move.l	8(a0),a2
	move.b	d0,(a2)+	; plane 2
	move.l	a2,8(a0)
	move.l	4(a0),a2
	Move.B	D1,(a2)+	; plane 1
	move.l	a2,4(a0)
	Swap	D1
	move.l	20(a0),a2
	move.b	d1,(a2)+
	move.l	a2,20(a0)
	RoR.L	#8,D1
;modify_me_plane7:
;	move.b	d1,$1234(a1)
	Swap	D1
	move.l	12(a0),a2
	Move.B	D1,(a2)+	; plane 3
	move.l	a2,12(a0)
	rts

planes_7:
	move.l	(a0),a2		; get plane 0 pointer
	move.b	d0,(a2)+
	move.l	a2,(a0)		; restore pointer
	Swap	D0
	LSR.L	#1,D1
	And.L	D5,D1
	move.l	16(a0),a2
	move.b	d0,(a2)+
	move.l	a2,16(a0)
	RoR.L	#8,D0
	move.l	24(a0),a2
	move.b	d0,(a2)+
	move.l	a2,24(a0)
	Or.L	D2,D1
	Swap	D0
	move.l	8(a0),a2
	move.b	d0,(a2)+	; plane 2
	move.l	a2,8(a0)
	move.l	4(a0),a2
	Move.B	D1,(a2)+	; plane 1
	move.l	a2,4(a0)
	Swap	D1
	move.l	20(a0),a2
	move.b	d1,(a2)+
	move.l	a2,20(a0)
	RoR.L	#8,D1
	Swap	D1
	move.l	12(a0),a2
	Move.B	D1,(a2)+	; plane 3
	move.l	a2,12(a0)
	rts

planes_8:
	move.l	(a0),a2		; get plane 0 pointer
	move.b	d0,(a2)+
	move.l	a2,(a0)		; restore pointer
	Swap	D0
	LSR.L	#1,D1
	And.L	D5,D1
	move.l	16(a0),a2
	move.b	d0,(a2)+
	move.l	a2,16(a0)
	RoR.L	#8,D0
	move.l	24(a0),a2
	move.b	d0,(a2)+
	move.l	a2,24(a0)
	Or.L	D2,D1
	Swap	D0
	move.l	8(a0),a2
	move.b	d0,(a2)+	; plane 2
	move.l	a2,8(a0)
	move.l	4(a0),a2
	Move.B	D1,(a2)+	; plane 1
	move.l	a2,4(a0)
	Swap	D1
	move.l	20(a0),a2
	move.b	d1,(a2)+
	move.l	a2,20(a0)
	RoR.L	#8,D1
	move.l	28(a0),a2
	move.b	d1,(a2)+
	move.l	a2,28(a0)
	Swap	D1
	move.l	12(a0),a2
	Move.B	D1,(a2)+	; plane 3
	move.l	a2,12(a0)
	rts

get_planes_8:
	move.l	12(a0),a2
	move.b	(a2)+,d1	; plane 3	0003
	move.l	a2,12(a0)

	swap	d1		;		0300	

	move.l	28(a0),a2
	move.b	(a2)+,d1	; plane 7	0307
	move.l	a2,28(a0)

	rol.l	#8,d1		;		3070

	move.l	20(a0),a2
	move.b	(a2)+,d1	; plane 5	3075
	move.l	a2,20(a0)

	swap	d1		;		7530

	move.l	4(a0),a2
	move.b	(a2)+,d1	; plane 1	7531
	move.l	a2,4(a0)

	move.l	8(a0),a2
	move.b	(a2)+,d0	; plane 2	0002
	move.l	a2,8(a0)

	swap	d0		; 		0200

	move.l	24(a0),a2
	move.b	(a2)+,d0	; plane 6	0206
	move.l	a2,24(a0)

	rol.l	#8,d0		; 		2060

	move.l	16(a0),a2
	move.b	(a2)+,d0	; plane 4	2064
	move.l	a2,16(a0)

	swap	d0		; 		6420

	move.l	(a0),a2
	move.b	(a2)+,d0	; plane 0 	6420
	move.l	a2,(a0)
	rts

get_8planes:
	movem.l	a4/a5,-(a7)
	bsr	init_planes

	lea	ch_planes(a5),a0	; points to plane data
	move.l	im_chunk(a4),a6
	move.l	im_height(a4),d6	
	subq.l	#1,d6
	move.l	im_width(a4),d7
	move.l	ch_mod(a5),a4
.rep_y:
	move.l	#0,a5
.rep_x:
	bsr	get_planes_8

; D0 = a2b2c2d2e2f2g2h2 a0b0c0d0e0f0g0h0
; D1 = a3b3c3d3e3f3g3h3 a1b1c1d1e1f1g1h1

	move.l	d1,d2
	and.l	#$aaaaaaaa,d2
	and.l	#$55555555,d1

; D1 = --b3--d3--f3--h3 --b1--d1--f1--h1
; d2 = a3--c3--e3--g3-- a1--c1--e1--g1--

	move.l	d0,d3

	and.l	#$aaaaaaaa,d0
	and.l	#$55555555,d3

; D0 = a2--c2--e2--g2-- a0--c0--e0--g0--
; D3 = --b2--d2--f2--h2 --b0--d0--f0--h0

	lsr.l	#1,d0

; D0 = --a2--c2--e2--g2 --a0--c0--e0--g0

	or.l	d2,d0

; D0 = a3a2c3c2e3e2g3g2 a1a0c1c0e1e0g1g0

	add.l	d1,d1

; D1 = b3--d3--f3--h3-- b1--d1--f1--h1--
	
	or.l	d3,d1

; D1 = b3b2d3d2f3f2h3h2 b1b0d1d0f1f0h1h0

	move.l	d1,d2
	and.l	#$00ff00ff,d2
	and.l	#$ff00ff00,d1

; D1 = b3b2d3d2f3f2h3h2 ---------------
; D2 = ---------------- b1b0d1d0f1f0h1h0

	move.l	d0,d3
	and.l	#$00ff00ff,d0
	and.l	#$ff00ff00,d3

; D0 = ---------------- a1a0c1c0e1e0g1g0
; D3 = a3a2c3c2e3e2g3g2 ----------------

	lsr.l	#8,d3

; D3 = ---------------- a3a2c3c2e3e2g3g2

	lsl.l	#8,d2
	
; D2 = b1b0d1d0f1f0h1h0 ----------------

	or.l	d2,d0
	or.l	d3,d1
	
; D0 = b1b0d1d0f1f0h1h0 a1a0c1c0e1e0g1g0
; D1 = b3b2d3d2f3f2h3h2 a3a2c3c2e3e2g3g2

	move.l	d1,d2
	and.l	#$33333333,d1
	and.l	#$cccccccc,d2

; D1 = ----d3d2----h3h2 ----c3c2----g3g2
; D2 = b3b2----f3f2---- a3a2----e3e2----

	move.l	d0,d3
	and.l	#$cccccccc,d0
	and.l	#$33333333,d3


; D0 = b1b0----f1f0---- a1a0----e1e0----
; D3 = ----d1d0----h1h0 ----c1c0----g1g0

	lsl.l	#2,d1

; D1 = d3d2----h3h2---- c3c2----g3g2----

	or.l	d3,d1
		
; D1 = d3d2d1d0h3h2h1h0 c3c2c1c0g3g2g1g0

	lsr.l	#2,d0

; D0 = ----b1b0----f1f0 ----a1a0----e1e0

	or.l	d2,d0
	
; D0 = b3b2b1b0f3f2f1f0 a3a2a1a0e3e2e1e0
; D1 = d3d2d1d0h3h2h1h0 c3c2c1c0g3g2g1g0

	swap	d0

; D0 = b3b2b1b0f3f2f1f0 a3a2a1a0e3e2e1e0 ---------------- ----------------

	move.w	d0,d2
	move.w	d1,d0
	move.w	d2,d1
	
; D0 = b3b2b1b0f3f2f1f0 a3a2a1a0e3e2e1e0 d3d2d1d0h3h2h1h0 c3c2c1c0g3g2g1g0

	swap	d0
	
; D0 = d3d2d1d0h3h2h1h0 c3c2c1c0g3g2g1g0 b3b2b1b0f3f2f1f0 a3a2a1a0e3e2e1e0

	move.l	d0,d3
	and.l	#$0f0f0f0f,d3
	and.l	#$f0f0f0f0,d0

	move.l	d1,d2
	and.l	#$0f0f0f0f,d2
	and.l	#$f0f0f0f0,d1
	
; D0 = d3d2d1d0-------- c3c2c1c0-------- b3b2b1b0-------- a3a2a1a0--------
; D3 = --------h3h2h1h0 --------g3g2g1g0 --------f3f2f1f0 --------e3e2e1e0

	lsr.l	#4,d0

	or.l	d1,d0
	lsl.l	#4,d2

	or.l	d2,d3
	
; D0 = --------d3d2d1d0 --------c3c2c1c0 --------b3b2b1b0 --------a3a2a1a0

	ror.w	#8,d0
	swap	d0
	ror.w	#8,d0

; D0 = abcd

	ror.w	#8,d3
	swap	d3
	ror.w	#8,d3

; d3 = efgh

	move.l	d0,(a6)+
`	move.l	d3,(a6)+

	addq.w	#8,a5	
	cmp.w	d7,a5
	blt	.rep_x
	move.l	a4,d0
	add.l	d0,(a0)
	add.l	d0,4(a0)
	add.l	d0,8(a0)
	add.l	d0,12(a0)
	add.l	d0,16(a0)
	add.l	d0,20(a0)
	add.l	d0,24(a0)
	add.l	d0,28(a0)

	dbf	d6,.rep_y	
	movem.l	(a7)+,a4/a5
	rts


do_4planes_old:
	bsr	init_planes
	move.l	(a6)+,a0
	move.l	(a6)+,a1
	move.l	(a6)+,a2
	move.l	(a6)+,a3

	move.l	im_chunk(a4),a6

	move.l	im_height(a4),d6	
	subq.l	#1,d6
	move.l	im_width(a4),d7
	move.l	ch_mod(a5),d1
.rep_y
	moveq	#0,d5
.rep_x:
	moveq	#7,d4
.rep_b:
	move.b	(a6)+,d0
	and.w	#$f,d0				; not more than 16 colors
	lsl.w	#2,d0
	jsr	table(pc,d0.w)
	subq.w	#1,d4
	bpl	.rep_b
	addq.w	#8,d5	
	addq.l	#1,a0
	addq.l	#1,a1
	addq.l	#1,a2
	addq.l	#1,a3
	cmp.w	d7,d5
	blt	.rep_x
	add.l	d1,a0			; pointer to next line
	add.l	d1,a1
	add.l	d1,a2
	add.l	d1,a3
	dbf	d6,.rep_y	
	rts
table:
	bra	no_p0
	bra	no_p1
	bra	no_p2
	bra	no_p3
	bra	no_p4
	bra	no_p5
	bra	no_p6
	bra	no_p7
	bra	no_p8
	bra	no_p9
	bra	no_p10
	bra	no_p11
	bra	no_p12
	bra	no_p13
	bra	no_p14
	bra	no_p15

no_p0:
	rts
no_p1:
	bset.b	d4,(a0)
	rts
no_p2:
	bset.b	d4,(a1)
	rts
no_p3:
	bset.b	d4,(a0)
	bset.b	d4,(a1)
	rts
no_p4:
	bset.b	d4,(a2)
	rts
no_p5:
	bset.b	d4,(a0)
	bset.b	d4,(a2)
	rts
no_p6:
	bset.b	d4,(a1)
	bset.b	d4,(a2)
	rts
no_p7:
	bset.b	d4,(a0)
	bset.b	d4,(a1)
	bset.b	d4,(a2)
	rts
no_p8:
	bset.b	d4,(a3)
	rts
no_p9:
	bset.b	d4,(a0)
	bset.b	d4,(a3)
	rts
no_p10:
	bset.b	d4,(a1)
	bset.b	d4,(a3)
	rts
no_p11:
	bset.b	d4,(a0)
	bset.b	d4,(a1)
	bset.b	d4,(a3)
	rts
no_p12:
	bset.b	d4,(a2)
	bset.b	d4,(a3)
	rts
no_p13:
	bset.b	d4,(a0)
	bset.b	d4,(a2)
	bset.b	d4,(a3)
	rts
no_p14:
	bset.b	d4,(a1)
	bset.b	d4,(a2)
	bset.b	d4,(a3)
	rts
no_p15:
	bset.b	d4,(a0)
	bset.b	d4,(a1)
	bset.b	d4,(a2)
	bset.b	d4,(a3)
	rts

imb:	blk.b	im_SIZEOF,0
*
* Grey remap with treshold
*
contrast:
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	a0,a5
	move.l	im_chunk(a5),a0		; pointer to chunky data
	move.l	im_width(a5),d5
	move.l	im_height(a5),d7
	lea	cont_tab(pc),a1
	moveq	#0,d0
	subq.l	#1,d7
	bra	.in_c_h
.rep_c_h:
	move.l	im_width(a5),d6
	bra	.in_c_w
.rep_c_w:
	move.b	(a0),d0
	move.b	0(a1,d0.w),(a0)+
.in_c_w:
	dbf	d6,.rep_c_w
.in_c_h:
	dbf	d7,.rep_c_h
	movem.l	(a7)+,d0-d7/a0-a6
	rts

cont_tab:
	dc.b	1,1,1,1
	dc.b	2,3,4,5
	dc.b	6,7,8,9
	dc.b	$9,$a,$b,$c
