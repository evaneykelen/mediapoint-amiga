*
* Create a dummy call routine to init and display an anim in 
* a bitmap, the bitmap for the anim is the anim size 18-03-1994
*
* Convert for anim in document play 16-11-93
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
	INCDIR	"wp:asm/"
	INCLUDE	"anim/aw_data.i"
	INCLUDE	"errors.i"
	INCLUDE	"proces.i"
	INCLUDE	"system.i"

DEBUG = 0
XAPP = 1
MLMMU = 1
ANIM32 = 1

speed= 50
LOOP = 0
DISKANIM = 1
PICTURE = 0
CONTINUE = 0

ROTS = 1

MEMF_STAY = $8000
MTF_INIT = 1
MTF_SETCLR = $80000000

LIBV39 = 39
LIBV36 = 36

exec=4

mode_old=1005
key = $bfec01

MEMSIZE = 230000
MAX_MULTABEL = 580

;	bra	start

	IFNE	DEBUG
	XREF	KPutFmt
	XREF	KPutStr
	PRINTT	"De debug info staat aan dus je moet nog met debug.lib linken"
	ENDC
	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	22(a1),a1
;	lea	dbstr212(pc),a0
	jsr	KPutFmt
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

*
* Init the libraries and init the memory and the dst bitmap
*
start:
	bsr	get_libraries
	bsr	get_memory
	bsr	init_views
	bsr	get_anim
	bsr	get_frame
	

	bsr	free_views
	bsr	free_memory
	bsr	free_libraries
	rts

ACUR = 8
*
* Calculate the speed so that speeds of non jiffy size are possible
*
calc_speed:
	movem.l	a0/d0/d1/d2,-(a7)
	moveq	#50,d0				; depends on pal or ntsc

	move.l	aw_mlsysstruct(a3),a0
	move.l	ml_miscFlags(a0),d2
	btst	#0,d2
	bne	.pal
	moveq	#60,d0
.pal:
	tst.l	d1
	bne.b	oke_non_zero
	moveq	#25,d1
oke_non_zero:
	lsl.l	#ACUR,d0
	divu	d1,d0
	and.l	#$ffff,d0
set_wait:
;	lea	teller(pc),a0
;	move.l	d0,(a0)
;	lea	tellerstart(pc),a0
;	move.l	d0,(a0)
	movem.l	(a7)+,d0/d1/a0/d2
	rts
*
* The anhd gives a number of jiffies to wait
*
set_jiffies:
	movem.l	a0/d0/d1,-(a7)
	tst.l	d0
	bne.b	oke_jif
	moveq	#1,d0
oke_jif:
	lsl.l	#ACUR,d0
	bra.b	set_wait	

init_anim:
	move.w	aw_planes(a3),d7
	move.l	aw_breedte_x(a3),d0
	mulu	d7,d0
	move.l	d0,aw_rowmulti(a3)

* create a mul-table for the anim play

	move.l	d0,d1
	lea	aw_multabel(a3),a0
	move.w	aw_leny(a3),d7
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
	tst.b	aw_looping(a3)
	bne.b	no_loop4
	subq.l	#2,aw_dpan_count(a3)		; the anim is looping so sub 2
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
	dbf	d7,rr_skip
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
	bra.w	exit
*
* Search for all the FORMs in the file which is on disk
*
find_disk_FORMs:

	tst.l	aw_temp_frame_list(a3)
	beq.b	no_head_list1

	move.b	#1,aw_show(a3)

	tst.b	aw_continue(a3)
	bne.b	no_continue6

	move.l	aw_temp_frame_list(a3),a0
	move.l	a0,aw_head_frame_list(a3)		; set temp list, first
	clr.l	aw_temp_frame_list(a3)

	move.l	aw_head_frame_list(a3),d0
	move.l	d0,a0
	move.l	d0,a4

	move.l	aw_packed(a3),a1
	lea.l	12(a1),a1
	bra.b	from_c1

	bra.b	dza1
	
no_continue6:
	bra.b	no_continue5

no_head_list1:
	move.l	#12,aw_global_diskoffset(a3)
	move.l	#0,aw_FORM_counter(a3)

	tst.b	aw_continue(a3)
	beq.b	no_continue5

	bsr.w	skip_two_forms

	move.b	#1,aw_show(a3)
	addq.l	#2,aw_frame_counter(a3)
	addq.l	#2,aw_FORM_counter(a3)

no_continue5:

* Allocate head pointer
*
	bsr.w	alloc_frame
	move.l	d0,aw_head_frame_list(a3)
	beq.w	frame_alloc_fail

	move.l	d0,a0
	move.l	d0,a4

dza1:
	move.l	a0,-(a7)
	bsr.w	load_next_form
	move.l	(a7)+,a0
	addq.l	#1,aw_frame_counter(a3)
	addq.l	#1,aw_FORM_counter(a3)

	tst.l	d0
	beq.w	exitfind_d
	move.l	d0,a1

from_c1:
	move.l	4(a1),d7
	movem.l	a0/d7,-(a7)
	move.l	a0,a4			; remember previous pointer
	bsr.w	create_next_frame1

	tst.b	aw_show(a3)
	bne.b	oke_show5
	moveq	#1,d7
	tst.b	aw_picture(a3)
	bne.b	oke_pic1
	moveq	#2,d7
oke_pic1:
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
	move.l	d0,a0

	tst.b	aw_quit(a3)
	bne.b	exitfind_d

	cmp.l	#1,aw_num_rot(a3)
	bne.b	no_pro4
	tst.l	aw_dpan_count(a3)
	beq.b	no_pro4
	move.l	aw_frame_counter(a3),d7

	move.b	#1,aw_quit(a3)
	cmp.l	aw_dpan_count(a3),d7
	beq.b	exitfind_d
	move.b	#0,aw_quit(a3)
no_pro4:
;	btst	#6,$bfe001
;	beq.b	exitfind_d
	bra.w	dza1

exitfind_d:
	subq.l	#1,aw_num_rot(a3)
	move.l	#0,ani_next(a4)	; clear last frame
	move.l	a0,a1
	bsr.w	free_frame
	move.l	aw_head_frame_list(a3),a0
	move.l	a0,aw_frame_pointer(a3)
	subq.l	#1,aw_FORM_counter(a3)

	tst.l	aw_dpan_count(a3)
	bne.b	no_loop5
	
	move.l	aw_FORM_counter(a3),aw_dpan_count(a3)
	tst.b	aw_looping(a3)
	bne.b	no_loop5
	subq.l	#2,aw_dpan_count(a3)		; the anim is looping so sub 2
no_loop5:
	rts

*
* Search for all the FORMs in the file which is on disk
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

	move.l	d0,a1
	move.l	4(a1),d7

	tst.b	aw_continue(a3)
	beq.b	no_cont10
	addq.l	#8,d7
	add.l	d7,aw_global_diskoffset(a3)

no_cont10:
	move.l	#1,aw_frame_counter(a3)
	move.l	#1,aw_FORM_counter(a3)

	tst.b	aw_continue(a3)
	beq.b	exitfind_d2

	bsr.w	skip_one_form

	move.l	#2,aw_frame_counter(a3)
	move.l	#2,aw_FORM_counter(a3)

exitfind_d2:
	move.b	#$0,aw_diskloaded(a3)
	rts

frame_alloc_fail:
	moveq	#1,d7
	bra.w	exit

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

	tst.b	aw_looping(a3)
	bne.b	ok_nonlooping
	move.l	ani_next(a0),a0
	move.l	ani_next(a0),a0		; skip first two frames ??
	move.l	#2,aw_frame_counter(a3)
ok_nonlooping:

ok_getFORM:
	move.l	a0,aw_frame_pointer(a3)
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

	tst.b	aw_diskloaded(a3)
	bne.w	exit_nextframe2

; All chunks are handled now check the actions taken
; to see if any changes should be copied to the other view

; but first show the newly made view

show_new_frame:

	tst.b	aw_bmhd_found(a3)
	beq.b	no_anim_bmhd_found

	move.l	aw_view_hidden(a3),a5		; change hidden
;	bsr.w	freememory
	move.l	aw_frame_hidden(a3),aw_w_bitpointer(a3)
;	bsr.w	maak_viewport			; if excist the new colors
	bra.w	exit_nextframe1			; are loaded at the same time

no_anim_bmhd_found:

	tst.b	aw_show(a3)
	bne.b	show5
	tst.b	aw_continue(a3)
	beq.b	show5
	move.b	#$0,aw_bmhd_found(a3)
	move.b	#$0,aw_bmhd_changed(a3)
	bra.w	no_show
show5:

exit_nextframe1:
	tst.b	aw_show(a3)
	beq.b	no_show

	tst.b	aw_continue(a3)
	beq.b	normal_show

	cmp.l	#2,aw_frame_counter(a3)
	bne.b	normal_show

;	bsr.w	change_view2

	bra.w	exit_nextframe2

normal_show:
;	bsr.w	change_view	

exit_nextframe2:
	rts

no_ilbm1:				; what to do now ??????

	rts

no_show:
	tst.b		aw_continue(a3)
	bne.w		no_cont5
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

	move.l	aw_breedte_x(a3),d0
	move.w	aw_leny(a3),d1
	mulu	d1,d0
	moveq	#0,d1	
	move.w	aw_planes(a3),d1
	mulu	d1,d0
	move.l	d0,aw_unpacked_size(a3)

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

	tst.b	aw_bmhd_changed(a3)	; in this case the data from the active must
				; be copied to the hidden ( the first 2 frames)
	beq.b	no_bmhd_ch2			
	tst.b	aw_show(a3)
	bne.b	do_showd1
	tst.b	aw_picture(a3)
	bne.b	do_showd1
	bra.b	bmhd_ch3
do_showd1:
	IFNE ANIM32
	cmp.b	#$64,aw_acompression(a3)
	beq.b	anim322
	cmp.b	#$65,aw_acompression(a3)
	bne.w	no_anim322
anim322:
;	bsr.w	convert_int_non
no_anim322:
	ENDC
	
;	bsr.w	copy_active_hidden

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
	
	move.l	aw_frame_hidden(a3),a6
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
	
	move.l	aw_frame_hidden(a3),a6
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
	move.l	aw_frame_hidden(a3),a6		; plane pointer
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
	move.l	aw_frame_hidden(a3),a6		; plane pointer
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
	bne.b	do_show2

	tst.b	aw_picture(a3)
	beq.b	no_show2

;	bsr.w	copy_active_hidden

	bra.b	no_show2

do_show2:
	cmp.b	#1,aw_compression(a3)
	beq.s	anim_un1

; compression 0 or unknown do a simple copy

	move.l	aw_frame_hidden(a3),a2
	move.l	4(a1),d0
	lsl.l	#2,d0
	subq.l	#1,d0
	addq.l	#8,a1
anim_no_co:
	move.l	(a1)+,(a2)+
	dbf	d0,anim_no_co
	bra.s	anim_un_end

anim_un1:

	move.l	aw_frame_hidden(a3),a2
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
	beq.b	cno_hires
	move.b	#$ff,aw_hires(a3)
	or.w	#$8000,d2
cno_hires:
	move.w	d1,d0
	and.w	#$4,d0
	beq.b	cno_lace
	move.b	#$ff,aw_interlace(a3)
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

	move.l	aw_breedte_x(a3),d0
	move.w	aw_leny(a3),d1
	mulu	d1,d0
	moveq	#0,d1	
	move.w	aw_planes(a3),d1
	mulu	d1,d0
	move.l	d0,aw_unpacked_size(a3)

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
	
	move.l	aw_frame_hidden(a3),a6
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
	
	move.l	aw_frame_hidden(a3),a6
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
	
	move.l	aw_frame_hidden(a3),a6
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
	
	move.l	aw_frame_hidden(a3),a6

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

	IFNE MLMMU
read_whole_file:
	move.l	aw_mlmmubase(a3),a6
	move.l	aw_filenaam(a3),a1		; name of the file mnmmulib
	jsr	_LVOMLMMU_FindMemBlk(a6)
	move.l	d0,aw_packed(a3)
	bne.w	file_loaded2

	moveq	#ERR_FILE_NOT_FOUND,d7
	move.l	aw_filenaam(a3),d1
	move.l	#mode_old,d2
	move.l	aw_dosbase(a3),a6
	jsr	_LVOOpen(a6)
	move.l	d0,aw_filehandle(a3)
	beq.w	exit
	move.l	d0,d1
	lea.l	aw_buffer(a3),a2
	move.l	a2,d2
	moveq	#12,d3
	move.l	aw_dosbase(a3),a6
	jsr	_LVORead(a6)
	lea.l	aw_buffer(a3),a1
	moveq	#ERR_UNKNOWN_FILE_TYPE,d7
	cmp.l	#'FORM',(a1)+
	bne.w	exit
	tst.b	aw_diskanim(a3)
	beq.b	no_diskanim1
	rts

