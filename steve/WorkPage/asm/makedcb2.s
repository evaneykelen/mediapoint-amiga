* Probeer het eens met een viewport
* 28 augustus 1992
*
* IFF reader door Cees Lieshout 14-april-1990
* start vanuit SEKA op met j start 
* vanuit CLI  IFFREAD filenaam 
*
	INCDIR "asm:include/"

	INCLUDE "exec/execbase.i"
	INCLUDE "exec/exec_lib.i"
	INCLUDE "graphics/gfxbase.i"
	INCLUDE "graphics/graphics_lib.i"
	INCLUDE "graphics/view.i"
	INCLUDE "graphics/gfx.i"
	INCLUDE "intuition/intuition_lib.i"
	INCLUDE "intuition/preferences.i"

SIZEX_IMG = 3
SIZEY_IMG = 21
NUMIMG = 2

l_colors = -128
l_masking = -129
l_compression = -130
l_interlace = -131
l_hires = -132
l_mode = -134
l_planes = -136
l_leny = -138
l_lenx = -140
l_hoogte_y = -144
l_breedte_x = -148
l_color_count = -152
l_bitmap_size = -156
l_cmap_size = -160
l_body_start = -164
l_unpacked_size = -168
l_unpacked = -172
l_conhandle = -176
l_graphbase = -180
l_dosbase = -184
l_easy_exit = -188
l_packed_size = -192
l_packed = -196
l_oldview = -200
l_filenaam = -204
l_filehandle = -208
l_mem_size = -212
l_body_found = -213
l_bmhd_found = -214
l_imess = -250

l_view = -268
l_viewport = -308
l_rasinfo = -320
l_bitmap = -360
l_GfxBase = -364

findresident = -96
openlib = -408
closelib = -414
allocmem = -198
freemem = -210

open = -30
close = -36
output = -60
seek = -66
read = -42
write = -48

DisplayAllert = -90

mode_old = 1005
cop_size = 1000

	link	a5,#-500

	subq	#1,d0
	beq.W	exit5
zoeken:
	cmp.b	#$20,(a0)+
	bne.s	gevonden
	dbra	d0,zoeken
	bra.W	exit5
gevonden:
	subq.l	#1,a0
	move.l	a0,l_filenaam(a5)
zet:	cmp.b	#10,(a0)+
	bne.s	zet
	move.b	#0,-(a0)
	bra.b	no_in

start:	
	link	a5,#-500

	lea.l	filename(pc),a0
	move.l	a0,l_filenaam(a5)

no_in:
	move.w	#3,l_color_count(a5)
	move.w	#0,l_bmhd_found(a5)
	move.l	a7,l_easy_exit(a5)

	bsr.w	openlibs

	bsr.w	read_whole_file
*****
	bsr.w	check_chunks
	moveq	#1,d7
	cmp.w	#$ffff,l_bmhd_found(a5)
	bne.w	exit

	bsr.w	unpack
	
	move.l	$4,a6
	clr.l	d0
	lea.l	intui(pc),a1
	jsr	openlib(a6)
	move.l	d0,intbas
	beq	exit
	move.l	d0,a6
	lea	prefs(pc),a0
	move.l	#400,d0
	jsr	_LVOGetPrefs(a6)
	lea	prefs(pc),a0

	move.b	pf_ViewXOffset(a0),Xoff
	move.b	pf_ViewYOffset(a0),Yoff

	move.w	pf_ViewInitX(a0),initxoff
	move.w	pf_ViewInitY(a0),inityoff

	move.l	a6,a1
	move.l	$4,a6
	jsr	_LVOCloseLibrary(a6)	

	bsr.w	maak_viewport

	bsr	generate_dc
	
	moveq	#0,d7
	bra.w	exit

	rts

*
* Schrijf naar standaard output assembly coded uitvoer
*
generate_dc:
	tst.l	l_conhandle(a5)
	beq	no_output
	cmp.w	#1,l_planes(a5)
	bne	no_output

	move.l	l_unpacked(a5),a0
	move.l	l_breedte_x(a5),d1
	moveq	#0,d2
	moveq	#0,d3
	move.l	d1,d4
	mulu	#SIZEY_IMG,d4

	move.l	a0,a3
	move.l	a0,a2
	move.l	a0,a1
	move.l	#NUMIMG-1,d7
rep_img1:
	move.l	a2,a0
	move.l	d2,d0
	bsr	plaats_label

	move.l	#SIZEY_IMG-1,d6
rep_img2:
	move.l	a0,a1
	bsr	plaats_dcb
	move.l	#SIZEX_IMG-2,d5
rep_img3:
	move.b	(a1)+,d0
	bsr	printd0k
	dbf	d5,rep_img3

	bsr	printd0

	add.l	d1,a0
	dbf	d6,rep_img2

	add.l	#SIZEX_IMG,a2	
	add.l	#SIZEX_IMG,d3
	cmp.l	d1,d3
	blt	no_pro

	add.l	d4,a3		; next line
	move.l	a3,a2
	moveq	#0,d3
no_pro:
	addq	#1,d2
	dbf	d7,rep_img1
no_output:
	rts

printd0:
	movem.l	a0-a6/d0-d7,-(a7)
	lea	outbuf(pc),a0
	bsr	asc_d0
	move.b	#$a,(a0)+
	moveq	#4,d3
	bsr	print	
	movem.l	(a7)+,d0-d7/a0-a6
	rts

printd0k:
	movem.l	a0-a6/d0-d7,-(a7)
	lea	outbuf(pc),a0
	bsr	asc_d0
	move.b	#',',(a0)+
	moveq	#4,d3
	bsr	print	
	movem.l	(a7)+,d0-d7/a0-a6
	rts

asc_d0:
	move.b	#'$',(a0)+
asc_d02:
	move.l	d0,d1
	lsr.b	#4,d1
	and	#$f,d1
	add	#$30,d1
	cmp	#'9',d1
	bls	nibok
	add	#7,d1
nibok:	
	move.b	d1,(a0)+
	move.l	d0,d1
;	lsr.b	#4,d1
	and	#$f,d1
	add	#$30,d1
	cmp	#'9',d1
	bls	nibok2
	add	#7,d1
nibok2:	
	move.b	d1,(a0)+
	rts

print:
	move.l	l_dosbase(a5),a6
	jsr	output(a6)
	move.l	d0,d1
	move.l	#outbuf,d2
	jsr	write(a6)
	rts

plaats_label:
	movem.l	a0-a6/d0-d7,-(a7)
	lea	hstr4+7(pc),a0
	move.l	d0,d7
	lsr.w	#8,d0
	and.l	#$ff,d0
	bsr	asc_d02
	move.l	d7,d0
	and.l	#$ff,d0
	bsr	asc_d02
	move.l	l_dosbase(a5),a6
	jsr	output(a6)
	move.l	d0,d1
	move.l	#hstr4,d2
	moveq	#12,d3
	jsr	write(a6)
	movem.l	(a7)+,d0-d7/a0-a6
	rts

plaats_dcb:
	movem.l	a0-a6/d0-d7,-(a7)

	move.l	l_dosbase(a5),a6
	jsr	output(a6)
	move.l	d0,d1
	move.l	#hstr1,d2
	moveq	#6,d3
	jsr	write(a6)
	movem.l	(a7)+,d0-d7/a0-a6

	rts
		
hstr1:	dc.b	"	dc.b	",0
hstr2:	dc.b	",",0
hstr3:	dc.b	10,0
hstr4:	dc.b	10,"effimg    :",0

outbuf:	blk.b	20,0

	even
		
maak_viewport:

	move.l		#0,d0
	lea.l		graphname(pc),a1
	move.l		$4,a6
	jsr		_LVOOpenLibrary(a6)
	move.l		d0,l_GfxBase(a5)
	beq		no_gfx

	move.l		d0,a6
	move.l		gb_ActiView(a6),l_oldview(a5)

	move.w		gb_NormalDisplayColumns(a6),colums
	move.w		gb_NormalDisplayRows(a6),rows

	move.w		gb_MaxDisplayColumn(a6),maxcolums
	move.w		gb_MaxDisplayRow(a6),maxrows

	lea		l_view(a5),a1
	jsr		_LVOInitView(a6)

	lea		l_viewport(a5),a0
	jsr		_LVOInitVPort(a6)

	lea		l_view(a5),a0
	lea		l_viewport(a5),a1
	move.l		a1,v_ViewPort(a0)
;	move.w		#0,v_DxOffset(a0)
	move.w		l_mode(a5),v_Modes(a0)

	lea		l_bitmap(a5),a0
	move.w		l_planes(a5),d0			; depth

	move.l		l_breedte_x(a5),d1		; breedte in bytes * 8
	mulu		l_planes(a5),d1
	lsl.l		#3,d1				; width
	move.w		l_leny(a5),d2			; height
	jsr		_LVOInitBitMap(a6)

	lea		l_bitmap(a5),a0

	move.l		l_unpacked(a5),d0

	move.l		d0,bm_Planes(a0)		; zet de bitplanes

	add.l		l_breedte_x(a5),d0
	move.l		d0,bm_Planes+4(a0)

	add.l		l_breedte_x(a5),d0
	move.l		d0,bm_Planes+8(a0)

	add.l		l_breedte_x(a5),d0
	move.l		d0,bm_Planes+12(a0)

	add.l		l_breedte_x(a5),d0
	move.l		d0,bm_Planes+16(a0)

	add.l		l_breedte_x(a5),d0
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
	move.w		l_lenx(a5),vp_DWidth(a0)
	move.w		l_leny(a5),vp_DHeight(a0)
	move.w		l_mode(a5),vp_Modes(a0)

;	move.w	#368,d0
;	sub.w	l_lenx(a5),d0
;	lsr.w	#1,d0

	move.w	colums(pc),d1
	tst.b	l_hires(a5)
	bne	hi1
	lsr.w	#1,d1
hi1:
	move.w	l_lenx(a5),d0
	sub.w	d1,d0
	asr.w	#1,d0
	neg.w	d0
	move.b	Xoff(pc),d1
	ext.w	d1
	add.w	d1,d0
	move.w	d0,vp_DxOffset(a0)

	move.w	rows(pc),d1
	tst.b	l_interlace(a5)
	beq	no_inter
	lsl.w	#1,d1
no_inter:
	move.w	l_leny(a5),d0
	sub.w	d1,d0
	asr.w	#1,d0
	neg.w	d0
	move.b	Yoff(pc),d1
	ext.w	d1
	add.w	d1,d0
	move.w	d0,vp_DyOffset(a0)	

	move.l		#32,d0
	jsr		_LVOGetColorMap(a6)
	lea		l_viewport(a5),a0
	move.l		d0,vp_ColorMap(a0)
	beq		no_colormap

	lea		l_viewport(a5),a0
	lea		l_colors(a5),a1
	move.l		#32,d0
	jsr		_LVOLoadRGB4(a6)

	lea		l_view(a5),a0
	lea		l_viewport(a5),a1
	jsr		_LVOMakeVPort(a6)

	lea		l_view(a5),a1
	jsr		_LVOMrgCop(a6)

	lea		l_view(a5),a1
	jsr		_LVOLoadView(a6)

	lea.l		l_view(a5),a0
	move.l		v_LOFCprList(a0),a0
	move.l		8(a0),d2		; aantal elementen
	move.l		4(a0),a0		; eigenlijke copperlist

	subq.l		#1,d2
	lea.l		copdata(pc),a1
rep_copcop:
	move.w		(a0)+,d0
	cmp.w		#$ffff,d0
	beq		nokke
	move.w		d0,(a1)+
	move.w		(a0)+,d0
	cmp.w		#$ffff,d0
	beq		nokke
	move.w		d0,(a1)+
	dbf		d2,rep_copcop
nokke:

wacht_change:
	btst		#6,$bfe001
	beq		cleanexit
	move.l		l_GfxBase(a5),a6
	lea		l_view(a5),a0
	move.l		a0,d0
	cmp.l		gb_ActiView(a6),d0
	beq		wacht_change

