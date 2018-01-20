*
* Create animtocdxl 15-11-1993
*
* Updated with anim8 11-05-1993
*
* 7 december 1992
*
* Create a universal player wich can handle
*
* CMAP changes 
* BMHD changes ( screen format changes )
* Variant speeds
* ( Possibly other formats than the anim-5 format )
* 7-12-1992 anim-5 XOR mode
*
* Try double-buffered viewports to play an anim
* 28 august 1992
*
* ANIM reader modification 1-1-1991 23-05-1991
*
* ANIM reader by Cees Lieshout 14-april-1990
* 
*

	INCDIR	"include:"
	INCLUDE	"devices/timer.i"
	INCLUDE "exec/execbase.i"
	INCLUDE "exec/ports.i"
	INCLUDE "exec/memory.i"
	INCLUDE	"exec/lists.i"
	INCLUDE	"exec/libraries.i"

	INCDIR	"wp:inclibs/"
	INCLUDE "dos_lib.i"
	INCLUDE "mathffp_lib.i"
	INCLUDE "mathtrans_lib.i"
	INCLUDE "exec_lib.i"
	INCLUDE "layers_lib.i"
	INCLUDE "intuition_lib.i"
	INCLUDE "graphics_libv39.i"
	INCLUDE	"mpmmu_lib.i"

	INCDIR	"include:"
	INCLUDE "graphics/gfxbase.i"
	INCLUDE "graphics/displayinfo.i"
	INCLUDE	"dos/dos.i"

	INCDIR	"include:"
	INCLUDE "graphics/view.i"
	INCLUDE "graphics/rastport.i"
	INCLUDE "graphics/gfx.i"
	INCLUDE "graphics/sprite.i"
	INCLUDE "graphics/copper.i"
	INCLUDE	"graphics/layers.i"
	INCLUDE	"graphics/clip.i"
	INCLUDE "hardware/custom.i"
	INCLUDE	"intuition/intuitionbase.i"
	INCLUDE "intuition/preferences.i"


	INCDIR	"wa:asm/"
	INCLUDE	"anhd.i"
	INCLUDE	"anim_infonew.i"
	INCDIR	"wa:asm/convert/"
	INCLUDE	"cd2an_data.i"
	INCDIR	"wp:asm/"
	INCLUDE	"errors.i"
	INCLUDE	"proces.i"
	INCLUDE	"system.i"
	INCDIR	"cdxl:asm/"
	INCLUDE	"pan.i"

DEBUG = 0

MEMF_STAY = $8000
MTF_INIT = 1
MTF_SETCLR = $80000000

LIBV39 = 39
LIBV36 = 36

exec=4

mode_old=1005
mode_new=1006
key = $bfec01

MEMSIZE = 230000
MAX_MULTABEL = 580

;	bra	start

	IFNE	DEBUG
	XREF	KPutFmt
	XREF	KPutStr
	PRINTT	"De debug info staat aan dus je moet nog met debug.lib linken"
	ENDC

top
	lea	datablock(pc),a3
	move.b	#1,ab_show(a3)
	move.l	#-1,ab_num_rot(a3)
	subq	#1,d0
	beq.W	input_error
zoeken:
	cmp.b	#$20,(a0)+
	bne.s	gevonden
	dbra	d0,zoeken
	bra.W	input_error
gevonden:
	subq.l	#1,a0
	move.l	a0,ab_filenaam(a3)
zet:	cmp.b	#10,(a0)
	beq.w	zetnul
	cmp.b	#' ',(a0)
	beq.s	zetnul2

	addq.l	#1,a0
	bra.s	zet
zetnul2:
	move.b	#0,(a0)	; maak van de spatie na de ab_filenaam(a3) een nul
	addq.l	#1,a0	; a0 naar volgende commando

get_nextpar:
	tst.b	(a0)
	beq.w	no_in
	cmp.b	#'l',(a0)
	bne.b	no_loop
	move.b	#1,ab_looping(a3)
	addq.l	#1,a0
no_loop:
	cmp.b	#"i",(a0)
	bne.b	no_info
	move.b	#1,ab_info(a3)
	addq.l	#1,a0
	bra.b	get_nextpar
no_info:
	cmp.b	#"s",(a0)
	bne.b	no_speed
	moveq	#0,d0
	addq.l	#1,a0
	move.b	(a0),d0
	cmp.b	#'0',d0
	blt.b	get_nextpar
	cmp.b	#'9',d0
	bgt.b	get_nextpar
	sub.l	#'0',d0
	addq.l	#1,a0
	move.l	d0,ab_frame_speed(a3)
	move.l	d0,ab_overallspeed(a3)
	tst.b	(a0)
	beq.w	no_in
	move.b	(a0),d1
	cmp.b	#'0',d1
	blt.b	get_nextpar
	cmp.b	#'9',d1
	bgt.b	get_nextpar
	sub.b	#'0',d1
	mulu	#10,d0
	add.b	d1,d0
	move.l	d0,ab_frame_speed(a3)
	move.l	d0,ab_overallspeed(a3)
no_speed:
	addq.l	#1,a0
	bra.w	get_nextpar

	bra.b	no_in
zetnul:
	move.b	#0,(a0)
	bra.s	no_in

start:
	lea	datablock(pc),a3
	lea	filename(pc),a0
	move.l	a0,ab_filenaam(a3)
	move.b	#1,ab_show(a3)
	move.l	#320,ab_lenx(a3)
	move.l	#256,ab_leny(a3)
	move.w	#5,ab_planes(a3)
	move.l	#40,ab_breedte_x(a3)
	move.l	#51200,ab_unpacked_size(a3)

no_in:
	move.b	#$0,ab_lize(a3)
	move.l	#0,ab_frames(a3)
	move.w	#640,ab_columns(a3)
	move.w	#256,ab_rows(a3)
	lea	ab_view1(a3),a0
	move.l	a0,ab_wview1(a3)
	lea	ab_view2(a3),a0
	move.l	a0,ab_wview2(a3)
	move.l	a7,ab_easy_exit(a3)

	bsr.w	openlibs

	bsr.w	setprefs

	bsr.w	read_whole_file

	lea	ab_buffer(a3),a1
no_ch1:
	cmpi.l	#"ANIM",8(a1)
	bne.w	no_anim

; reserveer geheugen voor tweede scherm

	tst.l	ab_unpackedsecond(a3)
	bne.b	already_assigned_second

	move.l	exec,a6
	move.l	#MEMSIZE,d0
	move.l	#$10002,d1
	jsr	_LVOAllocMem(a6)
	move.l	#1,d7
	move.l	d0,ab_unpackedsecond(a3)
	beq.w	exit

already_assigned_second:
	move.l	#$10002,d1
	move.l	exec,a6
	move.l	#MEMSIZE,d0
	jsr	_LVOAllocMem(a6)
	move.l	d0,ab_unpacked(a3)
	beq.w	exit

	move.l	ab_unpacked(a3),ab_frame_hidden(a3)
	move.l	ab_unpackedsecond(a3),ab_frame_show(a3)

	move.l	ab_graphbase(a3),a6
	move.l	gb_ActiView(a6),ab_oldview(a3)

	move.l	ab_wview2(a3),ab_view_visible(a3)
	move.l	ab_wview1(a3),ab_view_hidden(a3)

	bsr	find_first_FORM

	bsr.w	find_disk_FORMs

aaa:
	tst.b	ab_quit(a3)
	bne	exit_wi2

	tst.l	ab_num_rot(a3)
	beq	exit_wi2
	move.l	#0,ab_frame_counter(a3)
	move.l	ab_filehandle(a3),d1
	move.l	#0,d2
	move.l	#-1,d3					; start
	move.l	ab_dosbase(a3),a6
	jsr	_LVOSeek(a6)

	tst.b	ab_looping(a3)
	bne.b	ww

	move.l	ab_head_frame_list(a3),a0

	move.l	ani_next(a0),a0
	move.l	ani_next(a0),a0		; skip first two frames ??

	move.l	#2,ab_frame_counter(a3)

	move.l	a0,ab_frame_pointer(a3)
	move.l	#0,ab_relfpointer(a3)

ww:
	bsr.w	create_next_frame

	tst.b	ab_show(a3)
	bne	no_c
	moveq	#1,d7
	tst.b	ab_picture(a3)
	bne	oke_c2
	moveq	#2,d7
oke_c2:
	cmp.l	ab_frame_counter(a3),d7
	bne	no_c
	move.b	#1,ab_show(a3)
no_c:
	cmp.l	#1,ab_num_rot(a3)
	bne	no_one

	move.l	ab_frame_counter(a3),d0
	cmp.l	ab_dpan_count(a3),d0
	beq	exit_wi2

no_one:
	cmp.l	#0,ab_num_rot(a3)
	bmi.b	set_again2
	beq.b	exit_wi2
	btst	#6,$bfe001
	bne.b	ww
set_again2:
	move.l	#-1,ab_num_rot(a3)
	btst	#6,$bfe001
	bne.b	ww
exit_wi2:
;	btst	#6,$bfe001
;	bne.b	exit_wi2

no_anim:
	moveq	#0,d7
	bra.w	exit

input_error:
	rts

fill_in_pan_block:
	lea	panblock(pc),a0
	move.b	#1,pan_Type(a0)
	move.b	#0,pan_Res(a0)
	move.w	ab_lenx(a3),pan_XSize(a0)
	move.w	ab_leny(a3),pan_YSize(a0)
	move.w	ab_planes(a3),d0
	move.b	d0,pan_PixelSize(a0)
	move.w	#0,pan_AudioSize(a0)
	move.w	ab_mode(a3),d2
	and.w	#$800,d2			; check for ham
	beq	.no_ham
	move.b	#1,pan_Info(a0)			; set ham info bit
	cmp.w	#6,d0
	beq	.ham6
