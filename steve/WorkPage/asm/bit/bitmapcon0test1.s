	incdir	"wp:inclibs/"
	include	"graphics_libv39.i"
	include	"exec_lib.i"
	incdir	"include:"
	include	"graphics/view.i"
	include	"exec/memory.i"
	include	"graphics/gfxbase.i"
	include	"hardware/custom.i"

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
MAXPLANES	equ	5
MYMODE		equ	V_LACE
MEMSIZE		equ	100*1024

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

	lea	vi_view(a5),a0
	bsr	set_con0

	move.l	#320,d0
	move.l	#200,d1
	lea	ball(pc),a0
	add.l	#64,a0
	lea	obj_struct(pc),a1
	bsr.w	create_object
	cmp.l	#-1,d0
	beq.w	no_obj

	bsr	init_timer
	bne	no_obj

	move.l	a5,-(a7)
	lea	secs(pc),a5
	bsr	get_time
	move.l	(a7)+,a5

;	move.l	#399,d7
rep_test:
;	move.l	d7,-(a7)

	bsr.w	my_prog

;	bsr	clear_screen
	bsr	re_show

;	move.l	(a7)+,d7
;	dbf	d7,rep_test

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

	lea	obj_struct(pc),a4
	bsr.w	kill_object_structure

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
	move.l		gfx_base(pc),a6
	lea		vi_view(a5),a1
	jsr		_LVOLoadView(a6)
	rts

set_con0:
	move.l		v_LOFCprList(a0),a0
	cmp.l		#0,a0
	beq.b		exit_putcopy
	move.w		crl_MaxCount(a0),d2		; aantal elementen
	move.l		crl_start(a0),a0		; eigenlijke copperlist
	subq.l		#1,d2
mrep_putcop:
	move.w		(a0)+,d0
	cmp.w		#$ffff,d0
	beq.b		exit_putcopy

	cmp.w		#$100,d0
	bne.b		no_putmod8
	bra.b		exit_getmodulo
no_putmod8:
	addq.l		#2,a0
	dbf		d2,mrep_putcop
exit_getmodulo:
	move.w		(a0),d1
	or.w		#8,d1
	move.w		d1,(a0)
exit_putcopy:
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

	move.w	#31,d0
	lea	dt_colors(a3),a1
init_dt_loop:
	move.w	(a0)+,(a1)+
	dbf	d0,init_dt_loop
	rts

* d0 width
* d1 height
* a0 bitmap non interleaved
* a1 object pointer
create_object:
	move.l	a0,-(sp)
	move.l	a1,a4
	clr.l	os_x(a4)
	move.w	d0,os_width(a4)
	move.w	d1,os_heigth(a4)
	ext.l	d0
	add.l	#15,d0
	lsr.l	#4,d0
	add.l	d0,d0
	move.l	d0,os_width_bytes(a4)

	move.l	dt_breedte_x(a3),d1
	sub.l	d0,d1
	move.w	d1,os_modulo(a4)

	move.l	os_width_bytes(a4),d0
	mulu	os_heigth(a4),d0
	mulu	dt_planes(a3),d0
	move.l	d0,d7
	add.l	d0,d0
	move.l	#MEMF_CHIP+MEMF_CLEAR,d1
;	move.l	#MEMF_FAST+MEMF_CLEAR,d1
	move.l	d0,os_obj_size(a4)
	move.l	4,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,os_obj_ptr(a4)
	beq.w	.no_obj
	add.l	d7,d0
	move.l	d0,os_mask_ptr(a4)

	moveq	#0,d0
	move.w	os_heigth(a4),d0
	mulu	dt_planes(a3),d0
	lsl.w	#6,d0
	move.l	os_width_bytes(a4),d1
	lsr.w	#1,d1
	or.w	d1,d0
	move.w	d0,os_blt_size(a4)

	move.l	os_width_bytes(a4),d4
	mulu	os_heigth(a4),d4
	move.l	(sp)+,a0
	move.l	a0,d3
	move.l	os_obj_ptr(a4),a1
	move.l	os_width_bytes(a4),d0
	subq	#1,d0
	move.w	os_heigth(a4),d6
	subq	#1,d6
