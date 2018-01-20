*
* File	: viewiff.s
* Uses	: viewiff.i viewbloknew.i
* Date	: July 1993
* Author: ing. C. Lieshout
* Desc	: Show an IFF picture ( easy converted IFF's with more than 8 planes )
*	: Based on the
*	: IFF reader by Cees Lieshout 14-april-1990
*	: MP transitions XaPP
*

LIBV39 = 39
LIBV36 = 36

	INCDIR "include:"
	INCLUDE "exec/execbase.i"
	INCLUDE "exec/funcdef.i"
	INCLUDE "exec/exec_lib.i"
	INCLUDE	"exec/memory.i"
	INCLUDE "graphics/gfxbase.i"
	INCLUDE	"dos/dos.i"
	INCLUDE "graphics/view.i"
	INCLUDE "graphics/rastport.i"
	INCLUDE "graphics/gfx.i"
	INCLUDE	"intuition/intuitionbase.i"
	INCLUDE "intuition/preferences.i"

	INCDIR	"include:"
	INCLUDE	"egs/lvo/lvo_egs.i"
	INCLUDE	"egs/lvo/lvo_egsblit.i"
	INCLUDE	"egs/egs.i"
	INCLUDE	"egs/egsblit.i"

	INCDIR	"wp:asm/"
	INCLUDE "viewbloknew.i"
	INCDIR	"wp:asm/bits24/"
	INCLUDE	"viewiff24.i"
	INCDIR	"wp:inclibs/"
	INCLUDE "dos_lib.i"
	INCLUDE "graphics_libv39.i"
	INCLUDE "intuition_lib.i"
	INCLUDE	"mpmmu_lib.i"

	RSRESET
im_block:	rs.w	0
im_width:	rs.l	1
im_height:	rs.l	1
im_depth:	rs.w	1
im_width_bytes:	rs.l	1
im_total_width:	rs.l	1
im_planar:	rs.l	1
im_planar_size:	rs.l	1
im_rgb:		rs.l	1
im_rgb_size:	rs.l	1
im_modulo:	rs.l	1
im_rgbmodulo:	rs.l	1
im_planes:	rs.l	24
im_SIZEOF:	rs.w	0

COLSIZE = 4000

mode_old = 1005

	link	a5,#-db_SIZEOF
	
	movem.l	d0-d7/a0-a6,-(a7)
	sub.l	#db_SIZEOF,a5

	move.l	a5,a3

	movem.l	d0/a0,-(a7)
	move.l	a5,a0
	move.l	#db_SIZEOF-1,d0
rep_cl:
	move.b	#0,(a0)+
	dbf	d0,rep_cl
	movem.l	(a7)+,d0/a0

	subq	#1,d0
	beq.W	input_error
zoeken:
	cmp.b	#$20,(a0)+
	bne.s	gevonden
	dbra	d0,zoeken
	bra.W	input_error
gevonden:
	cmp.b	#'"',-1(a0)		; skip the '"' sign
	beq.b	found_cc
gev2:
	subq.l	#1,a0
	move.l	a0,db_filename_pointer(a3)
zet:	cmp.b	#10,(a0)
	beq.s	zetnul
	cmp.b	#' ',(a0)
	beq.s	zetnul2
	cmp.b	#'"',(a0)
	beq.b	zetnul2
	addq.l	#1,a0
	bra.s	zet

zetnul2:
	move.b	#0,(a0)	; maak van de spatie na de filenaam een nul
	addq.l	#1,a0

rep_comm:
	tst.b	(a0)
	beq.b	no_in
	cmp.b	#10,(a0)
	beq.b	no_in
	cmp.b	#'e',(a0)
	bne.b	no_loop
	move.b	#1,db_nostop(a3)
	bsr.w	get_number
	move.w	d0,db_waittofs(a3)
no_loop:
	addq.l	#1,a0
	bra.b	rep_comm
	
zetnul:
	move.b	#0,(a0)	; maak van de spatie na de filenaam een nul
	bra.s	no_in
*
* There is a '"' in the command line read filename until closing '"'
*
found_cc:
	move.l	a0,db_filename_pointer(a3)

cczet:	cmp.b	#10,(a0)
	beq.s	zetnul
	cmp.b	#'"',(a0)
	beq.b	zetnul2
	addq.l	#1,a0
	bra.s	cczet

	bra.b	no_in
	
start:
	link	a5,#-db_SIZEOF
	movem.l	d0-d7/a0-a6,-(a7)
	sub.l	#db_SIZEOF,a5
	move.l	a5,a3
	move.l	a5,a0
	move.l	#db_SIZEOF-1,d0
rep_cl2:
	move.b	#0,(a0)+
	dbf	d0,rep_cl2
	move.l	#filename,db_filename_pointer(a3)
	move.b	#0,db_nostop(a3)
	move.w	#5,db_waittofs(a3)
no_in:
	move.l	a7,db_easy_exit(a3)

	bsr.w	openlibs

	lea	db_fileblok1(a3),a0
	move.l	a0,db_inactive_fileblok(a3)
	move.l	#COLSIZE,d0
	move.l	$4.w,a6
	moveq	#MEMF_PUBLIC,d1
	jsr	_LVOAllocMem(a6)
	move.l	d0,db_waarcolors1(a3)
	beq	exit
	cmp.w	#1,db_waittofs(a3)
	bne	.oke
	bsr	test_egs
	bra	.ex
.oke:
	move.l	db_filename_pointer(a3),a0
	bsr.w	laadfile
	bsr	init_im_block
;	bsr.w	showpicture		; inactive -> active
	bsr	test_egs
	bsr	clear_im_block

	bsr.w	wacht_change

.ex:
	moveq	#0,d7
	bra.w	exit

	rts

input_error:
	moveq	#1,d7
	bra.w	exit5
	
;filename:	dc.b	"pics:esther0",0
;filename:	dc.b	"pics:grab/petra1.512x580",0
;filename:	dc.b	"work:data/marlboro.24",0
filename:	dc.b	"a:24bit/billboard.24b",0
;filename:	dc.b	"a:24bit/cees&joost.24b",0
;filename:	dc.b	"work:tets001",0
	even
egs_mode:	dc.b	"PICOa:SVGA 800x600",0
egs_dataname:	dc.b	"EGS_store",0
	even
egs_newscreen:	dc.l	egs_mode
		dc.w	24,0
		dc.l	0,0,0,0,0,0

test_egs:
	bsr	open_egs_screen
	beq	.err_egs1
	bsr	get_egs_size

	IF 0
	moveq	#100,d0
	moveq	#100,d1
	moveq	#24,d2
	move.l	#E_PIXELMAP,d3
	move.l	#E_EB_BLITABLE,d4
	move.l	#0,d4
	move.l	db_egsbase(a3),a6
	jsr	_LVOE_AllocBitMap(a6)
	move.l	d0,a0
	beq	.err1
	move.b	ebm_Type(a0),d0
	jsr	_LVOE_DisposeBitMap(a6)
.err1:
	ENDC

;	bsr	clear_egs
	bsr	calc_offset
	cmp.w	#1,db_waittofs(a3)
	bne	.no1
	bsr	place_picture1
	bra	.end
.no1:
	cmp.w	#2,db_waittofs(a3)
	bne	.no2
	bsr	place_picture2
	bra	.end
.no2:
	cmp.w	#3,db_waittofs(a3)
	bne	.no3
	bsr	place_picture3
	bra	.end
.no3:
	cmp.w	#4,db_waittofs(a3)
	bne	.no4
	bsr	place_picture4
	bra	.end
.no4:
	cmp.w	#5,db_waittofs(a3)
	bne	.no5

	bsr	close_egs_main
.no5:

.end:
;	bsr	close_egs_screen
.err_egs1:
	rts

close_egs_main:
	bsr	open_egs_screen
	bsr	close_egs_screen
	rts
*
* Try to open an EGS screen
*
* You could also check if there is a MPMMU entry with a predefined name
* and get the screen pointer from there
*
open_egs_screen:
	clr.l	db_egsscreen_ptr(a3)
	move.l	db_mpmmubase(a3),a6
	lea	egs_dataname(pc),a1
	moveq	#20,d0
	move.l	#MEMF_FAST,d1
	jsr	_LVOMLMMU_FindMemBlk(a6)
	move.l	d0,db_mpmmu_pointer(a3)
	beq	.no_found

; the memory is found get the screen pointer

	move.l	d0,a0
	tst.l	(a0)
	beq	.no_found
	move.l	(a0),db_egsscreen_ptr(a3)
	bra	.no_mem	

.no_found:
	move.l	db_egsbase(a3),a6
	lea	egs_newscreen(pc),a0
	jsr	_LVOE_OpenScreen(a6)
	move.l	d0,db_egsscreen_ptr(a3)
	beq	.no_mem
	move.l	db_mpmmubase(a3),a6
	lea	egs_dataname(pc),a1
	moveq	#20,d0
	move.l	#MEMF_FAST,d1
	or.l	#$8000,d1			; MEMF_STAY
	jsr	_LVOMLMMU_AllocMem(a6)
	move.l	d0,db_mpmmu_pointer(a3)
	beq	.no_mem
	move.l	d0,a0
	move.l	db_egsscreen_ptr(a3),(a0)	; store the screen pointer
.no_mem:
	rts

close_egs_screen:
	move.l	db_egsbase(a3),a6
	tst.l	db_egsscreen_ptr(a3)
	beq	.no_scr
	move.l	db_egsscreen_ptr(a3),a0
	jsr	_LVOE_CloseScreen(a6)
	clr.l	db_egsscreen_ptr(a3)
	move.l	db_mpmmu_pointer(a3),d0
	beq	.no_scr
	move.l	d0,a0
	clr.l	(a0)
	move.l	a0,a1
	moveq	#20,d0
	move.l	db_mpmmubase(a3),a6
	jsr	_LVOMLMMU_FreeMem(a6)
	clr.l	db_mpmmu_pointer(a3)
.no_scr:
	rts

*
* Given the screen pointer, get the x,y size
*
get_egs_size:
	lea	imb(pc),a5
	move.l	im_width(a5),d0
	add.l	d0,d0
	add.l	im_width(a5),d0
	subq.l	#1,d0
	move.l	d0,im_rgbmodulo(a5)
	move.l	db_egsscreen_ptr(a3),d0
	beq	.no_screen
	move.l	d0,a0
	move.l	esc_Map(a0),a0
	moveq	#0,d0
	move.w	ebm_Width(a0),d0
	move.l	d0,db_egs_width(a3)
	move.w	ebm_Height(a0),d0
	move.l	d0,db_egs_height(a3)
.no_screen:
	rts

clear_egs:
	move.l	db_egsbase(a3),a6
	move.l	db_egsscreen_ptr(a3),a0
	move.l	esc_Map(a0),a0
	jsr	_LVOE_ClearBitMap(a6)
	rts
*
* Calculate x and y offset for this picture
*
calc_offset:
	lea	imb(pc),a4
	lea	db_fileblok1(a3),a5
	moveq	#0,d0
	moveq	#0,d1
	move.w	vb_lenx(a5),d0
	move.w	vb_leny(a5),d1
	move.l	db_egs_width(a3),d2
	move.l	db_egs_height(a3),d3

; also check clipping here

	sub.l	d0,d2
	lsr.l	#1,d2
	move.l	d2,db_egs_offsetx(a3)
	sub.l	d1,d3
	lsr.l	#1,d3
	move.l	d3,db_egs_offsety(a3)
	rts
*
* Top down
*
place_picture1:
	move.l	db_egsblitbase(a3),a6
	move.l	db_egsscreen_ptr(a3),a0
	move.l	esc_Map(a0),a2

	lea	imb(pc),a5
	move.l	im_rgb(a5),a4		; source rgb values
	move.l	im_width(a5),d7
	add.l	d7,d7
	move.l	d7,d4			; modulo
	move.l	im_height(a5),d5
	move.l	db_egs_offsetx(a3),d1
	move.l	db_egs_offsety(a3),d2
 	subq.l	#1,d5
	moveq	#0,d3
.rep_rgb2:
	bsr	draw_egs_line
	add.l	d4,a4			; source modulo
	move.l	db_egs_offsetx(a3),d1
	addq.l	#1,d2
	dbf	d5,.rep_rgb2
	rts

draw_egs_line:
	move.l	im_width(a5),d6
	subq.l	#1,d6			; number off pixels
.rep_rgb:
	move.l	im_width(a5),d7
	moveq	#0,d0
	move.b	0(a4,d7),d0		; green
	lsl.w	#8,d0
	add.l	d7,d7
	move.b	0(a4,d7),d0		; blue
	swap	d0
	move.b	(a4)+,d0
	swap	d0
	lsl.l	#8,d0
	move.l	a2,a0
	jsr	_LVOEB_WritePixel(a6)		; no register trashing ?
	addq.l	#1,d1
	dbf	d6,.rep_rgb
	rts
*
* Bottom up
*
place_picture2:
	move.l	db_egsblitbase(a3),a6
	move.l	db_egsscreen_ptr(a3),a0
	move.l	esc_Map(a0),a2
	lea	imb(pc),a5
	move.l	im_rgb(a5),a4		; source rgb values
	move.l	im_width(a5),d4
	add.l	d4,d4
	add.l	d4,d4
	move.l	im_width(a5),d7		; modulo
	add.l	d7,d7
	add.l	im_width(a5),d7		; * 3 for width one line
	move.l	im_height(a5),d6
	subq.l	#1,d6
	mulu	d6,d7
	add.l	d7,a4			; set to last line
	move.l	im_height(a5),d5
	move.l	db_egs_offsetx(a3),d1
	move.l	db_egs_offsety(a3),d2
	add.l	d5,d2			; start at end of picture

 	subq.l	#1,d5
	moveq	#0,d3
.rep_rgb2:
	bsr	draw_egs_line
	sub.l	d4,a4			; source modulo
	move.l	db_egs_offsetx(a3),d1
	subq.l	#1,d2
	dbf	d5,.rep_rgb2
	rts

draw_egs_vline:
	move.l	im_height(a5),d6
	subq.l	#1,d6			; number off pixels
.rep_rgb:
	move.l	im_width(a5),d7
	moveq	#0,d0
	move.b	0(a4,d7),d0		; green
	lsl.w	#8,d0
	add.l	d7,d7
	move.b	0(a4,d7),d0		; blue
	swap	d0
	move.b	(a4)+,d0
	swap	d0
	lsl.l	#8,d0
	move.l	a2,a0
	jsr	_LVOEB_WritePixel(a6)	; no register trashing ?
	addq.l	#1,d2
	add.l	im_rgbmodulo(a5),a4
	dbf	d6,.rep_rgb
	rts
*
* Left right
*
place_picture3:
	move.l	db_egsblitbase(a3),a6
	move.l	db_egsscreen_ptr(a3),a0
	move.l	esc_Map(a0),a2

	lea	imb(pc),a5
	move.l	im_rgb(a5),a4		; source rgb values
	move.l	im_width(a5),d4
	add.l	d4,d4
	add.l	d4,d4
	move.l	im_width(a5),d7		; modulo
	add.l	d7,d7
	add.l	im_width(a5),d7		; * 3 for width one line

	move.l	im_width(a5),d5
	move.l	db_egs_offsetx(a3),d1
	move.l	db_egs_offsety(a3),d2
 	subq.l	#1,d5
	moveq	#0,d3
.rep_rgb2:
	move.l	a4,-(a7)
	bsr	draw_egs_vline
	move.l	(a7)+,a4		; goto next line
	add.l	#1,a4			; source modulo
	move.l	db_egs_offsety(a3),d2
	addq.l	#1,d1
	dbf	d5,.rep_rgb2
	rts
*
* Right left
*
place_picture4:
	move.l	db_egsblitbase(a3),a6
	move.l	db_egsscreen_ptr(a3),a0
	move.l	esc_Map(a0),a2

	lea	imb(pc),a5
	move.l	im_rgb(a5),a4		; source rgb values
	move.l	im_width(a5),d4
	add.l	d4,a4
	subq.l	#1,a4
	add.l	d4,d4
	add.l	d4,d4
	move.l	im_width(a5),d7		; modulo
	add.l	d7,d7
	add.l	im_width(a5),d7		; * 3 for width one line

	move.l	im_width(a5),d5
	move.l	db_egs_offsetx(a3),d1
	add.l	d5,d1
	move.l	db_egs_offsety(a3),d2
 	subq.l	#1,d5
	moveq	#0,d3
.rep_rgb2:
	move.l	a4,-(a7)
	bsr	draw_egs_vline
	move.l	(a7)+,a4		; goto next line
	subq.l	#1,a4			; source modulo
	move.l	db_egs_offsety(a3),d2
	subq.l	#1,d1
	dbf	d5,.rep_rgb2
	rts

get_number:
	moveq	#1,d2
	moveq	#0,d0
rep_number:
	addq.l	#1,a0
	cmp.b	#'-',(a0)
	bne.b	no_minus
	neg.l	d2
	bra.b	rep_number
no_minus:
	moveq	#0,d1
	move.b	(a0),d1
	cmp.b	#'0',d1
	blt.b	exit_gn
	cmp.b	#'9',d1
	bgt.b	exit_gn
	sub.l	#'0',d1
	mulu	#10,d0
	add.l	d1,d0
	bra.b	rep_number
exit_gn:
	rts

clear_im_block
	lea	imb(pc),a5
	tst.l	im_planar(a5)
	beq	.no_plan
	move.l	$4.w,a6
	move.l	im_planar(a5),a1
	move.l	im_planar_size(a5),d0
	jsr	_LVOFreeMem(a6)
.no_plan:
	tst.l	im_rgb(a5)
	beq	.no_rgb
	move.l	$4.w,a6
	move.l	im_rgb(a5),a1
	move.l	im_rgb_size(a5),d0
	jsr	_LVOFreeMem(a6)
.no_rgb:
	rts

*
* Fill in the image block and allocate memory
*
init_im_block:
	lea	db_fileblok1(a3),a4
	lea	imb(pc),a5
	moveq	#0,d0

	move.l	d0,im_rgb(a5)
	move.l	d0,im_planar(a5)
	
	move.w	vb_lenx(a4),d0
	move.l	d0,im_width(a5)
	move.w	vb_leny(a4),d0
	move.l	d0,im_height(a5)
	moveq	#0,d0
	move.w	vb_planes(a4),d0
	move.w	d0,im_depth(a5)

; get memory for the planar data uncrunched 
; and for the rgb data

	move.l	im_width(a5),d0
	add.l	#15,d0
	lsr.l	#4,d0
	add.l	d0,d0			; width in bytes
	move.l	d0,im_width_bytes(a5)
	move.l	d0,d1
	move.w	vb_planes(a4),d2
	mulu	d2,d1
	move.l	d1,im_total_width(a5)
	
	move.l	im_height(a5),d1
	mulu	d1,d0
	move.w	im_depth(a5),d1
	mulu	d1,d0	
	move.l	d0,im_planar_size(a5)
	move.l	#MEMF_PUBLIC,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,im_planar(a5)	
	beq	init_error

	move.l	im_width(a5),d0
	move.l	im_height(a5),d1
	mulu	d1,d0
	move.l	d0,d1
	add.l	d0,d0
	add.l	d1,d0
	move.l	d0,im_rgb_size(a5)
	move.l	#MEMF_PUBLIC,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,im_rgb(a5)
	beq	init_error

	lea	db_fileblok1(a3),a4
	move.l	vb_body_start(a4),a0
	move.l	im_planar(a5),a1

	cmp.b	#1,vb_compression(a4)
	beq.w	.un1

* compression geen 1 dus of 0 of een onbekende 
* copieer van body_start naar unpacked

	move.l	vb_unpacked_size(a4),d0

	lsr.l	#1,d0
	subq.l	#1,d0
	move.l	vb_body_start(a4),a0

.no_co:	move.w	(a0)+,(a1)+
	subq.l	#1,d0
	bpl.b	.no_co
	moveq	#0,d0
	rts

.un1:
	move.l	im_planar(a5),a6
	add.l	im_planar_size(a5),a6
.un_again:
	cmp.l	a6,a1
	bge.s	.un_end

	moveq	#0,d5
	move.b	(a0)+,d5
	bmi.b	.un_minus

.un_plu:
	move.b	(a0)+,(a1)+
	dbf	d5,.un_plu
	bra.s	.un_again

.un_minus:
	neg.b	d5
	move.b	(a0)+,d0

.un_rm:	move.b	d0,(a1)+
	dbf	d5,.un_rm
	bra.s	.un_again
.un_end:	

; the data is unpacked
; convert now to rgb values

	lea	im_planes(a5),a0
	move.l	im_planar(a5),d0
	move.l	im_width_bytes(a5),d1
	moveq	#23,d7
.rep_p:
	move.l	d0,(a0)+
	add.l	d1,d0
	dbf	d7,.rep_p

	move.l	im_width_bytes(a5),d0
	move.w	im_depth(a5),d1
	subq.w	#1,d1
	mulu	d1,d0
	move.l	d0,im_modulo(a5)

	move.l	im_rgb(a5),a6		; point to rgb data to be filled

	move.l	im_height(a5),d6
	subq.l	#1,d6
	clr.l	offset
	clr.l	toff
.rep_rgb1:
	move.l	toff(pc),offset
	lea	im_planes(a5),a0
	bsr	get_8planes		; get R data

	move.l	toff(pc),offset
	lea	im_planes+32(a5),a0
	bsr	get_8planes		; get G data

	move.l	toff(pc),offset
	lea	im_planes+64(a5),a0
	bsr	get_8planes		; get B data
	move.l	toff(pc),d0
	add.l	im_total_width(a5),d0
	move.l	d0,toff
	dbf	d6,.rep_rgb1

	moveq	#0,d0
	rts

offset:	dc.l	0
toff:	dc.l	0

get_planes_8:
	move.l	offset(pc),d2
	move.l	12(a0),a2
	move.b	(a2,d2.l),d1	; plane 3	0003
	swap	d1		;		0300	
	move.l	28(a0),a2
	move.b	(a2,d2.l),d1	; plane 7	0307
	rol.l	#8,d1		;		3070
	move.l	20(a0),a2
	move.b	(a2,d2.l),d1	; plane 5	3075
	swap	d1		;		7530
	move.l	4(a0),a2
	move.b	(a2,d2.l),d1	; plane 1	7531
	move.l	8(a0),a2
	move.b	(a2,d2.l),d0	; plane 2	0002
	swap	d0		; 		0200
	move.l	24(a0),a2
	move.b	(a2,d2.l),d0	; plane 6	0206
	rol.l	#8,d0		; 		2060
	move.l	16(a0),a2
	move.b	(a2,d2.l),d0	; plane 4	2064
	swap	d0		; 		6420
	move.l	(a0),a2
	move.b	(a2,d2.l),d0	; plane 0 	6420
	addq.l	#1,offset
	rts
*
* in a0 the pointer to the planes
* in a6 the pointer to the chunk data
*
get_8planes:
	movem.l	a5,-(a7)
	move.l	im_width(a5),d7
	move.l	im_modulo(a5),a4
.rep_y:
	move.l	#0,a5
.rep_x:
	bsr	get_planes_8
	move.l	d1,d2
	and.l	#$aaaaaaaa,d2
	and.l	#$55555555,d1
	move.l	d0,d3

	and.l	#$aaaaaaaa,d0
	and.l	#$55555555,d3
	lsr.l	#1,d0
	or.l	d2,d0
	add.l	d1,d1
	or.l	d3,d1
	move.l	d1,d2
	and.l	#$00ff00ff,d2
	and.l	#$ff00ff00,d1
	move.l	d0,d3
	and.l	#$00ff00ff,d0
	and.l	#$ff00ff00,d3
	lsr.l	#8,d3
	lsl.l	#8,d2
	or.l	d2,d0
	or.l	d3,d1
	move.l	d1,d2
	and.l	#$33333333,d1
	and.l	#$cccccccc,d2
	move.l	d0,d3
	and.l	#$cccccccc,d0
	and.l	#$33333333,d3
	lsl.l	#2,d1
	or.l	d3,d1
	lsr.l	#2,d0
	or.l	d2,d0
	swap	d0
	move.w	d0,d2
	move.w	d1,d0
	move.w	d2,d1
	swap	d0
	move.l	d0,d3
	and.l	#$0f0f0f0f,d3
	and.l	#$f0f0f0f0,d0
	move.l	d1,d2
	and.l	#$0f0f0f0f,d2
	and.l	#$f0f0f0f0,d1
	lsr.l	#4,d0
	or.l	d1,d0
	lsl.l	#4,d2
	or.l	d2,d3
	ror.w	#8,d0
	swap	d0
	ror.w	#8,d0
	ror.w	#8,d3
	swap	d3
	ror.w	#8,d3
	move.l	d0,(a6)+
	move.l	d3,(a6)+
	addq.w	#8,a5	
	cmp.w	d7,a5
	blt	.rep_x
	move.l	a4,d0
	add.l	d0,offset
	movem.l	(a7)+,a5
	rts

init_error:
	moveq	#-1,d0
	rts
	
imb:	blk.b	im_SIZEOF,0

openlibs:
	moveq	#0,d0
	move.l	d0,db_graphbase(a3)
	move.l	d0,db_dosbase(a3)

	move.l	$4,a6
	lea	dosnaam(pc),a1
	moveq	#0,d0
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,db_dosbase(a3)
	beq.w	exit

	lea	graphname(pc),a1
	moveq	#0,d0
	move.l	$4,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,db_graphbase(a3)
	beq.w	exit

	lea	egsname(pc),a1
	moveq	#0,d0
	move.l	$4,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,db_egsbase(a3)
	beq.w	exit

	lea	egsblitname(pc),a1
	moveq	#0,d0
	move.l	$4,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,db_egsblitbase(a3)
	beq.w	exit

	lea	mpmmuname(pc),a1
	moveq	#0,d0
	move.l	$4,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,db_mpmmubase(a3)
	beq.w	exit

	
	move.l	db_graphbase(a3),a6
	move.l	gb_ActiView(a6),db_oldview(a3)
	rts

dosnaam:	dc.b	'dos.library',0
graphname:	dc.b	'graphics.library',0
intui:		dc.b	'intuition.library',0
egsname:	dc.b	'egs.library',0
egsblitname:	dc.b	'egsblit.library',0
mpmmuname:	dc.b	'nb:system/mpmmu.library',0
	even

*
* laadfile leest de file in 
* de data pointer is a5 welke naar de filedata wijst
* a0 wijst naar de in de laden file
*
laadfile:
	move.l	a0,-(a7)

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
	bne.w	exit
	rts
*
* Wait until the view changed or a mous button
*
wacht_change:
	btst	#6,$bfe001
	beq.b	cleanexit
	bra.b	wacht_change
cleanexit:
	rts

unpack:	
	move.l	vb_body_start(a5),a0
	move.l	vb_tempbuffer(a5),a1

	cmp.b	#1,vb_compression(a5)
	beq.s	un1

* compression geen 1 dus of 0 of een onbekende 
* copieer van body_start naar unpacked

	move.l	vb_unpacked_size(a5),d0

	lsr.l	#1,d0
	subq.l	#1,d0
	move.l	vb_body_start(a5),a0

no_co:	move.w	(a0)+,(a1)+
	subq.l	#1,d0
	bpl.b	no_co
	moveq	#0,d0
	rts

un1:
	move.l	vb_tempbuffer(a5),a6
	add.l	vb_unpacked_size(a5),a6
un_again:	
	cmp.l	a6,a1
	bge.s	un_end

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
	moveq	#-1,d0
	rts
*
*
*
check_chunks:
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	vb_packed(a5),a1
	move.l	4(a1),d0		; file size
	lea	8(a1),a1
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
	btst	#0,d1
	beq.b	no_funny_add
	addq.w	#1,d1
no_funny_add:
	move.w	d1,vb_lenx(a5)
	move.w	2(a6),vb_leny(a5)
	moveq	#0,d1
	move.w	vb_leny(a5),d1
	moveq	#0,d1
	move.b	8(a6),d1
	move.w	d1,vb_planes(a5)
	move.b	10(a6),vb_compression(a5)
	move.b	9(a6),vb_masking(a5)
	moveq	#0,d1
	moveq	#0,d2
	move.w	vb_lenx(a5),d1

	move.w	#0,vb_mode(a5)

	move.w	vb_lenx(a5),d1
	cmp.w	#370,d1
	ble.s	geen_hires
	move.b	#$FF,vb_hires(a5)
	move.w	#$8000,d2
geen_hires:
	move.w	vb_leny(a5),d1
	cmp.w	#390,d1
	ble.s	geen_lace
	move.b	#$ff,vb_interlace(a5)
	or.w	#$4,d2
geen_lace:
	move.w	d2,vb_mode(a5)
	
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
	beq.b	cno_shires
	move.b	#$ff,vb_shires(a5)
	and.w	#V_SUPERHIRES,d0
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
	bra.b	continue_chunks

no_cmap_proc:

	cmp.l	#"BODY",(a1)
	bne.b	no_body_proc
	move.b	#$ff,vb_body_found(a5)
	lea	8(a1),a6
	move.l	a6,vb_body_start(a5)
	bra.w	continue_chunks

no_body_proc:
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
	move.l	#-1,a1
cchunks2:
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
	lsl.w	#3,d1

	move.w	vb_leny(a5),d1
	mulu	d1,d0

	move.l	d0,d2
	moveq	#0,d0
	moveq	#0,d1	
	move.w	vb_planes(a5),d1
	cmp.w	#8,d1
	ble.b	no_24b
	move.w	#4,d1
	tst.b	db_aa_present(a3)
	beq.b	no_24b
	move.w	#6,d1
no_24b:
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
	move.l	d0,db_unpacked_size(a3)

	move.l	4,a6
	moveq	#2,d1
	jsr	_LVOAllocMem(a6)
	move.l	d0,db_unpacked(a3)
	beq.w	exit

	move.l	db_unpacked(a3),vb_tempbuffer(a5)
	rts

read_whole_file:
	moveq	#3,d7
	move.l	vb_filenaam(a5),d1
	move.l	#mode_old,d2
	move.l	db_dosbase(a3),a6
	jsr	_LVOOpen(a6)
	move.l	d0,vb_filehandle(a5)
	beq.w	exit
	move.l	d0,d1
	lea.l	vb_breedte_x(a5),a2
	move.l	a2,d2
	moveq	#8,d3
	move.l	db_dosbase(a3),a6
	jsr	_LVORead(a6)
	lea.l	vb_breedte_x(a5),a1
	moveq	#1,d7
	cmp.l	#'FORM',(a1)+
	bne.b	exit
	move.l	(a1),d0
	add.l	#8,d0
	move.l	d0,vb_packed_size(a5)

	moveq	#2,d7
	move.l	#$10000,d1
	move.l	4,a6
	jsr	_LVOAllocMem(a6)

	move.l	d0,vb_packed(a5)
	beq.b	exit
	move.l	d0,a0

	move.l	vb_filehandle(a5),d1
	moveq	#0,d2				; seek beginning
	moveq	#-1,d3
	move.l	db_dosbase(a3),a6
	jsr	_LVOSeek(a6)		

	move.l	vb_packed_size(a5),d3
	move.l	vb_packed(a5),d2
	move.l	vb_filehandle(a5),d1
	move.l	db_dosbase(a3),a6
	jsr	_LVORead(a6)		

* hele file nu in het geheugen 

	bsr.b	close_file
	moveq	#0,d0
	rts

close_file:
	move.l	db_dosbase(a3),a6
	move.l	vb_filehandle(a5),d1
	jsr	_LVOClose(a6)
	clr.l	vb_filehandle(a5)
	rts

exit:	
	move.l	db_waarcolors1(a3),a1
	move.l	#COLSIZE,d0
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)
	clr.l	db_waarcolors1(a3)
	bsr.w	cleanexit
	move.l	db_easy_exit(a3),a7

	tst.l	db_unpacked(a3)
	beq.s	exit1
	move.l	db_unpacked(a3),a1
	move.l	db_unpacked_size(a3),d0
	move.l	$4,a6
	jsr	_LVOFreeMem(a6)
	clr.l	db_unpacked(a3)
