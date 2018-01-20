*
* File	: easy_up.s
* Uses	: cb_data.i credit_struct.i
* Date	: 5 march 1994 ( nov 1993 credit scroll )
* Author: ing. C. Lieshout
* Desc	: Create a easy vertical limited scroll

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

	INCDIR	"wp:inclibs/"
	INCLUDE "graphics_libv39.i"
	INCLUDE	"dos_lib.i"
	INCLUDE	"diskfont_lib.i"
	INCLUDE	"intuition_lib.i"
	INCLUDE	"mpmmu_lib.i"

;	INCDIR	"aprogs:graph/easy_up/"
	INCDIR	"wp:asm/"
	INCLUDE	"proces.i"
	INCLUDE	"system.i"

	incdir	"wp:easy_up/"
	incdir	"vscroll:asm/"
	INCLUDE	"cb_data.i"
	INCLUDE	"credit_struct.i"

;MON_ID = DBLPAL_MONITOR_ID
MON_ID = PAL_MONITOR_ID

;MON_ID = $39004

DEBUG = 0
XAPP = 0

; Default values when there is no viewport

DEF_XSIZE = 640
DEF_YSIZE = 566
DEF_MODE = V_HIRES!V_LACE
DEF_PLANES = 2

LIBV39 = 39
LIBV40 = 40

	IFNE	DEBUG
	XREF	KPutFmt
	XREF	KPutStr
	PRINTT	"De debug info staat aan dus je moet nog met debug.lib linken"
	ENDC


MEMSIZE = 227000

	IFEQ XAPP
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

start:
	lea	datablock(pc),a3
	lea	filename(pc),a0
	move.l	a0,cb_filenamepointer(a3)

instart:
	bsr	open_libs
	bne	lib_err

	clr.l	cb_mlsysstruct(a3)
	clr.l	cb_sig_ptoc(a3)

	bsr	misc_init

	move.l	#MON_ID,cb_monid(a3)
	bsr	setprefs
	
	bsr	read_file
	bsr	calc_size_bitmap

	bsr	get_chipmem
	beq	chipmem_err

	bsr.w	get_args			; from crawl rec

;	bsr.w	do_scroll

chipmem_err:

	bsr	release_filemem

	bsr.w	close_all

lib_err:
	bsr	free_screenmem
err:
	rts
	ENDC

	XDEF	_preview
*
* prview( monitorID, datasegment, filename, quitsignal )
*
_preview:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)

	move.l	12(a5),a3			; data segment pointer
	move.l	8(a5),cb_monid(a3)		; monitor ID
	move.l	16(a5),cb_filenamepointer(a3)
	move.l	20(a5),cb_sig_quit(a3)

	clr.l	cb_mlsysstruct(a3)
	clr.l	cb_sig_ptoc(a3)

	bsr	open_libs
	bne	.lib_err

	bsr	misc_init

	bsr	setprefs
	
	bsr	read_file
	bsr	calc_size_bitmap

	bsr	get_chipmem
	beq	.chipmem_err

	bsr.w	get_args			; from crawl rec

	bsr.w	do_scroll

.chipmem_err:

	bsr	release_filemem

	bsr.w	close_all

.lib_err:
	bsr	free_screenmem

	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

free_screenmem:
	tst.l	cb_screenmem(a3)
	beq	.nomem
	move.l	cb_screenmem(a3),a1
	move.l	cb_memsize(a3),d0
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)
.nomem:
	rts
	
misc_init:

	clr.b	cb_views_init(a3)
	clr.l	cb_fontpointer(a3)
	move.w	#DEF_XSIZE,cb_lenviewx(a3)
	move.w	#DEF_YSIZE,cb_lenviewy(a3)
	move.l	#320,cb_middle(a3)
	move.l	#4,cb_middle_gap(a3)
	move.l	#0,cb_intergap(a3)
	move.b	#$0f,cb_red(a3)
	move.b	#$00,cb_green(a3)
	move.b	#$0f,cb_blue(a3)
	move.b	#$0f,cb_bred(a3)
	move.b	#$0f,cb_bgreen(a3)
	move.b	#$0f,cb_bblue(a3)
	move.b	#$00,cb_sred(a3)
	move.b	#$00,cb_sgreen(a3)
	move.b	#$00,cb_sblue(a3)
	move.w	#2,cb_allign(a3)
;	IFEQ	XAPP
	lea	temp_tags(pc),a0
	move.l	a0,cb_tags(a3)
;	ENDC
	move.w	#4,cb_shadow_depth(a3)
	move.w	#1,cb_shadow_type(a3)
	rts
	
	IF	XAPP

	XDEF	_get_varsize
_get_varsize:
	move.l	#cb_SIZEOF+1000,d0
	rts
	
	XDEF	_pass_mlsystem
*
* haal de info uit de mlsystem struct
*
_pass_mlsystem:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)

	move.l	12(a5),a3
	move.l	8(a5),a0			; pointer naar mlsystem
	move.l	a0,cb_mlsysstruct(a3)

	bsr	misc_init
	move.l	cb_mlsysstruct(a3),a0
	move.l	ml_taglist(a0),cb_tags(a3)
	move.l	ml_monitor_ID(a0),cb_monid(a3)

	move.l	16(a5),a1
	move.l	a1,cb_msgpointer(a3)
	moveq	#0,d0
	move.b	MP_SIGBIT(a1),d0
	moveq	#1,d1
	lsl.l	d0,d1
	move.l	d1,cb_sig_ptoc(a3)

	bsr.w	open_libs

	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

	XDEF	_load_easy_up
	
_load_easy_up:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	12(a5),a3

	tst.b	cb_loaded(a3)		; file loaded ??
	bne.w	already_loaded

	move.l	8(a5),a0		; filename
