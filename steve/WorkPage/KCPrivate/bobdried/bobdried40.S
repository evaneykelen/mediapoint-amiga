*
* File	: bobdried.s
* Uses	: bo_data.i
* Date	: 7 apr 1994
* Author: ing. C. Lieshout
* Desc	: ReCreate the 3d bobbie thing

	INCDIR "include:"
	INCLUDE "exec/types.i"
	INCLUDE "exec/execbase.i"
	INCLUDE "exec/funcdef.i"
	INCLUDE "exec/exec_lib.i"
	INCLUDE	"exec/memory.i"
	INCLUDE "graphics/gfxbase.i"
	INCLUDE "graphics/view.i"
	INCLUDE "graphics/gfx.i"
	INCLUDE "graphics/rastport.i"
	INCLUDE	"graphics/text.i"
	INCLUDE	"graphics/videocontrol.i"
	INCLUDE	"libraries/dos.i"
	INCLUDE	"hardware/custom.i"

	INCDIR	"wp:inclibs/"
	INCLUDE "graphics_libv39.i"
	INCLUDE	"dos_lib.i"
	INCLUDE	"diskfont_lib.i"
	INCLUDE	"intuition_lib.i"

;	INCDIR	"aprogs:graph/bobdried/"
	incdir	"wp:kcprivate/bobdried/"
	INCLUDE	"bo_data.i"

MON_ID = PAL_MONITOR_ID

DEBUG = 0
XAPP = 0

; Default values when there is no viewport

DEF_XSIZE = 320
DEF_YSIZE = 200
DEF_MODE = 0
DEF_PLANES = 5

LIBV39 = 39
LIBV40 = 40

	IFNE	DEBUG
	XREF	KPutFmt
	XREF	KPutStr
	PRINTT	"De debug info staat aan dus je moet nog met debug.lib linken"
	ENDC

MEMSIZE = 227000


test:
	move.l	#180,d0
	move.l	#100,d1
	bsr	get_easy_cosinus
	rts	
	moveq	#0,d7
	lea	tdat(pc),a2
.rep:
	move.l	d7,d0
	moveq	#100,d1
	bsr	get_easy_sinus
	swap	d0
	move.w	d0,(a2)+
	addq.l	#1,d7
	cmp.l	#370,d7	
	ble	.rep
	rts

tdat:	blk.l	4000,0

	lea	datablock(pc),a3
	subq	#1,d0
	beq.W	err
zoeken:
	cmp.b	#$20,(a0)+
	bne.s	gevonden
	dbra	d0,zoeken
	bra.W	err
gevonden:
	cmp.b	#'"',-1(a0)		; skip the '"' sign
	beq.b	found_cc
gev2:
	subq.l	#1,a0
	move.l	a0,cb_filenamepointer(a3)
zet:	cmp.b	#10,(a0)
	beq.s	zetnul2
	cmp.b	#' ',(a0)
	beq.s	zetnul2
	cmp.b	#'"',(a0)
	beq.b	zetnul2
	addq.l	#1,a0
	bra.s	zet
zetnul2:
	move.b	#0,(a0)
	bra	instart

found_cc:
	move.l	a0,cb_filenamepointer(a3)
cczet:	cmp.b	#10,(a0)
	beq.s	zetnul2
	cmp.b	#'"',(a0)
	beq.b	zetnul2
	addq.l	#1,a0
	bra.s	cczet
	bra.b	instart

	IFEQ	XAPP
start:
	lea	datablock(pc),a3
;	lea	filename(pc),a0
	move.l	a0,cb_filenamepointer(a3)

instart:
	bsr	open_libs
	bne	lib_err

	move.w	#DEF_XSIZE,cb_lenviewx(a3)
	move.w	#DEF_YSIZE,cb_lenviewy(a3)
	move.w	#DEF_PLANES,cb_planes(a3)

	lea	temp_tags(pc),a0
	move.l	a0,cb_tags(a3)

	move.l	#MON_ID,cb_monid(a3)
	bsr	setprefs

	bsr	get_chipmem
	beq	chipmem_err

	bsr.w	do_scroll

chipmem_err:
	bsr.w	close_all
lib_err:
	tst.l	cb_screenmem(a3)
	beq	err
	move.l	cb_screenmem(a3),a1
	move.l	cb_memsize(a3),d0
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)
err:
	rts

*
* Retrieve the overscan settings
*
get_oscan_vals:
	move.l	cb_intbase(a3),a6
	move.l	cb_monid(a3),d0
	or.l	#HIRES_KEY,d0
	move.l	d0,a0
	move.l	#1,d0				//OSCAN_MAX
 	lea.l	cb_viewblock1(a3),a1
	jsr	_LVOQueryOverscan(a6)
 	lea.l	cb_viewblock1(a3),a1

	move.w	ra_MaxX(a1),d0
	move.w	ra_MaxY(a1),d1
	addq.w	#1,d0
	addq.w	#1,d1
	lsr.w	#1,d0
	lsr.w	#1,d1
	add.w	d0,d0
	add.w	d1,d1
	move.w	d0,cb_columns(a3)
	move.w	d1,cb_rows(a3)

	move.l	cb_monid(a3),d0
	or.l	#HIRES_KEY,d0
	move.l	d0,a0
	move.l	#4,d0				//OSCAN_MAX
 	lea.l	cb_viewblock1(a3),a1
	jsr	_LVOQueryOverscan(a6)
 	lea.l	cb_viewblock1(a3),a1

	move.w	ra_MaxY(a1),d0
	sub.w	ra_MinY(a1),d0
	add.w	d0,d0
	move.w	d0,cb_max_y(a3)

	move.w	ra_MaxX(a1),d0
	sub.w	ra_MinX(a1),d0
	addq.l	#7,d0
	lsr.l	#3,d0
	lsl.l	#3,d0
	move.w	d0,cb_max_x(a3)

	move.w	#0,a0
 	lea.l	cb_viewblock1(a3),a1
	moveq	#mtr_SIZEOF,d0
	move.l	#DTAG_MNTR,d1
	move.l	cb_monid(a3),d2
	move.l	cb_graphbase(a3),a6
	jsr	_LVOGetDisplayInfoData(a6)
 	lea.l	cb_viewblock1(a3),a1

	move.w	mtr_ViewPositionRange+2(a1),d0		; get minimal y pos
	move.w	mtr_ViewPosition+2(a1),d1		; get actual y pos
	move.w	d0,d2
	addq.w	#4,d0					; diff < 4 ?
	cmp.w	d0,d1
	bge	.minimum_oke
	move.l	d2,d1
	addq.w	#4,d1
.minimum_oke:
	move.w	mtr_ViewPosition(a1),cb_viewx(a3)
	move.w	d1,cb_viewy(a3)

	and.w	#$fffe,cb_viewx(a3)
	and.w	#$fffe,cb_viewy(a3)

	rts

set_lace:
	IFNE XAPP
	move.w	#0,cb_or_vmode_mask(a3)
	move.l	cb_mlsysstruct(a3),a0
	move.l	ml_miscFlags(a0),d0
	btst	#4,d0
	beq	.nolace
	ENDC
	move.w	#V_LACE,cb_or_vmode_mask(a3)	; set depending on LACE on/off
.nolace:
	rts

*
* Set depending on monitor the LACE bits
* Data should be somewhere in the mlsystem structure
*
* For test purpose also open the monitor here
*
set_and_or_vmode:
	move.l	cb_graphbase(a3),a6
	move.l	cb_monid(a3),d0
	move.w	#0,a1
	jsr	_LVOOpenMonitor(a6)
	move.l	d0,cb_monitorspec(a3)

	bsr	set_lace

	move.l	cb_monid(a3),d0
	move.w	#~0,cb_and_vmode_mask(a3)
	move.l	d0,d1
	and.l	#DBLPAL_MONITOR_ID,d1		; what about super72 ?????
	cmp.l	#DBLPAL_MONITOR_ID,d1
	beq	.set
	move.l	d0,d1
	and.l	#DBLNTSC_MONITOR_ID,d1
	cmp.l	#DBLNTSC_MONITOR_ID,d1
	beq	.set
	rts
.set:	
	move.w	#~V_LACE,cb_and_vmode_mask(a3)
	rts

setprefs:
	bsr	set_and_or_vmode
	bsr	get_oscan_vals
	rts
*
* Alloc the required chipmem
*
get_chipmem:
	moveq	#0,d0
	moveq	#0,d1
	move.w	cb_lenviewx(a3),d1
	add.l	#15,d1
	lsr.l	#4,d1				; words
	add.l	d1,d1				; bytes
	move.w	cb_lenviewy(a3),d0
	add.w	#100,d0
	mulu	cb_planes(a3),d0
	mulu	d0,d1				; total bytes needed
	add.l	d1,d1				; need it twice Double buf
	move.l	d1,cb_memsize(a3)
	move.l	cb_memsize(a3),d2
	lsr.l	#3,d2
	move.l	d2,cb_plane_size(a3)
	move.l	d1,d0
	move.l	#MEMF_CHIP!MEMF_CLEAR,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)		; chipmem for the viewport
	move.l	d0,cb_screenmem(a3)
	rts

datablock:	blk.b	2000,0	
	ENDC

own_delay:
	move.l	cb_waitmask(a3),d0
	move.l	$4.w,a6
	jsr	_LVOWait(a6)
	btst	#6,$bfe001
	bne	.no
	moveq	#-1,d0
	rts
.no:
	moveq	#0,d0
	rts

check_key:
	move.b	$bfec01,d0
	not	d0
	ror.b	#1,d0
	cmp.b	#$60,d0
	bhi.b	.no_key_ch1

	move.l	cb_dosbase(a3),a6
	movem.l	d1/a0/a1,-(a7)
	moveq	#1,d1
	jsr	_LVODelay(a6)		; give system time to check keys
	movem.l	(a7)+,d1/a1/a0
.no_key_ch1
	moveq	#0,d0
	rts

*
* Open the needed libraries
*
open_libs:
	moveq	#0,d0
	lea	graphname(pc),a1
	move.l	$4.w,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,cb_graphbase(a3)
	beq	lib_err2

	moveq	#0,d0
	lea	dosname(pc),a1
	move.l	$4.w,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,cb_dosbase(a3)
	beq	lib_err2

	moveq	#0,d0
	lea	intname(pc),a1
	move.l	$4.w,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,cb_intbase(a3)
	beq	lib_err2

	move.l	cb_graphbase(a3),a6
	move.w	LIB_VERSION(a6),d0
	cmp.w	#LIBV39,d0
	blt.w	no_v39_1
	move.b	#$ff,cb_v39_present(a3)
no_v39_1:
	move.b	gb_ChipRevBits0(a6),d0
	and.b	#SETCHIPREV_AA,d0
	cmp.b	#SETCHIPREV_AA,d0
	bne.b	no_aa1
	move.b	#$ff,cb_aa_present(a3)
no_aa1:
	moveq	#0,d0
	rts

lib_err2:
	moveq	#-1,d0
	rts

close_all:
	bsr.w	free_viewports
	move.l	cb_colormap(a3),d0
	beq.b	.no_cmap
	move.l	cb_graphbase(a3),a6
	move.l	d0,a0
	jsr	_LVOFreeColorMap(a6)
	move.l	#0,cb_colormap(a3)
.no_cmap:
	
	bsr.w	close_libs
	rts
	
do_scroll:

	jsr	load_background
	jsr	load_bob1

	bsr.w	set_size
	bsr.w	initialize_viewports

	bsr.w	setsam
	tst.b	d0
	bne.b	not_loaded

	bsr.w	install_50h

	lea	sigmask(pc),a0
	move.l	(a0),d0
	move.l	d0,cb_waitmask(a3)

	move.l	cb_graphbase(a3),a6

	move.l	gb_ActiView(a6),cb_oldview(a3)
	move.l	gb_ActiView(a6),a1
	lea	cb_viewblock1(a3),a5
	move.l	a5,cb_active_view(a3)
	lea	cb_viewblock2(a3),a5
	move.l	a5,cb_inactive_view(a3)

	bsr.w	test_pixels

	bsr.w	restore_oldview
	
	bsr.w	remove_50h
	bsr.w	freesam

	move.l	cb_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)

not_loaded:
	moveq	#0,d0
	rts

change_viewport:
	move.l	cb_inactive_view(a3),d0
	move.l	cb_active_view(a3),cb_inactive_view(a3)
	move.l	d0,cb_active_view(a3)
	move.l	cb_active_view(a3),a5
;	bsr	own_delay
	bsr	show_viewport
	bsr	own_delay
	move.l	cb_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)
	rts

test_pixels:
	bsr	init_bob1
	bsr	init_mul_tabel