.yloop:	move.l	d3,a0
	move.w	dt_planes(a3),d5
	subq.w	#1,d5
.ploop:	move.l	a0,a6
	move.l	d0,d7
.xloop:	move.b	(a6)+,(a1)+
	dbf	d7,.xloop
	add.l	d4,a0
	dbf	d5,.ploop
	add.l	os_width_bytes(a4),d3
	dbf	d6,.yloop

	lea	os_bitmap(a4),a0
	move.l	os_width_bytes(a4),d1			; breedte in bytes * 8
	lsl.l		#3,d1				; width
	mulu		dt_planes(a3),d1		; interleaved
	move.w		os_heigth(a4),d2			; height
	move.l	gfx_base(pc),a6
	jsr		_LVOInitBitMap(a6)

	lea	os_bitmap(a4),a0
	move.l	os_obj_ptr(a4),d0
	move.l	os_width_bytes(a4),d1
	move.l	d0,bm_Planes(a0)		; zet de bitplanes
	add.l	d1,d0
	move.l	d0,bm_Planes+4(a0)
	add.l	d1,d0
	move.l	d0,bm_Planes+8(a0)
	add.l	d1,d0
	move.l	d0,bm_Planes+12(a0)
	add.l	d1,d0
	move.l	d0,bm_Planes+16(a0)
	add.l	d1,d0
	move.l	d0,bm_Planes+20(a0)
	add.l	d1,d0
	move.l	d0,bm_Planes+24(a0)
	add.l	d1,d0
	move.l	d0,bm_Planes+28(a0)
	moveq	#0,d0
	rts

.no_obj:move.l	(sp)+,d0
	moveq	#-1,d0
	rts

* a4 os pointer
kill_object_structure:
	move.l	4,a6
	move.l	os_obj_size(a4),d0
	move.l	os_obj_ptr(a4),a1
	jsr	_LVOFreeMem(a6)
	rts

old_view:	dc.l	0

gfx_base:	dc.l	0
screen_mem:	dc.l	0
screen_struct:	blk.b	dt_SIZEOF,0
obj_struct:	blk.b	os_SIZEOF,0

gfx_name:	dc.b	'graphics.library',0
	even

clear_screen:
	move.l	dt_breedte_x(a3),d0
	lsr.l	#2,d0				; longs
	mulu	dt_leny(a3),d0
	mulu	dt_planes(a3),d0
	move.l	dt_screenmem(a3),a0
	moveq	#0,d1
cc:	move.l	d1,(a0)+
	subq.l	#1,d0
	bne	cc
	rts
	
* don't touch a3 & a5

my_prog:movem.l	d0-d7/a0-a6,-(sp)

; do a bltbitmap here

	move.l	gfx_base(pc),a6
	moveq	#0,d2
	moveq	#0,d3
	move.l	#320,d4
	move.l	#200,d5
	move.l	#$ff,d7
	move.w	#$30,d6
	move.w	#0,d0
	move.w	#0,d1
	lea	obj_struct(pc),a4
	lea	os_bitmap(a4),a0
	lea	vi_bitmap(a5),a1
	bsr	_BltBitMapFM

;	jsr	_LVOBltBitMap(a6)

	bsr	re_show

; There is something on the screen now
; Try to blit it to fastmem

	lea	fstore(pc),a1		; to bitmapFM
	lea	vi_bitmap(a5),a0	; from screen bitmap
	clr.w	fstore_width(a1)	; allocate new memory
	move.l	#0,d0
	moveq	#0,d1
	move.l	#0,d2
	move.l	#0,d3
	move.l	#320,d4
	move.l	#200,d5
	jsr	_bltBitMapStoreFM

	bsr	clear_screen
ptt:

.again:
	move.l	#65,d7