; should be ham8
	moveq	#6,d0
	bra	.no_ham
.ham6:
	moveq	#4,d0
.no_ham:
	moveq	#1,d1
	lsl.w	d0,d1
	add.w	d1,d1
	move.w	d1,pan_ColorMapSize(a0)
	ext.l	d1
	move.l	d1,ab_colormapsize(a3)
	tst.b	ab_nonleaved(a3)
	beq	.leaved
	or.b	#PIF_LINES,pan_Info(a3)
	rts
.leaved:
	rts

open_cdxl_out:
	moveq	#3,d7
	move.l	ab_filenaam_out(a3),d1
	move.l	#mode_new,d2
	move.l	ab_dosbase(a3),a6
	jsr	_LVOOpen(a6)
	move.l	d0,ab_filehandle_out(a3)
	beq.w	exit
	rts

close_cdxl_out:
	move.l	ab_dosbase(a3),a6
	move.l	ab_filehandle_out(a3),d1
	beq.b	.no_closefile
	jsr	_LVOClose(a6)
	clr.l	ab_filehandle_out(a3)
.no_closefile:
	rts

write_cdxl_header:
	lea	panblock(pc),a0
	move.l	a0,d2
	move.l	ab_filehandle_out(a3),d1
	move.l	#pan_SIZEOF,d3
	move.l	ab_dosbase(a3),a6
	jsr	_LVOWrite(a6)
	rts
*
* In a0 the pointer to the frame
*
write_cdxl_frame:
	move.l	a0,d2
	move.l	ab_filehandle_out(a3),d1
	move.l	ab_unpacked_size(a3),d3
	move.l	ab_dosbase(a3),a6
	jsr	_LVOWrite(a6)
	rts

write_cdxl_cmap:
	move.l	ab_col_pointer(a3),d2
	move.l	ab_colormapsize(a3),d3
	move.l	ab_filehandle_out(a3),d1
	move.l	ab_dosbase(a3),a6
	jsr	_LVOWrite(a6)
	rts

write_cdxl_audio:
	move.l	ab_col_pointer(a3),d2
	move.l	ab_audiosize(a3),d3
	beq	.no_write_audio
	move.l	ab_filehandle_out(a3),d1
	move.l	ab_dosbase(a3),a6
	jsr	_LVOWrite(a6)
.no_write_audio:
	rts
*
* Convert from interleaved to non-interleaved
*
convert_int_non:
	movem.l	d0-d7/a0-a6,-(a7)
	move.b	#$ff,ab_nonleaved(a3)	; set the nonleaved byte

	move.w	ab_planes(a3),d7
	cmp.w	#1,d7
	beq.w	no_convert

	move.l	ab_breedte_x(a3),d6
	move.l	d6,d5
;	subq.w	#2,d7
	mulu	d7,d6			; total width in bytes of
					; interleaved line

	move.l	ab_frame_show(a3),a0	; data from
	move.l	ab_frame_hidden(a3),a1	; data to

	move.l	a0,a5			; temporary from
	move.l	a0,a6			; temporary from


	move.w	ab_planes(a3),d4	; width picture
	subq.w	#1,d4
rep_h1:

	move.l	a6,a5
	move.w	ab_leny(a3),d7		; height picture
	subq.w	#1,d7
rep_pl1:


	move.l	ab_breedte_x(a3),d5	; no planes
	move.l	a5,a0
	subq.w	#1,d5
rep_b1:
	move.b	(a0)+,(a1)+
	dbf	d5,rep_b1		; copy one line


	add.l	d6,a5			; goto next line
	
	dbf	d7,rep_pl1

	add.l	ab_breedte_x(a3),a6	; goto next plane
	dbf	d4,rep_h1

	move.l	ab_view_hidden(a3),a5		; change hidden
	bsr.w	freememory
	move.l	ab_frame_hidden(a3),ab_w_bitpointer(a3)
	bsr.w	maak_viewport			; create new non leaved view
	bsr.w	change_view2
	movem.l	(a7)+,d0-d7/a0-a6
	rts

	IFNE DEBUG
dbstr1:	dc.b	"In convert to interleaved",10,0
	even
	ENDC
*
* Convert from non-interleaved to interleaved
*
convert_non_int:
	movem.l	d0-d7/a0-a6,-(a7)

	move.b	#$0,ab_nonleaved(a3)	; set the nonleaved byte

	move.w	ab_planes(a3),d7
	cmp.w	#1,d7
	beq.b	no_convert

	move.l	ab_breedte_x(a3),d6
	move.l	d6,d5
	mulu	d7,d6			; total width in bytes of
					; interleaved line

	move.l	ab_frame_hidden(a3),a0	; data to
	move.l	ab_frame_show(a3),a1	; data from

	move.l	a0,a5			; temporary from
	move.l	a0,a6			; temporary from


	move.w	ab_planes(a3),d4	; width picture
	subq.w	#1,d4
rep_h2:

	move.l	a6,a5
	move.w	ab_leny(a3),d7		; height picture
	subq.w	#1,d7
rep_pl2:
	move.l	ab_breedte_x(a3),d5	; no planes
	move.l	a5,a0
	subq.w	#1,d5
rep_b2:
	move.b	(a1)+,(a0)+
	dbf	d5,rep_b2		; copy one line

	add.l	d6,a5			; goto next line
	
	dbf	d7,rep_pl2

	add.l	ab_breedte_x(a3),a6	; goto next plane
	dbf	d4,rep_h2

	move.l	ab_view_hidden(a3),a5		; change hidden
	bsr.w	freememory
	move.l	ab_frame_hidden(a3),ab_w_bitpointer(a3)
	bsr.w	maak_viewport			; create new non leaved view
	bsr.w	change_view2
no_convert:
	movem.l	(a7)+,d0-d7/a0-a6
	rts
	
verstring:	dc.b	0,"$VER:CDXL2ANIM 1.0 © MediaPoint",0
	even

exit112:
	tst.l	ab_temp_formpointer(a3)
	beq.b	no_free_poi2
	move.l	ab_temp_formdisksize(a3),d0
	move.l	ab_temp_formpointer(a3),a1
	move.l	$4,a6
	jsr	_LVOFreeMem(a6)
no_free_poi2:
	bsr.w	free_frame_list
	move.l	ab_temp_frame_list(a3),d0
	beq.b	no_t_frame
	move.l	d0,a1
	bsr.w	free_frame
no_t_frame:
	move.b	#0,ab_loaded(a3)
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

setprefs:
	move.l	ab_intbase(a3),a6
	lea	ab_multabel(a3),a0
	move.l	#400,d0
	jsr	_LVOGetPrefs(a6)

	lea	ab_multabel(a3),a0

	move.b	pf_ViewXOffset(a0),ab_Xoff(a3)

	moveq	#0,d0
	move.b	pf_ViewYOffset(a0),d0	; adjust this to a minimum ?????
	cmp.b	#$4,d0
	bge	.no_adj
	move.b	#$4,pf_ViewYOffset(a0)
.no_adj:

	move.b	pf_ViewYOffset(a0),ab_Yoff(a3)

	move.l	ab_graphbase(a3),a6
	move.w	gb_NormalDisplayColumns(a6),ab_columns(a3)
	move.w	gb_NormalDisplayRows(a6),ab_rows(a3)

	move.l	ab_graphbase(a3),a6
	move.w	LIB_VERSION(a6),d0
	move.b	d0,ab_libversion(a3)
	cmp.w	#LIBV39,d0
	blt.b	no_v39_1
	move.b	#$ff,ab_v39_present(a3)
no_v39_1:
	cmp.w	#LIBV36,d0
	blt.b	no_v36				; er is geen chiprevbits

	move.b	gb_ChipRevBits0(a6),d0
	and.b	#SETCHIPREV_AA,d0
	cmp.b	#SETCHIPREV_AA,d0
	bne.b	no_aa1
	move.b	#$ff,ab_aa_present(a3)
no_aa1:

no_v36:
	move.l	$4,a6
	move.w	AttnFlags(a6),d0
	and.l	#AFF_68020,d0
	beq.b	no_cpu1
	move.b	#1,ab_more_cpu(a3)
no_cpu1:
	rts

own_wait:
	btst	#6,$bfe001
	bne	.no_q
	move.b	#$ff,ab_quit(a3)
	moveq	#-1,d0
	rts
.no_q:
	moveq	#0,d0
	rts

change_view:
	addq.l		#1,ab_frames(a3)	; frames played

	bsr.w		own_wait

change_view2:

	move.l		ab_graphbase(a3),a6
	move.l		ab_view_visible(a3),a1
	lea		vi_viewport(a1),a0
	jsr		_LVOWaitBOVP(a6)

	move.l		ab_graphbase(a3),a6
	move.l		ab_view_hidden(a3),a1
	jsr		_LVOLoadView(a6)

	tst.b		ab_interlace(a3)
	beq.b		no_wait
	move.l		ab_graphbase(a3),a6
	jsr		_LVOWaitTOF(a6)
no_wait:
	tst.b		ab_lize(a3)
	bne.b		yes_lize

	move.l		ab_view_visible(a3),d0
	move.l		ab_view_hidden(a3),ab_view_visible(a3)
	move.l		d0,ab_view_hidden(a3)

	move.l		ab_frame_show(a3),d0
	move.l		ab_frame_hidden(a3),ab_frame_show(a3)
	move.l		d0,ab_frame_hidden(a3)
