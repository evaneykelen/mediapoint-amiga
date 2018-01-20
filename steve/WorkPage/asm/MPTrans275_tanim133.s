* File	: MLTrans.s
* Uses	: databloknew.i mlmmu_lib.i viewbloknew.i system.i proces.i
*	: screen_modes.i parse_structs.i
*
* Date	: Feb 1993
* Author: ing. C. Lieshout
* Desc	: Create different screen effects
*	: Convert screens to match
*
*
* PC relative coded 3-11-92
*
* Hires - lowres , lowres - hires conversion
* 7 september 1992
*
* IFF reader by Cees Lieshout 14-april-1990
*

XAPP = 1
BIT24 = 1
NEWEFF = 0
NOSTAY = 1

VIEWANIM = 0
VIEWINFO = 0
GFXSNOOP = 0
DEBUG = 0
PRAND = 0
ESCLOOK = 0
NEWFLIP = 0
;TESTV_LACE = 0

INIT_COPLIST_SIZE = 250000

MEMF_STAY = $8000
MTF_INIT = 1
MTF_SETCLR = $80000000

CUT_WITHOUT_ERASE_NUM = 72

LIBV39 = 39
LIBV36 = 36
TRUE = 1
FALSE =0

	IFNE	DEBUG!VIEWINFO!PRAND!GFXSNOOP!ESCLOOK!VIEWANIM
	XREF	KPutFmt
	XREF	KPutStr
	PRINTT	"De debug info staat aan dus je moet nog met debug.lib linken"
	ENDC

	INCDIR "include:"
	INCLUDE	"devices/timer.i"
	INCLUDE "exec/execbase.i"
	INCLUDE "exec/funcdef.i"
	INCLUDE "exec/exec_lib.i"
	INCLUDE "exec/ports.i"
	INCLUDE	"exec/lists.i"
	INCLUDE	"exec/libraries.i"
	INCLUDE	"exec/memory.i"
	INCLUDE	"exec/tasks.i"
	INCLUDE	"exec/semaphores.i"
 	INCLUDE "graphics/gfxbase.i"
	INCLUDE "graphics/displayinfo.i"
	INCLUDE	"dos/dos.i"
	INCLUDE "graphics/view.i"
	INCLUDE "graphics/rastport.i"
	INCLUDE "graphics/gfx.i"
 	INCLUDE "graphics/sprite.i"
	INCLUDE "graphics/copper.i"
	INCLUDE	"graphics/layers.i"
	INCLUDE	"graphics/clip.i"
	INCLUDE	"graphics/videocontrol.i"
	INCLUDE "hardware/custom.i"
	INCLUDE	"intuition/intuitionbase.i"
	INCLUDE "intuition/preferences.i"
	INCDIR	"wa:asm/"
	INCLUDE "anhd.i"
	INCLUDE	"anim_infonew.i"
	
	INCDIR	"wp:asm/"
	INCLUDE	"anim/aw_data.i"
	INCLUDE "viewbloknew.i"
	INCLUDE	"system.i"
	INCLUDE	"newdatabloknew.i"
	INCLUDE "proces.i"
	INCLUDE	"screenmodes.i"
	INCLUDE	"parse_structs.i"
	INCLUDE	"copplay.i"
	INCLUDE	"errors.i"
	
	INCDIR "wp:inclibs/"

	INCLUDE "mpmmu_lib.i"
	INCLUDE "dos_lib.i"
	INCLUDE "mathffp_lib.i"
	INCLUDE "mathtrans_lib.i"
	INCLUDE "graphics_libv39.i"
	INCLUDE "layers_lib.i"
	INCLUDE "intuition_lib.i"

	INCDIR	"pascal:include/"
	INCLUDE	"txed.i"
	INCLUDE	"ttxt.i"
	INCLUDE	"editwindow.i"

;MON_ID = DEFAULT_MONITOR_ID!HIRESLACE_KEY
MON_ID = DBLPAL_MONITOR_ID   ;+HIRESLACE_KEY
;MON_ID = PAL_MONITOR_ID   ;+HIRESLACE_KEY
;MON_ID = NTSC_MONITOR_ID   ;+HIRESLACE_KEY
;MON_ID = VGA_MONITOR_ID

;MON_ID = $39004

mode_old = 1005

;MAX_SIZE_Y = 566

ROLSIZE = 10
MEMSIZE = 527000
;MEMSIZE = 222720
;MEMSIZE = 166000

CHIPSCRAPSIZE = 10000		; ???????????????

PATROONSIZE 	= 8000
PTABELSIZE 	= 400		; voor 100 patronen
STOREMOVESIZE 	= 4000

COL1SIZE	= 3*256*2	; 1536
COL2SIZE	= 3*256*2	; 1536
DCOL1SIZE	= 3*256*2	; 1536
DCOL2SIZE	= 3*256*2	; 1536

COL32SIZE	= 3*257*8	; 3084

VIEWBLOK1SIZE	= 1500		;vb_SIZEOF	; 1366	????
VIEWBLOK2SIZE	= 1500		;vb_SIZEOF	; 1366

;		+ ------

TFAST = PATROONSIZE+PTABELSIZE+STOREMOVESIZE+COL1SIZE*4
FASTSCRAPSIZE =	TFAST+COL32SIZE+VIEWBLOK1SIZE*2 + 1000	; safety ????

;FASTSCRAPSIZE 	= 25000		; ???????????????

MAXLINESIZE = 1280		; bytes
	IFNE NEWEFF
NUM_OFFWIPES = 92
	ELSE
NUM_OFFWIPES = 101
	ENDC

NUM_LINEWIPES = 82

PATHEND = -10000		; terminatie voor path plot

pstart:
	IFEQ	XAPP

	IFEQ	DEBUG
start:
	ENDC
	move.w	$dff002,d0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	#vb_SIZEOF,d0

	bsr	init_jump_tabel

	lea	startdatablock,a3
	move.l	#0,aw_waitmask(a3)

	move.l	a7,db_easy_exit(a3)

	bsr	openlibs

	move.l	#MEMF_STAY+2,d1
	move.l	db_mlmmubase(a3),a6
	jsr	_LVOMLMMU_FlushMem(a6)

	bsr	request_memory
	move.l	#MON_ID,db_monid(a3)
;	bsr	filter_monitor_id

	jsr	create_hi_to_low_tabel
	jsr	create_low_to_high_tabel

	lea	eff_nums2,a0
	move.l	a0,db_eff_nums(a3)
	jsr	setprefs

	bsr	filter_monitor_id

	bsr	init_datablock

	lea	temp_tags(pc),a0		; set tag_list pointer
	move.l	a0,db_tags(a3)

	bsr	set_view_structs

	bsr	init_cop_vports

	move.l	db_active_viewblok(a3),a4
	move.l	db_unpacked1(a3),vb_tempbuffer(a4)
;	addq.l	#4,vb_tempbuffer(a4)			; safety voor negblit

	move.l	db_inactive_viewblok(a3),a4
	move.l	db_unpacked2(a3),vb_tempbuffer(a4)
;	addq.l	#4,vb_tempbuffer(a4)

;	move.l	db_filepointersource(a3),a0	; initial file inladen
;	bsr.w	laadfile
;	bsr	showpicture		; inactive -> active

	move.l	db_active_viewblok(a3),a5
	move.w	#10,vb_lenx(a5)
	move.w	#10,vb_leny(a5)
	move.l	#2,vb_breedte_x(a5)
	move.w	#1,vb_planes(a5)
	move.l	#3,vb_color_count(a5)
	move.l	#2*10,vb_unpacked_size(a5)

	move.l	db_waarcolors32(a3),a0
	move.l	#'CEES',3*257*4(a0)

	jsr	setsam
	moveq	#ERR_GENERAL,d7
	tst.b	d0
	bne	sexit	
	jsr	install_50h
	tst.l	db_monitorspec(a3)
	beq	sexit

	move.b	#0,db_cyc_quit(a3)
	lea	sigmask,a0
	move.l	(a0),db_waitmasker(a3)
;	move.w	#$8400,$dff096

	jsr	do_slide		; voer internal slide uit

;	move.w	#$0400,$dff096

	move.l	#MEMF_STAY+2,d1
	move.l	db_mlmmubase(a3),a6
	jsr	_LVOMLMMU_FlushMem(a6)	; flush the mpmmu library

	moveq	#0,d7
	bra.w	sexit

	rts
	ENDC

	IFNE 0
cop_cop:
	move.l		db_active_viewblok(a3),a5
	move.l		vb_vieww(a5),a2
	move.l		v_LOFCprList(a2),a0
	move.l		v_SHFCprList(a2),a1

	cmp.l	#0,a1
	beq	exit_cc

	move.l		crl_Next(a1),a2
	move.w		crl_MaxCount(a0),d2		; aantal elementen
	add.w	d2,d2
	move.l		crl_start(a0),a0		; eigenlijke copperlist
	move.l		crl_start(a1),a1
	lea		testcop(pc),a2

	moveq	#0,d7
	subq.l		#1,d2
rep_copcop2:
	move.w		(a0)+,d0
	move.w	d0,(a2)+
	cmp.w		#$ffff,d0
	beq		exit_copcopy2
	move.w		(a1)+,d1
	cmp.w	d0,d1
	beq	no_diver1
	addq.l	#1,d7
no_diver1:
	dbf		d2,rep_copcop2
exit_copcopy2:
exit_cc:
	rts
testcop:	ds.l	500
	ENDC
	
init_datablock:
	move.w	#$09f0,db_maskbits(a3)
	move.w	#$0fca,db_blitin_con0(a3)

	move.l	db_waarviewblok1(a3),db_active_viewblok(a3)
	move.l	db_waarviewblok2(a3),db_inactive_viewblok(a3)

	lea	db_fileblok1(a3),a0
	move.l	a0,db_inactive_fileblok(a3)
	rts

	IFEQ XAPP
test_animclip:
	dc.l	animname,0,0,100,100,10,-1,3
	dc.l	-1,12,4,-1,10,0,0,0

test_clip:
	dc.l	cname1,0,0,124,82,0,0,0
	dc.l	-1,6,1,-1,10,0,0,0

;test_clip2:	dc.l	cname2,-192,-128,165,165

test_clip2:	dc.l	cname2,0,0,124,82,0,0,0
; effect
		dc.l	-1,6,1,-1,10,0,0,0

cname1:
	dc.b	"Work:MediaPoint/scripts/Pages/Clips/NewEffects21-1",0
;	dc.b	"HARD:Graphics/tekstje",0

cname2:
	dc.b	"Work:MediaPoint/scripts/Pages/Clips/NewEffects21-2",0
	even
*
* Pretend a document parse
*
test_document:
	lea	test_screen(pc),a0
	bsr	create_screen
	tst.w	d0
	bne	tijd

	bsr	test_set_colors

	lea	test_window(pc),a0
	bsr	create_window

;	lea	test_clip(pc),a0
;	bsr	create_clip

	lea	test_window2(pc),a0
	bsr	create_window

;	lea	test_clip2(pc),a0
;	bsr	create_clip

;	lea	test_animclip(pc),a0
;	bsr	create_animclip

;	lea	test_window3(pc),a0
;	bsr	create_window

pp:
	bsr	finish_doc
tijd:
	rts

	ENDC

	XDEF	_get_varsize
_get_varsize:
	move.l	#db_SIZEOF,d0
	rts

	XDEF _update_screen
_update_screen:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a3
	bsr	finish_doc
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

	XDEF	_finish_doc
_finish_doc:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a3
	bsr	finish_doc

;	bsr	set_viewed

	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)

	bsr	remove_50h
	bsr	freesam
	
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

*
* Finish screen and window wipes
*
finish_doc:
	tst.b	db_screen_wiped(a3)
	bne	screen_wiped1

	bsr	wipe_screen

screen_wiped1:
	tst.b	db_quit(a3)
	bne	no_wintr

	tst.b	db_wintriple(a3)		; is there a window ?
	beq	no_wintr1
	move.l	db_window_data(a3),a0
no_quit12:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Wipe in window",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	bsr	wipe_in_window

no_wintr1:
	tst.b	db_cliptriple(a3)		; is there a clip ?
	beq	no_cliptr
	bsr	wipe_clip_in3
no_cliptr:
no_wintr:
	bsr	clear_fastmask
	move.b	#0,db_quit(a3)
	rts

*
* Clear the bitmap data pointed at by a0
*
clear_bitmap:
	movem.l	d0/d1,-(a7)
	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)
	moveq	#0,d0
	move.w	bm_BytesPerRow(a0),d0
	mulu	bm_Rows(a0),d0
	move.l	d0,d1
	and.w	#$fffc,d0
	sub.l	d0,d1
	lsr.l	#2,d0
	
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Clear bitmap function",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC
	move.l	bm_Planes(a0),a0
.clr_bitm:
;	move.l	#$ff00ff00,(a0)+
	clr.l	(a0)+
	subq.l	#1,d0
	bne	.clr_bitm
	tst.w	d1
	beq	.noew
	clr.w	(a0)
.noew:
	movem.l	(a7)+,d0/d1
	rts

	XDEF	_create_text
_create_text:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	12(a5),a3
	move.l	8(a5),a0			; pointer to parameters
	move.l	16(a5),db_edit_window(a3)
	move.l	a7,db_easy_exit(a3)
	move.l	a0,db_text_data(a3)
	bsr	create_text
	bra	ret_err
*
*
*
create_text:
;
; First check any pending window, clip effects
;
	tst.b	db_quit(a3)
	beq	no_quit11t
	rts

no_quit11t:
	tst.b	db_skip_cut_woe(a3)
	beq	.no_skip
	moveq	#0,d0
	rts
.no_skip:

	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)

*
* The text must be placed in the screen before wipe in
* First create text and then place in the screen
*
	move.l	a0,db_text_data(a3)
	move.l	textp_eff+eff_innum(a0),d0
	move.w	d0,db_text_wipe(a3)

	cmp.w	#-1,db_text_wipe(a3)
	beq	no_text2

	tst.b	db_wintriple(a3)
	beq	no_text2
	move.l	a0,-(a7)

	bsr	wipe_in_window		; check the triple buffer for windows
	move.l	(a7)+,a0
no_text2:
	tst.b	db_quit(a3)
	beq	no_quit13t
	rts

no_quit13t:
	tst.b	db_cliptriple(a3)
	beq	no_text22
	move.l	a0,-(a7)
	bsr	wipe_clip_in3
	move.l	(a7)+,a0
no_text22:
	cmp.w	#-1,db_text_wipe(a3)
	beq	no_text3

	tst.b	db_screen_wiped(a3)
	bne	no_text3

	move.l	a0,-(a7)
	bsr	wipe_screen
	move.l	(a7)+,a0
no_text3:

* Everything is ready you can now create a text in the window or in the screen
* if there is a screen then place the data in the screen
* if there is a window then place the data in the window ( size )

	cmp.w	#-1,db_text_wipe(a3)		; need to put it in screen
	beq	wipe_text_in2			; or in the window

	tst.b	db_quit(a3)
	bne	wipe_text_in2			; need to put in window
	
	lea	screen_bitmap(pc),a0
	move.w	vb_planes(a5),d0		; depth
	move.w	db_screen_depth(a3),d0	
	move.l	db_window_data(a3),a1		; use window size
	move.l	winp_wx(a1),d1
	move.l	winp_wy(a1),d2
	move.l	db_triple_mem(a3),d3
	bsr	create_bitmap

; clear the bitmap

	lea	screen_bitmap(pc),a0
	bsr	clear_bitmap

wipe_text_in2:
	move.l	db_window_data(a3),a0
	move.l	winp_wx(a0),d0
	move.l	winp_wy(a0),d1

	move.l	winp_x(a0),d2
	move.l	winp_y(a0),d3
	moveq	#0,d2
	moveq	#0,d3

	moveq	#0,d6

	tst.b	db_screen_wiped(a3)
	bne	no_adjust2t
	
	move.w	db_winpos_x(a3),d6	; adjust with window screen position
	add.l	d6,d2
	move.w	db_winpos_y(a3),d6
	add.l	d6,d3
no_adjust2t:

; the bitmap with the text can be placed directly in the screen
; or the window

; check the window mode to see wether or not a mask is needed

	move.l	#$0c0,d6
	move.l	db_window_data(a3),a0

; d2,d3 give the coordinates in the window

	lea	ttdi(pc),a0
	move.w	#tdi_SIZE-1,d0			; clear the ttdi struct
.repp:
	move.b	#0,(a0)+
	dbf	d0,.repp

	lea	ttdi(pc),a0

	lea	screen_bitmap(pc),a1

	move.l	a1,tdi_dstBitMap(a0)

	move.l	db_inactive_viewblok(a3),a5

	move.l	vb_tempbuffer(a5),tdi_chipMem(a0)
	move.l	db_unpacked1_size(a3),tdi_chipSize(a0)

	move.l	$4.w,tdi_SysBase(a0)
	move.l	db_graphbase(a3),tdi_GfxBase(a0)

	move.w	d2,tdi_dstXPos(a0)

	move.l	db_edit_window(a3),a1
	move.w  ew_TopMargin(a1),tdi_dstYPos(a0)
	add.w	d3,tdi_dstYPos(a0)		; in case of in screen blit

	move.l	db_edit_window(a3),tdi_editWindow(a0)

	move.l	ew_teInfo(a1),a1
	move.l	tei_text(a1),tdi_textData(a0)
	
	move.l	db_active_viewblok(a3),a5
	move.l	vb_viewportw(a5),a1
	moveq	#0,d0
	move.w	vb_mode(a5),d0
	move.l	d0,tdi_viewModes(a0)

	lea	vb_colbytes(a5),a1
	tst.b	db_screen_wiped(a3)	; colormap from the active
	bne	.colmap_oke		; the page is on the screen
	lea	db_cop_colorsLOF(a3),a1
.colmap_oke:

	lea	ttdi(pc),a0
	move.l	a1,tdi_colorMap(a0)
	move.w	#0,tdi_renderLine(a0)
	move.w	#0,tdi_dstClear(a0)

;	tst.b	db_wintriple(a3)		; is there a window ?
;	bne	.do_fast

	tst.b	db_screen_wiped(a3)		; when screen is hidden
	beq	.nofastmask			; don't create a fastmask
.do_fast:
;	tst.b	db_win_noput(a3)
;	beq	.win_not_empty
;	lea	screen_bitmap(pc),a0
;	bsr	clear_bitmap
;.win_not_empty:

	tst.l	db_fastmask(a3)
	bne	.set_on_or

	move.b	#$0,db_or_fastmask(a3)
	bsr	clear_fastmask

	lea	screen_bitmap(pc),a0
	bsr	alloc_fastmask
	bra	.nofastmask
.set_on_or:
	move.b	#$1,db_or_fastmask(a3)
.nofastmask:
	move.l	db_fastmask(a3),db_where_blocks(a3)
	tst.l	db_fastmask(a3)
	beq	.no_add

	move.l	db_edit_window(a3),a1
	move.w  ew_TopMargin(a1),d1
;	lea	ttdi(pc),a0
;	tst.w	tdi_dstYPos(a0)
	beq	.no_add
	lea	screen_bitmap(pc),a0
	move.l	bm_Planes+4(a0),d0
	sub.l	bm_Planes(a0),d0
	mulu	d1,d0
	add.l	d0,db_where_blocks(a3)
.no_add:

	IFNE 1
	move.l	db_graphbase(a3),a6
	lea	txt_RPort(pc),a1
	jsr     _LVOInitRastPort(a6)

	lea	screen_bitmap(pc),a0
	move.l	bm_Planes+4(a0),d7
	sub.l	bm_Planes(a0),d7		; width one plane

	lea	txt_RPort(pc),a1
	move.l	a0,rp_BitMap(a1)

	lea	ttdi(pc),a0
	move.l	a1,tdi_dstRPort(a0)
	move.l	#0,tdi_maskBitMap(a0)
.loop:

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"In drawtext",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	jsr	_DrawText

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr2(pc),a0
	jsr	KPutStr
	bra	.tt2
.dbstr2:	dc.b	"Na drawtext",10,0
	even
.tt2:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC


	tst.w	tdi_resultHeight(a0)
	bmi.w	.free
	beq.w	.free

	tst.l	db_where_blocks(a3)
	beq	.no_mask

; copy resultheight to fast mask
	
	move.l	d7,d0
	mulu	tdi_resultHeight(a0),d0		; number off bytes to copy
	lsr.l	#1,d0				; words
	move.l	tdi_maskBitMap(a0),a1
	move.l	db_where_blocks(a3),a2

	cmp.l	#0,a1				; no mask clear area
	beq	.rep3

	tst.w	tdi_resultWidth(a0)		; empty line clear fastmask
	beq	.rep3

	tst.b	db_or_fastmask(a3)
	beq	.rep
.rep2:
	move.w	(a1)+,d1
	or.w	d1,(a2)+
	subq.l	#1,d0
	bne	.rep2
	bra	.goon
.rep:
;	move.w	#$f0f0,(a2)+
	move.w	(a1)+,(a2)+
	subq.l	#1,d0
	bne	.rep
	bra	.goon

.rep3:
	clr.w	(a2)+
	subq.l	#1,d0
	bne	.rep3

.goon:
	move.l	a2,db_where_blocks(a3)
.no_mask:
	move.w	tdi_resultHeight(a0),d0
	add.w	d0,tdi_dstYPos(a0)
	addq.w	#1,tdi_renderLine(a0)
	tst.w	tdi_resultHeight(a0)
	bmi.b	.free
	beq.b	.free
	bra.w	.loop
.free:
	ENDC

	cmp.w	#-1,db_text_wipe(a3)
	bne	wipe_text_in

.exit_dr:
	rts

	IFNE 0
	movem.l	a0-a6/d0-d7,-(a7)
	moveq	#7,d7
	moveq	#0,d6
	move.l	a1,a4
.rep:	lea	.cd(pc),a1
	move.b	(a4)+,d6
	move.w	d6,(a1)
	move.b	(a4)+,d6
	move.w	d6,2(a1)
	move.b	(a4)+,d6
	move.w	d6,4(a1)

	lea	.dbstrcol,a0
	jsr	KPutFmt
	dbf	d7,.rep
	movem.l	(a7)+,a0-a6/d0-d7
	bra	.tt
.cd:	dc.w	0,0,0
.dbstrcol:	dc.b	"Col - %d,%d,%d",10,0
	even
.tt:
	ENDC


	XREF	_DrawText

wipe_text_in:
	move.l	db_inactive_fileblok(a3),a5
	move.l	db_window_data(a3),a0
	moveq	#0,d0
	move.l	winp_wx(a0),d0
	move.w	d0,vb_lenx(a5)
	move.l	winp_wy(a0),d0
	move.w	d0,vb_leny(a5)
	move.w	db_screen_depth(a3),vb_planes(a5)
	move.w	db_screen_mode(a3),vb_mode(a5)

	move.l	db_triple_mem(a3),vb_bitdata(a5)    ; screen is in triple mem
	move.l	db_triple_mem(a3),vb_body_start(a5)
	move.b	#0,vb_compression(a5)		    ; indicate no compression
	move.b	#$ff,vb_bitmapped(a5)

	moveq	#0,d0
	move.w	vb_lenx(a5),d0
	add.w	#15,d0
	lsr.l	#4,d0
	add.l	d0,d0
	move.l	d0,d1
	move.l	d0,vb_breedte_x(a5)
	move.w	vb_planes(a5),d0
	mulu	vb_leny(a5),d1
	subq.w	#1,d0
	moveq	#0,d2
	bmi	no_addup122t

rep_add522t:
	add.l	d1,d2
	dbf	d0,rep_add522t
	move.l	d2,d1
no_addup122t:
	move.l	d1,vb_unpacked_size(a5)
	move.l	d1,vb_packed_size(a5)
	move.b	#0,vb_fastmask(a5)

	move.l	db_text_data(a3),a0
	addq.l	#textp_eff,a0
	move.l	a0,db_effect_data(a3)

	moveq	#0,d0
	move.w	db_winpos_x(a3),d0
	move.l	d0,db_blitin_x(a3)
	move.w	db_winpos_y(a3),d0
	move.l	d0,db_blitin_y(a3)

	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)

	bsr	wipe_effect_in
	rts

draw_text:
	rts

ttdi:		ds.b	tdi_SIZE
dst_RPort:	ds.b	rp_SIZEOF
txt_RPort:	ds.b	rp_SIZEOF


**************************************************

	XDEF	_create_animclip
_create_animclip:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	12(a5),a3
	move.l	8(a5),a0			; pointer to parameters
	move.l	a7,db_easy_exit(a3)

	move.b	#1,db_give_error(a3)		; set return error none
	move.l	a0,db_clip_data(a3)
	bsr	create_animclip
	move.b	#0,db_give_error(a3)
	bra	ret_err

create_animclip:
	tst.b	db_quit(a3)
	beq	.no_quit11
	rts

.no_quit11:
	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)

*
* First wipe in any window or screens that are in the buffer
*

	move.l	a0,db_clip_data(a3)
	tst.b	db_wintriple(a3)
	beq	.no_win2
	move.l	a0,-(a7)
	bsr	wipe_in_window		; check the triple buffer for windows
	move.l	(a7)+,a0
.no_win2:
	tst.b	db_quit(a3)
	beq	.no_quit13
	rts

.no_quit13:
	tst.b	db_screen_wiped(a3)
	bne	.no_win3

	move.l	a0,-(a7)
	bsr	wipe_screen
	move.l	(a7)+,a0
.no_win3:

	move.l	a0,-(a7)
	jsr	copy_info
	move.l	vb_breedte_x(a5),d1
	mulu	vb_planes(a5),d1
	mulu	vb_leny(a5),d1			; size of picture
	move.l	vb_tempbuffer(a4),a1
	move.l	vb_tempbuffer(a5),a0
	lsr.l	#2,d1				; longs ?
.rep_b_cop2:
	move.l	(a0)+,(a1)+
	subq.l	#1,d1
	bne	.rep_b_cop2
	move.l	db_inactive_viewblok(a3),a5
	jsr	maak_viewport			; create hidden view
	move.l	(a7)+,a0

	jsr	init_anim_struct
	tst.w	d0
	bne	.anim_err
.ww:
	jsr	showinactive2
	jsr	get_next_frame
	tst.b	d0
	bne	.exitac

	jsr	anim_set_base

;	jsr	wacht_tijd2
	tst.b	d0
	beq	.ww

.exitac:
	jsr	free_init_anim
.anim_err
	rts

**************************************************
					
	XDEF	_create_clip
_create_clip:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	12(a5),a3
	move.l	8(a5),a0			; pointer to parameters
	move.l	a7,db_easy_exit(a3)

	move.b	#1,db_give_error(a3)		; set return error none
	move.l	a0,db_clip_data(a3)
	bsr	create_clip
	move.b	#0,db_give_error(a3)
	bra	ret_err

create_clip:
	tst.b	db_quit(a3)
	beq	no_quit11
	rts

no_quit11:
	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)

*
* The clip must be placed in the screen before wipe in
* First load and decrunch and then place clip in the screen
*
	move.l	a0,db_clip_data(a3)
	move.l	clipp_eff+eff_innum(a0),d0
	move.w	d0,db_clip_wipe(a3)

	cmp.w	#-1,db_clip_wipe(a3)
	beq	no_win2
	tst.b	db_wintriple(a3)
	beq	no_win2

	move.l	a0,-(a7)
	bsr	wipe_in_window		; check the triple buffer for windows
	move.l	(a7)+,a0

no_win2:

	tst.b	db_quit(a3)
	beq	no_quit13
	cmp.l	#-1,clipp_eff+eff_outnum(a0)
 	beq	no_quit13
	rts
no_quit13:
	cmp.w	#-1,db_clip_wipe(a3)
	beq	no_win3

	tst.b	db_screen_wiped(a3)
	bne	no_win3

	move.l	a0,-(a7)
	bsr	wipe_screen
	move.l	(a7)+,a0

no_win3:
	tst.b	db_skip_cut_woe(a3)
	beq	.no_skip
	moveq	#0,d0
	rts
.no_skip:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	move.l	a0,a1
	lea	dbstr4(pc),a0
	jsr	KPutFmt
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	(a0),a0

	jsr	laadfile

	move.l	db_inactive_fileblok(a3),a5

	move.l	db_clip_data(a3),a1
	move.l	clipp_width(a1),d1
	move.l	clipp_height(a1),d2
	add.l	clipp_x(a1),d1
	add.l	clipp_y(a1),d2

	move.l	db_window_data(a3),a1
	move.l	winp_wx(a1),d3
	move.l	winp_wy(a1),d4

	move.l	db_clip_data(a3),a1
	cmp.l	d3,d1
	ble	oke_cl1
	sub.l	d3,d1
	sub.l	d1,clipp_width(a1)
oke_cl1:
	cmp.l	d4,d2
	ble	oke_cl2
	sub.l	d4,d2
	sub.l	d2,clipp_height(a1)
oke_cl2:
	move.l	clipp_height(a1),d0
	cmp.l	clipp_height(a1),d4		; clip can stil be > window
	bgt	oke_cl3
	move.l	d4,clipp_height(a1)

oke_cl3:
	move.l	clipp_width(a1),d0
	cmp.l	clipp_width(a1),d3
	bgt	oke_cl4
	move.l	d3,clipp_width(a1)
oke_cl4:
	cmp.w	#-1,db_clip_wipe(a3)		; use screen bitmap or window
	beq	wipe_clip_in2

	tst.b	db_quit(a3)
	bne	wipe_clip_in2			; need to stop

	lea	screen_bitmap(pc),a0
	move.w	vb_planes(a5),d0		; depth
	move.w	db_screen_depth(a3),d0
	move.l	db_clip_data(a3),a1
	move.l	clipp_width(a1),d1		; create a bitmap of clip-size
	move.l	clipp_height(a1),d2
	move.l	db_triple_mem(a3),d3
	bsr	create_bitmap

; clear the bitmap

	lea	screen_bitmap(pc),a0
	bsr	clear_bitmap

wipe_clip_in2:
	move.l	db_clip_data(a3),a0	
	move.l	clipp_x(a0),d0			; x pos clip in pic
	move.l	clipp_y(a0),d1			; y pos
	tst.l	d0
	bpl	lpic1
	neg.l	d0	
	moveq	#0,d2
	bra	lpic11
lpic1:
	move.l	d0,d2
	moveq	#0,d0
lpic11:	
	tst.l	d1
	bpl	lpic2
	neg.l	d1
	moveq	#0,d3
	bra	lpic21
lpic2:
	move.l	d1,d3
	moveq	#0,d1
lpic21:
	move.l	clipp_width(a0),d4
	move.l	clipp_height(a0),d5
	cmp.w	db_winsize_x(a3),d4
	ble	oke_wx_1
	move.w	db_winsize_x(a3),d4
oke_wx_1:
	cmp.w	db_winsize_y(a3),d5
	ble	oke_wy_1
	move.w	db_winsize_y(a3),d5
oke_wy_1:
	move.w	d4,d6
	add.w	d0,d6
	move.w	vb_lenx(a5),d7
	cmp.w	d7,d6
	ble	clip_x_oke
	sub.w	d7,d6
	sub.w	d6,d4
clip_x_oke:
	move.w	vb_leny(a5),d7
	move.w	d5,d6
	add.w	d1,d6
	cmp.w	d7,d6
	ble	clip_y_oke
	sub.w	d7,d6
	sub.w	d6,d5
clip_y_oke:
	moveq	#0,d6
	cmp.w	#-1,db_clip_wipe(a3)
	bne	wipe_clip_in

	tst.b	db_screen_wiped(a3)
	bne	no_adjust2
	
	move.w	db_winpos_x(a3),d6	; adjust with window screen position
	add.l	d6,d2
	move.w	db_winpos_y(a3),d6
	add.l	d6,d3
no_adjust2:

	move.l	db_window_data(a3),a0
	cmp.l	#-1,winp_lbc(a0)
	beq	no_rbc
	add.l	winp_bw(a0),d2
no_rbc:
	cmp.l	#-1,winp_tbc(a0)
	beq	no_tbc
	add.l	winp_bw(a0),d3

no_tbc:
	bsr	check_height

; Store the parameters for fastmasking

	lea	fm_params(pc),a0
	movem.l	d0-d5,(a0)

; check the window mode to see wether or not a mask is needed

	move.l	#$0c0,d6

	movem.l	d0-d4,-(a7)
	bsr	set_blit_fast_direct	; is there somethin beneath me ?
	movem.l	(a7)+,d0-d4

	tst.b	db_screen_empty(a3)
	bne	.no_transp1

	move.l	db_window_data(a3),a0

	clr.b	db_clip_leaved(a3)

; at this moment the picture is required
; decrunch what you need in the inactive viewblok

	bsr	get_decrunched_clip_chip

	move.l	db_window_data(a3),a1
	move.l	winp_flags(a1),d7
	and.w	#16,d7
	bne	back_fill_mode

; Decrunched clip now in chipmem inactive viewblok

	move.l	#$0e0,d6
	lea	screen_bitmap(pc),a1	; to screen or window
	bsr	mask_window		; mask clip out

	bra	.dec_ready

.no_transp1:

	move.l	db_window_data(a3),a1

	move.l	winp_flags(a1),d7
	and.w	#16,d7
	bne	.normal

	tst.w	d0
	bne	.normal

	tst.w	d2
	bne	.normal

	cmp.w	vb_lenx(a5),d3				; is the clip bigger ??
	bne	.normal

	cmp.w	#$0e0,d6
	beq	.dec_ready

; source and dest x are zero and you don't need a mask
; you can directly decrunch to destination chipmem

	bsr	get_decrunched_direct

	bra	.setmm

.normal:
	cmp.w	#$0e0,d6
	beq	.dec_ready

	bsr	get_decrunched_clip_chip

	move.l	db_window_data(a3),a1
	move.l	winp_flags(a1),d7
	and.w	#16,d7
	bne	back_fill_mode

	tst.b	db_win_noput(a3)		; is there something in
	bne	.dec_ready

	move.l	#$0e0,d6
	lea	screen_bitmap(pc),a1	; to screen or window
	bsr	mask_window		; mask clip out

.dec_ready:
	move.w	#$0e0,d6
;	move.w	#$0c0,d6
	move.l	#$ff,d7
	move.w	#0,a2
	move.l	db_graphbase(a3),a6
	move.l	vb_bitmapw(a5),a0	; from clip
	lea	screen_bitmap(pc),a1	; to screen or window
	jsr	_LVOBltBitMap(a6)

.setmm:
	tst.b	db_screen_wiped(a3)
	beq	.no_mask_to_fast
	move.l	db_window_data(a3),a0

	cmp.l	#1,winp_it(a0)
	bne	.no_solid
	cmp.l	#0,winp_sdir(a0)
	beq	.no_mask_to_fast
.no_solid
	bsr	copy_imask_to_fast
.no_mask_to_fast:

	bsr	set_min_max_values
	jsr	clear_inactive_buffers

	rts

fm_params:	blk.l	6,0

clear_fastmask:
	tst.l	db_fastmask(a3)
	beq	.nom
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Free fastmask",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	db_fastmask(a3),a1
	move.l	db_fastmask_size(a3),d0
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)
	clr.l	db_fastmask(a3)
.nom:
	rts
*
* Bitmap in a0
*
alloc_fastmask:
	moveq	#0,d0
	move.b	bm_Depth(a0),d0
	move.l	bm_Planes+4(a0),d1
	sub.l	bm_Planes(a0),d1		; plane width in bytes
	move.l	d1,d7				; width in bytes
	mulu	d0,d1				; offset to mask
	move.l	bm_Planes(a0),d0
	add.l	d1,d0				; points to mask
	move.l	d0,a2

	add.l	d7,d1				; total width 1 line
	move.l	d1,d6
	move.l	d7,d0
	mulu.w	bm_Rows(a0),d0			; memory size
	move.l	d0,db_fastmask_size(a3)

	move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1	; Dont use clear here ?????
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,db_fastmask(a3)
	rts
*
* Copy the mask from the interleaving to fastmem
*
copy_imask_to_fast:
	bsr	clear_fastmask
	move.w	db_winsize_x(a3),d0

	lea	fm_params(pc),a0
	movem.l	(a0),d0-d5
	tst.b	db_clip_leaved(a3)
	beq	.no_mask_inclip

	tst.w	d0
	bne	.offset_mask_inclip
	tst.w	d1
	bne	.offset_mask_inclip
	tst.w	d2
	bne	.offset_mask_inclip
	tst.w	d3
	bne	.offset_mask_inclip
	cmp.w	db_winsize_x(a3),d4
	bne	.offset_mask_inclip
	cmp.w	db_winsize_y(a3),d5
	bne	.offset_mask_inclip

	move.l	vb_bitmapw(a5),a0	; from clip

	bsr	alloc_fastmask
	tst.l	db_fastmask(a3)
	beq	.no_mask_inclip

	move.l	vb_bitmapw(a5),a0	; from clip

	move.w	bm_Rows(a0),d5
	move.l	d0,a0
	move.l	a2,a4
	lsr.w	#1,d7
	bra	.in2
.rep_l2:
	move.l	d7,d0
	move.l	a4,a2
	bra	.in1
.rep_l:
	move.w	(a2)+,(a0)+
.in1:	dbf	d0,.rep_l
	add.l	d6,a4
.in2:	dbf	d5,.rep_l2

.no_mask_inclip:
	rts

*	
* The size of the clip and therefor the mask is not the same
* Use the dummy bitmap to copy the mask to at the right position
*
.offset_mask_inclip:
tpy:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Copy clip-mask to fast with bltbitmap",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC
	move.l	vb_bitmapw(a5),a0
	move.w	bm_BytesPerRow(a0),d0
	mulu	bm_Rows(a0),d0			; size bitmap
	move.l	d0,d7

; is there space for a one plane bitmap

	lea.l	screen_bitmap(pc),a0		; should be the window bitmap
	moveq	#1,d0
	move.l	bm_Planes+4(a0),d1
	sub.l	bm_Planes(a0),d1		; plane width in bytes
	lsl.l	#3,d1				; width in pixels
	move.w	bm_Rows(a0),d2
	move.l	db_graphbase(a3),a6
	lea	dum_bm(pc),a0
	jsr	_LVOInitBitMap(a6)

	lea.l	dum_bm(pc),a0			; one plane bitmap
	move.w	bm_BytesPerRow(a0),d0
	mulu	bm_Rows(a0),d0			; size bitmap
	move.l	d0,db_fastmask_size(a3)
	add.l	d7,d0				; total size needed

	IFNE XAPP
	move.l	db_mlsysstruct(a3),a2
	move.l	sbu_Size(a2),d2
	ELSE
	move.l	#MEMSIZE,d2
	ENDC
	cmp.l	d0,d2
	ble	.no_space
	move.l	vb_bitmapw(a5),a0
	move.l	bm_Planes(a0),a1
	add.l	d7,a1				; at the end of clip

	lea	dum_bm(pc),a0
	move.l	a1,bm_Planes(a0)		; the one plane

	move.l	vb_bitmapw(a5),a0
	moveq	#0,d0
	move.b	bm_Depth(a0),d0
	move.l	bm_Planes+4(a0),d1
	sub.l	bm_Planes(a0),d1		; plane width in bytes
	mulu	d0,d1				; offset to mask
	move.l	bm_Planes(a0),d0
	add.l	d1,d0				; points to mask
	move.l	d0,bm_Planes(a0)
	move.l	d0,bm_Planes+4(a0)
	move.l	d0,bm_Planes+8(a0)
	move.l	d0,bm_Planes+12(a0)
	move.l	d0,bm_Planes+16(a0)
	move.l	d0,bm_Planes+20(a0)
	move.l	d0,bm_Planes+24(a0)
	move.l	d0,bm_Planes+28(a0)

	lea	dum_bm(pc),a0
	move.l	db_fastmask_size(a3),d7
	move.l	bm_Planes(a0),a0
	lsr.l	#2,d7
	beq	.no_space
.repc2:
	clr.l	(a0)+
	subq.l	#1,d7
	bne	.repc2

	lea	fm_params(pc),a0
	movem.l	(a0),d0-d5
	move.l	vb_bitmapw(a5),a0
	lea	dum_bm(pc),a1
	move.w	#$0c0,d6
	moveq	#1,d7				; just the one plane
	move.w	#0,a2
	move.l	db_graphbase(a3),a6
	jsr	_LVOBltBitMap(a6)

	move.l	db_fastmask_size(a3),d0

	move.l	#MEMF_PUBLIC,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,db_fastmask(a3)
	beq	.no_space

	move.l	db_graphbase(a3),a6		; wait until the blit is ready
	jsr	_LVOWaitBlit(a6)

	lea	dum_bm(pc),a0
	move.l	d0,a1
	move.l	db_fastmask(a3),a1
	move.l	db_fastmask_size(a3),d7
	move.l	bm_Planes(a0),a0
	lsr.l	#2,d7
	beq	.no_space
.repc:
	move.l	(a0)+,(a1)+
	subq.l	#1,d7
	bne	.repc
.no_space:
	rts

*
* Decrunch the clip to chip memory non optimized
* (Optimisation decrunch only the part you need)
*
get_decrunched_clip_chip:
	movem.l	d0-d7/a0-a6,-(a7)
.try_again:
	jsr	decrunch_clip			; decrunch in inactive block
	tst.w	d0
	beq	.unp_ready
	bsr	check_height_block	
	bra	.try_again

.unp_ready:
	move.l	db_inactive_viewblok(a3),a5
	moveq	#0,d0
	moveq	#0,d1
	moveq	#0,d2
	move.l	vb_bitmapw(a5),a0
	move.w	vb_planes(a5),d0		; depth
	move.w	vb_lenx(a5),d1
	move.w	vb_leny(a5),d2			; height
	move.l	vb_tempbuffer(a5),d3
	tst.b	db_clip_leaved(a3)
	beq	.norm
	bsr	create_bitmap_leaved
	bra	.what
.norm:
	bsr	create_bitmap
.what:
	movem.l	(a7)+,d0-d7/a0-a6
	move.l	db_inactive_viewblok(a3),a5
	rts

check_height_block:
; Reduce height clip to fit
	movem.l	d7/d6/a5,-(a7)
	move.l	db_inactive_fileblok(a3),a5
	move.l	vb_unpacked_size(a5),d7
	move.l	db_inactive_viewblok(a3),a5

	IFNE XAPP
	move.l	db_mlsysstruct(a3),a2
	move.l	sbu_Size(a2),d0
	ELSE
	move.l	#MEMSIZE,d0
	ENDC
	move.l	db_inactive_fileblok(a3),a5
	move.l	vb_breedte_x(a5),d6
	mulu	vb_planes(a5),d6
	sub.l	d0,d7
	divu	d6,d7				; lines to much
	addq.w	#1,d7
	ext.l	d7
	move.w	vb_leny(a5),d6
	sub.w	d7,vb_leny(a5)
	move.l	vb_breedte_x(a5),d7
	mulu	vb_planes(a5),d7
	mulu	vb_leny(a5),d7
	move.l	d7,vb_unpacked_size(a5)

	movem.l	(a7)+,d7/d6/a5
	rts

check_height:
	movem.l	d6/d7,-(a7)
	move.w	d2,d6
	add.w	d4,d6
	cmp.w	db_screen_x(a3),d6
	ble	.okex
	sub.w	db_screen_x(a3),d6
	sub.w	d6,d4				; reduce Xsize
.okex:
	move.w	d3,d6
	add.w	d5,d6
	cmp.w	db_screen_y(a3),d6
	ble	.okey
	sub.w	db_screen_y(a3),d6
	sub.w	d6,d5				; reduce Ysize
.okey:
	movem.l	(a7)+,d6/d7
	rts

	IFNE DEBUG
dbstr1:		dc.b	"Decrunch whole",10,0
dbstr2:		dc.b	"Decrunch partly",10,0
dbstr3:		dc.b	"Normal bitmapped",10,0
dbstr3f:	dc.b	"Exit Normal bitmapped",10,0
dbstr4:		dc.b	"Doing clip [%s]",10,0
dbstr4d:	dc.b	"Unpack failed",10,0

	even
	ENDC
*
* Decrunch directly to chipmem ( the screen bitmap )
* This bitmap is interleaved
*
get_decrunched_direct:

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Decrunch direct",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC
tt99:

	movem.l	d0-d7/a0-a6,-(a7)
	lea	screen_bitmap(pc),a1	; to screen or window
	move.w	bm_BytesPerRow(a1),d7
	lsl.w	#3,d7			; bits per row
	moveq	#0,d0
	move.b	bm_Depth(a1),d0
	divu	d0,d7
	cmp.w	d4,d7
	blt	.clip_bigger

; for now use coords (0,0)
; decrunch directly to screen_bitmap
;
; xsize - d4
; ysize - d5

	move.l	vb_body_start(a5),db_tempbodystart(a3)
	move.l	db_wpatroonruimte(a3),a0
	move.l	a0,db_templinebuffer1(a3)

	move.w	d4,d7
	add.w	#15,d7
	lsr.w	#4,d7			; words needed
	add.w	d7,d7			; bytes needed
	
	move.w	bm_BytesPerRow(a1),d6
	divu	d0,d6
	sub.w	d7,d6			; modulo per line
	bmi	.clip_bigger		; error ??
	bne	.no_faster2

	move.w	vb_planes(a5),d0
	cmp.b	bm_Depth(a1),d0
	bne	.no_faster

	move.l	db_inactive_fileblok(a3),a5
	tst.b	vb_masking(a5)
	bne	.no_faster

; There is a direct match so unpack the whole file direct

	move.l	db_inactive_fileblok(a3),a5
	move.l	bm_Planes(a1),vb_tempbuffer(a5)
	move.l	vb_breedte_x(a5),d1
	mulu	vb_planes(a5),d1
	mulu	d3,d1
	add.l	d1,vb_tempbuffer(a5)
	moveq	#0,d0
	move.w	bm_Rows(a1),d0
	move.w	vb_leny(a5),d1
	cmp.w	vb_leny(a5),d5
	bgt	.no_pro
	move.w	d5,vb_leny(a5)		; force iff to be window size
	move.l	vb_breedte_x(a5),d1
	mulu	vb_planes(a5),d1
	mulu	d5,d1
	move.l	d1,vb_unpacked_size(a5)
.no_pro:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	dbstr1(pc),a0
	jsr	KPutStr
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	db_inactive_viewblok(a3),a4
	move.l	vb_mlscrpointer(a4),vb_mlscrpointer(a5)
	jsr	unpack
	clr.l	vb_tempbuffer(a5)
	bra	.clip_bigger

.no_faster2:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"No fast Bit Modulo dif",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC
	bra	.tmep
	
.no_faster:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	dbstr2(pc),a0
	jsr	KPutStr
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC
.tmep:
	move.l	db_inactive_fileblok(a3),a5
	lea	screen_bitmap(pc),a1	; to screen or window
	moveq	#0,d1
	move.w	bm_BytesPerRow(a1),d1
	mulu	d3,d1

	move.l	vb_breedte_x(a5),d0
	move.l	d0,d3
	sub.l	d7,d3			; modulo source
	mulu	vb_planes(a5),d0
	tst.b	vb_masking(a5)
	beq	.nom
	add.l	vb_breedte_x(a5),d0		; add another to skip mask
.nom:
	move.l	d0,db_tempbreedtelijn(a3)	; total width line
	and.w	#$f,d4
	move.l	#$ffff0000,d2
	lsr.l	d4,d2
	move.w	d2,db_tempmasker(a3)
	move.l	d0,d4

	move.l	bm_Planes(a1),a2

	add.l	d1,a2

	move.w	bm_BytesPerRow(a1),d2

	bra.s	.in_nl
.next_line:
	bsr	decrunch_part_line		; get line
	bsr	get_part			; copy line to chip
.in_nl:
	dbf	d5,.next_line
.clip_bigger:
	movem.l	(a7)+,d0-d7/a0-a6
	move.l	db_inactive_viewblok(a3),a5
	rts
*
* a2 points to chipmem
* d3 holds the modulo bytes source
* d6 holds the modulo bytes destination
* d7 holds the width total line
* d2 holds the bytes per row
*
get_part:
	move.l	a2,a0
	move.l	db_templinebuffer1(a3),a1
	move.l	d7,d0
	move.w	vb_planes(a5),d1
	bra	.inppl
.reppl:
	move.l	d7,d0
	bra	.incc
.repcc:
	move.b	(a1)+,(a0)+
.incc:
	dbf	d0,.repcc	
	move.w	db_tempmasker(a3),d0
	and.w	d0,-2(a0)
	add.w	d6,a0
	add.w	d3,a1
.inppl:	dbf	d1,.reppl
	add.w	d2,a2
	rts
*
* Decrunch a line from an iff file partly
* d4 holds total line size
* a5 holds the viewblock pointer
*
* Results:
* a0 is stored, line is decrunched in templinebuffer1
*
decrunch_part_line:
	movem.l	d6/d5,-(a7)
	move.l	db_templinebuffer1(a3),a1
	move.l	db_tempbodystart(a3),a0
	move.l	a1,a6
	add.l	d4,a6				; end line pointer
	
	cmp.b	#1,vb_compression(a5)
	beq.w	.dun_again

* compression geen 1 dus of 0 of een onbekende 
* copieer van body_start naar unpacked

.dno_co:
	move.b	(a0)+,(a1)+
	cmp.l	a6,a1
	blt	.dno_co
	add.l	d7,a0				; add modulo to source
	move.l	a0,db_tempbodystart(a3)
	movem.l	(a7)+,d6/d5
	rts

.dun_again:	
	cmp.l	a6,a1
	bge.s	.dun_end

	moveq	#0,d5
	move.b	(a0)+,d5
	bmi.b	.dun_minus

.dun_plu:
	move.b	(a0)+,(a1)+
	dbf	d5,.dun_plu
	bra.s	.dun_again
.dun_minus:
	neg.b	d5
	move.b	(a0)+,d0
.dun_rm:
	move.b	d0,(a1)+
	dbf	d5,.dun_rm
	bra.s	.dun_again

.dun_end:	
	move.l	a0,db_tempbodystart(a3)		; start next line here
	movem.l	(a7)+,d6/d5
	rts

*
* The back fill mode fills the background with this clip or window
*
back_fill_mode:
	move.l	a0,-(a7)
	move.l	vb_bitmapw(a5),a0	; from clip
	moveq	#0,d2
	move.w	vb_planes(a5),d2
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	vb_planes(a5),a1
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"Planes is %d",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC
	cmp.w	db_screen_depth(a3),d2
	beq	.noc
	bsr	clear_screen
.noc:	
	move.l	(a7)+,a0
	move.b	#1,db_screen_cleared(a3)	; with the background tiles

	moveq	#0,d2
	moveq	#0,d3
.rep_cc1:
	movem.l	d0-d6/a0/a1,-(a7)
	move.w	db_screen_x(a3),d7
	move.w	d2,d7
	add.w	d4,d7
	cmp.w	db_screen_x(a3),d7
	ble	.no_pro1
	sub.w	db_screen_x(a3),d7
	sub.w	d7,d4
;	move.w	#14,d4
.no_pro1:
	move.w	d3,d7
	add.w	d5,d7
	cmp.w	db_screen_y(a3),d7
	ble	.no_pro2
	sub.w	db_screen_y(a3),d7
	sub.w	d7,d5
.no_pro2:

	move.l	#$ff,d7
	move.w	#0,a2
	move.l	db_graphbase(a3),a6
	move.l	vb_bitmapw(a5),a0	; from clip
	lea	screen_bitmap(pc),a1	; to screen or window
	jsr	_LVOBltBitMap(a6)
	bsr	set_min_max_values
	movem.l	(a7)+,d0-d6/a0/a1
	add.w	d4,d2
	cmp.w	db_screen_x(a3),d2
	blt	.rep_cc1
	moveq	#0,d2
	add.w	d5,d3
	cmp.w	db_screen_y(a3),d3
	blt	.rep_cc1

	rts
*
* The clip can be wiped in, in the normal way only adjust the x y position
* and the x,y size.
* So copy the desired clip
*
wipe_clip_in:
	bsr	get_decrunched_clip_chip

	move.l	#$0c0,d6
	move.l	#$ff,d7
	move.w	#0,a2
	move.l	db_graphbase(a3),a6
	move.l	vb_bitmapw(a5),a0	; from clip
	lea	screen_bitmap(pc),a1	; to triple
	moveq	#0,d2
	moveq	#0,d3
	jsr	_LVOBltBitMap(a6)

	move.b	#$ff,db_cliptriple(a3)	; wipe when text found
	rts

wipe_clip_in3:
	move.l	db_inactive_fileblok(a3),a5
	move.l	db_clip_data(a3),a0
	moveq	#0,d0
	move.l	clipp_width(a0),d0
	move.w	d0,vb_lenx(a5)
	move.l	clipp_height(a0),d0
	move.w	d0,vb_leny(a5)
	move.w	db_screen_depth(a3),vb_planes(a5)
	move.w	db_screen_mode(a3),vb_mode(a5)

	move.l	db_triple_mem(a3),vb_bitdata(a5)    ; screen is in triple mem
	move.l	db_triple_mem(a3),vb_body_start(a5)
	move.b	#0,vb_compression(a5)		    ; indicate no compression
	move.b	#$ff,vb_bitmapped(a5)

	moveq	#0,d0
	move.w	vb_lenx(a5),d0
	add.w	#15,d0
	lsr.l	#4,d0
	add.l	d0,d0
	move.l	d0,d1
	move.l	d0,vb_breedte_x(a5)
	move.w	vb_planes(a5),d0
	mulu	vb_leny(a5),d1
	subq.w	#1,d0
	moveq	#0,d2
	bmi	no_addup122

rep_add522:
	add.l	d1,d2
	dbf	d0,rep_add522
	move.l	d2,d1
no_addup122:
	move.l	d1,vb_unpacked_size(a5)
	move.l	d1,vb_packed_size(a5)

	move.l	db_window_data(a3),a0
	move.b	#0,vb_fastmask(a5)
	cmp.l	#1,winp_it(a0)
	bne	no_fastmask1
	move.b	#1,vb_fastmask(a5)
no_fastmask1:
	move.l	db_clip_data(a3),a0
	lea	clipp_eff(a0),a0
	move.l	a0,db_effect_data(a3)

	move.l	db_clip_data(a3),a0
	moveq	#0,d0
	move.w	db_winpos_x(a3),d0
	move.l	d0,db_blitin_x(a3)
	move.w	db_winpos_y(a3),d0
	move.l	d0,db_blitin_y(a3)
	move.l	db_clip_data(a3),a0
	move.l	clipp_x(a0),d0
	bmi	no_addcl1
	add.l	d0,db_blitin_x(a3)
no_addcl1:
	move.l	clipp_y(a0),d0
	bmi	no_addcl2
	add.l	d0,db_blitin_y(a3)
no_addcl2:
	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)

	move.l	db_window_data(a3),a0
	cmp.l	#-1,winp_lbc(a0)
	beq	no_rbc3bt2
	move.l	winp_bw(a0),d0
	add.l	d0,db_blitin_x(a3)
no_rbc3bt2:
	cmp.l	#-1,winp_tbc(a0)
	beq	no_tbc3bt2
	move.l	winp_bw(a0),d0
	add.l	d0,db_blitin_y(a3)
no_tbc3bt2:

	bsr	wipe_effect_in
	move.b	#$0,db_cliptriple(a3)
	rts

wipe_screen:
	
	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)

	move.l	db_inactive_fileblok(a3),a5
	move.b	#0,vb_hires(a5)
	move.b	#0,vb_shires(a5)
	move.b	#0,vb_interlace(a5)
	move.b	#0,vb_masking(a5)

	move.w	db_screen_x(a3),vb_lenx(a5)
	move.w	db_screen_y(a3),vb_leny(a5)
	move.w	db_screen_depth(a3),vb_planes(a5)
	move.w	db_screen_mode(a3),vb_mode(a5)

	moveq	#0,d0
	move.w	vb_lenx(a5),d0
	add.w	#15,d0
	lsr.l	#4,d0
	add.l	d0,d0
	move.l	d0,d1
	move.l	d0,vb_breedte_x(a5)

	move.w	vb_planes(a5),d0
	mulu	vb_leny(a5),d1
	subq.w	#1,d0
	moveq	#0,d2
	bmi	no_addup12

rep_add52:
	add.l	d1,d2
	dbf	d0,rep_add52
	move.l	d2,d1
no_addup12:
	move.l	d1,vb_unpacked_size(a5)
	move.l	d1,vb_packed_size(a5)

	bsr	get_col_mode

	move.l	db_inactive_fileblok(a3),a5
	move.l	db_triple_mem(a3),vb_bitdata(a5)    ; screen is in triple mem
	move.l	db_triple_mem(a3),vb_body_start(a5)
	move.b	#0,vb_compression(a5)		    ; indicate no compression
	move.b	#$ff,vb_bitmapped(a5)
	move.b	#$ff,db_screen_wiped(a3)	    ; screen is active

	move.w	db_screen_wipe(a3),d0
	move.l	d0,db_which_wipe(a3)

	move.w	db_screen_effsp(a3),d0
	move.l	d0,db_effect_speed(a3)
	move.l	d0,db_effect_speed_org(a3)
	move.l	d0,db_effect_speed_in(a3)
	move.w	db_screen_varl(a3),d0
	move.l	d0,db_varltimes(a3)
	move.l	d0,db_varltimes_in(a3)
	move.w	db_screen_var(a3),d0
	moveq	#0,d0

	move.l	d0,db_variation(a3)

	bsr	get_active_info

	jsr	do_wipe_file_in

	move.w	db_screen_wipe(a3),d0	; see if it was a flip
	lea	wipe_types_new(pc),a0
	move.b	0(a0,d0.w),d1
	and.b	#FLIP_S,d1
	beq	.no_flip
	move.w	#0,db_screen_wipe(a3)	; set on bang page
	bra	wipe_screen
.no_flip:
	bsr	set_mouse_colors
	IFNE	XAPP
	bsr	signal_globeproc
	ENDC
	lea	teller(pc),a0
	move.l	#1,(a0)
	move.l	#1,off_tel_start(a0)
	move.b	#1,db_should_wait(a3)
	rts

	IFNE	XAPP
	XREF	_SendReady

signal_globeproc:
	movem.l	d0-d7/a0-a6,-(a7)
	jsr	_SendReady
	movem.l	(a7)+,d0-d7/a0-a6
	rts
	ENDC

set_mouse_colors:
;
; Set here the mouse colors to original colors
;
	move.l	db_active_viewblok(a3),a5
	cmp.w	#4,vb_planes(a5)
	bgt	.no_restore
	move.w	#$0e44,$dff1a2
	move.w	#$0000,$dff1a4
	move.w	#$0eec,$dff1a6
.no_restore:
	rts
color17:	dc.b	0,0,0
color18:	dc.b	0,0,0
color19:	dc.b	0,0,0
	even

get_col_mode:
	move.w	vb_planes(a5),d1
	moveq	#1,d0
	lsl.l	d1,d0
	move.l	d0,d1
	move.l	d0,d2

	add.l	d0,d0
	add.l	d1,d0
	move.l	d0,vb_color_count(a5)
	lea	db_cop_colorsLOF(a3),a1		; use as temporary storage
	lea	vb_colbytes(a5),a2
	lea	vb_colors(a5),a0
	subq.l	#1,d2
	bmi	no_col_clear2
	moveq	#0,d4
rep_col_clear2:
	moveq	#0,d0
	move.b	(a1),d3
	lsl.w	#4,d3
	and.w	#$f00,d3
	move.b	(a1)+,(a2)+
	or.b	(a1),d3
	and.w	#$ff0,d3
	move.b	(a1)+,(a2)+
	move.b	(a1),d4
	lsr.w	#4,d4
	or.w	d4,d3
	move.b	(a1)+,(a2)+
	move.w	d3,(a0)+
	dbf	d2,rep_col_clear2
no_col_clear2:
	move.w	vb_mode(a5),d0
	and.w	#V_HAM,d0
	beq	no_ham22
	move.l	#30,vb_color_count(a5)
	tst.b	db_aa_present(a3)
	beq	no_ham22
	move.l	#192,vb_color_count(a5)
no_ham22:
	move.w	vb_mode(a5),d0
	and.w	#V_LACE,d0
	beq	no_lace22
	move.b	#$ff,vb_interlace(a5)
no_lace22:
	move.w	vb_mode(a5),d0
	and.w	#V_HIRES,d0
	beq	no_hires22
	move.b	#$ff,vb_hires(a5)
no_hires22:
	move.w	vb_mode(a5),d0
	and.w	#V_SUPERHIRES,d0
	beq	no_shires22
	move.b	#$ff,vb_shires(a5)
no_shires22:
	rts

dum_bm:	blk.b	bm_SIZEOF,0			; DATA in pogram ????

*
* Cut out the mask in the different planes of the window or screen
* a1 gives the dest bitmap
* d0-d5 are the correct bitblt parameters
* a5 points to the inactive_viewblok which contains the bitmap pointer
*
mask_window:
	movem.l	d0-d7/a0-a6,-(a7)
	tst.b	db_clip_leaved(a3)
	beq	.no_mask_inclip

; the mask is interleaved with the picture
; create a dummy bitmap with all planes pointing to the mask

	move.l	vb_bitmapw(a5),a0
	moveq	#0,d0
	move.b	bm_Depth(a0),d0
	move.l	bm_Planes+4(a0),d1
	sub.l	bm_Planes(a0),d1		; plane width in bytes
	lsl.l	#3,d1				; width in pixels
	addq.w	#1,d0
	mulu	d0,d1
	subq.w	#1,d0
	move.w	bm_Rows(a0),d2
	move.l	db_graphbase(a3),a6
	lea	dum_bm(pc),a0
	jsr	_LVOInitBitMap(a6)

	move.l	vb_bitmapw(a5),a0
	moveq	#0,d0
	move.b	bm_Depth(a0),d0
	move.l	bm_Planes+4(a0),d1
	sub.l	bm_Planes(a0),d1		; plane width in bytes
	mulu	d0,d1				; offset to mask
	move.l	bm_Planes(a0),d0
	add.l	d1,d0				; points to mask

	lea	dum_bm(pc),a0
	move.l	d0,bm_Planes(a0)
	move.l	d0,bm_Planes+4(a0)
	move.l	d0,bm_Planes+8(a0)
	move.l	d0,bm_Planes+12(a0)
	move.l	d0,bm_Planes+16(a0)
	move.l	d0,bm_Planes+20(a0)
	move.l	d0,bm_Planes+24(a0)
	move.l	d0,bm_Planes+28(a0)

	movem.l	(a7),d0-d7/a0-a6		; get back registers

	lea	dum_bm(pc),a0
	move.w	#$020,d6
	move.w	#$ff,d7
	move.w	#0,a2
	move.l	db_graphbase(a3),a6
	jsr	_LVOBltBitMap(a6)
	bra	.maskready

.no_mask_inclip:

	move.l	bm_Planes+4(a1),d6
	sub.l	bm_Planes(a1),d6
	lsl.l	#3,d6
	move.l	d6,a2
	move.l	vb_bitmapw(a5),a0
	move.l	bm_Planes+4(a0),d6
	sub.l	bm_Planes(a0),d6
	lsl.l	#3,d6
	move.l	d6,d7

	move.l	db_graphbase(a3),a6
	move.w	db_screen_depth(a3),d6
.punch_plane2:
	movem.l	d0-d7/a1,-(a7)

	move.w	db_screen_depth(a3),d6
.punch_plane:
	movem.l	d0-d7/a1/a2,-(a7)
	move.l	vb_bitmapw(a5),a0
	moveq	#$020,d6
	moveq	#$1,d7
	move.w	#0,a2
	jsr	_LVOBltBitMap(a6)
	movem.l	(a7)+,d0-d7/a1/a2
	add.l	a2,d2
	subq.w	#1,d6
	bne	.punch_plane
	movem.l	(a7)+,d0-d7/a1
	add.l	d7,d0
	subq.l	#1,d6
	bne	.punch_plane2
.maskready:
	movem.l	(a7)+,d0-d7/a0-a6
	rts

*
* Copy the window in the inactive buffer to the screen
* if the screen is in the triple 
* otherwise copy the window to the triple buffer
* where is has to be filled in later
*
copy_window:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Copy window",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	tst.b	db_screen_wiped(a3)
	bne	copy_window_window

sorry_copy_to_screen:
	bsr	create_screen_bitmap

	tst.b	db_win_noput(a3)
	bne	win_empty
	move.l	db_graphbase(a3),a6
	lea	screen_bitmap(pc),a1
	move.l	db_inactive_viewblok(a3),a5	; from the inactive buffer

	bsr	set_blit_fast_direct

	tst.b	db_screen_empty(a3)
	bne	.db1

	moveq	#0,d0				; the whole window
	moveq	#0,d1
	move.w	db_winpos_x(a3),d2
	move.w	db_winpos_y(a3),d3
	move.w	db_winsize_x(a3),d4
	move.w	db_winsize_y(a3),d5

	move.l	#$0c0,d6

	move.l	db_window_data(a3),a0
	cmp.l	#0,winp_sdir(a0)
	bne	.shadow_mask

	cmp.l	#1,winp_it(a0)
	beq	no_transp2
.shadow_mask:
	move.l	#$0e0,d6
	bsr	mask_window
	bra	no_transp2
.db1:
	moveq	#0,d0				; the whole window
	moveq	#0,d1
	move.w	db_winpos_x(a3),d2
	move.w	db_winpos_y(a3),d3
	move.w	db_winsize_x(a3),d4
	move.w	db_winsize_y(a3),d5
	move.l	#$0c0,d6

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	dbstr42(pc),a0
	jsr	KPutStr
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

no_transp2:

	move.l	vb_bitmapw(a5),a0
	move.l	#$ff,d7
	move.w	#0,a2
	jsr	_LVOBltBitMap(a6)
	move.b	#$ff,db_wininscreen(a3)		; window is in the screen
	bsr	set_min_max_values
	rts

win_empty:
	move.b	#$ff,db_wininscreen(a3)		; window is in the screen
	rts

	IFNE	DEBUG
dbstr1003:	dc.b	"Pattern %d",10,0
dbstr41:	dc.b	"Window empty",10,0
dbstr42:	dc.b	"Screen empty",10,0
dbstr43:	dc.b	"clip screen empty",10,0
	even
	ENDC

*
* The window has an effect but is has to wait for the clips and texts
* to be put in. After al has been filled in the effect can take place
*
copy_window_window:

	tst.b	db_quit(a3)
	bne	sorry_copy_to_screen		; copy directly to screen
	
	move.b	#$ff,db_wintriple(a3)
	bsr	create_window_bitmap

	tst.b	db_win_noput(a3)
	bne	.win_empty

	lea	screen_bitmap(pc),a1
	move.l	db_inactive_viewblok(a3),a5	; from the inactive buffer
	move.l	vb_bitmapw(a5),a0
	moveq	#0,d0				; the whole window
	moveq	#0,d1
	moveq	#0,d2
	moveq	#0,d3
	move.w	db_winsize_x(a3),d4
	move.w	db_winsize_y(a3),d5
	move.l	#$0c0,d6

	move.l	#$ff,d7
	move.w	#0,a2
	jsr	_LVOBltBitMap(a6)		; use a memcpy ????????
	rts

.win_empty:				; window is empty just clear the dest
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Window copy skipped clearing",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	lea.l	screen_bitmap(pc),a0
	bsr	clear_bitmap
	rts

*
* Init the window bitmap for copy-ing
*
create_window_bitmap:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Create window bitmap",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	moveq	#0,d0
	moveq	#0,d1
	moveq	#0,d2
	lea.l	screen_bitmap(pc),a0
	move.w	db_screen_depth(a3),d0		; depth
	move.w	db_winsize_x(a3),d1
	move.w	db_winsize_y(a3),d2		; height
	move.l	db_triple_mem(a3),d3
	bsr	create_bitmap

;	lea.l	screen_bitmap(pc),a0
;	bsr	clear_bitmap
;	rts
		
	tst.b	db_win_noput(a3)
	bne	.win_empty

	moveq	#0,d0
	moveq	#0,d1
	moveq	#0,d2
	move.w	db_winsize_x(a3),d1
	add.l	#15,d1
	lsr.l	#4,d1
	add.l	d1,d1
	mulu	db_winsize_y(a3),d1
	mulu	db_screen_depth(a3),d1
	move.l	d1,d2

	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)

	moveq	#0,d1
	move.w	db_winsize_x(a3),d1
	add.l	#15,d1
	lsr.l	#4,d1
	add.l	d1,d1				; width in bytes	

	move.w	db_winsize_y(a3),d0		; height
	mulu.w	db_screen_depth(a3),d0		; total height
	move.l	db_triple_mem(a3),a0

	add.l	d1,a0
	subq.l	#2,a0
	moveq	#0,d2
	bra	.in
.rep_cl2:
	move.w	d2,(a0)				; only clear not used part
	add.l	d1,a0
.in:	dbf	d0,.rep_cl2
.win_empty:
	rts

*
* Init the screen bitmap for copy-ing
*
create_screen_bitmap:
	moveq	#0,d0
	moveq	#0,d1
	moveq	#0,d2
	lea.l	screen_bitmap(pc),a0
	move.w	db_screen_depth(a3),d0		; depth
	move.w	db_screen_x(a3),d1
	move.w	db_screen_y(a3),d2		; height
	move.l	db_triple_mem(a3),d3
	tst.b	db_quit(a3)
	beq	no_quit1
	move.l	db_active_viewblok(a3),a5
	move.l	vb_tempbuffer(a5),d3
no_quit1:	
	bsr	create_bitmap
	rts
	
screen_bitmap:
	ds.b	bm_SIZEOF

; 123,192,379,83,-1,-1,-1,-1,3,7,3,1

	IFEQ XAPP

test_window:
	dc.l	18,12,172,164
; borders	tbc,rbc,bbc,bwidht
	dc.l	4,15,15,4,2
; filled	ic,it ( 1 filled, 2 pattern, 3 transp), flags, patnr.
	dc.l	5,1,0,0,0,0,0,8,4,0,1
; effect
	dc.l	54,6,1,28,6,1,0,0

; WINDOW 236,25,76,81,-1,-1,-1,-1,1,1,1

test_window2:
	dc.l	17,104,620,176
; borders
	dc.l	4,15,15,4,2
; filled	ic,it ( 1 filled, 2 pattern, 3 transp), flags, patnr.
	dc.l	7,1,0,0,0,0,0,8,4,0,1
; effect
; 0,71,10,0,-1,-1,-1,0,0
;	dc.l	4,14,1,-1,6,6,0,0
	dc.l	75,14,1,-1,12,0,0,0

test_window3:
	dc.l	207,53,236,113
; borders
	dc.l	-1,-1,-1,-1,1
; filled
	dc.l	1,3,0,-1,-1,-1,-1,8,4,0,1
; effect
	dc.l	4,10,0,-1,12,4,0,0

	ENDC
*
* The window can be wiped in, in the normal way only adjust the x y position
* and the x,y size.
*
wipe_in_window:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"In wipe in window",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	tst.b	db_screen_wiped(a3)
	bne	oke_scr_w

	bsr	wipe_screen

oke_scr_w:

	move.b	#$0,db_wintriple(a3)	; it will be wiped in 

; First copy to the triple buffer then treat it as a non compressed IFF

	move.l	db_inactive_fileblok(a3),a4
	move.w	db_winsize_x(a3),vb_lenx(a4)
	move.w	db_winsize_y(a3),vb_leny(a4)
	move.w	db_winsize_y(a3),d0
	move.w	db_screen_depth(a3),vb_planes(a4)

	move.l	db_triple_mem(a3),vb_bitdata(a4)    ; screen is in triple mem
	move.l	db_triple_mem(a3),vb_body_start(a4)
	move.b	#0,vb_compression(a4)		    ; indicate no compression
	move.b	#$ff,vb_bitmapped(a4)

	move.b	#0,vb_fastmask(a4)

	moveq	#0,d0
	move.w	db_winsize_x(a3),d0
	add.l	#15,d0
	lsr.l	#4,d0
	add.l	d0,d0
	move.l	d0,vb_breedte_x(a4)
	move.w	vb_planes(a4),d7
	moveq	#0,d2
	mulu	vb_leny(a4),d0
	subq.w	#1,d7
	bmi	no_add
add_pl1:
	add.l	d0,d2
	dbf	d7,add_pl1
no_add:
	move.l	d2,vb_packed_size(a4)
	move.l	d2,vb_unpacked_size(a4)

	move.l	db_window_data(a3),a0
	lea	winp_eff(a0),a0
	move.l	a0,db_effect_data(a3)
	move.l	db_window_data(a3),a0
	cmp.l	#0,winp_sdir(a0)
	bne	.solid2
	cmp.l	#1,winp_it(a0)
	bne	.solid2
	move.b	#$1,vb_fastmask(a4)		; if solid use fastmask
.solid2:	
	moveq	#0,d0
	move.w	db_winpos_x(a3),d0
	ext.l	d0
	move.l	d0,db_blitin_x(a3)		; x pos blitin
	move.w	db_winpos_y(a3),d0
	ext.l	d0
	move.l	d0,db_blitin_y(a3)		; y pos blitin
	move.l	winp_eff+eff_inthick(a0),db_varltimes(a3)
	move.l	#0,db_variation(a3)

	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)
	bsr	wipe_effect_in
	rts

*
* Set given the new window the min max screen values for empty screen wipes
*
set_min_max_values:
	move.l	db_window_data(a3),a0
	move.l	winp_x(a0),d0
	move.l	winp_y(a0),d1
	move.l	d0,d2
	move.l	d1,d3
	add.l	winp_wx(a0),d2
	add.l	winp_wy(a0),d3
	cmp.w	db_clip_maxx(a3),d2
	ble	no_max1
	move.w	d2,db_clip_maxx(a3)
no_max1:
	cmp.w	db_clip_maxy(a3),d3
	ble	no_max2
	move.w	d3,db_clip_maxy(a3)
no_max2:
	cmp.w	db_clip_minx(a3),d0
	bge	no_min1
	move.w	d0,db_clip_minx(a3)
no_min1:	
	cmp.w	db_clip_miny(a3),d1
	bge	no_min2
	move.w	d1,db_clip_miny(a3)
no_min2:
	rts
				
	XDEF	_create_window

_create_window:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	12(a5),a3
	move.l	8(a5),a0			; pointer to parameters

	bsr	create_window

ret_err:
	lea	err1(pc),a0
	move.b	db_quit(a3),(a0)
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	movem.l	a0,-(a7)
	lea	err1(pc),a0
	tst.b	(a0)
	bne	ret_true
	moveq	#0,d0
	movem.l	(a7)+,a0
	rts

ret_true:
	movem.l	(a7)+,a0
	moveq	#1,d0
	rts
	
err1:	dc.w	0

*
* Calculate the bitmap size in bytes for the viewblok (a5)
*
calc_size:
	move.l	vb_breedte_x(a5),d0
	mulu	vb_planes(a5),d0
	mulu	vb_leny(a5),d0
	rts

temp_win_data:	ds.l	32

*
* Create_window creates a window
* Before doing so it wipes the window in the buffer ( if excists )
*
create_window:
	tst.b	db_quit(a3)
	beq	no_quit10
	rts

no_quit10:

; check the effect nr. and if not -1, check if the screen is wiped
; if not do it now
; if so check other windows in the buffer

	tst.b	db_wintriple(a3)
	beq	triple_free		; window is already in the screen

; there is a window in the triple buffer that needs to be wiped in
; do it now

	move.l	a0,-(a7)
	bsr	wipe_in_window
	move.l	(a7)+,a0

; the buffer is empty now, so you can continue with the new window

triple_free:
	tst.b	db_quit(a3)
	beq	no_quit14
	rts

no_quit14:
	tst.b	db_cliptriple(a3)
	beq	no_clipt4
	move.l	a0,-(a7)

	bsr	wipe_clip_in3
	move.l	(a7)+,a0

no_clipt4:

	lea	temp_win_data(pc),a1
	moveq	#28,d0
rep_c_wd:
	move.l	(a0)+,(a1)+
	dbf	d0,rep_c_wd

	lea	temp_win_data(pc),a1
	move.b	#$0,db_skip_cut_woe(a3)

	move.l	a1,db_window_data(a3)		; store new data
	cmp.w	#CUT_WITHOUT_ERASE_NUM,db_screen_wipe(a3)
	bne	.no_stay_screen
	cmp.l	#-1,winp_eff+eff_innum(a1)
	bne	.no_stay_screen

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Window skipped effect -1",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC
	move.b	#$1,db_skip_cut_woe(a3)
	rts

.no_stay_screen:
	clr.b	db_clip_leaved(a3)

	move.l	winp_flags(a1),d0		; total background fill ?????
	and.w	#16,d0
	bne	.back_ground

	bsr	clear_screen

.back_ground:


	lea	temp_win_data(pc),a1
	cmp.l	#-1000,winp_patnr(a1)		; no pattern ?
	beq	.default_patt

; can also be a margin

	cmp.l	#-1000,winp_tm(a1)		; is next end of arguments ?
	beq	.oke_patt			; yes the pattern is oke
						; else check extended format
	cmp.l	#-1000,winp_lm(a1)		; last var is set 
	beq	.default_patt			; it is the patnr
	move.l	winp_lm(a1),winp_patnr(a1)	; set patt nr.
	bra	.oke_patt
.default_patt:					; old style format
	move.l	#8,winp_patnr(a1)		; set to standaard old
.oke_patt:

	cmp.l	#-1000,winp_sdir(a1)		; is there a shadow
	bne	.nos				; either 0 or a direction
	move.l	#0,winp_sdir(a1)		; set shadow on 0
.nos:
	lea	temp_win_data(pc),a0
	move.l	a0,db_window_data(a3)		; store new data
	
	move.b	#$0,db_wininscreen(a3)		; window is in inactive buffer

	move.l	a0,a4
	move.l	winp_eff+eff_innum(a4),d0
	cmp.l	#-1,d0
	beq	no_pro3
	tst.b	db_screen_wiped(a3)
	bne	no_pro3

	movem.l	d0-d7/a0-a6,-(a7)
	bsr	wipe_screen
	movem.l	(a7)+,d0-d7/a0-a6

no_pro3:
	move.w	d0,db_window_wipe(a3)

	lea	temp_win_data(pc),a4
	move.l	db_inactive_viewblok(a3),a5	; use the inactive buffer
	move.l	winp_x(a4),d0
	move.w	d0,db_winpos_x(a3)
	move.l	winp_y(a4),d1
	move.w	d1,db_winpos_y(a3)

;	move.l	winp_wx(a4),d4		; due to memory problems the
;	move.l	winp_wy(a4),d5		; window may be smaller
;	bsr	check_height		; check and adjust
;	ext.l	d4
;	ext.l	d5
;	move.l	d4,winp_wx(a4)		; store the new adjusted values
;	move.l	d5,winp_wy(a4)

	move.l	winp_wx(a4),d0
	move.w	d0,vb_lenx(a5)
	move.w	d0,db_winsize_x(a3)
	add.w	#15,d0
	lsr.l	#4,d0
	add.l	d0,d0
	move.l	d0,vb_breedte_x(a5)
	move.l	winp_wy(a4),d0
	move.w	d0,vb_leny(a5)
	move.w	d0,db_winsize_y(a3)
	move.w	db_screen_depth(a3),vb_planes(a5)	

	moveq	#0,d0
	moveq	#0,d1
	moveq	#0,d2
	move.l	vb_bitmapw(a5),a0
	move.w	vb_planes(a5),d0		; depth
	move.w	vb_lenx(a5),d1
	move.w	vb_leny(a5),d2
	move.l	vb_tempbuffer(a5),d3

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dat(pc),a1
	move.l	d1,(a1)
	move.l	d2,4(a1)
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"x,y %lx,%lx,%lx,%lx",10,0
	even
.dat:	dc.l	0,0,0,0
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	bsr	create_bitmap

* The bitmap is ready now create a Rastport to do some drawing

	move.l	db_graphbase(a3),a6
	lea	win_rp(pc),a1
	jsr	_LVOInitRastPort(a6)

	move.l	vb_bitmapw(a5),a0

	lea	win_rp(pc),a1
	move.l	a0,rp_BitMap(a1)	

* The rastport is ready so now we can to some drawing

	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)

;	cmp.l	#3,winp_it(a4)
;	bne	.do_not_clear
;	bsr	clear_window
	
.do_not_clear:
	move.b	#$1,db_win_noput(a3)		; indicate the window
						; is empty
* set the fill values

	move.w	#0,db_fill_startx(a3)
	move.w	#0,db_fill_starty(a3)
	move.w	db_winsize_x(a3),db_fill_stopx(a3)
	move.w	db_winsize_y(a3),db_fill_stopy(a3)

	bsr	adjust_window_size

	bsr	fill_window

	bsr	create_shadow

* First check the borders

	move.l	winp_tbc(a4),d0
	cmp.l	#-1,d0
	beq	no_top_border

; The Top border

	move.b	#0,db_win_noput(a3)

	lea	win_rp(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVOSetAPen(a6)

	move.l	winp_bw(a4),d7

	subq.l	#1,d7
	move.l	winp_wx(a4),d6			; the width
	add.w	db_fill_startx(a3),d6
	subq.w	#1,d6
	move.w	db_fill_starty(a3),d5		; start line

rep_tborder:
	move.w	db_fill_startx(a3),d0
	move.l	d5,d1
	lea	win_rp(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVOMove(a6)
	move.l	d6,d0
	move.l	d5,d1
	lea	win_rp(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVODraw(a6)
	addq.l	#1,d5
	dbf	d7,rep_tborder

no_top_border:
	move.l	winp_rbc(a4),d0
	cmp.l	#-1,d0
	beq	no_right_border

; The Right border

	move.b	#0,db_win_noput(a3)

	lea	win_rp(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVOSetAPen(a6)

	move.l	winp_bw(a4),d7

	subq.l	#1,d7
	move.l	winp_wx(a4),d5
	move.l	winp_wy(a4),d6		; the width

	add.w	db_fill_startx(a3),d5
	add.w	db_fill_starty(a3),d6

	moveq	#0,d4
	move.w	db_fill_starty(a3),d4
	
	moveq	#0,d3

	cmp.l	#-1,winp_tbc(a4)
	beq	no_tbc2
	move.w	#1,d3
no_tbc2:
	swap	d3

	cmp.l	#-1,winp_bbc(a4)
	beq	no_bbc2
	move.w	#1,d3
no_bbc2:	
	swap	d3
rep_rborder:
	subq.w	#1,d5
	swap	d3
	sub.w	d3,d6

	move.l	d5,d0
	move.l	d4,d1

	lea	win_rp(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVOMove(a6)
	move.l	d5,d0
	move.l	d6,d1

	lea	win_rp(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVODraw(a6)
	swap	d3
	add.w	d3,d4
	dbf	d7,rep_rborder

no_right_border:

	move.l	winp_bbc(a4),d0
	cmp.l	#-1,d0
	beq	no_bot_border

; The Bottom border

	move.b	#0,db_win_noput(a3)

	lea	win_rp(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVOSetAPen(a6)


	moveq	#0,d3
	cmp.l	#-1,winp_rbc(a4)
	beq	no_tbc3
	move.w	#1,d3
no_tbc3:
	swap	d3
	cmp.l	#-1,winp_lbc(a4)
	beq	no_bbc3
	move.w	#1,d3
no_bbc3:
	swap	d3

	move.l	winp_bw(a4),d7

	subq.l	#1,d7
	move.l	winp_wy(a4),d6		; the width
	add.w	db_fill_starty(a3),d6
	moveq	#0,d5			; start line
	move.w	db_fill_startx(a3),d5

	move.l	winp_wx(a4),d4
	add.w	db_fill_startx(a3),d4

rep_bborder:
	subq.w	#1,d6
	sub.w	d3,d4
	swap	d3

	move.l	d5,d0
	move.l	d6,d1
	lea	win_rp(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVOMove(a6)
	move.l	d4,d0
	move.l	d6,d1
	lea	win_rp(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVODraw(a6)
	add.w	d3,d5
	swap	d3
	
	dbf	d7,rep_bborder

no_bot_border:
	move.l	winp_lbc(a4),d0
	cmp.l	#-1,d0
	beq	no_left_border

; The Left border

	move.b	#0,db_win_noput(a3)

	lea	win_rp(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVOSetAPen(a6)

	move.l	winp_bw(a4),d7

	moveq	#0,d3
	cmp.l	#-1,winp_bbc(a4)
	beq	no_tbc4
	move.w	#1,d3
no_tbc4:
	swap	d3
	cmp.l	#-1,winp_tbc(a4)
	beq	no_bbc4
	move.w	#1,d3
no_bbc4:
	swap	d3

	subq.l	#1,d7

	move.w	db_fill_starty(a3),d6
	move.w	db_fill_startx(a3),d5		; start line
	move.l	winp_wy(a4),d4
	add.w	db_fill_starty(a3),d4

rep_lborder:
	sub.w	d3,d4
	swap	d3
	add.w	d3,d6
	swap	d3
	move.l	d5,d0
	move.l	d6,d1
	lea	win_rp(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVOMove(a6)
	move.l	d5,d0
	move.l	d4,d1
	lea	win_rp(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVODraw(a6)
	addq.l	#1,d5
	dbf	d7,rep_lborder
no_left_border:

	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)

	lea	win_rp(pc),a1
	move.l	#0,rp_AreaPtrn(a1)

;	rts

	bsr	copy_window		; copy the window to its destination

	rts

clear_window:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Clearing window 1",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	bsr	calc_size			; get packed size
	move.l	vb_tempbuffer(a5),a0
	lsr.l	#2,d0
.rep_cl1:
	clr.l	(a0)+
	subq.l	#1,d0
	bpl	.rep_cl1
	rts

*
* If there is a shadow the window is actualy smaller
* adjust size here
* also check if the window needs to be cleared
*
adjust_window_size:
	clr.w	db_fill_startx(a3)
	clr.w	db_fill_starty(a3)
	tst.l	winp_sdir(a4)
	beq	.no_shadow

	bsr	clear_window

	move.l	winp_sdep(a4),d0		; horizonatal size
	move.l	d0,d1				; vertical size
	tst.b	db_screen_lace(a3)
	bne	.nol
	add.l	d0,d0				; hor size * twee
.nol:
	move.l	winp_sdir(a4),d2
	cmp.l	#1,d2
	bne	.no1
	sub.l	d0,winp_wx(a4)
	sub.l	d1,winp_wy(a4)
	bra	.no_shadow
.no1:
	cmp.l	#2,d2
	bne	.no2
	add.w	d0,db_fill_startx(a3)
	sub.l	d0,winp_wx(a4)
	sub.l	d1,winp_wy(a4)
	bra	.no_shadow
.no2:
	cmp.l	#3,d2
	bne	.no3
	add.w	d0,db_fill_startx(a3)
	add.w	d1,db_fill_starty(a3)
	sub.l	d0,winp_wx(a4)
	sub.l	d1,winp_wy(a4)
	bra	.no_shadow
.no3:	
	cmp.l	#4,d2
	bne	.no_shadow
	add.w	d1,db_fill_starty(a3)
	sub.l	d0,winp_wx(a4)
	sub.l	d1,winp_wy(a4)
.no_shadow:
	rts

create_shadow:
	tst.l	winp_sdir(a4)
	beq	.no_shadow

	move.b	#0,db_win_noput(a3)		; window is not empty
	move.l	winp_spen(a4),d0
	lea	win_rp(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVOSetAPen(a6)			; set shadow color
	bsr	set_window_pattern

	move.l	winp_sdep(a4),d6		; horizonatal size
	move.l	d6,d7				; vertical size
	tst.b	db_screen_lace(a3)
	bne	.nol
	add.l	d6,d6				; hor size * twee
.nol:
	move.l	winp_sdir(a4),d5
	cmp.l	#1,d5
	bne	.no1
	move.l	winp_wx(a4),d0
	move.l	d7,d1
	moveq	#0,d2
	add.w	db_winsize_x(a3),d2
	subq.w	#1,d2
	moveq	#0,d3
	add.w	db_winsize_y(a3),d3
	lea	win_rp(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVORectFill(a6)

	move.l	d6,d0				; + sdepth_lr
	move.l	winp_wy(a4),d1
	move.l	winp_wx(a4),d2
	moveq	#0,d3
	add.w	db_winsize_y(a3),d3
	subq.w	#1,d3
	lea	win_rp(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVORectFill(a6)
	bra	.no_shadow

.no1:
	cmp.l	#2,d5
	bne	.no2
	moveq	#0,d0
	move.l	d7,d1
	moveq	#0,d3
	move.l	d6,d2
	add.w	db_winsize_y(a3),d3
	subq.w	#1,d3
	lea	win_rp(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVORectFill(a6)

	moveq	#0,d0
	move.l	winp_wy(a4),d1
	moveq	#0,d2
	moveq	#0,d3
	move.w	db_winsize_x(a3),d2
	sub.l	d6,d2
	move.w	db_winsize_y(a3),d3
	subq.w	#1,d3
	lea	win_rp(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVORectFill(a6)

.no2:
	cmp.l	#3,d5
	bne	.no3

	moveq	#0,d0
	moveq	#0,d1
	moveq	#0,d2
	move.w	db_winsize_x(a3),d2
	sub.l	d6,d2
	move.l	d7,d3
	lea	win_rp(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVORectFill(a6)

	moveq	#0,d0
	move.l	d7,d1
	moveq	#0,d3
	move.l	d6,d2
	move.w	db_winsize_y(a3),d3
	sub.l	d7,d3
	lea	win_rp(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVORectFill(a6)

.no3:
	cmp.l	#4,d5
	bne	.no_shadow

	move.l	d6,d0
	moveq	#0,d1
	moveq	#0,d2
	move.w	db_winsize_x(a3),d2
	move.l	d7,d3
	lea	win_rp(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVORectFill(a6)

	move.l	winp_wx(a4),d0
	move.l	d7,d1
	moveq	#0,d2
	move.w	db_winsize_x(a3),d2
	moveq	#0,d3
	move.w	db_winsize_y(a3),d3
	sub.l	d7,d3
	lea	win_rp(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVORectFill(a6)
.no_shadow:
	rts

fill_window:
	move.l	winp_ic(a4),d0
	lea	win_rp(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVOSetAPen(a6)

	cmp.l	#1,winp_it(a4)
	bne	win_not_filled

	move.b	#0,db_win_noput(a3)
fill_it:
	lea	win_rp(pc),a1
	moveq	#0,d0
	moveq	#0,d1
	moveq	#0,d2
	moveq	#0,d3
	move.w	db_fill_startx(a3),d0
	move.w	db_fill_starty(a3),d1
	move.l	d0,d2
	move.l	d1,d3
	add.l	winp_wx(a4),d2
	add.l	winp_wy(a4),d3
	subq.l	#1,d2
	subq.l	#1,d3
	move.l	db_graphbase(a3),a6
	jsr	_LVORectFill(a6)
	bra	win_filled

win_not_filled:
	cmp.l	#2,winp_it(a4)
	bne	win_not_patterned
	bsr	set_window_pattern
	bra	fill_it
win_not_patterned:			; check if there are borders
					; if so clear the window after all
	tst.l	winp_sdir(a4)
	bne	win_filled
	moveq	#-1,d0
	cmp.l	winp_tbc(a4),d0
	bne	.clear
	cmp.l	winp_rbc(a4),d0
	bne	.clear
	cmp.l	winp_lbc(a4),d0
	bne	.clear
	cmp.l	winp_bbc(a4),d0
	bne	.clear
	rts
.clear:
	bsr	clear_window
win_filled:
	rts

set_window_pattern:
	move.b	#0,db_win_noput(a3)
	lea	win_rp(pc),a1
	move.b	#3,rp_AreaPtSz(a1)
	move.l	db_horizontalmask(a3),a0
	move.l	a0,rp_AreaPtrn(a1)
	move.l	db_patterns(a3),a1

	move.l	winp_patnr(a4),d0
	cmp.w	#21,d0
	ble	.oke
	moveq	#0,d0
.oke
	add.l	d0,d0
	move.l	d0,d1
	lsl.l	#3,d0
	add.l	d1,d0
	add.l	d0,a1

	IFEQ	XAPP
	lea	pats(pc),a1
	ENDC
	tst.b	db_screen_lace(a3)
	bne	double_pat

	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+
	rts
	
double_pat:
	moveq	#7,d7
.rpp:
	move.w	(a1),(a0)+
	move.w	(a1)+,(a0)+
	dbf	d7,.rpp

	lea	win_rp(pc),a1
	move.b	#4,rp_AreaPtSz(a1)
	move.l	db_horizontalmask(a3),a0
	move.l	a0,rp_AreaPtrn(a1)
	rts
	
	IFEQ	XAPP
pats:
	dc.w	$aaaa
	dc.w	$5555
	dc.w	$aaaa
	dc.w	$5555
	dc.w	$aaaa
	dc.w	$5555
	dc.w	$aaaa
	dc.w	$5555
	ENDC

win_rp:	ds.b	rp_SIZEOF

	IFEQ XAPP
test_colors1:	dc.l	1,col1
test_colors2:	dc.l	2,col2
test_colors3:	dc.l	3,col3
test_colors4:	dc.l	4,col4
test_colors5:	dc.l	5,col5
test_colors6:	dc.l	6,col6
test_colors7:	dc.l	7,col7
test_colors8:	dc.l	8,col8
test_colors9:	dc.l	9,col9
test_colors10:	dc.l	10,col10
test_colors11:	dc.l	11,col11
test_colors12:	dc.l	12,col12
test_colors13:	dc.l	13,col13
test_colors14:	dc.l	14,col14
test_colors15:	dc.l	15,col15
test_colors16:	dc.l	16,col16

col1:	dc.b	"d6d6d6 c4c4bf ffffff ff7171 e35c00 5465e2 0000ee 838a88 "
col2:	dc.b	"2a3be2 00a400 9b0000 733c00 45484c 000070 3f0000 000000 "

col3:	dc.b	"ffff00 cccc00 999900 666600 00ffff 00cccc 009999 006666 "
col4:	dc.b	"ff00ff cc00cc 990099 660066 ffffff aaaaaa 555555 000000 "
col5:	dc.b	"ffffff 888888 ffffff cccccc 444444 ffffff ffffff 88ffff "
col6:	dc.b	"448888 ccffff 66cccc 224444 aaffff eeffff ccffff 668888 "
col7:	dc.b	"ffffff 99cccc 334444 ffffff ffffff 44ffff 228888 66ffff "
col8:	dc.b	"33cccc 114444 55ffff 77ffff ff88ff 884488 ffccff cc66cc "
col9:	dc.b	"442244 ffaaff ffeeff 8888ff 444488 ccccff 6666cc 222244 "
col10:	dc.b	"aaaaff eeeeff cc88ff 664488 ffccff 9966cc 332244 ffaaff "
col11:	dc.b	"ffeeff 4488ff 224488 66ccff 3366cc 112244 55aaff 77eeff "
col12:	dc.b	"ffccff 886688 ffffff cc99cc 443344 ffffff ffffff 88ccff "
col13:	dc.b	"446688 ccffff 6699cc 223344 aaffff eeffff ccccff 666688 "
col14:	dc.b	"ffffff 9999cc 333344 ffffff ffffff 44ccff 226688 66ffff "
col15:	dc.b	"3399cc 113344 55ffff 77ffff ff44ff 882288 ff66ff cc33cc "
col16:	dc.b	"441144 ff55ff ff77ff 8844ff 442288 cc66ff 6633cc 221144 "

;col1:	dc.b	"116699 001111 ffeedd aa99cc ff0000 cc0000 990000 660000 "
;col2:	dc.b	"00ff00 00cc00 009900 006600 0000ff 0000cc 000099 000066 "
;col3:	dc.b	"ffff00 cccc00 999900 666600 00ffff 00cccc 009999 006666 "
;col4:	dc.b	"ff00ff cc00cc 990099 660066 ffffff aaaaaa 555555 000000 "

	even

test_set_colors:
	lea	test_colors1(pc),a0
	bsr	set_colors
	lea	test_colors2(pc),a0
	bsr	set_colors
	lea	test_colors3(pc),a0
	bsr	set_colors
	lea	test_colors4(pc),a0
	bsr	set_colors
	lea	test_colors5(pc),a0
	bsr	set_colors
	lea	test_colors6(pc),a0
	bsr	set_colors
	lea	test_colors7(pc),a0
	bsr	set_colors
	lea	test_colors8(pc),a0
	bsr	set_colors
	rts
	lea	test_colors9(pc),a0
	bsr	set_colors
	lea	test_colors10(pc),a0
	bsr	set_colors
	lea	test_colors11(pc),a0
	bsr	set_colors
	lea	test_colors12(pc),a0
	bsr	set_colors
	lea	test_colors13(pc),a0
	bsr	set_colors
	lea	test_colors14(pc),a0
	bsr	set_colors
	lea	test_colors15(pc),a0
	bsr	set_colors
	lea	test_colors16(pc),a0
	bsr	set_colors
	rts
	ENDC	
sexit:
	bra	sexit2

	XDEF	_set_colors
_set_colors:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	12(a5),a3
	move.l	8(a5),a0			; pointer to parameters

	bsr	set_colors

	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

set_colors:
	move.l	(a0),d0
	subq.l	#1,d0
	lsl.l	#3,d0
	move.l	d0,d1
	add.l	d1,d1
	add.l	d0,d1
	lea	db_cop_colorsLOF(a3),a1
	add.l	d1,a1
	move.l	4(a0),a2		; pointer to asci color data
	moveq	#7,d7			; 8 colors
rep_get_acol:
	bsr	get_acol
	move.b	d0,(a1)+
	bsr	get_acol
	move.b	d0,(a1)+
	bsr	get_acol
	move.b	d0,(a1)+
	addq.l	#1,a2			; skip space
	dbf	d7,rep_get_acol
	rts	

get_acol:
	move.b	(a2)+,d1
	cmp.b	#'9',d1
	ble	oke_num
	sub.b	#'a',d1
	add.b	#'0'+10,d1
oke_num:	
	sub.b	#'0',d1

	move.b	(a2)+,d0
	cmp.b	#'9',d0
	ble	oke_num2
	sub.b	#'a',d0
	add.b	#'0'+10,d0
oke_num2:	
	sub.b	#'0',d0
	and.b	#$f,d1
	lsl.b	#4,d1
	and.b	#$f,d0
	or.b	d1,d0
	rts

	IFNE DEBUG
dbstr400:	dc.b	"in create screen PAL = %d",10,0
dbstr401:	dc.b	"in create window",10,0
dbstr4023:	dc.b	"Creating triple",10,0
dbstr4024:	dc.b	"Doing fastmask",10,0
dbstr4028:	dc.b	"Create screen",10,0
dbstr4029:	dc.b	"Screen to big",10,0
dbstr4030:	dc.b	"Screen to big reducing",10,0
dbstr4031:	dc.b	"Checking aa sizes",10,0
dbstr4032:	dc.b	"Adapting mask",10,0
dbstr4042:	dc.b	"Nr off cols is %d",10,0
dbstr5003:	dc.b	"X coord is %d, Y coord is %d",10,0
dbstr5023:	dc.b	"Hard move",10,0
dbstr5001:	dc.b	"Wipe text in",10,0
dbstr5004:	dc.b	"Masking window out",10,0
	even
	ENDC
	
	XDEF	_create_screen
_create_screen:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	12(a5),a3
	move.l	8(a5),a0			; pointer to parameters
	move.l	16(a5),a1			; pointer to script pars

	move.w	4(a1),db_screen_wipe(a3)
	move.w	6(a1),db_screen_effsp(a3)
	move.w	8(a1),db_screen_varl(a3)
	move.w	10(a1),db_screen_var(a3)

	move.b	#$0,db_screen_wiped(a3)

	move.b	#1,db_cyc_quit(a3)		; no color cycling

	move.l	a0,-(a7)
	bsr	set_lace

	bsr	setsam
	tst.b	d0
	bne	no_bits

	bsr	install_50h
	lea	sigmask(pc),a0
	move.l	(a0),d0
	move.l	d0,db_waitmasker(a3)

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	dbstr4028(pc),a0
	jsr	KPutStr
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC
	move.l	(a7)+,a0

	bsr	create_screen
	tst.w	d0
	bne	no_doc			; screen to big

	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

no_bits:
	move.l	(a7)+,a0
exit_cs:
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#1,d0
	rts

no_doc:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	dbstr4029(pc),a0
	jsr	KPutStr
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.b	#1,db_screen_wiped(a3)
	
	lea	docn(pc),a0
	moveq	#ERR_NO_MEM,d0
	bsr	copy_err_stub
	bra	exit_cs

docn:	dc.b	"Document",0
	even

	IFEQ XAPP
test_screen:	dc.l	4,0,4,0,-1000,-1000
;test_screen:	dc.l	640,512,4,0,0,4,-1000,-1000
	ENDC

*
* Create a blank screen with parameters ( longs ) in ( a0 )
* a0 -> mode1, mode2, depth, mode3
*
create_screen:
	IFEQ	XAPP
	move.w	#0,db_screen_wipe(a3)
	move.w	#10,db_screen_effsp(a3)
	move.w	#0,db_screen_var(a3)
	move.w	#0,db_screen_varl(a3)
	ENDC
	
	move.b	#$0,db_screen_cleared(a3)

	move.b	#0,db_should_wait(a3)
	move.w	#-10000,db_clip_maxx(a3)		; screen is empty
	move.w	#-10000,db_clip_maxy(a3)
	move.w	#10000,db_clip_minx(a3)
	move.w	#10000,db_clip_miny(a3)
	move.b	#$ff,db_screen_empty(a3)
	move.w	#$0100,db_blitin_clear(a3)

	move.b	#$0,db_screen_wiped(a3)
	move.b	#$0,db_wintriple(a3)
	move.b	#$ff,db_wininscreen(a3)		    ; window is in screen ( d )
	move.l	db_inactive_fileblok(a3),a5
	move.l	db_triple_mem(a3),vb_bitdata(a5)    ; screen is in triple mem
	move.l	db_triple_mem(a3),vb_body_start(a5)
	move.b	#0,vb_compression(a5)		    ; indicate no compression
	move.b	#$ff,vb_bitmapped(a5)

	cmp.l	#-1000,16(a0)			; is this old version
	beq	.no_fix
	move.l	(a0),d0

	move.l	#0,db_dummy_width(a3)
	
	move.w	d0,vb_lenx(a5)
	cmp.l	#-1000,20(a0)
	beq	.no_fix
	move.l	4(a0),d0
	move.w	d0,vb_leny(a5)
	move.l	20(a0),(a0)
	move.l	12(a0),4(a0)
	
	bra	get_mode

.no_fix:
	cmp.l	#4,(a0)
	ble	lo_or_hi
;						screen is super or vga
	cmp.l	#5,(a0)
	bne	no_super
	move.w	#HOR_SUPER,vb_lenx(a5)
	move.w	#VER_SUPER,vb_leny(a5)
	bra	get_mode	
no_super:
	cmp.l	#6,(a0)
	bne	no_super_lace
	move.w	#HOR_SUPER,vb_lenx(a5)
	move.w	#VER_SUPER_LACE,vb_leny(a5)
	bra	get_mode	
no_super_lace:	
	cmp.l	#7,(a0)
	bne	no_vga
	move.w	#HOR_VGA,vb_lenx(a5)
	move.w	#VER_VGA,vb_leny(a5)

	bra	get_mode	
no_vga:
	cmp.l	#8,(a0)
	bne	no_vga_lace
	move.w	#HOR_VGA,vb_lenx(a5)
	move.w	#VER_VGA_LACE,vb_leny(a5)
	bra	get_mode	

no_vga_lace:					; mode not defined
	move.w	#HOR_HIRES,vb_lenx(a5)
	move.w	#VER_PAL,vb_leny(a5)
	bra	get_mode	

lo_or_hi:
	bsr	scr_table

get_mode:
	move.l	(a0),d7
	subq.l	#1,d7
	and.l	#7,d7
	add.l	d7,d7
	lea	screen_modes(pc),a1
	move.w	0(a1,d7.l),d0
	move.w	0(a1,d7.l),vb_mode(a5)
	cmp.l	#1,4(a0)
	bne	no_ham_mode
	or.w	#V_HAM,vb_mode(a5)
no_ham_mode:
	cmp.l	#2,4(a0)
	bne	no_ehb
	or.w	#V_EXTRA_HALFBRITE,vb_mode(a5)
no_ehb:
	move.b	#0,db_screen_lace(a3)
	move.w	vb_mode(a5),d0
	and.w	#V_LACE,d0
	beq	no_lace2
	move.b	#$ff,db_screen_lace(a3)
	move.b	#$ff,vb_interlace(a5)
no_lace2:

	move.w	vb_mode(a5),d0
	and.w	#V_HIRES,d0
	beq	no_hires2
	move.b	#$ff,vb_hires(a5)
no_hires2:

	move.w	vb_mode(a5),d0
	and.w	#V_SUPERHIRES,d0
	beq	no_shires2
	move.b	#$ff,vb_shires(a5)
no_shires2:

; For the large blits add a word extra to the x size

test_again:	
	moveq	#0,d0
	move.w	vb_lenx(a5),d0
	add.w	#15,d0
	lsr.l	#4,d0
	add.l	d0,d0
	move.l	d0,d1

	add.l	#7,d0
	lsr.l	#3,d0
	lsl.l	#3,d0

	move.l	d0,vb_breedte_x(a5)
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	vb_breedte_x(a5),a1
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"Width screen %ld",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC


	move.l	d0,d1
	lsl.l	#3,d0
	move.w	d0,vb_lenx(a5)

	move.l	8(a0),d0
	move.w	d0,vb_planes(a5)
	mulu	d0,d1
	mulu	vb_leny(a5),d1

	move.l	d1,vb_unpacked_size(a5)

	move.l	db_mlsysstruct(a3),a1
	IFNE	XAPP
	move.l	sbu_Size(a1),d0
	ELSE
	move.l	#MEMSIZE,d0
	ENDC
	cmp.l	d1,d0
	blt	doc_to_big1

	move.w	vb_planes(a5),d1
	moveq	#1,d0
	lsl.l	d1,d0
	move.l	d0,d1
	move.l	d0,d2
	add.l	d0,d0
	add.l	d1,d0

	move.w	vb_mode(a5),d1
	and.w	#V_HAM,d1
	beq	no_ham23
	moveq	#30,d0
	tst.b	db_aa_present(a3)
	beq	no_ham23
	move.l	#192,d0
no_ham23:
	move.l	d0,vb_color_count(a5)

	and.l	#$ff,d2				; not more than 256
	lea	db_cop_colorsLOF(a3),a2		; use as temporary storage
	subq.l	#2,d2
	bmi	no_col_clear
	move.b	#0,(a2)+
	move.b	#0,(a2)+
	move.b	#0,(a2)+
rep_col_clear:
	move.b	#$ff,(a2)+
	move.b	#$ff,(a2)+
	move.b	#$ff,(a2)+
	dbf	d2,rep_col_clear
no_col_clear:
	move.w	vb_lenx(a5),db_screen_x(a3)
	move.w	vb_leny(a5),db_screen_y(a3)
	move.w	vb_planes(a5),db_screen_depth(a3)
	move.w	vb_mode(a5),db_screen_mode(a3)

	IFNE	XAPP
wpp:
	cmp.w	#CUT_WITHOUT_ERASE_NUM,db_screen_wipe(a3)
	bne	no_stay_screen

	move.w	#10000,db_clip_maxx(a3)		; screen is empty
	move.w	#10000,db_clip_maxy(a3)
	move.w	#-10000,db_clip_minx(a3)
	move.w	#-10000,db_clip_miny(a3)
	move.b	#$0,db_screen_empty(a3)

	move.b	#$ff,db_screen_wiped(a3)
	move.b	#$0,db_wintriple(a3)
	move.b	#$0,db_wininscreen(a3)		    ; window is in screen ( d )
	move.b	#$1,db_screen_cleared(a3)

	bsr	get_active_info

	moveq	#0,d0
	rts
	
no_stay_screen:
	ENDC

	IFNE XAPP
	bsr	get_active_info
	move.b	#$0,db_screen_cleared(a3)
	ENDC
	move.l	db_inactive_fileblok(a3),a5
	moveq	#0,d0
	rts

clear_screen:

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	db_screen_cleared(a3),a1
	lea	.dbstr2(pc),a0
	jsr	KPutFmt
	bra	.tt2
.dbstr2:	dc.b	"Want to clear %d",10,0
	even
.tt2:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	tst.b	db_screen_cleared(a3)
	bne	.no_cl

	movem.l	d0/a1,-(a7)
	moveq	#0,d0
	move.w	db_screen_x(a3),d0
	add.w	#15,d0
	lsr.l	#4,d0
	add.w	d0,d0
	mulu	db_screen_depth(a3),d0
	mulu	db_screen_y(a3),d0

	IFNE PRAND
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.temp(pc),a1
	move.l	d0,(a1)
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"Clear screen %lx",10,0
	even
.temp:	dc.l	0
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	lsr.l	#2,d0
	move.l	db_triple_mem(a3),a1
.clr_tri:
	clr.l	(a1)+
	subq.l	#1,d0
	bpl	.clr_tri
	move.b	#1,db_screen_cleared(a3)
	movem.l	(a7)+,a1/d0
.no_cl:
	rts

doc_to_big1:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	dbstr4030(pc),a0
	jsr	KPutStr
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	sub.w	#10,vb_leny(a5)			; reduce height
	bra	test_again

doc_to_big:
	moveq	#1,d0
	rts

scr_table:
	move.w	#0,vb_mode(a5)
	lea	screen_modes_table(pc),a1
	move.l	(a0),d7
	subq.l	#1,d7
	lsl.l	#2,d7
	or.l	12(a0),d7
	lsl.l	#1,d7
	move.w	0(a1,d7.w),d0
	move.w	d0,vb_lenx(a5)

	lea	screen_modes_table_ver_pal(pc),a1
	tst.b	db_pal(a3)
	bne	oke_pal1
	lea	screen_modes_table_ver_ntsc(pc),a1

oke_pal1:
	move.l	(a0),d7
	subq.l	#1,d7
	lsl.l	#2,d7
	or.l	12(a0),d7
	lsl.l	#1,d7
	move.w	0(a1,d7.w),d0
	move.w	d0,vb_leny(a5)

	rts

*
* Create an interleaved bitmap
* a0 - pointer to the bitmap
* d0 - depth
* d1 - width
* d2 - height
* d3 - plane memory
*
create_bitmap_leaved:
	movem.l	a0/a1/a4/d0-d7,-(a7)

	move.l	a0,a4				; save bitmap pointer
	move.w	d1,d7
	add.w	#15,d7
	lsr.l	#4,d7
	add.l	d7,d7
	move.l	d7,d1				; width in bytes * 8
	addq.w	#1,d0				; depth+1 interleaved with mask
	mulu	d0,d1
	lsl.l	#3,d1				; width
	subq.w	#1,d0
	bra	in_cbm
	
create_bitmap:
	movem.l	a0/a1/a4/d0-d7,-(a7)
	moveq	#0,d7
	move.l	a0,a4				; save bitmap pointer
	move.w	d1,d7
	add.w	#15,d7
	lsr.l	#4,d7
	add.l	d7,d7
	move.l	d7,d1				; width in bytes * 8
	mulu	d0,d1
	lsl.l	#3,d1				; width
in_cbm:
	move.l	db_graphbase(a3),a6
	jsr	_LVOInitBitMap(a6)

	move.l	a4,a0
	move.l	d3,d0

	move.l	d0,bm_Planes(a0)		; set the bitplanes
	add.l	d7,d0
	move.l	d0,bm_Planes+4(a0)
	add.l	d7,d0
	move.l	d0,bm_Planes+8(a0)
	add.l	d7,d0
	move.l	d0,bm_Planes+12(a0)
	add.l	d7,d0
	move.l	d0,bm_Planes+16(a0)
	add.l	d7,d0
	move.l	d0,bm_Planes+20(a0)
	add.l	d7,d0
	move.l	d0,bm_Planes+24(a0)
	add.l	d7,d0
	move.l	d0,bm_Planes+28(a0)
	movem.l	(a7)+,a0/a1/a4/d0-d7
	rts

screen_modes:
	dc.w	0,V_LACE,V_HIRES,V_HIRES+V_LACE

	dc.w	V_SUPERHIRES+V_HIRES, V_SUPERHIRES+V_HIRES+V_LACE

	dc.w	V_HIRES, V_HIRES+V_LACE

screen_modes_table:
	dc.w	HOR_LORES,HOR_LORES_STD,HOR_LORES_MAX,HOR_LORES_MAX
	dc.w	HOR_LORES,HOR_LORES_STD,HOR_LORES_MAX,HOR_LORES_MAX
	dc.w	HOR_HIRES,HOR_HIRES_STD,HOR_HIRES_MAX,HOR_HIRES_MAX
	dc.w	HOR_HIRES,HOR_HIRES_STD,HOR_HIRES_MAX,HOR_HIRES_MAX

screen_modes_table_ver_pal:
	dc.w	VER_PAL,VER_PAL_STD,VER_PAL_MAX,VER_PAL_MAX
	dc.w	VER_PAL_LACE,VER_PAL_STD_LACE,VER_PAL_MAX_LACE,VER_PAL_MAX_LACE

	dc.w	VER_PAL,VER_PAL_STD,VER_PAL_MAX,VER_PAL_MAX
	dc.w	VER_PAL_LACE,VER_PAL_STD_LACE,VER_PAL_MAX_LACE,VER_PAL_MAX_LACE

screen_modes_table_ver_ntsc:
	dc.w	VER_NTSC,VER_NTSC_STD,VER_NTSC_MAX,VER_NTSC_MAX
	dc.w	VER_NTSC_LACE,VER_NTSC_STD_LACE,VER_NTSC_MAX_LACE,VER_NTSC_MAX_LACE

	dc.w	VER_NTSC,VER_NTSC_STD,VER_NTSC_MAX,VER_NTSC_MAX
	dc.w	VER_NTSC_LACE,VER_NTSC_STD_LACE,VER_NTSC_MAX_LACE,VER_NTSC_MAX_LACE

filter_monitor_id:
	move.w	#0,db_alternate_lowres(a3)
	move.l	db_monid(a3),d1
	and.l	#MONITOR_ID_MASK,d1
	move.l	d1,db_monid(a3)
	cmp.l	#SUPER72_MONITOR_ID,d1
	bne	.no72
	or.l	#SUPER_KEY,db_monid(a3)
	move.w	#HIRES_KEY,db_alternate_lowres(a3)
.no72:
	cmp.l	#EURO72_MONITOR_ID,d1
	bne	.noe72
	or.l	#SUPER_KEY,db_monid(a3)
	move.w	#HIRES_KEY,db_alternate_lowres(a3)
.noe72:
	cmp.l	#VGA_MONITOR_ID,d1
	bne	.nov72
	or.l	#SUPER_KEY,db_monid(a3)
	move.w	#HIRES_KEY,db_alternate_lowres(a3)
.nov72:
	rts


	XDEF	_pass_mlsystem
*
* haal de info uit de mlsystem struct
*
_pass_mlsystem:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	12(a5),a3

	move.b	#1,db_set_error(a3)
	move.b	#0,db_cycle_wait(a3)
	move.b	#0,db_give_error(a3)
	move.l	a7,db_easy_exit(a3)
	move.b	#0,db_stay_on(a3)

	move.l	8(a5),a0			; pointer naar mlsystem
	move.l	a0,db_mlsysstruct(a3)

	move.l	ml_miscFlags(a0),d0
	btst	#6,d0
	beq	.nostay
	move.b	#1,db_stay_on(a3)
.nostay:

	move.l	#0,db_dummy_width(a3)

	move.b	#0,db_sema_on(a3)

	lea	ml_sema_trans(a0),a0
	move.l	$4.w,a6
	jsr	_LVOObtainSemaphore(a6)

	move.b	#$ff,db_sema_on(a3)

	move.l	20(a5),d0
	move.l	d0,db_eff_nums(a3)

	clr.l	db_go_on_key_sig(a3)
	move.l	24(a5),d0
	beq	.nodoc
	moveq	#1,d1
	lsl.l	d0,d1
	move.l	d1,db_go_on_key_sig(a3)
.nodoc:
	move.l	28(a5),d0
	move.l	d0,db_patterns(a3)
	move.l	16(a5),a1

	move.l	a1,db_msgpointer(a3)
	moveq	#0,d0
	move.b	MP_SIGBIT(a1),d0
	moveq	#1,d1
	lsl.l	d0,d1
	move.l	d1,db_sig_ptoc(a3)

	move.l	db_mlsysstruct(a3),a0
	move.l	ml_taglist(a0),db_tags(a3)

	move.l	ml_monitor_ID(a0),db_monid(a3)

	move.l	sbu_Base(a0),d0				; 1e base
	move.l	d0,db_unpacked1(a3)
	move.l	sbu_Size(a0),db_unpacked1_size(a3)	; 1e size

	move.l	sbu_SIZEOF+sbu_Base(a0),d0		; 2e base
	move.l	d0,db_unpacked2(a3)
	move.l	sbu_SIZEOF+sbu_Size(a0),db_unpacked2_size(a3)	; 2e size

	move.l	sbu_SIZEOF*2+sbu_Base(a0),d0		; 3e base
	beq	no_triple55

	move.l	d0,db_triple_mem(a3)
	move.l	sbu_SIZEOF*2+sbu_Size(a0),db_triple_size(a3)

	tst.w	sbu_SIZEOF*2+sbu_Viewed(a0)
	beq	no_change_buffers		; triple is on display
						; switch triple en unpacked2

	move.l	sbu_SIZEOF*2+sbu_Size(a0),db_unpacked2_size(a3)
	move.l	sbu_SIZEOF*2+sbu_Base(a0),db_unpacked2(a3)

	move.l	sbu_SIZEOF+sbu_Size(a0),db_triple_size(a3)
	move.l	sbu_SIZEOF+sbu_Base(a0),db_triple_mem(a3)

no_change_buffers:
	move.b	#$ff,db_triple_present(a3)

no_triple55:
	move.b	#0,db_quit(a3)
	move.b	#0,db_easy_found(a3)

	bsr	openlibs

	tst.l	db_graphbase(a3)
	beq	libs_not_open

	lea	initialised(pc),a0
	tst.b	(a0)
	bne	no_init_this_time2
	move.b	#$ff,(a0)

	bsr	init_jump_tabel

 	bsr.w	no_triple1		; haal de rest van het geheugen

	jsr	create_tabels

	bsr	setprefs
	bsr	filter_monitor_id
	bsr	init_cop_vports

	bsr	copy_local_global

	bra	already_init

no_init_this_time2:
	bsr	copy_global_local
	move.l	in_chipscrap_mem(pc),d0
	cmp.l	db_monid(a3),d0
	beq	.no_prefs
	bsr	setprefs
	bsr	filter_monitor_id
	lea.l	in_chipscrap_mem(pc),a0
	move.l	db_monid(a3),(a0)
	lea	in_max_Depth_Lores(pc),a0
	move.w	db_max_Depth_Lores(a3),(a0)+
	move.w	db_max_Depth_Hires(a3),(a0)+
	move.w	db_max_Depth_SHires(a3),(a0)+
	bra	already_init
.no_prefs:
	bsr	get_oscan_vals
	bsr	set_and_or_vmode

already_init:

	bsr	init_datablock

	move.b	#0,db_sema_on(a3)
	move.l	db_mlsysstruct(a3),a0
	lea	ml_sema_trans(a0),a0
	move.l	$4.w,a6
	jsr	_LVOReleaseSemaphore(a6)

nothing:
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

	IFEQ XAPP
temp_tags:
	dc.l	VTAG_ATTACH_CM_SET,0
	dc.l	VTAG_VIEWPORTEXTRA_SET,0
	dc.l	VTAG_NORMAL_DISP_SET,0
	
	dc.l	VTAG_BORDERNOTRANS_CLR,0
	dc.l	VTAG_CHROMAKEY_SET,0
	dc.l	VTAG_CHROMA_PEN_SET,0
	dc.l	VTAG_END_CM,0

; as it was

	dc.l	VTAG_BORDERBLANK_CLR,0
	dc.l	VTAG_CHROMAKEY_SET,0
	dc.l	VTAG_CHROMA_PEN_CLR,0
	dc.l	VTAG_END_CM,0

; case 0 normal color 0 is video

	dc.l	VTAG_BORDERNOTRANS_CLR,0
	dc.l	VTAG_CHROMAKEY_SET,0
	dc.l	VTAG_CHROMA_PEN_SET,0
	dc.l	VTAG_END_CM,0

; case 1

; case 2

	dc.l	VTAG_BORDERBLANK_CLR,0
	dc.l	VTAG_CHROMAKEY_SET,0
	dc.l	VTAG_CHROMA_PEN_CLR,0
	dc.l	VTAG_END_CM,0

; case 3

	dc.l	VTAG_BORDERBLANK_SET,0
	dc.l	VTAG_CHROMAKEY_SET,0
	dc.l	VTAG_CHROMA_PEN_CLR,0
	dc.l	VTAG_END_CM,0

; case 4
	dc.l	VTAG_BORDERBLANK_CLR,0
	dc.l	VTAG_BORDERNOTRANS_SET,0
	dc.l	VTAG_CHROMAKEY_SET,0
	dc.l	VTAG_CHROMA_PEN_SET,0
	dc.l	VTAG_END_CM,0

	ENDC

libs_not_open:
	move.b	#0,db_sema_on(a3)
	move.l	db_mlsysstruct(a3),a0
	lea	ml_sema_trans(a0),a0
	move.l	$4.w,a6
	jsr	_LVOReleaseSemaphore(a6)

	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#20,d0
	rts

initialised:		dc.b	0
jump_initialised:	dc.b	0

*
* copy the local resident data to the global data
*
copy_local_global:
	lea	in_chipscrap_mem(pc),a0
	move.l	db_monid(a3),(a0)+
	move.l	db_chipscrap_mem(a3),(a0)+
	move.l	db_chipscrap_size(a3),(a0)+
	move.l	db_fastscrap_mem(a3),(a0)+
	move.l	db_fastscrap_size(a3),(a0)+
	move.l	db_conhandle(a3),(a0)+
	move.l	db_oldview(a3),(a0)+
	move.l	db_welkmasker(a3),(a0)+
	move.l	db_horizontalmask(a3),(a0)+
	move.l	db_oldmask(a3),(a0)+
	move.l	db_wpatroon_tabel(a3),(a0)+
	move.l	db_wpatroonruimte(a3),(a0)+
	move.l	db_wstore_moves2(a3),(a0)+
	move.w	db_gapsize(a3),(a0)+
	move.w	db_max_Depth_Lores(a3),(a0)+
	move.w	db_max_Depth_Hires(a3),(a0)+
	move.w	db_max_Depth_SHires(a3),(a0)+
	move.b	db_aa_present(a3),(a0)+
	move.b	db_V39_present(a3),(a0)+
	move.b	db_pal(a3),(a0)+
	move.b	#0,(a0)+
	move.b	db_Xoff(a3),(a0)+
	move.b	db_Yoff(a3),(a0)+
	move.l	db_waarcolors1(a3),(a0)+
	move.l	db_waarcolors2(a3),(a0)+
	move.l	db_waarcolors32(a3),(a0)+
	move.l	db_waardeltacol1(a3),(a0)+
	move.l	db_waardeltacol2(a3),(a0)+
	move.l	db_waarviewblok1(a3),(a0)+
	move.l	db_waarviewblok2(a3),(a0)+
	move.w	db_libversion(a3),(a0)+
	rts
*
* copy the global resident data to the local data
*
copy_global_local:
	lea	in_chipscrap_mem(pc),a0
	addq.l	#4,a0
	move.l	(a0)+,db_chipscrap_mem(a3)
	move.l	(a0)+,db_chipscrap_size(a3)
	move.l	(a0)+,db_fastscrap_mem(a3)
	move.l	(a0)+,db_fastscrap_size(a3)
	move.l	(a0)+,db_conhandle(a3)
	move.l	(a0)+,db_oldview(a3)
	move.l	(a0)+,db_welkmasker(a3)
	move.l	(a0)+,db_horizontalmask(a3)
	move.l	(a0)+,db_oldmask(a3)
	move.l	(a0)+,db_wpatroon_tabel(a3)
	move.l	(a0)+,db_wpatroonruimte(a3)
	move.l	(a0)+,db_wstore_moves2(a3)
	move.w	(a0)+,db_gapsize(a3)
	move.w	(a0)+,db_max_Depth_Lores(a3)
	move.w	(a0)+,db_max_Depth_Hires(a3)
	move.w	(a0)+,db_max_Depth_SHires(a3)
	move.b	(a0)+,db_aa_present(a3)
	move.b	(a0)+,db_V39_present(a3)
	move.b	(a0)+,db_pal(a3)
	addq.l	#1,a0
	move.b	(a0)+,db_Xoff(a3)
	move.b	(a0)+,db_Yoff(a3)
	move.l	(a0)+,db_waarcolors1(a3)
	move.l	(a0)+,db_waarcolors2(a3)
	move.l	(a0)+,db_waarcolors32(a3)
	move.l	(a0)+,db_waardeltacol1(a3)
	move.l	(a0)+,db_waardeltacol2(a3)
	move.l	(a0)+,db_waarviewblok1(a3)
	move.l	(a0)+,db_waarviewblok2(a3)
	move.w	(a0)+,db_libversion(a3)
	rts

in_chipscrap_mem:	dc.l	0
			dc.l	0
in_chipscrap_size:	dc.l	0
in_fastscrap_mem:	dc.l	0
in_fastscrap_size:	dc.l	0
in_conhandle:		dc.l	0
in_oldview:		dc.l	0
in_welkmasker:		dc.l	0
in_horizontalmask:	dc.l	0
in_oldmask:		dc.l	0
in_wpatroon_tabel:	dc.l	0
in_wpatroon_ruimte:	dc.l	0
in_wstore_moves2:	dc.l	0

in_gapsize:		dc.w	1
in_max_Depth_Lores:	dc.w	5
in_max_Depth_Hires:	dc.w	4
in_max_Depth_SHires:	dc.w	0

in_aa_present:		dc.b	0
in_V39_present:		dc.b	0
in_pal:			dc.b	0
			dc.b	0
in_Xoff:		dc.b	0
in_Yoff:		dc.b	0
in_waarcolors1:		dc.l	0
in_waarcolors2:		dc.l	0
in_waarcolors32		dc.l	0
in_waardeltacol1:	dc.l	0
in_waardeltacol2:	dc.l	0
in_waarviewblok1:	dc.l	0
in_waarviewblok2:	dc.l	0
in_libversion:		dc.w	0
atoz_teller:		dc.l	0

	XDEF	_wipe_in
_wipe_in:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)

	move.l	8(a5),a3
	move.l	a7,db_easy_exit(a3)
	tst.l	db_monitorspec(a3)
	beq	not_loaded2

	move.l	#0,db_replypointer(a3)
	lea	db_fileblok1(a3),a1
	tst.b	vb_id(a1)
	beq	not_loaded

;	move.l	db_inactive_fileblok(a3),a5

	bsr	set_lace

	lea	db_fileblok1(a3),a5
	move.w	vb_planes(a5),d0

	IFNE	BIT24
	cmp.w	#24,d0
	beq	do_24b_bang
	ENDC

	tst.b	vb_shires(a5)
	beq	.no_shir1
	cmp.w	db_max_Depth_SHires(a3),d0
	bgt	not_loaded2
.no_shir1:
	tst.b	vb_hires(a5)
	beq	no_hir1
	cmp.w	db_max_Depth_Hires(a3),d0
	bgt	not_loaded2
	bra	checked_pl
no_hir1:
	cmp.w	db_max_Depth_Lores(a3),d0
	bgt	not_loaded2
	bra	checked_pl
do_24b_bang:
	move.l	db_which_wipe(a3),d0
	lea	wipe_types_new(pc),a0
	move.b	0(a0,d0.w),d0
	and.b	#STAY_S,d0
	bne	.check
	move.l	#0,db_which_wipe(a3)		; set wipe on cut with 24bits
.check:
	cmp.w	#8,db_max_Depth_Hires(a3)	; only works on AA machines
	bne	not_loaded2
checked_pl:
	bsr	get_active_info			; get view info

	move.l	db_active_viewblok(a3),a5
	tst.l	vb_color_count(a5)
	bne	oke_cols
	move.l	#3,vb_color_count(a5)		; just to be sure
oke_cols:

	move.l	#0,db_counttimer(a3)

	move.l	db_effect_speed_in(a3),db_effect_speed(a3)
	move.l	db_effect_speed_in(a3),db_effect_speed_org(a3)
	move.l	db_varltimes_in(a3),db_varltimes(a3)

	bsr	setsam
	tst.b	d0
	bne	not_loaded

	bsr	install_50h

	lea	sigmask(pc),a0
	move.l	(a0),d0
;	or.l	db_sig_ptoc(a3),d0
	move.l	d0,db_waitmasker(a3)

	bsr	do_wipe_file_in

	bsr	remove_50h
	bsr	freesam

not_loaded:
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	rts

not_loaded2:
	lea	db_fileblok1(a3),a5
	move.l	vb_filenaam(a5),a0
	moveq	#ERR_RES_NOT_SUP,d0
	bsr	copy_err2
	bra	not_loaded

terror:	dc.l	0
;	ENDC

setsam:
	lea	sigmask(pc),a0
	tst.l	(a0)
	bne	no_install

	move.l	$4.w,a6
	moveq	#-1,d0
	jsr	_LVOAllocSignal(a6)	

	cmp.l	#-1,d0
	beq	setsam_error
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
	beq	nofreesam
	cmp.l	#-1,d0
	beq	nofreesam
	move.l	$4.w,a6
	jsr	_LVOFreeSignal(a6)
nofreesam:
	lea	signum(pc),a0
	clr.l	(a0)
	lea	sigmask(pc),a0
	clr.l	(a0)
	rts

stuur_signaal:
	movem.l	d0/a1,-(a7)
	clr.l	off_teller(a1)
	move.l	$4.w,a6
	move.l	off_task(a1),a1

	move.l	sigmask(pc),d0
	jsr	_LVOSignal(a6)
	movem.l	(a7)+,d0/a1
	rts

install_50h:
	lea	handle_installed(pc),a0
	tst.b	(a0)
	bne	no_install

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
	lea	handle_installed(pc),a0
	tst.b	(a0)
	beq	no_install2
	lea.l	intstruct50h(pc),a1
	moveq	#5,d0
	move.l	$4.w,a6
	jsr	_LVORemIntServer(a6)
	lea	handle_installed(pc),a0
	move.b	#$00,(a0)
no_install2:
	rts

proc_50h:
	move.l	a0,-(a7)
	subq.l	#1,off_teller(a1)
	bpl	no_send
	bsr	stuur_signaal
	move.l	off_tel_start(a1),off_teller(a1)
no_send:
	move.l	(a7)+,a0
	moveq	#0,d0
	rts

proc50hnaam:	dc.b	"50Hz interupt",0
	even
		
intstruct50h:
	dc.l	0,0
	dc.b	2,64		; type en pri
	dc.l	0,0		; pointer naar naam en data
	dc.l	0

teller:			dc.l	0
tellerstart:		dc.l	0
task:			dc.l	0
signum:			dc.l	0
sigmask:		dc.l	0
handle_installed:	dc.b	0,0

off_teller 	= 0
off_tel_start 	= 4
off_task 	= 8
off_signum 	= 12
off_sigmask 	= 16
off_timer	= 20

openlibs:
	moveq	#0,d0
	move.l	d0,db_graphbase(a3)
	move.l	d0,db_dosbase(a3)

	moveq	#ERR_GENERAL,d7
	move.l	$4.w,a6
	lea	dosnaam(pc),a1
	moveq	#0,d0
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,db_dosbase(a3)
	beq.w	sexit

	move.l	db_dosbase(a3),a6
	jsr	_LVOOutput(a6)
	move.l	d0,db_conhandle(a3)

	moveq	#ERR_GENERAL,d7
	lea	graphname(pc),a1
	moveq	#0,d0
	move.l	$4.w,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,db_graphbase(a3)
	beq.w	sexit

	moveq	#ERR_GENERAL,d7
	move.l	$4.w,a6
	moveq	#0,d0
	lea.l	intui(pc),a1
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,db_intbase(a3)
	
	beq.w	sexit

	moveq	#ERR_GENERAL,d7
	lea	mathtransnaam(pc),a1
	moveq	#0,d0
	move.l	$4.w,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,db_mathtransbase(a3)
	beq.w	sexit

	moveq	#ERR_GENERAL,d7
	lea	mathffpname(pc),a1
	moveq	#0,d0
	move.l	$4.w,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,db_mathffpbase(a3)
	beq.w	sexit

	moveq	#ERR_GENERAL,d7
	lea	layersname(pc),a1
	moveq	#0,d0
	move.l	$4.w,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,db_layersbase(a3)
	beq.w	sexit

	moveq	#ERR_GENERAL,d7
	lea	mlmmuname(pc),a1
	moveq	#0,d0
	move.l	$4.w,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,db_mlmmubase(a3)
	beq.w	sexit

	move.l	db_graphbase(a3),a6

	move.l	gb_ActiView(a6),db_oldview(a3)
	move.l	gb_ActiView(a6),tt
	rts

tt:	dc.l	0

dosnaam:	dc.b	'dos.library',0
graphname:	dc.b	'graphics.library',0
intui:		dc.b	'intuition.library',0
mathtransnaam:	dc.b	'mathtrans.library',0
mathffpname:	dc.b	'mathffp.library',0
layersname:	dc.b	'layers.library',0
	IFEQ	XAPP
mlmmuname:	dc.b	'nb:system/mpmmu.library',0
	ELSE
mlmmuname:	dc.b	'mpmmu.library',0
	ENDC
	even

	IFNE XAPP

	XDEF	_load_wipe
_load_wipe:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)

	move.l	12(a5),a3
	move.l	a7,db_easy_exit(a3)

	lea	db_fileblok1(a3),a1	; only when fileblok is private
	tst.b	vb_id(a1)
	bne	already_loaded

	move.l	8(a5),a0
	moveq	#0,d0			; filename

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dat(pc),a1
	move.l	a0,(a1)
	lea	.dbstr2(pc),a0
	jsr	KPutFmt
	bra	.tt2
.dbstr2:	dc.b	"Load file %s",10,0
	even
.dat:	dc.l	0
.tt2:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	16(a5),a1

	move.w	24(a1),d0

	move.b	#0,db_cyc_quit(a3)
	cmp.w	#1,d0
	beq	.no_col
	move.b	#1,db_cyc_quit(a3)
.no_col:	

	move.w	4(a1),d0
	move.l	d0,db_which_wipe(a3)	; wipe nr.
	move.w	6(a1),d0
	move.l	d0,db_effect_speed(a3)	; speed
	move.l	d0,db_effect_speed_org(a3)
	move.l	d0,db_effect_speed_in(a3)
	move.w	8(a1),d0
	move.l	d0,db_varltimes(a3)		; line thickness
	move.l	d0,db_varltimes_in(a3)		; line thickness
	move.w	10(a1),d0
	move.l	d0,db_variation(a3)		; variation
	move.b	#1,db_set_error(a3)

	move.l	24(a5),d0
	beq	.skip_err
	move.b	#0,db_set_error(a3)
.skip_err:

	lea	db_fileblok1(a3),a1
	move.b	#$0,vb_id(a1)			; file not loaded

	bsr	laadfile

	lea	db_fileblok1(a3),a1
	move.b	#$ff,vb_id(a1)			; file loaded

already_loaded:

	move.b	#1,db_set_error(a3)

	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0				; succes
	rts

dont_load:
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#2,d0				; succes
	rts

	ENDC
	
* alloceer geheugen voor twee chip mem buffer 
* met de groote MEMSIZE in deze buffers moet 
* het grootste plaatje passen 738*588*4 ????????? 1400*588*8 ???????
* 						823.200 bytes	
	IFEQ XAPP
request_memory:
	moveq	#ERR_NO_MEM,d7
	move.l	#MEMSIZE,d0
	move.l	d0,db_unpacked1_size(a3)
	move.l	#$10002,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,db_unpacked1(a3)
	beq.w	sexit
	add.l	#MEMSIZE,d0

	move.l	#MEMSIZE,d0
	move.l	d0,db_unpacked2_size(a3)
	move.l	#$10002,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,db_unpacked2(a3)
	beq.w	sexit
	add.l	#MEMSIZE,d0

	move.b	#$0,db_triple_present(a3)
	move.l	#MEMSIZE,d0
	move.l	d0,db_triple_size(a3)
	move.l	#$10002,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,db_triple_mem(a3)
	beq	no_triple1
	add.l	#MEMSIZE,d0

	move.b	#$ff,db_triple_present(a3)
	ENDC

no_triple1:
*
* vraag een stuk chipmem aan voor verschillende maskers
	moveq	#ERR_NO_MEM,d7
	move.l	#CHIPSCRAPSIZE,d0
	move.l	d0,db_chipscrap_size(a3)

	move.l	#$10002,d1
	IFNE XAPP
	or.l	#MEMF_STAY,d1
	ENDC

	lea	chipscrapname(pc),a1
	move.l	db_mlmmubase(a3),a6
	jsr	_LVOMLMMU_AllocMem(a6)
	move.l	d0,db_chipscrap_mem(a3)
	beq	sexit				; heb dit zeker nodig ???

	move.l	d0,a0
	move.l	d0,db_horizontalmask(a3)
	move.l	d0,db_oldmask(a3)
	move.l	d0,db_welkmasker(a3)
	addq.l	#2,db_oldmask(a3)
*
* vraag een stuk whatever-mem aan voor verschillende functies
	moveq	#ERR_NO_FMEM,d7

	move.l	#FASTSCRAPSIZE,d0
	move.l	d0,db_fastscrap_size(a3)
	
	move.l	#MEMF_CLEAR,d1
	IFNE	XAPP
	or.l	#MEMF_STAY,d1
	ENDC
	lea	fastscrapname(pc),a1
	move.l	db_mlmmubase(a3),a6
 	jsr	_LVOMLMMU_AllocMem(a6)
	move.l	d0,db_fastscrap_mem(a3)
	beq	sexit				; heb dit zeker nodig ???

	move.l	d0,a0
	move.l	#FASTSCRAPSIZE-1,d7
clear_scrap:
	move.b	#0,(a0)+
	dbf	d7,clear_scrap

	move.l	d0,db_wpatroonruimte(a3)
	add.l	#PATROONSIZE,d0
	move.l	d0,db_wpatroon_tabel(a3)
	add.l	#PTABELSIZE,d0
	move.l	d0,db_wstore_moves2(a3)
	add.l	#STOREMOVESIZE,d0
	move.l	d0,db_waarcolors1(a3)
	add.l	#COL1SIZE,d0
	move.l	d0,db_waarcolors2(a3)
	add.l	#COL2SIZE,d0

	move.l	d0,db_waardeltacol1(a3)
	add.l	#DCOL1SIZE,d0
	move.l	d0,db_waardeltacol2(a3)
	add.l	#DCOL2SIZE,d0

	move.l	d0,db_waarcolors32(a3)
	add.l	#COL32SIZE,d0

	move.l	d0,db_waarviewblok1(a3)
	add.l	#VIEWBLOK1SIZE,d0

	move.l	d0,db_waarviewblok2(a3)
	add.l	#VIEWBLOK2SIZE,d0

	rts

	
chipscrapname:	dc.b	"Chip_scrap",0
fastscrapname:	dc.b	"Fast_scrap",0

	XDEF	_load_mem_wipe
_load_mem_wipe:
	link	a5,#0

	movem.l	d0-d7/a0-a6,-(a7)
	move.l	12(a5),a3
	
	move.l	a7,db_easy_exit(a3)

	lea	db_fileblok1(a3),a1	; only when fileblok is private

	move.l	8(a5),a0
	moveq	#0,d0			; pointer

	move.l	16(a5),a1

	move.w	4(a1),d0
	move.l	d0,db_which_wipe(a3)	; wipe nr.
	move.w	6(a1),d0
	move.l	d0,db_effect_speed(a3)	; speed
	move.l	d0,db_effect_speed_org(a3)
	move.l	d0,db_effect_speed_in(a3)
	move.w	8(a1),d0
	move.l	d0,db_varltimes(a3)		; line thickness
	move.l	d0,db_varltimes_in(a3)		; line thickness
	move.w	10(a1),d0
	move.l	d0,db_variation(a3)		; variation
	move.w	12(a1),d0
	ext.l	d0
	move.l	d0,db_blitin_x(a3)		; x pos blitin
	move.w	14(a1),d0
	ext.l	d0
	move.l	d0,db_blitin_y(a3)		; y pos blitin

	move.w	16(a1),d0			; wipe_out
	swap	d0
	move.w	4(a1),d0
	move.l	d0,db_which_wipe(a3)

	move.b	#1,db_cyc_quit(a3)

	lea	db_fileblok1(a3),a1
	move.b	#$0,vb_id(a1)			; file not loaded

	jsr	laadmem

	tst.w	d0
	bne	err_mem1			; error no FORM

	lea	db_fileblok1(a3),a1
	move.b	#$ff,vb_id(a1)			; file loaded

already_loaded2:
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0				; succes
	rts

err_mem1:
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#1,d0				; no IFF
	rts

*
* get the needed info from the active viewport 
*
get_active_info:

	IFNE XAPP
	bsr	set_view_structs_ML		; set active viewport
	ENDC
	
	move.l	db_active_viewblok(a3),a5

	move.l	db_active_viewblok(a3),a5
	move.b	#0,vb_interlace(a5)
	move.b	#0,vb_hires(a5)
	move.b	#0,vb_shires(a5)
	move.b	#0,vb_masking(a5)
	move.b	#0,vb_compression(a5)
	move.b	#0,vb_id(a5)

	move.l	vb_viewportw(a5),a1
	cmp.l	#0,a1
	beq	no_init

	moveq	#0,d0
	move.w	vp_DWidth(a1),d0
	beq	take_over
	
	move.w	d0,vb_lenx(a5)
	add.w	#15,d0				; in bytes ???
	lsr.w	#4,d0
	add.l	d0,d0
	move.l	d0,vb_breedte_x(a5)
	move.w	vp_DHeight(a1),vb_leny(a5)
	move.w	vp_Modes(a1),d0

	move.w	d0,d1
;	and.w	#SUPER_KEY,d1
	and.w	#V_SUPERHIRES,d1
	beq	.noshires
	move.b	#$ff,vb_shires(a5)
.noshires:
	move.w	d0,d1
	and.w	#V_HIRES,d1
	beq	no_hir
	move.b	#$ff,vb_hires(a5)
no_hir:
	move.w	d0,d1
	and.w	#V_LACE,d1
	beq	no_lac
	move.b	#$ff,vb_interlace(a5)
no_lac:
	move.w	d0,vb_mode(a5)

	move.l	vb_bitmapw(a5),a1
;	lea.l	sbu_Display+dp_BitMap(a0),a1

	moveq	#0,d0
	move.b	bm_Depth(a1),d0
	move.w	d0,vb_planes(a5)

	moveq	#1,d2
	lsl.l	d0,d2
	move.l	d2,d3
	add.l	d2,d2
	add.l	d3,d2

	move.l	vb_viewportw(a5),a1
	move.w	vp_Modes(a1),d7
	and.w	#V_HAM,d7
	beq	no_ham24
	moveq	#$30,d2
	tst.b	db_aa_present(a3)
	beq	no_ham24
	move.l	#192,d2
no_ham24:

	move.l	d2,vb_color_count(a5)		; n o colors * 3

	move.b	#0,vb_masking(a5)
	move.l	vb_breedte_x(a5),d0
	moveq	#0,d1
	move.w	vb_leny(a5),d1
	mulu	d0,d1
	move.w	vb_planes(a5),d0
	mulu	d0,d1
	move.l	d1,vb_unpacked_size(a5)

; get the colors from the colormap ??

	move.l	vb_viewportw(a5),a1
	move.l	vp_ColorMap(a1),a0
	cmp.l	#0,a0
	beq	no_init

	tst.b	db_aa_present(a3)
	bne	getcols256

	moveq	#31,d7
	moveq	#0,d5				; entry nummer

	lea	vb_colors(a5),a4
	move.l	db_graphbase(a3),a6
	move.l	a0,d6				; save colormap pointer
rep_getcols32:
	move.l	d5,d0
	move.l	d6,a0
	move.l	db_graphbase(a3),a6
	jsr	_LVOGetRGB4(a6)
	move.w	d0,(a4)+
	addq.l	#1,d5				; next entry
	dbf	d7,rep_getcols32

no_init:
	tst.w	vb_leny(a5)
	bne	oke_leny
	move.w	#50,vb_leny(a5)
oke_leny:
	tst.w	vb_lenx(a5)
	bne	oke_lenx
	move.w	#64,vb_lenx(a5)
	move.l	#8,vb_breedte_x(a5)
oke_lenx:
	tst.w	vb_planes(a5)
	bne	oke_planes
	move.w	#1,vb_planes(a5)
ecs:
oke_planes:
	tst.l	vb_unpacked_size(a5)
	bne	oke_packed_size
	move.l	#1000,vb_unpacked_size(a5)
oke_packed_size:
	tst.l	vb_color_count(a5)
	bne	oke_color_count
	move.l	#3,vb_color_count(a5)
oke_color_count:

	rts

getcols256:
	moveq	#0,d0				; first color
	move.l	#256,d1				; n o colors
	move.l	db_waarcolors32(a3),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVOGetRGB32(a6)


	move.l	db_waarcolors32(a3),a1
	lea	vb_colbytes(a5),a2
	move.l	#255,d7
rep_getcols256:

	moveq	#2,d6				; for R G and B
rep_getcols256_2:
	move.l	(a1)+,d0
	swap	d0
	lsr.w	#8,d0
	move.b	d0,(a2)+
	dbf	d6,rep_getcols256_2
	dbf	d7,rep_getcols256

	rts

*
* The jump table is PC relative so make it absolute
* ( is this legal ??? )
*
init_jump_tabel:
	lea	jump_initialised(pc),a0
	tst.b	(a0)
	bne	no_jt_abs2

	IFEQ	XAPP
	lea	wipes,a0
	ELSE
	lea	wipes(pc),a0
	ENDC
	lea	pstart(pc),a1
rep_jt1:
	move.l	(a0),d0
	cmp.l	#-1,d0
	beq	no_jt_abs
	add.l	a1,d0
	move.l	d0,(a0)+
	bra	rep_jt1
no_jt_abs:

	IFEQ	XAPP
	lea	blitin_wipes,a0
	ELSE
	lea	blitin_wipes(pc),a0
	ENDC
	lea	pstart(pc),a1
rep_jt3:
	move.l	(a0),d0
	cmp.l	#-1,d0
	beq	no_jt_abs3
	add.l	a1,d0
	move.l	d0,(a0)+
	bra	rep_jt3
no_jt_abs3:

	IFEQ XAPP
	lea	wipesrev,a0
	ELSE
	lea	wipesrev(pc),a0
	ENDC

	lea	pstart(pc),a1
rep_jt2:
	move.l	(a0),d0
	cmp.l	#-1,d0
	beq	no_jt_abs2
	add.l	a1,d0
	move.l	d0,(a0)+
	bra	rep_jt2
no_jt_abs2:
	lea	jump_initialised(pc),a0
	move.b	#$ff,(a0)
	rts
*
* returns in a0 a pointer to the active screenbuffer
*
give_active_onscreen:
	moveq	#MAXSCREENBUFS-1,d0
	move.l	db_mlsysstruct(a3),a0
search_ag1:

	tst.w	sbu_Viewed(a0)
	bne	found_act1
	lea.l	sbu_SIZEOF(a0),a0
	dbf	d0,search_ag1
	move.l	db_mlsysstruct(a3),a0		; no active take the first
	rts

found_act1:
	rts
*
* returns in a0 a pointer to the inactive screenbuffer
*
give_inactive_onscreen:
	moveq	#MAXSCREENBUFS-1,d0
	move.l	db_mlsysstruct(a3),a0

search_ag2:
	tst.w	sbu_Viewed(a0)
	beq	found_act2
	lea.l	sbu_SIZEOF(a0),a0
	dbf	d0,search_ag2
	move.l	db_mlsysstruct(a3),a0		; no inactive take the second ?
	lea.l	db_SIZEOF(a0),a0
	rts
found_act2:
	rts

*
* De bedoeling is dat hier in de locale structuren voor het scherm
* de globale bitmap rasinfo view en viewport structuur gezet moeten worden
*
set_view_structs_ML:

	bsr	give_active_onscreen

	move.l	db_active_viewblok(a3),a5
	move.w	#TRUE,sbu_Viewed(a0)		; set just in case
	move.l	a0,vb_mlscrpointer(a5)
	move.l	#0,vb_vextra(a5)
	move.l	#0,vb_vpextra(a5)

;	move.l	sbu_Display+dp_vextra(a0),vb_vextra(a5)
;	move.l	sbu_Display+dp_vpextra(a0),vb_vpextra(a5)
	lea.l	sbu_Display+dp_View(a0),a1
	move.l	a1,vb_vieww(a5)

	lea.l	sbu_Display+dp_ViewPort(a0),a1
	move.l	a1,vb_viewportw(a5)

	lea.l	sbu_Display+dp_RasInfo(a0),a1
	move.l	a1,vb_rasinfow(a5)		

	lea.l	sbu_Display+dp_BitMap(a0),a1
	move.l	a1,vb_bitmapw(a5)

	move.l	sbu_Base(a0),vb_tempbuffer(a5)

	move.l	db_inactive_viewblok(a3),a5
	bsr	give_inactive_onscreen
	move.l	a0,vb_mlscrpointer(a5)
	move.l	#0,vb_vextra(a5)
	move.l	#0,vb_vpextra(a5)

;	move.l	sbu_Display+dp_vextra(a0),vb_vextra(a5)
;	move.l	sbu_Display+dp_vpextra(a0),vb_vpextra(a5)

	lea.l	sbu_Display+dp_View(a0),a1
	move.l	a1,vb_vieww(a5)
	lea.l	sbu_Display+dp_ViewPort(a0),a1
	move.l	a1,vb_viewportw(a5)
	lea.l	sbu_Display+dp_RasInfo(a0),a1
	move.l	a1,vb_rasinfow(a5)		
	lea.l	sbu_Display+dp_BitMap(a0),a1
	move.l	a1,vb_bitmapw(a5)
	move.l	sbu_Base(a0),vb_tempbuffer(a5)
	rts

	IFEQ	XAPP
*
* De bedoeling is dat hier in de locale structuren voor het scherm
* de globale bitmap rasinfo view en viewport structuur gezet moeten worden
*
set_view_structs:
	move.l	db_active_viewblok(a3),a5
	move.l	#0,vb_vextra(a5)
	move.l	#0,vb_vpextra(a5)

	lea.l	db_view1(a3),a0
	move.l	a0,vb_vieww(a5)
	lea	db_viewport1(a3),a0
	move.l	a0,vb_viewportw(a5)
	lea	db_rasinfo1(a3),a0
	move.l	a0,vb_rasinfow(a5)		
	lea	db_bitmap1(a3),a0
	move.l	a0,vb_bitmapw(a5)

	move.l	db_inactive_viewblok(a3),a5
	move.l	#0,vb_vextra(a5)
	move.l	#0,vb_vpextra(a5)

	lea.l	db_view2(a3),a0
	move.l	a0,vb_vieww(a5)
	lea	db_viewport2(a3),a0
	move.l	a0,vb_viewportw(a5)
	lea	db_rasinfo2(a3),a0
	move.l	a0,vb_rasinfow(a5)		
	lea	db_bitmap2(a3),a0
	move.l	a0,vb_bitmapw(a5)
	rts

mname:	dc.b	"pal",0
	even
	ENDC

wipes:
	dc.l	showdirect-pstart
	dc.l	blend-pstart
	dc.l	cop_cover_down-pstart,cop_up1-pstart,cop_down1_v1-pstart,cop_up2-pstart

	dc.l	wipe_down-pstart,wipe_up-pstart
	dc.l	edges_in_vertical-pstart,center_out_vertical-pstart
	dc.l	rows_top_and_back-pstart,rows_bottom_and_back-pstart
	dc.l	rows_from_edges_vertical-pstart
	dc.l	blinds_sub1-pstart,blinds_sub2-pstart

	dc.l	random_rows-pstart

	dc.l	torn_down-pstart,torn_up-pstart
        dc.l	roll_down-pstart,roll_up-pstart

	dc.l	wipe_right-pstart,wipe_left-pstart
	dc.l	edges_in_horizontal-pstart,center_out_horizontal-pstart
	dc.l	columns_left_back-pstart,columns_right_back-pstart
	dc.l	edges_horizontal-pstart
	dc.l	blinds_vertical-pstart
	dc.l	random_columns-pstart

	dc.l	climb_bars_1-pstart
	dc.l	climb_bars_2-pstart
	dc.l	climb_bars_3-pstart
	dc.l	climb_bars_4-pstart
	dc.l	dissolve-pstart

	dc.l	blockvar-pstart
	dc.l	blocks1-pstart,blocks2-pstart,blocks3-pstart
	dc.l	blocks4-pstart,blocks5-pstart,blocks6-pstart
	dc.l	blocks7-pstart,blocks8-pstart,blocks9-pstart
	dc.l	blocks10-pstart,blocks11-pstart

	dc.l	lijnvariation-pstart,lijnh2-pstart,lijnh3-pstart
	dc.l	lijnh4-pstart,lijnh5-pstart
	dc.l	lijnh1b-pstart,lijnh2b-pstart,lijnh3b-pstart
	dc.l	lijnh4b-pstart,lijnh5b-pstart
	dc.l	crawling_rows_1-pstart
	dc.l	crawling_rows_2-pstart
	dc.l	crawling_rows_3-pstart

	dc.l	crawling_even_odd-pstart

	dc.l	fade_to_white-pstart,fade_to_black-pstart
	dc.l	fade_to_zero_active-pstart,fade_to_zero_inactive-pstart
	dc.l	fade_to_first_active-pstart,fade_to_first_inactive-pstart
	dc.l	test_pixel1-pstart
	dc.l	random_eff1-pstart
	dc.l	random_eff2-pstart
	dc.l	cop_flip1-pstart
	dc.l	cop_flip2-pstart
	dc.l	cop_coin1-pstart
	dc.l	showdirect-pstart	; stub for documents over old

; effect nr 73 

	dc.l	cop_down_double-pstart,cop_up_double-pstart
	dc.l	cop_reveal_down-pstart,cop_reveal_up-pstart
	dc.l	cop_down_bottom-pstart,cop_up_top-pstart
	dc.l	cop_reveal_down2-pstart,cop_reveal_up2-pstart
	dc.l	cop_cover_down2-pstart,cop_cover_up-pstart

	dc.l	random_eff3-pstart
	dc.l	kcut-pstart
	dc.l	mozaik1-pstart
	dc.l	mozaik_down-pstart
	dc.l	mozaik_up-pstart
	dc.l	page_fold1-pstart

	dc.l	corner_in_top_left-pstart
	dc.l	corner_in_top_right-pstart
	dc.l	corner_in_bottom_right-pstart
	dc.l	corner_in_bottom_left-pstart
	dc.l	wipe_to_centre-pstart
	dc.l	wipe_from_centre-pstart
	dc.l	wipe_from_centre2-pstart
	dc.l	wipe_to_centre2-pstart
	dc.l	wipe_from_centre3-pstart
	dc.l	wipe_to_centre3-pstart
	dc.l	diagonal1-pstart
	dc.l	cross1-pstart
	dc.l	cross2-pstart
		
	IFNE	NEWEFF
	dc.l	cop_drip1-pstart,cop_drip2-pstart
	ENDC
	dc.l	-1

blitin_wipes:
	dc.l	blitin_direct-pstart
	dc.l	blitin_down_up-pstart
	dc.l	blitin_up_down-pstart
	dc.l	blitin_left_right-pstart
	dc.l	blitin_right_left-pstart
	dc.l	blitin_lefttop_right-pstart
	dc.l	blitin_righttop_left-pstart
	dc.l	blitin_rightbot_left-pstart
	dc.l	blitin_leftbot_right-pstart
	dc.l	blitin_bend_path1-pstart,blitin_bend_path2-pstart
	dc.l	blitin_bend_path3-pstart,blitin_bend_path4-pstart
	dc.l	blitin_lijnh1-pstart,blitin_lijnh2-pstart,blitin_lijnh3-pstart

	dc.l	blitin_h1m1-pstart,blitin_h2m1-pstart

	dc.l	blitin_h1-pstart,blitin_h2-pstart,blitin_h3-pstart
	dc.l	blitin_h4-pstart,blitin_h5-pstart,blitin_h6-pstart
	dc.l	blitin_h3_ver2-pstart

	dc.l	blitin_v1-pstart,blitin_v2-pstart
	dc.l	blitin_v3-pstart,blitin_v4-pstart
	dc.l	blitin_v5-pstart,blitin_v6-pstart
	dc.l	blitin_v7-pstart

	dc.l	blitin_blink-pstart
	dc.l	blitin_blink_stick-pstart

	dc.l	blitin_point1-pstart,blitin_point2-pstart
	dc.l	blitin_point3-pstart,blitin_point4-pstart
	dc.l	flip_blitin1-pstart
	dc.l	flip_blitin2-pstart
	dc.l	flip_blitin3-pstart

; new effects with the screen effect stub 16-04-1994

	dc.l	blitin_blinds1-pstart,blitin_blinds2-pstart

	dc.l	blitin_random_rows-pstart

	dc.l	blitin_torn_down-pstart,blitin_torn_up-pstart
        dc.l	blitin_roll_down-pstart,blitin_roll_up-pstart

	dc.l	blitin_blinds_vertical-pstart
	dc.l	blitin_random_columns-pstart

	dc.l	blitin_climb_bars_1-pstart
	dc.l	blitin_climb_bars_2-pstart
	dc.l	blitin_climb_bars_3-pstart
	dc.l	blitin_climb_bars_4-pstart
	dc.l	blitin_dissolve-pstart

	dc.l	blitin_blockvar-pstart
	dc.l	blitin_blocks1-pstart,blitin_blocks2-pstart
	dc.l	blitin_blocks3-pstart,blitin_blocks4-pstart
	dc.l	blitin_blocks5-pstart,blitin_blocks6-pstart
	dc.l	blitin_blocks7-pstart,blitin_blocks8-pstart
	dc.l	blitin_blocks9-pstart,blitin_blocks10-pstart
	dc.l	blitin_blocks11-pstart

	dc.l	blitin_mozaik_down-pstart
	dc.l	blitin_mozaik_up-pstart
	dc.l	blitin_page_fold1-pstart
	dc.l	blitin_corner_in_top_left-pstart
	dc.l	blitin_corner_in_top_right-pstart
	dc.l	blitin_corner_in_bottom_right-pstart
	dc.l	blitin_corner_in_bottom_left-pstart
	dc.l	blitin_wipe_to_centre-pstart
	dc.l	blitin_wipe_from_centre-pstart
	dc.l	blitin_wipe_from_centre2-pstart
	dc.l	blitin_wipe_to_centre2-pstart
	dc.l	blitin_wipe_from_centre3-pstart
	dc.l	blitin_wipe_to_centre3-pstart
	dc.l	blitin_diagonal1-pstart
	dc.l	blitin_cross1-pstart
	dc.l	blitin_cross2-pstart

	dc.l	-1

wipesrev:
	dc.l	blitin_direct_rev-pstart
	dc.l	blitin_up_down_rev-pstart
	dc.l	blitin_down_up_rev-pstart
	dc.l	blitin_right_left_rev-pstart
	dc.l	blitin_left_right_rev-pstart

	dc.l	blitin_rightbot_left_rev-pstart
	dc.l	blitin_leftbot_right_rev-pstart
	dc.l	blitin_lefttop_right_rev-pstart
	dc.l	blitin_righttop_left_rev-pstart

	dc.l	blitin_bend_path1_rev-pstart,blitin_bend_path2_rev-pstart
	dc.l	blitin_bend_path3_rev-pstart,blitin_bend_path4_rev-pstart

	dc.l	blitin_lijnh1_rev-pstart,blitin_lijnh2_rev-pstart
	dc.l	blitin_lijnh3_rev-pstart

	dc.l	blitin_h1m1_rev-pstart,blitin_h2m1_rev-pstart

	dc.l	blitin_h2_rev-pstart,blitin_h1_rev-pstart,blitin_h4_rev-pstart
	dc.l	blitin_h3_rev-pstart,blitin_h6_rev-pstart,blitin_h5_rev-pstart
	dc.l	blitin_h3_ver2_rev-pstart

	dc.l	blitin_v2_rev-pstart,blitin_v1_rev-pstart
	dc.l	blitin_v4_rev-pstart,blitin_v3_rev-pstart
	dc.l	blitin_v6_rev-pstart,blitin_v5_rev-pstart
	dc.l	blitin_v7_rev-pstart

	dc.l	blitin_blink-pstart,blitin_blink-pstart

	dc.l	blitin_point1-pstart,blitin_point2-pstart
	dc.l	blitin_point3-pstart,blitin_point4-pstart

; new
	dc.l	flip_blitin1_rev-pstart
	dc.l	flip_blitin2_rev-pstart
	dc.l	flip_blitin3_rev-pstart

	dc.l	blitin_blinds1-pstart,blitin_blinds2-pstart

	dc.l	blitin_random_rows-pstart

	dc.l	blitin_torn_down-pstart,blitin_torn_up-pstart
        dc.l	blitin_roll_down-pstart,blitin_roll_up-pstart

	dc.l	blitin_blinds_vertical-pstart
	dc.l	blitin_random_columns-pstart

	dc.l	blitin_climb_bars_1-pstart
	dc.l	blitin_climb_bars_2-pstart
	dc.l	blitin_climb_bars_3-pstart
	dc.l	blitin_climb_bars_4-pstart
	dc.l	blitin_dissolve-pstart

	dc.l	blitin_blockvar-pstart
	dc.l	blitin_blocks1-pstart,blitin_blocks2-pstart
	dc.l	blitin_blocks3-pstart,blitin_blocks4-pstart
	dc.l	blitin_blocks5-pstart,blitin_blocks6-pstart
	dc.l	blitin_blocks7-pstart,blitin_blocks8-pstart
	dc.l	blitin_blocks9-pstart,blitin_blocks10-pstart
	dc.l	blitin_blocks11-pstart

	dc.l	blitin_mozaik_down-pstart
	dc.l	blitin_mozaik_up-pstart
	dc.l	blitin_page_fold1-pstart
	dc.l	blitin_corner_in_top_left-pstart
	dc.l	blitin_corner_in_top_right-pstart
	dc.l	blitin_corner_in_bottom_right-pstart
	dc.l	blitin_corner_in_bottom_left-pstart
	dc.l	blitin_wipe_to_centre-pstart
	dc.l	blitin_wipe_from_centre-pstart
	dc.l	blitin_wipe_from_centre2-pstart
	dc.l	blitin_wipe_to_centre2-pstart
	dc.l	blitin_wipe_from_centre3-pstart
	dc.l	blitin_wipe_to_centre3-pstart
	dc.l	blitin_diagonal1-pstart
	dc.l	blitin_cross1-pstart
	dc.l	blitin_cross2-pstart

	dc.l	-1

* dit zijn constanten

CONV_S = 0
STAY_S = 1
FLIP_S = 2
RAND_S = 4

wipe_types_new:
		dc.b	STAY_S,STAY_S
		dc.b	STAY_S,STAY_S,STAY_S,STAY_S

		dc.b	CONV_S,CONV_S,CONV_S,CONV_S
		dc.b	CONV_S,CONV_S,CONV_S
		dc.b	CONV_S,CONV_S		; blinds
		dc.b	CONV_S
		dc.b	CONV_S,CONV_S
		dc.b	CONV_S,CONV_S

		dc.b	CONV_S,CONV_S,CONV_S,CONV_S	; wipe right
		dc.b	CONV_S,CONV_S,CONV_S
		dc.b	CONV_S
		dc.b	CONV_S
		dc.b	CONV_S,CONV_S,CONV_S,CONV_S	; climbing bars
		dc.b	CONV_S				; dissolve

		dc.b	CONV_S,CONV_S,CONV_S,CONV_S,CONV_S,CONV_S
		dc.b	CONV_S,CONV_S,CONV_S,CONV_S,CONV_S,CONV_S

		dc.b	CONV_S,CONV_S,CONV_S,CONV_S,CONV_S,CONV_S	; rows
		dc.b	CONV_S,CONV_S,CONV_S,CONV_S,CONV_S,CONV_S,CONV_S

		dc.b	CONV_S
		
		dc.b	STAY_S,STAY_S	; fade to white and black
		dc.b	STAY_S,STAY_S	; fade to zero active, inactive
		dc.b	STAY_S,STAY_S	; fade to first color

		dc.b	CONV_S	; pixel

		dc.b	RAND_S	; random_eff1
		dc.b	RAND_S	; random_eff2

		dc.b	FLIP_S	; flip
		dc.b	FLIP_S	; flip2
		dc.b	FLIP_S	; coin1

		dc.b	STAY_S	; stub for cut without erase

		dc.b	STAY_S,STAY_S
		dc.b	STAY_S,STAY_S
		dc.b	STAY_S,STAY_S
		dc.b	STAY_S,STAY_S
		dc.b	STAY_S,STAY_S

		dc.b	RAND_S	; random_eff3
		dc.b	STAY_S	; kcut 

		dc.b	STAY_S	; mozaik 1
		dc.b	CONV_S	; mozaik_down
		dc.b	CONV_S	; mozaik_up
		dc.b	CONV_S	; page fold1
		dc.b	CONV_S	; corner in top left
		dc.b	CONV_S	; corner in top right
		dc.b	CONV_S	; corner in bottom right
		dc.b	CONV_S	; corner in bottom left
		dc.b	CONV_S	; to centre
		dc.b	CONV_S	; from centre
		dc.b	CONV_S	; from centre2
		dc.b	CONV_S	; to centre2
		dc.b	CONV_S	; from centre3
		dc.b	CONV_S	; to centre3
		dc.b	CONV_S	; diagonal1
		dc.b	CONV_S	; cross1
		dc.b	CONV_S	; cross2
		

		IFNE	NEWEFF
		dc.b	STAY_S,STAY_S	; drip1,2
		dc.b	STAY_S,STAY_S	; drip1,2
		ENDC
		dc.b	-1
		
DOUBLE_BUF = 1			; these are bit vars
NON_DOUBLE = 2			; for non double buffering
BLIT_STAY = 4			; when used as an out effect I stay on

blitin_types:

; direct
		dc.b	DOUBLE_BUF

		dc.b	DOUBLE_BUF
		dc.b	DOUBLE_BUF
		dc.b	DOUBLE_BUF
		dc.b	DOUBLE_BUF

		dc.b	DOUBLE_BUF
		dc.b	DOUBLE_BUF
		dc.b	DOUBLE_BUF
		dc.b	DOUBLE_BUF

		dc.b	DOUBLE_BUF,DOUBLE_BUF
		dc.b	DOUBLE_BUF,DOUBLE_BUF

; h1

		dc.b	NON_DOUBLE,NON_DOUBLE,NON_DOUBLE
		dc.b	NON_DOUBLE,NON_DOUBLE,NON_DOUBLE
		dc.b	NON_DOUBLE
; v1

		dc.b	NON_DOUBLE,NON_DOUBLE
		dc.b	NON_DOUBLE,NON_DOUBLE
		dc.b	NON_DOUBLE,NON_DOUBLE
		dc.b	NON_DOUBLE

		dc.b	NON_DOUBLE,NON_DOUBLE,NON_DOUBLE
		dc.b	NON_DOUBLE,NON_DOUBLE

; blink
		dc.b	DOUBLE_BUF, DOUBLE_BUF+BLIT_STAY

		dc.b	DOUBLE_BUF+BLIT_STAY,DOUBLE_BUF+BLIT_STAY
		dc.b	DOUBLE_BUF+BLIT_STAY,DOUBLE_BUF+BLIT_STAY

		dc.b	NON_DOUBLE
		dc.b	NON_DOUBLE
		dc.b	NON_DOUBLE

	IFNE 0

; new
		dc.b	DOUBLE_BUF,DOUBLE_BUF	; blitin_blinds1,2
		dc.b	DOUBLE_BUF

		dc.b	DOUBLE_BUF,DOUBLE_BUF
		dc.b	DOUBLE_BUF,DOUBLE_BUF

		dc.b	DOUBLE_BUF,DOUBLE_BUF

		dc.b	DOUBLE_BUF,DOUBLE_BUF	; climbing
		dc.b	DOUBLE_BUF,DOUBLE_BUF

		dc.b	DOUBLE_BUF		; dissolve

		dc.b	DOUBLE_BUF,DOUBLE_BUF
		dc.b	DOUBLE_BUF,DOUBLE_BUF
		dc.b	DOUBLE_BUF,DOUBLE_BUF
		dc.b	DOUBLE_BUF,DOUBLE_BUF
		dc.b	DOUBLE_BUF,DOUBLE_BUF
		dc.b	DOUBLE_BUF,DOUBLE_BUF

		dc.b	DOUBLE_BUF,DOUBLE_BUF	; moz down up

		dc.b	DOUBLE_BUF		; page fold
		dc.b	DOUBLE_BUF,DOUBLE_BUF	; corner
		dc.b	DOUBLE_BUF,DOUBLE_BUF
		dc.b	DOUBLE_BUF,DOUBLE_BUF	; centre
		dc.b	DOUBLE_BUF,DOUBLE_BUF	; 2
		dc.b	DOUBLE_BUF,DOUBLE_BUF	; 3
		dc.b	DOUBLE_BUF		; diagonal

	ENDC
;	IFNE 0
		dc.b	NON_DOUBLE,NON_DOUBLE	; blitin_blinds1,2
		dc.b	NON_DOUBLE

		dc.b	NON_DOUBLE,NON_DOUBLE
		dc.b	NON_DOUBLE,NON_DOUBLE

		dc.b	NON_DOUBLE,NON_DOUBLE

		dc.b	NON_DOUBLE,NON_DOUBLE	; climbing
		dc.b	NON_DOUBLE,NON_DOUBLE

		dc.b	NON_DOUBLE		; dissolve

		dc.b	NON_DOUBLE,NON_DOUBLE
		dc.b	NON_DOUBLE,NON_DOUBLE
		dc.b	NON_DOUBLE,NON_DOUBLE
		dc.b	NON_DOUBLE,NON_DOUBLE
		dc.b	NON_DOUBLE,NON_DOUBLE
		dc.b	NON_DOUBLE,NON_DOUBLE

		dc.b	NON_DOUBLE,NON_DOUBLE	; moz down up

		dc.b	NON_DOUBLE		; page fold
		dc.b	NON_DOUBLE,NON_DOUBLE	; corner
		dc.b	NON_DOUBLE,NON_DOUBLE
		dc.b	NON_DOUBLE,NON_DOUBLE	; centre
		dc.b	NON_DOUBLE,NON_DOUBLE	; 2
		dc.b	NON_DOUBLE,NON_DOUBLE	; 3
		dc.b	NON_DOUBLE		; diagonal
		dc.b	NON_DOUBLE		; cross1
		dc.b	NON_DOUBLE		; cross2
;	ENDC

	even

*
* Set the increment for the viewports
* and if interlace set the temp mode on LACE
*
check_cop_modes:
	move.b	#0,db_active_lace(a3)
	move.b	#0,db_inactive_lace(a3)
	move.w	db_or_vmode_mask(a3),db_tmode(a3)		; always LACE 
	move.l	db_effect_speed(a3),d0
	move.l	d0,db_active_inc(a3)
	move.w	#0,db_active_min(a3)

	move.w	db_max_y(a3),d1
	lsr.w	#1,d1
	move.w	d1,db_active_max(a3)

	tst.b	vb_interlace(a5)
	beq	.ch_cop_m1
	move.b	#1,db_active_lace(a3)
	move.w	#0,db_active_min(a3)
	move.w	db_max_y(a3),db_active_max(a3)
	add.l	d0,db_active_inc(a3)				; * 2
	move.w	db_or_vmode_mask(a3),db_tmode(a3)
.ch_cop_m1:

	move.w	#0,db_inactive_min(a3)
	move.w	d1,db_inactive_max(a3)
	move.l	d0,db_inactive_inc(a3)
	tst.b	vb_interlace(a4)
	beq	ch_cop_m2
	move.b	#1,db_inactive_lace(a3)
	move.w	#0,db_inactive_min(a3)
	move.w	db_max_y(a3),db_inactive_max(a3)
	add.l	d0,db_inactive_inc(a3)				; * 2
	move.w	db_or_vmode_mask(a3),db_tmode(a3)
ch_cop_m2:
	rts
*
* For the up version
*
cop_init_new2:
	move.w	#0,db_tmode(a3)
	move.l	#0,db_varwtime(a3)

	lea	teller(pc),a0
	move.l	db_varwtime(a3),off_teller(a0)
	move.l	db_varwtime(a3),off_tel_start(a0)
	clr.w	db_cop_inactive_test(a3)	; There is no top viewport 
	move.w	#TRUE,db_cop_active_test(a3)	; active is on
	
	bsr	check_cop_modes

	bsr	copy_view2
	bsr	switch_temp_views
	bsr	copy_view3

	bsr	init_user_coplistb
	move.l	itviewp(pc),a1
	move.l	db_ucop_pointer(a3),vp_UCopIns(a1)
	bsr	init_user_coplist00
	move.l	atviewp(pc),a1
	move.l	db_ucop_pointer(a3),vp_UCopIns(a1)

; So now there are copies of the two viewports in itviewp an atviewp

	move.l	atviewp(pc),a1
	move.l	itview(pc),a0		; Put the first in de view struct
	move.l	a1,v_ViewPort(a0)

	move.l	atview(pc),a0		; Put the first in de view struct
	move.l	a1,v_ViewPort(a0)

	move.l	itviewp(pc),a1
	move.w	vp_DyOffset(a1),db_inactive_min(a3)
	move.w	vp_DHeight(a1),db_final_height(a3)
	bra	set_finals
	rts

cop_init_new3:
	move.w	#0,db_tmode(a3)
	move.l	#0,db_varwtime(a3)
	lea	teller(pc),a0
	move.l	db_varwtime(a3),off_teller(a0)
	move.l	db_varwtime(a3),off_tel_start(a0)
	clr.w	db_cop_inactive_test(a3)	; There is no top viewport 
	move.w	#TRUE,db_cop_active_test(a3)	; active is on
	bsr	check_cop_modes

	bsr	copy_view2
	bsr	switch_temp_views
	bsr	copy_view3

;	move.l	atview(pc),a1
;	move.w	v_Modes(a1),d0

	bsr	init_user_coplist
	move.l	atviewp(pc),a1		; Put the first in de view struct
	move.l	db_ucop_pointer(a3),vp_UCopIns(a1)

	bsr	init_user_coplist0
	move.l	itviewp(pc),a1
	move.l	db_ucop_pointer(a3),vp_UCopIns(a1)
	
; So now there are copies of the two viewports in itviewp an atviewp
	move.l	atviewp(pc),a1
	move.l	itview(pc),a0		; Put the first in de view struct
	move.l	a1,v_ViewPort(a0)
	move.l	atview(pc),a0		; Put the first in de view struct
	move.l	a1,v_ViewPort(a0)
set_finals:
	move.l	itviewp(pc),a1
	move.w	vp_DHeight(a1),db_final_height(a3)
	move.w	vp_DyOffset(a1),db_dy_org_inactive(a3)

	move.l	atviewp(pc),a1
	move.w	vp_DHeight(a1),db_final_height_active(a3)
	move.w	vp_DyOffset(a1),db_dy_org_active(a3)
	rts

show_cops:
	tst.b	db_reverse_cop(a3)
	beq	normal_cop

	move.l	db_copstore(a3),a5		; use last saved cop
	subq.l	#8,a5
rep_cop_place_rev:
	move.l	itview(pc),a1
	move.l	(a5),d0
	beq	no_more_place
	move.l	4(a5),d1
	subq.l	#8,a5
	move.l	d0,a2
	move.l	crl_start(a2),a2

	move.l	d0,v_LOFCprList(a1)
	move.l	d1,v_SHFCprList(a1)
	move.l	db_graphbase(a3),a6
	jsr	_LVOLoadView(a6)
	bsr	wacht_tijd2
	tst.b	d0
	bne	no_more_place
	bra	rep_cop_place_rev
	
normal_cop:
	move.l	db_wstore_moves2(a3),a5
	addq.l	#8,a5
rep_cop_place:
	move.l	itview(pc),a1
	move.l	(a5)+,d0
	beq	no_more_place
	move.l	(a5)+,d1
	move.l	d0,a2
	move.l	crl_start(a2),a2

	move.l	d0,v_LOFCprList(a1)
	move.l	d1,v_SHFCprList(a1)
	move.l	db_graphbase(a3),a6
	jsr	_LVOLoadView(a6)

	bsr	wacht_tijd2
	tst.b	d0
	bne	no_more_place
	bra	rep_cop_place
no_more_place:	
	move.l	itview(pc),a1
	move.l	#0,v_LOFCprList(a1)
	move.l	#0,v_SHFCprList(a1)
	rts

lofframe:	ds.b	crl_SIZEOF
shfframe:	ds.b	crl_SIZEOF

clear_cop_lists2:
	move.l	db_graphbase(a3),a6
	move.l	db_wstore_moves2(a3),a5
	addq.l	#8,a5
rep_clear_copl1:
	move.l	(a5)+,d0
	beq	nom_copl1
	move.l	d0,a0
	jsr	_LVOFreeCprList(a6)
	move.l	(a5)+,d0
	move.l	d0,a0
	jsr	_LVOFreeCprList(a6)
;	addq.l	#8,a5
	bra	rep_clear_copl1
nom_copl1:
	rts

	move.l	db_wstore_moves2(a3),a5
	lea.l	16(a5),a5
rep_clear_copl2
	move.l	(a5)+,d0
	beq	nom_copl2
	move.l	d0,a0
	jsr	_LVOFreeCprList(a6)
	move.l	(a5)+,d0
	move.l	d0,a0
	jsr	_LVOFreeCprList(a6)
	addq.l	#8,a5
	bra	rep_clear_copl2
nom_copl2:
	rts

clear_cop_lists:
	move.l	db_graphbase(a3),a6
	move.l	db_wstore_moves2(a3),a5
	addq.l	#8,a5
rep_clear_copl:
	move.l	(a5)+,d0
	beq	nom_copl
	move.l	d0,a0
	jsr	_LVOFreeCprList(a6)
	bra	rep_clear_copl
nom_copl:
	rts

DD = 10

*
* Create a fall down with eas-in and eas-out
* update a1 pointer with values
* in d6 & d5 the sum value
*
create_do_up:
	move.l	db_varltimes(a3),d0	; drop down varltimes as size
	bne	oke_pl2
	moveq	#1,d0
oke_pl2:
	add.l	d0,d0
	move.l	d0,db_varltimes(a3)

	move.l	db_effect_speed(a3),d1
	lsr.w	d1
	bne	oke_fl1
	moveq	#1,d1
oke_fl1:
	bsr	init_flip_angles
	moveq	#0,d6
	move.l	db_varltimes(a3),d7
	move.l	db_wpatroonruimte(a3),a1
rep_tt3:
	movem.l	a1/d6/d7,-(a7)
	bsr	get_sinus
	movem.l	(a7)+,a1/d6/d7
	add.w	d0,d6
	neg.w	d0
	move.w	d0,(a1)+
	dbf	d7,rep_tt3
	move.w	d6,d5

	move.l	a1,a2
	move.l	db_varltimes(a3),d0	; slow down to bottom
rep_tt2:
	move.w	-(a2),(a1)+
	dbf	d0,rep_tt2
	add.w	d5,d5			; nr. off pixels down
	rts
	
create_sinus_easin_out:
	move.w	db_max_y(a3),d2
	add.w	db_gapsize(a3),d2
	move.w	d2,db_max_height_cop(a3)	; total height cop effect
	move.l	db_wpatroonruimte(a3),a1
	moveq	#0,d5
	cmp.l	#0,db_variation(a3)
	beq	no_downfall

	cmp.l	#1,db_variation(a3)
	beq	hard_move

	bsr	create_do_up		; fall down a bit
	cmp.l	#4,db_variation(a3)
	bne	no_var4
	move.l	a1,db_tempoff(a3)	; start from here later on

no_var4:
	cmp.l	#3,db_variation(a3)
	bne	no_addd			; we want this at the top too
	add.w	d5,d5			; so increase to way to move
no_addd:
no_downfall:

	moveq	#DD,d0
	move.l	db_effect_speed(a3),d1
	movem.l	d5/a1,-(a7)
	bsr	init_flip_angles
	movem.l	(a7)+,d5/a1

	moveq	#DD-1,d7
	moveq	#0,d6
rep_tt:
	movem.l	a1/d6/d7,-(a7)
	bsr	get_sinus
	movem.l	(a7)+,a1/d6/d7
	move.w	d0,(a1)+
	add.w	d0,d6
	dbf	d7,rep_tt

	move.l	a1,a2
;	move.w	#297,d7
;	move.w	db_max_y(a3),d7
	move.w	db_max_height_cop(a3),d7
	lsr.w	#1,d7
	add.w	d5,d7		; add delta falldown value
	move.w	d7,d3

	move.w	d6,d4
	add.w	d4,d4
	sub.w	d4,d7
	bpl	no_rep_a1
	moveq	#0,d7
no_rep_a1:
	move.l	db_effect_speed(a3),d1
	divu	d1,d7
	move.l	d7,d0
rep_sames:
	add.w	d1,d6
	move.w	d1,(a1)+	; add rest/speed times to speed value
rep_sames2:
	dbf	d7,rep_sames

	moveq	#DD-1,d7	; add the slow down range again
rep_agains:
	move.w	-2(a2),(a1)+
	add.w	-2(a2),d6
	subq.l	#2,a2
	dbf	d7,rep_agains

exit_agains:
	moveq	#1,d2
	cmp.l	#3,db_variation(a3)
	beq	do_addd2
	cmp.l	#4,db_variation(a3)
	bne	no_addd2
do_addd2:
	move.l	db_effect_speed(a3),d1
	swap	d0
	sub.w	d0,d1
rep_addd4:
	move.l	a1,a2
rep_addd4_b:
	sub.w	#1,-(a2)
	sub.w	#1,d6
	sub.w	#1,d1
	beq	no_addd3
	bpl	rep_addd4_b
	neg.w	d1

	move.w	d1,(a1)+
	add.w	d1,d6
	
no_addd3:
	moveq	#-1,d2
	move.l	db_wpatroonruimte(a3),a2	; we are now shooting over
	move.l	db_varltimes(a3),d0		; so add the down range here
	add.l	d0,d0
rep_tt4:
	move.w	(a2)+,(a1)+
	dbf	d0,rep_tt4

	move.w	d3,d7
	sub.w	d6,d7		; should be zero if not add one to make it so
	bmi	eas_minus_min
	beq	no_eas_sin
	bra	rep_one2
	
no_addd2:
;	move.w	#297,d7
;	move.w	db_max_y(a3),d7
	move.w	db_max_height_cop(a3),d7
	lsr.w	#1,d7
	add.w	d5,d7		; add delta falldown value

	move.w	d3,d7

	sub.w	d6,d7		; should be zero if not add one to make it so
	bmi	eas_minus
	beq	no_eas_sin
rep_one2:
	subq.w	#1,d7
rep_one1:
	move.w	d2,(a1)+
	add.w	d2,d6
	dbf	d7,rep_one1

no_eas_sin:
	move.w	#-1000,(a1)+
	cmp.l	#4,db_variation(a3)
	bne	no_var42
	rts
*
* A non eas move indicating speed until you reach the end
*
hard_move:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	dbstr5023(pc),a0
	jsr	KPutStr
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

;	move.w	#297,d7
;	move.w	db_max_y(a3),d7
	move.w	db_max_height_cop(a3),d7
	lsr.w	#1,d7

	moveq	#0,d0
	move.l	db_effect_speed(a3),d1
rep_hm:
	move.w	d1,(a1)+
	add.w	d1,d0
	cmp.w	d0,d7
	bgt	rep_hm	

	beq	oke_hm
	sub.w	d7,d0
	sub.w	d0,-2(a1)		; exact position
oke_hm:
	move.w	#-1000,(a1)+
;	rts

no_var42:
	move.l	db_wpatroonruimte(a3),db_tempoff(a3)
	rts

eas_minus:
	move.l	a1,a2
rep_eas_minus:	
	tst.w	-2(a2)
	bne	.non_nul
	subq.l	#2,a2
	bra	rep_eas_minus
.non_nul:
	sub.w	#1,-(a2)
	subq.w	#1,d6
	cmp.w	d6,d3
	blt	rep_eas_minus
	bra	no_addd2

eas_minus2:
	move.w	-(a1),d0
	sub.w	d0,d6
	cmp.w	d6,d3
	blt	eas_minus
	move.w	d3,d7
	sub.w	d6,d7
	beq	no_addd2
	add.w	d7,d6
	move.w	d7,(a1)+
	bra	no_addd2

eas_minus_min:
	move.l	a1,a2
rep_eas_minus_min:	
	sub.w	#1,-(a2)
	add.w	#1,d6
	cmp.w	d6,d3
	blt	rep_eas_minus_min
	bra	no_addd2

eas_minus_min2:
	move.w	-(a1),d0
	add.w	d0,d6
	cmp.w	d6,d3
	blt	eas_minus_min
	move.w	d3,d7
	sub.w	d6,d7
	beq	no_addd2
	sub.w	d7,d6
	move.w	d7,(a1)+
	bra	no_addd2

init_cop_up:
	move.l	db_inactive_viewblok(a3),a4
	move.l	db_active_viewblok(a3),a5
do_init_cop:
	bsr	create_sinus_easin_out
	bsr	cop_init_new2
	move.l	vb_viewportw(a4),a0
	move.w	vp_DyOffset(a0),d0
	move.w	db_max_y(a3),d2

	move.w	db_max_height_cop(a3),d2

	tst.b	vb_interlace(a4)
	bne	.no_lace101
	lsr.w	#1,d2
.no_lace101:
	add.w	d2,d0
	move.l	itviewp(pc),a0
	move.w	d0,vp_DyOffset(a0)	; set start value dy offset
	move.l	db_active_inc(a3),d0
	move.l	atviewp(pc),a0
	move.l	vp_RasInfo(a0),a2

	move.l	atviewp(pc),a0
	move.w	vp_DyOffset(a0),d1
	move.l	d0,db_dyinactive(a3)
	cmp.w	db_active_min(a3),d1
	bge	.no_chan
	move.w	d1,db_active_min(a3)	; use the dy value if its smaller(NTSC)
.no_chan:
	rts

init_cop_down:
	move.w	db_max_y(a3),d2
	add.w	db_gapsize(a3),d2
	move.w	d2,db_max_height_cop(a3)	; total height cop effect
	
	move.l	#0,db_tuser_cop(a3)
	move.l	db_inactive_viewblok(a3),a4
	move.l	db_active_viewblok(a3),a5
	bsr	create_sinus_easin_out
	bsr	cop_init_new3

	move.l	vb_viewportw(a4),a0
	move.w	vp_DyOffset(a0),d0

	move.w	db_max_height_cop(a3),d2
	tst.b	vb_interlace(a4)
	bne	.no_lace101_d
	lsr.w	#1,d2
.no_lace101_d:
	sub.w	d2,d0
	ext.l	d0

	move.l	atviewp(pc),a0
	move.w	vp_DyOffset(a0),d1
	move.l	d0,db_dyinactive(a3)
	cmp.w	db_active_min(a3),d1
	bge	.no_chan
	move.w	d1,db_active_min(a3)	; use the dy value if its smaller(NTSC)
.no_chan:
	move.w	db_active_min(a3),db_dyactive(a3)

	move.l	itviewp(pc),a0
	move.w	vp_DyOffset(a0),d0
	sub.w	db_inactive_min(a3),d0
	move.w	d0,db_topoffset(a3)
	move.w	db_inactive_max(a3),d1
	sub.w	db_inactive_min(a3),d1

	move.w	db_max_height_cop(a3),d1
	tst.b	vb_interlace(a4)
	bne	.no_lace1012
	lsr.w	#1,d1
.no_lace1012:

	sub.w	d0,d1
	sub.w	db_final_height(a3),d1
	move.w	d1,db_bottom_offset(a3)
	move.l	itviewp(pc),a0
	move.l	vp_RasInfo(a0),a2
	move.w	#TRUE,db_cop_active_test(a3)	; set active viewport on
	move.w	#FALSE,db_cop_inactive_test(a3)	; set inactive viewport off
	move.l	itviewp(pc),a1
	move.w	#0,vp_DHeight(a1)
	move.w	vp_DyOffset(a1),d0
	cmp.w	db_inactive_min(a3),d0
	bge	.use_old_dy
	move.w	vp_DyOffset(a1),db_inactive_min(a3)
.use_old_dy:
	move.w	db_inactive_min(a3),vp_DyOffset(a1)
	rts
*
* Find and set the loadview pointer in db_coplist
* also set the aview in the cpp_view pointer
*
find_and_set_loadview:
	move.l	atview(pc),a0
	move.l	db_coplist(a3),a1
	move.l	a0,cpp_view(a1)
	move.l	db_graphbase(a3),a6
	jsr	_LVOGfxLookUp(a6)
	move.l	d0,d7
	move.l	d0,a0
	beq	.normal_loadview
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Found lookup",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	ve_Monitor(a0),d0
	beq	.normal_loadview
	move.l	d0,a0
	move.l	ms_LoadView(a0),d0
	beq	.normal_loadview
	move.l	d0,a0
	bra	.set_loadview	

.normal_loadview:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr2(pc),a0
	jsr	KPutStr
	bra	.tt2
.dbstr2:	dc.b	"Set to normal",10,0
	even
.tt2:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC
	moveq	#0,d7
	move.l	db_graphbase(a3),a6
	lea	_LVOLoadView(a6),a0

.set_loadview:
	move.l	db_coplist(a3),a1
	move.l	a0,cpp_loadview(a1)
	move.l	d7,cpp_vextra(a1)
	rts

cop_down_bottom:
	move.l	#4,db_variation(a3)
	bra	do_cop_down

cop_down_double:
	move.l	#3,db_variation(a3)

do_cop_down:

cop_down1_v1:
	bsr	check_hard
	bsr	init_cop_play_list
	move.b	#0,db_reverse_cop(a3)
	bsr	init_cop_down
	bsr	find_and_set_loadview

;	lea	teller,a0
;	move.l	#10,(a0)
;	move.l	#10,4(a0)

	bsr	rep_cop_down1

end_cop_down1:
	bsr	remove_50h_cpl
	bsr	showpicture
	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)
	move.l	atviewp(pc),a0
	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBOVP(a6)
	bsr	remove_cpr_task
	bsr	clear_cpp_block
	bsr	switch_temp_views
	bsr	free_view_ucop
	bsr	switch_temp_views
	bsr	free_view_ucop
	rts

cop_reveal_down2:
	move.l	#2,db_variation(a3)
cop_reveal_down:
	bsr	check_hard
	bsr	init_cop_play_list
	move.b	#0,db_reverse_cop(a3)
	bsr	init_cop_down
	bsr	find_and_set_loadview
	moveq	#0,d0
	move.w	db_inactive_min(a3),d0
	sub.w	db_gapsize(a3),d0
	sub.w	db_gapsize(a3),d0
	ext.l	d0
	move.l	d0,db_dyinactive(a3)
	bsr	rep_cop_down4
	bra	end_cop_down1

rep_cop_down4:
	bsr	easy_check		; set increment
	move.l	atviewp(pc),a1
	move.l	db_active_inc(a3),d0
	add.w	d0,vp_DyOffset(a1)	; down the active
	move.l	db_inactive_inc(a3),d0
	ext.l	d0
	add.l	d0,db_dyinactive(a3)	; down the inactive

	bsr	check_inactive2		; set the visible part inactive
	bsr	wait_free
	bne	exit_cop_down4
	bsr	create_cop_view

	move.l	db_tempoff(a3),a0
	cmp.w	#-1000,(a0)
	beq	exit_cop_up22
	bra	rep_cop_down4
exit_cop_down4:
	rts
*
* The picture is revealed here
*
check_inactive2:

	move.l	itviewp(pc),a1
	move.w	#0,vp_DHeight(a1)

	moveq	#0,d1
	move.w	db_inactive_min(a3),d1
	move.l	db_dyinactive(a3),d0
	move.w	d0,vp_DyOffset(a1)

	cmp.w	db_dy_org_inactive(a3),d0
	ble	.no_height

	move.w	db_dy_org_inactive(a3),vp_DyOffset(a1)
	sub.w	db_dy_org_inactive(a3),d0		; the height
	cmp.w	db_final_height(a3),d0
	bge	.full_height	
	move.w	d0,vp_DHeight(a1)
	bra	.do_make_down1
.full_height:
	move.w	db_final_height(a3),vp_DHeight(a1)
	move.l	vp_RasInfo(a1),a0
	move.w	#0,ri_RyOffset(a0)
.no_height:

.do_make_down1:
	move.l	db_dyinactive(a3),d0
	move.w	db_dy_org_inactive(a3),d1
	sub.w	db_gapsize(a3),d0
	cmp.w	d0,d1
	bge	.no_inac	
	move.w	#TRUE,db_cop_inactive_test(a3)	; set inactive viewport on
.no_inac:
	rts

rep_cop_down1:
	bsr	easy_check		; set increment
	move.l	atviewp(pc),a1
	move.l	db_active_inc(a3),d0
	add.w	d0,vp_DyOffset(a1)	; down the active
	move.w	vp_DyOffset(a1),d0
	
	move.l	db_inactive_inc(a3),d0
	ext.l	d0
	add.l	d0,db_dyinactive(a3)	; down the inactive

	bsr	check_inactive		; set the visible part inactive

	bsr	wait_free
;	bsr	wacht_tijd2
	bne	exit_cop_down1

	bsr	create_cop_view

	move.l	itviewp(pc),a1
	move.w	vp_DHeight(a1),d0
	move.w	vp_DyOffset(a1),d0

	move.l	atviewp(pc),a1
	move.w	vp_DHeight(a1),d0
	move.w	vp_DyOffset(a1),d0

	move.l	db_tempoff(a3),a0
	cmp.w	#-1000,(a0)
	beq	exit_cop_up22
	bra	rep_cop_down1
exit_cop_down1:
	rts

cop_cover_down2:
	move.l	#3,db_variation(a3)
cop_cover_down:
	bsr	check_hard
	bsr	init_cop_play_list
	move.b	#0,db_reverse_cop(a3)
	bsr	init_cop_down
	bsr	find_and_set_loadview
	bsr	rep_cop_down2
	bra	end_cop_down1
	
rep_cop_down2:
	bsr	easy_check		; set increment

	move.l	db_active_inc(a3),d0
	add.w	d0,db_dyactive(a3)

	moveq	#0,d0
	move.w	db_dyactive(a3),d0

	sub.w	db_dy_org_active(a3),d0
	bmi	.no_active_move	

; if positive the active should reduce in height from the top

	move.l	atviewp(pc),a1

	move.w	db_dy_org_active(a3),vp_DyOffset(a1)

	add.w	d0,vp_DyOffset(a1)	; down the active
	move.l	vp_RasInfo(a1),a0
	move.w	d0,ri_RyOffset(a0)
	move.w	db_final_height_active(a3),vp_DHeight(a1)
	sub.w	d0,vp_DHeight(a1)
	cmp.w	#0,vp_DHeight(a1)
	bge	.no_active_move
	move.w	#0,vp_DHeight(a1)
.no_active_move:
	move.l	db_inactive_inc(a3),d0
	ext.l	d0
	add.l	d0,db_dyinactive(a3)	; down the inactive

	bsr	check_inactive		; set the visible part inactive

;ppp:
;	move.l	itviewp(pc),a1
;	move.w	vp_DHeight(a1),d0
;	move.w	vp_DyOffset(a1),d0

;	move.l	atviewp(pc),a1
;	move.w	vp_DHeight(a1),d0
;	move.w	vp_DyOffset(a1),d0

	bsr	wait_free
	bne	exit_cop_down1

	bsr	create_cop_view

	move.l	db_tempoff(a3),a0
	cmp.w	#-1000,(a0)
	beq	exit_cop_up22
	bra	rep_cop_down2

wait_free:
	move.l	db_coplist(a3),a0
	move.l	cpp_storep(a0),a1
	tst.w	(a1)
	beq	oke_wait_free
	tst.w	cpp_init(a0)
	bne	al_init_cpl2
	bsr	init_cpl_int		; start the interrupt
al_init_cpl2:
	bsr	wacht_tijd2
	tst.b	d0
	bne	exit_wait_free
	bra	wait_free		; wait for pointer to clear

oke_wait_free:
	bsr	check_but2
	tst.b	d0
	bne	exit_wait_free_deb
	moveq	#0,d0
	rts

exit_wait_free:
	IFNE PRAND
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Exit wait free",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	moveq	#-1,d0
	rts

exit_wait_free_deb:
	IFNE PRAND
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dat1(pc),a1
	move.l	d0,(a1)
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"Exit wait free na check but %lx",10,0
	even
.dat1:	dc.l	0
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	moveq	#-1,d0
	rts
*
* Create the visible part of the inactive view
* the inactive stays off until it is needed
*
check_inactive:
	moveq	#0,d1
	move.w	db_inactive_min(a3),d1
	move.l	db_dyinactive(a3),d0
	add.w	db_final_height(a3),d0
	ext.l	d0	
	cmp.w	d1,d0
	blt	do_make_down2
	sub.l	d1,d0

	cmp.w	db_final_height(a3),d0
	bgt	whole_vis2

	move.l	itviewp(pc),a1
	cmp.w	#0,d0
	bge	.okie
	moveq	#0,d0
.okie:
;	sub.w	#4,d0
	move.w	d0,vp_DHeight(a1)		; set height

	move.w	db_final_height(a3),d1
	sub.w	d0,d1
	move.w	d1,ri_RyOffset(a2)
	
	move.w	db_inactive_min(a3),vp_DyOffset(a1)
	move.w	#TRUE,db_cop_inactive_test(a3)	; set inactive viewport on
	bra	do_make_down1

whole_vis2:
	move.w	db_inactive_min(a3),d1		; should never come here
	move.l	db_dyinactive(a3),d0

; the whole view is visible calculate offset
;
whole_vis1:
	move.l	db_dyinactive(a3),d0
	move.l	itviewp(pc),a1
	move.w	vp_DHeight(a1),d1		; debug 
	move.w	db_final_height(a3),vp_DHeight(a1)	; set orginal height
	move.w	#0,ri_RyOffset(a2)			; set offset orginal
	move.w	d0,vp_DyOffset(a1)			; the offset
	bra	do_make_down1

do_make_down2:

	move.l	atviewp(pc),a1
	move.w	vp_DyOffset(a1),d1
	move.l	db_dyinactive(a3),d0
	add.w	db_final_height(a3),d0
	ext.l	d0	
	cmp.w	d1,d0
	blt	do_make_down1

	move.l	db_dyinactive(a3),d0
	add.w	db_final_height(a3),d0
	add.w	db_bottom_offset(a3),d0
	sub.w	db_inactive_min(a3),d0
	ext.l	d0
	bmi	do_make_down1
	beq	do_make_down1
	move.w	#TRUE,db_cop_inactive_test(a3)	; set inactive viewport on
do_make_down1:
	rts

check_hard:
	tst.l	db_variation(a3)
	bne	.no_varlcheck
	cmp.l	#1,db_varltimes(a3)
	beq	.no_varlcheck
	move.l	#1,db_variation(a3)
.no_varlcheck:
	rts	


cop_up1_v4:
	move.l	#4,db_variation(a3)
	bra	cop_up1
cop_up_double:
	move.l	#3,db_variation(a3)
	bra	cop_up1
cop_up_top:
	move.l	#2,db_variation(a3)
*
* Scroll up with two different viewports
* and a user copper list
*
cop_up1:
	bsr	check_hard
	bsr	init_cop_play_list
	move.b	#0,db_reverse_cop(a3)
	bsr	init_cop_up
	bsr	find_and_set_loadview
	bsr	rep_cop_up1
end_cop_up1:
	bsr	remove_50h_cpl
	bsr	showpicture
	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)
	move.l	atviewp(pc),a0
	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBOVP(a6)
	bsr	remove_cpr_task
	bsr	clear_cpp_block
	bsr	switch_temp_views
	bsr	free_view_ucop
	bsr	switch_temp_views
	bsr	free_view_ucop
	rts

cop_reveal_up2:
	move.l	#2,db_variation(a3)
cop_reveal_up:
	bsr	check_hard
	bsr	init_cop_play_list
	move.b	#0,db_reverse_cop(a3)
	bsr	init_cop_up
	bsr	find_and_set_loadview
	bsr	rep_cop_up3
	bra	end_cop_up1

clear_cpp_block:
	move.l	db_coplist(a3),a2
	move.w	cpp_size(a2),d7
	lea	cpp_lists(a2),a4
rep_cl_cpp:
	move.l	2(a4),a0
	move.l	db_graphbase(a3),a6
	jsr	_LVOFreeCprList(a6)
	move.l	6(a4),a0
	move.l	db_graphbase(a3),a6
	jsr	_LVOFreeCprList(a6)

	lea	12(a4),a4
	dbf	d7,rep_cl_cpp

	move.l	atview(pc),a1
	clr.l	v_LOFCprList(a1)		; already freed
	clr.l	v_SHFCprList(a1)
	rts

rep_cop_up3:				; Push the active up
	bsr	easy_check		; set increment

	move.l	atviewp(pc),a1
	move.l	db_active_inc(a3),d0
	sub.w	d0,vp_DyOffset(a1)
	move.w	vp_DyOffset(a1),d1

	move.w	db_active_min(a3),d2
	cmp.w	d1,d2
	ble	up3_nopo

	cmp.w	#6,vp_DHeight(a1)
	bgt	no1
tpp:
	nop
	nop
no1:
	move.l	vp_RasInfo(a1),a2
	move.w	vp_DHeight(a1),d0
	move.w	ri_RyOffset(a2),d0

	move.w	db_active_min(a3),d0
	move.w	db_active_min(a3),vp_DyOffset(a1)
	sub.w	d1,d2
	move.l	vp_RasInfo(a1),a2
	sub.w	d2,vp_DHeight(a1)
	bgt	up3_ok1
set4:
	move.w	#0,vp_DHeight(a1)
	move.w	#0,ri_RyOffset(a2)
	bra	up3_nopo
up3_ok1:
;	cmp.w	#4,vp_DHeight(a1)
;	ble	set4
	
	add.w	d2,ri_RyOffset(a2)
up3_nopo:

; Is there room at the bottom for the inactive	

	move.l	db_inactive_inc(a3),d0

	move.w	#TRUE,db_cop_inactive_test(a3)

	move.l	itviewp(pc),a1
	move.l	db_inactive_inc(a3),d0
	sub.w	d0,vp_DyOffset(a1)

;
; Check how much of the inactive view is visible
;

	move.w	#0,vp_DHeight(a1)
	move.w	db_dy_org_inactive(a3),d0
	add.w	db_final_height(a3),d0
	sub.w	vp_DyOffset(a1),d0
	bmi	.no_height
	cmp.w	db_final_height(a3),d0
	bge	.end_height
	move.w	d0,vp_DHeight(a1)
	move.w	db_final_height(a3),d1
	sub.w	d0,d1
	move.l	vp_RasInfo(a1),a0
	move.w	d1,ri_RyOffset(a0)
	bra	.no_height

.end_height:
	move.w	db_final_height(a3),vp_DHeight(a1)
	move.l	vp_RasInfo(a1),a0
	move.w	#0,ri_RyOffset(a0)
.no_height:

	move.l	itviewp(pc),a1
	move.w	vp_DyOffset(a1),d0
	move.l	vb_viewportw(a5),a0
	move.w	vp_DyOffset(a0),d1
	add.l	db_active_inc(a3),d1
	add.w	db_gapsize(a3),d1
	add.w	db_gapsize(a3),d1
	cmp.w	d1,d0
	bge	no_inactive_up3
	move.w	#FALSE,db_cop_active_test(a3)	; set active viewport off
no_inactive_up3:
	move.l	atviewp(pc),a0
	move.w	vp_DyOffset(a0),d0
	move.w	vp_DHeight(a0),d0

	move.l	vp_RasInfo(a0),a0
	move.w	ri_RyOffset(a0),d0

	move.l	itviewp(pc),a0
	move.w	vp_DyOffset(a0),d0

	move.l	vp_RasInfo(a0),a0
	move.w	ri_RyOffset(a0),d0

	bsr	wait_free
	bne	exit_cop_up3
	bsr	create_cop_view

	move.l	db_tempoff(a3),a0
	cmp.w	#-1000,(a0)
	beq	exit_cop_up22
	bra	rep_cop_up3

exit_cop_up3:
	rts

rep_cop_up1:				; Push the active up

	bsr	easy_check		; set increment

	move.l	atviewp(pc),a1
	move.l	db_active_inc(a3),d0
	sub.w	d0,vp_DyOffset(a1)
	move.w	vp_DyOffset(a1),d1
	move.w	db_active_min(a3),d2
	cmp.w	d1,d2
	ble	up1_nopo

	move.w	db_active_min(a3),vp_DyOffset(a1)
	sub.w	d1,d2
	move.w	vp_DHeight(a1),d0
	move.l	vp_RasInfo(a1),a2
	sub.w	d2,vp_DHeight(a1)
	cmp.w	#0,vp_DHeight(a1)
	bge	up1_ok1
	move.w	#0,vp_DHeight(a1)
	move.w	#0,ri_RyOffset(a2)
	bra	up1_nopo
up1_ok1:
	add.w	d2,ri_RyOffset(a2)
	move.w	ri_RyOffset(a2),d2
up1_nopo:

; Is there room at the bottom for the inactive	

	move.l	db_inactive_inc(a3),d0
	move.w	d7,d2
	add.w	d0,d2
	add.w	db_gapsize(a3),d2

	move.w	#TRUE,db_cop_inactive_test(a3)

	move.l	itviewp(pc),a1
	move.l	db_inactive_inc(a3),d0
	sub.w	d0,vp_DyOffset(a1)

	move.w	vp_DyOffset(a1),d0
	move.l	vb_viewportw(a5),a0
	move.w	vp_DyOffset(a0),d1
	add.l	db_active_inc(a3),d1
	add.w	db_gapsize(a3),d1
	add.w	db_gapsize(a3),d1
;	add.w	#4,d1
	cmp.w	d1,d0
	bge	no_inactive_up1
ptt:
	move.w	#FALSE,db_cop_active_test(a3)	; set active viewport off
no_inactive_up1:
	bsr	wait_free
	bne	exit_cop_up1

oke_create_cop:

	bsr	create_cop_view

	move.l	itviewp(pc),a1
	move.w	vp_DHeight(a1),d0
	move.w	vp_DyOffset(a1),d0

	move.l	atviewp(pc),a1
	move.w	vp_DHeight(a1),d0
	move.w	vp_DyOffset(a1),d0

	move.l	db_tempoff(a3),a0
	cmp.w	#-1000,(a0)
	beq	exit_cop_up22
	bra	rep_cop_up1

exit_cop_up1:
	rts

*
* You must finish up by playing the buffer empty
*
exit_cop_up22:
	move.l	db_coplist(a3),a0	; make sure the task is running
	tst.w	cpp_init(a0)
	bne	no_init_cpl2
	bsr	init_cpl_int		; if not start afterall
no_init_cpl2:

	move.l	db_coplist(a3),a0
	lea.l	cpp_lists(a0),a1
	move.w	cpp_size(a0),d0		; numer of lists
check_lists1:
	cmp.w	#2,(a1)
	bge	more_lists
	lea.l	12(a1),a1
	dbf	d0,check_lists1
	bra	all_lists_checked
more_lists:

	bsr	wacht_tijd2
	tst.b	d0
	bne	exit_cop_up1
	bra	exit_cop_up22
all_lists_checked:
	rts

remove_cpr_task:
	move.l	db_coplist(a3),a1
	move.l	cpp_signum(a1),d0
	beq	no_sign2
	move.l	$4.w,a6
	jsr	_LVOFreeSignal(a6)
	move.l	db_coplist(a3),a1
	clr.l	cpp_signum(a1)			; the task waits for this
	clr.l	cpp_sig(a1)
no_sign2:
	move.l	db_task(a3),d0
	beq	no_task1
	move.l	d0,a1
	move.l	$4.w,a6
	jsr	_LVORemTask(a6)
	move.l	db_task(a3),a1
	move.l	#TC_SIZE+STACK_SIZE,d0
	jsr	_LVOFreeMem(a6)
	clr.l	db_task(a3)
no_task1:
	rts

STACK_SIZE = 1000
*
* First intialize the interrupt than initialize the task
*
init_cpl_int:
	lea.l	intstructcpl(pc),a1
	lea	proc50hcplname(pc),a0
	move.l	a0,10(a1)
	lea	cpp_data(pc),a0
	move.l	a0,14(a1)
	lea	proc_50h_cpl(pc),a0
	move.l	a0,18(a1)

	moveq	#5,d0
	move.l	$4.w,a6
	jsr	_LVOAddIntServer(a6)
	lea	cpp_data(pc),a0
	move.w	#1,cpp_init(a0)

	moveq	#-1,d0
	move.l	$4.w,a6
	jsr	_LVOAllocSignal(a6)
	cmp.l	#-1,d0
	beq	task_mem_err

	move.l	db_coplist(a3),a1
	move.l	d0,cpp_signum(a1)			; now it starts
	moveq	#1,d1
	lsl.l	d0,d1
	move.l	d1,cpp_sig(a1)

	move.l	#TC_SIZE+STACK_SIZE,d0
	move.l	#MEMF_CLEAR!MEMF_PUBLIC,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,db_task(a3)
	beq	task_mem_err
	move.l	d0,a0
	move.l	d0,a1
	add.l	#TC_SIZE,a1
	move.l	a1,TC_SPLOWER(a0)
	move.l	a1,TC_SPUPPER(a0)
	add.l	#STACK_SIZE,TC_SPUPPER(a0)
	move.l	TC_SPUPPER(a0),TC_SPREG(a0)
	move.b	#21,LN_PRI(a0)
	move.b	#NT_TASK,LN_TYPE(a0)
	lea	taskcprname(pc),a6
	move.l	a6,LN_NAME(a0)
	
	movem.l	a1/a2/a3,-(a7)
	move.l	db_task(a3),a1
	lea	task_cpl(pc),a2
	move.l	#0,a3	
	move.l	$4.w,a6
	jsr	_LVOAddTask(a6)
	movem.l	(a7)+,a1/a2/a3

	move.l	db_coplist(a3),a1
	move.l	db_task(a3),cpp_task(a1)

	moveq	#0,d0
	rts
	
task_mem_err:
	moveq	#-1,d0
	rts

remove_50h_cpl:
	move.l	db_coplist(a3),a2
	tst.w	cpp_init(a2)
	beq	no_cl_cpl
	lea.l	intstructcpl(pc),a1
	moveq	#5,d0
	move.l	$4.w,a6
	jsr	_LVORemIntServer(a6)
	move.l	db_coplist(a3),a0
	move.w	#$0,cpp_init(a0)
no_cl_cpl:
	rts

proc_50h_cpl:
;	addq.l	#1,stell
;	cmp.l	#4,stell
;	ble	no_do
;	clr.l	stell
;	move.w	#$f00,$dff180
	
	lea	cpp_data(pc),a1
	move.l	cpp_task(a1),d1
	beq	no_do
	move.l	cpp_sig(a1),d0
	move.l	d1,a1
	move.l	$4.w,a6
	jsr	_LVOSignal(a6)			; signal the cpl task
no_do:
	moveq	#0,d0
	rts

stell:	dc.l	0

*
* This task waits for 50hz and then puts the next cprlist on the screen
*
task_cpl:
wait_further:
	lea	cpp_data(pc),a1
	move.l	cpp_sig(a1),d0
	move.l	$4.w,a6
	jsr	_LVOWait(a6)		; wait for the 50hz signal

	lea	cpp_data(pc),a1

	move.l	cpp_playedp(a1),a0
check_old:
	cmp.w	#3,(a0)
	beq	no_clear11

	tst.w	(a0)
	beq	no_clear11
	
	cmp.w	#2,(a0)
	bne	no_first_time_pd
; the first time the first is a 2 
	move.w	#1,(a0)
	bra	no_clear11

no_first_time_pd:
	move.w	#0,(a0)			; from 1 to zero
	lea.l	12(a0),a0
	cmp.w	#-1,(a0)
	bne	no_clear1b
	lea	cpp_lists(a1),a0	; set to new start	
no_clear1b:
	move.l	a0,cpp_playedp(a1)

	cmp.w	#2,(a0)
	bne	no_clear11
	move.w	#1,(a0)			; from 2 to 1
no_clear11:

	move.l	cpp_playp(a1),a0
	cmp.w	#3,(a0)
	bne	no_ll

	tst.w	(a0)
	beq	no_ll

	move.w	#2,(a0)			; you are using this now
	move.l	a0,cpp_playp(a1)	; update start pointer

;	tst.l	cpp_vextra(a1)
;	beq	.nos
	move.l	cpp_vextra(a1),a2
	cmp.w	#-1,10(a0)
	beq	.nos
	move.w	10(a0),ve_TopLine(a2)	; only > 39 ???????????????????????????
.nos:
	move.l	cpp_view(a1),a2
	move.l	2(a0),v_LOFCprList(a2)
	move.l	6(a0),v_SHFCprList(a2)
	move.l	cpp_loadview(a1),a0
	move.l	cpp_gfxbase(a1),a6
	move.l	a2,a1
	move.l	a1,-(a7)
	jsr	_LVOLoadView(a6)
;	jsr	(a0)
	addq.l	#4,a7

;	move.l	#10000,d0
;.tt:	move.w	#$f00,$dff180
;	dbf	d0,.tt

	lea	cpp_data(pc),a1
	move.l	cpp_playp(a1),a0
	lea.l	12(a0),a0
	cmp.w	#-1,(a0)
	bne	no_clear2a
	lea	cpp_lists(a1),a0	; set to new start	
no_clear2a:
	move.l	a0,cpp_playp(a1)

	bra	wait_further
no_ll:
	bra	wait_further

	moveq	#0,d0
	rts

proc50hcplname:	dc.b	"50Hz cpl",0
taskcprname:	dc.b	"CPR Task",0 
	even
		
intstructcpl:
	dc.l	0,0
	dc.b	2,64		; type en pri
	dc.l	0,0		; pointer naar naam en data
	dc.l	0

*
* Scroll up with two different viewports
* and a user copper list
*
	
;cop_reveal_down:

do_cop_reveal_down:
	bra	cop_up2

cop_cover_up:
cop_up2_v2:			; variation should be 4 here
	move.l	#4,db_variation(a3)

cop_up2:
	bsr	check_hard
	bsr	init_cop_play_list
	
	move.b	#0,db_reverse_cop(a3)
	bsr	init_cop_up

	bsr	find_and_set_loadview

	move.w	d0,db_bottom_offset(a3)
	move.w	d0,d7

	move.w	#TRUE,db_cop_active_test(a3)	; set active viewport on
	move.w	#TRUE,db_cop_inactive_test(a3)	; set inactive viewport on
	bsr	rep_cop_up2

	bra	end_cop_up1

rep_cop_up2:

	bsr	easy_check		; set increment

	move.l	atviewp(pc),a1

	move.l	db_inactive_inc(a3),d0
	sub.w	d0,d7

	move.l	db_active_inc(a3),d0
	sub.w	d0,vp_DHeight(a1)

	cmp.w	#0,vp_DHeight(a1)
	bgt	no_active_scroll1

	move.w	#0,vp_DHeight(a1)

	sub.w	d0,vp_DyOffset(a1)

	move.w	db_active_min(a3),d2
	add.w	db_gapsize(a3),d2
	cmp.w	vp_DyOffset(a1),d2
	ble	no_active_scroll1
	move.w	db_active_min(a3),vp_DyOffset(a1)
	move.w	#FALSE,db_cop_active_test(a3)	; set active viewport off
no_active_scroll1:

	move.l	itviewp(pc),a1
	move.l	db_inactive_inc(a3),d0

	sub.w	d0,vp_DyOffset(a1)
;	move.w	vp_DyOffset(a1),d0

no_inactive_up2:

	move.l	db_coplist(a3),a0
	move.l	cpp_storep(a0),a0
	tst.w	(a0)
	beq	oke_create_cop2

	move.l	db_coplist(a3),a0

	move.l	cpp_playedp(a0),a1
	move.l	cpp_playp(a0),a1

	tst.w	cpp_init(a0)
	bne	al_init_cpl3
*
* the interrupt is not yet initialized 
* do it now
* and start the coplist task

	bsr	init_cpl_int

al_init_cpl3:
	bsr	wacht_tijd2
	tst.b	d0
	bne	exit_cop_up2
	bra	no_inactive_up2		; wait for pointer to clear
oke_create_cop2:

	bsr	create_cop_view

	move.l	db_tempoff(a3),a0
	cmp.w	#-1000,(a0)
	beq	exit_cop_up22
	bra	rep_cop_up2
exit_cop_up2:
	rts

easy_check:
	move.l	db_tempoff(a3),a0
	moveq	#0,d0
	move.w	(a0)+,d0
	cmp.w	#-1000,d0
	bne	no_prox
	move.w	#-1000,-2(a0)
	move.w	#1,d0
	bra	prox
no_prox:
	move.l	a0,db_tempoff(a3)
prox:
	move.l	d0,db_active_inc(a3)
	tst.b	db_active_lace(a3)
	beq	no_addl1
	add.l	d0,db_active_inc(a3)
no_addl1:
	move.l	d0,db_inactive_inc(a3)
	tst.b	db_inactive_lace(a3)
	beq	no_addl2
	add.l	d0,db_inactive_inc(a3)
no_addl2:
	rts

init_drip1:
	move.l	a5,-(a7)
	move.l	a4,a5
	move.l	vb_vieww(a5),a0
	bsr	get_modulos
	move.l	(a7)+,a5

	move.l	vb_vieww(a5),a0
	bsr	get_modulos

	move.w	#0,db_tmode(a3)
	move.l	#0,db_varwtime(a3)
	lea	teller(pc),a0
	move.l	db_varwtime(a3),(a0)
	lea	tellerstart(pc),a0
	move.l	db_varwtime(a3),(a0)
	clr.w	db_cop_inactive_test(a3)	; There is no top viewport 
	move.w	#TRUE,db_cop_active_test(a3)	; active is on

	bsr	check_cop_modes

	bsr	copy_view2
	bsr	switch_temp_views
	bsr	copy_view3

	rts
	
lofm1:	dc.l	0
shfm1:	dc.l	0

	IF NEWFLIP

do_modus:
	move.l	lofm1(pc),a0
	move.l	shfm1(pc),a1

	move.l	(a2),d7
	move.w	vb_leny(a5),d2
	sub.w	d7,d2
	lsr.w	#1,d2

	tst.b	vb_interlace(a5)
	beq	.noin
	lsr.w	#1,d2
.noin:
	tst.w	d2
	ble	.on
	move.w	d2,d3
	move.w	#$0100,d2
	bsr	set_bmpoff_a0
	bsr	set_bmpoff_a1
	move.w	d3,d2
	subq.w	#1,d2
	ble	.on
;	move.w	vb_moda(a5),d1
	moveq	#0,d1
.trep:
	bsr	set_modulo_a0
	bne	.exit
	bsr	set_modulo_a1
	bne	.exit
	subq.w	#1,d2
	bgt	.trep
.on:
	move.w	#$8100,d2
	bsr	set_bmpoff_a0
	bsr	set_bmpoff_a1

	moveq	#0,d2
	move.w	vb_leny(a5),d2
	lsl.l	#8,d2
	sub.w	#1,d7
	ble	.sw_off

	bsr	divide_part
	bne	.exit

; switch off bitplane dma here

.sw_off:
	move.w	#$0100,d2
	bsr	set_bmpoff_a0
	bsr	set_bmpoff_a1
.exit:
	rts
*
* d2 the nr off lines
* d7 crop to how many
*
divide_part:
	divu	d7,d2
	and.l	#$ffff,d2
	moveq	#0,d3
	moveq	#0,d6
	move.l	vb_totalwidth(a5),d1
	move.l	#$7fff,d5
	divu	d1,d5				; d5 the maximum delta posible
	and.l	#$ffff,d5
	lsr.l	#1,d5	
.rep_dm:
	add.l	d2,d3

	move.l	d3,d4
	lsr.l	#8,d4

	sub.l	d6,d4
	cmp.w	d4,d5
	bge	.nobig
	move.l	d5,d4
.nobig:
	add.l	d4,d6

	tst.b	vb_interlace(a5)
	beq	.noin2
	add.l	d4,d4
	subq.l	#1,d4
.noin2:
	subq.l	#1,d4

	move.l	vb_totalwidth(a5),d1
	mulu	d4,d1
	beq	.noi
	sub.l	vb_totalwidth(a5),d1
.noi:
	bsr	set_modulo_a0
	bne	.exit
	bsr	set_modulo_a1
	bne	.exit
	tst.b	vb_interlace(a5)
	beq	.noin3
	subq.l	#1,d7
.noin3:
	subq.l	#1,d7
	bge	.rep_dm
	moveq	#0,d0
	rts
.exit:
	moveq	#-1,d0
	rts

set_bmpoff_a0:
.repfill:
	cmp.w	#$0108,(a0)
	bne	.ex1
.setmod1:
	cmp.w	#$84,8(a0)
	beq	.exit1
	move.w	#$0096,(a0)
	move.w	d2,2(a0)
	add.w	#12,a0
	moveq	#0,d0
	rts
.ex1:
	cmp.w	#$0096,(a0)
	beq	.setmod1
	cmp.l	#$fffffffe,(a0)
	beq	.exit1
	addq.l	#4,a0
	cmp.w	#$0108,(a0)
	beq	.repfill
	bra	.ex1
.exit1:
	moveq	#-1,d0
	rts

set_bmpoff_a1:
	cmp.l	#0,a1
	beq	.zero
.repfill:
	cmp.w	#$0108,(a1)
	bne	.ex1
.setmod1:
	cmp.w	#$84,8(a1)
	beq	.exit2
	move.w	#$0096,(a1)
	move.w	d2,2(a1)
	move.w	vb_moda(a5),6(a1)
	add.w	#12,a1
.zero:
	moveq	#0,d0
	rts
.ex1:
	cmp.w	#$0096,(a1)
	beq	.setmod1
	cmp.l	#$fffffffe,(a1)
	beq	.exit1
	addq.l	#4,a1
	cmp.w	#$0108,(a1)
	beq	.repfill
	bra	.ex1
.exit1:
	moveq	#-1,d0
	rts
.exit2:
	moveq	#-1,d0
	rts

set_modulo_a0:
.repfill:
	cmp.w	#$0108,(a0)
	bne	.ex1
	move.w	d1,d0
	add.w	vb_moda_even(a5),d0
	move.w	d0,2(a0)
	addq.l	#4,a0
	cmp.w	#$010a,(a0)
	bne	.ex1
	move.w	d1,d0
	add.w	vb_moda_odd(a5),d0
	move.w	d0,2(a0)
	addq.l	#8,a0
	moveq	#0,d0
	rts
.setmod1:
	move.w	#$0108,(a0)
	bra	.repfill
.ex1:
	cmp.w	#$0096,(a0)
	beq	.setmod1
	cmp.l	#$fffffffe,(a0)
	beq	.exit1
	add.l	#4,a0
	cmp.w	#$0108,(a0)
	beq	.repfill
	bra	.ex1
.exit1:
	moveq	#-1,d0
	rts

set_modulo_a1:
	cmp.l	#0,a1
	beq	.zero
.repfill:
	cmp.w	#$0108,(a1)
	bne	.ex1
	move.w	d1,d0
	add.w	vb_moda_even(a5),d0
	move.w	d0,2(a1)
;	move.w	vb_moda_even(a5),2(a1)
	addq.l	#4,a1
	cmp.w	#$010a,(a1)
	bne	.ex1
	move.w	d1,d0
	add.w	vb_moda_odd(a5),d0
	move.w	d0,2(a1)
	addq.l	#8,a1
.zero:
	moveq	#0,d0
	rts
.setmod1:
	move.w	#$0108,(a1)
	bra	.repfill
.ex1:
	cmp.w	#$0096,(a1)
	beq	.setmod1
	cmp.l	#$fffffffe,(a1)
	beq	.exit1
	add.l	#4,a1
	cmp.w	#$0108,(a1)
	beq	.repfill
	bra	.ex1
.exit1:
	moveq	#-1,d0
	rts

*
* Give sinus of number in d0 with amplitude in d1
*
get_easy_sinus:
	lea	sinus(pc),a0
	cmp.w	#180,d0
	bge	.minus_sinus
	add.w	d0,d0
	move.w	0(a0,d0.w),d0
	mulu	d1,d0
	swap	d0
	and.l	#$ffff,d0
	rts
	
.minus_sinus:
	sub.w	#180,d0
	add.w	d0,d0
	move.w	0(a0,d0.w),d0
	mulu	d1,d0
	swap	d0
	and.l	#$ffff,d0
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
	DC.W	$0000,$FFF6,$FFD8,$FFA6,$FF60,$FF06,$FE98,$FE17,$FD82,$FCD9
	DC.W	$FC1C,$FB4B,$FA67,$F970,$F865,$F746,$F615,$F4D0,$F378,$F20D
	DC.W	$F08F,$EEFF,$ED5B,$EBA6,$E9DE,$E803,$E617,$E418,$E208,$DFE7
	DC.W	$DDB3,$DB6F,$D919,$D6B3,$D43B,$D1B3,$CF1B,$CC73,$C9BB,$C6F3
	DC.W	$C41B,$C134,$BE3E,$BB3A,$B826,$B504,$B1D5,$AE97,$AB4C,$A7F3
	DC.W	$A48D,$A11B,$9D9C,$9A10,$9679,$92D5,$8F27,$8B6D,$87A8,$83D9
	DC.W	$8000,$7C1C,$782F,$7438,$7039,$6C30,$6820,$6407,$5FE6,$5BBE
	DC.W	$578E,$5358,$4F1B,$4AD9,$4690,$4242,$3DEE,$3996,$3539,$30D9
	DC.W	$2C74,$280C,$23A1,$1F33,$1AC2,$1650,$11DB,$0D66,$08EF,$0478
	
try_sinus2:
	move.l	db_effect_speed(a3),d0
	moveq	#0,d1
	move.w	vb_leny(a5),d1
.repp:
	move.l	d1,(a2)+
	sub.l	d0,d1
	bge	.repp
	move.l	#-1,(a2)
	rts
	
try_sinus:
	move.l	db_effect_speed(a3),d0
	add.l	d0,d0
	add.l	d0,d0
	moveq	#0,d1
	move.w	vb_leny(a5),d1
	lsr.w	#1,d1

	move.l	#180,d4
	divu	d0,d4
	moveq	#0,d0
	tst.w	d4
	bne	.ok
	moveq	#1,d4
.ok:
	move.w	#90,d0
.rep:
	add.w	d4,d0
	cmp.w	#270,d0
	bge	.exit
	move.l	d0,d6
	bsr	get_easy_sinus
	move.l	d1,d3
	sub.l	d0,d3	
	move.l	d6,d0
	cmp.w	vb_leny(a5),d3
	bge	.exit
	cmp.l	-4(a2),d3
	beq	.rep
	move.l	d3,(a2)+
	bra	.rep
.exit:
	moveq	#0,d3
	move.w	vb_leny(a5),d3
	move.l	d3,(a2)+
	subq.l	#1,d3
	move.l	#-1,(a2)
	rts

find_first_mod:
	move.l	itview(pc),a1
	move.l	v_LOFCprList(a1),a0
	move.l	crl_start(a0),a0
.search:
	cmp.l	#$01080001,(a0)
	beq	.found1
	addq.l	#4,a0
	bra	.search
.found1:
	move.l	a0,lofm1

	clr.l	shfm1
	move.l	itview(pc),a1
	move.l	v_SHFCprList(a1),d0
	beq	.noshf
	move.l	d0,a0
	move.l	crl_start(a0),a0
.search2:
	cmp.l	#$01080001,(a0)
	beq	.found2
	addq.l	#4,a0
	bra	.search2
.found2:
	move.l	a0,shfm1

.noshf:
	rts
*
* Drip1
*
cop_coin1:
	bsr	set_speed_reverse
	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_viewblok(a3),a4

	bsr	init_flip_cop1

	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_viewblok(a3),a4

	bsr	init_drip1

	bsr	init_widths

	move.l	#0,teller
	move.l	#0,tellerstart

; atviewp now copy of the active view

	bsr	free_view25u

	bsr	init_user_coplist_mod_list

	bsr	switch_temp_views

	move.l	itviewp(pc),a1
	move.l	db_ucop_pointer(a3),vp_UCopIns(a1)

.rep_cop_drip1:

	move.l	itview(pc),a0			; first the active view
	move.l	itviewp(pc),a1
	move.w	vp_Modes(a1),d0
	or.w	db_or_vmode_mask(a3),d0
	and.w	db_and_vmode_mask(a3),d0
	move.w	d0,v_Modes(a0)
	or.w	#EXTEND_VSTRUCT,v_Modes(a0)
	move.l	db_graphbase(a3),a6
	jsr	_LVOMakeVPort(a6)

	move.l	itview(pc),a1
	jsr	_LVOMrgCop(a6)

	bsr	find_first_mod

;	move.w	#-88,vb_moda_even(a5)
;	move.w	#-88,vb_moda_odd(a5)

	move.l	db_wpatroonruimte(a3),a2
	move.l	#-1,(a2)+
	bsr	try_sinus
;	move.l	db_wpatroonruimte(a3),a2
.rep_test1:
	subq.l	#4,a2
	cmp.l	#-1,(a2)
	beq	.exit_cop_drip1

	bsr	do_modus

	move.l	db_graphbase(a3),a6
	move.l	itview(pc),a1
	move.l	v_LOFCprList(a1),a0
	move.l	crl_start(a0),d0
	move.l	v_SHFCprList(a1),a0
	move.l	crl_start(a0),d1
	jsr	_LVOLoadView(a6)

	bsr	wacht_tijd2
	tst.b	d0
	bne	.exit_cop_drip
	bra	.rep_test1
.exit_cop_drip1:

	bsr	switch_temp_views
	bsr	free_view25u

	move.l	db_inactive_viewblok(a3),a5

	bsr	init_user_coplist_mod_list

	move.l	itviewp(pc),a1
	move.l	db_ucop_pointer(a3),vp_UCopIns(a1)

	move.l	itview(pc),a0			; first the active view
	move.l	itviewp(pc),a1
	move.w	vp_Modes(a1),d0
	or.w	db_or_vmode_mask(a3),d0
	and.w	db_and_vmode_mask(a3),d0
	move.w	d0,v_Modes(a0)
	or.w	#EXTEND_VSTRUCT,v_Modes(a0)
	move.l	db_graphbase(a3),a6
	jsr	_LVOMakeVPort(a6)

	move.l	itview(pc),a1
	jsr	_LVOMrgCop(a6)

	bsr	find_first_mod

	move.l	db_inactive_viewblok(a3),a5
	move.l	db_wpatroonruimte(a3),a2
	move.l	#-1,(a2)+
	bsr	try_sinus
	move.l	db_wpatroonruimte(a3),a2
.rep_test2:
	addq.l	#4,a2
	cmp.l	#-1,(a2)
	beq	.exit_cop_drip2

	bsr	do_modus

	move.l	db_graphbase(a3),a6
	move.l	itview(pc),a1
	jsr	_LVOLoadView(a6)

	bsr	wacht_tijd2
	tst.b	d0
	bne	.exit_cop_drip
	bra	.rep_test2
.exit_cop_drip2:
.exit_cop_drip:
	move.w	#$8100,$dff096
	bsr	showpicture

	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)
	
	bsr	free_view_ucop
	bsr	switch_temp_views
	bsr	free_view_ucop
	move.w	#$8100,$dff096
	rts


init_user_coplist_mod_list:
	move.l	a4,-(a7)
	moveq	#ucl_SIZEOF,d0
	move.l	#MEMF_CLEAR,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)

	move.l	d0,db_ucop_pointer(a3)
	beq	no_ucop_mem
	
	move.l	db_ucop_pointer(a3),a0
	move.l	#3000,d0
	move.l	db_graphbase(a3),a6
	jsr	_LVOUCopperListInit(a6)

	move.l	db_ucop_pointer(a3),a2
	moveq	#0,d7

.make:
	move.l	d7,d0
	move.l	#0,d1
	move.l	a2,a1
	jsr	_LVOCWait(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)
	addq.w	#1,d7
	tst.b	vb_interlace(a5)
	beq	.noadd
	addq.w	#1,d7
.noadd:

	move.w	#bpl1mod,d0
	move.w	#1,d1
	move.l	a2,a1
	jsr	_LVOCMove(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)

	move.w	#bpl2mod,d0
	move.w	#1,d1
	move.l	a2,a1
	jsr	_LVOCMove(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)

	cmp.w	vb_leny(a5),d7
	blt	.make
	
	move.l	#10000,d0
	move.l	#255,d1
	move.l	a2,a1
	jsr	_LVOCWait(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)
	move.l	(a7)+,a4
	rts

	ELSE

*
* In atview is the active view !!!!
*
cop_coin1:
	bsr	set_speed_reverse
	
	move.w	#$0,db_tell(a3)

	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_viewblok(a3),a4

	bsr	init_flip_cop1
	bsr	init_drip1

	bsr	init_widths

	move.l	atviewp(pc),a1
	move.w	vp_DyOffset(a1),db_final_end(a3)

	move.l	itviewp(pc),a1
	move.w	vp_DyOffset(a1),db_topoffset(a3)

	move.l	db_wstore_moves2(a3),a0
	move.l	#0,(a0)+
	move.l	#0,(a0)+
	move.l	a0,db_copstore(a3)

; atviewp now copy of the active view

	move.l	itviewp(pc),a1

	move.w	vp_DHeight(a1),db_final_height(a3)

	move.l	db_effect_speed(a3),d7
	add.l	d7,d7
	add.l	d7,d7

	move.w	d7,d6
	move.l	d7,db_final_height(a3)

	moveq	#1,d7
frep_cop_coin1:
	move.l	d7,-(a7)
	bsr	store_angles
	move.w	vb_leny(a4),d4
	lsr.l	#1,d4
	addq.w	#4,d3			; when shocky at the end lower this
	cmp.w	d4,d3
	bgt	no_coin_done
no_coin_done:

	bsr	free_view25u
	bsr	init_user_coplist_mod_test3_in

	move.l	itviewp(pc),a1
	move.l	db_ucop_pointer(a3),vp_UCopIns(a1)

	move.l	itview(pc),a0
	move.l	itviewp(pc),a1
	move.l	db_wpatroonruimte(a3),a2
	move.l	(a2),d0
	move.w	db_topoffset(a3),vp_DyOffset(a1)
	move.w	vb_leny(a4),d1
	lsr.w	#1,d1
	subq.w	#2,d1
	sub.w	d0,d1
	bpl	coke_ttt
	moveq	#0,d1
coke_ttt:
	add.w	d1,vp_DyOffset(a1)

	move.w	vp_DyOffset(a1),d1
	and.w	#1,d1
	bne	cno_add1
	subq.w	#1,vp_DyOffset(a1)
cno_add1:
	add.l	d0,d0
	move.w	d0,vp_DHeight(a1)
	move.w	vp_Modes(a1),v_Modes(a0)
	or.w	#V_EXTENDED_MODE,v_Modes(a0)
	
	move.w	db_or_vmode_mask(a3),d0
	or.w	d0,v_Modes(a0)
	jsr	_LVOMakeVPort(a6)
	move.l	itview(pc),a1
	jsr	_LVOMrgCop(a6)

	move.l	itview(pc),a1
	move.l	db_copstore(a3),a0
	move.l	v_LOFCprList(a1),(a0)+		; store to use later
	move.l	v_SHFCprList(a1),(a0)+
	clr.l	v_LOFCprList(a1)		; free later too
	clr.l	v_SHFCprList(a1)
	move.l	#0,(a0)
	move.l	a0,db_copstore(a3)

**** now for the active view

	bsr	switch_temp_views
	bsr	free_view25u
	move.w	vb_moda(a5),d0
	cmp.w	vb_moda(a4),d0
	bne	no_sec_use_cop

	bra	no_need_cop
no_sec_use_cop:
	bsr	init_user_coplist_mod_test3

no_need_cop:
	move.l	itview(pc),a0
	move.l	itviewp(pc),a1
	move.l	db_ucop_pointer(a3),vp_UCopIns(a1)
	move.l	db_wpatroonruimte(a3),a2
	move.l	(a2),d0
	move.w	db_topoffset(a3),vp_DyOffset(a1)
	move.w	vb_leny(a5),d1
	lsr.w	#1,d1
	subq.w	#2,d1
	sub.w	d0,d1
	bpl	coke_ttt2
	moveq	#0,d1
coke_ttt2:
	add.w	d1,vp_DyOffset(a1)
	move.w	vp_DyOffset(a1),d1
	and.w	#1,d1
	bne	cno_add12
	subq.w	#1,vp_DyOffset(a1)
cno_add12:
	add.l	d0,d0
	move.w	d0,vp_DHeight(a1)
	move.w	vp_Modes(a1),v_Modes(a0)
	move.w	db_or_vmode_mask(a3),d0
	or.w	d0,v_Modes(a0)
	or.w	#V_EXTENDED_MODE,v_Modes(a0)
	jsr	_LVOMakeVPort(a6)
	move.l	itview(pc),a1
	jsr	_LVOMrgCop(a6)
	move.l	itview(pc),a1
	move.l	db_copstore(a3),a0
	move.l	v_LOFCprList(a1),d0
	move.l	d0,(a0)+		; store to use later
	move.l	v_SHFCprList(a1),(a0)+
	clr.l	v_LOFCprList(a1)		; free later too
	clr.l	v_SHFCprList(a1)
	move.l	#0,(a0)
	move.l	a0,db_copstore(a3)

	move.w	vb_moda(a5),d0
	cmp.w	vb_moda(a4),d0
	bne	no_sec_use_cop2

	move.l	itviewp(pc),a1

	move.l	#0,vp_UCopIns(a1)	; don't free this twice

no_sec_use_cop2:
	bsr	switch_temp_views

*********

no_store_coin1:
	movem.l	(a7)+,d7
	addq.l	#1,d7
	cmp.l	db_final_height(a3),d7
	bge	fexit_cop_coin1

	bra	frep_cop_coin1

done_coin1:
	move.l	(a7)+,d7
	
fexit_cop_coin1:
	bsr	show_cops2

	bsr	showpicture

	bsr	clear_cop_lists2
	
	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)

	bsr	free_view_ucop
	bsr	switch_temp_views
	bsr	free_view_ucop
	rts
	ENDC

show_cops2:
	move.l	db_copstore(a3),a5
rep_cop_place3:
	move.l	atview(pc),a1
	move.l	-4(a5),d1
	beq	no_more_place2
	move.l	-8(a5),d0
	lea.l	-16(a5),a5
	move.l	d0,v_LOFCprList(a1)
	move.l	d1,v_SHFCprList(a1)
	move.l	db_graphbase(a3),a6
	jsr	_LVOLoadView(a6)
	bsr	wacht_tijd2
	tst.b	d0
	beq	rep_cop_place3

no_more_place2:
	move.l	atview(pc),a1
	move.l	#0,v_LOFCprList(a1)
	move.l	#0,v_SHFCprList(a1)
	move.l	db_wstore_moves2(a3),a5
	addq.l	#8,a5
rep_cop_place2:
	move.l	itview(pc),a1
	move.l	(a5)+,d0
	beq	no_more_place
	move.l	(a5)+,d1
	addq.l	#8,a5
	move.l	d0,v_LOFCprList(a1)
	move.l	d1,v_SHFCprList(a1)
	move.l	db_graphbase(a3),a6
	jsr	_LVOLoadView(a6)
	bsr	wacht_tijd2
	tst.b	d0
	bne	no_more_place
	bra	rep_cop_place2
*
* Convert speed from 0-20 to 25-5
*
set_speed_reverse:
	move.l	db_effect_speed(a3),d0
	moveq	#25,d1
	sub.l	d0,d1
	bpl	oke_spe
	moveq	#5,d1
oke_spe:
	move.l	d1,db_effect_speed(a3)
	rts

init_widths:
	move.l	vb_breedte_x(a5),d1
	mulu	vb_planes(a5),d1
	move.l	d1,vb_totalwidth(a5)
	move.l	vb_breedte_x(a4),d1
	mulu	vb_planes(a4),d1
	move.l	d1,vb_totalwidth(a4)
	rts

cop_flip2_rev:
	move.w	#$ff,db_tell(a3)
	move.b	#$ff,db_reverse_cop(a3)
	move.l	db_active_viewblok(a3),a4
	move.l	db_inactive_viewblok(a3),a5
	bra	cop_flip12

cop_flip1_rev:
	move.w	#$0,db_tell(a3)
	move.b	#$ff,db_reverse_cop(a3)
	move.l	db_active_viewblok(a3),a4
	move.l	db_inactive_viewblok(a3),a5
	bra	cop_flip12
*
* Slide the active pic down with it
*
cop_flip2:
	cmp.l	#2,db_varltimes(a3)
	beq	cop_flip2_rev

	move.w	#$ff,db_tell(a3)

	bra	cop_flip11
*
* Flip over the active pic
*
cop_flip1:
	cmp.l	#2,db_varltimes(a3)
	beq	cop_flip1_rev

	move.w	#$0,db_tell(a3)
cop_flip11:

	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_viewblok(a3),a4

cop_flip12:

	bsr	set_speed_reverse
	bsr	init_flip_cop1

	bsr	init_drip1

	bsr	init_widths

	
	move.l	atviewp(pc),a1
	move.w	vp_DyOffset(a1),db_topoffset(a3)

	move.l	db_wstore_moves2(a3),a0
	move.l	#0,(a0)+
	move.l	#0,(a0)+
	move.l	a0,db_copstore(a3)	

; atviewp now copy of the active view

	move.l	itviewp(pc),a1

	move.w	vp_DHeight(a1),db_final_height(a3)
	move.l	db_effect_speed(a3),d7
	add.l	d7,d7
	add.l	d7,d7

	move.w	d7,d6
	move.l	d7,db_final_height(a3)

	moveq	#1,d7
frep_cop_flip1:
	move.l	d7,-(a7)
	bsr	store_angles2
	move.w	vb_leny(a5),d4
	lsr.l	#1,d4
	addq.w	#6,d3			; when shocky at the end lower this
	cmp.w	d4,d3
	ble	no_pro_done

	bra	done_flip1
no_pro_done:

	bsr	free_view25u
	bsr	init_user_coplist_mod_test3_in

	move.l	itviewp(pc),a1
	move.l	db_ucop_pointer(a3),vp_UCopIns(a1)

	move.l	itview(pc),a0			; first the active view
	move.l	itviewp(pc),a1

	move.l	atviewp(pc),d0
	move.l	d0,vp_Next(a1)

	move.l	db_wpatroonruimte(a3),a2
	move.l	(a2),d0
	add.l	d0,d0
	move.w	d0,vp_DHeight(a1)
	move.w	vp_Modes(a1),d0
	or.w	db_or_vmode_mask(a3),d0
	move.w	d0,v_Modes(a0)
	or.w	#EXTEND_VSTRUCT,v_Modes(a0)
	jsr	_LVOMakeVPort(a6)

	move.l	itview(pc),a0			; first the active view
	move.l	atviewp(pc),a1
	
	move.l	db_wpatroonruimte(a3),a2
	move.l	(a2),d0
	add.l	d0,d0
	add.w	db_gapsize(a3),d0
	add.w	db_gapsize(a3),d0
	move.w	db_topoffset(a3),vp_DyOffset(a1)
	add.w	d0,vp_DyOffset(a1)

	tst.w	db_tell(a3)
	bne	no_reveal

	move.l	vp_RasInfo(a1),a2		; reveal flip
	addq.w	#1,d0
	move.w	d0,ri_RyOffset(a2)		; end reveal flip
no_reveal:

	move.w	vp_Modes(a1),d0
	or.w	db_or_vmode_mask(a3),d0
	move.w	d0,v_Modes(a0)
	or.w	#EXTEND_VSTRUCT,v_Modes(a0)
	jsr	_LVOMakeVPort(a6)

	move.l	itview(pc),a1
	jsr	_LVOMrgCop(a6)

	move.l	itviewp(pc),a1
	move.l	#0,vp_Next(a1)

	move.l	itview(pc),a1
	move.l	db_copstore(a3),a0
	move.l	v_LOFCprList(a1),(a0)+		; store to use later
	move.l	v_SHFCprList(a1),(a0)+
	clr.l	v_LOFCprList(a1)		; free later too
	clr.l	v_SHFCprList(a1)
	move.l	#0,(a0)
	move.l	a0,db_copstore(a3)

no_store_flip1:
	movem.l	(a7)+,d7
	addq.l	#1,d7
	cmp.l	db_final_height(a3),d7
	bge	fexit_cop_flip1

	bra	frep_cop_flip1

done_flip1:
	move.l	(a7)+,d7

fexit_cop_flip1:

	bsr	show_cops

	bsr	showpicture

	bsr	clear_cop_lists
	
	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)

	bsr	free_view_ucop
	bsr	switch_temp_views
	bsr	free_view_ucop
	rts

	IFNE NEWEFF
	RSRESET
bld_struct:	rs.w	0
bld_start:	rs.w	1
bld_stop:	rs.w	1
bld_SIZEOF	rs.w	0

*
* Drip1
*
cop_drip1:

	move.l	db_inactive_viewblok(a3),a5
	move.l	db_inactive_viewblok(a3),a4

	bsr	init_drip1

	move.l	#1,teller
	move.l	#1,tellerstart

	move.l	db_effect_speed(a3),db_max_blocksize(a3)
	
	move.l	itviewp(pc),a1
	move.l	db_wpatroonruimte(a3),a0
	move.w	vp_DHeight(a1),d7
	move.w	d7,d6
	move.w	d7,d5
	move.l	db_max_blocksize(a3),d1
	moveq	#0,d0
.init_list:
	move.w	d0,bld_stop(a0)
	move.w	d6,bld_start(a0)
	add.l	#bld_SIZEOF,a0	
	add.l	d1,d0
	add.l	d5,d6
	lsr.l	#1,d5
	bgt	.noq
	moveq	#1,d5
.noq:
	cmp.l	d7,d0
	blt	.init_list
	move.l	a0,a4
	sub.l	#bld_SIZEOF,a4
	move.w	#-1000,bld_stop(a0)		; mark end of the list
	move.w	#-1000,bld_start(a0)
	
; atviewp now copy of the active view

	move.l	itviewp(pc),a1

	moveq	#1,d7
	move.w	vp_DHeight(a1),d7
	move.w	vp_DHeight(a1),db_final_height(a3)

.rep_cop_drip1:
	
	bsr	free_view25u
	bsr	init_user_coplist_mod_test_dma_off

	move.l	itviewp(pc),a1
	move.l	db_ucop_pointer(a3),vp_UCopIns(a1)

	move.l	itview(pc),a0			; first the active view
	move.l	itviewp(pc),a1
	move.w	vp_Modes(a1),d0
	or.w	db_or_vmode_mask(a3),d0
	and.w	db_and_vmode_mask(a3),d0
	move.w	d0,v_Modes(a0)
	or.w	#EXTEND_VSTRUCT,v_Modes(a0)	; MONI ?????????????????
	jsr	_LVOMakeVPort(a6)

	move.l	itview(pc),a1
	jsr	_LVOMrgCop(a6)

	move.l	db_graphbase(a3),a6
	move.l	itview(pc),a1
	jsr	_LVOLoadView(a6)

	bsr	wacht_tijd2
	tst.b	d0
	bne	exit_cop_drip1

	bsr	switch_temp_views
	cmp.w	#-1,bld_start(a4)
	bne	.rep_cop_drip1

;	sub.l	db_effect_speed(a3),d7
;	tst.l	d7
;	bmi	exit_cop_drip1
;	cmp.w	db_final_height(a3),d7
;	bge	exit_cop_drip1
;	bra	rep_cop_drip1

exit_cop_drip1:

	bsr	showpicture

	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)
	bsr	free_view_ucop
	bsr	switch_temp_views
	bsr	free_view_ucop
	rts
*
* Drip2
*
cop_drip2:
	bsr	init_drip1
	
; atviewp now copy of the active view

	move.l	itviewp(pc),a1
	move.l	vp_RasInfo(a1),a2

	move.w	vp_DHeight(a1),ri_RyOffset(a2)
	move.l	db_inactive_inc(a3),d6
	sub.w	d6,ri_RyOffset(a2)

	moveq	#0,d7
	move.w	vp_DHeight(a1),d7
rep_cop_drip2:

	move.l	d7,-(a7)
	bsr	free_view25u
	move.l	(a7)+,d7

	bsr	init_user_coplist_mod_test2

	move.l	itviewp(pc),a1
	move.l	db_ucop_pointer(a3),vp_UCopIns(a1)

	move.l	itview(pc),a0			; first the active view
	move.l	itviewp(pc),a1
	move.w	vp_Modes(a1),d0
	or.w	db_or_vmode_mask(a3),d0
	move.w	d0,v_Modes(a0)
	jsr	_LVOMakeVPort(a6)

	move.l	itview(pc),a1
	jsr	_LVOMrgCop(a6)

	move.l	itview(pc),a1
	jsr	_LVOLoadView(a6)

	bsr	switch_temp_views
	
	bsr	wacht_tijd2
	tst.b	d0
	bne	exit_cop_drip2

	move.l	db_inactive_inc(a3),d6
	sub.w	d6,ri_RyOffset(a2)
	bpl	drip2_oke
	move.w	#0,ri_RyOffset(a2)
drip2_oke:

	sub.l	d6,d7
	bmi	exit_cop_drip2
	bne	rep_cop_drip2
exit_cop_drip2:

	bsr	showpicture

	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)
	bsr	free_view_ucop
	bsr	switch_temp_views
	bsr	free_view_ucop
	rts
	ENDC
*
* copieer de inactive view naar het itview pointer
*
copy_view3:
	movem.l		a5,-(a7)
	move.l		a4,a5		; use the inactive for this
	bra		copy_vit
*
* copieer de inactive view naar het itview pointer
*
copy_view2:
	movem.l		a5,-(a7)
copy_vit:
	move.l		db_graphbase(a3),a6

	bsr		free_view		; free the itview

	bsr		init_vportsn

	move.l		vb_vieww(a5),a0
	move.l		itview(pc),a1

	move.w		v_Modes(a0),v_Modes(a1)
	move.w		v_Modes(a0),d0

	move.w		db_viewx(a3),v_DxOffset(a1)
	move.w		db_viewy(a3),v_DyOffset(a1)
	
	moveq		#VIEW_EXTRA_TYPE,d0
	move.l		db_graphbase(a3),a6
	jsr		_LVOGfxNew(a6)
	move.l		d0,it_vextra
	beq		.no_ass

	IFNE GFXSNOOP
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.temp(pc),a1
	move.l	it_vextra(pc),(a1)
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"cc1 %d,%d",10,0
	even
.temp:	dc.l	0
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l		d0,a0
	move.l		db_monitorspec(a3),ve_Monitor(a0)

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	ve_Monitor(a0),a1
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"mon is  %d,%d",10,0
	even
.temp:	dc.l	0
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	moveq		#VIEWPORT_EXTRA_TYPE,d0
	jsr		_LVOGfxNew(a6)
	move.l		d0,it_vpextra
	beq		.no_ass

	IFNE GFXSNOOP
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.temp2(pc),a1
	move.l	it_vpextra(pc),(a1)
	lea	.dbstr2(pc),a0
	jsr	KPutFmt
	bra	.tt2
.dbstr2:	dc.b	"cc2 %d,%d",10,0
	even
.temp2:	dc.l	0
.tt2:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.w		#0,a0
	lea		vb_dimquery(a5),a1
	moveq		#dim_SIZEOF,d0
	move.l		#DTAG_DIMS,d1
	move.l		db_monid(a3),d2
	or.w		vb_mode(a5),d2
	or.w		db_or_vmode_mask(a3),d2
	jsr		_LVOGetDisplayInfoData(a6)

	lea		vb_dimquery(a5),a0
	move.l		it_vpextra(pc),a1
	move.l		dim_MaxOScan(a0),vpe_DisplayClip(a1)
	move.l		dim_MaxOScan+4(a0),vpe_DisplayClip+4(a1)

	move.l		itview(pc),a0
	move.l		it_vextra(pc),a1
	jsr		_LVOGfxAssociate(a6)

	move.l		itview(pc),a0
	or.w		#EXTEND_VSTRUCT,v_Modes(a0)
.no_ass:

	move.l		itview(pc),a0
	move.l		itviewp(pc),a1
	move.l		a1,v_ViewPort(a0)

	move.l		#0,vp_Next(a1)

	move.l		itviewp(pc),a0
	move.l		vb_rasinfow(a5),a1
	move.w		#0,ri_RyOffset(a1)		; clear settings

	move.l		a1,vp_RasInfo(a0)

	move.l		vb_viewportw(a5),a1
	move.w		vp_DWidth(a1),vp_DWidth(a0)
	move.w		vp_DHeight(a1),vp_DHeight(a0)
	move.w		vp_DxOffset(a1),vp_DxOffset(a0)
	move.w		vp_DyOffset(a1),vp_DyOffset(a0)

	move.w		vb_mode(a5),vp_Modes(a0)

	move.l		db_monid(a3),d0
	or.w		vb_mode(a5),d0
	move.w		vb_mode(a5),d3
	and.w		#~$884,d3
	bne		.nolow2
	and.w		#$7fdf,d0
	or.w		db_alternate_lowres(a3),d0
.nolow2:
	move.l		db_graphbase(a3),a6
	jsr		_LVOFindDisplayInfo(a6)
	move.l		db_tags(a3),a0
	move.l		d0,20(a0)		

	tst.b		db_aa_present(a3)
	bne		aa12
	
	moveq		#32,d0
	jsr		_LVOGetColorMap(a6)
	tst.l		d0
	beq.w		no_colormap2

	move.l		d0,a0
	move.l		db_tags(a3),a1
	move.l		it_vpextra(pc),12(a1)
	move.l		itviewp(pc),4(a1)
	move.l		db_graphbase(a3),a6
	jsr		_LVOVideoControl(a6)	; connect colormap to temp view

	move.l		itviewp(pc),a0
	lea		vb_colors(a5),a1
	moveq		#32,d0
no_pro_th2:
	jsr		_LVOLoadRGB4(a6)
no_colormap2:
	movem.l		(a7)+,a5
	rts

aa12:
	move.l		#256,d0
	bsr		convert_colors
	move.l		#256,d0
	move.l		db_graphbase(a3),a6
	jsr		_LVOGetColorMap(a6)
	tst.l		d0
	beq		no_colormap2

	move.l		d0,a0
	move.l		db_tags(a3),a1
	move.l		it_vpextra(pc),12(a1)
	move.l		itviewp(pc),4(a1)
	move.l		db_graphbase(a3),a6
	jsr		_LVOVideoControl(a6)	; connect colormap to temp view

	move.l		db_waarcolors32(a3),a1
	move.l		itviewp(pc),a0
	move.l		db_graphbase(a3),a6
	jsr		_LVOLoadRGB32(a6)

	bra		no_colormap2

init_flip_cop1:
	move.l	db_effect_speed(a3),d0
	add.l	d0,d0
	add.l	d0,d0
	moveq	#0,d1
	move.w	vb_leny(a5),d1
	lsr.w	#1,d1
	bsr	init_flip_angles
	bsr	init_flip_sub
	cmp.w	#8,vb_planes(a5)
	bne	.no_8
	move.w	#10,db_gapsize(a3)		; change gapsize here ????
.no_8:
	rts

store_angles2:
	movem.l	a4,-(a7)

	moveq	#0,d2
	move.w	db_max_y(a3),d2				; screen height
;	move.l	#FLIP_SIZE,d2
	lsr.l	#1,d2
	move.l	d2,db_shiftit(a3)

	bsr	get_flip_start_sub
	move.l	db_wpatroonruimte(a3),a2
	move.l	d0,(a2)+
	move.l	d0,d6
	move.l	d6,d5
	lsr.l	#1,d5
	neg.l	d5
	moveq	#0,d7
	lea	t1(pc),a4
	move.l	#0,(a4)
	move.l	#0,4(a4)
	moveq	#0,d3
frep_flip_inc2:
	move.l	d5,d0
	bsr	get_flip_proj
	move.l	d0,(a4)
	bsr	get_delta
	move.l	4(a4),d1
	sub.l	d1,d0
	move.l	d0,(a2)+
	cmp.l	#1,d0
	bne	no_add32
	addq.l	#1,d3
no_add32:
	move.l	(a4),4(a4)
	addq.l	#1,d5
	dbf	d6,frep_flip_inc2
	movem.l	(a7)+,a4
	rts

store_angles:	
	move.l	a4,-(a7)
	bsr	get_flip_start_sub
	move.l	db_wpatroonruimte(a3),a2
	move.l	d0,(a2)+
	move.l	d0,d6
	moveq	#1,d5
	moveq	#0,d7
	lea	t1(pc),a4
	move.l	#0,(a4)
	move.l	#0,4(a4)
	moveq	#0,d3
frep_flip_inc:
	move.l	d5,d0
	bsr	get_flip_proj
	move.l	d0,(a4)
	bsr	get_delta
	move.l	4(a4),d1
	sub.l	d1,d0
	move.l	d0,(a2)+
	cmp.l	#1,d0
	bne	no_add3
	addq.l	#1,d3
no_add3:
	move.l	(a4),4(a4)
	addq.l	#1,d5
	dbf	d6,frep_flip_inc
	move.l	(a7)+,a4
	rts

t1:	dc.l	0
t2:	dc.l	0

get_delta:
	move.l	db_mathffpbase(a3),a6
	move.l	d7,d1
	move.l	d2,d0
	jsr	_LVOSPSub(a6)
	jsr	_LVOSPFix(a6)
	rts
	
init_user_coplist_mod_test3_in:
	movem.l	a5,-(a7)
	move.l	a4,a5
	bsr	init_user_coplist_mod_test3
	movem.l	(a7)+,a5
	rts

init_user_coplist_mod_test3:
	moveq	#ucl_SIZEOF,d0
	move.l	#MEMF_CLEAR,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)

	move.l	d0,db_ucop_pointer(a3)
	beq	no_ucop_mem

	movem.l	a4/a5,-(a7)

	moveq	#0,d6
		
	move.l	db_wpatroonruimte(a3),a2
	move.l	(a2),d0
	add.l	d0,d0
	add.l	#20,d0
	move.l	db_ucop_pointer(a3),a0
	move.l	db_graphbase(a3),a6
	jsr	_LVOUCopperListInit(a6)

	move.l	db_ucop_pointer(a3),a4

	move.l	(a2)+,d7
	subq.l	#1,d7

	move.l	(a2),d5
	add.l	d5,d5
	moveq	#-1,d3
rep_make_flip:
	move.l	(a2),d4
	cmp.l	-4(a2),d4
	beq	no_c_put
	add.l	d4,d4
	subq.l	#2,d4
	cmp.l	#-100,d4
	ble	no_c_put

	move.l	vb_totalwidth(a5),d1
;	subq.l	#1,d4
;	bpl	.k
;	moveq	#0,d4
;.k:
	mulu	d4,d1
	add.w	vb_moda(a5),d1

	move.w	d1,d3
	moveq	#0,d1
	move.l	d6,d0
	move.l	a4,a1
	jsr	_LVOCWait(a6)
	move.l	a4,a1
	jsr	_LVOCBump(a6)

	move.l	#$10a,d0

	move.w	d3,d1
	move.l	a4,a1
	jsr	_LVOCMove(a6)
	move.l	a4,a1
	jsr	_LVOCBump(a6)

	move.l	#$108,d0

	move.w	d3,d1
	move.l	a4,a1
	jsr	_LVOCMove(a6)
	move.l	a4,a1
	jsr	_LVOCBump(a6)
no_c_put:
	move.l	(a2)+,d5
	add.l	d5,d5
	addq.l	#2,d6

	dbf	d7,rep_make_flip

	move.l	#10000,d0
	move.l	#255,d1
	move.l	a4,a1
	jsr	_LVOCWait(a6)

	move.l	a4,a1
	jsr	_LVOCBump(a6)

	move.w	#267,db_terminate(a3)
	movem.l	(a7)+,a4/a5
	rts

	IFNE NEWEFF
*
* Drip down 
*	
init_user_coplist_mod_test:
	moveq	#ucl_SIZEOF,d0
	move.l	#MEMF_CLEAR,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)

	move.l	d0,db_ucop_pointer(a3)
	beq	no_ucop_mem
	
	move.l	db_ucop_pointer(a3),a0
	moveq	#10,d0
	move.l	db_graphbase(a3),a6
	jsr	_LVOUCopperListInit(a6)

	move.l	db_ucop_pointer(a3),a2

	moveq	#0,d1
	move.l	d7,d0
	move.l	a2,a1
	jsr	_LVOCWait(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)

	move.l	#$10a,d0
	move.l	db_inactive_viewblok(a3),a5

	move.l	vb_breedte_x(a5),d5
	sub.w	vb_modulodisp(a5),d5
	neg.w	d5

;	move.l	vb_breedte_x(a5),d5
;	mulu	vb_planes(a5),d5
;	add.w	d5,d5
;	add.w	vb_moda(a5),d5	

	move.w	d5,d1

	move.l	a2,a1
	jsr	_LVOCMove(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)

	move.l	#$108,d0

	move.w	d5,d1
	move.l	a2,a1
	jsr	_LVOCMove(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)

	move.l	#10000,d0
	move.l	#255,d1
	move.l	a2,a1
	jsr	_LVOCWait(a6)

	move.l	a2,a1
	jsr	_LVOCBump(a6)

	move.w	#267,db_terminate(a3)
	rts

init_user_coplist_mod_test_dma_off:
	move.l	a4,-(a7)
	moveq	#ucl_SIZEOF,d0
	move.l	#MEMF_CLEAR,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)

	move.l	d0,db_ucop_pointer(a3)
	beq	no_ucop_mem
	
	move.l	db_ucop_pointer(a3),a0
	move.l	#1000,d0
	move.l	db_graphbase(a3),a6
	jsr	_LVOUCopperListInit(a6)

	move.l	db_wpatroonruimte(a3),a4	; walk through the whole list
	move.l	db_ucop_pointer(a3),a2

	move.w	#$96,d0
	move.w	#$8100,d1			; switch on all dma
	move.l	a2,a1
	jsr	_LVOCMove(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)

	move.l	db_effect_speed(a3),d5
	move.w	vb_leny(a5),d7
.rep_make:
	sub.w	d5,bld_start(a4)
	move.w	bld_start(a4),d3
	cmp.w	bld_stop(a4),d3
	bgt	.doit
	move.w	#-1,bld_start(a4)
	bra	.no_place
.doit:
	cmp.w	d3,d7
	blt	.no_place

	moveq	#0,d1
	move.w	d3,d0
	move.l	a2,a1
	jsr	_LVOCWait(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)

	move.w	#$96,d0
	move.w	#$0100,d1			; switch off all dma
	move.l	a2,a1
	jsr	_LVOCMove(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)
	add.l	db_max_blocksize(a3),d3
	move.w	bld_start(a4),d3
	cmp.w	d3,d7
	blt	.no_place
	moveq	#0,d1
	move.w	d3,d0
	move.l	a2,a1
	jsr	_LVOCWait(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)

	move.w	#$96,d0
	move.w	#$8100,d1			; switch on all dma
	move.l	a2,a1
	jsr	_LVOCMove(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)

.no_place:
	add.l	#bld_SIZEOF,a4
	cmp.w	#-1000,bld_start(a4)
	bne	.rep_make

.fi_list:
	move.l	#10000,d0
	move.l	#255,d1
	move.l	a2,a1
	jsr	_LVOCWait(a6)

	move.l	a2,a1
	jsr	_LVOCBump(a6)
	move.l	(a7)+,a4
	rts

init_user_coplist_mod_test2:
	move.l	a2,-(a7)
	moveq	#ucl_SIZEOF,d0
	move.l	#MEMF_CLEAR,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)

	move.l	d0,db_ucop_pointer(a3)
	beq	no_ucop_mem
	
	move.l	db_ucop_pointer(a3),a0
	moveq	#10,d0
	move.l	db_graphbase(a3),a6
	jsr	_LVOUCopperListInit(a6)

	move.l	db_ucop_pointer(a3),a2

	moveq	#0,d1
	move.l	d7,d0
	moveq	#0,d0
	add.l	db_inactive_inc(a3),d0
	move.l	a2,a1
	jsr	_LVOCWait(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)

	move.l	#$10a,d0

	move.l	db_inactive_viewblok(a3),a5

	move.l	vb_breedte_x(a5),d5
	sub.w	vb_modulodisp(a5),d5
	neg.w	d5

	move.w	d5,d1

	move.l	a2,a1
	jsr	_LVOCMove(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)

	move.l	#$108,d0

	move.w	d5,d1
	move.l	a2,a1
	jsr	_LVOCMove(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)

	moveq	#0,d1
	move.l	d7,d0
	move.l	a2,a1
	jsr	_LVOCWait(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)

	move.w	#$108,d0
	move.w	vb_moda(a5),d1
	move.l	a2,a1
	jsr	_LVOCMove(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)

	move.w	#$10a,d0
	move.w	vb_moda(a5),d1
	move.l	a2,a1
	jsr	_LVOCMove(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)

	move.l	#10000,d0
	move.l	#255,d1
	move.l	a2,a1
	jsr	_LVOCWait(a6)

	move.l	a2,a1
	jsr	_LVOCBump(a6)

	move.w	#267,db_terminate(a3)
	move.l	(a7)+,a2
	rts
	ENDC

switch_temp_views:
	move.l	itview(pc),d0
	lea	itview(pc),a0
	move.l	atview(pc),(a0)
	lea	atview(pc),a0
	move.l	d0,(a0)
	move.l	it_vextra(pc),d0
	lea	it_vextra(pc),a0
	move.l	at_vextra(pc),(a0)
	lea	at_vextra(pc),a0
	move.l	d0,(a0)
switch_temp_views3:
	move.l	itviewp(pc),d0
	lea	itviewp(pc),a0
	move.l	atviewp(pc),(a0)
	lea	atviewp(pc),a0
	move.l	d0,(a0)
	move.l	it_vpextra(pc),d0
	lea	it_vpextra(pc),a0
	move.l	at_vpextra(pc),(a0)
	lea	at_vpextra(pc),a0
	move.l	d0,(a0)
	rts

switch_temp_views2:
	move.l	itview(pc),d0
	lea	itview(pc),a0
	move.l	atview(pc),(a0)
	lea	atview(pc),a0
	move.l	d0,(a0)
	move.l	it_vextra(pc),d0
	lea	it_vextra(pc),a0
	move.l	at_vextra(pc),(a0)
	lea	at_vextra(pc),a0
	move.l	d0,(a0)
	rts

free_it_colormap:
	move.l	db_graphbase(a3),a6
	move.l	itviewp(pc),a0
	tst.l	vp_ColorMap(a0)
	beq.b	.no_colormem1
	move.l	vp_ColorMap(a0),a0
	jsr	_LVOFreeColorMap(a6)
	move.l	itviewp(pc),a0
	clr.l	vp_ColorMap(a0)
.no_colormem1:
	rts

free_it_cop_frames:
	move.l		db_graphbase(a3),a6
	move.l		itview(pc),a0
	tst.l		v_LOFCprList(a0)
	beq.b		.no_lofmem1
	move.l		v_LOFCprList(a0),a0
	jsr		_LVOFreeCprList(a6)
	move.l		itview(pc),a0
	clr.l		v_LOFCprList(a0)
.no_lofmem1:
	move.l		itview(pc),a0
	tst.l		v_SHFCprList(a0)
	beq.b		.no_shfmem2
	move.l		v_SHFCprList(a0),a0
	jsr		_LVOFreeCprList(a6)
	move.l		itview(pc),a0
	clr.l		v_SHFCprList(a0)
.no_shfmem2:
	rts	

free_it_gfx_new:
	move.l	db_graphbase(a3),a6
	move.l	it_vextra(pc),d0
	beq	.no_vex

	IFNE GFXSNOOP
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.temp(pc),a1
	move.l	d0,(a1)
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"fit1 %d,%d",10,0
	even
.temp:	dc.l	0
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	d0,a0
	jsr	_LVOGfxFree(a6)
	clr.l	it_vextra
.no_vex:
	move.l	it_vpextra(pc),d0
	beq	.no_vpex

	IFNE GFXSNOOP
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.temp2(pc),a1
	move.l	d0,(a1)
	lea	.dbstr2(pc),a0
	jsr	KPutFmt
	bra	.tt2
.dbstr2:	dc.b	"fit2 %d,%d",10,0
	even
.temp2:	dc.l	0
.tt2:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	d0,a0
	jsr	_LVOGfxFree(a6)
	clr.l	it_vpextra
.no_vpex:
	rts

*
* Free view without the copper lists
*
free_view:
	bsr	free_it_colormap

	move.l		itviewp(pc),a0
	tst.l		vp_UCopIns(a0)
	beq		.no_cop

	move.l		$4.w,a6
	jsr		_LVOForbid(a6)

	move.l		itviewp(pc),a0
	move.l		vp_DspIns(a0),d5
	move.l		vp_SprIns(a0),d6
	move.l		vp_ClrIns(a0),d7

	move.l		#0,vp_DspIns(a0)
	move.l		#0,vp_SprIns(a0)
	move.l		#0,vp_ClrIns(a0)
	move.l		db_graphbase(a3),a6
	jsr		_LVOFreeVPortCopLists(a6)

	move.l		itviewp(pc),a0
	move.l		d5,vp_DspIns(a0)
	move.l		d6,vp_SprIns(a0)
	move.l		d7,vp_ClrIns(a0)

	move.l		$4.w,a6
	jsr		_LVOPermit(a6)
.no_cop:

	bsr		free_it_cop_frames
	bsr		free_it_gfx_new
	rts
*
* Free total view
*
free_view_ucop:
	bsr		free_it_colormap
	move.l		db_graphbase(a3),a6
	move.l		itviewp(pc),a0
	jsr		_LVOFreeVPortCopLists(a6)
	bsr		free_it_cop_frames
	bsr		free_it_gfx_new
	rts

free_view25u:
	move.l		db_graphbase(a3),a6
	move.l		itviewp(pc),a0
	jsr		_LVOFreeVPortCopLists(a6)
	bsr		free_it_cop_frames
	rts

init_cop_vports:
	lea	t1view(pc),a0
	lea	itview(pc),a1
	move.l	a0,(a1)
	lea	t2view(pc),a0
	lea	atview(pc),a1
	move.l	a0,(a1)

	lea	t1viewp(pc),a0
	lea	itviewp(pc),a1
	move.l	a0,(a1)

	lea	t2viewp(pc),a0
	lea	atviewp(pc),a1
	move.l	a0,(a1)

	bsr	init_vportsn			; voor de copper effecten
	bsr	switch_temp_views
	bsr	init_vportsn

	rts

init_vportsn:
	move.l		db_graphbase(a3),a6
	move.l		itview(pc),a1
	jsr		_LVOInitView(a6)

	move.l		itviewp(pc),a0
	jsr		_LVOInitVPort(a6)

	rts

*
* Add color 0 at the bottom of the active and use the inactive color
*
init_user_coplist00:
	move.l	atviewp(pc),a1
	move.w	vp_DyOffset(a1),d7
	exg	a4,a5
	bsr	in_coplist0
	exg	a4,a5
	rts

*
* Add color 0 at the bottom of the inactive and use the active color
*
init_user_coplist0:
	move.l	itviewp(pc),a1
	move.w	vp_DyOffset(a1),d7

in_coplist0:
	IFNE XAPP
	move.l	db_mlsysstruct(a3),a0
	move.l	ml_miscFlags(a0),d0
	btst	#5,d0
	beq	.do_ucop
	move.l	#0,db_ucop_pointer(a3)
	rts
	ENDC
.do_ucop:
	moveq	#ucl_SIZEOF,d0
	move.l	#MEMF_CLEAR,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)

	move.l	d0,db_ucop_pointer(a3)
	beq	no_ucop_mem
	
	move.l	db_ucop_pointer(a3),a0
	moveq	#10,d0
	tst.b	db_aa_present(a3)
	beq	no_aa124b
	moveq	#20,d0
no_aa124b:
	move.l	db_graphbase(a3),a6
	jsr	_LVOUCopperListInit(a6)

	move.w	db_inactive_max(a3),d6		; first dy offset

	sub.w	d7,d6
	move.w	d6,d0

	bra	no_iuc_adj
*
* Add color 0 at the top of the inactive viewport us the inactive color
*
init_user_coplistb:
	move.l	itviewp(pc),a1
	move.w	vp_DyOffset(a1),d7
	exg	a4,a5
	bsr	in_coplist
	exg	a4,a5
	rts
*
* Add color 0 at the top of the active viewport
*
init_user_coplist:
	move.l	atviewp(pc),a1
	move.w	vp_DyOffset(a1),d7
in_coplist:
	IFNE XAPP
	move.l	db_mlsysstruct(a3),a0
	move.l	ml_miscFlags(a0),d0
	btst	#5,d0
	beq	.do_ucop
	move.l	#0,db_ucop_pointer(a3)
	rts
	ENDC
.do_ucop:
	moveq	#ucl_SIZEOF,d0
	move.l	#MEMF_CLEAR,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)

	move.l	d0,db_ucop_pointer(a3)
	beq	no_ucop_mem
	
	move.l	db_ucop_pointer(a3),a0
	moveq	#10,d0
	tst.b	db_aa_present(a3)
	beq	no_aa124
	moveq	#20,d0
no_aa124:
	move.l	db_graphbase(a3),a6
	jsr	_LVOUCopperListInit(a6)

	move.w	db_active_min(a3),d6		; first dy offset

	moveq	#0,d0

	cmp.w	d6,d7
	ble	no_iuc_adj
	sub.w	d6,d7
	move.w	d7,d0
	neg.w	d0
no_iuc_adj:
;	tst.b	vb_interlace(a5)
;	beq	no_int5
;	tst.b	vb_interlace(a4)
;	bne	no_int5
;	asr.w	#1,d0
;no_int5:
	move.w	d0,db_topoffset(a3)
	moveq	#10,d1
	move.l	db_ucop_pointer(a3),a2
	move.l	a2,a1
	jsr	_LVOCWait(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)
	exg	a4,a5
	bsr	init_nul
	exg	a4,a5


;	move.l	#$180,d0
;	move.w	#$f0f,d1
;	move.l	db_ucop_pointer(a3),a1
;	jsr	_LVOCMove(a6)
;	move.l	db_ucop_pointer(a3),a1
;	jsr	_LVOCBump(a6)


	move.l	#10000,d0
	move.l	#255,d1
	move.l	a2,a1
	jsr	_LVOCWait(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)
no_ucop_mem:
	rts

init_nul:
	move.l	db_ucop_pointer(a3),a2
	tst.b	db_aa_present(a3)
	beq	no_aa11

	move.w	#$106,d0
	move.w	#$0c01,d1			; set the high nibble
	move.l	a2,a1
	jsr	_LVOCMove(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)

	move.b	vb_colbytes(a4),d1
	and.w	#$f0,d1
	lsl.w	#4,d1
	move.b	vb_colbytes+1(a4),d0
	and.w	#$f0,d0
	or.w	d0,d1
	move.b	vb_colbytes+2(a4),d0
	and.w	#$f0,d0
	lsr.w	#4,d0
	or.w	d0,d1
	move.w	#$180,d0
	move.l	a2,a1
	jsr	_LVOCMove(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)

	move.w	#$106,d0
	move.w	#$0e01,d1		; set low nibble mode
	move.l	a2,a1
	jsr	_LVOCMove(a6)
	move.l	a2,a1
	jsr	_LVOCBump(a6)

	move.b	vb_colbytes(a4),d1
	and.w	#$f,d1
	lsl.w	#8,d1
	move.b	vb_colbytes+1(a4),d0
	and.w	#$f,d0
	lsl.w	#4,d0
	or.w	d0,d1
	moveq	#0,d0
	move.b	vb_colbytes+2(a4),d0
	and.w	#$f,d0
	or.w	d0,d1
	move.w	#$180,d0
	move.l	a2,a1
	jsr	_LVOCMove(a6)

	bra	aa121
no_aa11
	move.w	#$180,d0
	move.w	vb_colors(a4),d1
	move.l	a2,a1
	jsr	_LVOCMove(a6)
aa121:
	move.l	a2,a1
	jsr	_LVOCBump(a6)
	rts

	IFNE NEWEFF
*
* For the drip versions
* In d7 the line to wait
*
init_user_coplist3:

; first free the old list

	move.l		itviewp(pc),a0
	jsr		_LVOFreeVPortCopLists(a6)
	move.l		itviewp(pc),a0
	move.l		#0,vp_UCopIns(a0)
	
	moveq	#ucl_SIZEOF,d0
	move.l	#MEMF_CLEAR,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)

	move.l	d0,db_ucop_pointer(a3)
	beq	no_ucop_mem3
	
	moveq	#10,d0
	tst.b	db_aa_present(a3)
	beq	no_aa123
	moveq	#14,d0
no_aa123:
	move.l	db_ucop_pointer(a3),a0
	move.l	db_graphbase(a3),a6
	jsr	_LVOUCopperListInit(a6)

	move.l	d7,d0
	moveq	#10,d1
	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCWait(a6)
	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCBump(a6)

	move.l	#$180,d0
	move.l	db_inactive_viewblok(a3),a5
	move.w	vb_colors(a5),d1
	move.w	#$f0f,d1
	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCMove(a6)
	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCBump(a6)

	move.l	#10000,d0
	move.l	#255,d1
	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCWait(a6)

	move.l	db_ucop_pointer(a3),a1
	jsr	_LVOCBump(a6)
no_ucop_mem3:
	rts

	ENDC
*
* Get from de copperlist the modulo value
* a0 points to the view
* put the data in the view block pointed at by a5
*
get_modulos:
	cmp.l		#0,a0
	beq		exit_putcopy
	move.l		v_LOFCprList(a0),a0
	cmp.l		#0,a0
	beq		exit_putcopy
	move.w		crl_MaxCount(a0),d2		; aantal elementen
	move.l		crl_start(a0),a0		; eigenlijke copperlist
	subq.l		#1,d2
mrep_putcop:
	move.w		(a0)+,d0
	cmp.w		#$ffff,d0
	beq		exit_putcopy

	cmp.w		#$108,d0
	bne		no_putmod8
modt:
	move.w		(a0),vb_moda_even(a5)
;	bra		exit_getmodulo
no_putmod8:
	cmp.w		#$10a,d0
	bne		no_putmoda
	move.w		(a0),vb_moda_odd(a5)
	bra		exit_getmodulo
no_putmoda:	
	addq.l		#2,a0
	dbf		d2,mrep_putcop
exit_getmodulo:
	move.l		vb_breedte_x(a5),d0
	move.w		vb_planes(a5),d1
	subq.w		#1,d1
	tst.b		vb_interlace(a5)
	beq		gm_no_int1
	add.w		vb_planes(a5),d1
gm_no_int1:
	mulu		d1,d0
	move.w		vb_moda_even(a5),d2
	move.w		d2,vb_moda(a5)
	sub.w		d0,d2
	move.w		d2,vb_modulodisp(a5)

exit_putcopy:
	rts

cpp_data:	ds.b	cpp_SIZEOF

init_cop_play_list:
	lea	cpp_data(pc),a2
	move.l	a2,db_coplist(a3)
	move.w	cpp_size(a2),d0		; number of elements
	move.w	#0,cpp_init(a2)
	move.l	db_graphbase(a3),cpp_gfxbase(a2)
	move.l	#0,cpp_sig(a2)
	move.l	#0,cpp_task(a2)

	lea	cpp_lists(a2),a0

	move.l	atview(pc),cpp_view(a2)
	
	move.l	a0,cpp_playp(a2)		; set start pointer
	move.l	a0,cpp_storep(a2)		; set start store pointer
	move.l	a0,cpp_playedp(a2)		; clear pointer
rep_set_l_z:
	move.w	#0,(a0)+
	move.l	#0,(a0)+
	move.l	#0,(a0)+
	move.w	#0,(a0)+
	dbf	d0,rep_set_l_z
	move.w	#-1,(a0)			; indicate the end of the list
	rts

create_cop_view:

	move.l	db_graphbase(a3),a6

	tst.w	db_cop_active_test(a3)
	beq	no_active_cop

	move.l	itview(pc),a0			; first the active view
	move.l	atviewp(pc),a1
	move.w	vp_Modes(a1),d0
	or.w	db_or_vmode_mask(a3),d0
	move.w	d0,v_Modes(a0)
	or.w	#EXTEND_VSTRUCT,v_Modes(a0)	; MONI ?????????????????
	move.w	db_tmode(a3),d0
	or.w	d0,v_Modes(a0)
	jsr	_LVOMakeVPort(a6)

no_active_cop:

	tst.w	db_cop_inactive_test(a3)
	beq	no_cop_inactive
qpp:
	move.l	atviewp(pc),a0
	move.l	itviewp(pc),a1		; Place the inactive in the next var

;	move.w	vp_DyOffset(a1),d0
;	move.l	vp_RasInfo(a1),a2
;	move.w	ri_RyOffset(a2),d0

	move.w	vp_DHeight(a0),d0
	bne	.okie
;	add.w	#8,vp_DHeight(a1)
;	sub.w	#8,ri_RyOffset(a2)
	nop
.okie:

	move.l	a1,vp_Next(a0)

	move.l	itview(pc),a0
	move.l	itviewp(pc),a1
	move.w	vp_Modes(a1),d0
	or.w	#EXTEND_VSTRUCT,d0		; MONI ?????????????????
	or.w	db_or_vmode_mask(a3),d0
	move.w	d0,v_Modes(a0)
	move.w	db_tmode(a3),d0
	or.w	d0,v_Modes(a0)
	jsr	_LVOMakeVPort(a6)

	tst.w	db_cop_active_test(a3)
	bne	no_cop_inactive
	move.l	itview(pc),a1
	move.l	itviewp(pc),a0
	move.l	a0,v_ViewPort(a1)	; the active viewport is gone
no_cop_inactive:
	move.l	itview(pc),a1
	jsr	_LVOMrgCop(a6)

	move.l	db_coplist(a3),a0
	move.l	cpp_storep(a0),a0

	move.l	2(a0),d0
	beq	no_free2
	move.l	6(a0),d3
	move.l	d0,a0
	jsr	_LVOFreeCprList(a6)
no_free1:
	tst.l	d3
	beq	no_free2
	move.l	d3,a0
	jsr	_LVOFreeCprList(a6)
no_free2:
	move.l	db_coplist(a3),a6
	move.l	cpp_playp(a6),a0
	move.l	cpp_storep(a6),a0

	move.l	itview(pc),a1
	move.l	v_LOFCprList(a1),2(a0)		; store to use later
	move.l	v_SHFCprList(a1),6(a0)
	move.w	#3,(a0)				; set list to be used

	move.w	#-1,10(a0)
	cmp.w	#39,db_libversion(a3)
	blt	.no_v39
	move.l	it_vextra(pc),a1
	move.w	ve_TopLine(a1),10(a0)
.no_v39:

	lea	12(a0),a0			; set to next pointer
	cmp.w	#-1,(a0)
	bne	no_end_cpl
	lea	cpp_lists(a6),a0
no_end_cpl:
	move.l	a0,cpp_storep(a6)

;	cmp.w	#39,db_libversion(a3)
;	blt	.no_v39
;	move.l	it_vextra(pc),a0
;	move.l	at_vextra(pc),a1
;	move.w	ve_TopLine(a0),d0
;	move.w	ve_TopLine(a0),ve_TopLine(a1)
;	move.w	#$500,ve_TopLine(a1)
;.no_v39:
;	move.l	itview(pc),a1
;	move.l	db_graphbase(a3),a6
;	jsr	_LVOLoadView(a6)

;	move.l	itview(pc),a1
;	move.l	v_LOFCprList(a1),a1
;	move.l	crl_start(a1),a1		; eigenlijke copperlist

	move.l	itview(pc),a1
	clr.l	v_LOFCprList(a1)		; free later too
	clr.l	v_SHFCprList(a1)
	rts
*
* The width of the viewport is 0 so this is the first time ??
* Set effect on bang 30 mrt 1994
*
take_over:
	move.l	#0,db_which_wipe(a3)
	rts

it_vextra:	dc.l	0		; extra structure pointers
it_vpextra:	dc.l	0

at_vextra:	dc.l	0
at_vpextra:	dc.l	0

itview:		dc.l	0
atview:		dc.l	0
itviewp:	dc.l	0
atviewp:	dc.l	0

t1view:		ds.b	v_SIZEOF
t2view:		ds.b	v_SIZEOF

t1viewp:	ds.b	vp_SIZEOF
t2viewp:	ds.b	vp_SIZEOF

laad_kleuren_stub:
	bra	laad_kleuren
*
* zet de kleuren zo dat je geen verschil op het beeldscherm ziet
*
set_blend_colors_source:
	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_fileblok(a3),a4
	move.w	db_final_planes(a3),d0		; totaal aantal planes	
	moveq	#1,d1
	lsl.w	d0,d1				; totaal aantal kleuren

	lea.l	vb_colors(a5),a0		; destination kleuren
	move.w	db_active_planes(a3),d0
	moveq	#1,d2
	lsl.w	d0,d2				; active kleuren

	move.l	a0,a1

	moveq	#0,d3
	move.w	d2,d3
	add.w	d3,d3
	add.l	d3,a1				; dest
	move.w	d2,d3
rep_c_k:
	move.w	(a0)+,(a1)+
	subq.w	#1,d1
	subq.w	#1,d3	
	bne	rep_c_k
	lea	vb_colors(a5),a0
;	addq.l	#2,a0
	move.w	d2,d3
	tst.w	d1				; tot hele palette vol
	bne	rep_c_k
	rts

set_blend_colors_dest:
	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_fileblok(a3),a4

	move.w	db_final_planes(a3),d0		; totaal aantal planes	
	moveq	#1,d1
	lsl.w	d0,d1				; totaal aantal kleuren

	lea.l	vb_colors(a4),a0		; destination kleuren
	move.w	db_active_planes(a3),d0
	moveq	#1,d2
	lsl.w	d0,d2				; inactive kleuren

	move.l	a0,a1
	lea	db_cop_colorsLOF(a3),a1
	moveq	#0,d3
	move.w	d2,d3
rep_c_kd:
	move.w	(a0),(a1)+
	subq.w	#1,d1
	subq.w	#1,d3	
	bne	rep_c_kd
	addq.l	#2,a0
	move.w	d2,d3
	tst.w	d1				; tot hele palette vol
	bne	rep_c_kd
	rts

*
* zet de kleuren zo dat je geen verschil op het beeldscherm ziet
* de 256 kleuren versie
*
set_blend_colors_source256:
	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_fileblok(a3),a4
	move.w	db_final_planes(a3),d0		; totaal aantal planes	
	moveq	#1,d1
	lsl.w	d0,d1				; totaal aantal kleuren

	lea.l	vb_colbytes(a5),a0		; destination kleuren
	move.w	db_active_planes(a3),d0
	moveq	#1,d2
	lsl.w	d0,d2				; active kleuren

	move.l	a0,a1

	moveq	#0,d3
	move.w	d2,d3
	add.w	d3,d3
	add.w	d2,d3				; maal 3

	add.l	d3,a1				; dest
	move.w	d2,d3
rep_c_k256:
	move.b	(a0)+,(a1)+
	move.b	(a0)+,(a1)+
	move.b	(a0)+,(a1)+
	subq.w	#1,d1
	subq.w	#1,d3	
	bne	rep_c_k256
	lea	vb_colbytes(a5),a0
	move.w	d2,d3
	tst.w	d1				; tot hele palette vol
	bne	rep_c_k256
	rts

set_blend_colors_dest256:
	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_fileblok(a3),a4

	move.w	db_final_planes(a3),d0		; totaal aantal planes	
	moveq	#1,d1
	lsl.w	d0,d1				; totaal aantal kleuren

	lea.l	vb_colbytes(a4),a0		; destination kleuren
	move.w	db_active_planes(a3),d0
	moveq	#1,d2
	lsl.w	d0,d2				; inactive kleuren

	move.l	a0,a1
	lea	db_cop_colorsLOF(a3),a1
	moveq	#0,d3
	move.w	d2,d3
rep_c_kd256:
	move.b	(a0),(a1)+
	move.b	1(a0),(a1)+
	move.b	2(a0),(a1)+
	subq.w	#1,d1
	subq.w	#1,d3	
	bne	rep_c_kd256
	addq.l	#3,a0
;	lea	vb_colors(a4),a0
	move.w	d2,d3
	tst.w	d1				; tot hele palette vol
	bne	rep_c_kd256
	rts
*
* probeer een blend te maken
*
blend:
	move.l	db_active_viewblok(a3),a5
	move.l	vb_breedte_x(a5),d0

	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_fileblok(a3),a4
	move.l	vb_breedte_x(a5),d0

	move.w	vb_planes(a4),d0
	move.w	d0,db_inactive_planes(a3)
	move.w	vb_planes(a5),db_active_planes(a3)	
	add.w	vb_planes(a5),d0
	move.w	d0,db_final_planes(a3)
	move.w	db_max_Depth_Lores(a3),d1
	cmp.w	d1,d0
	bgt	no_blend

; het aantal planes is kleiner dan 5 of gelijk

	tst.b	vb_hires(a4)
	bne	d_hires1

	tst.b	vb_hires(a5)
	bne	d_hires1

; beide zijn lowres en er is dus een echte blend mogelijk

blend_posible:
	move.w	vb_planes(a4),d1
	move.w	d0,vb_planes(a4)	; zet dest op gewenste aantal planes

	move.l	d1,-(a7)
	bsr	clear_convert_bytes
	bsr	adaptfile_pcj		; maak meer planes
	move.l	(a7)+,d1

	move.l	db_inactive_fileblok(a3),a4
	move.w	d1,vb_planes(a4)	; orginele aantal planes
	tst.w	d0
	bne	no_blend

	move.l	db_inactive_fileblok(a3),a4
	move.w	d1,vb_planes(a4)	; orginele aantal planes


	move.b	#$ff,db_wipe_in_add_planes(a3)
	jsr	adaptdecrunch		; zet de source in meer planes

	tst.w	d0
	bne	no_blend

; zet nu de kleuren die niet gebruikt worden op de achtergrond kleur
; de fade moet nu van de eerste paar kleuren de 0 kleur inactive maken 
; en van de laatste paar kleuren de inactive kleuren maken

	tst.b	db_aa_present(a3)
	beq	no_aa5

	bsr	set_blend_colors_source256
	bsr	set_blend_colors_dest256
	bsr	combine_inac_active
	lea	db_cop_colorsLOF(a3),a0
	bsr	calc_it256

	move.l	#0,db_fade_counter(a3)

wacht_blend256:

	bsr	fade_colors

	bsr	wacht_tijd2
	bne	no_blend

	cmp.l	#255,db_fade_counter(a3)
	bge	exit_blend1

	bra	wacht_blend256

exit_blend1:
	move.l	#255,db_fade_counter(a3)
	bsr	fade_colors256_no_add
	bra	exit_blend

exit_b2561
	rts

no_aa5:
	bsr	set_blend_colors_source
	bsr	set_blend_colors_dest
	bsr	combine_inac_active
	lea	db_cop_colorsLOF(a3),a0
	bsr	calc_it

wacht_blend:
	bsr	fade_colors
	bsr	wacht_tijd2
	bne	exit_blend
	cmp.l	#16,db_fade_counter(a3)
	bge	exit_blend
	bra	wacht_blend
exit_blend:
; er staan nu twee plaatjes op het scherm waarvan je er een niet ziet
no_blend:

	bsr	proces_file			; show orgineel
	bsr	showpicture	
	rts
	
d_hires1:
; een of beiden zijn hires doe nu nog geen terug conversies
	move.w	db_max_Depth_Hires(a3),d1
	cmp.w	d1,d0
	bgt	no_blend
; het aantal planes is kleiner dan max_depth_hires
	bra	blend_posible

*
* combineer de inactive pic met de active picture
*
combine_inac_active:
	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_viewblok(a3),a4

	move.l	#256,d0
	bsr	laad_kleuren2
	
	move.l	vb_breedte_x(a5),d1
	move.l	d1,d0				; after adapt should be thesame

	mulu	db_inactive_planes(a3),d0	; breedte van de inactive lijn
	mulu	db_active_planes(a3),d1		; breedte van de active lijn

	move.l	vb_tempbuffer(a5),a1
	move.l	vb_tempbuffer(a4),a0
	add.l	d1,a1

	move.l	vb_breedte_x(a5),d1
	mulu	vb_planes(a5),d1		;breedte van de old active lijn

	move.l	a0,a2
	move.l	a1,a6

	move.w	vb_leny(a5),d7
	subq.w	#1,d7
rep_c1:
	move.l	a0,a2
	move.l	a1,a6
	move.l	d0,d6
	subq.l	#1,d6
rep_c2:
	move.b	(a2)+,(a6)+
	dbf	d6,rep_c2
	add.l	d1,a1
	add.l	d1,a0
	dbf	d7,rep_c1
	rts

calc_effect_speed_fade:
	tst.b	db_aa_present(a3)
	beq	no_aa7
	move.l	db_effect_speed(a3),db_effect_speed_fade(a3)
	moveq	#0,d7
	bra	set_varwtime
no_aa7:

	move.l	#1,db_effect_speed_fade(a3)		; standard 1/16

	move.l	db_effect_speed(a3),d7
	cmp.l	#14,d7
	blt	slow_fade
	sub.l	#13,d7
	move.l	d7,db_effect_speed_fade(a3)
	moveq	#0,d7
	bra	set_varwtime

slow_fade:
	moveq	#14,d7
	sub.l	db_effect_speed(a3),d7
	tst.l	d7
	bpl	no_min_50hz
	moveq	#1,d7
no_min_50hz:

set_varwtime:
	move.l	d7,db_varwtime(a3)
no_po_f:
	lea	tellerstart(pc),a0
	move.l	d7,(a0)
	lea	teller(pc),a0
	move.l	d7,(a0)
	rts

calc_effect_speed_blitin_ver:
	move.l	db_effect_speed_org(a3),d7
	cmp.l	#5,d7
	bgt	normal_vsp
	move.l	#1,db_effect_speed(a3)
	move.l	d7,db_varwtime(a3)
	lea	tellerstart(pc),a0
	move.l	db_varwtime(a3),(a0)
	rts
normal_vsp:
	subq.l	#5,d7
	move.l	d7,db_effect_speed(a3)
	rts

calc_effect_speed_blitin_ver1:
	move.l	db_effect_speed_org(a3),d7

	move.l	db_varltimes(a3),d6
	bne	oke_varl6
	moveq	#1,d6
oke_varl6:
	move.l	d6,db_effect_speed(a3)

	subq.w	#1,d6
	add.l	d6,d6
	add.l	d6,d6
	sub.l	d6,d7
	bmi	effv1_neg2
	move.l	d7,db_varltimes(a3)
	move.l	d7,db_counttimer(a3)
	move.l	#0,db_varwtime(a3)
	lea	tellerstart(pc),a0
	move.l	db_varwtime(a3),(a0)
	move.l	db_varwtime(a3),-4(a0)
	rts

effv1_neg2:
	neg.l	d7
	move.l	d7,db_varwtime(a3)
	move.l	#0,db_varltimes(a3)
	move.l	#0,db_counttimer(a3)
	lea	tellerstart(pc),a0
	move.l	db_varwtime(a3),(a0)
	move.l	db_varwtime(a3),-4(a0)
	rts


* reken aan de hand van effect_speed de snelheid van de wipe uit
*
calc_effect_speed_blitin:
	move.l	db_effect_speed_org(a3),d7
	cmp.l	#8,d7
	bge	no_50hzwait
	move.l	#1,db_effect_speed(a3)
	moveq	#8,d6
	sub.w	d7,d6

	move.l	d6,db_varwtime(a3)
	lea	tellerstart(pc),a0
	move.l	db_varwtime(a3),(a0)
	move.l	db_varwtime(a3),-4(a0)
	rts

no_50hzwait:
	move.l	#0,db_varwtime(a3)
	subq.l	#7,d7
	move.l	d7,db_effect_speed(a3)
	lea	tellerstart(pc),a0
	move.l	db_varwtime(a3),(a0)
	move.l	db_varwtime(a3),-4(a0)
	rts

calc_effect_speed_blitin2:
	move.l	db_effect_speed_org(a3),d7
	move.l	db_varltimes(a3),d6
	bne	ok_b2
	moveq	#1,d6			; screen mode ????
ok_b2:
	move.l	d7,db_varltimes(a3)
	move.l	d7,db_effect_speed(a3)
	lea	tellerstart(pc),a0
	move.l	db_varwtime(a3),(a0)
	rts


calc_effect_speed_ver1:			; vertikale lijnen
	move.l	db_effect_speed(a3),d7
	move.l	db_varltimes(a3),d6	; variatie
	bne	oke_v1_eff

	moveq	#1,d6			; adjust with screen mode ???

oke_v1_eff:
	cmp.l	#5,d6
	ble	no_po_calceffv1
	moveq	#5,d6			; maximaal 5
no_po_calceffv1:
	subq.w	#1,d6
	moveq	#1,d4
	lsl.l	d6,d4			; lijndikte
	add.l	d6,d6
	add.l	d6,d6
	sub.l	d6,d7
	bmi	effv1_neg
	move.l	d7,db_varltimes(a3)
	move.l	d7,db_counttimer(a3)
	move.l	#0,db_varwtime(a3)
	move.l	d4,db_effect_speed(a3)
	lea	tellerstart(pc),a0
	move.l	db_varwtime(a3),(a0)
	rts

effv1_neg:
	move.l	d4,db_effect_speed(a3)
	neg.l	d7
	move.l	#0,db_varltimes(a3)
	move.l	#0,db_counttimer(a3)
	move.l	d7,db_varwtime(a3)
	move.l	d4,db_effect_speed(a3)
	lea	tellerstart(pc),a0
	move.l	db_varwtime(a3),(a0)
	rts

calc_effect_speed_blitin3:
	move.l	db_effect_speed_org(a3),d0
	move.l	db_varltimes_in(a3),d1
	bne	no_ad_h5h6
	moveq	#1,d1
no_ad_h5h6:
	cmp.w	#6,d0
	bge	no_sub_h5h6
	moveq	#6,d2
	sub.w	d0,d2
	move.l	d2,d0
	move.l	d0,db_varwtime(a3)
	moveq	#6,d0
	bra	g_h5h6
no_sub_h5h6:
	move.l	#0,db_varwtime(a3)
g_h5h6:
	subq.w	#6,d0
	move.l	d1,d2
	add.l	d2,d2
	sub.l	d2,d0
	bpl	plus_h5h6
	neg.l	d0
	move.l	d0,db_varwtime(a3)
	moveq	#1,d0
plus_h5h6:
	move.l	d0,db_counttimer(a3)
	move.l	d0,db_varltimes(a3)
	move.l	d1,db_effect_speed(a3)
	lea	tellerstart(pc),a0
	move.l	db_varwtime(a3),(a0)
	rts

calc_effect_speed_blitin4:
	move.l	#0,db_varwtime(a3)
	move.l	db_effect_speed(a3),d0
	move.l	db_varltimes(a3),d1
	bne	ok_lijnb4
	moveq	#1,d1			; screen mode ??
ok_lijnb4:
	move.l	d0,d2
	move.l	d1,db_effect_speed(a3)

	move.l	d0,db_varltimes(a3)
	lea	tellerstart(pc),a0
	move.l	db_varwtime(a3),(a0)
	rts
	
calc_effect_speed_hor1:
	move.l	db_effect_speed(a3),d7
	move.l	db_varltimes(a3),d6	; variatie
	bne	oke_h1_notnul

	moveq	#1,d4			; adjust with screen mode ???

	bra	oke_h1_go_on

oke_h1_notnul:
	move.l	d6,d4			; lijndikte
	sub.l	d6,d7
	bmi	effh1_neg

oke_h1_go_on:
	move.l	d7,db_varltimes(a3)
	move.l	d7,db_counttimer(a3)
	move.l	#0,db_varwtime(a3)
	move.l	d4,db_effect_speed(a3)
	lea	tellerstart(pc),a0
	move.l	db_varwtime(a3),(a0)
	rts

effh1_neg:
	move.l	d4,db_effect_speed(a3)
	neg.l	d7
	move.l	#0,db_varltimes(a3)
	move.l	#0,db_counttimer(a3)
	move.l	d7,db_varwtime(a3)
	move.l	d4,db_effect_speed(a3)
	lea	tellerstart(pc),a0
	move.l	db_varwtime(a3),(a0)
	rts

calc_effect_speed_mov1:
	move.l	db_effect_speed(a3),d7
	move.l	db_varltimes(a3),d6	; variatie
	bne	oke_m1_notnul

	moveq	#16,d6			; adjust with screen mode ???

oke_m1_notnul:
	addq.l	#1,d6			; always 2

	move.l	d6,db_varltimes(a3)

	moveq	#0,d6
	sub.l	#18,d7
	bpl	no_subm1
	moveq	#1,d6
	move.l	d7,d6
	neg.l	d6
	moveq	#1,d7
no_subm1:
	move.l	d6,db_varwtime(a3)
	move.l	d7,db_effect_speed(a3)
	lea	tellerstart(pc),a0
	move.l	db_varwtime(a3),(a0)
	rts

	IFNE	DEBUG
dbstr6000:	dc.b	"Wait for key",10,0
dbstr6001:	dc.b	"Should wait is %d",10,0
dbstr6002:	dc.b	"Misc flags zijn %d, %d",10,0
dbstr6003:	dc.b	"Creating new colormap",10,0
dbstr6004:	dc.b	"Take over %d,%d",10,0
	even
	ENDC
	
*
* Wipes the inactive in with effect pointed at by db_effect_data(a3)
*
wipe_effect_in:

	IFEQ XAPP
	bra	dont_wait_for_obj
	ENDC

; it is possible that you must wait here for a time to pass
; or for the user to push a key

	tst.b	db_should_wait(a3)
	beq	dont_wait_for_obj

	move.l	db_mlsysstruct(a3),a0
	move.l	ml_miscFlags(a0),d0

	btst	#3,d0
	bne	wait_key

	bsr	wacht_key

	bra	dont_wait_for_obj

wait_key:
	bsr	wacht_key
	tst.b	db_quit(a3)
	bne	exit_wipein
	cmp.b	#2,d0
	bne	wait_key

dont_wait_for_obj:

	tst.b	db_quit(a3)
	bne	exit_wipein

	move.l	db_varltimes(a3),db_varltimes_in(a3)
	move.w	#$fca,db_blitin_con0(a3)
	move.w	#$9f0,db_blitin_clear(a3)
	move.b	#$0,db_screen_empty(a3)

	move.l	db_inactive_viewblok(a3),a4
	move.w	#0,vb_clip_width(a4)
	move.l	db_active_viewblok(a3),a5
	move.w	#0,vb_clip_width(a5)
	
	move.l	db_effect_data(a3),a0
	move.l	eff_inspeed(a0),d1
	cmp.l	#0,d1
	bgt	no_ipro1
	moveq	#1,d1
no_ipro1:
	move.l	d1,db_effect_speed(a3)
	move.l	d1,db_effect_speed_in(a3)
	move.l	d1,db_effect_speed_org(a3)
	move.l	eff_inthick(a0),d1
	cmp.l	#0,d1
	bge	no_ipro2
	moveq	#0,d1
no_ipro2:
	move.l	d1,db_varltimes(a3)
	move.l	#0,db_variation(a3)

	move.l	eff_innum(a0),d0
	cmp.l	#NUM_LINEWIPES,d0
	ble	effect_oke
	moveq	#0,d0
effect_oke:
	cmp.l	#-1,d0
	bne	no_bang1
	moveq	#0,d0
no_bang1:
	lea	blitin_types(pc),a4
	move.b	#$0,db_please_no_double(a3)
	move.b	0(a4,d0.l),d1
	and.b	#DOUBLE_BUF,d1
	bne	no_doub
	move.b	#$ff,db_please_no_double(a3)
no_doub:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dat(pc),a1
	move.l	d0,(a1)
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"Wipe eff in %ld",10,0
	even
.dat:	dc.l	0
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC
	move.b	#0,db_reverse_effect(a3)

	lea	blitin_wipes(pc),a4
	add.l	d0,d0
	add.l	d0,d0			; longwords
	move.l	0(a4,d0),a4
	jsr	(a4)

	move.w	#$fca,db_blitin_con0(a3)
	move.w	#$9f0,db_blitin_clear(a3)
	move.b	#$0,db_screen_empty(a3)

	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)

	tst.b	db_quit(a3)
	bne	exit_wipein

	move.l	db_effect_data(a3),a0
	cmp.l	#-1,eff_outnum(a0)
	beq	exit_wipein2
	
	bsr	wacht_tijd2

	move.l	db_effect_data(a3),a0
	move.l	eff_delay1(a0),d0
	mulu	#50,d0			; PAL or NTSC
	lea	teller(pc),a0
	move.l	d0,(a0)
	move.l	d0,off_tel_start(a0)

	bsr	wacht_key

; laat de wipe in zien en wipe hem daarna weer uit

	move.l	db_triple_mem(a3),db_waardestpic(a3)

	move.l	db_effect_data(a3),a0
	move.l	eff_outnum(a0),d0
;	move.b	#$0,db_please_no_double(a3)	; should be here ?????

	move.l	eff_outspeed(a0),d1
	cmp.l	#0,d1
	bgt	no_pro1
	moveq	#1,d1
no_pro1:
	move.l	d1,db_effect_speed(a3)
	move.l	d1,db_effect_speed_in(a3)
	move.l	d1,db_effect_speed_org(a3)
	move.l	eff_outthick(a0),d1
	cmp.l	#0,d1
	bge	no_pro2
	moveq	#0,d1
no_pro2:
	move.l	d1,db_varltimes(a3)
	move.l	#0,db_variation(a3)

	bsr	calc_effect_speed_blitin

	move.b	#$ff,db_reverse_effect(a3)
	cmp.l	#NUM_LINEWIPES,d0
	ble	effect_oke2
	moveq	#0,d0
effect_oke2:
	lea	wipesrev(pc),a4
	add.l	d0,d0
	add.l	d0,d0
	move.l	0(a4,d0),a4
	jsr	(a4)	
	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)

	bra	exit_wipein3

	rts

exit_wipein:
	move.l	db_effect_data(a3),a0
	move.l	eff_delay2(a0),d0
	beq	no_delay2

	bsr	wacht_tijd2

	move.l	db_effect_data(a3),a0
	move.l	eff_delay2(a0),d0
	mulu	#50,d0			; PAL or NTSC
	lea	teller(pc),a0
	move.l	d0,(a0)
	move.l	d0,off_tel_start(a0)
	move.b	#1,db_should_wait(a3)

no_delay2:
	move.b	#0,db_triple_changed(a3)
	move.b	#0,db_blitin_adapted(a3)
	move.b	#0,db_blitin_mask_ready(a3)
	rts

exit_wipein3:
	move.l	db_effect_data(a3),a0
	lea	blitin_types(pc),a4
	move.l	eff_outnum(a0),d0
	move.b	0(a4,d0.l),d1
	and.b	#BLIT_STAY,d1
	beq	exit_wipein
	bsr	set_min_max_values
	bra	exit_wipein

exit_wipein2:
	bsr	set_min_max_values

	bsr	wacht_tijd2

	move.l	db_effect_data(a3),a0
	move.l	eff_delay1(a0),d0
	beq	no_delay2

;	bsr	wacht_tijd2

	move.l	db_effect_data(a3),a0
	move.l	eff_delay1(a0),d0
	mulu	#50,d0			; PAL or NTSC
	lea	teller(pc),a0
	move.l	d0,(a0)
	move.l	d0,off_tel_start(a0)
	move.b	#1,db_should_wait(a3)
	bra	no_delay2

*
* intialiseer de verschillende hoek vars
* in d0 dient de x-radius te staan
* in d1 dient de y-radius te staan en in d3 de effect speed
* retourneert in d6 het aantal keer waarin de cirkel doorlopen wordt
*
init_hoeken:
	tst.l	d0
	bne	no_pro_inith
	moveq	#1,d0
no_pro_inith:
	move.l	a6,-(a7)
	move.l	d1,-(a7)
	move.l	db_mathffpbase(a3),a6
	jsr	_LVOSPFlt(a6)
	move.l	d0,db_radiusx(a3)
	move.l	(a7)+,d1
	move.l	d1,d0
	tst.l	d0
	bne	no_pro_inith2
	moveq	#1,d0
no_pro_inith2:
	move.l	db_mathffpbase(a3),a6
	jsr	_LVOSPFlt(a6)
	move.l	d0,db_radiusy(a3)
	
	move.l	db_radiusy(a3),d0
	move.l	db_radiusx(a3),d1
	jsr	_LVOSPCmp(a6)
	ble	take_x
	move.l	db_radiusy(a3),d0
	bra	take_y
take_x:
	move.l	db_radiusx(a3),d0

take_y:
	move.l	d0,d4
	move.l	d3,d0
	jsr	_LVOSPFlt(a6)
	move.l	d4,d1
	exg	d0,d1
	jsr	_LVOSPDiv(a6)
	move.l	d0,d4
	jsr	_LVOSPFix(a6)
	move.l	d0,d6			; aantal keer
	
	moveq	#0,d0
	jsr	_LVOSPFlt(a6)
	move.l	d0,db_temphoek(a3)
	move.l	db_mathtransbase(a3),a6
	jsr	_LVOSPAcos(a6)		; d0 is .5 * pi

	move.l	d0,-(a7)
	move.l	db_mathffpbase(a3),a6
	moveq	#2,d1
	move.l	db_mathffpbase(a3),a6
	jsr	_LVOSPFlt(a6)
	move.l	d0,d1
	move.l	(a7),d0
	jsr	_LVOSPDiv(a6)		; delta hoek nu in d0
	move.l	d0,db_temphoeky(a3)
	move.l	(a7)+,d0

	move.l	db_mathffpbase(a3),a6
	move.l	d4,d1
	jsr	_LVOSPDiv(a6)		; delta hoek nu in d0
	move.l	d0,db_deltahoek(a3)
	move.l	(a7)+,a6
	rts
*
* retourneer in d3 radiusx maal cos( a )
* 		d4 radiusy maal sin( a )
* bij temphoek wordt delta hoek opgeteld
*
sincos:
	move.l	a6,-(a7)
	move.l	db_temphoek(a3),d0
	move.l	db_mathtransbase(a3),a6
	jsr	_LVOSPSin(a6)
	move.l	d0,d3
	move.l	db_temphoeky(a3),d0
	jsr	_LVOSPCos(a6)
	move.l	db_radiusy(a3),d1
	move.l	db_mathffpbase(a3),a6
	jsr	_LVOSPMul(a6)		; r * sin ( a )		
	jsr	_LVOSPFix(a6)
	move.l	d0,d4
	move.l	db_radiusx(a3),d0
	move.l	d3,d1
	jsr	_LVOSPMul(a6)		; r * cos ( a )
	jsr	_LVOSPFix(a6)
	move.l	d0,d3
	move.l	db_temphoek(a3),d0
	move.l	db_deltahoek(a3),d1
	move.l	db_mathffpbase(a3),a6
	jsr	_LVOSPAdd(a6)
	move.l	d0,db_temphoek(a3)		; een keer delta verder	
	move.l	db_temphoeky(a3),d0
	move.l	db_deltahoek(a3),d1
	move.l	db_mathffpbase(a3),a6
	jsr	_LVOSPAdd(a6)
	move.l	d0,db_temphoeky(a3)		; een keer delta verder	
	move.l	(a7)+,a6
	rts

neg_rady:
	move.l	a6,-(a7)
	move.l	db_radiusy(a3),d0
	move.l	db_mathffpbase(a3),a6
	jsr	_LVOSPNeg(a6)		; r * sin ( a )		
	move.l	d0,db_radiusy(a3)
	move.l	(a7)+,a6
	rts

neg_radx:
	move.l	a6,-(a7)
	move.l	db_radiusx(a3),d0
	move.l	db_mathffpbase(a3),a6
	jsr	_LVOSPNeg(a6)		; r * sin ( a )		
	move.l	d0,db_radiusx(a3)
	move.l	(a7)+,a6
	rts
*
* retourneer in d3 radiusx maal sin( a )
* 		d4 radiusy maal sin( a )
* bij temphoek wordt delta hoek opgeteld
*
sinsin:
	move.l	a6,-(a7)
	move.l	db_temphoek(a3),d0
	move.l	db_mathtransbase(a3),a6
	jsr	_LVOSPSin(a6)
	move.l	d0,d3

	move.l	db_radiusy(a3),d1
	move.l	db_mathffpbase(a3),a6
	jsr	_LVOSPMul(a6)		; r * sin ( a )		
	jsr	_LVOSPFix(a6)
	move.l	d0,d4
	move.l	db_radiusx(a3),d0
	move.l	d3,d1
	jsr	_LVOSPMul(a6)		; r * cos ( a )
	jsr	_LVOSPFix(a6)
	move.l	d0,d3
	move.l	db_temphoek(a3),d0
	move.l	db_deltahoek(a3),d1
	move.l	db_mathffpbase(a3),a6
	jsr	_LVOSPAdd(a6)
	move.l	d0,db_temphoek(a3)		; een keer delta verder	
	move.l	db_temphoeky(a3),d0
	move.l	db_deltahoek(a3),d1
	move.l	db_mathffpbase(a3),a6
	jsr	_LVOSPAdd(a6)
	move.l	d0,db_temphoeky(a3)		; een keer delta verder	
	move.l	(a7)+,a6
	rts
		
path_init:
	bsr	calc_effect_speed_blitin
	move.l	db_effect_speed(a3),d7
	move.l	d7,-(a7)
	add.l	d7,d7
	move.l	d7,db_effect_speed(a3)
	jsr	adapt_blitin_exvert_moving
	move.l	(a7)+,d7
	move.l	d7,db_effect_speed(a3)

path_init2:
	jsr	maak_mask_blitin	
	bsr	copy_inactive_triple
	tst.w	d0
	bne	no_triple11

path_init3:
	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_fileblok(a3),a4
	move.l	db_triple_mem(a3),db_waardestpic(a3)
	move.l	db_blitin_x(a3),d0
	move.l	db_blitin_y(a3),d1
	move.l	db_effect_speed(a3),d3

path_end1:
	move.l	db_wpatroonruimte(a3),a6	; sla hier het pad op
path_end2:
	move.w	#PATHEND,(a6)+
	move.w	#PATHEND,(a6)+
	rts

blitin_direct_rev:
	move.w	#$0faa,db_blitin_con0(a3)

	move.l	db_blitin_ox(a3),d0
	move.l	db_blitin_y(a3),d1
	bsr	put_blitin_in3		; zet de blitin neer
	move.w	#$0fca,db_blitin_con0(a3)
	rts
	
blitin_down_up_rev:
	bsr	blitin_mov_init2
	bsr	maak_path_down_up
	bsr	set_blit_fast_du
	bra	reverse_it

blitin_up_down_rev:
	bsr	blitin_mov_init2
	bsr	maak_path_up_down
	bsr	set_blit_fast_ud
	bra	reverse_it

blitin_left_right_rev:
	bsr	blitin_mov_init2
	bsr	maak_path_left_right
	bsr	set_blit_fast_lr
	bra	reverse_it


blitin_right_left_rev:
	bsr	blitin_mov_init2
	bsr	maak_path_right_left
	bsr	set_blit_fast_rl
	bra	reverse_it

blitin_lefttop_right_rev:
	bsr	blitin_mov_init2
	bsr	maak_path_lefttop_right
	bsr	set_blit_fast_ltr
	bra	reverse_it

blitin_righttop_left_rev:
	bsr	blitin_mov_init2
	bsr	maak_path_righttop_left
	bsr	set_blit_fast_rtl
	bra	reverse_it

blitin_rightbot_left_rev:
	bsr	blitin_mov_init2
	bsr	maak_path_rightbot_left
	bsr	set_blit_fast_rbl
	bra	reverse_it

blitin_leftbot_right_rev:
	bsr	blitin_mov_init2
	bsr	maak_path_leftbot_right
	bsr	set_blit_fast_lbr
reverse_it:
	move.l	a6,db_path_einde(a3)
	bsr	by_path_reverse
	rts

blitin_h2m1_rev:
	bsr	calc_effect_speed_blitin
	bsr	init_h1m1_rev

	move.l	db_inactive_fileblok(a3),a4
	moveq	#0,d5
	move.w	#1,d3
	move.w	db_blitin_fy(a3),d3
rep_b_h2m1_rev:
	movem.l	d0-d5/a4/a5,-(a7)

	movem.l	d0-d5/a4/a5,-(a7)
	move.l	d1,d6
	move.l	db_effect_speed(a3),d1
;	sub.l	d1,d6
	tst.b	db_double_buf(a3)
	beq	no_db1
	add.l	d1,d1
no_db1:
	sub.l	d1,d6
	bsr	restore_pic_xy
	movem.l	(a7)+,d0-d5/a4/a5

	move.l	d1,-(a7)
	bsr	put_blitin_in2		; zet de blitin neer
	move.l	(a7)+,d1

	bsr	wacht_tijd2
	movem.l	(a7)+,d0-d5/a4/a5
	bne	exit_h2m1_rev

	sub.l	db_effect_speed(a3),d3
	add.l	db_effect_speed(a3),d1

	cmp.w	db_blitin_fy(a3),d3
	cmp.w	#0,d3
	bge	rep_b_h2m1_rev
exit_h2m1_rev:

	move.l	d1,d6
	move.l	db_effect_speed(a3),d1
	tst.b	db_double_buf(a3)
	beq	no_db4
	add.l	d1,d1
no_db4:
	sub.l	d1,d6
	bsr	restore_pic_xy
	bsr	showinactive2
	rts

blitin_h1m1_rev:
	bsr	calc_effect_speed_blitin
	bsr	init_h1m1_rev

	move.l	db_inactive_fileblok(a3),a4
	move.w	db_blitin_fy(a3),d3
	move.w	#0,d5
rep_b_h1m1_rev:
	movem.l	d0-d5/a4/a5,-(a7)

	movem.l	d0-d5/a4/a5,-(a7)
	move.l	db_blitin_y(a3),d6
	move.l	db_effect_speed(a3),d1
	tst.b	db_double_buf(a3)
	beq	no_db2
	add.l	d1,d1
no_db2:
	add.l	d3,d6
	bsr	restore_pic_xy
	movem.l	(a7)+,d0-d5/a4/a5

	move.l	d3,-(a7)
	bsr	put_blitin_in2		; zet de blitin neer
	move.l	(a7)+,d3
	
	bsr	wacht_tijd2
	movem.l	(a7)+,d0-d5/a4/a5
	bne	exit_blh1m1_rev

	sub.l	db_effect_speed(a3),d3
	add.l	db_effect_speed(a3),d5
	cmp.w	#0,d3
	bge	rep_b_h1m1_rev
exit_blh1m1_rev:
	move.l	db_blitin_y(a3),d6
	move.l	db_effect_speed(a3),d1
	tst.b	db_double_buf(a3)
	beq	no_db3
	add.l	d1,d1
no_db3:
	add.l	d3,d6
	bsr	restore_pic_xy
	bsr	showinactive2
	rts

blitin_v1_rev:
	lea	blitin_v1_in(pc),a0
	bra	do_rev2
blitin_v2_rev:
	lea	blitin_v2_in(pc),a0
	bra	do_rev2
blitin_v3_rev:
	lea	blitin_v3_in(pc),a0
	bra	do_rev2
blitin_v4_rev:
	lea	blitin_v4_in(pc),a0
	bra	do_rev2
blitin_v5_rev:
	lea	blitin_v5_in(pc),a0
	bra	do_rev2
blitin_v6_rev:
	lea	blitin_v6_in(pc),a0
	bra	do_rev2

blitin_v7_rev:
	lea	blitin_v7_in(pc),a0
do_rev2:
	move.w	#$0fae,db_blitin_con0(a3)
	jsr	(a0)
	move.w	#$0fca,db_blitin_con0(a3)
	rts
	
blitin_lijnh1_rev:
blitin_lijnh2_rev:
blitin_lijnh3_rev:
	bra	blitin_direct_rev

blitin_h1_rev:
	lea	blitin_h1(pc),a0
	bra	do_hrev
blitin_h2_rev:
	lea	blitin_h2(pc),a0
	bra	do_hrev
blitin_h3_rev:
	lea	blitin_h3(pc),a0
	bra	do_hrev
blitin_h4_rev:
	lea	blitin_h4(pc),a0
	bra	do_hrev
blitin_h5_rev:
	lea	blitin_h5(pc),a0
	bra	do_hrev
blitin_h6_rev:
	lea	blitin_h6(pc),a0
	bra	do_hrev
blitin_h3_ver2_rev:
	lea	blitin_h3_ver2(pc),a0
;	bra	do_hrev
do_hrev:
	move.w	#$0faa,db_blitin_con0(a3)
	jsr	(a0)
	move.w	#$0fca,db_blitin_con0(a3)
	rts

blitin_bend_path1_rev:
	bsr	path_init3
	bsr	bend_path1_in
	bra	check_rev_above

blitin_bend_path2_rev:
	bsr	path_init3
	bsr	bend_path2_in
check_rev_above:
	move.l	a6,db_path_einde(a3)
	bsr	set_blit_fast_above
	bsr	by_path_reverse
	rts

blitin_bend_path3_rev:
	bsr	path_init3
	bsr	bend_path3_in
	bra	check_rev_below

blitin_bend_path4_rev:
	bsr	path_init3
	bsr	bend_path4_in
check_rev_below:
	move.l	a6,db_path_einde(a3)
	bsr	set_blit_fast_below
	bsr	by_path_reverse
	rts

blitin_point_init:
	bsr	calc_effect_speed_blitin
	move.l	db_effect_speed(a3),d7
	move.l	d7,-(a7)
	jsr	adapt_blitin_exvert_moving
	move.l	(a7)+,d7
	move.l	d7,db_effect_speed(a3)

	bsr	path_init2
	sub.l	db_blitin_dx(a3),d0

	move.l	db_effect_speed(a3),d6
	bne	bl_pi_nopo
	moveq	#1,d6
bl_pi_nopo:

	move.l	db_active_viewblok(a3),a5
	moveq	#50,d7				; screen mode 
	tst.b	vb_hires(a5)
	bne	no_hi100
	lsr.l	#1,d7
no_hi100:
	tst.b	vb_shires(a5)
	beq	no_hi101
	lsl.l	#1,d7
no_hi101:
	divu	d6,d7
	move.w	d0,vb_clip_x(a5)
	move.w	d1,vb_clip_y(a5)
	move.l	db_inactive_viewblok(a3),a4
	move.w	d0,vb_clip_x(a4)
	move.w	d1,vb_clip_y(a4)
	rts

blitin_point4:
	bsr	blitin_point_init

	move.l	d6,d5
	moveq	#0,d6
	bra	blitin_do_point

blitin_point3:
	bsr	blitin_point_init

	move.l	d6,d5
;	neg.l	d6
	neg.l	d5
	bra	blitin_do_point

blitin_point2:
	bsr	blitin_point_init

	moveq	#0,d5
	bra	blitin_do_point

blitin_point1:
	bsr	blitin_point_init

	neg.l	d6
	move.l	d6,d5

blitin_do_point:
	move.w	d0,(a6)+	; start waarden
	move.w	d1,(a6)+
	move.l	d7,-(a7)
	neg.w	d6
	neg.w	d5
bl_p1_rep1:
	move.w	d5,(a6)+
	move.w	d6,(a6)+
	dbf	d7,bl_p1_rep1
	move.l	(a7)+,d7

	neg.w	d6
	neg.w	d5

bl_p1_rep2:
	move.w	d5,(a6)+
	move.w	d6,(a6)+
	dbf	d7,bl_p1_rep2
	move.w	#PATHEND,(a6)+
	move.w	#PATHEND,(a6)+

	lea	teller(pc),a0
	move.l	#1,(a0)
	move.l	#1,off_tel_start(a0)

blitin_do_path:
	move.l	db_varltimes(a3),d7
	beq	bl_p1_rep3
	subq.w	#1,d7
	cmp.w	#8,d7
	bne	bl_p1_rep3
	move.b	#$1,db_cycle_wait(a3)
bl_p1_rep3:
	move.l	d7,-(a7)
	bsr	by_path_relative
	move.l	(a7)+,d7
	tst.b	db_quit(a3)
	bne	.ex_do_p
	cmp.w	#8,d7
	beq	bl_p1_rep3
	dbf	d7,bl_p1_rep3
.ex_do_p:
	move.b	#$0,db_cycle_wait(a3)
	rts

blitin_bend_path4:
	bsr	blitin_bend_maak_path4
	bra	check_clear
blitin_bend_path3:
	bsr	blitin_bend_maak_path3
check_clear:
	bsr	set_blit_fast_below
	bra	by_path
blitin_bend_path2:
	bsr	blitin_bend_maak_path2
	bra	check_clear2
blitin_bend_path1:
	bsr	blitin_bend_maak_path1
check_clear2:
	bsr	set_blit_fast_above
	bra	by_path
*
* start rechts onder verborgen
*
blitin_bend_maak_path4:
	bsr	path_init
bend_path4_in:
	move.l	db_blitin_y(a3),d0
	moveq	#0,d1
	move.w	vb_leny(a5),d1
	sub.w	vb_leny(a4),d1
	sub.l	d0,d1

	move.w	vb_lenx(a5),d0
	move.l	d0,d7
	sub.l	db_blitin_dx(a3),d7
	sub.l	db_blitin_x(a3),d0

	bsr	init_hoeken
	
	tst.w	d6
	beq	rep_cirkel3
	subq.l	#1,d6
rep_cirkel4:
	bsr	sincos

	move.l	d7,d2
	sub.l	d3,d2
	move.w	d2,(a6)+

	move.l	db_blitin_y(a3),d1
	add.l	d4,d1
	move.w	d1,(a6)+
	dbf	d6,rep_cirkel4
	bra	path_end2
	
*
* start links onder verborgen
*
blitin_bend_maak_path3:
	bsr	path_init
bend_path3_in:
	moveq	#0,d7
	move.w	vb_lenx(a4),d7
	neg.l	d7
	sub.l	db_blitin_dx(a3),d7

	move.l	db_blitin_y(a3),d0
	moveq	#0,d1
	move.w	vb_leny(a5),d1
	sub.w	vb_leny(a4),d1
	sub.l	d0,d1

	moveq	#0,d0
	move.w	vb_lenx(a4),d0
	add.l	db_blitin_x(a3),d0

	bsr	init_hoeken
	tst.w	d6
	beq	rep_cirkel3
rep_cirkel3:
	bsr	sincos

	move.l	d7,d2
	add.l	d3,d2
	move.w	d2,(a6)+

	move.l	db_blitin_y(a3),d1
	add.l	d4,d1
	move.w	d1,(a6)+

	dbf	d6,rep_cirkel3
	bra	path_end2

*
* start links boven verborgen
*
blitin_bend_maak_path2:
	bsr	path_init
bend_path2_in:

	moveq	#0,d2
	move.w	vb_lenx(a4),d2		; start waarde x
	move.l	d2,d7			; temp x coordinaat
	neg.l	d7
	sub.l	db_blitin_dx(a3),d7

	add.l	db_blitin_x(a3),d2
	move.l	d2,d0

	move.l	db_blitin_y(a3),d1

	bsr	init_hoeken
	tst.w	d6
	beq	rep_cirkel1
	subq.l	#1,d6
rep_cirkel2:
	bsr	sincos
	move.l	d7,d2
	add.l	d3,d2
	move.w	d2,(a6)+
	move.l	db_blitin_y(a3),d1
	sub.l	d4,d1
	move.w	d1,(a6)+
	dbf	d6,rep_cirkel2
	bra	path_end2

*
* start rechts boven verborgen
* Try to create a eliptic movement
*	
blitin_bend_maak_path1:
	bsr	path_init
bend_path1_in:
	moveq	#0,d2
	move.w	vb_lenx(a5),d2		; start waarde x
	move.l	d2,d7			; temp x coordinaat
	sub.l	db_blitin_dx(a3),d7

	sub.l	db_blitin_x(a3),d2	; x radius

; de blitin is nu aangekomen op de postitie vanwaar
; de cirkel beweging kan beginnen
; ga nu via een kwart cirkel vanaf positie d2,0 naar de uiteindelijke postitie
; doe dit in radius / effect_speed stappen ( radius is blitin_y )


	move.l	db_blitin_y(a3),d1
	move.l	d2,d0
	bsr	init_hoeken

	tst.w	d6
	beq	rep_cirkel1
	subq.l	#1,d6
rep_cirkel1:
	bsr	sincos
	move.l	d7,d2
	sub.l	d3,d2
	move.w	d2,(a6)+
	move.l	db_blitin_y(a3),d1
	sub.l	d4,d1
	move.w	d1,(a6)+
	dbf	d6,rep_cirkel1
	bra	path_end2

by_path:
	move.l	a6,db_path_einde(a3)
	move.l	db_wpatroonruimte(a3),a6	; doorloop het pad op
	addq.l	#4,a6

	moveq	#0,d0
	moveq	#0,d1
	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_viewblok(a3),a4
	move.w	(a6),vb_clip_x(a5)
	move.w	(a6),vb_clip_x(a4)
	moveq	#0,d7
rep_bp1:
	move.w	(a6)+,d0
	move.w	(a6)+,d1
	cmp.w	#PATHEND,d0
	beq	exit_bp1

	movem.l	a6/d7,-(a7)
	bsr	put_blitin_in_new
	bsr	wacht_tijd2
	movem.l	(a7)+,d7/a6
	addq.w	#1,d7
	tst.b	d0
	bne	exit_bp1

	bra	rep_bp1
exit_bp1:
	tst.b	db_quit(a3)
	bne	no_triple11
	move.l	d0,-(a7)

	move.l	db_blitin_ox(a3),d0
	move.l	db_blitin_y(a3),d1

	bsr	put_blitin_in_new
	move.l	(a7)+,d0
no_triple11:
	rts

by_path_reverse:
	move.l	db_path_einde(a3),a6
	subq.l	#4,a6		; skip de -PATHEND's
	moveq	#0,d0
	moveq	#0,d1
rep_bpr1:
	move.w	-(a6),d1
	move.w	-(a6),d0
	cmp.w	#PATHEND,d0
	beq	exit_bpr1
	move.l	a6,-(a7)
	bsr	put_blitin_in_new
	bsr	wacht_tijd2
;	bsr	reshowpicture
	move.l	(a7)+,a6
	tst.b	d0
	bne	exit_bpr1d0
	bra	rep_bpr1
exit_bpr1:
	moveq	#0,d0
	rts

exit_bpr1d0:
	moveq	#-1,d0
	rts

by_path_relative:
	move.l	db_wpatroonruimte(a3),a6
	addq.l	#4,a6
	moveq	#0,d0
	moveq	#0,d1
	move.w	(a6)+,d0		; start waarde
	move.w	(a6)+,d1
rep_r1:
	move.w	(a6)+,d2
	move.w	(a6)+,d3
	cmp.w	#PATHEND,d2
	beq	exit_r1
	add.w	d2,d0
	add.w	d3,d1
	movem.l	a6/d0/d1,-(a7)
	bsr	put_blitin_in_new
	bsr	wacht_tijd2
	movem.l	(a7)+,d0/d1/a6
	bne	exit_r1
	bra	rep_r1
exit_r1:
	move.l	db_blitin_ox(a3),d0
	move.l	db_blitin_y(a3),d1
	bsr	put_blitin_in_new
	rts

blitin_direct:
	moveq	#0,d7
	bsr	set_trip_need
	bsr	init_movement
	move.b	#0,db_no_trip_needed(a3)
	move.l	db_blitin_ox(a3),d0
	move.l	db_blitin_y(a3),d1

	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_viewblok(a3),a4
	move.l	vb_tempbuffer(a5),db_waardestpic(a3)
	bsr	put_blitin_in3		; zet de blitin neer
set_clipxy:
	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_fileblok(a3),a4
	move.l	db_blitin_ox(a3),d0
	move.l	db_blitin_y(a3),d1
	move.w	d0,vb_clip_x(a5)
	move.w	d1,vb_clip_y(a5)
	move.w	d0,vb_clip_x(a4)
	move.w	d1,vb_clip_y(a5)
	rts

blitin_blink_stick:
	move.l	#1,db_variation(a3)
	bra	blit_b1
	
blitin_blink:
	move.l	#0,db_variation(a3)
blit_b1:
	move.l	db_effect_speed_org(a3),d7
	moveq	#21,d6
	sub.w	d7,d6
	add.l	d6,d6
	move.l	d6,db_varwtime(a3)

	move.l	db_varltimes(a3),d7	; aantal keren blinken
	beq	no_subb
	subq.w	#1,d7
no_subb:	
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Start blink",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	d7,-(a7)

	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)
	jsr	adapt_blitin_exvert_moving	; voeg eventueel alleen planes toe
	bsr	maak_mask_blitin	
	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_viewblok(a3),a4
	bsr	copy_inactive_triple
	move.l	(a7)+,d7

	tst.w	d0
	bne	no_triple_blink
	move.l	db_triple_mem(a3),db_waardestpic(a3)
	lea	teller(pc),a0
	move.l	db_varwtime(a3),(a0)
	move.l	db_varwtime(a3),off_tel_start(a0)
	bsr	wacht_tijd2
	tst.b	d0
	bne	exit_bl1
	cmp.w	#8,d7
	bne	.no_easy_stop
	move.b	#$1,db_cycle_wait(a3)
.no_easy_stop:

	tst.b	db_double_buf(a3)
	bne	blink_double_buf

bl_blinkrep:
	move.l	d7,-(a7)

	move.l	db_blitin_ox(a3),d0
	move.l	db_blitin_y(a3),d1

	bsr	put_blitin_in3		; zet de blitin neer

	lea	teller(pc),a0
	move.l	db_varwtime(a3),(a0)

	bsr	wacht_tijd2
	bne	exit_bl1d7

	move.w	#$0faa,db_blitin_con0(a3)

	move.l	db_blitin_ox(a3),d0
	move.l	db_blitin_y(a3),d1
	bsr	put_blitin_in3		; zet de blitin neer
	move.w	#$0fca,db_blitin_con0(a3)

	lea	teller(pc),a0
	move.l	db_varwtime(a3),(a0)
	bsr	wacht_tijd2
	move.l	(a7)+,d7

	tst.b	d0
	bne	exit_bl1

	cmp.w	#8,d7
	beq	bl_blinkrep		; blink endless with var set to 10

	dbf	d7,bl_blinkrep
exit_bl1:
	cmp.l	#0,db_variation(a3)
	beq	no_triple_blink
	
	move.l	db_blitin_ox(a3),d0
	move.l	db_blitin_y(a3),d1
	bsr	put_blitin_in3		; zet de blitin neer

no_triple_blink:
	move.b	#0,db_cycle_wait(a3)
	rts

exit_bl1d7:
	move.l	(a7)+,d7
	bra	exit_bl1

*
* The buffers are large enought for DB so place on en off in a seperate buffer
*
blink_double_buf:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Blink double buf",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	db_inactive_viewblok(a3),a5
	move.l	vb_tempbuffer(a5),db_waarsourcepic(a3)
	move.l	d7,-(a7)
	move.l	db_blitin_ox(a3),d0
	move.l	db_blitin_y(a3),d1
	move.w	#$0fca,d6
	move.w	d6,db_blitin_con0(a3)
	bsr	put_blitin_in3		; zet de blitin neer

	move.l	db_inactive_viewblok(a3),db_rechts(a3)
	move.l	db_active_viewblok(a3),db_links(a3)

	lea	teller(pc),a0
	move.l	db_varwtime(a3),(a0)
	move.l	db_varwtime(a3),off_tel_start(a0)

	bsr	wacht_tijd2
	tst.b	d0
	bne	exit_bl1_DB1
	
; Be sure that the other is clear

	move.w	#$0faa,db_blitin_con0(a3)
	move.l	db_blitin_ox(a3),d0
	move.l	db_blitin_y(a3),d1
	bsr	put_blitin_in3		; zet de blitin neer
	move.w	#$0fca,db_blitin_con0(a3)

; since the clip is now on the screen the inactive holds the one without clip

	bra	DB_wait1

bl_blinkrep_DB:
	move.l	d7,-(a7)
	lea	teller(pc),a0
	move.l	db_varwtime(a3),(a0)
	bsr	wacht_tijd2
	tst.b	d0
	bne	exit_bl1_DB1
	bsr	showinactive2
DB_wait1:
	lea	teller(pc),a0
	move.l	db_varwtime(a3),(a0)
	bsr	wacht_tijd2
	move.l	(a7)+,d7

	tst.b	db_quit(a3)
	bne	exit_bl1_DB

	bsr	showinactive2

	cmp.w	#8,d7
	beq	bl_blinkrep_DB
	dbf	d7,bl_blinkrep_DB

	lea	teller(pc),a0
	move.l	db_varwtime(a3),(a0)
	bsr	wacht_tijd2

exit_bl1_DB:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Exit bl1_DB",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	cmp.l	#0,db_variation(a3)
	beq	no_triple_blink_DB
	move.l	db_rechts(a3),db_active_viewblok(a3)
	move.l	db_links(a3),db_inactive_viewblok(a3)
	lea	teller(pc),a0
	move.l	db_varwtime(a3),(a0)
	bsr	wacht_tijd2
	bra	DB_place
no_triple_blink_DB:
	move.l	db_rechts(a3),db_inactive_viewblok(a3)
	move.l	db_links(a3),db_active_viewblok(a3)
DB_place:
	bsr	showinactive2
	move.b	#0,db_cycle_wait(a3)
	rts

exit_bl1_DB1:
	addq	#4,a7
;	move.l	(a7)+,d7
	bra	exit_bl1_DB

set_single:
	move.l	db_active_viewblok(a3),a5
	move.l	vb_tempbuffer(a5),d0
	move.l	d0,db_waarsourcepic(a3)
	move.b	#$ff,db_please_no_double(a3)
	rts
*
* Create a vertikal lines effect on clips left to right
*
blitin_v1:
	bsr	init_h1h2h3h4
blitin_v1_in:
	bsr	set_single
	bsr	calc_effect_speed_blitin_ver
	move.l	db_effect_speed(a3),d4		; width line
	bne	oke_varl1
	moveq	#1,d4
oke_varl1:
	move.l	#$f,d4
	move.l	db_blitin_x(a3),d2		; dest x
	move.l	db_blitin_y(a3),d3		; dest y
	move.l	db_blitin_dx(a3),d0		; start value x
	moveq	#0,d5
	move.w	db_blitin_fx(a3),d5
	sub.l	d0,d5
;	sub.l	d0,d5
	moveq	#0,d1				; start value y
rep_b_v1:
	movem.l	d0-d5/a4/a5,-(a7)
	move.l	db_inactive_fileblok(a3),a4
	bsr	put_blitin_lijn		; zet de blitin neer
	bsr	wacht_tijd2
	movem.l	(a7)+,d0-d5/a4/a5
	bne	exit_blv1

	add.l	d4,d0
	add.l	d4,d2
	
	cmp.w	d5,d0
	ble	rep_b_v1

exit_blv1:
	bra	exit_blv2
*
* Create a vertikal lines effect on clips
* Right to left
*
blitin_v2:
	bsr	init_h1h2h3h4
	bsr	calc_effect_speed_blitin_ver
blitin_v2_in:
	bsr	set_single
	move.l	db_effect_speed(a3),d4		; width line
	bne	oke_varl2
	moveq	#1,d4
oke_varl2:
	moveq	#0,d0
	move.w	db_blitin_fx(a3),d0
	sub.l	db_blitin_dx(a3),d0		; start value x
;	sub.l	d4,d0
	move.l	db_blitin_ox(a3),d2		; dest x
	add.l	d0,d2
	moveq	#0,d1				; start value y
	move.l	db_blitin_y(a3),d3		; dest y
	move.l	db_blitin_dx(a3),d5
	sub.l	d4,d5
rep_b_v2:
	movem.l	d0-d5/a4/a5,-(a7)
	move.l	db_inactive_fileblok(a3),a4
	bsr	put_blitin_lijn		; zet de blitin neer
	bsr	wacht_tijd2
	movem.l	(a7)+,d0-d5/a4/a5
	bne	exit_blv1

;	bsr	reshowpicture		; DEBUGDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD

	sub.l	d4,d0
	sub.l	d4,d2
	bmi	exit_blv2
	cmp.w	d5,d0
	bge	rep_b_v2

exit_blv2:
	move.l	d0,-(a7)
	move.l	db_blitin_ox(a3),d0
	move.l	db_active_viewblok(a3),a5
	move.w	d0,vb_clip_x(a5)
	move.l	db_blitin_y(a3),d1
	bsr	put_blitin_in_new
	move.l	(a7)+,d0
	rts

blitin_v3:
	bsr	init_h1h2h3h4
blitin_v3_in:
	bsr	set_single
	bsr	calc_effect_speed_blitin_ver
	move.l	db_effect_speed(a3),d4		; width line
	bne	oke_varl3
	moveq	#1,d4
oke_varl3:
	move.l	db_blitin_x(a3),d2		; dest x
	move.l	db_blitin_y(a3),d3		; dest y
	move.w	db_blitin_fx(a3),d6
	move.l	d6,d5				; end value middle
	lsr.l	#1,d6
	sub.l	db_blitin_dx(a3),d5
	sub.l	db_blitin_dx(a3),d5
	lsr.l	#1,d5
	moveq	#0,d1				; start value y
	moveq	#0,d0
rep_b_v3:
	movem.l	d0-d6/a4/a5,-(a7)
	move.l	db_inactive_fileblok(a3),a4
	add.l	d6,d0
	move.l	db_blitin_ox(a3),d2		; dest x
	add.l	d0,d2
	ble	no_blitv3
	bsr	put_blitin_lijn		; zet de blitin neer
no_blitv3:
	movem.l	(a7)+,d0-d6/a4/a5

	movem.l	d0-d6/a4/a5,-(a7)
	move.l	db_inactive_fileblok(a3),a4
	exg	d6,d0
	sub.l	d6,d0
	move.l	db_blitin_ox(a3),d2		; dest x
	add.l	d0,d2
	ble	no_blitv4
	bsr	put_blitin_lijn		; zet de blitin neer
no_blitv4:
	bsr	wacht_tijd2
	movem.l	(a7)+,d0-d6/a4/a5
	bne	exit_blv3

	add.l	d4,d0
	cmp.w	d5,d0
	ble	rep_b_v3

exit_blv3:
	bra	exit_blv2

blitin_v4:
	bsr	init_h1h2h3h4
blitin_v4_in:
	bsr	set_single
	bsr	calc_effect_speed_blitin_ver
	move.l	db_effect_speed(a3),d4		; width line
	bne	oke_varl4
	moveq	#1,d4
oke_varl4:
	move.l	db_blitin_y(a3),d3		; dest y
	move.w	db_blitin_fx(a3),d6
	move.l	d6,d5				; end value middle
;	sub.l	db_blitin_dx(a3),d5
;	sub.l	db_blitin_dx(a3),d5
	lsr.l	#1,d5
;	add.l	d4,d5
	move.l	db_blitin_dx(a3),d0
	moveq	#0,d1				; start value y
rep_b_v4:
	movem.l	d0-d6/a4/a5,-(a7)
	move.l	db_inactive_fileblok(a3),a4
	move.l	db_blitin_ox(a3),d2		; dest x
	add.l	d0,d2
	bsr	put_blitin_lijn		; zet de blitin neer
	movem.l	(a7)+,d0-d6/a4/a5

	movem.l	d0-d6/a4/a5,-(a7)
	move.l	db_inactive_fileblok(a3),a4
	exg	d6,d0
	sub.l	d6,d0
	sub.l	d4,d0
	move.l	db_blitin_ox(a3),d2		; dest x
	add.l	d0,d2
	bsr	put_blitin_lijn		; zet de blitin neer
	bsr	wacht_tijd2
	movem.l	(a7)+,d0-d6/a4/a5
	bne	exit_blv4

	add.l	d4,d0
	cmp.w	d5,d0
	ble	rep_b_v4

exit_blv4:
	bra	exit_blv2

blitin_v6:
	bsr	init_h1h2h3h4
blitin_v6_in:
	bsr	set_single
	bsr	wacht_tijd2
	bsr	calc_effect_speed_blitin_ver1
	move.l	db_blitin_y(a3),d3		; dest y
	move.l	db_effect_speed(a3),d4		; width line
	move.l	d4,d7

	move.b	#$0,db_evendone(a3)
	moveq	#0,d6
	move.w	db_blitin_fx(a3),d6
	sub.l	db_blitin_dx(a3),d6
	sub.l	d7,d6
	move.l	d6,db_rechts(a3)
	neg.l	d7
	bra	rep_b_v5

blitin_v5:
	bsr	init_h1h2h3h4
blitin_v5_in:
	bsr	set_single
	bsr	wacht_tijd2
	bsr	calc_effect_speed_blitin_ver1
	move.l	db_blitin_y(a3),d3		; dest y
	move.l	db_effect_speed(a3),d4		; width line
	move.l	d4,d7

	move.b	#$0,db_evendone(a3)
	move.l	db_blitin_dx(a3),db_rechts(a3)
	moveq	#0,d6
	move.w	db_blitin_fx(a3),d6
	sub.l	db_blitin_dx(a3),d6
rep_b_v5:
	movem.l	d0-d7/a4/a5,-(a7)
	moveq	#0,d1
	move.l	db_rechts(a3),d0
	move.l	db_inactive_fileblok(a3),a4
	move.l	db_blitin_ox(a3),d2		; dest x
	add.l	d0,d2
	cmp.w	#2,d2
	bne	no_tt
;	moveq	#1,d2
no_tt:
	bsr	put_blitin_lijn		; zet de blitin neer
	movem.l	(a7)+,d0-d7/a4/a5

	move.l	d7,d0
	asl.w	#1,d0
	add.l	d0,db_rechts(a3)

	move.l	db_rechts(a3),d0
	cmp.w	d6,d0
	blt.b	goon_blitinv5_1

	tst.b	db_evendone(a3)
	bne	exit_blitin_v5

	sub.l	d7,db_rechts(a3)		; doe de oneven rijen
	neg.l	d7				; andere kant op
	move.b	#$ff,db_evendone(a3)
goon_blitinv5_1:
	cmp.l	db_blitin_dx(a3),d0
	bge	goon_blitinv5_2			; voor de neerwaartse bewegin
	tst.b	db_evendone(a3)
	bne	exit_blitin_v5

	move.l	d7,d0
	asl.w	#1,d0
	sub.l	d0,db_rechts(a3)

	neg.l	d7				; andere kant op
	add.l	d7,db_rechts(a3)			; doe de oneven rijen
	move.b	#$ff,db_evendone(a3)

goon_blitinv5_2:

	move.l	d6,-(a7)
	bsr	wacht_tijd
	move.l	(a7)+,d6
	
	tst.w	d0
	beq	rep_b_v5

exit_blitin_v5:
	bra	exit_blv2

blitin_v7:
	bsr	init_h1h2h3h4
blitin_v7_in:
	bsr	set_single
	move.l	db_inactive_fileblok(a3),a4
	bsr	wacht_tijd2
	bsr	calc_effect_speed_blitin_ver1

	moveq	#0,d0
	move.w	db_blitin_fx(a3),d0
	sub.l	db_blitin_dx(a3),d0
	sub.l	db_blitin_dx(a3),d0
	move.l	db_effect_speed(a3),d7
	bsr	check_even_oneven
	add.l	db_blitin_dx(a3),d0
	move.l	d0,db_rechts(a3)
	
	move.l	db_blitin_y(a3),d3		; dest y
	move.l	db_effect_speed(a3),d4		; width line

	move.b	#$0,db_evendone(a3)
	move.l	db_blitin_dx(a3),db_links(a3)

rep_b_v7:
	movem.l	a4/a5,-(a7)
	move.l	db_blitin_y(a3),d3		; dest y
	move.l	db_effect_speed(a3),d4		; width line
	moveq	#0,d1
	move.l	db_rechts(a3),d0
	move.l	db_blitin_ox(a3),d2		; dest x
	add.l	d0,d2
	bsr	put_blitin_lijn		; zet de blitin neer
	movem.l	(a7)+,a4/a5

	movem.l	a4/a5,-(a7)
	move.l	db_blitin_y(a3),d3		; dest y
	move.l	db_effect_speed(a3),d4		; width line
	moveq	#0,d1
	move.l	db_links(a3),d0
	move.l	db_blitin_ox(a3),d2		; dest x
	add.l	d0,d2
	bsr	put_blitin_lijn		; zet de blitin neer
	movem.l	(a7)+,a4/a5


	move.l	db_effect_speed(a3),d0
	add.l	d0,d0
	sub.l	d0,db_rechts(a3)
	add.l	d0,db_links(a3)

	move.l	db_rechts(a3),d0
	cmp.l	db_blitin_dx(a3),d0
	ble.b	exit_blitin_v7

	move.l	d6,-(a7)
	bsr	wacht_tijd
	move.l	(a7)+,d6
	
	tst.w	d0
	beq	rep_b_v7

exit_blitin_v7:
	bra	exit_blv2

*
* Try to draw a vertikal line with width's up to 16 bits
*
* d0,d1 : source x,y
* d2,d3 : dest x,y
* d4    : line width
*
put_blitin_lijn:
	and.w	#$f,d4			; no more than 16 bits
	move.w	d0,d5
	and.w	#$f,d5			; bit position source

	move.l	#$ffff,d7
	lsr.l	d5,d7
	move.l	d7,db_Afirstword(a3)
	
	move.l	d2,d6
	and.w	#$f,d6			; bit position dest
	cmp.w	d5,d6
	ble	use_desc_mode

; so here we need to shift right 0100 -> 0010

	move.w	d6,d7
	sub.w	d5,d6			; amount to shift
	move.l	d6,db_shiftsource(a3)

	add.w	d4,d5
	cmp.w	#15,d5
	ble	fits_in_one_word
	move.w	#2,db_tmodulo(a3)	; the source spreads over 2 words
	sub.w	#15,d5
	move.l	#$ffff0000,d7
	lsr.l	d5,d7
	move.l	d7,db_Alastword(a3)
	bra	added_word

fits_in_one_word:
	add.w	d4,d7
	move.w	#0,db_tmodulo(a3)
	cmp.w	#15,d7
	ble	no_add_word1
	move.w	#2,db_tmodulo(a3)
	sub.l	#16,d7
	move.l	#$ffff0000,d6
	lsr.l	d7,d6
	move.l	d6,db_Alastword(a3)

	move.l	d0,d5
	and.w	#$f,d5
	add.w	d4,d5
	cmp.w	#16,d5
	bge	no_and
	move.l	#$ffff0000,d6
	lsr.l	d5,d6
	move.l	db_Afirstword(a3),d5
	and.l	d6,d5
	and.l	d6,db_Afirstword(a3)
	move.l	#0,db_Alastword(a3)
no_and:
	bra	added_word

no_add_word1:
	move.l	d5,d7
	move.l	#$ffff0000,d6
	lsr.l	d7,d6
	move.l	d6,db_Alastword(a3)

added_word:

; calculate the address
	move.w	vb_leny(a4),d7
	mulu	vb_planes(a4),d7	; blit height

	move.l	vb_breedte_x(a4),d5
	move.l	d5,d4
	mulu	vb_planes(a4),d5
	mulu	d1,d5			; y word offset source

	move.l	d0,d6
	lsr.l	#4,d6
	add.l	d6,d6
	add.l	d6,d5			; total offset source

	move.l	db_blitinpic(a3),a1
	add.l	d5,a1			; source pointer

	move.l	db_blitinmask(a3),a6
	add.l	d5,a6			; mask pointer

	move.l	vb_breedte_x(a5),d5
	move.l	d5,d1
	mulu	vb_planes(a5),d5
	mulu	d3,d5			; y word offset dest

	move.l	d2,d6
	lsr.l	#4,d6
	add.l	d6,d6
	add.l	d6,d5			; total offset dest

	move.l	db_waarsourcepic(a3),a2
	add.l	d5,a2			; dest pointer

	move.l	db_waardestpic(a3),a4
	add.l	d5,a4

; calculate the modulo's

	moveq	#2,d2
	add.w	db_tmodulo(a3),d2	; width blit
	move.l	d4,d0
	sub.l	d2,d0			; source modulo
	sub.l	d2,d1			; dest modulo

	move.l	db_shiftsource(a3),d6
	swap	d6
	lsr.l	#4,d6
	move.w	d6,d4
;	or.w	#$fca,d6		; for now use normal copy
	or.w	db_blitin_con0(a3),d6

do_hard_blit:
	move.l	a6,a5
	move.l	db_graphbase(a3),a6
	jsr	_LVOOwnBlitter(a6)	; blitter multi-tasking ends here
	move.l	a5,a6

	lea	$dff000,a0
wbl77:	btst	#14,dmaconr(a0)		; wacht tot blitter vrij is
	bne.b	wbl77

	move.l	db_Afirstword(a3),d3
	move.w	d3,bltafwm(a0)
	move.l	db_Alastword(a3),d3
	move.w	d3,bltalwm(a0)

	move.w	d0,bltamod(a0)
	move.w	d0,bltbmod(a0)
	move.w	d1,bltcmod(a0)
	move.w	d1,bltdmod(a0)		; zet de modulo destination

	move.w	d6,bltcon0(a0)
	move.w	d4,bltcon1(a0)

	move.l	a6,bltapt(a0)
	move.l	a1,bltbpt(a0)
	move.l	a4,bltcpt(a0)
	move.l	a2,bltdpt(a0)

	cmp.b	#$ae,d6
	bne	no_rev1
	move.l	a2,bltbpt(a0)
	move.w	d1,bltbmod(a0)
	and.w	#$f,d4
	move.w	d4,bltcon1(a0)
no_rev1:

	lsr.w	#1,d2			; words voor breedte blit

	move.w	d7,bltsizv(a0)
	move.w	d2,bltsizh(a0)

	move.l	db_graphbase(a3),a6
	jsr	_LVODisownBlitter(a6)	; geef bliter weer vrij
	rts	


; so here we need to shift left 0100 -> 1000

use_desc_mode:
	move.w	#2,db_tmodulo(a3)
	move.w	d5,d7
	sub.w	d6,d5			; amount to shift
	move.l	d5,db_shiftsource(a3)
	add.w	d4,d7
	cmp.w	#16,d7
	ble	no_add_word2
	move.w	#4,db_tmodulo(a3)
	sub.l	#16,d7
no_add_word2:
	move.l	#$ffff0000,d6
	lsr.l	d7,d6
	move.l	db_Afirstword(a3),db_Alastword(a3)
	move.l	d6,db_Afirstword(a3)

; calculate the address

	move.w	vb_leny(a4),d7
;	addq.w	#1,d7
	mulu	vb_planes(a4),d7	; blit height
	addq.w	#1,d7


	move.l	vb_breedte_x(a4),d5
	move.l	d5,d4
	mulu	vb_planes(a4),d5

	add.w	vb_leny(a4),d1
	mulu	d1,d5			; y word offset source

	move.l	d0,d6
	lsr.l	#4,d6
	add.l	d6,d6
	add.l	d6,d5			; total offset source
	subq.l	#2,d5
	moveq	#0,d6
	move.w	db_tmodulo(a3),d6
	add.l	d6,d5
	
	move.l	db_blitinpic(a3),a1
	add.l	d5,a1			; source pointer

	move.l	db_blitinmask(a3),a6
	add.l	d5,a6			; mask pointer

	move.l	vb_breedte_x(a5),d5
	move.l	d5,d1
	mulu	vb_planes(a5),d5
	add.w	vb_leny(a4),d3
	mulu	d3,d5			; y word offset dest

	move.l	d2,d6
	lsr.l	#4,d6
	add.l	d6,d6
	add.l	d6,d5			; total offset dest

	subq.l	#2,d5
	moveq	#0,d6
	move.w	db_tmodulo(a3),d6
	add.l	d6,d5

	move.l	db_waarsourcepic(a3),a2
	add.l	d5,a2			; dest pointer

	move.l	db_waardestpic(a3),a4
	add.l	d5,a4

; calculate the modulo's

	moveq	#0,d2
	move.w	db_tmodulo(a3),d2	; width blit

	move.l	d4,d0
	sub.l	d2,d0			; source modulo
	sub.l	d2,d1			; dest modulo

	move.l	db_shiftsource(a3),d6
	swap	d6
	lsr.l	#4,d6
	move.w	d6,d4
	or.w	db_blitin_con0(a3),d6
	or.w	#2,d4			; descending mode
	bra	do_hard_blit

set_trip_need:
	move.b	#0,db_no_trip_needed(a3)
	move.l	db_effect_data(a3),a0
	cmp.l	#-1,eff_outnum(a0)
	bne	yes_out1
	move.b	#$ff,db_no_trip_needed(a3)
yes_out1:
	rts

init_h1h2h3h4:
	bsr	set_trip_need
	bsr	init_movement
	move.b	#0,db_no_trip_needed(a3)

	move.b	#$ff,db_please_no_double(a3)
	move.l	vb_tempbuffer(a5),db_waardestpic(a3)
	move.l	db_blitin_ox(a3),d0
	move.l	db_blitin_y(a3),d1
	moveq	#0,d5
	move.l	db_effect_speed(a3),d3
	rts

blitin_blinds1:
	lea	blinds_sub1(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_blinds2:
	lea	blinds_sub2(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_random_rows:
	lea	random_rows(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_torn_down:
	lea	torn_down(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_torn_up:
	lea	torn_up(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_roll_down:
	lea	roll_down(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_roll_up:
	lea	roll_up(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_blinds_vertical:
	lea	blinds_vertical(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_random_columns:
	lea	random_columns(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_climb_bars_1:
	lea	climb_bars_1(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_climb_bars_2:
	lea	climb_bars_2(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_climb_bars_3:
	lea	climb_bars_3(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_climb_bars_4:
	lea	climb_bars_4(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_dissolve:
	lea	test_pixel1(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_blockvar:
	lea	blockvar(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_blocks1:
	lea	blocks1(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts
blitin_blocks2:
	lea	blocks2(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts
blitin_blocks3:
	lea	blocks3(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts
blitin_blocks4:
	lea	blocks4(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts
blitin_blocks5:
	lea	blocks5(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts
blitin_blocks6:
	lea	blocks6(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts
blitin_blocks7:
	lea	blocks7(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts
blitin_blocks8:
	lea	blocks8(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts
blitin_blocks9:
	lea	blocks9(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts
blitin_blocks10:
	lea	blocks10(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts
blitin_blocks11:
	lea	blocks11(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_mozaik_down:
	lea	mozaik_down,a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_mozaik_up:
	lea	mozaik_up,a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_page_fold1:
	lea	page_fold1,a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_corner_in_top_left:
	lea	corner_in_top_left(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts
blitin_corner_in_top_right:
	lea	corner_in_top_right(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts
blitin_corner_in_bottom_right:
	lea	corner_in_bottom_right(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts
blitin_corner_in_bottom_left:
	lea	corner_in_bottom_left(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_wipe_to_centre:
	lea	wipe_to_centre(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_wipe_from_centre:
	lea	wipe_from_centre(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_wipe_from_centre2:
	lea	wipe_from_centre2(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts
blitin_wipe_to_centre2:
	lea	wipe_to_centre2(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_wipe_from_centre3:
	lea	wipe_from_centre3(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts
blitin_wipe_to_centre3:
	lea	wipe_to_centre3(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

blitin_diagonal1:
	lea	diagonal1(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts
blitin_cross1:
	lea	cross1(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts
blitin_cross2:
	lea	cross2(pc),a0
	move.l	a0,db_blitin_func(a3)
	bsr	blitin_test1
	rts

*
* Try a blitin effect with the aid of the screen effects ??
*
blitin_test1:
	tst.b	db_reverse_effect(a3)
	bne	blitin_test2

	bsr	init_h1h2h3h4

	tst.b	db_double_buf(a3)
	beq	.no_doub
	
	move.l	db_waarsourcepic(a3),d6
	move.l	db_waardestpic(a3),d7
	movem.l	d6/d7,-(a7)

	move.l	db_inactive_fileblok(a3),a4
	move.l	db_inactive_viewblok(a3),a5
	move.l	vb_tempbuffer(a5),db_waarsourcepic(a3)

	move.l	db_blitin_ox(a3),d0
	move.l	db_blitin_y(a3),d1

	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_viewblok(a3),a5
	move.l	vb_tempbuffer(a5),db_waardestpic(a3)

	bsr	put_blitin_in3			; zet de blitin neer

	move.l	db_inactive_viewblok(a3),a4
	move.l	db_active_viewblok(a3),a5

	move.l	vb_tempbuffer(a4),d0
	move.l	vb_tempbuffer(a5),d1
	move.w	vb_leny(a5),d2
	move.w	vb_lenx(a5),d3
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	db_blitin_y(a3),d1
	bpl	.okie2
	moveq	#0,d1
.okie2:
	move.l	vb_breedte_x(a5),d0
	mulu	vb_planes(a5),d0
	mulu	d1,d0
	move.l	db_blitin_ox(a3),d1
	add.l	#32,d1
	bmi	.okie
	lsr.l	#4,d1
	add.l	d1,d1
	add.l	d1,d0
;	add.l	#4,d0
.okie:
	add.l	d0,vb_tempbuffer(a4)
	add.l	d0,vb_tempbuffer(a5)

	move.l	db_active_viewblok(a3),a4
	bsr	calc_fade_colors_no_a4
	move.l	db_inactive_viewblok(a3),a4
	move.w	db_blitin_fx(a3),d3
	move.w	db_blitin_fy(a3),d2

	sub.l	#64,d3

	move.l	db_blitin_ox(a3),d1
	add.l	#32,d1

	move.l	d3,d4
	add.l	d1,d4				; final pixel blitin

	lsr.w	#4,d1
	add.w	#15,d4
	lsr.w	#4,d4
	sub.w	d1,d4
	lsl.w	#4,d4
	move.l	d4,d3

;	add.w	d1,d3

	move.w	d2,vb_leny(a5)
	move.w	d3,vb_lenx(a5)

	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)
	
	move.l	db_blitin_func(a3),a0
	jsr	(a0)

	movem.l	(a7)+,d0-d7/a0-a6

	move.l	d0,vb_tempbuffer(a4)
	move.l	d1,vb_tempbuffer(a5)
	move.w	d2,vb_leny(a5)
	move.w	d3,vb_lenx(a5)
	movem.l	(a7)+,d6/d7

	move.l	d6,db_waarsourcepic(a3)
	move.l	d7,db_waardestpic(a3)
;.no_doub:
	move.l	db_blitin_ox(a3),d0
	move.l	db_blitin_y(a3),d1
	clr.w	vb_clip_width(a5)
	clr.w	vb_clip_width(a4)
;uip:
	bsr	put_blitin_in3			; zet de blitin neer

	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)

	move.b	#$0,db_please_no_double(a3)

	move.l	db_inactive_viewblok(a3),a4
	move.w	vb_clip_x(a5),vb_clip_x(a4)
	move.w	vb_clip_y(a5),vb_clip_y(a4)

	move.w	vb_clip_width(a5),d0
	move.w	d0,vb_clip_width(a4)

	move.l	db_effect_data(a3),a0
	cmp.l	#-1,eff_outnum(a0)
	beq	.nout

	bsr	set_clipxy

	bsr	showinactive2

;	move.l	db_inactive_viewblok(a3),d0
;	move.l	db_active_viewblok(a3),db_inactive_viewblok(a3)
;	move.l	d0,db_active_viewblok(a3)

	move.l	db_inactive_viewblok(a3),a5
	move.l	vb_tempbuffer(a5),db_waarsourcepic(a3)
.nout:
	rts

.no_doub:
	move.l	db_blitin_ox(a3),d0
	move.l	db_blitin_y(a3),d1
	clr.w	vb_clip_width(a5)
	clr.w	vb_clip_width(a4)
	bsr	put_blitin_in3			; zet de blitin neer
	rts

blitin_test2:
	tst.b	db_double_buf(a3)
	beq	.no_doub

	move.w	#$0fae,db_blitin_con0(a3)

	move.l	db_inactive_viewblok(a3),a5
	move.l	vb_breedte_x(a5),d0
	mulu	vb_planes(a5),d0
	mulu	vb_leny(a5),d0
	lsr.l	#2,d0				; longs ?
	move.l	vb_tempbuffer(a5),a0
	move.l	db_triple_mem(a3),a1
.rep_c_it:
	move.l	(a1)+,(a0)+
	subq.l	#1,d0
	bpl	.rep_c_it

	move.l	db_waarsourcepic(a3),d6
	move.l	db_waardestpic(a3),d7
	movem.l	d6/d7,-(a7)

	move.l	db_inactive_fileblok(a3),a4
	move.l	db_inactive_viewblok(a3),a5
	move.l	vb_tempbuffer(a5),db_waarsourcepic(a3)

	move.l	db_blitin_ox(a3),d0
	move.l	db_blitin_y(a3),d1

	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_viewblok(a3),a5
	move.l	vb_tempbuffer(a5),db_waardestpic(a3)

;	bsr	put_blitin_in3			; zet de blitin neer

	move.l	db_inactive_viewblok(a3),a4
	move.l	db_active_viewblok(a3),a5

	move.l	vb_tempbuffer(a4),d0
	move.l	vb_tempbuffer(a5),d1
	move.w	vb_leny(a5),d2
	move.w	vb_lenx(a5),d3
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	db_blitin_y(a3),d1
	bpl	.okie2
	moveq	#0,d1
.okie2:
	move.l	vb_breedte_x(a5),d0
	mulu	vb_planes(a5),d0
	mulu	d1,d0
	move.l	db_blitin_ox(a3),d1
	add.l	#32,d1
	bmi	.okie
	lsr.l	#4,d1
	add.l	d1,d1
	add.l	d1,d0
;	add.l	#4,d0
.okie:
	add.l	d0,vb_tempbuffer(a4)
	add.l	d0,vb_tempbuffer(a5)

	move.l	db_active_viewblok(a3),a4
	bsr	calc_fade_colors_no_a4
	move.l	db_inactive_viewblok(a3),a4
	move.w	db_blitin_fx(a3),d3
	move.w	db_blitin_fy(a3),d2

	sub.l	#64,d3

	move.l	db_blitin_ox(a3),d1
	add.l	#32,d1

	move.l	d3,d4
	add.l	d1,d4				; final pixel blitin

	lsr.w	#4,d1
	add.w	#15,d4
	lsr.w	#4,d4
	sub.w	d1,d4
	lsl.w	#4,d4
	move.l	d4,d3

;	add.w	d1,d3

	move.w	d2,vb_leny(a5)
	move.w	d3,vb_lenx(a5)

	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)
	
	move.l	db_blitin_func(a3),a0
	jsr	(a0)

	movem.l	(a7)+,d0-d7/a0-a6

	move.l	d0,vb_tempbuffer(a4)
	move.l	d1,vb_tempbuffer(a5)
	move.w	d2,vb_leny(a5)
	move.w	d3,vb_lenx(a5)
	movem.l	(a7)+,d6/d7

	move.l	d6,db_waarsourcepic(a3)
	move.l	d7,db_waardestpic(a3)
	move.l	db_blitin_ox(a3),d0
	move.l	db_blitin_y(a3),d1
	clr.w	vb_clip_width(a5)
	clr.w	vb_clip_width(a4)

;	bsr	put_blitin_in3			; zet de blitin neer

	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)
;.nono:
;	btst	#10,$dff016
;	bne	.nono

	move.b	#$0,db_please_no_double(a3)
	move.l	db_inactive_viewblok(a3),a4
	move.w	vb_clip_x(a5),vb_clip_x(a4)
	move.w	vb_clip_y(a5),vb_clip_y(a4)

	move.w	#$0fca,db_blitin_con0(a3)
	rts
.no_doub:
	bra	blitin_direct_rev

blitin_h1:
	bsr	init_h1h2h3h4
	move.l	db_inactive_fileblok(a3),a4
	move.w	vb_leny(a4),d6
blitin_h1_in:
rep_b_h1:
	movem.l	d0-d6/a4/a5,-(a7)
	move.l	db_inactive_fileblok(a3),a4
	bsr	put_blitin_in2		; zet de blitin neer
	bsr	wacht_tijd2
	movem.l	(a7)+,d0-d6/a4/a5
	bne	exit_blh1

	add.l	db_effect_speed(a3),d5
	add.l	db_effect_speed(a3),d1

	move.w	d5,d4
	add.w	d3,d4
	cmp.w	d6,d4
	ble	.no_min
	move.l	db_effect_speed(a3),d3	; save never zero
	sub.w	d6,d4
	sub.w	d4,d3
.no_min:
	cmp.w	db_blitin_fy(a3),d5
	ble	rep_b_h1

exit_blh1:
	moveq	#0,d5
	move.w	db_blitin_fy(a3),d5
	move.l	db_blitin_y(a3),d1
	bsr	put_blitin_in2		; zet de blitin neer
	rts
*
* Init the different variables used to flip something
* In d0 the number of different frames
* In d1 the height of the blitin
*
init_flip_angles:
	move.l	d1,d6
	move.l	db_mathffpbase(a3),a6
	jsr	_LVOSPFlt(a6)
	move.l	d0,d7
	move.l	d6,d0
	jsr	_LVOSPFlt(a6)
	move.l	d0,db_radiusx(a3)	
	moveq	#0,d0
	jsr	_LVOSPFlt(a6)
	move.l	d0,db_temphoek(a3)	; start value
	move.l	db_mathtransbase(a3),a6
	jsr	_LVOSPAcos(a6)		; d0 is .5 * pi
	move.l	d7,d1
	move.l	db_mathffpbase(a3),a6
	jsr	_LVOSPDiv(a6)
	move.l	d0,db_deltahoek(a3)	; delta value to add to the angle
	rts
*
* Set the start angle at .5 * pi
*
init_flip_sub:
	move.l	db_mathffpbase(a3),a6
	moveq	#0,d0
	jsr	_LVOSPFlt(a6)
	move.l	db_mathtransbase(a3),a6
	jsr	_LVOSPAcos(a6)		; d0 is .5 * pi
	move.l	d0,db_temphoek(a3)
	rts

*
* Neg the increment value
*
init_flip_add:
	move.l	db_mathffpbase(a3),a6
	move.l	db_deltahoek(a3),d0
	jsr	_LVOSPNeg(a6)
	move.l	d0,db_deltahoek(a3)
	rts

get_sinus:
	move.l	d7,-(a7)
	move.l	db_mathffpbase(a3),a6
	move.l	db_temphoek(a3),d0
	move.l	db_deltahoek(a3),d1
	jsr	_LVOSPAdd(a6)		; set to next angle
	move.l	d0,db_temphoek(a3)
	move.l	db_mathtransbase(a3),a6
	jsr	_LVOSPSin(a6)
	move.l	db_radiusx(a3),d1
	move.l	db_mathffpbase(a3),a6
	jsr	_LVOSPMul(a6)
	jsr	_LVOSPFix(a6)
	move.l	(a7)+,d7
	rts

*
* Return the max value for a flip and add up the angle
* It uses the value radiusx as the height
*
get_flip_start_sub:
	move.l	d7,-(a7)
	move.l	db_mathffpbase(a3),a6
	move.l	db_temphoek(a3),d0
	move.l	db_deltahoek(a3),d1
	jsr	_LVOSPSub(a6)
	bra	get_flip_start_in

get_flip_start:
	move.l	d7,-(a7)
	move.l	db_mathffpbase(a3),a6
	move.l	db_temphoek(a3),d0
	move.l	db_deltahoek(a3),d1
	jsr	_LVOSPAdd(a6)

get_flip_start_in:
	move.l	d0,db_temphoek(a3)
	move.l	db_mathtransbase(a3),a6
	jsr	_LVOSPCos(a6)
	move.l	d0,db_radiusy(a3)		; store the cosinus
	move.l	d0,d7
	moveq	#1,d0
	move.l	db_mathffpbase(a3),a6
	jsr	_LVOSPFlt(a6)
	move.l	d7,d1
	tst.l	d0
	beq	no_div_flip
	jsr	_LVOSPDiv(a6)
	move.l	d0,db_temphoeky(a3)		; store 1 / cos( a )
no_div_flip:
	move.l	db_radiusy(a3),d0
	move.l	db_radiusx(a3),d1
	move.l	db_mathffpbase(a3),a6
	jsr	_LVOSPMul(a6)
	jsr	_LVOSPFix(a6)
	move.l	(a7)+,d7
	rts
*
* Give the projected value with d0
*
get_flip_proj:
	move.l	db_mathffpbase(a3),a6
	jsr	_LVOSPFlt(a6)
	move.l	db_temphoeky(a3),d1
	jsr	_LVOSPMul(a6)
	move.l	d0,d2
	jsr	_LVOSPFix(a6)
	rts

	IFNE 0
test_num:
	movem.l	d0/d1/d7/a6,-(a7)
;	move.l	d0,d7
;	move.l	db_mathffpbase(a3),a6
;	move.l	#1000,d0
;	jsr	_LVOSPFlt(a6)
;	move.l	d7,d1
;	jsr	_LVOSPMul(a6)
	jsr	_LVOSPFix(a6)
	movem.l	(a7)+,d0/d1/d7/a6
	rts

test_num2:
	movem.l	d0/d1/d7/a6,-(a7)
	move.l	d1,d7
	move.l	db_mathffpbase(a3),a6
	move.l	#1000,d0
	jsr	_LVOSPFlt(a6)
	move.l	d7,d1
	jsr	_LVOSPMul(a6)
	jsr	_LVOSPFix(a6)
	movem.l	(a7)+,d0/d1/d7/a6
	rts
	ENDC
	
init_flip:
	bsr	adapt_blitin_exvert_moving
	bsr	maak_mask_blitin	
init_flip_rev:
	move.l	db_active_viewblok(a3),a5
	move.l	vb_tempbuffer(a5),d0
	move.l	d0,db_waarsourcepic(a3)
	move.b	#$ff,db_please_no_double(a3)
	lea	tellerstart(pc),a0
	move.l	#0,(a0)
	move.l	#0,-4(a0)
	moveq	#21,d0
	sub.l	db_effect_speed_org(a3),d0
	lsl.l	#4,d0
	move.l	d0,db_effect_speed(a3)
	bsr	set_blit_fast_direct
	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_viewblok(a3),a4
	move.l	db_blitin_ox(a3),d0
	move.l	db_blitin_y(a3),d1
	moveq	#0,d5
	bsr	copy_trip
	rts

copy_trip:
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	db_active_viewblok(a3),a5
	bsr	copy_inactive_triple
	tst.w	d0
	bne	no_triple_flip1
	move.l	db_triple_mem(a3),db_waardestpic(a3)
no_triple_flip1:
	movem.l	(a7)+,d0-d7/a0-a6
	rts

clear_flip_store:
	move.l	db_wstore_moves2(a3),a2
	moveq	#-1,d1
	swap	d1
	move.w	#-1,d1
	move.l	#380,d0
cl_fl:	move.l	d1,(a2)+
	dbf	d0,cl_fl
	rts

flip_blitin1_rev:
	move.b	#$ff,db_reverse(a3)
	bsr	init_flip_rev
	moveq	#0,d0
	move.w	db_blitin_fy(a3),d0
	move.l	d0,db_links(a3)
	move.l	db_effect_speed(a3),d0
	moveq	#0,d1
	move.w	db_blitin_fy(a3),d1
	bsr	init_flip_angles
	bsr	init_flip_add
	bra	flip_blitin1_con
*
* Try to create a flip coin effect
*
flip_blitin1:
	move.b	#$0,db_reverse(a3)
	bsr	init_flip

	bsr	clear_flip_store
	moveq	#0,d1
	move.l	db_effect_speed(a3),d0
	move.w	db_blitin_fy(a3),d1
	bsr	init_flip_angles
	bsr	init_flip_sub
	move.l	#0,db_links(a3)			; remove check
flip_blitin1_con:
	move.l	db_inactive_fileblok(a3),a4
	move.l	db_active_viewblok(a3),a5
	move.l	db_wstore_moves2(a3),a2

	move.l	db_effect_speed(a3),d7
	subq.l	#2,d7
rep_flip_angle:
	bsr	get_flip_start_sub
	move.l	d0,db_rechts(a3)
	move.l	d0,d6
	moveq	#0,d5
	bra	in_rep_flip_inc
rep_flip_inc:
	move.l	d5,d0
	bsr	get_flip_proj
	movem.l	d0/d5/d6/d7/a2/a4/a5,-(a7)
	move.l	d0,d1
	exg	d1,d5
	moveq	#1,d3
	move.l	db_blitin_ox(a3),d0
	add.l	db_blitin_y(a3),d1
	moveq	#0,d4
	move.w	d1,d6
	add.w	d6,d6
	cmp.w	0(a2,d6.w),d5
	beq	no_put_flip1
	move.w	d5,0(a2,d6.w)
	bsr	put_blitin_in2
no_put_flip1:
	movem.l	(a7)+,d0/d5/d6/d7/a2/a4/a5
	addq.l	#1,d5
in_rep_flip_inc:
	dbf	d6,rep_flip_inc
	move.l	db_rechts(a3),d0
	cmp.l	db_links(a3),d0
	bge	no_remove_flip1
	movem.l	d0/d5/d6/d7/a2/a4/a5,-(a7)

	move.l	db_links(a3),d6
	move.l	d6,d1
	sub.l	d0,d1
	bne	no_zer1

	moveq	#1,d1
no_zer1:
	subq.l	#1,d6
	addq.l	#1,d1
	add.l	db_blitin_y(a3),d6
	move.l	d0,db_links(a3)
	bsr	restore_pic_xy
	movem.l	(a7)+,d0/d5/d6/d7/a2/a4/a5
no_remove_flip1:
	bsr	wacht_tijd2
	bne	exit_flip_blitin
	dbf	d7,rep_flip_angle
exit_flip_blitin:
	tst.b	db_reverse(a3)
	beq	exit_h2m1

	move.l	db_blitin_y(a3),d6
	move.w	db_blitin_fy(a3),d1
	bsr	restore_pic_xy
	rts

flip_blitin2_rev:
	move.b	#$ff,db_reverse(a3)
	bsr	init_flip_rev
	moveq	#0,d0
	move.w	db_blitin_fy(a3),d0
;	subq.w	#1,d0
	move.l	d0,db_links(a3)
	move.l	db_effect_speed(a3),d0
	moveq	#0,d1
	move.w	db_blitin_fy(a3),d1
	bsr	init_flip_angles
	bsr	init_flip_add
	bra	flip_blitin2_con

flip_blitin2:
	move.b	#$0,db_reverse(a3)
	bsr	init_flip
	bsr	clear_flip_store
	moveq	#0,d1
	move.l	db_effect_speed(a3),d0
	move.w	db_blitin_fy(a3),d1
	bsr	init_flip_angles
	bsr	init_flip_sub
	move.l	#0,db_links(a3)			; remove check
flip_blitin2_con:
	move.l	db_inactive_fileblok(a3),a4
	move.l	db_active_viewblok(a3),a5
	move.l	db_wstore_moves2(a3),a2

	move.l	db_effect_speed(a3),d7
	subq.l	#2,d7
rep_flip_angle2:
	bsr	get_flip_start_sub
	move.l	d0,db_rechts(a3)
	move.l	d0,d6
	moveq	#0,d5
rep_flip_inc2:
	move.l	d5,d0
	bsr	get_flip_proj
	movem.l	d0/d5/d6/d7/a2/a4/a5,-(a7)
	moveq	#0,d1
	move.w	db_blitin_fy(a3),d1
	move.l	d1,d2
	sub.l	d5,d2
	move.l	d2,d5
	sub.l	d0,d1
	exg	d1,d5
	moveq	#1,d3
	subq.l	#1,d5
	subq.l	#1,d1
	move.l	db_blitin_ox(a3),d0
	add.l	db_blitin_y(a3),d1
	moveq	#0,d4
	move.w	d1,d6
	add.w	d6,d6
	cmp.w	0(a2,d6.w),d5
	beq	no_put_flip2
	move.w	d5,0(a2,d6.w)
	bsr	put_blitin_in2
no_put_flip2:
	movem.l	(a7)+,d0/d5/d6/d7/a2/a4/a5
	addq.l	#1,d5
in_rep_flip_inc2:
	dbf	d6,rep_flip_inc2

	move.l	db_rechts(a3),d0
	cmp.l	db_links(a3),d0
	bge	no_remove_flip2

	movem.l	d0/d5/d6/d7/a2/a4/a5,-(a7)
	move.l	db_links(a3),d1
	moveq	#0,d2
	move.w	db_blitin_fy(a3),d2
	move.l	d0,db_links(a3)
	sub.l	d1,d2
	sub.l	d0,d1
	move.l	d2,d6
	subq.l	#1,d6
	add.l	db_blitin_y(a3),d6
	move.l	d0,db_links(a3)
	bsr	restore_pic_xy
	movem.l	(a7)+,d0/d5/d6/d7/a2/a4/a5
no_remove_flip2:
	bsr	wacht_tijd2
	bne	exit_flip_blitin
	dbf	d7,rep_flip_angle2

exit_flip_blitin2:
	tst.b	db_reverse(a3)
	beq	exit_h2m1

	move.l	db_blitin_y(a3),d6
	move.w	db_blitin_fy(a3),d1
	bsr	restore_pic_xy
	rts

div_speed:
	move.l	db_effect_speed(a3),d0
	lsr.l	#1,d0
	move.l	d0,db_effect_speed(a3)
	rts

flip_blitin4_rev:
	move.b	#$ff,db_reverse(a3)

	move.l	db_varltimes(a3),d7
	subq.l	#1,d7
	bra	rep_flip4

flip_blitin4:

	move.b	#$0,db_reverse(a3)
	move.l	db_varltimes(a3),d7
	subq.l	#1,d7
	move.l	d7,-(a7)
	bra	start_flip

rep_flip4:
	move.l	d7,-(a7)

	bsr	init_flip_rev
	bsr	div_speed

	moveq	#0,d0
	move.w	db_blitin_fy(a3),d0
	move.l	d0,db_links(a3)
	move.l	db_effect_speed(a3),d0

	move.w	db_blitin_fy(a3),d1
	bsr	init_flip_angles
	bsr	init_flip_add
	move.l	db_inactive_fileblok(a3),a4
	move.l	db_active_viewblok(a3),a5
	bsr	clear_flip_store
	bsr	normal_rotate
	tst.b	db_quit(a3)
	bne	quit_flip

start_flip:
	bsr	init_flip
	bsr	div_speed

	move.l	db_effect_speed(a3),d0
	moveq	#0,d1
	move.w	db_blitin_fy(a3),d1
	bsr	init_flip_angles
	bsr	init_flip_sub
	move.l	#0,db_links(a3)			; remove check
	move.l	db_inactive_fileblok(a3),a4
	move.l	db_active_viewblok(a3),a5
	bsr	clear_flip_store
	bsr	inverse_rotate
	tst.b	db_quit(a3)
	bne	quit_flip

	bsr	init_flip_rev
	bsr	div_speed
	moveq	#0,d0
	move.w	db_blitin_fy(a3),d0
	move.l	d0,db_links(a3)
	move.l	db_effect_speed(a3),d0
	move.w	db_blitin_fy(a3),d1

	bsr	init_flip_angles
	bsr	init_flip_add
	bsr	init_flip
	bsr	div_speed
	move.l	db_inactive_fileblok(a3),a4
	move.l	db_active_viewblok(a3),a5
	bsr	clear_flip_store
	bsr	inverse_rotate
	tst.b	db_quit(a3)
	bne	quit_flip
	move.l	db_effect_speed(a3),d0
	moveq	#0,d1
	move.w	db_blitin_fy(a3),d1
	bsr	init_flip_angles
	bsr	init_flip_sub
	move.l	#0,db_links(a3)
	move.l	db_inactive_fileblok(a3),a4
	move.l	db_active_viewblok(a3),a5
	bsr	clear_flip_store
	bsr	normal_rotate
	tst.b	db_quit(a3)
	bne	quit_flip
	move.l	(a7)+,d7
	cmp.w	#8,d7
	beq	rep_flip4			; flip endless
	dbf	d7,rep_flip4

	tst.b	db_reverse(a3)
	beq	no_remove_flip4

	bsr	init_flip_rev
	bsr	div_speed
	moveq	#0,d0
	move.w	db_blitin_fy(a3),d0
	move.l	d0,db_links(a3)
	move.l	db_effect_speed(a3),d0

	move.w	db_blitin_fy(a3),d1
	bsr	init_flip_angles
	bsr	init_flip_add
	move.l	db_inactive_fileblok(a3),a4
	move.l	db_active_viewblok(a3),a5
	bsr	clear_flip_store
	bsr	normal_rotate
	bra	exit_flip_with_remove
no_remove_flip4:

	rts

quit_flip:
	move.l	(a7)+,d7
	rts
	
flip_blitin3_rev:
	cmp.l	#0,db_varltimes(a3)
	bgt	flip_blitin4_rev
	move.b	#$ff,db_reverse(a3)
	bsr	init_flip_rev
	moveq	#0,d0
	move.w	db_blitin_fy(a3),d0
	move.l	d0,db_links(a3)
	move.l	db_effect_speed(a3),d0

	move.w	db_blitin_fy(a3),d1
	bsr	init_flip_angles
	bsr	init_flip_add
	bra	flip_blitin3_con
	
flip_blitin3:
	cmp.l	#0,db_varltimes(a3)
	bgt	flip_blitin4
	move.b	#$0,db_reverse(a3)
	bsr	init_flip

	bsr	clear_flip_store
	move.l	db_effect_speed(a3),d0
	moveq	#0,d1
	move.w	db_blitin_fy(a3),d1
;	lsr.l	#1,d1
	bsr	init_flip_angles
	bsr	init_flip_sub
	move.l	#0,db_links(a3)			; remove check
flip_blitin3_con:
	move.l	db_inactive_fileblok(a3),a4
	move.l	db_active_viewblok(a3),a5

	bsr	normal_rotate
	
	tst.b	db_reverse(a3)
	beq	exit_h2m1

exit_flip_with_remove:
	move.l	db_blitin_y(a3),d6
	move.w	db_blitin_fy(a3),d1
	bsr	restore_pic_xy
	rts

normal_rotate:
	move.l	db_wstore_moves2(a3),a2
	moveq	#0,d2
	move.w	db_blitin_fy(a3),d2
	lsr.l	#1,d2
	move.l	d2,db_shiftit(a3)

	move.l	db_effect_speed(a3),d7
	subq.l	#2,d7
rep_flip_angle3:
	bsr	get_flip_start_sub
	move.l	d0,db_rechts(a3)
	move.l	d0,d6
	move.l	d6,d5
	lsr.l	#1,d5
	neg.l	d5
;	bra	in_rep_flip_inc3
rep_flip_inc3:
	move.l	d5,d0
	bsr	get_flip_proj
	movem.l	d0/d5/d6/d7/a2/a4/a5,-(a7)
	move.l	d0,d1
	exg	d1,d5
	add.l	db_shiftit(a3),d5
	moveq	#1,d3
	move.l	db_blitin_ox(a3),d0
	add.l	db_blitin_y(a3),d1
	add.l	db_shiftit(a3),d1
	moveq	#0,d4
	move.w	d1,d6
	add.w	d6,d6
	cmp.w	0(a2,d6.w),d5
	beq	no_put_flip3
	move.w	d5,0(a2,d6.w)
	bsr	put_blitin_in2
no_put_flip3:
	movem.l	(a7)+,d0/d5/d6/d7/a2/a4/a5
	addq.l	#1,d5
in_rep_flip_inc3:
	dbf	d6,rep_flip_inc3
	move.l	db_rechts(a3),d0
	cmp.l	db_links(a3),d0
	bge	no_remove_flip3

	bsr	restore_flip
	
no_remove_flip3:
	bsr	wacht_tijd2
	bne	exit_flip_blitin3
	dbf	d7,rep_flip_angle3
exit_flip_blitin3:
	rts

restore_flip:
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	db_links(a3),d6
	addq.l	#1,d6
	move.l	d6,d1
	sub.l	d0,d1
	lsr.l	#1,d6
	add.l	db_blitin_y(a3),d6
	add.l	db_shiftit(a3),d6
	addq.l	#1,d1
	bsr	restore_pic_xy
	movem.l	(a7)+,d0-d7/a0-a6

	movem.l	d0-d7/a0-a6,-(a7)
	move.l	db_links(a3),d6
	addq.l	#1,d6
	move.l	d6,d1
	sub.l	d0,d1
	lsr.l	#1,d6
	move.l	db_shiftit(a3),d5
	sub.l	d6,d5
	add.l	db_blitin_y(a3),d5
	move.l	d5,d6
	addq.l	#1,d1
	move.l	d0,db_links(a3)
	bsr	restore_pic_xy
	movem.l	(a7)+,d0-d7/a0-a6
	rts

inverse_rotate:
	move.l	db_wstore_moves2(a3),a2
	moveq	#0,d2
	move.w	db_blitin_fy(a3),d2
	lsr.l	#1,d2
	move.l	d2,db_shiftit(a3)

	move.l	db_effect_speed(a3),d7
	subq.l	#2,d7
rep_flip_angle3b:
	bsr	get_flip_start_sub
	move.l	d0,db_rechts(a3)
	move.l	d0,d6
	move.l	d6,d5
	lsr.l	#1,d5
	neg.l	d5
;	bra	in_rep_flip_inc4
rep_flip_inc3b:
	move.l	d5,d0
	bsr	get_flip_proj
	movem.l	d0/d5/d6/d7/a2/a4/a5,-(a7)
	move.l	d0,d1
	exg	d1,d5
	neg.l	d5
	add.l	db_shiftit(a3),d5
	moveq	#1,d3
	move.l	db_blitin_ox(a3),d0
	add.l	db_blitin_y(a3),d1
	add.l	db_shiftit(a3),d1
	moveq	#0,d4
	move.w	d1,d6
	add.w	d6,d6
	cmp.w	0(a2,d6.w),d5
	beq	no_put_flip4
	move.w	d5,0(a2,d6.w)
	bsr	put_blitin_in2
no_put_flip4:
	movem.l	(a7)+,d0/d5/d6/d7/a2/a4/a5
	addq.l	#1,d5
in_rep_flip_inc4:
	dbf	d6,rep_flip_inc3b
	move.l	db_rechts(a3),d0
	cmp.l	db_links(a3),d0
	bge	no_remove_flip3b
	bsr	restore_flip
no_remove_flip3b:
	bsr	wacht_tijd2
	bne	exit_flip_blitin3b
	dbf	d7,rep_flip_angle3b
exit_flip_blitin3b:
	rts
	
init_h1m1:
;	bsr	calc_effect_speed_blitin
;	bsr	adapt_blitin_exvert_moving
;	bsr	maak_mask_blitin	

	bsr	init_movement

init_h1m1_rev:
	bsr	set_blit_fast_direct
	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_viewblok(a3),a4
	move.l	db_blitin_ox(a3),d0
	move.l	db_blitin_y(a3),d1
	moveq	#0,d5
	move.l	db_effect_speed(a3),d3
	rts
	
blitin_h1m1:

	move.l	db_active_viewblok(a3),a5
	move.l	vb_tempbuffer(a5),db_waardestpic(a3)

	bsr	init_h1m1
	bsr	copy_trip

	move.l	db_inactive_fileblok(a3),a4
	move.w	db_blitin_fy(a3),d5
	move.w	#1,d3
rep_b_h1m1:
	movem.l	d0-d5/a4/a5,-(a7)
	bsr	put_blitin_in2		; zet de blitin neer
	bsr	wacht_tijd2
	movem.l	(a7)+,d0-d5/a4/a5
	bne	exit_blh1m1
	add.l	db_effect_speed(a3),d3
	sub.l	db_effect_speed(a3),d5
	cmp.w	#0,d5
	bge	rep_b_h1m1

exit_blh1m1:
	move.l	db_blitin_ox(a3),d0
	move.l	db_blitin_y(a3),d1

	move.w	db_blitin_fy(a3),d3
	moveq	#0,d5
	move.l	db_inactive_viewblok(a3),a4
	bsr	put_blitin_in3			; zet de blitin neer
	rts


blitin_h2m1:
	move.l	db_active_viewblok(a3),a5
	move.l	vb_tempbuffer(a5),db_waardestpic(a3)

	bsr	init_h1m1

	movem.l	d0-d7/a0-a6,-(a7)
	move.l	db_active_viewblok(a3),a5
	bsr	copy_inactive_triple
	tst.w	d0
	bne	no_triple_h2m1
	move.l	db_triple_mem(a3),d7
	move.l	db_triple_mem(a3),db_waardestpic(a3)
no_triple_h2m1:
	movem.l	(a7)+,d0-d7/a0-a6

	move.l	db_inactive_fileblok(a3),a4
	moveq	#0,d5
	move.w	#1,d3
	add.w	db_blitin_fy(a3),d1
	subq.w	#1,d1
rep_b_h2m1:
	movem.l	d0-d5/a4/a5,-(a7)
	bsr	put_blitin_in2		; zet de blitin neer
	bsr	wacht_tijd2
	movem.l	(a7)+,d0-d5/a4/a5
	bne	exit_h2m1
	add.l	db_effect_speed(a3),d3
	sub.l	db_effect_speed(a3),d1

	cmp.w	db_blitin_fy(a3),d3
	ble	rep_b_h2m1

exit_h2m1:
	move.l	db_blitin_ox(a3),d0
	move.l	db_blitin_y(a3),d1
	move.w	db_blitin_fy(a3),d3
	moveq	#0,d5

	move.l	db_inactive_viewblok(a3),a4
	bsr	put_blitin_in3		; zet de blitin neer
	rts


blitin_h2:
	bsr	init_h1h2h3h4
	move.l	db_inactive_fileblok(a3),a4
	
;	move.w	vb_leny(a4),d5
	move.w	db_blitin_fy(a3),d5
	sub.l	d3,d5

	add.w	d5,d1
rep_b_h2:
	movem.l	d0-d5/a4/a5,-(a7)
	bsr	put_blitin_in2		; zet de blitin neer
	bsr	wacht_tijd2
	movem.l	(a7)+,d0-d5/a4/a5
	bne	exit_h2
	sub.l	db_effect_speed(a3),d5
	sub.l	db_effect_speed(a3),d1
	cmp.w	#0,d5
	bge	rep_b_h2

exit_h2:
	moveq	#0,d5
	move.l	db_blitin_y(a3),d1
	bsr	put_blitin_in2		; zet de laatste blitin zeker neer
	rts

do_blitind4d5:
	move.l	db_inactive_fileblok(a3),a4
	move.l	db_blitin_y(a3),d1
	add.l	d5,d1
	movem.l	d0-d6/a4/a5,-(a7)
	bsr	put_blitin_in2		; zet de blitin neer
	movem.l	(a7)+,d0-d6/a4/a5
	movem.l	d0-d6/a4/a5,-(a7)
	move.l	d4,d5
	move.l	db_blitin_y(a3),d1
	add.l	d5,d1
	bsr	put_blitin_in2		; zet de blitin neer
	bsr	wacht_tijd2
	movem.l	(a7)+,d0-d6/a4/a5	; ????????????????????
	rts

blitin_h3:
	bsr	init_h1h2h3h4

blitin_h3_in:
	move.l	db_inactive_fileblok(a3),a4
	move.w	db_blitin_fy(a3),d5
	sub.l	d3,d5
	moveq	#0,d4			; van boven
rep_b_h3:

	bsr	do_blitind4d5
	bne	exit_bh3
	sub.l	db_effect_speed(a3),d5
	add.l	db_effect_speed(a3),d4

	cmp.w	d4,d5
	bge	rep_b_h3

exit_bh3:
	move.l	db_blitin_y(a3),d1
	add.l	d5,d1
	bsr	put_blitin_in2		; zet de laatste blitin zeker neer

	rts

blitin_h4:
	bsr	init_h1h2h3h4
	move.l	db_inactive_fileblok(a3),a4

	move.w	db_blitin_fy(a3),d5
	lsr.w	#1,d5			; vanuit het midden
	move.l	d5,d4			; naar beneden
	sub.l	d3,d5			; naar boven
rep_b_h4:

	bsr	do_blitind4d5
	bne	exit_bh4
	
	sub.l	db_effect_speed(a3),d5
	add.l	db_effect_speed(a3),d4

	cmp.w	#0,d5
	bge	rep_b_h4

exit_bh4:
	move.l	db_blitin_y(a3),d1
	moveq	#0,d5

	movem.l	d0/d1/a4/a5,-(a7)
	bsr	put_blitin_in2		; zet de laatste blitin zeker neer
	movem.l	(a7)+,d0/d1/a4/a5

	move.w	db_blitin_fy(a3),d5
	sub.l	db_effect_speed(a3),d5
	add.w	d5,d1
	bsr	put_blitin_in2
	rts

init_h5h6:

	bsr	calc_effect_speed_blitin3
	bsr	init_movement2
	
;	bsr	adapt_blitin_exvert_moving
;	bsr	maak_mask_blitin	
;	move.l	db_active_viewblok(a3),a5
;	move.l	vb_tempbuffer(a5),d0
;	move.l	d0,db_waarsourcepic(a3)
;	move.l	db_active_viewblok(a3),a5
;	move.l	db_inactive_viewblok(a3),a4
;	tst.b	db_triple_changed(a3)
;	bne	no_ch_mem2
;	move.l	vb_tempbuffer(a5),db_waardestpic(a3)
;no_ch_mem2:

	clr.b	db_evendone(a3)
	move.b	#$ff,db_please_no_double(a3)
	move.l	db_blitin_ox(a3),d0
	move.l	db_blitin_y(a3),d1
	move.l	db_effect_speed(a3),d3
	moveq	#0,d5
	move.l	db_effect_speed(a3),d6
	rts
	
blitin_h5:
	bsr	init_h5h6
	move.l	db_inactive_fileblok(a3),a4
blitin_h5_in:
	move.l	db_blitin_ox(a3),d0
rep_b_h5:
	move.l	db_blitin_y(a3),d1
	add.l	d5,d1
	movem.l	d0-d6/a4/a5,-(a7)
	bsr	put_blitin_in2		; zet de blitin neer
	bsr	wacht_tijd
	movem.l	(a7)+,d0-d6/a4/a5
	bne	exit_b_h5
	add.l	d6,d5
	add.l	d6,d5
	cmp.w	#0,d5
	blt	exit_b_h5
	cmp.w	db_blitin_fy(a3),d5
	ble	rep_b_h5
	neg.l	d6
	tst.b	db_evendone(a3)
	bne	exit_b_h5
	add.l	d6,d5			; oneven lijnen
	move.b	#$ff,db_evendone(a3)
	bra	rep_b_h5
exit_b_h5:
	move.l	db_blitin_y(a3),d1
	add.l	d5,d1
	bsr	put_blitin_in2		; zet de laatste blitin zeker neer
	rts

blitin_h6:
	bsr	init_h5h6
	move.l	db_inactive_fileblok(a3),a4
	move.w	db_blitin_fy(a3),d5
	neg.l	d6
	move.l	db_blitin_ox(a3),d0
rep_b_h6:
	move.l	db_blitin_y(a3),d1
	add.l	d5,d1
	movem.l	d0-d6/a4/a5,-(a7)
	bsr	put_blitin_in2		; zet de blitin neer
	bsr	wacht_tijd
	movem.l	(a7)+,d0-d6/a4/a5
	bne	exit_b_h6
	add.l	d6,d5
	add.l	d6,d5

	cmp.w	db_blitin_fy(a3),d5
	bgt	exit_b_h6

	cmp.w	#0,d5
	bge	rep_b_h6

	neg.l	d6
	tst.b	db_evendone(a3)
	bne	exit_b_h6
	add.l	d6,d5			; oneven lijnen
	move.b	#$ff,db_evendone(a3)
	bra	rep_b_h5
exit_b_h6:
	move.l	db_blitin_y(a3),d1
	add.l	d5,d1
	bsr	put_blitin_in2		; zet de laatste blitin zeker neer
	rts

blitin_h3_ver2:
	bsr	init_h5h6
	move.l	db_inactive_fileblok(a3),a4
	moveq	#0,d0
	move.w	db_blitin_fy(a3),d0
	move.l	db_effect_speed(a3),d7
	sub.l	db_effect_speed(a3),d0
	bsr	check_even_oneven
	move.l	d0,d5			; van onderen
	moveq	#0,d4			; van boven
	move.l	db_effect_speed(a3),d6
	add.l	d6,d6			; maal 2
	move.l	db_blitin_ox(a3),d0
rep_b_h3_v2:
	move.l	db_blitin_y(a3),d1
	add.l	d5,d1
	movem.l	d0-d6/a4/a5,-(a7)
	bsr	put_blitin_in2		; zet de blitin neer
	movem.l	(a7)+,d0-d6/a4/a5
	movem.l	d0-d6/a4/a5,-(a7)
	move.l	d4,d5
	move.l	db_blitin_y(a3),d1
	add.l	d5,d1
	bsr	put_blitin_in2		; zet de blitin neer
	bsr	wacht_tijd
	movem.l	(a7)+,d0-d6/a4/a5	; ????????????????????
	bne	exit_h3_v2

	sub.l	d6,d5
	add.l	d6,d4

	cmp.w	#0,d5
	bge	rep_b_h3_v2
exit_h3_v2:
	moveq	#0,d5
	bsr	do_blitind4d5		; zet de laatste blitin zeker neer
	rts

init_movement:
	bsr	calc_effect_speed_blitin

init_movement2:
	bsr	adapt_blitin_exvert_moving

	tst.b	db_screen_empty(a3)
	bne	.no_mask_needed1

	bsr	maak_mask_blitin	

	bra	.mask_made

.no_mask_needed1:

; set the different sources and destinations

	move.l	db_inactive_viewblok(a3),a4
	move.l	db_inactive_fileblok(a3),a5
	move.l	vb_breedte_x(a4),vb_breedte_x(a5)
	move.w	vb_leny(a4),vb_leny(a5)
	move.w	vb_lenx(a4),vb_lenx(a5)

	move.l	db_inactive_viewblok(a3),a4
	move.l	vb_tempbuffer(a4),db_blitinpic(a3)
	move.l	vb_tempbuffer(a4),db_blitinmask(a3)
	move.l	db_active_viewblok(a3),a5
	move.l	vb_tempbuffer(a5),db_waarsourcepic(a3)	; to screen
.mask_made:

	move.l	db_active_viewblok(a3),a5
	move.l	vb_tempbuffer(a5),db_waardestpic(a3)	; if no double use this

	tst.b	db_no_trip_needed(a3)
	bne	no_triple3_im

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	dbstr4023(pc),a0
	jsr	KPutStr
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	bsr	copy_inactive_triple
	tst.w	d0
	bne	no_triple3_im
	move.l	db_triple_mem(a3),db_waardestpic(a3)
no_triple3_im:
	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_viewblok(a3),a4
	rts
	
blitin_mov_init:
	bsr	init_movement
blitin_mov_init2:
	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_viewblok(a3),a4
	moveq	#0,d1
	moveq	#0,d0
	bra	path_end1

minmaxdata:	dc.w	0,0,0,0

mm_maxx = 0
mm_maxy = 2
mm_minx = 4
mm_miny = 6

init_min_max:
	move.l	db_window_data(a3),a0
	move.l	winp_x(a0),d0
	move.l	winp_y(a0),d1
	move.l	d0,d2
	move.l	d1,d3
	add.l	winp_wx(a0),d2
	cmp.w	#0,d0
	beq	.nowrap
	add.l	#16,d2				; for wrap around ????
.nowrap:
	add.l	winp_wy(a0),d3
	lea	minmaxdata(pc),a0
	move.w	db_clip_maxx(a3),mm_maxx(a0)
	move.w	db_clip_maxy(a3),mm_maxy(a0)
	move.w	db_clip_minx(a3),mm_minx(a0)
	move.w	db_clip_miny(a3),mm_miny(a0)
	rts

set_blit_fast_above:
	bsr	init_min_max
	move.w	#10000,mm_maxx(a0)
	move.w	#-10000,mm_minx(a0)
	move.w	#10000,mm_maxy(a0)
	bra	set_blit_fast

set_blit_fast_below:
	bsr	init_min_max
	move.w	#10000,mm_maxx(a0)
	move.w	#-10000,mm_miny(a0)
	move.w	#-10000,mm_minx(a0)
	bra	set_blit_fast

set_blit_fast_lbr:
	bsr	init_min_max
	bsr	check_blit_fast_min_left
	move.w	#10000,mm_maxx(a0)
	move.w	#-10000,mm_miny(a0)
	bra	set_blit_fast

set_blit_fast_rbl:
	bsr	init_min_max
	bsr	check_blit_fast_min_left
	move.w	#-10000,mm_minx(a0)
	move.w	#-10000,mm_miny(a0)
	bra	set_blit_fast

set_blit_fast_rtl:
	bsr	init_min_max
	bsr	check_blit_fast_min_left
	move.w	#-10000,mm_minx(a0)
	move.w	#10000,mm_maxy(a0)
	bra	set_blit_fast

set_blit_fast_ltr:
	bsr	init_min_max
	move.w	#10000,mm_maxx(a0)
	move.w	#10000,mm_maxy(a0)
	bra	set_blit_fast

set_blit_fast_lrrl:
	bsr	init_min_max
	bsr	check_blit_fast_min_left
	move.w	#10000,mm_maxx(a0)
	move.w	#-10000,mm_minx(a0)
	bra	set_blit_fast

set_blit_fast_lr:
	bsr	init_min_max
	move.w	#10000,mm_maxx(a0)
	bra	set_blit_fast

set_blit_fast_rl:
	bsr	init_min_max
	bsr	check_blit_fast_min_left
	move.w	#-10000,mm_minx(a0)
	and.w	#$fff0,d0			; set on word boundary
	bra	set_blit_fast
	
set_blit_fast_du:
	bsr	init_min_max
	move.w	#-10000,mm_miny(a0)
	and.w	#$fff0,d0			; set on word boundary
	bra	set_blit_fast

set_blit_fast_direct:
	bsr	init_min_max
	and.w	#$fff0,d0			; set on word boundary
	bra	set_blit_fast
*
* Perform an extra check to see if the left side word is clear
* the blitter functions use an extra word at the left side of the screen
*
check_blit_fast_min_left:
	lea	minmaxdata(pc),a0
	move.w	mm_minx(a0),d7
	cmp.w	#16,mm_minx(a0)
	bge	.no_pro
	move.l	#10000,d2			; force no fast blit later
.no_pro:
	rts
	
set_blit_fast_ud:
	bsr	init_min_max
	move.w	#10000,mm_maxy(a0)
	and.w	#$fff0,d0			; set on word boundary
;	bra	set_blit_fast

set_blit_fast:
	move.l	db_active_viewblok(a3),a0
	move.w	vb_lenx(a0),a0
	move.l	db_active_viewblok(a3),a0
	cmp.w	vb_lenx(a0),d2
	bgt	no_fast

	lea	minmaxdata(pc),a0
	cmp.w	mm_maxy(a0),d1
	bge	do_fast
	cmp.w	mm_maxx(a0),d0
	bge	do_fast
	cmp.w	mm_minx(a0),d2
	ble	do_fast
	cmp.w	mm_miny(a0),d3
	ble	do_fast

	bra	no_fast

do_fast:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Doing fast",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.b	#$ff,db_screen_empty(a3)
	move.w	#$5ca,db_blitin_con0(a3)
	move.w	#$100,db_blitin_clear(a3)
	rts
no_fast:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Sorry no fast",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC
	move.b	#$0,db_screen_empty(a3)
	move.w	#$fca,db_blitin_con0(a3)
	move.w	#$9f0,db_blitin_clear(a3)
	rts

maak_path_righttop_left_eas:
	move.l	db_effect_speed(a3),d3

	bsr	init_hoeken
	bsr	neg_rady

	moveq	#0,d5
	move.w	vb_lenx(a5),d5
	sub.l	db_blitin_dx(a3),d5

	moveq	#0,d7
	move.w	db_blitin_fy(a3),d7
	neg.l	d7

	bra	make_path

maak_path_leftbot_right_eas:

	move.l	db_effect_speed(a3),d3

	bsr	init_hoeken
	bsr	neg_radx
	moveq	#0,d5
	move.w	db_blitin_fx(a3),d5
	neg.l	d5
	add.l	db_blitin_dx(a3),d5

	moveq	#0,d7
	move.w	vb_leny(a5),d7
	bra	make_path

maak_path_rightbot_left_eas:

	move.l	db_effect_speed(a3),d3

	bsr	init_hoeken
	moveq	#0,d5
	move.w	vb_lenx(a5),d5
	sub.l	db_blitin_dx(a3),d5

	moveq	#0,d7
	move.w	vb_leny(a5),d7
	bra	make_path

maak_path_lefttop_right_eas:
	move.l	db_effect_speed(a3),d3

	bsr	init_hoeken
	bsr	neg_rady
	bsr	neg_radx
	moveq	#0,d5
	move.w	db_blitin_fx(a3),d5
	neg.l	d5
	add.l	db_blitin_dx(a3),d5
	moveq	#0,d7
	move.w	db_blitin_fy(a3),d7
	neg.l	d7
	bra	make_path

maak_path_left_right_eas:
	move.l	db_effect_speed(a3),d3
	move.w	db_blitin_fx(a3),d0
	add.l	db_blitin_ox(a3),d0
	sub.l	db_blitin_dx(a3),d0
	moveq	#0,d1
	bsr	init_hoeken
	bsr	neg_radx
	moveq	#0,d5
	move.w	db_blitin_fx(a3),d5
	neg.l	d5
	add.l	db_blitin_dx(a3),d5
	move.l	db_blitin_y(a3),d7
	bra	make_path

maak_path_right_left_eas:
	move.l	db_effect_speed(a3),d3

	move.l	db_effect_speed(a3),d3
	move.w	vb_lenx(a5),d0
	sub.l	db_blitin_x(a3),d0
	moveq	#0,d1
	bsr	init_hoeken
	moveq	#0,d5
	move.w	vb_lenx(a5),d5
	sub.l	db_blitin_dx(a3),d5
	move.l	db_blitin_y(a3),d7
	bra	make_path

maak_path_up_down_eas:
	move.l	db_effect_speed(a3),d3
	move.l	db_blitin_x(a3),d0
	moveq	#0,d0
	move.w	db_blitin_fy(a3),d1
	move.l	d1,d7
	move.l	db_blitin_y(a3),d2
	add.w	d2,d1
	bsr	init_hoeken
	bsr	neg_rady
	move.l	db_blitin_ox(a3),d5
	neg.l	d7
	bra	make_path

maak_path_down_up_eas:
	move.l	db_effect_speed(a3),d3
	move.l	db_blitin_x(a3),d0
	moveq	#0,d0
	move.w	vb_leny(a5),d1
	move.l	d1,d7
	move.l	db_blitin_y(a3),d2
	sub.w	d2,d1
	bsr	init_hoeken
	move.l	db_blitin_ox(a3),d5

make_path:
	tst.w	d6
	beq	rep_cirkel_eas1
	subq.l	#1,d6
rep_cirkel_eas1:
	bsr	sinsin

	move.l	d5,d0
	sub.l	d3,d0
	move.w	d0,(a6)+

	move.l	d7,d1
	sub.l	d4,d1
	move.w	d1,(a6)+

	dbf	d6,rep_cirkel_eas1
	bra	path_end2
	
blitin_down_up:
	bsr	blitin_mov_init
	bsr	maak_path_down_up 
	bsr	set_blit_fast_du
	bsr	by_path
	rts

maak_path_down_up:
	cmp.l	#1,db_varltimes(a3)
	beq	maak_path_down_up_eas
	move.w	vb_leny(a5),d1
	move.l	db_blitin_ox(a3),d0
	move.l	db_effect_speed(a3),d3
	move.l	db_blitin_y(a3),d2
rep_b_d_u1:
	move.w	d0,(a6)+
	move.w	d1,(a6)+
	sub.l	d3,d1

	cmp.l	d2,d1
	bge	rep_b_d_u1
	bra	path_end2

blitin_up_down:
	bsr	blitin_mov_init
	bsr	maak_path_up_down 
	bsr	set_blit_fast_ud
	bsr	by_path
	rts

maak_path_up_down:
	cmp.l	#1,db_varltimes(a3)
	beq	maak_path_up_down_eas
	
	move.w	db_blitin_fy(a3),d1
	neg.l	d1
	move.l	db_blitin_ox(a3),d0

	move.l	db_effect_speed(a3),d3
	move.l	db_blitin_y(a3),d2
rep_b_u_d:
	move.w	d0,(a6)+
	move.w	d1,(a6)+
	add.l	d3,d1
	cmp.l	d2,d1
	ble	rep_b_u_d
no_triple4:
	bra	path_end2

blitin_left_right:
	bsr	blitin_mov_init
	bsr	maak_path_left_right
	bsr	set_blit_fast_lr
	bsr	by_path
	rts

maak_path_left_right:
	cmp.l	#1,db_varltimes(a3)
	beq	maak_path_left_right_eas
	move.l	db_blitin_ox(a3),d2
	moveq	#0,d0
	move.w	db_blitin_fx(a3),d0
	neg.l	d0
	add.l	db_blitin_dx(a3),d0
	move.l	db_blitin_y(a3),d1
	move.l	db_effect_speed(a3),d3
rep_b_l_r:
	move.w	d0,(a6)+
	move.w	d1,(a6)+
	add.l	d3,d0
	cmp.l	d2,d0
	ble	rep_b_l_r
no_triple5:
	bra	path_end2

blitin_right_left:
	bsr	blitin_mov_init
	bsr	maak_path_right_left
	bsr	set_blit_fast_rl

	move.l	db_inactive_viewblok(a3),a4
	move.w	vb_lenx(a4),d0
	
	bsr	by_path
	rts

maak_path_right_left:
	cmp.l	#1,db_varltimes(a3)
	beq	maak_path_right_left_eas
	move.l	db_blitin_ox(a3),d2
	move.w	vb_lenx(a5),d0
	move.l	db_blitin_y(a3),d1
	move.l	db_effect_speed(a3),d3
rep_b_r_l:
	move.w	d0,(a6)+
	move.w	d1,(a6)+
	sub.l	d3,d0
	cmp.l	d2,d0
	bge	rep_b_r_l
no_triple6:
	bra	path_end2

blitin_lefttop_right:
	bsr	blitin_mov_init
	bsr	maak_path_lefttop_right
	bsr	set_blit_fast_ltr
	bsr	by_path
	rts

maak_path_lefttop_right:

	move.w	db_blitin_fx(a3),d0
	add.l	db_blitin_ox(a3),d0
	sub.l	db_blitin_dx(a3),d0

	move.w	db_blitin_fy(a3),d1
	add.l	db_blitin_y(a3),d1		

	cmp.l	#1,db_varltimes(a3)
	beq	maak_path_lefttop_right_eas

	lsl.l	#8,d1			; d0 * 256
	move.l	db_effect_speed(a3),d2
	divu	d2,d0
	and.w	#$ffff,d0
	divu	d0,d1			; per x pixels nu 16 * y pixels
	move.w	d1,d2

	moveq	#0,d0
	moveq	#0,d1

	move.w	db_blitin_fx(a3),d0
	neg.l	d0
	add.l	db_blitin_dx(a3),d0

	move.w	vb_leny(a4),d1
	move.w	db_blitin_fy(a3),d1
	neg.l	d1

	neg.l	d2
	bra	path_lesser

blitin_righttop_left:
	bsr	blitin_mov_init
	bsr	maak_path_righttop_left
	bsr	set_blit_fast_rtl
	bsr	by_path
	rts

maak_path_righttop_left:

	move.w	vb_lenx(a5),d0
	sub.l	db_blitin_x(a3),d0	; totaal aantal x-as pixels

	moveq	#0,d1
	move.w	db_blitin_fy(a3),d1
	add.l	db_blitin_y(a3),d1		

	cmp.l	#1,db_varltimes(a3)
	beq	maak_path_righttop_left_eas

	lsl.l	#8,d1			; d0 * 256
	move.l	db_effect_speed(a3),d2
	divu	d2,d0
	and.w	#$ffff,d0
	divu	d0,d1			; per x pixels nu 16 * y pixels
	move.w	d1,d2
	moveq	#0,d0
	moveq	#0,d1

	move.w	vb_lenx(a5),d0
	sub.l	db_blitin_dx(a3),d0

	move.w	vb_leny(a4),d1
	move.w	db_blitin_fy(a3),d1
	neg.l	d1

	neg.l	d2
	bra	path_greater

blitin_rightbot_left:
	bsr	blitin_mov_init
	bsr	maak_path_rightbot_left
	bsr	set_blit_fast_rbl
	bsr	by_path
	rts

maak_path_rightbot_left:
	move.w	vb_lenx(a5),d0
	sub.l	db_blitin_x(a3),d0		; totaal aantal x-as pixels
	
	move.w	vb_leny(a5),d1		; totaal aantal y-as pixels
	sub.l	db_blitin_y(a3),d1		

	cmp.l	#1,db_varltimes(a3)
	beq	maak_path_rightbot_left_eas

	lsl.l	#8,d1			; d0 * 256
	move.l	db_effect_speed(a3),d2
	divu	d2,d0
	and.w	#$ffff,d0
	divu	d0,d1			; per x pixels nu 16 * y pixels
	move.w	d1,d2
	moveq	#0,d0
	moveq	#0,d1

	move.w	vb_lenx(a5),d0
	sub.l	db_blitin_dx(a3),d0

	move.w	vb_leny(a5),d1

path_greater:
	move.l	db_effect_speed(a3),d3
	asl.l	#8,d1	
	move.l	db_blitin_ox(a3),d4
rep_b_rb_l:
	move.l	d1,d5
	asr.l	#8,d5
	move.w	d0,(a6)+
	move.w	d5,(a6)+
	sub.l	d3,d0
	sub.l	d2,d1
	cmp.l	d4,d0
	bge	rep_b_rb_l
no_triple9:
	bra	path_end2

blitin_leftbot_right:
	bsr	blitin_mov_init
	bsr	maak_path_leftbot_right
	bsr	set_blit_fast_lbr
	bsr	by_path
	rts

maak_path_leftbot_right:
	move.w	db_blitin_fx(a3),d0
	
	add.l	db_blitin_ox(a3),d0		; totaal aantal x-as pixels
	sub.l	db_blitin_dx(a3),d0
	
	move.w	vb_leny(a5),d1		; totaal aantal y-as pixels
	sub.l	db_blitin_y(a3),d1		

	cmp.l	#1,db_varltimes(a3)
	beq	maak_path_leftbot_right_eas

	lsl.l	#8,d1			; d0 * 256
	move.l	db_effect_speed(a3),d2
	divu	d2,d0
	and.w	#$ffff,d0
	divu	d0,d1			; per x pixels nu 16 * y pixels
	move.w	d1,d2
	moveq	#0,d0
	moveq	#0,d1

	move.w	db_blitin_fx(a3),d0
	neg.l	d0
	add.l	db_blitin_dx(a3),d0

	move.w	vb_leny(a5),d1

path_lesser:
	asl.l	#8,d1	
	move.l	db_blitin_ox(a3),d4
	move.l	db_effect_speed(a3),d3
rep_b_lb_r:
	move.l	d1,d5
	asr.l	#8,d5
	move.w	d0,(a6)+
	move.w	d5,(a6)+
	add.l	d3,d0
	sub.l	d2,d1
	cmp.l	d4,d0
	ble	rep_b_lb_r
no_triple10:
	bra	path_end2

	IFNE	BIT24
decrunch24bit:
	moveq	#0,d2
	move.l	vb_breedte_x(a5),d1
	and.l	#$7,d1
	beq	.no_to_small
	move.l	vb_breedte_x(a5),d1
	addq.l	#7,d1
	lsr.l	#3,d1
	lsl.l	#3,d1
	sub.l	vb_breedte_x(a5),d1
	move.l	d1,d2				; modulo AA quad assign
.no_to_small:
	move.l	vb_breedte_x(a5),d0
	add.l	d2,d0
	move.w	vb_leny(a5),d1
	mulu	d1,d0
	lsl.l	#3,d0			; 8 planes
	IFNE XAPP
	move.l	vb_mlscrpointer(a5),a2
	move.l	sbu_Size(a2),d7
	ELSE
	move.l	#MEMSIZE,d7
	ENDC

	cmp.l	d0,d7
	bge	dec24

	moveq	#-1,d0			; do not fit in chipmem available
	rts

dec24:
	move.l	vb_body_start(a5),a0
	move.l	vb_tempbuffer(a5),a1
	move.l	a0,db_data_pointer(a3)
	move.l	a1,db_dest_pointer(a3)

	move.w	vb_leny(a5),d6
	subq.w	#1,d6
rep_lenyaa:
	bsr.w	skipline
	bsr.w	skipline
	bsr.w	skipline
	bsr.w	skipline
	bsr.w	skipline
	bsr.w	decrunchline24
	bsr.w	decrunchline24
	bsr.w	decrunchline24
; green
	bsr.w	skipline
	bsr.w	skipline
	bsr.w	skipline
	bsr.w	skipline
	bsr.w	skipline
	bsr.w	decrunchline24
	bsr.w	decrunchline24
	bsr.w	decrunchline24
; blue
	bsr.w	skipline
	bsr.w	skipline
	bsr.w	skipline
	bsr.w	skipline
	bsr.w	skipline
	bsr.w	skipline
	bsr.w	decrunchline24
	bsr.w	decrunchline24
	dbf	d6,rep_lenyaa

	move.l	vb_breedte_x(a5),d1
	and.l	#$7,d1
	beq	.no_to_small
	move.l	vb_breedte_x(a5),d1
	addq.l	#7,d1
	lsr.l	#3,d1
	lsl.l	#3,d1
	move.l	d1,vb_breedte_x(a5)	
.no_to_small:
	bsr.b	set_cols_6planes
	rts

set_cols_6planes:
	lea	vb_colbytes(a5),a0
	move.w	#8,vb_planes(a5)		; for RG & B
	move.l	#768,vb_color_count(a5)

	moveq	#0,d2
	move.w	vb_lenx(a5),d1
	cmp.w	#370,d1
	ble.s	.geen_hires
	move.b	#$FF,vb_hires(a5)
	move.w	#$8000,d2
.geen_hires:
	move.w	vb_leny(a5),d1
	cmp.w	#390,d1
	ble.s	.geen_lace
	move.b	#$ff,vb_interlace(a5)
	or.w	#$4,d2
.geen_lace:
	move.w	d2,vb_mode(a5)


	movem.l	d0-d7/a0-a6,-(a7)
	moveq	#0,d4
	lea	blue_inc(pc),a1
	lea	red_inc(pc),a2
	lea	red_inc(pc),a3
	moveq	#0,d0
	moveq	#0,d1
	moveq	#0,d2

	moveq	#3,d7
.rep_blue:
	moveq	#7,d6
	moveq	#0,d1
.rep_green:
	moveq	#7,d5
	moveq	#0,d0
.rep_red:	
	move.b	d0,(a0)+
	move.b	d1,(a0)+
	move.b	d2,(a0)+
	move.b	(a3)+,d0
	addq.w	#1,d4
	dbf	d5,.rep_red
	lea	red_inc(pc),a3
	move.b	(a2)+,d1
	dbf	d6,.rep_green
	lea	red_inc(pc),a2
	move.b	(a1)+,d2
	dbf	d7,.rep_blue
	movem.l	(a7)+,d0-d7/a0-a6
	moveq	#0,d0
	rts

blue_inc:	dc.b	70,165,255
;red_inc:	dc.b	36,73,109,146,182,219,255
red_inc:	dc.b	32,96,128,160,192,224,255
	even
*
* decrunch een regel
*
decrunchline24:
	move.l	db_data_pointer(a3),a0		; de crunch data

	move.l	db_dest_pointer(a3),a1
	move.l	a1,a2
	add.l	vb_breedte_x(a5),a2
	
	cmp.b	#1,vb_compression(a5)
	beq	.dun1

* compression geen 1 dus of 0 of een onbekende 
* copieer van body_start naar unpacked

	move.l	vb_breedte_x(a5),d0
	lsr.l	#1,d0
	subq.l	#1,d0

.dno_co:
	move.w	(a0)+,(a1)+
	dbf	d0,.dno_co
	add.l	d2,a1
	move.l	a0,db_data_pointer(a3)		; volgende regel data
	move.l	a1,db_dest_pointer(a3)
	rts

.dun1:
.dun_again:	
	cmp.l	a2,a1
	bge.s	.dun_end

	moveq	#0,d5
	move.b	(a0)+,d5
	bmi.b	.dun_minus
.dun_plu:
	move.b	(a0)+,(a1)+
	dbf	d5,.dun_plu
	bra.s	.dun_again

.dun_minus:
	neg.b	d5
	move.b	(a0)+,d0

.dun_rm:	move.b	d0,(a1)+
	dbf	d5,.dun_rm

	bra.s	.dun_again
.dun_end:	
	add.l	d2,a1
	move.l	a0,db_data_pointer(a3)		; voor volgende lijn
	move.l	a1,db_dest_pointer(a3)
	rts
*
* skip een regel
*
skipline:	
	move.l	db_data_pointer(a3),a0		; de crunch data

	move.l	db_dest_pointer(a3),a1
	move.l	a1,a2
	add.l	vb_breedte_x(a5),a2
	
	cmp.b	#1,vb_compression(a5)
	beq.s	skipdun1

* compression geen 1 dus of 0 of een onbekende 
* copieer van body_start naar unpacked

	add.l	vb_breedte_x(a5),a0
	move.l	a0,db_data_pointer(a3)		; volgende regel data
	rts

skipdun1:

skipdun_again:	
	cmp.l	a2,a1
	bge.s	skipdun_end
	moveq	#0,d5
	move.b	(a0)+,d5
	bmi.b	skipdun_minus
skipdun_plu:
	addq.l	#1,d5
	add.l	d5,a0
	add.l	d5,a1
	bra.s	skipdun_again
skipdun_minus:
	neg.b	d5
	addq.l	#1,a0
skipdun_rm:
	addq.l	#1,d5
	add.l	d5,a1
	bra.s	skipdun_again
skipdun_end:	
	move.l	a0,db_data_pointer(a3)		; voor volgende lijn
	rts
	ENDC
*
* unpack de file naar de inactive buffer en maak een viewport
* van de data die in de inactive file buffer staat
*
proces_file:
	move.l	db_active_viewblok(a3),a4
	move.l	vb_breedte_x(a4),d0

	move.l	db_inactive_fileblok(a3),a5
	move.l	db_inactive_viewblok(a3),a4
	lea	vb_bmhd_found(a5),a2
	lea	vb_bmhd_found(a4),a1
	move.l	a5,a0
	move.l	a4,a1
rep_copy_proces:			; copieer de info data
	move.l	(a0)+,(a1)+		; naar de viewblok structuur
	cmp.l	a0,a2
	bge.b	rep_copy_proces

	move.l	db_inactive_viewblok(a3),a5

	move.l	db_active_viewblok(a3),a4

	IFNE	BIT24
	cmp.w	#24,vb_planes(a5)
	bne	.no24

	bsr	decrunch24bit
	tst.w	d0
	bne	no_cando

	bra	mask_done
.no24:
	ENDC
	
	cmp.b	#1,vb_masking(a5)
	bne	no_mask5

	bsr	clear_convert_bytes

	move.l	vb_breedte_x(a5),vb_breedte_x(a4)	; tricky ???
	move.w	vb_planes(a5),vb_planes(a4)

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	dbstr4032(pc),a0
	jsr	KPutStr
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	bsr	adaptdecrunch

	bra	mask_done

no_mask5:
	
; do some OS special checks here

dotest:
	move.l	db_active_viewblok(a3),a4

	tst.b	db_aa_present(a3)
	beq	no_to_small

	cmp.w	#6,vb_planes(a5)
	bgt	.real_aa

	move.l	vb_breedte_x(a5),d0
	and.w	#$3,d0
	beq	no_to_small

.real_aa:	
	move.l	vb_breedte_x(a4),d6		; save orginal values
	move.w	vb_lenx(a4),d7
	move.w	vb_leny(a4),d4
	move.w	vb_planes(a4),d5

	move.l	vb_breedte_x(a5),d0
	and.l	#$7,d0
	beq	no_to_small

	move.l	vb_breedte_x(a5),d0
	add.l	#7,d0
	lsr.l	#3,d0
	lsl.l	#3,d0

.no_aas:	
	move.l	d0,vb_breedte_x(a4)
	lsl.l	#3,d0

	move.w	d0,vb_lenx(a4)
	move.w	vb_leny(a5),vb_leny(a4)
	move.w	vb_planes(a5),vb_planes(a4)

	move.l	vb_breedte_x(a4),d0
	mulu.w	vb_planes(a4),d0
	mulu.w	vb_leny(a4),d0
	move.l	d0,d2

	IFNE XAPP
	movem.l	a2,-(a7)
	move.l	db_mlsysstruct(a3),a2
	move.l	sbu_Size(a2),d1
	movem.l	(a7)+,a2
	ELSE
	move.l	#MEMSIZE,d1
	ENDC

	cmp.l	d1,d0
	blt	.size_oke

	move.l	d2,d0
	sub.l	d1,d0
	move.l	vb_breedte_x(a4),d2
	mulu	vb_planes(a4),d2	; total width 1 line
	divu	d2,d0			; nr of lines to many
	tst.w	d0
	bmi	.size_oke
	addq.w	#1,d0
	move.l	db_inactive_fileblok(a3),a5
	sub.w	d0,vb_leny(a5)
.size_oke:

	movem.l	d4-d7,-(a7)
	bsr	clear_convert_bytes
	move.b	#$ff,db_wipe_in_largerx(a3)
	bsr	adaptdecrunch
	movem.l	(a7)+,d4-d7

	move.l	db_active_viewblok(a3),a5
	move.l	d6,vb_breedte_x(a5)
	move.w	d7,vb_lenx(a5)
	move.w	d4,vb_leny(a5)
	move.w	d5,vb_planes(a5)

	move.l	db_inactive_viewblok(a3),a5
	move.l	db_inactive_fileblok(a3),a4

	move.w	vb_planes(a4),vb_planes(a5)
	move.l	vb_breedte_x(a4),d0

	add.l	#7,d0
	lsr.l	#3,d0
	lsl.l	#3,d0
	move.l	d0,vb_breedte_x(a5)
	lsl.l	#3,d0
	move.w	d0,vb_lenx(a5)
	move.l	vb_breedte_x(a4),d0
	move.w	vb_lenx(a4),d0
	move.w	vb_planes(a4),d0
	bra	mask5y

no_to_small:
	bsr.w	unpack
	tst.w	d0
	bne	no_cando

mask5y:

mask_done:
	move.l	db_inactive_viewblok(a3),a5
	bsr.w	maak_viewport		; de inactive view is nu klaar
;	bsr	showinactive		; display het veranderde plaatje
	moveq	#0,d0
no_cando:
	rts

*
* laadfile leest de file in 
* de data pointer is a5 welke naar de filedata wijst
* a0 wijst naar de in de laden file
*
laadfile:
	move.l	a0,-(a7)
	bsr.w	clear_inactive_buffers		; clear eventueel geheugen en view

	move.l	db_inactive_fileblok(a3),a5
	move.b	#$0,vb_bitmapped(a5)
	move.l	(a7)+,a0

	move.l	a0,vb_filenaam(a5)

	move.w	#3,vb_color_count(a5)	; minimum colors ???
	move.b	#0,vb_bmhd_found(a5)
	move.b	#0,vb_body_found(a5)

	move.l	db_inactive_fileblok(a3),a5
	bsr.w	read_whole_file		; lees de file geheel in het geheugen ?

*****
go_check:
	move.l	db_inactive_fileblok(a3),a5

	bsr.w	check_chunks		; controleer chunks

	moveq	#1,d7
	cmp.b	#$ff,vb_bmhd_found(a5)	; is er een bmhd header zo niet exit
	bne.w	.sexit

	cmp.b	#$ff,vb_body_found(a5)
	bne	.sexit
	rts

.sexit:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr2(pc),a0
	jsr	KPutStr
	bra	.tt2
.dbstr2:	dc.b	"BMHD of BODY not found",10,0
	even
.tt2:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	bra	sexit

* copieer de a5 buffer naar de triple buffer
* ( Met de blitter ???????????????????????? )
*
copy_inactive_triple:
	tst.b	db_triple_present(a3)
	beq	no_triple2
	tst.b	db_triple_changed(a3)
	bne	trip_ready

	move.l	vb_breedte_x(a5),d0
	mulu	vb_planes(a5),d0
	mulu	vb_leny(a5),d0
	lsr.l	#2,d0				; longs ?
	move.l	vb_tempbuffer(a5),a0
	move.l	db_triple_mem(a3),a1
rep_c_it:
	move.l	(a0)+,(a1)+
	subq.l	#1,d0
	bpl	rep_c_it
	move.b	#$ff,db_triple_changed(a3)
trip_ready:
	moveq	#0,d0
	rts
no_triple2:
	moveq	#-1,d0
	rts

	IFEQ	XAPP
*
* voer de internal slide uit stop bij een muis druk
*
do_slide:
	lea	slide_it,a1
ww:
	move.l	(a1)+,a0
	cmp.l	#0,a0
	bne	again_slide
	lea	slide_it,a1
	move.l	(a1)+,a0
;	bra	exit
again_slide:

	move.l	(a1),db_effect_speed(a3)
	move.l	(a1)+,db_effect_speed_org(a3)
	move.l	(a1),db_varltimes(a3)
	move.l	(a1)+,db_counttimer(a3)
	move.l	(a1)+,db_variation(a3)
	move.l	(a1)+,db_blitin_x(a3)
	move.l	(a1)+,db_blitin_y(a3)
	move.l	(a1)+,d0
	bpl	no_atoz_wipe

	move.l	atoz_teller(pc),d0
	cmp.l	#NUM_OFFWIPES,d0
	ble	no_clearaz
	lea	atoz_teller(pc),a2
	clr.l	(a2)
	clr.l	d0
no_clearaz:
	lea	atoz_teller(pc),a2
	addq.l	#1,(a2)

no_atoz_wipe:
	move.l	d0,db_which_wipe(a3)

	bsr	wipe_file_in

	move.l	(a1)+,d0
;	bsr	wacht50hertzjes

 	move.l	#21,db_which_wipe(a3)
	move.l	#5,db_effect_speed(a3)
	move.l	#5,db_effect_speed_org(a3)
	move.l	#0,db_varltimes(a3)
	move.l	#0,db_counttimer(a3)
	move.l	#0,db_variation(a3)

	btst	#6,$bfe001
	beq	eww

	move.l	a1,-(a7)
	jsr	test_document
	move.l	(a7)+,a1

	btst	#6,$bfe001
	bne	ww
eww:
	rts
	ENDC

;FLIP_SIZE = 480
*
* For now we use the same sizes to flip a page ( 580 )
*
convert_to_flipsize:
	movem.l	d0-d7/a0-a6,-(a7)

; first check the size of the active picture
;	bra	end_check_f_size

	move.l	db_active_viewblok(a3),a5

	move.w	vb_leny(a5),d0
	cmp.w	db_max_y(a3),d0
;	cmp.w	#FLIP_SIZE,vb_leny(a5)
	bge	active_flip_ready
	bsr	copy_info
	move.l	db_inactive_viewblok(a3),a4
	move.l	db_active_viewblok(a3),a5

	tst.b	vb_interlace(a5)
	bne	.no_i_flip

	bsr	convert_nolace_lace

	tst.w	d0
	bne	err_flip
	move.l	db_inactive_viewblok(a3),a4
	move.l	db_active_viewblok(a3),a5
	move.b	vb_mode(a5),vb_mode(a4)
	move.b	vb_interlace(a5),vb_interlace(a4)
.no_i_flip:
	move.w	db_max_y(a3),vb_leny(a4)
;	move.w	#FLIP_SIZE,vb_leny(a4)
	bsr	no_leny_bigger

active_flip_ready:

	move.l	db_inactive_fileblok(a3),a4
	move.w	vb_leny(a4),d0
	cmp.w	db_max_x(a3),d0	
;	cmp.w	#FLIP_SIZE,vb_leny(a4)
	bge	end_check_f_size

	bsr	clear_convert_bytes
	move.l	db_active_viewblok(a3),a5
	move.l	vb_breedte_x(a5),d0
	move.l	vb_breedte_x(a4),vb_breedte_x(a5)
	move.w	vb_planes(a5),d1
	move.w	vb_planes(a4),vb_planes(a5)

	tst.b	vb_interlace(a4)
	bne	no_i_flip
	move.b	#$ff,db_wipe_in_to_lace(a3)
	move.b	#$ff,vb_interlace(a4)
no_i_flip:
;	move.w	db_or_vmode_mask(a3),d7
;	or.w	vb_mode(a4),d7
	or.w	#V_LACE,vb_mode(a4)			; set mode on LACE
	move.b	#$ff,db_wipe_in_largery(a3)
	movem.l	d0/d1,-(a7)
	bsr	adaptdecrunch
	move.l	db_inactive_viewblok(a3),a5

;	move.w	#FLIP_SIZE,vb_leny(a5)
	move.w	db_max_y(a3),vb_leny(a5)
	bsr	maak_viewport
;	bsr	showpicture
	movem.l	(a7)+,d0/d1
	move.l	db_active_viewblok(a3),a5
	move.w	d1,vb_planes(a5)
	move.l	d0,vb_breedte_x(a5)
err_flip:
	movem.l	(a7)+,d0-d7/a0-a6
	rts
	
inactive_flip_ready:
	movem.l	(a7)+,d0-d7/a0-a6
	rts

end_check_f_size:
	movem.l	(a7)+,d0-d7/a0-a6
	movem.l	d0,-(a7)
	bsr	proces_file
	movem.l	(a7)+,d0
	rts

sexit2:	jmp	exit
*
* wipe de file met de naam waarnaar a0 wijst
* wipe which_wipe wordt gebruikt
* in geval van een echte wipe-in wordt de picture achteraf volledig geladen
*
do_wipe_file_in:
	movem.l	d0-d7/a0-a6,-(a7)
	bra	do_it

wipe_file_in:
	movem.l	d0-d7/a0-a6,-(a7)
	bsr	laadfile		; laad in inactive fileblok

	IFEQ	XAPP
	move.l	db_inactive_fileblok(a3),a5
	move.w	vb_planes(a5),d0

	IFNE	BIT24
	cmp.w	#24,d0
	beq	do_it
	ENDC

	tst.b	vb_shires(a5)
	beq	.no_shir2
	cmp.w	db_max_Depth_SHires(a3),d0
	bgt	to_deep
	bra	checked_pl2
.no_shir2:	
	tst.b	vb_hires(a5)
	beq	no_hir2
	cmp.w	db_max_Depth_Hires(a3),d0
	bgt	to_deep
	bra	checked_pl2
no_hir2:
	move.w	db_max_Depth_Lores(a3),d1
	cmp.w	d1,d0
	bgt	to_deep
checked_pl2:
	ENDC
do_it:
	move.l	db_which_wipe(a3),d0
	cmp.w	#-1,d0
	bne	no_atoz

	move.l	atoz_teller(pc),d0
	cmp.l	#NUM_OFFWIPES,d0
	ble	no_clearaz2
	lea	atoz_teller(pc),a2
	clr.l	(a2)
	clr.l	d0
no_clearaz2:
	lea	atoz_teller(pc),a2
	addq.l	#1,(a2)
no_atoz:
	move.l	d0,db_which_wipe(a3)
	and.l	#$ffff,d0
	cmp.l	#NUM_OFFWIPES,d0
	ble	do_wipe
	moveq	#0,d0

; hier moet je kijken wat voor soort wipe-in er verlangd wordt
; bijv. een colorfade na een colorfade out dient te inactive view
; gecopieerd te worden naar de active view met de kleuren overeenkomstig.
 
do_wipe:
	lea	wipe_types_new(pc),a4
check_again:
	move.b	0(a4,d0),d1
	and.b	#RAND_S,d1
	beq	no_random_eff

	add.l	d0,d0
	add.l	d0,d0			; longwords
	lea	wipes(pc),a4
	move.l	0(a4,d0),a4
	jsr	(a4)

	IFNE PRAND
	movem.l	a0-a6/d0-d7,-(a7)
	lea	db_varltimes(a3),a1
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	lea	db_variation(a3),a1
	lea	.dbstr2(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"Varl is %ld",10,0
.dbstr2:	dc.b	"Vari is %ld",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	bra	check_again
no_random_eff:

	move.b	0(a4,d0),d1
	and.b	#STAY_S,d1		; its a non changing effect
	beq	no_fade

	move.l	d0,-(a7)
	bsr	proces_file
	tst.w	d0
	bne	too_big
	move.l	(a7)+,d0

	move.l	d0,-(a7)
	bsr	calc_effect_speed_fade
	move.l	(a7)+,d0

	add.l	d0,d0
	add.l	d0,d0			; longwords

	lea	wipes(pc),a4
	move.l	0(a4,d0),a4
	jsr	(a4)

	tst.b		db_aa_present(a3)
	beq		no_aa100
	move.l		db_inactive_fileblok(a3),a5	
	lea		vb_colbytes(a5),a1
	move.l		db_active_viewblok(a3),a5
	lea		vb_colbytes(a5),a2
	move.w		#255,d0
copy_org_colors_aa:
	move.b		(a1)+,(a2)+
	move.b		(a1)+,(a2)+
	move.b		(a1)+,(a2)+
	dbf		d0,copy_org_colors_aa
	bra		load_org_cols

no_aa100:	
	move.l		db_inactive_fileblok(a3),a5	
	lea		vb_colors(a5),a1
	move.l		db_active_viewblok(a3),a5
	lea		vb_colors(a5),a2
	moveq		#31,d0
copy_org_colors:
	move.w		(a1)+,(a2)+
	dbf		d0,copy_org_colors

load_org_cols:
;	bsr		laad_kleuren		; ??????????????????????
;	bsr		showinactive
	bra		exit_wipe_in

too_big:

	move.l	(a7)+,d0
	bra	to_deep

no_fade:
; check to see if it is a flip effect
	move.b	0(a4,d0),d1
	and.b	#FLIP_S,d1
	beq	no_flip_eff

; see to it that al the pics here are the same size a hight of 580
; and perhaps interlace ???	

****************************

	bsr	convert_to_flipsize

	move.l	d0,-(a7)
	bsr	calc_effect_speed_fade
	move.l	(a7)+,d0

	add.l	d0,d0
	add.l	d0,d0			; longwords

	lea	wipes(pc),a4
	move.l	0(a4,d0),a4
	jsr	(a4)

	tst.b		db_aa_present(a3)
	beq		no_aa1002
	move.l		db_inactive_fileblok(a3),a5	
	lea		vb_colbytes(a5),a1
	move.l		db_active_viewblok(a3),a5
	lea		vb_colbytes(a5),a2
	move.w		#255,d0
copy_org_colors_aa2:
	move.b	(a1)+,(a2)+
	move.b	(a1)+,(a2)+
	move.b	(a1)+,(a2)+
	dbf	d0,copy_org_colors_aa2
	bra	load_org_cols2
no_aa1002:	
	move.l		db_inactive_fileblok(a3),a5	
	lea		vb_colors(a5),a1
	move.l		db_active_viewblok(a3),a5
	lea		vb_colors(a5),a2
	moveq		#31,d0
copy_org_colors2:
	move.w		(a1)+,(a2)+
	dbf		d0,copy_org_colors2

load_org_cols2:

***************************
	bra	exit_wipe_in
	
no_flip_eff:
	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_fileblok(a3),a4

	move.w	vb_mode(a4),d0
	and.w	#V_HAM,d0
	beq	no_dest_ham

	move.w	vb_mode(a5),d0
	and.w	#V_HAM,d0
	beq	no_wipe
	
no_dest_ham:

; het active view wordt in adaptfile verandert 
; eigenlijk is het misschien noodzakelijk om eerst de adaptdecrunch
; functie aan te roepen
	bsr	clear_convert_bytes

	bsr	adaptfile		; active -> size ( inactive )
	tst.w	d0
	bne	no_wipe
					; verander zo nodig de active view
	tst.b	db_change(a3)
	beq	no_change

; De pic ongecrunched in het geheugen moet van resolutie veranderd worden
; en mischien van aantal planes. Dus decrunchen en direct veranderen ??
; Of naar een derde buffer decrunchen en daarna converteren.
; De file staat ongecrunched in de inactive file buffer

	bsr	adaptdecrunch

	bra	setd	

no_change:
					; orginele data nog steeds in 
					; inactive file blok ???????????
	bsr	proces_file		; maak van de wipe-in file een viewblok
					; en decrunch
	tst.w	d0
	bne	exit_wipe_in
setd:	

	bsr	calc_fade_colors

	move.l	db_which_wipe(a3),d0
	add.l	d0,d0
	add.l	d0,d0			; longwords
	lea	wipes(pc),a4
	move.l	0(a4,d0),a4
	jsr	(a4)

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Done effect",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC


no_wipe:
	bsr	proces_file		; maak van de wipe-in file een viewblok
					; zet deze in de inactive buffer
	tst.w	d0
	bne	exit_wipe_in

	bsr.w	showpicture		; laad orgineel

exit_wipe_in:

	bsr	cycle_cols

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"exit wipe file in",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	movem.l	(a7)+,d0-d7/a0-a6
	rts

cycle_cols:
	tst.b	db_quit(a3)
	bne	.no_colcyc
	tst.b	db_cyc_quit(a3)
	bne	.no_colcyc
	move.w	db_colflags(a3),d0
	and.w	#1,d0
	beq	.no_colcyc
	move.w	db_colrate(a3),d0
	beq	.no_colcyc
	cmp.w	#36,d0
	beq	.no_colcyc
	move.b	#1,db_cycle_wait(a3)
	bsr	col_cycle			; col cycle off
	move.b	#0,db_cycle_wait(a3)
.no_colcyc:
	rts

check_drng:
	tst.b	db_drng_found(a3)
	beq	no_col_cyc

	move.l	db_active_viewblok(a3),a5
	move.l	db_colcyc_point(a3),a6
	moveq	#0,d0

	move.b	6(a6),d0	; nr. Dcolor
	add.b	7(a6),d0	; nr. Dindex	; asume nomore than 256 both
	beq	no_col_cyc
	
	lea	8(a6),a1
	
	move.l	db_waarcolors1(a3),a0		; point to piece off memory
						; for cycle table	
	move.l	#0,(a0)				; set guardian for reverse
	lsl.l	#2,d0
	lea	0(a0,d0.l),a0

	move.l	d0,db_end_dr(a3)
	move.l	#0,4(a0)			; set guardian forward
	addq.l	#8,a0
	move.l	a0,db_col_tab(a3)
	move.l	a0,a2				; here is the index table

	move.l	db_waarcolors1(a3),a0		; point to piece off memory
	moveq	#0,d0
	move.b	6(a6),d0
	beq	.no_dcolor
	bra	.ind
.rep_dc:
	moveq	#0,d1
	move.b	(a1)+,d1			; cell number
	lsl.l	#2,d1				; longs
	addq.l	#4,d1				; add one for reverse end check

	move.b	#2,0(a0,d1.l)			; set type Dstruct
	move.b	(a1)+,1(a0,d1.l)		; red
	move.b	(a1)+,2(a0,d1.l)		; green
	move.b	(a1)+,3(a0,d1.l)		; blue
.ind:			
	dbf	d0,.rep_dc

.no_dcolor:
	moveq	#0,d0
	move.b	7(a6),d0
	beq	no_col_cyc
	bra	.ind2
.rep_cc:
	moveq	#0,d1
	move.b	(a1)+,d1			; cell number
	lsl.l	#2,d1				; longs
	addq.l	#4,d1
	move.b	#1,0(a0,d1.l)			; set type D index
	move.b	(a1),1(a0,d1.l)			; set color index
	moveq	#0,d2
	move.b	(a1)+,d2
	move.w	d2,(a2)+			; store color number
	move.w	d1,(a2)+			; set pointer ( relative )
.ind2:			
	dbf	d0,.rep_cc
	move.w	#-1,(a2)			; end of the list

; all the tables are ready now try to cycle it

.rep_colc2:
	move.l	db_col_tab(a3),a4
	move.l	db_waarcolors1(a3),a2		; cycle table pointer

.rep_colc1:
	moveq	#0,d0
	move.w	(a4)+,d0			; color number
	cmp.w	#-1,d0
	beq	.end_drc
	move.w	(a4),d1
	move.b	0(a2,d1.w),d2			; get type ( true or index )
	bne	.p_oke				; check for guardian
	move.l	db_end_dr(a3),d1
	move.w	d1,(a4)				; reset pointer to start
	move.b	0(a2,d1.w),d2			; and get type
.p_oke:	
	cmp.b	#1,d2
	beq	.index

; d2 should be 2 so we have a true color struct here

	lea	1(a2,d1.w),a0			; a0 points to RGB bytes
	bsr	load_color2			; set d0 to the RGB values
	bra	.set_col
.index:
	move.l	d0,d6
	moveq	#0,d7
	move.b	1(a2,d1.w),d7
	bsr	load_color			; load d6 with color d7
.set_col:
	subq.w	#4,(a4)+
	bra	.rep_colc1
.end_drc:
	bsr	col_cyc_wait
	tst.b	d0
	beq	.rep_colc2
no_col_cyc:
	rts
	
col_cycle:
	moveq	#0,d6
	moveq	#0,d7
	tst.b	db_crng_found(a3)
	beq	check_drng

	move.l	db_active_viewblok(a3),a5
	move.w	db_colflags(a3),d0
	and.w	#2,d0
	bne	col_c_up
	move.w	db_colhigh(a3),d7	; the start color from low color
	move.w	db_colhigh(a3),d5	; the end color number
.rep_colc2:
	move.w	db_collow(a3),d6	; the color number
	move.l	d7,a4
.rep_colc1:
	bsr	load_color
	addq.w	#1,d7
	cmp.w	d7,d5
	bge	.no_loop
	move.w	db_collow(a3),d7	; when at the end start again
.no_loop:
	add.w	#1,d6
	cmp.w	db_colhigh(a3),d6
	ble	.rep_colc1

	move.l	a4,d7

	subq.w	#1,d7
	cmp.w	db_collow(a3),d7
	bge	.no_l
	move.w	db_colhigh(a3),d7	; when at the end start again
.no_l:
	bsr	col_cyc_wait
	tst.b	d0
	beq.w	.rep_colc2		; start again

	rts

col_c_up:
	move.w	db_collow(a3),d7	; the start color from low color
	move.w	db_colhigh(a3),d5	; the end color number
.rep_colc2:
	move.w	db_collow(a3),d6	; the color number
	move.l	d7,a4
.rep_colc1:
	bsr	load_color
	addq.w	#1,d7
	cmp.w	d7,d5
	bge	.no_loop
	move.w	db_collow(a3),d7	; when at the end start again
.no_loop:
	add.w	#1,d6
	cmp.w	d6,d5
	bge	.rep_colc1
	move.l	a4,d7

	addq.w	#1,d7
	cmp.w	d7,d5
	bge	.no_l
	move.w	db_collow(a3),d7	; when at the end start again
.no_l:
	bsr	col_cyc_wait
	tst.b	d0
	beq.w	.rep_colc2		; start again
	rts

col_cyc_wait:
	moveq	#0,d2
	move.l	d2,d3
	move.w	db_colrate(a3),d2
	move.w	#16384,d3
	divu	d2,d3
	and.l	#$ff,d3
	lea	teller(pc),a0
	move.l	d3,(a0)
	move.l	d3,off_tel_start(a0)
	movem.l	d1-d7/a0-a6,-(a7)
	bsr	wacht_tijd2
	movem.l	(a7)+,d1-d7/a0-a6
	rts

*
* Load color d6 with value's from d7
*
load_color:
	move.l	d6,d0
	lea	vb_colbytes(a5),a0
	move.l	d7,d1
	move.l	d1,d2
	add.l	d1,d1
	add.l	d2,d1
	add.l	d1,a0

load_color2:
	cmp.w	#39,db_libversion(a3)
	blt	.no_v39

	move.b	(a0)+,d4
	move.b	d4,d1
	lsl.w	#8,d1
	move.b	d4,d1
	move.w	d1,d4
	swap	d1
	move.w	d4,d1

	move.b	(a0)+,d4
	move.b	d4,d2
	lsl.w	#8,d2
	move.b	d4,d2
	move.w	d2,d4
	swap	d2
	move.w	d4,d2

	move.b	(a0)+,d4
	move.b	d4,d3
	lsl.w	#8,d3
	move.b	d4,d3
	move.w	d3,d4
	swap	d3
	move.w	d4,d3
	and.l	#$ff,d0
	move.l	vb_viewportw(a5),a0
	move.l	db_graphbase(a3),a6
	jsr	_LVOSetRGB32(a6)
	rts
.no_v39:
	move.b	(a0)+,d1
	move.b	(a0)+,d2
	move.b	(a0)+,d3
	lsr.b	#4,d1
	lsr.b	#4,d2
	lsr.b	#4,d3
	move.l	vb_viewportw(a5),a0
	move.l	db_graphbase(a3),a6

	jsr	_LVOSetRGB4(a6)
	rts		

to_large:	
	moveq	#ERR_NO_MEM,d0
	bra	into_deep
to_deep:
	moveq	#ERR_RES_NOT_SUP,d0
into_deep:
	move.l	db_inactive_fileblok(a3),a5
	move.l	vb_filenaam(a5),a0
	bsr	copy_err
	movem.l	(a7)+,d0-d7/a0-a6
	rts

ran3:	dc.b	2,3,4,5,6,7,8,9,77,78,79,80,81,82,83,84,0,0
	even
	
copy_err2:
	bra	copy_err	

random_eff3:
	moveq	#16,d0
	bsr	rnd
	and.l	#$ff,d0
	move.b	ran3(pc,d0.w),d0
	cmp.w	oldeff(pc),d0
	beq	random_eff3
	move.w	d0,oldeff
	move.l	db_eff_nums(a3),a0
	bsr	set_random_eff
.oke:
	rts
oldeff:	dc.w	0
*
* Generate a random effect with all the effects
*
random_eff2:
	moveq	#NUM_OFFWIPES,d0
	bsr	rnd
;	subq.w	#1,d0
	and.w	#$ff,d0
	cmp.w	#74,d0
	beq	random_eff2
	cmp.w	#75,d0
	beq	random_eff2
	cmp.w	#76,d0
	beq	random_eff2
	cmp.w	oldeff(pc),d0
	beq	random_eff2
	move.w	d0,oldeff
	
	move.l	db_eff_nums(a3),a0
	bra	set_random_eff

find_block_eff:
	moveq	#NUM_OFFWIPES,d7
rep_fb:
	cmp.w	#34,(a0)
	beq	found_block_eff
	lea.l	en_SIZEOF(a0),a0
	dbf	d7,rep_fb
	move.l	db_eff_nums(a3),a0	; no block effect ?
found_block_eff:
	rts

	IFNE PRAND
dbstr1000:	dc.b	"Random eff num = %d",10,0
dbstr1001:	dc.b	"Random eff point = %d",10,0
	even
ttdb:	dc.w	0
	even
	ENDC

*
* Generate a random effect with only the blocks
*
random_eff1:
	moveq	#12,d0
	bsr	rnd
	and.w	#$f,d0
	move.l	db_eff_nums(a3),a0
	bsr	find_block_eff

set_random_eff:
	move.l	d0,db_which_wipe(a3)

	IFNE PRAND
	movem.l	a0-a6/d0-d7,-(a7)
	lea	dbstr1001(pc),a0
	lea	db_which_wipe+2(a3),a1
	jsr	KPutFmt
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	cmp.l	#NUM_OFFWIPES,d0
	ble	oke_wp
	moveq	#0,d0
oke_wp:
	mulu	#en_SIZEOF,d0
	add.l	d0,a0
	moveq	#0,d0
	move.w	en_number(a0),d0

	cmp.l	#NUM_OFFWIPES,d0
	ble	.oke_wp
	moveq	#0,d0
.oke_wp:
	move.l	d0,db_which_wipe(a3)

	IFNE PRAND
	movem.l	a0-a6/d0-d7,-(a7)
	lea	dbstr1000(pc),a0
	lea	db_which_wipe+2(a3),a1
	jsr	KPutFmt
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.w	en_vari(a0),d0
;	addq.w	#1,d0
	move.l	d0,db_variation(a3)

	move.w	en_chunk(a0),d0
	addq.w	#1,d0
	move.l	d0,db_varltimes(a3)
	move.l	d0,db_varltimes_in(a3)

	move.l	db_which_wipe(a3),d0
	lea	wipe_types_new(pc),a4
	rts

clear_convert_bytes:
	move.b	#$0,db_change(a3)
	move.b	#$0,db_wipe_in_to_hires(a3)
	move.b	#$0,db_wipe_in_to_shires(a3)
	move.b	#$0,db_wipe_in_to_lowres(a3)
	move.b	#$0,db_wipe_in_to_non_lace(a3)
	move.b	#$0,db_wipe_in_to_lace(a3)
	move.b	#$0,db_wipe_in_largerx(a3)
	move.b	#$0,db_wipe_in_largery(a3)
	move.b	#$0,db_wipe_in_add_planes(a3)
	rts

* showpicture laat de inactive view zien
* en verandert deze naar de active view
*
showpicture:
	move.l		db_inactive_viewblok(a3),a5
	move.l		db_graphbase(a3),a6
	move.l		vb_vieww(a5),a1
	jsr		_LVOLoadView(a6)

switch_blok:
	move.l		db_inactive_viewblok(a3),d0
	move.l		db_active_viewblok(a3),db_inactive_viewblok(a3)
	move.l		d0,db_active_viewblok(a3)

set_viewed:
	move.l		a0,-(a7)
	move.l		db_inactive_viewblok(a3),a0
	cmpa.w		#0,a0
	beq		no_change100

	IFNE XAPP
	move.l		vb_mlscrpointer(a0),a0
	move.w		#FALSE,sbu_Viewed(a0)

	move.l		db_active_viewblok(a3),a0
	cmpa.w		#0,a0
	beq		no_change100

	move.l		vb_mlscrpointer(a0),a0
	move.w		#TRUE,sbu_Viewed(a0)

	ENDC

no_change100:
	move.l		(a7)+,a0
	rts
*
* Copy data from fileblok to viewblok and decrunch the file
*
decrunch_clip:
	move.l	db_inactive_fileblok(a3),a5
	move.l	db_inactive_viewblok(a3),a4
	lea	vb_bmhd_found(a5),a2
	lea	vb_bmhd_found(a4),a1
	move.l	a5,a0
	move.l	a4,a1
rep_copy_proces2:			; copieer de info data
	move.l	(a0)+,(a1)+		; naar de viewblok structuur
	cmp.l	a0,a2
	bge.b	rep_copy_proces2

	move.l	db_inactive_viewblok(a3),a5
	move.l	db_active_viewblok(a3),a4

	cmp.b	#1,vb_masking(a5)
	bne	no_mask52

	move.b	#1,db_clip_leaved(a3)
	
no_mask52:
	bsr.w	unpack
	tst.w	d0
	bne	no_cando2
mask5y2:
	move.l	db_inactive_viewblok(a3),a5
	moveq	#0,d0
no_cando2:
	rts

check_but2:
	movem.l	d0/d1/a0/a1,-(a7)

	moveq	#0,d1
	moveq	#0,d0
	move.l	db_sig_ptoc(a3),d2
	move.l	$4.w,a6
	jsr	_LVOSetSignal(a6)
	move.l	d0,d1
	bra	check_mes2

	movem.l	(a7)+,d0/d1/a0/a1
	moveq	#0,d0
	rts

check_but:
	movem.l	d0/d1/a0/a1,-(a7)

	moveq	#0,d1
	moveq	#0,d0
	move.l	db_sig_ptoc(a3),d2
	move.l	$4.w,a6
	jsr	_LVOSetSignal(a6)
	move.l	d0,d1

	and.l	#SIGF_ABORT,d1
	bne	do_term2

	movem.l	(a7)+,d0/d1/a0/a1

	IFEQ	XAPP
	btst	#6,$bfe001
	bne	no_mouse
	moveq	#-1,d0
	move.b	#$ff,db_quit(a3)
	rts
no_mouse2:
	ENDC
	bsr	check_key
	rts
	
	moveq	#0,d0
	rts

do_term2:
	movem.l	(a7)+,d0/d1/a0/a1
	move.b	#$ff,db_quit(a3)
	moveq	#-1,d0
	rts

wacht_key:
	movem.l	d0/d1/a0/a1,-(a7)

	move.l	db_waitmasker(a3),d0

	or.l	db_sig_ptoc(a3),d0
	or.l	#SIGF_ABORT,d0
	or.l	db_go_on_key_sig(a3),d0
	bra	no_sigptoc
	
* wacht varwtime keer 50hz sla dit varltimes over zo varltimes keer snel 
* weer terug. Er worden zo varltimes lijnen tegelijk neer gezet
*
wacht_tijd:
	subq.l	#1,db_counttimer(a3)
	bpl	no_wacht
	move.l	db_varltimes(a3),db_counttimer(a3)

wacht_tijd2:
	movem.l	d0/d1/a0/a1,-(a7)

	move.l	db_waitmasker(a3),d0

	or.l	db_sig_ptoc(a3),d0
	or.l	#SIGF_ABORT,d0
no_sigptoc:

	move.l	$4.w,a6
	jsr	_LVOWait(a6)

check_mes2:
	movem.l	d0/d1/a0/a1,-(a7)
	move.l	$4.w,a6
	jsr	_LVOForbid(a6)
	movem.l	(a7)+,d0/d1/a0/a1

	move.l	d0,d6
	move.l	d0,d1
	and.l	#SIGF_ABORT,d0
	bne	doterm
	move.l	d1,d0

	and.l	db_sig_ptoc(a3),d1
	beq	no_wacht1

	IFNE ESCLOOK
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr3(pc),a0
	jsr	KPutStr
	bra	.tt3
.dbstr3:	dc.b	"Got signal from above",10,0
	even
.tt3:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

; Er is een message van boven kijk wat zij doet en handel ernaar

search_list:
	move.l	db_msgpointer(a3),a0		;get msgport ptr
	lea.l	MP_MSGLIST(a0),a1		;make ptr to msglist
	move.l	LH_HEAD(a1),d1			;get first 	

scan:	move.l	d1,a1
	move.l	LH_HEAD(a1),d1
	beq	end_of_list

	IFNE ESCLOOK
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr3(pc),a0
	lea	pd_Cmd(a1),a1
	jsr	KPutFmt
	bra	.tt3
.dbstr3:	dc.b	"Cmd is %lx",10,0
	even
.tt3:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	pd_Cmd(a1),d2		; pd command

	cmp.l	#DCC_DORUN,d2
	beq	doterm			; let de C-prog handle the reply

	cmp.l	#DCC_DOTERM,d2
	beq	doterm			; let de C-prog handle the reply

	cmp.l	#DCC_DOSTOP,d2
	beq	doterm

	tst.b	db_cycle_wait(a3)
	beq	scan

	cmp.l	#DCC_DOEASYSTOP,d2
	beq	doterm

	cmp.l	#DCC_EASYTERM,d2
	beq	doterm


	bra	scan

doterm:
	IFNE ESCLOOK
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr3(pc),a0
	jsr	KPutStr
	bra	.tt3
.dbstr3:	dc.b	"Terminate",10,0
	even
.tt3:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	movem.l	d0/d1/a0/a1,-(a7)
	sub.l	a1,a1
	move.l	$4.w,a6
	jsr	_LVOFindTask(a6)
	move.l	d0,a1
	move.l	d6,d0
	jsr	_LVOSignal(a6)		; restore signal
	movem.l	(a7)+,d0/d1/a0/a1

	move.b	#$ff,db_quit(a3)
	move.l	$4.w,a6
	jsr	_LVOPermit(a6)

	movem.l	(a7)+,d0/d1/a0/a1
	moveq	#-1,d0
	rts

no_wacht1:
	move.l	d6,d0
	and.l	db_go_on_key_sig(a3),d0
	beq	end_of_list

	IFNE PRAND
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr3(pc),a0
	lea	.dat3(pc),a1
	move.l	d0,(a1)
	move.l	db_go_on_key_sig(a3),4(a1)
	jsr	KPutFmt
	bra	.tt3
.dbstr3:	dc.b	"Go on key pressed %lx,%lx",10,0
	even
.dat3:	dc.l	0,0
.tt3:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	$4.w,a6
	jsr	_LVOPermit(a6)
	movem.l	(a7)+,d0/d1/a0/a1
	moveq	#2,d0
	rts

end_of_list:
	move.l	$4.w,a6
	jsr	_LVOPermit(a6)
	movem.l	(a7)+,d0/d1/a0/a1

no_wacht:

	IFEQ	XAPP
	btst	#6,$bfe001
	bne	no_mouse
	moveq	#-1,d0
	move.b	#$ff,db_quit(a3)
	rts

no_mouse:
;	move.l	d6,d0
;	and.l	db_waitmasker(a3),d0
;	beq	wacht_tijd2			; sure to wait for 50hz
	ENDC

	bsr	check_key
	rts

	moveq	#0,d0
	rts

check_key:
	move.b	$bfec01,d0
	not	d0
	ror.b	#1,d0
	cmp.b	#$60,d0
	bhi	no_key_ch1

;	cmp.b	#$45,d0
;	beq	arrow_found
;	cmp.b	#$4e,d0

;	beq	arrow_found
;	cmp.b	#$4f,d0
;	beq	arrow_found
;key:	
	IFNE ESCLOOK
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr3(pc),a0
	jsr	KPutStr
	bra	.tt3
.dbstr3:	dc.b	"There was a key pressed",10,0
	even
.tt3:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	db_dosbase(a3),a6
	movem.l	d1/a0/a1,-(a7)
	moveq	#1,d1
	jsr	_LVODelay(a6)		; give system time to check keys
	movem.l	(a7)+,d1/a1/a0
no_key_ch1
	moveq	#0,d0
	rts

arrow_found:
	moveq	#-1,d0
	move.b	#$ff,db_quit(a3)
	rts

copy_err_stub:
	bra	copy_err

kcut:
	lea	teller(pc),a0
	move.l	#1,(a0)
	move.l	#1,4(a0)
	move.l	db_inactive_viewblok(a3),a5
	bsr	maak_viewport
	move.l	db_effect_speed(a3),d4
	moveq	#25,d5
	sub.w	d4,d5
	move.l	d5,d7
	move.l	db_varltimes(a3),d3
	cmp.l	#2,d3
	bne	.no_ff
	moveq	#2,d5
.no_ff:
.rep_kcut:
	move.l	db_graphbase(a3),a6
	move.l	db_active_viewblok(a3),a5
	move.l	db_graphbase(a3),a6
	move.l	vb_vieww(a5),a1
	jsr	_LVOLoadView(a6)		; show old 

	move.w	d5,d6
.ww:	
	movem.l	d6/d7,-(a7)
	bsr	wacht_tijd2
	movem.l	(a7)+,d6/d7
	dbf	d6,.ww

	move.l	db_graphbase(a3),a6
	move.l	db_inactive_viewblok(a3),a5
	move.l	db_graphbase(a3),a6
	move.l	vb_vieww(a5),a1
	jsr	_LVOLoadView(a6)		; show inactive

	move.w	d5,d6
.ww2:	
	movem.l	d6/d7,-(a7)
	bsr	wacht_tijd2
	movem.l	(a7)+,d6/d7
	tst.w	d0
	bne	exit_kcut
	dbf	d6,.ww2
	cmp.l	#2,db_varltimes(a3)
	beq	.no_sub
	subq.w	#1,d5
.no_sub:	
	dbf	d7,.rep_kcut

exit_kcut:
	move.l	db_graphbase(a3),a6
	move.l	db_inactive_viewblok(a3),a5
	move.l	db_graphbase(a3),a6
	move.l	vb_vieww(a5),a1
	jsr	_LVOLoadView(a6)		; show inactive

	move.l	db_inactive_viewblok(a3),d0
	move.l	db_active_viewblok(a3),db_inactive_viewblok(a3)
	move.l	d0,db_active_viewblok(a3)
	bsr	set_viewed
	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)		; wait until view loaded

	rts

showdirect:
	bsr	showinactive2
	rts
*
* fade_colors fade met delta colors het active viewblok
*
fade_colors:
	tst.b		db_aa_present(a3)
	bne		fade_colors256

	move.l		db_effect_speed_fade(a3),d4
	add.l		d4,db_fade_counter(a3)

	cmp.l		#16,db_fade_counter(a3)	; ????????????????
	blt		.no_more
	move.l		#16,db_fade_counter(a3)
.no_more:
	move.l		db_waardeltacol1(a3),a2
	move.l		db_waarcolors1(a3),a6

	move.l		db_active_viewblok(a3),a5

	lea		vb_colors(a5),a0	; kleuren active screen
	moveq		#32,d0

	bsr		calc_new_cols

	move.l		vb_viewportw(a5),a0
	lea		vb_colors(a5),a1
	moveq		#32,d0				; ???????????????????
	move.l		db_graphbase(a3),a6
	jsr		_LVOLoadRGB4(a6)
no_more_fade:
	rts

*
* fade_colors fade met delta colors het active viewblok
* de 256 kleuren versie
*
fade_colors256:
	move.l		db_effect_speed_fade(a3),d4
	add.l		d4,db_fade_counter(a3)

fade_colors256_no_add:
	cmp.l		#256,db_fade_counter(a3)	; ????????????????
	blt		.fade_f
	move.l		#255,db_fade_counter(a3)
.fade_f:
	move.l		db_waardeltacol1(a3),a2
	move.l		db_waarcolors1(a3),a6
	move.l		db_active_viewblok(a3),a5

	lea		vb_colbytes(a5),a0	; kleuren active screen

	move.l		#256,d0
	bsr		add_up_delta_cols256

	move.l		#256,d0
	bsr		convert_colors

	move.l		db_waarcolors32(a3),a1
	move.l		vb_viewportw(a5),a0
	move.l		vp_ColorMap(a0),d0
	beq		no_way
	move.l		db_graphbase(a3),a6
	jsr		_LVOLoadRGB32(a6)
no_way:
no_more_fade32:
	rts

*
* tel bij de cols a6 de delta cols a2 op
* store de 4 bits RGB waarden in a0 doe dit met d0 kleuren
* de kleuren worden naar (a0) geschreven
*
add_up_delta_cols256:
	move.l	db_effect_speed_fade(a3),d4

	move.l	db_fade_counter(a3),d4
	subq.w	#1,d0
p3:

frep_kleur256:	
	moveq	#0,d1
	moveq	#0,d2

	move.w	(a6)+,d1			; R nibble source * 16
	move.w	(a2)+,d2
	ext.l	d2
	mulu	d4,d2
	add.l	d2,d1
	lsr.w	#8,d1
	move.b	d1,(a0)+

	move.w	(a6)+,d1			; G nibble source * 16
	move.w	(a2)+,d2
	ext.l	d2
	mulu	d4,d2
	add.l	d2,d1
	lsr.w	#8,d1
	move.b	d1,(a0)+

	move.w	(a6)+,d1			; B nibble source * 16
	move.w	(a2)+,d2
	ext.l	d2
	mulu	d4,d2
	add.l	d2,d1

	lsr.w	#8,d1
	move.b	d1,(a0)+

	dbf	d0,frep_kleur256
	rts
*
* tel bij de cols a6 de delta cols a2 op
* store de 4 bits RGB waarden in a0 doe dit met d0 kleuren
* de kleuren worden naar (a0) geschreven
*
add_up_delta_cols:
	subq.w	#1,d0
frep_kleur:	
	moveq	#0,d3

	move.w	(a6),d1			; R nibble source * 16
	add.w	(a2)+,d1		; R + R delta waarde * 16
	move.w	d1,(a6)+
	lsl.w	#4,d1
	move.w	d1,d3			; R nibble gezet

	move.w	(a6),d1			; G nibble source * 16
	add.w	(a2)+,d1		; G + G delta waarde * 16
	move.w	d1,(a6)+
	and.w	#$f0,d1
	move.b	d1,d3	

	move.w	(a6),d1			; B nibble source * 16
	add.w	(a2)+,d1		; B + B delta waarde * 16
	move.w	d1,(a6)+

	lsr.w	#4,d1
	or.b	d1,d3
	move.w	d3,(a0)+
	dbf	d0,frep_kleur
	rts

get_hi_tablea2:
	lea	low_hi_tabel(pc),a2
	rts

*
* The value of the fade_counter is multiplied with the delta values (a2)
* and stored in at the pointer (a0) in 4 bits RGB's
*
calc_new_cols:
	move.l	db_fade_counter(a3),d4
	cmp.l	#16,d4
	ble	no_fade_adjust
	moveq	#16,d4
no_fade_adjust:	
	subq.w	#1,d0
frep_kleur2:
	moveq	#0,d3

	move.w	(a2)+,d1		; R nibble source * 16
	ext.l	d1
	mulu	d4,d1
	add.w	(a6)+,d1		; R + R delta waarde * 16
	lsl.w	#4,d1
	move.w	d1,d3			; R nibble gezet

	move.w	(a2)+,d1		; G nibble source * 16
	ext.l	d1
	mulu	d4,d1
	add.w	(a6)+,d1		; G + G delta waarde * 16
	and.w	#$f0,d1
	move.b	d1,d3	

	move.w	(a2)+,d1		; B nibble source * 16
	mulu	d4,d1
	add.w	(a6)+,d1		; B + B delta waarde * 16

	lsr.w	#4,d1
	or.b	d1,d3
	move.w	d3,(a0)+
	dbf	d0,frep_kleur2
	rts

calc_fade_to_white_on:			; kleuren van wit
	tst.b	db_aa_present(a3)
	bne	calc_fade_to_white_on256

	move.l	db_inactive_viewblok(a3),a4
	lea	vb_colors(a4),a0	; destination kleuren
	move.l	#$fff,d7		; source kleur
	bra	init_destcols_on

calc_fade_to_white_on256:			; kleuren van wit
	move.l	db_inactive_viewblok(a3),a4
	lea	vb_colbytes(a4),a0		; destination kleuren
	move.b	#$ff,d7				; source kleur
	move.b	#$ff,d6
	move.b	#$ff,d5
	bra	init_destcols_on256

calc_fade_to_black_on:			; kleuren van zwart
	move.l	db_inactive_viewblok(a3),a4
	tst.b	db_aa_present(a3)
	bne	calc_fade_to_black_on256
	lea	vb_colors(a4),a0	; destination kleuren
	moveq	#0,d7			; source kleur
	bra	init_destcols_on

calc_fade_to_black_on256:			; kleuren van zwart
	lea	vb_colbytes(a4),a0	; destination kleuren
	moveq	#0,d5			; source kleur
	moveq	#0,d6			; source kleur
	moveq	#0,d7			; source kleur
	bra	init_destcols_on256

init_destcols_on:
	move.l	db_horizontalmask(a3),a1
	moveq	#31,d0			; 32 kleuren zetten !!!!!!!!!!!!!!
rep_colon:
	move.w	d7,(a1)+
	dbf	d0,rep_colon
	move.l	db_horizontalmask(a3),a1

	bra	calc_it2


calc_fade_to_white_off:		; kleuren naar wit
	tst.b	db_aa_present(a3)
	bne	calc_fade_to_white_off256
	move.l	#$fff,d7
	bra	init_destcols

calc_fade_to_white_off256:		; kleuren naar wit
	move.b	#$ff,d5
	move.b	#$ff,d6
	move.b	#$ff,d7
	bra	init_destcols256

calc_fade_to_black_off:		; kleuren naar zwart
	tst.b	db_aa_present(a3)
	bne	calc_fade_to_black_off256
	moveq	#0,d7

init_destcols:
	move.l	db_horizontalmask(a3),a0
	moveq	#31,d0		; initialiseer 32 kleuren !!!!!!!!!!!!!!
rep_black:
	move.w	d7,(a0)+
	dbf	d0,rep_black
	move.l	db_horizontalmask(a3),a0
	bra	calc_it

* calc_fade_colors rekent de kleuren delta's uit
* om van het active naar  naar het inactive pallete te faden
* Deze functie neemt 16 de stapjes naar het resultaat
*
calc_fade_colors:
	move.l	db_inactive_viewblok(a3),a4
calc_fade_colors_no_a4:
	tst.b	db_aa_present(a3)
	bne	calc_fade_colors256

	lea	vb_colors(a4),a0	; destination kleuren
calc_it:
	move.l	db_active_viewblok(a3),a5
	lea	vb_colors(a5),a1	; source kleuren

calc_it2:
	move.l	db_waardeltacol1(a3),a2
	move.l	db_waarcolors1(a3),a6
calc_it3:
	clr.l	db_fade_counter(a3)
	move.w	#31,d0			; altijd 32 kleuren ?????????
rep_kleur:	
	moveq	#0,d1
	moveq	#0,d2
	
	move.b	(a0),d1			; R nibble destination
	move.b	(a1),d2			; R nibble source

	lsl.w	#4,d1			; maal 16
	lsl.w	#4,d2			; maal 16
	move.w	d2,(a6)+
	sub.w	d2,d1
	asr.w	#4,d1
	move.w	d1,(a2)+		; R delta waarde / 16

	move.b	1(a0),d1		; G nibble destination
	move.b	1(a1),d2		; G nibble source
	and.w	#$f0,d1			; zijn al maal 16
	and.w	#$f0,d2
	move.w	d2,(a6)+
	sub.w	d2,d1
	asr.w	#4,d1
	move.w	d1,(a2)+		; G delta waarde / 16


	move.b	1(a0),d1			; B nibble destination
	move.b	1(a1),d2			; B nibble source
	and.w	#$f,d1
	and.w	#$f,d2

	lsl.w	#4,d1			; maal 16
	lsl.w	#4,d2			; maal 16
	move.w	d2,(a6)+
	sub.w	d2,d1
	asr.w	#4,d1
	move.w	d1,(a2)+		; B delta waarde / 16

	addq.l	#2,a0
	addq.l	#2,a1

no_col_change:
	dbf	d0,rep_kleur
	rts

init_destcols_on256:
	move.l	db_horizontalmask(a3),a1
	move.l	#255,d0			; 256 kleuren zetten !!!!!!!!!!!!!!
rep_colon256:
	move.b	d5,(a1)+
	move.b	d6,(a1)+
	move.b	d7,(a1)+
	dbf	d0,rep_colon256
	move.l	db_horizontalmask(a3),a1
	bra	calc_it256_2

calc_fade_to_black_off256:		; kleuren naar zwart
	moveq	#0,d5
	moveq	#0,d6
	moveq	#0,d7

init_destcols256:
	move.l	db_horizontalmask(a3),a0
	move.l	#255,d0		; initialiseer 256 kleuren !!!!!!!!!!!!!!
rep_black256:
	move.b	d5,(a0)+	; R
	move.b	d6,(a0)+	; G
	move.b	d7,(a0)+	; B
	dbf	d0,rep_black256
	move.l	db_horizontalmask(a3),a0
	bra	calc_it256

* calc_fade_colors rekent de kleuren delta's uit
* om van het active naar  naar het inactive pallete te faden
* Deze functie neemt 16 de stapjes naar het resultaat
*
calc_fade_colors256:

	lea	vb_colbytes(a4),a0	; destination kleuren
calc_it256:
	move.l	db_active_viewblok(a3),a5
	lea	vb_colbytes(a5),a1	; source kleuren

calc_it256_2:
	move.l	db_waardeltacol1(a3),a2
	move.l	db_waarcolors1(a3),a6
calc_it256_3:
	clr.l	db_fade_counter(a3)
	move.w	#255,d0			; altijd 256 kleuren ?????????
rep_kleur256:	
	moveq	#0,d1
	moveq	#0,d2
	
	move.b	(a0),d1			; R byte destination
	move.b	(a1),d2			; R byte source

	sub.w	d2,d1
	lsl.w	#8,d2			; maal 256
	move.w	d2,(a6)+		; store source kleuren info

	move.w	d1,(a2)+		; R delta waarde * 256
	moveq	#0,d1
	moveq	#0,d2
	move.b	1(a0),d1		; G byte destination
	move.b	1(a1),d2		; G byte source
	sub.w	d2,d1
	lsl.w	#8,d2
	move.w	d2,(a6)+
	move.w	d1,(a2)+		; G delta waarde * 256

	moveq	#0,d1
	moveq	#0,d2
	move.b	2(a0),d1		; B byte destination
	move.b	2(a1),d2		; B byte source
	sub.w	d2,d1
	lsl.w	#8,d2
	move.w	d2,(a6)+
	move.w	d1,(a2)+		; B delta waarde * 256

	addq.l	#3,a0
	addq.l	#3,a1

no_col_change256:
	dbf	d0,rep_kleur256
	rts

check_pal:
	IFEQ XAPP
	move.l	db_graphbase(a3),a6
	move.w	gb_DisplayFlags(a6),d0
	and.w	#PAL,d0
	beq	no_pal
	ELSE
	move.l	db_mlsysstruct(a3),a0
	move.l	ml_miscFlags(a0),d0
	btst	#0,d0
	beq	no_pal
	ENDC

	moveq	#1,d0
	rts
no_pal:
	moveq	#0,d0
	rts

*
* Retrieve the overscan settings
*
get_oscan_vals:
	move.l	db_intbase(a3),a6
	move.l	db_monid(a3),d0
	move.l	d0,d1
	and.l	#MONITOR_ID_MASK,d1
	cmp.l	#SUPER72_MONITOR_ID,d1
	bne	.no_mul
	or.l	#SUPER_KEY,d0
	or.l	#SUPER_KEY,db_monid(a3)
.no_mul:
	cmp.l	#VGA_MONITOR_ID,d1
	bne	.no_mul2
	or.l	#SUPER_KEY,d0
	or.l	#SUPER_KEY,db_monid(a3)
.no_mul2:
;	and.l	#MONITOR_ID_MASK,d0
	or.l	#HIRES_KEY,d0

	move.l	d0,a0
	move.l	#1,d0				//OSCAN_MAX
 	move.l	db_waarcolors32(a3),a1
	jsr	_LVOQueryOverscan(a6)
 	move.l	db_waarcolors32(a3),a1

	move.w	ra_MaxX(a1),d0
	move.w	ra_MaxY(a1),d1
	addq.w	#1,d0
	lsr.w	#1,d0
	lsr.w	#1,d1
	add.w	d0,d0
	add.w	d1,d1
	move.w	d0,db_colums(a3)
	move.l	db_monid(a3),d2
	and.w	#V_LACE,d2
	beq	.nol
	lsr.w	#1,d1
.nol:
	move.w	d1,db_rows(a3)


	move.l	db_monid(a3),d0
	or.l	#HIRES_KEY,d0
	move.l	d0,a0
	move.l	#4,d0				//OSCAN_MAX
 	move.l	db_waarcolors32(a3),a1
	jsr	_LVOQueryOverscan(a6)
 	move.l	db_waarcolors32(a3),a1

	move.w	ra_MaxY(a1),d0
	sub.w	ra_MinY(a1),d0
	add.w	d0,d0
	move.l	db_monid(a3),d2
	and.w	#V_LACE,d2
	beq	.nol2
	lsr.w	#1,d0
.nol2:
	move.w	d0,db_max_y(a3)
;	move.w	#400,db_max_y(a3)
;	move.w	#FLIP_SIZE,db_max_y(a3)

	move.w	ra_MaxX(a1),d0
	sub.w	ra_MinX(a1),d0

	add.l	#63,d0
	lsr.l	#6,d0
	lsl.l	#6,d0
	move.w	d0,db_max_x(a3)

	move.w	#0,a0
 	move.l	db_waarcolors32(a3),a1
	moveq	#mtr_SIZEOF,d0
	move.l	#DTAG_MNTR,d1

	move.l	db_monid(a3),d2

;	and.l	#MONITOR_ID_MASK,d2

	move.l	db_graphbase(a3),a6
	jsr	_LVOGetDisplayInfoData(a6)
 	move.l	db_waarcolors32(a3),a1

	move.l	mtr_Mspc(a1),d0
	move.l	mtr_Mspc(a1),db_monitorspec(a3)
	
	move.w	mtr_ViewPositionRange+2(a1),d0		; get minimal y pos
	move.w	mtr_ViewPosition+2(a1),d1		; get actual y pos
	move.w	d0,d2
	addq.w	#4,d0					; diff < 4 ?
	cmp.w	d0,d1
	bge	.minimum_oke
	move.l	d2,d1
	addq.w	#4,d1
.minimum_oke:

	move.w	mtr_ViewPosition(a1),db_viewx(a3)
	move.w	d1,db_viewy(a3)
	and.w	#$fffe,db_viewx(a3)
	and.w	#$fffe,db_viewy(a3)

	clr.b	db_Xoff(a3)
	clr.b	db_Yoff(a3)
	rts

set_lace:
	IFNE	XAPP
	move.w	#0,db_or_vmode_mask(a3)		; set depending on LACE on/off
	move.l	db_mlsysstruct(a3),a0
	move.l	ml_miscFlags(a0),d0
	btst	#4,d0
	beq	.nolace
	ENDC
	move.w	#V_LACE,db_or_vmode_mask(a3)	; set depending on LACE on/off
;	move.w	#0,db_or_vmode_mask(a3)	; set depending on LACE on/off
	rts
.nolace
	rts
*
* Set depending on monitor the LACE bits
* Data should be somewhere in the mlsystem structure
*
* For test purpose also open the monitor here
*
set_and_or_vmode:
;	move.l	db_graphbase(a3),a6
;	move.l	db_monid(a3),d0
;	move.w	#0,a1
;	jsr	_LVOOpenMonitor(a6)
;	move.l	d0,db_monitorspec(a3)

	bsr	set_lace

	move.l	db_monid(a3),d0
	move.w	#~0,db_and_vmode_mask(a3)
	move.l	d0,d1
	and.l	#DBLPAL_MONITOR_ID,d1		; what about super72 ?????
	cmp.l	#DBLPAL_MONITOR_ID,d1
	beq	.set
	move.l	d0,d1
	and.l	#DBLNTSC_MONITOR_ID,d1
	cmp.l	#DBLNTSC_MONITOR_ID,d1
	beq	.set
	move.l	d0,d1
	and.l	#VGA_MONITOR_ID,d1
	cmp.l	#VGA_MONITOR_ID,d1
	beq	.set

	move.l	d0,d1
	and.l	#SUPER72_MONITOR_ID,d1
	cmp.l	#SUPER72_MONITOR_ID,d1
	beq	.set

	move.l	d0,d1
	and.l	#EURO72_MONITOR_ID,d1
	cmp.l	#EURO72_MONITOR_ID,d1
	beq	.set

	move.l	d0,d1
	and.l	#EURO36_MONITOR_ID,d1
	cmp.l	#EURO36_MONITOR_ID,d1
	beq	.set

	rts
.set:	
	move.w	#~V_LACE,db_and_vmode_mask(a3)
	rts

setprefs:
	bsr	set_and_or_vmode
	bsr	check_pal
	move.b	d0,db_pal(a3)

	move.w	#6,db_max_Depth_Lores(a3)		; standaard instelling
	move.w	#4,db_max_Depth_Hires(a3)
	move.w	#0,db_max_Depth_SHires(a3)

	bsr	get_oscan_vals

	move.l	db_graphbase(a3),a6
	move.w	LIB_VERSION(a6),d0
	cmp.w	#LIBV39,d0
	blt	.no_v39_1
	move.b	#$ff,db_V39_present(a3)
.no_v39_1:
	cmp.w	#LIBV36,d0
	blt	.no_v36				; er is geen chiprevbits

	move.l	db_graphbase(a3),a6
	move.w	LIB_VERSION(a6),d0
	move.w	d0,db_libversion(a3)

	move.l	db_graphbase(a3),a6
	move.l	db_graphbase(a3),a6
	move.l	db_monid(a3),d0
	or.l	#HAM_KEY,d0
	jsr	_LVOFindDisplayInfo(a6)
	move.l	d0,a0
	move.l	db_waarcolors32(a3),a1
	moveq	#dim_SIZEOF,d0
	move.l	#DTAG_DIMS,d1
	moveq	#0,d2
	jsr	_LVOGetDisplayInfoData(a6)
	move.l	db_waarcolors32(a3),a1
	move.w	dim_MaxDepth(a1),d0
	move.w	d0,db_max_Depth_Lores(a3)

	move.l	db_monid(a3),d0
	or.l	#HIRES_KEY,d0
	jsr	_LVOFindDisplayInfo(a6)
	move.l	d0,a0
	move.l	db_waarcolors32(a3),a1
	moveq	#dim_SIZEOF,d0
	move.l	#DTAG_DIMS,d1
	moveq	#0,d2
	jsr	_LVOGetDisplayInfoData(a6)
	move.l	db_waarcolors32(a3),a1
	move.w	dim_MaxDepth(a1),d0
	move.w	d0,db_max_Depth_Hires(a3)

	move.l	db_monid(a3),d0
	or.l	#SUPER_KEY,d0
	jsr	_LVOFindDisplayInfo(a6)
	move.l	d0,a0
	move.l	db_waarcolors32(a3),a1
	moveq	#dim_SIZEOF,d0
	move.l	#DTAG_DIMS,d1
	moveq	#0,d2
	jsr	_LVOGetDisplayInfoData(a6)
	move.l	db_waarcolors32(a3),a1
	move.w	dim_Nominal+4(a1),d0
	cmp.w	#1000,d0
	ble	.no_shires
	move.w	dim_MaxDepth(a1),d0
	move.w	d0,db_max_Depth_SHires(a3)
.no_shires

	move.b	gb_ChipRevBits0(a6),d0
	and.b	#SETCHIPREV_AA,d0
	cmp.b	#SETCHIPREV_AA,d0
	bne	.no_aa1
	move.b	#$ff,db_aa_present(a3)
.no_aa1:

.no_v36:
	move.w	#4,db_gapsize(a3)
	tst.b	db_aa_present(a3)
	beq	no_aa101
	move.w	#20,db_gapsize(a3)
no_aa101:
	lea	cpp_data(pc),a0
	move.w	#9,cpp_size(a0)		; set nr. off coplist on big cpu
	move.l	$4.w,a6
	move.w	AttnFlags(a6),d0
	and.l	#AFF_68030,d0
	bne	.no_large_cpu
	move.w	#29,cpp_size(a0)
.no_large_cpu:
	rts
*
* maak het frame van de inactive view buffer schoon
* Het is natuurlijk alleen maar nodig het gebruikte deel schoon te maken ??????
*
clear_inactive_chipmem:

	move.l	db_inactive_viewblok(a3),a5
	move.l	vb_tempbuffer(a5),a0

	IFNE XAPP
	move.l	db_mlsysstruct(a3),a1
	move.l	sbu_Size(a1),d0
	ELSE
	move.l	#MEMSIZE,d0
	ENDC
clear_active_chipmem:
	subq.l	#4,d0		; vanwege de negblit
	lsr.l	#2,d0
	moveq	#0,d1
rep_clear:
	move.l	d1,(a0)+
	subq.l	#1,d0
	bne	rep_clear
	rts

* set_fade_teler zet de colorstep var
* in d0 de breedte in d1 het aantal stappen waarin deze breedte genomen wordt
*
set_fade_teller:
	movem.l	d0/d1,-(a7)
	tst.w	d1
	beq	no_div1		; DIV zero test ?????????????
	divu	d1,d0
no_div1:
	move.l	#16,db_effect_speed_fade(a3)
	tst.b	db_aa_present(a3)
	beq	no_aa6

	lsr.w	#6,d0
	move.l	#4,db_effect_speed_fade(a3)
	tst.w	d0
	bne	no_addsft
	moveq	#1,d0
	bra	no_addsft

no_aa6:
	move.l	#1,db_effect_speed_fade(a3)
	lsr.w	#4,d0				; delen door zestien
go_aa6:
	subq	#1,d0
	bpl	no_addsft
	move.l	#2,db_effect_speed_fade(a3)
add_one1:
	moveq	#1,d0
no_addsft:
	beq	add_one1

	move.w	d0,db_colorstep(a3)
	move.w	d0,db_colorteller(a3)
	movem.l	(a7)+,d0/d1

	move.w	#$ff,d3
	tst.b	db_aa_present(a3)
	bne	.aa
	moveq	#16,d3
.aa

	divu	d1,d0				; number of times you pass
	tst.w	d0
	bne	.pl
	moveq	#1,d0
.pl:	
	and.l	#$ffff,d0
.rep_up:
	move.l	db_effect_speed_fade(a3),d1
	mulu	d0,d1				; should be bigger than 256 ??
	cmp.w	d3,d1
	bge	.oke_do
	addq.l	#1,db_effect_speed_fade(a3)
	bra	.rep_up
.oke_do:
	rts

* test_fade kijkt of er een fade uitgevoerd moet worden
*
test_fade:

	movem.l	d7/d6,-(a7)
	subq.w	#1,db_colorteller(a3)
	bne	no_fade4
	move.w	db_colorstep(a3),db_colorteller(a3)	; restore teller
	bsr	fade_colors
no_fade4:
	movem.l	(a7)+,d7/d6
	rts

fade_to_zero_active:
	tst.b	db_aa_present(a3)
	bne	fade_to_zero_active256

	move.l	db_active_viewblok(a3),a5
	move.w	vb_colors(a5),d7	; de kleur waarnaar te faden
	bra	fade_f_in

fade_to_zero_active256:
	move.l	db_active_viewblok(a3),a5
	move.b	vb_colbytes(a5),d5
	move.b	vb_colbytes+1(a5),d6
	move.b	vb_colbytes+2(a5),d7
	bra	fade_to_f256_in	
*
* fade_to_first_active fade naar de eerste kleur uit het active pallette
*
fade_to_first_active:
	tst.b	db_aa_present(a3)
	bne	fade_to_first_active256
	
	move.l	db_active_viewblok(a3),a5
	move.w	vb_colors+2(a5),d7	; de kleur waarnaar te faden
fade_f_in:
	move.l	d7,-(a7)
	bsr	init_destcols		; fade naar d7
	bsr	color_wipe		; active nu kleur black
	move.l	(a7)+,d7

	move.l	d7,-(a7)
	move.l	db_inactive_viewblok(a3),a4
	lea	vb_colors(a4),a0	; destination kleuren
	bsr	init_destcols_on
	move.l	(a7)+,d7

	move.l	db_inactive_viewblok(a3),a4
	lea	vb_colors(a4),a0	; maak inactive ook first color
	moveq	#31,d0			; altijd 32 kleuren
rep_bcol1_fa:
	move.w	d7,(a0)+
	dbf	d0,rep_bcol1_fa

	bsr	showinactive		; met alle kleuren zwart
	bsr	color_wipe		; wipe kleuren naar normaal
	rts

fade_to_first_active256:
	move.l	db_active_viewblok(a3),a5
	move.b	vb_colbytes+3(a5),d5
	move.b	vb_colbytes+4(a5),d6
	move.b	vb_colbytes+5(a5),d7

fade_to_f256_in:
	movem.l	d5-d7,-(a7)
	bsr	init_destcols256	; fade naar d5,d6,d7
	bsr	color_wipe		; active nu kleur black
	movem.l	(a7)+,d5-d7

	movem.l	d5-d7,-(a7)
	move.l	db_inactive_viewblok(a3),a4
	lea	vb_colbytes(a4),a0	; destination kleuren
	bsr	init_destcols_on256
	movem.l	(a7)+,d5-d7

	move.l	db_inactive_viewblok(a3),a4
	lea	vb_colbytes(a4),a0	; maak inactive ook first color
	move.l	#255,d0			; altijd 256 kleuren
rep_bcol1_fa256:
	move.b	d5,(a0)+
	move.b	d6,(a0)+
	move.b	d7,(a0)+
	dbf	d0,rep_bcol1_fa256

	bsr	showinactive		; met alle kleuren zwart
	bsr	color_wipe		; wipe kleuren naar normaal
	rts


fade_to_zero_inactive:
	move.l	db_inactive_viewblok(a3),a4
	tst.b	db_aa_present(a3)
	bne	fade_to_zero_inactive256
	move.w	vb_colors(a4),d7	; de kleur waarnaar te faden
	bra	fade_to_z_in
	
fade_to_zero_inactive256:
	move.l	db_inactive_viewblok(a3),a4
	move.b	vb_colbytes(a4),d5	; de kleur waarnaar te faden
	move.b	vb_colbytes+1(a4),d6
	move.b	vb_colbytes+2(a4),d7
	bra	fade_to_z256_in

* fade_to_first_inactive fade naar de eerste kleur uit het inactive pallette
*
fade_to_first_inactive:
	move.l	db_inactive_viewblok(a3),a4
	tst.b	db_aa_present(a3)
	bne	fade_to_first_inactive256
	move.w	vb_colors+2(a4),d7	; de kleur waarnaar te faden

fade_to_z_in:	
	move.l	d7,-(a7)
	bsr	init_destcols		; fade naar d7
	bsr	color_wipe		; active nu kleur black
	move.l	(a7)+,d7

	move.l	d7,-(a7)
	move.l	db_inactive_viewblok(a3),a4
	lea	vb_colors(a4),a0	; destination kleuren
	bsr	init_destcols_on
	move.l	(a7)+,d7

	move.l	db_inactive_viewblok(a3),a4
	lea	vb_colors(a4),a0	; maak inactive ook first color

	moveq	#31,d0			; altijd 32 kleuren
rep_bcol1_fi:
	move.w	d7,(a0)+
	dbf	d0,rep_bcol1_fi	

	bsr	showinactive		; met alle kleuren zwart
	bsr	color_wipe		; wipe kleuren naar normaal
	rts

fade_to_first_inactive256:
	move.l	db_inactive_viewblok(a3),a4
	move.b	vb_colbytes+3(a4),d5	; de kleur waarnaar te faden
	move.b	vb_colbytes+4(a4),d6
	move.b	vb_colbytes+5(a4),d7

fade_to_z256_in:
	movem.l	d5-d7,-(a7)
	bsr	init_destcols256	; fade naar d7
	bsr	color_wipe		; active nu kleur black
	movem.l	(a7)+,d5-d7

	movem.l	d5-d7,-(a7)
	move.l	db_inactive_viewblok(a3),a4
	lea	vb_colbytes(a4),a0	; destination kleuren
	bsr	init_destcols_on256
	movem.l	(a7)+,d5-d7

	move.l	db_inactive_viewblok(a3),a4
	lea	vb_colbytes(a4),a0	; maak inactive ook first color

	move.l	#255,d0			; altijd 32 kleuren
rep_bcol1_fi256:
	move.b	d5,(a0)+
	move.b	d6,(a0)+
	move.b	d7,(a0)+
	dbf	d0,rep_bcol1_fi256

	bsr	showinactive		; met alle kleuren zwart
	bsr	color_wipe		; wipe kleuren naar normaal
	rts

* fade_to_black
*
fade_to_black:
	bsr	calc_fade_to_black_off
	bsr	color_wipe		; active nu kleur black
	bsr	calc_fade_to_black_on
	move.l	db_inactive_viewblok(a3),a4
	tst.b	db_aa_present(a3)
	bne	fade_to_black256

	lea	vb_colors(a4),a0	; maak inactive ook zwart
	moveq	#0,d7
	moveq	#31,d0			; altijd 32 kleuren
rep_bcol1:
	move.w	d7,(a0)+
	dbf	d0,rep_bcol1

	bsr	showinactive		; met alle kleuren zwart
	bsr	color_wipe		; wipe kleuren naar normaal
	rts

fade_to_black256:
	lea	vb_colbytes(a4),a0	; maak inactive ook zwart
	moveq	#0,d7
	move.l	#255,d0			; altijd 32 kleuren
rep_bcol256_1:
	move.b	d7,(a0)+
	move.b	d7,(a0)+
	move.b	d7,(a0)+
	dbf	d0,rep_bcol256_1

	bsr	showinactive		; met alle kleuren zwart
	bsr	color_wipe		; wipe kleuren naar normaal
	rts
*
* fade_to_white
*
fade_to_white:

	bsr	calc_fade_to_white_off

	bsr	color_wipe		; active nu kleur white
	tst.w	d0
	bne	no_f_w
	
	bsr	calc_fade_to_white_on
	tst.b	db_aa_present(a3)
	bne	fade_to_white256

	move.l	db_inactive_viewblok(a3),a4
	lea	vb_colors(a4),a0	; maak inactive ook wit
	move.l	#$fff,d7
	moveq	#31,d0			; altijd 32 kleuren
rep_wcol1:
	move.w	d7,(a0)+
	dbf	d0,rep_wcol1	

	bsr	showinactive		; met alle kleuren wit
	bsr	color_wipe		; wipe kleuren naar normaal
	rts
no_f_w:
	bsr	showinactive		; met alle kleuren wit
	moveq	#0,d0
	rts
	
fade_to_white256:
	move.l	db_inactive_viewblok(a3),a4
	lea	vb_colbytes(a4),a0	; maak inactive ook wit
	move.l	#$fff,d7
	move.l	#255,d0			; altijd 255 kleuren
rep_wcol256_1:
	move.b	#$ff,(a0)+
	move.b	#$ff,(a0)+
	move.b	#$ff,(a0)+
	dbf	d0,rep_wcol256_1

	bsr	showinactive		; met alle kleuren wit
	bsr	color_wipe		; wipe kleuren naar normaal
	rts

color_wipe:
	tst.b	db_aa_present(a3)
	bne	color_wipe256
	
	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4		; source
wwfw:
	bsr	fade_colors
	cmp.l	#16,db_fade_counter(a3)
	bge	exit_fadew

	bsr	wacht_tijd2
	bne	exit_fadew2
	bra.b	wwfw
exit_fadew:
	moveq	#0,d0
	rts	
exit_fadew2:
	moveq	#-1,d0
	rts	

color_wipe256:
	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4		; source
wwfw256:
	bsr	fade_colors256

	cmp.l	#255,db_fade_counter(a3)
	bge	exit_fadew256

	bsr	wacht_tijd2
	bne	exit_fadew256_2
	bra.b	wwfw256
exit_fadew256:
	moveq	#0,d0
	rts
exit_fadew256_2:
	moveq	#-1,d0
	rts	

blockvar:
	cmp.l	#0,db_variation(a3)
	beq	blocks0

	move.l	db_variation(a3),d0
	and.l	#$3,d0
	move.l	d0,db_lastline(a3)
	move.l	db_variation(a3),d0
	lsr.l	#2,d0
	lea	blockmoves1,a0
	cmp.w	#0,d0
	beq	test_block_blit
	lea	blockmoves2,a0
	cmp.w	#1,d0
	beq	test_block_blit
	lea	blockmoves3,a0
	cmp.w	#2,d0
	beq	test_block_blit
	lea	blockmoves4,a0
	cmp.w	#3,d0
	beq	test_block_blit
	lea	blockmoves5,a0
	cmp.w	#4,d0
	beq	test_block_blit
	lea	blockmoves6,a0
	cmp.w	#5,d0
	beq	test_block_blit
	lea	blockmoves7,a0
	cmp.w	#6,d0
	beq	test_block_blit
	lea	blockmoves8,a0
	cmp.w	#7,d0
	beq	test_block_blit
	lea	blockmoves9,a0
	cmp.w	#8,d0
	beq	test_block_blit
	lea	blockmoves10,a0
	cmp.w	#9,d0
	beq	test_block_blit
	lea	blockmoves11,a0
	cmp.w	#10,d0
	beq	test_block_blit
*
* Block functies met verschillende masks
* block functies kunnen ook verschillende opkom patronen hebben ???????? 
*
blocks0:
	moveq	#2,d1
	lea	blockmoves9,a0			; bubble
	bra	test_block_blit
blocks1:
	moveq	#0,d1
	lea	blockmoves1,a0		; blocks1
	bra	test_block_blit
blocks2:
	moveq	#1,d1
	lea	blockmoves1,a0		; blocks2
	bra	test_block_blit
blocks3:
	moveq	#0,d1
	lea	blockmoves2,a0		; spiral
	bra	test_block_blit
blocks4:
	moveq	#0,d1
	lea	blockmoves3,a0		; Narrow arrows
	bra	test_block_blit
blocks5:
	moveq	#1,d1
	lea	blockmoves3,a0		; arrow vertical
	bra	test_block_blit
blocks6:
	moveq	#0,d1
	lea	blockmoves4,a0		; checker down/up
	bra	test_block_blit
blocks7:
	moveq	#2,d1
	lea	blockmoves4,a0		; checker left/right
	bra	test_block_blit
blocks8:
	moveq	#0,d1
	lea	blockmoves6,a0		; blocks3
	bra	test_block_blit
blocks9:
	moveq	#0,d1
	lea	blockmoves7,a0		; blocks4
	bra	test_block_blit
blocks10:
	moveq	#0,d1
	lea	blockmoves8,a0		; blocks5
	bra	test_block_blit
blocks11:
	moveq	#2,d1
	lea	blockmoves10,a0		; spiral 2
	bra	test_block_blit

do_block_blit:
	move.l	d1,db_lastline(a3)
	move.l	a1,db_blockpointer(a3)

	bra	start_block_blit
	
test_block_blit:
	move.l	d1,db_lastline(a3)

	tst.l	db_triple_mem(a3)
	beq	exit_block1

	move.l	a0,db_blockpointer(a3)
	move.l	db_varltimes(a3),d0		; variatie

	lea	draw_mask5(pc),a0

	cmp.b	#0,d0
	bne	.no_m0
	lea	draw_mask5(pc),a0
.no_m0:
	cmp.b	#1,d0
	bne	.no_m1
	lea	draw_mask4(pc),a0
.no_m1:
	cmp.b	#2,d0
	bne	.no_m2
	lea	draw_mask3(pc),a0
.no_m2:
	cmp.b	#3,d0
	bne	.no_m3
	lea	draw_mask2(pc),a0
.no_m3:
	cmp.b	#4,d0
	bne	.no_m4
	lea	draw_mask1(pc),a0
.no_m4:
	cmp.b	#5,d0
	bne	.no_m5
	lea	draw_mask6(pc),a0
.no_m5:
	cmp.b	#6,d0
	bne	.no_m6
	lea	draw_mask7(pc),a0
.no_m6:
	cmp.b	#7,d0
	bne	.no_m7
	lea	draw_mask8(pc),a0
.no_m7:
	cmp.b	#8,d0
	bne	.no_m8
	lea	draw_mask9(pc),a0
.no_m8:
	cmp.b	#9,d0
	bne	.no_m9
	lea	draw_mask10(pc),a0
.no_m9:

start_block_blit:
	lea	welkmask(pc),a1
	move.l	a0,(a1)

	move.l	db_effect_speed(a3),d0

	moveq	#4,d2
	moveq	#21,d1
	cmp.w	#4,d0
	bge	.nosl
	add.w	#20,d1
	add.w	d0,d0
.nosl:
	sub.w	d0,d1
no_bl_pro:

	move.l	d2,db_varltimes(a3)
	move.l	d2,db_counttimer(a3)

	move.l	d1,db_effect_speed(a3)
	move.l	#0,db_varwtime(a3)
	lea	tellerstart(pc),a0
	move.l	#0,(a0)

	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	move.l	db_effect_speed(a3),d0	; ruimte voor effect_speed pointers
	move.l	d0,d1
	add.l	d0,d0
	add.l	d1,d0
	bsr	clear_move_storage

; set fade teller

	move.l	#16*16,d0
	moveq	#1,d1
	bsr	set_fade_teller

; reken uit hoe groot de blokken moet zijn 
; om standaard reden zijn er zowel in breedte als in lengte richting 16 blokken
; Dit kunnen er natuurlijk ook meer zijn. ?????????????? ( 32 )

	moveq	#0,d2
	move.w	vb_lenx(a5),d2
	add.w	#15,d2
	lsr.l	#4,d2
	add.l	d2,d2

;	move.l	vb_breedte_x(a5),d2		; aantal bytes als uitgang
						; immers altijd even ??????
	move.l	d2,d4
	lsr.l	#1,d4				; 16 de deel van de active view

	move.l	d4,db_blockbreedte(a3)

	moveq	#0,d3
	move.w	vb_leny(a5),d3
	add.w	#15,d3			; zeker alle data blitten ???
	move.l	d3,d4
	lsr.w	#4,d4			; gaat uiteraard fout ???????
	move.l	d4,db_blockhoogte(a3)

	move.l	db_effect_speed(a3),d0

	lsl.l	#4,d3
	lsl.l	#7,d2

	divu	d0,d3			; stappen in y richting * 2^16
	divu	d0,d2			; stappen in x richting * 2^16

	and.l	#$ffff,d3
	and.l	#$ffff,d2
	lsl.l	#8,d3
	lsl.l	#8,d2

	move.l	d2,db_blockdx(a3)
	move.l	d3,db_blockdy(a3)

	move.l	#$14,db_minbloklb(a3)		; for now

	bsr	maak_block_mask2
	bne	exit_block1

	move.l	db_effect_speed(a3),db_minbloklb(a3)		; for now
	addq.l	#1,db_minbloklb(a3)

	moveq	#0,d0
	moveq	#0,d1
	move.l	db_blockbreedte(a3),d2
	move.l	db_blockhoogte(a3),d3
rep_blok_h1:
	movem.l	d0-d7,-(a7)	; save alle D regs ??????????????

	bsr	update_shifts4

	bsr	wacht_tijd

	movem.l	(a7)+,d0-d7
	bne	exit_block1

	tst.b	db_blit_done(a3)
	beq	exit_block1

	bra	rep_blok_h1

exit_block1:
	move.l	db_blockmem(a3),a0
	add.l	db_blockmem_size(a3),a0

	move.l	db_blockmem(a3),d0
	beq	no_free_blocks
	move.l	d0,a1
	move.l	db_blockmem_size(a3),d0
	move.l	db_mlmmubase(a3),a6
	jsr	_LVOMLMMU_FreeMem(a6)
no_free_blocks:
	rts

adaptfile_pcj:
	bsr	adaptfile
	rts

maak_small_layer:
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	db_layersbase(a3),a6
	moveq	#0,d0
	jsr	_LVONewLayerInfo(a6)
	lea	new_layer_pointer(pc),a0
	move.l	d0,(a0)
	beq	no_info

	lea.l	mask_bitmap(pc),a1
	move.l	d0,a0

	moveq	#0,d0
	moveq	#0,d1
	move.l	db_blockbreedte(a3),d2
	move.l	db_hoogtemask(a3),d3
	subq.l	#1,d2
	move.w	#0,a2
	moveq	#LAYERSIMPLE,d4			; flags
	jsr	_LVOCreateUpfrontLayer(a6)
	lea	mask_layerp(pc),a0
	move.l	d0,(a0)
	beq	no_layer
	move.l	d0,a0

	lea	layer_rastport(pc),a1
	move.l	lr_rp(a0),(a1)
	movem.l	(a7)+,d0-d7/a0-a6
	moveq	#0,d0
	rts

del_layer:
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	db_layersbase(a3),a6
	move.w	#0,a0
	move.l	mask_layerp(pc),a1
	cmp.l	#0,a1
	beq	no_layer
	jsr	_LVODeleteLayer(a6)
no_layer:
	move.l	new_layer_pointer(pc),a0
	beq	no_info
	jsr	_LVODisposeLayerInfo(a6)
no_info:
	movem.l	(a7)+,d0-d7/a0-a6
	moveq	#-1,d0
	rts

* maak een klein rastportje met lengte breedtemask* planes en hoogte hoogtemask
*
maak_small_rastport:
	movem.l	d0-d1/a0-a1,-(a7)
	move.l	db_graphbase(a3),a6
	move.l	db_blockbreedte(a3),d1
	add.l	#16,d1

	move.l	db_hoogtemask(a3),d2
	moveq	#1,d0
	lea	mask_bitmap(pc),a0
	jsr	_LVOInitBitMap(a6)
	movem.l	(a7)+,d0-d1/a0-a1
	rts

clean_mask:
	movem.l	d0-d1/a0-a1,-(a7)
	move.l	db_graphbase(a3),a6
	lea	mask_bitmap(pc),a0
	move.l	layer_rastport(pc),a1
	jsr	_LVOClearScreen(a6)
	movem.l	(a7)+,d0-d1/a0-a1
	rts
*
* maak het bitmapje schoon en zet a0 als bitmap pointer
*
clear_mask:
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	db_sizemask(a3),d0
	lsr.l	#2,d0
	subq.l	#1,d0
	moveq	#0,d1
	move.l	a0,a6
rep_clrrast:
	move.l	d1,(a6)+
	dbf	d0,rep_clrrast	

	move.l	db_graphbase(a3),a6	
	lea	mask_bitmap(pc),a1
	move.l	a0,bm_Planes(a1)

	movem.l	(a7)+,d0-d7/a0-a6
	rts

fill_bitmask:
	movem.l	d0-d3/a0-a1,-(a7)
	move.l	db_blockhoogte(a3),d3
	move.l	db_blockbreedte(a3),d2
	moveq	#0,d0
	moveq	#0,d1
	move.l	layer_rastport(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVORectFill(a6)
	movem.l	(a7)+,d0-d3/a0-a1
	rts
	
draw_mask1:
	movem.l	d0-d3/a0-a1,-(a7)
	move.l	d1,d3
	moveq	#0,d0
	moveq	#0,d1
	move.l	layer_rastport(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVORectFill(a6)
	movem.l	(a7)+,d0-d3/a0-a1
	rts

draw_mask2:
	movem.l	d0-d3/a0-a1,-(a7)
	move.l	d1,d3
	moveq	#0,d1
	move.l	db_blockbreedte(a3),d0
	sub.l	d2,d0
	move.l	db_blockbreedte(a3),d2
	move.l	layer_rastport(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVORectFill(a6)
	movem.l	(a7)+,d0-d3/a0-a1
	rts

draw_mask3:
	movem.l	d0-d3/a0-a1,-(a7)
	move.l	d1,d3
	move.l	db_blockbreedte(a3),d0
	sub.l	d2,d0
	move.l	db_blockhoogte(a3),d1
	sub.l	d3,d1
	move.l	db_blockbreedte(a3),d2
	move.l	db_blockhoogte(a3),d3
	move.l	layer_rastport(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVORectFill(a6)
	movem.l	(a7)+,d0-d3/a0-a1
	rts

draw_mask4:
	movem.l	d0-d3/a0-a1,-(a7)
	move.l	d1,d3
	moveq	#0,d0
	move.l	db_blockhoogte(a3),d1
	sub.l	d3,d1
	move.l	db_blockhoogte(a3),d3
	move.l	layer_rastport(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVORectFill(a6)
	movem.l	(a7)+,d0-d3/a0-a1
	rts

draw_mask5:
	movem.l	d0-d5/a0-a1,-(a7)
	exg	d1,d2
	move.l	d1,d4				; y coordinaat
	move.l	d2,d5				; x coordinaat
	asr.l	#1,d4
	asr.l	#1,d5

	move.l	db_blockbreedte(a3),d0
	asr.l	#1,d0				; midden x bitmapje
	move.l	d0,d2
	add.l	d4,d2
	sub.l	d4,d0
	move.l	db_blockhoogte(a3),d1
	asr.l	#1,d1				; midden y
	move.l	d1,d3
	sub.l	d5,d1
	add.l	d5,d3
	move.l	layer_rastport(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVORectFill(a6)
	movem.l	(a7)+,d0-d5/a0-a1
	rts

draw_mask6:
	movem.l	d0-d5/a0-a1,-(a7)
	exg	d1,d2
	move.l	d2,d5				; x coordinaat
	asr.l	#1,d5

	move.l	db_blockbreedte(a3),d2
	moveq	#0,d0
	
	move.l	db_blockhoogte(a3),d1
	asr.l	#1,d1				; midden y
	move.l	d1,d3
	sub.l	d5,d1
	add.l	d5,d3

	move.l	layer_rastport(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVORectFill(a6)
	movem.l	(a7)+,d0-d5/a0-a1
	rts

draw_mask7:
	movem.l	d0-d5/a0-a1,-(a7)
	exg	d1,d2
	move.l	d1,d4
	asr.l	#1,d4

	move.l	db_blockbreedte(a3),d0
	asr.l	#1,d0				; midden x bitmapje
	move.l	d0,d2
	add.l	d4,d2
	sub.l	d4,d0
	move.l	db_blockhoogte(a3),d3
	moveq	#0,d1
	move.l	layer_rastport(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVORectFill(a6)
	movem.l	(a7)+,d0-d5/a0-a1
	rts

draw_mask8:
	movem.l	d0-d7/a0-a6,-(a7)
rep_drawmc5:
	move.l	layer_rastport(pc),a1
	move.l	d1,d3
	move.l	db_graphbase(a3),a6
	move.l	db_hoogtemask(a3),d1
	lsr.l	#1,d1
	move.l	db_blockbreedte(a3),d0
	lsr.l	#1,d0
	move.l	db_graphbase(a3),a6
	jsr	_LVOAreaEllipse(a6)
	move.l	layer_rastport(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVOAreaEnd(a6)
	movem.l	(a7)+,d0-d7/a0-a6
	rts

draw_mask9:
	movem.l	d0-d7/a0-a6,-(a7)
	movem.l	d1/d2,-(a7)
	move.l	d1,d7
	add.l	d7,d7
	move.l	d2,d6
	add.l	d6,d6
	add.l	d6,d7				; total pixels round

	move.l	layer_rastport(pc),a1
	move.l	db_graphbase(a3),a6
	move.l	db_hoogtemask(a3),d1
	move.l	d1,d4				; d4 holds total height
	lsr.l	#1,d1
	move.l	db_blockbreedte(a3),d0
	move.l	d0,d3				; d3 holds total width
	lsr.l	#1,d0
	move.l	db_graphbase(a3),a6
	jsr	_LVOAreaMove(a6)		; to middle

	moveq	#0,d0
	moveq	#0,d1
	move.l	layer_rastport(pc),a1
	jsr	_LVOAreaDraw(a6)		; to first corner

	move.l	d3,d0				; set to net corner
	moveq	#0,d1
	cmp.l	d7,d3
	ble	.c1
	move.l	d7,d0				; until here
.c1:
	move.l	layer_rastport(pc),a1
	jsr	_LVOAreaDraw(a6)		; to first corner

	cmp.l	d7,d3
	bge	.ready
	sub.l	d3,d7

	move.l	d3,d0
	move.l	d4,d1
	cmp.l	d7,d1
	ble	.c2
	move.l	d7,d1
.c2:
	move.l	layer_rastport(pc),a1
	jsr	_LVOAreaDraw(a6)		; to second corner

	cmp.l	d7,d4
	bge	.ready
	sub.l	d4,d7

	move.l	d4,d1
	moveq	#0,d0
	cmp.l	d7,d3
	ble	.c3
	move.l	d3,d0
	sub.l	d7,d0
.c3:
	move.l	layer_rastport(pc),a1
	jsr	_LVOAreaDraw(a6)		; to second corner

	cmp.l	d7,d3
	bge	.ready
	sub.l	d3,d7
	moveq	#0,d0
	move.l	d4,d1
	sub.l	d7,d1
	move.l	layer_rastport(pc),a1
	jsr	_LVOAreaDraw(a6)		; to second corner
.ready:
	move.l	layer_rastport(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVOAreaEnd(a6)

	movem.l	(a7)+,d1/d2
	movem.l	(a7)+,d0-d7/a0-a6
	rts

draw_mask10:
	movem.l	d0-d5/a0/a1/a6,-(a7)
	move.l	d1,d3
	move.l	db_graphbase(a3),a6
	move.l	db_hoogtemask(a3),d1
	lsr.l	#1,d1
	move.l	d1,d5				; d5 holds middle y
	move.l	db_blockbreedte(a3),d0
	lsr.l	#1,d0
	move.l	d0,d4				; d4 holds middle x
	move.l	db_graphbase(a3),a6
	sub.l	d3,d1
	move.l	layer_rastport(pc),a1
	jsr	_LVOAreaMove(a6)

	move.l	d4,d0
	move.l	d5,d1
	add.l	d2,d0
	move.l	layer_rastport(pc),a1
	jsr	_LVOAreaDraw(a6)

	move.l	d4,d0
	move.l	d5,d1
	add.l	d3,d1
	move.l	layer_rastport(pc),a1
	jsr	_LVOAreaDraw(a6)

	move.l	d4,d0
	move.l	d5,d1
	sub.l	d2,d0
	move.l	layer_rastport(pc),a1
	jsr	_LVOAreaDraw(a6)

	move.l	layer_rastport(pc),a1
	move.l	db_graphbase(a3),a6
	jsr	_LVOAreaEnd(a6)

	movem.l	(a7)+,d0-d5/a0/a1/a6
	rts


TMPSIZE = 10000

init_areainfo:
	movem.l	d0-d2/a0-a6,-(a7)
	move.l	db_chipscrap_mem(a3),a0
	move.l	db_chipscrap_size(a3),d0
	lsr.l	#2,d0
.cl:
	clr.l	(a0)+
	subq.l	#1,d0
	bne	.cl

	move.l	db_graphbase(a3),a6
	lea	ainfo(pc),a0
	move.l	db_wpatroonruimte(a3),a1
	move.l	#80,d0
	jsr	_LVOInitArea(a6)

	move.l	layer_rastport(pc),a1
	lea	ainfo(pc),a0
	move.l	a0,rp_AreaInfo(a1)

	lea	tmpras(pc),a0
	move.l	db_chipscrap_mem(a3),a1
	move.l	db_chipscrap_size(a3),d0
	jsr	_LVOInitTmpRas(a6)

	move.l	layer_rastport(pc),a1
	lea	tmpras(pc),a0
	move.l	a0,rp_TmpRas(a1)	
.err:
	movem.l	(a7)+,d0-d2/a0-a6
	rts

ainfo:			blk.b	ai_SIZEOF,0
tmpras:			blk.b	tr_SIZEOF,0
mask_bitmap:		ds.b	bm_SIZEOF

mask_layerp:		dc.l	0
new_layer_pointer:	dc.l	0
layer_rastport:		dc.l	0

*
* maak_block_mask maakt de maskers voor de block_blit functie
* maak een masker met een klein rastportje ????????????
*
maak_block_mask2:
	move.l	db_blockbreedte(a3),d2
	move.l	db_blockbreedte(a3),db_breedtemaskx(a3)	

	add.l	#16,d2

	add.l	#15,d2
	lsr.l	#4,d2			; breedte blit in words 
	add.l	d2,d2			; in bytes
	move.l	d2,db_breedtemask(a3)

	moveq	#0,d4
	move.w	vb_planes(a5),d4
	mulu	d4,d2			; totale breedte

	move.l	db_blockhoogte(a3),d3
	mulu	d3,d2			; totale size van de rastport
	move.l	d2,db_sizemask(a3)
	move.l	db_blockhoogte(a3),db_hoogtemask(a3)	

	move.l	db_effect_speed(a3),d3
	addq.l	#2,d3
	mulu	d3,d2				; should be size of chipmem
	add.l	#12,d2
	move.l	d2,db_blockmem_size(a3)
ttt:

	move.l	db_mlmmubase(a3),a6
	lea.l	blockname(pc),a1		; name of the chipmem chunk
	moveq	#MEMF_CHIP,d1
	jsr	_LVOMLMMU_FindMemBlk(a6)
	move.l	d0,db_blockmem(a3)
	beq	block_not_ready

;	bra	block_not_ready

	move.l	d0,a1
	move.l	db_blockbreedte(a3),d2
	cmp.w	(a1),d2
	bne	block_wrong_dims
	move.l	db_blockhoogte(a3),d2
	cmp.w	2(a1),d2
	bne	block_wrong_dims
	move.l	welkmask(pc),d2
	cmp.l	8(a1),d2
	bne	block_wrong_dims
	move.l	db_sizemask(a3),d2
	cmp.l	4(a1),d2
	bne	block_wrong_dims

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Using same blocks",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC


	add.l	#12,d0
	move.l	d0,db_where_blocks(a3)

	move.l	db_effect_speed(a3),d3
	subq.w	#1,d3

	move.l	db_wpatroon_tabel(a3),a1	; sla hier de pointers op
	move.l	db_where_blocks(a3),a0

rep_easy_block:
	add.l	db_sizemask(a3),a0		; te groot ? voor de eenvoud
	move.l	a0,(a1)+
	dbf	d3,rep_easy_block
	bra	easy_block1

block_wrong_dims:
	jsr	_LVOMLMMU_FreeMem(a6)		; free the block and go on

block_not_ready:
	move.l	db_mlmmubase(a3),a6
	move.l	db_blockmem_size(a3),d0
	move.l	d0,db_blockmem_size(a3)
	moveq	#MEMF_CHIP,d1
	or.l	#MEMF_STAY,d1
	lea	blockname(pc),a1
	jsr	_LVOMLMMU_AllocMem(a6)
	move.l	d0,db_blockmem(a3)
	beq	block_nomem

	move.l	d0,a0
	move.w	db_blockbreedte+2(a3),(a0)+
	move.w	db_blockhoogte+2(a3),(a0)+
	move.l	db_sizemask(a3),(a0)+
	move.l	welkmask(pc),(a0)+
	add.l	#12,d0
	move.l	d0,db_where_blocks(a3)

	move.l	db_blockhoogte(a3),db_hoogtemask(a3)	
	bsr	maak_small_rastport

	move.l	db_effect_speed(a3),d3
	move.l	db_sizemask(a3),d2
	subq.w	#1,d3

	move.l	db_wpatroon_tabel(a3),a1	; sla hier de pointers op

	move.l	db_where_blocks(a3),a0

	bsr	clear_mask

	bsr	maak_small_layer
	tst.l	d0
	bne	exit
	
	bsr	init_areainfo

	add.l	db_sizemask(a3),a0		; te groot ? voor de eenvoud

	moveq	#0,d2
	moveq	#0,d1
rep_maak_block_masks:
	add.l	db_blockdx(a3),d2
	add.l	db_blockdy(a3),d1

	movem.l	d1/d2,-(a7)
	swap	d2
	and.l	#$ffff,d2
	swap	d1
	and.l	#$ffff,d1
	move.l	a0,(a1)+		; store buffer pointer
	bsr	clean_mask		; leeg de bitmap
	lea	welkmask(pc),a6
	move.l	(a6),a6
	jsr	(a6)			; zet wat in de bitmap
	bsr	copy_bitmask
	movem.l	(a7)+,d1/d2
	dbf	d3,rep_maak_block_masks

no_more_block_masks:
	move.l	-4(a1),a0		; de laatste is altijd vol
	bsr	fill_bitmask
	bsr	copy_bitmask
	bsr	del_layer
easy_block1:
	moveq	#0,d0
	rts

blockname:	dc.b	"BlockMem",0
	even

block_nomem:
	IFNE XAPP
	moveq	#ERR_NO_MEM,d0
	move.l	db_active_viewblok(a3),a5
	move.l	vb_filenaam(a5),a0
	bsr	copy_err
	ENDC
	moveq	#-1,d0
	rts
	
test:	dc.l	0

copy_bitmask:
	move.l	db_where_blocks(a3),a6
	move.l	a0,a2
	
	move.l	db_breedtemask(a3),d7
	lsr.l	#1,d7			; words
	subq.w	#1,d7
	
	move.l	db_blockhoogte(a3),d5
rep_pl3:		

	move.w	vb_planes(a5),d6
	subq.w	#1,d6
rep_pl2:

	move.l	d7,d0
rep_pl1:				; copy een lijn
	move.w	(a6)+,(a2)+
	dbf	d0,rep_pl1		; herhaal breedtemask keer

	sub.l	db_breedtemask(a3),a6	; zelfde data nog een keer

	dbf	d6,rep_pl2

	add.l	db_breedtemask(a3),a6	; volgende regel

	dbf	d5,rep_pl3

	add.l	db_sizemask(a3),a0
	rts
	
welkmask:	dc.l	0	; initial teken mask functie

*
*
update_shifts4:
	move.l	db_wstore_moves2(a3),a0
	clr.b	db_blit_done(a3)
	clr.b	db_add_one(a3)			; per keer maximaal een rij erbij

	move.l	db_effect_speed(a3),d7			; maximaal aantal rijen nu 16
	subq.w	#1,d7
urep_stm4:
	move.l	(a0),d0			; x positie van het blok
	move.l	4(a0),d1		; y positie van het blok
	move.l	8(a0),d2		; advance teller geeft aan hoever
	bpl	uno_resetm4		; dit blok is

	cmp.l	#-1,d2
	bne	no_blitm43		; einde van de tabel

	move.l	db_wstore_moves2(a3),a0
	bra	urep_stm4
uno_resetm4:
	bne	no_pro_m4
	tst.b	db_add_one(a3)			; deze is nul
	bne	exit_m4			; al een toegevoegd stop maar
	move.b	#$ff,db_add_one(a3)
new_one_m4:
	bsr	set_new_block
	move.l	(a0),d0
	move.l	4(a0),d1
	move.l	8(a0),d2
	cmp.l	#-2,d2
	beq	no_blitm43
no_pro_m4:
	addq.l	#1,8(a0)		; volgende mask nemen

	move.l	db_minbloklb(a3),d4
	cmp.l	8(a0),d4
	bge	no_pro_m42

	bra	new_one_m4		; deze is klaar maak een nieuwe

no_pro_m42:
	movem.l	d7/a0,-(a7)
	move.b	#$ff,db_blit_done(a3)		; een blit gedaan
	bsr	do_blok
	movem.l	(a7)+,d7/a0
no_blitm43:
	lea.l	12(a0),a0
	dbf	d7,urep_stm4
	bsr	test_fade
exit_m4:
	rts

* set_new_block dient in (a0) en 4(a0) de positie van het volgende blok
* neer te zetten.
* Dit kan op verschillende manieren gebeuren
*
set_new_block:
	move.l	d7,-(a7)
	move.l	db_blockpointer(a3),a1
	cmp.b	#-1,(a1)
	bne	set_new_more
	cmp.b	#-1,1(a1)
	beq	set_new_no_more
set_new_more:
	moveq	#0,d6
	moveq	#0,d7
	move.b	(a1)+,d6
	move.b	d6,d7
	and.b	#$f0,d6
	lsr.b	#4,d6
	and.b	#$f,d7

	cmp.l	#0,db_lastline(a3)
	beq	set_new_l1
	exg	d6,d7

	cmp.l	#1,db_lastline(a3)
	beq	set_new_l1

	move.w	d6,d5
	moveq	#15,d6
	sub.w	d5,d6

	cmp.l	#2,db_lastline(a3)
	beq	set_new_l1

	move.w	d7,d5
	moveq	#15,d7
	sub.w	d5,d7
	exg	d6,d7

	cmp.l	#3,db_lastline(a3)
	beq	set_new_l1

set_new_l1:
	move.l	db_blockbreedte(a3),d5
	mulu	d5,d6
	move.l	db_blockhoogte(a3),d5
	mulu	d5,d7
	move.l	d6,(a0)
	move.l	d7,4(a0)
	move.l	#1,8(a0)
	move.l	a1,db_blockpointer(a3)
	move.l	(a7)+,d7
	rts

set_new_no_more:
	move.l	#0,(a0)
	move.l	#0,4(a0)
	move.l	#-2,8(a0)
	move.l	(a7)+,d7
	rts

* do_blok zet een blok op d0,d1 neer met mask d2
* met lengte en breedte in blocklengte en blokbreedte
* het masker wordt uit tabel patroon_tabel gehaald
*
do_blok:
	subq.l	#1,d2
	move.l	db_wpatroon_tabel(a3),a6
	add.l	d2,d2
	add.l	d2,d2
	move.l	0(a6,d2.l),a6		; het te gebruiken masker
	move.l	db_blockbreedte(a3),d2
	move.l	db_blockhoogte(a3),d3

	bsr	block_blit

	rts

* block_blit probeert een blok van het ene veld naar het andere te blitten
* in d0, d1 en x en y positie van de blit
* in d2, d3 de x en y size van de blit
* a5 wijst naar het active view blok
* a4 wijst naar het inactive view blok
* a6 wijst naar het te gebruiken masker
*
* Het is misschien mogelijk eerst het eerste word te blitten om zo een 
* groot blok met zonder masker en destination te copieren. ?????????????
*
* Deze functie copieert de blokken naar de zelfde plaats in de destination
* Zie voor een echte blok copy put_lijn
*
block_blit:
	movem.l	d0/d1,-(a7)
	move.w	#2,db_tmodulo(a3)

	move.l	d0,d4
	and.l	#$f,d4			; shift waarde	

	lsr.l	#4,d0			; op een heel word gezet
	add.l	d0,d0			; bytes

	move.l	vb_breedte_x(a5),d5
	mulu	vb_planes(a5),d5

	mulu	d1,d5			; offset in y richting
	add.l	d0,d5			; plus x richting

	add.l	#15,d2
	lsr.l	#4,d2			; breedte blit in words 
	add.l	d2,d2			; in bytes

	move.l	d2,d7
	lsl.l	#3,d7
	move.l	db_blockbreedte(a3),d6
	add.l	d4,d6
	cmp.l	d6,d7
	bge	no_pro5

	addq.l	#2,d2			; extra word
	move.w	#0,db_tmodulo(a3)
no_pro5:
	move.l	a6,-(a7)
	move.l	db_graphbase(a3),a6
	jsr	_LVOOwnBlitter(a6)	; blitter multi-tasking ends here
	move.l	(a7)+,a6

	move.l	#$dff000,a0		; hardware basis registers

	move.l	vb_tempbuffer(a4),a1	; source pointer
	add.l	d5,a1
	move.l	vb_tempbuffer(a5),a2	; destination pointer
	add.l	d5,a2

	move.w	vb_planes(a5),d7	; Reken de blithoogte uit
	mulu.w	d3,d7

	move.l	vb_breedte_x(a5),d0	; destination
	sub.w	d2,d0			; min breedte aantal bytes

	lsr.w	#1,d2			; breedte in words
	
	move.l	d0,d1			; zelfde voor desination ?????

	move.w	#$0fe2,d6		; de min termen voor de blitter

	swap	d4
	lsr.l	#4,d4

wbb2:	btst	#14,dmaconr(a0)		; wacht tot blitter vrij is
	bne.b	wbb2

	move.w	d1,bltamod(a0)
	move.w	db_tmodulo(a3),bltbmod(a0)		; zet de modulo masker
	move.w	d0,bltcmod(a0)		; zet de modulo destination
	move.w	d0,bltdmod(a0)		; zet de modulo destination

	move.l	#$ffffffff,bltafwm(a0)

	move.w	d6,bltcon0(a0)
	move.w	d4,bltcon1(a0)		; schuif het masker ook

	move.l	a1,bltapt(a0)
	move.l	a6,bltbpt(a0)		; masker pointer
	move.l	a2,bltcpt(a0)
	move.l	a2,bltdpt(a0)

	move.w	d7,bltsizv(a0)
	move.w	d2,bltsizh(a0)

	move.l	db_graphbase(a3),a6
	jsr	_LVODisownBlitter(a6)	; geef bliter weer vrij
	movem.l	(a7)+,d0/d1
	rts	

*
* maak een horizontal line movement
* de lijnen komen van links en rechts recht naar binnen
* met een varltimes aantal lijnen
*
crawling_even_odd:
	move.l	db_effect_speed(a3),d1
	move.l	db_varltimes(a3),d0
	bne	no_lb2a

	moveq	#1,d0				; screen mode ???
no_lb2a:
	move.l	d0,db_effect_speed(a3)			; dikte lijnen
	mulu	#50,d0
	cmp.w	vb_leny(a5),d0
	ble	no_b2_pro
	move.w	vb_leny(a5),d0
no_b2_pro:
	move.l	d0,db_varltimes(a3)			; aantal lijnen

	move.l	d1,db_tempoff(a3)

	move.l	#0,db_tell(a3)
	clr.w	db_oneventeller(a3)
	bsr	fill_mask			; zet het masker aan
	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

; set fade teller

	move.l	#1,db_shiftit(a3)
	moveq	#0,d0
	
	move.w	vb_leny(a5),d0
	move.l	db_varltimes(a3),d1
	divu	d1,d0
	and.l	#$ffff,d0

	move.l	db_tempoff(a3),d2
	mulu	d2,d0

	move.l	db_effect_speed(a3),d1

	bsr	set_fade_teller

	moveq	#0,d2
	move.w	vb_lenx(a5),d2

	add.l	db_varltimes(a3),d2
	add.l	db_varltimes(a3),d2
	move.l	d2,db_minbloklb(a3)

rep_lijn_h2:

	move.l	#1,db_shiftit(a3)
	move.b	#$ff,db_evendone(a3)
	bsr	draw_lijnen	


	bsr	wacht_tijd2
	bne	exit_lijn2

	move.l	db_varltimes(a3),d0
	move.l	db_effect_speed(a3),d1
	mulu	d1,d0
	add.l	d0,db_tell(a3)
	moveq	#0,d0
	move.w	vb_leny(a5),d0
	cmp.l	db_tell(a3),d0
	ble	exit_lijn2

	bra	rep_lijn_h2

exit_lijn2:
	rts

*
* zet varltimes lijnen neer op horizontale positie shiftit
* zowel van links als rechts
*
draw_lijnen:
	moveq	#0,d6
	move.w	vb_leny(a5),d6
	move.l	db_shiftit(a3),d2
	moveq	#0,d0
	move.w	vb_lenx(a5),d0
	sub.w	d2,d0		; lengte van de lijnen

	move.l	db_varltimes(a3),d7
	lsr.l	#1,d7		; d7 > 0 ?????????????????????????

	tst.w	d7
	beq	no_pod7
	subq.w	#1,d7
no_pod7:

	move.l	db_effect_speed(a3),d3
	move.l	db_tell(a3),d1	; y positie van lijnen
rep_dl:	
	move.l	d1,d5
	movem.l	d0-d7,-(a7)
	moveq	#0,d4
	cmp.l	d6,d1
	bgt	no_put1
	bsr	put_lijn
no_put1:	
	movem.l	(a7)+,d0-d7
	add.l	db_effect_speed(a3),d1

	move.l	d1,d5
	movem.l	d0-d7,-(a7)
	moveq	#0,d0
	moveq	#0,d4
	move.w	vb_lenx(a5),d4
	sub.w	d2,d4

	cmp.l	d6,d1
	bgt	no_put2
	bsr	put_lijn
no_put2:

	movem.l	(a7)+,d0-d7
	add.l	db_effect_speed(a3),d1

	dbf	d7,rep_dl

	bsr	test_fade

	bsr	check_but
	bne	exit_draw_lijnen
	
	move.l	db_tempoff(a3),d7
	add.l	d7,db_shiftit(a3)

	moveq	#0,d0
	move.w	vb_lenx(a5),d0

	cmp.l	db_shiftit(a3),d0
	bge	draw_lijnen

;	bra	draw_lijnen
	tst.b	db_evendone(a3)
	beq	exit_draw_lijnen
	move.w	vb_lenx(a5),d0
	move.l	d0,db_shiftit(a3)
	move.b	#$0,db_evendone(a3)
	bra	draw_lijnen

exit_draw_lijnen:
	rts

*
* Convert vb_colbytes a5 to db_waarcolors
*
convert_colors:
	lea		vb_colbytes(a5),a1
	move.l		db_waarcolors32(a3),a0
	move.w		d0,(a0)+		; aantal kleuren
	move.w		#0,(a0)+		; vanaf kleur 0
	subq.w		#1,d0
rep_rgb1:

	moveq		#2,d7
rep_rgbcol1:
	move.b		(a1)+,d1
	move.b		d1,d2
	lsl.w		#8,d2
	move.b		d1,d2
	move.w		d2,d1
	swap		d1
	move.w		d2,d1
	move.l		d1,(a0)+
	dbf		d7,rep_rgbcol1	
	dbf		d0,rep_rgb1
	move.l		#0,(a0)+

	rts

reshowpicture:
	movem.l	d0-d2/a5/a6/a0/a1,-(a7)
	move.l		db_active_viewblok(a3),a5
	move.l		db_graphbase(a3),a6
	move.l		vb_vieww(a5),a1
	jsr		_LVOLoadView(a6)
	movem.l	(a7)+,d0-d2/a5/a6/a0/a1
	rts

create_tabels:
	bsr	create_hi_to_low_tabel
	bsr	create_low_to_high_tabel
	rts
*
* a0 points to the FORM
*
laadmem:
	move.l	db_inactive_fileblok(a3),a5

	move.l	a0,vb_packed(a5)

	cmp.l	#'FORM',(a0)+
	bne.w	mem_exit
	move.l	(a0),d0
	addq.l	#8,d0

	move.l	d0,vb_packed_size(a5)
	move.w	#3,vb_color_count(a5)
	move.b	#0,vb_bmhd_found(a5)
	move.b	#0,vb_body_found(a5)
	bsr	go_check
	moveq	#0,d0
	rts

mem_exit:
	moveq	#-1,d0
	rts

*
* laad_kleuren zet de kleuren afhankelijk van het OS
* in a5 de pointer naar het viewblok
*
*
laad_kleuren:
	tst.b		db_aa_present(a3)
	beq		no_v39_2

	move.l		vb_color_count(a5),d0
	divu		#3,d0
	move.l		#256,d0
laad_kleuren2:
	tst.b		db_aa_present(a3)
	beq		no_v39_3

	bsr	convert_colors

	move.l		db_waarcolors32(a3),a1
	move.l		vb_viewportw(a5),a0

;	IFNE	DEBUG
;	bsr	print_cols
;	ENDC
;	move.l		vp_ColorMap(a0),d0

	move.l		db_graphbase(a3),a6
	jsr		_LVOLoadRGB32(a6)
	bra		goon_aa1
no_v39_2:

	move.l		vb_viewportw(a5),a0
	move.l		vp_ColorMap(a0),d0

;	IFNE	DEBUG
;	bsr	print_cols
;	ENDC

	lea		vb_colors(a5),a1
	moveq		#32,d0
	move.l		db_graphbase(a3),a6
	jsr		_LVOLoadRGB4(a6)
goon_aa1:
	rts

	IFNE DEBUG
dbstrcol:	dc.b	"%d,",0
	even
print_cols:
	movem.l	a0-a6/d0-d7,-(a7)
	moveq	#7,d7
	lea	vb_colors(a5),a4
.rep:
	move.l	a4,a1
	lea	dbstrcol,a0
	jsr	KPutFmt
	addq.l	#2,a4
	dbf	d7,.rep
	movem.l	(a7)+,a0-a6/d0-d7
	rts
	ENDC

no_v39_3:
	cmp.l	#32,d0
	ble	no_pro_th4
	moveq	#32,d0
no_pro_th4:
	move.l		vb_viewportw(a5),a0
	lea		vb_colors(a5),a1
	move.l		db_graphbase(a3),a6
	jsr		_LVOLoadRGB4(a6)
	rts

*
* Clear the moving storage 
* in d0 het aantal nullen
*
clear_move_storage:
	move.l	db_wstore_moves2(a3),a0
	moveq	#0,d1
r_clearm6:
	move.l	d1,(a0)+
	dbf	d0,r_clearm6
	moveq	#-1,d1
	move.l	d1,(a0)+
	move.l	d1,(a0)+
	move.l	d1,(a0)+
	rts
	
crawling_rows_1:
	move.w	#11,db_lijnmovement(a3)
	bra	test_lijn_blit
crawling_rows_2:
	move.w	#13,db_lijnmovement(a3)
	bra	test_lijn_blit
crawling_rows_3:
	move.w	#17,db_lijnmovement(a3)
	bra	test_lijn_blit

lijnvariation:
	move.l	db_variation(a3),d0
	move.w	d0,db_lijnmovement(a3)
	bra	test_lijn_blit
*
* zet hier welke lijnmovement er gewenst is
*
lijnh1:
	move.w	#1,db_lijnmovement(a3)
	bra	test_lijn_blit
lijnh2:
	move.w	#3,db_lijnmovement(a3)
	bra	test_lijn_blit
lijnh3:
	move.w	#5,db_lijnmovement(a3)
	bra	test_lijn_blit
lijnh4:
	move.w	#7,db_lijnmovement(a3)
	bra	test_lijn_blit
lijnh5:
	move.w	#9,db_lijnmovement(a3)
	bra	test_lijn_blit
lijnh3c:
	move.w	#11,db_lijnmovement(a3)
	bra	test_lijn_blit

lijnh1b:
	move.w	#2,db_lijnmovement(a3)
	bra	test_lijn_blit
lijnh2b:
	move.w	#4,db_lijnmovement(a3)
	bra	test_lijn_blit
lijnh3b:
	move.w	#6,db_lijnmovement(a3)
	bra	test_lijn_blit
lijnh4b:
	move.w	#8,db_lijnmovement(a3)
	bra	test_lijn_blit
lijnh5b:
	move.w	#10,db_lijnmovement(a3)
	bra	test_lijn_blit

blinds_sub1:
	moveq	#1,d7
	move.l	db_varltimes(a3),d0
	cmp.l	#8,d0
	ble	no_add_speed11
	subq.l	#8,d0
	moveq	#3,d7
no_add_speed11
	cmp.l	#3,d0
	ble	no_add_speed1
	moveq	#2,d7
	subq.l	#4,d0
no_add_speed1:
	move.w	#56,db_lijnmovement(a3)
	cmp.l	#0,d0
	beq	test_blinds
	move.w	#57,db_lijnmovement(a3)
	cmp.l	#1,d0
	beq	test_blinds
	move.w	#55,db_lijnmovement(a3)
	cmp.l	#2,d0
	beq	test_blinds
	move.w	#54,db_lijnmovement(a3)
	bra	test_blinds		

blinds_sub2:
	moveq	#1,d7

	move.l	db_varltimes(a3),d0
	cmp.l	#8,d0
	ble	no_add_speed21
	subq.l	#8,d0
	moveq	#3,d7
no_add_speed21

	cmp.l	#3,d0
	ble	no_add_speed2
	moveq	#2,d7
	subq.l	#4,d0
no_add_speed2:
	move.w	#52,db_lijnmovement(a3)
	cmp.l	#0,d0
	beq	test_blinds
	move.w	#53,db_lijnmovement(a3)
	cmp.l	#1,d0
	beq	test_blinds
	move.w	#50,db_lijnmovement(a3)
	cmp.l	#2,d0
	beq	test_blinds
	move.w	#51,db_lijnmovement(a3)
	bra	test_blinds		

test_blinds:

	moveq	#21,d0
	sub.l	db_effect_speed(a3),d0

	move.l	d0,d1
	bne	ok_blinds1
	moveq	#20,d1			; default value screen mode ??
	move.l	d1,d0
ok_blinds1:
	add.l	d1,d1
	move.l	d1,db_effect_speed(a3)
	addq.l	#4,db_effect_speed(a3)
	lsr.l	#1,d1
	sub.l	d1,d0
	addq.l	#1,d0
	lsl.l	d7,d0
	
	move.l	d0,db_varltimes(a3)
	move.l	#0,db_varwtime(a3)

	lea	teller,a0
	move.l	#0,(a0)
	move.l	#0,off_tel_start(a0)

	clr.w	db_oneventeller(a3)
	bsr	fill_mask
	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	move.w	vb_leny(a5),db_lastline(a3)
	move.l	db_effect_speed(a3),d0

	add.w	d0,db_lastline(a3)		; ????????

	clr.l	db_shiftit(a3)
	move.l	#191,d0
	bsr	clear_move_storage	

; set fade teller

	moveq	#0,d0
	move.w	vb_leny(a5),d0
	move.l	db_varltimes(a3),d1
	bsr	set_fade_teller

	moveq	#0,d2
	move.l	db_effect_speed(a3),d2
	move.l	d2,db_max_blocksize(a3)
	add.l	db_varltimes(a3),d2
	add.l	db_varltimes(a3),d2
	move.l	d2,db_minbloklb(a3)
	bra	go_on_lijn_h1
	
************************************************************
* probeer een horizontal line movement te maken
* varltimes geeft hier de horizontale snelheid weer
*
test_lijn_blit:

	move.l	db_effect_speed(a3),d0
	move.l	db_varltimes(a3),d1
	bne	ok_lijnb1

	moveq	#1,d1			; screen mode ??

ok_lijnb1:
	move.l	d1,db_effect_speed(a3)
	add.l	d0,d0
	move.l	d0,db_varltimes(a3)
	move.l	#0,db_varwtime(a3)
	lea	teller,a0
	move.l	#0,(a0)
	move.l	#0,off_tel_start(a0)

	clr.w	db_oneventeller(a3)
	bsr	fill_mask
	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source
	move.w	vb_leny(a5),db_lastline(a3)

	move.l	db_effect_speed(a3),d0

	clr.l	db_shiftit(a3)

	move.l	#191,d0
	bsr	clear_move_storage

; set fade teller

	moveq	#0,d0
	move.w	vb_lenx(a5),d0
	move.l	db_varltimes(a3),d1
	tst.w	d1			; DIV zero test ??????????
	beq	no_div3
	divu	d1,d0
no_div3:
	mulu	vb_leny(a5),d0
	move.l	db_effect_speed(a3),d1

	bsr	set_fade_teller

	moveq	#0,d2
	move.w	vb_lenx(a5),d2
	addq.w	#1,d2
	lsr.w	d2
	add.w	d2,d2
	move.w	d2,vb_lenx(a5)

	add.l	db_varltimes(a3),d2
	add.l	db_varltimes(a3),d2
	move.l	d2,db_minbloklb(a3)

go_on_lijn_h1:
	moveq	#0,d0
	move.w	vb_leny(a5),d0
	move.l	db_effect_speed(a3),d1
	divu	d1,d0
	btst	#0,d0
	beq	no_addq1
	addq.l	#1,d0
no_addq1:
	and.w	#$ffff,d0
	mulu	d1,d0
	move.l	d0,db_tempoff(a3)

rep_lijn_h1:

	bsr	update_shifts5

;	bsr	reshowpicture

	bsr	wacht_tijd2
	bne	exit_lijn1

	tst.b	db_blit_done(a3)
	beq	exit_lijn1

	bra	rep_lijn_h1

exit_lijn1:
	rts

* zoek vanaf links en rechts de grootste afstand
*
*
zoek_grootste_dx:
	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_fileblok(a3),a4
	moveq	#0,d2
	move.w	vb_lenx(a5),d2
	move.w	vb_lenx(a4),d3
	sub.l	db_blitin_x(a3),d2		; totale afstand rechts
	add.l	db_blitin_x(a3),d3		; totale afstand links
	move.l	d2,d4
	cmp.l	d3,d2
	bge	no_chang5
	move.l	d3,d4
no_chang5:
	rts
	
blitin_lijnh1:
	bsr	calc_effect_speed_blitin4

	bsr	zoek_grootste_dx
	move.l	d2,db_startscrollx(a3)
	add.l	db_varltimes(a3),d2
	add.l	db_varltimes(a3),d2
	move.l	d2,db_minbloklb(a3)
	move.w	#20,db_lijnmovement(a3)
	bsr	set_blit_fast_rl
	bra	test_lijn_blitin

blitin_lijnh2:
	bsr	calc_effect_speed_blitin4
	bsr	zoek_grootste_dx
	move.l	d3,db_startscrollx(a3)
	add.l	db_varltimes(a3),d3
	add.l	db_varltimes(a3),d3
	move.l	d3,db_minbloklb(a3)
	move.w	#21,db_lijnmovement(a3)
	bsr	set_blit_fast_lr
	bra	test_lijn_blitin

blitin_lijnh3:
	bsr	calc_effect_speed_blitin4
	bsr	zoek_grootste_dx
	move.l	d4,db_startscrollx(a3)
	add.l	db_varltimes(a3),d4
	add.l	db_varltimes(a3),d4
	move.l	d4,db_minbloklb(a3)		; de grootste afstand links of rechts
	move.w	#22,db_lijnmovement(a3)
	bsr	set_blit_fast_lrrl
	bra	test_lijn_blitin

* probeer een horizontal line movement te maken van een blitin
* varltimes geeft hier de horizontale snelheid weer
*
test_lijn_blitin:
	move.l	#30000,db_colorteller(a3)

	bsr	init_movement2

	clr.w	db_oneventeller(a3)
	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4		; source

	move.l	db_inactive_fileblok(a3),a4

	clr.l	db_shiftit(a3)

	move.w	vb_leny(a4),d0
	move.w	d0,db_lastline(a3)

	move.l	db_blitin_y(a3),d0
	move.l	d0,db_shiftit(a3)
	add.w	d0,db_lastline(a3)

	move.l	#191,d0
	bsr	clear_move_storage

	moveq	#0,d0
	move.w	db_blitin_fy(a3),d0
	move.l	db_effect_speed(a3),d1
	divu	d1,d0
	btst	#0,d0
	beq	no_addq2
	subq.l	#1,d0
no_addq2:
	and.w	#$ffff,d0
	mulu	d1,d0
	move.l	d0,db_tempoff(a3)

rep_blijn_h1:

	bsr	update_shifts5

	bsr	wacht_tijd2
	bne	exit_blijn1

	tst.b	db_blit_done(a3)
	beq	exit_blijn1

	bra	rep_blijn_h1
no_tripleb5:
exit_blijn1:
	rts

*
*
update_shifts5:
	move.l	db_wstore_moves2(a3),a0
	clr.b	db_blit_done(a3)
	clr.b	db_add_one(a3)			; per keer maximaal een rij erbij

	moveq	#63,d7			; maximaal aantal rijen nu 64
urep_stm5:
	move.l	(a0),d0			; x positie van het blok
	move.w	4(a0),d1		; y positie van het blok

	move.l	8(a0),d2		; advance teller geeft aan hoever
	bpl	uno_resetm5		; dit blok is

	cmp.l	#-1,d2
	bne	no_blitm53

	move.l	db_wstore_moves2(a3),a0
	bra	urep_stm5
uno_resetm5:
	bne	no_pro_m5
	tst.b	db_add_one(a3)			; deze is nul
	bne	exit_m5				; al een toegevoegd stop maar
	move.b	#$ff,db_add_one(a3)
new_one_m5:

	bsr	set_new_lijn

	move.l	(a0),d0
	move.w	4(a0),d1
	move.l	8(a0),d2
	cmp.l	#-2,d2
	beq	no_blitm53
no_pro_m5:
	move.l	db_varltimes(a3),d4
	add.l	d4,8(a0)		; schuif lijn een op

	move.l	db_minbloklb(a3),d4
	cmp.l	8(a0),d4
	bge	no_pro_m52
	bra	new_one_m5		; deze is klaar maak een nieuwe
no_pro_m52:
	movem.l	d7/a0,-(a7)
	move.b	#$ff,db_blit_done(a3)		; een blit gedaan
	bsr	do_lijn
	bsr	test_fade
	movem.l	(a7)+,d7/a0
no_blitm53:
	lea.l	12(a0),a0
	dbf	d7,urep_stm5
exit_m5:
	rts

set_new_lijn:
	addq.w	#1,db_oneventeller(a3)
	move.l	db_shiftit(a3),d1
	moveq	#0,d4
	move.w	db_lastline(a3),d4
	cmp.l	d4,d1
	bgt	no_more_sh5
	move.l	db_effect_speed(a3),d4
	add.l	d4,db_shiftit(a3)
	move.l	#0,(a0)
	move.w	d1,4(a0)
	move.w	db_oneventeller(a3),6(a0)
	move.l	db_varltimes(a3),8(a0)
	rts

no_more_sh5:
	move.l	#0,(a0)
	move.l	#0,4(a0)
	move.l	#-2,8(a0)	; stop maar
	rts

* Voer de specifiek movement uit
*
* een tabel zou hier sneller zijn ??????????????????????
*
do_lijn:
	cmp.w	#0,db_lijnmovement(a3)
	beq	do_lijn1
	
	cmp.w	#1,db_lijnmovement(a3)
	beq	do_lijn1

	cmp.w	#3,db_lijnmovement(a3)
	beq	do_lijn2

	cmp.w	#5,db_lijnmovement(a3)
	beq	do_lijn3

	cmp.w	#7,db_lijnmovement(a3)
	beq	do_lijn4

	cmp.w	#9,db_lijnmovement(a3)
	beq	do_lijn5

	cmp.w	#11,db_lijnmovement(a3)
	beq	do_lijn3c

	cmp.w	#13,db_lijnmovement(a3)
	beq	do_lijn3d

	cmp.w	#14,db_lijnmovement(a3)
	beq	do_lijn3dr

	cmp.w	#15,db_lijnmovement(a3)
	beq	do_lijn3e

	cmp.w	#17,db_lijnmovement(a3)
	beq	do_lijn3f

	cmp.w	#20,db_lijnmovement(a3)
	beq	do_blitin_lijn1

	cmp.w	#21,db_lijnmovement(a3)
	beq	do_blitin_lijn2

	cmp.w	#22,db_lijnmovement(a3)
	beq	do_blitin_lijn3

	cmp.w	#50,db_lijnmovement(a3)
	beq	do_blinds1

	cmp.w	#52,db_lijnmovement(a3)
	beq	do_blinds2

	cmp.w	#54,db_lijnmovement(a3)
	beq	do_blinds3

	cmp.w	#56,db_lijnmovement(a3)
	beq	do_blinds4


	moveq	#0,d7

	move.l	db_tempoff(a3),d7
	ext.l	d1
	sub.l	d1,d7		; voor reverse effect
	move.l	d7,d1		; alleen de y coordinaat wordt omgedraaid


	cmp.w	#2,db_lijnmovement(a3)
	beq	do_lijn1

	cmp.w	#4,db_lijnmovement(a3)
	beq	do_lijn2

	cmp.w	#6,db_lijnmovement(a3)
	beq	do_lijn3

	cmp.w	#8,db_lijnmovement(a3)
	beq	do_lijn4

	cmp.w	#10,db_lijnmovement(a3)
	beq	do_lijn5

	cmp.w	#12,db_lijnmovement(a3)
	beq	do_lijn3c

	cmp.w	#16,db_lijnmovement(a3)
	beq	do_lijn3e

	cmp.w	#18,db_lijnmovement(a3)
	beq	do_lijn3f

	cmp.w	#51,db_lijnmovement(a3)
	beq	do_blinds1

	cmp.w	#53,db_lijnmovement(a3)
	beq	do_blinds2

	cmp.w	#55,db_lijnmovement(a3)
	beq	do_blinds3

	cmp.w	#57,db_lijnmovement(a3)
	beq	do_blinds4

	rts

do_blitin_lijn3:
	move.w	6(a0),d0
	btst	#0,d0
	bne	do_blitin_lijn2

do_blitin_lijn1:
	move.l	#4,db_blit_offstart(a3)
	move.l	#0,db_blit_offstop(a3)
	move.l	db_blitin_ox(a3),d0
	
	add.l	db_startscrollx(a3),d0
	sub.l	d2,d0

	move.l	db_blitin_ox(a3),d7
	
	cmp.l	d7,d0
	bge	no_pol1
	move.l	d7,d0
no_pol1:
	bra	no_pol2

do_blitin_lijn2:
	move.l	#0,db_blit_offstart(a3)
	move.l	#4,db_blit_offstop(a3)
	move.l	db_blitin_ox(a3),d0
	sub.l	db_startscrollx(a3),d0
	add.l	d2,d0

	move.l	db_blitin_ox(a3),d7
	
	cmp.l	d7,d0
	ble	no_pol2
	move.l	d7,d0
no_pol2:

	move.l	db_effect_speed(a3),d3		; hoogte
	moveq	#0,d4				; start van source lijn
	move.l	d1,d5				; zelfde lijn
	sub.l	db_blitin_y(a3),d5

	movem.l	d7/a4/a5,-(a7)
	move.l	d5,d7
	add.l	d3,d7
	cmp.w	vb_leny(a4),d7
	ble	.not_min

	sub.w	vb_leny(a4),d7
	sub.w	d7,d3
.not_min:
	bsr	put_blitin_in2_nostst
	movem.l	(a7)+,a4/a5/d7
	rts	

do_lijn3c:
	move.w	6(a0),d0
	btst	#0,d0
	bne	do_lijn2

	move.l	db_tempoff(a3),d7
	sub.l	d1,d7		; voor reverse effect
	move.l	d7,d1		; alleen de y coordinaat wordt omgedraaid
	bra	do_lijn1

do_lijn3dr:
	move.w	6(a0),d0
	btst	#0,d0
	bne	do_lijn3

	move.l	db_tempoff(a3),d7
	sub.w	d1,d7		; voor reverse effect
	move.w	d7,d1		; alleen de y coordinaat wordt omgedraaid
	bra	do_lijn2

do_lijn3d:
	move.w	6(a0),d0
	btst	#0,d0
	beq	do_lijn3

	move.l	db_tempoff(a3),d7
	sub.w	d1,d7		; voor reverse effect
	move.w	d7,d1		; alleen de y coordinaat wordt omgedraaid
	bra	do_lijn1

do_lijn3e:
	move.w	6(a0),d0
	btst	#0,d0
	bne	do_lijn4
	bra	do_lijn5

do_lijn3f:
	move.w	6(a0),d0
	btst	#0,d0
	bne	do_lijn4

	move.l	db_tempoff(a3),d7
	sub.w	d1,d7		; voor reverse effect
	move.w	d7,d1		; alleen de y coordinaat wordt omgedraaid
	bra	do_lijn5

*
* even vanaf rechts oneven vanaf links
*
do_lijn3:
	move.w	6(a0),d0
	btst	#0,d0
	bne	do_lijn2
	bra	do_lijn1

*
* zet een blok neer op met size varltimes op x pos + size
*
do_blinds1:
	moveq	#0,d0
	add.l	d2,d1
	move.l	db_varltimes(a3),d3				; hoogte
	move.w	vb_lenx(a5),d2
	moveq	#0,d4				; start van source lijn
	move.l	d1,d5				; zelfde lijn

	bsr	put_lijn
	rts
*
*
do_blinds2:
	moveq	#0,d0
	add.l	db_effect_speed(a3),d1
	sub.l	d2,d1
	move.l	db_varltimes(a3),d3				; hoogte
	move.w	vb_lenx(a5),d2
	moveq	#0,d4				; start van source lijn
	move.l	d1,d5				; zelfde lijn

	bsr	put_lijn
	rts

do_blinds3:

ttd2:
	moveq	#0,d4				; start van source lijn
	moveq	#0,d0
	move.l	d2,d3
	cmp.l	db_max_blocksize(a3),d3
	ble	max_bli1
	move.l	d3,d4
	move.l	db_max_blocksize(a3),d3
	move.l	d3,d2
	sub.l	d3,d4
	sub.l	d4,d1
	bpl	max_bli1
	moveq	#0,d1
max_bli1:
	move.l	d1,d5				; zelfde lijn
;	sub.l	d4,d5
	moveq	#0,d4				; start van source lijn
	add.l	db_effect_speed(a3),d5
	sub.l	d2,d5
;	bmi	no_bli1
	move.w	vb_lenx(a5),d2
	bsr	put_lijn
no_bli1:
	rts

do_blinds4:
	moveq	#0,d0
ttd:
	move.l	d2,d3
	cmp.l	db_max_blocksize(a3),d3
	ble	max_bli2
	cmp.l	db_max_blocksize(a3),d3
	ble	max_bli2
	move.l	d3,d4
	move.l	db_max_blocksize(a3),d3
	move.l	d3,d2
	sub.l	d3,d4
	sub.l	d4,d1
	bpl	max_bli2
	moveq	#0,d1
max_bli2:

	moveq	#0,d4				; start van source lijn
	move.l	d1,d5				; zelfde lijn
	add.l	db_effect_speed(a3),d1
	sub.l	d2,d1
;	bmi	no_bli2
	move.w	vb_lenx(a5),d2
	bsr	put_lijn
no_bli2:
	rts

*
* zet lijn neer op postitie d2 van de linker scherm rand
*
do_lijn1:
	cmp.w	vb_lenx(a5),d2
	ble	oke_lijn_plus
	move.w	vb_lenx(a5),d2
oke_lijn_plus:
	moveq	#0,d0
	move.w	vb_lenx(a5),d0
	sub.l	d2,d0
	move.l	db_effect_speed(a3),d3		; hoogte
	moveq	#0,d4				; start van source lijn
	move.l	d1,d5				; zelfde lijn
	bsr	put_lijn
	rts

*
* zet een lijn neer met lengte d2 vanaf de rechter scherm rand
*
do_lijn2:
	cmp.w	vb_lenx(a5),d2
	ble	oke_lijn_plus2
	move.w	vb_lenx(a5),d2
oke_lijn_plus2:
	move.w	vb_lenx(a5),d4
	sub.l	d2,d4
	move.l	db_effect_speed(a3),d3		; hoogte
	moveq	#0,d0				; altijd start lijn
	move.l	d1,d5				; zelfde lijn
	bsr	put_lijn
	rts

*
* zet lijn neer op postitie d2/2 in het midden van het scherm
*
do_lijn4:
	cmp.w	vb_lenx(a5),d2
	ble	oke_lijn_plus4
	move.w	vb_lenx(a5),d2
oke_lijn_plus4:
	move.l	d2,d7
	moveq	#0,d0
	move.w	vb_lenx(a5),d0
	lsr.w	#1,d0
	lsr.w	#1,d7
	sub.w	d7,d0

	move.l	db_effect_speed(a3),d3		; hoogte
	moveq	#0,d4				; start van source lijn
	move.l	d1,d5				; zelfde lijn
	lsr.w	#1,d2
	movem.l	d0-d4,-(a7)
	bsr	put_lijn
	movem.l	(a7)+,d0-d4
	move.w	vb_lenx(a5),d0
	move.w	d0,d7
	lsr.w	#1,d0
	sub.w	d2,d7
	move.w	d7,d4
	bsr	put_lijn
	rts

*
* zet lijn neer op postitie d2/2 in het midden van het scherm
*
do_lijn5:
	cmp.w	vb_lenx(a5),d2
	ble	oke_lijn_plus5
	move.w	vb_lenx(a5),d2

oke_lijn_plus5:
	movem.l	d0-d7,-(a7)

	move.l	d2,d7
	moveq	#0,d0
	move.w	vb_lenx(a5),d0
;	addq.l	#1,d7
	lsr.w	#1,d7
	sub.w	d7,d0

	move.l	db_effect_speed(a3),d3		; hoogte
	moveq	#0,d4
	move.w	vb_lenx(a5),d4
	lsr.w	#1,d4
	move.l	d1,d5				; zelfde lijn
	lsr.w	#1,d2
	bsr	put_lijn
	movem.l	(a7)+,d0-d7

	move.l	db_effect_speed(a3),d3		; hoogte
	lsr.w	#1,d2

	moveq	#0,d6
	move.w	vb_lenx(a5),d6
	lsr.w	#1,d6
	sub.w	d2,d6		
	move.l	d6,d4
	moveq	#0,d0
	move.l	d1,d5				; zelfde lijn
	bsr	put_lijn
	rts

*
* vul het masker met enen
*
fill_mask:
	move.l	db_horizontalmask(a3),a0
	moveq	#39,d0			; ruimte voor 1280 pixels ????????
	moveq	#-1,d1
rep_fill_mask:
	move.l	d1,(a0)+
	dbf	d0,rep_fill_mask		
	rts

*
* put_lijn zet een horizontale lijn neer 
* er wordt hierbij een breedte van de lijn masker gemaakt ?????????
* d0,d1 bevat de x,y coordinaat van de destination
* d2,d3 bevatten de lengte en hoogte in pixels
* d4,d5 bevat de x,y coordinaat van de source
*
put_lijn:
	ext.l	d1
	ext.l	d3
	tst.l	d1
	bpl	oke
	add.l	d1,d3
	ble	no_put_lijn
	sub.l	d1,d5
	moveq	#0,d1
oke:
	move.l	db_active_viewblok(a3),a5		; destination
	cmp.w	vb_leny(a5),d1
	bge	no_put_lijn

	move.w	d1,d7
	add.w	d3,d7
	cmp.w	vb_leny(a5),d7
	ble	no_pro_put_lijn
	sub.w	vb_leny(a5),d7
	move.w	d7,d3

no_pro_put_lijn:
	move.l	db_horizontalmask(a3),a0
	move.w	#0,(a0)			; nul voor schuiven

wbl6:	btst	#14,$dff000+dmaconr		; wacht tot blitter vrij is
	bne.b	wbl6

	move.l	db_oldmask(a3),a0
	move.l	#$ffffffff,(a0)

	move.l	d2,d7
	lsr.l	#4,d7
	add.l	d7,d7
	move.l	db_horizontalmask(a3),a0
	add.l	d7,a0
	move.l	d2,d6
	lsl.l	#3,d7	
	sub.l	d7,d6			; pixel restwaarde in d6
	move.l	#$ffff0000,d7
	lsr.l	d6,d7
	addq.l	#2,a0

	move.l	a0,db_oldmask(a3)
	move.w	d7,(a0)			; laatste word ( let op de extra 0 )
	move.w	#$0,2(a0)

	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source
ag:
	movem.l	d0-d7,-(a7)

	bsr	blit_lijn4

	movem.l	(a7)+,d0-d7

no_put_lijn:
	rts

* het masker is even lang als het te blitten object en wordt
* evenals de source bron doorgeschoven 
* in d0, d1 en x en y positie van de blit
* in d2, d3 de x en y size van de blit
* in d4 staat de x offset van de source lijn ( d5 doet nog niets )
* a5 wijst naar het active view blok
* a4 wijst naar het inactive view blok
*
blit_lijn4:
	movem.l	d0/d1,-(a7)
	move.l	db_horizontalmask(a3),db_tempmasker(a3)
	addq.l	#2,db_tempmasker(a3)

	move.l	d0,d6
	and.l	#$f,d6			; shift waarde	destination
	move.l	d6,db_shiftdest(a3)

	move.l	d5,d7


	move.l	d4,d5
	and.l	#$f,d5
	move.l	d5,db_shiftsource(a3)

	cmp.l	d5,d6
	bge	dest_g_source
				; hier moet de blitter naar links schuiven
				; dus door een extra word heen

	sub.l	d6,d5		; pixel terug schuiven
	moveq	#16,d6
	sub.l	d5,d6		; vooruit je schuift nu door een word heen
				; het eerste dest word blijft hier dus leeg
				; zet een leeg word voor het masker
	move.l	d6,db_source_schuif(a3)
	move.l	db_shiftdest(a3),db_mask_schuif(a3)
	subq.l	#2,db_tempmasker(a3)

	add.l	#16,d2		; extra word blitten
	add.l	db_shiftdest(a3),d2	; ??????????????????
	sub.l	#16,d0		; zet dest een word terug

	bra	dest_s_source

dest_g_source:			; als dest > source 
				; schuif source dest - source pixels op
				; S staat als op 9 en moet naar 12 -> 3 Pixels
				; het masker moet echter 12 pixels schuiven
	sub.l	d5,d6		; source schuif waarde
	move.l	d6,db_source_schuif(a3)
	move.l	db_shiftdest(a3),db_mask_schuif(a3)
	add.l	db_shiftdest(a3),d2	; ??????????????????
;	add.l	#16,d2	; ??????????????????
	
dest_s_source:
	lsr.l	#4,d0			; op een heel word gezet
	add.l	d0,d0			; bytes


	move.l	vb_breedte_x(a5),d5	; moet buiten de blit functie !!!!!!!!!
	mulu	vb_planes(a5),d5

	mulu	d5,d7
	mulu	d1,d5			; offset in y richting

	move.l	vb_tempbuffer(a4),a1	; source pointer
	add.l	d7,a1
;	add.l	d5,a1			; nu nog op x = 0 ???????????????

	lsr.l	#4,d4
	add.l	d4,d4
	add.l	d4,a1			; plus source offset

	add.l	d0,d5			; plus x richting

	move.l	db_graphbase(a3),a6
	jsr	_LVOOwnBlitter(a6)	; blitter multi-tasking ends here

	move.l	vb_tempbuffer(a5),a2	; destination pointer
	add.l	d5,a2

	add.l	#15,d2			; extra word om te schuiven
	lsr.l	#4,d2			; breedte blit in words 
	add.l	d2,d2			; in bytes

	move.l	#$dff000,a0		; hardware basis registers

	clr.l	d7
	move.w	vb_planes(a5),d7	; Reken de blithoogte uit
	mulu.w	d3,d7			; kan ook buiten de blit func !!!!!!!

	move.l	vb_breedte_x(a5),d0	; destination
	sub.w	d2,d0			; min breedte aantal bytes

	move.w	d2,d3
	neg.w	d3			; negatieve modulo voor masker

	move.l	d0,d1			; zelfde voor desination ?????

	move.w	#$0fca,d6		; de min termen voor de blitter

	move.l	db_mask_schuif(a3),d4
	swap	d4
	lsr.l	#4,d4
	or.w	d4,d6			; schuif source 

wbl5:	btst	#14,dmaconr(a0)		; wacht tot blitter vrij is
	bne.b	wbl5

	move.w	d3,bltamod(a0)
	move.w	d1,bltbmod(a0)
	move.w	d0,bltcmod(a0)
	move.w	d0,bltdmod(a0)		; zet de modulo destination
	move.l	#$ffffffff,bltafwm(a0)

	move.w	d6,bltcon0(a0)

	move.l	db_source_schuif(a3),d4
	swap	d4
	lsr.l	#4,d4
	move.w	d4,bltcon1(a0)

	move.l	db_tempmasker(a3),bltapt(a0)
	move.l	a1,bltbpt(a0)
	move.l	a2,bltcpt(a0)
	move.l	a2,bltdpt(a0)

	lsr.w	#1,d2			; words voor breedte blit
	
	move.w	d7,bltsizv(a0)
	move.w	d2,bltsizh(a0)

	move.l	db_graphbase(a3),a6
	jsr	_LVODisownBlitter(a6)	; geef bliter weer vrij
	movem.l	(a7)+,d0/d1
	rts	

* het masker is even lang als het te blitten object en wordt
* evenals de source bron doorgeschoven 
* in d0, d1 en x en y positie van de blit
* in d2, d3 de x en y size van de blit
* in d4 staat de x offset van de source lijn ( d5 doet nog niets )
* a5 wijst naar het active view blok
* a4 wijst naar het inactive view blok
*
* probeer een vertikale lijn te tekenen
*
blit_lijn5:
	movem.l	d0/d1,-(a7)

	move.l	d4,d6
	add.l	d2,d6			; laatste pixel van blit ????
	move.l	d6,d7
	and.w	#$fff0,d6
	sub.w	d6,d7
	move.l	#$ffff0000,d6
	lsr.l	d7,d6
	move.w	d6,db_Alastword(a3)

	move.l	d0,d6
	and.l	#$f,d6			; shift waarde	destination
	move.l	d6,db_shiftdest(a3)

	move.l	d4,d5
	and.l	#$f,d5
	move.l	d5,db_shiftsource(a3)

	cmp.l	d5,d6
	bge	dest_g_source5
				; hier moet de blitter naar links schuiven
				; dus door een extra word heen

	sub.l	d6,d5		; pixel terug schuiven
	moveq	#16,d6
	sub.l	d5,d6		; vooruit je schuift nu door een word heen
				; het eerste dest word blijft hier dus leeg
				; zet een leeg word voor het masker
	move.l	d6,db_source_schuif(a3)
	move.l	db_shiftdest(a3),db_mask_schuif(a3)

	add.l	#16,d2		; extra word blitten
	add.l	db_shiftdest(a3),d2	; ??????????????????
	sub.l	#16,d0		; zet dest een word terug

	bra	dest_s_source5

dest_g_source5:			; als dest > source 
				; schuif source dest - source pixels op
				; S staat als op 9 en moet naar 12 -> 3 Pixels
				; het masker moet echter 12 pixels schuiven
	sub.l	d5,d6		; source schuif waarde
	move.l	d6,db_source_schuif(a3)
	move.l	db_shiftdest(a3),db_mask_schuif(a3)
	add.l	db_shiftdest(a3),d2	; ??????????????????
	
dest_s_source5:
	lsr.l	#4,d0			; op een heel word gezet
	add.l	d0,d0			; bytes

	move.l	vb_breedte_x(a5),d5	; moet buiten de blit functie !!!!!!!!!
	mulu	vb_planes(a5),d5
	mulu	d1,d5			; offset in y richting

	move.l	vb_tempbuffer(a4),a1	; source pointer
	lsr.l	#4,d4
	add.l	d4,d4
	add.l	d4,a1			; plus source offset

	move.l	db_blitinmask(a3),a6
	add.l	d4,a6

	add.l	d0,d5			; plus x richting

	move.l	a6,-(a7)		; ??????????????????
	move.l	db_graphbase(a3),a6
	jsr	_LVOOwnBlitter(a6)	; blitter multi-tasking ends here
	move.l	(a7)+,a6

	move.l	vb_tempbuffer(a5),a2	; destination pointer
	add.l	d5,a2

	add.l	#15,d2
	lsr.l	#4,d2			; breedte blit in words 
	add.l	d2,d2			; in bytes

	move.l	#$dff000,a0		; hardware basis registers

	move.w	vb_planes(a5),d7	; Reken de blithoogte uit
	mulu.w	d3,d7			; kan ook buiten de blit func !!!!!!!

	move.l	vb_breedte_x(a5),d0	; destination
	sub.w	d2,d0			; min breedte aantal bytes

	move.l	vb_breedte_x(a4),d1
	sub.w	d2,d1			; modulo source

	move.w	#$0fca,d6		; de min termen voor de blitter

	move.l	db_source_schuif(a3),d4
	swap	d4
	lsr.l	#4,d4
	or.w	d4,d6			; schuif source 

wbl11:	btst	#14,dmaconr(a0)		; wacht tot blitter vrij is
	bne.b	wbl11

	move.w	d1,bltamod(a0)
	move.w	d1,bltbmod(a0)

	move.w	d0,bltcmod(a0)
	move.w	d0,bltdmod(a0)		; zet de modulo destination
	move.l	#$0,bltafwm(a0)

	move.l	db_shiftsource(a3),d0
	move.l	#$ffff0000,d1
	lsr.l	d0,d1
	swap	d1
	move.w	d1,bltafwm(a0)
	move.w	db_Alastword(a3),bltalwm(a0)

	move.w	d6,bltcon0(a0)

	move.l	db_source_schuif(a3),d4
	swap	d4
	lsr.l	#4,d4
	move.w	d4,bltcon1(a0)

	move.l	a6,bltapt(a0)
	move.l	a1,bltbpt(a0)
	move.l	a2,bltcpt(a0)
	move.l	a2,bltdpt(a0)

	lsr.w	#1,d2			; words voor breedte blit

	move.w	d7,bltsizv(a0)
	move.w	d2,bltsizh(a0)

	move.l	db_graphbase(a3),a6
	jsr	_LVODisownBlitter(a6)	; geef bliter weer vrij
	movem.l	(a7)+,d0/d1
	rts	

*
* hardblitv_random blitter random vertikale fill
*
random_columns:
	bsr	calc_effect_speed_ver1

	clr.b	db_random_end(a3)

; reken uit na hoeveel stappen er een colorfade moet plaats vinden

	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	moveq	#0,d1
	move.w	vb_lenx(a5),d1			; alle lijnen nog leeg

	move.l	db_effect_speed(a3),d7
	tst.w	d7
	beq	no_div4				; DIV zero test ??????
	divu	d7,d1				; maal effect_speed
no_div4:
	move.l	d1,d0
	and.l	#$ffff,d1
	move.l	d1,db_max_rand_lijn(a3)

	move.l	d1,d0
	moveq	#1,d1
	bsr	set_fade_teller

	move.l	db_max_rand_lijn(a3),d0
	addq.l	#1,db_max_rand_lijn(a3)
	
	bsr	getmask

	move.l	d7,d0		; start value	
rep_dis2:
	bsr.w	copy_line3
	bne	exit_ran_col
	lsr.l	#1,d0
	bhi.s	rep_dis2
	eor.l	d7,d0
	cmp.l	d7,d0
	bne.s	rep_dis2
exit_ran_col:
	rts

copy_line3:
	cmp.l	db_max_rand_lijn(a3),d0
	bgt	no_line3
	movem.l	d0/d7,-(a7)
	subq.l	#1,d0
	move.l	db_effect_speed(a3),d7
	mulu	d7,d0
	bsr.w	change_mask4
	bsr.w	hardblitvertikal
	bsr	wacht_tijd
	bne	exit_hardc1
	bsr	test_fade
	moveq	#0,d0			; clear condition
exit_hardc1:
	movem.l	(a7)+,d0/d7
	rts

no_line3:
	moveq	#0,d6			; clear condition
	rts	

*
* hardblith_random1 blitter random horizontal
*
random_rows:
	bsr	calc_effect_speed_hor1
	clr.b	db_random_end(a3)
; reken uit na hoeveel stappen er een colorfade moet plaats vinden

	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	moveq	#0,d1
	move.w	vb_leny(a5),d1			; alle lijnen nog leeg

	move.l	db_effect_speed(a3),d7
	tst.w	d7		; DIV zero test ????????
	beq	no_div5
	divu	d7,d1
no_div5:
	move.l	d1,d0
	and.l	#$ffff,d1
	move.l	d1,db_max_rand_lijn(a3)

	move.l	d1,d0
	moveq	#1,d1
	bsr	set_fade_teller

	move.l	db_max_rand_lijn(a3),d0
	addq.l	#1,db_max_rand_lijn(a3)
	bsr	getmask
	move.l	d7,d0		; start value	
rep_dis1:
	bsr.w	copy_line2
	bne	exit_ran_row
	lsr.l	#1,d0
	bhi.s	rep_dis1
	eor.l	d7,d0
	cmp.l	d7,d0
	bne.s	rep_dis1
exit_ran_row:
	rts

copy_line2:
	cmp.l	db_max_rand_lijn(a3),d0
	bgt	no_line2
	movem.l	d0/d7,-(a7)
	subq.l	#1,d0
	move.l	db_effect_speed(a3),d7
	mulu	d7,d0

	bsr.w	hardblithorizontal
	bsr	wacht_tijd
	bne	exit_hardhr1
	bsr	test_fade
	moveq	#0,d0			; clear condition
exit_hardhr1:
	movem.l	(a7)+,d0/d7
	rts
no_line2:
	moveq	#0,d6			; clear condition
	rts	
*
* returns the mask in d7 for number in d0
*
getmask:
	moveq	#0,d1
rep_getmask:
	tst.l	d0
	beq	found_mask
	lsr.l	#1,d0
	addq.l	#1,d1
	bra	rep_getmask
found_mask:
	lsl.l	#2,d1
	move.l	randmasks(pc,d1),d7
	rts
randmasks:
	dc.l	0,0,3,6,$c,$14,$30,$60,$b8
	dc.l	$110,$240,$500,$ca0,$1b00,$3500,$6000,$b400
	dc.l	$12000,$20400,$72000,$90000,$140000,$300000
	dc.l	$400000,$d80000,$1200000,$3880000,$7200000
	dc.l	$9000000,$14000000,$32800000,$48000000,$a3000000

*
* hardblith_random2 blitter random horizontal en gemaskeerd
*
dissolve:
	bsr	calc_effect_speed_hor1

	move.l	db_varltimes(a3),d0

	add.l	d0,db_varltimes(a3)
	add.l	d0,db_counttimer(a3)

	clr.b	db_random_end(a3)
	move.w	#$0fe2,db_maskbits(a3)

	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	bsr	create_random_mask		; of een keer in het begin ????

; reken uit na hoeveel stappen er een colorfade moet plaats vinden

	moveq	#0,d1
	move.w	vb_leny(a5),d1			; alle lijnen nog leeg

	move.l	db_effect_speed(a3),d7
	tst.w	d7		; DIV zero test ????????
	beq	no_div6
	divu	d7,d1				; maal effect_speed
no_div6:
	move.l	d1,d0
	and.l	#$ffff,d1
	move.l	d1,db_max_rand_lijn(a3)

	lsl.l	#3,d1				; maal 8 * random fill

	move.l	d1,d0
	moveq	#1,d1
	bsr	set_fade_teller

	moveq	#0,d0
	move.w	vb_leny(a5),d0
	
	addq.l	#1,d0				; safe ??????????
	lsr.l	#2,d0
	move.l	db_wpatroonruimte(a3),a0
	moveq	#0,d1
rep_clr2:
	move.l	d1,(a0)+
	dbf	d0,rep_clr2

	move.l	db_max_rand_lijn(a3),d0
	lsl.l	#3,d0				; multiply by 16
	addq.l	#1,db_max_rand_lijn(a3)
	bsr	getmask
	move.l	d7,d0		; start value	
rep_dis4:
	bsr.w	copy_line4
	bne	exit_hardhr2
	lsr.l	#1,d0
	bhi.s	rep_dis4
	eor.l	d7,d0
	cmp.l	d7,d0
	bne.s	rep_dis4

exit_hardhr2:
	move.l	db_horizontalmask(a3),db_welkmasker(a3)
	move.w	#$09f0,db_maskbits(a3)
	move.w	#0,db_con1(a3)
	rts	

copy_line4:
	movem.l	d0/d7,-(a7)
	move.l	db_wpatroonruimte(a3),a0
	lsr.l	#3,d0

	moveq	#0,d1
	move.b	0(a0,d0),d1
	addq.b	#4,0(a0,d0)

	lea	rand_longs(pc),a0
	move.l	0(a0,d1.w),db_welkmasker(a3)		; pointer naar het masker
	move.l	d0,d7
	moveq	#16,d0
	bsr	rnd
	add.l	d0,d0
	add.l	d0,db_welkmasker(a3)
	move.l	d7,d0
	cmp.l	db_max_rand_lijn(a3),d0
	bgt	no_line4
	subq.l	#1,d0
	move.l	db_effect_speed(a3),d7
	mulu	d7,d0

	bsr.w	hardblithorizontal
	bsr	wacht_tijd
	bne	exit_hardr4
	bsr	test_fade
	moveq	#0,d0			; clear condition
exit_hardr4:
	movem.l	(a7)+,d0/d7
	rts
no_line4:
	movem.l	(a7)+,d0/d7
	moveq	#0,d6			; clear condition
	rts	

*
* create_random_mask maakt acht masks met een variabel aantal bits aan
* en zet deze achter elkaar in het hmasker, de pointers naar deze maskers
* worden in rand_longs gezet
*
create_random_mask:

	move.l	db_horizontalmask(a3),a2
	lea	rand_longs(pc),a1

	moveq	#6,d7
rep_longs1:
	move.l	a2,(a1)+		; zet masker pointer

	move.l	vb_breedte_x(a5),d5
	lsr.w	#2,d5			; longs
rep_make_mask1:

	moveq	#0,d2

	moveq	#8,d6
	sub.l	d7,d6
	lsl.w	#2,d6
	subq.l	#1,d6

rep_longs2:			; maak een word met d6 pixels aan
	moveq	#32,d0
	bsr	rnd
	btst	d0,d2
	bne	rep_longs2
	bset	d0,d2
	dbf	d6,rep_longs2

	move.l	d2,(a2)+	; store het word en maak een volgende
	dbf	d5,rep_make_mask1
	dbf	d7,rep_longs1

exit_longs:
	move.l	a2,(a1)+
	move.l	vb_breedte_x(a5),d0
	lsr.l	#2,d0
	addq.l	#7,d0
rep_make_mask3:	
	move.l	#$ffffffff,(a2)+
	dbf	d0,rep_make_mask3

	rts

rand_longs:
	ds.l	8		; ruimte voor 8 mask pointers

*
* eigenlijke random generator uit magazine ????????
*
rnd:
	lea	rndseed(pc),a0
	move.w	d0,d1
	tst.w	d1
	ble.s	newseed
	move.l	(a0),d0
	add.l	d0,d0
	bhi.s	hi
	eor.l	#$1d872b41,d0
hi:
	move.l	d0,(a0)
	and.l	#$ffff,d0
	tst.w	d1
	beq	no_div7		; DIV zero test ????????
	divu	d1,d0
no_div7:
	swap	d0
	and.l	#$ffff,d0
	rts

newseed:
	neg.w	d1
	move.l	d1,(a0)
	rts

rndseed:
	dc.l	1234567			; ???????????????

*
* hardblitm1 blitter movement lamelle effect ??
*
blinds_vertical:
;	move.l	#1,db_effect_speed(a3)	; nu nog alleen met snelheid 1 ???????????
	move.l	db_effect_speed(a3),d0
	divu	#5,d0
	moveq	#1,d1
	lsl.w	d0,d1
	move.l	d1,db_effect_speed(a3)
	move.l	#1,teller
	move.l	#1,tellerstart
	move.l	db_wstore_moves2(a3),a0
	move.l	db_active_viewblok(a3),a5
	move.l	vb_breedte_x(a5),d7
	move.l	db_effect_speed(a3),d3
	
	move.l	#0,d2
	move.l	a0,db_tempoff(a3)
	moveq	#0,d1
.r_clearm:
	move.w	d1,(a0)+
	move.w	d2,(a0)+
	sub.w	d3,d2
	addq.w	#2,d1
	cmp.w	d1,d7
	bgt	.r_clearm
	subq.l	#4,a0
	move.l	a0,db_tempoff2(a3)
	move.w	#10000,4(a0)
	move.w	#10000,6(a0)

; reken uit na hoeveel stappen er een colorfade moet plaats vinden

	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4		; source

	moveq	#0,d0
	move.w	vb_lenx(a5),d0
	ext.l	d2
	neg.l	d2
	move.l	d2,d0
	move.l	db_effect_speed(a3),d1
	bsr	set_fade_teller

	clr.l	db_links(a3)			; gebruik links voor tempteller
	clr.l	db_shiftit(a3)
.wwm1:
	bsr	update_shifts
;	bsr	reshowpicture

	move.l	db_tempoff2(a3),a0
	cmp.w	#1000,2(a0)
	beq	.exit_hardm1

	bsr	test_fade
	bsr	wacht_tijd2
;	bsr	check_but
	tst.w	d0
	bne	.exit_hardm1
	bra.w	.wwm1

.exit_hardm1:
	rts	

*
* hardblitm2 blitter movement 16 bits breede balken komen na elkaar omhoog
*
climb_bars_1:
	bra	hardblitm2
climb_bars_2:
	move.l	#1,db_variation(a3)
	bra	hardblitm2
climb_bars_3:
	move.l	#2,db_variation(a3)
	bra	hardblitm2
climb_bars_4:
	move.l	#3,db_variation(a3)
hardblitm2:
	lea	teller,a0		; PC rel ????????????????????
	move.l	#0,(a0)
	move.l	#0,off_tel_start(a0)
	move.l	#0,db_varwtime(a3)

	move.l	db_effect_speed(a3),d0
	add.l	d0,db_effect_speed(a3)

	tst.l	db_effect_speed(a3)
	bne	oke_varl
	move.l	#1,db_effect_speed(a3)

oke_varl:
	move.l	db_wstore_moves2(a3),a0
	moveq	#95,d0
r_clearm2:
	move.l	#0,(a0)+
	dbf	d0,r_clearm2

	move.l	db_horizontalmask(a3),a0
	move.w	#$ffff,(a0)

; reken uit na hoeveel stappen er een colorfade moet plaats vinden

	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	move.l	vb_breedte_x(a5),d0
	mulu	vb_planes(a5),d0
	move.l	d0,db_line_size(a3)			; aantal bytes per regel

	move.w	vb_leny(a5),d1
	subq.w	#1,d1
	mulu	d1,d0
	move.l	d0,db_tempoff(a3)			; start offset

;	moveq	#0,d1
;	move.w	vb_lenx(a5),d1
;	add.w	#15,d1
;	lsr.w	#4,d1
;	add.w	d1,d1
	
	add.l	vb_breedte_x(a5),d0

;	add.l	d1,d0

	move.l	d0,db_final_off(a3)			; eind offset


	move.l	db_line_size(a3),d0
	move.l	db_effect_speed(a3),d1
	mulu	d1,d0
	move.l	d0,db_line_size(a3)
	move.l	db_wstore_moves2(a3),a0
	move.l	db_tempoff(a3),(a0)
	move.l	db_line_size(a3),d1
	sub.l	d1,(a0)
	clr.l	db_tempoff2(a3)		; rij offset
	move.l	db_effect_speed(a3),4(a0)
	move.l	#0,8(a0)

	moveq	#0,d0
	move.l	vb_breedte_x(a4),d0
	lsr.l	#1,d0			; aantal words
	move.w	vb_leny(a4),d1
	move.l	db_effect_speed(a3),d2
	tst.w	d2			; DIV zero test ????????
	beq	no_div8
	divu	d2,d1			; aantal blokjes in rij
no_div8:
	mulu	d1,d0			; totaal aantal blokjes

	moveq	#1,d1
	bsr	set_fade_teller

wwm2:
	bsr	update_shifts2


	tst.b	db_blit_done(a3)
	beq	exit_hardm2

	bsr	wacht_tijd2
	bne	exit_hardm2

	bra.w	wwm2

exit_hardm2:
	rts	

*
* zet een x aantal blits neer iets verschoven
* deze functie regelt het lamelle effect
*
update_shifts:
	move.l	db_wstore_moves2(a3),a0	
.rep_update
	move.l	db_effect_speed(a3),d0
	add.w	d0,2(a0)
	tst.w	2(a0)
	bmi	.no_add
	cmp.w	#16,2(a0)
	bgt	.no_add2
	bsr	hardblitmove
	bra	.no_add
.no_add2:
	move.w	#1000,2(a0)			; set to done
.no_add:
	addq.w	#4,a0
	cmp.w	#10000,(a0)
	bne	.rep_update
	rts

* voer een x aantal blits uit
* deze functie regelt het balken effect
*
update_shifts2:
	move.l	db_wstore_moves2(a3),a0
	clr.b	db_blit_done(a3)
	clr.b	db_add_one(a3)			; per keer maximaal een rij erbij
	
	moveq	#31,d7			; maximaal aantal rijen nu 32
urep_stm:
	move.l	(a0),d0			; relative positie in bytes
	move.l	8(a0),d2
	move.l	4(a0),d1		; hoogte
	bpl	uno_resetm

	cmp.l	#-1,d1
	bne	no_blitm22
	move.l	db_wstore_moves2(a3),a0
	bra	urep_stm
uno_resetm:
	bne	no_pro_m2
	tst.b	db_add_one(a3)			; deze is nul
	bne	exit_m2			; al een toegevoegd stop maar
	move.b	#$ff,db_add_one(a3)

new_one_m2:
	move.l	#0,4(a0)		; nieuwe hoogte
	addq.l	#2,db_tempoff(a3)		; volgende rij
	addq.l	#2,db_tempoff2(a3)
	move.l	db_tempoff2(a3),8(a0)
	move.l	db_tempoff(a3),d3
	cmp.l	db_final_off(a3),d3
	blt	no_pro_m23
	move.l	#-2,4(a0)		; negatieve hoogte
	bra	no_pro_m22

no_pro_m23:
	move.l	d3,(a0)
no_pro_m2:
	move.l	db_effect_speed(a3),d4
	add.l	d4,4(a0)
	move.l	db_line_size(a3),d3
	sub.l	d3,(a0)

	tst.l	(a0)
	bpl	no_pro_m22

	move.w	vb_leny(a5),d1		; totale vertikale lijn blit
	move.l	8(a0),d0		; rij offset
	bra	new_one_m2

no_pro_m22:
	movem.l	d7/a0,-(a7)
	move.b	#$ff,db_blit_done(a3)		; een blit gedaan
	bsr.w	hardblitmove2
	bsr	test_fade
	movem.l	(a7)+,d7/a0
no_blitm22:
	lea.l	12(a0),a0
	dbf	d7,urep_stm
exit_m2:
	rts

* change_maskm verandert het blitter masker op een bepaalde manier
* deze functie zet een d0 mod 16 breed vertikaal balkje op 
* positie  D0  neer.
* De effect speed moet hier een macht van twee zijn ie. 1,2,4,8,16
*
change_maskm:
wbchm:	btst	#14,$dff000+dmaconr		; wacht tot blitter vrij is
	bne.b	wbchm

	move.l	d0,d1
	and.l	#$f,d1
	move.l	#$0000ffff,d2
	add.l	db_effect_speed(a3),d1
	lsl.l	d1,d2			; in d2 nu speed bits %111000000000000
	swap	d2
	move.l	db_horizontalmask(a3),a0
	move.w	d2,(a0)
	rts

*
* a5 wijst naar destination data 
* a4 wijst naar source data
* d0 wijst geeft de horizonatale positie aan
*
* Er wordt in deze functie een word breed blok zonder masker geblit
*
hardblitmove2:
	cmp.l	#0,db_variation(a3)
	beq	zero_vm1

	cmp.l	#1,db_variation(a3)
	bne	no_exgm2
	exg	d2,d0
	bra	zero_vm1
	
no_exgm2:
	move.l	vb_breedte_x(a5),d7
	subq.l	#2,d7
	sub.l	d2,d0
	add.l	d7,d0
	sub.l	d2,d0
	sub.l	d2,d7
	move.l	d7,d2

	cmp.l	#2,db_variation(a3)
	beq	zero_vm1
	exg	d2,d0

zero_vm1:
	tst.w	d1
	beq	bullshitblitm2
	addq.l	#1,d1
	
	move.l	db_graphbase(a3),a6
	jsr	_LVOOwnBlitter(a6)	; blitter multi-tasking ends here

	movem.l	d0/d1,-(a7)

	move.l	#$dff000,a0		; hardware basis registers

	move.l	vb_tempbuffer(a4),a1	; source pointer
	add.l	d2,a1

	move.l	vb_tempbuffer(a5),a2	; destination pointer
	add.l	d0,a2

	move.w	vb_planes(a5),d7	; Reken de blithoogte uit
	mulu.w	d1,d7

	move.l	vb_breedte_x(a5),d6
	lsr.w	#1,d6			; bytes -> words

	move.l	vb_breedte_x(a5),d0	; destination
	subq.w	#2,d0			; min 1 word

	move.l	d0,d1

	move.w	#$09f0,d6		; de min termen voor de blitter ABx

wbm2:	btst	#14,dmaconr(a0)		; wacht tot blitter vrij is
	bne.b	wbm2

	move.w	d1,bltamod(a0)
	move.w	d0,bltdmod(a0)		; zet de modulo waarden

	move.l	#$ffffffff,bltafwm(a0)

	move.w	d6,bltcon0(a0)
	move.w	#0,bltcon1(a0)

	move.l	a1,bltapt(a0)
	move.l	a2,bltdpt(a0)

	move.w	d7,bltsizv(a0)
	move.w	#1,bltsizh(a0)

	move.l	db_graphbase(a3),a6
	jsr	_LVODisownBlitter(a6)	; geef bliter weer vrij
	movem.l	(a7)+,d0/d1
bullshitblitm2:
	rts	

*
* a5 wijst naar destination data 
* a4 wijst naar source data
* d0 wijst geeft de horizonatale positie aan
*
* Er wordt in deze functie een word breed blok gemaskeerd neer gezet
*
hardblitmove:
	movem.l	d0/a0,-(a7)

	moveq	#0,d4
	move.w	2(a0),d4
	move.w	d4,d5
	moveq	#0,d0
	move.w	#$ffff,d0
	lsl.l	d4,d0
	swap	d0
	move.w	d0,db_Afirstword(a3)
	swap	d4
	lsr.l	#4,d4
	moveq	#0,d4
	moveq	#0,d0
	move.w	(a0),d0
	lea.l	$dff000,a0		; hardware basis registers
	move.l	vb_tempbuffer(a4),a1	; source pointer
	add.l	d0,a1

	move.l	vb_tempbuffer(a5),a2	; destination pointer
	add.l	d0,a2

	move.w	vb_planes(a5),d7	; Reken de blithoogte uit
	mulu.w	vb_leny(a5),d7

	move.l	vb_breedte_x(a5),d6
	lsr.w	#1,d6			; bytes -> words

	move.l	vb_breedte_x(a5),d0	; destination
	subq.w	#2,d0			; min 1 word

	move.l	d0,d1
	
	move.w	#$0be2,d6		; de min termen voor de blitter ABx

	or.w	d4,d6

	move.l	db_graphbase(a3),a6
	jsr	_LVOOwnBlitter(a6)	; blitter multi-tasking ends here

.wbm:	btst	#14,dmaconr(a0)		; wacht tot blitter vrij is
	bne.b	.wbm

	move.w	d1,bltamod(a0)
	move.w	d0,bltcmod(a0)
	move.w	d0,bltdmod(a0)		; zet de modulo waarden

	move.l	#$ffffffff,bltafwm(a0)
	move.w	db_Afirstword(a3),bltbdat(a0)
	move.w	d6,bltcon0(a0)

	move.w	#0,bltcon1(a0)

	move.l	a1,bltapt(a0)
	move.l	a2,bltcpt(a0)
	move.l	a2,bltdpt(a0)

	move.w	d7,bltsizv(a0)
	move.w	db_Afirstword(a3),bltbdat(a0)
	move.w	#1,bltsizh(a0)

	move.l	db_graphbase(a3),a6
	jsr	_LVODisownBlitter(a6)	; geef bliter weer vrij
	movem.l	(a7)+,d0/a0
	rts	

* hardblitv1 blitter left right vertical scroll
* De effect speed moet hier een macht van twee zijn ie. 1,2,4,8,16
*
wipe_right:
	bsr	calc_effect_speed_ver1

; reken uit na hoeveel stappen er een colorfade moet plaats vinden

	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	moveq	#0,d0
	move.w	vb_lenx(a5),d0
	move.l	db_effect_speed(a3),d1

	bsr	set_fade_teller

	clr.l	db_shiftit(a3)
ww44:
	move.l	db_shiftit(a3),d0		; masker bits
	bsr.w	change_mask4
	move.l	db_shiftit(a3),d0
	bsr.w	hardblitvertikal


	move.l	db_effect_speed(a3),d0
	add.l	d0,db_shiftit(a3)

	move.l	db_shiftit(a3),d0
	cmp.w	vb_lenx(a5),d0
	bge.b	exit_hard441

	bsr	wacht_tijd
	bne	exit_hard441
	bsr	test_fade

	bra.b	ww44
exit_hard441:
	rts	

* hardblitv2 blitter right left vertical scroll
* De effect speed moet hier een macht van twee zijn ie. 1,2,4,8,16
*
wipe_left:
	bsr	calc_effect_speed_ver1

	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	moveq	#0,d0
	move.w	vb_lenx(a5),d0
	move.l	db_effect_speed(a3),d1

	bsr	set_fade_teller

	move.l	vb_breedte_x(a5),d0
	lsl.l	#3,d0
	sub.l	db_effect_speed(a3),d0
	move.l	d0,db_shiftit(a3)
ww45:
	move.l	db_shiftit(a3),d0		; masker bits
	bsr.w	change_mask4

	move.l	db_shiftit(a3),d0
	bsr.w	hardblitvertikal


	move.l	db_effect_speed(a3),d0
	sub.l	d0,db_shiftit(a3)

	move.l	db_shiftit(a3),d0
	cmp.w	#0,d0
	blt.b	exit_hard541

	bsr	wacht_tijd
	bne	exit_hard541
	bsr	test_fade

	bra.b	ww45

exit_hard541:
	rts	

*
* hardblitv3 blitter right and left middle vertical scroll
* De effect speed moet hier een macht van twee zijn ie. 1,2,4,8,16
*
edges_in_horizontal:
	bsr	calc_effect_speed_ver1

	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	moveq	#0,d0
	move.w	vb_lenx(a5),d0
	move.l	db_effect_speed(a3),d1
	add.l	d1,d1				; twee lijnen tegelijk

	bsr	set_fade_teller

	moveq	#0,d0
	move.w	vb_lenx(a5),d0
	move.l	vb_breedte_x(a5),d0
	lsl.l	#3,d0
	sub.l	db_effect_speed(a3),d0
	move.l	d0,db_rechts(a3)
	clr.l	db_links(a3)
ww46:
	move.l	db_rechts(a3),d0
	bsr.w	change_mask4
	move.l	db_rechts(a3),d0
	bsr.w	hardblitvertikal

	move.l	db_links(a3),d0
	bsr.w	change_mask4
	move.l	db_links(a3),d0
	bsr.w	hardblitvertikal


	move.l	db_effect_speed(a3),d0
	sub.l	d0,db_rechts(a3)
	add.l	d0,db_links(a3)

	move.l	db_rechts(a3),d0
	cmp.l	db_links(a3),d0
	ble.b	exit_hard641

	bsr	wacht_tijd
	bne	exit_hard641
	bsr	test_fade

	bra.b	ww46

exit_hard641:
	rts	

* hardblitv3_ver2 blitter right and left middle skip vertical scroll
* De effect speed moet hier een macht van twee zijn ie. 1,2,4,8,16
*
edges_horizontal:
	bsr	calc_effect_speed_ver1

	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	moveq	#0,d0
	move.w	vb_lenx(a5),d0
	move.l	db_effect_speed(a3),d1
	add.l	d1,d1				; twee lijnen tegelijk

	bsr	set_fade_teller

	moveq	#0,d0
	move.l	vb_breedte_x(a5),d0
	lsl.l	#3,d0
	sub.l	db_effect_speed(a3),d0
	move.l	d0,db_rechts(a3)
	clr.l	db_links(a3)

; check op even en oneven ??????????

	move.l	db_effect_speed(a3),d6
	move.l	d0,d1
	tst.w	d6		; DIV zero test ????????
	beq	no_div9
	divu	d6,d1		; kijk op veelvoud speed
no_div9:
	swap	d1
	tst.w	d1
	beq	veel_espv
	sub.w	d1,d0		; tel rest bij offset op
veel_espv:
	asl.w	#1,d6
	move.l	d0,d1
	tst.w	d6		; DIV zero test ????????
	beq	no_div10
	divu	d6,d1
no_div10:
	swap	d1		; is er een rest
	tst.w	d1
	bne	doorvv2
	sub.l	d7,d0
doorvv2:
	move.l	d0,db_rechts(a3)

wwvv2:

	move.l	db_rechts(a3),d0
	bsr.w	change_mask4
	move.l	db_rechts(a3),d0
	bsr.w	hardblitvertikal

	move.l	db_links(a3),d0
	bsr.w	change_mask4
	move.l	db_links(a3),d0
	bsr.w	hardblitvertikal


	move.l	db_effect_speed(a3),d0
	add.l	d0,d0
	sub.l	d0,db_rechts(a3)
	add.l	d0,db_links(a3)

	move.l	db_rechts(a3),d0
	cmp.l	#0,d0
	ble.b	exit_hardvv2

	bsr	wacht_tijd
	bne	exit_hardvv2
	bsr	test_fade

	bra.b	wwvv2

exit_hardvv2:
	rts	

* hardblitv4 blitter middel right and left vertical scroll
* De effect speed moet hier een macht van twee zijn ie. 1,2,4,8,16
*
center_out_horizontal:
	bsr	calc_effect_speed_ver1

	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	moveq	#0,d0
	move.w	vb_lenx(a5),d0
	move.l	db_effect_speed(a3),d1
	add.l	d1,d1				; twee lijnen tegelijk

	bsr	set_fade_teller

	moveq	#0,d0
	move.w	vb_lenx(a5),d0
	lsr.w	#1,d0
	and.w	#$fff0,d0
	move.l	d0,db_rechts(a3)			; vanuit het midden
	sub.l	db_effect_speed(a3),d0
	move.l	d0,db_links(a3)
	
ww47:
	move.l	db_rechts(a3),d0
	bsr.w	change_mask4
	move.l	db_rechts(a3),d0
	bsr.w	hardblitvertikal

	move.l	db_links(a3),d0
	bsr.w	change_mask4
	move.l	db_links(a3),d0
	bsr.w	hardblitvertikal


	move.l	db_effect_speed(a3),d0
	add.l	d0,db_rechts(a3)
	sub.l	d0,db_links(a3)

	move.l	db_rechts(a3),d0
	cmp.w	vb_lenx(a5),d0
	bge.b	exit_hard741

	bsr	wacht_tijd
	bne	exit_hard741
	bsr	test_fade
	
	bra.b	ww47

exit_hard741:
	rts	

* hardblitv6 blitter right to left skip vertical scroll
* De effect speed moet hier een macht van twee zijn ie. 1,2,4,8,16
*
columns_right_back:
	bsr	calc_effect_speed_ver1

	move.b	#$0,db_evendone(a3)
	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	moveq	#0,d0
	move.w	vb_lenx(a5),d0
	move.l	db_effect_speed(a3),d1

	bsr	set_fade_teller

	move.l	db_effect_speed(a3),d7
	moveq	#0,d0
	move.w	vb_lenx(a5),d0
	move.l	vb_breedte_x(a5),d0
	lsl.l	#3,d0
	move.l	d0,d6
	sub.l	d7,d0
	move.l	d0,db_rechts(a3)
	neg.l	d7				; vanaf rechts
	bra	ww48

* hardblitv5 blitter left to right skip vertical scroll
* De effect speed moet hier een macht van twee zijn ie. 1,2,4,8,16
*
columns_left_back:
	bsr	calc_effect_speed_ver1

	move.b	#$0,db_evendone(a3)
	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	moveq	#0,d0
	move.w	vb_lenx(a5),d0
	move.l	db_effect_speed(a3),d1

	bsr	set_fade_teller

	move.l	#0,db_rechts(a3)
	move.l	db_effect_speed(a3),d7
	move.w	vb_lenx(a5),d6

ww48:
	movem.l	d6/d7,-(a7)

	move.l	db_rechts(a3),d0
	bsr.w	change_mask4		;!!!!!!!!!!!!!!!!!!!

	move.l	db_rechts(a3),d0
	bsr.w	hardblitvertikal


	movem.l	(a7)+,d6/d7

	move.l	d7,d0
	asl.w	#1,d0
	add.l	d0,db_rechts(a3)

	move.l	db_rechts(a3),d1
	cmp.w	d6,d1
	blt.b	goon_hard841_1

	tst.b	db_evendone(a3)
	bne	exit_hard841

	sub.l	d7,db_rechts(a3)		; doe de oneven rijen
	neg.l	d7				; andere kant op
	move.b	#$ff,db_evendone(a3)
goon_hard841_1:
	cmp.w	#0,d1
	bge	goon_hard841_2			; voor de neerwaartse bewegin
	tst.b	db_evendone(a3)
	bne	exit_hard841

	neg.l	d7				; andere kant op
	add.l	d7,db_rechts(a3)			; doe de oneven rijen
	move.b	#$ff,db_evendone(a3)

goon_hard841_2:

	move.l	d6,-(a7)
	bsr	wacht_tijd
	move.l	(a7)+,d6
	
	tst.w	d0
	bne	exit_hard841

	bsr	test_fade

	bra.w	ww48

exit_hard841:

	rts

* change_mask verandert het blitter masker op een bepaalde manier
* deze functie zet een effect_speed breed vertikaal balkje op 
* positie  D0  neer.
* De effect speed moet hier een macht van twee zijn ie. 1,2,4,8,16
*
change_mask4:
wbch4:	btst	#14,$dff000+dmaconr		; wacht tot blitter vrij is
	bne.b	wbch4
	cmp.l	#16,db_effect_speed(a3)
	beq	easy_ch_mask4

	move.l	db_effect_speed(a3),d1
	move.l	#$ffff0000,d2
	lsr.l	d1,d2			; in d2 nu speed bits %111000000000000

	move.l	d0,d1

	and.w	#$fff0,d1		; het zestiental
	sub.w	d0,d1			; rest waarde tussen 0 en 15
					; pixel waarde
	neg.l	d1
	move.l	db_horizontalmask(a3),a0
	lsr.w	d1,d2
	move.w	d2,(a0)
	rts
	
easy_ch_mask4:
	move.l	db_horizontalmask(a3),a0
	move.w	#$ffff,(a0)
	rts

init_mul_tabel:
	move.l	vb_breedte_x(a5),d1
	mulu	vb_planes(a5),d1
	lea.l	mt,a0
	moveq	#0,d0
	move.w	vb_leny(a5),d7
rep_mt:	move.l	d0,(a0)+
	add.l	d1,d0
	dbf	d7,rep_mt
	rts

*
* Set pixel size in a2
* init multable
*
init_pixels:
	lea	masks(pc),a0
	lea	eor_mask(pc),a1
	move.l	a1,(a0)+
	lea	and_mask(pc),a1
	move.l	a1,(a0)+
	lea	eor_mask2(pc),a1
	move.l	a1,(a0)+
	lea	and_mask2(pc),a1
	move.l	a1,(a0)+
	lea	eor_mask3(pc),a1
	move.l	a1,(a0)+
	lea	and_mask3(pc),a1
	move.l	a1,(a0)+
	lea	eor_mask4(pc),a1
	move.l	a1,(a0)+
	lea	and_mask4(pc),a1
	move.l	a1,(a0)+
	lea	eor_mask5(pc),a1
	move.l	a1,(a0)+
	lea	and_mask5(pc),a1
	move.l	a1,(a0)+
	move.l	db_active_viewblok(a3),a5	; destination
	move.l	db_inactive_viewblok(a3),a4	; source
	move.w	vb_leny(a5),db_links(a3)
	move.w	vb_lenx(a5),db_rechts(a3)

	move.l	db_varltimes(a3),d2
	and.w	#$1f,d2

	move.w	vb_leny(a5),d0
	move.w	vb_lenx(a5),d1
	moveq	#100,d6

	move.w	#0,db_dotsize(a3)
	lea	copy_dot1(pc),a2
	cmp.b	#1,d2
	beq	oke_dot1
	sub.l	#10,d6
	lsr.w	#1,d0
	lsr.w	#1,d1
	move.w	#1,db_dotsize(a3)
	lea	copy_dot2(pc),a2
	cmp.b	#2,d2
	beq	oke_dot1
	lsr.w	#1,d0
	lsr.w	#1,d1
	sub.l	#30,d6
	move.w	#2,db_dotsize(a3)
	lea	copy_dot3(pc),a2
	cmp.b	#4,d2
	beq	oke_dot1
	lsr.w	#1,d0
	lsr.w	#1,d1
	sub.l	#40,d6
	move.w	#3,db_dotsize(a3)
	lea	copy_dot4(pc),a2
	cmp.b	#8,d2
	beq	oke_dot1
	lsr.w	#1,d0
	lsr.w	#1,d1
	sub.l	#10,d6
	move.w	#4,db_dotsize(a3)
	lea	copy_dot5(pc),a2
	cmp.b	#16,d2
	beq	oke_dot1

oke_dot1:
	move.l	a2,db_dotfunc(a3)
	move.w	d0,db_links(a3)
	move.w	d1,db_rechts(a3)

	move.l	db_effect_speed(a3),d1
	mulu	d6,d1
	move.l	d1,db_varltimes(a3)

	lea	tellerstart,a0			; pcpcpcpc
	move.l	#0,(a0)
	move.l	#0,-4(a0)
	bsr	init_mul_tabel
	rts

init_lines1:
	bsr	init_pixels
	move.l	vb_breedte_x(a5),d0
	mulu	vb_planes(a5),d0
	move.l	d0,vb_totalwidth(a5)
	move.l	db_effect_speed(a3),d2
	subq.l	#1,d2
	move.l	d2,db_varltimes(a3)
	move.l	d2,db_counttimer(a3)
	moveq	#0,d0
	moveq	#0,d1

	move.w	db_rechts(a3),d0
	moveq	#1,d1
	bsr	set_fade_teller
	bsr	create_y_aspect_table
	rts

create_y_aspect_table:
	moveq	#0,d0
	moveq	#0,d1
	move.w	db_rechts(a3),d0		; width
	move.w	db_links(a3),d1			; heigth
; create a table with Y to aspect
	lsl.l	#8,d1
	divu	d0,d1
	and.l	#$ffff,d1
	moveq	#0,d2
	moveq	#0,d3

	move.l	db_wpatroonruimte(a3),a0
.repsmt:
	move.l	d2,d4
	lsr.l	#8,d4
	move.w	d4,(a0)+	
	add.l	d1,d2
	addq.w	#1,d3
	cmp.w	db_rechts(a3),d3
	ble	.repsmt	
	move.w	db_links(a3),d0
	move.w	d0,(a0)+
	move.w	d0,(a0)+
	move.w	d0,(a0)+
	rts
	
corner_in_top_left:
	bsr	init_lines1
	moveq	#1,d2				; start x coord
	moveq	#1,d3				; start y coord
.rep_tl:
	moveq	#0,d0
	move.w	d3,d1

	movem.w	d2/d3,-(a7)
	bsr	horizontal_line
	movem.w	(a7),d2/d3
	moveq	#0,d1
	move.w	d2,d0
	bsr	vertical_line		
	movem.w	(a7)+,d2/d3
	addq	#1,d2
	addq	#1,d3

	movem.w	d2/d3,-(a7)
	bsr	test_fade
	bsr	wacht_tijd
	movem.w	(a7)+,d2/d3
	tst.w	d0
	bne	.exit_tp2

	cmp.w	db_rechts(a3),d2
	ble	.rep_tl
.exit_tp2:
	rts

corner_in_top_right:
	bsr	init_lines1

	moveq	#0,d2
	move.w	db_rechts(a3),d2		; start x coord
	subq.w	#1,d2
	moveq	#1,d3				; start y coord
.rep_tl:
	move.l	d2,d0
	move.l	d3,d1
	movem.w	d2/d3,-(a7)
	moveq	#0,d2
	move.w	db_rechts(a3),d2
	addq.w	#1,d0
	bsr	horizontal_line
	movem.w	(a7)+,d2/d3
	movem.w	d2/d3,-(a7)
	moveq	#0,d1
	moveq	#0,d0
	move.w	d2,d0
	bsr	vertical_line		
	movem.w	(a7)+,d2/d3
	subq	#1,d2
	addq	#1,d3
	movem.w	d2/d3,-(a7)
	bsr	test_fade
	bsr	wacht_tijd
	movem.w	(a7)+,d2/d3
	tst.w	d0
	bne	.exit_tp2
	tst.w	d2
	bge	.rep_tl
.exit_tp2:
	rts

corner_in_bottom_right:
	bsr	init_lines1
	moveq	#0,d2
	moveq	#0,d3
	move.w	db_rechts(a3),d2		; start x coord
	subq.w	#1,d2
	move.w	db_rechts(a3),d3
	subq	#1,d3				; start y coord
.rep_tl:
	move.l	d2,d0
	move.l	d3,d1
	movem.w	d2/d3,-(a7)
	move.w	db_rechts(a3),d2
	addq.w	#1,d0
	bsr	horizontal_line
	movem.w	(a7),d2/d3
	move.w	d3,d1
	move.w	d2,d0
	move.w	db_rechts(a3),d3
	bsr	vertical_line
	movem.w	(a7)+,d2/d3
	subq	#1,d2
	subq	#1,d3
	movem.w	d2/d3,-(a7)
	bsr	test_fade
	bsr	wacht_tijd
	movem.w	(a7)+,d2/d3
	tst.w	d0
	bne	.exit_tp2
	tst.w	d2
	bge	.rep_tl
.exit_tp2:
	rts

corner_in_bottom_left:
	bsr	init_lines1
	moveq	#1,d2				; start x coord
	move.w	db_rechts(a3),d3		; start y coord
	subq.w	#1,d3
.rep_tl:
	moveq	#0,d0
	move.w	d3,d1

	movem.w	d2/d3,-(a7)
	bsr	horizontal_line
	movem.w	(a7),d2/d3
	move.w	d3,d1
	move.w	d2,d0
	move.w	db_rechts(a3),d3
	bsr	vertical_line		
	movem.w	(a7)+,d2/d3
	addq	#1,d2
	subq	#1,d3

	movem.w	d2/d3,-(a7)
	bsr	test_fade
	bsr	wacht_tijd
	movem.w	(a7)+,d2/d3
	tst.w	d0
	bne	.exit_tp2

	cmp.w	db_rechts(a3),d2
	ble	.rep_tl
.exit_tp2:
	rts

cross2:
	move.l	#1,db_variation(a3)
	bra	incross1
cross1:
	move.l	#2,db_variation(a3)
incross1:
	bsr	init_lines1
	move.w	db_rechts(a3),d0

	moveq	#0,d2				; start x coord

	lsr.l	#1,d0
	movem.l	d0,-(a7)
	moveq	#1,d1
	bsr	set_fade_teller
	movem.l	(a7)+,d7
	moveq	#0,d3
	moveq	#0,d4
.rep_tl:
	movem.l	d7,-(a7)
	movem.w	d2,-(a7)
	cmp.l	#1,db_variation(a3)
	bne	.norev
	exg	d7,d2
	sub.w	d7,d2
.norev:

	bsr	do_cross
	bsr	test_fade
	bsr	wacht_tijd
	movem.w	(a7)+,d2
	addq	#1,d2
	movem.l	(a7)+,d7
	tst.w	d0
	bne	.exit_tp2
	cmp.w	d7,d2
	ble	.rep_tl
.exit_tp2:
	rts

*
* d2 which line to draw
* d7 size d3,d4 offset
*
do_cross:
	move.w	db_rechts(a3),d7
	movem.w	d2/d7,-(a7)
	moveq	#0,d0
	move.w	d2,d1
	move.w	d1,d3
	bsr	horizontal_line
	movem.w	(a7),d2/d7
	moveq	#0,d0
	moveq	#0,d1
	add.w	d2,d0
	move.w	d2,d3
	move.w	d0,d2
	bsr	vertical_line

	movem.w	(a7),d2/d7
	move.w	d7,d0
	sub.w	d2,d0
	move.w	d2,d1
	move.w	d7,d2
	move.w	d1,d3
	bsr	horizontal_line
	movem.w	(a7),d2/d7
	move.w	d7,d0
	sub.w	d2,d0
	moveq	#0,d1
	move.w	d2,d3
	move.w	d0,d2
	bsr	vertical_line

	movem.w	(a7),d2/d7
	moveq	#0,d0
	move.w	d7,d1
	sub.w	d2,d1
	move.w	d1,d3
	bsr	horizontal_line
	movem.w	(a7),d2/d7
	move.w	d2,d0
	move.w	d7,d1
	sub.w	d2,d1
	move.w	d7,d3
	move.w	d0,d2
	bsr	vertical_line

	movem.w	(a7),d2/d7
	move.w	d7,d0
	sub.w	d2,d0
	move.w	d7,d1
	sub.w	d2,d1
	move.w	d1,d3
	move.w	d7,d2
	bsr	horizontal_line
	movem.w	(a7),d2/d7
	move.w	d7,d0
	sub.w	d2,d0
	move.w	d7,d1
	sub.w	d2,d1
	move.w	d7,d3
	move.w	d0,d2
	bsr	vertical_line

	movem.w	(a7)+,d2/d7
	rts

diagonal1:
	tst.l	db_variation(a3)
	bne	.nozero
;	move.l	#2,db_variation(a3)
	move.l	db_varltimes(a3),d0
	and.w	#$3,d0
	addq.w	#1,d0
	move.l	d0,db_variation(a3)
	move.l	db_varltimes(a3),d0
	lsr.w	#2,d0
	moveq	#1,d1
	lsl.w	d0,d1
	move.l	d1,db_varltimes(a3)
.nozero:
	bsr	init_lines1
	move.w	db_rechts(a3),d0
	cmp.l	#1,db_variation(a3)
	beq	.okie
	lsr.w	#1,d0
	cmp.l	#2,db_variation(a3)
	beq	.okie
	lsr.w	#1,d0
	cmp.l	#3,db_variation(a3)
	beq	.okie
	lsr.w	#1,d0
.okie:
	moveq	#0,d2				; start x coord

	movem.l	d0,-(a7)
	moveq	#1,d1
	bsr	set_fade_teller
	movem.l	(a7)+,d7
.rep_tl:
	movem.l	d7,-(a7)

	bsr	do_diagonal

	addq	#1,d2
	movem.w	d2,-(a7)
	bsr	test_fade
	bsr	wacht_tijd
	movem.w	(a7)+,d2
	movem.l	(a7)+,d7
	tst.w	d0
	bne	.exit_tp2
	cmp.w	d7,d2
	ble	.rep_tl
.exit_tp2:
	rts

do_diagonal:
	cmp.l	#1,db_variation(a3)
	bne	.no1
	moveq	#0,d0
	moveq	#0,d1
	move.w	db_rechts(a3),d3
	bsr	diagonal_block
	rts
.no1:
	cmp.l	#2,db_variation(a3)
	bne	.no2
	moveq	#0,d0
	moveq	#0,d1
	move.w	db_rechts(a3),d3
	lsr.w	#1,d3
	bsr	diagonal_block
	moveq	#0,d0
	moveq	#0,d1
	move.w	db_rechts(a3),d3
	lsr.w	#1,d3
	add.w	d3,d0
	bsr	diagonal_block
	moveq	#0,d0
	moveq	#0,d1
	move.w	db_rechts(a3),d3
	lsr.w	#1,d3
	add.w	d3,d1
	bsr	diagonal_block
	moveq	#0,d0
	moveq	#0,d1
	move.w	db_rechts(a3),d3
	lsr.w	#1,d3
	add.w	d3,d0
	add.w	d3,d1
	bsr	diagonal_block
	rts
.no2:
	cmp.l	#3,db_variation(a3)
	bne	.no3
	move.w	db_rechts(a3),d3
	lsr.l	#2,d3
	moveq	#0,d0
	moveq	#0,d1
	moveq	#3,d7
.rep1:
	moveq	#3,d6
	moveq	#0,d0
.rep2:
	movem.l	d6/d7,-(a7)
	bsr	diagonal_block
	movem.l	(a7)+,d6/d7
	add.w	d3,d0
	dbf	d6,.rep2
	add.w	d3,d1
	dbf	d7,.rep1
	rts

.no3:
	move.w	db_rechts(a3),d3
	lsr.l	#3,d3
	moveq	#0,d0
	moveq	#0,d1
	moveq	#7,d7
.rep11:
	moveq	#0,d0
	moveq	#7,d6
.rep21:
	movem.l	d6/d7,-(a7)
	bsr	diagonal_block
	movem.l	(a7)+,d6/d7
	add.w	d3,d0
	dbf	d6,.rep21
	add.w	d3,d1
	dbf	d7,.rep11
	rts

diagonal_block:
	movem.w	d0-d3,-(a7)
	move.w	d2,d4
	add.w	d2,d1
	move.w	d3,d2
	sub.w	d4,d2
	add.w	d0,d2
	move.w	d1,d3
	bsr	horizontal_line
	movem.w	(a7),d0-d3
	move.w	d0,d4
	add.w	d3,d1
	sub.w	d2,d1
	add.w	d2,d0
	move.w	d3,d2
	add.w	d4,d2
	move.w	d1,d3
	bsr	horizontal_line
	movem.w	(a7)+,d0-d3
	rts

wipe_to_centre:
	bsr	init_lines1
	move.w	db_rechts(a3),d0
	lsr.w	#1,d0
	moveq	#1,d1
	bsr	set_fade_teller
	move.w	db_rechts(a3),d2
	moveq	#0,d0
	moveq	#0,d1
.rep_centre:
	move.w	d2,d3
	bsr	draw_box
	movem.l	d0/d1/d2/d4,-(a7)
	bsr	test_fade
	bsr	wacht_tijd
	movem.l	(a7)+,d0/d1/d2/d4
	bne	.exit_centre
	addq.w	#1,d1
	addq.w	#1,d0
	subq.w	#2,d2
	cmp.w	#0,d2
	bge	.rep_centre
.exit_centre:
	rts

wipe_from_centre:
	bsr	init_lines1
	move.w	db_rechts(a3),d0
	lsr.w	#1,d0
	moveq	#1,d1
	bsr	set_fade_teller
	move.w	db_rechts(a3),d0
	move.w	db_rechts(a3),d1
	lsr.w	#1,d0
	lsr.w	#1,d1
	moveq	#1,d2
.rep_centre:
	move.w	d2,d3
	bsr	draw_box
	movem.l	d0/d1/d2/d4,-(a7)
	bsr	test_fade
	bsr	wacht_tijd
	movem.l	(a7)+,d0/d1/d2/d4
	bne	.exit_centre
	subq.w	#1,d1
	subq.w	#1,d0
	addq.w	#2,d2
	cmp.w	#0,d0
	bge	.rep_centre
.exit_centre:
	rts

wipe_to_centre2:
	bsr	init_lines1
	move.w	db_rechts(a3),d0
	lsr.w	#2,d0
	moveq	#1,d1
	bsr	set_fade_teller
	move.w	db_rechts(a3),d2
	lsr.w	#1,d2
	moveq	#0,d0
	moveq	#0,d1
.rep_centre:
	move.w	d2,d3
	bsr	draw_4boxes
	bne	.exit_centre
	addq.w	#1,d1
	addq.w	#1,d0
	subq.w	#2,d2
	cmp.w	#0,d2
	bge	.rep_centre
.exit_centre:
	rts

wipe_to_centre3:
	bsr	init_lines1
	move.w	db_rechts(a3),d0
	lsr.w	#3,d0
	moveq	#1,d1
	bsr	set_fade_teller
	move.w	db_rechts(a3),d2
	lsr.w	#2,d2
	moveq	#0,d0
	moveq	#0,d1
.rep_centre:
	move.w	d2,d3
	bsr	draw_16boxes
	bne	.exit_centre
	addq.w	#1,d1
	addq.w	#1,d0
	subq.w	#2,d2
	cmp.w	#0,d2
	bge	.rep_centre
.exit_centre:
	rts

wipe_from_centre2:
	bsr	init_lines1
	move.w	db_rechts(a3),d0
	lsr.w	#2,d0
	moveq	#1,d1
	bsr	set_fade_teller
	move.w	db_rechts(a3),d0
	move.w	db_rechts(a3),d1
	lsr.w	#2,d0
	lsr.w	#2,d1
	moveq	#1,d2
.rep_centre:
	move.w	d2,d3
	bsr	draw_4boxes
	bne	.exit_centre
	subq.w	#1,d1
	subq.w	#1,d0
	addq.w	#2,d2
	cmp.w	#0,d0
	bge	.rep_centre
.exit_centre:
	rts

wipe_from_centre3:
	bsr	init_lines1
	move.w	db_rechts(a3),d0
	lsr.w	#3,d0
	moveq	#1,d1
	bsr	set_fade_teller
	move.w	db_rechts(a3),d0
	move.w	db_rechts(a3),d1
	lsr.w	#3,d0
	lsr.w	#3,d1
	moveq	#1,d2
.rep_centre:
	move.w	d2,d3
	bsr	draw_16boxes
	bne	.exit_centre
	subq.w	#1,d1
	subq.w	#1,d0
	addq.w	#2,d2
	cmp.w	#0,d0
	bge	.rep_centre
.exit_centre:
	rts

draw_16boxes:
	movem.l	d0-d4,-(a7)
	move.w	db_rechts(a3),d5
	lsr.w	#2,d5
	moveq	#3,d7
.rep_h1:
	movem.l	d0-d1,-(a7)
	moveq	#3,d6
.rep_h2:
	movem.l	d0/d1/d2/d3/d5/d6/d7,-(a7)
	bsr	draw_box
	movem.l	(a7)+,d0/d1/d2/d3/d5/d6/d7
	add.w	d5,d0
	dbf	d6,.rep_h2
	movem.l	(a7)+,d0-d1
	add.w	d5,d1
	dbf	d7,.rep_h1

;	bsr	reshowpicture
	bsr	test_fade
	bsr	wacht_tijd
	movem.l	(a7)+,d0-d4
	rts
	
draw_4boxes:
	bsr	draw_box
	move.w	db_rechts(a3),d5
	lsr.w	#1,d5
	movem.l	d0/d5,-(a7)
	add.w	d5,d0
	bsr	draw_box
	movem.l	(a7)+,d0/d5
	movem.l	d1/d5,-(a7)
	add.w	d5,d1
	bsr	draw_box
	movem.l	(a7)+,d1/d5
	movem.l	d0/d1/d5,-(a7)
	add.w	d5,d1
	add.w	d5,d0
	bsr	draw_box
	movem.l	(a7)+,d0/d1/d5
	movem.l	d0-d4,-(a7)
	bsr	test_fade
	bsr	wacht_tijd
	movem.l	(a7)+,d0-d4
	rts
*
* Draw a box with start d0,d1 and width and height d2,d3
*
draw_box:
	movem.w	d0-d3,-(a7)	
	move.w	d2,d4
	move.w	d0,d2
	add.w	d4,d2
	move.w	d1,d3
;	add.w	#2,d2
	bsr	horizontal_line

	move.w	0(a7),d0
	move.w	2(a7),d1
;	addq.w	#1,d1
	move.w	d1,d3
	move.w	d0,d2
	add.w	6(a7),d3
	subq.w	#1,d3
	bsr	vertical_line

	move.w	0(a7),d0
	move.w	2(a7),d1
	add.w	6(a7),d1
	move.w	d1,d3
	move.w	d0,d2
	add.w	4(a7),d2
;	addq.w	#1,d2
	bsr	horizontal_line

	move.w	0(a7),d0
	add.w	4(a7),d0
	move.w	2(a7),d1
	move.w	d1,d3
	move.w	d0,d2
	add.w	6(a7),d3
;	addq.w	#1,d3
	bsr	vertical_line
.tijd:
	movem.w	(a7)+,d0-d3
	rts

*
* test_pixel1 fool around with pixels
* is the real dissolve
*
test_pixel1:

	move.l	db_inactive_viewblok(a3),a4	; source
	move.l	db_active_viewblok(a3),a5	; destination

	bsr	init_pixels

	move.l	vb_breedte_x(a5),d0
	mulu	vb_planes(a5),d0
	move.l	d0,vb_totalwidth(a5)

	moveq	#0,d0
	move.w	db_links(a3),d0
	mulu	db_rechts(a3),d0

	move.l	db_varltimes(a3),d1
	move.l	db_varltimes(a3),db_counttimer(a3)

	bsr	set_fade_teller

;	bsr	reshowpicture

	bsr	do_dissolve

	move.l	db_fade_counter(a3),d0
	rts

exit_byte1:
	rts	
	
do_dissolve:
	moveq	#0,d0
	move.w	db_rechts(a3),d0
	moveq	#0,d6
f_rep1:
	tst.w	d0
	beq	f_w1
	lsr.l	#1,d0
	addq.w	#1,d6
	bra	f_rep1
f_w1:

	move.w	db_links(a3),d0
	moveq	#0,d7
f_rep2:
	tst.w	d0
	beq	f_w2
	lsr.l	#1,d0
	addq.w	#1,d7
	bra	f_rep2
f_w2:
	moveq	#1,d5
	lsl.l	d6,d5
	subq.l	#1,d5			; colmask
	add.l	d6,d7
	lsl.l	#2,d7
	lea	randmasks(pc),a0
	move.l	d6,d2

	move.l	0(a0,d7.w),d7		; random mask

	move.l	d7,d0
rep_dis5:
	movem.l	d0/d2,-(a7)
	bsr.w	copy_line5
	movem.l	(a7)+,d2/d0
	bne.s	exit_ran_dis1
	lsr.l	#1,d0
	bhi.s	rep_dis5
	eor.l	d7,d0
	cmp.l	d7,d0
	bne.s	rep_dis5
exit_ran_dis1:
	moveq	#0,d0
	moveq	#0,d1
	jsr	(a2)
	rts

copy_line5:
	move.l	d0,d1
	and.l	d5,d0
	cmp.w	db_rechts(a3),d0
	bge	no_cl1
	lsr.l	d2,d1
	cmp.w	db_links(a3),d1
	bge	no_cl1
	jsr	(a2)
	subq.l	#1,db_counttimer(a3)
	bpl	no_cl1
	move.l	db_varltimes(a3),db_counttimer(a3)
	bsr	wacht_tijd2
	bne	exit_cl1
	movem.l	a2,-(a7)
	bsr	test_fade
	movem.l	(a7)+,a2
no_cl1:
	moveq	#0,d0
	rts
exit_cl1:
	rts

*
* Copy a line from buffer to screen
* with the dot function in db_dotfunc
* xs,ys - xd,yd -->> d0,d1 - d2,d3
*
draw_line:
	cmp.w	d0,d2
	beq	vertical_line

	cmp.w	d1,d3
	beq	horizontal_line
;
; Use the fast line drawing here
;
	rts
*
* Special case use fast hor drawing
*
horizontal_line:
	moveq	#0,d5
	move.l	db_wpatroonruimte(a3),a0
	add.w	d1,d1
	move.w	2(a0,d1.w),d5
	move.w	0(a0,d1.w),d1
	add.w	d3,d3
	move.w	0(a0,d3.w),d3

	cmp.w	d0,d2
;	beq	.dottijd
	bne	.dotset1
	addq.w	#1,d2
	bra	.big1
.dotset1:

	bge	.big1
	exg	d2,d0		; swap high and low value
.big1:
	moveq	#0,d4
	move.w	d2,d4
	move.w	db_dotsize(a3),d3
	lsl.l	d3,d4
	lsl.l	d3,d0
	lsl.l	d3,d1
	lsl.l	d3,d5

	IFNE PRAND
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dat(pc),a1
	move.l	d0,(a1)
	move.l	d1,4(a1)
	move.l	d4,8(a1)
	move.l	d5,12(a1)
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"H %lx,%lx,%lx,%lx",10,0
	even
.dat:	dc.l	0,0,0,0
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	sub.l	d1,d5
	move.w	d5,db_tempoff(a3)
	move.w	d0,d6
	lsr.w	#4,d0
	add.w	d0,d0				; address x
	and.w	#$f,d6				; pixel x
	move.w	d6,d7
	lea	mt,a0
	lsl.w	#2,d1
	move.l	0(a0,d1.w),d1
	move.l	vb_tempbuffer(a5),a0
	move.l	vb_tempbuffer(a4),a1
	add.l	d0,d1				; address offset
	add.l	d1,a0
	add.l	d1,a1

	moveq	#0,d3
	move.w	#$ffff,d3
	swap	d3
	lsr.l	d6,d3			; start mask
	move.w	d3,d6
	eor.w	#$ffff,d3		; inverse start mask

	add.w	d4,d7
	cmp.w	#16,d7
	blt	.one_word

	move.w	d4,d2
	lsr.w	#4,d2
	add.w	d2,d2			; address x

	sub.w	d0,d2
	beq	.one_word

	and.w	#$f,d4
	moveq	#0,d7
	move.w	#$ffff,d7
	swap	d7			; end mask
	lsr.l	d4,d7

	move.l	a1,db_end_hline_source(a3)
	move.l	a0,db_end_hline_dest(a3)
	and.l	#$0000ffff,d2
	add.l	d2,db_end_hline_source(a3)
	add.l	d2,db_end_hline_dest(a3)

	lsr.w	#1,d2
	move.l	a1,a2
	move.l	a0,a6

	move.w	db_tempoff(a3),d4
	ble	.dot
	mulu	vb_planes(a5),d4
	subq.w	#1,d4
	move.w	d4,db_tempoff+2(a3)

	move.w	d4,d5
	move.l	vb_breedte_x(a5),d1
.rep_vh2:
	move.w	(a1),d0
	and.w	d3,d0			; only what we want
	and.w	d6,(a0)
	or.w	d0,(a0)
	add.l	d1,a0
	add.l	d1,a1
	dbf	d4,.rep_vh2
	move.w	d5,d4
	addq.w	#2,a2
	addq.w	#2,a6
	move.l	a2,a1
	move.l	a6,a0
	subq.w	#2,d2
	blt	.lastword
	move.w	d2,d5
.rep_vh4:
	move.w	d5,d2
.rep_vh3:
	move.w	(a1)+,(a0)+
	dbf	d2,.rep_vh3
	add.l	d1,a2			; next plane
	add.l	d1,a6			; next plane
	move.l	a2,a1
	move.l	a6,a0
	dbf	d4,.rep_vh4

.lastword:
	move.w	d7,d3
	eor.w	#$ffff,d3
	move.w	db_tempoff+2(a3),d4
	move.l	db_end_hline_source(a3),a1
	move.l	db_end_hline_dest(a3),a0
.rep_vh5:
	move.w	(a1),d0
	and.w	d7,d0
	move.w	(a0),d5
	and.w	d3,d5
	or.w	d0,d5
	move.w	d5,(a0)
	add.l	d1,a0
	add.l	d1,a1
	dbf	d4,.rep_vh5
.dot:
	rts
.dottijd:
	rts

*
* Very small line
*
.one_word:
	move.w	d6,d3
	move.w	d4,d6
	and.w	#$f,d6			; modulo value end x
	moveq	#0,d4
	move.w	#$ffff,d4
	swap	d4
	lsr.l	d6,d4			; start mask
	swap	d4
	or.w	d4,d3
	move.w	d3,d6
	eor.w	#$ffff,d3

	move.w	db_dotsize(a3),d1

	moveq	#0,d4
	move.w	db_tempoff(a3),d4
	mulu.w	vb_planes(a5),d4

;	lsl.w	d1,d4
;	subq.w	#1,d4

	move.l	vb_breedte_x(a5),d1
	bra	.in1

.rep_vh1:
	move.w	(a1),d0
	and.w	d3,d0
	move.w	(a0),d2
	and.w	d6,d2
	or.w	d0,d2
	move.w	d2,(a0)
	add.l	d1,a0
	add.l	d1,a1
.in1:
	dbf	d4,.rep_vh1
	rts

*
* Use fast vertical drawing
*
vertical_line:
	addq	#1,d3
;	lea	sizemt(pc),a0
	move.l	db_wpatroonruimte(a3),a0
	add.w	d1,d1
	move.w	0(a0,d1.w),d1
	
	add.w	d3,d3
	move.w	0(a0,d3.w),d3

	cmp.w	d1,d3
	beq	.dot
	bge	.big1
	exg	d1,d3		; swap high and low value
.big1:
	sub.w	d1,d3
	move.w	d3,d5
	move.w	db_dotsize(a3),d3

	lsl.l	d3,d5
	lsl.l	d3,d1		; shift with the pixel size
	lsl.l	d3,d0		; shift with the pixel size to get address

	move.w	d0,d6
	lsr.w	#4,d0
	add.w	d0,d0		; address x
	and.w	#$f,d6		; pixel x

	add.w	d6,d6
	lsr.w	d3,d6

	lea	mt,a0
	lsl.w	#2,d1
	move.l	0(a0,d1.w),d1
	move.l	vb_tempbuffer(a5),a0
	move.l	vb_tempbuffer(a4),a1
	add.l	d0,d1				; address offset
	add.l	d1,a0
	add.l	d1,a1
	lsl.l	#3,d3
	move.w	d3,d4
	lea	masks(pc),a6
	move.l	0(a6,d4.w),a2		; get masking table eor
	move.w	0(a2,d6.w),d3		; get eor mask

	move.l	4(a6,d4.w),a2		; get masking table and
	move.w	0(a2,d6.w),d6		; get and mask

	move.l	vb_breedte_x(a5),d1
	move.w	vb_planes(a5),d4
	mulu	d5,d4
	subq.w	#1,d4
.rep_vd2:
	move.w	(a1),d0
	move.w	(a0),d2
	and.w	d6,d0
	and.w	d3,d2
	or.w	d0,d2
	move.w	d2,(a0)
	add.l	d1,a0
	add.l	d1,a1
	dbf	d4,.rep_vd2
.dot:
	rts

masks:	dc.l	0,0		; 1 pixel mask pointers
	dc.l	0,0		; 2
	dc.l	0,0		; 4
	dc.l	0,0		; 8
	dc.l	0,0		; 16

*
* Copy a point from a4 to the a5 buffer
* in d0.w, d1.w the x and y coordinate
*
copy_dot1:
	move.w	d7,a6
	move.w	d0,d6
	lsr.w	#4,d0
	add.w	d0,d0				; address x
	and.w	#$f,d6				; pixel x
	add.w	d6,d6
	lea	mt,a0
	lsl.w	#2,d1
	move.l	0(a0,d1.w),d1
	move.l	vb_tempbuffer(a5),a0
	move.l	vb_tempbuffer(a4),a1
	add.l	d0,d1				; address offset
	add.l	d1,a0
	add.l	d1,a1
	move.w	eor_mask(pc,d6.w),d3
	move.w	and_mask(pc,d6.w),d6
	move.l	vb_breedte_x(a5),d1
	move.w	vb_planes(a5),d4
	subq.w	#1,d4
rep_pdot1:
	move.w	(a1),d0
	move.w	(a0),d7
	and.w	d6,d0
	and.w	d3,d7
	or.w	d0,d7
	move.w	d7,(a0)
	add.l	d1,a0
	add.l	d1,a1
	dbf	d4,rep_pdot1
	move.w	a6,d7
	rts

and_mask:
	dc.w	$8000,$4000,$2000,$1000
	dc.w	$0800,$0400,$0200,$0100
	dc.w	$0080,$0040,$0020,$0010
	dc.w	$0008,$0004,$0002,$0001
eor_mask:
	dc.w	$7fff,$bfff,$dfff,$efff
	dc.w	$f7ff,$fbff,$fdff,$feff
	dc.w	$ff7f,$ffbf,$ffdf,$ffef
	dc.w	$fff7,$fffb,$fffd,$fffe

copy_dot2:
	move.w	d5,a6
	add.w	d0,d0
	add.w	d1,d1
	move.w	d0,d6
	lsr.w	#4,d0
	add.w	d0,d0				; address x
	and.w	#$f,d6				; pixel x
	lea	mt,a0
	lsl.w	#2,d1
	move.l	0(a0,d1.w),d1
	move.l	vb_tempbuffer(a5),a0
	move.l	vb_tempbuffer(a4),a1
	add.l	d0,d1				; address offset
	add.l	d1,a0
	add.l	d1,a1
	move.w	eor_mask2(pc,d6.w),d3
	move.w	and_mask2(pc,d6.w),d6
	move.l	vb_breedte_x(a5),d1
	move.l	vb_totalwidth(a5),d2
	move.w	vb_planes(a5),d4
	subq.w	#1,d4
rep_dot2:
	move.w	(a1),d0
	and.w	d6,d0
	move.w	(a0),d5
	and.w	d3,d5
	or.w	d0,d5
	move.w	d5,(a0)

	move.w	0(a1,d2.w),d0
	and.w	d6,d0
	move.w	0(a0,d2.w),d5

	and.w	d3,d5
	or.w	d0,d5
	move.w	d5,0(a0,d2.w)
	add.l	d1,a0
	add.l	d1,a1
	dbf	d4,rep_dot2
	move.w	a6,d5
	rts

and_mask2:
	dc.w	$c000,$3000
	dc.w	$0c00,$0300
	dc.w	$00c0,$0030
	dc.w	$000c,$0003
eor_mask2:
	dc.w	$3fff,$cfff
	dc.w	$f3ff,$fcff
	dc.w	$ff3f,$ffcf
	dc.w	$fff3,$fffc

copy_dot3:
	movem.l	d5,-(a7)
	move.w	d7,a6
	lsl.w	#2,d0
	lsl.w	#2,d1
	move.w	d0,d6
	lsr.w	#4,d0
	add.w	d0,d0				; address x
	and.w	#$f,d6				; pixel x
	lea	mt,a0
	lsl.w	#2,d1
	move.l	0(a0,d1.w),d1
	move.l	vb_tempbuffer(a5),a0
	move.l	vb_tempbuffer(a4),a1
	add.l	d0,d1				; address offset
	add.l	d1,a0
	add.l	d1,a1
	lsr.w	#1,d6
	move.w	eor_mask3(pc,d6.w),d3
	move.w	and_mask3(pc,d6.w),d6
	move.l	vb_breedte_x(a5),d1
	move.l	vb_totalwidth(a5),d2
	move.l	d2,d5
	move.w	vb_planes(a5),d4
	subq.w	#1,d4
rep_dot3:
	move.l	vb_totalwidth(a5),d2

	move.w	(a1),d0
	move.w	(a0),d7
	and.w	d6,d0
	and.w	d3,d7
	or.w	d0,d7
	move.w	d7,(a0)

	move.w	0(a1,d2.w),d0
	move.w	0(a0,d2.w),d7
	and.w	d6,d0
	and.w	d3,d7
	or.w	d0,d7
	move.w	d7,0(a0,d2.w)
	
	add.l	d5,d2
	move.w	0(a1,d2.w),d0
	move.w	0(a0,d2.w),d7
	and.w	d6,d0
	and.w	d3,d7
	or.w	d0,d7
	move.w	d7,0(a0,d2.w)

	add.l	d5,d2
	move.w	0(a1,d2.w),d0
	move.w	0(a0,d2.w),d7
	and.w	d6,d0
	and.w	d3,d7
	or.w	d0,d7
	move.w	d7,0(a0,d2.w)
	add.l	d1,a0
	add.l	d1,a1
	dbf	d4,rep_dot3
	move.w	a6,d7
	movem.l	(a7)+,d5
	rts

and_mask3:
	dc.w	$f000
	dc.w	$0f00
	dc.w	$00f0
	dc.w	$000f
eor_mask3:
	dc.w	$0fff
	dc.w	$f0ff
	dc.w	$ff0f
	dc.w	$fff0

and_mask4:
	dc.w	$ff00
	dc.w	$00ff
eor_mask4:
	dc.w	$00ff
	dc.w	$ff00
and_mask5:
	dc.w	$ffff
eor_mask5:
	dc.w	$0000

copy_dot4:
	movem.l	d5/a4/a5,-(a7)
	lsl.w	#3,d1
	lea	mt,a0
	lsl.w	#2,d1
	move.l	0(a0,d1.w),d1
	move.l	vb_tempbuffer(a5),a0
	move.l	vb_tempbuffer(a4),a1
	add.l	d0,d1				; address offset
	add.l	d1,a0
	add.l	d1,a1

	move.l	vb_breedte_x(a5),d1
	move.l	vb_totalwidth(a5),d2
	move.l	d2,d5
	move.w	vb_planes(a5),d4
	move.l	vb_totalwidth(a5),d2
	move.l	a0,a4
	move.l	a1,a5
	subq.w	#1,d4
rep_dot4:
	move.l	a4,a0
	move.l	a5,a1
	moveq	#7,d5
rep_dot4a:
	move.b	(a1),(a0)
	add.l	d2,a1
	add.l	d2,a0
	dbf	d5,rep_dot4a
	
	add.l	d1,a4
	add.l	d1,a5
	dbf	d4,rep_dot4
	movem.l	(a7)+,d5/a4/a5
	rts

copy_dot5:
	movem.l	d5/a4/a5,-(a7)
	add.w	d0,d0
	lsl.w	#4,d1
	lea	mt,a0
	lsl.w	#2,d1
	move.l	0(a0,d1.w),d1
	move.l	vb_tempbuffer(a5),a0
	move.l	vb_tempbuffer(a4),a1
	add.l	d0,d1				; address offset
	add.l	d1,a0
	add.l	d1,a1

	move.l	vb_breedte_x(a5),d1
	move.l	vb_totalwidth(a5),d2
	move.l	d2,d5
	move.w	vb_planes(a5),d4
	move.l	vb_totalwidth(a5),d2
	move.l	a0,a4
	move.l	a1,a5
	subq.w	#1,d4
rep_dot5:
	move.l	a4,a0
	move.l	a5,a1
	moveq	#15,d5
rep_dot5a:
	move.w	(a1),(a0)
	add.l	d2,a1
	add.l	d2,a0
	dbf	d5,rep_dot5a
	
	add.l	d1,a4
	add.l	d1,a5
	dbf	d4,rep_dot5
	movem.l	(a7)+,d5/a4/a5
	rts
*
* hardblith1 blitter up down horizontal scroll
*
wipe_down:
	bsr	calc_effect_speed_hor1

; reken uit na hoeveel stappen er een colorfade moet plaats vinden
	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	moveq	#0,d0
	move.w	vb_leny(a5),d0
	move.l	db_effect_speed(a3),d1

	bsr	set_fade_teller

	clr.l	db_shiftit(a3)
wwh41:
	move.l	db_shiftit(a3),d0

	bsr.w	hardblithorizontal


	bsr	wacht_tijd
	bne	exit_hardh441

	move.l	db_effect_speed(a3),d0
	add.l	d0,db_shiftit(a3)

	move.l	db_shiftit(a3),d0
	cmp.w	vb_leny(a5),d0

	bge.b	exit_hardh441

	bsr	test_fade

	bra.b	wwh41
exit_hardh441:
	rts	

*
* hardblith2 blitter down up horizontal scroll
*
wipe_up:
	bsr	calc_effect_speed_hor1

; reken uit na hoeveel stappen er een colorfade moet plaats vinden

	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	moveq	#0,d0
	move.w	vb_leny(a5),d0
	move.l	db_effect_speed(a3),d1

	bsr	set_fade_teller

	moveq	#0,d0
	move.w	vb_leny(a5),d0
	sub.l	db_effect_speed(a3),d0
	move.l	d0,db_shiftit(a3)
wwh42:
	move.l	db_shiftit(a3),d0
	bsr.w	hardblithorizontal


	bsr	wacht_tijd
	bne	exit_hardh442

	move.l	db_effect_speed(a3),d0
	sub.l	d0,db_shiftit(a3)

	move.l	db_shiftit(a3),d0
	cmp.w	#0,d0
	blt.b	exit_hardh442

	bsr	test_fade

	bra.b	wwh42
exit_hardh442:
	rts	

*
* hardblith3 blitter down up middel horizontal scroll
*
edges_in_vertical:
	bsr	calc_effect_speed_hor1

	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	moveq	#0,d0
	move.w	vb_leny(a5),d0
	move.l	db_effect_speed(a3),d1
	add.l	d1,d1				; twee lijnen tegelijk

	bsr	set_fade_teller

	moveq	#0,d0
	move.w	vb_leny(a5),d0
	sub.l	db_effect_speed(a3),d0
	move.l	d0,db_rechts(a3)
	clr.l	db_links(a3)
	move.l	db_rechts(a3),d0
	bsr.w	hardblithorizontal

	move.l	db_links(a3),d0
	bsr.w	hardblithorizontal

wwh43:

	bsr	wacht_tijd
	bne	exit_hardh641

	move.l	db_effect_speed(a3),d0
	sub.l	d0,db_rechts(a3)
	add.l	d0,db_links(a3)

	move.l	db_rechts(a3),d0
	bsr.w	hardblithorizontal

	move.l	db_links(a3),d0
	bsr.w	hardblithorizontal


	move.l	db_rechts(a3),d0
	cmp.l	db_links(a3),d0
	ble.b	exit_hardh641

	bsr	test_fade

	bra.b	wwh43

exit_hardh641:
	rts	

* check op even en oneven ?????????
* return het oneven getal d0
*
check_even_oneven:
	move.l	db_effect_speed(a3),d6
	move.l	d0,d1
	tst.w	d6		; DIV zero test ????????
	beq	no_div11
	divu	d6,d1		; kijk op veelvoud speed
no_div11:
	swap	d1
	tst.w	d1
	beq	veel_esp
	sub.w	d1,d0		; tel rest bij offset op
veel_esp:
	asl.w	#1,d6
	move.l	d0,d1
	tst.w	d6		; DIV zero test ????????
	beq	no_div12
	divu	d6,d1
no_div12:
	swap	d1		; is er een rest
	tst.w	d1
	bne	doorv2
	sub.l	d7,d0
doorv2:
	rts

*
* hardblith3 blitter down up middel horizontal skip scroll
*
rows_from_edges_vertical:

	bsr	calc_effect_speed_hor1


	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	moveq	#0,d0
	move.w	vb_leny(a5),d0
	move.l	db_effect_speed(a3),d1
	add.l	d1,d1				; twee lijnen tegelijk

	bsr	set_fade_teller

	moveq	#0,d0
	move.w	vb_leny(a5),d0
	move.l	db_effect_speed(a3),d7
	sub.l	db_effect_speed(a3),d0
	move.l	d0,db_rechts(a3)
	clr.l	db_links(a3)

	bsr	check_even_oneven

	move.l	d0,db_rechts(a3)
wwh43v2:
	move.l	d7,-(a7)
	move.l	db_rechts(a3),d0
	bsr.w	hardblithorizontal

	move.l	db_links(a3),d0
	bsr.w	hardblithorizontal
	move.l	(a7)+,d7


	bsr	wacht_tijd
	bne	exit_hardh3v2

	move.l	d7,d0
	add.l	d0,d0				; twee keer

	sub.l	d0,db_rechts(a3)
	add.l	d0,db_links(a3)

	move.l	db_links(a3),d0
	cmp.w	vb_leny(a5),d0
	bge.b	exit_hardh3v2

no_pro_even:
	bsr	test_fade

	bra.b	wwh43v2
exit_hardh3v2:
	rts	

*
* hardblith4 blitter middel up down horizontal  scroll
*
center_out_vertical:
	bsr	calc_effect_speed_hor1

	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	moveq	#0,d0
	move.w	vb_leny(a5),d0
	move.l	db_effect_speed(a3),d1
	add.l	d1,d1				; twee lijnen tegelijk

	bsr	set_fade_teller

	moveq	#0,d0
	move.w	vb_leny(a5),d0
	lsr.w	#1,d0
;	and.w	#$fff0,d0
	move.l	d0,db_rechts(a3)			; vanuit het midden
	sub.l	db_effect_speed(a3),d0
	move.l	d0,db_links(a3)

wwh44:

	move.l	db_rechts(a3),d0
	bsr.w	hardblithorizontal

	move.l	db_links(a3),d0
	bsr.w	hardblithorizontal


	bsr	wacht_tijd
	bne	exit_hardh741

	move.l	db_effect_speed(a3),d0
	add.l	d0,db_rechts(a3)
	sub.l	d0,db_links(a3)

	move.l	db_rechts(a3),d0
	cmp.w	vb_leny(a5),d0
	bge.b	exit_hardh741

	bsr	test_fade
	
	bra.b	wwh44

exit_hardh741:
	rts	
*
* hardblith6 blitter down up skip horizontal scroll
*
rows_bottom_and_back:
	bsr	calc_effect_speed_hor1

	move.b	#$0,db_evendone(a3)
	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	moveq	#0,d0
	move.w	vb_leny(a5),d0
	move.l	db_effect_speed(a3),d1

	bsr	set_fade_teller

	move.l	db_effect_speed(a3),d7
	moveq	#0,d0
	move.w	vb_leny(a5),d0
	sub.l	d7,d0
	move.l	d0,db_rechts(a3)
	neg.l	d7				; vanaf rechts
	bra	wwh45

*
* hardblith5 blitter up down skip horizontal scroll
*
rows_top_and_back:
	bsr	calc_effect_speed_hor1

	move.b	#$0,db_evendone(a3)
	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	moveq	#0,d0
	move.w	vb_leny(a5),d0
	move.l	db_effect_speed(a3),d1
	bsr	set_fade_teller

	move.l	#0,db_rechts(a3)
	move.l	db_effect_speed(a3),d7
wwh45:
	move.l	d7,-(a7)

	move.l	db_rechts(a3),d0
	bsr.w	hardblithorizontal

	move.l	(a7)+,d7


	bsr	wacht_tijd
	bne	exit_hard841

	move.l	d7,d0
	add.l	d0,d0				; twee keer
	add.l	d0,db_rechts(a3)

	move.l	db_rechts(a3),d0
	cmp.w	vb_leny(a5),d0
	blt.b	goon_hardh841_1

	tst.b	db_evendone(a3)
	bne	exit_hardh841

	sub.l	d7,db_rechts(a3)		; doe de oneven rijen
	neg.l	d7				; andere kant op
	move.b	#$ff,db_evendone(a3)
goon_hardh841_1:
	cmp.w	#0,d0
	bge	goon_hardh841_2		; voor de neerwaartse beweging
	tst.b	db_evendone(a3)
	bne	exit_hardh841

	move.l	d7,-(a7)		; zeker bovenste blok doen ??????
	moveq	#0,d0
	bsr.w	hardblithorizontal
	move.l	(a7)+,d7

	neg.l	d7				; andere kant op
	add.l	d7,db_rechts(a3)			; doe de oneven rijen
	move.b	#$ff,db_evendone(a3)

goon_hardh841_2:

	bsr	test_fade

	bra.b	wwh45

exit_hardh841:

	rts

*
* hardblith1_ver2 blitter up down horizontal scroll met randomness
*
torn_down:
	bsr	calc_effect_speed_hor1
br1:
; reken uit na hoeveel stappen er een colorfade moet plaats vinden

	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	bsr	create_random_mask		; of een keer in het begin ????

	moveq	#0,d0
	move.w	vb_leny(a5),d0
	move.l	db_effect_speed(a3),d1

	bsr	set_fade_teller

	move.l	db_effect_speed(a3),d7
	lsl.l	#4,d7
	neg.l	d7
	move.l	d7,db_shiftit(a3)		; begin vanaf positie -8

wwh41v2:
	move.l	db_shiftit(a3),d0

	lea	rand_longs(pc),a0
	lea.l	7*4(a0),a0			; laatste mask eerst
	moveq	#0,d6
	move.w	vb_leny(a5),d6

	move.w	#$09f0,db_maskbits(a3)
	moveq	#7,d7
rep_h1_ver2
	move.l	(a0),db_welkmasker(a3)

	add.l	db_effect_speed(a3),d0
	move.l	(a0),db_welkmasker(a3)
	subq.l	#4,a0

	cmp.l	#0,d0
	blt	no_put5b
	cmp.l	d6,d0
	bgt	no_put5b

	movem.l	d0/d6/d7/a0,-(a7)
	move.l	d0,d2
	and.l	#$f,d2
	add.l	d2,d2
	add.l	d2,db_welkmasker(a3)
	bsr.w	hardblithorizontal
	movem.l	(a7)+,d0/d6/d7/a0
no_put5b:
	move.w	#$0fe2,db_maskbits(a3)
	add.l	db_effect_speed(a3),d0
	dbf	d7,rep_h1_ver2

	bsr	wacht_tijd
	bne	exit_hardh441v2

	move.l	db_effect_speed(a3),d0
	add.l	d0,db_shiftit(a3)

	move.l	db_shiftit(a3),d0
	cmp.w	vb_leny(a5),d0
	bge.b	exit_hardh441v2

	bsr	test_fade

	bra.w	wwh41v2
exit_hardh441v2:
	move.w	#$09f0,db_maskbits(a3)
	move.w	#0,db_con1(a3)
	rts	

*
* hardblith2_ver2 blitter up down horizontal scroll met randomness
*
torn_up:
	bsr	calc_effect_speed_hor1

	move.w	#$0fe2,db_maskbits(a3)

; reken uit na hoeveel stappen er een colorfade moet plaats vinden

	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	bsr	create_random_mask		; of een keer in het begin ????

	moveq	#0,d0
	move.w	vb_leny(a5),d0
	move.l	db_effect_speed(a3),d1

	bsr	set_fade_teller


	moveq	#0,d0
	move.w	vb_leny(a5),d0
	move.l	db_effect_speed(a3),d1
	lsl.l	#4,d1
	add.l	d1,d0
	move.l	db_effect_speed(a3),d1
	move.l	d0,d2
	divu	d1,d2
	addq.l	#1,d2
	and.l	#$ffff,d2
	mulu	d1,d2	
	move.l	d2,db_shiftit(a3)

wwh42v2:
	move.l	db_shiftit(a3),d0

	lea	rand_longs(pc),a0
	lea.l	7*4(a0),a0			; laatste mask eerst
	moveq	#0,d6
	move.w	vb_leny(a5),d6
	
	moveq	#7,d7
	move.w	#$09f0,db_maskbits(a3)
rep_h2_ver2
	move.l	(a0),db_welkmasker(a3)

	sub.l	db_effect_speed(a3),d0

	move.l	(a0),db_welkmasker(a3)
	subq.l	#4,a0
	cmp.l	#0,d0
	blt	no_put6b
	cmp.l	d6,d0
	bgt	no_put6b
	movem.l	d0/d6/d7/a0,-(a7)
	move.l	d0,d2
	and.l	#$f,d2
	add.l	d2,d2
	add.l	d2,db_welkmasker(a3)
	bsr.w	hardblithorizontal
	movem.l	(a7)+,d0/d6/d7/a0

no_put6b:
	move.w	#$0fe2,db_maskbits(a3)
	sub.l	db_effect_speed(a3),d0
	dbf	d7,rep_h2_ver2

	bsr	wacht_tijd
	bne	exit_hardh442v2

	move.l	db_effect_speed(a3),d0
	sub.l	d0,db_shiftit(a3)

	move.l	db_shiftit(a3),d0
	cmp.w	#0,d0
	blt.b	exit_hardh442v2

	bsr	test_fade

	bra.w	wwh42v2
exit_hardh442v2:
	move.w	#$09f0,db_maskbits(a3)
	move.w	#0,db_con1(a3)
	rts	
*
* hardblith2_ver3 blitter up down horizontal scroll met rol-endje
*
roll_up:
	lea	tellerstart,a0
	move.l	#0,(a0)

; reken uit na hoeveel stappen er een colorfade moet plaats vinden

	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	moveq	#0,d0
	move.w	vb_leny(a5),d0
	move.l	db_effect_speed(a3),d1

	bsr	set_fade_teller

	move.l	vb_breedte_x(a5),d0
;	moveq	#0,d0
;	move.w	vb_lenx
	mulu	vb_planes(a5),d0
	move.l	d0,vb_totalwidth(a5)
	
	moveq	#0,d0
	move.w	vb_leny(a5),d0
	move.l	d0,db_shiftit(a3)
	move.w	d0,db_links(a3)
wwh42v3:
	move.l	db_shiftit(a3),d0
	moveq	#0,d6
	move.w	vb_leny(a5),d6

	bsr	place_roll_up

;	bsr	reshowpicture

	bsr	wacht_tijd
	bne	exit_hardh442v3

	move.l	db_effect_speed(a3),d0
	sub.l	d0,db_shiftit(a3)

	move.l	db_shiftit(a3),d0
	move.l	db_effect_speed(a3),d1
	neg.l	d1
	cmp.w	d1,d0
	blt.b	exit_hardh442v3

	bsr	test_fade

	bra.b	wwh42v3
exit_hardh442v3:
	rts	

*
* Place a roll at position d0
*
place_roll_up:
	tst.l	d0
	bpl	.plus
	moveq	#0,d0
.plus:
	move.l	d0,d7
	mulu	d0,d7			; x^2
	lsl.l	#5,d7
	swap	d7
	and.l	#$ff,d7			; < 64
	addq.w	#1,d7			; d7 = ( x^2 / 2^13) + 1

; d7 now the roll size
; does the roll fit in the display

	move.l	d0,d1
	add.l	d7,d1
	cmp.w	d6,d1
	ble	.roll_fits

; reduce the roll size
	sub.w	d6,d1
	sub.w	d1,d7

.roll_fits:	
	move.w	d0,d1
	add.w	d7,d1

	clr.w	db_rechts(a3)

	move.w	db_links(a3),d2
	sub.w	d1,d2
	bmi	.nos
	beq	.nos
	move.w	d2,db_rechts(a3)
	move.w	d1,db_links(a3)		; new value
.nos:

	move.l	d0,d1
	move.l	vb_totalwidth(a5),d2
	mulu	d2,d1			; line offset

	move.l	vb_tempbuffer(a5),a1	; destination
	move.l	vb_tempbuffer(a4),a0	; source
	add.l	d1,a0
	add.l	d1,a1
	move.l	a0,a2
	move.l	d2,d5
	moveq	#0,d1
	
	moveq	#0,d0
	move.w	vb_lenx(a5),d0
	add.w	#15,d0
	lsr.w	#4,d0
	add.w	d0,d0
	cmp.l	vb_breedte_x(a5),d0
	beq	.normal			; normal full screen mode
	move.l	d0,d2
	move.l	vb_breedte_x(a5),d1
	addq.l	#3,d2	
	lsr.l	#2,d2
	lsl.l	#2,d2
	sub.l	d2,d1
	move.w	vb_planes(a5),d6
	subq.w	#1,d6
	lsr.l	#2,d2			; longs
	subq.l	#1,d2
	bra	.in_cl2

.copy_line2:
	move.l	a2,a0
	move.w	d6,d0
.copy_pl:
	move.l	d2,d4
.copy_longs2:
	move.l	(a0)+,(a1)+
	dbf	d4,.copy_longs2
	add.l	d1,a0
	add.l	d1,a1
	dbf	d0,.copy_pl
	
	sub.l	d5,a2			; one line back
.in_cl2:
	dbf	d7,.copy_line2
	bra	.goon

.normal:	

; start copy d7 lines

	lsr.l	#2,d2			; longs
	subq.l	#1,d2
	bra	.in_cl
.copy_line:
	move.l	d2,d4
	move.l	a2,a0
.copy_longs:
	move.l	(a0)+,(a1)+
	dbf	d4,.copy_longs
	sub.l	d5,a2			; one line back
.in_cl:
	dbf	d7,.copy_line
.goon:
;	bra	.no_res


; now a1 points to the portion below the roll
; copy the original data here

	move.l	vb_tempbuffer(a5),d1
	move.l	a1,d4
	sub.l	d1,d4
	move.l	vb_tempbuffer(a4),a0	; source
	add.l	d4,a0			; set to same offset

; now how much need I copy
	move.w	db_rechts(a3),d7
	bmi	.no_res
	beq	.no_res
	move.l	a0,d1
	move.l	a1,d2
	move.l	d7,d4
	move.l	db_graphbase(a3),a6
	jsr	_LVOOwnBlitter(a6)	; blitter multi-tasking ends here
	moveq	#0,d5
	move.w	#$9f0,d6

	move.l	vb_totalwidth(a5),d7
	lsr.l	#1,d7
	lea	$dff000,a0
	moveq	#0,d0
	move.w	vb_lenx(a5),d0
	add.w	#15,d0
	lsr.w	#4,d0
	add.w	d0,d0
	cmp.l	vb_breedte_x(a5),d0
	beq	wb5			; normal full screen mode

; The to copy piece is smaller than the screen calculate modulo
; and adjust height

	mulu	vb_planes(a5),d4
	move.l	vb_breedte_x(a5),d5
	addq.l	#3,d0
	lsr.l	#2,d0
	add.l	d0,d0
	lsr.l	#1,d5
	move.l	d0,d7
	sub.l	d0,d5			; modulo between screen and blitin
	add.l	d5,d5

.wb5:	btst	#14,dmaconr(a0)		; wacht tot blitter vrij is
	bne.b	.wb5

	move.w	d5,bltdmod(a0)		; zet de modulo waarden
	move.w	d5,bltamod(a0)
	move.w	d5,bltcmod(a0)
	moveq	#0,d5
	bra	wb5_in
.no_res:
	rts

*
* hardblith1_ver3 blitter down up horizontal scroll met rol-endje
*
roll_down:
	bsr	calc_effect_speed_hor1

	bsr	create_rol_tabel_down

; reken uit na hoeveel stappen er een colorfade moet plaats vinden

	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	moveq	#0,d0
	move.w	vb_leny(a5),d0
	move.l	db_effect_speed(a3),d1

	bsr	set_fade_teller


	moveq	#0,d0
	move.l	#-ROLSIZE,db_shiftit(a3)

wwh41v3:
	move.l	db_shiftit(a3),d0

	moveq	#0,d6
	move.w	vb_leny(a5),d6
	
	move.l	db_wpatroonruimte(a3),a0
	moveq	#ROLSIZE-1,d7
rep_h1_ver3:

	movem.l	d0/d1/d6/d7/a0,-(a7)
	move.l	db_shiftit(a3),d1
	add.l	(a0),d1
	cmp.l	#0,d0
	blt	no_put8
	cmp.l	#0,d1
	blt	no_put8
	cmp.l	d6,d1
	bgt	no_put8

	cmp.l	d6,d0
	bgt	no_put8

	sub.l	db_effect_speed(a3),d0

	bsr.w	hardblithorizontal3

no_put8:
	movem.l	(a7)+,d0/d1/d6/d7/a0
	addq.l	#4,a0

	add.l	db_effect_speed(a3),d0
	dbf	d7,rep_h1_ver3

	bsr	wacht_tijd
	bne	exit_hardh441v3

	move.l	db_effect_speed(a3),d0
	add.l	d0,db_shiftit(a3)

	move.l	db_shiftit(a3),d0
	cmp.w	vb_leny(a5),d0
	bgt.b	exit_hardh441v3

	bsr	test_fade

	bra.b	wwh41v3
exit_hardh441v3:
	rts	

create_rol_tabel_down:
	moveq	#ROLSIZE,d7

	move.l	db_effect_speed(a3),d0
	move.l	db_wpatroonruimte(a3),a0
	move.l	#0,(a0)+			; eerst een nul
						; laat hele plaatje achter
	moveq	#ROLSIZE,d7
	subq.w	#1,d7
	move.l	d0,d1
	mulu	d7,d1
rep_cr_rtab_d:
	move.l	d1,(a0)+
	sub.l	d0,d1
	dbf	d7,rep_cr_rtab_d
	rts

*
* a5 wijst naar destination data 
* a4 wijst naar source data
* d0 wijst geeft de horizontale positie aan
*
* Er wordt in deze functie een word breed blok gemaskeerd neer gezet
*
hardblitvertikal:
	move.l	d0,d7			; save d0 reg

	move.l	db_graphbase(a3),a6
	jsr	_LVOOwnBlitter(a6)	; blitter multi-tasking ends here

	move.l	d7,d0

	move.l	#$dff000,a0		; hardware basis registers

	lsr.l	#3,d0
	and.l	#$fffffffe,d0
	move.l	vb_tempbuffer(a4),a1	; source pointer
	add.l	d0,a1

	move.l	vb_tempbuffer(a5),a2	; destination pointer
	add.l	d0,a2

	move.w	vb_planes(a5),d7	; Reken de blithoogte uit
	mulu.w	vb_leny(a5),d7

	tst.w	d7			; zero blit ??????
	beq.b	bulshitblit4

	move.l	vb_breedte_x(a5),d6
	lsr.w	#1,d6			; bytes -> words

	move.l	vb_breedte_x(a5),d0	; destination
	subq.w	#2,d0			; min 1 word

	move.l	d0,d1
	move.w	#-1,d5			; 1 word

	move.w	#$0fe2,d6		; de min termen voor de blitter ABx
	move.w	#$0be2,d6		; de min termen voor de blitter ABx

	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)

	move.w	d1,bltamod(a0)
	move.w	d0,bltcmod(a0)
	move.w	d0,bltdmod(a0)		; zet de modulo waarden
	move.w	d5,bltbmod(a0)

	move.l	#$ffffffff,bltafwm(a0)

	move.w	d6,bltcon0(a0)
br5:

	move.w	#0,bltcon1(a0)
	move.l	db_horizontalmask(a3),bltbpt(a0)

	move.l	a1,bltapt(a0)
	move.l	a2,bltcpt(a0)
	move.l	a2,bltdpt(a0)

	move.w	d7,bltsizv(a0)
	move.l	db_horizontalmask(a3),a6
	move.w	(a6),bltbdat(a0)
	move.w	#1,bltsizh(a0)

bulshitblit4:
	move.l	db_graphbase(a3),a6
	jsr	_LVODisownBlitter(a6)	; geef bliter weer vrij
	rts	

*
* Deze versie werkt zonder mask maar met verschil tussen source en dest
*
hardblithorizontal3:
	move.l	db_effect_speed(a3),db_effect_speed_org(a3)
	move.l	d0,d7			; save d0 reg

	moveq	#0,d6
	move.w	vb_leny(a5),d6
	cmp.l	d6,d0
	bge	no_blith

	add.l	db_effect_speed(a3),d7
	cmp.w	d6,d7
	ble	no_horb_pro3
	sub.l	d6,d7
	sub.l	d7,db_effect_speed(a3)
no_horb_pro3:
	tst.l	d0
	bpl	no_probhor3
	moveq	#0,d0
no_probhor3:

	move.l	d1,d6
	move.l	db_graphbase(a3),a6
	jsr	_LVOOwnBlitter(a6)	; blitter multi-tasking ends here

	move.l	d7,d0
	move.l	#$dff000,a0		; hardware basis registers

	move.l	vb_breedte_x(a5),d7
	mulu	vb_planes(a5),d7
	mulu	d7,d0			; vertikale offset dest
	mulu	d7,d6			; vertikale offset source 

	move.l	vb_tempbuffer(a4),d1	; source pointer
	add.l	d6,d1
	move.l	vb_tempbuffer(a5),d2	; destination pointer
	add.l	d0,d2

	bra	hardblithorizontal2

* probeer wat in het active venster te blitten 
* mbv. de hardware
* a5 wijst naar destination data 
* a4 wijst naar source data
* d0 wijst geeft de vertikale positie aan
*
* Er wordt in deze test een lijn ( gemaskeerd ) neer gezet
*
hardblithorizontal:
	move.l	db_effect_speed(a3),db_effect_speed_org(a3)

	move.l	d0,d7			; save d0 reg
	moveq	#0,d6
	move.w	vb_leny(a5),d6
	cmp.l	d6,d0
	bge	no_blith

	add.l	db_effect_speed(a3),d7
	cmp.w	d6,d7
	ble	no_horb_pro
	sub.l	d6,d7
	sub.l	d7,db_effect_speed(a3)
no_horb_pro:
	tst.l	d0
	bpl	no_probhor
	moveq	#0,d0
no_probhor:

	move.l	d0,d7			; save d0 reg
	move.l	db_graphbase(a3),a6
	jsr	_LVOOwnBlitter(a6)	; blitter multi-tasking ends here

	move.l	d7,d0
	move.l	#$dff000,a0		; hardware basis registers

	move.l	vb_breedte_x(a5),d7
	mulu	vb_planes(a5),d7	; in D1 de totale breedte 
	mulu	d7,d0			; vertikale offset 

	move.l	vb_tempbuffer(a4),d1	; source pointer
	add.l	d0,d1

	move.l	vb_tempbuffer(a5),d2	; destination pointer
	add.l	d0,d2

hardblithorizontal2:

	move.l	vb_breedte_x(a5),d7
	lsr.w	#1,d7			; bytes -> words

	move.l	db_effect_speed(a3),d4
	mulu.w	vb_planes(a5),d4
	move.l	vb_breedte_x(a5),d5
	neg.w	d5			; negatieve modulo voor hmask
	move.w	db_maskbits(a3),d6

wb5:	btst	#14,dmaconr(a0)		; wacht tot blitter vrij is
	bne.b	wb5

	move.w	#0,bltdmod(a0)		; zet de modulo waarden
	move.w	#0,bltamod(a0)
	move.w	#0,bltcmod(a0)

wb5_in:
	move.w	d5,bltbmod(a0)		; hmasker modulo

	move.l	#$ffffffff,bltafwm(a0)

	move.w	d6,bltcon0(a0)		; mintermen

	move.w	db_con1(a3),bltcon1(a0)

	move.l	db_welkmasker(a3),bltbpt(a0)

	move.l	d1,bltapt(a0)		; source
	move.l	d2,bltdpt(a0)		; destination
	move.l	d2,bltcpt(a0)		; destination

	move.w	d4,bltsizv(a0)
	move.w	d7,bltsizh(a0)

bulshitblith:
	move.l	db_graphbase(a3),a6
	jsr	_LVODisownBlitter(a6)	; geef bliter weer vrij

	move.l	db_effect_speed_org(a3),db_effect_speed(a3)
no_blith:
	rts	

maak_viewport:
	bsr		free_memory_view
	
	move.w		vb_mode(a5),d0
	move.l		db_graphbase(a3),a6
	move.l		vb_vieww(a5),a1
	jsr		_LVOInitView(a6)

	move.l		vb_vieww(a5),a1

	move.w		db_viewx(a3),v_DxOffset(a1)
	move.w		db_viewy(a3),v_DyOffset(a1)

	move.l		vb_vieww(a5),a1
	move.w		vb_mode(a5),d0
	or.w		db_or_vmode_mask(a3),d0
	and.w		db_and_vmode_mask(a3),d0
;	move.w		#$9021,d0
	move.w		d0,v_Modes(a1)
	move.l		vb_viewportw(a5),a0
	jsr		_LVOInitVPort(a6)

	move.l		vb_vieww(a5),a0
	move.l		vb_viewportw(a5),a1
	move.l		a1,v_ViewPort(a0)

	move.l		vb_bitmapw(a5),a0
	move.w		vb_planes(a5),d0		; depth
	move.l		vb_breedte_x(a5),d1		; breedte in bytes * 8
	mulu		vb_planes(a5),d1
	lsl.l		#3,d1				; width
	move.w		vb_leny(a5),d2			; height
	jsr		_LVOInitBitMap(a6)

	move.l		vb_bitmapw(a5),a0

	move.l		vb_tempbuffer(a5),d0

	move.l		d0,bm_Planes(a0)		; zet de bitplanes
	move.l		vb_breedte_x(a5),d1

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

	move.l		vb_rasinfow(a5),a0		; initialiseer rasinfo
	move.l		vb_bitmapw(a5),a1
	move.l		a1,ri_BitMap(a0)
	move.w		#0,ri_RxOffset(a0)
	move.w		#0,ri_RyOffset(a0)
	move.l		#0,ri_Next(a0)

	move.l		vb_viewportw(a5),a0
	move.l		vb_rasinfow(a5),a1
	move.l		a1,vp_RasInfo(a0)

	move.w		vb_lenx(a5),vp_DWidth(a0)
	move.w		vb_leny(a5),vp_DHeight(a0)
	move.w		vb_mode(a5),vp_Modes(a0)

	move.w	db_colums(a3),d1	; zet de offsets berekend uit de prefs offsets
	tst.b	vb_hires(a5)
	bne.b	hi1
	lsr.w	#1,d1
hi1:
	tst.b	vb_shires(a5)
	beq	no_shi4
	lsl.w	#1,d1
no_shi4:
	move.w	vb_lenx(a5),d0	; pref.offset-(breedte - normal.breedte)/2

; test de offset berekening met de byte lengte

	move.l	vb_breedte_x(a5),d0
	lsl.l	#3,d0

;	cmp.w	db_max_x(a3),d0
;	ble	.oke1
;	move.w	db_max_x(a3),d0
.oke1:

	sub.w	d1,d0
	asr.w	#1,d0
	neg.w	d0

	IFNE 0
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.temp(pc),a1
	move.w	d0,.temp
	lea	.dbstr2(pc),a0
	jsr	KPutFmt
	bra	.tt2
.dbstr2:	dc.b	"Dx is %d",10,0
	even
.tt2:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

;	bpl	.oke
;	moveq	#0,d0
;.oke:

	move.w	d0,vp_DxOffset(a0)

	bsr	get_dy_offset
	
	move.w	d0,vp_DyOffset(a0)	

	IFNE 0
	movem.l	a0-a6/d0-d7,-(a7)
	lea	vp_DyOffset(a0),a1
	lea	.dbstr(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr:	dc.b	"Trans Dy offset is %d",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

qrt:
	move.l	vb_vieww(a5),a0

	IFNE 0
	movem.l	a0-a6/d0-d7,-(a7)
	move.l	vb_vieww(a5),a0
	lea	v_DyOffset(a0),a1
	lea	.dbstr(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr:	dc.b	"Dy view offset is %d,%d",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC
prrt:
	IFNE 0
	movem.l	a0-a6/d0-d7,-(a7)
	lea	db_colums(a3),a1
	lea	.dbstr(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr:	dc.b	"kol en row %d,%d",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	db_graphbase(a3),a6
	move.l	#VIEW_EXTRA_TYPE,d0
	jsr	_LVOGfxNew(a6)
	tst.l	d0
	beq	exit_maakv

	move.l	d0,vb_vextra(a5)

	IFNE GFXSNOOP
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.temp(pc),a1
	move.l	vb_vextra(a5),(a1)
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"mv1 %d,%d",10,0
	even
.temp:	dc.l	0
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	d0,a1
	move.l	vb_vieww(a5),a0
	move.l	db_graphbase(a3),a6
	jsr	_LVOGfxAssociate(a6)

	move.l	vb_vieww(a5),a0
	or.w	#EXTEND_VSTRUCT,v_Modes(a0)

	move.l	vb_vextra(a5),a0
	move.l	db_monitorspec(a3),ve_Monitor(a0)

	move.l	db_graphbase(a3),a6
	move.l	#VIEWPORT_EXTRA_TYPE,d0
	jsr	_LVOGfxNew(a6)
	move.l	d0,vb_vpextra(a5)
	beq	exit_maakv

	IFNE GFXSNOOP
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.temp2(pc),a1
	move.l	vb_vpextra(a5),(a1)
	lea	.dbstr2(pc),a0
	jsr	KPutFmt
	bra	.tt2
.dbstr2:	dc.b	"mv2 %d,%d",10,0
	even
.temp2:	dc.l	0
.tt2:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	d0,a1
	move.l	vb_viewportw(a5),a0
	move.l	db_graphbase(a3),a6
	jsr	_LVOGfxAssociate(a6)

	move.l	db_tags(a3),a0
	move.l	vb_vpextra(a5),12(a0)

	move.w	#0,a0
	lea	vb_dimquery(a5),a1
	moveq	#dim_SIZEOF,d0
	move.l	#DTAG_DIMS,d1
	move.l	db_monid(a3),d2
	or.w	vb_mode(a5),d2
	or.w	db_or_vmode_mask(a3),d2
;	tst.w	vb_mode(a5)
;	bne	.nolow
;	and.w	#~SUPER_KEY,d2
;	or.w	#HIRES_KEY,d2
.nolow:
	move.l	db_graphbase(a3),a6
	jsr	_LVOGetDisplayInfoData(a6)

	lea	vb_dimquery(a5),a0
	move.l	vb_vpextra(a5),a1

	move.l	dim_MaxOScan(a0),vpe_DisplayClip(a1)
	move.l	dim_MaxOScan+4(a0),vpe_DisplayClip+4(a1)

	move.l	db_monid(a3),d0
	or.w	vb_mode(a5),d0
;	or.w	db_or_vmode_mask(a3),d0
	move.l	db_graphbase(a3),a6
	move.w	vb_mode(a5),d3
	and.w	#~$884,d3
	bne	.nolow2
	and.w	#$7fdf,d0
	or.w	db_alternate_lowres(a3),d0
.nolow2:
;	and.l	#~$4,d0

	jsr	_LVOFindDisplayInfo(a6)
	move.l	db_tags(a3),a0
	move.l	d0,20(a0)		
no_v36_1:
	IFNE 0
	movem.l	a0-a6/d0-d7,-(a7)
	lea	20(a0),a1
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"display in trans is %d,%d",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	moveq		#32,d0
no_pro_th5:
	tst.b		db_aa_present(a3)
	beq		no_aa51
	move.l		#256,d0				; voor de blend ??????
no_aa51:
	move.l		db_graphbase(a3),a6
	jsr		_LVOGetColorMap(a6)
	tst.w	d0
	beq	no_colormap

	move.l	d0,a0
	move.l	db_tags(a3),a1
	move.l	vb_viewportw(a5),4(a1)
	move.l	db_graphbase(a3),a6
	jsr	_LVOVideoControl(a6)

	bsr		laad_kleuren

.no_ucop_mem:
	move.l		db_graphbase(a3),a6
	move.l		vb_vieww(a5),a0
	move.l		vb_viewportw(a5),a1
	jsr		_LVOMakeVPort(a6)

	move.l		vb_vieww(a5),a1
	move.w		v_Modes(a1),d0
	jsr		_LVOMrgCop(a6)

exit_maakv:
;	btst	#10,$dff016
;	bne	exit_maakv
	rts

get_dy_offset:
	move.w	db_rows(a3),d1
	move.w	vb_leny(a5),d0

	lsl.w	#1,d1
	lsl.w	#1,d0

	tst.b	vb_interlace(a5)
	beq.w	.no_inter1
	lsr.w	#1,d0			; was already lace
.no_inter1:
	and.w	#$fffe,d0
	sub.w	d1,d0
	neg.w	d0

	asr.w	#1,d0

	tst.w	d0
	bge	.okie
	moveq	#0,d0
.okie:
	tst.b	vb_interlace(a5)
	bne.s	.inter5
	asr.w	#1,d0
	rts
.inter5:
	and.w	#$fffe,d0
	rts

	IFNE	0
*
* In a view a1
*
find_gen_and_switch:
	move.l		a1,a2
	move.l		v_LOFCprList(a2),a0
	move.l		v_SHFCprList(a2),a1

	cmp.l	#0,a1
	beq	exit_cc

	move.l		crl_Next(a1),a2
	move.w		crl_MaxCount(a0),d2		; aantal elementen
;	add.w		d2,d2
	move.l		crl_start(a0),a0		; eigenlijke copperlist
	move.l		crl_start(a1),a1

	subq.l		#1,d2
	move.w	d2,d7
.rep_copcop2:
	move.w	(a0)+,d0
	cmp.w	#$ffff,d0
	beq	hh
	cmp.w	#$100,d0
	bne	.no_diver1
	and.w	#$fefd,(a0)
.no_diver1:
	cmp.w	#$104,d0
	bne	.no_diver2
	and.w	#$81ff,(a0)
	or.w	#$0400,(a0)
.no_diver2:
	cmp.w	#$106,d0
	bne	.no_diver3
;	and.w	#$81ff,(a0)
	nop
	nop

.no_diver3:
	addq.l	#2,a0
	dbf	d2,.rep_copcop2
hh:

.rep_copcop2:
	move.w	(a1)+,d0
	cmp.w	#$ffff,d0
	beq	exit_copcopy2
	cmp.w	#$100,d0
	bne	.no_diver1
	and.w	#$fefd,(a1)
.no_diver1:
	cmp.w	#$104,d0
	bne	.no_diver2
	and.w	#$81ff,(a1)
	or.w	#$0400,(a1)
.no_diver2:
	addq.l	#2,a1
	dbf	d7,.rep_copcop2
exit_copcopy2:
exit_cc:
	rts
	ENDC
		
convert_shires_hires:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Convert Shires hires",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	bsr	convert_hi_low
	clr.b	vb_shires(a4)
	and.w	#$ffdf,vb_mode(a4)
	bra	showinactive

convert_hires_lowres:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Convert Hires lowres",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC
	bsr	convert_hi_low
	clr.b	vb_hires(a4)
	and.w	#$7fff,vb_mode(a4)

showinactive:
	move.l		db_inactive_viewblok(a3),a5
	bsr		maak_viewport
showinactive2:
	move.l		db_inactive_viewblok(a3),a5
	move.l		db_graphbase(a3),a6
	move.l		vb_vieww(a5),a1
	jsr		_LVOLoadView(a6)

	move.l		db_inactive_viewblok(a3),d0
	move.l		db_active_viewblok(a3),db_inactive_viewblok(a3)
	move.l		d0,db_active_viewblok(a3)

	bsr		set_viewed

	move.l		db_graphbase(a3),a6
	jsr		_LVOWaitTOF(a6)		; wait until view loaded

;ww1:	btst	#10,$dff016
;	bne	ww1

	rts

* find_chipmem zoekt in het systeem naar een stuk van d0 bytes aan chipmem
* dit geheugen hoeft niet vast gezet te worden en kan dus een stuk van
* een van de chipmem buffers zijn.
* Deze functie houdt geen rekening met eerder toegezegde stukken geheugen
*
* Het is uiteraard voor een wipe maar toegestaan deze functie een keer
* aan te roepen.
*
* Het resultaat wordt in a0 teruggegeven
*
find_chipmem:
	move.l	db_inactive_viewblok(a3),a4
	move.l	vb_tempbuffer(a4),a0		; de data al in chip
	add.l	d0,a0
	rts

*
* Copy the picture data to restore the picture
* Use the clip_width
* The height is in d1
* The y coordinate is in d6
*
restore_pic_xy:
	bsr	restore_pic_xy_later
	bsr	actual_restore
	rts
	
restore_pic_xy_later:
	clr.l	db_res_source(a3)		; clear the old values
	move.w	vb_leny(a5),d4
	tst.w	vb_clip_width(a5)
	beq	no_restore

	move.w	d6,d7
	bge	oke_res_y
	add.w	d1,d6
	bgt	oke_res_y2
	bra	no_restore
oke_res_y2:
	moveq	#0,d6
	add.w	d7,d1
oke_res_y:

	cmp.w	vb_leny(a5),D6
	bge	no_restore
	
	move.w	d6,d5
	add.w	d1,d5				; finale height
	cmp.w	vb_leny(a5),d5
	ble	oke_res_y3
ptt2:
	sub.w	vb_leny(a5),d5
	sub.w	d5,d1
oke_res_y3:

	move.l	vb_breedte_x(a5),d7
	mulu	vb_planes(a5),d7
	mulu	d6,d7

	move.l	d7,d5
	moveq	#0,d7
	move.w	vb_clip_x(a5),d7
	bgt	no_x_neg1
	moveq	#0,d7
	subq.l	#2,d5
no_x_neg1:
	lsr.w	#4,d7
	add.w	d7,d7
	move.l	d7,d6

	addq.l	#2,d5

	add.l	d7,d5
	move.l	db_waarsourcepic(a3),a1
	add.l	d5,a1				; address dest restore 
	move.l	db_waardestpic(a3),a2
	add.l	d5,a2				; address source restore

	move.l	vb_breedte_x(a5),d5
	moveq	#0,d2
	move.w	vb_clip_width(a5),d2		; width clip in bytes
	sub.l	d2,d5
	move.w	db_blitin_clear(a3),d6
	mulu	vb_planes(a5),d1
	lsr.w	#1,d2				; set on words
;	subq.w	#1,d2

; store the info and perform the clear later
; this ensures that the new image is placed before removing the old one
; specialy when using with non double buffering

	move.l	a2,db_res_source(a3)
	move.l	a1,db_res_dest(a3)
	move.w	d1,db_res_height(a3)
	move.w	d2,db_res_width(a3)
	move.l	d5,db_res_modulo(a3)
	rts

actual_restore:
	move.l	db_res_source(a3),d0
	beq	.no_restore
	move.l	d0,a2
	clr.l	db_res_source(a3)
	move.l	db_res_dest(a3),a1
	move.w	db_res_height(a3),d1
	move.w	db_res_width(a3),d2
	move.l	db_res_modulo(a3),d5

	move.w	db_blitin_clear(a3),d3
	cmp.w	#$100,db_blitin_clear(a3)
	beq	incpufast

	btst	#0,d2
	beq	longword2
	lsr.w	#1,d2
	subq.w	#1,d2
	bmi	single_word1
	bra	.incpu1
.repcpu1:
	move.l	d2,-(a7)
.repcpu2:
	move.l	(a2)+,(a1)+
	dbf	d2,.repcpu2
	move.w	(a2)+,(a1)+
	add.l	d5,a2
	add.l	d5,a1
	move.l	(a7)+,d2
.incpu1:
	dbf	d1,.repcpu1
.no_restore:
	rts

single_word1:
	move.w	(a2)+,(a1)+
	add.l	d5,a2
	add.l	d5,a1
	dbf	d1,single_word1
	rts
	
longword2:
	add.w	#1,d2
	lsr.w	#1,d2
	subq.w	#1,d2
	bmi	single_word1
	bra	.incpu1
.repcpu1:
	move.l	d2,-(a7)
.repcpu2:
	move.l	(a2)+,(a1)+
	dbf	d2,.repcpu2
	add.l	d5,a2
	add.l	d5,a1
	move.l	(a7)+,d2
.incpu1:
	dbf	d1,.repcpu1
.no_restore:
	rts

single_word2:
	clr.w	(a1)+
	add.l	d5,a1
	dbf	d1,single_word2
	rts

incpufast:
	btst	#0,d2
	beq	longplusword
	lsr.w	#1,d2
	subq.w	#1,d2
	bmi	single_word2
	bra	.incpufast
.repcpu3:
	move.l	d2,-(a7)
.repcpu4:
	clr.l	(a1)+
	dbf	d2,.repcpu4
	clr.w	(a1)+
	add.l	d5,a1
	move.l	(a7)+,d2
.incpufast:
	dbf	d1,.repcpu3
	rts

longplusword:
	add.w	#1,d2
	lsr.w	#1,d2
	subq.w	#1,d2
	bmi	single_word2
	bra	.incpufast
.repcpu3:
	move.l	d2,-(a7)
.repcpu4:
	clr.l	(a1)+
	dbf	d2,.repcpu4
	add.l	d5,a1
	move.l	(a7)+,d2
.incpufast:
	dbf	d1,.repcpu3
	rts

	move.l	db_graphbase(a3),a6
	jsr	_LVOOwnBlitter(a6)	; blitter multi-tasking ends here

	move.l	vb_breedte_x(a5),d5
	moveq	#0,d2
	move.w	vb_clip_width(a5),d2		; width clip in bytes
	sub.l	d2,d5
	move.w	db_blitin_clear(a3),d6
;	or.w	#$ff,d6
	mulu	vb_planes(a5),d1
	lea	$dff000,a0
wbl7b:	btst	#14,dmaconr(a0)		; wacht tot blitter vrij is
	bne.b	wbl7b
	move.w	d5,bltamod(a0)
	move.w	d5,bltdmod(a0)		; zet de modulo destination
	move.w	d6,bltcon0(a0)
	move.l	#$ffffffff,bltafwm(a0)
	move.w	#0,bltcon1(a0)
	move.l	a2,bltapt(a0)
	move.l	a1,bltdpt(a0)
	lsr.w	#1,d2			; words voor breedte blit
	move.w	d1,bltsizv(a0)
	move.w	d2,bltsizh(a0)
	move.l	db_graphbase(a3),a6
	jsr	_LVODisownBlitter(a6)	; geef bliter weer vrij
no_restore:
	rts

*
* Zet de blitin uit de inactive buffer in het plaatje
* De masker pointer staat in blitinmask
* In d0,d1 staan de x en y coordinaat waar het plaatje geplaatst moet worden
* Before the actual blit you must first remove the remains of the old blit
* This is only true if the blit moved verticaly
* the horizontal movement is wiped out by the blit itself
*
put_blitin_in_new:

pttt:
	move.l	db_inactive_fileblok(a3),a4
	move.l	db_active_viewblok(a3),a5
	moveq	#0,d5
	move.w	vb_clip_y(a5),d5			; old value
	cmp.w	d5,d1
	beq	check_x_movement
	bgt	moved_down

*
* The clip is moved up so remove the portion below the clip
*
moved_up:
	movem.l	d0-d3/a0-a2,-(a7)
	move.w	d1,d6
	exg	d5,d1
	sub.w	d5,d1
	move.w	vb_clip_height(a5),d4

	move.w	vb_clip_y(a5),d6			; from this position
	add.w	vb_clip_height(a5),d6
	sub.w	d1,d6

	tst.b	db_double_buf(a3)
	beq	.no_b1
;	add.w	d1,d1
.no_b1:
	bsr	restore_pic_xy_later

	movem.l	(a7)+,d0-d3/a0-a2
	bra	check_x_movement
*
* The clip is moved down so remove the portion above the clip
* This is the easy one because you stil know the adress and the width
*
moved_down:
	movem.l	d0-d3/a0-a2,-(a7)
	sub.w	d5,d1				; restore height
	move.w	vb_clip_y(a5),d6
	bsr	restore_pic_xy_later
	movem.l	(a7)+,d0-d3/a0-a2
	bra	check_x_movement

*
* The Y movement is handled set the variables
* that crop the x length of the blit
*
check_x_movement:
	move.l	#4,db_blit_offstart(a3)
	move.l	#0,db_blit_offstop(a3)
	cmp.w	vb_clip_x(a5),d0
	beq	same_x1
	bgt	moved_to_right
	move.w	vb_clip_x(a5),d7
	sub.w	d0,d7
	move.l	#4,db_blit_offstart(a3)
	bra	put_blitin_in

moved_to_right:
	moveq	#0,d7
	move.w	d0,d7
	asr.w	#4,d7
	move.w	vb_clip_x(a5),d6
	asr.w	#4,d6
	sub.w	d6,d7
	add.w	d7,d7
	sub.l	d7,db_blit_offstart(a3)
	move.l	#4,db_blit_offstop(a3)
	bra	put_blitin_in

*
* There is no x movement so there is no need to add the two extra words
*
same_x1:
	move.l	#4,db_blit_offstart(a3)
	move.l	#4,db_blit_offstop(a3)
	bra	put_blitin_in

put_blitin_in2_nostst:			; the calling needs to set the offstart
	tst.w	d3			; and the offstop
	beq	no_b1
	bra	put_blitin_in22

put_blitin_in3:
	move.l	db_inactive_fileblok(a3),a4
	move.l	db_active_viewblok(a3),a5
	moveq	#0,d5
	moveq	#0,d3
	move.w	vb_leny(a4),d3

put_blitin_in2:				; in d3 nu de hoogte
	tst.w	d3
	beq	no_b1
	move.l	#4,db_blit_offstart(a3)
	move.l	#4,db_blit_offstop(a3)
	bra	put_blitin_in22
*
* Zet de blitin uit de inactive buffer in het plaatje
* De masker pointer staat in blitinmask
* In d0,d1 staan de x en y coordinaat waar het plaatje geplaatst moet worden
*
put_blitin_in:
	move.l	db_inactive_fileblok(a3),a4
	move.l	db_active_viewblok(a3),a5
	moveq	#0,d5

	move.w	db_blitin_fy(a3),d3
	move.w	vb_leny(a4),d3

put_blitin_in22:				; in d3 nu de hoogte
	move.w	d3,vb_clip_height(a5)
	move.w	d0,vb_clip_x(a5)
	move.w	d1,vb_clip_y(a5)
	moveq	#0,d7
	move.w	d3,d7		
	add.w	d1,d7
	move.w	vb_leny(a5),d6
	cmp.w	vb_leny(a5),d7
	ble	b_in_nopro2
	sub.w	vb_leny(a5),d7
	sub.w	d7,d3
	ble	no_blit
b_in_nopro2:
	tst.w	d1
	bpl	b_in_nopro4
	move.w	d1,d6
	neg.w	d6
	add.l	d6,d5
	add.w	d1,d3		; pas lengte aan
	moveq	#0,d1
	tst.w	d3
	ble	no_blit
b_in_nopro4:
	tst.w	d0
	bpl	b_in_nopro3	; een negative blit

	moveq	#0,d4
	move.w	d0,d4
	
	neg.w	d4		; x offset van blitin

	moveq	#0,d2
	move.w	vb_lenx(a4),d2
	sub.w	d4,d2		; overgebleven lengte van blitin
	bmi	no_blit		; te ver naar links geschoven
	beq	no_blit
b_in_no_po1:
	bsr	test_blit_inactive2
	bra	end_p_b

b_in_nopro3:
	moveq	#0,d2

	move.w	vb_lenx(a4),d2		; debug only
	moveq	#0,d7
	move.w	vb_lenx(a4),d7		
	add.l	d0,d7

	cmp.w	vb_lenx(a5),d7
	ble	b_in_nopro1
					; blitin valt buiten de rand
	sub.w	vb_lenx(a5),d7
	sub.w	d7,d2
	bmi	no_blit
	beq	no_blit

	move.w	db_blitin_con0(a3),d6
;	bra	.no
;	or.w	#$ff,db_blitin_con0(a3)
;.no:
	move.l	d6,-(a7)
	bsr	test_blit_inactive4
	move.l	(a7)+,d6
	move.w	d6,db_blitin_con0(a3)

no_blit:
	bra	end_p_b
	
b_in_nopro1:

	bsr	test_blit_inactive

end_p_b:

; Delete here the portion below or above the bob

	bsr	actual_restore

	tst.b	db_double_buf(a3)
	beq	no_b1

	tst.b	db_please_no_double(a3)
	bne	no_b1

wbl17:	btst	#14,$dff000+dmaconr		; wacht tot blitter vrij is
	bne.b	wbl17

	bsr	showinactive2

	move.l	db_inactive_viewblok(a3),a5
	move.l	vb_tempbuffer(a5),db_waarsourcepic(a3)
no_b1:

;	bsr	reshowpicture

	rts

*
* blit de blitin in het plaatje
* d0,d1 geven x en y aan
* d3 geeft de hoogte van de blit aan
* d5 geeft de y offset in de blitin aan
*
test_blit_inactive:
	move.l	db_inactive_fileblok(a3),a4
	move.w	vb_leny(a4),d6
	move.w	d3,d7
	add.w	d5,d7
	cmp.w	d6,d7
	ble	.fit
	sub.w	d6,d7
	sub.w	d7,d3
	beq	no_b1			; don't blit 
	bmi	no_b1
.fit:
	tst.b	db_double_buf(a3)
	bne	skip_wait1
	move.l	db_inactive_fileblok(a3),a4
	move.l	db_active_viewblok(a3),a5
	bsr	waitline1
skip_wait1:
	move.w	#$0,db_Alastword(a3)
	move.l	d0,d6
	and.l	#$f,d6			; shift waarde	destination
	move.l	d6,db_shiftdest(a3)
	beq	no_A_lastw
	move.l	#$ffff0000,d7
	lsr.l	d6,d7
	move.w	d7,db_Alastword(a3)

no_A_lastw:
	lsr.l	#4,d0			; op een heel word gezet
	add.l	d0,d0			; bytes

	move.l	vb_breedte_x(a4),d7
	mulu	d7,d5
	mulu	vb_planes(a5),d5

	moveq	#0,d7
	
	add.l	db_blit_offstart(a3),d5
	
	move.l	db_blitinpic(a3),a1	; source pointer
	add.l	d5,a1
	move.l	db_blitinmask(a3),a6
	add.l	d5,a6

	move.l	vb_breedte_x(a5),d5	; moet buiten de blit functie !!!!!!!!!
	mulu	vb_planes(a5),d5

	mulu	d1,d5			; offset in y richting
	add.l	d0,d5

	add.l	db_blit_offstart(a3),d5
	
	move.l	a6,-(a7)		; ??????????????????
	move.l	db_graphbase(a3),a6
	jsr	_LVOOwnBlitter(a6)	; blitter multi-tasking ends here
	move.l	(a7)+,a6

	move.l	db_waarsourcepic(a3),a2
	add.l	d5,a2

	moveq	#0,d1
	move.l	vb_breedte_x(a4),d2

	sub.l	db_blit_offstart(a3),d2
	move.l	db_blit_offstart(a3),d1

	move.b	#0,largetest

	cmp.l	vb_breedte_x(a5),d2
	ble	no_pro17

	move.l	vb_breedte_x(a5),d2
	subq.l	#2,d2
	move.b	#1,largetest
	
no_pro17:
	tst.w	db_Alastword(a3)
	beq	no_add_blit_word1
	addq.l	#2,d2			; extra word om te schuiven
	subq.l	#2,d1			; source modulo
no_add_blit_word1:

	move.w	vb_planes(a5),d7	; Reken de blithoogte uit
	mulu.w	d3,d7			; kan ook buiten de blit func !!!!!!!

	move.l	vb_breedte_x(a5),d0	; destination
	sub.w	d2,d0			; min breedte aantal bytes

	move.w	db_blitin_con0(a3),d6	; de min termen voor de blitter

	move.l	db_shiftdest(a3),d4
	swap	d4
	lsr.l	#4,d4
	or.w	d4,d6			; schuif source 

	move.w	d2,vb_clip_width(a5)

	add.l	db_blit_offstop(a3),d1
	sub.l	db_blit_offstop(a3),d2
	add.l	db_blit_offstop(a3),d0

	move.l	db_waardestpic(a3),a4
	add.l	d5,a4

	move.l	#$dff000,a0		; hardware basis registers

wbl7:	btst	#14,dmaconr(a0)		; wacht tot blitter vrij is
	bne.b	wbl7

	move.w	#0,bltcon1(a0)
	move.l	#$ffffffff,bltafwm(a0)

	tst.b	db_screen_empty(a3)
	bne	try_to_fast_blit

	move.w	d1,bltamod(a0)
	move.w	d1,bltbmod(a0)
	move.w	d0,bltcmod(a0)
	move.w	d0,bltdmod(a0)		; zet de modulo destination

	tst.w	db_Alastword(a3)
	beq	no_l
	move.w	#$0,bltalwm(a0)
no_l:
	move.w	d6,bltcon0(a0)

	move.l	db_shiftdest(a3),d4
	swap	d4
	lsr.l	#4,d4
	move.w	d4,bltcon1(a0)

	move.l	a6,bltapt(a0)
	move.l	a1,bltbpt(a0)
	move.l	a4,bltcpt(a0)
	move.l	a2,bltdpt(a0)

	lsr.w	#1,d2			; words voor breedte blit

	move.w	d7,bltsizv(a0)
	move.w	d2,bltsizh(a0)

	move.l	db_graphbase(a3),a6
	jsr	_LVODisownBlitter(a6)	; geef blitter weer vrij
	rts	

largetest:	dc.w	0
storetregs:	blk.l	8
storeregs:	dc.l	0

try_to_fast_blit:
	move.w	d1,bltamod(a0)
	move.w	d0,bltdmod(a0)		; zet de modulo destination

	and.w	#$f000,d6
	or.w	#$9f0,d6
	move.w	d6,bltcon0(a0)

	move.l	a1,bltapt(a0)
	move.l	a2,bltdpt(a0)

	lsr.w	#1,d2			; words voor breedte blit

	move.w	d7,bltsizv(a0)
	move.w	d2,bltsizh(a0)

	bra	endblit
	
	move.l	db_graphbase(a3),a6
	jsr	_LVODisownBlitter(a6)	; geef bliter weer vrij

;	move.w	$dff002,d0
	rts	

*
* probeer dit in descending mode
*
* blit de blitin in het plaatje ( speciaal voor de linkerkant van het plaatje )
* |->      |
* d0,d1 geven x en y aan
* d2 geeft de breedte aan in pixels
* d3 geeft de hoogte van de blit aan
* d4 geeft de postitieve offset aan in het source plaatje ( x as )
*
test_blit_inactive2:
	subq.l	#2,db_blit_offstop(a3)
	moveq	#0,d0
	tst.b	db_double_buf(a3)
	bne	skip_wait2
	move.l	db_inactive_fileblok(a3),a4
	move.l	db_active_viewblok(a3),a5
	bsr	waitline1

skip_wait2:
	move.l	d4,d6
	add.w	#15,d6
	lsr.w	#4,d6
	add.w	d6,d6
	neg.l	d6
	addq.l	#4,d6
	move.l	d6,storeregs

	move.l	d4,d6
	and.l	#$f,d6			; shift waarde source
	move.l	d6,db_shiftdest(a3)

	move.l	#$ffff0000,d7
	lsr.l	d6,d7			; firstword mask
	swap	d7
	move.w	d7,db_Afirstword(a3)

	lsr.l	#4,d4
	add.l	d4,d4			; source offset en mask offset

	add.l	d6,d2
	add.l	#15,d2
	lsr.l	#4,d2
	add.l	d2,d2			; bytes breedte blit

	moveq	#0,d7
	move.w	d2,d7
	sub.l	db_blit_offstop(a3),d7
	ble	no_blit1

	addq.l	#2,d2
	move.b	#0,largetest
	cmp.l	vb_breedte_x(a5),d2	; check the total width of the blit
	ble	no_po15			; with the picture size
tt2:
	move.b	#1,largetest
	move.l	vb_breedte_x(a5),d2	; set width to picture width
	tst.l	db_blit_offstop(a3)
	bmi	.no_pro
	add.l	db_blit_offstop(a3),d2
	add.l	db_blit_offstop(a3),d2
.no_pro:
	move.l	#$ffff0000,d7
	lsr.l	d6,d7
	move.w	d7,db_Alastword(a3)
	eor.w	#$ffff,d7
	move.w	d7,db_Afirstword(a3)
no_po15:
	subq.l	#2,d2
	move.l	vb_breedte_x(a4),d7
	add.l	d3,d5			; einde y pointer
	mulu	d7,d5
	move.w	vb_planes(a5),d6
	mulu	d6,d5
	add.l	d4,d5

	sub.l	d7,d5

	sub.l	db_blit_offstop(a3),d5

	move.l	db_blitinpic(a3),a1	; source pointer
	subq.l	#2,d5

	add.l	d2,d5

	add.l	d5,a1

	move.l	db_blitinmask(a3),a6
	add.l	d5,a6

	move.l	vb_breedte_x(a5),d5	; moet buiten de blit functie !!!!!!!!!
	mulu	vb_planes(a5),d5

	add.l	d3,d1			; einde y pointer dest
	
	mulu	d1,d5			; offset in y richting
	add.l	d0,d5
	subq.l	#2,d5
	add.l	d2,d5
	sub.l	vb_breedte_x(a5),d5

	move.l	a6,-(a7)
	move.l	db_graphbase(a3),a6
	jsr	_LVOOwnBlitter(a6)	; blitter multi-tasking ends here
	move.l	(a7)+,a6

	sub.l	db_blit_offstop(a3),d5

	move.l	db_waarsourcepic(a3),a2	; destination pointer
	add.l	d5,a2

	move.l	vb_breedte_x(a4),d1
	sub.l	d2,d1			; masker modulo

	move.w	vb_planes(a5),d7	; Reken de blithoogte uit
	mulu.w	d3,d7			; kan ook buiten de blit func !!!!!!!

	move.l	vb_breedte_x(a5),d0	; destination
	sub.w	d2,d0			; min breedte aantal bytes

	move.w	db_blitin_con0(a3),d6

	move.l	db_shiftdest(a3),d4
	swap	d4
	lsr.l	#4,d4
	or.w	d4,d6			; schuif source 

	move.l	db_waardestpic(a3),a4
	add.l	d5,a4

	move.w	d2,vb_clip_width(a5)

	add.l	db_blit_offstop(a3),d1
	sub.l	db_blit_offstop(a3),d2
	add.l	db_blit_offstop(a3),d0
	move.l	db_shiftdest(a3),d4
	swap	d4
	lsr.l	#4,d4
	or.w	#2,d4			; descending mode

	tst.b	largetest
	beq	.no

	lea	storeregs(pc),a0
	movem.l	a1/a2/a4/a6/d7/d6/d4,-(a0)
.no:

	move.l	#$dff000,a0		; hardware basis registers

wbl117:	btst	#14,dmaconr(a0)		; wacht tot blitter vrij is
	bne.b	wbl117

	move.w	d4,bltcon1(a0)
	move.w	db_Afirstword(a3),bltalwm(a0)
	move.w	db_Alastword(a3),bltafwm(a0)

	tst.b	db_screen_empty(a3)
	bne	try_to_fast_blit

	move.w	d1,bltamod(a0)
	move.w	d1,bltbmod(a0)
	move.w	d0,bltcmod(a0)
	move.w	d0,bltdmod(a0)		; zet de modulo destination

	move.w	d6,bltcon0(a0)

	move.l	a6,bltapt(a0)
	move.l	a1,bltbpt(a0)
	move.l	a4,bltcpt(a0)
	move.l	a2,bltdpt(a0)

	lsr.w	#1,d2			; words voor breedte blit

	move.w	d7,bltsizv(a0)
	move.w	d2,bltsizh(a0)

endblit:
	tst.b	largetest
	beq	.no_large

; the blit was larger than the screen so there 
; is a word row missing on the right

	lea	storeregs-4*7(pc),a0
	movem.l	(a0)+,a1/a2/a4/a6/d7/d6/d4
	moveq	#2,d2
	move.l	a4,a0
	move.l	db_inactive_fileblok(a3),a4
	move.l	vb_breedte_x(a4),d1
	sub.l	d2,d1			; masker modulo
	move.l	a0,a4

	add.l	storeregs(pc),a6
	add.l	storeregs(pc),a1

;	addq.l	#2,a2
;	addq.l	#2,a4

;	move.w	db_Alastword(a3),d0
;	move.w	db_Afirstword(a3),d0
;	or.w	d0,db_Alastword(a3)

	move.l	vb_breedte_x(a5),d0	; destination
	sub.l	d2,d0			; min breedte aantal bytes

	clr.b	largetest

	move.w	#$ffff,db_Afirstword(a3)
	move.w	#$ffff,db_Alastword(a3)
	move.l	#$dff000,a0		; hardware basis registers
	and.w	#$0fff,d6
	and.w	#$0fff,d4
	bra	wbl117

.no_large:

	move.l	db_graphbase(a3),a6
	jsr	_LVODisownBlitter(a6)	; geef bliter weer vrij

no_blit1:
	rts

*
* test het rechterkant blitje
* using descending mode
*
* blit de blitin in het plaatje
* d0,d1 geven x en y aan
* d2 geeft de breedte aan in pixels
* d3 geeft de hoogte van de blit aan
* d5 indicates the y-offset in the blit
*
test_blit_inactive4:

	tst.b	db_double_buf(a3)
	bne	skip_wait3

	move.l	db_inactive_viewblok(a3),a4
	move.l	db_active_viewblok(a3),a5
	bsr	waitline1
skip_wait3:
	move.l	d0,d6
	and.l	#$f,d6			; shift value destination
	moveq	#$10,d7
	sub.b	d6,d7
	cmp.w	#$10,d7
	bne	no_po14
	sub.l	#16,d0
no_po14:
	and.l	#$f,d7
	move.l	d7,db_shiftdest(a3)	; shift to the left

	move.l	d2,d7
	lsr.l	#4,d7
	add.l	d7,d7
	move.l	d7,d4			; width blit in bytes

	move.l	d2,d6
	lsl.l	#3,d7	
	sub.l	d7,d6			; pixel restvalue in d6

	move.l	#$ffff0000,d7
	lsr.l	d6,d7			; masker laatste word
	and.l	#$ffff,d7
	move.w	d7,db_Alastword(a3)

	move.l	d4,d2

	tst.w	d2
	beq	no_blit_4		; don't blit with zero value

	cmp.l	vb_breedte_x(a5),d2	; check the total width of the blit
	ble	no_po16			; with the picture size
	move.l	vb_breedte_x(a5),d2	; set width to picture width

	subq.l	#2,d2
no_po16:
	move.w	#$ffff,db_Afirstword(a3)

	lsr.l	#4,d0			; set on words
	add.l	d0,d0			; bytes

	move.l	vb_breedte_x(a4),d7
	add.l	d3,d5			; end y pointer

	mulu	d7,d5
	mulu	vb_planes(a5),d5	; d5 points to the last byte
					; at the start of the blit

	add.l	d2,d5			; at the end of the blit

 	sub.l	vb_breedte_x(a4),d5	; ??
	
	move.l	db_blitinpic(a3),a1	; source pointer
	add.l	d5,a1
	move.l	db_blitinmask(a3),a6
	add.l	d5,a6

	move.l	vb_totalwidth(a5),d5

	add.l	d3,d1			; end y pointer dest
	mulu	d1,d5			; offset in y 
	add.l	d0,d5			; add x bytes

	add.l	d2,d5			; add width blit

	sub.l	vb_breedte_x(a5),d5

	move.l	a6,a2			; remember a6
	move.l	db_graphbase(a3),a6
	jsr	_LVOOwnBlitter(a6)	; blitter multi-tasking ends here
	move.l	a2,a6

	move.l	db_waarsourcepic(a3),a2

	addq.l	#2,d5

	add.l	d5,a2

	move.l	vb_breedte_x(a4),d1
	sub.l	d2,d1

	move.l	#$dff000,a0		; hardware basis registers

	move.w	vb_planes(a5),d7	; Reken de blithoogte uit
	mulu.w	d3,d7			; kan ook buiten de blit func !!!!!!!

	move.l	vb_breedte_x(a5),d0	; destination
	sub.w	d2,d0			; min breedte aantal bytes

	move.w	db_blitin_con0(a3),d6
	move.l	db_shiftdest(a3),d4
	swap	d4
	lsr.l	#4,d4
	or.w	d4,d6			; schuif source 

	move.l	db_waardestpic(a3),a4
	add.l	d5,a4

	move.w	d2,vb_clip_width(a5)

	sub.l	db_blit_offstop(a3),d1
	add.l	db_blit_offstop(a3),d2
	sub.l	db_blit_offstop(a3),d0

;	add.l	db_blit_offstart(a3),d0
;	add.l	db_blit_offstart(a3),d1
;	sub.l	db_blit_offstart(a3),d2
	
	move.l	db_shiftdest(a3),d4
	swap	d4
	lsr.l	#4,d4
	or.w	#2,d4			; descending mode

wbl27:	btst	#14,dmaconr(a0)		; wacht tot blitter vrij is
	bne.b	wbl27

	move.w	db_Alastword(a3),bltafwm(a0)
	move.w	db_Afirstword(a3),bltalwm(a0)

	move.w	d4,bltcon1(a0)

	tst.b	db_screen_empty(a3)
	bne	try_to_fast_blit

	move.w	d1,bltamod(a0)
	move.w	d1,bltbmod(a0)
	move.w	d0,bltcmod(a0)
	move.w	d0,bltdmod(a0)		; zet de modulo destination

	move.w	d6,bltcon0(a0)

	move.l	a6,bltapt(a0)
	move.l	a1,bltbpt(a0)
	move.l	a4,bltcpt(a0)
	move.l	a2,bltdpt(a0)

	lsr.w	#1,d2			; words voor breedte blit

	move.w	d7,bltsizv(a0)
	move.w	d2,bltsizh(a0)

	move.l	db_graphbase(a3),a6
	jsr	_LVODisownBlitter(a6)	; geef bliter weer vrij
no_blit_4:
	rts	
*
* maak_mask_blitin maakt van alle planes een masker
* dit is nodig om de pixels die niet in de blitin zitten maar het plaatje
* wel beinvloeden uit te zetten
*
maak_mask_blitin:
	tst.b	db_blitin_mask_ready(a3)
	bne	mask_al_ready
*
* Is there enought memory in the buffer for double buffering
* if so take action to install double buffering
* ie copy the blitin below the picture
* and copy the mask below the second buffer
* the triple buffer still holds the original picture
*
check_double_buffering:
	move.b	#0,db_double_buf(a3)

	move.l	db_inactive_viewblok(a3),a4
	move.l	vb_tempbuffer(a4),db_blitinpic(a3)	; for non double buf
	move.l	vb_tempbuffer(a4),db_blitinmask(a3)

	move.l	db_inactive_fileblok(a3),a5
	move.l	vb_breedte_x(a4),vb_breedte_x(a5)
	move.w	vb_leny(a4),vb_leny(a5)
;	move.w	vb_lenx(a4),d0
	move.w	vb_lenx(a4),vb_lenx(a5)

	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_viewblok(a3),a4
	move.l	vb_breedte_x(a4),d0
	mulu	vb_planes(a5),d0
	move.w	vb_leny(a4),d1
	mulu	vb_leny(a4),d0			; size of blitin

	move.l	vb_breedte_x(a5),d1
	mulu	vb_planes(a5),d1
	mulu	vb_leny(a5),d1			; size of picture

	move.l	d0,d2
	add.l	d1,d0				; total size

	IFNE XAPP
	move.l	vb_mlscrpointer(a5),a2
	move.l	sbu_Size(a2),d3
	ELSE
	move.l	#MEMSIZE,d3
	ENDC

	cmp.l	d3,d0			; double buffers off
	bgt	no_d_pos1

	move.l	db_inactive_viewblok(a3),a4	; copy data below picture

	move.l	vb_tempbuffer(a4),a0
	move.l	vb_tempbuffer(a5),a1
	add.l	d1,a1				; add picture size

	move.l	a1,db_blitinpic(a3)		; new pointer
	move.l	vb_breedte_x(a5),d0
	mulu	vb_planes(a5),d0
	add.l	d0,d2
	lsr.l	#2,d2				; copy longs ?
rep_b_cop:
	move.l	(a0)+,(a1)+
	subq.l	#1,d2
	bne	rep_b_cop

* Now copy the picture to the inactive view 
* the inactive viewdata is lost after this
*
	bsr	copy_info

	move.l	vb_breedte_x(a5),d1
	mulu	vb_planes(a5),d1
	mulu	vb_leny(a5),d1			; size of picture
	move.l	vb_tempbuffer(a4),a1
	move.l	vb_tempbuffer(a5),a0
	lsr.l	#2,d1				; longs ?
rep_b_cop2:
	move.l	(a0)+,(a1)+
	subq.l	#1,d1
	bne	rep_b_cop2

	move.l	vb_breedte_x(a5),d1		; clear last line ???????
	subq.l	#1,d1
.cl_rp1:
	clr.l	(a1)+
	dbf	d1,.cl_rp1
	move.l	db_inactive_viewblok(a3),a5
	bsr	maak_viewport			; create hidden view


	move.l	vb_tempbuffer(a5),d0
	tst.b	db_please_no_double(a3)
	beq	oke_doub	
	move.l	db_active_viewblok(a3),a5
	move.l	vb_tempbuffer(a5),d0
oke_doub:
	move.l	d0,db_waarsourcepic(a3)
	move.b	#$ff,db_double_buf(a3)

	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_viewblok(a3),a4
	move.l	vb_tempbuffer(a4),a0
	move.l	vb_breedte_x(a4),d0
	mulu	vb_planes(a4),d0
	mulu	vb_leny(a4),d0
	add.l	d0,a0
	move.l	db_inactive_fileblok(a3),a4
	bra	cont_mask1

*************

no_d_pos1:
	move.l	db_inactive_fileblok(a3),a4
	move.l	db_active_viewblok(a3),a5
	move.l	vb_tempbuffer(a4),a1		; the data is in chip
	move.l	vb_tempbuffer(a5),db_waarsourcepic(a3)

; check if buffering is uberhaupt possible

	add.l	d2,d2				; size blitin with mask
	cmp.l	d3,d2
	ble	buf_posible

nott:

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	dbstr5c2,a0
	jsr	KPutStr
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	db_blitinpic(a3),db_blitinmask(a3)		; same as blitin ?
	bra	mask_al_ready

	IFNE	DEBUG
dbstr5c2:	dc.b	"Buffering not pos",10,0
	even
	ENDC

buf_posible:

; als je de blitin in een keer wil blitten dient het mask dezelfde groote
; te hebben dan de blitin.
; bij een lijn copy kan er volstaan worden met een mask per lijn,
; immers het masker is voor ieder plane hetzelfde

	move.l	vb_breedte_x(a4),d0
	mulu	vb_planes(a5),d0
	mulu	vb_leny(a4),d0			; chipmemsize
	bsr	find_chipmem
	move.l	a0,db_blitinmask(a3)

cont_mask1:
	move.l	a0,db_blitinmask(a3)

	move.l	db_inactive_fileblok(a3),a4
	move.l	db_blitinmask(a3),a0
	addq.l	#4,db_blitinmask(a3)
	move.l	#0,(a0)+

	move.l	vb_breedte_x(a4),d0
	mulu	vb_planes(a5),d0
	move.l	d0,d2				; lijn size
	sub.l	vb_breedte_x(a4),d2
	mulu	vb_leny(a4),d0			; chipmemsize
	move.l	d0,db_blitinsize(a3)

	move.l	db_blitinpic(a3),a1

	tst.b	vb_fastmask(a4)
	bne	do_fastmask
*
* Get mask from fastmem ?????????????
*
	tst.l	db_fastmask(a3)
	beq	.nofm
	movem.l	d0-d7/a0-a6,-(a7)

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Using fastmask",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	db_fastmask(a3),a1	
	move.l	vb_breedte_x(a4),d4
	move.l	d4,d2
	mulu	vb_planes(a4),d2
	move.w	vb_leny(a4),d3
	subq.l	#1,d3

	move.l	d4,d5
	subq.l	#8,d5
	move.l	d5,d7
;	addq.l	#4,a0
	clr.l	(a0)+
	move.l	a0,a2
.rep_height:
	move.l	d7,d5
	lsr.w	#1,d5
	subq.w	#1,d5
	move.l	a2,a6
.rep_width:
	move.l	a2,a0
	move.w	vb_planes(a5),d6
	subq.w	#1,d6
.rep_m_b4:
	move.w	(a1),(a0)
	add.l	d4,a0
	dbf	d6,.rep_m_b4
	addq.l	#2,a1
	addq.l	#2,a2	
	dbf	d5,.rep_width
	move.l	a6,a2

	move.l	a2,a0

	move.w	vb_planes(a5),d6
	subq.w	#1,d6
.rep_m_b4_2:
	clr.l	-4(a0)
	add.l	d4,a0
	dbf	d6,.rep_m_b4_2
	add.l	d2,a2

	move.l	a2,a0
	move.w	vb_planes(a5),d6
	subq.w	#1,d6
.rep_m_b4_3:
	clr.l	-8(a0)
	add.l	d4,a0
	dbf	d6,.rep_m_b4_3
	dbf	d3,.rep_height

	movem.l	(a7)+,d0-d7/a0-a6
.nofm:	

*
* End get mask from fastmem ???????????????????
*

	move.l	a0,a2
	move.l	a1,a6
	move.w	vb_leny(a4),d7		; add one extra line ??????
	subq.w	#1,d7
rep_m_b3:
	move.l	vb_breedte_x(a4),d5
	move.l	d5,d4
	lsr.l	#1,d5			; words
	subq.w	#1,d5
rep_m_b2:
	move.l	a2,a0
	move.l	a6,a1
	move.w	vb_planes(a5),d6
	subq.w	#1,d6
	moveq	#0,d0
rep_m_b1:
	or.w	(a1),d0
	add.l	d4,a1
	dbf	d6,rep_m_b1

	tst.l	db_fastmask(a3)
	beq	.other

	move.w	vb_planes(a5),d6
	subq.w	#1,d6
.rep_m_b4:
	or.w	d0,(a0)
	add.l	d4,a0
	dbf	d6,.rep_m_b4
	addq.l	#2,a2
	addq.l	#2,a6
	dbf	d5,rep_m_b2
	bra	.normal
.other:
	move.w	vb_planes(a5),d6
	subq.w	#1,d6
.rep_m_b42:
	move.w	d0,(a0)
	add.l	d4,a0
	dbf	d6,.rep_m_b42
	addq.l	#2,a2
	addq.l	#2,a6
	dbf	d5,rep_m_b2
.normal:
	add.l	d2,a2			; volgende regel
	add.l	d2,a6
	dbf	d7,rep_m_b3

; DEBUG DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
; add one line for debug

	move.l	vb_breedte_x(a4),d5
	move.l	d5,d4
	lsr.l	#1,d5			; words
	subq.w	#1,d5

.rep_m_b222:
	move.l	a2,a0
	move.w	vb_planes(a5),d6
	subq.w	#1,d6
.rep_m_b422:
	move.w	#0,(a0)
	add.l	d4,a0
	dbf	d6,.rep_m_b422
	addq.l	#2,a2
	dbf	d5,.rep_m_b222

	move.b	#$ff,db_blitin_mask_ready(a3)
mask_al_ready:
	jsr	clear_fastmask		; free memory
	rts

*
* When a clip is on a non transparant window you can use
* a rectangle as a mask.
*
* When using a simple rectangle mask you can use the Mask A trick
* to blit using start and endmask and DAT line of $ffff ???????????
*
do_fastmask:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	dbstr4024,a0
	jsr	KPutStr
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.w	vb_lenx(a4),d1
	sub.w	#64,d1
	moveq	#0,d2
	move.w	#$ffff,d2
	swap	d2
	and.w	#$f,d1
	bne	.zero
	swap	d2
.zero:	
	lsr.l	d1,d2			; d2 the lastword mask
	
	addq.l	#4,a0			; first long is clearing area
	move.l	a0,a2			; points to dest mask

	move.w	#$ffff,d0
	move.l	vb_breedte_x(a4),d4

	move.w	vb_leny(a4),d7
	subq.w	#1,d7
rep_m_b3_fast:

	move.w	vb_planes(a5),d6
	subq.w	#1,d6
rep_m_b4_fast:

	move.l	vb_breedte_x(a4),d5
	subq.l	#8,d5
	lsr.l	#1,d5			; words
	subq.w	#1,d5
	move.l	a2,a0
	move.l	#0,-4(a0)
rep_m_b2_fast:
	move.w	d0,(a0)+
	dbf	d5,rep_m_b2_fast
	and.w	d2,-2(a0)
	move.l	#0,(a0)
	add.l	d4,a2
	dbf	d6,rep_m_b4_fast

	dbf	d7,rep_m_b3_fast
	move.b	#$ff,db_blitin_mask_ready(a3)
	bra	mask_al_ready

*
* delta_x var dient gezet te zijn, d6 bevat de effect_speed
* d7 bevat de lengte in bytes van de inactive blitin
*
do_adapt_blitin:
	move.l	db_inactive_fileblok(a3),a5
	move.l	vb_body_start(a5),db_tempbodystart(a3)
	move.l	vb_packed_size(a5),db_temppacked_size(a3)

	move.l	d7,db_tempbreedtelijn(a3)	; van de blitin
	
	bsr	copy_info_file_view	; copieer inactive file naar view data

;	bsr	clear_inactive_chipmem	; eigenlijk het gebruikte deel ????????

	move.l	db_inactive_viewblok(a3),a4
	move.l	db_active_viewblok(a3),a5
	move.l	vb_tempbuffer(a4),a1	; destination chipmembuffer

	move.l	vb_breedte_x(a4),d2	; houd dezelfde breedte
	add.l	db_delta_x(a3),d2

	mulu	vb_planes(a5),d2	; match de planes van active screen ???
	move.l	d2,d3			; d3 is de regel offset van destination
	move.l	d3,db_destlijnmodulo(a3)

	move.l	db_wpatroonruimte(a3),a0
	move.l	a0,db_templinebuffer1(a3)
	add.l	d7,a0
	move.l	a0,db_templinebufferend1(a3)	; een lijn verder stoppen
	move.l	db_wpatroonruimte(a3),a0
	lea.l	MAXLINESIZE(a0),a0
	move.l	a0,db_templinebuffer2(a3)
	add.l	d7,a0
	move.l	a0,db_templinebufferend2(a3)	; een lijn verder stoppen

	move.l	#0,(a1)
	mulu	d6,d3
	move.l	db_delta_x(a3),d6
	lsr.l	#1,d6
	add.l	d3,a1
	move.l	#0,(a1)
;	move.l	#0,-4(a1)
	add.l	d6,d3			; 32 pixels overslaan
	add.l	d6,a1
	move.l	a1,db_destchippointer(a3)
	move.w	vb_leny(a4),d7	; aantal lijnen
	bra	in_replb
rep_lijnenb:
	bsr	decrunchline	; haal een lijn op van picture ????
	bsr	copy_line_bl	; copieer lijn in chipmem destination
in_replb:
	dbf	d7,rep_lijnenb

; add four empty lines at the bottom

	move.l	db_destchippointer(a3),a1

	moveq	#0,d6
	moveq	#2,d1			; two empty lines
	bra	in_emp1
emp1:
	move.l	vb_breedte_x(a4),d0
	mulu	vb_planes(a4),d0
	lsr.l	#1,d0
	bra	in_emp2
emp2:
	move.w	d6,(a1)+
in_emp2:
	dbf	d0,emp2
	move.l	#0,(a1)+
	move.l	#0,(a1)+
in_emp1:
	dbf	d1,emp1

	rts

no_adapt_blitin:
	rts
*
* met extra vertikale snelheid 
*
adapt_blitin_exvert_moving:

	tst.b	db_blitin_adapted(a3)
	bne	no_adapt_blitin

	move.l	#8,db_delta_x(a3)	; maak blitin 16+16 pixels groter
	move.l	db_inactive_fileblok(a3),a4
	move.l	vb_breedte_x(a4),d7
	mulu	vb_planes(a4),d7
	move.l	db_effect_speed(a3),d6
	add.l	d6,d6			; ????????????

	moveq	#0,d6
	bsr	do_adapt_blitin

	move.l	db_inactive_viewblok(a3),a4
	addq.l	#8,vb_breedte_x(a4)

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	vb_lenx(a4),a1
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"X adapt is %d",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC


	add.w	#64,vb_lenx(a4)

	move.l	#32,db_blitin_dx(a3)		; ??????????????
	move.b	#$ff,db_blitin_adapted(a3)
	move.w	vb_leny(a4),db_blitin_fy(a3)
	move.w	vb_lenx(a4),db_blitin_fx(a3)
	move.l	db_blitin_x(a3),d0
	sub.l	db_blitin_dx(a3),d0
	move.l	d0,db_blitin_ox(a3)

	move.l	db_active_viewblok(a3),a5
	move.l	vb_breedte_x(a5),d1
	mulu	vb_planes(a5),d1
	move.l	d1,vb_totalwidth(a5)
	move.l	vb_breedte_x(a4),d1
	mulu	vb_planes(a4),d1
	move.l	d1,vb_totalwidth(a4)
	rts
*
* maak een conversie tabel voor hires naar lowres
*
create_low_to_high_tabel:
	lea	low_hi_tabel(pc),a0
	move.w	#256,d0
	moveq	#0,d1
rep_create2:
	moveq	#0,d2

	btst	#0,d1
	beq.b	no_seth0
	or.w	#%11,d2
no_seth0:
	btst	#1,d1
	beq.b	no_seth1
	or.w	#%1100,d2
no_seth1:
	btst	#2,d1
	beq.b	no_seth2
	or.w	#%110000,d2
no_seth2:
	btst	#3,d1
	beq.b	no_seth3
	or.w	#%11000000,d2
no_seth3:

	btst	#4,d1
	beq.b	no_seth4
	or.w	#%1100000000,d2
no_seth4:
	btst	#5,d1
	beq.b	no_seth5
	or.w	#%110000000000,d2
no_seth5:
	btst	#6,d1
	beq.b	no_seth6
	or.w	#%11000000000000,d2
no_seth6:
	btst	#7,d1
	beq.b	no_seth7
	or.w	#%1100000000000000,d2
no_seth7:
	move.w	d2,(a0)+
	addq	#1,d1
	dbf	d0,rep_create2
	rts
*
* maak een conversie tabel voor hires naar lowres
*
create_hi_to_low_tabel:
	lea	hi_low_tabel(pc),a0
	move.w	#256,d0
	moveq	#0,d1
rep_create1:
	moveq	#0,d2
	btst	#0,d1
	beq.b	no_set1
	bset	#0,d2
no_set1:
	btst	#2,d1
	beq.b	no_set2
	bset	#1,d2
no_set2:
	btst	#4,d1
	beq.b	no_set4
	bset	#2,d2
no_set4:
	btst	#6,d1
	beq.b	no_set6
	bset	#3,d2
no_set6:
	move.b	d2,(a0)
	lsl.b	#4,d2
	or.b	d2,(a0)+
	addq	#1,d1
	dbf	d0,rep_create1
	rts
*
* Adapt the bitmapped file in the third buffer
*
adapt_bitmapped:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Adapt bitmapped",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	vb_bitdata(a5),db_tempbodystart(a3)
	move.b	#0,vb_compression(a5)

	move.l	vb_breedte_x(a5),d0
	mulu	vb_planes(a5),d0
	move.l	d0,db_tempbreedtelijn(a3)

	move.w	vb_leny(a5),d0
	mulu	vb_planes(a5),d0
	move.l	vb_breedte_x(a5),d1
	mulu	d1,d0
	bsr	check_buf_size
	tst.w	d0
	bne	no_adapt_pos

	bsr	copy_info_file_view	; copieer inactive file naar view data
	bsr	clear_inactive_chipmem	; eigenlijk het gebruikte deel ????????

	move.l	db_inactive_viewblok(a3),a4
	move.l	db_active_viewblok(a3),a5
	move.l	vb_tempbuffer(a4),a1	; destination chipmembuffer

	tst.b	db_wipe_in_to_hires(a3)
	beq	bit_now_hi
	move.w	vb_lenx(a4),d0
	lsl.w	#1,d0
	move.w	d0,vb_lenx(a4)
	move.l	vb_breedte_x(a4),d0
	lsl.l	#1,d0
	move.l	d0,vb_breedte_x(a4)
bit_now_hi:
	tst.b	db_wipe_in_to_shires(a3)
	beq	bit_now_shi
	move.w	vb_lenx(a4),d0
	lsl.w	#1,d0
	move.w	d0,vb_lenx(a4)
	move.l	vb_breedte_x(a4),d0
	lsl.l	#1,d0
	move.l	d0,vb_breedte_x(a4)
bit_now_shi:

	tst.b	db_wipe_in_to_lowres(a3)
	beq	bit_now_lo
	move.w	vb_lenx(a4),d0
	lsr.w	#1,d0
	move.w	d0,vb_lenx(a4)

	move.l	vb_breedte_x(a4),d0
	lsr.l	#1,d0
	move.l	d0,vb_breedte_x(a4)
bit_now_lo:

	moveq	#0,d0			; initial offset
	moveq	#0,d1
	clr.l	db_delta_x(a3)
	tst.b	db_wipe_in_largerx(a3)
	beq	bit_no_x_offset
	move.l	vb_breedte_x(a5),d0	; active is nu groter dan wipe_in of evengroot
	sub.l	vb_breedte_x(a4),d0	; de delta x waarde tussen de screens

	move.l	d0,db_delta_x(a3)
	asr.l	#1,d0
	add.l	d0,a1		; tel xoffset op
bit_no_x_offset:

	tst.b	db_wipe_in_to_lace(a3)
	beq	bit_no_w_to_lace2

	move.w	vb_leny(a4),d0
	add.w	d0,d0
	move.w	d0,vb_leny(a4)
bit_no_w_to_lace2:
	tst.b	db_wipe_in_largery(a3)
	beq	bit_no_y_offset

	move.w	vb_leny(a5),d1	; active is eventueel bij adaptfile al op grote
	move.w	vb_leny(a4),d2

	tst.b	db_wipe_in_to_non_lace(a3)
	beq	.bit_now_lace
	lsr.w	#1,d2
.bit_now_lace:
	and.w	#$fffe,d2	; pad both down to even
	and.w	#$fffe,d1

	move.w	db_rows(a3),d0
	tst.b	vb_interlace(a5)
	beq.w	.no_inter1
	lsl.w	#1,d0
.no_inter1:
	sub.w	d0,d1
	neg.w	d1
	asr.w	#1,d1
	and.w	#$fffe,d1
	tst.w	d1
	bge	.okie2
	moveq	#0,d1
.okie2:

	move.w	db_rows(a3),d0
	tst.b	db_wipe_in_to_lace(a3)
	bne	.force_lace
	tst.b	vb_interlace(a4)
	beq.w	.no_inter2
.force_lace:
	lsl.w	#1,d0
.no_inter2:
	sub.w	d0,d2
	neg.w	d2
	asr.w	#1,d2
	and.w	#$fffe,d2
	tst.w	d2
	bge	.okie
	moveq	#0,d2
.okie:

	sub.w	d1,d2	; gebracht
	bpl	.okie22

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dat(pc),a1
	move.l	d2,(a1)
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"d2 is %ld",10,0
	even
.dat:	dc.l	0
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC
	moveq	#0,d2
.okie22:

	move.w	d2,d1
	add.w	d1,d1
	asr.w	#1,d1

;	move.l	d1,db_delta_y(a3)

bit_no_y_offset:

	move.l	vb_breedte_x(a5),d2
	mulu	vb_planes(a5),d2	; match de planes van active screen ???

	move.l	d2,d3			; d3 is de regel offset van destination
	move.l	d3,db_destlijnmodulo(a3)

	mulu	d1,d2

	add.l	d2,a1			; tel yoffset op
					; a1 wijst nu naar destination chipmem
	move.l	vb_breedte_x(a4),d0

	moveq	#0,d7
	move.w	vb_planes(a4),d7
	cmp.b	#1,vb_masking(a4)
	bne	bit_no_mask2
	addq.w	#1,d7
bit_no_mask2:
	mulu	d7,d0

	tst.b	db_wipe_in_to_lowres(a3)
	beq	bit_no_div
	add.l	d0,d0
bit_no_div:	
	tst.b	db_wipe_in_to_hires(a3)
	beq	bit_no_mul
	lsr.l	#1,d0
bit_no_mul:
	tst.b	db_wipe_in_to_shires(a3)
	beq	bit_no_mul2
	lsr.l	#1,d0
bit_no_mul2:
	move.l	db_wpatroonruimte(a3),a0
	move.l	a0,db_templinebuffer1(a3)
	add.l	d0,a0
	move.l	a0,db_templinebufferend1(a3)	; een lijn verder stoppen
	move.l	db_wpatroonruimte(a3),a0
	lea.l	MAXLINESIZE(a0),a0
	move.l	a0,db_templinebuffer2(a3)
	add.l	d0,a0
	move.l	a0,db_templinebufferend2(a3)	; een lijn verder stoppen

	move.l	a1,db_destchippointer(a3)
	move.w	vb_leny(a4),d7	; aantal lijnen

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	vb_leny(a4),a1
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"Ab leny is %d",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
pyt:
	movem.l	a0-a6/d0-d7,-(a7)
	lea	db_delta_x(a3),a1
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt

.dbstr1:	dc.b	"Ab delta x,y is %ld,%ld",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
pytry:
	movem.l	a0-a6/d0-d7,-(a7)
	lea	db_destchippointer(a3),a1
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt

.dbstr1:	dc.b	"start chipmem is %ld",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
pytew:
	movem.l	a0-a6/d0-d7,-(a7)
	lea	db_destlijnmodulo(a3),a1
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt

.dbstr1:	dc.b	"dest  line modulo is %ld",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7


	ENDC

bit_rep_lijnen:
	bsr	decrunchline	; haal een lijn op van picture ????

	tst.b	db_wipe_in_to_non_lace(a3)
	beq	bit_no_lijnnonlace

; skip elke oneven lijn
	btst	#0,d7
	bne	bit_skip_this_line

bit_no_lijnnonlace:
	bsr	convertline	; converteer van hires naar ... etc.
	bsr	copy_line	; copieer lijn in chipmem destination
	tst.b	db_wipe_in_to_lace(a3)
	beq	bit_skip_this_line
	bsr	copy_line
	subq.w	#1,d7
bit_skip_this_line:
	subq.w	#1,d7
	bne	bit_rep_lijnen

;	move.l	db_inactive_viewblok(a3),a5
;	bsr.w	free_memory_view	; geef view geheugen vrij
;	bsr	showinactive		; display het veranderde plaatje

	moveq	#0,d0
	rts
*
* Temporary don't adapt 24 bit files
*
dummy_24bit:
	move.l	#0,db_which_wipe(a3)	; set on bang
	rts

*
* adaptdecrunch past de inactive file aan aan het active scherm
* de mode is al eerder bepaald in adaptfile ( wipe_in_to ).
*
adaptdecrunch:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Adapt decrunch",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC
p:
	move.l	db_inactive_fileblok(a3),a5
	cmp.w	#24,vb_planes(a5)
	beq	dummy_24bit

	clr.l	db_delta_x(a3)
	move.l	db_inactive_fileblok(a3),a5
	tst.b	vb_bitmapped(a5)
	beq	no_adapt_bitmapped
	bra	adapt_bitmapped

no_adapt_bitmapped:
	move.l	vb_body_start(a5),db_tempbodystart(a3)

go_on_bitm1:
	move.l	vb_packed_size(a5),db_temppacked_size(a3)

	move.l	vb_breedte_x(a5),d0
	mulu	vb_planes(a5),d0
	move.l	d0,db_tempbreedtelijn(a3)

	move.w	vb_leny(a5),d0
	mulu	vb_planes(a5),d0
	move.l	vb_breedte_x(a5),d1
	mulu	d1,d0
	bsr	check_buf_size
	tst.w	d0
	bne	no_adapt_pos

	bsr	copy_info_file_view	; copieer inactive file naar view data

	bsr	clear_inactive_chipmem	; eigenlijk het gebruikte deel ????????

	move.l	db_inactive_viewblok(a3),a4
	move.l	db_active_viewblok(a3),a5
	move.l	vb_tempbuffer(a4),a1	; destination chipmembuffer

	tst.b	db_wipe_in_to_hires(a3)
	beq	now_hi
	move.w	vb_lenx(a4),d0
	lsl.w	#1,d0
	move.w	d0,vb_lenx(a4)
	move.l	vb_breedte_x(a4),d0
	lsl.l	#1,d0
	move.l	d0,vb_breedte_x(a4)
now_hi:
	tst.b	db_wipe_in_to_shires(a3)
	beq	now_shi
	move.w	vb_lenx(a4),d0
	lsl.w	#1,d0
	move.w	d0,vb_lenx(a4)
	move.l	vb_breedte_x(a4),d0
	lsl.l	#1,d0
	move.l	d0,vb_breedte_x(a4)
now_shi:

	tst.b	db_wipe_in_to_lowres(a3)
	beq	now_lo
	moveq	#0,d0
	move.w	vb_lenx(a4),d0
	lsr.w	#1,d0
	move.w	d0,vb_lenx(a4)

	move.l	vb_breedte_x(a4),d0
	lsr.l	#1,d0
	move.l	d0,vb_breedte_x(a4)
now_lo:

	moveq	#0,d0			; initial offset
	moveq	#0,d1

	tst.b	db_wipe_in_largerx(a3)
	beq	no_x_offset

	move.l	vb_breedte_x(a5),d0	; active is nu groter dan wipe_in of evengroot
	sub.l	vb_breedte_x(a4),d0	; de delta x waarde tussen de screens
	move.l	d0,db_delta_x(a3)
	asr.l	#1,d0

;	move.l	vb_breedte_x(a5),d1	; active is nu groter dan wipe_in of evengroot
;	lsl.l	#3,d1
;	cmp.w	db_max_x(a3),d1
;	ble	.oke1
;	move.w	db_max_x(a3),d0
;	lsr.l	#3,d0
;	sub.l	vb_breedte_x(a4),d0	; de delta x waarde tussen de screens
;	asr.l	#1,d0

	bge	.oke1
	moveq	#0,d0
.oke1:

	add.l	d0,a1		; tel xoffset op
	moveq	#0,d1
no_x_offset:

	tst.b	db_wipe_in_to_lace(a3)
	beq	no_w_to_lace2

	move.w	vb_leny(a4),d0
	add.w	d0,d0
	move.w	d0,vb_leny(a4)
no_w_to_lace2:
	tst.b	db_wipe_in_largery(a3)
	beq	no_y_offset

	move.w	vb_leny(a5),d1	; active is eventueel bij adaptfile al op grote
	move.w	vb_leny(a4),d2

	tst.b	db_wipe_in_to_non_lace(a3)
	beq	now_lace
	lsr.w	#1,d2
now_lace:

	tst.b	vb_interlace(a4)
	beq.w	.no_inter11
	and.w	#$fffe,d2	; pad both down to even
.no_inter11:

	tst.b	vb_interlace(a5)
	beq.w	.no_inter12
	and.w	#$fffe,d1
.no_inter12

	move.w	db_rows(a3),d0
	tst.b	vb_interlace(a5)
	beq.w	.no_inter1
	lsl.w	#1,d0
.no_inter1:
	sub.w	d0,d1
	neg.w	d1
	asr.w	#1,d1
	tst.b	vb_interlace(a5)
	beq.w	.no_inter2
	and.w	#$fffe,d1
.no_inter2:
	tst.w	d1
	bge	.okie2
	moveq	#0,d1
.okie2:

	move.w	db_rows(a3),d0
	tst.b	db_wipe_in_to_lace(a3)
	bne	.tla
	tst.b	vb_interlace(a4)
	beq.w	.no_inter22
.tla:
	lsl.w	#1,d0
.no_inter22:
	sub.w	d0,d2
	neg.w	d2
	asr.w	#1,d2
	tst.b	vb_interlace(a4)
	beq	.no_inter3
	and.w	#$fffe,d2
.no_inter3:	
	tst.w	d2
	bge	.okie
	moveq	#0,d2
.okie:

	sub.w	d1,d2	; gebracht

	move.w	d2,d1
	add.w	d1,d1

	move.l	d1,db_delta_y(a3)

	asr.w	#1,d1

no_y_offset:
	move.l	vb_breedte_x(a5),d2
	mulu	vb_planes(a5),d2	; match de planes van active screen ???

	move.l	d2,d3			; d3 is de regel offset van destination
	move.l	d3,db_destlijnmodulo(a3)

;	addq.l	#1,d1

	tst.b	vb_interlace(a4)
	beq	.no_inter4
	and.w	#$fffe,d1
.no_inter4:
	mulu	d1,d2
	add.l	d2,a1			; tel yoffset op
					; a1 wijst nu naar destination chipmem
	move.l	vb_breedte_x(a4),d0

	moveq	#0,d7
	move.w	vb_planes(a4),d7
	cmp.b	#1,vb_masking(a4)
	bne	no_mask2
	addq.w	#1,d7
no_mask2:
	mulu	d7,d0

	tst.b	db_wipe_in_to_lowres(a3)
	beq	no_div
	add.l	d0,d0
no_div:	
	tst.b	db_wipe_in_to_hires(a3)
	beq	no_mul
	lsr.l	#1,d0
no_mul:
	tst.b	db_wipe_in_to_shires(a3)
	beq	no_mul2
	lsr.l	#1,d0
no_mul2:
	move.l	db_wpatroonruimte(a3),a0
	move.l	a0,db_templinebuffer1(a3)
	add.l	d0,a0
	move.l	a0,db_templinebufferend1(a3)	; een lijn verder stoppen
	move.l	db_wpatroonruimte(a3),a0
	lea.l	MAXLINESIZE(a0),a0
	move.l	a0,db_templinebuffer2(a3)
	add.l	d0,a0
	move.l	a0,db_templinebufferend2(a3)	; een lijn verder stoppen

	move.l	a1,db_destchippointer(a3)
	move.w	vb_leny(a4),d7	; aantal lijnen

rep_lijnen:
	bsr	decrunchline	; haal een lijn op van picture ????

	tst.b	db_wipe_in_to_non_lace(a3)
	beq	no_lijnnonlace

; skip elke oneven lijn
	btst	#0,d7
	bne	skip_this_line

no_lijnnonlace:
	bsr	convertline	; converteer van hires naar ... etc.
	bsr	copy_line	; copieer lijn in chipmem destination
	tst.b	db_wipe_in_to_lace(a3)
	beq	skip_this_line
	bsr	copy_line
	subq.w	#1,d7
skip_this_line:
	subq.w	#1,d7
	bne	rep_lijnen
ttt1:
;	move.l	db_inactive_viewblok(a3),a5
;	bsr.w	free_memory_view	; geef view geheugen vrij
;	bsr	showinactive		; display het veranderde plaatje

	moveq	#0,d0
	rts


* convertline converteert een lijn naar hires of naar lowres
*
convertline:
	tst.b	db_wipe_in_to_shires(a3)
	bne	cto_shires

	tst.b	db_wipe_in_to_hires(a3)
	bne	cto_hires
	tst.b	db_wipe_in_to_lowres(a3)
	bne	cto_lowres
; doe niets
	rts

cto_shires:
	tst.b	db_wipe_in_to_hires(a3)
	beq	cto_hires			; from hires to shires

	lea	low_hi_tabel(pc),a2		; from lowres to shires ????
	move.l	db_templinebuffer1(a3),a0
	move.l	db_templinebuffer2(a3),a1
	move.l	db_tempbreedtelijn(a3),d0
lrepconv4sh:
	moveq	#0,d1
	move.b	(a0)+,d1
	add.w	d1,d1
	move.w	0(a2,d1),d1
	move.w	d1,d2
	and.w	#$ff,d2
	add.w	d2,d2
	move.w	0(a2,d2),(a1)+
	lsl.w	#8,d1
	add.w	d1,d1
	move.w	0(a2,d1),(a1)+

	subq.l	#1,d0
	bne.b	lrepconv4sh

	bra	draai_line_buffers


cto_hires:	; converteer naar hires

	lea	low_hi_tabel(pc),a2
	move.l	db_templinebuffer1(a3),a0
	move.l	db_templinebuffer2(a3),a1
	move.l	db_tempbreedtelijn(a3),d0
lrepconv4:
	moveq	#0,d1
	move.b	(a0)+,d1
	add.w	d1,d1
	move.w	0(a2,d1),(a1)+

	subq.l	#1,d0
	bne.b	lrepconv4

	bra	draai_line_buffers

cto_lowres:	; converteer naar lowres
	lea	hi_low_tabel(pc),a2
	move.l	db_templinebuffer1(a3),a0
	move.l	db_templinebuffer2(a3),a1
	move.l	db_tempbreedtelijn(a3),d0

lrepconv3:
	moveq	#0,d1
	move.b	(a0)+,d1
	move.b	0(a2,d1),d2
	and.b	#$f0,d2

	move.b	(a0)+,d1
	move.b	0(a2,d1),d3
	and.b	#$f,d3
	or.b	d3,d2
	move.b	d2,(a1)+

	subq.l	#2,d0
	bne.b	lrepconv3

draai_line_buffers:
	move.l	db_templinebuffer1(a3),d0
	move.l	db_templinebuffer2(a3),db_templinebuffer1(a3)
	move.l	d0,db_templinebuffer2(a3)

	move.l	db_templinebufferend1(a3),d0
	move.l	db_templinebufferend2(a3),db_templinebufferend1(a3)
	move.l	d0,db_templinebufferend2(a3)

	rts

* copy_line copieert een lijn uit de buffer eventueel met verschaling 
* naar de destination chipmem buffer ( inactive view buffer )
*
copy_line:
	move.l	db_destchippointer(a3),a1
	move.l	db_templinebuffer1(a3),a0
	move.w	vb_planes(a4),d1
	subq.w	#1,d1
rep_copplane:
	move.l	vb_breedte_x(a4),d0
	lsr.l	#1,d0
	subq.l	#1,d0
rep_copline:
	move.w	(a0)+,(a1)+
	dbf	d0,rep_copline	
	add.l	db_delta_x(a3),a1	; planemodulo source ( wipe_in )
	dbf	d1,rep_copplane
	move.l	db_destlijnmodulo(a3),d0
	add.l	d0,db_destchippointer(a3)
	rts	

copy_line_bl:
	move.l	db_destchippointer(a3),a1
	move.l	db_templinebuffer1(a3),a0
	move.l	#0,-4(a1)
	move.w	vb_planes(a4),d1
	bra	in_blrep_cpp
blrep_copplane:
	move.l	vb_breedte_x(a4),d0
	lsr.l	#1,d0
	bra	in_blrepcl
blrep_copline:
	move.w	(a0)+,(a1)+
in_blrepcl:
	dbf	d0,blrep_copline	
	move.l	#0,(a1)+
	move.l	#0,(a1)+
in_blrep_cpp:
	dbf	d1,blrep_copplane
	move.l	db_destlijnmodulo(a3),d0
	add.l	d0,db_destchippointer(a3)
	rts	
*
* adaptfile doet een poging om de file uit active zo veel mogelijk
* te laten lijken op de file uit buffer inactive 
* buffer inactive (a4) is hier dus de destination buffer
* Het is nu niet nodig om de a4 buffer de decrunchen. Alleen de benodigde data
* dient aanwezig te zijn ( breedte, hoogte enz.)
*
* Normaal is de destination buffer groter dan de source (a4) > (a5)
* Er wordt hiervan nu uitgegaan, de groote moet eigenlijk de grootste
* x en y hebben van destination en source ( KC-7-09-1992)
*
*    h 					   h	
*    e					   e
*    l					   l	
*    l		-->	world	==>	world
*    o					   o
*
* 10-09-1992 als je in de active view het aantal planes verandert copieer
* dan de niet gebruikte kleuren van de wipe-in mee
*
adaptfile:

;	move.l	db_inactive_viewblok(a3),a4
;	move.l	db_active_viewblok(a3),a5
;	move.w	vb_leny(a4),d0
;	move.w	vb_leny(a5),d0

;	move.l	vb_breedte_x(a5),d0
;	move.l	vb_breedte_x(a4),d0
;	move.w	vb_lenx(a4),d0

	bsr	copy_info_file_view	; copieer inactive file naar view data

	bsr	test_sizes
adaptfilelater:

	move.l	db_inactive_viewblok(a3),a4
	move.l	db_active_viewblok(a3),a5


	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	vb_leny(a4),a1
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"Adapt file inactive y,x %d,%d",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7

prtio:
	movem.l	a0-a6/d0-d7,-(a7)
	lea	vb_leny(a5),a1
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"Adapt file active y,x %d,%d",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

;	move.l	vb_breedte_x(a5),d0
;	move.l	vb_breedte_x(a4),d0
;	move.w	vb_lenx(a4),d0

**************

	tst.b	vb_shires(a5)		; source geen Shires ?
	beq	no_source_shires
	
	tst.b	vb_shires(a4)
	bne	yes_dest_shires		; destination ook Shires -> noproblemo

; er zijn nu twee verschillende resoluties wat te doen ?

	move.w	db_max_Depth_SHires(a3),d1
	cmp.w	vb_planes(a4),d1
	bge	 convert_wipe_in_to_shires

; wipe-in heeft groter aantal planes dus conversie naar shires is niet mogelijk
; dus converteer active screen naar hires

convert_down_SH_H:
	bsr	convert_shires_hires
	bra	adaptfile		; try again met view modes het zelfde
;	bra	adaptfilelater

no_source_shires:

	tst.b	vb_shires(a4)
	beq	no_dest_shires

; destination is hires convert source naar hires als dit kan

	move.w	db_max_Depth_SHires(a3),d1
	cmp.w	vb_planes(a5),d1
	blt	no_source_to_shires

	bsr	convert_hires_shires
	tst.w	d0
	bne	no_source_to_shires		; Convert wipe in afterall

	bra	adaptfile
;	bra	adaptfilelater

convert_wipe_in_to_shires:

; de wipe-in moet naar shires geconverteerd worden
; en moet daarna passen in het huidige scherm
; pas later de scherm afmeting aan, aan het later te maken hires scherm.

	move.l	vb_breedte_x(a4),d0
	asl.l	#1,d0
	mulu	vb_planes(a4),d0
	mulu	vb_leny(a4),d0

	bsr	check_buf_size
	tst.w	d0
	bne	convert_down_SH_H	; Can't create wipe-in conversion

	move.b	#$ff,vb_shires(a4)
	or.w	#V_SUPERHIRES,vb_mode(a4)	; zet hires mode
	move.w	vb_lenx(a4),d0
	asl.w	#1,d0
	move.w	d0,vb_lenx(a4)			; Width *= 2 

	move.l	vb_breedte_x(a4),d0
	asl.l	#1,d0
	move.l	d0,vb_breedte_x(a4)

	move.b	#$ff,db_wipe_in_to_shires(a3)
	move.b	#$ff,db_change(a3)

	bra	adaptfilelater		; scale with the shires values

no_source_to_shires:			; active has to many planes
	move.b	#$ff,db_wipe_in_to_lowres(a3)
	move.b	#$ff,db_change(a3)

	move.w	vb_lenx(a4),d0
	asr.w	#1,d0
	move.w	d0,vb_lenx(a4)		; de breedte wordt twee keer zo klein

	move.l	vb_breedte_x(a4),d0
	lsr.l	#1,d0
	move.l	d0,vb_breedte_x(a4)

	clr.b	vb_shires(a4)
	and.w	#$ffdf,vb_mode(a4)	; clear hires mode
	bra	cont_adaphires

**************
no_dest_shires:
yes_dest_shires:
cont_adaphires:
	tst.b	vb_hires(a5)	; source geen hires ?
	beq	no_source_hires
	
	tst.b	vb_hires(a4)
	bne	yes_dest_hires	; destination ook hires -> noproblemo

; er zijn nu twee verschillende resoluties wat te doen ?

	move.w	db_max_Depth_Hires(a3),d1
	cmp.w	vb_planes(a4),d1
	bge	 convert_wipe_in_to_hires

; wipe-in heeft groter aantal planes dus conversie naar hires is niet mogelijk
; dus converteer active screen naar lowres.

convert_down_HI_LO:

	bsr	convert_hires_lowres	

	bra	adaptfile		; try again met view modes het zelfde

no_source_hires:
	tst.b	vb_hires(a4)
	beq	no_dest_hires

; destination is hires convert source naar hires als dit kan

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	vb_leny(a4),a1
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"Source is no Hires dest is lowres",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.w	db_max_Depth_Hires(a3),d1
	cmp.w	vb_planes(a5),d1
	blt	no_source_to_hires

	bsr	convert_lowres_hires

	tst.w	d0
	bne	no_source_to_hires

	bra	adaptfile

convert_wipe_in_to_hires:

; de wipe-in moet naar hires geconverteerd worden
; en moet daarna passen in het huidige scherm
; pas later de scherm afmeting aan, aan het later te maken hires scherm.

	move.l	vb_breedte_x(a4),d0
	asl.l	#1,d0
	mulu	vb_planes(a4),d0
	mulu	vb_leny(a4),d0

	bsr	check_buf_size
	tst.w	d0
	bne	convert_down_HI_LO	; Can't create wipe-in conversion

	move.b	#$ff,vb_hires(a4)
	or.w	#$8000,vb_mode(a4)	; set hires mode
	move.w	vb_lenx(a4),d0
	asl.w	#1,d0
	move.w	d0,vb_lenx(a4)		; Width *= 2

	move.l	vb_breedte_x(a4),d0
	asl.l	#1,d0
	move.l	d0,vb_breedte_x(a4)

	move.b	#$ff,db_wipe_in_to_hires(a3)
	move.b	#$ff,db_change(a3)
	bra	adaptfilelater		; schaal nu met de hires waarden

no_source_to_hires:			; active heeft teveel planes
	move.b	#$ff,db_wipe_in_to_lowres(a3)
	move.b	#$ff,db_change(a3)

	move.w	vb_lenx(a4),d0
	asr.w	#1,d0
	move.w	d0,vb_lenx(a4)		; de breedte wordt twee keer zo klein

	move.l	vb_breedte_x(a4),d0
	lsr.l	#1,d0
	move.l	d0,vb_breedte_x(a4)

	clr.b	vb_hires(a4)
	and.w	#$7fff,vb_mode(a4)	; clear hires mode
	bra	cont_adap

; zorg er eerst voor dat alle grootste waarden in de destination worden
; gezet dus de lengte, breedte en planes

yes_dest_hires:
no_dest_hires:
cont_adap:

	tst.b	vb_interlace(a5)
	beq	source_no_lace
	tst.b	vb_interlace(a4)
	bne	both_lace

; active is laced en wipe-in niet 
; converteer wipe-in naar lace
	move.l	vb_breedte_x(a4),d0
	move.w	vb_leny(a4),d1
	asl.l	#1,d1
	mulu	d1,d0
	move.w	vb_planes(a4),d7
	move.l	d0,d1
	moveq	#0,d0
	subq.w	#1,d7

rr7:	add.l	d1,d0
	dbf	d7,rr7
	bsr	check_buf_size
	tst.w	d0
	beq	oke_conv_lace
	bsr	convert_lace_nolace
	bra	adaptfile

oke_conv_lace:
	move.b	#$ff,db_wipe_in_to_lace(a3)
	move.b	#$ff,db_change(a3)
	move.w	vb_leny(a4),d0
	add.w	d0,d0			; lengte maal twee
	move.w	d0,vb_leny(a4)
	move.b	#$ff,vb_interlace(a4)
	or.w	#$4,vb_mode(a4)		; set interlace bit

	bra	both_lace

source_no_lace:
	tst.b	vb_interlace(a4)
	beq	dest_no_lace

; wipe-in is interlaced en active view niet. Jammer
; converteer active naar lace en na het weipen naar terug naar non-lace


	bsr	convert_nolace_lace
	tst.w	d0
	bne	no_adapt_pos

	bra	adaptfile

both_lace:
dest_no_lace:
	move.w	vb_lenx(a5),d0
	cmp.w	vb_lenx(a4),d0
	ble	no_lenx_bigger

	move.l	vb_breedte_x(a5),vb_breedte_x(a4)

	move.w	d0,vb_lenx(a4)	; huidige plaatje is breder dan wipe-in
	move.b	#$ff,db_change(a3)	; wipe-in moet straks ook worden aangepast

	move.b	#$ff,db_wipe_in_largerx(a3)

; wipe-in straks breder

no_lenx_bigger:
	move.w	vb_leny(a5),d0
	cmp.w	vb_leny(a4),d0
	ble	no_leny_bigger
	move.w	d0,vb_leny(a4)	; huidige plaatje is langer dan wipe-in
	move.b	#$ff,db_change(a3)	; wipe-in moet straks ook worden aangepast
	move.b	#$ff,db_wipe_in_largery(a3)

; wipe-in straks langer

no_leny_bigger:

	moveq	#0,d0
	move.w	vb_lenx(a4),d0
	sub.w	vb_lenx(a5),d0		; de delta x waarde tussen de screens

	move.w	d0,db_delta_x(a3)

	asr.w	#1,d0
	move.w	d0,db_conv_xoff(a3)	; de X offset in het nieuwe screen

	move.l	d0,d3
	bsr	get_dy_offset
	move.w	d0,d2
	exg	a4,a5
	bsr	get_dy_offset
	exg	a4,a5
	sub.w	d0,d2
	move.w	d2,d1

	move.l	d3,d0
	
	add.w	d1,d1
	move.w	d1,db_delta_y(a3)

	asr.w	#1,d1

;	addq.l	#1,d1
	tst.b	vb_interlace(a5)
	beq	.noi1
	and.w	#$fffe,d1
.noi1:
	move.w	d1,db_conv_yoff(a3)	; de Y offset in het nieuwe screen

	move.w	vb_planes(a4),d2
	IFNE	BIT24
	cmp.w	#24,d2
	bne	.no24
	moveq	#8,d2
.no24:
	ENDC
	sub.w	vb_planes(a5),d2	; delta planes	????
	move.w	d2,db_delta_planes(a3)
	bpl	oke_plusplanes	

	move.b	#$ff,db_wipe_in_add_planes(a3)
	
	move.b	#$ff,db_change(a3)	; de destination heeft een kleiner
				; aantal planes.
				; Dit zou kleur verlies betekenen op het scherm
				; later zullen de planes verwijderd kunnen 
				; worden. Zet het change byte aan.
				; Het binnen komende plaatje moet aangepast
				; worden.




;	bra	no_adjust


; converteer hier dus alleen het mogelijke
; Maak een HAM plaatje hier dus alleen de goede groote
; en probeer met het inladen het in te weipen plaatje zo goed mogelijk
; de laten lijken ???????? ( dus gewoon zes planes en gaan ????)
; Als de wipe klaar is terug naar de orginele settings

oke_plusplanes:

;	move.w	db_conv_yoff(a3),d0	; de Y offset in het nieuwe screen

	tst.l	d0
	bne	goon

	tst.w	d1
	bgt	goon

	tst.w	d2
	ble	no_adjust

;	tst.b	db_wipe_in_largerx(a3)
;	bne	no_adjust
;	move.l	vb_breedte_x(a5),d0
;	cmp.l	vb_breedte_x(a4),d0
;	beq	no_adjust
goon:

	tst.w	db_delta_planes(a3)
	bpl	add_planes1

	move.w	vb_planes(a5),db_final_planes(a3)
	bra	old_planes

add_planes1:
	move.w	vb_planes(a4),db_final_planes(a3)

old_planes:			; het aantal planes blijft hetzelfde
				; de binnen komende picture moet aangepast
				; worden


	bsr	clear_inactive_chipmem	; eigenlijk het gebruikte deel ????????

	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_viewblok(a3),a4	; data gecopieerd

	move.w	db_final_planes(a3),d0
	move.w	d0,vb_planes(a4)	; zet destination planes

	move.l	vb_breedte_x(a4),d1	; a4 > a5 ??????????

;	lsl.l	#3,d1
;	cmp.w	db_max_x(a3),d1
;	ble	.oke1
;	move.w	db_max_x(a3),d1
;.oke1:
;	lsr.l	#3,d1

	sub.l	vb_breedte_x(a5),d1
	asr.l	#1,d1			; start offset
	bge	.oke2
	moveq	#0,d1
.oke2:

	move.l	vb_tempbuffer(a4),a1	; to buffer ( destination )
	move.l	vb_tempbuffer(a5),a0	; from buffer ( source )
;br3:		
	move.l	vb_breedte_x(a4),d2
	mulu	d0,d2			; aantal bytes per lijn		
	move.l	d2,d4			; destination lijn offset

	move.w	db_conv_yoff(a3),d3
	mulu	d3,d2			; yoffset in bytes

	movem.l	d1/d2,-(a7)

	moveq	#0,d0
	moveq	#0,d1

	move.w	vb_leny(a4),d0
	mulu	vb_planes(a4),d0
	move.l	vb_breedte_x(a4),d1
	mulu	d1,d0
	bsr	check_buf_size
	movem.l	(a7)+,d1/d2
	tst.w	d0
	bne	no_adapt_pos

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dat(pc),a1
	move.l	d1,(a1)
	move.l	d2,4(a1)
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"Doing an Adapt screen x offset, y offset %ld,%ld",10,0
	even
.dat:	dc.l	0,0
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	vb_tempbuffer(a4),a1	; to buffer ( destination )
	move.l	vb_tempbuffer(a5),a0	; from buffer ( source )

	add.l	d2,a1			; y offset
	add.l	d1,a1			; x offset
					; destination pointer a1 klaar
	move.l	a1,a6
	move.w	vb_leny(a5),d1
	tst.l	d1
	beq	o_yrep
	subq.w	#1,d1			; herhaal y keer 
o_yrep:

	move.w	vb_planes(a5),d0	; herhaal plane keer ( interleaved )
	tst.l	d0
	beq	o_pl
	subq.w	#1,d0			; -1 voor dbf
o_pl:
	move.l	a6,a1
o_planrep:

	move.l	vb_breedte_x(a5),d2	; herhaal breedte_x keer

	move.l	a1,a2			; save lijn pointer
	tst.l	d2
	beq	o_xrep
	subq.w	#1,d2
o_xrep:
	move.b	(a0)+,(a2)+
	dbf	d2,o_xrep

	add.l	vb_breedte_x(a4),a1	; volgende plane interleaved
	dbf	d0,o_planrep

	add.l	d4,a6

	dbf	d1,o_yrep
					; data gecopieerd
ttt5:
	move.w	vb_mode(a4),d1
	and.w	#V_EXTRA_HALFBRITE,d1
	beq	no_ehb2
	move.w	vb_mode(a4),d1
	or.w	#V_EXTRA_HALFBRITE,d1
	move.w	d1,vb_mode(a4)
	bra	ehbed
no_ehb2:
	move.w	vb_mode(a5),vb_mode(a4)		; use mode active screen
ehbed:
	move.l	vb_color_count(a5),vb_color_count(a4)

	move.l	db_inactive_fileblok(a3),a6	; eventueel extra kleuren
	bsr	copy_color

	exg	a4,a5

	bsr	showinactive		; display het veranderde plaatje

no_8voud:

no_adjust:
	moveq	#0,d0
	rts

no_adapt_pos:
	moveq	#-1,d0
	rts

 
test_sizes2:
	movem.l	d0/a4/a5,-(a7)
test_sizes3:
	move.l	db_active_viewblok(a3),a4
	move.l	vb_breedte_x(a4),d0
	
	move.l	db_inactive_viewblok(a3),a4

	tst.b	db_aa_present(a3)
	beq	no_v39_sizes

	cmp.w	#6,vb_planes(a5)
	bgt	.real_aa

; only need even word allignment

	move.l	vb_breedte_x(a5),d0
	and.w	#$3,d0
	beq	no_to_small2

.real_aa:	
	move.l	vb_breedte_x(a5),d0
	and.w	#$7,d0
	beq	no_to_small2
	move.l	vb_breedte_x(a5),d0
	addq.l	#7,d0
	lsr.l	#3,d0
	lsl.l	#3,d0	
	move.l	d0,vb_breedte_x(a5)
	lsl.l	#3,d0	
	move.w	d0,vb_lenx(a5)
	move.w	vb_planes(a4),vb_planes(a5)
	move.b	#$ff,db_wipe_in_largerx(a3)
no_v39_sizes:
no_to_small2:
	movem.l	(a7)+,d0/a4/a5
	rts
*
* Do some OS dependent size check here
*
test_sizes:
	movem.l	d0/a4/a5,-(a7)
	move.l	db_inactive_viewblok(a3),a5
	bra	test_sizes3

*
* copieer de kleuren van a5 naar a4
* Als de destination kleuren op zijn ga verder met het a6 kleuren blok
* 
copy_color:
	tst.b	db_aa_present(a3)
	bne	aa_pres1
	
	lea	vb_colors(a6),a2	; kleuren van wipe-in picture
	lea	vb_colors(a5),a0
	lea	vb_colors(a4),a1

	moveq	#32,d0			; altijd 32 kleuren
	move.l	vb_color_count(a5),d1
	divu	#3,d1			; ?????????????????????????
	subq.l	#1,d1
rep_col1:
	move.w	(a0)+,(a1)+
	addq.l	#2,a2
	subq.l	#1,d0
	dbf	d1,rep_col1	
rep_col2:				; rest van de kleuren
	move.w	(a2)+,(a1)+
	subq.w	#1,d0
	bpl	rep_col2
	rts

aa_pres1:
	lea	vb_colbytes(a6),a2	; kleuren van wipe-in picture
	lea	vb_colbytes(a5),a0
	lea	vb_colbytes(a4),a1

	move.l	#256,d0			; altijd 256 kleuren
	move.l	vb_color_count(a5),d1
	divu	#3,d1			; ?????????????????????????
	subq.l	#1,d1
rep_col3:
	move.b	(a0)+,(a1)+
	move.b	(a0)+,(a1)+
	move.b	(a0)+,(a1)+
	addq.l	#3,a2
	subq.l	#1,d0
	dbf	d1,rep_col3	

rep_col4:				; rest van de kleuren
	move.b	(a2)+,(a1)+
	move.b	(a2)+,(a1)+
	move.b	(a2)+,(a1)+
	subq.w	#1,d0
	bpl	rep_col4
	rts

*
* copieer de data en verklein deze in de vertikale richting
* van lace naar nolace
* 
convert_lace_nolace:

	bsr.w	clear_inactive_view		; clear destination buffer

	bsr	copy_info			; copy data naar destination

	move.l	db_inactive_viewblok(a3),a4	; destination
	move.l	db_active_viewblok(a3),a5		; source

	move.l	vb_tempbuffer(a5),a0
	move.l	vb_tempbuffer(a4),a1

	moveq	#0,d2
	move.l	vb_breedte_x(a5),d1
	mulu	vb_planes(a5),d1		; d1 nu de regel lengte
						; in bytes
	move.l	d1,d2
	lsr.w	#1,d1				; in words


	move.w	vb_leny(a5),d0
	lsr.w	#1,d0				; de helft wordt geskipt

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dat(pc),a2
	move.l	a0,(a2)
	move.l	a1,4(a2)
	move.l	d0,8(a2)
	move.l	d1,12(a2)
	lea	.dat(pc),a1
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"Convert lace to nolace %ld->%ld, %ld",10,0
	even
.dat:	dc.l	0,0,0,0
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

repconvl:
	move.l	d1,d3
repconvl2:
	move.w	(a0)+,(a1)+
	subq.w	#1,d3
	bne	repconvl2

	add.l	d2,a0				; sla een regel over

	subq.l	#1,d0
	bne.b	repconvl

	move.w	vb_leny(a5),d0	
	lsr.w	#1,d0
	move.w	d0,vb_leny(a4)
	clr.b	vb_interlace(a4)
	and.w	#$fffb,vb_mode(a4)

	bra	showinactive

*
* d0 geeft de gewenste size aan
*
check_buf_size:
	IFNE XAPP
	movem.l	a2/a5,-(a7)
	move.l	db_inactive_viewblok(a3),a5
	move.l	vb_mlscrpointer(a5),a2
	move.l	sbu_Size(a2),d1
	movem.l	(a7)+,a2/a5
	ELSE
	move.l	#MEMSIZE,d1
	ENDC
	cmp.l	d1,d0
	bgt	no_size
	moveq	#0,d0
	rts
no_size:

; kijk of er een mogelijkheid bestaat om twee buffers te koppelen

	moveq	#-1,d0
	rts
*
* copieer de data en vergroot deze in de vertikale richting
* van nolace naar lace
* 
convert_nolace_lace:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Convert nolace to lace",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	vb_unpacked_size(a5),d0
	add.l	d0,d0				; pic 2 * zo groot
	bsr	check_buf_size

	tst.w	d0
	bne	no_convert_nl

	bsr.w	clear_inactive_view		; clear destination buffer

	bsr	copy_info			; copy data naar destination

	move.l	db_inactive_viewblok(a3),a4	; destination
	move.l	db_active_viewblok(a3),a5		; source

	move.l	vb_tempbuffer(a5),a0
	move.l	vb_tempbuffer(a4),a1

	moveq	#0,d2
	move.l	vb_breedte_x(a5),d1
	mulu	vb_planes(a5),d1		; d1 nu de regel lengte
						; in bytes
	move.l	d1,d2
	lsr.w	#1,d1				; in words

	move.w	vb_leny(a5),d0
repconvnl:
	move.l	d1,d3
	move.l	a0,a2
repconvnl2:
	move.w	(a0)+,(a1)+
	subq.w	#1,d3
	bne	repconvnl2

	move.l	a2,a0
	move.l	d1,d3
repconvnl3:
	move.w	(a0)+,(a1)+
	subq.w	#1,d3
	bne	repconvnl3
	subq.l	#1,d0
	bne.b	repconvnl

	move.w	vb_leny(a5),d0	
	add.w	d0,d0
	move.w	d0,vb_leny(a4)
	move.b	#$ff,vb_interlace(a4)
	or.w	#$4,vb_mode(a4)
	bsr	showinactive
	moveq	#0,d0
	rts

no_convert_nl:
	moveq	#-1,d0
	rts
	
convert_nolace_lace_black:
	bsr.w	clear_inactive_view		; clear destination buffer
	bsr	copy_info			; copy data naar destination
	move.l	db_inactive_viewblok(a3),a4	; destination
	move.l	db_active_viewblok(a3),a5		; source

	move.l	vb_tempbuffer(a5),a0
	move.l	vb_tempbuffer(a4),a1

	moveq	#0,d2
	move.l	vb_breedte_x(a5),d1
	mulu	vb_planes(a5),d1		; d1 nu de regel lengte
						; in bytes
	move.l	d1,d2
	lsr.w	#1,d1				; in words

	move.w	vb_leny(a5),d0
repconvbl:
	move.l	d1,d3
	move.l	a0,a2
repconvbl2:
	move.w	(a0)+,(a1)+
	subq.w	#1,d3
	bne	repconvbl2
	move.l	d1,d3
repconvbl3:
	move.w	#0,(a1)+
	subq.w	#1,d3
	bne	repconvbl3
	subq.l	#1,d0
	bne.b	repconvbl

	move.w	vb_leny(a5),d0	
	add.w	d0,d0
	move.w	d0,vb_leny(a4)
	move.b	#$ff,vb_interlace(a4)
	or.w	#$4,vb_mode(a4)
	bra	showinactive
*
* raw copy van chipmem data
*
normal_copy:
	move.l	vb_tempbuffer(a5),a0
	move.l	vb_tempbuffer(a4),a1
	move.l	vb_unpacked_size(a5),d0
repconv1:
	move.b	(a0)+,(a1)+
	subq.l	#1,d0
	bne.b	repconv1
	rts
*
eor_copy:
	move.l	vb_tempbuffer(a5),a0
	move.l	vb_tempbuffer(a4),a1
	move.l	vb_unpacked_size(a5),d0
repconv2:
	move.b	(a0)+,d1
	eor.b	#$ff,d1
	move.b	d1,(a1)+
	subq.l	#1,d0
	bne.b	repconv2
	rts
*
* copieer de data en verklein deze in de horizontale richting
* van hires naar lowres
* De functie maakt gebruik van de hi_low_tabel tabel 
* 

convert_hi_low:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Convert hi low",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	bsr.w	clear_inactive_view		; clear destination buffer
	bsr	copy_info			; copy data naar destination
	lea	hi_low_tabel(pc),a2

	move.l	db_inactive_viewblok(a3),a4	; destination
	move.l	db_active_viewblok(a3),a5	; source

	move.l	vb_tempbuffer(a5),a0
	move.l	vb_tempbuffer(a4),a1

	move.l	vb_breedte_x(a5),d0	; stel breedte bij
	move.l	d0,d1

	lsr.l	#1,d0
	btst	#0,d0
	bne	hi_low_line_by_line

;	move.l	vb_unpacked_size(a5),d0

	mulu	vb_leny(a5),d1
	mulu	vb_planes(a5),d1
	move.l	d1,d0
repconv3:
	moveq	#0,d1
	move.b	(a0)+,d1
	move.b	0(a2,d1),d2
	and.b	#$f0,d2

	move.b	(a0)+,d1
	move.b	0(a2,d1),d3
	and.b	#$f,d3
	or.b	d3,d2
	move.b	d2,(a1)+
	subq.l	#2,d0
	bne.b	repconv3

	move.l	vb_breedte_x(a5),d0	; adjust width
	lsr.l	#1,d0
	move.l	d0,vb_breedte_x(a4)

	move.w	vb_lenx(a5),d0	
	lsr.w	#1,d0
	move.w	d0,vb_lenx(a4)
	rts
*
* It is not possible to divide the line by two so copy line by line
*
hi_low_line_by_line:
	moveq	#0,d7
	move.w	vb_lenx(a5),d7
	lsr.w	#1,d7
	add.w	#15,d7
	lsr.w	#4,d7
	add.w	d7,d7			; new width in bytes

	move.l	a1,a6
	move.w	vb_leny(a5),d4
rep_hl_y:
	move.w	vb_planes(a5),d5
rep_hl_planes:	

	move.l	vb_breedte_x(a5),d6
rep_hl_bytes:
	moveq	#0,d1
	move.b	(a0)+,d1
	move.b	0(a2,d1),d2
	and.b	#$f0,d2

	move.b	(a0)+,d1
	move.b	0(a2,d1),d3
	and.b	#$f,d3
	or.b	d3,d2
	move.b	d2,(a1)+

	subq.w	#2,d6
	bne	rep_hl_bytes
	move.b	#0,(a1)+
	move.b	#0,(a1)+
	
	add.l	d7,a6
	move.l	a6,a1
	subq.w	#1,d5
	bne	rep_hl_planes
	subq.w	#1,d4
	bne	rep_hl_y

	move.l	d7,vb_breedte_x(a4)
	
	move.w	vb_lenx(a5),d0	
	lsr.w	#1,d0
	move.w	d0,vb_lenx(a4)

	rts

hi_low_tabel:
	ds.b	258
low_hi_tabel:
	ds.w	258

*
* copieer de data en vergroot deze in de horizontale richting
* van lowres naar hires
* De functie maakt gebruik van de low_hi_tabel tabel 
* 
convert_lowres_hires:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Convert low hi",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

;	move.l	vb_unpacked_size(a5),d0

	move.l	vb_breedte_x(a5),d0
	mulu	vb_leny(a5),d0
	mulu	vb_planes(a5),d0

	add.l	d0,d0

	bsr	check_buf_size
	
	tst.w	d0
	bne	no_convert_hl

	bsr	adjust_width

	move.b	#$ff,vb_hires(a4)
	or.w	#$8000,vb_mode(a4)
	bsr	showinactive
	moveq	#0,d0
	rts

convert_hires_shires:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Convert hires Shires",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	vb_unpacked_size(a5),d0
	add.l	d0,d0

	bsr	check_buf_size
	
	tst.w	d0
	bne	no_convert_hl

	bsr	adjust_width

	move.b	#$ff,vb_shires(a4)
	or.w	#V_SUPERHIRES,vb_mode(a4)
	bsr	showinactive
	moveq	#0,d0
	rts

no_convert_hl:
	moveq	#-1,d0
	rts

adjust_width:
	bsr.w	clear_inactive_view		; clear destination buffer
	bsr	copy_info			; copy data naar destination
	lea	low_hi_tabel(pc),a2
	move.l	db_inactive_viewblok(a3),a4
	move.l	db_active_viewblok(a3),a5
	
	move.l	vb_tempbuffer(a5),a0
	move.l	vb_tempbuffer(a4),a1
	move.l	vb_unpacked_size(a5),d0
repconv4:
	moveq	#0,d1
	move.b	(a0)+,d1
	add.w	d1,d1
	move.w	0(a2,d1),(a1)+

	subq.l	#1,d0
	bne.b	repconv4

	move.l	vb_breedte_x(a4),d0	; stel breedte bij
	lsl.l	#1,d0
	move.l	d0,vb_breedte_x(a4)

	move.w	vb_lenx(a4),d0	
	lsl.w	#1,d0
	move.w	d0,vb_lenx(a4)
	rts


*
* copy_info copieert de iff data van active naar inactive
*
copy_info:
	move.l	db_inactive_viewblok(a3),a4
	move.l	db_active_viewblok(a3),a5
	
	lea	vb_bmhd_found(a5),a2
	move.l	a5,a0
	move.l	a4,a1
rep_copy_info:
	move.l	(a0)+,(a1)+
	cmp.l	a0,a2
	bge	rep_copy_info
	rts

*
* copy_info copieert de iff data van inactive file naar inactive view
*
copy_info_file_view:
	move.l	db_inactive_viewblok(a3),a4
	move.l	db_inactive_fileblok(a3),a5
	
	lea	vb_bmhd_found(a5),a2
	move.l	a5,a0
	move.l	a4,a1
rep_file_view_info:
	move.l	(a0)+,(a1)+
	cmp.l	a0,a2
	bge	rep_file_view_info
	rts

*
* clear_inactive_view geeft alleen de inactive view vrij
*
clear_inactive_view:
	move.l	db_inactive_viewblok(a3),a5
	bsr	free_memory_view		; geef view geheugen vrij
	rts
*
* geef eventueel het geheugen in de inactive buffer
* vrij en geef eventueel de inactive view vrij
*
clear_inactive_buffers:
	move.l	db_inactive_fileblok(a3),a5
	tst.l	vb_packed(a5)			; geef file geheugen vrij
	beq.s	clear_fr1
	move.l	vb_packed(a5),a1
	move.l	vb_packed_size(a5),d0
	move.l	db_mlmmubase(a3),a6
	jsr	_LVOMLMMU_FreeMem(a6)
	clr.l	vb_packed(a5)
clear_fr1:
	tst.l	vb_filehandle(a5)
	beq.s	exit_clear
	bsr.w	close_file
exit_clear:
	rts

* laat active view nog een keer zien
* debug only ??
*

wacht_change:
	btst		#6,$bfe001
	beq.w		cleanexit
	move.l		db_graphbase(a3),a6
	move.l		vb_vieww(a5),a0
	move.l		a0,d0
	cmp.l		gb_ActiView(a6),d0	; is de view weg ?
	beq.b		wacht_change
	rts

cleanexit:
	tst.l		db_oldview(a3)		; is er een oldview
	beq.b		no_oldview
	move.l		db_oldview(a3),a1
	move.l		db_graphbase(a3),a6
	jsr		_LVOLoadView(a6)	; restore deze
	jsr		_LVOWaitTOF(a6)
no_oldview:
	rts

free_memory_view:
	move.l		db_graphbase(a3),a6
	move.l		vb_viewportw(a5),a0
	jsr		_LVOGfxLookUp(a6)
	move.l		d0,vb_vpextra(a5)
	move.l		vb_vieww(a5),a0
	jsr		_LVOGfxLookUp(a6)
	move.l		d0,vb_vextra(a5)

	move.l		vb_viewportw(a5),a0
	tst.l		vp_ColorMap(a0)
	beq.w		no_colormem

	move.l		vp_ColorMap(a0),a0
	jsr		_LVOFreeColorMap(a6)
	move.l		vb_viewportw(a5),a0
	clr.l		vp_ColorMap(a0)
no_colormem:

	move.l		vb_viewportw(a5),a0
	jsr		_LVOFreeVPortCopLists(a6)

	move.l		vb_vieww(a5),a0
	tst.l		v_LOFCprList(a0)
	beq.b		no_lofmem
	move.l		v_LOFCprList(a0),a0
	jsr		_LVOFreeCprList(a6)
	move.l		vb_vieww(a5),a0
	clr.l		v_LOFCprList(a0)
no_lofmem:

	move.l		vb_vieww(a5),a0
	tst.l		v_SHFCprList(a0)
	beq.b		no_shfmem
	move.l		v_SHFCprList(a0),a0
	jsr		_LVOFreeCprList(a6)
	move.l		vb_vieww(a5),a0
	clr.l		v_SHFCprList(a0)
no_shfmem:

no_colormap:
	tst.l	vb_vpextra(a5)
	beq	no_vp
	move.l	vb_vpextra(a5),a0

	IFNE GFXSNOOP
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.temp2(pc),a1
	move.l	a0,(a1)
	lea	.dbstr2(pc),a0
	jsr	KPutFmt
	bra	.tt2
.dbstr2:	dc.b	"free1 %d,%d",10,0
	even
.temp2:	dc.l	0
.tt2:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	db_graphbase(a3),a6
	jsr	_LVOGfxFree(a6)
	clr.l	vb_vpextra(a5)
no_vp:
	tst.l	vb_vextra(a5)
	beq	no_v
	move.l	vb_vextra(a5),a0

	IFNE GFXSNOOP
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.temp2(pc),a1
	move.l	a0,(a1)
	lea	.dbstr2(pc),a0
	jsr	KPutFmt
	bra	.tt2
.dbstr2:	dc.b	"free2 %d,%d",10,0
	even
.temp2:	dc.l	0
.tt2:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC
	move.l	db_graphbase(a3),a6
	jsr	_LVOGfxFree(a6)
	clr.l	vb_vextra(a5)
no_v:
	moveq		#0,d0
	rts

unpack:	
	movem.l	d0/d7/a2,-(a7)
	move.l	vb_unpacked_size(a5),d0

	IFNE XAPP
	move.l	vb_mlscrpointer(a5),a2
	move.l	sbu_Size(a2),d7
	ELSE
	move.l	#MEMSIZE,d7
	ENDC
	cmp.l	d0,d7
	movem.l	(a7)+,d0/d7/a2
	blt	no_unpack

	move.l	vb_body_start(a5),a0
	move.l	vb_tempbuffer(a5),a1

	cmp.b	#1,vb_compression(a5)
	beq.w	un1

* compression geen 1 dus of 0 of een onbekende 
* copieer van body_start naar unpacked

	move.l	vb_unpacked_size(a5),d0
	lsr.l	#2,d0
	move.l	vb_body_start(a5),a0
no_co:	move.l	(a0)+,(a1)+
	subq.l	#1,d0
	bpl	no_co
	moveq	#0,d0
	rts
un1:
	move.l	vb_tempbuffer(a5),a6
	add.l	vb_unpacked_size(a5),a6
un_again:	
	cmp.l	a6,a1
	bhi.s	un_end

	moveq	#0,d5
	move.b	(a0)+,d5
	bmi.b	un_minus

un_plu:	move.b	(a0)+,(a1)+
	dbf	d5,un_plu
	bra.s	un_again

un_minus:
	neg.b	d5
	move.b	(a0)+,d0

un_rm:	move.b	d0,(a1)+
	dbf	d5,un_rm

	bra.s	un_again
un_end:	
	moveq	#0,d0
	rts

no_unpack:
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	vb_unpacked_size(a5),a1
	lea	dbstr4123(pc),a0
	jsr	KPutFmt
	movem.l	(a7)+,a0-a6/d0-d7

	movem.l	a0-a6/d0-d7,-(a7)
	move.l	vb_mlscrpointer(a5),a2
	lea.l	sbu_Size(a2),a1
	lea	dbstr4123b(pc),a0
	jsr	KPutFmt
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC
	moveq	#-1,d0
	rts
	
* decrunch een regel uit de temp vars
* als alles goed is is de source lijn gecrunched dus een size check
* hoeft alleen bij een parameter fetch plaats te vinden. ????????????
*
decrunchline:	
	move.l	db_tempbodystart(a3),a0
	move.l	db_templinebuffer1(a3),a1
	move.l	db_templinebufferend1(a3),a2	; end criterium

	cmp.b	#1,vb_compression(a4)	; ?????????????
	beq.w	dun1


* compression geen 1 dus of 0 of een onbekende 
* copieer van body_start naar unpacked

	move.l	db_tempbreedtelijn(a3),d0

	lsr.l	#1,d0
	subq.l	#1,d0

dno_co:	move.w	(a0)+,(a1)+
	dbf	d0,dno_co
	move.l	a0,db_tempbodystart(a3)
	rts
dun1:
	move.l	db_templinebufferend1(a3),a6
dun_again:	
	cmp.l	a6,a1
	bge.s	dun_end

	moveq	#0,d5
	move.b	(a0)+,d5
	bmi.b	dun_minus

dun_plu:	move.b	(a0)+,(a1)+
	dbf	d5,dun_plu
	bra.s	dun_again

dun_minus:
	neg.b	d5
	move.b	(a0)+,d0

dun_rm:	move.b	d0,(a1)+
	dbf	d5,dun_rm

	bra.s	dun_again
dun_end:	
	move.l	a0,db_tempbodystart(a3)	; voor volgende lijn
	rts
*
*
*
check_chunks:
	movem.l	d0-d7/a0-a6,-(a7)
	move.b	#0,db_crng_found(a3)
	move.b	#0,db_drng_found(a3)
	move.l	vb_packed(a5),a1
	move.l	4(a1),d0		; file size
	addq.l	#8,a1
	move.l	a1,a2
	addq.l	#4,a1			; skip het ILBM sign
	add.l	d0,a2			; einde data
cchunks1:
	cmp.l	#"BMHD",(a1)
	bne.w	no_bmhd_proc
	move.b	#$ff,vb_bmhd_found(a5)
	clr.b	vb_hires(a5)
	clr.b	vb_interlace(a5)
	clr.b	vb_shires(a5)

	lea	8(a1),a6
	move.w	(a6),d1

;	btst	#0,d1
;	beq	no_funny_add
;	addq.w	#1,d1
;no_funny_add:
	move.w	d1,vb_lenx(a5)
	move.w	2(a6),vb_leny(a5)
	moveq	#0,d1
	move.w	vb_leny(a5),d1
	cmp.w	db_max_y(a3),d1
	ble	.ysize_oke
	move.w	db_max_y(a3),vb_leny(a5)
.ysize_oke:

	moveq	#0,d1
	move.b	8(a6),d1
	move.w	d1,vb_planes(a5)
	move.b	10(a6),vb_compression(a5)
	move.b	9(a6),vb_masking(a5)
	moveq	#0,d1
	moveq	#0,d2
	move.w	vb_lenx(a5),d1

	IFNE	BIT24
	cmp.w	#24,vb_planes(a5)
	bne	.no24

	cmp.w	#370,d1
	ble.s	.geen_hires
	move.b	#$FF,vb_hires(a5)
	move.w	#$8000,d2
.geen_hires:
	move.w	vb_leny(a5),d1
	cmp.w	#390,d1
	ble.s	.geen_lace
	move.b	#$ff,vb_interlace(a5)
	or.w	#$4,d2
.geen_lace:
	move.w	d2,vb_mode(a5)
	bra	.y24
.no24:
	ENDC
	move.w	#0,vb_mode(a5)
.y24:
	move.l	a1,-(a7)
	bsr.w	unpack_init
	move.l	(a7)+,a1

	bra.w	continue_chunks

no_bmhd_proc:
	cmp.l	#"CAMG",(a1)
	bne.w	no_camg_proc	

	move.w	vb_mode(a5),d2
	move.l	8(a1),d0
	and.l	#VGAPRODUCT_KEY,d0
	cmp.l	#VGAPRODUCT_KEY,d0
	bne	.no_vga
	move.w	10(a1),d0
	or.w	d0,d2			; rechtstreeks in het de mode-id
	and.w	#$8efd,d2		; oldstyle mask
	move.w	d0,d1
	bra	cno_shires
	
.no_vga:
	move.w	10(a1),d0
	or.w	d0,d2			; rechtstreeks in het de mode-id
	and.w	#$8efd,d2		; oldstyle mask
	move.w	d0,d1
	and.w	#V_SUPERHIRES,d0
	beq	cno_shires
	move.b	#$ff,vb_shires(a5)
	or.w	#V_SUPERHIRES,d2
cno_shires:
	move.w	d1,d0
	and.w	#$8000,d0
	beq.b	cno_hires
	move.b	#$ff,vb_hires(a5)
	or.w	#$8000,d2
cno_hires:
	move.w	d1,d0
	and.w	#$4,d0
	beq.b	cno_lace
	move.b	#$ff,vb_interlace(a5)
	or.w	#$4,d2
cno_lace:
	move.w	d1,d0
	and.w	#$800,d0
	beq.b	cno_ham
	or.w	#$800,d2
cno_ham:
	move.w	d1,d0
	and.w	#$80,d0	
	beq.b	cno_half
	and.w	#$f7ff,d2	; clear het ham bit
cno_half:
	move.w	d2,vb_mode(a5)

no_camg_proc:
	cmp.l	#"CMAP",(a1)
	bne.b	no_cmap_proc

	lea	4(a1),a6
	move.l	(a6),vb_cmap_size(a5)
	move.l	(a6)+,d1

	move.l	d1,vb_color_count(a5)

* in d1 het aantal kleuren * 3

	lea.l	vb_colors(a5),a4

	move.l	a0,-(a7)
	lea	vb_colbytes(a5),a0		; store alle kleuren bytes
cmap4:
	moveq	#0,d3
	moveq	#0,d2
	move.b	(a6)+,d3
	move.b	d3,(a0)+
	lsl.w	#4,d3

	move.b	(a6)+,d3
	move.b	d3,(a0)+

	move.b	(a6)+,d2
	move.b	d2,(a0)+
	lsr.b	#4,d2
	and.b	#$f0,d3
	or.b	d2,d3
	move.w	d3,(a4)+
	subq.w	#3,d1
	bne.b	cmap4

	move.l	(a7)+,a0
	bra.w	continue_chunks

no_cmap_proc:

	cmp.l	#"BODY",(a1)
	bne.b	no_body_proc
	move.b	#$ff,vb_body_found(a5)
	lea	8(a1),a6
	move.l	a6,vb_body_start(a5)
	bra.w	continue_chunks

no_body_proc:

	cmp.l	#"CRNG",(a1)
	bne.b	no_crng_chunk

	lea	8(a1),a6
	move.b	#1,db_crng_found(a3)
	move.l	a6,db_colcyc_point(a3)

	move.w	2(a6),d0	; rate
	move.w	d0,db_colrate(a3)
	move.w	4(a6),d0	; flags
	move.w	d0,db_colflags(a3)
	moveq	#0,d0
	move.b	6(a6),d0	; first color
	move.w	d0,db_collow(a3)
	move.b	7(a6),d0	; last color
	move.w	d0,db_colhigh(a3)
	bra.w	continue_chunks

no_crng_chunk:
	cmp.l	#"DRNG",(a1)
	bne.b	no_drng_chunk

	lea	8(a1),a6
	move.b	#1,db_drng_found(a3)
	move.l	a6,db_colcyc_point(a3)
	move.w	2(a6),d0	; rate
	move.w	d0,db_colrate(a3)	
	move.w	4(a6),d0	; flags
	move.w	d0,db_colflags(a3)
	bra.w	continue_chunks
no_drng_chunk:
continue_chunks:
	add.l	4(a1),a1
	move.l	a1,d0
	btst	#0,d0
	beq.b	no_oneven2
	addq.l	#1,a1
no_oneven2:
	addq.l	#8,a1
	cmp.l	a2,a1
	blt.w	cchunks1
	move.w	#-1,a1
cchunks2:
;	or.w	#$4,vb_mode(a5)

	tst.b	vb_interlace(a5)
	bne	.nolace
	move.w	db_max_y(a3),d0
	lsr.w	#1,d0
	cmp.w	vb_leny(a5),d0
	bge	.nolace
	move.w	d0,vb_leny(a5)
.nolace:
	move.w	vb_mode(a5),db_tmode(a3)
	movem.l	(a7)+,d0-d7/a0-a6
	rts

unpack_init:
	moveq	#0,d0
	moveq	#0,d1
	move.w	vb_lenx(a5),d0
	move.w	d0,d1
	add.l	#15,d1
	lsr.l	#4,d1			; aantal words
	add.l	d1,d1			; aantal bytes
	move.l	d1,vb_breedte_x(a5)
	move.l	d1,d0

;	lsl.w	#3,d1
;	move.w	d1,vb_lenx(a5)		; zet len_x op word lengte

	move.w	vb_leny(a5),d1
	mulu	d1,d0

	move.l	d0,d2
	moveq	#0,d0
	moveq	#0,d1	
	move.w	vb_planes(a5),d1

	IFNE	BIT24
	cmp.w	#24,d1
	bne	.no24
	move.w	#8,d1			; convert to 8 planes
.no24:
	ENDC
	cmp.b	#1,vb_masking(a5)
	bne.b	no_masking
	addq.w	#1,d1
no_masking:
	subq.w	#1,d1
mul_size:
	add.l	d2,d0
	dbf	d1,mul_size	
	moveq	#2,d7	
	move.l	d0,vb_unpacked_size(a5)

	IFNE XAPP
	movem.l	a2,-(a7)
	move.l	db_mlsysstruct(a3),a2
	move.l	sbu_Size(a2),d1
	movem.l	(a7)+,a2
	ELSE
	move.l	#MEMSIZE,d1
	ENDC

	cmp.l	d1,d0
	blt	.size_oke

	move.l	vb_unpacked_size(a5),d0
	sub.l	d1,d0
	move.l	vb_breedte_x(a5),d2
	mulu	vb_planes(a5),d2	; total width 1 line
	divu	d2,d0			; nr of lines to many
	tst.w	d0
	bmi	.size_oke
	addq.w	#1,d0
	sub.w	d0,vb_leny(a5)

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	vb_leny(a5),a1
	lea	.dbstr2(pc),a0
	jsr	KPutFmt
	bra	.tt2
.dbstr2:	dc.b	"New Y is %d",10,0
	even
.tt2:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC
	bra	unpack_init	

.size_oke:
	IFNE	BIT24
	cmp.w	#24,vb_planes(a5)
	bne	.no24
	bsr	set_cols_6planes	; set colors for this 24bit pic
	move.w	#24,vb_planes(a5)	; don't alter this !
.no24:
	ENDC

	rts

	IFNE DEBUG
dbstr4122:	dc.b	"Load first frame anim",10,0
dbstr4123:	dc.b	"skipped unpack %d,%d",10,0
dbstr4123b:	dc.b	"Screen size is  %d,%d",10,0
	even
	ENDC

load_first_frame_anim:

	move.l	vb_filenaam(a5),a1
.add_f:
	tst.b	(a1)+
	bne	.add_f				; add frame num to filename
	subq.l	#1,a1
	move.b	#'.',(a1)+
	move.b	#'f',(a1)+
	move.b	#'1',(a1)+
	move.b	#0,(a1)+

.notready:
	move.l	db_mlmmubase(a3),a6
	move.l	vb_filenaam(a5),a1		; naam van de file mlmmulib
	jsr	_LVOMLMMU_FindMemBlk(a6)
	move.l	d0,vb_packed(a5)
	beq	.go_on_load

	move.l	db_mlmmubase(a3),a6
	move.l	vb_packed(a5),a1
	jsr	_LVOMLMMU_GetMemStat(a6)	; haal status van memblock op

	and.l	#MTF_INIT,d0			; wait until inited
	beq	.notready
	bra	file_loaded

.go_on_load:

; file is already open with the first 12 bytes read
; FORM....ANIM is loaded there should be a FORM....ILBM after it

	move.l	vb_filehandle(a5),d1
	lea.l	vb_breedte_x(a5),a2
	move.l	a2,d2
	moveq	#8,d3
	move.l	db_dosbase(a3),a6
	jsr	_LVORead(a6)

	lea.l	vb_breedte_x(a5),a1
	moveq	#6,d7
	cmp.l	#'FORM',(a1)+
	bne.w	exit
	move.l	(a1),d0
	addq.l	#8,d0
	move.l	d0,vb_packed_size(a5)
	moveq	#ERR_NO_FMEM,d7

	move.l	$4.w,a6
	jsr	_LVOForbid(a6)

	moveq	#0,d1
	tst.b	db_stay_on(a3)
	beq	.nostay
	or.l	#MEMF_STAY,d1
.nostay:

	move.l	db_mlmmubase(a3),a6
	move.l	vb_filenaam(a5),a1		; naam van de file mpmmulib
	jsr	_LVOMLMMU_AllocMem(a6)
	move.l	d0,vb_packed(a5)
	tst.l	d0
	bne.w	.okie_load

	move.l	$4.w,a6
	jsr	_LVOPermit(a6)

	bra	load_err2

.okie_load:

	move.l	d0,a0
	moveq	#12,d2
	bra	act_load

read_whole_file:
.notready:
	move.l	db_mlmmubase(a3),a6
	move.l	vb_filenaam(a5),a1		; naam van de file mlmmulib
	jsr	_LVOMLMMU_FindMemBlk(a6)
	move.l	d0,vb_packed(a5)
	beq	.no_loadmpmmu
	move.l	d0,d6

	IFNE VIEWINFO
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr2(pc),a0
	lea	.dat2(pc),a1
	move.l	d0,(a1)
	jsr	KPutFmt
	bra	.tt2
.dbstr2:	dc.b	"REload at %lx",10,0
	even
.dat2:	dc.l	0
.tt2:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	db_mlmmubase(a3),a6
	move.l	vb_packed(a5),a1
	jsr	_LVOMLMMU_GetMemStat(a6)	; haal status van memblock op

	and.l	#MTF_INIT,d0			; wait until inited
	beq	.notready

	move.l	d6,a0
	move.l	4(a0),d0
	addq.l	#8,d0

	moveq	#0,d1
	tst.b	db_stay_on(a3)
	beq	.nostay
	or.l	#MEMF_STAY,d1
.nostay:

	move.l	db_mlmmubase(a3),a6
	move.l	vb_filenaam(a5),a1		; naam van de file mpmmulib
	jsr	_LVOMLMMU_AllocMem(a6)
	move.l	d0,vb_packed(a5)
	beq	load_err2

	bra	file_loaded2

.no_loadmpmmu:
	moveq	#3,d7
	move.l	vb_filenaam(a5),d1
	move.l	#mode_old,d2
	move.l	db_dosbase(a3),a6
	jsr	_LVOOpen(a6)
	move.l	d0,vb_filehandle(a5)
	beq.w	load_err1

	move.l	d0,d1
	lea.l	vb_breedte_x(a5),a2
	move.l	a2,d2
	moveq	#12,d3
	move.l	db_dosbase(a3),a6
	jsr	_LVORead(a6)
	lea.l	vb_breedte_x(a5),a1
	moveq	#6,d7

	cmp.l	#'FORM',(a1)+
	bne.w	load_err1

	cmp.l	#'ANIM',4(a1)
	beq	load_first_frame_anim
	
	cmp.l	#'ILBM',4(a1)
	bne.w	load_err1

	move.l	(a1),d0
	addq.l	#8,d0
	move.l	d0,vb_packed_size(a5)

	move.l	$4.w,a6
	jsr	_LVOForbid(a6)

	moveq	#ERR_NO_FMEM,d7

	moveq	#0,d1

	tst.b	db_stay_on(a3)
	beq	.nostay2
	or.l	#MEMF_STAY,d1
.nostay2:

	move.l	db_mlmmubase(a3),a6
	move.l	vb_filenaam(a5),a1		; naam van de file mpmmulib
	jsr	_LVOMLMMU_AllocMem(a6)
	move.l	d0,vb_packed(a5)

	IFNE VIEWINFO
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr3(pc),a0
	lea	.dat3(pc),a1
	move.l	d0,(a1)
	jsr	KPutFmt
	bra	.tt3
.dbstr3:	dc.b	"ALLOCED at %lx",10,0
	even
.dat3:	dc.l	0
.tt3:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	tst.l	d0
	bne.w	.okie_load
	
	move.l	$4.w,a6
	jsr	_LVOPermit(a6)
	bra	load_err2
	
.okie_load:
	move.l	d0,a0
	moveq	#0,d2				; seek beginning
check_load:
	IFNE VIEWINFO
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr3(pc),a0
	jsr	KPutStr
	bra	.tt3
.dbstr3:	dc.b	"check",10,0
	even
.tt3:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

act_load:

* test of de file al in het geheugen zit

	move.l	db_mlmmubase(a3),a6
	move.l	vb_packed(a5),a1
	jsr	_LVOMLMMU_GetMemStat(a6)	; haal status van memblock op

	and.l	#MTF_INIT,d0
	beq	.file_load

	move.l	$4.w,a6
	jsr	_LVOPermit(a6)

	bra	file_loaded

.file_load:

	move.l	vb_packed(a5),a1
	jsr	_LVOMLMMU_OwnMemBlk(a6)

	tst.l	d0
	bne	.oke_owned
;
; Another has locked this memblock wait until it is finished
;	
	move.l	$4.w,a6
	jsr	_LVOPermit(a6)

.notready:

	IFNE VIEWINFO
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr4(pc),a0
	jsr	KPutStr
	bra	.tt4
.dbstr4:	dc.b	"Wait for init",10,0
	even
.tt4:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	db_mlmmubase(a3),a6
	move.l	vb_packed(a5),a1
	jsr	_LVOMLMMU_GetMemStat(a6)	; haal status van memblock op

	and.l	#MTF_INIT,d0			; wait until inited
	beq	.notready

	bra	file_loaded

.oke_owned:
	move.l	$4.w,a6
	jsr	_LVOPermit(a6)

	move.l	vb_filehandle(a5),d1
	moveq	#-1,d3
	move.l	a0,-(a7)
	move.l	db_dosbase(a3),a6
	jsr	_LVOSeek(a6)		
	move.l	(a7)+,a0

	move.l	vb_packed_size(a5),d3

	move.l	d3,d6

	move.l	vb_packed(a5),d2
	move.l	vb_filehandle(a5),d1
	move.l	db_dosbase(a3),a6
	jsr	_LVORead(a6)		

	cmp.l	d0,d6
	beq	.okie

	IFNE VIEWINFO
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr2(pc),a0
	jsr	KPutStr
	bra	.tt2
.dbstr2:	dc.b	"Error in loadfile",10,0
	even
.tt2:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

.okie:
	move.l	db_mlmmubase(a3),a6
	move.l	vb_packed(a5),a1
	jsr	_LVOMLMMU_DisOwnMemBlk(a6)

	move.l	db_mlmmubase(a3),a6
	move.l	vb_packed(a5),a1
	moveq	#MTF_INIT,d0
	or.l	#MTF_SETCLR,d0
	jsr	_LVOMLMMU_SetMemStat(a6)	; file nu wel geladen

* hele file nu in het geheugen 

file_loaded:

	bsr.w	close_file

file_loaded2:
	moveq	#0,d0
	rts

load_err1:
	IFNE VIEWINFO
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr2(pc),a0
	jsr	KPutStr
	bra	.tt2
.dbstr2:	dc.b	"Error in loadfile",10,0
	even
.tt2:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	moveq	#1,d7
	bra	exit

load_err2:
	IFNE VIEWINFO
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr2(pc),a0
	jsr	KPutStr
	bra	.tt2
.dbstr2:	dc.b	"Load err 2",10,0
	even
.tt2:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC
	bra	exit

*
* Copies the filename with the error
* filenname in a0, error num in d0
* the destination is db_cop_colorsLOF(a3)
*
copy_err:
	IFNE XAPP
	tst.b	db_set_error(a3)	; when preloading don't give error's
	beq	zero_addr1

	IFNE VIEWINFO
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dat(pc),a1
	move.l	db_inactive_fileblok(a3),a0
	move.l	vb_filenaam(a0),a0
	move.l	a0,(a1)
	lea	.dbstr2(pc),a0
	jsr	KPutFmt
	bra	.tt2
.dbstr2:	dc.b	"Error loading %s",10,0
	even
.dat:	dc.l	0
.tt2:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	cmp.l	#0,a0
	bne	.oke_error
	lea	unk(pc),a0
.oke_error:
	subq.l	#1,d0
	lea.l	foutmelding(pc),a1
	lsl.l	#2,d0
	add.l	d0,a1
	lea.l	foutmelding(pc),a2
	add.l	(a1),a2
	move.l	db_waarcolors1(a3),a1
	cmp.l	#0,a1
	beq	zero_addr1
rep_cerr:
	move.b	(a2)+,(a1)+
	tst.b	(a2)
	bne	rep_cerr
rep_cerr2:
	move.b	(a0)+,(a1)+
	tst.b	(a0)
	bne	rep_cerr2
	move.b	#0,(a1)+
	move.l	db_mlmmubase(a3),a6
	move.l	db_waarcolors1(a3),a0
	moveq	#0,d0
	jsr	_LVOMLMMU_AddMsgToQueue(a6)
zero_addr1:
	ENDC
	rts	
unk:	dc.b	"Unknown",0
	even

close_file:
	move.l	db_dosbase(a3),a6
	move.l	vb_filehandle(a5),d1
	jsr	_LVOClose(a6)
	clr.l	vb_filehandle(a5)
	rts

waitline1:
	movem.l	d0/d1/a0,-(a7)

	add.w	d3,d1
	add.w	d3,d1

	move.l	vb_viewportw(a5),a0
	add.w	vp_DyOffset(a0),d1
	move.l	db_active_viewblok(a3),a0

	tst.b	vb_interlace(a0)
	beq	no_adjustint
	lsr.l	#1,d1
no_adjustint:
	bsr	waitline
now1:
	movem.l	(a7)+,d1/a0/d0
	rts

*
* wait until beam reaches d0
*
waitline:
	lea	$dff006,a0
	and.w	#$ff,d1
	cmp.w	#311,d1
	ble	lastline
	move.l	#311,d1
lastline:
	moveq	#0,d0
rep_wl:
;	btst	#6,$bfe001
;	beq	ew
	move.w	-2(a0),d0
	and.l	#$1,d0
	swap	d0
	move.w	(a0),d0
	lsr.l	#8,d0
	cmp.w	d0,d1
	bgt	rep_wl	
ew:
;	move.w	#$f0f,$dff180
	rts

*
* Need to set db_moz_source and db_mod_dest for this
*
calc_addr:
	tst.w	d0
	bpl	.oke
	move.l	d0,d1
	add.l	db_effect_speed(a3),d1
	tst.w	d1
	bmi	.no_addr
	move.l	db_effect_speed(a3),d1
	add.l	d0,d1
	moveq	#0,d0
	tst.w	d1
	bmi	.no_addr
	bra	.twee
.oke:
	move.l	db_effect_speed(a3),d1
	cmp.w	vb_leny(a5),d0
	bge	.no_addr
	add.l	d0,d1
	cmp.w	vb_leny(a5),d1
	ble	.go_addr
	sub.w	vb_leny(a5),d1
	move.l	d1,d2
	move.l	db_effect_speed(a3),d1
	sub.l	d2,d1
	beq	.no_addr
	bmi	.no_addr	
	bra	.twee
.go_addr:
	move.l	db_effect_speed(a3),d1
.twee:

;	move.l	vb_breedte_x(a5),d7
;	mulu	vb_planes(a5),d7	; in D1 de totale breedte 

	move.l	vb_totalwidth(a5),d7

	mulu	d7,d0			; vertikale offset 
	move.l	db_moz_dest(a3),a0
	add.l	d0,a0
	move.l	db_moz_source(a3),a6
	add.l	d0,a6

;	move.l	vb_breedte_x(a5),d7
;	mulu	vb_planes(a5),d7

	move.l	d7,d3
	lsr.l	#2,d7

	move.l	d3,d5
	move.l	d7,d2
	lsl.l	#2,d2
	sub.l	d2,d5
	add.l	d3,d5
	subq.l	#1,d7
	moveq	#0,d6
	rts
.no_addr:
	moveq	#-1,d6
	rts

enlarge_line_2:
	bsr	calc_addr
	bne	.now

	move.l	#$aaaaaaaa,d2

	move.l	d7,d6
	tst.l	d1
	beq	.one_line
	cmp.l	#1,d1
	beq	.one_line

	move.l	d1,-(a7)
	lsr.l	#1,d1
	subq.l	#1,d1
	move.l	d1,d4
.rep_lin:
	move.l	d6,d7
.rep_bigger:
	move.l	(a6)+,d0
	and.l	d2,d0
	move.l	d0,d1
	lsr.l	#1,d1
	or.l	d1,d0
	move.l	d0,0(a0,d3.w)
	move.l	d0,(a0)+
	dbf	d7,.rep_bigger
	add.w	d5,a0
	add.w	d5,a6
	subq.l	#2,(a7)
	dbf	d4,.rep_lin
	move.l	(a7)+,d1
	beq	.now
.one_line:
	move.l	d6,d7
.rep_bigger2:
	move.l	(a6)+,d0
	and.l	d2,d0
	move.l	d0,d1
	lsr.l	#1,d1
	or.l	d1,d0
	move.l	d0,(a0)+
	dbf	d7,.rep_bigger2
.now:
	rts

enlarge_line2_2:
	bsr	calc_addr
	bne	.now

	move.l	#$88888888,d2

	move.l	d7,d6

	tst.l	d1
	beq	.one_line
	cmp.l	#3,d1
	ble	.one_line

	move.l	d1,-(a7)
	lsr.l	#2,d1
	subq.l	#1,d1
	move.l	d1,d4
	move.l	a0,a1

	add.w	d3,a1
	add.w	d3,a1

	move.l	d3,d5
	lsl.l	#2,d5
	sub.l	d3,d5

.rep_lin:
	move.l	d6,d7
.rep_bigger:
	move.l	(a6)+,d0
	and.l	d2,d0
	move.l	d0,d1
	lsr.l	#1,d1
	or.l	d1,d0
	lsr.l	#1,d1
	or.l	d1,d0
	lsr.l	#1,d1
	or.l	d1,d0

	move.l	d0,0(a0,d3.w)
	move.l	d0,(a0)+
	move.l	d0,0(a1,d3.w)
	move.l	d0,(a1)+
	dbf	d7,.rep_bigger

	add.w	d5,a0
	add.w	d5,a1
	add.w	d5,a6

	subq.l	#4,(a7)
	dbf	d4,.rep_lin
	move.l	(a7)+,d1
	beq	.now
	move.l	a1,a0
	sub.w	d5,a0
	add.w	d3,a0
.one_line:
	move.l	d1,d4
	bra	.in_big
.rep_lin2:
	move.l	d6,d7
.rep_bigger2:
	move.l	(a6)+,d0
	and.l	d2,d0
	move.l	d0,d1
	lsr.l	#1,d1
	or.l	d1,d0
	lsr.l	#1,d1
	or.l	d1,d0
	lsr.l	#1,d1
	or.l	d1,d0
	move.l	d0,(a0)+
	dbf	d7,.rep_bigger2
.in_big:
	dbf	d4,.rep_lin2

.now:
	rts

enlarge_line3_2:
	movem.l	a2/a3,-(a7)
	bsr	calc_addr
	bne	.now

	move.l	d1,d2
	move.l	d7,d6
	
	lsr.l	#3,d1

	move.l	d3,d5
	lsl.l	#1,d5

	move.l	a0,a1
	add.w	d5,a1
	move.l	a1,a2
	add.w	d5,a2
	move.l	a2,a3
	add.w	d5,a3
	lsl.l	#2,d5

	tst.w	d1
	beq	.one_line

	subq.l	#1,d1
	move.l	d1,d4
	
	move.l	d2,-(a7)
	sub.l	d3,d5
.rep_lin:
	move.l	d6,d7
.rep_bigger:
	moveq	#0,d0
	move.l	(a6)+,d2
	btst	#31,d2
	beq	.z1
	move.w	#$ff00,d0
.z1:
	btst	#23,d2
	beq	.z2
	or.w	#$ff,d0
.z2:
	swap	d0
	btst	#15,d2
	beq	.z3
	or.w	#$ff00,d0
.z3:
	btst	#15,d2
	beq	.z4
	or.w	#$00ff,d0
.z4:
	move.l	d0,0(a0,d3.w)
	move.l	d0,(a0)+
	move.l	d0,0(a1,d3.w)
	move.l	d0,(a1)+
	move.l	d0,0(a2,d3.w)
	move.l	d0,(a2)+
	move.l	d0,0(a3,d3.w)
	move.l	d0,(a3)+
	dbf	d7,.rep_bigger

	add.l	d5,a0
	add.l	d5,a1
	add.l	d5,a2
	add.l	d5,a3
	add.l	d5,a6
	subq.l	#8,(a7)
	dbf	d4,.rep_lin

	move.l	(a7)+,d1
	beq	.now
	bmi	.now
	move.l	d1,d2
	bra	.one_line
.now:
	movem.l	(a7)+,a2/a3
	rts

.one_line:
onl:
	move.l	d2,d4
	bra	.in_rep
.rep_lin2:
	move.l	d6,d7
.rep_bigger2:
	moveq	#0,d0
	move.l	(a6)+,d2
	btst	#31,d2
	beq	.z1
	move.w	#$ff00,d0
.z1:
	btst	#23,d2
	beq	.z2
	or.w	#$ff,d0
.z2:
	swap	d0
	btst	#15,d2
	beq	.z3
	or.w	#$ff00,d0
.z3:
	btst	#15,d2
	beq	.z4
	or.w	#$00ff,d0
.z4:
	move.l	d0,(a0)+
	dbf	d7,.rep_bigger2
.in_rep:
	dbf	d4,.rep_lin2
	movem.l	(a7)+,a2/a3
	rts

enlarge_line4_2:
	bsr	calc_addr
	bne	.now

	move.l	d1,d2
	lsr.l	#4,d1

	move.l	d1,d4
	move.l	d7,d6
	tst.w	d1
	beq	one_line_2
	
	move.l	d2,-(a7)
	bra	.in_line
.rep_lin:
	move.l	d6,d7
.rep_bigger:
	moveq	#0,d0
	move.l	(a6)+,d2
	btst	#31,d2
	beq	.z1
	move.w	#$ffff,d0
.z1:
	swap	d0
	btst	#15,d2
	beq	.z3
	move.w	#$ffff,d0
.z3:
	move.l	a0,a1
	moveq	#15,d1
.repp:
	move.l	d0,(a1)
	add.w	d3,a1
	dbf	d1,.repp
	addq.l	#4,a0
	dbf	d7,.rep_bigger
	lea.l	4(a1),a0
	sub.w	d3,a0

	move.l	db_moz_dest(a3),d2
	move.l	db_moz_source(a3),a6
	move.l	a0,d0
	sub.l	d2,d0
	add.l	d0,a6

	sub.l	#16,(a7)
.in_line:
	dbf	d4,.rep_lin
	move.l	(a7)+,d1
	beq	.now
	bmi	.now
	move.l	d1,d2
	bra	one_line_2
.now:
	rts

one_line_2:
	move.l	d2,d4
	bra	.in_line
.rep_lin:
	move.l	d6,d7
.rep_bigger:
	moveq	#0,d0
	move.l	(a6)+,d2
	btst	#31,d2
	beq	.z1
	move.w	#$ffff,d0
.z1:
	swap	d0
	btst	#15,d2
	beq	.z3
	move.w	#$ffff,d0
.z3:
	move.l	d0,(a0)+
	dbf	d7,.rep_bigger
.in_line:
	dbf	d4,.rep_lin
	rts

enlarge_line5_2:
	bsr	calc_addr
	bne	.now

	move.l	d1,d2
	lsr.l	#5,d1
	
	move.l	d1,d4
	move.l	d7,d6
	tst.w	d1
	beq	one_line22
	move.l	d2,-(a7)
	bra	.in_line
.rep_line:
	move.l	d6,d7
.rep_bigger:
	moveq	#0,d0
	move.w	(a6),d2
	btst	#15,d2
	beq	.zer
	moveq	#-1,d0
.zer:
	moveq	#31,d1
	move.l	a0,a1
.repp:
	move.l	d0,(a1)
	add.w	d3,a1
	dbf	d1,.repp
	addq.l	#4,a0
	addq.l	#4,a6
	dbf	d7,.rep_bigger
	lea.l	4(a1),a0
	sub.w	d3,a0
	move.l	db_moz_dest(a3),d2
	move.l	db_moz_source(a3),a6
	move.l	a0,d0
	sub.l	d2,d0
	add.l	d0,a6

	sub.l	#32,(a7)
.in_line:
	dbf	d4,.rep_line
	move.l	(a7)+,d1
	beq	.now
	bmi	.now
	move.l	d1,d2
	bra	one_line22
.now:
	rts

one_line22:
	move.l	d2,d4
	beq	.in_line
	move.l	d6,d7
.rep_bigger:
	moveq	#0,d0
	move.w	(a6),d2
	btst	#15,d2
	beq	.zer
	moveq	#-1,d0
.zer:
	move.l	d4,d1
	move.l	a0,a1
.rep_line:
	move.l	d0,(a1)
	add.w	d3,a1
	dbf	d1,.rep_line

	addq.l	#4,a6
	addq.l	#4,a0
	dbf	d7,.rep_bigger
.in_line:
	rts

enlarge_line6_2:
	bsr	calc_addr
	bne	.now

	move.l	d1,d2
	lsr.l	#6,d1
	addq.l	#1,d7
	lsr.l	#1,d7		; need double longs
	tst.w	d7
	beq	.now
	subq.l	#1,d7
	move.l	d1,d4
	move.l	d7,d6
	tst.w	d1
	beq	one_line32
	move.l	d2,-(a7)
	bra	.in_line
.rep_line:
	move.l	d6,d7
.rep_bigger:
	moveq	#0,d0
	move.l	(a6),d2
	btst	#31,d2
	beq	.zer
	moveq	#-1,d0
.zer:
	moveq	#63,d1
	move.l	a0,a1
.repp:
	move.l	d0,(a1)
	move.l	d0,4(a1)
	add.w	d3,a1
	dbf	d1,.repp
	addq.l	#8,a0
	addq.l	#8,a6
	dbf	d7,.rep_bigger
	move.l	a1,a0
	sub.w	d3,a0
	move.l	db_moz_dest(a3),d2
	move.l	db_moz_source(a3),a6
	move.l	a0,d0
	sub.l	d2,d0
	add.l	d0,a6
	sub.l	#64,(a7)
.in_line:
	dbf	d4,.rep_line
	move.l	(a7)+,d1
	beq	.now
	bmi	.now
	move.l	d1,d2
	bra	one_line32
.now:
	rts

one_line32:
	move.l	d2,d4
	beq	.now
	subq.l	#1,d4
	move.l	d6,d7
.rep_bigger:

	moveq	#0,d0
	move.l	(a6),d2
	btst	#31,d2
	beq	.zer
	moveq	#-1,d0
.zer:
	move.l	d4,d1
	move.l	a0,a1
.repp:
	move.l	d0,(a1)
	move.l	d0,4(a1)
	add.w	d3,a1
	dbf	d1,.repp
	addq.l	#8,a0
	addq.l	#8,a6
	dbf	d7,.rep_bigger
.now:
	rts

enlarge_line7_2:
	bsr	calc_addr
	bne	.now

	move.l	d1,d2
	lsr.l	#7,d1
	addq.l	#1,d7
	lsr.l	#2,d7		; need Joost longs
	tst.w	d7
	beq	.now

	subq.l	#1,d7
	move.l	d1,d4
	move.l	d7,d6
	tst.w	d1
	beq	one_line42
	move.l	d2,-(a7)
	bra	.in_line
.rep_line:
	move.l	d6,d7
.rep_bigger:
	moveq	#0,d0
	move.l	(a6),d2
	btst	#31,d2
	beq	.zer
	moveq	#-1,d0
.zer:
	moveq	#127,d1
	move.l	a0,a1
.repp:
	move.l	d0,(a1)
	move.l	d0,4(a1)
	move.l	d0,8(a1)
	move.l	d0,12(a1)
	add.w	d3,a1
	dbf	d1,.repp
	add.w	#16,a0
	add.w	#16,a6
	dbf	d7,.rep_bigger
	move.l	a1,a0
	sub.w	d3,a0
	move.l	db_moz_dest(a3),d2
	move.l	db_moz_source(a3),a6
	move.l	a0,d0
	sub.l	d2,d0
	add.l	d0,a6
	sub.l	#128,(a7)
.in_line:
	dbf	d4,.rep_line

	move.l	(a7)+,d1
	beq	.now
	bmi	.now
	move.l	d1,d2
	bra	one_line42
.now:
	rts

one_line42:
	move.l	d2,d4
	beq	.now
	subq.l	#1,d4
	move.l	d6,d7
.rep_bigger:
	move.l	(a6),d2
	moveq	#0,d0
	btst	#31,d2
	beq	.zer
	moveq	#-1,d0
.zer:
	move.l	d4,d1
	move.l	a0,a1
.repp:
	move.l	d0,(a1)
	move.l	d0,4(a1)
	move.l	d0,8(a1)
	move.l	d0,12(a1)
	add.w	d3,a1
	dbf	d1,.repp
	add.w	#16,a0
	add.w	#16,a6
	dbf	d7,.rep_bigger
.now:
	rts

place_line:
	bsr	calc_addr
	bne	.now
	move.l	d1,d4
	move.l	d7,d6
.rep_line:
	move.l	d6,d7
.rep_bigger:
	move.l	(a6)+,(a0)+
	dbf	d7,.rep_bigger
	dbf	d4,.rep_line
.now:
	rts

larger_procs:
	dc.l	enlarge_line_2
	dc.l	enlarge_line2_2
	dc.l	enlarge_line3_2
	dc.l	enlarge_line4_2
	dc.l	enlarge_line5_2
	dc.l	enlarge_line6_2
	dc.l	enlarge_line7_2
	dc.l	0

*
* Allocate and copy the chipmem buffer to fastmem
*
copy_to_fast:
	move.l	vb_breedte_x(a5),d0
	mulu	vb_planes(a5),d0
	mulu	vb_leny(a5),d0
	move.l	d0,vb_fastsize(a5)
	move.l	d0,d2
	move.l	#MEMF_PUBLIC,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,vb_fastbuffer(a5)
	beq	.nof
	move.l	d0,a1
	move.l	vb_tempbuffer(a5),a0
	move.l	d2,d0
	lsr.l	#2,d0
.repp:
	move.l	(a0)+,(a1)+
	subq.l	#1,d0
	bne	.repp
.nof:
	rts

release_fast:
	tst.l	vb_fastbuffer(a5)
	beq	.nof
	move.l	vb_fastsize(a5),d0
	move.l	vb_fastbuffer(a5),a1
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)
	clr.l	vb_fastbuffer(a5)
.nof:
	rts

	RSRESET
colm_struct:	rs.w	0
colm_source:	rs.l	1
colm_dest:	rs.l	1
colm_amount:	rs.l	1
colm_offset:	rs.l	1
colm_SIZEOF:	rs.w	0

*
* Copy the column pointed at by a0 ( colm_struct *)
* a5 points to the active viewblok
*
*
copy_column:
	move.l	vb_breedte_x(a5),d7
	move.l	colm_offset(a0),d0
	move.l	colm_amount(a0),d1
	add.l	d1,d0
	cmp.w	vb_leny(a5),d0
	ble	.y_size_oke
	sub.w	vb_leny(a5),d0
	move.l	d0,d2
	move.l	colm_amount(a0),d1
	sub.l	d2,d1
	move.l	#0,colm_amount(a0)		; switch off this one
.y_size_oke:

; in d0 the number off lines to copy

	move.l	d1,d0

	add.l	d0,colm_offset(a0)
	move.l	colm_offset(a0),d6		; copy from 
	bmi	.no_colm
	mulu	vb_planes(a5),d0		; nr off line to copy
	move.l	colm_source(a0),a1
	move.l	colm_dest(a0),a2
	tst.b	db_fold_long(a3)
	bne	.inlongmode
	bra	.incopy
.repli:
	move.w	(a1),(a2)
	add.w	d7,a1
	add.w	d7,a2
.incopy:
	dbf	d0,.repli
	bra	.gocol

.replilong:
	move.l	(a1),(a2)
	add.w	d7,a1
	add.w	d7,a2
.inlongmode:
	dbf	d0,.replilong
	bra	.gocol

.gocol:
	move.l	a1,colm_source(a0)
	move.l	a2,colm_dest(a0)

; now copy from the original picture reverse below the new picture
; start with the copy from dest and reverse offset lines or until the bottom

	move.l	vb_tempbuffer(a5),d0
	move.l	a2,d1
	sub.l	d0,d1
	move.l	db_moz_source(a3),a1
	add.l	d1,a1			; point to the old picture in fast

; you are now at point d6, need to mirror d6 lines back does that fit	

	move.l	d6,d0
	beq	.no_colm
	bmi	.no_colm
	add.l	d0,d0
	cmp.w	vb_leny(a5),d0
	ble	.y_size2_oke
	sub.w	vb_leny(a5),d0
	sub.l	d0,d6
	bmi	.no_colm
.y_size2_oke:
	move.l	vb_totalwidth(a5),d1
	move.w	vb_planes(a5),d2
	move.l	a1,a6
	tst.b	db_fold_long(a3)
	bne	longmode
	bra	.incopy2
.repli2:
	move.w	d2,d0
	move.l	a6,a1
	bra	.in_pl
.rep_pl:
	move.w	(a1),(a2)
	add.w	d7,a1
	add.w	d7,a2
.in_pl:
	dbf	d0,.rep_pl
	sub.l	d1,a6
.incopy2:
	dbf	d6,.repli2
.no_colm:
	rts
longmode:
	bra	.incopy2
.repli2:
	move.w	d2,d0
	move.l	a6,a1
	bra	.in_pl
.rep_pl:
	move.l	(a1),(a2)
	add.w	d7,a1
	add.w	d7,a2
.in_pl:
	dbf	d0,.rep_pl
	sub.l	d1,a6
.incopy2:
	dbf	d6,.repli2
	rts

fold_vers1:
	move.l	db_effect_speed(a3),d2
	move.l	vb_tempbuffer(a5),a1
	move.l	vb_tempbuffer(a4),a2
	sub.l	d2,d1
	bra	.in_init
.rep_init:
	move.l	a1,colm_dest(a0)
	move.l	a2,colm_source(a0)
	move.l	d1,colm_offset(a0)
	move.l	d2,colm_amount(a0)
	add.w	d3,a1
	add.w	d3,a2
	sub.l	d2,d1
	add.l	#colm_SIZEOF,a0
.in_init:
	dbf	d0,.rep_init
	sub.l	#colm_SIZEOF,a0
	rts

fold_vers2:
	move.l	d0,d7
	add.l	d7,d7
	tst.b	db_fold_long(a3)
	beq	.no_doub
	add.l	d7,d7
.no_doub:
	sub.l	d3,d7
	move.l	db_effect_speed(a3),d2
	move.l	vb_tempbuffer(a5),a1
	move.l	vb_tempbuffer(a4),a2
	add.l	d7,a1
	add.l	d7,a2
	sub.l	d2,d1
	bra	.in_init
.rep_init:
	move.l	a1,colm_dest(a0)
	move.l	a2,colm_source(a0)
	move.l	d1,colm_offset(a0)
	move.l	d2,colm_amount(a0)
	sub.w	d3,a1
	sub.w	d3,a2
	sub.l	d2,d1
	add.l	#colm_SIZEOF,a0
.in_init:
	dbf	d0,.rep_init
	sub.l	#colm_SIZEOF,a0
	rts

fold_vers3:
	move.l	d0,d7
	lsr.w	#1,d7
	tst.b	db_fold_long(a3)
	beq	.nol
	add.w	d7,d7
.nol:
	add.w	d7,d7				; even
	move.l	db_effect_speed(a3),d2
	move.l	vb_tempbuffer(a5),a1
	move.l	vb_tempbuffer(a4),a2
	move.l	a1,a6
	sub.w	d3,a6
	add.l	d7,a1
	add.l	d7,a2
	sub.l	d2,d1
	bra	.in_init
.rep_init:
	move.l	a1,colm_dest(a0)
	move.l	a2,colm_source(a0)
	move.l	d1,colm_offset(a0)
	move.l	d2,colm_amount(a0)
	sub.w	d3,a1
	sub.w	d3,a2
	sub.l	d2,d1
	add.l	#colm_SIZEOF,a0
.in_init:
	cmp.l	a6,a1
	bne	.rep_init

	move.l	d1,d6
	move.l	a0,d0

	move.l	vb_tempbuffer(a5),a1
	move.l	vb_tempbuffer(a4),a2
	move.l	a1,a6
	add.l	d7,a6
	add.l	d7,d7
	sub.l	d3,d7
	add.l	d7,a1
	add.l	d7,a2
	add.l	d2,d1
	bra	.in_init2
.rep_init2:
	move.l	a1,colm_dest(a0)
	move.l	a2,colm_source(a0)
	move.l	d1,colm_offset(a0)
	move.l	d2,colm_amount(a0)
	sub.w	d3,a1
	sub.w	d3,a2
	add.l	d2,d1
	add.l	#colm_SIZEOF,a0
.in_init2:
	cmp.l	a6,a1
	bne	.rep_init2
	move.l	d6,d1
	move.l	d0,a0
	sub.l	#colm_SIZEOF,a0
	rts

page_fold1:
	lea	teller,a0
	move.l	#0,(a0)
	move.l	#0,4(a0)
	move.l	db_active_viewblok(a3),a5
	bsr	copy_to_fast
	move.l	db_inactive_viewblok(a3),a4
	move.l	vb_fastbuffer(a5),d0
	beq	.no_effect
	move.l	d0,db_moz_source(a3)
	move.l	vb_breedte_x(a5),d0
	mulu	vb_planes(a5),d0
	move.l	d0,vb_totalwidth(a5)
	move.l	db_wpatroonruimte(a3),a0
	move.l	vb_breedte_x(a5),d0

	addq.l	#1,db_varltimes(a3)
	move.b	#0,db_fold_long(a3)
	moveq	#2,d3
	cmp.l	#3,db_varltimes(a3)
	ble	.nom
	sub.l	#3,db_varltimes(a3)
	move.b	#1,db_fold_long(a3)
	moveq	#4,d3
	lsr.l	#1,d0				; nr off longs
.nom:
	lsr.l	#1,d0				; nr off words

	moveq	#0,d1				; start offset
	cmp.l	#1,db_varltimes(a3)
	bne	.no1
	bsr	fold_vers1
	bra	.id
.no1:
	cmp.l	#2,db_varltimes(a3)
	bne	.no2
	bsr	fold_vers2
	bra	.id
.no2:
	bsr	fold_vers3
.id:	
	move.l	a0,a2
	neg.l	d1
	move.l	d1,d0
	add.w	vb_leny(a5),d0
	move.l	db_effect_speed(a3),d1
	bsr	set_fade_teller

.cont:
	bsr	wacht_tijd2
	tst.w	d0
	bne	.nok

	move.l	a2,-(a7)
	move.l	vb_breedte_x(a5),d0
	tst.b	db_fold_long(a3)
	beq	.nof1
	lsr.l	#1,d0
.nof1:
	lsr.l	#1,d0				; nr off words
	move.l	db_wpatroonruimte(a3),a0
	bra	.inco1
.rept:
	move.l	d0,-(a7)
	bsr	copy_column
	move.l	(a7)+,d0
	add.l	#colm_SIZEOF,a0
.inco1:
	dbf	d0,.rept
	move.l	a0,-(a7)
	bsr	test_fade
	move.l	(a7)+,a0
	move.l	(a7)+,a2
	tst.l	colm_amount(a2)
	bne	.cont
.nok:
	move.l	db_active_viewblok(a3),a5
	bsr	release_fast

.no_effect:
	rts

mozaik1:
	move.l	db_inactive_viewblok(a3),a5
	bsr	copy_to_fast
	move.l	vb_breedte_x(a5),d0
	mulu	vb_planes(a5),d0
	move.l	d0,vb_totalwidth(a5)

	move.l	db_active_viewblok(a3),a5
	clr.l	vb_fastbuffer(a5)
;	bsr	copy_to_fast
	move.l	vb_breedte_x(a5),d0
	mulu	vb_planes(a5),d0
	move.l	d0,vb_totalwidth(a5)

	moveq	#0,d0
	move.w	vb_leny(a5),d0
	move.l	db_effect_speed(a3),d1
	bsr	set_fade_teller
	clr.l	db_varltimes(a3)
	clr.l	db_shiftit(a3)

	lea	teller,a0
	moveq	#20,d1
	sub.l	db_effect_speed(a3),d1
	lsl.l	#2,d1
	move.l	d1,(a0)
	move.l	d1,4(a0)

	moveq	#0,d0
	move.w	vb_leny(a5),d0
	move.l	d0,db_effect_speed(a3)
	moveq	#0,d0

	move.l	vb_tempbuffer(a5),db_moz_source(a3)
	move.l	vb_tempbuffer(a5),db_moz_dest(a3)

	lea	larger_procs(pc),a2

	move.l	#6,d1
.rep_la:
	movem.l	d1,-(a7)
	moveq	#0,d0
	move.l	(a2)+,a0
	jsr	(a0)
	bsr	wacht_tijd2
	movem.l	(a7)+,d1
	bne	.exit_moz2
	dbf	d1,.rep_la

	move.l	db_inactive_viewblok(a3),a5
	tst.l	vb_fastbuffer(a5)
	beq	.exit_moz2

	move.l	vb_fastbuffer(a5),db_moz_source(a3)
	move.l	vb_tempbuffer(a5),db_moz_dest(a3)
	move.w	vb_leny(a5),d0
	move.l	d0,db_effect_speed(a3)

	moveq	#0,d0
	move.l	-(a2),a0
	jsr	(a0)

	move.l	a2,-(a7)
	bsr	showinactive
	move.l	(a7)+,a2

	bsr	wacht_tijd2
	bne	.exit_moz
	move.l	db_active_viewblok(a3),a5

	move.l	#5,d1
.rep_la2:
	movem.l	d1,-(a7)
	moveq	#0,d0
	move.l	-(a2),a0
	jsr	(a0)
	bsr	wacht_tijd2
	movem.l	(a7)+,d1
	bne	.exit_moz
	dbf	d1,.rep_la2
.exit_moz:
	moveq	#0,d0
	bsr	place_line
	bra	.release
	rts

.exit_moz2:
	bsr	showinactive
.release:
	move.l	db_active_viewblok(a3),a5
	bsr	release_fast
;	move.l	db_inactive_viewblok(a3),a5
;	bsr	release_fast
	rts

mozaik_down:
	bsr	set_moz_init_vals
.wwh41:
	bsr	do_mozaik_partly
	bsr	wacht_tijd2
	bne	.exit_moz
	move.l	db_effect_speed(a3),d0
	add.l	d0,db_shiftit(a3)
	move.l	db_shiftit(a3),d0
	cmp.w	vb_leny(a5),d0
	bge.b	.exit_moz
	bsr	test_fade
	bra.w	.wwh41
.exit_moz:
	rts	

mozaik_up:
	bsr	set_moz_init_vals
	neg.l	db_links(a3)
	neg.l	db_shiftit(a3)
	moveq	#0,d0
	move.w	vb_leny(a5),d0
	add.l	d0,db_shiftit(a3)
.wwh41:
	bsr	do_mozaik_partly
	bsr	wacht_tijd2
	bne	.exit_moz

	move.l	db_effect_speed(a3),d0
	sub.l	d0,db_shiftit(a3)

	move.l	db_shiftit(a3),d0
	cmp.w	#0,d0
	ble.b	.exit_moz

	bsr	test_fade

	bra.w	.wwh41
.exit_moz:
	rts	

set_moz_init_vals:
	lea	tellerstart,a0
	move.l	#0,(a0)

;	bsr	calc_effect_speed_hor1

; reken uit na hoeveel stappen er een colorfade moet plaats vinden
	move.l	db_active_viewblok(a3),a5		; destination
	move.l	db_inactive_viewblok(a3),a4	; source

	move.l	db_wstore_moves2(a3),a0
	clr.l	(a0)
	clr.l	4(a0)
	moveq	#0,d0
	move.l	db_varltimes(a3),d1
	lsl.l	#3,d1
	cmp.l	db_effect_speed(a3),d1
	bge	.no_pro
	move.l	db_effect_speed(a3),d1
.no_pro:
	move.l	d1,db_links(a3)
	move.l	d1,d0
	lsl.l	#3,d0
	add.l	d1,d0
	add.l	d1,d0
	neg.l	d0
	move.l	d0,db_shiftit(a3)		; last line to place

	moveq	#0,d0
	move.w	vb_leny(a5),d0
	move.l	db_shiftit(a3),d1
	neg.l	d1
	add.l	d1,d0
	move.l	db_effect_speed(a3),d1
	bsr	set_fade_teller

	move.l	vb_breedte_x(a5),d0
	mulu	vb_planes(a5),d0
	move.l	d0,vb_totalwidth(a5)

	rts
	
do_mozaik_partly:
	move.l	db_shiftit(a3),db_rechts(a3)
	move.l	db_inactive_viewblok(a3),a4
	move.l	vb_tempbuffer(a4),db_moz_source(a3)
	move.l	vb_tempbuffer(a5),db_moz_dest(a3)
	move.l	db_rechts(a3),d0
	bsr	place_line
	lea	larger_procs(pc),a2
	move.l	#4,d1
.rep_la:
	movem.l	d1,-(a7)
	move.l	db_links(a3),d1
	add.l	d1,db_rechts(a3)
	move.l	db_rechts(a3),d0
	move.l	(a2)+,a0
	jsr	(a0)
	movem.l	(a7)+,d1
	dbf	d1,.rep_la

	move.l	vb_tempbuffer(a5),db_moz_source(a3)
	move.l	vb_tempbuffer(a5),db_moz_dest(a3)

	move.l	#4,d1
.rep_la2:
	movem.l	d1,-(a7)
	move.l	db_links(a3),d1
	add.l	d1,db_rechts(a3)
	move.l	db_rechts(a3),d0
	move.l	-(a2),a0
	jsr	(a0)
	movem.l	(a7)+,d1
	dbf	d1,.rep_la2
	rts
	
	XDEF	_unload_file

_unload_file:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a3

	lea	db_fileblok1(a3),a5
	tst.l	vb_packed(a5)
	beq.s	no_rem

	IFNE VIEWINFO
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dat(pc),a1
	move.l	vb_filenaam(a5),a0
	move.l	a0,(a1)
	lea	.dbstr2(pc),a0
	jsr	KPutFmt
	bra	.tt2
.dbstr2:	dc.b	"UNLoad file %s",10,0
	even
.dat:	dc.l	0
.tt2:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	vb_packed(a5),a1
	move.l	vb_packed_size(a5),d0
	move.l	db_mlmmubase(a3),a6
	jsr	_LVOMLMMU_FreeMem(a6)
	move.b	#0,vb_id(a5)		; unload file
	clr.l	vb_packed(a5)
no_rem:
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	rts
	
	XDEF	_release_slide2
_release_slide2
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a3
;	move.l	db_monitorspec(a3),d0
;	beq	.nomon
;	move.l	d0,a0
;	move.l	db_graphbase(a3),a6
;	jsr	_LVOCloseMonitor(a6)
;.nomon:
	bsr	closelibs
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

	IFEQ XAPP
exit:	
	lea	startdatablock(pc),a3

	jsr	remove_50h
	jsr	freesam

	bsr.w	cleanexit
	move.l	db_easy_exit(a3),a7
ee:
	move.l	d7,db_tell(a3)		; store error
;	jsr	set_sprite

	move.l	db_inactive_viewblok(a3),a5
	cmp.l	#0,a5
	beq	no_free11
	bsr.w	free_memory_view
no_free11:
	move.l	db_active_viewblok(a3),a5
	cmp.l	#0,a5
	beq	no_free12
	bsr.w	free_memory_view
no_free12:

	move.l	db_graphbase(a3),a6
	move.l	it_vextra,d0
	beq	.no1
	move.l	d0,a0
	jsr	_LVOGfxFree(a6)
.no1:
	move.l	at_vextra,d0
	beq	.no2
	move.l	d0,a0
	jsr	_LVOGfxFree(a6)
.no2:
	move.l	it_vpextra,d0
	beq	.no3
	move.l	d0,a0
	jsr	_LVOGfxFree(a6)
.no3:
	move.l	at_vpextra,d0
	beq	.no4
	move.l	d0,a0
	jsr	_LVOGfxFree(a6)
.no4:
;	move.l	db_monitorspec(a3),d0
;	beq	.nomitor
;	move.l	db_graphbase(a3),a6
;	move.l	d0,a0
;	jsr	_LVOCloseMonitor(a6)
;.nomitor:
	tst.l	db_fastscrap_mem(a3)
	beq	exit02
	move.l	db_fastscrap_mem(a3),a1
	move.l	db_fastscrap_size(a3),d0
	move.l	db_mlmmubase(a3),a6
	jsr	_LVOMLMMU_FreeMem(a6)
	clr.l	db_fastscrap_mem(a3)
exit02:
	tst.l	db_chipscrap_mem(a3)
	beq	exit01
	move.l	db_chipscrap_mem(a3),a1
	move.l	db_chipscrap_size(a3),d0
;	move.l	CHIPSCRAPSIZE-4(a1),test
	move.l	db_mlmmubase(a3),a6
	jsr	_LVOMLMMU_FreeMem(a6)
	clr.l	db_chipscrap_mem(a3)

exit01:
	tst.l	db_unpacked1(a3)
	beq.s	exit1
	move.l	db_unpacked1(a3),a1
	move.l	db_unpacked1_size(a3),d0
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)
	clr.l	db_unpacked1(a3)

exit1:	
	tst.l	db_unpacked2(a3)
	beq.s	exit11
	move.l	db_unpacked2(a3),a1
	move.l	db_unpacked2_size(a3),d0
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)
	clr.l	db_unpacked2(a3)

exit11:
	moveq	#0,d7
	tst.l	db_triple_mem(a3)
	beq.s	exit11t
	move.l	db_triple_mem(a3),a1
	move.l	db_triple_size(a3),d0
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)
	clr.l	db_triple_mem(a3)

exit11t:
	lea	db_fileblok1(a3),a5
	tst.l	vb_packed(a5)
	beq.s	exit221
	move.l	vb_packed(a5),a1
	move.l	vb_packed_size(a5),d0
	move.l	db_mlmmubase(a3),a6
	jsr	_LVOMLMMU_FreeMem(a6)
	clr.l	vb_packed(a5)
exit221:

	lea	db_fileblok1(a3),a5
	tst.l	vb_filehandle(a5)
	beq.s	exit21
	bsr.w	close_file
exit21:

	tst.l	db_graphbase(a3)
	beq.s	exit3
	move.l	db_graphbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	db_graphbase(a3)
exit3:

	tst.l	db_intbase(a3)
	beq	exit4c
	move.l	db_intbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	db_intbase(a3)
exit4c:
	tst.l	db_mathtransbase(a3)
	beq.s	exit4b
	move.l	db_mathtransbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	db_mathtransbase(a3)
exit4b:
	tst.l	db_mathffpbase(a3)
	beq.s	exit4d
	move.l	db_mathffpbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	db_mathffpbase(a3)
exit4d:
	tst.l	db_layersbase(a3)
	beq.s	exit4e
	move.l	db_layersbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	db_layersbase(a3)
exit4e:
	tst.l	db_mlmmubase(a3)
	beq.s	exit4
	move.l	db_mlmmubase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	db_mlmmubase(a3)
exit4:
	tst.l	d7
	beq.s	exit51

	subq.l	#1,d7

	move.l	db_dosbase(a3),a6
	lea.l	foutmelding(pc),a1
	lsl.l	#2,d7
	add.l	d7,a1

	move.l	db_inactive_fileblok(a3),a0
	move.l	vb_filenaam(a0),a0
	lea	foutmelding(pc),a2
	move.l	a2,d2
	add.l	(a1),d2
	moveq	#0,d3
	move.l	d2,a0
calc_len:
	addq.l	#1,d3
	cmp.b	#0,(a0)+

	bsr	no_con_open

exit51:
	move.l	db_dosbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	db_dosbase(a3)
exit5:
	move.l	db_tell(a3),d0
	lea	error(pc),a0
	move.l	d0,(a0)

	movem.l	(a7)+,d0-d7/a0-a6
	move.l	error(pc),d0
	rts

no_con_open:
	move.l	d2,a0
	move.l	#FOUT_LENGTE-3,d3

	lea.l	db_fileblok1(a3),a1	; use as temp

	move.b	#0,(a1)+
	move.b	#20,(a1)+
	move.b	#20,(a1)+

c_fout:	move.b	(a0)+,(a1)+
	cmp.b	#10,(a0)
	bne	c_fout

	cmp.l	#3,d7
	bgt	no_filenaam

	move.b	#' ',(a1)+
	move.b	#':',(a1)+
	move.b	#' ',(a1)+

	lea.l	db_fileblok1(a3),a5
	move.l	vb_filenaam(a5),a0
c2_fout:
	move.b	(a0)+,d0
	move.b	d0,(a1)+
	bne.b	c2_fout

no_filenaam:		
	move.b	#0,(a1)+
	move.b	#0,(a1)+

	move.l	$4.w,a6
	lea.l	intui,a1
	jsr	_LVOOpenLibrary(a6)

	tst.l	d0
	beq.b	err_intui
	move.l	d0,a6
	moveq	#0,d0

	lea.l	db_fileblok1(a3),a0
	moveq	#40,d1
	jsr	_LVODisplayAlert(a6)

	move.l	a6,a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
err_intui
	rts

FOUT_LENGTE = 21

	ELSE
exit:
	tst.b	db_sema_on(a3)
	beq	.no_sema
	move.l	db_mlsysstruct(a3),a0
	lea	ml_sema_trans(a0),a0
	move.l	$4.w,a6
	jsr	_LVOObtainSemaphore(a6)
.no_sema:

exit11t:
	tst.w	d7
	beq	no_err
	move.l	vb_filenaam(a5),a0
	move.l	d7,d0
	bsr	copy_err
no_err:
	move.l	d7,d0
	move.l	db_easy_exit(a3),a7
	lea	error(pc),a0
	clr.l	(a0)
	tst.b	db_give_error(a3)
	bne	.noerr
	move.l	d0,(a0)
.noerr:
	move.b	#0,db_give_error(a3)
	
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	move.l	error(pc),d0
	rts

	ENDC

foutmelding:
	dc.l	fout1-foutmelding,fout2-foutmelding,fout3-foutmelding
	dc.l	fout4-foutmelding,fout5-foutmelding,fout6-foutmelding
	dc.l	fout7-foutmelding

fout1:		dc.b	"File not found ",0
fout2:		dc.b	"Not enough chip memory ",0
fout3:		dc.b	"Not enough fast memory ",0
fout4:		dc.b	"Resolution not supported ",0
fout5:		dc.b	"General error",10,0
fout6:		dc.b	"File type no supported ",0
fout7:		dc.b	"Mangled IFF file ",10,0	
	even

closelibs:
	tst.l	db_graphbase(a3)
	beq.s	cexit3
	move.l	db_graphbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	db_graphbase(a3)
cexit3:
	tst.l	db_intbase(a3)
	beq	cexit4c
	move.l	db_intbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	db_intbase(a3)
cexit4c:
	tst.l	db_mathtransbase(a3)
	beq.s	cexit4b
	move.l	db_mathtransbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	db_mathtransbase(a3)
cexit4b:
	tst.l	db_mathffpbase(a3)
	beq.s	cexit4d
	move.l	db_mathffpbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	db_mathffpbase(a3)
cexit4d:
	tst.l	db_layersbase(a3)
	beq.s	cexit4e
	move.l	db_layersbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	db_layersbase(a3)
cexit4e:
	tst.l	db_mlmmubase(a3)
	beq.s	cexit4
	move.l	db_mlmmubase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	db_mlmmubase(a3)
cexit4:
	rts
error:			dc.l	0

	IFEQ XAPP
startdatablock:	
	ds.b	db_SIZEOF
	ENDC

*
* The anim playing part uses the same routines as the normal
* anim player, you can't however :
* add loop frames
* use continue mode or picture mode
*
*

speed= 4
DISKANIM = 1
ROTS = 5

MLMMU = 1
ANIM32 = 1

	IFEQ XAPP
;animname:	dc.b	"hd2:anims/part3",0
;animname:	dc.b	"work:mediapoint/graphics/animations/Anim_Cube",0
animname:	dc.b	"work:tanim/tel1-5",0
	even
	ENDC
*
* Fill in all the data animplay needs
*
init_anim_struct:
	move.l	a0,a4
	lea	anim_struct(pc),a2

	move.l	db_graphbase(a3),aw_graphbase(a2)
	move.l	db_dosbase(a3),aw_dosbase(a2)
	move.l	db_intbase(a3),aw_intbase(a2)
	move.l	db_mlmmubase(a3),aw_mlmmubase(a2)
	move.l	db_triple_mem(a3),aw_memory(a2)
	move.l	db_mlsysstruct(a3),aw_mlsysstruct(a2)
;	move.l	db_waitmasker(a3),aw_waitmask(a2)

	move.b	db_stay_on(a3),aw_stay_on(a2)
	clr.l	aw_fastback(a2)
	move.w	db_screen_depth(a3),aw_screendepth(a2)

	move.b	#0,aw_XORmode(a2)

	move.l	db_sig_ptoc(a3),aw_sig_ptoc(a2)
	move.l	db_msgpointer(a3),aw_msgpointer(a2)

	move.l	db_inactive_viewblok(a3),a5
	move.l	vb_bitmapw(a5),a0
;	move.b	bm_Depth(a0),d0
	move.l	a0,aw_bitmap_pscr1(a2)

	move.l	db_active_viewblok(a3),a5
	move.l	vb_bitmapw(a5),a0
;	move.b	bm_Depth(a0),d0
	move.l	a0,aw_bitmap_pscr2(a2)

	moveq	#0,d2
	moveq	#0,d3
	move.w	db_winpos_x(a3),d2
	move.w	db_winpos_y(a3),d3

	move.l	db_window_data(a3),a0
	cmp.l	#-1,winp_lbc(a0)
	beq	.no_rbc
	add.l	winp_bw(a0),d2
.no_rbc:
	cmp.l	#-1,winp_tbc(a0)
	beq	.no_tbc
	add.l	winp_bw(a0),d3

.no_tbc:
	move.l	clipa_x(a4),d0
	bmi	.okx
	add.w	d0,d2
	moveq	#0,d0
	bra	.okx2
.okx:
	neg.l	d0
.okx2:
	move.l	clipa_y(a4),d1
	bmi	.oky
	add.w	d1,d3
	moveq	#0,d1
	bra	.oky2
.oky:
	neg.l	d1
.oky2:

	move.l	clipa_width(a4),d4
	move.l	clipa_height(a4),d5
	movem.l	d0-d5,aw_bltcoords(a2)

	move.l	#MEMSIZE,aw_memsize(a2)
	clr.l	aw_packed(a2)
	clr.l	aw_head_frame_list(a2)
	move.l	(a4),a0
	move.l	a0,aw_filenaam(a2)

	move.l	clipa_loops(a4),aw_num_rot(a2)
	move.l	clipa_fps(a4),d1
	move.l	d1,aw_overallspeed(a2)
	move.l	d1,aw_frame_speed(a2)
	move.l	d1,aw_old_speed(a2)

	moveq	#0,d1
	move.l	clipa_disk(a4),d0
	btst	#0,d0
	beq	.nod
	moveq	#1,d1
.nod:
	move.b	d1,aw_diskanim(a2)
	moveq	#0,d1

	btst	#1,d0
	beq	.nob
	moveq	#1,d1
.nob:
	move.b	d1,aw_background(a2)

	move.b	#0,aw_show(a2)
	move.b	#0,aw_quit(a2)

	movem.l	a3/a5,-(a7)
	move.l	a2,a3

	move.l	aw_old_speed(a3),d1
	bsr	calc_speed

	bsr	read_whole_animfile
	bne	.anim_err
	tst.b	aw_diskanim(a3)
	bne	.do_diskanim

	bsr	find_FORMs

	bra	.anim_ready

.do_diskanim:

	bsr	find_first_FORM

.anim_ready:

	bsr	anim_setsam
	bsr	anim_install_50h

	move.b	#0,aw_show(a3)
	bsr	init_anim_bitmaps

	bsr	get_background

	move.b	#1,aw_show(a3)

	bsr	get_next_frame
	movem.l	(a7)+,a3/a5
	moveq	#0,d0
	rts

.anim_err:
	IFNE VIEWANIM
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"ANIM ERROR",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	movem.l	(a7)+,a3/a5
	moveq	#-1,d0
	rts

tbitmap:	ds.b	bm_SIZEOF

free_init_anim:
	bsr	anim_remove_50h
	bsr	anim_freesam
	lea	anim_struct(pc),a2
	movem.l	a3/a5,-(a7)
	move.l	a2,a3
	bsr	free_background
	bsr	free_anim
	movem.l	(a7)+,a3/a5
	rts

get_background:
	tst.l	aw_fastback(a3)
	beq	.nob
	movem.l	aw_bltcoords(a3),d0-d5
	exg	d0,d2
	exg	d1,d3
	add.l	#15,d4
	lsr.l	#4,d4
	lsl.l	#4,d4
	subq.l	#1,d4
	move.l	#$00ff,d7
	move.l	#$0c0,d6
	lea	tbitmap(pc),a1
	move.l	aw_bitmap_pscr1(a3),a0		; from screen to fastback
	move.w	#0,a2
	jsr	_BltBitMapFM
	rts
.nob:

	move.w	aw_planes(a3),d0		; check if there are more
	cmp.w	aw_screendepth(a3),d0		; planes in the screen
	bge	.no_hole			; you need to punch a hole

	IFNE VIEWANIM
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Punch hole",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	movem.l	aw_bltcoords(a3),d0-d5
	move.l	aw_bitmap_pscr1(a3),a0		; from screen to fastback
	move.l	aw_bitmap_pscr1(a3),a1		; to screen dummy
	move.l	#$00ff,d7
	move.l	#$000,d6
	move.w	#0,a2
	move.l	aw_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)
	jsr	_LVOBltBitMap(a6)

	movem.l	aw_bltcoords(a3),d0-d5
	move.l	aw_bitmap_pscr2(a3),a0		; from screen to fastback
	move.l	aw_bitmap_pscr2(a3),a1		; to screen dummy
	move.l	#$00ff,d7
	move.l	#$000,d6
	move.w	#0,a2
	move.l	aw_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)
	jsr	_LVOBltBitMap(a6)

.no_hole:
	rts

restore_background:
	tst.l	aw_fastback(a3)
	beq	.nob
	movem.l	aw_bltcoords(a3),d0-d5
	add.l	#15,d4
	lsr.l	#4,d4
	lsl.l	#4,d4
	move.l	#$00ff,d7
	move.l	#$0c0,d6
	lea	tbitmap(pc),a0
	move.l	aw_bitmap_pscr1(a3),a1		; from fastback to screen
	move.w	#0,a2
	jsr	_BltBitMapFM
.nob:
	rts

	
free_background:
	tst.l	aw_fastback(a3)
	beq	.nob
	move.l	aw_fastbacksize(a3),d0
	move.l	aw_fastback(a3),a1
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)
	clr.l	aw_fastback(a3)
.nob:
	rts
*
* Place next frame in bitmap
*
get_next_frame:
	move.l	a3,-(a7)
	lea	anim_struct(pc),a3

	bsr	create_next_frame

	tst.l	aw_num_rot(a3)
	beq	.last

	bsr	copy_bitmap

	bsr	switch_bitmap

	move.l	(a7)+,a3
	moveq	#0,d0
	rts

.last:	move.l	(a7)+,a3
	moveq	#-1,d0
	rts

*
* Copy the created frame to its destination
*
copy_bitmap:
	tst.b	aw_background(a3)
	beq	.nob
	bsr	restore_background

	bsr	punch_hole
.nob:
	movem.l	aw_bltcoords(a3),d0-d5
	move.l	aw_graphbase(a3),a6
	move.l	#$00ff,d7
	move.l	#$0e0,d6
	tst.b	aw_background(a3)
	bne	.nofast
	move.l	#$0c0,d6
.nofast:
	move.l	aw_bitmap_pa1(a3),a0
	move.l	aw_bitmap_pscr1(a3),a1
	move.w	#0,a2
	jsr	_LVOWaitBlit(a6)
	jsr	_LVOBltBitMap(a6)
	rts
*
* Cut out all the planes in the destination bitmap
*
punch_hole:
	movem.l	aw_bltcoords(a3),d0-d5
	move.l	aw_bitmap_pscr1(a3),a1
	move.l	bm_Planes+4(a1),d6
	sub.l	bm_Planes(a1),d6
	lsl.l	#3,d6
	move.l	d6,a2

	move.l	aw_bitmap_pa1(a3),a0
	move.l	bm_Planes+4(a0),d6
	sub.l	bm_Planes(a0),d6
	lsl.l	#3,d6
	move.l	d6,d7

	move.l	aw_graphbase(a3),a6
	move.w	aw_planes(a3),d6
.punch_plane2:
	movem.l	d0-d7/a1,-(a7)

	move.w	aw_screendepth(a3),d6
.punch_plane:
	movem.l	d0-d7/a1/a2,-(a7)
	move.l	aw_bitmap_pa1(a3),a0
	moveq	#$020,d6
	moveq	#$1,d7
	move.w	#0,a2
	jsr	_LVOBltBitMap(a6)
	movem.l	(a7)+,d0-d7/a1/a2
	add.l	a2,d2
	subq.w	#1,d6
	bne	.punch_plane
	movem.l	(a7)+,d0-d7/a1
	add.l	d7,d0
	subq.l	#1,d6
	bne	.punch_plane2
	rts
	
*
* Switch the two bitmap pointers
*
switch_bitmap:
	tst.b	aw_XORmode(a3)
	bne	.xor
	move.l	aw_bitmap_pa1(a3),d0
	move.l	aw_bitmap_pa2(a3),aw_bitmap_pa1(a3)
	move.l	d0,aw_bitmap_pa2(a3)
.xor:

	move.l	aw_bitmap_pscr1(a3),d0
	move.l	aw_bitmap_pscr2(a3),aw_bitmap_pscr1(a3)
	move.l	d0,aw_bitmap_pscr2(a3)

	rts

	IFNE MLMMU	
free_anim:
	tst.l	aw_packed(a3)
	beq	.nomem
	move.l	aw_mlmmubase(a3),a6
	move.l	aw_packed(a3),a1
	move.l	aw_packed_size(a3),d0
	jsr	_LVOMLMMU_FreeMem(a6)
	clr.l	aw_packed(a3)
.nomem:
	bsr	free_frame_list
	rts
	ELSE
free_anim:
	tst.l	aw_packed(a3)
	beq	.nomem
	move.l	$4.w,a6
	move.l	aw_packed(a3),a1
	move.l	aw_packed_size(a3),d0
	jsr	_LVOFreeMem(a6)
	clr.l	aw_packed(a3)
.nomem:
	bsr	free_frame_list
	rts
	ENDC	

*
* You need the width height and depth for the bitmaps
* If al is well it't in the first FORM
*
init_anim_bitmaps:
	tst.b	aw_diskanim(a3)
	beq	.no_disk
	move.l	aw_temp_frame_list(a3),a1
	bra	.from_disk
.no_disk:
	move.l	aw_head_frame_list(a3),a1
	cmp.l	#0,a1
	beq	.error

.from_disk:
	move.l	ani_form(a1),d0			; should be first form
	beq	.error				; with BMHD info

	move.l	a1,a4				; ani struct pointer
	move.l	d0,a1
	move.b	#$0,aw_bmhd_found(a3)

	move.l	4(a1),aw_temp_formsize(a3)	; store size
	addq.l	#8,a1
	move.l	a1,a2
	add.l	aw_temp_formsize(a3),a2	; end FORM pointer

	cmp.l	#'ILBM',(a1)+
	bne.w	.error
.next_chunk_FORM1:
	bsr	check_anim_chunks
	add.l	4(a1),a1
	move.l	a1,d0
	btst	#0,d0
	beq.b	.no_oneven2
	addq.l	#1,a1
.no_oneven2:
	addq.l	#8,a1
	cmp.l	a2,a1
	blt.b	.next_chunk_FORM1
.found_bmhd

;
; Now we know the width and the height
; We can initialize the bitmaps and other structs now
; if there is enough memory for the two buffers
;

	move.l	aw_breedte_x(a3),d0
	mulu	aw_planes(a3),d0
	mulu	aw_leny(a3),d0
	move.l	d0,d7
	move.l	d0,aw_bitmap_animsize(a3)
	add.l	d0,d0
	cmp.l	aw_memsize(a3),d0
	bgt	.error
	
	lea	aw_bitmapanim1(a3),a0
	move.l	a0,aw_bitmap_pa1(a3)
	move.l	aw_memory(a3),a1
	bsr	a_init_bitmap

	lea	aw_bitmapanim2(a3),a0
	move.l	a0,aw_bitmap_pa2(a3)
	move.l	aw_memory(a3),a1
	add.l	d7,a1

	bsr	a_init_bitmap


; ************************************* Back ground
	tst.b	aw_background(a3)
	beq	.noback

	move.l	aw_breedte_x(a3),d0
	addq.l	#2,d0
	move.l	d0,d6
	mulu	aw_screendepth(a3),d0
	mulu	aw_leny(a3),d0
	move.l	d0,aw_fastbacksize(a3)
	move.l	#MEMF_ANY,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,aw_fastback(a3)
	beq	.noback
	lea	tbitmap(pc),a0
	move.l	d0,a1

	move.w	aw_planes(a3),d7
	move.l	aw_breedte_x(a3),d5
	
	movem.l	d5/d7,-(a7)
	move.w	aw_screendepth(a3),aw_planes(a3)
	move.l	d6,aw_breedte_x(a3)
	bsr	a_init_bitmap
	movem.l	(a7)+,d5/d7

	move.w	d7,aw_planes(a3)
	move.l	d5,aw_breedte_x(a3)

.noback:

; ************************************* Back ground

	moveq	#0,d0
	rts
.error:
	moveq	#-1,d0
	rts
*
* Initialize bitmap with values in aw_breedte_x(a3) enz.
* a0 points to the bitmap
* a1 points to the chip mem
*
a_init_bitmap:
	movem.l		a0/a1,-(a7)
	move.w		aw_planes(a3),d0		; depth
	move.l		aw_breedte_x(a3),d1		; width in bytes * 8
	lsl.l		#3,d1				; width
	mulu		aw_planes(a3),d1		; interleaved
	move.w		aw_leny(a3),d2			; height
	move.l		aw_graphbase(a3),a6
	jsr		_LVOInitBitMap(a6)
	movem.l		(a7)+,a0/a1

	move.l		a1,d0
	move.l		aw_breedte_x(a3),d1
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
	rts

anim_struct:	blk.b	aw_SIZEOF	


MAX_MULTABEL = 580

ACUR = 8
*
* Calculate the speed so that speeds of non jiffy size are possible
*
calc_speed:
	movem.l	a0/d0/d1/d2,-(a7)
	moveq	#50,d0				; depends on pal or ntsc

	IFNE XAPP
	move.l	aw_mlsysstruct(a3),a0
	move.l	ml_miscFlags(a0),d2
	move.w	ml_refreshRate(a0),d0
	ENDC

	cmp.l	d0,d1
	ble	.ratebigger
	move.l	d0,d1
.ratebigger:
	tst.l	d1
	bne.b	oke_non_zero
	moveq	#25,d1
oke_non_zero:
	lsl.l	#ACUR,d0
	divu	d1,d0
	and.l	#$ffff,d0
	tst.l	d0
	bne	set_wait
	moveq	#1,d0
set_wait:
	lea	anim_teller(pc),a0
	move.l	d0,(a0)
	lea	anim_tellerstart(pc),a0
	move.l	d0,(a0)
	movem.l	(a7)+,d0/d1/a0/d2
	rts
*
* The anhd gives a number of jiffies to wait
*
set_jiffies:
	movem.l	a0/d0/d1/d2,-(a7)
	tst.l	d0
	bne.b	oke_jif
	moveq	#1,d0
oke_jif:
	lsl.l	#ACUR,d0
	bra.b	set_wait	

anim_setsam:
	lea	anim_sigmask(pc),a0
	tst.l	(a0)
	bne.w	anim_no_install

	move.l	$4,a6
	moveq	#-1,d0
	jsr	_LVOAllocSignal(a6)	

	cmp.l	#-1,d0
	beq.b	.setsam_error
	lea	anim_signum(pc),a0
	move.l	d0,(a0)
	move.l	d0,d1
	moveq	#$1,d0
	lsl.l	d1,d0	
	lea	anim_sigmask(pc),a0
	move.l	d0,aw_waitmask(a3)
	move.l	d0,(a0)
		
	sub.l	a1,a1
	move.l	$4,a6
	jsr	_LVOFindTask(a6)
	lea	anim_task(pc),a0
	move.l	d0,(a0)
	moveq	#0,d0
	rts

.setsam_error:
	rts

anim_freesam:
	move.l	anim_signum(pc),d0
	cmp.l	#-1,d0
	beq.b	.nofreesam
	move.l	$4,a6
	jsr	_LVOFreeSignal(a6)
.nofreesam:
	lea	anim_signum(pc),a0
	clr.l	(a0)
	lea	anim_sigmask(pc),a0
	clr.l	(a0)
	rts

anim_stuur_signaal:
	movem.l	d0/a1,-(a7)
	move.l	$4,a6
	move.l	off_task(a1),a1
	move.l	anim_sigmask(pc),d0
	jsr	_LVOSignal(a6)
	movem.l	(a7)+,d0/a1
	rts

anim_install_50h:
	lea	anim_handle_installed(pc),a0
	tst.b	(a0)
	bne.b	anim_no_install

	lea.l	anim_intstruct50h(pc),a1

	lea	anim_proc50hnaam(pc),a0
	move.l	a0,10(a1)
	lea	anim_teller(pc),a0
	move.l	a0,14(a1)
	lea	anim_proc_50h(pc),a0
	move.l	a0,18(a1)

	moveq	#5,d0
	move.l	$4,a6
	jsr	_LVOAddIntServer(a6)
	lea	anim_handle_installed(pc),a0
	move.b	#$ff,(a0)
anim_no_install:
	rts

anim_remove_50h:
	lea	anim_handle_installed(pc),a0
	tst.b	(a0)
	beq.b	.no_remove
	lea.l	anim_intstruct50h(pc),a1
	moveq	#5,d0
	move.l	$4,a6
	jsr	_LVORemIntServer(a6)
	lea	anim_handle_installed(pc),a0
	move.b	#$00,(a0)
.no_remove:
	rts

anim_proc_50h:
	move.l	a0,-(a7)
	moveq	#1,d0
	lsl.l	#ACUR,d0
	sub.l	d0,off_teller(a1)
	addq.l	#2,off_timer(a1)
	move.l	off_teller(a1),d1
	bmi	.send

	lsr.l	#ACUR,d1
	tst.l	d1
	bne.b	.no_send
.send:	
	bsr.w	anim_stuur_signaal
	move.l	off_tel_start(a1),d0
	add.l	d0,off_teller(a1)
.no_send:
	move.l	(a7)+,a0
	moveq	#0,d0
	rts

anim_proc50hnaam:	dc.b	"anim 50Hz interupt",0
	even
		
anim_intstruct50h:
	dc.l	0,0
	dc.b	2,15	; type en pri
	dc.l	0,0	; pointer naar naam en data
	dc.l	0

anim_teller:		dc.l	0
anim_tellerstart:	dc.l	0
anim_task:		dc.l	0
anim_signum:		dc.l	-1
anim_sigmask:		dc.l	0
anim_timer:		dc.l	0
anim_handle_installed:	dc.b	0,0

anim_set_base:
	movem.l	a3,-(a7)
	lea	anim_struct(pc),a3

	bsr	anim_own_wait
	movem.l	(a7)+,a3
	rts

anim_own_wait:
	movem.l	d0/d1/a0/a1,-(a7)

	move.l	aw_waitmask(a3),d0
	or.l	aw_sig_ptoc(a3),d0
.no_sigptoc:

	move.l	$4,a6
	jsr	_LVOWait(a6)

	movem.l	d0/d1,-(a7)
	move.l	aw_frame_speed(a3),d0
	cmp.l	aw_old_speed(a3),d0
	beq.b	.no_pro

	bsr.w	set_jiffies

	move.l	d0,aw_old_speed(a3)
.no_pro:
	movem.l	(a7)+,d0/d1

	movem.l	d0/d1/a0/a1,-(a7)
	move.l	$4,a6
	jsr	_LVOForbid(a6)
	movem.l	(a7)+,d0/d1/a0/a1

	move.l	d0,d1
	and.l	aw_sig_ptoc(a3),d1
	beq.b	.no_wacht1

; Er is een message van boven kijk wat zij doet en handel ernaar

.search_list:
	move.l	aw_msgpointer(a3),a0		;get msgport ptr
	lea.l	MP_MSGLIST(a0),a1		;make ptr to msglist
	move.l	LH_HEAD(a1),d1			;get first 	

.scan:	move.l	d1,a1
	move.l	LH_HEAD(a1),d1
	beq.b	.end_of_list

	move.l	pd_Cmd(a1),d2	; pd command
	cmp.l	#DCC_DORUN,d2
	beq	.doterm			; let de C-prog handle the reply
	cmp.l	#DCC_DOTERM,d2
	beq.b	.doterm			; let de C-prog handle the reply
	cmp.l	#DCC_DOSTOP,d2
	beq.b	.doterm

	bra.b	.scan

.doterm:
	movem.l	d0/d1/a0/a1,-(a7)
	sub.l	a1,a1
	move.l	$4,a6
	jsr	_LVOFindTask(a6)
	move.l	d0,a1
	move.l	aw_sig_ptoc(a3),d0
	jsr	_LVOSignal(a6)		; restore signal
	movem.l	(a7)+,d0/d1/a0/a1

	move.b	#$ff,aw_quit(a3)
	move.l	$4,a6
	jsr	_LVOPermit(a6)

	movem.l	(a7)+,d0/d1/a0/a1
	moveq	#-1,d0
	rts

.no_wacht1:
.end_of_list:
	move.l	$4,a6
	jsr	_LVOPermit(a6)
	movem.l	(a7)+,d0/d1/a0/a1
.no_wacht:
	IFEQ	XAPP
	btst	#6,$bfe001
	bne	.check_key
	moveq	#-1,d0
	move.b	#$ff,aw_quit(a3)
	rts
	ENDC
.check_key:
	move.b	$bfec01,d0
	not	d0
	ror.b	#1,d0
	cmp.b	#$60,d0
	bhi.b	.no_key_ch1
	bra	.wait_check_key

.no_key_ch1:
	btst	#10,$dff016
	beq	.wait_check_key	
	moveq	#0,d0
	rts

.wait_check_key:
	move.l	aw_dosbase(a3),a6
	movem.l	d1/a0/a1,-(a7)
	moveq	#1,d1
	jsr	_LVODelay(a6)		; give system time to check keys
	movem.l	(a7)+,d1/a1/a0
	moveq	#0,d0
	rts

*
* Free the total frame_list
*
free_frame_list:
	tst.l	aw_head_frame_list(a3)
	beq.b	exit_free_list
	move.l	aw_head_frame_list(a3),a1
rep_free_list
	cmp.l	#0,a1
	beq.b	exit_free_list
	move.l	ani_next(a1),a4		; get next pointer
	bsr.b	free_frame
	move.l	a4,a1
	bra.b	rep_free_list
exit_free_list:
	move.l	#0,aw_head_frame_list(a3)
	rts

alloc_frame:
	movem.l	d1/a0/a1,-(a7)
	move.l	$4,a6
	moveq	#ani_SIZEOF,d0
	move.l	#MEMF_PUBLIC+MEMF_CLEAR,d1
	jsr	_LVOAllocMem(a6)
	movem.l	(a7)+,d1/a0/a1
	rts
*
* Requires pointer in a1
*
free_frame:
	movem.l	d0/d1/a0/a1,-(a7)

	move.l	ani_size(a1),d0
	beq.b	no_size_free
	move.l	a1,-(a7)
	move.l	$4,a6
	move.l	ani_form(a1),a1
	jsr	_LVOFreeMem(a6)
	move.l	(a7)+,a1
no_size_free:
	move.l	$4,a6
	moveq	#ani_SIZEOF,d0

	jsr	_LVOFreeMem(a6)
	movem.l	(a7)+,d0/d1/a0/a1
	rts

*
* Search for all the FORMs in the file which is in the memory
*
find_FORMs:
	move.l	#0,aw_FORM_counter(a3)
	move.l	#"FORM",d2	
*
* Allocate head pointer
*
	bsr.w	alloc_frame

	move.l	d0,aw_head_frame_list(a3)
	beq.w	frame_alloc_fail

	move.l	d0,a0
	move.l	d0,a4

	move.l	aw_packed(a3),a1
	move.l	a1,a2
	addq.l	#8,a1
	addq.l	#4,a1			; skip the ILBM sign
	add.l	aw_packed_size(a3),a2	; end data
fza1:
	cmp.l	a2,a1
	bge.b	exitfind_f

	move.l	(a1),d3
	cmp.l	d2,d3
	bne.s	fza2

	addq.l	#1,aw_FORM_counter(a3)

	move.l	a1,ani_form(a0)		; store the pointer
	move.l	a0,a4			; remember previous pointer
	bsr.w	alloc_frame
	move.l	d0,ani_next(a0)
	beq.w	frame_alloc_fail
	move.l	d0,a0
fza2:
	move.l	4(a1),d0
	btst	#0,d0
	beq.b	fno_add		; check for uneven chunksize
	addq.l	#1,d0
	
fno_add:
				; next chunk
	add.l	d0,a1
	addq.l	#8,a1
	bra.s	fza1
exitfind_f:

	move.l	#0,ani_next(a4)	; clear last frame
	move.l	a0,a1
	bsr.w	free_frame

	move.l	aw_head_frame_list(a3),a0
	move.l	a0,aw_frame_pointer(a3)

	move.l	aw_FORM_counter(a3),aw_dpan_count(a3)

	subq.l	#2,aw_dpan_count(a3)		; the anim is looping so sub 2
no_loop4:
	rts
*
* Load the next FORM from disk
* the aw_filehandle(a3) is valid
*
load_next_form:
	tst.l	aw_temp_formpointer(a3)
	beq.b	no_free_poi

	move.l	aw_temp_formdisksize(a3),d0
	move.l	aw_temp_formpointer(a3),a1
	move.l	$4,a6
	jsr	_LVOFreeMem(a6)
	clr.l	aw_temp_formpointer(a3)
no_free_poi:
	lea	aw_buffer(a3),a0
	move.l	a0,d2
	moveq	#8,d3
	move.l	aw_filehandle(a3),d1
	move.l	aw_dosbase(a3),a6
	jsr	_LVORead(a6)
	cmp.l	#8,d0
	bne.b	file_fail
	lea	aw_buffer+4(a3),a0
	move.l	(a0),d0
	addq.l	#8,d0
	move.l	d0,aw_temp_formdisksize(a3)

	moveq	#MEMF_PUBLIC,d1
	move.l	$4,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,aw_temp_formpointer(a3)
	beq.b	file_mem_fail
	move.l	d0,a0
	lea	aw_buffer(a3),a1
	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+

	move.l	d0,d2
	move.l	aw_temp_formdisksize(a3),d3
	subq.l	#8,d3
	addq.l	#8,d2
	move.l	aw_dosbase(a3),a6
	move.l	aw_filehandle(a3),d1
	jsr	_LVORead(a6)
	move.l	aw_temp_formdisksize(a3),d3
	subq.l	#8,d3
	cmp.l	d3,d0
	bne.b	file_fail	
	move.l	aw_temp_formpointer(a3),d0
	rts
file_fail:
	moveq	#0,d0
	rts

file_mem_fail:
	moveq	#1,d7
	bra.w	animexit

skip_two_forms:
	moveq	#1,d7
rr_skip:
	lea	aw_buffer(a3),a0
	move.l	a0,d2
	moveq	#8,d3
	move.l	aw_filehandle(a3),d1
	move.l	aw_dosbase(a3),a6
	jsr	_LVORead(a6)
	cmp.l	#8,d0
	bne.w	file_fail
	lea	aw_buffer+4(a3),a0

	move.l	(a0),d2
	move.l	aw_filehandle(a3),d1
	moveq	#0,d3					; start
	move.l	aw_dosbase(a3),a6
	jsr	_LVOSeek(a6)

	lea	aw_buffer+4(a3),a0
	move.l	(a0),d0
	addq.l	#8,d0
	add.l	d0,aw_global_diskoffset(a3)
;	dbf	d7,rr_skip
	rts

*
* Search for all the FORMs in the file which is on disk
*
find_disk_FORMs:

	tst.l	aw_temp_frame_list(a3)		; the first time ?
	beq.b	no_head_list1

	move.b	#1,aw_show(a3)

	move.l	aw_temp_frame_list(a3),a0
	move.l	a0,aw_head_frame_list(a3)	; set temp list, first
	clr.l	aw_temp_frame_list(a3)

	move.l	aw_head_frame_list(a3),d0
	move.l	d0,a0
	move.l	d0,a4

	move.l	aw_packed(a3),a1
	lea.l	12(a1),a1
	bra.b	from_c1

no_head_list1:
	move.l	aw_frame_pointer(a3),d0

	move.l	d0,a0
	move.l	d0,a4

	move.l	a0,-(a7)
	bsr.w	load_next_form
	move.l	(a7)+,a0
	addq.l	#1,aw_frame_counter(a3)
	addq.l	#1,aw_FORM_counter(a3)

	tst.l	d0
	beq.w	exitfind_dd
	move.l	d0,a1

from_c1:
	move.l	4(a1),d7
	movem.l	a0/d7,-(a7)
	move.l	a0,a4			; remember previous pointer

	bsr.w	create_next_frame1

	tst.b	aw_show(a3)
	bne.b	oke_show5
	moveq	#2,d7
	cmp.l	aw_frame_counter(a3),d7
	bne.b	oke_show5
	move.b	#1,aw_show(a3)
oke_show5:
	movem.l	(a7)+,a0/d7
	addq.l	#8,d7
	add.l	d7,aw_global_diskoffset(a3)
	move.l	#0,ani_form(a4)

	move.l	a0,a4			; remember previous pointer
	bsr.w	alloc_frame
	move.l	d0,ani_next(a0)
	beq.w	frame_alloc_fail
	move.l	d0,aw_frame_pointer(a3)
	move.l	d0,a0

	tst.b	aw_quit(a3)
	bne.b	exitfind_d

	cmp.l	#1,aw_num_rot(a3)
	bne.b	no_pro4

	tst.l	aw_dpan_count(a3)
	beq.b	no_pro4

poi:
	move.l	aw_frame_counter(a3),d7
	move.b	#1,aw_quit(a3)

	cmp.l	aw_dpan_count(a3),d7
	beq.b	exitfind_dd

	move.b	#0,aw_quit(a3)
no_pro4:
	rts

exitfind_d:
	move.l	#0,ani_next(a4)	; clear last frame
	move.l	a0,a1
	bsr.w	free_frame

exitfind_dd:
	subq.l	#1,aw_num_rot(a3)
	beq	no_pro4
	bpl	.cont
	move.l	#-1,aw_num_rot(a3)
.cont:
	move.l	aw_head_frame_list(a3),a0
	cmp.l	#0,a0
	beq	.minframes

	move.l	ani_next(a0),a0

	cmp.l	#0,a0
	beq	.minframes

	tst.b	aw_XORmode(a3)
	bne	.xor

	move.l	ani_next(a0),a0		; skip first two frames ??

.xor:
	move.l	#2,aw_frame_counter(a3)
.minframes:
	move.l	a0,aw_frame_pointer(a3)
	subq.l	#1,aw_FORM_counter(a3)

	move.l	aw_filehandle(a3),d1
	move.l	#12,d2
	moveq	#-1,d3					; beginning
	move.l	aw_dosbase(a3),a6
	jsr	_LVOSeek(a6)

	cmp.l	#2,aw_dpan_count(a3)
	ble.b	no_loop5

	bsr	skip_two_forms

	move.l	aw_FORM_counter(a3),aw_dpan_count(a3)
	subq.l	#2,aw_dpan_count(a3)		; the anim is looping so sub 2
no_loop5:
	bra	no_head_list1
	
*
* Load the first FORM from disk
*
find_first_FORM:
	move.l	#0,aw_FORM_counter(a3)
	move.l	#12,aw_global_diskoffset(a3)

	move.b	#$ff,aw_diskloaded(a3)
	move.b	#$ff,aw_show(a3)
	
	bsr.w	alloc_frame
	move.l	d0,aw_temp_frame_list(a3)
	beq.b	frame_alloc_fail
	move.l	d0,a0
	move.l	d0,a4

	move.l	a0,-(a7)
	bsr.w	load_next_form
	move.l	(a7)+,a0
	move.l	d0,aw_packed(a3)
	sub.l	#12,aw_packed(a3)

	tst.l	d0
	beq.b	exitfind_d2

	move.l	d0,ani_form(a4)
	move.l	d0,a1
	move.l	4(a1),d7

	move.l	#1,aw_frame_counter(a3)
	move.l	#1,aw_FORM_counter(a3)

exitfind_d2:
	move.b	#$0,aw_diskloaded(a3)
	rts

frame_alloc_fail:
	moveq	#1,d7
	bra.w	animexit

*
* Get next FORM pointer from the internal array
* returns the pointer in a1
*
get_next_FORM:
	move.l	aw_frame_pointer(a3),a0
	addq.l	#1,aw_frame_counter(a3)

	move.l	a0,a4
	move.l	ani_form(a0),a1
	move.l	ani_next(a0),a0
	cmp.l	#0,a0
	bne.b	ok_getFORM

	move.l	#0,aw_frame_counter(a3)
	move.l	aw_head_frame_list(a3),a0
	subq.l	#1,aw_num_rot(a3)	; one time round

	move.l	ani_next(a0),a0
	tst.b	aw_XORmode(a3)
	bne	.xor
	move.l	ani_next(a0),a0		; skip first two frames ??
.xor:
	move.l	#2,aw_frame_counter(a3)
ok_getFORM:
	move.l	a0,aw_frame_pointer(a3)
	rts
*
* a1 points to the chunk
* a4 points to the anim_list struct
*
check_anim_chunks:
	move.l	(a1),d0
	cmp.l	#'DLTA',d0
	bne.b	no_dlta_anim_found
	bsr.w	dlta_anim_found
	bra.b	continue_check_FORM
no_dlta_anim_found:
	cmp.l	#'ANHD',d0
	bne.b	no_anhd_anim_found
	bsr.w	anhd_anim_found
	bra.b	continue_check_FORM
no_anhd_anim_found:
	cmp.l	#'BMHD',d0
	bne.b	no_bmhd_anim_found
	bsr.w	bmhd_anim_found
	bra.b	continue_check_FORM
no_bmhd_anim_found:
	cmp.l	#'CAMG',d0
	bne.b	no_camg_anim_found
	bsr.w	camg_anim_found
	bra.b	continue_check_FORM
no_camg_anim_found:
	cmp.l	#'BODY',d0
	bne.b	no_body_anim_found
	bsr.w	body_anim_found
	bra.b	continue_check_FORM
no_body_anim_found:
	cmp.l	#'DPAN',d0
	bne.b	no_dpan_anim_found
	moveq	#0,d0
	move.w	10(a1),d0
	move.l	d0,aw_dpan_count(a3)
	bra.w	continue_check_FORM
no_dpan_anim_found:

continue_check_FORM:
	rts

create_next_frame:
	tst.b	aw_diskanim(a3)
	beq	.no_disk
	bsr	find_disk_FORMs
	rts
	
.no_disk:
	move.b	#0,aw_bmhd_found(a3)

	bsr.w	get_next_FORM		; retrieve FORM pointer	

	tst.b	ani_type(a4)
	bne.w	already_known_form

create_next_frame1:

	move.b	#$0,aw_bmhd_found(a3)

	move.l	4(a1),aw_temp_formsize(a3)	; store size
	addq.l	#8,a1
	move.l	a1,a2
	add.l	aw_temp_formsize(a3),a2	; end FORM pointer

	cmp.l	#'ILBM',(a1)+
	bne.w	no_ilbm1

next_chunk_FORM1:

	bsr.w	check_anim_chunks

	add.l	4(a1),a1
	move.l	a1,d0
	btst	#0,d0
	beq.w	.no_oneven2
	addq.l	#1,a1
.no_oneven2:
	addq.l	#8,a1
	cmp.l	a2,a1
	blt.b	next_chunk_FORM1
show_new_frame:
no_ilbm1:
	rts

already_known_form:
	move.l	ani_form(a4),d7
	bne.b	no_disk1

	bsr.w	get_chunk_from_disk

	move.l	ani_form(a4),d7
no_disk1:
	move.l	d7,a1
	move.b	ani_type(a4),d7
	and.b	#BMHD_type,d7
	beq.w	no_bmhd2

	move.w	#0,aw_mode(a3)
	move.b	#$ff,aw_bmhd_found(a3)

	move.w	ani_lenx(a4),aw_lenx(a3)
	moveq	#0,d0
	move.w	aw_lenx(a3),d0
	add.l	#15,d0
	lsr.l	#4,d0
	add.l	d0,d0
	move.l	d0,aw_breedte_x(a3)

	move.w	ani_planes(a4),aw_planes(a3)
	move.b	ani_compression(a4),aw_compression(a3)
	move.b	ani_masking(a4),aw_masking(a3)

	move.w	aw_planes(a3),d6
	move.l	aw_breedte_x(a3),d0
	mulu	d6,d0

	cmp.l	aw_rowmulti(a3),d0
	bne.b	abmhd_multable
	move.w	ani_leny(a4),d0
	cmp.w	aw_leny(a3),d0
	beq.b	abmhd_no_multable

abmhd_multable:
	move.w	ani_leny(a4),aw_leny(a3)
	moveq	#0,d1
	move.w	aw_leny(a3),d1

	move.l	d0,aw_rowmulti(a3)
* create a mul-table for the anim play
	move.l	d0,d1

	lea	aw_multabel(a3),a0
	move.w	aw_leny(a3),d6
	cmp.w	#MAX_MULTABEL,d6
	ble.b	no_mpro3
	move.w	#MAX_MULTABEL,d6
no_mpro3:
	move.l	#0,(a0)+
	subq.w	#1,d6
abmhd_rep_make_mult:
	move.l	d0,(a0)+
	add.l	d1,d0
	dbf	d6,abmhd_rep_make_mult
abmhd_no_multable:
	move.b	#$ff,aw_bmhd_changed(a3)		; copy to other view later on
no_bmhd2:
	move.b	ani_type(a4),d7
	and.b	#ANHD_type,d7
	beq.b	no_anhd2

	move.l	ani_reltime(a4),aw_frame_speed(a3)
	move.b	ani_interleave(a4),aw_ainterleaved(a3)
	move.b	ani_bits(a4),aw_XORmode(a3)
no_anhd2:
	move.b	ani_type(a4),d7
	and.b	#DLTA_type,d7
	beq.b	no_dlta2
	bsr.w	dlta_anim_found
no_dlta2:
	move.b	ani_type(a4),d7
	and.b	#BODY_type,d7
	beq.b	no_body2
	bsr.w	body_anim_found
no_body2:
	move.b	ani_type(a4),d7
	and.b	#CAMG_type,d7
	beq.b	no_camg2
	move.b	ani_camghires(a4),aw_hires(a3)
	move.b	ani_camglace(a4),aw_interlace(a3)
	move.w	ani_camgmode(a4),aw_mode(a3)
no_camg2:
	bsr.b	free_disk_frame
	rts

*
* Give the memory used for this frame free
* There is the posibility to do this in a special way
*
free_disk_frame:
	move.l	ani_size(a4),d0
	beq.b	no_free_disk_frame
	move.l	ani_form(a4),a1
	move.l	$4,a6
	jsr	_LVOFreeMem(a6)
	move.l	#0,ani_size(a4)
	move.l	#0,ani_form(a4)
no_free_disk_frame:
	rts
	
*
* a4 points to the frame
*
get_chunk_from_disk:
	move.l	ani_chunksize(a4),d0
	moveq	#MEMF_PUBLIC,d1
	move.l	$4,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,ani_form(a4)
	beq.w	frame_alloc_fail
	move.l	ani_chunksize(a4),ani_size(a4)		; for freemem later on

	move.l	aw_filehandle(a3),d1
	move.l	ani_chunkpos(a4),d2
	sub.l	aw_relfpointer(a3),d2
	add.l	d2,aw_relfpointer(a3)
	moveq	#0,d3					; current
	move.l	aw_dosbase(a3),a6
	jsr	_LVOSeek(a6)

	move.l	ani_form(a4),d2
	move.l	ani_chunksize(a4),d3
	add.l	d3,aw_relfpointer(a3)
	move.l	aw_filehandle(a3),d1
	jsr	_LVORead(a6)
	cmp.l	ani_chunksize(a4),d0
	bne.w	frame_alloc_fail

	rts

*
*
*	
dlta_anim_found:
	movem.l	d0-d7/a0-a6,-(a7)

	tst.b	aw_bmhd_changed(a3)
				; in this case the data from the active must
				; be copied to the hidden ( the first 2 frames)
	beq.b	no_bmhd_ch2			

	tst.b	aw_show(a3)	; Show this frame ?
	beq.b	bmhd_ch3

	IFNE ANIM32
	cmp.b	#$64,aw_acompression(a3)
	beq.b	anim322
	cmp.b	#$65,aw_acompression(a3)
	bne.w	no_anim322
anim322:
;	bsr.w	convert_int_non
no_anim322:
	ENDC
	
	bsr.w	copy_active_hidden

	move.b	#0,aw_bmhd_changed(a3)
	bra.b	bmhd_ch3
no_bmhd_ch2:
	cmp.b	#1,aw_ainterleaved(a3)
	bne.w	bmhd_ch3

;	bsr.w	actual_copy

bmhd_ch3:
	tst.b	aw_show(a3)
	beq.b	no_show3

	cmp.b	#5,aw_acompression(a3)
	bne.b	no_anim5
	addq.l	#8,a1
	bsr.w	de_animate
	bra.b	no_show3
no_anim5:
	IFNE ANIM32
	cmp.b	#$64,aw_acompression(a3)
	bne.b	no_anim32
	bsr.w	anim32
	bra.b	no_show3
no_anim32:
	cmp.b	#$65,aw_acompression(a3)
	bne.b	no_anim16
	bsr.w	anim16
	bra.b	no_show3

no_anim16:
	ENDC

	cmp.b	#7,aw_acompression(a3)
	bne.b	no_anim7

	addq.l	#8,a1

	move.b	aw_bits(a3),d0
	btst	#0,d0
	beq.b	no_anim6
			
	bsr.b	anim7
	bra.b	no_show3

no_anim6:

	bsr.w	anim6
	bra.b	no_show3
no_anim7:

	cmp.b	#8,aw_acompression(a3)
	bne.b	no_anim8

	addq.l	#8,a1

	move.b	aw_bits(a3),d0
	btst	#0,d0
	beq.b	no_anim8_long
			
	bsr.w	anim8
	bra.b	no_show3

no_anim8_long:

	bsr.w	anim8_word

no_anim8:


no_show3:
	movem.l	(a7)+,d0-d7/a0-a6
	tst.l	ani_chunkpos(a4)
	bne.b	no_store3
	move.l	a1,ani_chunkpos(a4)
	move.l	a1,ani_form(a4)
	move.l	aw_temp_formpointer(a3),d7
	sub.l	d7,ani_chunkpos(a4)
	move.l	aw_global_diskoffset(a3),d7
	add.l	d7,ani_chunkpos(a4)
	move.l	ani_chunkpos(a4),d1
	
	move.l	4(a1),ani_chunksize(a4)
	addq.l	#8,ani_chunksize(a4)
	or.b	#DLTA_type,ani_type(a4)
no_store3:
	rts

anim7:
	move.l	aw_breedte_x(a3),d2
	lsr.l	#2,d2			; long words

	move.l	a1,a5			; pointer naar DLTA data 

	move.l	aw_rowmulti(a3),d0
	
;	move.l	aw_frame_hidden(a3),a6

	move.l	aw_bitmap_pa1(a3),a6
	move.l	bm_Planes(a6),a6

	lea	aw_multabel(a3),a4
	move.w	aw_planes(a3),d7
	subq.w	#1,d7	
	move.l	a3,-(a7)
rep_anim_planes7:

	move.l	(a1),d6
	beq.b	no_dlta_data7
	lea	0(a5,d6.l),a0		; a0 points to commands

	move.l	32(a1),d6
	lea	0(a5,d6.l),a3		; a3 points to data
	addq.l	#4,a1
	
	move.l	d2,d6
	subq.w	#1,d6
	move.l	a6,-(a7)
rep_anim_kolom7:
	moveq	#0,d5
	move.b	(a0)+,d5
	beq.b	opcountzero7

	move.l	a6,a2

	subq.w	#1,d5
rep_opcounts7:
	move.b	(a0)+,d4
	beq.b	same_ops7
	bpl.b	skip_ops7

uniq_ops7:
	and.w	#$7f,d4

rrr7:	move.l	(a3)+,(a6)
	adda.l	d0,a6	
	subq.w	#1,d4
	bne.b	rrr7
	dbf	d5,rep_opcounts7
cont_more_opcounts7:

	move.l	a2,a6

opcountzero7:			; opcount zero dus volgende kolom
	addq.l	#4,a6
	dbf	d6,rep_anim_kolom7

	move.l	(a7)+,a6

no_dlta_data7:
	add.l	d2,a6
	add.l	d2,a6
	add.l	d2,a6
	add.l	d2,a6
	dbf	d7,rep_anim_planes7
	move.l	(a7)+,a3
	rts

same_ops7:
	moveq	#0,d4
	move.b	(a0)+,d4
	move.l	(a3)+,d1	; de data die gezet moet worden

rrr720:	move.l	d1,(a6)
	adda.l	d0,a6
	subq.w	#1,d4
	bne.b	rrr720
	dbf	d5,rep_opcounts7
	bra.b	cont_more_opcounts7

skip_ops7:

	and.w	#$7f,d4
	add.w	d4,d4
	add.w	d4,d4
	add.l	0(a4,d4.w),a6
	dbf	d5,rep_opcounts7
	bra.b	cont_more_opcounts7

anim6:
	move.l	aw_breedte_x(a3),d2
	lsr.l	#1,d2			; words

	move.l	a1,a5			; pointer naar DLTA data 

	move.l	aw_rowmulti(a3),d0
	
;	move.l	aw_frame_hidden(a3),a6
	move.l	aw_bitmap_pa1(a3),a6
	move.l	bm_Planes(a6),a6

	lea	aw_multabel(a3),a4
	move.w	aw_planes(a3),d7
	subq.w	#1,d7	
	move.l	a3,-(a7)
rep_anim_planes6:

	move.l	(a1),d6
	beq.b	no_dlta_data7
	lea	0(a5,d6.l),a0		; a0 points to commands

	move.l	32(a1),d6
	lea	0(a5,d6.l),a3		; a3 points to data
	addq.l	#4,a1
	
	move.l	d2,d6
	subq.w	#1,d6
	move.l	a6,-(a7)
rep_anim_kolom6:
	moveq	#0,d5
	move.b	(a0)+,d5
	beq.b	opcountzero6

	move.l	a6,a2

	subq.w	#1,d5
rep_opcounts6:
	move.b	(a0)+,d4
	beq.b	same_ops6
	bpl.b	skip_ops6

uniq_ops6:
	and.w	#$7f,d4

rrr6:	move.w	(a3)+,(a6)
	adda.l	d0,a6	
	subq.w	#1,d4
	bne.b	rrr6
	dbf	d5,rep_opcounts6
cont_more_opcounts6:

	move.l	a2,a6

opcountzero6:			; opcount zero dus volgende kolom
	addq.l	#2,a6
	dbf	d6,rep_anim_kolom6

	move.l	(a7)+,a6

no_dlta_data6:
	add.l	d2,a6		; volgende plane
	add.l	d2,a6
	dbf	d7,rep_anim_planes6
	move.l	(a7)+,a3
	rts

same_ops6:
	moveq	#0,d4
	move.b	(a0)+,d4
	move.w	(a3)+,d1	; de data die gezet moet worden

rrr620:	move.w	d1,(a6)
	adda.l	d0,a6
	subq.w	#1,d4
	bne.b	rrr620
	dbf	d5,rep_opcounts6
	bra.b	cont_more_opcounts6

skip_ops6:
	and.w	#$7f,d4
	add.w	d4,d4
	add.w	d4,d4
	add.l	0(a4,d4.w),a6
	dbf	d5,rep_opcounts6
	bra.b	cont_more_opcounts6

	IFNE ANIM32

init_anim32_16:
	btst	#4,aw_bits(a3)
	beq.b	horizontal
	btst	#6,aw_bits(a3)
	beq.b	no_skip_line
	move.l	aw_breedte_x(a3),d3
	add.l	d3,d3
	move.b	#$ff,aw_lize(a3)
	bra.b	ex_init

no_skip_line:
	move.l	aw_breedte_x(a3),d3
	bra.b	ex_init
horizontal:
	moveq	#4,d3
ex_init:
	rts

*
* a1 points to the delta data
*
anim32:
	movem.l	d0-d7/a0-a6,-(a7)
	addq.l	#8,a1
	bsr.b	init_anim32_16
;	move.l	aw_frame_hidden(a3),a6		; plane pointer

	move.l	aw_bitmap_pa1(a3),a6
	move.l	bm_Planes(a6),a6

	move.l	aw_breedte_x(a3),d2		; width plane
	mulu	aw_leny(a3),d2

	move.w	aw_planes(a3),d7
	subq.w	#1,d7
	move.l	a1,a5

rep_planes_a32:
	move.l	(a1)+,d6
	beq.b	no_dlta_data32

	lea	0(a5,d6.l),a0			; plane delta data

	move.l	(a0)+,d6			; no of commands
	subq.l	#1,d6

rep_command_a32:
	move.l	(a0)+,d5	
	move.l	(a0)+,d0
	lea	0(a6,d0.l),a4			; in plane pointer
	tst.l	d5
	bpl.b	same_ops_a32

skip_ops_a32:
	neg.l	d5
	subq.l	#1,d5
rep_skip_ops_a32:
	move.l	(a0)+,(a4)
	add.l	d3,a4
	dbf	d5,rep_skip_ops_a32
	dbf	d6,rep_command_a32
	add.l	d2,a6				; next plane
	dbf	d7,rep_planes_a32
	bra.b	exit_a32

same_ops_a32:
	move.l	(a0)+,d1
	subq.l	#1,d5
rep_same_ops_a32:
	move.l	d1,(a4)
	add.l	d3,a4

	dbf	d5,rep_same_ops_a32
	dbf	d6,rep_command_a32

no_dlta_data32:
	add.l	d2,a6				; next plane
	dbf	d7,rep_planes_a32
exit_a32:
	movem.l	(a7)+,d0-d7/a0-a6
	rts

anim16:		
	movem.l	d0-d7/a0-a6,-(a7)
	addq.l	#8,a1
	bsr.w	init_anim32_16
;	move.l	aw_frame_hidden(a3),a6		; plane pointer
	move.l	aw_bitmap_pa1(a3),a6
	move.l	bm_Planes(a6),a6

	move.l	aw_breedte_x(a3),d2		; width plane
	mulu	aw_leny(a3),d2

	move.w	aw_planes(a3),d7
	subq.w	#1,d7
	move.l	a1,a5

rep_planes_a16:
	move.l	(a1)+,d6
	beq.b	no_dlta_data16

	lea	0(a5,d6.l),a0			; plane delta data

	move.w	(a0)+,d6			; no of commands
	subq.l	#1,d6

rep_command_a16:
	move.w	(a0)+,d5	
	move.l	(a0)+,d0
	lea	0(a6,d0.l),a4			; in plane pointer
	tst.w	d5
	bpl.b	same_ops_a16

skip_ops_a16:
	neg.w	d5
	subq.w	#1,d5
rep_skip_ops_a16:
	move.w	(a0)+,(a4)
	add.l	d3,a4
	dbf	d5,rep_skip_ops_a16
	dbf	d6,rep_command_a16
	add.l	d2,a6				; next plane
	dbf	d7,rep_planes_a16
	bra.b	exit_a16

same_ops_a16:
	move.w	(a0)+,d1
	subq.w	#1,d5
rep_same_ops_a16:
	move.w	d1,(a4)
	add.l	d3,a4
	dbf	d5,rep_same_ops_a16
	dbf	d6,rep_command_a16

no_dlta_data16:
	add.l	d2,a6				; next plane
	dbf	d7,rep_planes_a16
exit_a16:
	movem.l	(a7)+,d0-d7/a0-a6
	rts

	ENDC

body_anim_found:
	movem.l	d0/a1/a2/a4,-(a7)

	tst.b	aw_diskloaded(a3)
	bne.w	no_body

	or.b	#BODY_type,ani_type(a4)

	tst.b	aw_show(a3)
	beq.b	no_show2

	cmp.b	#1,aw_compression(a3)
	beq.s	anim_un1

; compression 0 or unknown do a simple copy

;	move.l	aw_frame_hidden(a3),a2
	move.l	aw_bitmap_pa1(a3),a2
	move.l	bm_Planes(a2),a2

	move.l	4(a1),d0
	lsl.l	#2,d0
	subq.l	#1,d0
	addq.l	#8,a1
anim_no_co:
	move.l	(a1)+,(a2)+
	dbf	d0,anim_no_co
	bra.s	anim_un_end

anim_un1:

;	move.l	aw_frame_hidden(a3),a2
	move.l	aw_bitmap_pa1(a3),a2
	move.l	bm_Planes(a2),a2

	lea	8(a1),a4
	add.l	4(a1),a4
	lea	8(a1),a0
anim_un_again:	
	cmp.l	a4,a0
	bhi.s	anim_un_end

	moveq	#0,d5
	move.b	(a0)+,d5
	bmi.b	anim_un_minus

anim_un_plu:
	move.b	(a0)+,(a2)+
	dbf	d5,anim_un_plu
	bra.b	anim_un_again

anim_un_minus:
	neg.b	d5
	move.b	(a0)+,d0
anim_un_rm:
	move.b	d0,(a2)+
	dbf	d5,anim_un_rm
	bra.b	anim_un_again
anim_un_end:	
no_show2:
	movem.l	(a7)+,d0/a1/a2/a4
	tst.l	ani_chunkpos(a4)
	bne.b	no_store4
	move.l	a1,ani_chunkpos(a4)
	move.l	a1,ani_form(a4)
	move.l	aw_temp_formpointer(a3),d7
	sub.l	d7,ani_chunkpos(a4)
	move.l	aw_global_diskoffset(a3),d7
	add.l	d7,ani_chunkpos(a4)
	move.l	ani_chunkpos(a4),d1

	move.l	4(a1),ani_chunksize(a4)
	addq.l	#8,ani_chunksize(a4)
no_store4:
;	bsr	convert
	rts
no_body:
	movem.l	(a7)+,d0/a1/a2/a4
	rts

camg_anim_found:
	move.l	d0,-(a7)

	move.b	#$ff,aw_bmhd_changed(a3)	; change the viewports accordingly
	move.b	#$ff,aw_bmhd_found(a3)
	move.b	ani_type(a4),d7
	and.b	#CAMG_type,d7
	bne.b	easy_camg

	moveq	#0,d2
	move.w	10(a1),d0

	or.w	d0,d2			; straight in the mode-id
	and.w	#$8efd,d2		; oldstyle mask
	move.w	d0,d1
	and.w	#$8000,d0
	beq.w	.cno_hires
	move.b	#$ff,aw_hires(a3)
	or.w	#$8000,d2
.cno_hires:
	move.w	d1,d0
	and.w	#$4,d0
	beq.w	.cno_lace
	move.b	#$ff,aw_interlace(a3)
	or.w	#$4,d2
.cno_lace:
	move.w	d1,d0
	and.w	#$800,d0
	beq.b	.cno_ham
	or.w	#$800,d2
.cno_ham:
	move.w	d1,d0
	and.w	#$80,d0	
	beq.b	.cno_half
	and.w	#$f7ff,d2	; clear the ham bit
.cno_half:
	move.w	d2,aw_mode(a3)
	move.b	aw_hires(a3),ani_camghires(a4)

	move.b	aw_interlace(a3),ani_camglace(a4)
	move.w	d2,ani_camgmode(a4)
	or.b	#CAMG_type,ani_type(a4)
	move.l	(a7)+,d0
	rts

easy_camg:

	move.b	ani_camghires(a4),aw_hires(a3)
	move.b	ani_camglace(a4),aw_interlace(a3)
	move.w	ani_camgmode(a4),aw_mode(a3)
	move.l	(a7)+,d0
	rts

anhd_anim_found:
	movem.l	d0/a1/a4,-(a7)
	addq.l	#8,a1
	move.b	$17(a1),aw_bits(a3)

	
	move.b	anhd_operation(a1),aw_acompression(a3)
	move.l	anhd_reltime(a1),d0
	tst.l	aw_overallspeed(a3)
	bne.b	no_reltime

	move.l	d0,aw_frame_speed(a3)

no_reltime:	
	move.l	anhd_bits(a1),d0
	move.l	d0,d1
	and.l	#$3f,d1
	cmp.l	d0,d1
	bne.b	no_xor_mode1		; carbage found in ANHD
	btst	#1,d0
	beq.b	no_xor_mode1
	move.b	#$ff,aw_XORmode(a3)
no_xor_mode1:
	move.b	anhd_interleave(a1),d0
	move.b	d0,d1
	and.b	#1,d1
	cmp.b	d0,d1
	bne.b	carbage_found1
	move.b	d0,aw_ainterleaved(a3)
carbage_found1:
	movem.l	(a7)+,d0/a1/a4
	move.b	ani_type(a4),d7
	and.b	#BMHD_type,d7
	bne.b	no_store_struct2
	move.b	ani_type(a4),d7
	and.b	#ANHD_type,d7
	bne.b	no_store_struct2
	move.l	aw_frame_speed(a3),ani_reltime(a4)
	move.b	aw_ainterleaved(a3),ani_interleave(a4)
	move.b	aw_XORmode(a3),ani_bits(a4)
	or.b	#ANHD_type,ani_type(a4)
no_store_struct2:
	rts
*
* In the anim is a BMHD chunk
* initialize the variables with this new info
*
bmhd_anim_found:
	movem.l	d0/a1/a4,-(a7)
	move.b	#$ff,aw_bmhd_found(a3)
	move.w	#0,aw_mode(a3)
	addq.l	#8,a1
	move.w	(a1),aw_lenx(a3)

	moveq	#0,d0
	move.w	aw_lenx(a3),d0
	add.l	#15,d0
	lsr.l	#4,d0
	add.l	d0,d0
	move.l	d0,aw_breedte_x(a3)

	moveq	#0,d1
	move.b	8(a1),d1
	move.w	d1,aw_planes(a3)
	move.b	10(a1),aw_compression(a3)
;	move.b	aw_compression(a3),d0
	move.b	9(a1),aw_masking(a3)

	move.w	aw_planes(a3),d7
	move.l	aw_breedte_x(a3),d0
	mulu	d7,d0

	cmp.l	aw_rowmulti(a3),d0
	bne.b	bmhd_multable
	move.w	2(a1),d0
	cmp.w	aw_leny(a3),d0
	beq.b	bmhd_no_multable

bmhd_multable:
	move.w	2(a1),aw_leny(a3)
	moveq	#0,d1
	move.w	aw_leny(a3),d1

	move.l	d0,aw_rowmulti(a3)
* create a mul-table for the anim play
	move.l	d0,d1

	lea	aw_multabel(a3),a0
	move.w	aw_leny(a3),d7
	cmp.w	#MAX_MULTABEL,d7
	ble.b	no_mpro5
	move.l	#MAX_MULTABEL,d7
no_mpro5:
	move.l	#0,(a0)+
	subq.w	#1,d7
bmhd_rep_make_mult:
	move.l	d0,(a0)+
	add.l	d1,d0
	dbf	d7,bmhd_rep_make_mult
bmhd_no_multable:
	move.b	#$ff,aw_bmhd_changed(a3)		; copy to other view later on
	movem.l	(a7)+,a1/d0/a4
	tst.b	ani_type(a4)
	bne.b	no_store_struct1
	move.w	aw_lenx(a3),ani_lenx(a4)
	move.w	aw_leny(a3),ani_leny(a4)
	move.w	aw_planes(a3),ani_planes(a4)
	move.b	aw_compression(a3),ani_compression(a4)
;	move.b	aw_masking(a3),ani_masking(a4)
	or.b	#BMHD_type,ani_type(a4)
no_store_struct1:
	rts

*
* 
* de_animate needs a pointer de de DLTA data ( DLTA pointer + 8 )
* it also uses a mul-table 
de_animate:
	tst.b	aw_XORmode(a3)
	bne.w	de_animate_xormode

de_animate_cpu20:
	move.l	aw_breedte_x(a3),d2
	move.l	a1,a5			; pointer naar DLTA data 

	move.l	aw_rowmulti(a3),d0
	
;	move.l	aw_frame_hidden(a3),a6
	move.l	aw_bitmap_pa1(a3),a6
	move.l	bm_Planes(a6),a6

	lea	aw_multabel(a3),a4

	move.w	aw_planes(a3),d7
	subq.w	#1,d7	
rep_anim_planes20:
	move.l	(a1)+,d6
	beq.b	no_dlta_data20
	lea	0(a5,d6.l),a0		; a0 wijst nu naar de plane-data

	move.l	d2,d6
	subq.w	#1,d6
	move.l	a6,-(a7)
rep_anim_kolom20:
	moveq	#0,d5
	move.b	(a0)+,d5
	beq.b	opcountzero20

	move.l	a6,a2

	subq.w	#1,d5
rep_opcounts20:
	move.b	(a0)+,d4
	beq.b	same_ops20
	bpl.b	skip_ops20

uniq_ops20:
	and.w	#$7f,d4

rrr20:	move.b	(a0)+,(a6)
	adda.l	d0,a6	
	subq.w	#1,d4
	bne.b	rrr20
	dbf	d5,rep_opcounts20
cont_more_opcounts20:

	move.l	a2,a6

opcountzero20:			; opcount zero dus volgende kolom
	addq.l	#1,a6
	dbf	d6,rep_anim_kolom20

	move.l	(a7)+,a6

no_dlta_data20:
	add.l	d2,a6		; volgende plane
	dbf	d7,rep_anim_planes20
	rts

same_ops20:
	moveq	#0,d4
	move.b	(a0)+,d4
	move.b	(a0)+,d1	; de data die gezet moet worden

rrr220:	move.b	d1,(a6)
	adda.l	d0,a6
	subq.w	#1,d4
	bne.b	rrr220
	dbf	d5,rep_opcounts20
	bra.b	cont_more_opcounts20

skip_ops20:

	and.w	#$7f,d4

	add.w	d4,d4
	add.w	d4,d4
	add.l	0(a4,d4.w),a6

	dbf	d5,rep_opcounts20
	bra.b	cont_more_opcounts20

******************
*
* anim 8 decrunch assume a 68020 ( no 20 code used however )
* the long mode
*
anim8:
	move.l	aw_breedte_x(a3),d2
	lsr.l	#2,d2			; long words
	move.l	a1,a5			; pointer naar DLTA data 

	move.l	aw_rowmulti(a3),d0
	
;	move.l	aw_frame_hidden(a3),a6
	move.l	aw_bitmap_pa1(a3),a6
	move.l	bm_Planes(a6),a6

	lea	aw_multabel(a3),a4

	move.w	aw_planes(a3),d7
	subq.w	#1,d7	
rep_anim_planes8:
	move.l	(a1)+,d6
	beq.b	no_dlta_data8
	lea	0(a5,d6.l),a0		; a0 wijst nu naar de plane-data

	move.l	d2,d6
	subq.w	#1,d6
	move.l	a6,-(a7)
rep_anim_kolom8:
	move.l	(a0)+,d5
	beq.b	opcountzero8

	move.l	a6,a2

	subq.l	#1,d5
rep_opcounts8:
	move.l	(a0)+,d4
	beq.b	same_ops8
	bpl.b	skip_ops8

uniq_ops8:
	and.l	#$7fffffff,d4

rrr8:	move.l	(a0)+,(a6)
	adda.l	d0,a6	
	subq.l	#1,d4
	bne.b	rrr8
	dbf	d5,rep_opcounts8
cont_more_opcounts8:

	move.l	a2,a6

opcountzero8:			; opcount zero dus volgende kolom
	addq.l	#4,a6
	dbf	d6,rep_anim_kolom8

	move.l	(a7)+,a6

no_dlta_data8:
	add.l	aw_breedte_x(a3),a6		; volgende plane
	dbf	d7,rep_anim_planes8
	rts

same_ops8:
	move.l	(a0)+,d4
	move.l	(a0)+,d1	; de data die gezet moet worden

rrr80:	move.l	d1,(a6)
	adda.l	d0,a6
	subq.l	#1,d4
	bne.b	rrr80
	dbf	d5,rep_opcounts8
	bra.b	cont_more_opcounts8

skip_ops8:
	and.l	#$7fffffff,d4
	add.l	d4,d4
	add.l	d4,d4
	add.l	0(a4,d4.l),a6
	dbf	d5,rep_opcounts8
	bra.b	cont_more_opcounts8

*
* The anim8 word version
*
anim8_word:
	move.l	aw_breedte_x(a3),d2
	lsr.l	#1,d2			; long words
	move.l	a1,a5			; pointer naar DLTA data 

	move.l	aw_rowmulti(a3),d0
	
;	move.l	aw_frame_hidden(a3),a6
	move.l	aw_bitmap_pa1(a3),a6
	move.l	bm_Planes(a6),a6
	lea	aw_multabel(a3),a4

	move.w	aw_planes(a3),d7
	subq.w	#1,d7	
rep_anim_planes8w:
	move.l	(a1)+,d6
	beq.b	no_dlta_data8w
	lea	0(a5,d6.l),a0		; a0 wijst nu naar de plane-data

	move.l	d2,d6
	subq.w	#1,d6
	move.l	a6,-(a7)
rep_anim_kolom8w:
	move.w	(a0)+,d5
	beq.b	opcountzero8w

	move.l	a6,a2

	subq.w	#1,d5
rep_opcounts8w:
	move.w	(a0)+,d4
	beq.b	same_ops8w
	bpl.b	skip_ops8w

uniq_ops8w:
	and.w	#$7fff,d4

rrr8w:	move.w	(a0)+,(a6)
	adda.l	d0,a6	
	subq.w	#1,d4
	bne.b	rrr8w
	dbf	d5,rep_opcounts8w
cont_more_opcounts8w:

	move.l	a2,a6

opcountzero8w:			; opcount zero dus volgende kolom
	addq.l	#2,a6
	dbf	d6,rep_anim_kolom8w

	move.l	(a7)+,a6

no_dlta_data8w:
	add.l	aw_breedte_x(a3),a6		; volgende plane
	dbf	d7,rep_anim_planes8w
	rts

same_ops8w:
	move.w	(a0)+,d4
	move.w	(a0)+,d1	; de data die gezet moet worden

rrr80w:	move.w	d1,(a6)
	adda.l	d0,a6
	subq.w	#1,d4
	bne.b	rrr80w
	dbf	d5,rep_opcounts8w
	bra.b	cont_more_opcounts8w

skip_ops8w:

	and.w	#$7fff,d4

	add.w	d4,d4
	add.w	d4,d4
	add.l	0(a4,d4.w),a6

	dbf	d5,rep_opcounts8w
	bra.b	cont_more_opcounts8w
	rts

***************************************
*
* de_animate needs a pointer de de DLTA data ( DLTA pointer + 8 )
* it also uses a mul-table 
* use the xor mode
de_animate_xormode:
	move.l	aw_breedte_x(a3),d2
	move.l	a1,a5			; pointer naar DLTA data 

	move.l	aw_rowmulti(a3),d0
	
;	move.l	aw_frame_hidden(a3),a6
	move.l	aw_bitmap_pa1(a3),a6
	move.l	bm_Planes(a6),a6

	move.w	aw_planes(a3),d7
	subq.w	#1,d7	
rep_anim_planesx:
	move.l	(a1)+,d6
	beq.b	no_dlta_datax
	lea	0(a5,d6.l),a0		; a0 wijst nu naar de plane-data

	move.l	d2,d6
	subq.w	#1,d6
	move.l	a6,-(a7)
rep_anim_kolomx:
	moveq	#0,d5
	move.b	(a0)+,d5
	beq.b	opcountzerox

	move.l	a6,a2

	subq.w	#1,d5
rep_opcountsx:
	move.b	(a0)+,d4
	beq.b	same_opsx
	bpl.b	skip_opsx

uniq_opsx:
	and.w	#$7f,d4

	subq.w	#1,d4
rrrx:	move.b	(a0)+,d3
	eor.b	d3,(a6)
	adda.l	d0,a6	
	dbf	d4,rrrx
	dbf	d5,rep_opcountsx
cont_more_opcountsx:

	move.l	a2,a6

opcountzerox:			; opcount zero dus volgende kolom
	addq.l	#1,a6
	dbf	d6,rep_anim_kolomx

	move.l	(a7)+,a6

no_dlta_datax:
	add.l	d2,a6		; volgende plane
	dbf	d7,rep_anim_planesx
	rts

same_opsx:
	moveq	#0,d4
	move.b	(a0)+,d4
	move.b	(a0)+,d1	; de data die gezet moet worden

	move.w	d4,d3
	asr.w	#3,d3
	andi.w	#7,d4
	add.w	d4,d4
	add.w	d4,d4
	neg.w	d4
	jmp	no_sam2x(pc,d4.w)	
same_copy2x:
	eor.b	d1,(a6)
	adda.w	d0,a6
	eor.b	d1,(a6)
	adda.w	d0,a6
	eor.b	d1,(a6)
	adda.w	d0,a6
	eor.b	d1,(a6)
	adda.w	d0,a6
	eor.b	d1,(a6)
	adda.w	d0,a6
	eor.b	d1,(a6)
	adda.w	d0,a6
	eor.b	d1,(a6)
	adda.w	d0,a6
	eor.b	d1,(a6)
	adda.w	d0,a6
no_sam2x:
	dbra	d3,same_copy2x
	dbf	d5,rep_opcountsx
	bra.b	cont_more_opcountsx
skip_opsx:
	and.w	#$7f,d4

	lea	aw_multabel(a3),a4
	add.w	d4,d4
	add.w	d4,d4
	add.l	0(a4,d4.w),a6
	dbf	d5,rep_opcountsx
	bra.b	cont_more_opcountsx

*
* Copy the data from the shown bitmap to the hidden bitmap
*
copy_active_hidden:
	movem.l	a0/a1/d7,-(a7)
	move.l	aw_bitmap_pa2(a3),a0		; Source
	move.l	aw_bitmap_pa1(a3),a1		; Destination
	move.l	bm_Planes(a0),a0
	move.l	bm_Planes(a1),a1
	move.l	aw_bitmap_animsize(a3),d7
	lsr.l	#2,d7
.rep:	move.l	(a0)+,(a1)+
	subq.l	#1,d7
	bne	.rep
	movem.l	(a7)+,a0/a1/d7
	rts

	IFNE MLMMU
read_whole_animfile:
	move.l	aw_mlmmubase(a3),a6
	move.l	aw_filenaam(a3),a1		; name of the file mnmmulib
	jsr	_LVOMLMMU_FindMemBlk(a6)
	move.l	d0,aw_packed(a3)
	bne.w	.file_loaded2

	moveq	#ERR_FILE_NOT_FOUND,d7
	move.l	aw_filenaam(a3),d1
	move.l	#mode_old,d2
	move.l	aw_dosbase(a3),a6
	jsr	_LVOOpen(a6)
	move.l	d0,aw_filehandle(a3)
	beq.w	animexit
	move.l	d0,d1
	lea.l	aw_buffer(a3),a2
	move.l	a2,d2
	moveq	#12,d3
	move.l	aw_dosbase(a3),a6
	jsr	_LVORead(a6)
	lea.l	aw_buffer(a3),a1
	moveq	#ERR_UNKNOWN_FILE_TYPE,d7
	cmp.l	#'FORM',(a1)+
	bne.w	animexit
	tst.b	aw_diskanim(a3)
	beq.b	.no_diskanim1
	moveq	#0,d0
	rts

.no_diskanim1:
	move.l	(a1),d0
	addq.l	#8,d0
	move.l	d0,aw_packed_size(a3)

	moveq	#ERR_NO_FMEM,d7

	moveq	#0,d1
	tst.b	aw_stay_on(a3)
	beq	.nostay
	or.l	#MEMF_STAY,d1
.nostay:

	move.l	aw_mlmmubase(a3),a6
	move.l	aw_filenaam(a3),a1		; naam van de file mlmmulib
	jsr	_LVOMLMMU_AllocMem(a6)

	move.l	d0,aw_packed(a3)
	beq.w	exit_read_whole

	move.l	d0,a0
.check_load:

* test of de file al in het geheugen zit

	move.l	aw_packed(a3),a1
	jsr	_LVOMLMMU_GetMemStat(a6)	; haal status van memblock op

	and.l	#MTF_INIT,d0
	bne.w	.file_loaded

	move.l	aw_packed(a3),a1
	jsr	_LVOMLMMU_OwnMemBlk(a6)
	tst.l	d0
	beq.b	.check_load

	move.l	aw_packed_size(a3),d3
	move.l	aw_packed(a3),d2
	sub.l	#12,d3
	add.l	#12,d0
	move.l	d0,d2
	move.l	d3,d7				; number off bytes to read
	move.l	aw_filehandle(a3),d1
	move.l	aw_dosbase(a3),a6
	jsr	_LVORead(a6)		
	cmp.l	d7,d0				; whole file ?
	bne.w	file_cropped
	move.l	aw_packed(a3),a0
	lea	aw_buffer(a3),a1
	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+

	move.l	aw_mlmmubase(a3),a6
	move.l	aw_packed(a3),a1
	jsr	_LVOMLMMU_DisOwnMemBlk(a6)

	move.l	aw_packed(a3),a1
	moveq	#MTF_INIT,d0
	or.l	#MTF_SETCLR,d0
	jsr	_LVOMLMMU_SetMemStat(a6)	; file nu wel geladen

* hele file nu in het geheugen 

.file_loaded:
	bsr.w	anim_close_file
	clr.b	aw_diskanim(a3)
	move.l	aw_packed(a3),a0
	move.l	4(a0),d0
	addq.l	#8,d0
	move.l	d0,aw_packed_size(a3)
	moveq	#0,d0
	rts

.file_loaded2:
	move.l	aw_packed(a3),a0
	move.l	4(a0),d0
	addq.l	#8,d0

	moveq	#0,d1
	tst.b	aw_stay_on(a3)
	beq	.nostay2
	or.l	#MEMF_STAY,d1
.nostay2:
	move.l	aw_mlmmubase(a3),a6
	move.l	aw_filenaam(a3),a1		; naam van de file mlmmulib
	jsr	_LVOMLMMU_AllocMem(a6)
	move.l	d0,aw_packed(a3)
	bra	.file_loaded

file_cropped:
	bsr.b	anim_close_file
	moveq	#ERR_UNKNOWN_FILE_TYPE,d7
	bra.w	animexit

	ELSE

read_whole_animfile:
	move.l	#3,d7
	move.l	aw_filenaam(a3),d1
	move.l	#mode_old,d2
	move.l	aw_dosbase(a3),a6
	jsr	_LVOOpen(a6)
	move.l	d0,aw_filehandle(a3)
	beq.w	animexit
	move.l	d0,d1
	lea	aw_buffer(a3),a0
	move.l	a0,d2
	move.l	#12,d3
	move.l	aw_dosbase(a3),a6
	jsr	_LVORead(a6)
	lea.l	aw_buffer(a3),a1
	move.l	#1,d7
	cmp.l	#'FORM',(a1)+
	bne.w	animexit

	tst.b	aw_diskanim(a3)
	beq.b	.no_diskanim1
	moveq	#0,d0
	rts

.no_diskanim1:
	move.l	(a1),d0
	add.l	#8,d0
	move.l	d0,aw_packed_size(a3)
	move.l	#2,d7
	move.l	#$10000,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,aw_packed(a3)
	beq.b	exit_read_whole

	move.l	aw_packed_size(a3),d3
	sub.l	#12,d3
	add.l	#12,d0
	move.l	d0,d2
	move.l	aw_filehandle(a3),d1
	move.l	aw_dosbase(a3),a6
	jsr	_LVORead(a6)		

	move.l	aw_packed(a3),a0
	lea	aw_buffer(a3),a1
	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+

* hele file nu in het geheugen 

	bsr.w	anim_close_file
	moveq	#0,d0
	rts

	ENDC				; MLMMU

*
* Can't read the whole file in at once or I don't want to
* Use diskanim instead
*
exit_read_whole:
	move.b	#1,aw_diskanim(a3)
	moveq	#0,d0
	rts

anim_close_file:
	move.l	aw_dosbase(a3),a6
	move.l	aw_filehandle(a3),d1
	beq.b	no_closefile
	jsr	_LVOClose(a6)
	clr.l	aw_filehandle(a3)
no_closefile:
	rts

animexit:
	tst.l	aw_temp_formpointer(a3)
	beq.b	no_free_poi3
	move.l	aw_temp_formdisksize(a3),d0
	move.l	aw_temp_formpointer(a3),a1
	move.l	$4,a6
	jsr	_LVOFreeMem(a6)
no_free_poi3:

	IFNE XAPP
	tst.w	d7
	beq	no_err1
	tst.l	aw_mlmmubase(a3)
	beq	no_err1

	move.l	aw_filenaam(a3),a0
	cmp.l	#0,a0
	bne	.oke_error
	lea	unk(pc),a0
.oke_error:
	move.w	d7,d0
	subq.l	#1,d0
	and.w	#$7,d0

	lea.l	foutmelding(pc),a1
	lsl.l	#2,d0
	add.l	d0,a1
	lea.l	foutmelding(pc),a2
	add.l	(a1),a2
	lea.l	aw_multabel(a3),a1
	cmp.l	#0,a1
	beq	.zero_addr1

.rep_cerr:
	move.b	(a2)+,(a1)+
	tst.b	(a2)
	bne	.rep_cerr
.rep_cerr2:
	move.b	(a0)+,(a1)+
	tst.b	(a0)
	bne	.rep_cerr2
	move.b	#0,(a1)+
	move.l	aw_mlmmubase(a3),a6
	lea.l	aw_multabel(a3),a0
	moveq	#0,d0
	jsr	_LVOMLMMU_AddMsgToQueue(a6)
.zero_addr1:

no_err1:
	moveq	#-1,d0
	ENDC
	rts

animerror:	dc.l	0
;unk:	dc.b	"unknown",0
	even
	IF 0
animfoutmelding:
 dc.l	afout1-animfoutmelding,afout2-animfoutmelding,afout3-animfoutmelding
 dc.l	afout4-animfoutmelding,afout5-animfoutmelding,afout6-animfoutmelding
 dc.l	afout7-animfoutmelding

afout1:	dc.b	"File not found ",0
afout2:	dc.b	"Not enough chip memory ",0
afout3:	dc.b	"Not enough fast memory ",0
afout4:	dc.b	"Resolution not supported ",0
afout5:	dc.b	"General error ",10,0
afout6:	dc.b	"Unknown file type ",10,0
afout7:	dc.b	"Mangled IFF file ",10,0	
	even
	ENDC

; filenaam, effect speed, ltimes , wachttijd, wipenr, showtijd
;
; effect speed geeft vertikaal aan hoeveel rijen tegelijk ( 1 2 4 8 )
; ltimes geeft aan hoeveel keer tegelijk horizontaal een regel geblit wordt

W = 0
WIPE = 0
EP = 16
V = 4
VA = 1

; filenaam, effect speed, ltimes , wachttijd, wipenr, showtijd
; filenaam, effect speed, thicknes , variation, x,y,wipenr, showtijd
;
; effect speed geeft vertikaal aan hoeveel rijen tegelijk ( 1 2 4 8 )
; ltimes geeft aan hoeveel keer tegelijk horizontaal een regel geblit wordt

slide_it:
	IFEQ XAPP
	dc.l	file1,EP,V,VA,0, 0,0,W
	dc.l	file2,EP,V,VA,0, 0,WIPE,W
	dc.l	file1,EP,V,VA,0, 0,WIPE,W
	dc.l	0

;file1:	dc.b	"hd3:data/pictures/klad/cycle1",0
;file2:	dc.b	"hd3:data/pictures/klad/cross",0
file1:	dc.b	"pics:8bit/cindy11",0
;file1:	dc.b	"work:mediapoint/graphics/maps/ireland",0
file2:	dc.b	"work:mediapoint/graphics/maps/italy",0

	even
	INCDIR	"wp:asm/"
	INCLUDE "eff_nums.s"
	ENDC

	include "wp:asm/bltfastmem.s"

	even
	INCDIR	"wp:asm/"
	INCLUDE	"blokdata1.s"

mt:	ds.l	580			; allocmem for this ????????????????

	END