.repp:
	lea	fstore(pc),a0		; from bitmapFM
	lea	vi_bitmap(a5),a1	; to screen bitmap
	jsr	_bltBitMapRestoreFM
	moveq	#0,d6
	move.w	bm_BytesPerRow(a1),d6
	add.l	d6,d6
	add.l	d6,fstore_offset(a0)

	dbf	d7,.repp

	move.l	#65,d7
.repp2:
	lea	fstore(pc),a0		; from bitmapFM
	lea	vi_bitmap(a5),a1	; to screen bitmap
	jsr	_bltBitMapRestoreFM
	moveq	#0,d6
	move.w	bm_BytesPerRow(a1),d6
	add.l	d6,d6
	sub.l	d6,fstore_offset(a0)
	dbf	d7,.repp2
	btst	#6,$bfe001
	bne.s	.again

	bsr	re_show

	lea	fstore(pc),a1		; to bitmapFM
	jsr	_bltBitMapFreeFM

wait:	btst	#6,$bfe001
	bne.s	wait

	movem.l	(sp)+,d0-d7/a0-a6
	rts


	incdir	"wp:asm/bit/"
	include	"fastbitmap.i"

fstore:	blk.b	fstore_SIZEOF,0

	XDEF	_bltBitMapFreeFM
*
* Free the fmbitmap pointed at by a1
*
_bltBitMapFreeFM:
	move.l	fstore_planes(a1),d0
	beq	.no_mem
	move.l	a1,-(a7)
	move.l	fstore_size(a1),d0
	move.l	fstore_planes(a1),a1
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)
	move.l	(a7)+,a1
	clr.w	fstore_width(a1)
	clr.l	fstore_planes(a1)
	clr.l	fstore_size(a1)
.no_mem:
	rts

	XDEF	_bltBitMapAllocFM
*
* Allocate and initialize memory for bitmap operation
* Same parameters as bltbitmap
* d4 sizex, d5 sizey
*
_bltBitMapAllocFM:
	movem.l	d0-d7/a0-a6,-(a7)		; save all ?
	clr.l	fstore_planes(a1)		; clear memory place
	move.w	d4,fstore_width(a1)
	move.w	d5,fstore_height(a1)

	move.l	bfm_addr_offset(a5),fstore_offset(a1)
	move.w	bfm_totalx_w(a5),d4
	move.w	d4,fstore_width_w(a1)

	moveq	#0,d0
	move.b	bm_Depth(a0),d0
	move.w	d0,d6
	add.l	d4,d4				; bytes
	move.l	d4,d7
	mulu	d5,d4
	mulu	d0,d4				; total size bitmap
	move.l	d4,fstore_size(a1)
	move.l	d4,d0
	movem.l	a0/a1,-(a7)
	move.l	#MEMF_ANY,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
	movem.l	(a7)+,a0/a1
	tst.l	d0
	beq	.alloc_fail
	lea	fstore_planes(a1),a2
	bra.s	.in_pl
.rep_pl:
	move.l	d0,(a2)+
	add.l	d7,d0
.in_pl:	dbf	d6,.rep_pl
	movem.l	(a7)+,d0-d7/a0-a6
	moveq	#0,d7
	rts

.alloc_fail:
	movem.l	(a7)+,d0-d7/a0-a6
	clr.w	fstore_width(a1)
	moveq	#-1,d7
	rts

	XDEF	_bltBitMapRestoreFM

*
* a0 src bitmap , a1 dest bitmap
* src is a FMbitmap, dest is a normal bitmap
*
_bltBitMapRestoreFM:
	link	a5,#-bfm_SIZEOF
	movem.l	d1-d7/a0-a6,-(a7)
	sub.l	#bfm_SIZEOF,a5

; use depth of source

	moveq	#0,d7
	move.w	fstore_depth(a0),d7
	move.w	d7,bfm_depth(a5)
	beq	.no_blit
	sub.w	#1,bfm_depth(a5)