exit1:	
	lea	db_fileblok1(a3),a5
	tst.l	vb_packed(a5)
	beq.s	exit221
	move.l	vb_packed(a5),a1
	move.l	vb_packed_size(a5),d0
	move.l	4,a6
	jsr	_LVOFreeMem(a6)
	clr.l	vb_packed(a5)
exit221:

	lea	db_fileblok1(a3),a5
	tst.l	vb_filehandle(a5)
	beq.s	exit21
	bsr.b	close_file
exit21:
	tst.l	db_mpmmubase(a3)
	beq	.no_mp
	move.l	db_mpmmubase(a3),a1
	move.l	$4,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	db_mpmmubase(a3)
.no_mp:
	tst.l	db_egsblitbase(a3)
	beq	.no_eb
	move.l	db_egsblitbase(a3),a1
	move.l	$4,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	db_egsblitbase(a3)
.no_eb:
	tst.l	db_egsbase(a3)
	beq	.no_egb
	move.l	db_egsbase(a3),a1
	move.l	$4,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	db_egsbase(a3)
.no_egb:
	tst.l	db_graphbase(a3)
	beq.s	exit3
	move.l	db_graphbase(a3),a1
	move.l	$4,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	db_graphbase(a3)
exit3:
	move.l	db_dosbase(a3),a1
	move.l	$4,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	db_dosbase(a3)
exit5:
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts
