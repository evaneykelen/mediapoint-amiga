
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

PREFS = 1
LIBV39 = 39
LIBV36 = 36

ANNO_SIZE = 200

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
	INCLUDE "graphics/videocontrol.i"
	INCLUDE	"intuition/intuitionbase.i"
	INCLUDE "intuition/preferences.i"
	INCDIR	"wp:asm/"
	INCLUDE "viewbloknew.i"
;	INCDIR	"aprogs:iff/"

	INCDIR	"wp:asm/riff/"
	INCLUDE	"viewiff.i"
	INCDIR	"wp:inclibs/"
	INCLUDE "dos_lib.i"
	INCLUDE "graphics_libv39.i"
	INCLUDE "intuition_lib.i"

;MON_ID = DEFAULT_MONITOR_ID

;MON_ID = DBLPAL_MONITOR_ID

;MON_ID = DBLPALLORESFF_KEY;

MON_ID = DEFAULT_MONITOR_ID

;MON_ID = DEFAULT_MONITOR_ID!HIRESLACE_KEY

mode_old = 1005

;	link	a5,#-db_SIZEOF
	
	movem.l	d0-d7/a0-a6,-(a7)
;	sub.l	#db_SIZEOF,a5
	lea	datablock(pc),a5

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
	cmp.b	#'c',(a0)
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
;	link	a5,#-db_SIZEOF
	movem.l	d0-d7/a0-a6,-(a7)
;	sub.l	#db_SIZEOF,a5
	lea	datablock(pc),a5
	move.l	a5,a3
	move.l	a5,a0
	move.l	#db_SIZEOF-1,d0
rep_cl2:
	move.b	#0,(a0)+
	dbf	d0,rep_cl2
	move.l	#filename,db_filename_pointer(a3)
	move.b	#0,db_nostop(a3)
	move.w	#50,db_waittofs(a3)
no_in:

	move.l	a7,db_easy_exit(a3)

	bsr.w	openlibs

	lea	db_fileblok1(a3),a0
	move.l	a0,db_inactive_fileblok(a3)
	lea	db_cop_colorsLOF(a3),a0
	move.l	a0,db_waarcolors1(a3)

	IF PREFS
	bsr.w	setprefs
	ENDC
	move.l	db_graphbase(a3),a6
	move.w	LIB_VERSION(a6),db_libversion(a3)
	
	bsr.w	set_view_structs

	move.l	db_filename_pointer(a3),a0
	bsr.w	laadfile
	bsr.w	proces_file		; maak van de eerste file een viewblok
	bsr.w	showpicture		; inactive -> active

;	bsr	convert_file

	move.w	db_colflags(a3),d0
	and.w	#1,d0
	beq	.no_colcyc
	move.w	db_colrate(a3),d0
	cmp.w	#36,d0
	beq	.no_colcyc

	bsr	col_cycle

.no_colcyc:
	bsr.w	wacht_change

	moveq	#0,d7
	bra.w	exit

	rts
input_error:
	moveq	#1,d7
	bra.w	exit5
	
;filename:	dc.b	"hd3:data/pictures/meisjes/cindy6",0
filename:	dc.b	"pics:cycling",0
	even
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

check_drng:
	tst.b	db_drng_found(a3)
	beq	no_col_cyc

	lea	db_fileblok1(a3),a5
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
	bsr	showpicture
	btst	#6,$bfe001
	bne.w	.rep_colc2		; start again
no_col_cyc:
	rts
	
col_cycle:
	tst.b	db_crng_found(a3)
	beq	check_drng
	lea	db_fileblok1(a3),a5
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
	bsr	showpicture
	btst	#6,$bfe001
	bne.w	.rep_colc2		; start again
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
	bsr	showpicture
	btst	#6,$bfe001
	bne.w	.rep_colc2		; start again
	rts

col_cyc_wait:
	moveq	#0,d2
	move.l	d2,d3
	move.w	db_colrate(a3),d2
	move.w	#16384,d3
	divu	d2,d3
	and.w	#$ff,d3
	bra	.ind
	move.l	db_graphbase(a3),a6	; wait 1/50
.delay:
	jsr	_LVOWaitTOF(a6)
.ind:
	dbf	d3,.delay
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