; calculate source start word
	move.w	fstore_x(a0),d0
	move.w	fstore_y(a0),d1

	move.l	fstore_offset(a0),bfm_addr_offset(a5)
	move.w	fstore_width_w(a0),d7
	move.w	d7,bfm_totalx_w(a5)		; words to copy
	beq	.no_blit

	add.l	d7,d7				; copy range in bytes
	move.w	bm_BytesPerRow(a1),d6
	ext.l	d6
	sub.l	d7,d6
	move.l	d6,bfm_moddest(a5)
	moveq	#0,d6

	move.w	fstore_modulo(a0),d6
	move.l	d6,bfm_modsource(a5)		; set source mod zero

; assume dstx and dsty to be 0,0

	move.l	bfm_modsource(a5),d0
	move.l	bfm_moddest(a5),d1
	move.w	bfm_totalx_w(a5),d7
	move.w	d7,d6
	and.b	#1,d7
	beq	.long_mode
	cmp.w	#1,d6
	bgt	.long_word_mode

	sub.w	#1,bfm_totalx_w(a5)		; for the dbf
	lea.l	bm_Planes(a1),a4		; dest pointer planes
	lea.l	fstore_planes(a0),a3		; source pointer planes
	move.w	bfm_depth(a5),d7
.rep_planes:
	move.l	(a3)+,a2
	move.l	(a4)+,a6
	add.l	bfm_addr_offset(a5),a6		; start dest
	move.w	bfm_totaly(a5),d6
.rep_lines:	
	move.w	bfm_totalx_w(a5),d5
.rep_words:
	move.w	(a2)+,(a6)+
	dbf	d5,.rep_words
	add.l	d0,a2				; add modulo's
	add.l	d1,a6
	dbf	d6,.rep_lines
	dbf	d7,.rep_planes
.no_blit:	
.alloc_error:
	movem.l	(a7)+,d1-d7/a0-a6
	unlk	a5
	rts

.long_mode:
	move.w	bfm_totalx_w(a5),d7
	lsr.w	#1,d7
	move.w	d7,bfm_totalx_w(a5)
	sub.w	#1,bfm_totalx_w(a5)		; for the dbf
	lea.l	bm_Planes(a1),a4		; dest pointer planes
	lea.l	fstore_planes(a0),a3		; source pointer planes
	move.w	bfm_depth(a5),d7
.lrep_planes:
	move.l	(a3)+,a2
	move.l	(a4)+,a6
	add.l	bfm_addr_offset(a5),a6		; start dest
	move.w	bfm_totaly(a5),d6
.lrep_lines:	
	move.w	bfm_totalx_w(a5),d5
.lrep_words:
	move.l	(a2)+,(a6)+
	dbf	d5,.lrep_words
	add.l	d0,a2				; add modulo's
	add.l	d1,a6
	dbf	d6,.lrep_lines
	dbf	d7,.lrep_planes
	bra	.no_blit

.long_word_mode:
	move.w	bfm_totalx_w(a5),d7
	lsr.w	#1,d7
	move.w	d7,bfm_totalx_w(a5)
	sub.w	#1,bfm_totalx_w(a5)		; for the dbf
	lea.l	bm_Planes(a1),a4		; dest pointer planes
	lea.l	fstore_planes(a0),a3		; source pointer planes
	move.w	bfm_depth(a5),d7
.lwrep_planes:
	move.l	(a3)+,a2
	move.l	(a4)+,a6
	add.l	bfm_addr_offset(a5),a6		; start dest
	move.w	bfm_totaly(a5),d6
.lwrep_lines:	
	move.w	bfm_totalx_w(a5),d5
.lwrep_words:
	move.l	(a2)+,(a6)+
	dbf	d5,.lwrep_words
	move.w	(a2)+,(a6)+			; add the uneven word
	add.l	d0,a2				; add modulo's
	add.l	d1,a6
	dbf	d6,.lwrep_lines
	dbf	d7,.lwrep_planes
	bra	.no_blit

	XDEF	_bltBitMapStoreFM

