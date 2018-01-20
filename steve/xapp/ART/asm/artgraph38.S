*
* File	: artgraph.s
* Uses	: art_data.i
* Date	: 1 february 1993 
* Author: ing. C. Lieshout
* Desc	: Try to display some art idea
*

	INCDIR "include:"
	INCLUDE "exec/types.i"
	INCLUDE "exec/execbase.i"
	INCLUDE "exec/funcdef.i"
	INCLUDE	"exec/memory.i"
	INCLUDE "graphics/gfxbase.i"

	INCLUDE	"hardware/custom.i"
	INCLUDE	"hardware/blit.i"
	INCLUDE	"hardware/dmabits.i"
	
	INCLUDE "graphics/view.i"
	INCLUDE "graphics/gfx.i"
	INCLUDE "graphics/rastport.i"
	INCLUDE	"graphics/text.i"
	INCLUDE	"graphics/videocontrol.i"
	INCLUDE "intuition/intuitionbase.i"
        INCLUDE "intuition/preferences.i"
        INCLUDE	"libraries/dos.i"

	INCDIR	"wp:inclibs/"
	INCLUDE	"diskfont_lib.i"
	INCLUDE	"intuition_lib.i"
	INCLUDE	"dos_lib.i"
	INCLUDE "exec_lib.i"
	INCLUDE "graphics_libv39.i"

	INCDIR	"art:asm/"
	INCLUDE	"art_data.i"
	INCDIR	"wp:asm/"
	INCLUDE "proces.i"
	INCLUDE	"system.i"

;MON_ID = DEFAULT_MONITOR_ID!HIRESLACE_KEY
MON_ID = DBLNTSC_MONITOR_ID

DEBUG = 0
XAPP = 1

; Default values when there is no viewport

DEF_MODE = V_LACE+V_HIRES
DEF_PLANES = 1
REP_LINES = 0

BOUND = 10
NLINES = 20
HO = 566
BR = 704
LIBV39 = 39

POINT_MEM_SIZE = 1000		; Space for 60 points ( 2*(p*8) + 4 = 1000 )

	IF	DEBUG
	XREF	KPutFmt
	XREF	KPutStr
	PRINTT	"De debug info staat aan dus je moet nog met debug.lib linken"
	ENDC

	IFEQ XAPP
start:
	lea	datablock(pc),a3

	move.l	#MON_ID,sb_monid(a3)

	lea	temp_tags(pc),a0
	move.l	a0,sb_tags(a3)

	bsr	get_points_mem
	tst.l	d0
	beq	no_points_mem
	
	bsr	setsam
	bne	no_samsignal

	bsr	install_50h
	bsr.w	openlibs
	bne	close_it

	lea	sigmask(pc),a0
	move.l	(a0),d0
	move.l	d0,sb_waitmask(a3)

	bsr	get_prefs
	bsr	set_size

	lea	colors(pc),a0
	move.l	a0,sb_colors(a3)
	bsr	set_fades
		
	bsr	get_screenmem
	bne	close_it

	bsr	initialize_viewports

	bsr	show_viewport

	move.w	#NLINES,sb_num_lines(a3)
	bsr	artit

	move.b		sb_cols(a3),d1
	move.b		sb_cols+1(a3),d2
	move.b		sb_cols+2(a3),d3

ww:	btst	#6,$bfe001
	bne	ww

	bsr	free_viewports
	
	bsr	free_screenmem
close_it:
	bsr	close_libs

	bsr	remove_50h
	bsr	freesam
no_samsignal:
	bsr	free_points_mem
no_points_mem:
	rts

datablock:	blk.b	sbsc_SIZEOF,0
	even
	ENDC

set_fades:
	move.b	#$0,sb_cols(a3)
	move.b	#$0,sb_cols+1(a3)
	move.b	#$0,sb_cols+2(a3)
	move.b	#1,sb_inc_cols(a3)
	move.b	#1,sb_inc_cols+1(a3)
	move.b	#1,sb_inc_cols+2(a3)
	move.w	#0,sb_fade_count(a3)
	rts
	
get_points_mem:
	move.l	$4,a6
	move.l	#MEMF_CLEAR,d1
	move.l	#POINT_MEM_SIZE,d0
	jsr	_LVOAllocMem(a6)
	move.l	d0,sb_points_mem(a3)
	rts
free_points_mem:
	move.l	sb_points_mem(a3),d0
	beq	no_f_mem
	move.l	d0,a1
	move.l	#POINT_MEM_SIZE,d0
	move.l	$4,a6
	jsr	_LVOFreeMem(a6)
no_f_mem:
	rts
*
* Perform a sort of art line drawing display 
*
artit:
	bsr	copy_points
	bsr	set_bounds

*
* Since there is nothing on the screen start with the first X lines
*

	move.w	sb_num_lines(a3),d7
rep_i_lines:
	move.l	sb_start_points(a3),a0
	bsr	get_lines_pointer1
	move.l	#$ffff,a2				; draw
	bsr	drawit

	move.l	sb_start_points(a3),a0
	move.l	sb_start_deltas(a3),a1
	bsr	add_deltas

	move.l	sb_start_points(a3),a0
	move.l	sb_start_deltas(a3),a1
	bsr	check_points
	dbf	d7,rep_i_lines

	move.w	#REP_LINES,sb_replines(a3)
rep_art:
	move.l	sb_start_points(a3),a0
	bsr	get_lines_pointer1
	move.l	#$ffff,a2				; draw
	bsr	drawit

	move.l	sb_start_points(a3),a0
	move.l	sb_start_deltas(a3),a1
	bsr	add_deltas

	move.l	sb_start_points(a3),a0
	move.l	sb_start_deltas(a3),a1
	bsr	check_points

*
* Clear the lines from the beginning
*
	move.l	sb_stop_points(a3),a0
	bsr	get_lines_pointer1
	move.l	#$0,a2				; draw
	bsr	drawit

	move.l	sb_stop_points(a3),a0
	move.l	sb_stop_deltas(a3),a1
	bsr	add_deltas

	move.l	sb_stop_points(a3),a0
	move.l	sb_stop_deltas(a3),a1
	bsr	check_points

	sub.w	#1,sb_replines(a3)
	bpl	rep_art
	move.w	#REP_LINES,sb_replines(a3)	
	bsr	own_delay
	tst.w	d0
	bne	exit_artit


; perform some fade functions here
	cmp.w	#254,sb_fade_count(a3)
	ble	no_new_inc

	bsr	get_new_cols
no_new_inc:
	move.b	sb_inc_cols(a3),d0
	add.b	d0,sb_cols(a3)
	move.b	sb_inc_cols+1(a3),d0
	add.b	d0,sb_cols+1(a3)
	move.b	sb_inc_cols+2(a3),d0
	add.b	d0,sb_cols+2(a3)
	add.w	#1,sb_fade_count(a3)

	tst.b	sb_aa_present(a3)
	beq	no_aa_fade

	lea	sb_viewblock1(a3),a5

	bsr	set_color

	IFNE 0
	lea	vi_view(a5),a0
	jsr	_LVOGfxLookUp(a6)
	move.l	d0,a0
	beq	.nop
	move.l	$1c(a0),d0
	beq	.nop
	move.l	d0,a0
	move.l	$98(a0),d0
	beq	.nop
	move.l	d0,a0
	lea	vi_view(a5),a1
	move.l	a1,-(a7)
	jsr	(a0)	
	addq.l	#4,a7
	bra	rep_art
.nop:
	ENDC
	
	lea	vi_view(a5),a1
	jsr	_LVOLoadView(a6)

	bra	rep_art

no_aa_fade:
	moveq	#0,d1
	moveq	#0,d2
	moveq	#0,d3
	move.b	sb_cols(a3),d1
	move.b	sb_cols+1(a3),d2
	move.b	sb_cols+2(a3),d3
	lsr.l	#4,d1
	lsr.l	#4,d2
	lsr.l	#4,d3
	moveq	#1,d0

	lea	sb_viewblock1(a3),a5
	lea	vi_viewport(a5),a0
	move.l	sb_graphbase(a3),a6
	jsr	_LVOSetRGB4(a6)

	lea	vi_view(a5),a1
	jsr	_LVOLoadView(a6)
	bra	rep_art

exit_artit:
	rts

set_color:
	move.b	sb_cols(a3),d1
	move.b	d1,d7
	lsl.w	#8,d7
	move.b	d1,d7
	move.w	d7,d1
	swap	d1
	move.w	d7,d1
	
	move.b	sb_cols+1(a3),d2
	move.b	d2,d7
	lsl.w	#8,d7
	move.b	d2,d7
	move.w	d7,d2
	swap	d2
	move.w	d7,d2
	move.b	sb_cols+2(a3),d3
	move.b	d3,d7
	lsl.w	#8,d7
	move.b	d3,d7
	move.w	d7,d3
	swap	d3
	move.w	d7,d3
	moveq	#1,d0
	lea	vi_viewport(a5),a0
	move.l	sb_graphbase(a3),a6
	jsr	_LVOSetRGB32(a6)
	rts
	
get_new_cols:
	moveq	#15,d0
	bsr	rnd
	lsr.w	#1,d0
	and.w	#7,d0
	bne	no_zero
	moveq	#1,d0
no_zero:
	move.b	#0,sb_inc_cols(a3)
	move.b	#0,sb_inc_cols+1(a3)
	move.b	#0,sb_inc_cols+2(a3)

	move.b	sb_cols(a3),d1
	move.b	sb_cols+1(a3),d1
	move.b	sb_cols+2(a3),d1

	btst	#0,d0
	bne	no_ch_col1
	cmp.b	#$f0,sb_cols(a3)
	bhi	no_d_col1
	move.b	#1,sb_inc_cols(a3)
	bra	no_ch_col1
no_d_col1:
	move.b	#-1,sb_inc_cols(a3)
no_ch_col1:

	btst	#1,d0
	bne	no_ch_col2
	cmp.b	#$f0,sb_cols+1(a3)
	bhi	no_d_col2
	move.b	#1,sb_inc_cols+1(a3)
	bra	no_ch_col2
no_d_col2:
	move.b	#-1,sb_inc_cols+1(a3)
no_ch_col2:
	btst	#2,d0
	bne	no_ch_col3
	cmp.b	#$f0,sb_cols+2(a3)
	bhi	no_d_col3
	move.b	#1,sb_inc_cols+2(a3)
	bra	no_ch_col3
no_d_col3:
	move.b	#-1,sb_inc_cols+2(a3)
no_ch_col3:

	move.w	#1,sb_fade_count(a3)
	rts

*
* random generator from magazine
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
	beq	no_div7		; DIV zero test
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
	dc.l	1234567
	