openlibs:
	moveq	#0,d0
	move.l	d0,db_graphbase(a3)
	move.l	d0,db_dosbase(a3)
	move.l	d0,db_intbase(a3)

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

	IF PREFS
	move.l	$4,a6
	moveq	#0,d0
	lea.l	intui(pc),a1
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,db_intbase(a3)
	beq.w	exit
	ENDC
	
	move.l	db_graphbase(a3),a6
	move.l	gb_ActiView(a6),db_oldview(a3)
	rts

dosnaam:	dc.b	'dos.library',0
graphname:	dc.b	'graphics.library',0
intui:		dc.b	'intuition.library',0
	even
*
* De bedoeling is dat hier in de locale structuren voor het scherm
* de globale bitmap rasinfo view en viewport structuur gezet moeten worden
*
set_view_structs:

	lea	db_fileblok1(a3),a5
	lea.l	db_view1(a3),a0
	move.l	a0,vb_vieww(a5)
	lea	db_viewport1(a3),a0
	move.l	a0,vb_viewportw(a5)
	lea	db_rasinfo1(a3),a0
	move.l	a0,vb_rasinfow(a5)		
	lea	db_bitmap1(a3),a0
	move.l	a0,vb_bitmapw(a5)
	rts

* unpack de file naar de inactive buffer en maak een viewport
* van de data die in de inactive file buffer staat
*
proces_file:
	lea	db_fileblok1(a3),a5

	cmp.w	#8,vb_planes(a5)
	ble.b	normal_unpack
	bsr.b	line_by_line
	bra.b	m_view
normal_unpack:

	bsr.w	unpack
	tst.w	d0
	bne.b	no_cando

m_view:
	lea	db_fileblok1(a3),a5
	bsr.w	maak_viewport		; de inactive view is nu klaar

	moveq	#0,d0
no_cando:
	rts

line_by_line:
	cmp.w	#24,vb_planes(a5)
	bne.b	no_24
	move.w	#6,db_skip_planes(a3)
	bra.b	line_by_linevar
no_24:
	cmp.w	#18,vb_planes(a5)
	bne.b	no_18
	move.w	#4,db_skip_planes(a3)
	bra.b	line_by_linevar
no_18:
	cmp.w	#12,vb_planes(a5)
	bne.b	no_12
	move.w	#2,db_skip_planes(a3)
	bra.b	line_by_linevar
no_12:
	rts