*
* BltBitMapStoreFM( SrcBitMap, SrcX, SrcY, DstBitMap, 
*            A0         D0:16 D1:16 A1         
* DstX, DstY, SizeX, SizeY, Minterm, Mask [, TempA]) 
* D2:16 D3:16 D4:16  D5:16  D6:8     D7:8   [A2]
*
* Copy data from bitmap to a special FM bitmap
* this procedure is used only for temporary storage of the data
* The start and end masks are remembered
* and so are the x and y position
*
* Note that DstX, DstY, MinTerm and Mask are not used
*
_bltBitMapStoreFM:
	link	a5,#-bfm_SIZEOF

	movem.l	d1-d7/a0-a6,-(a7)

	sub.l	#bfm_SIZEOF,a5

; fill in dest bitmap

	move.w	d0,fstore_x(a1)
	move.w	d1,fstore_y(a1)

	move.w	d5,bfm_totaly(a5)
	beq	.no_blit
	sub.w	#1,bfm_totaly(a5)		; for dbf

; use depth of source

	moveq	#0,d7
	move.b	bm_Depth(a0),d7
	move.w	d7,fstore_depth(a1)
	move.w	d7,bfm_depth(a5)
	beq	.no_blit
	sub.w	#1,bfm_depth(a5)

; calculate source start word

	move.w	d0,d7
	and.w	#$fff0,d7
	sub.w	d7,d0
	move.w	d0,bfm_restval(a5)
	lsr.l	#4,d7
	move.l	d7,bfm_addr_offset(a5)		; save offset x

	move.w	d4,d7
	add.w	d0,d7				; total nr of pixels to copy
	move.w	d7,bfm_totalx(a5)
	ext.l	d7
	add.l	#15,d7
	lsr.l	#4,d7

	move.w	d7,bfm_totalx_w(a5)		; words to copy
	beq	.no_blit

	add.l	d7,d7				; copy range in bytes
	move.w	bm_BytesPerRow(a0),d6
	ext.l	d6
	sub.l	d7,d6
	move.l	d6,bfm_modsource(a5)

	move.l	d7,d6
	moveq	#0,d0
	move.b	bm_Depth(a0),d0
	mulu	d0,d6
;	move.w	fstore_width_w(a1),d6		; width in words
;	add.w	d6,d6
	ext.l	d6
	sub.l	d7,d6

	move.w	d6,fstore_modulo(a1)
	move.l	d6,bfm_moddest(a5)

	move.w	bm_BytesPerRow(a0),d7
	mulu	d1,d7
	add.l	d7,bfm_addr_offset(a5)		; add y offset


	tst.w	fstore_width(a1)
	bne	.already_alloc

	bsr	_bltBitMapAllocFM
	tst.w	d7
	bne	.alloc_error
	
.already_alloc:				; bitmap is allocated


; assume dstx and dsty to be 0,0

	move.l	bfm_modsource(a5),d0
	move.l	bfm_moddest(a5),d1
	move.w	bfm_totalx_w(a5),d7
	move.w	d7,d6
	and.b	#1,d7
	beq	.long_mode
	cmp.w	#1,d6
	bgt	.long_word_mode

	sub.w	#1,bfm_totalx_w(a5)		; for the dbf
	lea.l	bm_Planes(a0),a3		; source pointer planes
	lea.l	fstore_planes(a1),a4		; dest pointer planes
	move.w	bfm_depth(a5),d7
.rep_planes:
	move.l	(a3)+,a2
	move.l	(a4)+,a6
	add.l	bfm_addr_offset(a5),a2		; start source
	move.w	bfm_totaly(a5),d6
.rep_lines:	
	move.w	bfm_totalx_w(a5),d5
.rep_words:
	move.w	(a2)+,(a6)+
	dbf	d5,.rep_words
	add.l	d0,a2				; add modulo's
	add.l	d1,a6
	dbf	d6,.rep_lines
	dbf	d7,.rep_planes
.no_blit:	
.alloc_error:
	movem.l	(a7)+,d1-d7/a0-a6
	unlk	a5
	rts

.long_mode:
	move.w	bfm_totalx_w(a5),d7
	lsr.w	#1,d7
	move.w	d7,bfm_totalx_w(a5)
	sub.w	#1,bfm_totalx_w(a5)		; for the dbf
	lea.l	bm_Planes(a0),a3		; source pointer planes
	lea.l	fstore_planes(a1),a4		; dest pointer planes
	move.w	bfm_depth(a5),d7
.lrep_planes:
	move.l	(a3)+,a2
	move.l	(a4)+,a6
	add.l	bfm_addr_offset(a5),a2		; start source
	move.w	bfm_totaly(a5),d6
.lrep_lines:	
	move.w	bfm_totalx_w(a5),d5
.lrep_words:
	move.l	(a2)+,(a6)+
	dbf	d5,.lrep_words
	add.l	d0,a2				; add modulo's
	add.l	d1,a6
	dbf	d6,.lrep_lines
	dbf	d7,.lrep_planes
	bra	.no_blit

.long_word_mode:
	move.w	bfm_totalx_w(a5),d7
	lsr.w	#1,d7
	move.w	d7,bfm_totalx_w(a5)
	sub.w	#1,bfm_totalx_w(a5)		; for the dbf
	lea.l	bm_Planes(a0),a3		; source pointer planes
	lea.l	fstore_planes(a1),a4		; dest pointer planes
	move.w	bfm_depth(a5),d7
.lwrep_planes:
	move.l	(a3)+,a2
	move.l	(a4)+,a6
	add.l	bfm_addr_offset(a5),a2		; start source
	move.w	bfm_totaly(a5),d6
.lwrep_lines:	
	move.w	bfm_totalx_w(a5),d5
.lwrep_words:
	move.l	(a2)+,(a6)+
	dbf	d5,.lwrep_words
	move.w	(a2)+,(a6)+			; add the uneven word
	add.l	d0,a2				; add modulo's
	add.l	d1,a6
	dbf	d6,.lwrep_lines
	dbf	d7,.lwrep_planes
	bra	.no_blit

*
* File	: bltbitmapfm.s
* Uses	:
* Date	: 22 august 1993 
* Author: ing. C. Lieshout
* Desc	: blit bitmap from fast

	INCDIR	"include:"
	INCLUDE "graphics/gfx.i"

	RSRESET
bfm_block:		rs.w	0
bfm_restval:		rs.l	1
bfm_addr_offset:	rs.l	1
bfm_totalx:		rs.w	1
bfm_totaly:		rs.w	1
bfm_totalx_w:		rs.w	1
bfm_modsource:		rs.l	1
bfm_moddest:		rs.l	1
bfm_depth:		rs.w	1
bfm_SIZEOF:		rs.w	0

	XDEF _BltBitMapFM
*
* BltBitMap(SrcBitMap, SrcX, SrcY, DstBitMap, 
*            A0         D0:16 D1:16 A1         
* DstX, DstY, SizeX, SizeY, Minterm, Mask [, TempA]) 
* D2:16 D3:16 D4:16  D5:16  D6:8     D7:8   [A2]
*
_BltBitMapFM:
	link	a5,#-bfm_SIZEOF

	movem.l	d1-d7/a0-a6,-(a7)

	sub.l	#bfm_SIZEOF,a5

	move.w	d5,bfm_totaly(a5)
	beq	no_blit
	sub.w	#1,bfm_totaly(a5)		; for dbf

; check depth's

	moveq	#0,d6
	move.b	bm_Depth(a1),d7
	cmp.b	bm_Depth(a0),d7
	ble	oke_depth
	move.b	bm_Depth(a0),d7			; use the smallest depth
oke_depth:
	move.w	d7,bfm_depth(a5)
	beq	no_blit
	sub.w	#1,bfm_depth(a5)

