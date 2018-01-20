*
* 7 december 1992
*
* Create a universal player wich can handle
*
* CMAP changes 
* BMHD changes ( screen format changes )
* Variant speeds
* ( Possibly other formats than the anim-5 format )
* 
*
* Try double-buffered viewports to play an anim
* 28 august 1992
*
* ANIM reader modification 1-1-1991 23-05-1991
*
* ANIM reader by Cees Lieshout 14-april-1990
* 
*
	INCDIR "oma:include/"

	INCLUDE "exec/execbase.i"
	INCLUDE "exec/exec_lib.i"
	INCLUDE "graphics/gfxbase.i"
	INCLUDE "graphics/graphics_lib.i"
	INCLUDE "graphics/view.i"
	INCLUDE "graphics/gfx.i"
	INCDIR	"apr:"
	INCLUDE	"anhd.i"

speed= 0
LOOP = 1

;exec=4
addint=-168
remint=-174
setint=-162
allocsignal = -330
findtask = -294
freesignal = -336
signal = -324
wait = -318


openlib=-408
closelib=-414
allocmem=-198
freemem=-210
exec=4

open=-30
close=-36
output=-60
seek=-66
read=-42
write=-48
mode_old=1005
mode_new=1006
key = $bfec01

l_view = 0
l_viewport = 18
l_bitmap = 58
l_rasinfo = 98		; totale size 110

MEMSIZE = 230000
	bra	start

	subq	#1,d0
	beq.W	input_error
zoeken:
	cmp.b	#$20,(a0)+
	bne.s	gevonden
	dbra	d0,zoeken
	bra.W	input_error
gevonden:
	subq.l	#1,a0
	move.l	a0,filenaam
zet:	cmp.b	#10,(a0)
	beq.s	zetnul
	cmp.b	#' ',(a0)
	beq.s	zetnul2

	addq.l	#1,a0
	bra.s	zet
zetnul2:
	move.b	#0,(a0)	; maak van de spatie na de filenaam een nul
	addq.l	#1,a0	; a0 naar volgende commando

	cmp.b	#"s",(a0)+
	bne.b	no_in
	clr.l	d0
	move.b	(a0)+,d0
	sub.l	#'0',d0
	move.l	d0,frame_speed
	move.b	(a0)+,d1
	cmp.b	#'0',d1
	blt	no_in
	cmp.b	#'9',d1
	bgt	no_in
	sub.b	#'0',d1
	mulu	#10,d0
	add.b	d1,d0
	move.l	d0,frame_speed

	bra.b	no_in
zetnul:
	move.b	#0,(a0)
	bra.s	no_in

start:	
	move.l	#filename,filenaam
	move.l	#filename2,filenaam2
	move.l	#filename_write,filenaam_write

no_in:
	move.l	a7,easy_exit

	bsr.w	openlibs

	bsr.w	read_whole_file
	bsr.w	read_whole_file2
	bsr	write_file

;	bsr.w	zoek_BMHD	

;	bsr.w	unpack_init

;	bsr.w	zoek_CMAP

	move.l	packed,a1
	cmpi.l	#"ANIM",(a1)
	bne.w	no_anim

; reserveer geheugen voor tweede scherm

	tst.l	unpackedsecond
	bne.b	already_assigned_second

	move.l	exec,a6
	move.l	#MEMSIZE,d0
	move.l	#$10002,d1
	jsr	allocmem(a6)
	move.l	#1,d7
	move.l	d0,unpackedsecond
	beq.w	exit
already_assigned_second:
	move.l	#$10002,d1
	move.l	exec,a6
	move.l	#MEMSIZE,d0
	jsr	allocmem(a6)
	move.l	d0,unpacked
	beq.w	exit

	move.l	unpacked(pc),frame_hidden
	move.l	unpackedsecond(pc),frame_show

	move.l	graphbase(pc),a6
	move.l	gb_ActiView(a6),oldview

	move.l	#view2,view_visible
	move.l	#view1,view_hidden

	tst.b	interlace
	beq	nop_interlace
	
	tst.b	hires
	beq	nop_interlace

	move.l	#view2,view_visible
	move.l	#view1,view_hidden