cleanexit:
	tst.l		l_oldview(a5)
	beq		no_oldview
	move.l		l_oldview(a5),a1
	jsr		_LVOLoadView(a6)

	jsr		_LVOWaitTOF(a6)

no_oldview:

freememory:
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
	move.l		l_GfxBase(a5),a1
	move.l		$4,a6
	jsr		_LVOCloseLibrary(a6)
no_gfx:
	moveq		#0,d0
	rts
unpack:	
	move.l	l_body_start(a5),a0
	move.l	l_unpacked(a5),a1

	cmp.b	#1,l_compression(a5)
	beq.s	un1

* compression geen 1 dus of 0 of een onbekende 
* copieer van body_start naar unpacked

	move.l	l_unpacked_size(a5),d0
	lsr.l	#1,d0
	subq.l	#1,d0

	move.l	l_body_start(a5),a0

no_co:	move.w	(a0)+,d1
	move.w	d1,(a1)+
	dbf	d0,no_co
	rts

un1:
	move.l	l_unpacked(a5),a3
	add.l	l_unpacked_size(a5),a3
un_again:	
	cmp.l	a3,a1
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
	rts

check_chunks:
	move.l	l_packed(a5),a1
	move.l	a1,a2
	addq.l	#4,a1			; skip het ILBM sign
	add.l	l_packed_size(a5),a2	; einde data
cchunks1:
	cmp.l	#"BMHD",(a1)
	bne.w	no_bmhd_proc
	move.b	#$ff,l_bmhd_found(a5)
	clr.w	l_hires(a5)
	lea	8(a1),a3
	move.w	(a3),l_lenx(a5)
	move.w	2(a3),l_leny(a5)
	moveq	#0,d1
	move.w	l_leny(a5),d1
	move.l	d1,l_hoogte_y(a5)
	moveq	#0,d1
	move.b	8(a3),d1
	move.w	d1,l_planes(a5)
	move.b	10(a3),l_compression(a5)
	move.b	9(a3),l_masking(a5)

	move.l	12(a3),transcolor
	
	moveq	#0,d1
	moveq	#0,d2
;	move.l	#$0200,d3
	move.w	l_lenx(a5),d1
	cmp.w	#370,d1
	ble.s	geen_hires
	move.b	#$FF,l_hires(a5)
	move.w	#$8000,d2
geen_hires:
	move.w	l_leny(a5),d1
	cmp.w	#390,d1
	ble.s	geen_lace
	move.b	#$ff,l_interlace(a5)
	or.w	#$4,d2
geen_lace:
	cmp.w	#6,l_planes(a5)
	bne.s	geen_ham
	or.w	#$800,d2
geen_ham:
	move.w	d2,d3
;	or.w	#$0200,d3
	move.w	d3,l_mode(a5)

	move.l	a1,-(a7)
	bsr.w	unpack_init
	move.l	(a7)+,a1

	bra.w	continue_chunks

no_bmhd_proc:
	cmp.l	#"CAMG",(a1)
	bne.b	no_camg_proc	

	move.w	l_mode(a5),d2
	move.w	10(a1),d0
	move.w	d0,d1
	and.w	#$8000,d0
	beq.b	cno_hires
	move.b	#$ff,l_hires(a5)
	or.w	#$8000,d2
cno_hires:
	move.w	d1,d0
	and.w	#$4,d0
	beq.b	cno_lace
	move.b	#$ff,l_interlace(a5)
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
	move.w	d2,l_mode(a5)

no_camg_proc:
	cmp.l	#"CMAP",(a1)
	bne.b	no_cmap_proc

	lea	4(a1),a3
	move.l	(a3),l_cmap_size(a5)
	move.l	(a3)+,d1

	move.l	d1,l_color_count(a5)

* in d1 het aantal kleuren * 3

	lea.l	l_colors(a5),a4
