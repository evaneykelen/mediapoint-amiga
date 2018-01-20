*
* File	: vscroll.s
* Uses	: sb_data.i crawl_struct.i
* Date	: february 1993 
* Author: ing. C. Lieshout
* Desc	: Create a horizontal scroll between two viewports
*	: Use two base pointers one db_ pointer and the sb_ pointer
*	: copy the necessary data to the new base
*
*	: 14 june the structure is changed so that scrolls are
*	: possible in the page structure


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
	INCLUDE	"libraries/dos.i"

	INCDIR	"wp:inclibs/"
	INCLUDE "graphics_libv39.i"
	INCLUDE	"dos_lib.i"
	INCLUDE	"diskfont_lib.i"
	INCLUDE	"intuition_lib.i"

	INCDIR	"wp:crawl/"
	INCLUDE	"sb_data.i"
	INCLUDE	"crawl_struct.i"

	INCDIR	"wp:asm/"
	INCLUDE "proces.i"
	INCLUDE	"system.i"
        INCLUDE "viewbloknew.i"
	INCLUDE	"newdatabloknew.i"

DEBUG = 0

; Default values when there is no viewport

DEF_XSIZE = 640
DEF_MODE = V_HIRES
DEF_PLANES = 1
COLMEMSIZE = 4500

LIBV39 = 39
LIBV40 = 40

	IFNE	DEBUG
	XREF	KPutFmt
	XREF	KPutStr
	PRINTT	"De debug info staat aan dus je moet nog met debug.lib linken"
	ENDC

own_delay:
	movem.l	d0-d2/a0/a1,-(a7)

wait_further:
	move.l	sb_waitmask(a3),d0
	or.l	sb_sig_ptoc(a3),d0
no_sigptoc:

	move.l	$4.w,a6
	jsr	_LVOWait(a6)

	movem.l	d0/d1/a0/a1,-(a7)
	move.l	$4.w,a6
	jsr	_LVOForbid(a6)
	movem.l	(a7)+,d0/d1/a0/a1

	move.l	d0,d1
	and.l	sb_sig_ptoc(a3),d1
	beq.w	no_wacht1

; Er is een message van boven kijk wat zij doet en handel ernaar

search_list:
	move.l	sb_msgpointer(a3),a0		;get msgport ptr
	lea.l	MP_MSGLIST(a0),a1		;make ptr to msglist
	move.l	LH_HEAD(a1),d1			;get first 	

scan:	move.l	d1,a1
	move.l	LH_HEAD(a1),d1
	beq.b	end_of_list

	move.l	pd_Cmd(a1),d2	; pd command
	cmp.l	#DCC_DOTERM,d2
	beq.b	doterm			; let de C-prog handle the reply
	cmp.l	#DCC_DOSTOP,d2
	beq.b	doterm

	bra.b	scan

doterm:
	movem.l	d0-d2/a0/a1,-(a7)
	sub.l	a1,a1
	move.l	$4.w,a6
	jsr	_LVOFindTask(a6)
	move.l	d0,a1
	move.l	sb_sig_ptoc(a3),d0
	jsr	_LVOSignal(a6)		; restore signal
	movem.l	(a7)+,d0-d2/a0/a1

	move.l	$4.w,a6
	jsr	_LVOPermit(a6)
	movem.l	(a7)+,d0-d2/a0/a1

	IFNE DEBUG
	movem.l	d0-d7/a0-a6,-(a7)
	lea	.dbstr(pc),a0
	jsr	KPutStr
	movem.l	(a7)+,d0-d7/a0-a6
	bra	.tt
.dbstr:	dc.b	"ESCESCESCESCESCESCESCESC",10,0
	even
.tt:
	ENDC

	move.w	#1,sb_esc_pres(a3)
	moveq	#-1,d0
	rts

no_wacht1:

end_of_list:
	move.l	$4.w,a6
	jsr	_LVOPermit(a6)

	move.l	d0,d1
	and.l	sb_waitmask(a3),d1
	beq.w	wait_further

	movem.l	(a7)+,d0-d2/a0/a1
no_wacht:
	bsr.b	check_key
	moveq	#0,d0
	rts

check_key:
	move.b	$bfec01,d0
	not	d0
	ror.b	#1,d0
	cmp.b	#$60,d0
	bhi.b	no_key_ch1

	move.l	sb_dosbase(a3),a6
	movem.l	d1/a0/a1,-(a7)
	moveq	#1,d1
	jsr	_LVODelay(a6)		; give system time to check keys
	movem.l	(a7)+,d1/a1/a0
no_key_ch1
	moveq	#0,d0
	rts

*
* call crawl(	*datablockcrawl, *datablocktrans, *crawlrec )
*

	XDEF	_crawl
_crawl:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)

	move.l	8(a5),a3			; crawlbase pointer

	move.l	12(a5),a4			; base trans pointer
	move.l	a4,sb_trans_base(a3)
	move.l	16(a5),a0			; crawl rec pointer
	move.l	a0,sb_crawl_rec(a3)

	bsr.w	copy_libs
	tst.w	d0
	bne.b	lib_err

	bsr.w	get_args			; from crawl rec

	bsr.b	count_colors
	
	bsr.w	do_scroll

	bsr.w	close_all
	tst.w	sb_esc_pres(a3)
	bne.b	esc_press
	
lib_err:
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts
esc_press:
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#1,d0
	rts
*
* Check the string for colors and calculate the number of planes
*
count_colors:
	move.l	sb_colors(a3),a4
	move.l	sb_crawl_rec(a3),a0

	move.l	cr_col(a0),d1

	IFNE DEBUG
	movem.l	d0-d7/a0-a6,-(a7)
	lea	cr_col+2(a0),a1
	lea	.dbstr(pc),a0
	jsr	KPutFmt
	movem.l	(a7)+,d0-d7/a0-a6
	bra	.tt
.dbstr:	dc.b	"Color scroll init is %d",10,0
	even
.tt:
	ENDC

	move.b	#1,0(a4,d1.w)		; set the start color
	moveq	#1,d7			; start with the predefined color

	move.l	sb_text_data(a3),a0