nop_interlace:
;	move.l	body_start(pc),a0
;	move.l	unpacked(pc),a1

;	bsr.w	unpack

	bsr	find_FORMs
;	bsr.w	find_ANHDs

	bsr.w	install_50h
	bsr	setsam
	bne	no_anim

;	bsr.w	init_anim
	
;	bsr	change_view

ww:
	bsr	create_next_frame

	btst	#6,$bfe001
	bne	ww
;	bsr.w	try_to_animate

no_anim:
	move.w	#$8020,$dff096
;	move.l	graphbase(pc),a6
;	move.l	38(a6),$dff080
;	clr.w	$dff088
	moveq	#0,d7
	bra.w	exit

input_error:
	rts

setsam:
	move.l	exec,a6
	move.l	#-1,d0
	jsr	allocsignal(a6)	

	cmp.l	#-1,d0
	beq	setsam_error
	move.l	d0,signum
	move.l	d0,d1
	moveq	#$1,d0
	lsl.l	d1,d0	
	move.l	d0,sigmask
		
	sub.l	a1,a1
	move.l	exec,a6
	jsr	findtask(a6)
	move.l	d0,task
	clr.l	d0
	rts
setsam_error:
	rts

freesam:
	move.l	signum(pc),d0
	cmp.l	#-1,d0
	beq	nofreesam
	move.l	exec,a6
	jsr	freesignal(a6)
nofreesam:
	rts

stuur_signaal:
	movem.l	d0/a1,-(a7)
	clr.l	teller
	move.l	exec,a6
	move.l	task(pc),a1
	move.l	sigmask(pc),d0
	jsr	signal(a6)
	movem.l	(a7)+,d0/a1
	rts

install_50h:
	move.l	#intstruct50h,a1
	move.l	#5,d0
	move.l	exec,a6
	jsr	addint(a6)
	move.b	#$ff,initialized
	rts

remove_50h:
	tst.b	initialized
	beq	no_rem
	move.l	#intstruct50h,a1
	move.l	#5,d0
	move.l	exec,a6
	jsr	remint(a6)
no_rem:
	rts

proc_50h:
	add.l	#1,teller
	move.l	frame_speed(pc),d0

	cmp.l	teller,d0
	bgt	no_send
	bsr	stuur_signaal
no_send:
	moveq	#0,d0
	rts

proc50hnaam:	dc.b	"50Hz interupt",0
	even
		
intstruct50h:
	dc.l	0,0
	dc.b	2,-60		; type en pri
	dc.l	proc50hnaam,0	; pointer naar naam en data
	dc.l	proc_50h
teller:	dc.l	0
task:	dc.l	0
signum:	dc.l	-1
sigmask:dc.l	0
initialized:	dc.b	0,0


change_view:

	move.l	sigmask(pc),d0		; wacht tot 50hz signal
	move.l	exec,a6
	jsr	wait(a6)

	move.l		graphbase(pc),a6
;	jsr		_LVOWaitTOF(a6)

	move.l		view_hidden(pc),a1
	jsr		_LVOLoadView(a6)

	move.l		graphbase(pc),a6
	jsr		_LVOWaitTOF(a6)

	move.l		view_visible(pc),d0
	move.l		view_hidden(pc),view_visible
	move.l		d0,view_hidden

	move.l		frame_show(pc),d0
	move.l		frame_hidden(pc),frame_show
	move.l		d0,frame_hidden

	rts

maak_viewport:

	lea		l_view(a5),a1
	jsr		_LVOInitView(a6)

	lea		l_viewport(a5),a0
	jsr		_LVOInitVPort(a6)

	lea		l_view(a5),a0
	lea		l_viewport(a5),a1
	move.l		a1,v_ViewPort(a0)