*
* Test a line by line unpack for 24 plane pictures
* The not AA version ( keep only 1 plane for each color
*
line_by_linevar:
	tst.b	db_aa_present(a3)
	bne.b	line_by_line_aa
	
	move.l	vb_body_start(a5),a0
	move.l	vb_tempbuffer(a5),a1
	move.l	a0,db_data_pointer(a3)
	move.l	a1,db_dest_pointer(a3)

	move.w	vb_leny(a5),d6
	subq.w	#1,d6
rep_leny:
	moveq	#2,d4			; for RGB
rep_1col:
	move.w	db_skip_planes(a3),d7
rep_planes:
	bsr.w	skipline
	dbf	d7,rep_planes
	bsr.w	decrunchline		; show the first line
	dbf	d4,rep_1col
	dbf	d6,rep_leny
	bsr.b	set_cols_3planes
	rts

set_cols_3planes:
	move.w	#3,vb_planes(a5)		; for RG & B
	lea.l	vb_colors(a5),a4

	move.l	db_waarcolors1(a3),a4

	lea	vb_colors(a5),a4
	move.l	#24,vb_color_count(a5)
	move.w	#$0000,(a4)+
	move.w	#$0f00,(a4)+
	move.w	#$00f0,(a4)+
	move.w	#$0ff0,(a4)+
	move.w	#$000f,(a4)+
	move.w	#$0f0f,(a4)+
	move.w	#$00ff,(a4)+
	move.w	#$0fff,(a4)+
	rts

line_by_line_aa:
	sub.w	#1,db_skip_planes(a3)
	move.l	vb_body_start(a5),a0
	move.l	vb_tempbuffer(a5),a1
	move.l	a0,db_data_pointer(a3)
	move.l	a1,db_dest_pointer(a3)

	move.w	vb_leny(a5),d6
	subq.w	#1,d6
rep_lenyaa:
	moveq	#2,d4			; for RGB
rep_1colaa:
	move.w	db_skip_planes(a3),d7
rep_planesaa:
	bsr.w	skipline
	dbf	d7,rep_planesaa
	bsr.w	decrunchline
	bsr.w	decrunchline
	dbf	d4,rep_1colaa
	dbf	d6,rep_lenyaa
	bsr.b	set_cols_6planes
	rts

set_cols_6planes:
	lea	vb_colbytes(a5),a0
	move.w	#6,vb_planes(a5)		; for RG & B
	move.l	#192,vb_color_count(a5)
	
	moveq	#0,d0
	moveq	#63,d7
rep_cols_aa:
	move.b	#0,(a0)
	move.b	#0,1(a0)
	move.b	#0,2(a0)

	btst	#1,d0
	beq.b	no_colb2
	move.b	#$7f,2(a0)
no_colb2:
	btst	#0,d0
	beq.b	no_colb1
	or.b	#$80,2(a0)
no_colb1:
	btst	#3,d0
	beq.b	no_colg2
	move.b	#$7f,1(a0)
no_colg2:
	btst	#2,d0
	beq.b	no_colg1
	or.b	#$80,1(a0)
no_colg1:
	btst	#5,d0
	beq.b	no_colr2
	move.b	#$7f,(a0)
no_colr2:
	btst	#4,d0
	beq.b	no_colr1
	or.b	#$80,(a0)
no_colr1:
	addq.l	#3,a0
	addq.l	#1,d0
	dbf	d7,rep_cols_aa
	rts

*
* decrunch een regel
*
decrunchline:	
	move.l	db_data_pointer(a3),a0		; de crunch data

	move.l	db_dest_pointer(a3),a1
	move.l	a1,a2
	add.l	vb_breedte_x(a5),a2
	
	cmp.b	#1,vb_compression(a5)
	beq.s	dun1

* compression geen 1 dus of 0 of een onbekende 
* copieer van body_start naar unpacked

	move.l	vb_breedte_x(a5),d0
	lsr.l	#1,d0
	subq.l	#1,d0

dno_co:	move.w	(a0)+,d1
	move.w	d1,(a1)+
	dbf	d0,dno_co
	move.l	a0,db_data_pointer(a3)		; volgende regel data
	rts

dun1:

dun_again:	
	cmp.l	a2,a1
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

* showpicture laat de inactive view zien
* en verandert deze naar de active view
* dit gebeurt ook met het fileblok
*
showpicture:
	lea		db_fileblok1(a3),a5
	move.l		db_graphbase(a3),a6
	move.l		vb_vieww(a5),a1
	jsr		_LVOLoadView(a6)

	rts

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

	IF PREFS
setprefs:
	move.l	db_intbase(a3),a6
	move.l	db_waarcolors1(a3),a0
	move.l	#400,d0
	jsr	_LVOGetPrefs(a6)
	move.l	db_waarcolors1(a3),a0

	move.b	pf_ViewXOffset(a0),db_Xoff(a3)
	move.b	pf_ViewYOffset(a0),db_Yoff(a3)

	move.l	db_graphbase(a3),a6
	move.b  gb_ChipRevBits0(a6),d0
	and.b   #SETCHIPREV_AA,d0     
	cmp.b   #SETCHIPREV_AA,d0     
	bne     no_aa1                
	move.b  #$ff,db_aa_present(a3)
no_aa1:
	rts
	ENDC
*
* a0 points to the FORM
*
laadmem:
	move.l	db_inactive_fileblok(a3),a5

	move.l	a0,vb_packed(a5)

	cmp.l	#'FORM',(a0)+
	bne.b	mem_exit
	move.l	(a0),d0
	add.l	#8,d0

	move.l	d0,vb_packed_size(a5)
	move.w	#3,vb_color_count(a5)
	move.b	#0,vb_bmhd_found(a5)
	move.b	#0,vb_body_found(a5)
	bsr.w	go_check
	moveq	#0,d0
	rts

mem_exit:
	moveq	#-1,d0
	rts

*
* Convert vb_colbytes a5 to db_waarcolors
*
convert_colors:
	lea		vb_colbytes(a5),a1
	move.l		db_waarcolors1(a3),a0
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

*
* laad_kleuren zet de kleuren afhankelijk van het OS
* in a5 de pointer naar het viewblok
*
*
laad_kleuren:
	tst.b		db_aa_present(a3)
	beq.b		no_v39_2

	move.l		vb_color_count(a5),d0
	divu		#3,d0
	move.l		#256,d0
laad_kleuren2:
	tst.b		db_aa_present(a3)
	beq.b		no_v39_3

	bsr.b		convert_colors

	move.l		db_waarcolors1(a3),a1
	move.l		vb_viewportw(a5),a0

	move.l		db_graphbase(a3),a6
	jsr		_LVOLoadRGB32(a6)
	bra.b		goon_aa1

no_v39_2:

	move.l		vb_viewportw(a5),a0
	move.l		vp_ColorMap(a0),d0

	lea		vb_colors(a5),a1
	move.l		vb_color_count(a5),d0
	divu		#3,d0
	moveq		#32,d0
	cmp.l		#32,d0
	ble.b		no_pro_th6
	moveq		#32,d0
no_pro_th6:
	move.l		db_graphbase(a3),a6
	jsr		_LVOLoadRGB4(a6)
goon_aa1:
	rts

no_v39_3:
	cmp.l	#32,d0
	ble.b	no_pro_th4
	moveq	#32,d0
no_pro_th4:
	move.l		vb_viewportw(a5),a0
	lea		vb_colors(a5),a1
	move.l		db_graphbase(a3),a6
	jsr		_LVOLoadRGB4(a6)
	rts

init_viewports:
	move.l  #0,vb_vextra(a3) 
	move.l  #0,vb_vpextra(a3)
	move.l  #0,vb_monspec(a3)
	rts
	
FLIP_SIZE = 580

maak_viewport:
ttt:
	move.w	vb_mode(a5),d0
;	and.w	#$fffb,vb_mode(a5)
	
	bsr.w	free_memory_view

	move.l	vb_vieww(a5),a1
	jsr	_LVOInitView(a6)

	move.l	vb_vieww(a5),a1
	move.w	vb_mode(a5),v_Modes(a1)
	and.w	#$fffb,v_Modes(a1)

	cmp.w	#36,db_libversion(a3)
	blt	no_v36_1

	move.l	#VIEW_EXTRA_TYPE,d0
	move.l	db_graphbase(a3),a6
	jsr	_LVOGfxNew(a6)
	move.l	d0,vb_vextra(a5)
	beq	no_v36

	move.l	vb_vieww(a5),a0

	move.l	db_oldview(a3),a1
	move.w	v_DxOffset(a1),v_DxOffset(a0)
	move.w	v_DyOffset(a1),v_DyOffset(a0)

	move.l	vb_vextra(a5),a1
	jsr	_LVOGfxAssociate(a6)

	move.l	vb_vieww(a5),a0
	or.w	#EXTEND_VSTRUCT,v_Modes(a0)

	move.l	#MON_ID,d0
	or.w	vb_mode(a5),d0
	move.l	#0,a1
	jsr	_LVOOpenMonitor(a6)
	move.l	d0,vb_monspec(a5)
	beq	no_v36_1
	move.l	vb_vextra(a5),a0
	move.l	d0,ve_Monitor(a0)

no_v36_1:

	bsr	do_init_bitmap


	move.l		vb_rasinfow(a5),a0		; initialiseer rasinfo
	move.l		vb_bitmapw(a5),a1

	move.l		a1,ri_BitMap(a0)
	move.w		#0,ri_RxOffset(a0)
	move.w		#0,ri_RyOffset(a0)
	move.l		#0,ri_Next(a0)

	move.l		vb_viewportw(a5),a0
	jsr		_LVOInitVPort(a6)

	move.l		vb_vieww(a5),a0
	move.l		vb_viewportw(a5),a1
	move.l		a1,v_ViewPort(a0)

;	move.w		vb_mode(a5),v_Modes(a0)
;	move.w		#0,v_Modes(a0)
;	or.w		#V_LACE,v_Modes(a0)

	move.l		vb_viewportw(a5),a0
	move.l		vb_rasinfow(a5),a1
	move.l		a1,vp_RasInfo(a0)

	move.w		vb_lenx(a5),d0
	move.w		vb_leny(a5),d1
	
	move.w		vb_lenx(a5),vp_DWidth(a0)
	move.w		vb_leny(a5),vp_DHeight(a0)
	move.w		vb_mode(a5),vp_Modes(a0)

	bsr	set_offsets

	cmp.w	#36,db_libversion(a3)
	blt	no_v36_2

	move.l	#VIEWPORT_EXTRA_TYPE,d0
	move.l	db_graphbase(a3),a6
	jsr	_LVOGfxNew(a6)
	move.l	d0,vb_vpextra(a5)	
	beq	no_v36

	lea	tags(pc),a0
	move.l	d0,12(a0)	

	move.l	#0,a0
	lea	vb_dimquery(a5),a1
	move.l	#dim_SIZEOF,d0
	move.l	#DTAG_DIMS,d1
	move.l	#MON_ID,d2
	or.w	vb_mode(a5),d2
	jsr	_LVOGetDisplayInfoData(a6)

	lea	vb_dimquery(a5),a0
	move.l	vb_vpextra(a5),a1
	move.l	dim_Nominal(a0),vpe_DisplayClip(a1)
	move.l	dim_Nominal+4(a0),vpe_DisplayClip+4(a1)

	move.l	#MON_ID,d0
	or.w	vb_mode(a5),d0
	jsr	_LVOFindDisplayInfo(a6)
	lea	tags(pc),a0
	move.l	d0,20(a0)


no_v36_2:
	moveq		#32,d0
	tst.b		db_aa_present(a3)
	beq.b		no_aa51
	move.w		#256,d0
no_aa51:
	jsr		_LVOGetColorMap(a6)

	move.l		d0,db_colmap(a3)
	beq.w		no_colormap

	cmp.w	#36,db_libversion(a3)
	blt	no_v36

* Try to create a display with the extra view-types

	move.l	vb_viewportw(a5),a2
	move.l	db_colmap(a3),a0
	lea	tags(pc),a1
	move.l	a2,4(a1)
	jsr	_LVOVideoControl(a6)
	bra	go_onv36

no_v36:
	move.l	db_colmap(a3),a0
	move.l	vb_viewportw(a5),a1
	move.l	a0,vp_ColorMap(a1)

go_onv36:
	bsr.w		laad_kleuren

	move.l		vb_vieww(a5),a0
	move.l		vb_viewportw(a5),a1
	jsr		_LVOMakeVPort(a6)

	move.l		vb_vieww(a5),a1
	jsr		_LVOMrgCop(a6)
	rts

tags:
	dc.l	VTAG_ATTACH_CM_SET,0
	dc.l	VTAG_VIEWPORTEXTRA_SET,0
	dc.l	VTAG_NORMAL_DISP_SET,0
	dc.l	VTAG_END_CM,0


do_init_bitmap:
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
	add.l		vb_breedte_x(a5),d0
	move.l		d0,bm_Planes+4(a0)
	add.l		vb_breedte_x(a5),d0
	move.l		d0,bm_Planes+8(a0)
	add.l		vb_breedte_x(a5),d0
	move.l		d0,bm_Planes+12(a0)
	add.l		vb_breedte_x(a5),d0
	move.l		d0,bm_Planes+16(a0)
	add.l		vb_breedte_x(a5),d0
	move.l		d0,bm_Planes+20(a0)
	add.l		vb_breedte_x(a5),d0
	move.l		d0,bm_Planes+24(a0)
	add.l		vb_breedte_x(a5),d0
	move.l		d0,bm_Planes+28(a0)
	rts
	
*************************

set_offsets:
	move.l		db_graphbase(a3),a6
	move.w		gb_NormalDisplayColumns(a6),db_colums(a3)
	move.w		gb_NormalDisplayRows(a6),db_rows(a3)
	move.w	db_colums(a3),d1	; zet de offsets berekend uit de prefs offsets
	tst.b	vb_hires(a5)
	bne.b	hi1
	lsr.w	#1,d1
hi1:
	tst.b	vb_shires(a5)
	beq.b	no_shi4
	lsl.w	#1,d1
no_shi4:
	move.w	vb_lenx(a5),d0	; pref.offset-(breedte - normal.breedte)/2

; test de offset berekening met de byte lengte

	move.l	vb_breedte_x(a5),d0
	lsl.l	#3,d0

	sub.w	d1,d0
	asr.w	#1,d0
	neg.w	d0
	IF PREFS
	move.b	db_Xoff(a3),d1	; standaard breedte
	ext.w	d1
	ELSE
	moveq	#0,d1
	ENDC
	tst.b	vb_hires(a5)
	bne.b	hi2
	asr.w	#1,d1
hi2:
	tst.b	vb_shires(a5)
	beq.b	no_shi2
	asl.w	#1,d1
no_shi2:
	add.w	d1,d0
	move.w	d0,vp_DxOffset(a0)

ptt10:
	move.w	db_rows(a3),d1
	tst.b	vb_interlace(a5)
	beq.b	no_inter1
	lsl.w	#1,d1
no_inter1:
	move.w	vb_leny(a5),d0
	cmp.w	#FLIP_SIZE,d0
	ble.b	oke_check_size
	move.l	#FLIP_SIZE,d0
oke_check_size:
	sub.w	d1,d0
	asr.w	#1,d0

	neg.w	d0

	btst	#0,d0
	beq.b	no_onev1
	add.w	#1,d0
no_onev1:

	IF PREFS
	move.b	db_Yoff(a3),d1
	ext.w	d1
	ELSE
	moveq	#0,d1
	ENDC
	tst.b	vb_interlace(a5)
	beq.b	no_inter5
	asl.w	#1,d1
no_inter5:
	add.w	d1,d0
	tst.b	db_aa_present(a3)
	beq.b	no_aa1001
	add.w	#1,d0
no_aa1001:
	move.w		d0,vp_DyOffset(a0)	
	rts

*************************


*
* Wait until the view changed or a mous button
*
wacht_change:
	tst.b	db_nostop(a3)
	beq.b	dostop

	move.w	db_waittofs(a3),d7
rep_ww:
	btst	#6,$bfe001
	beq.b	cleanexit
	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)
	dbf	d7,rep_ww
	rts