*
* Increment the points with the delta values
* a0 points to the point values
* a1 points to the delta values
*
add_deltas:
rep_add_d:
	move.w	(a1)+,d0
	move.w	(a1)+,d1
	cmp.w	#-1000,d0
	beq	end_add_d
	add.w	d0,(a0)+
	add.w	d1,(a0)+
	bra	add_deltas
end_add_d:
	rts

*
* Draw the lines pointed to by a1
* With the points pointed to by a0
*
drawit:
	movem.l	d6/d7,-(a7)
rep_draw1:
	move.w	(a1)+,d6
	move.w	(a1)+,d7
	cmp.w	#-1000,d6
	beq	exit_drawit
	lsl.w	#2,d6
	lsl.w	#2,d7
	move.w	0(a0,d6.w),d0
	move.w	2(a0,d6.w),d1
	move.w	0(a0,d7.w),d2
	move.w	2(a0,d7.w),d3
	move.w	(a1)+,d6
	bsr	drawline
;	bsr	show_it
	bra	rep_draw1
exit_drawit:
	movem.l	(a7)+,d6/d7
;	bsr	show_it
	rts

show_it:
	movem.l	d0/d1/a0/a1/a5,-(a7)
	lea	sb_viewblock1(a3),a5
	bsr	show_viewport
	movem.l	(a7)+,a0/a1/a5/d0/d1
	rts
*
* a0 points to the point to check 
* a1 points to the delta values
*
check_points:
rep_cp:	
	move.w	(a0)+,d0
	move.w	(a0)+,d1
	cmp.w	#-1000,d0
	beq	end_cp
	move.w	sb_hi_boundx(a3),d2
	move.w	sb_lo_boundx(a3),d3
	cmp.w	d0,d2
	bhi	no_xh1
	neg.w	(a1)
no_xh1:
	cmp.w	d3,d0
	bhi	no_xl1
	neg.w	(a1)
no_xl1:

	move.w	sb_hi_boundy(a3),d2
	move.w	sb_lo_boundy(a3),d3
	cmp.w	d1,d2
	bhi	no_yh1
	neg.w	2(a1)
no_yh1:	
	cmp.w	d3,d1
	bhi	no_yl1
	neg.w	2(a1)
no_yl1:
	lea	4(a1),a1
	bra	rep_cp	
end_cp:

	rts

set_bounds:
	move.w	sb_lenviewx(a3),d2
	move.w	sb_lenviewy(a3),d3
	sub.w	#BOUND,d2
	sub.w	#BOUND,d3
	move.w	d2,sb_hi_boundx(a3)
	move.w	d3,sb_hi_boundy(a3)
	move.w	#BOUND,sb_lo_boundx(a3)
	move.w	#BOUND,sb_lo_boundy(a3)
	rts
*
* Return a pointer to a lines table in a0
*
get_lines_pointer:
	lea	lines1(pc),a0
	rts

get_lines_pointer1:
	lea	lines1(pc),a1
	rts

*
* Return a pointer to a points table in a0
*
get_points_pointer:
	lea	points1(pc),a0
	rts

*
* Return a pointer to a delta table in a0
*
get_delta_pointer:
	lea	delta1(pc),a0
	rts

get_delta_pointer1:
	lea	delta1(pc),a0
	rts

copy_points:
	bsr	get_points_pointer
	move.l	sb_points_mem(a3),a1
	move.l	a1,sb_start_points(a3)
rep_cp_p1:
	move.w	(a0)+,d0
	move.w	(a0)+,d1
	move.w	d0,(a1)+
	move.w	d1,(a1)+
	cmp.w	#-1000,d0
	bne	rep_cp_p1
end_cp_p1:

	move.l	a1,sb_start_deltas(a3)
	bsr	get_delta_pointer
rep_cp_d1:
	move.w	(a0)+,d0
	move.w	(a0)+,d1
	move.w	d0,(a1)+
	move.w	d1,(a1)+
	cmp.w	#-1000,d0
	bne	rep_cp_d1
end_cp_d1:

	bsr	get_points_pointer
	move.l	a1,sb_stop_points(a3)

rep_cp_p2:
	move.w	(a0)+,d0
	move.w	(a0)+,d1
	move.w	d0,(a1)+
	move.w	d1,(a1)+
	cmp.w	#-1000,d0
	bne	rep_cp_p2
end_cp_p2:
	move.l	a1,sb_stop_deltas(a3)
	bsr	get_delta_pointer

rep_cp_d2:
	move.w	(a0)+,d0
	move.w	(a0)+,d1
	move.w	d0,(a1)+
	move.w	d1,(a1)+
	cmp.w	#-1000,d0
	bne	rep_cp_d2	
end_cp_d2:
	rts

*
* Starting points
*
points1:
	dc.w	320,200
	dc.w	320,200
	dc.w	320,200
	dc.w	320,200
	dc.w	-1000,-1000
*
* The delta values for the different points
*
delta1:
	dc.w	-4,-3
	dc.w	-8,5
	dc.w	3,6
	dc.w	7,-3
	dc.w	-1000,-1000

*
* The lines to connect the points
*
lines1:
	dc.w	0,1,1
	dc.w	1,2,1
	dc.w	2,3,1
	dc.w	3,0,1
	dc.w	-1000,-1000

; drawline routine 
; van d1,d2 naar d3,d4
; a2 wordt direkt in het mask register geschreven
; in D6 staat de gewenste kleur
;
drawline:

; bereking van het startadres van de lijn

	movem.l	d0-d7/a0-a5,-(a7)
	cmp.w	d0,d2
	bne.s	no_x_same
	cmp.w	d1,d3
	bne.s	no_x_same
	bra.L	puntje	