rep_col_c:
	tst.b	(a0)
	beq.b	end_of_string
	cmp.b	#'<',(a0)
	bne.b	no_command_cc

	addq.l	#1,a0
	move.b	(a0),d0
	cmp.b	#'<',d0			; the '<' character
	beq.b	no_command_cc
	cmp.b	#'c',d0
	bne.b	no_change_color
	addq	#1,a0
	bsr.w	get_number
	tst.w	d1
	bne.b	ok_ch_col_cc
	moveq	#1,d1
ok_ch_col_cc:
	and.w	#$ff,d1
	tst.b	0(a4,d1.w)
	bne.b	no_add_up
	addq.l	#1,d7
no_add_up:
	move.b	#1,0(a4,d1.w)	
	bra.b	rep_col_c
no_change_color:
	addq.l	#1,a0
no_command_cc:
	addq.l	#1,a0
	bra.b	rep_col_c
end_of_string:
	moveq	#0,d0
rep_c_pl:
	addq.l	#1,d0
	lsr.l	#1,d7
	bne.b	rep_c_pl
	move.w	d0,sb_pre_depth(a3)

	moveq	#0,d0			; start with color 0
	moveq	#0,d1

	move.l	sb_crawl_rec(a3),a0
	move.l	cr_zero_col(a0),d1

	move.l	d1,d0
	add.l	d0,d0
	add.l	d1,d0

	move.l	sb_coldata(a3),a0
	move.l	sb_transcols(a3),a1

	move.b	0(a1,d0),0(a0)		; copy the 0 color
	move.b	1(a1,d0),1(a0)
	move.b	2(a1,d0),2(a0)

	moveq	#1,d0
	moveq	#0,d1
	
	move.w	#255,d7
check_cols:
	tst.b	0(a4,d1.w)
	beq.b	no_col_here

	move.l	sb_coltable(a3),a0
	move.b	d0,0(a0,d1.w)		; if you find col d1 use col d0
	moveq	#0,d2
	move.b	d0,d2
	add.l	d2,d2
	add.l	d0,d2
	move.l	sb_coldata(a3),a0
	move.l	sb_transcols(a3),a1
	moveq	#0,d3
	move.b	d1,d3
	add.l	d3,d3
	add.l	d1,d3

	IFNE DEBUG
	movem.l	d0-d7/a0-a6,-(a7)
	lea	0(a1,d3.w),a1
	lea	.dbstr(pc),a0
	jsr	KPutFmt
	movem.l	(a7)+,d0-d7/a0-a6
	bra	.tt
.dbstr:	dc.b	"Set color %lx",10,0
	even
.tt:
	ENDC

	move.b	0(a1,d3.w),0(a0,d2.w)	; copy the color bytes
	move.b	1(a1,d3.w),1(a0,d2.w)
	move.b	2(a1,d3.w),2(a0,d2.w)
	addq.w	#1,d0
no_col_here:
	addq.w	#1,d1
	dbf	d7,check_cols

; in d0 the used number of colors

	move.l	#0,sb_colormap(a3)

	moveq	#1,d0
	move.w	sb_pre_depth(a3),d1
	lsl.l	d1,d0			; number off colors
	move.l	d0,d7
	move.l	d0,sb_nroffcols(a3)
	move.l	d7,d0
	bsr.b	convert_colors		; colors are ready to be loaded
.no_colmap:
	rts

*
* Convert vb_colbytes a5 to db_waarcolors
*
convert_colors:
	tst.b		sb_aa_present(a3)
	beq.b		normal_cols
	
	move.l		sb_colors(a3),a0		; scrap coldata
	move.l		sb_coldata(a3),a1		; shuffled colors
	move.w		d0,(a0)+			; aantal kleuren
	move.w		#0,(a0)+			; vanaf kleur 0
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

normal_cols:
	move.l	sb_colors(a3),a0		; scrap coldata
	move.l	sb_coldata(a3),a1		; shuffled colors

	subq.w	#1,d0
rep_rgb2:
	moveq	#0,d1
	move.b	(a1)+,d1
	and.w	#$f0,d1
	lsl.w	#4,d1
	move.b	(a1)+,d2
	and.b	#$f0,d2
	or.b	d2,d1
	move.b	(a1)+,d3
	and.w	#$f0,d3
	lsr.w	#4,d3
	or.w	d1,d3
	move.w	d3,(a0)+
	dbf	d0,rep_rgb2

	rts

*
* Copy data from trans pointer to local pointer
*
copy_libs:
	move.w	#0,sb_esc_pres(a3)
	move.l	db_dosbase(a4),sb_dosbase(a3)
	move.l	db_graphbase(a4),sb_graphbase(a3)
	move.l	db_msgpointer(a4),sb_msgpointer(a3)
	move.l	db_active_viewblok(a4),sb_trans_acview(a3)
	move.l	db_inactive_viewblok(a4),sb_trans_inacview(a3)
	move.l	db_mlsysstruct(a4),sb_mlsysstruct(a3)
	move.w	db_viewx(a4),sb_viewx(a3)
	move.w	db_viewy(a4),sb_viewy(a3)
	move.w	db_or_vmode_mask(a4),sb_or_vmode_mask(a3)
	move.w	db_and_vmode_mask(a4),sb_and_vmode_mask(a3)

	move.l	db_tags(a4),sb_tags(a3)

	move.l	db_monitorspec(a4),sb_monitorspec(a3)
	move.l	db_monid(a4),sb_monid(a3)

	move.l	db_waarcolors32(a4),sb_waarcolors32(a3)
	move.w	db_alternate_lowres(a4),sb_alternate_lowres(a3)
	move.l	db_active_viewblok(a4),a0
	lea	vb_colbytes(a0),a1
	move.l	a1,sb_transcols(a3)
	move.l	#0,sb_coldata(a3)

	move.w	#0,sb_startx(a3)
	move.w	#0,sb_delta(a3)
	move.w	#0,sb_rastx(a3)
	move.w	#0,sb_rasty(a3)
	move.w	#0,sb_rast_secx(a3)
	move.w	#0,sb_rast_secy(a3)

	move.l	sb_msgpointer(a3),a1
	moveq	#0,d0
	move.b	MP_SIGBIT(a1),d0
	moveq	#1,d1
	lsl.l	d0,d1
	move.l	d1,sb_sig_ptoc(a3)

	move.l	sb_graphbase(a3),a6
	move.w	#2,sb_gapsize(a3)
	move.w	LIB_VERSION(a6),d0
	cmp.w	#LIBV39,d0
	blt.b	no_v39_1
	move.b	#$ff,sb_v39_present(a3)
	move.w	#4,sb_gapsize(a3)