;	lea	filename(pc),a0

	move.l	a0,cb_filenamepointer(a3)

	IFNE	DEBUG
	movem.l	d0-d7/a0-a6,-(a7)
	lea	.dat(pc),a1
	move.l	a0,(a1)
	lea	.dbstr(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr:	dc.b	"Loading file [%s]",10,0
	even
.dat:	dc.l	0
 .tt:
	movem.l	(a7)+,d0-d7/a0-a6
	ENDC

	bsr	get_args
	bsr	read_file
	tst.w	d0
	bne	load_err

	bsr	calc_size_bitmap
	bsr	calcfinal_size

	bsr	setprefs

	move.b	#$ff,cb_loaded(a3)	; file loaded
already_loaded:
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0			; succes
	rts

load_err:
	lea	load_err_msg(pc),a0
	bsr	place_error
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#-1,d0			; fail
	rts

place_error:
	move.l	cb_filenamepointer(a3),a2
	lea	cb_fontname_buffer(a3),a1
.rname:
	move.b	(a2)+,(a1)+
	tst.b	(a2)
	bne	.rname
	move.b	#' ',(a1)+
;	move.b	#':',(a1)+
;	move.b	#' ',(a1)+
.cpperr:
	move.b	(a0)+,(a1)+
	tst.b	(a0)
	bne	.cpperr
	move.b	#0,(a1)+
	moveq	#0,d0
	lea	cb_fontname_buffer(a3),a0
	move.l	cb_mpmmubase(a3),a6
	jsr	_LVOMLMMU_AddMsgToQueue(a6)
	rts
	
	XDEF	_do_easy_up
_do_easy_up:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)

	move.l	8(a5),a3

	tst.b	cb_loaded(a3)
	beq.w	not_loaded

	move.l	cb_mlsysstruct(a3),a0
	move.l	sbu_Base(a0),cb_screenmem(a3)
	move.l	sbu_Size(a0),d0
	add.l	d0,d0
	move.l	d0,cb_screensize(a3)
	move.l	cb_memsize(a3),d1

	cmp.l	d0,d1
	bge	err_tobig

	bsr	do_scroll

	move.l	cb_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)

not_loaded:
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

err_tobig:
	lea	file_to_big_msg(pc),a0
	bsr	place_error
	bra	not_loaded
	
	XDEF	_unload_easy_up
_unload_easy_up:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a3
	bsr	release_filemem

	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

	XDEF _release_easy_up
_release_easy_up:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a3

	bsr	close_all

	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

	ENDC

*
* Retrieve the overscan settings
*