no_x_same:
	move.l	sb_screenmem(a3),a0
	lea.l	$dff000,a5
	move.l	sb_breedte_x(a3),a1
	move.l	a1,d4
	mulu	sb_planes(a3),d4
	mulu	d1,d4
	moveq	#-$10,d5
	and.w	d0,d5
	lsr.w	#3,d5
	add.w	d5,d4
	add.l	a0,d4
; d4 bevat nu het startadres van de lijn

; octant en delta waarden berekenen

	moveq	#0,d5

	sub.w	d1,d3
	roxl.b	#1,d5
	tst.w	d3
	bge.s	y2gy1
	neg.w	d3
y2gy1:
	sub.w	d0,d2
	roxl.b	#1,d5
	tst.w	d2
	bge.s	x2gx1
	neg.w	d2
x2gx1:
	move.w	d3,d1
	sub.w	d2,d1
	bge.s	dygdx
	exg	d2,d3
dygdx:	roxl.b	#1,d5
	move.b	octtabel(PC,d5),d5
	bra	pp
; octanttabel met LINE=1

octtabel:
	dc.b	0*4+1
	dc.b	4*4+1
	dc.b	2*4+1
	dc.b	5*4+1
	dc.b	1*4+1
	dc.b	6*4+1
	dc.b	3*4+1
	dc.b	7*4+1
	even

pp:
	add.w	d2,d2

	moveq	#0,d7
	
; test of de blitter de laatste berekening al af heeft

wblit:	btst	#14,dmaconr(a5)
	bne.s	wblit

	movem.l	d0-d6,-(a7)
	btst	d7,d6
	beq	not_this_plane

	move.w	d2,bltbmod(a5)
	sub.w	d3,d2
	bge.s	signn1
	or.b	#$40,d5
signn1:	
	ext.l	d2
	move.l	d2,bltapt(a5)
	sub.w	d3,d2
	move.w	d2,bltamod(a5)
	
; overige registers initialiseren

	move.w	#$8000,bltadat(a5)
* masker in a2
	move.w	a2,bltbdat(a5)
	move.w	#$ffff,bltafwm(a5)
	and.w	#$000f,d0
	ror.w	#4,d0
	or.w	#$0bca,d0

	move.w	d0,bltcon0(a5)
	move.w	d5,bltcon1(a5)
	move.l	d4,bltcpt(a5)
	move.l	d4,bltdpt(a5)

	move.l	sb_breedte_x(a3),d0
	mulu	sb_planes(a3),d0

	move.w	d0,bltcmod(a5)
	move.w	d0,bltdmod(a5)

; bltsize initialiseren en blitter starten

	lsl.w	#6,d3
	addq.w	#2,d3
	move.w	d3,bltsize(a5)

not_this_plane:

	movem.l	(a7)+,d0-d6
	add.l	sb_breedte_x(a3),d4
	addq.w	#1,d7
	cmp.w	sb_planes(a3),d7
	bne	wblit

puntje:	movem.l	(a7)+,d0-d7/a0-a5
	rts

check_but:
	movem.l	d0-d2/a0/a1,-(a7)

	moveq	#0,d1
	moveq	#0,d0
	move.l	sb_sig_ptoc(a3),d2
	move.l	$4,a6
	jsr	_LVOSetSignal(a6)
	move.l	d0,d1

	and.l	d2,d1
	bne	search_list

	move.l	d0,d1

	movem.l	(a7)+,d0-d2/a0/a1

	IFEQ	XAPP
	btst	#6,$bfe001
	bne	no_mouse2
	moveq	#-1,d0
	rts
no_mouse2:
	ENDC
	
	moveq	#0,d0
	rts

own_delay:
	movem.l	d0-d2/a0/a1,-(a7)

	move.l	sb_waitmask(a3),d0
	or.l	sb_sig_ptoc(a3),d0
no_sigptoc:

	move.l	$4,a6
	jsr	_LVOWait(a6)

	movem.l	d0/d1/a0/a1,-(a7)
	move.l	$4,a6
	jsr	_LVOForbid(a6)
	movem.l	(a7)+,d0/d1/a0/a1

	move.l	d0,d1
	and.l	sb_sig_ptoc(a3),d1
	beq.b	no_wacht1

; Er is een message van boven kijk wat zij doet en handel ernaar

search_list:
	bra	doterm
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
	move.l	$4,a6
	jsr	_LVOFindTask(a6)
	move.l	d0,a1
	move.l	sb_sig_ptoc(a3),d0
	jsr	_LVOSignal(a6)		; restore signal
	movem.l	(a7)+,d0-d2/a0/a1

exit_art:
	move.l	$4,a6
	jsr	_LVOPermit(a6)

	movem.l	(a7)+,d0-d2/a0/a1
	moveq	#-1,d0
	rts

no_wacht1:
	btst	#6,$bfe001
	beq	exit_art
end_of_list:
	move.l	$4,a6
	jsr	_LVOPermit(a6)
	movem.l	(a7)+,d0-d2/a0/a1
no_wacht:
	moveq	#0,d0
	rts

own_delay2:

	IF XAPP
	move.l	sb_waitmask(a3),d0
	ELSE
	move.l	sigmask(pc),d0
	ENDC
	
	move.l	$4,a6
	jsr	_LVOWait(a6)

	rts

	IF XAPP
	IF DEBUG