;	move.w		#100,v_DxOffset(a0)
	move.w		mode,v_Modes(a0)

	lea		l_bitmap(a5),a0
	move.w		planes,d0			; depth

	move.l		breedte_x(pc),d1		; breedte in bytes * 8
	mulu		planes(pc),d1
	lsl.l		#3,d1				; width
	move.w		leny(pc),d2			; height
	jsr		_LVOInitBitMap(a6)

	lea		l_bitmap(a5),a0

	move.l		w_bitpointer(pc),d0

	move.l		d0,bm_Planes(a0)		; zet de bitplanes

	add.l		breedte_x(pc),d0
	move.l		d0,bm_Planes+4(a0)

	add.l		breedte_x(pc),d0
	move.l		d0,bm_Planes+8(a0)

	add.l		breedte_x(pc),d0
	move.l		d0,bm_Planes+12(a0)

	add.l		breedte_x(pc),d0
	move.l		d0,bm_Planes+16(a0)

	add.l		breedte_x(pc),d0
	move.l		d0,bm_Planes+20(a0)

	lea.l		l_rasinfo(a5),a0		; initialiseer rasinfo
	lea		l_bitmap(a5),a1
	move.l		a1,ri_BitMap(a0)
	move.w		#0,ri_RxOffset(a0)
	move.w		#0,ri_RyOffset(a0)
	move.l		#0,ri_Next(a0)

	lea		l_viewport(a5),a0
	lea		l_rasinfo(a5),a1
	move.l		a1,vp_RasInfo(a0)
	move.w		lenx(pc),vp_DWidth(a0)
	move.w		leny(pc),vp_DHeight(a0)
	move.w		mode(pc),vp_Modes(a0)
	move.w		#0,vp_DxOffset(a0)

	move.l		#32,d0
	jsr		_LVOGetColorMap(a6)
	lea		l_viewport(a5),a0
	move.l		d0,vp_ColorMap(a0)
	beq		no_colormap

	lea		l_viewport(a5),a0
	lea		colors(pc),a1
	move.l		#32,d0
	jsr		_LVOLoadRGB4(a6)

	lea		l_view(a5),a0
	lea		l_viewport(a5),a1
	jsr		_LVOMakeVPort(a6)

	lea		l_view(a5),a1
	jsr		_LVOMrgCop(a6)

;	lea		l_view(a5),a1
;	jsr		_LVOLoadView(a6)

	rts
	
reset_oldview:
	tst.l		oldview
	beq		no_oldview
	move.l		graphbase(pc),a6
	move.l		oldview(pc),a1
	jsr		_LVOLoadView(a6)
	jsr		_LVOWaitTOF(a6)
no_oldview:
	rts

freememory:
	move.l		graphbase(pc),a6
	lea		l_viewport(a5),a0
	tst.l		vp_ColorMap(a0)
	beq		no_colormem
	move.l		vp_ColorMap(a0),a0
	jsr		_LVOFreeColorMap(a6)
no_colormem:
	lea		l_viewport(a5),a0
	jsr		_LVOFreeVPortCopLists(a6)

	lea		l_view(a5),a0
	tst.l		v_LOFCprList(a0)
	beq		no_lofmem
	move.l		v_LOFCprList(a0),a0
	jsr		_LVOFreeCprList(a6)
no_lofmem:

	lea		l_view(a5),a0
	tst.l		v_SHFCprList(a0)
	beq		no_shfmem
	move.l		v_SHFCprList(a0),a0
	jsr		_LVOFreeCprList(a6)
no_shfmem:

no_colormap:
	moveq		#0,d0
	rts

init_anim:
	move.w	planes(pc),d7
	move.l	breedte_x(pc),d0
	mulu	d7,d0
	move.l	d0,rowmulti

* create a mul-table for the anim play

	move.l	d0,d1
	lea	multabel(pc),a0
	move.w	leny(pc),d7
	move.l	#0,(a0)+
	subq.w	#1,d7
rep_make_mult:
	move.l	d0,(a0)+
	add.l	d1,d0
	dbf	d7,rep_make_mult
	lea	waar_frames(pc),a0
	move.l	a0,frame_pointer
	rts

*
* Search for all the FORMs in the file
*
find_FORMs:
	move.l	#"FORM",d2	

	lea.l	waar_frames(pc),a0
	move.l	packed(pc),a1
	move.l	a1,a2
	lea.l	4(a1),a1		; skip the ILBM sign
	add.l	packed_size(pc),a2	; end data
fza1:
	cmp.l	a2,a1
	bge.b	exitfind_f

	move.l	(a1),d3
	cmp.l	d2,d3
	bne.s	fza2

	add.l	#1,FORM_counter
	move.l	a1,(a0)+	; store the pointer