cmap4:
	moveq	#0,d3
	moveq	#0,d2
	move.b	(a3)+,d3
	lsl.w	#4,d3
	move.b	(a3)+,d3
	move.b	(a3)+,d2
	lsr.b	#4,d2
	and.b	#$f0,d3
	or.b	d2,d3
	move.w	d3,(a4)+
	subq.w	#3,d1
	bne.b	cmap4
	bra.b	continue_chunks

no_cmap_proc:

	cmp.l	#"BODY",(a1)
	bne.b	no_body_proc
	move.b	#$ff,l_body_found(a5)
	lea	8(a1),a3
	move.l	a3,l_body_start(a5)
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
	move.l	l_mode(a5),mode
	rts

unpack_init:

	moveq	#0,d0
	moveq	#0,d1
	move.w	l_lenx(a5),d0
	move.w	d0,d1
	add.l	#15,d1
	lsr.l	#4,d1			; aantal words
	add.l	d1,d1			; aantal bytes
	move.l	d1,l_breedte_x(a5)
	move.l	d1,d0
	move.w	l_leny(a5),d1
	mulu	d1,d0

	move.l	d0,d2
	ext.l	d2
	move.l	d2,l_bitmap_size(a5)
	moveq	#0,d1	
	move.w	l_planes(a5),d1
	cmp.b	#1,l_masking(a5)
	bne.b	no_masking
	addq.w	#1,d1
no_masking:
	mulu	d1,d0
	moveq	#2,d7	

	move.l	d0,l_unpacked_size(a5)

;	add.l	#cop_size,d0

	move.l	d0,l_mem_size(a5)

	move.l	#$10002,d1
	move.l	$4,a6
	jsr	allocmem(a6)
	move.l	d0,l_unpacked(a5)
	beq.w	exit

;	move.l	d0,l_mem_cop(a5)
;	add.l	#cop_size,l_unpacked(a5)
	move.l	l_breedte_x(a5),d0
	lsl.w	#3,d0
	move.w	d0,l_lenx(a5)

	rts

read_whole_file:
	moveq	#3,d7
	move.l	l_filenaam(a5),d1
	move.l	#mode_old,d2
	move.l	l_dosbase(a5),a6
	jsr	open(a6)
	move.l	d0,l_filehandle(a5)
	beq.w	exit
	move.l	d0,d1
	lea.l	l_breedte_x(a5),a2
	move.l	a2,d2
	moveq	#8,d3
	move.l	l_dosbase(a5),a6
	jsr	read(a6)
	lea.l	l_breedte_x(a5),a1
	moveq	#1,d7
	cmp.l	#'FORM',(a1)+
	bne.w	exit
	move.l	(a1),l_packed_size(a5)
	move.l	(a1),d0
	moveq	#2,d7
	move.l	#$10000,d1
	move.l	$4,a6
	jsr	allocmem(a6)
	move.l	d0,l_packed(a5)
	beq.b	exit
	move.l	l_packed_size(a5),d3
	move.l	d0,d2
	move.l	l_filehandle(a5),d1
	move.l	l_dosbase(a5),a6
	jsr	read(a6)		

* hele file nu in het geheugen 

	bsr.b	close_file
	moveq	#0,d0
	rts

close_file:
	move.l	l_dosbase(a5),a6
	move.l	l_filehandle(a5),d1
	jsr	close(a6)
	clr.l	l_filehandle(a5)
	rts
	
openlibs:
	moveq	#0,d0
;	move.l	d0,l_mem_cop(a5)
	move.l	d0,l_packed(a5)
	move.l	d0,l_unpacked(a5)
	move.l	d0,l_filehandle(a5)
	move.l	d0,l_graphbase(a5)
	move.l	d0,l_dosbase(a5)

	move.l	$4,a6
	lea	dosname(pc),a1
	moveq	#0,d0
	jsr	openlib(a6)
	move.l	d0,l_dosbase(a5)
	beq.s	exit

	lea	graphname(pc),a1
	moveq	#0,d0
	move.l	$4,a6
	jsr	openlib(a6)
	move.l	d0,l_graphbase(a5)
	beq.s	exit

	move.l	l_dosbase(a5),a6
	jsr	output(a6)
	move.l	d0,l_conhandle(a5)
	rts