dostop:
	btst		#6,$bfe001
	beq.b		cleanexit
	move.l		db_graphbase(a3),a6
	move.l		vb_vieww(a5),a0
	move.l		a0,d0
	cmp.l		gb_ActiView(a6),d0	; is de view weg ?
	beq.b		wacht_change
	rts

cleanexit:
	tst.l		db_oldview(a3)			; is er een oldview
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
	tst.l		vp_ColorMap(a0)
	beq.b		no_colormem

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
	move.l	vb_monspec(a5),d0
	beq	no_mon
	move.l	db_graphbase(a3),a6
	move.l	d0,a0
	jsr	_LVOCloseMonitor(a6)
	clr.l	vb_monspec(a5)
no_mon:
	move.l	vb_vpextra(a5),d0
	beq	no_vpextra
	move.l	d0,a0
	jsr	_LVOGfxFree(a6)
	clr.l	vb_vpextra(a5)
no_vpextra:
	move.l	vb_vextra(a5),d0
	beq	no_vextra
	move.l	d0,a0
	jsr	_LVOGfxFree(a6)
	clr.l	vb_vextra(a5)
no_vextra:
	rts

	moveq		#0,d0
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
	move.b	#0,db_crng_found(a3)
	move.b	#0,db_drng_found(a3)
	
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
cr_f:
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
	move.b	(a6),d0		; min cell
	move.b	1(a6),d0	; max cell
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
	move.l	#-1,a1
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
	move.w	#3,d1
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
	bne.w	exit
	move.l	(a1),d0
	add.l	#8,d0
	move.l	d0,vb_packed_size(a5)

	moveq	#2,d7
	move.l	#$10000,d1
	move.l	4,a6
	jsr	_LVOAllocMem(a6)

	move.l	d0,vb_packed(a5)
	beq.w	exit
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
*
* Copy only those chunks you want
*
convert_file:
	move.l	vb_packed_size(a5),d0
	add.l	#ANNO_SIZE,d0
	move.l	$4.w,a6
	move.l	#MEMF_PUBLIC!MEMF_CLEAR,d1
	jsr	_LVOAllocMem(a6)
	move.l	d0,db_newmem(a3)
	beq	exit
	move.l	d0,a4
	move.l	vb_packed(a5),a1

	move.l	(a1),(a4)
	move.l	4(a1),4(a4)
	move.l	8(a1),8(a4)		; the ILBM sign
	lea	12(a4),a4

	moveq	#4,d7			; new size
	move.l	4(a1),d0		; file size
	lea	8(a1),a1
	move.l	a1,a2
	addq.l	#4,a1			; skip het ILBM sign
	add.l	d0,a2			; einde data

	bsr	copy_anno
	