dbstr41:	dc.b	"Freeing the colormap %x %x",10,0
dbstr100:	dc.b	"Try to copy to mlsys",10,0
dbstr1:		dc.b	"start artit",10,0
dbstr2:		dc.b	"end artit",10,0

	even
	ENDC

	XDEF	_get_varsize
_get_varsize:
	move.l	#sbsc_SIZEOF,d0
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
	move.l	a0,sb_mlsysstruct(a3)

	move.l	ml_taglist(a0),sb_tags(a3)
	move.l	ml_monitor_ID(a0),sb_monid(a3)

	move.l	16(a5),a1
	move.l	a1,sb_msgpointer(a3)
	moveq	#0,d0
	move.b	MP_SIGBIT(a1),d0
	moveq	#1,d1
	lsl.l	d0,d1
	move.l	d1,sb_sig_ptoc(a3)

	bsr	set_fades
	
	bsr.w	openlibs

	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

	XDEF	_load_art
	
_load_art:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	12(a5),a3

	tst.b	sb_loaded(a3)		; file loaded ??
	bne.w	already_loaded

	move.l	8(a5),a0		; filename

	moveq	#0,d0
	move.l	16(a5),a1		; pointer to numargs

	move.w	4(a1),d0		; wipe nr.
	move.w	6(a1),d0
	move.l	d0,sb_speed(a3)

	move.w	8(a1),d0		; line thickness
	bpl	nomin1
	moveq	#25,d0
nomin1:
	move.w	d0,sb_num_lines(a3)
	move.w	10(a1),d0		; variation
	move.w	12(a1),d0		; x pos blitin
	move.w	14(a1),d0		; y pos blitin
	move.w	16(a1),d0		; wipe_out
	move.w	18(a1),d0		; anim speed override

	move.w	20(a1),d0		; number of rotations

	move.w	22(a1),sb_bits(a3)
	lea	colors(pc),a0
	move.l	a0,sb_colors(a3)

	bsr	get_points_mem
	tst.l	d0
	beq	already_loaded

	bsr	get_prefs

	move.b	#$ff,sb_loaded(a3)	; file loaded
already_loaded:
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0			; succes
	rts

load_err:
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#-1,d0			; fail
	rts

	XDEF	_do_art
_do_art:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)

	move.l	8(a5),a3

	tst.b	sb_loaded(a3)
	beq.w	not_loaded

	move.l	sb_mlsysstruct(a3),a0

	tst.w	sbu_Viewed(a0)
	bne.w	active_first

	move.w	#1,sbu_Viewed(a0)
	move.l	sbu_Base(a0),sb_screenmem(a3)
	lea	sbu_Display+dp_View(a0),a1
	move.l	a1,sb_wview1(a3)
	add.l	#sbu_SIZEOF,a0
	move.w	#0,sbu_Viewed(a0)
	bra	active_set

active_first:
	move.w	#0,sbu_Viewed(a0)
	add.l	#sbu_SIZEOF,a0
	move.w	#1,sbu_Viewed(a0)
	move.l	sbu_Base(a0),sb_screenmem(a3)
	lea	sbu_Display+dp_View(a0),a1
	move.l	a1,sb_wview1(a3)

active_set:

	bsr	set_size

	bsr	initialize_viewports

	bsr.w	setsam
	tst.b	d0
	bne.w	not_loaded

	bsr.w	install_50h

	lea	sigmask(pc),a0
	move.l	(a0),d0
	move.l	d0,sb_waitmask(a3)

	bsr	clear_screenmem

	move.l	sb_graphbase(a3),a6
	move.l	gb_ActiView(a6),sb_oldview(a3)

	bsr	show_it

	IF DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	dbstr1(pc),a0
	jsr	KPutStr
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	bsr	artit

	IF DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	dbstr2(pc),a0
	jsr	KPutStr
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	bsr	copy_view

	bsr.w	remove_50h
	bsr.w	freesam

	move.l	sb_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)

not_loaded:

	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts
	ENDC
*
* Set the different screen sizes
*
set_size:
	move.w	#BR,d0
	move.w	#HO,d1
	cmp.w	#BR,sb_max_x(a3)
	bge	.okex
	move.w	sb_max_x(a3),d0
.okex:
	move.w	sb_max_y(a3),d2
	cmp.w	#HO,d2
	bge	.okey
	move.w	sb_max_y(a3),d1
.okey:
	move.w	d0,sb_lenviewx(a3)
	move.w	d1,sb_lenviewy(a3)

	move.w	#DEF_MODE,sb_mode(a3)
	move.w	#DEF_PLANES,sb_planes(a3)

	move.w	sb_lenviewy(a3),sb_leny(a3)

	moveq	#0,d0
	move.w	sb_lenviewx(a3),d0
	move.l	d0,d1
	add.l	#15,d0
	lsr.l	#4,d0
	lsl.l	#4,d0
	move.w	d0,sb_lenx(a3)		; true length bitmap
	lsr.l	#3,d0
	move.l	d0,sb_breedte_x(a3)	; in bytes
	lea	teller(pc),a0
	move.l	#0,off_teller(a0)
	move.l	#0,off_tel_start(a0)
	rts

*
* Initialize two views with each a viewport
*
initialize_viewports:
	lea	sb_viewblock1(a3),a5
	bsr.w	create_viewport
	rts

free_viewports:
	rts