no_v39_1:
;	cmp.w	#LIBV40,d0
;	blt.b	no_v40_1
;	move.w	#4,sb_gapsize(a3)
;no_v40_1:
	move.b	gb_ChipRevBits0(a6),d0
	and.b	#SETCHIPREV_AA,d0
	cmp.b	#SETCHIPREV_AA,d0
	bne.b	no_aa1
	move.b	#$ff,sb_aa_present(a3)
	move.w	#4,sb_gapsize(a3)
no_aa1:
	lea	diskfontname(pc),a1
	moveq	#0,d0
	move.l	$4.w,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,sb_fontbase(a3)
	beq.b	lib_err2

	move.l	$4.w,a6
	move.l	#COLMEMSIZE,d0
	move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1
	jsr	_LVOAllocMem(a6)
	move.l	d0,sb_coldata(a3)	; space for 256 * 3 colbytes
	beq.b	lib_err2

	add.l	#3*256,d0

	move.l	d0,sb_coltable(a3)	; 256 bytes conversion table
	add.l	#256,d0

	move.l	d0,sb_colors(a3)	; space for colormap input 3*4*256 ?

	moveq	#0,d0
	rts
lib_err2:
	moveq	#-1,d0
	rts

close_all:
	bsr.w	free_viewports
	tst.l	sb_coldata(a3)
	beq.b	no_col_dat
	move.l	sb_coldata(a3),a1
	move.l	#COLMEMSIZE,d0
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)
	clr.l	sb_coldata(a3)
no_col_dat:
	bsr.w	close_libs
	rts
*
* Copy the data from crawl rec to local storage
*
get_args:
	move.l	sb_crawl_rec(a3),a0
	move.l	cr_txt(a0),sb_text_data(a3)
	move.l	cr_ypos(a0),d0
	addq.w	#1,d0
	lsr.w	#1,d0
	add.w	d0,d0
	move.w	d0,sb_offset_y(a3)
	move.l	cr_fontsize(a0),d0
	move.w	d0,sb_fontsize(a3)
	lea.l	cr_fontname(a0),a1
	move.l	a1,sb_fontname(a3)

	moveq	#0,d0

	move.l	d0,sb_num_rot(a3)
	move.l	d0,sb_rot_store(a3)
	move.l	cr_speed(a0),d0
	move.w	d0,sb_speed(a3)
	move.w	#1,sb_pre_depth(a3)
	rts

do_scroll:
	move.l	sb_rot_store(a3),sb_num_rot(a3)

	move.l	#0,sb_wview2(a3)		; set initial view zero

	move.l	sb_trans_inacview(a3),a5
	move.l	vb_tempbuffer(a5),sb_screenmem(a3)

	move.l	sb_trans_acview(a3),a5
	move.l	vb_vieww(a5),sb_wview2(a3)
	tst.l	sb_wview2(a3)
	beq.w	not_loaded

	bsr.w	load_font
	tst.w	d0
	bne.w	not_loaded

	bsr.w	set_size

	bsr.w	initialize_viewports

	bsr.w	set_font

	bsr.w	set_space_size
	
	bsr.w	setsam
	tst.b	d0
	bne.b	not_loaded

	bsr.w	install_50h

	lea	sigmask(pc),a0
	move.l	(a0),d0
	move.l	d0,sb_waitmask(a3)

	bsr.w	clear_screenmem

	move.l	sb_graphbase(a3),a6

	move.l	gb_ActiView(a6),sb_oldview(a3)
	move.l	gb_ActiView(a6),a1

	move.l	sb_text_data(a3),a0
	move.l	a0,sb_charpointer(a3)

	bsr.w	test_scroll

	bsr.w	restore_oldview
	
	bsr.w	remove_50h
	bsr.w	freesam

	move.l	sb_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)

not_loaded:
	moveq	#0,d0
	rts
*
* Set the different screen sizes
*
set_size:
; the minumum is apr. 150
	move.l	sb_wview2(a3),a0
	move.l	v_ViewPort(a0),a0
	cmp.l	#0,a0
	beq.b	no_vip

	move.w	vp_DWidth(a0),d0
	move.w	vp_DHeight(a0),d1
	cmp.w	#150,d0
	bge.b	no_pro_x

	move.l	#150,d0
	bra.b	no_pro_x
no_vip:
	move.l	#DEF_XSIZE,d0				; use defaults
	move.w	#DEF_MODE,sb_mode(a3)
	move.w	#DEF_PLANES,sb_planes(a3)
	bra.b	oke_vip

no_pro_x:
	move.w	vp_Modes(a0),sb_mode(a3)

	move.l	vp_RasInfo(a0),a1
	move.l	ri_BitMap(a1),a1
	moveq	#0,d2
	move.b	bm_Depth(a1),d2
	move.w	d2,sb_planes(a3)

oke_vip:
	cmp.w	#0,sb_pre_depth(a3)		; if excists take over pre
	ble.b	no_change_planes1		; user indicated
	move.w	sb_pre_depth(a3),d2
	and.w	#$7,d2
	move.w	d2,sb_planes(a3)
no_change_planes1:

	move.w	d0,sb_startx(a3)
	move.w	d0,sb_delta(a3)

	add.w	#120,sb_delta(a3)
	sub.w	#100,sb_startx(a3)

	move.w	d0,sb_lenviewx(a3)

	move.l	sb_crawl_rec(a3),a0
	move.l	cr_window_height(a0),d0
	cmp.w	sb_fontheight(a3),d0
	bge	.font_bigger
	move.w	sb_fontheight(a3),d0	; use fontheight as window height
.font_bigger:
	addq.w	#1,d0
	lsr.w	#1,d0
	add.w	d0,d0			; make length even
	move.w	d0,sb_leny(a3)

	moveq	#0,d0
	move.w	sb_lenviewx(a3),d0
	move.l	d0,d1
	add.l	d0,d0
	add.l	d1,d0			; * 3 ?????	
	add.l	#15,d0
	lsr.l	#4,d0
	lsl.l	#4,d0
	move.w	d0,sb_lenx(a3)		; true length bitmap
	lsr.l	#3,d0
	move.l	d0,sb_breedte_x(a3)	; in bytes
	rts