yes_lize:
	moveq		#0,d0
	rts

maak_viewport:
	lea		vi_view(a5),a1
	jsr		_LVOInitView(a6)

	lea		vi_viewport(a5),a0
	jsr		_LVOInitVPort(a6)

	lea		vi_view(a5),a0
	lea		vi_viewport(a5),a1
	move.l		a1,v_ViewPort(a0)
	move.w		ab_mode(a3),v_Modes(a0)

	lea		vi_bitmap(a5),a0
	move.w		ab_planes(a3),d0			; depth

	move.l		ab_breedte_x(a3),d1		; breedte in bytes * 8
	tst.b	ab_nonleaved(a3)
	bne.b	no_modu
	mulu		ab_planes(a3),d1
no_modu:
	lsl.l		#3,d1				; width
	move.w		ab_leny(a3),d2			; height
	jsr		_LVOInitBitMap(a6)

	lea		vi_bitmap(a5),a0

	move.l		ab_w_bitpointer(a3),d0

	move.l		ab_breedte_x(a3),d1
	tst.b		ab_nonleaved(a3)
	beq.b		no_modu2
	mulu		ab_leny(a3),d1
no_modu2:
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
	lea		vi_bitmap(a5),a1
	move.l		a1,ri_BitMap(a0)
	move.w		#0,ri_RxOffset(a0)
	move.w		#0,ri_RyOffset(a0)
	move.l		#0,ri_Next(a0)

	lea		vi_viewport(a5),a0
	lea		vi_rasinfo(a5),a1
	move.l		a1,vp_RasInfo(a0)
	move.w		ab_lenx(a3),vp_DWidth(a0)
	move.w		ab_leny(a3),vp_DHeight(a0)
	move.w		ab_mode(a3),vp_Modes(a0)

	move.w	ab_columns(a3),d1
	tst.b	ab_hires(a3)
	bne.b	hi1
	lsr.w	#1,d1
hi1:
	move.w	ab_lenx(a3),d0	; pref.offset-(breedte - normal.breedte)/2

; test de offset berekening met de byte lengte

	move.l	ab_breedte_x(a3),d0
	lsl.l	#3,d0
	sub.w	d1,d0
	asr.w	#1,d0
	neg.w	d0
	move.b	ab_Xoff(a3),d1	; standaard breedte
	ext.w	d1
	tst.b	ab_hires(a3)
	bne.b	hi2
	asr.w	#1,d1
	bra	hi3
hi2:
	tst.b	ab_aa_present(a3)
	beq	.no_aa
	subq.w	#1,d0
.no_aa:
hi3:
	add.w	d1,d0
	move.w	d0,vp_DxOffset(a0)


	move.w	ab_rows(a3),d1
	tst.b	ab_interlace(a3)
	beq.b	no_inter
	lsl.w	#1,d1
no_inter:
	move.w	ab_leny(a3),d0
	sub.w	d1,d0
	asr.w	#1,d0
	neg.w	d0

	btst	#0,d0
	beq.b	no_onev1
	addq.w	#1,d0
no_onev1:

	move.b	ab_Yoff(a3),d1
	ext.w	d1
	tst.b	ab_interlace(a3)
	beq.b	no_inter2
	asl.w	#1,d1
;	add.w	#4,d0
no_inter2:
;	add.w	#4,d0
	add.w	d1,d0

	move.w	d0,vp_DyOffset(a0)	

	tst.b	ab_aa_present(a3)
	beq.b	no_aa
	addq.w	#1,vp_DyOffset(a0)

	move.l		#256,d0
	jsr		_LVOGetColorMap(a6)
	lea		vi_viewport(a5),a0
	move.l		d0,vp_ColorMap(a0)
	beq.w		no_colormap

	lea		vi_viewport(a5),a0
	move.l		ab_col_pointer(a3),a1
	move.l		ab_graphbase(a3),a6
	jsr		-$372(a6)			; loadrgb32

	bra.b		aa	
no_aa:
	moveq		#32,d0
	jsr		_LVOGetColorMap(a6)
	lea		vi_viewport(a5),a0
	move.l		d0,vp_ColorMap(a0)
	beq.w		no_colormap

	lea		vi_viewport(a5),a0
	move.l		ab_col_pointer(a3),a1
	moveq		#32,d0
	jsr		_LVOLoadRGB4(a6)

aa:
	lea		vi_view(a5),a0
	lea		vi_viewport(a5),a1
	jsr		_LVOMakeVPort(a6)

	lea		vi_view(a5),a1
	jsr		_LVOMrgCop(a6)

	rts
	
reset_oldview:
	tst.l		ab_oldview(a3)
	beq.b		no_oldview
	move.l		ab_graphbase(a3),a6
	move.l		ab_oldview(a3),a1
	jsr		_LVOLoadView(a6)
	jsr		_LVOWaitTOF(a6)
no_oldview:
	rts

freememory:
	move.l		ab_graphbase(a3),a6
	lea		vi_viewport(a5),a0
	tst.l		vp_ColorMap(a0)
	beq.b		no_colormem
	move.l		vp_ColorMap(a0),a0
	jsr		_LVOFreeColorMap(a6)
	lea		vi_viewport(a5),a0
	clr.l		vp_ColorMap(a0)
no_colormem:
	lea		vi_viewport(a5),a0
	jsr		_LVOFreeVPortCopLists(a6)

	lea		vi_view(a5),a0
	tst.l		v_LOFCprList(a0)
	beq.b		no_lofmem
	move.l		v_LOFCprList(a0),a0
	jsr		_LVOFreeCprList(a6)
	lea		vi_view(a5),a0
	clr.l		v_LOFCprList(a0)
no_lofmem:

	lea		vi_view(a5),a0
	tst.l		v_SHFCprList(a0)
	beq.b		no_shfmem
	move.l		v_SHFCprList(a0),a0
	jsr		_LVOFreeCprList(a6)
	lea		vi_view(a5),a0
	clr.l		v_SHFCprList(a0)
no_shfmem:

no_colormap:
	moveq		#0,d0
	rts

init_anim:
	move.w	ab_planes(a3),d7
	move.l	ab_breedte_x(a3),d0
	mulu	d7,d0
	move.l	d0,ab_rowmulti(a3)

* create a mul-table for the anim play

	move.l	d0,d1
	lea	ab_multabel(a3),a0
	move.w	ab_leny(a3),d7
	cmp.w	#MAX_MULTABEL,d7
	ble.b	no_mpro2
	move.w	#MAX_MULTABEL,d7
no_mpro2:
	move.l	#0,(a0)+
	subq.w	#1,d7
rep_make_mult:
	move.l	d0,(a0)+
	add.l	d1,d0
	dbf	d7,rep_make_mult

	rts
*
* Free the total frame_list
*
free_frame_list:
	tst.l	ab_head_frame_list(a3)
	beq.b	exit_free_list
	move.l	ab_head_frame_list(a3),a1
rep_free_list
	cmp.l	#0,a1
	beq.b	exit_free_list
	move.l	ani_next(a1),a4		; get next pointer
	bsr.b	free_frame
	move.l	a4,a1
	bra.b	rep_free_list
exit_free_list:
	move.l	#0,ab_head_frame_list(a3)
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
	move.l	ani_cmap(a1),d0
	beq.b	no_cmap_free

	move.l	a1,-(a7)
	move.l	d0,a1
	move.l	$4,a6
	moveq	#0,d0
	move.w	(a1),d0
	tst.b	ab_aa_present(a3)
	beq.b	no_aa2
	move.l	d0,d1
	add.l	d0,d0
	add.l	d1,d0
	lsl.l	#2,d0
	add.l	#12,d0
	bra.b	aa2
no_aa2:
	add.l	d0,d0
	addq.l	#2,d0
aa2:
	jsr	_LVOFreeMem(a6)
	move.l	(a7)+,a1
no_cmap_free:
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
	move.l	#0,ab_FORM_counter(a3)
	move.l	#"FORM",d2	

* Allocate head pointer
*

	bsr.w	alloc_frame
	move.l	d0,ab_head_frame_list(a3)
	beq.w	frame_alloc_fail


	move.l	d0,a0
	move.l	d0,a4

	move.l	ab_packed(a3),a1
	move.l	a1,a2
	addq.l	#8,a1
	addq.l	#4,a1			; skip the ILBM sign
	add.l	ab_packed_size(a3),a2	; end data
fza1:
	cmp.l	a2,a1
	bge.b	exitfind_f

	move.l	(a1),d3
	cmp.l	d2,d3
	bne.s	fza2

	addq.l	#1,ab_FORM_counter(a3)

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

	move.l	ab_head_frame_list(a3),a0
	move.l	a0,ab_frame_pointer(a3)

	move.l	ab_FORM_counter(a3),ab_dpan_count(a3)
	tst.b	ab_looping(a3)
	bne.b	no_loop4
	subq.l	#2,ab_dpan_count(a3)		; the anim is looping so sub 2
no_loop4:
	rts

*
* Skip two forms for the continues diskanim
* Keep record of the absolute seek pointer
*
skip_one_form:
	moveq	#0,d7
	bra.b	rr_skip
	
skip_two_forms:
	moveq	#1,d7