fza2:
	move.l	4(a1),d0
	btst	#0,d0
	beq.b	fno_add		; check for uneven chunksize
	addq.l	#1,d0
	
fno_add:
				; next chunk
	add.l	d0,a1
	lea.l	8(a1),a1
	bra.s	fza1
exitfind_f:
	move.l	#0,(a0)+
	lea	waar_frames(pc),a0
	move.l	a0,frame_pointer
	rts
*
* Get next FORM pointer from the internal array
* returns the pointer in a1
*
get_next_FORM:
	move.l	frame_pointer(pc),a0
	move.l	(a0)+,a1
	cmp.l	#0,a1
	bne	ok_getFORM
	lea	waar_frames(pc),a0	; depends on cycle on or off
	tst.b	looping
	bne	ok_nonlooping
	lea	8(a0),a0
ok_nonlooping:
	move.l	(a0)+,a1
ok_getFORM:
	move.l	a0,frame_pointer
	rts

create_next_frame:
tt:
	move.b	#$0,bmhd_found
	move.b	#0,cmap_found
	bsr	get_next_FORM		; retrieve FORM pointer	
	move.l	4(a1),temp_formsize	; store size
	lea	8(a1),a1
	move.l	a1,a2
	add.l	temp_formsize(pc),a2	; end FORM pointer
	cmp.l	#'ILBM',(a1)+
	bne	no_ilbm1

next_chunk_FORM1:
	move.l	(a1),d0			; first chunk ID

	cmp.l	#'BMHD',d0
	bne	no_bmhd_anim_found
	bsr	bmhd_anim_found
	bra	continue_check_FORM
no_bmhd_anim_found:

	cmp.l	#'ANHD',d0
	bne	no_anhd_anim_found
	bsr	anhd_anim_found
	bra	continue_check_FORM
no_anhd_anim_found:

	cmp.l	#'CMAP',d0
	bne	no_cmap_anim_found
	bsr	cmap_anim_found
	bra	continue_check_FORM
no_cmap_anim_found

	cmp.l	#'CAMG',d0
	bne	no_camg_anim_found
	bsr	camg_anim_found
	bra	continue_check_FORM
no_camg_anim_found:
	cmp.l	#'BODY',d0
	bne	no_body_anim_found
	bsr	body_anim_found
	bra	continue_check_FORM
no_body_anim_found:
	cmp.l	#'DLTA',d0
	bne	no_dlta_anim_found
	bsr	dlta_anim_found
	bra	continue_check_FORM

no_dlta_anim_found:

continue_check_FORM:
	add.l	4(a1),a1
	move.l	a1,d0
	btst	#0,d0
	beq.b	no_oneven2
	addq.l	#1,a1
no_oneven2:
	addq.l	#8,a1
	cmp.l	a2,a1
	blt.w	next_chunk_FORM1

; All chunks are handled now check the actions taken
; to see if any changes should be copied to the other view

; but first show the newly made view

	tst.b	bmhd_found
	beq	no_anim_bmhd_found

	move.l	view_hidden(pc),a5		; change hidden

	bsr	freememory
	move.l	frame_hidden(pc),w_bitpointer
	bsr	maak_viewport			; if excist the new colors
	bra	exit_nextframe1			; are loaded at the same time

no_anim_bmhd_found:

	tst.b	cmap_found
	bne	cmap_ch1

	tst.b	cmap_changed
	beq	exit_nextframe1

	move.l	view_hidden(pc),a5		; copy last found cmap
	move.l	frame_hidden(pc),w_bitpointer
	lea	l_viewport(a5),a0
	lea	colors(pc),a1
	moveq	#32,d0
	move.l	graphbase(pc),a6
	jsr	_LVOLoadRGB4(a6)

	bsr	change_view			; show hidden
	move.b	#0,cmap_changed
	bra	exit_nextframe2

cmap_ch1:
	move.l	view_hidden(pc),a5		; copy cmap
	lea	l_viewport(a5),a0
	lea	colors(pc),a1
	moveq	#32,d0
	move.l	graphbase(pc),a6
	jsr	_LVOLoadRGB4(a6)

exit_nextframe1:

	bsr	change_view	

exit_nextframe2:

	rts