*
* Set the rastport on position rastx, rasty
*
set_move_first:
	move.l	sb_graphbase(a3),a6
	move.w	sb_rastx(a3),d0
	move.w	sb_rasty(a3),d1
	lea	sb_rastport(a3),a1
	jsr	_LVOMove(a6)
	rts

set_move_second:
	move.l	sb_graphbase(a3),a6
	move.w	sb_rast_secx(a3),d0
	move.w	sb_rast_secy(a3),d1
	lea	sb_rastport(a3),a1
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
	lea	sb_rastport(a3),a1
	move.l	sb_graphbase(a3),a6
	jsr	_LVOText(a6)
	lea	sb_rastport(a3),a1
	move.w	rp_cp_x(a1),sb_rastx(a3)	; same for secundair char
	move.w	rp_cp_y(a1),sb_rasty(a3)
	
	bsr.b	set_move_second
	move.l	(a7)+,a0
	moveq	#1,d0
	lea	sb_rastport(a3),a1
	move.l	sb_graphbase(a3),a6
	jsr	_LVOText(a6)
	lea	sb_rastport(a3),a1
	move.w	rp_cp_x(a1),sb_rast_secx(a3)	; same for secundair char
	move.w	rp_cp_y(a1),sb_rast_secy(a3)
	move.l	(a7)+,a0
	rts

test_scroll:
	move.l	sb_crawl_rec(a3),a0
	move.l	cr_col(a0),d1
	bsr.w	ok_ch_col		; set initial color

	lea	sb_viewblock1(a3),a5
	move.l	a5,sb_active_view(a3)
	bsr.w	make_viewport

	lea	sb_viewblock2(a3),a5
	move.l	a5,sb_inactive_view(a3)
	bsr.w	make_viewport

	move.w	sb_startx(a3),sb_rastx(a3)
	move.w	sb_startx(a3),d6

	move.w	sb_baseline(a3),sb_rasty(a3)
	move.w	sb_rastx(a3),sb_rast_secx(a3)
	move.w	sb_delta(a3),d0
	add.w	d0,sb_rast_secx(a3)

	move.w	sb_baseline(a3),sb_rast_secy(a3)

	move.l	sb_crawl_rec(a3),a0						; new add the topmargin within the window
	move.l	cr_topmargin(a0),d0
	add.w	d0,sb_rast_secy(a3)
	add.w	d0,sb_rasty(a3)

;	clr.w	sb_startx(a3)	
;	moveq	#0,d7
	move.w	sb_startx(a3),d7
	add.w	#100,d7

rep_test1:
	move.l	sb_inactive_view(a3),a5

	lea	vi_rasinfo(a5),a0

	move.w	d7,d0

	btst	#15,sb_mode(a3)
	beq.w	no_div

	tst.b	sb_v39_present(a3)		; ??????
	bne.w	no_div
	lsr.w	#1,d0
no_div:
	move.w	d0,ri_RxOffset(a0)

	lea	vi_viewport(a5),a0
	move.l	sb_graphbase(a3),a6
	jsr	_LVOScrollVPort(a6)

	bsr.w	show_viewport

	bsr.w	own_delay
	tst.w	d0
	bne.w	stop_scroll

	move.l	sb_inactive_view(a3),a5
	move.l	sb_active_view(a3),sb_inactive_view(a3)
	move.l	a5,sb_active_view(a3)

more_char:
	cmp.w	sb_rastx(a3),d6
	blt.b	no_char
	
	move.l	sb_charpointer(a3),a0
test_char:
	tst.b	(a0)
	bne.b	oke_char

; You could check here for number off rotations

	subq.w	#1,sb_space_place(a3)
	bmi.b	stop_scroll

	move.b	#' ',-1(a0)
	subq.l	#1,a0
	bra.b	no_command
go_on_rot:
	move.l	sb_text_data(a3),a0
oke_char:
	cmp.b	#10,(a0)
	bne.b	no_enter
	move.b	#' ',(a0)
;	addq.l	#1,a0
	bra.b	test_char
no_enter:
	cmp.b	#'<',(a0)
	bne.b	no_command
	bsr.b	remove_command
	bra.b	test_char
no_command:
	bsr.w	place_char
	addq	#1,a0
	move.l	a0,sb_charpointer(a3)
	bra.b	more_char
no_char:
	add.w	sb_speed(a3),d7
	add.w	sb_speed(a3),d6

	move.w	sb_startx(a3),d0
	add.w	sb_delta(a3),d0
	
	cmp.w	d0,d7
	ble.b	no_pro

	move.w	sb_delta(a3),d0
	sub.w	d0,d6
	sub.w	d0,d7
	sub.w	d0,sb_rastx(a3)
	sub.w	d0,sb_rast_secx(a3)

	IFNE DEBUG
	movem.l	d0-d7/a0-a6,-(a7)
	lea	.dbstr(pc),a0
	jsr	KPutStr
	movem.l	(a7)+,d0-d7/a0-a6
	bra	.tt
.dbstr:	dc.b	"Shift map",10,0
	even
.tt:
	ENDC

no_pro:
	bra.w	rep_test1
	
stop_scroll:
	rts	

*
* When a < is found in the text a one character command is expected
* a0 points to the command '<'
*
remove_command:
	addq	#1,a0
	move.b	(a0),d0
	cmp.b	#'<',d0			; the '<' character
	beq.b	no_com1
	cmp.b	#'s',d0
	beq.b	speed
	cmp.b	#'w',d0
	beq.b	wait_time
	cmp.b	#'c',d0
	beq.b	change_color
no_com1:
	rts
*
* Retrieve a number from the text pointed at by a0
* Read the number until a non number
* increment a0 over the last non number
*
get_number:
	moveq	#0,d1
more_number:	
	move.b	(a0)+,d0
	cmp.b	#'0',d0
	blt.b	exit_number
	cmp.b	#'9',d0
	bgt.b	exit_number
	move.w	d1,d2
	add.w	d2,d2
	lsl.w	#3,d1
	add.w	d2,d1
	sub.b	#'0',d0
	ext.w	d0
	add.w	d0,d1		
	bra.b	more_number
exit_number:
	rts
*
* A change in speed command
*
speed:
	addq.l	#1,a0			; place back the non number char
	bsr.b	get_number
	tst.w	d1
	bne.b	oke_speed1
	moveq	#1,d1