*
* a5 points to a vi_block with view viewport etc.
*		
create_viewport:
	move.l		sb_graphbase(a3),a6
	lea.l		vi_view(a5),a1
	jsr		_LVOInitView(a6)

	lea.l		vi_view(a5),a1
	move.w		sb_viewx(a3),v_DxOffset(a1)
	move.w		sb_viewy(a3),v_DyOffset(a1)

	lea.l		vi_view(a5),a1
	move.w		sb_mode(a3),d0
	or.w		sb_or_vmode_mask(a3),d0
	and.w		sb_and_vmode_mask(a3),d0
	move.w		d0,v_Modes(a1)
	lea.l		vi_viewport(a5),a0
	jsr		_LVOInitVPort(a6)

	lea.l		vi_view(a5),a0
	lea.l		vi_viewport(a5),a1
	move.l		a1,v_ViewPort(a0)

	lea		vi_bitmap(a5),a0
	move.w		sb_planes(a3),d0			; depth
	move.l		sb_breedte_x(a3),d1		; breedte in bytes * 8
	lsl.l		#3,d1				; width
	move.w		sb_leny(a3),d2			; height
	jsr		_LVOInitBitMap(a6)

	lea		vi_bitmap(a5),a0
	move.l		sb_screenmem(a3),d0
	move.l		d0,bm_Planes(a0)		; zet de bitplanes

	lea.l		vi_rasinfo(a5),a0		; initialiseer rasinfo
	lea.l		vi_bitmap(a5),a1
	move.l		a1,ri_BitMap(a0)
	move.w		#0,ri_RxOffset(a0)
	move.w		#0,ri_RyOffset(a0)
	move.l		#0,ri_Next(a0)

	lea		vi_viewport(a5),a0
	lea		vi_rasinfo(a5),a1
	move.l		a1,vp_RasInfo(a0)

	move.w		sb_lenx(a3),vp_DWidth(a0)
	move.w		sb_leny(a3),vp_DHeight(a0)
	move.w		sb_mode(a3),vp_Modes(a0)
	clr.l		vp_Next(a0)

	move.w	sb_columns(a3),d1	; zet de offsets berekend uit de prefs offsets

; test de offset berekening met de byte lengte

	move.l	sb_breedte_x(a3),d0
	lsl.l	#3,d0
	cmp.w	sb_max_x(a3),d0
	ble	.oke1
	move.w	sb_max_x(a3),d0
.oke1:
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
	move.l	sb_monitorspec(a3),ve_Monitor(a0)

	moveq	#VIEWPORT_EXTRA_TYPE,d0
	jsr	_LVOGfxNew(a6)
	move.l	d0,vi_vpextra(a5)
	beq	exit_maakv

	move.l	d0,a1
	lea.l	vi_viewport(a5),a0
	jsr	_LVOGfxAssociate(a6)

	move.l	sb_tags(a3),a0
	move.l	vi_vpextra(a5),12(a0)

	move.w	#0,a0
	lea	sb_dimquery(a3),a1
	moveq	#dim_SIZEOF,d0
	move.l	#DTAG_DIMS,d1
	move.l	sb_monid(a3),d2
	or.w	sb_mode(a3),d2
	or.w	sb_or_vmode_mask(a3),d2
	jsr	_LVOGetDisplayInfoData(a6)

	lea	sb_dimquery(a3),a0
	move.l	vi_vpextra(a5),a1

	move.l	dim_MaxOScan(a0),vpe_DisplayClip(a1)
	move.l	dim_MaxOScan+4(a0),vpe_DisplayClip+4(a1)

	move.l	sb_monid(a3),d0
	or.w	sb_mode(a3),d0
	jsr	_LVOFindDisplayInfo(a6)
	move.l	sb_tags(a3),a0
	move.l	d0,20(a0)		

	moveq		#2,d0
	jsr		_LVOGetColorMap(a6)

	move.l		d0,a0
	move.l		sb_tags(a3),a1
	lea.l		vi_viewport(a5),a2
	move.l		a2,4(a1)
	jsr		_LVOVideoControl(a6)

.no_ucop_mem:
	lea		vi_view(a5),a0
	lea		vi_viewport(a5),a1
	jsr		_LVOMakeVPort(a6)

	lea		vi_view(a5),a1
	jsr		_LVOMrgCop(a6)
exit_maakv:
	lea		vi_viewport(a5),a0
	move.l		sb_colors(a3),a1
	moveq		#2,d0
	move.l		sb_graphbase(a3),a6
	jsr		_LVOLoadRGB4(a6)
	rts

get_dy_offset:
	move.w	sb_rows(a3),d1
	move.w	sb_leny(a3),d0
	lsl.w	#1,d0			; set on interlace
	lsl.w	#1,d1
	lsr.w	#1,d0			; was lace so divide again
	and.w	#$fffe,d0
	sub.w	d1,d0
	neg.w	d0
	asr.w	#1,d0
	and.w	#$fffc,d0
	asr.w	#1,d0
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

testdata:	blk.b	mtr_SIZEOF,0
	
show_viewport:
;	move.l		sb_graphbase(a3),a6
;	lea		vi_view(a5),a0
;	lea		vi_viewport(a5),a1
;	jsr		_LVOMakeVPort(a6)
;	lea		vi_view(a5),a1
;	jsr		_LVOMrgCop(a6)

	lea		vi_view(a5),a1
	jsr		_LVOLoadView(a6)

	rts

;restore_oldview:

	move.l		sb_graphbase(a3),a6

cleanexit:
	tst.l		sb_oldview(a3)
	beq		no_oldview

	move.l		sb_oldview(a3),a1
	jsr		_LVOLoadView(a6)

	jsr		_LVOWaitTOF(a6)