no_ilbm1:				; what to do now ??????

	rts

dlta_anim_found:
	movem.l	d0-d7/a0-a6,-(a7)
	tst.b	bmhd_changed	; in this case the data from the active must
				; be copied to the hidden ( the first 2 frames)
	beq	no_bmhd_ch2			
	bsr	copy_active_hidden
	move.b	#0,bmhd_changed
no_bmhd_ch2:

	cmp.b	#5,acompression
	bne	no_anim5
	lea	8(a1),a1
	bsr	de_animate
no_anim5:
	movem.l	(a7)+,d0-d7/a0-a6
	rts

body_anim_found:
	movem.l	d0/a1/a2,-(a7)

	cmp.b	#1,compression
	beq.s	anim_un1

; compression 0 or unknown do a simple copy

	move.l	frame_hidden(pc),a2
	move.l	4(a1),d0
	lsl.l	#2,d0
	sub.l	#1,d0
	lea	8(a1),a1
anim_no_co:
	move.l	(a1)+,(a2)+
	dbf	d0,anim_no_co
	bra.s	anim_un_end

anim_un1:

	move.l	frame_hidden(pc),a2
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
	bra.w	anim_un_again

anim_un_minus:
	neg.b	d5
	move.b	(a0)+,d0

anim_un_rm:
	move.b	d0,(a2)+
	dbf	d5,anim_un_rm
	bra.w	anim_un_again
anim_un_end:	
	movem.l	(a7)+,d0/a1/a2
	rts

camg_anim_found:
	move.l	d0,-(a7)
	move.b	#$ff,bmhd_changed	; change the viewports accordingly
	move.b	#$ff,bmhd_found
	moveq	#0,d2
	move.w	10(a1),d0
	or.w	d0,d2			; straight in the mode-id
	and.w	#$8efd,d2		; oldstyle mask
	move.w	d0,d1
	and.w	#$8000,d0
	beq.b	cno_hires
	move.b	#$ff,hires
	or.w	#$8000,d2
cno_hires:
	move.w	d1,d0
	and.w	#$4,d0
	beq.b	cno_lace
	move.b	#$ff,interlace
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
	move.w	d2,mode

	move.l	(a7)+,d0
	rts
	
cmap_anim_found:
	movem.l	d0/a1/a2,-(a7)
	move.b	#$ff,cmap_found
	lea	4(a1),a1
	move.l	(a1)+,cmap_size
	move.l	cmap_size(pc),d1
	divu	#3,d1
	move.l	d1,color_count	
* in d1 het aantal kleuren
	lea.l	colors(pc),a2
	sub.l	#1,d1
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
	move.b	#$ff,cmap_changed

	movem.l	(a7)+,d0/a1/a2
	rts

anhd_anim_found:
	movem.l	d0/a1,-(a7)
	lea	8(a1),a1
	move.b	anhd_operation(a1),acompression
	move.l	anhd_reltime(a1),d0
	tst.l	overallspeed
	bne	no_reltime
	move.l	d0,frame_speed
no_reltime:	
	move.b	anhd_interleave(a1),ainterleaved
	movem.l	(a7)+,d0/a1
	rts
*
* In the anim is a BMHD chunk
* initialize the variables with this new info
*
bmhd_anim_found:
	movem.l	d0/a1,-(a7)
	move.b	#$ff,bmhd_found
	clr.b	hires
	clr.b	interlace
	lea	8(a1),a1
	move.w	(a1),lenx

	moveq	#0,d0
	move.w	lenx(pc),d0
	lsr.l	#4,d0
	add.l	d0,d0
	move.l	d0,breedte_x

	moveq	#0,d1
	move.b	8(a1),d1
	move.w	d1,planes
	move.b	10(a1),compression

	move.w	planes(pc),d7
	move.l	breedte_x(pc),d0
	mulu	d7,d0

	cmp.l	rowmulti(pc),d0
	bne	bmhd_multable
	move.w	2(a1),d0
	cmp.w	leny(pc),d0
	beq	bmhd_no_multable

bmhd_multable:
	move.w	2(a1),leny
	moveq	#0,d1
	move.w	leny(pc),d1
	move.l	d1,hoogte_y

	move.l	d0,rowmulti