oke_speed1:
	subq.l	#1,a0
	and.w	#$ff,d1
	move.w	d1,sb_speed(a3)
	rts	
*
* Delay the scroll number * 50hz times
*
wait_time:
	addq	#1,a0
	bsr.b	get_number
	move.l	a0,-(a7)
rep_wait:
	move.l	d1,-(a7)
	move.l	sb_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)
	move.l	(a7)+,d1
	dbf	d1,rep_wait
	move.l	(a7)+,a0
	subq.l	#1,a0
	rts

change_color:
	addq	#1,a0
	bsr.b	get_number
	tst.w	d1
	bne.b	ok_ch_col
	moveq	#1,d1
ok_ch_col:
	subq.l	#1,a0
	move.l	sb_coltable(a3),a1

	IFNE DEBUG
	movem.l	d0-d7/a0-a6,-(a7)
	lea	.temp(pc),a1
	move.w	d1,(a1)
	lea	.dbstr(pc),a0
	jsr	KPutFmt
	movem.l	(a7)+,d0-d7/a0-a6
	bra	.tt
.dbstr:	dc.b	"Incoming color %d",10,0
	even
.temp:	dc.w	0
.tt:
	ENDC
wpp:

	move.b	0(a1,d1.w),d1

	IFNE DEBUG
	movem.l	d0-d7/a0-a6,-(a7)
	lea	.temp(pc),a1
	move.w	d1,(a1)
	lea	.dbstr(pc),a0
	jsr	KPutFmt
	movem.l	(a7)+,d0-d7/a0-a6
	bra	.tt
.dbstr:	dc.b	"Set color %d",10,0
	even
.temp:	dc.w	0
.tt:
	ENDC

	move.l	a0,-(a7)
	lea	sb_rastport(a3),a1
	move.l	sb_graphbase(a3),a6
	move.w	d1,d0
	jsr	_LVOSetAPen(a6)
	move.l	(a7)+,a0
	rts
*
* load a diskfont for the rastports
*
load_font:
	move.l	sb_fontname(a3),a1
	lea	sb_fontattr(a3),a0
	move.l	a1,ta_Name(a0)	
	move.w	sb_fontsize(a3),ta_YSize(a0)
	move.l	sb_fontbase(a3),a6
	jsr	_LVOOpenDiskFont(a6)
	move.l	d0,sb_fontpointer(a3)
	beq.b	get_font_err
	move.l	d0,a0
	move.w	tf_YSize(a0),sb_fontheight(a3)
	move.w	tf_Baseline(a0),sb_baseline(a3)
	moveq	#0,d0
	rts

*
* Set the loaded font in the rastport
*
set_font:

	lea	sb_rastport(a3),a1
	move.l	sb_graphbase(a3),a6
	move.l	sb_fontpointer(a3),a0
	jsr	_LVOSetFont(a6)	
	moveq	#0,d0
	rts

get_font_err:
	moveq	#-1,d0
	rts

*
* Initialize two views with each a viewport
*
initialize_viewports:

	lea	sb_viewblock1(a3),a5
	bsr.w	create_viewport

	lea	sb_viewblock2(a3),a5
	bsr.w	create_viewport

	bsr.w	init_extra_views		; init top and bot view

	move.w	sb_offset_y(a3),d0		; y offset -> d0

	lea	sb_viewblocktop(a3),a1
	lea	vi_viewport(a1),a1
	move.w	vp_DHeight(a1),d2		; active height -> d2
	move.w	d0,vp_DHeight(a1)		; set new height to start
						; of the scroll
	move.w	sb_leny(a3),d3			; scrollviewheight -> d3
	move.w	d3,d1

	add.w	d0,d1
	cmp.w	d1,d2				; if total scroll height >
	ble.b	no_sec_view			; no need for bottom view
						
	lea	sb_viewblocktop(a3),a0
	lea	sb_viewblockbot(a3),a1

	lea	vi_viewport(a0),a0
	lea	vi_viewport(a1),a2		; bottom viewport -> a2

	move.l	a2,vp_Next(a0)			; place bottom viewport in top
	
	sub.w	d0,vp_DHeight(a2)		; smaller height
	sub.w	d3,vp_DHeight(a2)
	
	move.w	sb_gapsize(a3),d7
	sub.w	d7,vp_DHeight(a2)
	sub.w	d7,vp_DHeight(a2)

	add.w	d3,vp_DyOffset(a2)		; add fontsize
	add.w	d0,vp_DyOffset(a2)		; add portion above the scroll

	add.w	d7,vp_DyOffset(a2)		; gapsize ???
	add.w	d7,vp_DyOffset(a2)		; gapsize ???

	lea	vi_rasinfo(a1),a2

	add.w	d3,d0
	add.w	d7,d0
	add.w	d7,d0
	move.w	d0,ri_RyOffset(a2)		; rest picture

no_sec_view:
	lea		sb_viewblock1(a3),a4
	lea		sb_viewblocktop(a3),a5
	move.l		sb_graphbase(a3),a6
	lea		vi_view(a4),a0
	lea		vi_viewport(a5),a1
	jsr		_LVOMakeVPort(a6)

	lea		sb_viewblock1(a3),a4
	lea		sb_viewblockbot(a3),a5
	move.l		sb_graphbase(a3),a6
	lea		vi_view(a4),a0
	lea		vi_viewport(a5),a1
	jsr		_LVOMakeVPort(a6)

	lea		sb_viewblock2(a3),a4
	lea		sb_viewblocktop(a3),a5
	move.l		sb_graphbase(a3),a6
	lea		vi_view(a4),a0
	lea		vi_viewport(a5),a1
	jsr		_LVOMakeVPort(a6)

	lea		sb_viewblock2(a3),a4
	lea		sb_viewblockbot(a3),a5
	move.l		sb_graphbase(a3),a6
	lea		vi_view(a4),a0
	lea		vi_viewport(a5),a1
	jsr		_LVOMakeVPort(a6)

;	bsr.w	make_extra_views

no_extra_bits2:

	lea	sb_rastport(a3),a1
	jsr	_LVOInitRastPort(a6)

	moveq	#RP_JAM2,d0
	lea	sb_rastport(a3),a1
	jsr	_LVOSetDrMd(a6)

	lea	sb_viewblock1(a3),a5
	lea	sb_rastport(a3),a1
	lea	vi_bitmap(a5),a0
	move.l	a0,rp_BitMap(a1)
	rts
*
* find out how long a space is and calculate the number to print to fill
* the scroll with empty spaces.
*
set_space_size:
	move.l	sb_graphbase(a3),a6
	moveq	#0,d0
	move.w	sb_baseline(a3),d1
	lea	sb_rastport(a3),a1
	jsr	_LVOMove(a6)

	moveq	#1,d0
	lea	space(pc),a0
	lea	sb_rastport(a3),a1
	move.l	sb_graphbase(a3),a6
	jsr	_LVOText(a6)

	move.w	#1,sb_space_place(a3)
	lea	sb_rastport(a3),a1
	move.w	rp_cp_x(a1),sb_space_size(a3)		; size of one space ??
	move.w	sb_lenviewx(a3),d1
	move.w	rp_cp_x(a1),d0
	subq.w	#1,d0
	bmi.b	.no_sp
	divu	d0,d1
	and.w	#$ffff,d1			; number off spaces at the end
	addq.w	#1,d1
	move.w	d1,sb_space_place(a3)
.no_sp:
	rts
	
space:	dc.b	' ',0

free_viewports:
	lea	sb_viewblocktop(a3),a5
	bsr.w	free_view

	lea	sb_viewblockbot(a3),a5
	bsr.w	free_view

	lea	sb_viewblock2(a3),a5
	bsr.w	free_view

	lea	sb_viewblock1(a3),a5
	bsr.w	free_view
	rts

*
* The extra structs are needed for the scroll in the middle of the picture
* both viewblocktop and viewblockbot are initialized with the values
* of the active viewport
*
init_extra_views:
	lea		sb_viewblocktop(a3),a5

	bsr.b		init_extra_view
	lea		sb_viewblockbot(a3),a5

	bsr.b		init_extra_view
	rts

init_extra_view:

	move.l		sb_graphbase(a3),a6
	lea.l		vi_view(a5),a1
	jsr		_LVOInitView(a6)
	lea.l		vi_viewport(a5),a0
	jsr		_LVOInitVPort(a6)

	move.l		sb_wview2(a3),a0
	lea.l		vi_view(a5),a1

	move.w		v_Modes(a0),d0
	or.w            sb_or_vmode_mask(a3),d0 
	and.w           sb_and_vmode_mask(a3),d0
	move.w		d0,v_Modes(a1)

	move.w		sb_viewx(a3),v_DxOffset(a1)
	move.w		sb_viewy(a3),v_DyOffset(a1)
	
	moveq		#VIEW_EXTRA_TYPE,d0
	move.l		sb_graphbase(a3),a6
	jsr		_LVOGfxNew(a6)
	move.l		d0,vi_vextra(a5)
	beq		.no_ass

	move.l		d0,a0
	move.l		sb_monitorspec(a3),ve_Monitor(a0)

	moveq		#VIEWPORT_EXTRA_TYPE,d0
	jsr		_LVOGfxNew(a6)
	move.l		d0,vi_vpextra(a5)
	beq		.no_ass

	move.w		#0,a0
	lea		sb_dimquery(a3),a1
	moveq		#dim_SIZEOF,d0
	move.l		#DTAG_DIMS,d1
	move.l		sb_monid(a3),d2
	or.w		sb_mode(a3),d2
	or.w		sb_or_vmode_mask(a3),d2
	jsr		_LVOGetDisplayInfoData(a6)

	lea		sb_dimquery(a3),a0
	move.l		vi_vpextra(a5),a1
	move.l		dim_MaxOScan(a0),vpe_DisplayClip(a1)
	move.l		dim_MaxOScan+4(a0),vpe_DisplayClip+4(a1)

	lea		vi_view(a5),a0
	move.l		vi_vextra(a5),a1
	jsr		_LVOGfxAssociate(a6)

	lea		vi_view(a5),a0
	or.w		#EXTEND_VSTRUCT,v_Modes(a0)
.no_ass:

	lea		vi_viewport(a5),a1
	move.l		a1,v_ViewPort(a0)
	move.l		#0,vp_Next(a1)

	lea		vi_viewport(a5),a0

	move.l		sb_wview2(a3),a1
	move.l		v_ViewPort(a1),a1
	move.l		vp_RasInfo(a1),a1
	move.l		ri_BitMap(a1),a1

	lea		vi_rasinfo(a5),a2
	move.l		a1,ri_BitMap(a2)
	move.w		#0,ri_RyOffset(a2)		; clear settings
	move.l		a2,vp_RasInfo(a0)

	move.l		sb_wview2(a3),a1
	move.l		v_ViewPort(a1),a1
	move.w		vp_DWidth(a1),vp_DWidth(a0)
	move.w		vp_DHeight(a1),vp_DHeight(a0)
	move.w		vp_DxOffset(a1),vp_DxOffset(a0)
	move.w		vp_DyOffset(a1),vp_DyOffset(a0)

	move.l		sb_monid(a3),d0
	or.w		sb_mode(a3),d0
	move.w		sb_mode(a3),d3
	and.w		#~$884,d3
	bne		.nolow2
	and.w		#~SUPER_KEY,d0
	or.w		sb_alternate_lowres(a3),d0
.nolow2:
	move.l		sb_graphbase(a3),a6
	jsr		_LVOFindDisplayInfo(a6)
	move.l		sb_tags(a3),a0
	move.l		d0,20(a0)		

	moveq		#32,d0
	tst.b		sb_aa_present(a3)
	beq		.noaa12
	move.l		#256,d0
.noaa12:
	jsr		_LVOGetColorMap(a6)
	tst.l		d0
	beq.w		no_colormap2

	move.l		d0,a0
	move.l		sb_tags(a3),a1
	move.l		vi_vpextra(a5),12(a1)
	lea		vi_viewport(a5),a2
	move.l		a2,4(a1)
	move.l		sb_graphbase(a3),a6
	jsr		_LVOVideoControl(a6)	; connect colormap to temp view
	bsr		laad_kleuren
no_colormap2:
	rts

no_view1:
	moveq		#-1,d0
	rts

make_extra_views:
	lea		sb_viewblocktop(a3),a5
	bsr.b		make_viewport
	lea		sb_viewblockbot(a3),a5
	bsr.b		make_viewport
	rts