no_oldview:
	rts

*
* Copy the view the a view that the ml system knows of
* but first release any views stored there
*
copy_view:
	move.l		sb_wview1(a3),a5
	bsr		free_view
	move.l		sb_wview1(a3),a5
	bsr		create_viewport
	move.l		sb_wview1(a3),a5
	bsr	set_color
	bsr		show_viewport
	moveq		#0,d0
	rts

free_view:
	move.l		sb_graphbase(a3),a6

	lea.l		vi_viewport(a5),a0
	jsr		_LVOGfxLookUp(a6)
	move.l		d0,vi_vpextra(a5)

	lea.l		vi_view(a5),a0
	jsr		_LVOGfxLookUp(a6)
	move.l		d0,vi_vextra(a5)

	lea		vi_viewport(a5),a0
	tst.l		vp_ColorMap(a0)
	beq.b		.no_colormem
	move.l		vp_ColorMap(a0),a0
	jsr		_LVOFreeColorMap(a6)
	lea		vi_viewport(a5),a0
	clr.l		vp_ColorMap(a0)
.no_colormem:
	lea		vi_viewport(a5),a0
	jsr		_LVOFreeVPortCopLists(a6)

	lea		vi_view(a5),a0
	tst.l		v_LOFCprList(a0)
	beq.b		.no_lofmem
	move.l		v_LOFCprList(a0),a0
	jsr		_LVOFreeCprList(a6)
	lea		vi_view(a5),a0
	clr.l		v_LOFCprList(a0)
.no_lofmem:

	lea		vi_view(a5),a0
	tst.l		v_SHFCprList(a0)
	beq.b		.no_shfmem
	move.l		v_SHFCprList(a0),a0
	jsr		_LVOFreeCprList(a6)
	lea		vi_view(a5),a0
	clr.l		v_SHFCprList(a0)
.no_shfmem:

.no_colormap:
	tst.l	vi_vpextra(a5)
	beq	.no_vp
	move.l	vi_vpextra(a5),a0

	jsr	_LVOGfxFree(a6)
	clr.l	vi_vpextra(a5)
.no_vp:
	tst.l	vi_vextra(a5)
	beq	.no_v
	move.l	vi_vextra(a5),a0

	jsr	_LVOGfxFree(a6)
	clr.l	vi_vextra(a5)
.no_v:
	rts

get_screenmem:
	move.l	sb_breedte_x(a3),d0
	mulu	sb_planes(a3),d0
	mulu	sb_leny(a3),d0
	add.l	#10000,d0		; safety for now ???
	move.l	d0,sb_memsize(a3)
	move.l	#MEMF_CHIP+MEMF_CLEAR,d1
	move.l	$4,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,sb_screenmem(a3)
	bne	screenmem_oke
	moveq	#-1,d0
	rts
screenmem_oke:
	moveq	#0,d0
	rts

clear_screenmem:
	move.l	sb_breedte_x(a3),d0
	mulu	sb_leny(a3),d0
	move.w	sb_planes(a3),d7
	moveq	#0,d1
	subq.w	#1,d7
cc:	add.l	d0,d1
	dbf	d7,cc
	lsr.l	#2,d1
	move.l	sb_screenmem(a3),a0
	moveq	#0,d0
cc2:	move.l	d0,(a0)+
	subq.l	#1,d1
	bne	cc2
	rts
	
free_screenmem:
	move.l	sb_screenmem(a3),d0
	beq	no_screenmem1
	move.l	d0,a1
	move.l	sb_memsize(a3),d0
	move.l	$4,a6
	jsr	_LVOFreeMem(a6)
	clr.l	sb_screenmem(a3)
no_screenmem1:
	rts	

*
* Retrieve the overscan settings
*
get_oscan_vals:
	move.l	sb_intbase(a3),a6
	move.l	sb_monid(a3),d0
	or.l	#HIRES_KEY,d0
	move.l	d0,a0
	move.l	#1,d0				//OSCAN_MAX
	move.l	sb_points_mem(a3),a1
	jsr	_LVOQueryOverscan(a6)
	move.l	sb_points_mem(a3),a1

	move.w	ra_MaxX(a1),d0
	move.w	ra_MaxY(a1),d1
	addq.w	#1,d0
	addq.w	#1,d1
	lsr.w	#1,d0
	lsr.w	#1,d1
	add.w	d0,d0
	add.w	d1,d1
	move.w	d0,sb_columns(a3)
	move.w	d1,sb_rows(a3)

	move.l	sb_monid(a3),d0
	or.l	#HIRES_KEY,d0
	move.l	d0,a0
	move.l	#4,d0				//OSCAN_MAX
	move.l	sb_points_mem(a3),a1
	jsr	_LVOQueryOverscan(a6)
	move.l	sb_points_mem(a3),a1

	move.w	ra_MaxY(a1),d0
	sub.w	ra_MinY(a1),d0
	add.w	d0,d0
	move.w	d0,sb_max_y(a3)

	move.w	ra_MaxX(a1),d0
	sub.w	ra_MinX(a1),d0
	addq.l	#7,d0
	lsr.l	#3,d0
	lsl.l	#3,d0
	move.w	d0,sb_max_x(a3)

	move.w	#0,a0
	move.l	sb_points_mem(a3),a1
	moveq	#mtr_SIZEOF,d0
	move.l	#DTAG_MNTR,d1
	move.l	sb_monid(a3),d2
	move.l	sb_graphbase(a3),a6
	jsr	_LVOGetDisplayInfoData(a6)
	move.l	sb_points_mem(a3),a1

	move.w	mtr_ViewPositionRange+2(a1),d0		; get minimal y pos
	move.w	mtr_ViewPosition+2(a1),d1		; get actual y pos
	move.w	d0,d2
	addq.w	#4,d0					; diff < 4 ?
	cmp.w	d0,d1
	bge	.minimum_oke
	move.l	d2,d1
	addq.w	#4,d1