cchunks1b:
	cmp.l	#"BMHD",(a1)
	beq	copy_chunk
	cmp.l	#"CAMG",(a1)
	beq	copy_chunk
	cmp.l	#"CMAP",(a1)
	beq	copy_chunk
	cmp.l	#"BODY",(a1)
	beq	copy_chunk

	bra.w	continue_chunks2

copy_chunk:
	addq.l	#8,d7
	add.l	4(a1),d7		; total size addup
	btst	#0,d7
	beq	.nopro
	addq.l	#1,d7
.nopro:
	move.l	a1,a6

	move.l	4(a1),d6
	move.l	(a6)+,(a4)+
	move.l	(a6)+,(a4)+
	addq.l	#1,d6
	lsr.l	#1,d6
rep_copy:
	move.w	(a6)+,(a4)+
	subq.l	#1,d6
	bne	rep_copy
	
continue_chunks2:
	add.l	4(a1),a1
	move.l	a1,d0
	btst	#0,d0
	beq.b	no_oneven2b
	addq.l	#1,a1
no_oneven2b:
	addq.l	#8,a1
	cmp.l	a2,a1
	blt.w	cchunks1b
tt5:
	move.l	db_newmem(a3),a4
	move.l	4(a4),d6
;	cmp.l	d7,d6
;	beq	nodiff_files
	move.l	d7,4(a4)
	add.l	#8,d7
	move.l	d7,db_newsize(a3)
	bsr	write_file