exit:	
	move.l	l_easy_exit(a5),a7

	tst.l	l_unpacked(a5)
	beq.s	exit1
	move.l	l_unpacked(a5),a1
	move.l	l_mem_size(a5),d0
	move.l	$4,a6
	jsr	freemem(a6)
	clr.l	l_unpacked(a5)

exit1:	tst.l	l_packed(a5)
	beq.s	exit21
	move.l	l_packed(a5),a1
	move.l	l_packed_size(a5),d0
	move.l	$4,a6
	jsr	freemem(a6)
	clr.l	l_packed(a5)

exit21:
exit22:	tst.l	l_filehandle(a5)
	beq.s	exit2
	bsr.w	close_file
	
exit2:	tst.l	l_graphbase(a5)
	beq.s	exit3
	move.l	l_graphbase(a5),a1
	move.l	$4,a6
	jsr	closelib(a6)
	clr.l	l_graphbase(a5)

exit3:
exit4:	tst.l	l_dosbase(a5)
	beq.s	exit5
	tst.l	d7
	beq.s	exit51
	subq.l	#1,d7
	move.l	l_dosbase(a5),a6
	lea.l	foutmelding(pc),a1
	lsl.l	#2,d7
	add.l	d7,a1

	lea	foutmelding(pc),a2
	move.l	a2,d2
	add.l	(a1),d2
	move.l	l_conhandle(a5),d1

	move.l	l_conhandle(a5),d1
	beq.b	no_con_open

	move.l	#FOUT_LENGTE,d3
	jsr	write(a6)

exit51:	move.l	l_dosbase(a5),a1
	move.l	$4,a6
	jsr	closelib(a6)
	clr.l	l_dosbase(a5)
exit5:
	unlk	a5
	moveq	#0,d0
	rts

no_con_open:
	move.l	d2,a0
	move.l	#FOUT_LENGTE-3,d3

	lea.l	l_imess(a5),a1

	move.b	#0,(a1)+
	move.b	#20,(a1)+
	move.b	#20,(a1)+

c_fout:	move.b	(a0)+,(a1)+
	dbf	d3,c_fout

	move.b	#' ',(a1)+
	move.b	#':',(a1)+
	move.b	#' ',(a1)+
	
	move.l	l_filenaam(a5),a0
c2_fout:
	move.b	(a0)+,d0
	move.b	d0,(a1)+
	bne.b	c2_fout
		
	move.b	#0,(a1)+
	move.b	#0,(a1)+

	move.l	$4,a6
	lea.l	intui(pc),a1
	jsr	openlib(a6)

	tst.l	d0
	beq.b	err_intui
	move.l	d0,a6
	moveq	#0,d0

	lea.l	l_imess(a5),a0
	move.l	#40,d1
	jsr	DisplayAllert(a6)

	move.l	a6,a1
	move.l	$4,a6
	jsr	closelib(a6)
err_intui
	bra.w	exit51

FOUT_LENGTE = 21

foutmelding:	dc.l	fout0-foutmelding,fout1-foutmelding,fout2-foutmelding

fout0:		dc.b	"Geen IFF file       ",10,0
fout1:		dc.b	"Geen geheugen genoeg",10,0
fout2:		dc.b	"File niet gevonden  ",10,0

dosname:	dc.b	'dos.library',0
graphname:	dc.b	'graphics.library',0
intui:		dc.b	'intuition.library',0
;filename:	dc.b	'pics:1001pics/pictfiles/hb-und',0
filename:	dc.b	'pics:astron.ham',0
;filename:	dc.b	'pics:oog_brush',0
;filename:	dc.b	'pics:world.dctv',0
	even
prefs:		blk.l	1000,0
intbas:		dc.l	0
colums:		dc.w	0
rows:		dc.w	0
maxcolums:		dc.w	0
maxrows:		dc.w	0

initxoff:	dc.w	0
inityoff:	dc.w	0

Xoff:		dc.w	0
Yoff:		dc.w	0

copdata:	blk.l	100,0
transcolor:	dc.l	0
mode:		dc.l	0
	END