rr_skip:
	lea	ab_buffer(a3),a0
	move.l	a0,d2
	moveq	#8,d3
	move.l	ab_filehandle(a3),d1
	move.l	ab_dosbase(a3),a6
	jsr	_LVORead(a6)
	cmp.l	#8,d0
	bne.w	file_fail
	lea	ab_buffer+4(a3),a0

	move.l	(a0),d2
	move.l	ab_filehandle(a3),d1
	moveq	#0,d3					; start
	move.l	ab_dosbase(a3),a6
	jsr	_LVOSeek(a6)

	lea	ab_buffer+4(a3),a0
	move.l	(a0),d0
	addq.l	#8,d0
	add.l	d0,ab_global_diskoffset(a3)
	dbf	d7,rr_skip
	rts
*
* Load the next FORM from disk
* the ab_filehandle(a3) is valid
*
load_next_form:
	tst.l	ab_temp_formpointer(a3)
	beq.b	no_free_poi
	move.l	ab_temp_formdisksize(a3),d0
	move.l	ab_temp_formpointer(a3),a1
	move.l	$4,a6
	jsr	_LVOFreeMem(a6)
	clr.l	ab_temp_formpointer(a3)
no_free_poi:
	lea	ab_buffer(a3),a0
	move.l	a0,d2
	moveq	#8,d3
	move.l	ab_filehandle(a3),d1
	move.l	ab_dosbase(a3),a6
	jsr	_LVORead(a6)
	cmp.l	#8,d0
	bne.b	file_fail
	lea	ab_buffer+4(a3),a0
	move.l	(a0),d0
	addq.l	#8,d0
	move.l	d0,ab_temp_formdisksize(a3)

	moveq	#MEMF_PUBLIC,d1
	move.l	$4,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,ab_temp_formpointer(a3)
	beq.b	file_mem_fail
	move.l	d0,a0
	lea	ab_buffer(a3),a1
	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+

	move.l	d0,d2
	move.l	ab_temp_formdisksize(a3),d3
	subq.l	#8,d3
	addq.l	#8,d2
	move.l	ab_dosbase(a3),a6
	move.l	ab_filehandle(a3),d1
	jsr	_LVORead(a6)
	move.l	ab_temp_formdisksize(a3),d3
	subq.l	#8,d3
	cmp.l	d3,d0
	bne.b	file_fail	
	move.l	ab_temp_formpointer(a3),d0
	rts
file_fail:
	moveq	#0,d0
	rts

file_mem_fail:
	moveq	#1,d7
	bra.w	exit
*
* Search for all the FORMs in the file which is on disk
*
find_disk_FORMs:

	tst.l	ab_temp_frame_list(a3)
	beq.b	no_head_list1

	move.b	#1,ab_show(a3)

	tst.b	ab_continue(a3)
	bne.b	no_continue6

	move.l	ab_temp_frame_list(a3),a0
	move.l	a0,ab_head_frame_list(a3)		; set temp list, first
	clr.l	ab_temp_frame_list(a3)

	move.l	ab_head_frame_list(a3),d0
	move.l	d0,a0
	move.l	d0,a4

	move.l	ab_packed(a3),a1
	lea.l	12(a1),a1
	bra.b	from_c1

	bra.b	dza1
	
no_continue6:
	bra.b	no_continue5

no_head_list1:
	move.l	#12,ab_global_diskoffset(a3)
	move.l	#0,ab_FORM_counter(a3)

	tst.b	ab_continue(a3)
	beq.b	no_continue5

	bsr.w	skip_two_forms

	move.b	#1,ab_show(a3)
	addq.l	#2,ab_frame_counter(a3)
	addq.l	#2,ab_FORM_counter(a3)

no_continue5:

* Allocate head pointer
*
	bsr.w	alloc_frame
	move.l	d0,ab_head_frame_list(a3)
	beq.w	frame_alloc_fail

	move.l	d0,a0
	move.l	d0,a4

dza1:
	move.l	a0,-(a7)
	bsr.w	load_next_form
	move.l	(a7)+,a0
	addq.l	#1,ab_frame_counter(a3)
	addq.l	#1,ab_FORM_counter(a3)

	tst.l	d0
	beq.w	exitfind_d
	move.l	d0,a1

from_c1:
	move.l	4(a1),d7
	movem.l	a0/d7,-(a7)
	move.l	a0,a4			; remember previous pointer
	bsr.w	create_next_frame1

	tst.b	ab_show(a3)
	bne.b	oke_show5
	moveq	#1,d7
	tst.b	ab_picture(a3)
	bne.b	oke_pic1
	moveq	#2,d7
oke_pic1:
	cmp.l	ab_frame_counter(a3),d7
	bne.b	oke_show5
	move.b	#1,ab_show(a3)
oke_show5:
	movem.l	(a7)+,a0/d7
	addq.l	#8,d7
	add.l	d7,ab_global_diskoffset(a3)
	move.l	#0,ani_form(a4)

	move.l	a0,a4			; remember previous pointer
	bsr.w	alloc_frame
	move.l	d0,ani_next(a0)
	beq.w	frame_alloc_fail
	move.l	d0,a0

	tst.b	ab_quit(a3)
	bne.b	exitfind_d

	cmp.l	#1,ab_num_rot(a3)
	bne.b	no_pro4
	tst.l	ab_dpan_count(a3)
	beq.b	no_pro4
	move.l	ab_frame_counter(a3),d7

	move.b	#1,ab_quit(a3)
	cmp.l	ab_dpan_count(a3),d7
	beq.b	exitfind_d
	move.b	#0,ab_quit(a3)
no_pro4:
;	btst	#6,$bfe001
;	beq.b	exitfind_d
	bra.w	dza1

exitfind_d:
	subq.l	#1,ab_num_rot(a3)
	move.l	#0,ani_next(a4)	; clear last frame
	move.l	a0,a1
	bsr.w	free_frame
	move.l	ab_head_frame_list(a3),a0
	move.l	a0,ab_frame_pointer(a3)
	subq.l	#1,ab_FORM_counter(a3)

	tst.l	ab_dpan_count(a3)
	bne.b	no_loop5
	
	move.l	ab_FORM_counter(a3),ab_dpan_count(a3)
	tst.b	ab_looping(a3)
	bne.b	no_loop5
	subq.l	#2,ab_dpan_count(a3)		; the anim is looping so sub 2
no_loop5:
	rts

*
* Search for all the FORMs in the file which is on disk
*
find_first_FORM:
	move.l	#0,ab_FORM_counter(a3)
	move.l	#12,ab_global_diskoffset(a3)

	move.b	#$ff,ab_diskloaded(a3)
	move.b	#$ff,ab_show(a3)
	
	bsr.w	alloc_frame
	move.l	d0,ab_temp_frame_list(a3)
	beq.b	frame_alloc_fail
	move.l	d0,a0
	move.l	d0,a4

	move.l	a0,-(a7)
	bsr.w	load_next_form
	move.l	(a7)+,a0
	move.l	d0,ab_packed(a3)
	sub.l	#12,ab_packed(a3)

	tst.l	d0
	beq.b	exitfind_d2

	move.l	d0,a1
	move.l	4(a1),d7

	tst.b	ab_continue(a3)
	beq.b	no_cont10
	addq.l	#8,d7
	add.l	d7,ab_global_diskoffset(a3)

no_cont10:
	move.l	#1,ab_frame_counter(a3)
	move.l	#1,ab_FORM_counter(a3)

	tst.b	ab_continue(a3)
	beq.b	exitfind_d2

	bsr.w	skip_one_form

	move.l	#2,ab_frame_counter(a3)
	move.l	#2,ab_FORM_counter(a3)

exitfind_d2:
	move.b	#$0,ab_diskloaded(a3)
	rts

frame_alloc_fail:
	moveq	#1,d7
	bra.w	exit

*
* Get next FORM pointer from the internal array
* returns the pointer in a1
*
get_next_FORM:
	move.l	ab_frame_pointer(a3),a0
	addq.l	#1,ab_frame_counter(a3)

	move.l	a0,a4
	move.l	ani_form(a0),a1
	move.l	ani_next(a0),a0
	cmp.l	#0,a0
	bne.b	ok_getFORM

	move.l	#0,ab_frame_counter(a3)
	move.l	ab_head_frame_list(a3),a0
	subq.l	#1,ab_num_rot(a3)	; one time round

	tst.b	ab_looping(a3)
	bne.b	ok_nonlooping
	move.l	ani_next(a0),a0
	move.l	ani_next(a0),a0		; skip first two frames ??
	move.l	#2,ab_frame_counter(a3)
ok_nonlooping:

ok_getFORM:
	move.l	a0,ab_frame_pointer(a3)
	rts

*
* a1 points to the chunk
* a4 points to the anim_list struct
*
check_chunks:
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

	cmp.l	#'CMAP',d0
	bne.b	no_cmap_anim_found
	bsr.w	cmap_anim_found
	bra.b	continue_check_FORM
no_cmap_anim_found

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
	move.l	d0,ab_dpan_count(a3)
	bra.w	continue_check_FORM
no_dpan_anim_found:

continue_check_FORM:
	rts

create_next_frame:
	move.b	#$0,ab_bmhd_found(a3)
	move.b	#0,ab_cmap_found(a3)

	bsr.w	get_next_FORM		; retrieve FORM pointer	

	tst.b	ani_type(a4)
	bne.w	already_known_form

create_next_frame1:
	move.b	#$0,ab_bmhd_found(a3)
	move.b	#$0,ab_cmap_found(a3)

	move.l	4(a1),ab_temp_formsize(a3)	; store size
	addq.l	#8,a1
	move.l	a1,a2
	add.l	ab_temp_formsize(a3),a2	; end FORM pointer
	cmp.l	#'ILBM',(a1)+
	bne.w	no_ilbm1