.minimum_oke:
	move.w	mtr_ViewPosition(a1),sb_viewx(a3)
	move.w	d1,sb_viewy(a3)

	and.w	#$fffe,sb_viewx(a3)
	and.w	#$fffe,sb_viewy(a3)

	clr.b	sb_Xoff(a3)
	clr.b	sb_Yoff(a3)
	rts

set_lace:
	IFNE XAPP
	move.w	#0,sb_or_vmode_mask(a3)
	move.l	sb_mlsysstruct(a3),a0
	move.l	ml_miscFlags(a0),d0
	btst	#4,d0
	beq	.nolace
	ENDC
	move.w	#V_LACE,sb_or_vmode_mask(a3)	; set depending on LACE on/off
.nolace:
	rts
*
* Set depending on monitor the LACE bits
* Data should be somewhere in the mlsystem structure
*
* For test purpose also open the monitor here
*
set_and_or_vmode:
	move.l	sb_graphbase(a3),a6
	move.l	sb_monid(a3),d0
	move.w	#0,a1
	jsr	_LVOOpenMonitor(a6)
	move.l	d0,sb_monitorspec(a3)

	bsr	set_lace

	move.l	sb_monid(a3),d0
	move.w	#~0,sb_and_vmode_mask(a3)
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
	move.w	#~V_LACE,sb_and_vmode_mask(a3)
	rts

get_prefs:
	bsr	set_and_or_vmode
	bsr	get_oscan_vals

	move.l	sb_points_mem(a3),a0
	move.l	#130,d0
	lsr.l	#2,d0
.cc:	clr.l	(a0)+
	dbf	d0,.cc
	rts

openlibs:
	moveq	#0,d0
	move.l	d0,sb_graphbase(a3)
	move.l	d0,sb_intbase(a3)

	lea	graphname(pc),a1
	moveq	#0,d0
	move.l	$4,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,sb_graphbase(a3)
	beq.s	exit_openlibs
	move.l	d0,a6
	move.l	gb_ActiView(a6),sb_oldview(a3)
	move.w	LIB_VERSION(a6),d0
	move.w	d0,sb_libversion(a3)
	cmp.w	#LIBV39,d0
	blt	no_v39_1
	move.b	#$ff,sb_v39_present(a3)
no_v39_1:
	move.b	gb_ChipRevBits0(a6),d0
	and.b	#SETCHIPREV_AA,d0
	cmp.b	#SETCHIPREV_AA,d0
	bne.b	no_aa1
	move.b	#$ff,sb_aa_present(a3)
no_aa1:
	lea	intname(pc),a1
	moveq	#0,d0
	move.l	$4,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,sb_intbase(a3)
	moveq	#0,d0
	rts
	
exit_openlibs:
	moveq	#-1,d0
	rts

	IF XAPP

	XDEF	_unload_art
_unload_art:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)

	move.l	8(a5),a3

	bsr	free_points_mem
	bsr	free_viewports

	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

	XDEF _release_art
_release_art:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a3

        move.l  sb_monitorspec(a3),d0
	beq     .no_mon
	move.l  sb_graphbase(a3),a6
	move.l  d0,a0
	jsr     _LVOCloseMonitor(a6)
.no_mon

	bsr	close_libs

	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

	ENDC

close_libs:
	moveq	#0,d7

	tst.l	sb_graphbase(a3)
	beq.s	exit3

	move.l	sb_graphbase(a3),a1
	move.l	$4,a6
	jsr	_LVOCloseLibrary(a6)

	clr.l	sb_graphbase(a3)
exit3:
	tst.l	sb_intbase(a3)
	beq.s	exit4
	move.l	sb_intbase(a3),a1
	move.l	$4,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	sb_intbase(a3)
exit4:

	moveq	#0,d0
	rts

setsam:
	move.l	$4,a6
	move.l	#-1,d0
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
	move.l	$4,a6
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
	beq	nofreesam
	move.l	$4,a6
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
	move.l	$4,a6
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
	move.l	$4,a6
	jsr	_LVOAddIntServer(a6)
	lea	handle_installed(pc),a0
	move.b	#$ff,(a0)
no_install:
	rts

remove_50h:
	lea.l	intstruct50h(pc),a1
	moveq	#5,d0
	move.l	$4,a6
	jsr	_LVORemIntServer(a6)
	lea	handle_installed(pc),a0
	move.b	#$00,(a0)
	rts

proc_50h:
	move.l	a0,-(a7)
	sub.l	#1,off_teller(a1)
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
	dc.b	2,15		; type en pri
	dc.l	0,0		; pointer naar naam en data
	dc.l	0

teller:		dc.l	1
tellerstart:	dc.l	1
task:		dc.l	0
signum:		dc.l	0
sigmask:	dc.l	0
handle_installed:	dc.b	0,0

off_teller = 0
off_tel_start = 4
off_task = 8
off_signum = 12
off_sigmask = 16

graphname:	dc.b	'graphics.library',0
intname:	dc.b	'intuition.library',0
	even
colors:
	dc.w	$0000,$0000
	END