;	moveq	#100,d0
;	moveq	#100,d1
;	lea	tbob,a0
;	bsr	test_blit_inactive
;	moveq	#110,d0
;	moveq	#110,d1
;	lea	tbob,a0
;	bsr	test_blit_inactive

	bsr	change_viewport

;	bsr	vlakje
	bsr	patroontjes
;	bsr	scroll
;	bsr	tonnetje

	move.l	cb_active_view(a3),a5
	bsr	show_viewport
;	bsr	change_viewport

.ww:	btst	#6,$bfe001
	bne	.ww
	jsr	free_background
	jsr	free_bob1
	rts

init_mul_tabel:
	move.l	cb_breedte_x(a3),d1
	mulu	cb_planes(a3),d1
	lea.l	mt(pc),a0
	moveq	#0,d0
	move.l	#511,d7
rep_mt:	move.w	d0,(a0)+
	add.w	d1,d0
	dbf	d7,rep_mt
	rts

	IFNE 0
	RSRESET
bi_struct:	rs.w	0
bi_width_bytes:	rs.l	1
bi_height:	rs.w	1
bi_blitheight:	rs.l	1
bi_memsize:	rs.l	1
bi_image_data:	rs.l	1
bi_image_mask:	rs.l	1
bi_SIZEOF:	rs.w	0
	ENDC

init_bob1:
	lea	bob1struct,a0
init_bob:
	moveq	#0,d0
	move.w	iff_width(a0),d0
	add.w	#15,d0
	lsr.w	#4,d0
	add.w	d0,d0
	move.l	d0,iff_width_bytes(a0)
	move.w	iff_height(a0),d0
	mulu	cb_planes(a3),d0
	move.l	d0,iff_blitheight(a0)

	move.l	iff_width_bytes(a0),d0
	addq.w	#2,d0				; add two for mask
	mulu	cb_planes(a3),d0
	mulu	iff_height(a0),d0		; mask size
	move.l	a0,a4
	move.l	#MEMF_CHIP,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
	move.l	a4,a0
	move.l	d0,iff_mask(a0)
	beq	no_maskmem
;	rts

	move.l	iff_image(a0),a1
	move.l	iff_mask(a0),a2
	move.l	iff_width_bytes(a0),d3

	moveq	#0,d5
	move.w	cb_planes(a3),d5
	subq.w	#1,d5
	move.l	d5,d6
	mulu	d3,d5
	addq.w	#2,d3
	mulu	d3,d6
	subq.l	#2,d3
	move.w	iff_height(a0),d0
	subq.w	#1,d0	
.repl:
	move.w	#2,d4
	move.l	iff_width_bytes(a0),d4
	lsr.l	#1,d4
	subq.w	#1,d4
.repw:
	move.w	cb_planes(a3),d1
	subq.w	#1,d1
	movem.l	a1/a2,-(a7)
	move.w	(a1),(a2)
.repp:
	move.w	(a1),d2
	or.w	d2,(a2)
	add.l	d3,a1
	dbf	d1,.repp
	movem.l	(a7)+,a1/a2
	addq.w	#2,a1
	addq.w	#2,a2
	dbf	d4,.repw
	clr.w	(a2)+
	add.l	d5,a1
	add.l	d6,a2
	dbf	d0,.repl

dum:
	move.l	iff_mask(a0),a2
	move.l	a2,a1

	move.l	iff_width_bytes(a0),d3
	move.w	cb_planes(a3),d5
	subq.w	#1,d5
	addq.w	#2,d3
	mulu	d3,d5
	move.w	iff_height(a0),d0
	subq.w	#1,d0	
.repl:
	move.w	#2,d4
	move.l	iff_width_bytes(a0),d4
	addq.w	#2,d4
	lsr.l	#1,d4
	subq.w	#1,d4
.repw:
	move.w	cb_planes(a3),d1
	subq.w	#1,d1
	movem.l	a1/a2,-(a7)
	move.w	(a2),d2
.repp:
	move.w	d2,(a1)
	add.l	d3,a1
	dbf	d1,.repp
	movem.l	(a7)+,a1/a2
	addq.w	#2,a1
	addq.w	#2,a2
	dbf	d4,.repw
	add.l	d5,a2
	add.l	d5,a1
	dbf	d0,.repl
	moveq	#0,d0
	rts

no_maskmem:
	moveq	#-1,d0
	rts

*
* blit de blitin in het plaatje
* d0,d1 geven x en y aan
* a0 points to bob info block
*
test_blit_inactive:
	move.l	a0,a4
	move.l	d0,d6
	and.l	#$f,d6			; shift waarde	destination
	move.l	d6,cb_shiftdest(a3)
	beq	no_A_lastw
	move.l	#$ffff0000,d7
	lsr.l	d6,d7
	move.w	d7,cb_Alastword(a3)
no_A_lastw:
	lsr.l	#4,d0			; op een heel word gezet
	add.l	d0,d0			; bytes

	moveq	#0,d7

	move.l	iff_image(a4),a1	; source pointer
	move.l	iff_mask(a4),a6

	lea	mt(pc),a2
	add.w	d1,d1
	move.w	0(a2,d1.w),d5

	add.l	d0,d5

	move.l	a6,-(a7)		; ??????????????????
	move.l	cb_graphbase(a3),a6
	jsr	_LVOOwnBlitter(a6)	; blitter multi-tasking ends here
	move.l	(a7)+,a6

	move.l	vi_mem(a5),a2
	add.l	d5,a2

	moveq	#0,d1
	move.l	iff_width_bytes(a4),d2

	move.l	#$dff000,a0		; hardware basis registers

	move.l	iff_blitheight(a4),d7

	move.l	cb_breedte_x(a3),d0	; destination
	sub.w	d2,d0			; min breedte aantal bytes

	move.w	#$0fca,d6		; de min termen voor de blitter

	move.l	cb_shiftdest(a3),d4
	swap	d4
	lsr.l	#4,d4
	or.w	d4,d6			; schuif source 

	lsr.w	#1,d2			; words voor breedte blit

wbl7:	btst	#14,dmaconr(a0)		; wacht tot blitter vrij is
	bne.b	wbl7

	move.w	#0,bltcon1(a0)
	move.l	#$ffffffff,bltafwm(a0)

	move.w	d1,bltamod(a0)
	sub.w	#2,d1
	move.w	d1,bltbmod(a0)
	subq.w	#2,d0
	move.w	d0,bltcmod(a0)
	move.w	d0,bltdmod(a0)		; zet de modulo destination

	tst.w	cb_Alastword(a3)
	beq	no_l
	move.w	#$0,bltalwm(a0)
no_l:
	move.w	d6,bltcon0(a0)

	move.l	cb_shiftdest(a3),d4
	swap	d4
	lsr.l	#4,d4
	move.w	d4,bltcon1(a0)

	move.l	a6,bltapt(a0)
	move.l	a1,bltbpt(a0)
	move.l	a2,bltcpt(a0)
	move.l	a2,bltdpt(a0)

	move.w	d7,bltsizv(a0)
	addq.w	#1,d2
	move.w	d2,bltsizh(a0)

	move.l	cb_graphbase(a3),a6
	jsr	_LVODisownBlitter(a6)	; geef bliter weer vrij
	rts	

*
* Set the different screen sizes
* The y size should be a value * fontsize
*
set_size:
	move.w	cb_lenviewy(a3),cb_leny(a3)
	move.w	#DEF_MODE,cb_mode(a3)
	move.w	#DEF_PLANES,cb_planes(a3)

	moveq	#0,d0
	move.w	cb_lenviewx(a3),d0				; use defaults
	add.l	#15,d0
	lsr.l	#4,d0
	lsl.l	#4,d0
	move.w	d0,cb_lenx(a3)		; true length bitmap
	lsr.l	#3,d0
	move.l	d0,cb_breedte_x(a3)	; in bytes

	moveq	#0,d0
	move.w	cb_leny(a3),d0
	rts

*
* Initialize two views with each a viewport
* These two view are used to scroll the text up with double buffering
*
initialize_viewports:
	move.l	cb_graphbase(a3),a6
	moveq	#32,d0
	jsr	_LVOGetColorMap(a6)
	move.l	d0,cb_colormap(a3)

	lea	cb_viewblock1(a3),a5
	move.l	cb_screenmem(a3),vi_mem(a5)
	bsr.w	create_viewport

	lea	cb_viewblock2(a3),a5
	move.l	cb_screenmem(a3),vi_mem(a5)
	move.l	cb_memsize(a3),d0
	lsr.l	#1,d0
	add.l	d0,vi_mem(a5)
	bsr.w	create_viewport

	bsr	loadcolors

	lea	cb_rastport(a3),a1
	jsr	_LVOInitRastPort(a6)

	lea	cb_viewblock1(a3),a5
	bsr.w	make_viewport

	lea	cb_viewblock2(a3),a5
	bsr.w	make_viewport
	lea	cb_viewblock1(a3),a5
	rts

free_viewports:
	lea	cb_viewblock2(a3),a5
	bsr.w	free_view

	lea	cb_viewblock1(a3),a5
	bsr.w	free_view
	rts
*
* Simply create copperlists for the viewblock in a5
*
make_viewport:
	move.l		cb_graphbase(a3),a6
	lea		vi_view(a5),a0
	lea		vi_viewport(a5),a1
	jsr		_LVOMakeVPort(a6)

	lea		vi_view(a5),a1
	jsr		_LVOMrgCop(a6)

	rts

create_viewport:
	move.l		cb_graphbase(a3),a6
	lea.l		vi_view(a5),a1
	jsr		_LVOInitView(a6)

	lea.l		vi_view(a5),a1
	move.w		cb_viewx(a3),v_DxOffset(a1)
	move.w		cb_viewy(a3),v_DyOffset(a1)

	lea.l		vi_view(a5),a1
	move.w		cb_mode(a3),d0
	or.w		cb_or_vmode_mask(a3),d0
	and.w		cb_and_vmode_mask(a3),d0
	move.w		d0,v_Modes(a1)
	lea.l		vi_viewport(a5),a0
	jsr		_LVOInitVPort(a6)

	lea.l		vi_view(a5),a0
	lea.l		vi_viewport(a5),a1
	move.l		a1,v_ViewPort(a0)

	lea		vi_bitmap(a5),a0
	move.w		cb_planes(a3),d0		; depth
	move.l		cb_breedte_x(a3),d1		; breedte in bytes * 8
	mulu		cb_planes(a3),d1
	lsl.l		#3,d1				; width
	move.l		cb_bitmapheight(a3),d2
	jsr		_LVOInitBitMap(a6)

	lea		vi_bitmap(a5),a0
;	move.l		cb_screenmem(a3),d0
	move.l		vi_mem(a5),d0
	move.l		cb_breedte_x(a3),d1

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

	lea.l		vi_rasinfo(a5),a0		; initialiseer rasinfo
	lea.l		vi_bitmap(a5),a1
	move.l		a1,ri_BitMap(a0)

	move.w		#0,ri_RxOffset(a0)
	move.w		#0,ri_RyOffset(a0)
	move.l		#0,ri_Next(a0)

	lea		vi_viewport(a5),a0
	lea		vi_rasinfo(a5),a1
	move.l		a1,vp_RasInfo(a0)

	move.w		cb_lenviewx(a3),vp_DWidth(a0)
	move.w		cb_lenviewy(a3),vp_DHeight(a0)
	move.w		cb_mode(a3),vp_Modes(a0)
	clr.l		vp_Next(a0)

	move.w		cb_columns(a3),d1	; zet de offsets berekend uit de prefs offsets
	move.w		cb_mode(a3),d0
	and.w		#V_HIRES,d0
	bne		hi1
	lsr.w		#1,d1
hi1:

; test de offset berekening met de byte lengte

	move.l	cb_breedte_x(a3),d0
	lsl.l	#3,d0
	sub.w	d1,d0
	asr.w	#1,d0
	neg.w	d0

	move.w	d0,vp_DxOffset(a0)

	bsr	get_dy_offset
	
	move.w	d0,vp_DyOffset(a0)

	moveq	#VIEW_EXTRA_TYPE,d0
	jsr	_LVOGfxNew(a6)
	tst.l	d0
	beq	exit_maakv

	move.l	d0,vi_vextra(a5)

	move.l	d0,a1
	lea.l	vi_view(a5),a0
	jsr	_LVOGfxAssociate(a6)

	lea.l	vi_view(a5),a0
	or.w	#EXTEND_VSTRUCT,v_Modes(a0)

	move.l	vi_vextra(a5),a0
	move.l	cb_monitorspec(a3),ve_Monitor(a0)

	moveq	#VIEWPORT_EXTRA_TYPE,d0
	jsr	_LVOGfxNew(a6)
	move.l	d0,vi_vpextra(a5)
	beq	exit_maakv

	move.l	d0,a1
	lea.l	vi_viewport(a5),a0
	jsr	_LVOGfxAssociate(a6)

	move.l	cb_tags(a3),a0
	move.l	vi_vpextra(a5),12(a0)

	move.w	#0,a0
	lea	cb_dimquery(a3),a1
	moveq	#dim_SIZEOF,d0
	move.l	#DTAG_DIMS,d1
	move.l	cb_monid(a3),d2
	or.w	cb_mode(a3),d2
	or.w	cb_or_vmode_mask(a3),d2
	jsr	_LVOGetDisplayInfoData(a6)

	lea	cb_dimquery(a3),a0
	move.l	vi_vpextra(a5),a1

	move.l	dim_MaxOScan(a0),vpe_DisplayClip(a1)
	move.l	dim_MaxOScan+4(a0),vpe_DisplayClip+4(a1)

	move.l	cb_monid(a3),d0
	or.w	cb_mode(a3),d0
	jsr	_LVOFindDisplayInfo(a6)
	move.l	cb_tags(a3),a0
	move.l	d0,20(a0)		

	move.l	cb_colormap(a3),d0
	move.l	d0,a0
	move.l	cb_tags(a3),a1
	lea.l	vi_viewport(a5),a2
	move.l	a2,4(a1)
	jsr	_LVOVideoControl(a6)

exit_maakv:
	rts

loadcolors:
	lea	colors(pc),a4
	lea	background,a0
	tst.l	iff_colors(a0)
	beq	.no_ch
	move.l	iff_colors(a0),a4
.no_ch:
	move.l	cb_graphbase(a3),a6
	moveq	#0,d0
	tst.b	cb_aa_present(a3)
	beq	set_no_aa

.rep_cols:
	move.l	cb_colormap(a3),a0
	bsr	get_32bit_col
	move.l	d5,d1
	bsr	get_32bit_col
	move.l	d5,d2
	bsr	get_32bit_col
	move.l	d5,d3
	move.l	d0,-(a7)
	jsr	_LVOSetRGB32CM(a6)
	move.l	(a7)+,d0
	addq.l	#1,d0
	cmp.w	#31,d0
	ble	.rep_cols
	rts

set_no_aa:
.rep_cols:
	move.l	cb_colormap(a3),a0
	move.l	d0,-(a7)
	move.b	(a4)+,d1
	move.b	(a4)+,d2
	move.b	(a4)+,d3
	and.l	#$ff,d1
	and.l	#$ff,d2
	and.l	#$ff,d3
	lsr.b	#4,d1
	lsr.b	#4,d2
	lsr.b	#4,d3
	jsr	_LVOSetRGB4CM(a6)
	move.l	(a7)+,d0
	addq.l	#1,d0
	cmp.w	#31,d0
	ble	.rep_cols
	rts

*
* Get byte value from (a4) and convert to 32 bit value to d5
*
get_32bit_col:
	moveq	#0,d4
	moveq	#0,d5
	move.b	(a4)+,d5
	move.b	d5,d4
	swap	d4
	or.l	d5,d4
	move.l	d4,d5
	lsl.l	#8,d4
	or.l	d4,d5
	rts
		
get_dy_offset:
	move.w	cb_rows(a3),d1
	move.w	cb_lenviewy(a3),d0
	lsl.w	#1,d0			; set on interlace
	lsl.w	#1,d1
	move.w	cb_mode(a3),d2
	and.w	#V_LACE,d2
	beq	.no_inter1
	lsr.w	#1,d0			; was lace so divide again
.no_inter1:
	and.w	#$fffe,d0
	sub.w	d1,d0
	neg.w	d0
	asr.w	#1,d0
	and.w	#$fffc,d0

	move.w	cb_mode(a3),d2
	and.w	#V_LACE,d2
	bne.s	.inter5
	asr.w	#1,d0
.inter5:
	tst.w	d0
	bge	.okie
	moveq	#0,d0
.okie:
	rts

	IFEQ XAPP
temp_tags:
	dc.l	VTAG_ATTACH_CM_SET,0
	dc.l	VTAG_VIEWPORTEXTRA_SET,0
	dc.l	VTAG_NORMAL_DISP_SET,0

	dc.l	VTAG_BORDERBLANK_CLR,0
	dc.l	VTAG_CHROMAKEY_SET,0
	dc.l	VTAG_CHROMA_PEN_CLR,0

	dc.l	VTAG_END_CM,0
	ENDC

show_viewport:
	move.l		cb_graphbase(a3),a6
	lea		vi_view(a5),a1
	jsr		_LVOLoadView(a6)
	rts

restore_oldview:
	move.l		cb_graphbase(a3),a6
	lea		vi_view(a5),a0

cleanexit:
	move.l		cb_graphbase(a3),a6
	tst.l		cb_oldview(a3)
	beq.b		no_oldview
	move.l		cb_oldview(a3),a1
	jsr		_LVOLoadView(a6)
	jsr		_LVOWaitTOF(a6)
no_oldview:
	rts

free_view:
	move.l		cb_graphbase(a3),a6
	lea		vi_viewport(a5),a0
	move.l		#0,vp_Next(a0)
	jsr		_LVOFreeVPortCopLists(a6)

	lea		vi_view(a5),a0
	tst.l		v_LOFCprList(a0)
	beq.b		no_lofmem
	move.l		v_LOFCprList(a0),a0
	jsr		_LVOFreeCprList(a6)
no_lofmem:

	lea		vi_view(a5),a0
	tst.l		v_SHFCprList(a0)
	beq.b		no_shfmem
	move.l		v_SHFCprList(a0),a0
	jsr		_LVOFreeCprList(a6)
no_shfmem:

no_colormap:
	moveq		#0,d0
	rts

close_libs:
	tst.l	cb_intbase(a3)
	beq	.no_int
	move.l	cb_intbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	cb_intbase(a3)
.no_int:
	tst.l	cb_graphbase(a3)
	beq	.no_gr
	move.l	cb_graphbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	cb_graphbase(a3)
.no_gr:
	tst.l	cb_dosbase(a3)
	beq	.no_dos
	move.l	cb_dosbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	cb_dosbase(a3)
.no_dos:
	moveq	#0,d0
	rts

setsam:
	move.l	$4.w,a6
	moveq	#-1,d0
	jsr	_LVOAllocSignal(a6)	

	cmp.l	#-1,d0
	beq.b	setsam_error
	lea	signum(pc),a0
	move.l	d0,(a0)
	move.l	d0,d1
	moveq	#$1,d0
	lsl.l	d1,d0	
	lea	sigmask(pc),a0
	move.l	d0,(a0)
	sub.l	a1,a1
	move.l	$4.w,a6
	jsr	_LVOFindTask(a6)
	lea	task(pc),a0
	move.l	d0,(a0)
	moveq	#0,d0
	rts

setsam_error:
	rts

freesam:
	move.l	signum(pc),d0
	cmp.l	#-1,d0
	beq.b	nofreesam
	move.l	$4.w,a6
	jsr	_LVOFreeSignal(a6)
nofreesam:
	lea	signum(pc),a0
	clr.l	(a0)
	lea	sigmask(pc),a0
	clr.l	(a0)
	rts

install_50h:
	lea	handle_installed(pc),a0
	tst.b	(a0)
	bne.b	no_install

	lea.l	intstruct50h(pc),a1
	lea	proc50hnaam(pc),a0
	move.l	a0,10(a1)
	lea	teller(pc),a0
	move.l	a0,14(a1)
	lea	proc_50h(pc),a0
	move.l	a0,18(a1)

	moveq	#5,d0
	move.l	$4.w,a6
	jsr	_LVOAddIntServer(a6)
	lea	handle_installed(pc),a0
	move.b	#$ff,(a0)
no_install:
	rts

remove_50h:
	lea.l	intstruct50h(pc),a1
	moveq	#5,d0
	move.l	$4.w,a6
	jsr	_LVORemIntServer(a6)
	lea	handle_installed(pc),a0
	move.b	#$00,(a0)
	rts

proc_50h:
	movem.l	d0/a0/a1,-(a7)
	tst.l	off_teller(a1)
	beq	.si
	sub.l	#1,off_teller(a1)
	bne	.no_si
.si:
	move.l	off_tel_start(a1),off_teller(a1)
	move.l	$4.w,a6
	move.l	off_task(a1),a1
	move.l	sigmask(pc),d0
	jsr	_LVOSignal(a6)
.no_si:
	movem.l	(a7)+,d0/a1/a0
	moveq	#0,d0
	rts

proc50hnaam:	dc.b	"50Hz DrieD",0
	even
		
intstruct50h:
	dc.l	0,0
	dc.b	2,15		; type en pri
	dc.l	0,0		; pointer naar naam en data
	dc.l	0

teller:		dc.l	1
tellerstart:	dc.l	1
task:		dc.l	0
signum:		dc.l	0
sigmask:	dc.l	0
handle_installed:	dc.b	0,0

off_teller 	= 0
off_tel_start 	= 4
off_task 	= 8
off_signum 	= 12
off_sigmask 	= 16

graphname:	dc.b	"graphics.library",0
intname:	dc.b	"intuition.library",0
dosname:	dc.b	"dos.library",0
	even

mt:	blk.l	600,0





extra_mem = 20000
ciaapra = $bfe001
s_y=-300
breedte=29

dep=150

spee=1
spe=1
ixo=0
iyo=0
izo=1200
acur=6
xnul=160
ynul=100

* sprite routines door Cees Lieshout
* gemaakt in 1988 1989 en verder
*
vlakje:	
	move.w	#380,zo
	clr.w	transx
	clr.w	transy
	clr.w	transz
	move.w	#50,welke_z

	move.l	#-25,start_x
	move.l	#-25,start_y
	move.l	#10,aantal_x
	move.l	#10,aantal_y
	move.w	#10,mdelta_x
	move.w	#10,mdelta_y
	clr.l	aantalpoints
	lea	point_store_temp2,a0
	move.l	a0,opwaarpoints
	lea	point_store_temp1(pc),a0
	move.l	a0,oppoints
	move.w	#1,welke_sprite

	move.l	#10,d7
vrep:	move.l	d7,-(a7)
	bsr.L	init_points
	sub.l	#8,a0
	move.l	(a7)+,d7
	sub.w	#5,welke_z
	add.l	#1,aantal_y
	add.l	#1,aantal_x
;	dbf	d7,vrep
vww:
	bsr.L	put_all_sprites
	btst	#6,$bfe001
	bne.S	vww
	rts

scroll:
	lea	point_store_temp2,a0
	move.l	a0,opwaarpoints
	lea	point_store_temp1(pc),a0
	move.l	a0,oppoints
	move.w	#1,welke_sprite

	lea	patje,a0
	move.l	a0,waar_patroon

	clr.l	aantalpoints
	clr.l	scroll_teller
	clr.l	anglexinc+4
	move.l	#0,anglexinc

	clr.l	anglexinc+8

	move.l	#0,anglex
	move.l	#0,angley
	move.l	#0,angley

	clr.w	transx
	clr.w	transy
	move.w	#400,transz
	move.w	#-100,transx

	move.l	#90,anglez
	move.b	#0,modulbit
	move.w	#1280,zo
	bsr.L	maakpatroon

	move.w	#0,modulus

	move.l	waar_patroon,welk_patroon

	move.w	#0,welke_z

	move.l	#-60,start_x
	move.l	#s_y,start_y
	move.l	#5,aantal_x
	move.l	#breedte,aantal_y
	move.l	oppoints,a0

	bsr.L	sinit_points
	bsr.L	put_all_sprites

	move.l	#19,d4
sww99:
	movem.l	d0-d7/a0-a6,-(a7)
	sub.w	#20,transz
	add.w	#5,transx
	sub.w	#50,zo


	bsr.L	put_all_sprites
	movem.l	(a7)+,d0-d7/a0-a6

	dbf	d4,sww99

*	clr.l	anglexinc
*	clr.l	anglexinc+4
*	clr.l	anglexinc+8

	clr.w	transx
	clr.w	transy
	clr.w	transz

*	move.l	#5,anglexinc+4

	move.l	#5,change_teller
*	move.w	#270,zo

*	move.w	#80,modulus

	move.l	#0,sinusangle

	move.l	#5,tempo_angle
	clr.l	tempo_x
sww5:
	lea	$dff000,a0

	move.l	tempo_angle,d0
	add.l	d0,sinusangle
	cmpi.l	#359,sinusangle
	bls.S	snopo_sin
	move.l	#0,sinusangle