no_diskanim1:
	move.l	(a1),d0
	addq.l	#8,d0
	move.l	d0,aw_packed_size(a3)

	moveq	#ERR_NO_FMEM,d7
	move.l	#$10000,d1
	or.l	#MEMF_STAY,d1
	move.l	aw_mlmmubase(a3),a6
	move.l	aw_filenaam(a3),a1		; naam van de file mlmmulib
	jsr	_LVOMLMMU_AllocMem(a6)

	move.l	d0,aw_packed(a3)
	beq.w	exit_read_whole

	move.l	d0,a0
check_load:

* test of de file al in het geheugen zit

	move.l	aw_packed(a3),a1
	jsr	_LVOMLMMU_GetMemStat(a6)	; haal status van memblock op

	and.l	#MTF_INIT,d0
	bne.b	file_loaded

	move.l	aw_packed(a3),a1
	jsr	_LVOMLMMU_OwnMemBlk(a6)
	tst.l	d0
	beq.b	check_load

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
	bne.b	file_cropped
	move.l	aw_packed(a3),a0
	lea	aw_buffer(a3),a1
	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+

	move.l	aw_mlmmubase(a3),a6
	move.l	aw_packed(a3),a1
	moveq	#MTF_INIT,d0
	or.l	#MTF_SETCLR,d0
	jsr	_LVOMLMMU_SetMemStat(a6)	; file nu wel geladen

	move.l	aw_packed(a3),a1
	jsr	_LVOMLMMU_DisOwnMemBlk(a6)

* hele file nu in het geheugen 

file_loaded:
	bsr.b	close_file

file_loaded2:
	move.l	aw_packed(a3),a0
	move.l	4(a0),d0
	addq.l	#8,d0
	move.l	d0,aw_packed_size(a3)
	moveq	#0,d0
	rts

file_cropped:
	bsr.b	close_file
	moveq	#ERR_UNKNOWN_FILE_TYPE,d7
	bra.w	exit

	ELSE

read_whole_file:
	move.l	#3,d7
	move.l	aw_filenaam(a3),d1
	move.l	#mode_old,d2
	move.l	aw_dosbase(a3),a6
	jsr	_LVOOpen(a6)
	move.l	d0,aw_filehandle(a3)
	beq.w	exit
	move.l	d0,d1
	lea	aw_buffer(a3),a0
	move.l	a0,d2
	move.l	#12,d3
	move.l	aw_dosbase(a3),a6
	jsr	_LVORead(a6)
	lea.l	aw_buffer(a3),a1
	move.l	#1,d7
	cmp.l	#'FORM',(a1)+
	bne.w	exit

	tst.b	aw_diskanim(a3)
	beq.b	no_diskanim1
	rts

no_diskanim1:
	move.l	(a1),d0
	add.l	#8,d0
	move.l	d0,aw_packed_size(a3)
	move.l	#2,d7
	move.l	#$10000,d1
	move.l	exec,a6
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

	bsr.b	close_file
	moveq	#0,d0
	rts

	ENDC				; MLMMU

*
* Can't read the whole file in at once or I don't want to
* Use diskanim instead
*
exit_read_whole:
	move.b	#1,aw_diskanim(a3)
	rts

close_file:
	move.l	aw_dosbase(a3),a6
	move.l	aw_filehandle(a3),d1
	beq.b	no_closefile
	jsr	_LVOClose(a6)
	clr.l	aw_filehandle(a3)
no_closefile:
	rts

exit:	
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
	move.l	aw_mlmmubase(a3),a6
	lea.l	aw_multabel(a3),a0
	moveq	#0,d0
	jsr	_LVOMLMMU_AddMsgToQueue(a6)
zero_addr1:

no_err1:
	move.l	aw_easy_exit(a3),a7
	lea	error(pc),a0
	move.l	d7,(a0)
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	move.l	error(pc),d0
	rts
error:	dc.l	0
unk:	dc.b	"unknown",0
	even

foutmelding:
	dc.l	fout1-foutmelding,fout2-foutmelding,fout3-foutmelding
	dc.l	fout4-foutmelding,fout5-foutmelding,fout6-foutmelding
	dc.l	fout7-foutmelding

fout1:	dc.b	"File not found : ",0
fout2:	dc.b	"Not enough chip memory : ",0
fout3:	dc.b	"Not enough fast memory : ",0
fout4:	dc.b	"Resolution not supported : ",0
fout5:	dc.b	"General error : ",10,0
fout6:	dc.b	"Unknown file type : ",10,0
fout7:	dc.b	"Mangled IFF file : ",10,0	

	IFEQ XAPP
datablock:
	blk.b	aw_SIZEOF,0
	ENDC