nodiff_files:
	rts

copy_anno:

	lea	anno_chunk(pc),a6
	move.l	4(a6),d6
	add.l	#8,d7
	add.l	d6,d7

	move.l	(a6)+,(a4)+
	move.l	(a6)+,(a4)+

	addq.l	#1,d6
	lsr.l	#1,d6
	subq.w	#1,d6
rep_copy2:
	move.w	(a6)+,(a4)+	
	dbf	d6,rep_copy2
	rts

anno_chunk:
	dc.l	"ANNO",10
	dc.b	"MediaPoint"
	even
	
write_file:
	lea	db_fileblok1(a3),a5
	tst.l	vb_filehandle(a5)		; close old file
	beq	no_cf
	bsr	close_file
no_cf:
	move.l	vb_filenaam(a5),d1
	move.l	db_dosbase(a3),a6
	jsr	_LVODeleteFile(a6)

	moveq	#3,d7
	move.l	vb_filenaam(a5),d1
	move.l	#1006,d2
	move.l	db_dosbase(a3),a6
	jsr	_LVOOpen(a6)
	move.l	d0,vb_filehandle(a5)
	beq.w	exit
	move.l	db_newsize(a3),d3
	move.l	db_newmem(a3),d2
	move.l	vb_filehandle(a5),d1
	move.l	db_dosbase(a3),a6
	jsr	_LVOWrite(a6)
	bsr	close_file	
	rts