next_chunk_FORM1:

	bsr.w	check_chunks

	add.l	4(a1),a1
	move.l	a1,d0
	btst	#0,d0
	beq.b	no_oneven2
	addq.l	#1,a1
no_oneven2:
	addq.l	#8,a1
	cmp.l	a2,a1
	blt.b	next_chunk_FORM1

	tst.b	ab_diskloaded(a3)
	bne.w	exit_nextframe2

; All chunks are handled now check the actions taken
; to see if any changes should be copied to the other view

; but first show the newly made view

show_new_frame:

	tst.b	ab_bmhd_found(a3)
	beq.b	no_anim_bmhd_found

	move.l	ab_view_hidden(a3),a5		; change hidden
	bsr.w	freememory
	move.l	ab_frame_hidden(a3),ab_w_bitpointer(a3)
	bsr.w	maak_viewport			; if excist the new colors
	bra.w	exit_nextframe1			; are loaded at the same time

no_anim_bmhd_found:

	tst.b	ab_show(a3)
	bne.b	show5
	tst.b	ab_continue(a3)
	beq.b	show5
	move.b	#$0,ab_bmhd_found(a3)
	move.b	#$0,ab_bmhd_changed(a3)
	bra.w	no_show
show5:

	tst.b		ab_cmap_found(a3)
	bne.b		cmap_ch1

	tst.b		ab_cmap_changed(a3)
	beq.b		exit_nextframe1

	tst.b		ab_aa_present(a3)
	beq.b		no_aa4

	move.l		ab_view_hidden(a3),a5		; copy last found cmap
	lea		vi_viewport(a5),a0
	move.l		ab_col_pointer(a3),a1
	move.l		ab_graphbase(a3),a6
	jsr		-$372(a6)			; loadrgb32
	bra.b		aa4

no_aa4:
	move.l	ab_view_hidden(a3),a5		; copy last found cmap
	lea	vi_viewport(a5),a0
	move.l	ab_col_pointer(a3),a1
	moveq	#32,d0
	move.l	ab_graphbase(a3),a6
	jsr	_LVOLoadRGB4(a6)
aa4:

	move.b	#0,ab_cmap_changed(a3)
	bra.b	exit_nextframe1

cmap_ch1:
	tst.b	ab_aa_present(a3)
	beq.b	no_aa5

	move.l		ab_view_hidden(a3),a5		; copy last found cmap
	lea		vi_viewport(a5),a0
	move.l		ab_col_pointer(a3),a1
	move.l		ab_graphbase(a3),a6
	jsr		-$372(a6)			; loadrgb32
	bra.b		aa5

no_aa5:
	move.l	ab_view_hidden(a3),a5		; copy cmap
	lea	vi_viewport(a5),a0
	move.l	ab_col_pointer(a3),a1
	moveq	#32,d0
	move.l	ab_graphbase(a3),a6
	jsr	_LVOLoadRGB4(a6)
aa5:
exit_nextframe1:
	tst.b	ab_show(a3)
	beq.b	no_show

	tst.b	ab_continue(a3)
	beq.b	normal_show

	cmp.l	#2,ab_frame_counter(a3)
	bne.b	normal_show

	bsr.w	change_view2

	bra.b	exit_nextframe2

normal_show:
	bsr.w	change_view	

exit_nextframe2:
	rts

no_ilbm1:				; what to do now ??????

	rts

no_show:
	tst.b		ab_continue(a3)
	bne.w		no_cont5

;	move.l		ab_view_visible(a3),d0
;	move.l		ab_view_hidden(a3),ab_view_visible(a3)
;	move.l		d0,ab_view_hidden(a3)
;	move.l		ab_frame_show(a3),d0
;	move.l		ab_frame_hidden(a3),ab_frame_show(a3)
;	move.l		d0,ab_frame_hidden(a3)
no_cont5:
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

	move.w	#0,ab_mode(a3)
	move.b	#$ff,ab_bmhd_found(a3)

	move.w	ani_lenx(a4),ab_lenx(a3)
	moveq	#0,d0
	move.w	ab_lenx(a3),d0
	add.l	#15,d0
	lsr.l	#4,d0
	add.l	d0,d0
	move.l	d0,ab_breedte_x(a3)

	move.w	ani_planes(a4),ab_planes(a3)
	move.b	ani_compression(a4),ab_compression(a3)
	move.b	ani_masking(a4),ab_masking(a3)

	move.w	ab_planes(a3),d6
	move.l	ab_breedte_x(a3),d0
	mulu	d6,d0

	cmp.l	ab_rowmulti(a3),d0
	bne.b	abmhd_multable
	move.w	ani_leny(a4),d0
	cmp.w	ab_leny(a3),d0
	beq.b	abmhd_no_multable

abmhd_multable:
	move.w	ani_leny(a4),ab_leny(a3)
	moveq	#0,d1
	move.w	ab_leny(a3),d1

	move.l	d0,ab_rowmulti(a3)
* create a mul-table for the anim play
	move.l	d0,d1

	lea	ab_multabel(a3),a0
	move.w	ab_leny(a3),d6
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
	move.b	#$ff,ab_bmhd_changed(a3)		; copy to other view later on

	move.l	ab_breedte_x(a3),d0
	move.w	ab_leny(a3),d1
	mulu	d1,d0
	moveq	#0,d1	
	move.w	ab_planes(a3),d1
	mulu	d1,d0
	move.l	d0,ab_unpacked_size(a3)

no_bmhd2:
	move.b	ani_type(a4),d7
	and.b	#ANHD_type,d7
	beq.b	no_anhd2

	move.l	ani_reltime(a4),ab_frame_speed(a3)
	move.b	ani_interleave(a4),ab_ainterleaved(a3)
	move.b	ani_bits(a4),ab_XORmode(a3)
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
	move.l	ani_cmap(a4),d7
	beq.b	no_cmap2
	tst.b	ab_aa_present(a3)
	bne.b	aa6
	addq.l	#2,d7
aa6:	move.l	d7,ab_col_pointer(a3)
	move.b	#$ff,ab_cmap_changed(a3)
	move.b	#$ff,ab_cmap_found(a3)
no_cmap2:
	move.b	ani_type(a4),d7
	and.b	#CAMG_type,d7
	beq.b	no_camg2
	move.b	ani_camghires(a4),ab_hires(a3)
	move.b	ani_camglace(a4),ab_interlace(a3)
	move.w	ani_camgmode(a4),ab_mode(a3)
no_camg2:

	bsr.b	free_disk_frame

	bra.w	show_new_frame

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

	move.l	ab_filehandle(a3),d1
	move.l	ani_chunkpos(a4),d2
	sub.l	ab_relfpointer(a3),d2
	add.l	d2,ab_relfpointer(a3)
	moveq	#0,d3					; current
	move.l	ab_dosbase(a3),a6
	jsr	_LVOSeek(a6)

	move.l	ani_form(a4),d2
	move.l	ani_chunksize(a4),d3
	add.l	d3,ab_relfpointer(a3)
	move.l	ab_filehandle(a3),d1
	jsr	_LVORead(a6)
	cmp.l	ani_chunksize(a4),d0
	bne.w	frame_alloc_fail

	rts

*
*
*	
dlta_anim_found:
	movem.l	d0-d7/a0-a6,-(a7)

	tst.b	ab_bmhd_changed(a3)	; in this case the data from the active must
				; be copied to the hidden ( the first 2 frames)
	beq.b	no_bmhd_ch2			
	tst.b	ab_show(a3)
	bne.b	do_showd1
	tst.b	ab_picture(a3)
	bne.b	do_showd1
	bra.b	bmhd_ch3
do_showd1:
	cmp.b	#$64,ab_acompression(a3)
	beq.b	anim322
	cmp.b	#$65,ab_acompression(a3)
	bne.b	no_anim322
anim322:

	bsr.w	convert_int_non
no_anim322:
	
	bsr.w	copy_active_hidden

	move.b	#0,ab_bmhd_changed(a3)
	bra.b	bmhd_ch3
no_bmhd_ch2:
	cmp.b	#1,ab_ainterleaved(a3)
	bne.b	bmhd_ch3
	bsr.w	actual_copy
bmhd_ch3:
	tst.b	ab_show(a3)
	beq.b	no_show3

	cmp.b	#5,ab_acompression(a3)
	bne.b	no_anim5
	addq.l	#8,a1
	bsr.w	de_animate
	bra.b	no_show3
no_anim5:
	cmp.b	#$64,ab_acompression(a3)
	bne.b	no_anim32
	bsr.w	anim32
	bra.b	no_show3
no_anim32:
	cmp.b	#$65,ab_acompression(a3)
	bne.b	no_anim16
	bsr.w	anim16
	bra.b	no_show3

no_anim16:

	cmp.b	#7,ab_acompression(a3)
	bne.b	no_anim7

	addq.l	#8,a1

	move.b	ab_bits(a3),d0
	btst	#0,d0
	beq.b	no_anim6
			
	bsr.b	anim7
	bra.b	no_show3

no_anim6:

	bsr.w	anim6
	bra.b	no_show3
no_anim7:

	cmp.b	#8,ab_acompression(a3)
	bne.b	no_anim8

	addq.l	#8,a1

	move.b	ab_bits(a3),d0
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
	move.l	ab_temp_formpointer(a3),d7
	sub.l	d7,ani_chunkpos(a4)
	move.l	ab_global_diskoffset(a3),d7
	add.l	d7,ani_chunkpos(a4)
	move.l	ani_chunkpos(a4),d1
	
	move.l	4(a1),ani_chunksize(a4)
	addq.l	#8,ani_chunksize(a4)
	or.b	#DLTA_type,ani_type(a4)
no_store3:
	rts

anim7:
	move.l	ab_breedte_x(a3),d2
	lsr.l	#2,d2			; long words

	move.l	a1,a5			; pointer naar DLTA data 

	move.l	ab_rowmulti(a3),d0
	
	move.l	ab_frame_hidden(a3),a6
	lea	ab_multabel(a3),a4

	move.w	ab_planes(a3),d7
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
	move.l	ab_breedte_x(a3),d2
	lsr.l	#1,d2			; words

	move.l	a1,a5			; pointer naar DLTA data 

	move.l	ab_rowmulti(a3),d0
	
	move.l	ab_frame_hidden(a3),a6
	lea	ab_multabel(a3),a4

	move.w	ab_planes(a3),d7
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


init_anim32_16:
	btst	#4,ab_bits(a3)
	beq.b	horizontal
	btst	#6,ab_bits(a3)
	beq.b	no_skip_line
	move.l	ab_breedte_x(a3),d3
	add.l	d3,d3
	move.b	#$ff,ab_lize(a3)
;	move.l	ab_view_hidden(a3),ab_view_visible(a3)
;	move.l	ab_frame_hidden(a3),ab_frame_show(a3)
	bra.b	ex_init

no_skip_line:
	move.l	ab_breedte_x(a3),d3
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
	move.l	ab_frame_hidden(a3),a6		; plane pointer
	move.l	ab_breedte_x(a3),d2		; width plane
	mulu	ab_leny(a3),d2

	move.w	ab_planes(a3),d7
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
	move.l	ab_frame_hidden(a3),a6		; plane pointer
	move.l	ab_breedte_x(a3),d2		; width plane
	mulu	ab_leny(a3),d2

	move.w	ab_planes(a3),d7
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


body_anim_found:
	movem.l	d0/a1/a2/a4,-(a7)
ttt5:

	tst.b	ab_diskloaded(a3)
	bne.w	no_body

	
	or.b	#BODY_type,ani_type(a4)

	tst.b	ab_show(a3)
	bne.b	do_show2

	tst.b	ab_picture(a3)
	beq.b	no_show2

	bsr.w	copy_active_hidden

	bra.b	no_show2

do_show2:
	cmp.b	#1,ab_compression(a3)
	beq.s	anim_un1

; compression 0 or unknown do a simple copy

	move.l	ab_frame_hidden(a3),a2
	move.l	4(a1),d0
	lsl.l	#2,d0
	subq.l	#1,d0
	addq.l	#8,a1
anim_no_co:
	move.l	(a1)+,(a2)+
	dbf	d0,anim_no_co
	bra.s	anim_un_end

anim_un1:

	move.l	ab_frame_hidden(a3),a2
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
	move.l	ab_temp_formpointer(a3),d7
	sub.l	d7,ani_chunkpos(a4)
	move.l	ab_global_diskoffset(a3),d7
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

	move.b	#$ff,ab_bmhd_changed(a3)	; change the viewports accordingly
	move.b	#$ff,ab_bmhd_found(a3)
	move.b	ani_type(a4),d7
	and.b	#CAMG_type,d7
	bne.b	easy_camg

	moveq	#0,d2
	move.w	10(a1),d0

	or.w	d0,d2			; straight in the mode-id
	and.w	#$8efd,d2		; oldstyle mask
	move.w	d0,d1
	and.w	#$8000,d0
	beq.b	cno_hires
	move.b	#$ff,ab_hires(a3)
	or.w	#$8000,d2
cno_hires:
	move.w	d1,d0
	and.w	#$4,d0
	beq.b	cno_lace
	move.b	#$ff,ab_interlace(a3)
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
	and.w	#$f7ff,d2	; clear the ham bit
cno_half:
	move.w	d2,ab_mode(a3)
	move.b	ab_hires(a3),ani_camghires(a4)

	move.b	ab_interlace(a3),ani_camglace(a4)
	move.w	d2,ani_camgmode(a4)
	or.b	#CAMG_type,ani_type(a4)
	move.l	(a7)+,d0
	rts

easy_camg:

	move.b	ani_camghires(a4),ab_hires(a3)
	move.b	ani_camglace(a4),ab_interlace(a3)
	move.w	ani_camgmode(a4),ab_mode(a3)
	move.l	(a7)+,d0
	rts
	
cmap_anim_found:
	movem.l	d0/a1/a2/a4,-(a7)
	move.b	#$ff,ab_cmap_found(a3)
	addq.l	#4,a1
	move.l	(a1)+,d1
	divu	#3,d1
	move.l	d1,ab_color_count(a3)	
	tst.b	ab_aa_present(a3)
	bne.b	do_aa_cmap

	cmp.l	#3*32,d1
	ble.b	no_pro_cmap
	moveq	#3*32,d1
	move.l	d1,ab_color_count(a3)	
no_pro_cmap:
	movem.l	d1/a0/a1,-(a7)
	and.l	#$ffff,d1
	move.l	d1,d0
	add.l	d0,d0
	addq.l	#2,d0
	move.l	$4,a6
	moveq	#MEMF_PUBLIC,d1
ttt2:
	jsr	_LVOAllocMem(a6)
	movem.l	(a7)+,d1/a0/a1
	tst.l	d0
	beq.w	frame_alloc_fail
	
* in d1 het aantal kleuren
	move.l	d0,ani_cmap(a4)
	move.l	d0,ab_col_pointer(a3)
	addq.l	#2,ab_col_pointer(a3)
	move.l	d0,a2
	move.w	d1,(a2)+

	subq.l	#1,d1
anim_cmap4:
	moveq	#0,d3	
	moveq	#0,d2

	move.b	(a1)+,d3
	lsl.w	#4,d3
	move.b	(a1)+,d3
	move.b	(a1)+,d2
	lsr.b	#4,d2
	and.b	#$f0,d3
	or.b	d2,d3
	move.w	d3,(a2)+
	dbf	d1,anim_cmap4
	moveq	#0,d0
	move.b	#$ff,ab_cmap_changed(a3)
	movem.l	(a7)+,d0/a1/a2/a4
	rts

do_aa_cmap:
	movem.l	d1/a0/a1,-(a7)
	and.l	#$ffff,d1
	move.l	d1,d0
	add.l	d0,d0
	add.l	d1,d0			; * 3
	lsl.l	#2,d0			; long word color values
	add.l	#12,d0
	move.l	$4,a6
	moveq	#MEMF_PUBLIC,d1
	jsr	_LVOAllocMem(a6)
	movem.l	(a7)+,d1/a0/a1
	tst.l	d0
	beq.w	frame_alloc_fail

	move.l	d0,ani_cmap(a4)
	move.l	d0,ab_col_pointer(a3)
	move.l	d0,a2

	move.w	d1,(a2)+		; number off color
	move.w	#0,(a2)+		; from color zero
cmapaa:
	moveq	#0,d3
	moveq	#0,d2
	move.b	(a1)+,d3
	bsr.b	convert_byte_long
	move.l	d3,(a2)+

	move.b	(a1)+,d3
	bsr.b	convert_byte_long
	move.l	d3,(a2)+

	move.b	(a1)+,d3
	bsr.b	convert_byte_long
	move.l	d3,(a2)+
	subq.w	#1,d1
	bne.b	cmapaa

	move.l	#0,(a2)+

	moveq	#0,d0
	move.b	#$ff,ab_cmap_changed(a3)

	movem.l	(a7)+,d0/a1/a2/a4
	rts
*
* d3 containes the byte color value
* trash d2 return d3 long value
*
convert_byte_long:
	moveq		#0,d2
	move.b		d3,d2		; convert to 32 bits
	lsl.w		#8,d2
	move.b		d3,d2
	move.w		d2,d3
	swap		d3
	move.w		d2,d3
	rts

anhd_anim_found:
	movem.l	d0/a1/a4,-(a7)
	addq.l	#8,a1
	move.b	$17(a1),ab_bits(a3)

	
	move.b	anhd_operation(a1),ab_acompression(a3)
	move.l	anhd_reltime(a1),d0
	tst.l	ab_overallspeed(a3)
	bne.b	no_reltime

	move.l	d0,ab_frame_speed(a3)

no_reltime:	
	move.l	anhd_bits(a1),d0
;	move.l	d0,bits
	move.l	d0,d1
	and.l	#$3f,d1
	cmp.l	d0,d1
	bne.b	no_xor_mode1		; carbage found in ANHD
	btst	#1,d0
	beq.b	no_xor_mode1
	move.b	#$ff,ab_XORmode(a3)
no_xor_mode1:
	move.b	anhd_interleave(a1),d0
	move.b	d0,d1
	and.b	#1,d1
	cmp.b	d0,d1
	bne.b	carbage_found1
	move.b	d0,ab_ainterleaved(a3)
carbage_found1:
	movem.l	(a7)+,d0/a1/a4
	move.b	ani_type(a4),d7
	and.b	#BMHD_type,d7
	bne.b	no_store_struct2
	move.b	ani_type(a4),d7
	and.b	#ANHD_type,d7
	bne.b	no_store_struct2
	move.l	ab_frame_speed(a3),ani_reltime(a4)
	move.b	ab_ainterleaved(a3),ani_interleave(a4)
	move.b	ab_XORmode(a3),ani_bits(a4)
	or.b	#ANHD_type,ani_type(a4)