*
* Simply create copperlists for the viewblock in a5
*
make_viewport:
	move.l		sb_graphbase(a3),a6
	lea		vi_view(a5),a0
	lea		vi_viewport(a5),a1
	jsr		_LVOMakeVPort(a6)

	lea		vi_view(a5),a1
	jsr		_LVOMrgCop(a6)
	rts
*
* a5 points to a vi_block with view viewport etc.
* This creates the viewports for the actual scroll bitmap
*		
create_viewport:

	move.l		sb_graphbase(a3),a6
	lea		vi_view(a5),a1
	jsr		_LVOInitView(a6)

	lea		vi_view(a5),a1
	move.l		sb_wview2(a3),a0
	move.w		v_DxOffset(a0),v_DxOffset(a1)
	move.w		v_DyOffset(a0),v_DyOffset(a1)

	lea		vi_viewport(a5),a0
	jsr		_LVOInitVPort(a6)

	lea		vi_view(a5),a0
	lea		vi_viewport(a5),a1
	move.l		a1,v_ViewPort(a0)

	move.w		sb_mode(a3),d0
	or.w            sb_or_vmode_mask(a3),d0 
	and.w           sb_and_vmode_mask(a3),d0
	move.w		d0,v_Modes(a0)
	and.w		#~V_HAM,v_Modes(a0)

	lea		vi_bitmap(a5),a0
	move.w		sb_planes(a3),d0		; depth

	move.l		sb_breedte_x(a3),d1		; breedte in bytes * 8
	mulu		sb_planes(a3),d1
	lsl.l		#3,d1				; width
	move.w		sb_leny(a3),d2			; height
	jsr		_LVOInitBitMap(a6)

	lea		vi_bitmap(a5),a0

	move.l		sb_screenmem(a3),d0
	move.l		sb_breedte_x(a3),d1

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

	clr.w		ri_RxOffset(a0)
	clr.w		ri_RyOffset(a0)
	clr.l		ri_Next(a0)

	lea		vi_viewport(a5),a0
	lea		vi_rasinfo(a5),a1
	move.l		a1,vp_RasInfo(a0)
	move.w		sb_lenviewx(a3),vp_DWidth(a0)
	move.w		sb_leny(a3),vp_DHeight(a0)
	move.w		sb_mode(a3),vp_Modes(a0)
	and.w		#~V_HAM,vp_Modes(a0)

	move.l		sb_wview2(a3),a1
	move.l		v_ViewPort(a1),a1

	move.w		vp_DxOffset(a1),vp_DxOffset(a0)
	move.w		vp_DyOffset(a1),vp_DyOffset(a0)

	lea		sb_viewblocktop(a3),a1
	lea.l		vi_viewport(a1),a1

	move.w		sb_offset_y(a3),d0

	move.w		sb_gapsize(a3),d7
	add.w		d7,d0
	add.w		d0,vp_DyOffset(a0)

	move.l		a1,vp_Next(a0)

	move.l		sb_graphbase(a3),a6
	moveq		#VIEW_EXTRA_TYPE,d0
	jsr		_LVOGfxNew(a6)
	tst.l		d0
	beq		no_view2
	move.l		d0,vi_vextra(a5)

	move.l		d0,a1
	lea.l		vi_view(a5),a0
	jsr		_LVOGfxAssociate(a6)

	lea.l		vi_view(a5),a0
	or.w		#EXTEND_VSTRUCT,v_Modes(a0)

	move.l		vi_vextra(a5),a0
	move.l		sb_monitorspec(a3),ve_Monitor(a0)

	move.l		sb_graphbase(a3),a6
	moveq		#VIEWPORT_EXTRA_TYPE,d0
	jsr		_LVOGfxNew(a6)
	move.l		d0,vi_vpextra(a5)
	beq		no_view2

	IFNE DEBUG
	movem.l	d0-d7/a0-a6,-(a7)
	lea.l	.dat(pc),a1
	move.l	d0,(a1)
	lea	.dbstr(pc),a0
	jsr	KPutFmt
	movem.l	(a7)+,d0-d7/a0-a6
	bra	.tt
.dbstr:	dc.b	"Alloc vpextra %lx",10,0
	even
.dat:	dc.l	0
.tt:
	ENDC

	move.l		d0,a1
	lea.l		vi_viewport(a5),a0
	jsr		_LVOGfxAssociate(a6)

	move.l		sb_tags(a3),a0
	move.l		vi_vpextra(a5),12(a0)

	move.w		#0,a0
	lea		sb_dimquery(a3),a1
	moveq		#dim_SIZEOF,d0
	move.l		#DTAG_DIMS,d1
	move.l		sb_monid(a3),d2
	or.w		sb_mode(a3),d2
	or.w		sb_or_vmode_mask(a3),d2
	jsr		_LVOGetDisplayInfoData(a6)

	lea		sb_dimquery(a3),a0
	move.l		vi_vpextra(a5),a1

	move.l		dim_MaxOScan(a0),vpe_DisplayClip(a1)
	move.l		dim_MaxOScan+4(a0),vpe_DisplayClip+4(a1)

	move.l		sb_monid(a3),d0
	or.w		sb_mode(a3),d0
	move.w		sb_mode(a3),d3
	and.w		#~$884,d3
	bne		.nolow2
	and.w		#~SUPER_KEY,d0
	or.w		sb_alternate_lowres(a3),d0
.nolow2:
	and.l	#~V_HAM,d0

	jsr		_LVOFindDisplayInfo(a6)
	move.l		sb_tags(a3),a0
	move.l		d0,20(a0)		

	move.l		sb_nroffcols(a3),d0
	jsr		_LVOGetColorMap(a6)
	tst.w		d0
	beq		no_colormap

	move.l		d0,a0
	move.l		sb_tags(a3),a1
	lea.l		vi_viewport(a5),a2
	move.l		a2,4(a1)
	jsr		_LVOVideoControl(a6)

	lea		vi_viewport(a5),a0
	move.l		sb_colors(a3),a1
	move.l		sb_graphbase(a3),a6

	tst.b		sb_aa_present(a3)
	bne.b		.aa
	
	move.l		sb_nroffcols(a3),d0
	jsr		_LVOLoadRGB4(a6)
	bra		.end