* create a mul-table for the anim play
	move.l	d0,d1

	lea	multabel(pc),a0
	move.w	leny(pc),d7
	move.l	#0,(a0)+
	subq.w	#1,d7
bmhd_rep_make_mult:
	move.l	d0,(a0)+
	add.l	d1,d0
	dbf	d7,bmhd_rep_make_mult
bmhd_no_multable:
	move.b	#$ff,bmhd_changed		; copy to other view later on

	move.l	breedte_x(pc),d0
	move.w	leny(pc),d1
	mulu	d1,d0
	moveq	#0,d1	
	move.w	planes(pc),d1
	mulu	d1,d0
	move.l	d0,unpacked_size

	movem.l	(a7)+,a1/d0
	rts
*
* The size ie something else has changed so alter the
* hidden accordingly
*
copy_active_hidden:
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	view_hidden(pc),a5
	bsr	freememory
	move.l	frame_hidden(pc),w_bitpointer
	bsr	maak_viewport
	moveq	#0,d0
	move.w	leny(pc),d0
	mulu	planes(pc),d0
	move.l	breedte_x(pc),d1
	mulu	d1,d0
	lsr.l	#1,d0
	subq.l	#1,d0
	move.l	frame_hidden(pc),a0
	move.l	frame_show(pc),a1
rep_cop1:
	move.w	(a1)+,(a0)+
	subq.l	#1,d0
	bne	rep_cop1
	movem.l	(a7)+,d0-d7/a0-a6
	rts
	

*
* de_animate needs a pointer de de DLTA data ( DLTA pointer + 8 )
* it also uses a mul-table 
de_animate:
	move.l	breedte_x(pc),d2
	move.l	a1,a5			; pointer naar DLTA data 

	move.l	rowmulti(pc),d0
	
	move.l	frame_hidden(pc),a6

	move.w	planes(pc),d7
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

	lea	multabel(pc),a4
	add.w	d4,d4
	add.w	d4,d4
	add.l	0(a4,d4.w),a6
	dbf	d5,rep_opcounts
	bra.b	cont_more_opcounts


read_whole_file:
	move.l	#3,d7
	move.l	filenaam(pc),d1
	move.l	#mode_old,d2
	move.l	dosbase(pc),a6
	jsr	open(a6)
	move.l	d0,filehandle
	beq.w	exit
	move.l	d0,d1
	move.l	#buffer,d2
	move.l	#8,d3
	move.l	dosbase,a6
	jsr	read(a6)
	lea.l	buffer(pc),a1
	move.l	#1,d7
	cmp.l	#'FORM',(a1)+
	bne.w	exit
	move.l	(a1),packed_size
	move.l	(a1),d0
	move.l	#2,d7
	move.l	#$10000,d1
	move.l	exec,a6
	jsr	allocmem(a6)
	move.l	d0,packed
	beq.w	exit
	move.l	packed_size(pc),d3
	move.l	d0,d2
	move.l	filehandle(pc),d1
	move.l	dosbase(pc),a6
	jsr	read(a6)		

* hele file nu in het geheugen 

	bsr.w	close_file
	moveq	#0,d0
	rts

read_whole_file2:
	move.l	#3,d7
	move.l	filenaam2(pc),d1
	move.l	#mode_old,d2
	move.l	dosbase(pc),a6
	jsr	open(a6)
	move.l	d0,filehandle
	beq.w	exit
	move.l	d0,d1
	move.l	#buffer,d2
	move.l	#8,d3
	move.l	dosbase,a6
	jsr	read(a6)
	lea.l	buffer(pc),a1
	move.l	#1,d7
	cmp.l	#'FORM',(a1)+
	bne.w	exit
	move.l	(a1),packed_size2
	move.l	(a1),d0
	move.l	#2,d7
	move.l	#$10000,d1
	move.l	exec,a6
	jsr	allocmem(a6)
	move.l	d0,packed2
	beq.w	exit
	move.l	packed_size2(pc),d3
	move.l	d0,d2
	move.l	filehandle(pc),d1
	move.l	dosbase(pc),a6
	jsr	read(a6)		

* hele file nu in het geheugen 

	bsr.b	close_file
	moveq	#0,d0
	rts

