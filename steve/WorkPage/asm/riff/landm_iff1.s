*
* Mean and Lean IFF reader
* to be used in other programs
*
* C. Lieshout 18-04-1994
*

	INCDIR	"include:"
	INCLUDE	"exec/memory.i"
	
	INCDIR	"wp:inclibs/"
	INCLUDE "dos_lib.i"
	INCLUDE "exec_lib.i"

	RSRESET
iff_struct:	rs.w	0
iff_error:	rs.w	1
iff_width:	rs.w	1
iff_height:	rs.w	1
iff_planes:	rs.w	1
iff_mode:	rs.w	1
iff_colors:	rs.l	1
iff_image:	rs.l	1
iff_SIZEOF:	rs.w	0

	RSRESET
iloc_struct:		rs.w	0
iloc_filename:		rs.l	1
iloc_filehandle:	rs.l	1
iloc_deststruct:	rs.l	1
iloc_memtype:		rs.l	1
iloc_dosbase:		rs.l	1
iloc_body_start:	rs.l	1

iloc_lenx:		rs.w	1
iloc_leny:		rs.w	1
iloc_planes:		rs.w	1
iloc_mode:		rs.w	1

iloc_packed:		rs.l	1
iloc_packed_size:	rs.l	1
iloc_decrunched:	rs.l	1
iloc_decrunched_size:	rs.l	1
iloc_colors:		rs.l	1
iloc_colpointer:	rs.l	1

iloc_bmhd_found:	rs.b	1
iloc_body_found:	rs.b	1
iloc_compression:	rs.b	1
iloc_masking:		rs.b	1

iloc_buf:		rs.l	2
iloc_SIZEOF:		rs.w	0

mode_old = 1005

start:
	lea	test1(pc),a1
	lea	filename(pc),a0
	move.l	#MEMF_CHIP,d0
	bsr	get_iff
	rts

test1:	blk.b	iff_SIZEOF,0
filename:	dc.b	"pics:sexy",0

*
* decrunch iff file a0, in struct a1, in memory type d0
*
get_iff:
	link	a5,#-iloc_SIZEOF
	movem.l	d1-d7/a0-a6,-(a7)
	sub.l	#iloc_SIZEOF,a5

	move.l	a0,iloc_filename(a5)
	move.l	a1,iloc_deststruct(a5)
	move.l	d0,iloc_memtype(a5)

	bsr.w	loadfile
	bne	.err

	bsr	unpack
	bne	.err1

	move.l	iloc_deststruct(a5),a0
	move.w	#0,iff_error(a0)
	move.w	iloc_lenx(a5),iff_width(a0)
	move.w	iloc_leny(a5),iff_height(a0)
	move.w	iloc_planes(a5),iff_planes(a0)
	move.w	iloc_mode(a5),iff_mode(a0)
	move.l	iloc_colpointer(a5),iff_colors(a0)
	move.l	iloc_decrunched(a5),iff_image(a0)

	move.l	iloc_packed(a5),a1
	move.l	iloc_packed_size(a5),d0
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)
.exit
	movem.l	(a7)+,d1-d7/a0-a6
	unlk	a5
	rts

.err1:
	tst.l	iloc_colpointer(a5)
	beq	.err
	move.l	iloc_colors(a5),d1
	move.l	d1,d0
	add.l	d0,d0
	add.l	d1,d0
	move.l	iloc_colpointer(a5),a1
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)

.err:
	move.l	iloc_deststruct(a5),a0
	move.w	#1,iff_error(a0)
	bra.b	.exit

loadfile:
	move.l	$4.w,a6
	lea	dosname(pc),a1
	moveq	#0,d0
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,iloc_dosbase(a5)
	
	move.b	#0,iloc_bmhd_found(a5)
	move.b	#0,iloc_body_found(a5)

	clr.l	iloc_packed(a5)
	bsr.w	read_whole_file
	bne	.err

	clr.l	iloc_colpointer(a5)
	bsr.w	check_chunks
	cmp.b	#$ff,iloc_bmhd_found(a5)
	bne.w	.err
	cmp.b	#$ff,iloc_body_found(a5)
	bne.w	.err
	bsr	closedos
	moveq	#0,d0
	rts
.err:
	bsr	closedos
	moveq	#-1,d0
	rts
closedos:
	move.l	iloc_dosbase(a5),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	rts

unpack:	
	tst.l	iloc_decrunched(a5)
	beq	.err
	move.l	iloc_body_start(a5),a0
	move.l	iloc_decrunched(a5),a1

	cmp.b	#1,iloc_compression(a5)
	beq.s	.byte_run

* compression geen 1 dus of 0 of een onbekende 
* copieer van body_start naar unpacked

	move.l	iloc_decrunched_size(a5),d0
	lsr.l	#1,d0
	subq.l	#1,d0
	move.l	iloc_body_start(a5),a0
.copy:	move.w	(a0)+,(a1)+
	subq.l	#1,d0
	bpl.b	.copy
	moveq	#0,d0
	rts

.byte_run:
	move.l	iloc_decrunched(a5),a6
	add.l	iloc_decrunched_size(a5),a6
.un_again:	
	cmp.l	a6,a1
	bge.s	.un_end

	moveq	#0,d5
	move.b	(a0)+,d5
	bmi.b	.un_minus

.un_plus:
	move.b	(a0)+,(a1)+
	dbf	d5,.un_plus
	bra.s	.un_again

.un_minus:
	neg.b	d5
	move.b	(a0)+,d0

.un_rm:	move.b	d0,(a1)+
	dbf	d5,.un_rm

	bra.s	.un_again
.un_end:	
	moveq	#0,d0
	rts
.err:
	moveq	#-1,d0
	rts
*
*
*
check_chunks:
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	iloc_packed(a5),a1
	move.l	4(a1),d0		; file size
	lea	8(a1),a1
	move.l	a1,a2
	addq.l	#4,a1			; skip het ILBM sign
	add.l	d0,a2			; einde data
.cchunks1:
	cmp.l	#"BMHD",(a1)
	bne.w	.no_bmhd_proc
	move.b	#$ff,iloc_bmhd_found(a5)

	lea	8(a1),a6
	move.w	(a6),iloc_lenx(a5)
	move.w	2(a6),iloc_leny(a5)
	moveq	#0,d1
	move.b	8(a6),d1
	move.w	d1,iloc_planes(a5)
	move.b	10(a6),iloc_compression(a5)
	move.b	9(a6),iloc_masking(a5)

	move.l	a1,-(a7)
	bsr.w	unpack_init
	move.l	(a7)+,a1

	bra.w	.continue_chunks

.no_bmhd_proc:
	cmp.l	#"CAMG",(a1)
	bne.b	.no_camg_proc	

	move.w	10(a1),d0
	and.w	#$8efd,d0		; oldstyle mask
	move.w	d0,iloc_mode(a5)

.no_camg_proc:
	cmp.l	#"CMAP",(a1)
	bne.b	.no_cmap_proc

	lea	4(a1),a6


	movem.l	a0/a1,-(a7)

	move.l	(a6)+,d1
	move.w	iloc_planes(a5),d0
	beq	.col_ready
	moveq	#1,d1
	lsl.l	d0,d1
	move.l	d1,iloc_colors(a5)
	move.l	d1,d0
	add.l	d0,d0
	add.l	d1,d0

	move.l	#MEMF_PUBLIC,d1	
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,iloc_colpointer(a5)
	beq	.col_ready

	move.l	iloc_colors(a5),d1
	move.l	d0,a0
.cmap4:
	move.b	(a6)+,(a0)+
	move.b	(a6)+,(a0)+
	move.b	(a6)+,(a0)+
	subq.w	#1,d1
	bne.b	.cmap4

.col_ready:
	movem.l	(a7)+,a0/a1
	bra.b	.continue_chunks

.no_cmap_proc:

	cmp.l	#"BODY",(a1)
	bne.b	.no_body_proc
	move.b	#$ff,iloc_body_found(a5)
	lea	8(a1),a6
	move.l	a6,iloc_body_start(a5)
	bra.w	.continue_chunks

.no_body_proc:
.continue_chunks:
	add.l	4(a1),a1
	move.l	a1,d0
	btst	#0,d0
	beq.b	.no_odd
	addq.l	#1,a1
.no_odd:
	addq.l	#8,a1
	cmp.l	a2,a1
	blt.w	.cchunks1
	move.l	#-1,a1
.cchunks2:
	movem.l	(a7)+,d0-d7/a0-a6
	rts

unpack_init:
	moveq	#0,d0
	move.w	iloc_lenx(a5),d0
	add.l	#15,d0
	lsr.l	#4,d0			; words
	add.l	d0,d0			; bytes
	mulu	iloc_planes(a5),d0
	mulu	iloc_leny(a5),d0
	move.l	d0,iloc_decrunched_size(a5)
	move.l	4,a6
	move.l	iloc_memtype(a5),d1
	jsr	_LVOAllocMem(a6)
	move.l	d0,iloc_decrunched(a5)
	rts

read_whole_file:
	move.l	iloc_filename(a5),d1
	move.l	#mode_old,d2
	move.l	iloc_dosbase(a5),a6
	jsr	_LVOOpen(a6)
	move.l	d0,iloc_filehandle(a5)
	beq.w	.err_readfile

	move.l	d0,d1

	lea.l	iloc_buf(a5),a2
	move.l	a2,d2
	moveq	#8,d3
	move.l	iloc_dosbase(a5),a6
	jsr	_LVORead(a6)
	lea.l	iloc_buf(a5),a1
	moveq	#1,d7
	cmp.l	#'FORM',(a1)+
	bne.b	.err_readfile_open
	move.l	(a1),d0
	add.l	#8,d0
	move.l	d0,iloc_packed_size(a5)

	move.l	#$10000,d1
	move.l	4,a6
	jsr	_LVOAllocMem(a6)

	move.l	d0,iloc_packed(a5)
	beq.b	.err_readfile_open
	move.l	d0,a0

	move.l	iloc_filehandle(a5),d1
	moveq	#0,d2				; seek beginning
	moveq	#-1,d3
	move.l	iloc_dosbase(a5),a6
	jsr	_LVOSeek(a6)		

	move.l	iloc_packed_size(a5),d3
	move.l	iloc_packed(a5),d2
	move.l	iloc_filehandle(a5),d1
	move.l	iloc_dosbase(a5),a6
	jsr	_LVORead(a6)		
	bsr.b	close_file
	moveq	#0,d0
	rts

.err_readfile:
	moveq	#-1,d0
	rts

.err_readfile_open:
	bsr.b	close_file
	bra	.err_readfile

close_file:
	move.l	iloc_filehandle(a5),d1
	move.l	iloc_dosbase(a5),a6
	jsr	_LVOClose(a6)
	rts
dosname:	dc.b	"dos.library",0