no_store_struct2:
	rts
*
* In the anim is a BMHD chunk
* initialize the variables with this new info
*
bmhd_anim_found:
	movem.l	d0/a1/a4,-(a7)
	move.b	#$ff,ab_bmhd_found(a3)
	move.w	#0,ab_mode(a3)
	addq.l	#8,a1
	move.w	(a1),ab_lenx(a3)

	moveq	#0,d0
	move.w	ab_lenx(a3),d0
	add.l	#15,d0
	lsr.l	#4,d0
	add.l	d0,d0
	move.l	d0,ab_breedte_x(a3)

	moveq	#0,d1
	move.b	8(a1),d1
	move.w	d1,ab_planes(a3)
	move.b	10(a1),ab_compression(a3)
;	move.b	ab_compression(a3),d0
	move.b	9(a1),ab_masking(a3)

	move.w	ab_planes(a3),d7
	move.l	ab_breedte_x(a3),d0
	mulu	d7,d0

	cmp.l	ab_rowmulti(a3),d0
	bne.b	bmhd_multable
	move.w	2(a1),d0
	cmp.w	ab_leny(a3),d0
	beq.b	bmhd_no_multable

bmhd_multable:
	move.w	2(a1),ab_leny(a3)
	moveq	#0,d1
	move.w	ab_leny(a3),d1

	move.l	d0,ab_rowmulti(a3)
* create a mul-table for the anim play
	move.l	d0,d1

	lea	ab_multabel(a3),a0
	move.w	ab_leny(a3),d7
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
	move.b	#$ff,ab_bmhd_changed(a3)		; copy to other view later on

	move.l	ab_breedte_x(a3),d0
	move.w	ab_leny(a3),d1
	mulu	d1,d0
	moveq	#0,d1	
	move.w	ab_planes(a3),d1
	mulu	d1,d0
	move.l	d0,ab_unpacked_size(a3)

	movem.l	(a7)+,a1/d0/a4
	tst.b	ani_type(a4)
	bne.b	no_store_struct1
	move.w	ab_lenx(a3),ani_lenx(a4)
	move.w	ab_leny(a3),ani_leny(a4)
	move.w	ab_planes(a3),ani_planes(a4)
	move.b	ab_compression(a3),ani_compression(a4)
;	move.b	ab_masking(a3),ani_masking(a4)
	or.b	#BMHD_type,ani_type(a4)
no_store_struct1:
	rts

*
* The size or something else has changed so alter the
* hidden accordingly
*
copy_active_hidden:
;	tst.b	ab_show(a3)
;	beq	no_show30	


	movem.l	d0-d7/a0-a6,-(a7)
	move.l	ab_view_hidden(a3),a5
	bsr.w	freememory
	move.l	ab_frame_hidden(a3),ab_w_bitpointer(a3)
	bsr.w	maak_viewport
	movem.l	(a7)+,d0-d7/a0-a6
actual_copy:
	movem.l	d0-d7/a0-a6,-(a7)
	moveq	#0,d0
	move.w	ab_leny(a3),d0
	mulu	ab_planes(a3),d0
	move.l	ab_breedte_x(a3),d1
	mulu	d1,d0
	lsr.l	#1,d0
;	subq.l	#1,d0
	move.l	ab_frame_hidden(a3),a0
	move.l	ab_frame_show(a3),a1
rep_cop1:
	move.w	(a1)+,(a0)+
	subq.l	#1,d0
	bne.b	rep_cop1
	movem.l	(a7)+,d0-d7/a0-a6
no_show30:
	rts


*
* 
* de_animate needs a pointer de de DLTA data ( DLTA pointer + 8 )
* it also uses a mul-table 
de_animate:
	tst.b	ab_XORmode(a3)
	bne.w	de_animate_xormode
	tst.b	ab_more_cpu(a3)
	bne.w	de_animate_cpu20

 * de_anim de 68000 manier
 
	move.l	ab_breedte_x(a3),d2
	move.l	a1,a5			; pointer naar DLTA data 

	move.l	ab_rowmulti(a3),d0
	
	move.l	ab_frame_hidden(a3),a6

	move.w	ab_planes(a3),d7
	subq.w	#1,d7	
rep_anim_planes:
	move.l	(a1)+,d6
	beq.w	no_dlta_data
	lea	0(a5,d6.l),a0		; a0 wijst nu naar de plane-data

	move.l	d2,d6
	subq.w	#1,d6
	move.l	a6,-(a7)
rep_anim_kolom:
	moveq	#0,d5
	move.b	(a0)+,d5
	beq.w	opcountzero

	move.l	a6,a2

	subq.w	#1,d5
rep_opcounts:
	move.b	(a0)+,d4
	beq.w	same_ops	
	bpl.w	skip_ops

uniq_ops:
	and.w	#$7f,d4

	add.w	d4,d4
	add.w	d4,d4
	neg.w	d4
	lea	no_uniq(pc),a4
	jmp	0(a4,d4.w)

uniq_copy:
	move.b	(a0)+,(a6)
	adda.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
	move.b	(a0)+,(a6)
	add.l	d0,a6
no_uniq:
cont_opcounts:

	dbf	d5,rep_opcounts
cont_more_opcounts:

	move.l	a2,a6

opcountzero:			; opcount zero dus volgende kolom
	addq.l	#1,a6
	dbf	d6,rep_anim_kolom	

	move.l	(a7)+,a6

no_dlta_data:
	add.l	d2,a6		; volgende plane
	dbf	d7,rep_anim_planes
	rts

same_ops:
	moveq	#0,d4
	move.b	(a0)+,d4
	move.b	(a0)+,d1	; de data die gezet moet worden

	move.w	d4,d3
	asr.w	#3,d3
	andi.w	#7,d4
	add.w	d4,d4
	add.w	d4,d4
	neg.w	d4
	jmp	no_sam(pc,d4.w)	
same_copy:
	move.b	d1,(a6)
	adda.w	d0,a6
	move.b	d1,(a6)
	adda.w	d0,a6
	move.b	d1,(a6)
	adda.w	d0,a6
	move.b	d1,(a6)
	adda.w	d0,a6
	move.b	d1,(a6)
	adda.w	d0,a6
	move.b	d1,(a6)
	adda.w	d0,a6
	move.b	d1,(a6)
	adda.w	d0,a6
	move.b	d1,(a6)
	adda.w	d0,a6
no_sam:	dbra	d3,same_copy
	dbf	d5,rep_opcounts
	bra.b	cont_more_opcounts	



skip_ops:
	and.w	#$7f,d4

	lea	ab_multabel(a3),a4
	add.w	d4,d4
	add.w	d4,d4
	add.l	0(a4,d4.w),a6
	dbf	d5,rep_opcounts
	bra.b	cont_more_opcounts


de_animate_cpu20:
	move.l	ab_breedte_x(a3),d2
	move.l	a1,a5			; pointer naar DLTA data 

	move.l	ab_rowmulti(a3),d0
	
	move.l	ab_frame_hidden(a3),a6
	lea	ab_multabel(a3),a4

	move.w	ab_planes(a3),d7
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
	move.l	ab_breedte_x(a3),d2
	lsr.l	#2,d2			; long words
	move.l	a1,a5			; pointer naar DLTA data 

	move.l	ab_rowmulti(a3),d0
	
	move.l	ab_frame_hidden(a3),a6
	lea	ab_multabel(a3),a4

	move.w	ab_planes(a3),d7
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
	add.l	ab_breedte_x(a3),a6		; volgende plane
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
	move.l	ab_breedte_x(a3),d2
	lsr.l	#1,d2			; long words
	move.l	a1,a5			; pointer naar DLTA data 

	move.l	ab_rowmulti(a3),d0
	
	move.l	ab_frame_hidden(a3),a6
	lea	ab_multabel(a3),a4

	move.w	ab_planes(a3),d7
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
	add.l	ab_breedte_x(a3),a6		; volgende plane
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
	move.l	ab_breedte_x(a3),d2
	move.l	a1,a5			; pointer naar DLTA data 

	move.l	ab_rowmulti(a3),d0
	
	move.l	ab_frame_hidden(a3),a6

	move.w	ab_planes(a3),d7
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

	lea	ab_multabel(a3),a4
	add.w	d4,d4
	add.w	d4,d4
	add.l	0(a4,d4.w),a6
	dbf	d5,rep_opcountsx
	bra.b	cont_more_opcountsx


* hele file nu in het geheugen 

file_loaded:
	bsr.w	close_file

file_loaded2:
	move.l	ab_packed(a3),a0
	move.l	4(a0),d0
	addq.l	#8,d0
	move.l	d0,ab_packed_size(a3)
	moveq	#0,d0
	rts

file_cropped:
	bsr.w	close_file
	moveq	#ERR_UNKNOWN_FILE_TYPE,d7
	bra.w	exit


read_whole_file:
	move.l	#3,d7
	move.l	ab_filenaam(a3),d1
	move.l	#mode_old,d2
	move.l	ab_dosbase(a3),a6
	jsr	_LVOOpen(a6)
	move.l	d0,ab_filehandle(a3)
	beq.w	exit
	move.l	d0,d1
	lea	ab_buffer(a3),a0
	move.l	a0,d2
	move.l	#12,d3
	move.l	ab_dosbase(a3),a6
	jsr	_LVORead(a6)
	lea.l	ab_buffer(a3),a1
	move.l	#1,d7
	cmp.l	#'FORM',(a1)+
	bne.w	exit
	rts