get_oscan_vals:
	IFNE	DEBUG
	movem.l	d0-d7/a0-a6,-(a7)
	lea	cb_monid(a3),a1
	lea	.dbstr(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr:	dc.b	"1 - monID = %lx",10,0
	even
 .tt:
	movem.l	(a7)+,d0-d7/a0-a6
	ENDC

	move.l	cb_intbase(a3),a6
	move.l	cb_monid(a3),d0
	move.l	d0,d1
	and.l	#MONITOR_ID_MASK,d1
	cmp.l	#SUPER72_MONITOR_ID,d1
	bne	.no_mul
	or.l	#SUPER_KEY,d0
	or.l	#SUPER_KEY,cb_monid(a3)
.no_mul:
	cmp.l	#VGA_MONITOR_ID,d1
	bne	.no_mul2
	or.l	#SUPER_KEY,d0
	or.l	#SUPER_KEY,cb_monid(a3)
.no_mul2:
;	and.l	#MONITOR_ID_MASK,d0
	or.l	#HIRES_KEY,d0

	IFNE	DEBUG
	movem.l	d0-d7/a0-a6,-(a7)
	lea	cb_monid(a3),a1
	lea	.dbstr2(pc),a0
	jsr	KPutFmt
	bra	.tt2
.dbstr2:	dc.b	"2 - monID = %lx",10,0
	even
 .tt2:
	movem.l	(a7)+,d0-d7/a0-a6
	ENDC


	move.l	d0,a0
	move.l	#1,d0				//OSCAN_MAX
	lea	cb_viewblock1(a3),a1
	jsr	_LVOQueryOverscan(a6)
	lea	cb_viewblock1(a3),a1

	move.w	ra_MaxX(a1),d0
	move.w	ra_MaxY(a1),d1
	addq.w	#1,d0
	lsr.w	#1,d0
	lsr.w	#1,d1
	add.w	d0,d0
	add.w	d1,d1
	move.w	d0,cb_columns(a3)
	move.l	cb_monid(a3),d2
	and.w	#V_LACE,d2
	beq	.nol
	lsr.w	#1,d1
.nol:
	move.w	d1,cb_rows(a3)

	move.l	cb_monid(a3),d0
	or.l	#HIRES_KEY,d0
	move.l	d0,a0
	move.l	#4,d0				//OSCAN_MAX
	lea	cb_viewblock1(a3),a1
	jsr	_LVOQueryOverscan(a6)
	lea	cb_viewblock1(a3),a1

	move.w	ra_MaxY(a1),d0
	sub.w	ra_MinY(a1),d0
	add.w	d0,d0
	move.l	cb_monid(a3),d2
	and.w	#V_LACE,d2
	beq	.nol2
	lsr.w	#1,d0
.nol2:
	move.w	d0,cb_max_y(a3)

	move.w	ra_MaxX(a1),d0
	sub.w	ra_MinX(a1),d0

	add.l	#63,d0
	lsr.l	#6,d0
	lsl.l	#6,d0
	move.w	d0,cb_max_x(a3)

	move.w	#0,a0
	lea	cb_viewblock1(a3),a1
	moveq	#mtr_SIZEOF,d0
	move.l	#DTAG_MNTR,d1

	move.l	cb_monid(a3),d2

;	and.l	#MONITOR_ID_MASK,d2

	move.l	cb_graphbase(a3),a6
	jsr	_LVOGetDisplayInfoData(a6)
	lea	cb_viewblock1(a3),a1

	move.l	mtr_Mspc(a1),d0

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
	tst.l	cb_mlsysstruct(a3)
	beq	.nolace
	
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
	move.l	d0,d1
	and.l	#VGA_MONITOR_ID,d1
	cmp.l	#VGA_MONITOR_ID,d1
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
	bsr	calcfinal_size
	move.l	cb_memsize(a3),d0
	move.l	d0,cb_screensize(a3)
	move.l	#MEMF_CHIP!MEMF_CLEAR,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)		; chipmem for the viewport
	move.l	d0,cb_screenmem(a3)

	IFNE	DEBUG
	movem.l	d0-d7/a0-a6,-(a7)
	lea	cb_screenmem(a3),a1
	lea	.dbstr(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr:	dc.b	"chipmem %lx",10,0
	even
 .tt:
	movem.l	(a7)+,d0-d7/a0-a6
	ENDC
	rts

calcfinal_size:
	moveq	#0,d0
	move.w	cb_lenviewx(a3),d0
	add.l	#15,d0
	lsr.l	#4,d0				; words
	add.l	d0,d0				; bytes
	move.l	cb_bitmapsize(a3),d1
	moveq	#0,d2
	move.w	cb_lenviewy(a3),d2
	add.l	d2,d2
	add.l	d2,d1				; total line needed
	move.l	d1,cb_bitmapheight(a3)
	mulu	d0,d1				; total bytes needed
	add.l	d1,d1				; two planes ???
	move.l	d1,cb_memsize(a3)
	rts
*
* Calculate the size of the bitmap of the given file
*
calc_size_bitmap:
	moveq	#11,d1				; initial fontsize
	move.l	cb_filesize(a3),d7
	moveq	#0,d0
	move.l	cb_filemem(a3),a0		; char pointer
	move.l	a0,a1
.rep_search:
	cmp.b	#0,(a0)
	beq	.eoln
	cmp.b	#$0a,(a0)
	beq	.eoln
	cmp.b	#'^',(a0)			; command ?
	beq	.command
	addq.l	#1,a0
	bra	.rep_search
.eoln:
	add.l	d1,d0				; add one line
	add.l	cb_intergap(a3),d0
	tst.b	(a0)+
	bne	.rep_search
	move.l	d0,cb_bitmapsize(a3)
	rts

.command:
	addq.l	#1,a0
	cmp.b	#'f',(a0)
	beq	.skip_until
	cmp.b	#'!',(a0)
	beq	.skip_until
	cmp.b	#'a',(a0)
	beq	.skip_until
	cmp.b	#'v',(a0)
	beq	.skip_until
	cmp.b	#'m',(a0)
	beq	.skip_until
	cmp.b	#'g',(a0)
	beq	.skip_until
	cmp.b	#'d',(a0)
	beq	.skip_until
	cmp.b	#'t',(a0)
	beq	.skip_until
	cmp.b	#'c',(a0)
	beq	.set_color
	cmp.b	#'s',(a0)
	beq	.get_number
	cmp.b	#'i',(a0)
	bne	.rep_search

	addq.l	#1,a0
	moveq	#0,d3
	moveq	#0,d2
.rep_number2:
	tst.b	(a0)
	beq	.rep_search
	cmp.b	#'^',(a0)
	beq	.set_gap	
	move.b	(a0)+,d2
	sub.b	#'0',d2
	mulu	#10,d3
	add.w	d2,d3
	bra	.rep_number2
.set_gap:
	move.l	d3,cb_intergap(a3)
	bra	.skip_until

.set_color:
	addq.l	#1,a0
	cmp.b	#'f',(a0)
	beq	.setfor
	cmp.b	#'b',(a0)
	beq	.setback
	lea	cb_sred(a3),a2
	moveq	#2,d6
	bra	.repgc
.setback:
	lea	cb_bred(a3),a2
	moveq	#2,d6
	bra	.repgc
.setfor:
	lea	cb_red(a3),a2
	moveq	#2,d6
.repgc:
	addq.l	#1,a0
	tst.b	(a0)
	beq	.rep_search
	bsr	.get_color
	move.b	d2,(a2)+
	dbf	d6,.repgc
	bra	.skip_until

.get_number:
	addq.l	#1,a0
	moveq	#0,d1
	moveq	#0,d2
.rep_number:
	tst.b	(a0)
	beq	.rep_search
	cmp.b	#'^',(a0)
	beq	.skip_until	
	move.b	(a0)+,d2
	sub.b	#'0',d2
	mulu	#10,d1
	add.w	d2,d1
	bra	.rep_number	

.skip_until:
	tst.b	(a0)
	beq	.rep_search
	cmp.b	#'^',(a0)+
	bne	.skip_until
	bra	.rep_search

.get_color:
	move.b	(a0),d2
	cmp.b	#'9',d2
	bgt	.hex
	sub.b	#'0',d2
	rts
.hex:
	add.b	#10,d2
	sub.b	#'a',d2
	rts

get_file_size:

	move.l	cb_filename(a3),d1
	moveq	#-2,d2
	move.l	cb_dosbase(a3),a6

	jsr	_LVOLock(a6)
	move.l	d0,cb_filelock(a3)
	beq.w	.file_err
	move.l	d0,d1
	lea.l	cb_viewblock1(a3),a0
	addq.l	#8,a0
	move.l	a0,d2
	and.l	#$fffffffc,d2
	move.l	d2,cb_frame_pointer(a3)
	move.l	cb_dosbase(a3),a6
	jsr	_LVOExamine(a6)
	move.l	cb_frame_pointer(a3),a2
	move.l	124(a2),cb_filesize(a3)
	move.l	cb_filelock(a3),d1
	move.l	cb_dosbase(a3),a6
	jsr	_LVOUnLock(a6)
	moveq	#0,d0
	rts
.file_err:
	clr.l	cb_filesize(a3)
	moveq	#-1,d0
	rts

read_file:
;	lea	config_file(pc),a0
;	move.l	a0,cb_filename(a3)
;	clr.l	cb_filehandle(a3)
;	bsr.w	get_file_size

	clr.l	cb_filememsize(a3)
	clr.l	cb_config_size(a3)

;	move.l	cb_filesize(a3),cb_filememsize(a3)
;	move.l	cb_filesize(a3),cb_config_size(a3)

	move.l	cb_filenamepointer(a3),cb_filename(a3)
	bsr.w	get_file_size
	bne	.file_err
	move.l	cb_filesize(a3),d0
	add.l	d0,cb_filememsize(a3)	

	move.l	cb_filememsize(a3),d0
	addq.l	#2,d0
	move.l	#$10000,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,cb_filemem(a3)
	beq	.file_err

	IFEQ	XAPP
	tst.l	cb_config_size(a3)
	beq	.no_config
	lea	config_file(pc),a0
	move.l	a0,cb_filename(a3)
	move.l	cb_filemem(a3),a4
	bsr	load_file
.no_config:
	ENDC

	move.l	cb_filenamepointer(a3),cb_filename(a3)
	move.l	cb_filemem(a3),a4
	add.l	cb_config_size(a3),a4
	bsr	load_file
	move.l	cb_filemem(a3),a4
	move.l	cb_filememsize(a3),d0
	subq.l	#1,d0
	add.l	d0,a4
	move.b	#0,(a4)	
	moveq	#0,d0
	rts
.file_err:
	moveq	#-1,d0
	rts
	
load_file:

	moveq	#3,d7
	move.l	cb_filename(a3),d1
	move.l	#1005,d2
	move.l	cb_dosbase(a3),a6
	jsr	_LVOOpen(a6)
	move.l	d0,cb_filehandle(a3)
	move.l	cb_filehandle(a3),d1
	move.l	a4,d2
	move.l	cb_filesize(a3),d3
	move.l	cb_dosbase(a3),a6
	jsr	_LVORead(a6)		

* hele file nu in het geheugen 

	bsr.w	close_file
	moveq	#0,d0
	rts

close_file:
	move.l	cb_filehandle(a3),d1
	beq	.no_close
	move.l	cb_dosbase(a3),a6
	jsr	_LVOClose(a6)
	clr.l	cb_filehandle(a3)
.no_close:
	rts

release_filemem:
	tst.l	cb_filemem(a3)
	beq	.nomem
	move.l	cb_filememsize(a3),d0
	addq.l	#2,d0
	move.l	cb_filemem(a3),a1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
.nomem:
	rts

	IFEQ XAPP
datablock:	blk.b	2000,0	
	ENDC
credblock:	blk.b	cred_SIZEOF,0

;	IFNE XAPP

own_delay:
	tst.l	cb_mlsysstruct(a3)
	beq	own_delay_nx

	movem.l	d0-d3/a0/a1,-(a7)
wait_longer:
	move.l	cb_waitmask(a3),d0
	or.l	cb_sig_ptoc(a3),d0
no_sigptoc:

	move.l	$4,a6
	jsr	_LVOWait(a6)

	movem.l	d0/d1/a0/a1,-(a7)
	move.l	$4,a6
	jsr	_LVOForbid(a6)
	movem.l	(a7)+,d0/d1/a0/a1

	move.l	d0,d1
	move.l	d0,d3
	and.l	cb_sig_ptoc(a3),d1
	beq.w	no_wacht1

; Er is een message van boven kijk wat zij doet en handel ernaar

search_list:
;	bra	doterm
	move.l	cb_msgpointer(a3),a0		;get msgport ptr
	lea.l	MP_MSGLIST(a0),a1		;make ptr to msglist
	move.l	LH_HEAD(a1),d1			;get first 	

scan:	move.l	d1,a1
	move.l	LH_HEAD(a1),d1
	beq.w	end_of_list

	move.l	pd_Cmd(a1),d2	; pd command
	cmp.l	#DCC_DOTERM,d2
	beq.b	doterm			; let de C-prog handle the reply
	cmp.l	#DCC_DOSTOP,d2
	beq.b	doterm
	bra.b	scan

doterm:
	movem.l	d0-d2/a0/a1,-(a7)
	sub.l	a1,a1
	move.l	$4,a6
	jsr	_LVOFindTask(a6)
	move.l	d0,a1
	move.l	cb_sig_ptoc(a3),d0
	jsr	_LVOSignal(a6)		; restore signal
	movem.l	(a7)+,d0-d3/a0/a1

exit_art:
	move.l	$4,a6
	jsr	_LVOPermit(a6)
	movem.l	(a7)+,d0-d3/a0/a1
	moveq	#-1,d0
	rts

no_wacht1:
end_of_list:
	move.l	$4,a6
	jsr	_LVOPermit(a6)

	move.l	cb_waitmask(a3),d1
	and.l	d1,d3
	beq	wait_longer			; only when 50hz has

	movem.l	(a7)+,d0-d3/a0/a1
	moveq	#0,d0
	rts

;	ELSE

own_delay_nx:
	move.l	cb_waitmask(a3),d0
	or.l	cb_sig_quit(a3),d0
	move.l	$4.w,a6
	jsr	_LVOWait(a6)

	move.l	d0,d1

	IFEQ	XAPP
	btst	#6,$bfe001
	beq	.yes
	ENDC
	
	and.l	cb_sig_quit(a3),d1
	beq	.no
.yes:
	moveq	#-1,d0
	rts
.no:
	moveq	#0,d0
	rts

;	ENDC

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

	moveq	#0,d0
	lea	mpmmuname(pc),a1
	move.l	$4.w,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,cb_mpmmubase(a3)
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
	lea	diskfontname(pc),a1
	moveq	#0,d0
	move.l	$4.w,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,cb_fontbase(a3)
	beq.b	lib_err2

	moveq	#0,d0
	rts

lib_err2:
	moveq	#-1,d0
	rts

close_all:
;	tst.b	cb_views_init(a3)
;	beq	.no_cmap
;	bsr.w	free_viewports

	move.l	cb_monitorspec(a3),d0
	beq	.nomon
	move.l	d0,a0
	move.l	cb_graphbase(a3),a6
	jsr	_LVOCloseMonitor(a6)
	clr.l	cb_monitorspec(a3)
.nomon:
	bsr.w	close_libs
	rts
*
* Copy the data from crawl rec to local storage
*
get_args:
	move.l	#28,cb_linesize(a3)
	lea	credblock(pc),a0
	move.l	#11,cred_fontsize(a0)
	move.l	#1,cred_speed(a0)
	move.l	#1,cred_col(a0)
	move.l	#0,cred_zero_col(a0)
	lea	fontname(pc),a1
	lea	cred_fontname(a0),a2
.cc
	tst.b	(a1)
	beq	.no
	move.b	(a1)+,(a2)+
	bra	.cc
.no:
	move.b	#0,(a2)

	move.l	cred_txt(a0),cb_text_data(a3)
	move.l	cred_fontsize(a0),d0
	move.w	d0,cb_fontsize(a3)
	lea.l	cred_fontname(a0),a1
	move.l	a1,cb_fontname(a3)
	move.l	cred_speed(a0),d0
	move.w	d0,cb_speed(a3)
	rts
*
* Place the text in the bitmap
*
draw_text:
	move.l	#11,cb_linesize(a3)		; init line size
	moveq	#0,d0
	moveq	#0,d1
	move.w	cb_lenviewy(a3),d1		; start just not visible
;	moveq	#100,d1				; DEBUG
	move.l	d1,cb_current_y(a3)		; current write position
	add.w	cb_baseline(a3),d1
	bsr	set_move_first
	move.l	cb_filemem(a3),a4
	move.b	#0,cb_fontchange(a3)
.rep_lines:
	tst.b	(a4)
	beq	.eof
	cmp.b	#$0A,(a4)
	beq	.eoln
	cmp.b	#'^',(a4)
	beq	.command

; when you are here al the commands are stripped off

	bsr	print_line
	bra	.rep_lines
.eoln:
	addq.l	#1,a4
	move.l	cb_current_y(a3),d0
	add.l	cb_linesize(a3),d0		; goto next line
	add.l	cb_intergap(a3),d0
	move.l	d0,cb_current_y(a3)
	bra	.rep_lines
.eof:
	rts

.command:
	addq.l	#1,a4
	cmp.b	#'f',(a4)
	beq	.font_command
	cmp.b	#'a',(a4)
	beq	.a_command
	cmp.b	#'c',(a4)
	beq	.skip_until
	cmp.b	#'!',(a4)
	beq	.skip_until
	cmp.b	#'v',(a4)
	beq	.get_speed
	cmp.b	#'m',(a4)
	beq	.get_middle
	cmp.b	#'g',(a4)
	beq	.get_gapsize
	cmp.b	#'t',(a4)
	beq	.get_shadow_type
	cmp.b	#'d',(a4)
	beq	.get_shadow_depth
	cmp.b	#'i',(a4)
	beq	.get_intergapsize
	cmp.b	#'s',(a4)
	bne	.rep_lines
	bsr	get_number
	tst.b	(a4)
	beq	.rep_lines
	addq.l	#1,a4
	move.l	d1,cb_linesize(a3)
	bra	.rep_lines

.skip_until:
	tst.b	(a4)
	beq	.rep_lines
	cmp.b	#'^',(a4)+
	bne	.skip_until
	bra	.rep_lines

.a_command:
	addq.l	#1,a4
	cmp.b	#'l',(a4)
	bne	.nol
	move.w	#3,cb_allign(a3)
	bra	.skip_until
.nol:
	cmp.b	#'m',(a4)
	bne	.nom
	move.w	#2,cb_allign(a3)
	bra	.skip_until
.nom:
	cmp.b	#'r',(a4)
	bne	.skip_until
	move.w	#1,cb_allign(a3)
	bra	.skip_until

.get_middle:
	bsr	get_number
	tst.b	(a4)
	beq	.rep_lines
	addq.l	#1,a4
	move.l	d1,cb_middle(a3)
	bra	.rep_lines

.get_gapsize:
	bsr	get_number
	tst.b	(a4)
	beq	.rep_lines
	addq.l	#1,a4
	move.l	d1,cb_middle_gap(a3)
	bra	.rep_lines

.get_shadow_depth:
	bsr	get_number
	tst.b	(a4)
	beq	.rep_lines
	addq.l	#1,a4
	move.w	d1,cb_shadow_depth(a3)
	bra	.rep_lines

.get_shadow_type:
	move.w	#0,cb_shadow_type(a3)
	addq.l	#1,a4
	cmp.b	#'f',(a4)
	bne	.nol
	move.w	#1,cb_shadow_type(a3)
	bra	.skip_until

.get_speed:
	bsr	get_number
	tst.b	(a4)
	beq	.rep_lines
	addq.l	#1,a4
;	add.w	d1,d1
	move.w	d1,cb_speed(a3)
	bra	.rep_lines

.get_intergapsize:
	bsr	get_number
	tst.b	(a4)
	beq	.rep_lines
	addq.l	#1,a4
	move.l	d1,cb_intergap(a3)
	bra	.rep_lines
	
.font_command:
	move.b	#1,cb_fontchange(a3)
	lea	cb_fontname_buffer(a3),a2
	addq.l	#1,a4
.getf:
	tst.b	(a4)
	beq	.rep_lines
	cmp.b	#'^',(a4)
	beq	.fend
	move.b	(a4)+,(a2)+
	bra	.getf
.fend:
	move.b	#0,(a2)
	addq.l	#1,a4
	bra	.rep_lines
*
* Get a number from the stream a4
*
get_number:
	addq.l	#1,a4
	moveq	#0,d1
	moveq	#0,d2
.rep_number2:
	tst.b	(a4)
	beq	.ready_num
	cmp.b	#'^',(a4)
	beq	.ready_num
	move.b	(a4)+,d2
	sub.b	#'0',d2
	mulu	#10,d1
	add.w	d2,d1
	bra	.rep_number2
.ready_num:
	rts

change_font:
	lea	cb_fontname_buffer(a3),a0
	move.l	cb_linesize(a3),d0
	move.w	d0,cb_fontsize(a3)
	move.l	a0,cb_fontname(a3)
.rep:
	tst.b	(a0)+
	bne	.rep
	subq.l	#5,a0
	cmp.b	#'.',(a0)+
	bne	.addf
	cmp.b	#'f',(a0)+
	bne	.addf
	cmp.b	#'o',(a0)+
	bne	.addf
	cmp.b	#'n',(a0)+
	bne	.addf
	cmp.b	#'t',(a0)+
	bne	.addf
	bra	.noaddf
.addf:
	lea	cb_fontname_buffer(a3),a0
.rep2:
	tst.b	(a0)+
	bne	.rep2
	subq.l	#1,a0
	move.b	#'.',(a0)+
	move.b	#'f',(a0)+
	move.b	#'o',(a0)+
	move.b	#'n',(a0)+
	move.b	#'t',(a0)+
	move.b	#0,(a0)+
.noaddf:
	bsr	load_font
	bsr	set_font
	rts

test_line:
	move.l	a4,a0
	moveq	#0,d0
.rep:
	cmp.b	#$0a,(a0)
	beq	.eoln
	cmp.b	#0,(a0)
	beq	.eoln
	cmp.b	#'|',(a0)
	beq	.eoln
	addq.l	#1,d0
	addq.l	#1,a0
	bra	.rep
.eoln:
	cmp.b	#'|',(a0)
	bne	normal
	move.l	d0,d7

	move.l	a4,a0
	lea	cb_rastport(a3),a1
	move.l	cb_graphbase(a3),a6
	jsr	_LVOTextLength(a6)
	move.l	cb_middle(a3),d1
	sub.l	cb_middle_gap(a3),d1
	sub.l	d0,d1
	bpl	.oke
	moveq	#0,d1
.oke:
	move.w	d1,d0
	bsr	shadow
	move.l	cb_current_y(a3),d1
	add.w	cb_baseline(a3),d1
	bsr	set_move_first


	move.l	a4,a0
	add.l	d7,a4
	move.l	d7,d0
	lea	cb_rastport(a3),a1
	move.l	cb_graphbase(a3),a6
	jsr	_LVOText(a6)
qpp:
	addq.l	#1,a4				; skip the | sign
	move.l	a4,a0
	moveq	#0,d0
.rep:
	cmp.b	#$0a,(a0)
	beq	.eoln
	cmp.b	#0,(a0)
	beq	.eoln
	cmp.b	#'|',(a0)
	beq	.eoln
	addq.l	#1,d0
	addq.l	#1,a0
	bra	.rep
.eoln:
	move.l	d0,d7

	move.l	a4,a0
	lea	cb_rastport(a3),a1
	move.l	cb_graphbase(a3),a6
	jsr	_LVOTextLength(a6)
	move.l	cb_middle(a3),d0
	add.l	cb_middle_gap(a3),d0

	bsr	shadow

	move.l	cb_current_y(a3),d1
	add.w	cb_baseline(a3),d1
	bsr	set_move_first

	move.l	a4,a0
	add.l	d7,a4
	move.l	d7,d0
	lea	cb_rastport(a3),a1
	move.l	cb_graphbase(a3),a6
	jsr	_LVOText(a6)
	moveq	#-1,d0
	rts

shadow:
	tst.w	cb_shadow_depth(a3)
	beq	.no_shadow
	tst.w	cb_shadow_type(a3)
	bne	.filled_shadow
	movem.l	d0-d5,-(a7)
	move.w	cb_shadow_depth(a3),d5
	bsr	do_shadow
	movem.l	(a7)+,d0-d5
.no_shadow:
	rts

.filled_shadow:
	moveq	#1,d5
.repfill:
	movem.l	d0-d5,-(a7)
	bsr	do_shadow
	movem.l	(a7)+,d0-d5
	addq.w	#1,d5
	cmp.w	cb_shadow_depth(a3),d5
	ble	.repfill
	rts
	
do_shadow:
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	d0,d6

	move.l	cb_graphbase(a3),a6
	lea	cb_rastport(a3),a1
	moveq	#2,d0
	jsr	_LVOSetAPen(a6)

	move.l	d6,d0
	move.l	cb_current_y(a3),d1
	add.w	cb_baseline(a3),d1

	add.w	d5,d0
	add.w	d5,d1

	bsr	set_move_first
	move.l	a4,a0
	add.l	d7,a4
	move.l	d7,d0
	lea	cb_rastport(a3),a1
	move.l	cb_graphbase(a3),a6
	jsr	_LVOText(a6)
	moveq	#1,d0
	lea	cb_rastport(a3),a1
	jsr	_LVOSetAPen(a6)
	movem.l	(a7)+,d0-d7/a0-a6
	rts
	
normal:
	moveq	#0,d0
	rts

*
* print the line from a4 until 0 or eoln
*
print_line:
	tst.b	cb_fontchange(a3)
	beq	.nofc
	bsr	change_font
	move.b	#0,cb_fontchange(a3)
.nofc:
	bsr	test_line
	bne	.done
	move.l	a4,a0
	moveq	#0,d0
.rep:
	cmp.b	#$0a,(a0)
	beq	.eoln
	cmp.b	#0,(a0)
	beq	.eoln
	addq.l	#1,d0
	addq.l	#1,a0
	bra	.rep
.eoln:
	move.l	d0,d7

	move.l	a4,a0
	lea	cb_rastport(a3),a1
	move.l	cb_graphbase(a3),a6
	jsr	_LVOTextLength(a6)

	cmp.w	#1,cb_allign(a3)
	bne	.no_right
	move.w	cb_lenviewx(a3),d1
	sub.w	cb_shadow_depth(a3),d1
	exg	d0,d1
	sub.w	d1,d0
	bpl	.normal
	moveq	#0,d0
	bra	.normal
	
.no_right:
	cmp.w	#2,cb_allign(a3)
	bne	.no_middle
	move.l	cb_middle(a3),d1
	lsr.l	#1,d0
	exg	d0,d1
	sub.w	d1,d0
	bpl	.normal
	moveq	#0,d0
	bra	.normal
.no_middle:
	moveq	#0,d0
.normal:
	bsr	shadow
	move.l	cb_current_y(a3),d1
	add.w	cb_baseline(a3),d1
	bsr	set_move_first
	move.l	a4,a0
	add.l	d7,a4
	move.l	d7,d0
	move.l	cb_graphbase(a3),a6
	lea	cb_rastport(a3),a1
	jsr	_LVOText(a6)

.done:
	rts

clear_screen:
	IFNE	XAPP
	move.l	cb_screenmem(a3),a0
	move.l	cb_screensize(a3),d0
	lsr.l	#2,d0
.repclear
	clr.l	(a0)+
	subq.l	#1,d0
	bne	.repclear
	ENDC
	rts	

set_colors_to_zero:
	tst.l	cb_mlsysstruct(a3)
	beq	.nono

	move.l		cb_graphbase(a3),a6
	move.l		gb_ActiView(a6),cb_oldview(a3)
	move.l		gb_ActiView(a6),a1

	move.l		v_ViewPort(a1),a1
	move.l		vp_ColorMap(a1),a0
	move.w		cm_Count(a0),d7
	beq		.nono
	subq.l		#1,d7
	moveq		#0,d6
	move.l		a1,d5
.repsetzero:	
	move.b		cb_bred(a3),d1
	move.b		cb_bgreen(a3),d2
	move.b		cb_bblue(a3),d3
	move.l		d6,d0
	move.l		d5,a0
	jsr		_LVOSetRGB4(a6)

	addq.l		#1,d6
	dbf		d7,.repsetzero

.nono:
	rts
	
do_scroll:
	bsr.w	load_font
	tst.w	d0
	bne.w	.not_loaded
	bsr.w	set_size

	bsr.w	initialize_viewports
;	bra	.tempoff
	
	bsr.w	set_font

	bsr	set_colors_to_zero

	bsr	clear_screen
	
	bsr	draw_text

	moveq	#1,d1
	move.w	cb_speed(a3),d0
	cmp.w	#3,d0
	bge	.subit
	moveq	#3,d1
	sub.w	d0,d1
	moveq	#1,d0
	bra	.setspeed
.subit:
	sub.w	#2,d0
	add.w	d0,d0
.setspeed:

	move.l	d1,teller
	move.l	d1,tellerstart

	move.w	d0,cb_speed(a3)

	bsr.w	setsam
	tst.b	d0
	bne.w	.not_loaded

	bsr.w	install_50h

	lea	sigmask(pc),a0
	move.l	(a0),d0
	move.l	d0,cb_waitmask(a3)

	move.l	cb_graphbase(a3),a6
	move.l	gb_ActiView(a6),cb_oldview(a3)
;	move.l	gb_ActiView(a6),a1

	move.l	cb_text_data(a3),a0
	move.l	a0,cb_charpointer(a3)

	bsr.w	vertical_scroll

	bsr	clear_screen

	bsr.w	restore_oldview
	
	bsr.w	remove_50h
	bsr.w	freesam

	move.l	cb_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)

	bsr	clear_screen

.tempoff:
	tst.b	cb_views_init(a3)
	beq	.not_loaded
	bsr.w	free_viewports

.not_loaded:
	moveq	#0,d0
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
* Set the rastport on position d0,d1
*
set_move_first:
	move.l	cb_graphbase(a3),a6
	lea	cb_rastport(a3),a1
	jsr	_LVOMove(a6)
	rts
*
* Place a character in the rastport at the two positions
* a0 points to the character
*
place_char:
	move.l	a0,-(a7)
	move.l	a0,-(a7)
	bsr.b	set_move_first
	move.l	(a7)+,a0
	move.l	a0,-(a7)
	moveq	#1,d0
	lea	cb_rastport(a3),a1
	move.l	cb_graphbase(a3),a6
	jsr	_LVOText(a6)
	lea	cb_rastport(a3),a1
	move.w	rp_cp_x(a1),cb_rastx(a3)	; same for secundair char
	move.w	rp_cp_y(a1),cb_rasty(a3)
	
	move.l	(a7)+,a0
	moveq	#1,d0
	lea	cb_rastport(a3),a1
	move.l	cb_graphbase(a3),a6
	jsr	_LVOText(a6)
	lea	cb_rastport(a3),a1
	move.w	rp_cp_x(a1),cb_rast_secx(a3)	; same for secundair char
	move.w	rp_cp_y(a1),cb_rast_secy(a3)
	move.l	(a7)+,a0
	rts
*
* Do a vertical scroll
* With every scroll up you place X characters below and above the picture
* when the whole line is scrolled up the line should be finished.
* When the vertical position is greater than the bottom value the whole
* screen is banged down to the position where the same data is
* A seperate count is needed for the text position, to indicate when
* the text should move down the rastport.
*
* d7 is the absolute position of the rastport
* d6 is the relative position to the linestart
*
vertical_scroll:
	move.l	cb_crawl_rec(a3),a0

	lea	cb_viewblock1(a3),a5
	move.l	a5,cb_active_view(a3)
	bsr.w	make_viewport

	lea	cb_viewblock2(a3),a5
	move.l	a5,cb_inactive_view(a3)
	bsr.w	make_viewport
	bsr.w	show_viewport

	moveq	#0,d5
	move.w	cb_lenviewy(a3),d5
	move.l	cb_bitmapheight(a3),d6		; end check
	sub.l	d5,d6

	moveq	#0,d7
.rep_test1:
	bsr.w	own_delay
	tst.w	d0
	bne.w	stop_scroll
	move.l	cb_inactive_view(a3),a5
	lea	vi_rasinfo(a5),a0
	move.w	d7,ri_RyOffset(a0)
	lea	vi_viewport(a5),a0
	move.l	cb_graphbase(a3),a6
	jsr	_LVOScrollVPort(a6)

	bsr.w	show_viewport

;	bsr.w	own_delay
;	tst.w	d0
;	bne.w	stop_scroll

	move.l	cb_inactive_view(a3),a5
	move.l	cb_active_view(a3),cb_inactive_view(a3)
	move.l	a5,cb_active_view(a3)

	add.w	cb_speed(a3),d7
	cmp.l	d7,d6
	bge	.rep_test1

stop_scroll:
	rts	

*
* Place d0 chars on position check on return or zero
* a0 points to the text
* update the char_pointer variable
*
place_chars:
	move.l	d7,-(a7)
	moveq	#0,d7				; check for return and zero
	move.l	a0,a1
.check:
	cmp.b	#10,(a1)
	beq	.end_check
	tst.b	(a1)+
	beq	.end_check
	addq.l	#1,d7
	subq.l	#1,d0
	bne	.check
.end_check:					; in d1 the number of chars
	move.l	a0,a4
	bsr.w	set_move_first
	move.l	a4,a0
	move.l	d7,d0
	beq	.no_place
	lea	cb_rastport(a3),a1
	move.l	cb_graphbase(a3),a6
	jsr	_LVOText(a6)
	lea	cb_rastport(a3),a1
	move.w	rp_cp_x(a1),cb_rastx(a3)	; same for secundair char
	move.w	rp_cp_y(a1),cb_rasty(a3)
	
	move.l	a4,a0
	move.l	d7,d0
	lea	cb_rastport(a3),a1
	move.l	cb_graphbase(a3),a6
	jsr	_LVOText(a6)
	lea	cb_rastport(a3),a1
	move.w	rp_cp_x(a1),cb_rast_secx(a3)	; same for secundair char
	move.w	rp_cp_y(a1),cb_rast_secy(a3)
.no_place:
	move.l	a4,a0
	add.l	d7,a0
	move.l	a0,cb_charpointer(a3)
	move.l	(a7)+,d7
	rts
*
* load a diskfont for the rastports
*
load_font:
	tst.l	cb_fontpointer(a3)		; first close old font
	beq	.nofont
	move.l	cb_fontpointer(a3),a1
	move.l	cb_graphbase(a3),a6
	jsr	_LVOCloseFont(a6)
	clr.l	cb_fontpointer(a3)
.nofont:
	move.l	cb_fontname(a3),a1
.topaz:
	lea	cb_fontattr(a3),a0
	move.l	a1,ta_Name(a0)	
	move.w	cb_fontsize(a3),ta_YSize(a0)
	move.l	cb_fontbase(a3),a6
	jsr	_LVOOpenDiskFont(a6)
	move.l	d0,cb_fontpointer(a3)
	beq.b	.get_font_err
	move.l	d0,a0
	move.w	tf_YSize(a0),cb_fontheight(a3)
	move.w	tf_Baseline(a0),cb_baseline(a3)
	moveq	#0,d0
	rts
.get_font_err:
	lea	fontname(pc),a1
	move.w	#8,cb_fontsize(a3)
	bra	.topaz
*
* Set the loaded font in the rastport
*
set_font:
	lea	cb_rastport(a3),a1
	move.l	cb_graphbase(a3),a6
	move.l	cb_fontpointer(a3),a0
	jsr	_LVOSetFont(a6)	
	moveq	#0,d0
	rts

get_font_err:
	moveq	#-1,d0
	rts

*
* Initialize two views with each a viewport
* These two view are used to scroll the text up with double buffering
*
initialize_viewports:
	move.b	#1,cb_views_init(a3)

	lea	cb_viewblock1(a3),a5
	bsr.w	create_viewport

	lea	cb_viewblock2(a3),a5
	bsr.w	create_viewport

	lea	cb_rastport(a3),a1
	jsr	_LVOInitRastPort(a6)

	moveq	#RP_JAM1,d0
	lea	cb_rastport(a3),a1
	jsr	_LVOSetDrMd(a6)

	moveq	#1,d0
	lea	cb_rastport(a3),a1
	jsr	_LVOSetAPen(a6)


	lea	cb_viewblock1(a3),a5
	lea	cb_rastport(a3),a1
	lea	vi_bitmap(a5),a0
	move.l	a0,rp_BitMap(a1)
	rts

free_viewports:
	lea	cb_viewblock2(a3),a5
	bsr.w	free_view

	lea	cb_viewblock1(a3),a5
	bsr.w	free_view

	clr.b	cb_views_init(a3)
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
	move.l		cb_screenmem(a3),d0
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
	cmp.w	cb_max_x(a3),d0
	ble	.oke1
	move.w	cb_max_x(a3),d0
.oke1:
	sub.w	d1,d0
	asr.w	#1,d0
	neg.w	d0

	move.w	d0,vp_DxOffset(a0)

	bsr	get_dy_offset
	
	move.w	d0,vp_DyOffset(a0)

	move.l	cb_graphbase(a3),a6
	moveq	#VIEW_EXTRA_TYPE,d0
	jsr	_LVOGfxNew(a6)
	tst.l	d0
	beq	exit_maakv

	move.l	d0,vi_vextra(a5)

	IFNE	DEBUG
	movem.l	d0-d7/a0-a6,-(a7)
	lea	vi_vextra(a5),a1
	lea	.dbstr(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr:	dc.b	"View extra [%lx]",10,0
	even
 .tt:
	movem.l	(a7)+,d0-d7/a0-a6
	ENDC

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

	moveq	#4,d0
	jsr	_LVOGetColorMap(a6)
	tst.l	d0
	beq	exit_maakv

;	move.l		cb_colormap(a3),d0

	move.l		d0,a0
	move.l		cb_tags(a3),a1
	lea.l		vi_viewport(a5),a2
	move.l		a2,4(a1)
	jsr		_LVOVideoControl(a6)

	lea.l		vi_viewport(a5),a0
	move.b		cb_red(a3),d1
	move.b		cb_green(a3),d2
	move.b		cb_blue(a3),d3
	moveq		#1,d0
	jsr		_LVOSetRGB4(a6)
	lea.l		vi_viewport(a5),a0
	move.b		cb_bred(a3),d1
	move.b		cb_bgreen(a3),d2
	move.b		cb_bblue(a3),d3
	moveq		#0,d0
	jsr		_LVOSetRGB4(a6)
	lea.l		vi_viewport(a5),a0
	move.b		cb_sred(a3),d1
	move.b		cb_sgreen(a3),d2
	move.b		cb_sblue(a3),d3
	moveq		#2,d0
	jsr		_LVOSetRGB4(a6)
exit_maakv:
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

;	IFEQ XAPP
temp_tags:
	dc.l	VTAG_ATTACH_CM_SET,0
	dc.l	VTAG_VIEWPORTEXTRA_SET,0
	dc.l	VTAG_NORMAL_DISP_SET,0

	dc.l	VTAG_BORDERBLANK_CLR,0
	dc.l	VTAG_CHROMAKEY_SET,0
	dc.l	VTAG_CHROMA_PEN_CLR,0

	dc.l	VTAG_END_CM,0
;	ENDC

show_viewport:
	move.l		cb_graphbase(a3),a6
	lea		vi_view(a5),a1
	jsr		_LVOLoadView(a6)
.rep:
;	btst	#10,$dff016
;	bne	.rep
	rts

restore_oldview:
	move.l		cb_graphbase(a3),a6
cleanexit:
	IFNE	DEBUG
	movem.l	d0-d7/a0-a6,-(a7)
	lea	.dbstr(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr:	dc.b	"Load oldview",10,0
	even
 .tt:
	movem.l	(a7)+,d0-d7/a0-a6
	ENDC

	move.l		cb_graphbase(a3),a6
	tst.l		cb_oldview(a3)
	beq.b		no_oldview
	move.l		cb_oldview(a3),a1
	jsr		_LVOLoadView(a6)
	jsr		_LVOWaitTOF(a6)
no_oldview:
	rts

free_view:
	IFNE	DEBUG
	movem.l	d0-d7/a0-a6,-(a7)
	lea	.dbstr(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr:	dc.b	"Free view",10,0
	even
 .tt:
	movem.l	(a7)+,d0-d7/a0-a6
	ENDC
	lea		vi_viewport(a5),a0
	move.l		cb_graphbase(a3),a6
	jsr		_LVOGfxLookUp(a6)
	move.l		d0,vi_vpextra(a5)

	lea		vi_view(a5),a0
	move.l		cb_graphbase(a3),a6
	jsr		_LVOGfxLookUp(a6)
	move.l		d0,vi_vextra(a5)

	lea		vi_viewport(a5),a0
	tst.l		vp_ColorMap(a0)
	beq		.no_cmap

	move.l		vp_ColorMap(a0),a0
	jsr		_LVOFreeColorMap(a6)

.no_cmap:

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
	IFNE	DEBUG
	movem.l	d0-d7/a0-a6,-(a7)
	lea	vi_vextra(a5),a1
	lea	.dbstr(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr:	dc.b	"Viewfree extra [%lx]",10,0
	even
 .tt:
	movem.l	(a7)+,d0-d7/a0-a6
	ENDC

	tst.l	vi_vpextra(a5)
	beq	.novp
	move.l	vi_vpextra(a5),a0
	jsr	_LVOGfxFree(a6)
.novp:
	tst.l	vi_vextra(a5)
	beq	.nov
	move.l	vi_vextra(a5),a0
	jsr	_LVOGfxFree(a6)
.nov:


	moveq		#0,d0
	rts

close_libs:
	IFNE	DEBUG
	movem.l	d0-d7/a0-a6,-(a7)
	lea	.dbstr(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr:	dc.b	"Close libs",10,0
	even
 .tt:
	movem.l	(a7)+,d0-d7/a0-a6
	ENDC

	tst.l	cb_fontbase(a3)
	beq.b	exit2
	tst.l	cb_fontpointer(a3)
	beq.b	no_font_exit
	move.l	cb_fontpointer(a3),a1
	move.l	cb_graphbase(a3),a6
	jsr	_LVOCloseFont(a6)
	clr.l	cb_fontpointer(a3)
no_font_exit:
	move.l	cb_fontbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	cb_fontbase(a3)
exit2:
	tst.l	cb_mpmmubase(a3)
	beq	.no_mpmmu
	move.l	cb_mpmmubase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	cb_mpmmubase(a3)
.no_mpmmu:
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
	IFNE	DEBUG
	movem.l	d0-d7/a0-a6,-(a7)
	lea	.dbstr(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr:	dc.b	"Free signals",10,0
	even
 .tt:
	movem.l	(a7)+,d0-d7/a0-a6
	ENDC
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

proc50hnaam:	dc.b	"50Hz vscroll",0
	even
		
intstruct50h:
	dc.l	0,0
	dc.b	2,15		; type en pri
	dc.l	0,0		; pointer naar naam en data
	dc.l	0

teller:		dc.l	2
tellerstart:	dc.l	2
task:		dc.l	0
signum:		dc.l	0
sigmask:	dc.l	0
handle_installed:	dc.b	0,0

off_teller 	= 0
off_tel_start 	= 4
off_task 	= 8
off_signum 	= 12
off_sigmask 	= 16

	IFEQ XAPP
config_file:	dc.b	"wp:easy_up/easyup.config",0
filename:	dc.b	"mp:datafiles/crawl128",0
	ENDC
diskfontname:	dc.b	'diskfont.library',0
graphname:	dc.b	"graphics.library",0
intname:	dc.b	"intuition.library",0
	IFEQ	XAPP
mpmmuname:	dc.b	"nb:system/mpmmu.library",0
	ELSE
mpmmuname:	dc.b	"mpmmu.library",0
	ENDC
dosname:	dc.b	"dos.library",0
fontname:	dc.b	"topaz.font",0
load_err_msg:	dc.b	"File not found",0
file_to_big_msg:	dc.b	"File to big",0

	END