.aa:	
	jsr		_LVOLoadRGB32(a6)	
.end
	moveq		#0,d0
	rts

no_view2:
	moveq		#-1,d0
	rts

show_viewport:
	move.l		sb_graphbase(a3),a6
	lea		vi_view(a5),a1
	jsr		_LVOLoadView(a6)

	rts

restore_oldview:
	move.l		sb_graphbase(a3),a6
	lea		vi_view(a5),a0

cleanexit:
	move.l		sb_graphbase(a3),a6
	tst.l		sb_oldview(a3)
	beq.b		no_oldview
	move.l		sb_oldview(a3),a1
	jsr		_LVOLoadView(a6)
	jsr		_LVOWaitTOF(a6)
no_oldview:
	rts

free_view:
	move.l		sb_graphbase(a3),a6
;	lea		vi_view(a5),a0
;	jsr		_LVOGfxLookUp(a6)
;	move.l		d0,vi_vextra(a5)

;	lea		vi_viewport(a5),a0
;	jsr		_LVOGfxLookUp(a6)
;	move.l		d0,vi_vpextra(a5)

	IFNE DEBUG
	movem.l	d0-d7/a0-a6,-(a7)
	lea.l	.dat2(pc),a1
	move.l	vi_vextra(a5),(a1)
	move.l	vi_vpextra(a5),4(a1)
	lea	.dbstr2(pc),a0
	jsr	KPutFmt
	movem.l	(a7)+,d0-d7/a0-a6
	bra	.tt2
.dbstr2:	dc.b	"vextra,vpextra %lx,%lx",10,0
	even
.dat2:	dc.l	0,0
.tt2:
	ENDC

	lea		vi_viewport(a5),a0
	move.l		vp_ColorMap(a0),d0
	beq		.nocolormap
	move.l		d0,a0
	jsr		_LVOFreeColorMap(a6)

.nocolormap:
	lea		vi_viewport(a5),a0
	move.l		#0,vp_Next(a0)
	jsr		_LVOFreeVPortCopLists(a6)

	lea		vi_view(a5),a0
	tst.l		v_LOFCprList(a0)
	beq.b		.no_lofmem
	move.l		v_LOFCprList(a0),a0
	jsr		_LVOFreeCprList(a6)
.no_lofmem:
	lea		vi_view(a5),a0
	tst.l		v_SHFCprList(a0)
	beq.b		.no_shfmem
	move.l		v_SHFCprList(a0),a0
	jsr		_LVOFreeCprList(a6)
.no_shfmem:
	move.l		vi_vextra(a5),d0
	beq		.nov
	move.l		d0,a0
	jsr		_LVOGfxFree(a6)
.nov:
	move.l		vi_vpextra(a5),d0
	beq		.novp
	move.l		d0,a0

	IFNE DEBUG
	movem.l	d0-d7/a0-a6,-(a7)
	lea.l	.dat(pc),a1
	move.l	d0,(a1)
	lea	.dbstr(pc),a0
	jsr	KPutFmt
	movem.l	(a7)+,d0-d7/a0-a6
	bra	.tt
.dbstr:	dc.b	"Free vpextra %lx",10,0
	even
.dat:	dc.l	0
.tt:
	ENDC

	jsr		_LVOGfxFree(a6)
.novp:
	
no_colormap:
	moveq		#0,d0
	rts

clear_screenmem:
	move.l	sb_breedte_x(a3),d0
	mulu	sb_leny(a3),d0
	move.w	sb_planes(a3),d7
	moveq	#0,d1
	bra.s	.incc
.cc:	add.l	d0,d1
.incc:	dbf	d7,.cc

	lsr.l	#2,d1
	move.l	sb_screenmem(a3),a0

	moveq	#0,d0
cc2:	move.l	d0,(a0)+
	subq.l	#1,d1
	bpl.s	cc2
	rts
*
* laad_kleuren zet de kleuren afhankelijk van het OS
* in a5 de pointer naar het viewblok
*
*
laad_kleuren:
	move.l		sb_graphbase(a3),a6
	tst.b		sb_aa_present(a3)
	beq		.no_v39_2
	move.l		sb_waarcolors32(a3),a1
	lea.l		vi_viewport(a5),a0
	IFNE DEBUG
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	sb_waarcolors32(a3),a1
	lea	.dbstr(pc),a0
	jsr	KPutFmt
	movem.l	(a7)+,d0-d7/a0-a6
	bra	.tt
.dbstr:	dc.b	"Load org rgb32 %d,%d,%lx,%lx,%lx",10,"%lx,%lx,%lx",10,0
	even
.tt:
	ENDC

	jsr		_LVOLoadRGB32(a6)
	rts
.no_v39_2:
	lea.l		vi_viewport(a5),a0
	move.l		vp_ColorMap(a0),d0
	move.l		sb_trans_acview(a3),a1
	lea		vb_colors(a1),a1		; word colors org
;	move.l		sb_colors(a3),a1
	moveq		#32,d0
	jsr		_LVOLoadRGB4(a6)
	rts


close_libs:
	tst.l	sb_fontbase(a3)
	beq.b	exit2
	tst.l	sb_fontpointer(a3)
	beq.b	no_font_exit
	move.l	sb_fontpointer(a3),a1
	move.l	sb_graphbase(a3),a6
	jsr	_LVOCloseFont(a6)
	clr.l	sb_fontpointer(a3)
no_font_exit:
	move.l	sb_fontbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	sb_fontbase(a3)
exit2:
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
	move.l	$4.w,a6
	move.l	off_task(a1),a1
	move.l	sigmask(pc),d0
	jsr	_LVOSignal(a6)
	movem.l	(a7)+,d0/a1/a0
	moveq	#0,d0
	rts

proc50hnaam:	dc.b	"50Hz crawl",0
	even
		
intstruct50h:
	dc.l	0,0
	dc.b	2,15		; type en pri
	dc.l	0,0		; pointer naar naam en data
	dc.l	0

teller:		dc.l	0
tellerstart:	dc.l	0
task:		dc.l	0
signum:		dc.l	0
sigmask:	dc.l	0
handle_installed:	dc.b	0,0

off_teller 	= 0
off_tel_start 	= 4
off_task 	= 8
off_signum 	= 12
off_sigmask 	= 16

diskfontname:	dc.b	'diskfont.library',0
	END

