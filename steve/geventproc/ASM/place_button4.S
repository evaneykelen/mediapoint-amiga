DEBUG = 0

	IFNE	DEBUG
	XREF	KPutStr
	XREF	KPutFmt
	ENDC
	
	INCDIR "include:"
	INCLUDE "exec/execbase.i"
	INCLUDE "exec/funcdef.i"
	INCLUDE "exec/exec_lib.i"
	INCLUDE "graphics/gfxbase.i"
	INCLUDE "graphics/view.i"
	INCLUDE "graphics/rastport.i"
	INCLUDE "graphics/gfx.i"

	INCDIR	"wp:asm/"
	INCLUDE	"system.i"

	INCDIR "wp:inclibs/"
	INCLUDE "graphics_libv39.i"

	RSRESET
but_block:	rs.w	0
but_type:	rs.w	1
but_x:		rs.w	1
but_y:		rs.w	1
but_width:	rs.w	1
but_height:	rs.w	1
but_color:	rs.w	1
but_linesize:	rs.w	1
but_delay:	rs.w	1
but_blinks:	rs.w	1
but_stay:	rs.w	1
but_SIZEOF:	rs.w	0


	RSRESET
pb_block:	rs.w	0
pb_arguments:	rs.l	1
pb_graphbase:	rs.l	1
pb_mlsystem:	rs.l	1
pb_active_view:	rs.l	1
pb_SIZEOF:	rs.w	0

pd_data:	ds.b	pb_SIZEOF
win_rp:		ds.b	rp_SIZEOF
gfxname:	dc.b	"graphics.library",0
	even

*
* place_button( WORD *arguments, struct MLSYSTEM *mlsystem )
*
	XDEF	_place_button
_place_button:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)

	lea	pd_data(pc),a3

	move.l	8(a5),a1		; pointer to arguments
	move.l	a1,pb_arguments(a3)

	move.l	12(a5),a1
	move.l	a1,pb_mlsystem(a3)
	
	move.l	$4.w,a6
	lea	gfxname(pc),a1
	moveq	#0,d0
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,pb_graphbase(a3)
	beq	.gfx_err

	bsr	place_button
	
	move.l	pb_graphbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
.gfx_err:
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0			; succes
	rts
	IFNE	DEBUG
dbstr1:	dc.b	"view 1",10,0
dbstr2:	dc.b	"Bit is %d,%d",10,0
	even
	ENDC
*
* Get the active view from the mlsystem
*
get_active_view:
	moveq	#2,d0				; number off bufs - 1
	move.l	pb_mlsystem(a3),a0
.get_ac:
	tst.w	sbu_Viewed(a0)
	bne	.found_ac
	lea.l	sbu_SIZEOF(a0),a0
	dbf	d0,.get_ac
	move.l	pb_mlsystem(a3),a0		; no active take the first
.found_ac:
	move.l	a0,pb_active_view(a3)
	rts

place_button:
	bsr	get_active_view
	
	move.l	pb_graphbase(a3),a6
	lea	win_rp(pc),a1
	jsr	_LVOInitRastPort(a6)

	move.l	pb_active_view(a3),a0

	lea.l	sbu_Display+dp_BitMap(a0),a0

	lea	win_rp(pc),a1
	move.l	a0,rp_BitMap(a1)	


* The rastport is ready so now we can do some drawing

	move.l	pb_graphbase(a3),a6
	jsr	_LVOWaitBlit(a6)

	
* use the arguments to draw some lines in the active bitmap

	move.l	pb_arguments(a3),a2

	move.w	but_color(a2),d0
	lea	win_rp(pc),a1
	move.l	pb_graphbase(a3),a6
	jsr	_LVOSetAPen(a6)		; set color

	lea	win_rp(pc),a1
	move.l	pb_graphbase(a3),a6
	move.b	#RP_JAM2!RP_COMPLEMENT,d0
	jsr	_LVOSetDrMd(a6)		; set color

	move.l	pb_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)

	move.l	pb_arguments(a3),a2
	move.w	but_blinks(a2),d7
	add.w	d7,d7		

	tst.w	but_stay(a2)
	beq	rep_blink_but

	add.w	#1,d7			; always uneven sure to remove

rep_blink_but:
	move.w	d7,-(a7)
	move.w	but_type(a2),d6
	and.w	#$2,d6			; Invert
	bne	place_button3

	move.w	but_type(a2),d6
	and.w	#$4,d6			; Normal box
	bne	place_button1

	move.w	but_type(a2),d6
	and.w	#$8,d6			; Normal box
	bne	place_button2
end_place_button:
	move.w	(a7)+,d7
	tst.w	d7
	beq	no_more_blink_but

	move.l	pb_graphbase(a3),a6
	move.w	but_delay(a2),d6
rep_but_del:
	jsr	_LVOWaitTOF(a6)
	dbf	d6,rep_but_del

	subq.w	#1,d7
	bpl	rep_blink_but

no_more_blink_but:
	rts

*
* Place a button using a single outline
*
place_button1:
	move.w	#2,but_linesize(a2)
*
* Us a linesize thick outline
*
place_button2:
	move.w	but_width(a2),d6
	cmp.w	but_height(a2),d6
	bge	no_c_but
	move.w	but_height(a2),d6
no_c_but:
	move.w	but_linesize(a2),d7

	addq.w	#1,d7
	add.w	d7,d7
	cmp.w	d7,d6
	bge	oke_but_size
	subq.w	#2,d6
	lsr.w	#1,d6
	move.w	d6,but_linesize(a2)
oke_but_size:
	move.w	but_x(a2),d2
	move.w	but_y(a2),d3
	move.w	but_width(a2),d4
	move.w	but_height(a2),d5
	subq.w	#1,d4
	subq.w	#1,d5
	bsr	draw_box

	move.w	but_linesize(a2),d7
	subq.w	#2,d7
rep_tlin:
	addq.w	#1,d2
	addq.w	#1,d3
	subq.w	#2,d4
	subq.w	#2,d5
	bsr	draw_box
	subq.w	#1,d7
	bpl	rep_tlin
	bra 	end_place_button
*
* Us a filled rectangle
*
place_button3:
	IFNE 0
	lea	win_rp(pc),a1
	move.l	rp_BitMap(a1),a0
	moveq	#0,d0
	move.b	bm_Depth(a0),d0
	move.l	d0,d4
	move.l	bm_Planes+4(a0),d1
	bne	.oke
	moveq	#0,d1
	move.w	bm_BytesPerRow(a0),d1
	bra	.skip
.oke:
	sub.l	bm_Planes(a0),d1
.skip:
	move.l	d1,d3				; modulo value one line
	mulu	d0,d1				; total width 1 line
	move.l	d1,d2
	mulu	but_height(a2),d4		; total lines to XOR

	mulu	but_y(a2),d2			; line offset
	moveq	#0,d0
	move.w	but_x(a2),d0
	lsr.l	#4,d0				; words
	add.l	d0,d0				; bytes pointer
	move.l	bm_Planes(a0),a0
	add.l	d0,a0				; add x offset
	add.l	d2,a0				; add y offset

	move.w	but_width(a2),d7
	lsr.w	#5,d7				; nr off words
	subq.w	#1,d7
	move.l	a0,a1
	moveq	#-1,d0
.repw:
	move.l	d4,d6
	move.l	a1,a0
.repl:
	eor.l	d0,(a0)
	add.l	d3,a0
	dbf	d6,.repl
	addq.l	#4,a1
	dbf	d7,.repw

	bra 	end_place_button
	ENDC

	move.w	but_x(a2),d0
	move.w	but_y(a2),d1
	move.w	d0,d2
	move.w	d1,d3
	add.w	but_width(a2),d2
	add.w	but_height(a2),d3
	subq.w	#1,d2
	subq.w	#1,d3
	lea	win_rp(pc),a1
	move.l	pb_graphbase(a3),a6
	jsr	_LVORectFill(a6)

	bra 	end_place_button

*
* Draw a box with coordinates x,y -> d2,d3  widht,height -> d4,d5
*
draw_box:
	move.l	pb_graphbase(a3),a6
	move.w	d2,d0
	move.w	d3,d1
	lea	win_rp(pc),a1
	jsr	_LVOMove(a6)
	move.w	d2,d0
	move.w	d3,d1
	add.w	d4,d0
	subq.w	#1,d0
	lea	win_rp(pc),a1
	jsr	_LVODraw(a6)	; line to the right ->

	move.w	d2,d0
	move.w	d3,d1
	add.w	d4,d0
	lea	win_rp(pc),a1
	jsr	_LVOMove(a6)

	move.w	d2,d0
	move.w	d3,d1
	add.w	d4,d0
	add.w	d5,d1
	subq.w	#1,d1
	lea	win_rp(pc),a1
	jsr	_LVODraw(a6)	; line down |

	move.w	d2,d0
	move.w	d3,d1
	add.w	d4,d0
	add.w	d5,d1
	lea	win_rp(pc),a1
	jsr	_LVOMove(a6)

	move.w	d2,d0
	move.w	d3,d1
	add.w	d5,d1
	addq.w	#1,d0
	lea	win_rp(pc),a1
	jsr	_LVODraw(a6)	; line to the left <-

	move.w	d2,d0
	move.w	d3,d1
	add.w	d5,d1
	lea	win_rp(pc),a1
	jsr	_LVOMove(a6)

	move.w	d2,d0
	move.w	d3,d1
	lea	win_rp(pc),a1
	addq.w	#1,d1
	jsr	_LVODraw(a6)	; line to the top |
	rts