close_file:
	move.l	ab_dosbase(a3),a6
	move.l	ab_filehandle(a3),d1
	beq.b	no_closefile
	jsr	_LVOClose(a6)
	clr.l	ab_filehandle(a3)
no_closefile:
	rts

openlibs:
	clr.l	ab_packed(a3)
	clr.l	ab_unpacked(a3)
	clr.l	ab_filehandle(a3)
	clr.l	ab_graphbase(a3)
	clr.l	ab_intbase(a3)

	clr.l	ab_dosbase(a3)
	move.l	exec,a6

	lea	dosnam(pc),a1
	moveq	#0,d0
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,ab_dosbase(a3)
	beq.w	exit

	lea	graphname(pc),a1
	moveq	#0,d0
	move.l	exec,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,ab_graphbase(a3)
	beq.w	exit

	lea	intnam(pc),a1
	moveq	#0,d0
	move.l	exec,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,ab_intbase(a3)
	beq.w	exit

	move.l	ab_dosbase(a3),a6
	jsr	_LVOOutput(a6)
	move.l	d0,ab_conhandle(a3)

*	beq.s	exit
	rts

close_libs:
	tst.l	ab_graphbase(a3)
	beq.s	no_gbase
	move.l	ab_graphbase(a3),a1
	move.l	exec,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	ab_graphbase(a3)

no_gbase:
	tst.l	ab_intbase(a3)
	beq.s	no_intb
	move.l	ab_intbase(a3),a1
	move.l	exec,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	ab_intbase(a3)
no_intb:
	tst.l	ab_dosbase(a3)
	beq.b	no_dosb
	move.l	ab_dosbase(a3),a1
	move.l	exec,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	ab_dosbase(a3)
no_dosb:
	rts
	
exit:	
	tst.l	ab_temp_formpointer(a3)
	beq.b	no_free_poi3
	move.l	ab_temp_formdisksize(a3),d0
	move.l	ab_temp_formpointer(a3),a1
	move.l	$4,a6
	jsr	_LVOFreeMem(a6)
no_free_poi3:

	bsr.w	reset_oldview
	lea	ab_view1(a3),a5
	bsr.w	freememory
	lea	ab_view2(a3),a5
	bsr.w	freememory

	bsr.w	free_frame_list
	move.l	ab_temp_frame_list(a3),d0
	beq	no_t_frame2
	move.l	d0,a1
	bsr	free_frame
no_t_frame2:
	move.l	ab_easy_exit(a3),a7

exit11:	
	tst.l	ab_unpackedsecond(a3)
	beq.s	exit12
	move.l	ab_unpackedsecond(a3),a1
	move.l	#MEMSIZE,d0
	move.l	exec,a6
	jsr	_LVOFreeMem(a6)
	clr.l	ab_unpackedsecond(a3)
exit12:
	tst.l	ab_unpacked(a3)
	beq.s	exit1
	move.l	ab_unpacked(a3),a1
	move.l	#MEMSIZE,d0
	move.l	exec,a6
	jsr	_LVOFreeMem(a6)
	clr.l	ab_unpacked(a3)

exit1:

	tst.l	ab_packed(a3)
	beq.s	exit21
	move.l	ab_packed(a3),a1
	move.l	ab_packed_size(a3),d0
	move.l	exec,a6
	jsr	_LVOFreeMem(a6)
	clr.l	ab_packed(a3)
exit21:
exit22:	tst.l	ab_filehandle(a3)
	beq.s	exit2
	bsr.w	close_file
	
exit2:
	tst.l	ab_graphbase(a3)
	beq.s	exit3
	move.l	ab_graphbase(a3),a1
	move.l	exec,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	ab_graphbase(a3)

exit3:
	tst.l	ab_intbase(a3)
	beq.s	exit4
	move.l	ab_intbase(a3),a1
	move.l	exec,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	ab_intbase(a3)

exit4:	
	tst.l	ab_dosbase(a3)
	beq.s	exit5

	tst.b	ab_info(a3)
	beq.b	no_info2
	tst.l	ab_conhandle(a3)
	beq.b	no_info2

	movem.l	d0-d7/a0-a6,-(a7)
;	move.l	timer(pc),d0
	bsr.w	printsecs
	bsr.w	printframes
	bsr.b	print_persec
	movem.l	(a7)+,d0-d7/a0-a6

no_info2:
	tst.l	d7
	beq.s	exit51
	sub.l	#1,d7
	move.l	ab_dosbase(a3),a6
	lea.l	foutmelding(pc),a1
	lsl.l	#2,d7
	add.l	d7,a1
	move.l	(a1),d2
	move.l	ab_conhandle(a3),d1

	move.l	ab_conhandle(a3),d1
	beq.w	no_con_open

	move.l	#FOUT_LENGTE,d3
	jsr	_LVOWrite(a6)

exit51:
	move.l	ab_dosbase(a3),a1
	move.l	exec,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	ab_dosbase(a3)
exit5:
	moveq	#0,d0
	rts

veelvoud:
	move.l	#1,d5
	move.l	d4,d1
rephon:	cmp.l	d0,d1
	bhi.b	geenho
	add.l	d4,d1
	add.l	#1,d5
	cmpi.l	#10,d5
	bne.b	rephon
geenho:	sub.l	#1,d5
	sub.l	d4,d1
	sub.l	d1,d0

	bsr.w	putin
	rts


; print timer / frames decimaal

print_persec:
	move.l	ab_frames(a3),d0
	mulu	#100,d0
;	move.l	timer(pc),d1
	beq.b	no_div
	divu	d1,d0
	move.b	#1,ab_lead(a3)
	lea.l	ab_multabel(a3),a1
	move.l	#"Fr /",(a1)+
	move.l	#" s: ",(a1)+
	and.l	#$ffff,d0
	addq.l	#1,d0
	bra.b	printdec
no_div:
	rts	

printframes:
	move.b	#1,ab_lead(a3)
	move.l	ab_frames(a3),d0
	lea.l	ab_multabel(a3),a1
	move.l	#"Fram",(a1)+
	move.l	#"es: ",(a1)+
	bra.b	printdec
	
printsecs:
	move.b	#0,ab_lead(a3)
;	move.l	timer(pc),d0
	lea.l	ab_multabel(a3),a1
	move.l	#"Secs",(a1)+
	move.l	#"  : ",(a1)+

printdec:
	move.l	#10000000,d4
	bsr.w	veelvoud
	move.l	#1000000,d4
	bsr.w	veelvoud
	move.l	#100000,d4
	bsr.w	veelvoud
	move.l	#10000,d4
	bsr.w	veelvoud
		
	lea.l	tienvoud(pc),a4

	move.l	#3,d7
repdec:	move.l	(a4)+,d6
	tst.b	ab_lead(a3)
	bne.b	no_point
	cmp.l	#10,d6
	bne.b	no_point
	move.b	#'.',(a1)+
no_point:
	move.l	d0,d5
	divu	d6,d5
	bsr.b	putin
	swap	d5
	clr.l	d0
	move.w	d5,d0
	dbf	d7,repdec
	move.b	#$a,(a1)+
	lea	ab_multabel(a3),a2
	move.l	a1,d3
	sub.l	a2,d3

	move.l	a2,d2
	move.l	ab_conhandle(a3),d1
	move.l	ab_dosbase(a3),a6
	jsr	_LVOWrite(a6)
	move.b	#0,ab_aa_present(a3)
	rts

putin:	move.l	d5,-(a7)
	tst.b	ab_aa_present(a3)
	bne.b	nospa
	tst.b	d5
	bne.b	nospa
;	move.b	#" ",(a1)+
	move.l	(a7)+,d5
	rts
nospa:	move.b	#$f,ab_aa_present(a3)
	add.b	#"0",d5
	move.b	d5,(a1)+
	move.l	(a7)+,d5
	rts


tienvoud:	dc.l	1000,100,10,1

no_con_open:
	move.l	d2,a0
	move.l	#FOUT_LENGTE-3,d3

	lea	ab_multabel(a3),a1
	move.b	#0,(a1)+
	move.b	#20,(a1)+
	move.b	#20,(a1)+
c_fout:	move.b	(a0)+,(a1)+
	dbf	d3,c_fout

	move.l	exec,a6
	lea.l	intnam(pc),a1
	jsr	-408(a6)
	tst.l	d0
	beq.b	err_intui
	move.l	d0,a6
	clr.l	d0
	lea	ab_multabel(a3),a0
	move.l	#40,d1
	jsr	-90(a6)

	move.l	a6,a1
	move.l	4,a6
	jsr	-414(a6)
err_intui
	bra.w	exit51

	even


FOUT_LENGTE=21
foutmelding:
	dc.l	fout0,fout1,fout2,fout3,fout4,fout5

fout0:		dc.b	"Not an IFF file     ",10,0
fout1:		dc.b	"Not enough memory   ",10,0
fout2:		dc.b	"File not found      ",10,0
fout3:		dc.b	"Unknown anim mode   ",0
fout4:		dc.b	" ",0
fout5:		dc.b	" ",0

	even

filename:	dc.b	'work:mediapoint/graphics/animations/counter',0

	even

dosnam:		dc.b	'dos.library',0
graphname:	dc.b	'graphics.library',0
intnam:		dc.b	'intuition.library',0
	even
datablock:
	blk.b	ab_SIZEOF,0
panblock:	blk.b	pan_SIZEOF,0