close_file:
	move.l	dosbase(pc),a6
	move.l	filehandle(pc),d1
	jsr	close(a6)
	clr.l	filehandle
	rts

write_file:
	move.l	filenaam_write(pc),d1
	move.l	#mode_new,d2
	move.l	dosbase(pc),a6
	jsr	open(a6)
	move.l	d0,filehandle
	beq.w	exit

	lea.l	buffer(pc),a1
	move.l	#'FORM',(a1)+
	move.l	packed_size(pc),d3
	add.l	packed_size2(pc),d3
	subq.l	#4,d3				; one ANIM less
	move.l	d3,(a1)+
	moveq	#8,d3
	move.l	#buffer,d2
	move.l	filehandle(pc),d1
	move.l	dosbase(pc),a6
	jsr	write(a6)		

	move.l	packed_size(pc),d3
	move.l	packed(pc),d2
	move.l	filehandle(pc),d1
	move.l	dosbase(pc),a6
	jsr	write(a6)		


	move.l	packed_size2(pc),d3
	subq.l	#4,d3
	move.l	packed2(pc),d2
	addq.l	#4,d2
	move.l	filehandle(pc),d1
	move.l	dosbase(pc),a6
	jsr	write(a6)		

	bsr	close_file	
	rts

openlibs:
	clr.l	packed
	clr.l	unpacked
	clr.l	filehandle
	clr.l	graphbase
	
	clr.l	dosbase
	move.l	exec,a6
	lea	dosname(pc),a1
	moveq	#0,d0
	jsr	openlib(a6)
	move.l	d0,dosbase
	beq.s	exit

	lea	graphname(pc),a1
	moveq	#0,d0
	move.l	exec,a6
	jsr	openlib(a6)
	move.l	d0,graphbase
	beq.s	exit

	move.l	dosbase(pc),a6
	jsr	output(a6)
	move.l	d0,conhandle
*	beq.s	exit
	rts

exit:	


	move.w	#$c000,$dff096
	move.w	#$8020,$dff096
;	move.l	graphbase(pc),a6
;	move.l	38(a6),$dff080
;	clr.w	$dff088

	bsr.w	remove_50h
	bsr	freesam

	bsr	reset_oldview
	lea	view1(pc),a5
	bsr	freememory
	lea	view2(pc),a5
	bsr	freememory


	move.l	easy_exit(pc),a7

exit11:	
	tst.l	unpackedsecond
	beq.s	exit12
	move.l	unpackedsecond(pc),a1
	move.l	unpacked_size(pc),d0
	move.l	#MEMSIZE,d0
	move.l	exec,a6
	jsr	freemem(a6)
	clr.l	unpackedsecond
exit12:
	tst.l	unpacked
	beq.s	exit1
	move.l	unpacked(pc),a1
	move.l	unpacked_size(pc),d0
	move.l	#MEMSIZE,d0
	move.l	exec,a6
	jsr	freemem(a6)
	clr.l	unpacked

exit1:
	tst.l	packed
	beq.s	exit21
	move.l	packed(pc),a1
	move.l	packed_size(pc),d0
	move.l	exec,a6
	jsr	freemem(a6)
	clr.l	packed

exit21:
	tst.l	packed2
	beq.s	exit22
	move.l	packed2(pc),a1
	move.l	packed_size2(pc),d0
	move.l	exec,a6
	jsr	freemem(a6)
	clr.l	packed2

exit22:	tst.l	filehandle
	beq.s	exit2
	bsr.w	close_file
	
exit2:	tst.l	graphbase
	beq.s	exit3
	move.l	graphbase(pc),a1
	move.l	exec,a6
	jsr	closelib(a6)
	clr.l	graphbase

exit3:
exit4:	tst.l	dosbase
	beq.s	exit5
	tst.l	d7
	beq.s	exit51
	sub.l	#1,d7
	move.l	dosbase(pc),a6
	lea.l	foutmelding(pc),a1
	lsl.l	#2,d7
	add.l	d7,a1
	move.l	(a1),d2
	move.l	conhandle(pc),d1

	move.l	conhandle(pc),d1
	beq.b	no_con_open

	move.l	#FOUT_LENGTE,d3
	jsr	write(a6)