exit:	
	bsr.w	cleanexit
	move.l	db_easy_exit(a3),a7

	lea	db_fileblok1(a3),a5
	bsr.w	free_memory_view

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
	tst.l	db_newmem(a3)
	beq	exit222
	move.l	db_newmem(a3),a1
	move.l	vb_packed_size(a5),d0
	add.l	#ANNO_SIZE,d0
	move.l	4,a6
	jsr	_LVOFreeMem(a6)
	clr.l	db_newmem(a3)

exit222:
	lea	db_fileblok1(a3),a5
	tst.l	vb_filehandle(a5)
	beq.s	exit21
	bsr.w	close_file
exit21:
	tst.l	db_graphbase(a3)
	beq.s	exit3
	move.l	db_graphbase(a3),a1
	move.l	$4,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	db_graphbase(a3)
exit3:
	IF PREFS
	tst.l	db_intbase(a3)
	beq.b	exit4c
	move.l	db_intbase(a3),a1
	move.l	$4,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	db_intbase(a3)
exit4c:
	ENDC
exit51:
	move.l	db_dosbase(a3),a1
	move.l	$4,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	db_dosbase(a3)
exit5:
	movem.l	(a7)+,d0-d7/a0-a6
;	unlk	a5
	moveq	#0,d0
	rts
datablock:	ds.b	db_SIZEOF
w:
	dc.l	"WALL"