snopo_sin:

	clr.l	aantalpoints

	move.l	#-60,start_x
	move.l	#s_y,start_y
	move.l	#5,aantal_x
	move.l	#breedte,aantal_y
	move.l	oppoints,a0

	bsr.L	sinit_points
	bsr.L	put_all_sprites


	add.l	#6,welk_patroon
	move.l	eind_patroon,d0
	cmpi.l	welk_patroon,d0
	bhi.S	snopo12
	move.l	waar_patroon,welk_patroon
	bra.L	sstopmaar
snopo12:

	cmpi.b	#1,modulbit
*	bne	snopo_modu2

	move.w	mod_inc,d0
	add.w	d0,modulus
	cmpi.w	#0,modulus
	bgt.S	snopo_modu1
	neg.w	mod_inc

snopo_modu1:
	cmpi.w	#30,modulus
	bls.S	snopo_modu2
	neg.w	mod_inc

snopo_modu2:

	sub.l	#1,change_teller
	bne.S	sno_chan

	move.l	#10,change_teller
	add.l	#1,tempo_angle
	cmpi.l	#20,tempo_angle
	bls.S	sno_chan

	move.l	#anglexinc,a1
	move.l	#2,d1
srep_cin:
	moveq	#0,d0
	move.w	$dff006,d0
*	lsr.w	#8,d0
	andi.w	#$7,d0
	move.l	d0,(a1)+
	dbf	d1,srep_cin

*	neg.b	modulbit
*	move.w	#0,modulus

	move.l	#10,tempo_angle
sno_chan:
	btst	#10,$dff016
	bne.S	sno_patchange

	move.l	#90,anglez

	clr.l	anglexinc
	clr.l	anglexinc+4
	clr.l	anglexinc+8

	move.l	#10,anglex
*	clr.l	anglex
	clr.l	angley
*	clr.l	anglez
	move.b	#-1,modulbit


sno_patchange:
	btst	#10,$dff016
	bne.L	sww5

sstopmaar:
	move.l	#6,anglexinc
	move.l	#8,anglexinc+4
	move.l	#13,anglexinc+8
	move.l	#10,d7
sstop3:	move.l	d7,-(a7)
	add.w	#10,transz
	add.w	#10,transx
	bsr.L	put_all_sprites
	move.l	(a7)+,d7
	dbf	d7,sstop3

	move.l	#19,d7
sstopmaar2:
	add.w	#50,zo
	move.l	d7,-(a7)
	add.w	#10,transz
	add.w	#20,transx

	bsr.L	put_all_sprites
	move.l	d7,d3

	move.l	(a7)+,d7
	dbf	d7,sstopmaar2
	rts

**********************************
tonnetje:
	lea	point_store_temp2,a0
	move.l	a0,opwaarpoints
	lea	point_store_temp1(pc),a0
	move.l	a0,oppoints
	move.w	#1,welke_sprite
	clr.l	tijdteller
	move.w	#430,transz
	clr.w	transy
	clr.w	transx

	clr.l	anglexinc
	clr.l	anglexinc+4
	clr.l	anglexinc+8

	move.w	#720,zo
	move.w	#30,modulus
	move.l	#1,tempo_angle
	clr.l	cirhoek
	clr.l	sinusangle
	
	move.l	#44,d7
tww10:
	move.l	d7,-(a7)
	sub.w	#10,transz
	sub.w	#10,zo

	clr.w	welke_sprite
	clr.l	aantalpoints
	move.w	#-125,welke_z
	move.l	#-60,start_x
	move.l	#-100,start_y
	move.l	#0,aantal_x
	move.l	#24,aantal_y
	move.l	oppoints,a0
	bsr.L	tinit_points
	bsr.L	put_all_sprites
	add.l	#1,cirhoek
	move.l	(a7)+,d7
	dbf	d7,tww10

	move.l	#356,anglex+8

	move.l	#5,anglexinc
	move.l	#4,anglexinc+4
	move.l	#3,anglexinc+8

	clr.l	tempo_x
tww5:
	lea	$dff000,a0
	move.l	tempo_angle,d0
	add.l	d0,sinusangle
	cmpi.l	#359,sinusangle
	bls.S	tnopo_sin
	move.l	#0,sinusangle
tnopo_sin:
	move.l	#16,d0
	sub.l	tempo_angle,d0
	add.l	d0,sinusangle2
	cmpi.l	#359,sinusangle2
	bls.S	tnopo_sin2
	move.l	#0,sinusangle2
tnopo_sin2:
	clr.w	welke_sprite
	clr.l	aantalpoints
	move.w	#-125,welke_z
	move.l	#-60,start_x
	move.l	#-100,start_y
	move.l	#0,aantal_x
	move.l	#24,aantal_y
	move.l	oppoints,a0
	bsr.L	tinit_points
	bsr.L	put_all_sprites

	move.w	mod_inc,d0
	add.w	d0,modulus
	cmpi.w	#20,modulus
	bhi.S	tnopo_modu1
	neg.w	mod_inc
tnopo_modu1:
	cmpi.w	#40,modulus
	bls.S	tnopo_modu2
	neg.w	mod_inc
tnopo_modu2:
	sub.l	#1,change_teller
	bne.S	tno_chan
	move.l	#20,change_teller

	add.l	#1,tempo_angle
	cmpi.l	#25,tempo_angle
	bls.S	tnopo_ang
	move.l	#5,tempo_angle
tnopo_ang:

;	bsr.L	setspeed
tnopo_temp:
tno_chan:
	add.l	#1,tijdteller
;	cmpi.l	#500,tijdteller
;	bhi.S	tstopmaar
	btst	#10,$dff016
	bne.L	tww5

tstopmaar:
	move.l	#3,anglexinc
	move.l	#4,anglexinc+4
	move.l	#2,anglexinc+8

	move.l	#9,d7
trepeind1:
	move.l	d7,-(a7)
	add.w	#20,transz
*	add.w	#50,zo

	clr.w	welke_sprite
	clr.l	aantalpoints
	move.w	#-125,welke_z
	move.l	#-60,start_x
	move.l	#-100,start_y
	move.l	#0,aantal_x
	move.l	#14,aantal_y
	move.l	oppoints,a0
	bsr.W	tinit_points
	bsr.L	put_all_sprites
	move.l	(a7)+,d7
	dbf	d7,trepeind1

	move.l	#19,d7
trepeind2:
	move.l	d7,-(a7)
	add.w	#20,transz
	add.w	#50,zo

	clr.w	welke_sprite
	clr.l	aantalpoints
	move.w	#-125,welke_z
	move.l	#-60,start_x
	move.l	#-100,start_y
	move.l	#0,aantal_x
	move.l	#14,aantal_y
	move.l	oppoints,a0
	bsr.S	tinit_points
	bsr.L	put_all_sprites
	move.l	(a7)+,d7
	dbf	d7,trepeind2
	rts

setspeed:
	move.l	#anglexinc,a1
	move.l	#2,d1
rep_cin:
	moveq	#0,d0
	move.w	$dff006,d0
*	lsr.w	#8,d0
	andi.w	#$f,d0
	move.l	d0,(a1)+
	dbf	d1,rep_cin
	rts


tinit_points:
	move.l	sinusangle,d3
	move.l	aantal_y,d7
	move.l	sinusangle2,d5
trepy:	move.l	d5,d4
	move.l	#7,d6
	clr.w	welke_sprite
	add.l	#20,d3
	cmpi.l	#179,d3
	bls.S	tnopo_sin3
	sub.l	#180,d3
tnopo_sin3:

trepx:	
	move.l	d3,d2
	lsl.l	#1,d2
	lea.l	sinustabel(pc),a1
	move.w	0(a1,d2.l),d2
	muls	modulus,d2
	asr.w	#acur,d2	
	add.w	#10,d2
*	move.w	#80,d2
	lsl.l	#1,d4
	move.w	0(a1,d4.l),d0
	lea.l	costabel(pc),a1
	move.w	0(a1,d4.l),d1
	lsr.l	#1,d4
	muls	d2,d0
	muls	d2,d1
	asr.w	#acur,d0
	asr.w	#acur,d1
	move.w	d0,(a0)+
 	move.w	d1,(a0)+
	move.w	welke_z,(a0)+
	move.w	welke_sprite,(a0)+
	add.l	#1,aantalpoints
	add.w	#1,welke_sprite
	andi.w	#$3,welke_sprite

*	add.l	#45,d4
	add.l	cirhoek,d4
	cmpi.l	#359,d4
	bls.S	tnopo_rou
	sub.l	#360,d4
tnopo_rou:
	dbf	d6,trepx
	add.l	tempo_angle,d5
	cmpi.l	#359,d5
	bls.S	tnopo_rou2
	sub.l	#360,d5
tnopo_rou2:
	move.w	mdelta_y,d0
	add.w	d0,welke_z
	dbf	d7,trepy

	move.w	#1000,(a0)+
	move.w	#1000,(a0)+
	move.w	#1000,(a0)+
	move.w	#1000,(a0)+
	rts


SX = -60
SY = -100
AX = 7
AY = 7
*********************************************************
patroontjes:
	clr.l	aantalpoints
	lea	point_store_temp2,a0
	move.l	a0,opwaarpoints
	lea	point_store_temp1(pc),a0
	move.l	a0,oppoints
	move.w	#1,welke_sprite
;	move.l	mem_oppoints,oppoints
	move.w	#1250,zo
	clr.w	transx
	clr.w	transy
	clr.w	transz

	move.w	#0,modulus
	move.l	patroonlijst,welk_patroon
	move.l	#patroonlijst+4,currentpatroon
	move.w	#0,welke_z

	move.l	#SX,start_x
	move.l	#SY,start_y
	move.l	#AX,aantal_x
	move.l	#AY,aantal_y
	clr.l	aantalpoints

	move.l	oppoints,a0

	bsr.L	pinit_points
	bsr.L	put_all_sprites

	move.l	#19,d4
pww99:
	movem.l	d0-d7/a0-a6,-(a7)
	sub.w	#50,zo
pnof:
	bsr.L	put_all_sprites
	movem.l	(a7)+,d0-d7/a0-a6

	btst	#6,ciaapra
	beq.L	pstopmaar

	dbf	d4,pww99

*	move.w	#270,zo
	move.l	#21,d7
pww100:
	move.l	d7,-(a7)

	clr.w	welke_sprite
	clr.l	aantalpoints

*	add.w	#1,modulus

	move.l	#SX,start_x
	move.l	#SY,start_y
	move.l	#AX,aantal_x
	move.l	#AY,aantal_y
	move.l	oppoints,a0

	bsr.L	pinit_points

	bsr.L	put_all_sprites

	move.l	(a7)+,d7
	btst	#6,ciaapra
	beq.L	pstopmaar

	dbf	d7,pww100

*	move.w	#80,modulus

	move.l	#0,sinusangle

	clr.w	modulus
	move.l	#20,tempo_angle
	clr.l	tempo_x
pww5:
	lea	$dff000,a0
	move.l	tempo_angle,d0
	add.l	d0,sinusangle
	cmpi.l	#359,sinusangle
	bls.S	pnopo_sin
	sub.l	#360,sinusangle
*	move.l	#0,sinusangle
pnopo_sin:

	clr.l	aantalpoints

	move.l	#SX,start_x
	move.l	#SY,start_y
	move.l	#AX,aantal_x
	move.l	#AY,aantal_y
	move.l	oppoints,a0

	bsr.L	pinit_points
	bsr.L	put_all_sprites

	move.w	welke_z,d0
	add.w	wel_inc,d0

	cmp.w	#-80,d0
	bgt.S	pnopo_zw
	neg.w	wel_inc
pnopo_zw:	
	cmpi.w	#80,d0
	blt.S	pnopo_zw2
	neg.w	wel_inc
pnopo_zw2:
	move.w	d0,welke_z

	cmpi.b	#1,modulbit
	bne.S	pnopo_modu2

	move.w	mod_inc,d0
	add.w	d0,modulus
	tst.w	modulus
	bpl.S	pnopo_modu1
	move.w	#1,mod_inc
*	neg.w	mod_inc
pnopo_modu1:
	cmpi.w	#30,modulus
	blt.S	pnopo_modu2
	neg.w	mod_inc
pnopo_modu2:

	sub.l	#1,change_teller
	bne.S	pno_chan

	move.l	#100,change_teller
	add.l	#1,tempo_angle
	cmpi.l	#23,tempo_angle
	bne.S	pnopo5

	neg.b	modulbit
pnopo5:
	cmpi.l	#25,tempo_angle
	bls.S	pno_chan

	neg.b	modulbit

	move.w	#0,modulus
	move.l	#20,tempo_angle

	bra.S	pver

pno_chan:
	btst	#10,$dff016
	bne.S	pno_patchange
pver:
	bsr.L	fade_o
	move.l	currentpatroon,a4
	move.l	(a4)+,d0
	bne.S	pnopo12

*	add.l	#1,tijdteller
*	cmpi.l	#2,tijdteller
	bra.L	pstopmaar3

	move.l	#patroonlijst,a4
	move.l	(a4)+,d0	
pnopo12:
	move.l	a4,currentpatroon
	move.l	d0,welk_patroon
	clr.l	aantalpoints

	move.l	#SX,start_x
	move.l	#SY,start_y
	move.l	#AX,aantal_x
	move.l	#AY,aantal_y
	move.l	oppoints,a0
	clr.w	welke_z
	bsr.S	pinit_points

	bsr.L	fade_i
pno_patchange:
	btst	#6,ciaapra
	bne.L	pww5

pstopmaar:
	move.l	#2,anglexinc
	move.l	#3,anglexinc+4
	move.l	#6,anglexinc+8
	move.l	#10,d7
pstop3:	move.l	d7,-(a7)
	add.w	#20,transz
	bsr.L	put_all_sprites
	move.l	(a7)+,d7
	dbf	d7,pstop3

	move.l	#19,d7
pstopmaar2:
	add.w	#50,zo
	move.l	d7,-(a7)
	add.w	#10,transz
	bsr.L	put_all_sprites
	move.l	d7,d3
	andi.l	#$3,d3
	cmpi.b	#2,d3
*	bne	noo

pnoo:

	move.l	(a7)+,d7
	dbf	d7,pstopmaar2
pstopmaar3:

	rts

pinit_points:
	move.l	sinusangle,d4
	move.l	aantal_y,d7
	move.l	start_y,d1
	move.l	welk_patroon,a4

prepy:	move.l	aantal_x,d6
	move.l	start_x,d0
	move.l	d4,d3
prepx:	

	move.w	d0,(a0)
	add.l	#30,d3
	cmpi.l	#359,d3
	bls.S	pnopo_sin2
	sub.l	#360,d3
pnopo_sin2:
	tst.b	(a4)
	beq.S	pno_patr
	lea.l	2(a0),a0
	move.l	d3,d2
	lsl.l	#1,d2
	lea.l	sinustabel,a1
	move.w	0(a1,d2.l),d2
	mulu	modulus,d2
	asr.w	#acur,d2
	move.w	d1,(a0)+
	cmpi.b	#10,(a4)
	bls.S	pno_zadd
	add.w	welke_z,d2
pno_zadd:
	move.w	d2,(a0)+

*	move.w	welke_z,(a0)+

	moveq	#0,d2
	move.b	(a4),d2
	cmpi.b	#10,d2
	bls.S	pno_po10
	sub.b	#10,d2
pno_po10:
	sub.b	#1,d2
	move.w	d2,(a0)+
	add.l	#1,aantalpoints
pno_patr:
	add.w	mdelta_x,d0
	lea.l	1(a4),a4
	dbf	d6,prepx
	add.w	mdelta_y,d1
	add.l	#20,d4
	dbf	d7,prepy
	move.w	#1000,(a0)+
	move.w	#1000,(a0)+
	move.w	#1000,(a0)+
	move.w	#1000,(a0)+
	rts

fade_i:	move.l	#55,d1
floop:	move.l	d1,-(a7)
	sub.w	#30,zo
	bsr.L	put_all_sprites
	move.l	(a7)+,d1
	dbf	d1,floop
	rts

fade_o:	move.l	#55,d1

floop2:	move.l	d1,-(a7)
	add.w	#30,zo
	bsr.L	put_all_sprites
	move.l	(a7)+,d1
	dbf	d1,floop2
	rts

put_all_sprites:
	move.l	cb_inactive_view(a3),a5
;	lea	sexy+64,a1
	move.l	back_image,a1
	move.l	vi_mem(a5),a0
	move.l	cb_plane_size(a3),d0
.repcl:
	move.l	(a1)+,(a0)+
	subq.l	#1,d0
	bne	.repcl

;
; clear the screen here
;

	bsr.L	draai_obj
	bsr.L	redraw

	bsr.L	bubblesort

	move.l	opwaarpoints,a2

reppll:
	move.w	(a2)+,d6
	cmp.w	#1000,d6
	beq.S	no_more_put
	move.w	(a2)+,d7
	cmpi.w	#300,d6
	bhi.S	niet_plaatsen
	cmpi.w	#190,d7
	bhi.S	niet_plaatsen
	move.w	2(a2),d5

	bsr.S	put_sprite

niet_plaatsen:
	addq.l	#4,a2

	bra.S	reppll
no_more_put:	
	bsr	change_viewport

	rts

put_sprite:
	movem.l	a0/a2,-(a7)
	move.l	d6,d0
	move.l	d7,d1
	lea	bob1struct,a0
	bsr	test_blit_inactive
	movem.l	(a7)+,a0/a2
	rts

init_points:
	move.l	sinusangle,d4
	move.l	aantal_y,d7
	move.l	start_y,d1
repy:	move.l	aantal_x,d6
	move.l	start_x,d0
	move.l	d4,d3
repx:	move.w	d0,(a0)+
	add.l	#30,d3
	cmpi.l	#359,d3
	bls.S	nopo_sin2
	sub.l	#360,d3
nopo_sin2:
	move.l	d3,d2
	lsl.l	#1,d2
	lea.l	sinustabel,a1
	move.w	0(a1,d2.l),d2
	mulu	modulus,d2
	asr.w	#acur,d2
	move.w	d1,(a0)+
	move.w	welke_z,(a0)+
	move.w	welke_sprite,(a0)+

	add.w	mdelta_x,d0
	add.l	#1,aantalpoints
	dbf	d6,repx
	add.w	mdelta_y,d1
	add.l	#20,d4
	dbf	d7,repy

	move.w	#1000,(a0)+
	move.w	#1000,(a0)+
	move.w	#1000,(a0)+
	move.w	#1000,(a0)+
	rts


bubblesort:
	movem.l	d0-d7/a0-a2,-(a7)
	move.l	opwaarpoints,a0
	move.l	aantalpoints,d2
	lsl.l	#3,d2		;upper
	clr.l	d1		;lower
	move.l	d2,d7
	bsr.S	quicks
	movem.l	(a7)+,d0-d7/a0-a2
	rts


quicks:	cmp.l	d2,d1
	bge.S	uitquic
	movem.l	d1/d2,-(a7)
	addq.l	#8,d2
	bsr.S	partit
	movem.l	(a7)+,d1/d2

	movem.l	d1/d2/d3,-(a7)
	move.l	d3,d2
	tst.l	d2
	beq.S	noq
	subq.l	#8,d2
	bsr.S	quicks
noq:	movem.l	(a7)+,d1/d2/d3
	movem.l	d1/d2/d3,-(a7)
	move.l	d3,d1
	addq.l	#8,d1
	bsr.S	quicks
	movem.l	(a7)+,d1/d2/d3
uitquic:
	rts

partit:
	move.w	4(a0,d1.l),d4
	move.l	0(a0,d1.l),a1
	move.l	4(a0,d1.l),a2	
	move.l	d1,d5
inc_d5:	cmp.l	d7,d5
	bge.S	dec_d2
	addq.l	#8,d5
	cmp.w	4(a0,d5.l),d4
	bgt.S	inc_d5
dec_d2:	tst.l	d2
	beq.S	edec_d2
	subq.l	#8,d2
	cmp.w	4(a0,d2.l),d4
	blt.S	dec_d2
edec_d2:
	cmp.l	d2,d5
	bge.S	einc_d5
	move.l	4(a0,d5.l),d6
	move.l	4(a0,d2.l),4(a0,d5.l)
	move.l	d6,4(a0,d2.l)

	move.l	0(a0,d5.l),d6
	move.l	0(a0,d2.l),0(a0,d5.l)
	move.l	d6,0(a0,d2.l)

	bra.S	inc_d5
einc_d5:
	move.l	4(a0,d2.l),4(a0,d1.l)
	move.l	0(a0,d2.l),0(a0,d1.l)
	
	move.l	a1,0(a0,d2.l)
	move.l	a2,4(a0,d2.l)
	move.l	d2,d3
	rts

draai_obj:
	lea	anglex,a1
	lea	anglexinc,a2

	moveq	#2,d1
repinc:	move.l	(a1),d0
	add.l	(a2),d0
	move.l	d0,(a1)
	cmpi.l	#359,d0
	bls.s	nopangle
	sub.l	#360,d0
poip:
	move.l	d0,(a1)
*	clr.l	(a1)
nopangle:
	addq.l	#4,a1
	addq.l	#4,a2
	dbf	d1,repinc
	rts

redraw:	
	move.l	oppoints,a2
	move.l	opwaarpoints,a4
repdraw:
	cmpi.w	#1000,(a2)
	beq.S	no_more_points

	bsr.S	project1

	move.w	d0,(a4)+
	move.w	d1,(a4)+
	move.w	d2,(a4)+

	move.w	6(a2),(a4)+

	add.l	#8,a2
	bra.s	repdraw

no_more_points:
	move.w	#1000,(a4)+
	move.w	#1000,(a4)+
	move.w	#1000,(a4)+
	move.w	#1000,(a4)+
	clr.l	d0
	rts


project1:
	movem.l	d3-d7/a3/a2,-(a7)
	moveq	#0,d0
	moveq	#0,d1
	moveq	#0,d2
	move.w	(a2),d0
	move.w	2(a2),d1
	move.w	4(a2),d2

	add.w	transx,d0
	add.w	transy,d1
	add.w	transz,d2
	
* d0*2 om de schaal tussen y en z wat te herstelen

*	asl.w	#1,d0

	bsr.S	rotx
	bsr.S	roty
	bsr.S	rotz

*	sub.w	xo,d0
*	sub.w	yo,d1
	sub.w	zo,d2

	bsr.S	perspective3

	add.w	#xnul,d0
	add.w	#ynul,d1
	movem.l	(a7)+,d3-d7/a3/a2
	rts

rotz:	move.l	anglez,angle
	bsr.S	rot
	rts

roty:	move.l	angley,angle
	exg	d1,d2
	bsr.S	rot
	exg	d1,d2
	rts

rotx:	move.l	anglex,angle
	exg	d0,d2
	bsr.S	rot
	exg	d0,d2
	rts

perspective3:
	move.l	d3,-(a7)
	move.w	#64*dep,d3
	cmpi.w	#0,d2
	bne.S	nop_nul
	move.l	#1,d2
nop_nul:
*	neg.l	d2
	divs	d2,d3
	muls	d3,d1
	muls	d3,d0
	asr.w	#acur,d1
	asr.w	#acur,d0
	move.l	(a7)+,d3
	rts

rot:	movem.l	a0/d2,-(a7)
	movem.l	d0/d1,-(a7)

	move.l	d1,d2
	move.l	d0,d1
	move.l	angle,d0
	bsr	get_easy_cosinus
	move.l	d0,d3

	move.l	angle,d0
	move.l	d2,d1
	bsr	get_easy_sinus

	sub.l	d0,d3
	move.l	d3,d6

	movem.l	(a7)+,d0/d1

	move.l	d1,d2
	move.l	d0,d1
	move.l	angle,d0
	bsr	get_easy_sinus
	move.l	d0,d3

	move.l	angle,d0
	move.l	d2,d1
	bsr	get_easy_cosinus
	add.l	d0,d3
	swap	d6
	swap	d3

	moveq	#0,d0
	moveq	#0,d1

	move.w	d6,d0
	move.w	d3,d1

	movem.l	(a7)+,a0/d2
	rts

rot_old:
	movem.l	a3/d2,-(a7)
	movem.l	d0/d1,-(a7)

	move.l	angle,d4
	lsl.l	#1,d4
	lea.l	costabel(pc),a3
	muls	0(a3,d4.l),d0

	move.l	angle,d4
;	lsl.l	#1,d4

	lea.l	sinustabel(pc),a3
	muls	0(a3,d4.l),d1

	sub.w	d1,d0
	move.w	d0,d6

	movem.l	(a7)+,d0/d1

;	move.l	angle,d4
;	lsl.l	#1,d4
	lea.l	sinustabel(pc),a3
	muls	0(a3,d4.l),d0
;	move.l	angle,d4
;	lsl.l	#1,d4
	lea.l	costabel(pc),a3
	muls	0(a3,d4.l),d1
	add.w	d0,d1
	move.w	d6,d0
	asr.w	#acur,d0
	asr.w	#acur,d1
	movem.l	(a7)+,a3/d2
	rts

sinustabel:
	dc.w	$0000,$0001,$0002,$0003,$0004,$0005,$0006,$0007
	dc.w	$0008,$000A,$000B,$000C,$000D,$000E,$000F,$0010
	dc.w	$0011,$0012,$0013,$0014,$0015,$0016,$0018,$0019
	dc.w	$001A,$001B,$001C,$001D,$001E,$001F,$0020,$0021
	dc.w	$0021,$0022,$0023,$0024,$0025,$0026,$0027,$0028
	dc.w	$0029,$002A,$002A,$002B,$002C,$002D,$002E,$002E
	dc.w	$002F,$0030,$0031,$0031,$0032,$0033,$0033,$0034
	dc.w	$0035,$0035,$0036,$0036,$0037,$0038,$0038,$0039
	dc.w	$0039,$003A,$003A,$003A,$003B,$003B,$003C,$003C
	dc.w	$003C,$003D,$003D,$003D,$003E,$003E,$003E,$003E
	dc.w	$003F,$003F,$003F,$003F,$003F,$003F,$003F,$003F
	dc.w	$003F,$003F,$003F,$003F
costabel:
	dc.w	$003F,$003F,$003F,$003F
	dc.w	$003F,$003F,$003F,$003F,$003E,$003E,$003E,$003E
	dc.w	$003E,$003D,$003D,$003D,$003C,$003C,$003C,$003B
	dc.w	$003B,$003A,$003A,$0039,$0039,$0038,$0038,$0037
	dc.w	$0037,$0036,$0036,$0035,$0034,$0034,$0033,$0032
	dc.w	$0032,$0031,$0030,$0030,$002F,$002E,$002D,$002D
	dc.w	$002C,$002B,$002A,$0029,$0028,$0028,$0027,$0026
	dc.w	$0025,$0024,$0023,$0022,$0021,$0020,$001F,$001E
	dc.w	$001D,$001C,$001B,$001A,$0019,$0018,$0017,$0016
	dc.w	$0015,$0014,$0013,$0012,$0011,$0010,$000F,$000D
	dc.w	$000C,$000B,$000A,$0009,$0008,$0007,$0006,$0005
	dc.w	$0004,$0002,$0001,$0000,$0000,$FFFF,$FFFE,$FFFD
	dc.w	$FFFC,$FFFA,$FFF9,$FFF8,$FFF7,$FFF6,$FFF5,$FFF4
	dc.w	$FFF3,$FFF2,$FFF1,$FFEF,$FFEE,$FFED,$FFEC,$FFEB
	dc.w	$FFEA,$FFE9,$FFE8,$FFE7,$FFE6,$FFE5,$FFE4,$FFE3
	dc.w	$FFE2,$FFE1,$FFE0,$FFDF,$FFDE,$FFDD,$FFDC,$FFDB
	dc.w	$FFDA,$FFDA,$FFD9,$FFD8,$FFD7,$FFD6,$FFD5,$FFD4
	dc.w	$FFD4,$FFD3,$FFD2,$FFD1,$FFD1,$FFD0,$FFCF,$FFCE
	dc.w	$FFCE,$FFCD,$FFCC,$FFCC,$FFCB,$FFCB,$FFCA,$FFC9
	dc.w	$FFC9,$FFC8,$FFC8,$FFC7,$FFC7,$FFC6,$FFC6,$FFC5
	dc.w	$FFC5,$FFC5,$FFC4,$FFC4,$FFC3,$FFC3,$FFC3,$FFC3
	dc.w	$FFC2,$FFC2,$FFC2,$FFC2,$FFC1,$FFC1,$FFC1,$FFC1
	dc.w	$FFC1,$FFC1,$FFC1,$FFC1,$FFC1,$FFC1,$FFC1,$FFC1
	dc.w	$FFC1,$FFC1,$FFC1,$FFC1,$FFC1,$FFC1,$FFC1,$FFC1
	dc.w	$FFC2,$FFC2,$FFC2,$FFC2,$FFC3,$FFC3,$FFC3,$FFC4
	dc.w	$FFC4,$FFC4,$FFC5,$FFC5,$FFC5,$FFC6,$FFC6,$FFC7
	dc.w	$FFC7,$FFC8,$FFC8,$FFC9,$FFC9,$FFCA,$FFCB,$FFCB
	dc.w	$FFCC,$FFCD,$FFCD,$FFCE,$FFCF,$FFCF,$FFD0,$FFD1
	dc.w	$FFD1,$FFD2,$FFD3,$FFD4,$FFD5,$FFD5,$FFD6,$FFD7
	dc.w	$FFD8,$FFD9,$FFDA,$FFDB,$FFDC,$FFDC,$FFDD,$FFDE
	dc.w	$FFDF,$FFE0,$FFE1,$FFE2,$FFE3,$FFE4,$FFE5,$FFE6
	dc.w	$FFE7,$FFE8,$FFE9,$FFEA,$FFEB,$FFEC,$FFEE,$FFEF
	dc.w	$FFF0,$FFF1,$FFF2,$FFF3,$FFF4,$FFF5,$FFF6,$FFF7
	dc.w	$FFF8,$FFFA,$FFFB,$FFFC,$FFFD,$FFFE,$FFFF,$0000
	dc.w	$0000,$0002,$0003,$0004,$0005,$0006,$0007,$0008
	dc.w	$0009,$000A,$000C,$000D,$000E,$000F,$0010,$0011
	dc.w	$0012,$0013,$0014,$0015,$0016,$0017,$0018,$0019
	dc.w	$001A,$001B,$001C,$001D,$001E,$001F,$0020,$0021
	dc.w	$0022,$0023,$0024,$0025,$0026,$0027,$0028,$0029
	dc.w	$0029,$002A,$002B,$002C,$002D,$002D,$002E,$002F
	dc.w	$0030,$0030,$0031,$0032,$0033,$0033,$0034,$0035
	dc.w	$0035,$0036,$0036,$0037,$0037,$0038,$0038,$0039
	dc.w	$0039,$003A,$003A,$003B,$003B,$003C,$003C,$003C
	dc.w	$003D,$003D,$003D,$003E,$003E,$003E,$003E,$003F
	dc.w	$003F,$003F,$003F,$003F,$003F,$003F,$003F,$003F
	dc.w	$003F,$0040,$0040,$0040
*
* Give cosinus of number in d0 with amplitude in d1
*
get_easy_cosinus:
	add.w	#90,d0
	cmp.w	#360,d0
	blt	get_easy_sinus
	sub.w	#360,d0
*
* Give sinus of number in d0 with amplitude in d1
*
get_easy_sinus:
	lea	sinus(pc),a0
	cmp.w	#180,d0
	bge	.minus_sinus
	add.w	d0,d0
	move.w	0(a0,d0.w),d0
	tst.w	d1
	bpl	.minus2
	neg.w	d1
	mulu	d1,d0
	and.l	#$ffff0000,d0
	neg.l	d0
	rts
	
.minus2:
	mulu	d1,d0
	and.l	#$ffff0000,d0
	rts

.minus_sinus:
	sub.w	#180,d0
	add.w	d0,d0
	move.w	0(a0,d0.w),d0
	tst.w	d1
	bpl	.minus3
	neg.w	d1
	mulu	d1,d0
	and.l	#$ffff0000,d0
	rts
	
.minus3:
	mulu	d1,d0
;	swap	d0
	and.l	#$ffff0000,d0
	neg.l	d0
	rts

sinus:
	DC.W	$0000,$0477,$08EF,$0D65,$11DB,$164F,$1AC2,$1F32,$23A0,$280C
	DC.W	$2C74,$30D8,$3539,$3996,$3DEE,$4241,$4690,$4AD8,$4F1B,$5358
	DC.W	$578E,$5BBE,$5FE6,$6406,$681F,$6C30,$7039,$7438,$782F,$7C1C
	DC.W	$7FFF,$83D9,$87A8,$8B6D,$8F27,$92D5,$9679,$9A10,$9D9B,$A11B
	DC.W	$A48D,$A7F3,$AB4C,$AE97,$B1D5,$B504,$B826,$BB3A,$BE3E,$C134
	DC.W	$C41B,$C6F3,$C9BB,$CC73,$CF1B,$D1B3,$D43B,$D6B3,$D919,$DB6F
	DC.W	$DDB3,$DFE7,$E208,$E419,$E617,$E803,$E9DE,$EBA6,$ED5B,$EEFF
	DC.W	$F08F,$F20D,$F378,$F4D0,$F615,$F746,$F865,$F970,$FA67,$FB4B
	DC.W	$FC1C,$FCD9,$FD82,$FE17,$FE98,$FF06,$FF60,$FFA6,$FFD8,$FFF6
	DC.W	$FFFF,$FFF6,$FFD8,$FFA6,$FF60,$FF06,$FE98,$FE17,$FD82,$FCD9
	DC.W	$FC1C,$FB4B,$FA67,$F970,$F865,$F746,$F615,$F4D0,$F378,$F20D
	DC.W	$F08F,$EEFF,$ED5B,$EBA6,$E9DE,$E803,$E617,$E418,$E208,$DFE7
	DC.W	$DDB3,$DB6F,$D919,$D6B3,$D43B,$D1B3,$CF1B,$CC73,$C9BB,$C6F3
	DC.W	$C41B,$C134,$BE3E,$BB3A,$B826,$B504,$B1D5,$AE97,$AB4C,$A7F3
	DC.W	$A48D,$A11B,$9D9C,$9A10,$9679,$92D5,$8F27,$8B6D,$87A8,$83D9
	DC.W	$8000,$7C1C,$782F,$7438,$7039,$6C30,$6820,$6407,$5FE6,$5BBE
	DC.W	$578E,$5358,$4F1B,$4AD9,$4690,$4242,$3DEE,$3996,$3539,$30D9
	DC.W	$2C74,$280C,$23A1,$1F33,$1AC2,$1650,$11DB,$0D66,$08EF,$0478

	even
conhandle:	dc.l	0
aantal_x:	dc.l	0
aantal_y:	dc.l	0
start_x:	dc.l	0
start_y:	dc.l	0
mdelta_x:	dc.w	25
mdelta_y:	dc.w	15

transx:	dc.w	0
transy:	dc.w	0
transz:	dc.w	0

angle:	dc.l	0
anglex:	dc.l	0
angley:	dc.l	0
anglez:	dc.l	0

wel_inc:	dc.w	5

welke_z:	dc.w	0
welke_sprite:	dc.w	0

xo:	dc.w	ixo
yo:	dc.w	iyo
zo:	dc.w	izo
modulus:	dc.w	0
mod_inc:	dc.w	1
tempo_angle:	dc.l	0
tempo_x:	dc.l	0
tempo_y:	dc.l	0
tijdteller:	dc.l	0
change_teller:	dc.l	20
anglexinc:	dc.l	1,2,3,0

cirhoek:	dc.l	0
sinusangle:	dc.l	0
sinusangle2:	dc.l	0
aantalpoints:	dc.l	0
mem_oppoints:	dc.l	0
oppoints:	dc.l	0
opwaarpoints:	dc.l	0
welk_patroon:	dc.l	0

currentpatroon:	dc.l	0

patroonlijst:	dc.l	patroon9,patroon8,patroon6,patroon7
		dc.l	patroon1,patroon2,patroon3,patroon4
		dc.l	patroon5,0

modulbit:	dc.b	1

patroon6:	dc.b	2,13,2,13,2,13,2,13
		dc.b	13,2,13,2,13,2,13,2
		dc.b	2,13,2,13,2,13,2,13
		dc.b	13,2,13,2,13,2,13,2
		dc.b	2,13,2,13,2,13,2,13
		dc.b	13,2,13,2,13,2,13,2
		dc.b	2,13,2,13,2,13,2,13
		dc.b	13,2,13,2,13,2,13,2

patroon7:	dc.b	1,1,1,1,1,1,1,1
		dc.b	1,2,2,2,2,2,2,1
		dc.b	1,2,3,3,3,3,2,1
		dc.b	1,2,3,4,4,3,2,1
		dc.b	1,2,3,4,4,3,2,1
		dc.b	1,2,3,3,3,3,2,1
		dc.b	1,2,2,2,2,2,2,1
		dc.b	1,1,1,1,1,1,1,1

patroon8:	dc.b	4,4,4,4,4,4,4,11
		dc.b	4,11,11,11,11,11,11,11
		dc.b	4,4,4,4,4,4,4,11
		dc.b	4,11,11,11,11,11,11,11
		dc.b	4,4,4,4,4,4,4,11
		dc.b	4,11,11,11,11,11,11,11
		dc.b	4,4,4,4,4,4,4,11
		dc.b	4,11,11,11,11,11,11,11

patroon9:	dc.b	1,1,1,0,0,2,2,2
		dc.b	1,1,1,0,0,2,2,2
		dc.b	1,1,1,0,0,2,2,2
		dc.b	0,0,0,14,13,0,0,0
		dc.b	0,0,0,12,11,0,0,0
		dc.b	3,3,3,0,0,4,4,4
		dc.b	3,3,3,0,0,4,4,4
		dc.b	3,3,3,0,0,4,4,4

patroon1:	dc.b	1,1,1,1,1,1,1,1
		dc.b	1,1,1,1,1,1,1,1
		dc.b	1,1,13,13,13,13,1,1
		dc.b	1,1,13,12,12,13,1,1
		dc.b	1,1,13,12,12,13,1,1
		dc.b	1,1,13,13,13,13,1,1
		dc.b	1,1,1,1,1,1,1,1
		dc.b	1,1,1,1,1,1,1,1

patroon2:	dc.b	1,1,1,1,1,1,1,1
		dc.b	1,14,14,14,14,14,14,1
		dc.b	1,14,1,1,1,1,14,1
		dc.b	1,14,1,1,1,1,14,1
		dc.b	1,14,1,1,1,1,14,1
		dc.b	1,14,1,1,1,1,14,1
		dc.b	1,14,14,14,14,14,14,1
		dc.b	1,1,1,1,1,1,1,1

patroon3:	dc.b	0,0,0,0,0,0,0,0
		dc.b	1,0,0,0,1,4,4,0
		dc.b	1,0,0,1,4,0,0,0
		dc.b	1,0,1,0,4,0,0,0
		dc.b	1,1,1,0,4,0,0,0
		dc.b	1,0,1,0,4,0,0,0
		dc.b	1,0,0,1,0,4,4,0	
		dc.b	0,0,0,0,0,0,0,0

patroon4:	dc.b	13,13,1,1,1,1,13,13
		dc.b	1,13,13,1,1,13,13,1
		dc.b	1,1,13,13,13,13,1,1
		dc.b	1,1,1,13,13,1,1,1
		dc.b	1,1,13,13,13,13,1,1
		dc.b	1,13,13,1,1,13,13,1
		dc.b	13,13,1,1,1,1,13,13
		dc.b	0,0,0,0,0,0,0,0

patroon5:	dc.b	1,1,1,1,11,11,11,11
		dc.b	1,1,1,1,11,11,11,11
		dc.b	1,1,1,1,11,11,11,11
		dc.b	1,1,1,1,11,11,11,11
		dc.b	11,11,11,11,1,1,1,1
		dc.b	11,11,11,11,1,1,1,1
		dc.b	11,11,11,11,1,1,1,1	
		dc.b	11,11,11,11,1,1,1,1

	even

maakpatroon:
	movem.l	a3,-(a7)

	move.l	waar_patroon,a0
	move.l	a0,a5
	add.l	#extra_mem,a5
	sub.l	#4000,a5
	clr.l	d5

	lea.l	tekst,a1
	lea.l	letters,a2
replet:
	cmp.l	a0,a5
	bls.L	finito2

	clr.l	d0
	move.b	(a1)+,d0
	beq.S	finito
	cmp.b	#" ",d0
	bne.S	no_spatie
	add.l	#6*5,a0

	bra.S	replet
no_spatie:
	add.l	#1,d5
	cmpi.l	#4,d5
	bls.S	mnopo
	moveq	#1,d5
mnopo:
	sub.l	#"a",d0
	lsl.l	#3,d0
	lea.l	0(a2,d0.w),a3
	move.l	(a3),a4
	move.l	4(a3),d7
	sub.l	#1,d7
repcopy:
	move.l	#5,d6
repcopy4:
	tst.b	(a4)+
	beq.S	niets2
	move.b	d5,(a0)+
	bra.S	niets3
niets2:
	move.b	#0,(a0)+
niets3:
	dbf	d6,repcopy4

*	move.w	(a4)+,(a0)+
*	move.w	(a4)+,(a0)+
*	move.w	(a4)+,(a0)+

	dbf	d7,repcopy
	add.l	#6,a0
	bra.S	replet	
	
finito:	
	move.l	a0,eind_patroon

	move.l	#breedte,d3
	lea.l	tekst,a1
	lea.l	letters,a2
replet2:
	cmp.l	a0,a5
	bls.S	finito2

	clr.l	d0
	move.b	(a1)+,d0
	beq.S	finito2
	cmp.b	#" ",d0
	bne.S	no_spatie2
	add.l	#6*5,a0
	bra.S	replet2
no_spatie2:
	add.l	#1,d5
	cmpi.l	#4,d5
	bls.S	mnopo2
	moveq	#1,d5
mnopo2:

	sub.l	#"a",d0
	lsl.l	#3,d0
	lea.l	0(a2,d0.w),a3
	move.l	(a3),a4
	move.l	4(a3),d7
	sub.l	#1,d7
repcopy2:

	move.l	#5,d6
repcopy5:
	tst.b	(a4)+
	beq.S	niets4
	move.b	d5,(a0)+
	bra.S	niets5
niets4:
	move.b	#0,(a0)+
niets5:
	dbf	d6,repcopy5

	sub.l	#1,d3
	bmi.S	finito2
	dbf	d7,repcopy2
	add.l	#6,a0
	bra.S	replet2
	
finito2:	
	movem.l	(a7)+,a3
	rts


sinit_points:
	move.l	sinusangle,d4
	move.l	aantal_y,d7
	move.l	start_y,d1
	move.l	welk_patroon,a4

srepy:	move.l	aantal_x,d6
	move.l	start_x,d0
	move.l	d4,d3
srepx:	

	move.w	d0,(a0)
	add.l	#1,d3
	cmpi.l	#359,d3
	bls.S	snopo_sin2
	sub.l	#360,d3
snopo_sin2:
	tst.b	(a4)
	beq.S	sno_patr
	lea.l	2(a0),a0
	move.l	d3,d2
	lsl.l	#1,d2
	lea.l	sinustabel,a1
	move.w	0(a1,d2.l),d2
	mulu	modulus,d2
	asr.w	#acur,d2
	move.w	d1,(a0)+
	cmpi.b	#10,(a4)
	bls.S	sno_zadd
	add.w	welke_z,d2
sno_zadd:
	move.w	d2,(a0)+

*	move.w	welke_z,(a0)+

	moveq	#0,d2
	move.b	(a4),d2
	cmpi.b	#10,d2
	bls.S	sno_po10
	sub.b	#10,d2
sno_po10:
	sub.b	#1,d2
	move.w	d2,(a0)+
	add.l	#1,aantalpoints
sno_patr:
	add.w	#25,d0
	lea.l	1(a4),a4
	dbf	d6,srepx
	add.w	#20,d1
	add.l	#20,d4
	dbf	d7,srepy
	move.w	#1000,(a0)+
	move.w	#1000,(a0)+
	move.w	#1000,(a0)+
	move.w	#1000,(a0)+
	rts

scroll_teller:	dc.l	0
waar_patroon:	dc.l	0
eind_patroon:	dc.l	0

tekst:		dc.b	"    hallo dit is een test van bolletjes"
		dc.b	" scroll"
		dc.b	" als je dit goed wil lezen druk dan rechter"
		dc.b	" muisknop in",0

	even
letters:	dc.l	leta,3
		dc.l	letb,3
		dc.l	letc,3
		dc.l	letd,3
		dc.l	lete,3
		dc.l	letf,3
		dc.l	letg,3
		dc.l	leth,3
		dc.l	leti,1
		dc.l	letj,3
		dc.l	letk,3
		dc.l	letl,3
		dc.l	letm,5
		dc.l	letn,3
		dc.l	leto,3
		dc.l	letp,3
		dc.l	letq,3
		dc.l	letr,3
		dc.l	lets,3
		dc.l	lett,3
		dc.l	letu,3
		dc.l	letv,5
		dc.l	letw,5
		dc.l	letx,3
		dc.l	lety,3
		dc.l	letz,4
	

leta:
	dc.b	1,1,1,1,0,0
	dc.b	0,1,0,0,1,0
	dc.b	1,1,1,1,0,0

letb:
	dc.b	1,1,1,1,1,0
	dc.b	1,0,1,0,1,0
	dc.b	1,1,1,1,0,0

letc:
	dc.b	1,1,1,1,1,0
	dc.b	1,0,0,0,1,0
	dc.b	1,0,0,0,1,0

letd:	dc.b	1,1,1,1,1,0
	dc.b	1,0,0,0,1,0
	dc.b	1,1,1,1,0,0

lete:	dc.b	1,1,1,1,1,0
	dc.b	1,0,1,0,1,0
	dc.b	1,0,1,0,1,0

letf:	dc.b	1,1,1,1,1,0
	dc.b	0,0,1,0,1,0
	dc.b	0,0,0,0,1,0

letg:	dc.b	1,0,0,1,1,0
	dc.b	1,0,1,0,1,0
	dc.b	1,1,1,1,1,0

leth:	dc.b	1,1,1,1,1,0
	dc.b	0,0,1,0,0,0
	dc.b	1,1,1,1,1,0

leti:	dc.b	1,1,1,1,0,1

letj:	dc.b	0,1,0,0,0,0
	dc.b	1,0,0,0,0,0
	dc.b	0,1,1,1,0,1

letk:	dc.b	1,1,1,1,1,0
	dc.b	0,1,0,1,0,0
	dc.b	1,0,0,0,1,0

letl:	dc.b	1,1,1,1,1,0
	dc.b	1,0,0,0,0,0
	dc.b	1,0,0,0,0,0

letm:	dc.b	1,1,1,1,1,0
	dc.b	0,0,0,1,0,0
	dc.b	0,0,1,0,0,0
	dc.b	0,0,0,1,0,0
	dc.b	1,1,1,1,1,0

letn:	dc.b	1,1,1,1,1,0
	dc.b	0,0,0,0,1,0
	dc.b	1,1,1,1,0,0

leto:	dc.b	1,1,1,1,1,0
	dc.b	1,0,0,0,1,0
	dc.b	1,1,1,1,1,0

letp:	dc.b	1,1,1,1,1,0
	dc.b	0,0,1,0,1,0
	dc.b	0,0,1,1,1,0

letq:	dc.b	0,0,1,1,1,0
	dc.b	0,0,1,0,1,0
	dc.b	1,1,1,1,1,0

letr:	dc.b	1,1,1,1,1,0
	dc.b	0,1,1,0,1,0
	dc.b	1,0,1,1,1,0

lets:	dc.b	1,0,1,1,1,0
	dc.b	1,0,1,0,1,0
	dc.b	1,1,1,0,1,0

lett:	dc.b	0,0,0,0,1,0
	dc.b	1,1,1,1,1,0
	dc.b	0,0,0,0,1,0

letu:	dc.b	1,1,1,1,1,0
	dc.b	1,0,0,0,0,0
	dc.b	1,1,1,1,1,0

letv:	dc.b	0,0,0,1,1,0
	dc.b	0,1,1,0,0,0
	dc.b	1,0,0,0,0,0
	dc.b	0,1,1,0,0,0
	dc.b	0,0,0,1,1,0

letw:	dc.b	1,1,1,1,1,0
	dc.b	0,1,0,0,0,0
	dc.b	0,0,1,0,0,0
	dc.b	0,1,0,0,0,0
	dc.b	1,1,1,1,1,0

letx:	dc.b	1,1,0,1,1,0
	dc.b	0,1,1,1,0,0
	dc.b	1,1,0,1,1,0

lety:	dc.b	0,0,0,1,1,0
	dc.b	1,1,1,0,0,0
	dc.b	0,0,0,1,1,0

letz:	dc.b	1,0,0,0,1,0
	dc.b	1,0,1,1,1,0
	dc.b	1,1,1,0,1,0
	dc.b	1,0,0,0,1,0

colors:

	dc.b	$00,$00,$88
	dc.b	$B0,$B0,$B0
	dc.b	$98,$98,$98
	dc.b	$80,$80,$80
	dc.b	$68,$68,$68
	dc.b	$50,$50,$50
	dc.b	$38,$38,$38
	dc.b	$20,$20,$20
	dc.b	$00,$B0,$60
	dc.b	$00,$D0,$D0
	dc.b	$00,$A0,$F0
	dc.b	$00,$70,$C0
	dc.b	$00,$00,$F0
	dc.b	$70,$00,$F0
	dc.b	$C0,$00,$E0
	dc.b	$C0,$00,$80
	dc.b	$60,$20,$00
	dc.b	$E0,$50,$20
	dc.b	$A0,$50,$20
	dc.b	$F0,$C0,$A0
	dc.b	$30,$30,$30
	dc.b	$40,$40,$40
	dc.b	$50,$50,$50
	dc.b	$60,$60,$60
	dc.b	$70,$70,$70
	dc.b	$80,$80,$80
	dc.b	$90,$90,$90
	dc.b	$A0,$A0,$A0
	dc.b	$C0,$C0,$C0
	dc.b	$D0,$D0,$D0
	dc.b	$E0,$E0,$E0
	dc.b	$F0,$F0,$F0

;tbob:	blk.b	bi_SIZEOF,0
point_store_temp1:	blk.l	20000,0
point_store_temp2:	blk.l	20000,0
patje:			blk.l	10000,0

*
* Mean and Lean IFF reader
* to be used in other programs
*
* C. Lieshout 18-04-1994
*

	INCDIR	"include:"
	INCLUDE	"exec/memory.i"

	RSRESET
iff_struct:	rs.w	0
iff_error:	rs.w	1
iff_width:	rs.w	1
iff_height:	rs.w	1
iff_planes:	rs.w	1
iff_mode:	rs.w	1
iff_colors:	rs.l	1
iff_image:	rs.l	1
iff_mask:	rs.l	1

iff_width_bytes:rs.l	1
iff_blitheight:	rs.l	1
iff_SIZEOF:	rs.w	0

	RSRESET
iloc_struct:		rs.w	0
iloc_filename:		rs.l	1
iloc_filehandle:	rs.l	1
iloc_deststruct:	rs.l	1
iloc_memtype:		rs.l	1
iloc_dosbase:		rs.l	1
iloc_body_start:	rs.l	1

iloc_lenx:		rs.w	1
iloc_leny:		rs.w	1
iloc_planes:		rs.w	1
iloc_mode:		rs.w	1

iloc_packed:		rs.l	1
iloc_packed_size:	rs.l	1
iloc_decrunched:	rs.l	1
iloc_decrunched_size:	rs.l	1
iloc_colors:		rs.l	1
iloc_colpointer:	rs.l	1

iloc_bmhd_found:	rs.b	1
iloc_body_found:	rs.b	1
iloc_compression:	rs.b	1
iloc_masking:		rs.b	1

iloc_buf:		rs.l	2
iloc_SIZEOF:		rs.w	0

mode_old = 1005

load_bob1:
	lea	bob1struct(pc),a1
	lea	bob1name(pc),a0
	move.l	#MEMF_CHIP,d0
	bsr	get_iff
	rts

load_background:
	lea	background(pc),a1
	lea	filename(pc),a0
	move.l	#MEMF_FAST,d0
	bsr	get_iff
	lea	background(pc),a1
	move.l	iff_image(a1),back_image
	rts

free_bob1:
	lea	bob1struct(pc),a1
	bra	free_load

free_background:
	lea	background(pc),a1
free_load:
	move.l	a1,a4
	move.w	iff_width(a1),d0
	add.l	#15,d0
	lsr.l	#4,d0			; words
	add.l	d0,d0			; bytes
	mulu	iff_planes(a1),d0
	mulu	iff_height(a1),d0
	move.l	iff_image(a1),a1
	move.l	4.w,a6
	jsr	_LVOFreeMem(a6)
	move.l	a4,a1
	tst.l	iff_colors(a1)
	beq	.noc
	move.w	iff_planes(a1),d1
	moveq	#1,d0
	lsl.l	d1,d0
	move.l	d0,d1
	add.l	d0,d0
	add.l	d1,d0
	move.l	iff_colors(a1),a1
	move.l	4.w,a6
	jsr	_LVOFreeMem(a6)
.noc:
	move.l	a4,a1
	tst.l	iff_mask(a1)
	beq	.no_mask
	move.l	iff_width_bytes(a1),d0
	addq.w	#2,d0				; add two for mask
	mulu	iff_planes(a1),d0
	mulu	iff_height(a1),d0		; mask size
	move.l	iff_mask(a1),a1
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)
.no_mask:
	rts
*
* decrunch iff file a0, in struct a1, in memory type d0
*
get_iff:
	link	a5,#-iloc_SIZEOF
	movem.l	d1-d7/a0-a6,-(a7)
	sub.l	#iloc_SIZEOF,a5

	move.l	a0,iloc_filename(a5)
	move.l	a1,iloc_deststruct(a5)
	move.l	d0,iloc_memtype(a5)
	clr.l	iff_mask(a1)
	clr.l	iff_colors(a1)

	bsr.w	loadfile
	bne	.err

	bsr	unpack
	bne	.err1

	move.l	iloc_deststruct(a5),a0
	move.w	#0,iff_error(a0)
	move.w	iloc_lenx(a5),iff_width(a0)
	move.w	iloc_leny(a5),iff_height(a0)
	move.w	iloc_planes(a5),iff_planes(a0)
	move.w	iloc_mode(a5),iff_mode(a0)
	move.l	iloc_colpointer(a5),iff_colors(a0)
	move.l	iloc_decrunched(a5),iff_image(a0)

	move.l	iloc_packed(a5),a1
	move.l	iloc_packed_size(a5),d0
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)
.exit
	movem.l	(a7)+,d1-d7/a0-a6
	unlk	a5
	rts

.err1:
	tst.l	iloc_colpointer(a5)
	beq	.err
	move.l	iloc_colors(a5),d1
	move.l	d1,d0
	add.l	d0,d0
	add.l	d1,d0
	move.l	iloc_colpointer(a5),a1
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)

.err:
	move.l	iloc_deststruct(a5),a0
	move.w	#1,iff_error(a0)
	bra.b	.exit

loadfile:
	move.l	$4.w,a6
	lea	.dosname(pc),a1
	moveq	#0,d0
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,iloc_dosbase(a5)
	
	move.b	#0,iloc_bmhd_found(a5)
	move.b	#0,iloc_body_found(a5)

	clr.l	iloc_packed(a5)
	bsr.w	read_whole_file
	bne	.err

	clr.l	iloc_colpointer(a5)
	bsr.w	check_chunks
	cmp.b	#$ff,iloc_bmhd_found(a5)
	bne.w	.err
	cmp.b	#$ff,iloc_body_found(a5)
	bne.w	.err
	bsr	closedos
	moveq	#0,d0
	rts
.err:
	bsr	closedos
	moveq	#-1,d0
	rts
.dosname:	dc.b	"dos.library",0

closedos:
	move.l	iloc_dosbase(a5),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	rts

unpack:	
	tst.l	iloc_decrunched(a5)
	beq	.err
	move.l	iloc_body_start(a5),a0
	move.l	iloc_decrunched(a5),a1

	cmp.b	#1,iloc_compression(a5)
	beq.s	.byte_run

* compression geen 1 dus of 0 of een onbekende 
* copieer van body_start naar unpacked

	move.l	iloc_decrunched_size(a5),d0
	lsr.l	#1,d0
	subq.l	#1,d0
	move.l	iloc_body_start(a5),a0
.copy:	move.w	(a0)+,(a1)+
	subq.l	#1,d0
	bpl.b	.copy
	moveq	#0,d0
	rts

.byte_run:
	move.l	iloc_decrunched(a5),a6
	add.l	iloc_decrunched_size(a5),a6
.un_again:	
	cmp.l	a6,a1
	bge.s	.un_end

	moveq	#0,d5
	move.b	(a0)+,d5
	bmi.b	.un_minus

.un_plus:
	move.b	(a0)+,(a1)+
	dbf	d5,.un_plus
	bra.s	.un_again

.un_minus:
	neg.b	d5
	move.b	(a0)+,d0

.un_rm:	move.b	d0,(a1)+
	dbf	d5,.un_rm

	bra.s	.un_again
.un_end:	
	moveq	#0,d0
	rts
.err:
	moveq	#-1,d0
	rts
*
*
*
check_chunks:
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	iloc_packed(a5),a1
	move.l	4(a1),d0		; file size
	lea	8(a1),a1
	move.l	a1,a2
	addq.l	#4,a1			; skip het ILBM sign
	add.l	d0,a2			; einde data
.cchunks1:
	cmp.l	#"BMHD",(a1)
	bne.w	.no_bmhd_proc
	move.b	#$ff,iloc_bmhd_found(a5)

	lea	8(a1),a6
	move.w	(a6),iloc_lenx(a5)
	move.w	2(a6),iloc_leny(a5)
	moveq	#0,d1
	move.b	8(a6),d1
	move.w	d1,iloc_planes(a5)
	move.b	10(a6),iloc_compression(a5)
	move.b	9(a6),iloc_masking(a5)

	move.l	a1,-(a7)
	bsr.w	unpack_init
	move.l	(a7)+,a1

	bra.w	.continue_chunks

.no_bmhd_proc:
	cmp.l	#"CAMG",(a1)
	bne.b	.no_camg_proc	

	move.w	10(a1),d0
	and.w	#$8efd,d0		; oldstyle mask
	move.w	d0,iloc_mode(a5)

.no_camg_proc:
	cmp.l	#"CMAP",(a1)
	bne.b	.no_cmap_proc

	lea	4(a1),a6


	movem.l	a0/a1,-(a7)

	move.l	(a6)+,d1
	move.w	iloc_planes(a5),d0
	beq	.col_ready
	moveq	#1,d1
	lsl.l	d0,d1
	move.l	d1,iloc_colors(a5)
	move.l	d1,d0
	add.l	d0,d0
	add.l	d1,d0

	movem.l	a6,-(a7)
	move.l	#MEMF_PUBLIC,d1	
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,iloc_colpointer(a5)
	movem.l	(a7)+,a6
	beq	.col_ready

	move.l	iloc_colors(a5),d1
	move.l	d0,a0
.cmap4:
	move.b	(a6)+,(a0)+
	move.b	(a6)+,(a0)+
	move.b	(a6)+,(a0)+
	subq.w	#1,d1
	bne.b	.cmap4

.col_ready:
	movem.l	(a7)+,a0/a1
	bra.b	.continue_chunks

.no_cmap_proc:

	cmp.l	#"BODY",(a1)
	bne.b	.no_body_proc
	move.b	#$ff,iloc_body_found(a5)
	lea	8(a1),a6
	move.l	a6,iloc_body_start(a5)
	bra.w	.continue_chunks

.no_body_proc:
.continue_chunks:
	add.l	4(a1),a1
	move.l	a1,d0
	btst	#0,d0
	beq.b	.no_odd
	addq.l	#1,a1
.no_odd:
	addq.l	#8,a1
	cmp.l	a2,a1
	blt.w	.cchunks1
	move.l	#-1,a1
.cchunks2:
	movem.l	(a7)+,d0-d7/a0-a6
	rts

unpack_init:
	moveq	#0,d0
	move.w	iloc_lenx(a5),d0
	add.l	#15,d0
	lsr.l	#4,d0			; words
	add.l	d0,d0			; bytes
	mulu	iloc_planes(a5),d0
	mulu	iloc_leny(a5),d0
	move.l	d0,iloc_decrunched_size(a5)
	move.l	4,a6
	move.l	iloc_memtype(a5),d1
	jsr	_LVOAllocMem(a6)
	move.l	d0,iloc_decrunched(a5)
	rts

read_whole_file:
	move.l	iloc_filename(a5),d1
	move.l	#mode_old,d2
	move.l	iloc_dosbase(a5),a6
	jsr	_LVOOpen(a6)
	move.l	d0,iloc_filehandle(a5)
	beq.w	.err_readfile

	move.l	d0,d1

	lea.l	iloc_buf(a5),a2
	move.l	a2,d2
	moveq	#8,d3
	move.l	iloc_dosbase(a5),a6
	jsr	_LVORead(a6)
	lea.l	iloc_buf(a5),a1
	moveq	#1,d7
	cmp.l	#'FORM',(a1)+
	bne.b	.err_readfile_open
	move.l	(a1),d0
	add.l	#8,d0
	move.l	d0,iloc_packed_size(a5)

	move.l	#$10000,d1
	move.l	4,a6
	jsr	_LVOAllocMem(a6)

	move.l	d0,iloc_packed(a5)
	beq.b	.err_readfile_open
	move.l	d0,a0

	move.l	iloc_filehandle(a5),d1
	moveq	#0,d2				; seek beginning
	moveq	#-1,d3
	move.l	iloc_dosbase(a5),a6
	jsr	_LVOSeek(a6)		

	move.l	iloc_packed_size(a5),d3
	move.l	iloc_packed(a5),d2
	move.l	iloc_filehandle(a5),d1
	move.l	iloc_dosbase(a5),a6
	jsr	_LVORead(a6)		
	bsr.b	close_file
	moveq	#0,d0
	rts

.err_readfile:
	moveq	#-1,d0
	rts

.err_readfile_open:
	bsr.b	close_file
	bra	.err_readfile

close_file:
	move.l	iloc_filehandle(a5),d1
	move.l	iloc_dosbase(a5),a6
	jsr	_LVOClose(a6)
	rts

back_image:	dc.l	0

bob1struct:	blk.b	iff_SIZEOF,0
background:	blk.b	iff_SIZEOF,0
filename:	dc.b	"work:kladwerk/jos_lowres",0
bob1name:	dc.b	"work:t/sphere_5pl2",0

	END