exit51:	move.l	dosbase(pc),a1
	move.l	exec,a6
	jsr	closelib(a6)
	clr.l	dosbase
exit5:
	moveq	#0,d0
	rts

no_con_open:
	move.l	d2,a0
	move.l	#FOUT_LENGTE-3,d3

	lea.l	imess+3(pc),a1
c_fout:	move.b	(a0)+,(a1)+
	dbf	d3,c_fout

	move.l	exec,a6
	lea.l	intui(pc),a1
	jsr	-408(a6)
	tst.l	d0
	beq.b	err_intui
	move.l	d0,a6
	clr.l	d0
	lea.l	imess(pc),a0
	move.l	#40,d1
	jsr	-90(a6)

	move.l	a6,a1
	move.l	4,a6
	jsr	-414(a6)
err_intui
	bra.b	exit51

imess:	dc.b	0,20,20
	blk.b	100,0

	even

FOUT_LENGTE=21

foutmelding:	dc.l	fout0,fout1,fout2,fout3,fout4,fout5
fout0:		dc.b	"Geen IFF file       ",10,0
fout1:		dc.b	"Geen genoeg geheugen",10,0
fout2:		dc.b	"File niet gevonden  ",10,0
fout3:		dc.b	"Onbekende anim mode ",0
fout4:		dc.b	" ",0
fout5:		dc.b	" ",0

	even
easy_exit:	dc.l	0
dosbase:	dc.l	0
graphbase:	dc.l	0
conhandle:	dc.l	0
filehandle:	dc.l	0
filenaam:	dc.l	0
filenaam2:	dc.l	0
filenaam_write:	dc.l	0
;mem_cop:	dc.l	0
packed:		dc.l	0
packed_size:	dc.l	0
packed2:	dc.l	0
packed_size2:	dc.l	0

unpackedfirst:	dc.l	0
unpackedsecond:	dc.l	0

unpacked:	dc.l	0
unpacked_size:	dc.l	0
body_start:	dc.l	0
cmap_size:	dc.l	0
color_count:	dc.l	0
waar_cols_incop:dc.l	0

buffer:		blk.b	8,0
dosname:	dc.b	'dos.library',0
graphname:	dc.b	'graphics.library',0
intui:		dc.b	'intuition.library',0
;filename:	dc.b	'anim:castle.anim',0
;filename:	dc.b	'anim:technosure2.anim',0
;filename:	dc.b	'anim:Temit.anim',0
;filename:	dc.b	'dd:lady.comp',0
filename:	dc.b	'anims:lady',0
filename2:	dc.b	'anims:flagfull',0
filename_write:	dc.b	"ram:test.anim",0
;filename:	dc.b	"ram:test.anim",0
	even
lenx:		dc.w	320
leny:		dc.w	200
breedte_x:	dc.l	0
hoogte_y:	dc.l	0
dlta_size:	dc.l	0
temp_kol_adres:	dc.l	0
planes:		dc.w	0
hires:		dc.b	0
interlace:	dc.b	0
mode:		dc.w	0
compression:	dc.b	0
acompression:	dc.b	0
ainterleaved:	dc.b	0
cmap_changed:	dc.b	0
bmhd_changed:	dc.b	0
cmap_found:	dc.b	0
bmhd_found:	dc.b	0
looping:	dc.b	LOOP
	even
overallspeed:	dc.l	speed
oldview:	dc.l	0
w_cop:		dc.l	0
w_cop_hidden:	dc.l	0
w_bitpointer:	dc.l	0
frame_show:	dc.l	0
frame_hidden:	dc.l	0
last_clock:	dc.l	0

rowmulti:	dc.l	0
frame_pointer:	dc.l	0

view_visible:	dc.l	0
view_hidden:	dc.l	0
view1:		blk.l	40,0
view2:		blk.l	40,0

reltime:	dc.l	0
temp_formsize:	dc.l	0
FORM_counter:	dc.l	0
frame_teller:	dc.l	0
frame_speed:	dc.l	speed
waar_scherm:	dc.l	0
	even
colors:		blk.w	64,0
waar_frames:	blk.l	500,0	; maximaal 200 frames
multabel:	blk.l	600,0

