
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

XAPP = 0
BIT24 = 0
NEWEFF = 1

VIEWINFO = 0
GFXSNOOP = 0
DEBUG = 0
PRAND = 0
ESCLOOK = 0

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

	IFNE	DEBUG!VIEWINFO!PRAND!GFXSNOOP!ESCLOOK
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
	INCDIR	"wp:asm/"
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

;MON_ID = DEFAULT_MONITOR_ID
MON_ID = DBLPAL_MONITOR_ID
;MON_ID = PAL_MONITOR_ID
;MON_ID = NTSC_MONITOR_ID
;MON_ID = SUPER72_MONITOR_ID
;MON_ID = $31000

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
NUM_OFFWIPES = 2
	ELSE
NUM_OFFWIPES = 2
	ENDC

NUM_LINEWIPES = 40

PATHEND = -10000		; terminatie voor path plot

pstart:
	IFEQ	XAPP

	IFEQ	DEBUG
start:
	ENDC
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	#vb_SIZEOF,d0

	bsr	init_jump_tabel

	lea	startdatablock,a3

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
	bne	exit	
	jsr	install_50h
	tst.l	db_monitorspec(a3)
	beq	exit

	move.b	#1,db_cyc_quit(a3)
	lea	sigmask,a0
	move.l	(a0),db_waitmasker(a3)
	move.w	#$8400,$dff096
	jsr	do_slide		; voer internal slide uit
	move.w	#$0400,$dff096

	move.l	#MEMF_STAY+2,d1
	move.l	db_mlmmubase(a3),a6
	jsr	_LVOMLMMU_FlushMem(a6)	; flush the mpmmu library

	moveq	#0,d7
	bra.w	sexit

	rts
	ENDC
sexit:	jmp	exit
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

	XDEF	_get_varsize
_get_varsize:
	move.l	#db_SIZEOF,d0
	rts

	XDEF _update_screen
_update_screen:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a3
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

	XDEF	_finish_doc
_finish_doc:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a3
	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)

	bsr	remove_50h
	bsr	freesam
	
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

	XDEF	_create_text
_create_text:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	bra	ret_err
	XREF	_DrawText

**************************************************
					
	XDEF	_create_clip
_create_clip:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	bra	ret_err

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

	XDEF	_create_window

_create_window:
	link	a5,#0

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

	
	XDEF	_create_screen
_create_screen:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

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

	move.l	8(a5),a0			; pointer naar mlsystem
	move.l	a0,db_mlsysstruct(a3)
	move.l	#0,db_dummy_width(a3)

	move.b	#0,db_sema_on(a3)
	
	lea	ml_sema_trans(a0),a0
	move.l	$4.w,a6
	jsr	_LVOObtainSemaphore(a6)

	move.b	#$ff,db_sema_on(a3)

	move.l	20(a5),d0
	move.l	d0,db_eff_nums(a3)

	move.l	24(a5),d0
	moveq	#1,d1
	lsl.l	d0,d1
	move.l	d1,db_go_on_key_sig(a3)

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

;	lea	temp_tags(pc),a1
;	move.l	a1,db_tags(a3)

	move.l	ml_monitor_ID(a0),db_monid(a3)
;	bsr	filter_monitor_id

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

	bsr	create_tabels

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
;	move.l	db_nulmuis(a3),(a0)+
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
;	move.l	(a0)+,db_nulmuis(a3)
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
in_nulmuis:		dc.l	0
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
mlmmuname:	dc.b	'nb:system/mpmmu.library',0

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
;	move.l	d0,db_nulmuis(a3)
	addq.l	#8,d0
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

	bsr	laadmem

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
	bsr	set_view_structs_ML		; set active viewport

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

; Open global monitor for further use

;	move.l	db_graphbase(a3),a6
;	move.l	db_monid(a3),d0
;	move.w	#0,a1
;	moveq	#0,d0
;	lea	mname(pc),a1
;	jsr	_LVOOpenMonitor(a6)
;	move.l	d0,db_monitorspec(a3)
;	beq	sexit

	rts
mname:	dc.b	"pal",0
	even
	ENDC

wipes:
	dc.l	showdirect-pstart
	dc.l	cop_drip1-pstart
	dc.l	cop_flip1-pstart
;	dc.l	blend-pstart
	dc.l	cop_cover_down-pstart,cop_up1-pstart,cop_down1_v1-pstart,cop_up2-pstart

	IFNE	NEWEFF
	dc.l	cop_drip1-pstart,cop_drip2-pstart
	ENDC
	dc.l	-1

blitin_wipes:
;	dc.l	blitin_direct-pstart
	dc.l	-1

wipesrev:
	dc.l	blitin_direct_rev-pstart
	dc.l	-1

* dit zijn constanten

CONV_S = 0
STAY_S = 1
FLIP_S = 2
RAND_S = 4

wipe_types_new:
		dc.b	STAY_S,STAY_S
		dc.b	FLIP_S
		dc.b	STAY_S,STAY_S,STAY_S,STAY_S

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
	move.l	ve_Monitor(a0),d0
	beq	.normal_loadview
	move.l	d0,a0
	move.l	ms_LoadView(a0),d0
	beq	.normal_loadview
	move.l	d0,a0
	bra	.set_loadview	

.normal_loadview:
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
	IFNE PRAND
	movem.l	a0-a6/d0-d7,-(a7)
	lea	db_varltimes(a3),a1
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	lea	db_variation(a3),a1
	lea	.dbstr2(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"copd Varl is %ld",10,0
.dbstr2:	dc.b	"copd Vari is %ld",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC


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
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Exit wait free na check but",10,0
	even
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

do_modus3:
	move.w	vb_leny(a5),d7
	move.l	lofm1(pc),a0
	move.l	shfm1(pc),a1
.repdm3:
	move.l	(a2)+,d4
	move.w	vb_moda(a5),d1
	move.l	vb_totalwidth(a5),d1
	mulu	d4,d1
	add.w	vb_moda(a5),d1
.same_put2:
	bsr	set_modulo_a0
	bne	.exit
	bsr	set_modulo_a1
	bne	.exit
	subq.l	#1,d7
	bge	.repdm3
.exit:
	rts

do_modus2:
	move.l	lofm1(pc),a0
	move.l	shfm1(pc),a1

	move.l	(a2),d7
	lsr.l	#1,d7
	tst.w	d7
	beq	.exit
	move.w	vb_leny(a5),d2
	lsr.w	#1,d2
	sub.w	d7,d2
	lsr.w	#1,d2
	tst.b	vb_interlace(a5)
	beq	.noin
	lsr.w	#1,d2
.noin:
	moveq	#0,d2
	move.w	vb_leny(a5),d2
	lsr.l	#1,d2
	lsl.l	#8,d2
	divu	d7,d2
	and.l	#$ffff,d2
	moveq	#0,d3
	moveq	#0,d5
.rep_dm:
	add.l	d2,d3
	move.l	d3,d4
	lsr.l	#8,d4
	move.l	d4,d6
	sub.l	d5,d4
	move.l	d6,d5
	subq.l	#1,d4
	move.l	vb_totalwidth(a5),d1
;	sub.l	vb_breedte_x(a5),d1
;	add.l	d1,d1
	mulu	d4,d1
	add.w	vb_moda(a5),d1
.same_put:
	bsr	set_modulo_a0
	bne	.exit
	bsr	set_modulo_a1
	bne	.exit
	subq.l	#1,d7
	bge	.rep_dm

	moveq	#0,d7
	move.l	(a2),d2
	move.w	vb_leny(a5),d7
	sub.w	d2,d7
.reppp:
	move.w	vb_moda(a5),d1
	move.l	vb_totalwidth(a5),d1
	add.l	d1,d1
	neg.w	d1
	add.w	vb_moda(a5),d1
	bsr	set_modulo_a0
	bne	.exit
	bsr	set_modulo_a1
	bne	.exit
	subq.l	#1,d7
	bge	.reppp
	rts

	move.l	(a2),d7
	lsr.l	#1,d7
	tst.w	d7
	beq	.exit
	move.w	vb_leny(a5),d2
	lsr.w	#1,d2
	sub.w	d7,d2
	lsr.w	#1,d2
	tst.b	vb_interlace(a5)
	beq	.noin2
	lsr.w	#1,d2
.noin2:
	moveq	#0,d2
	move.w	vb_leny(a5),d2
	lsr.l	#1,d2
	lsl.l	#8,d2
	divu	d7,d2
	and.l	#$ffff,d2
	moveq	#0,d3
	moveq	#0,d5
.rep_dm2:
	add.l	d2,d3
	move.l	d3,d4
	lsr.l	#8,d4
	move.l	d4,d6
	sub.l	d5,d4
	move.l	d6,d5
	subq.l	#1,d4
	move.w	vb_moda(a5),d1
	move.l	vb_totalwidth(a5),d1
	mulu	d4,d1
	add.w	vb_moda(a5),d1
.same_put2:
	bsr	set_modulo_a0
	bne	.exit
	bsr	set_modulo_a1
	bne	.exit
	subq.l	#1,d7
	bge	.rep_dm2
.exit:
	rts

set_to_line:
	move.l	d4,d3
	move.l	vb_totalwidth(a5),d1
	move.l	#$7fff,d5
	divu	d1,d5				; d5 the maximum delta posible
	and.l	#$ffff,d5
	lsr.l	#1,d5	
	cmp.l	d4,d5
	bge	.doit
	divu	d5,d4
	swap	d4
	and.l	#$ffff,d4
;	move.l	d5,d4
.doit:
	move.l	vb_totalwidth(a5),d1
	mulu	d4,d1
	add.w	vb_moda(a5),d1
	bsr	set_modulo_a0
	bne	.exit
	bsr	set_modulo_a1
	bne	.exit
.ready:
	moveq	#0,d0
	rts

.exit:
	moveq	#-1,d0
	rts
	
do_modus2b:
	move.l	lofm1(pc),a0
	move.l	shfm1(pc),a1

	moveq	#0,d4
	move.w	vb_leny(a5),d4
	sub.l	(a2),d4

; set rep to line d4 often this can't be done in one line

	bsr	set_to_line
	bne	.exit

	moveq	#0,d7
	move.l	(a2),d2
	move.w	vb_leny(a5),d7
	sub.w	d2,d7
.reppp:
	move.w	vb_moda(a5),d1
	move.l	vb_totalwidth(a5),d1
	add.l	d1,d1
	neg.w	d1
	add.w	vb_moda(a5),d1
	bsr	set_modulo_a0
	bne	.exit
	bsr	set_modulo_a1
	bne	.exit
	subq.l	#1,d7
	bge	.reppp

*
	move.l	(a2),d7
	lsr.l	#1,d7
	tst.w	d7
	beq	.exit

	moveq	#0,d2
	move.w	vb_leny(a5),d2
	lsr.l	#1,d2
	lsl.l	#8,d2
	bsr	divide_part

.exit:
	rts
	
lofm1:	dc.l	0
shfm1:	dc.l	0

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
	move.w	vb_moda_even(a5),6(a0)
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


*
* Drip1
*
cop_drip1:
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


try_ripple:
	move.l	modulo1(pc),d1
;	move.l	inc2(pc),d7
;	divu	#360,d7
;	swap	d7
;	and.l	#$ffff,d7

	moveq	#0,d3
	moveq	#0,d7
	move.w	vb_leny(a5),d2
.reptr:
	move.l	d7,d0
	bsr	get_easy_sinus
	move.l	d0,d4
	sub.l	d3,d0
	move.l	d0,(a2)+
	move.l	d4,d3
	add.l	inc2,d7
	cmp.l	#360,d7
	ble	.okie
	sub.l	#360,d7
.okie:
	subq.w	#1,d2
	bne	.reptr
	rts

inc2:		dc.l	1
modulo1:	dc.l	10
	
cop_drip3:
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
	move.l	#1,inc2
	move.l	#10,modulo1
.rep_test1:
	move.l	db_wpatroonruimte(a3),a2
	bsr	try_ripple
;	addq.l	#1,modulo1
	add.l	#1,inc2

	move.l	db_wpatroonruimte(a3),a2
	bsr	do_modus3

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
	cmp.l	#90,inc2
	ble	.rep_test1
;	bra	.rep_test1
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
.rep_test2:
	move.l	db_wpatroonruimte(a3),a2
	bsr	try_ripple
	subq.l	#1,modulo1
	sub.l	#10,inc2

	move.l	db_wpatroonruimte(a3),a2
	bsr	do_modus3

	move.l	db_graphbase(a3),a6
	move.l	itview(pc),a1
	jsr	_LVOLoadView(a6)

	bsr	wacht_tijd2
	tst.b	d0
	bne	.exit_cop_drip
	tst.l	inc2
	bge	.rep_test2

;	bra	.rep_test2
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

;test_dat:	blk.l	1000,0

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
	and.w		#~$880,d3
	bne		.nolow2
	and.w		#~SUPER_KEY,d0
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

;	bsr	calc_effect_speed_blitin

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
;	bsr	set_min_max_values
	bra	exit_wipein

exit_wipein2:
;	bsr	set_min_max_values

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

blitin_direct_rev:
	move.w	#$0faa,db_blitin_con0(a3)

	move.l	db_blitin_ox(a3),d0
	move.l	db_blitin_y(a3),d1

;	bsr	put_blitin_in3		; zet de blitin neer
	move.w	#$0fca,db_blitin_con0(a3)
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
;	bsr	adapt_blitin_exvert_moving
;	bsr	maak_mask_blitin	
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
;	bsr	set_blit_fast_direct
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
	bne.w	exit
	cmp.b	#$ff,vb_body_found(a5)
	bne	exit
	rts

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

;	move.l	a1,-(a7)
;	jsr	test_document
;	move.l	(a7)+,a1

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
sexit2:	bra	exit
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
;	bsr	calc_effect_speed_fade
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
;	bsr	calc_effect_speed_fade
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
	tst.b	db_crng_found(a3)
	beq	check_drng

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr1(pc),a0
	jsr	KPutStr
	bra	.tt
.dbstr1:	dc.b	"Color cycling 2",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

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
	bsr	wacht_tijd2
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
	move.l	vb_viewportw(a5),a0
	move.l	db_graphbase(a3),a6
	jsr	_LVOLoadRGB4(a6)
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

	
copy_err2:
	bra	copy_err	

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

	move.l	pd_Cmd(a1),d2		; pd command
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

	IFNE ESCLOOK
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.dbstr3(pc),a0
	jsr	KPutStr
	bra	.tt3
.dbstr3:	dc.b	"Go on key pressed",10,0
	even
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
	move.l	db_inactive_viewblok(a3),a4
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
	IFNE VIEWINFO
	movem.l	a0-a6/d0-d7,-(a7)
	lea	db_monid(a3),a1
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"Mode is prefs in %lx",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC
pppi8:

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

	IFNE VIEWINFO
	movem.l	a0-a6/d0-d7,-(a7)
	lea	db_colums(a3),a1
	lea	.dbstr1(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr1:	dc.b	"MAXx MAXy %d,%d",10,0
	even
.tt:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

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
;	bsr	color_wipe		; active nu kleur black
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
;	bsr	color_wipe		; wipe kleuren naar normaal
	rts

fade_to_first_active256:
	move.l	db_active_viewblok(a3),a5
	move.b	vb_colbytes+3(a5),d5
	move.b	vb_colbytes+4(a5),d6
	move.b	vb_colbytes+5(a5),d7

fade_to_f256_in:
	movem.l	d5-d7,-(a7)
	bsr	init_destcols256	; fade naar d5,d6,d7
;	bsr	color_wipe		; active nu kleur black
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
;	bsr	color_wipe		; wipe kleuren naar normaal
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
;	bsr	color_wipe		; active nu kleur black
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
;	bsr	color_wipe		; wipe kleuren naar normaal
	rts

fade_to_first_inactive256:
	move.l	db_inactive_viewblok(a3),a4
	move.b	vb_colbytes+3(a4),d5	; de kleur waarnaar te faden
	move.b	vb_colbytes+4(a4),d6
	move.b	vb_colbytes+5(a4),d7

fade_to_z256_in:
	movem.l	d5-d7,-(a7)
	bsr	init_destcols256	; fade naar d7
;	bsr	color_wipe		; active nu kleur black
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
;	bsr	color_wipe		; wipe kleuren naar normaal
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
	lea	vp_DxOffset(a0),a1
	lea	.dbstr(pc),a0
	jsr	KPutFmt
	bra	.tt
.dbstr:	dc.b	"Dx offset is %d,%d",10,0
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
	and.w	#~$880,d3
	bne	.nolow2
	and.w	#~SUPER_KEY,d0
	or.w	db_alternate_lowres(a3),d0
.nolow2:
	IFNE VIEWINFO
	movem.l	d0-d7/a0-a6,-(a7)
	lea	.test(pc),a1
	move.l	d0,(a1)
	lea	.dbstr2(pc),a0
	jsr	KPutFmt
	movem.l	(a7)+,d0-d7/a0-a6
	bra	.tt2
.dbstr2:	dc.b	"mode id is %lx",10,0
	even
.test:	dc.l	0
.tt2:
	ENDC

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

;	cmp.w	#FLIP_SIZE,d0
;	ble	.oke_check_size
;	move.l	#FLIP_SIZE,d0
;.oke_check_size:
	sub.w	d1,d0
	neg.w	d0

	asr.w	#1,d0
;	and.w	#$fffe,d0

	tst.b	vb_interlace(a5)
	bne.s	.inter5
	asr.w	#1,d0
.inter5:
	tst.w	d0
	bge	.okie
	moveq	#0,d0
.okie:
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
	bne	.tla
	tst.b	vb_interlace(a4)
	beq.w	.no_inter2
.tla:
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
	and.w	#$fffe,d1

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
	and.w	#$fffe,d1

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
	bne.b	no_camg_proc	

	move.w	vb_mode(a5),d2
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
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	dbstr4122(pc),a0
	jsr	KPutStr
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	vb_filenaam(a5),a1
.add_f:
	tst.b	(a1)+
	bne	.add_f				; add frame num to filename
	subq.l	#1,a1
	move.b	#'.',(a1)+
	move.b	#'f',(a1)+
	move.b	#'1',(a1)+
	move.b	#0,(a1)+

	move.l	db_mlmmubase(a3),a6
	move.l	vb_filenaam(a5),a1		; naam van de file mlmmulib
	jsr	_LVOMLMMU_FindMemBlk(a6)
	move.l	d0,vb_packed(a5)
	bne	file_loaded			; exit and close file

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
	move.l	#$10000,d1
	or.l	#MEMF_STAY,d1
	move.l	db_mlmmubase(a3),a6
	move.l	vb_filenaam(a5),a1		; naam van de file mpmmulib
	jsr	_LVOMLMMU_AllocMem(a6)
	move.l	d0,vb_packed(a5)

	tst.l	d0
	beq.w	exit
	move.l	d0,a0
	moveq	#12,d2
	bra	act_load

read_whole_file:
	move.l	db_mlmmubase(a3),a6
	move.l	vb_filenaam(a5),a1		; naam van de file mlmmulib
	jsr	_LVOMLMMU_FindMemBlk(a6)
	move.l	d0,vb_packed(a5)
	bne	file_loaded2
	
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
	bne.w	exit

	cmp.l	#'ANIM',4(a1)
	beq	load_first_frame_anim
	
	cmp.l	#'ILBM',4(a1)
	bne.w	exit

	move.l	(a1),d0
	addq.l	#8,d0
	move.l	d0,vb_packed_size(a5)

	moveq	#ERR_NO_FMEM,d7
	move.l	#$10000,d1
	or.l	#MEMF_STAY,d1
	move.l	db_mlmmubase(a3),a6
	move.l	vb_filenaam(a5),a1		; naam van de file mpmmulib
	jsr	_LVOMLMMU_AllocMem(a6)
	move.l	d0,vb_packed(a5)

	tst.l	d0
	beq.w	exit

	move.l	d0,a0
	moveq	#0,d2				; seek beginning
check_load:
act_load:
	move.l	vb_filehandle(a5),d1
	moveq	#-1,d3
	move.l	a0,-(a7)
	move.l	db_dosbase(a3),a6
	jsr	_LVOSeek(a6)		
	move.l	(a7)+,a0

* test of de file al in het geheugen zit

	move.l	db_mlmmubase(a3),a6
	move.l	vb_packed(a5),a1
	jsr	_LVOMLMMU_GetMemStat(a6)	; haal status van memblock op

	and.l	#MTF_INIT,d0
	bne	file_loaded

	move.l	vb_packed(a5),a1
	jsr	_LVOMLMMU_OwnMemBlk(a6)
	tst.l	d0
	beq	check_load

	move.l	vb_packed_size(a5),d3
	move.l	vb_packed(a5),d2
	move.l	vb_filehandle(a5),d1
	move.l	db_dosbase(a3),a6
	jsr	_LVORead(a6)		

	move.l	db_mlmmubase(a3),a6
	move.l	vb_packed(a5),a1
	moveq	#MTF_INIT,d0
	or.l	#MTF_SETCLR,d0
	jsr	_LVOMLMMU_SetMemStat(a6)	; file nu wel geladen

	move.l	vb_packed(a5),a1
	jsr	_LVOMLMMU_DisOwnMemBlk(a6)

* hele file nu in het geheugen 

file_loaded:
	bsr.w	close_file
file_loaded2:
	moveq	#0,d0
	rts
load_err1:
	moveq	#1,d7
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
unk:	dc.b	"unknown",0
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

	
	XDEF	_unload_file

_unload_file:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a3

	lea	db_fileblok1(a3),a5
	tst.l	vb_packed(a5)
	beq.s	no_rem
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

fout1:		dc.b	"File not found : ",0
fout2:		dc.b	"Not enough chip memory : ",0
fout3:		dc.b	"Not enough fast memory : ",0
fout4:		dc.b	"Resolution not supported : ",0
fout5:		dc.b	"General error",10,0
fout6:		dc.b	"File type no supported : ",0

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

; filenaam, effect speed, ltimes , wachttijd, wipenr, showtijd
;
; effect speed geeft vertikaal aan hoeveel rijen tegelijk ( 1 2 4 8 )
; ltimes geeft aan hoeveel keer tegelijk horizontaal een regel geblit wordt

W = 0
WIPE = 1
EP = 2
V = 1
VA = 0

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
	dc.l	file2,EP,V,VA,0, 0,WIPE,W
	dc.l	0


file1:	dc.b	"pics:esther1",0
file2:	dc.b	"pics:esther2",0
;file1:	dc.b	"pics:8bit/cindy11",0
;file2:	dc.b	"hd3:data/pictures/8bit/cindy11",0
;file1:	dc.b	"hd3:data/pictures/allrestestfiles2/trans1.l",0
;file2:	dc.b	"hd3:data/pictures/allrestestfiles2/trans2.l",0
;file3:	dc.b	"pics:testbeeld32",0
;file2:	dc.b	"pics:8bit/yos.8",0


	even
	INCDIR	"wp:asm/"
	INCLUDE "eff_nums.s"
	ENDC
	even
	INCDIR	"wp:asm/"

	INCLUDE	"blokdata1.s"
mt:	ds.l	580			; allocmem for this ????????????????

	END