; calculate source start word

	move.w	d0,d7
	and.w	#$fff0,d7
	sub.w	d7,d0
	move.w	d0,bfm_restval(a5)
	lsr.l	#4,d7
	move.l	d7,bfm_addr_offset(a5)		; save offset x

	move.w	d4,d7
	add.w	d0,d7				; total nr of pixels to copy
	move.w	d7,bfm_totalx(a5)
	ext.l	d7
	add.l	#15,d7
	lsr.l	#4,d7

	move.w	d7,bfm_totalx_w(a5)		; words to copy
	beq	no_blit

	add.l	d7,d7				; copy range in bytes
	move.w	bm_BytesPerRow(a0),d6
	ext.l	d6
	sub.l	d7,d6
	move.l	d6,bfm_modsource(a5)

	move.w	bm_BytesPerRow(a1),d6
	ext.l	d6
	sub.l	d7,d6
	move.l	d6,bfm_moddest(a5)		; should be zero ?

	move.w	bm_BytesPerRow(a0),d7
	mulu	d1,d7
	add.l	d7,bfm_addr_offset(a5)		; add y offset

; assume dstx and dsty to be 0,0

	move.l	bfm_modsource(a5),d0
	move.l	bfm_moddest(a5),d1
	move.w	bfm_totalx_w(a5),d7
	move.w	d7,d6
	and.b	#1,d7
	beq	long_mode
	cmp.w	#1,d6
	bgt	long_word_mode

	sub.w	#1,bfm_totalx_w(a5)		; for the dbf
	lea.l	bm_Planes(a0),a3		; source pointer planes
	lea.l	bm_Planes(a1),a4		; dest pointer planes
	move.w	bfm_depth(a5),d7
rep_planes:
	move.l	(a3)+,a2
	move.l	(a4)+,a6
	add.l	bfm_addr_offset(a5),a2		; start source
	move.w	bfm_totaly(a5),d6
rep_lines:	
	move.w	bfm_totalx_w(a5),d5
rep_words:
	move.w	(a2)+,(a6)+
	dbf	d5,rep_words
	add.l	d0,a2				; add modulo's
	add.l	d1,a6
	dbf	d6,rep_lines
	dbf	d7,rep_planes
no_blit:	
	movem.l	(a7)+,d1-d7/a0-a6
	unlk	a5
	rts

long_mode:
	move.w	bfm_totalx_w(a5),d7
	lsr.w	#1,d7
	move.w	d7,bfm_totalx_w(a5)
	sub.w	#1,bfm_totalx_w(a5)		; for the dbf
	lea.l	bm_Planes(a0),a3		; source pointer planes
	lea.l	bm_Planes(a1),a4		; dest pointer planes
	move.w	bfm_depth(a5),d7
lrep_planes:
	move.l	(a3)+,a2
	move.l	(a4)+,a6
	add.l	bfm_addr_offset(a5),a2		; start source
	move.w	bfm_totaly(a5),d6
lrep_lines:	
	move.w	bfm_totalx_w(a5),d5
lrep_words:
	move.l	(a2)+,(a6)+
	dbf	d5,lrep_words
	add.l	d0,a2				; add modulo's
	add.l	d1,a6
	dbf	d6,lrep_lines
	dbf	d7,lrep_planes
	bra	no_blit

long_word_mode:
	move.w	bfm_totalx_w(a5),d7
	lsr.w	#1,d7
	move.w	d7,bfm_totalx_w(a5)
	sub.w	#1,bfm_totalx_w(a5)		; for the dbf
	lea.l	bm_Planes(a0),a3		; source pointer planes
	lea.l	bm_Planes(a1),a4		; dest pointer planes
	move.w	bfm_depth(a5),d7
lwrep_planes:
	move.l	(a3)+,a2
	move.l	(a4)+,a6
	add.l	bfm_addr_offset(a5),a2		; start source
	move.w	bfm_totaly(a5),d6
lwrep_lines:	
	move.w	bfm_totalx_w(a5),d5
lwrep_words:
	move.l	(a2)+,(a6)+
	dbf	d5,lwrep_words
	move.w	(a2)+,(a6)+			; add the uneven word
	add.l	d0,a2				; add modulo's
	add.l	d1,a6
	dbf	d6,lwrep_lines
	dbf	d7,lwrep_planes
	bra	no_blit

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



	INCDIR	"wp:asm/bit/"
ball:	INCBIN	"sexy.bin"
bend:	dc.l	0
	even

