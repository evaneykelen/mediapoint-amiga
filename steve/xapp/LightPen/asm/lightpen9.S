* File	: lightpen.s
* Uses	: lightpen.i
* Date	: 21 june 1994
* Author: ing. C. Lieshout
* Desc	: Functions to monitor the lightpen movements
*	: Freely from Trojan D.K.Phillips

	INCDIR	"include:"
	INCLUDE	"exec/interrupts.i"
	INCLUDE	"hardware/custom.i"
	INCLUDE	"graphics/gfxbase.i"
	include	"graphics/view.i"
	INCDIR  "gen:asm/"
	INCLUDE "lightpen.i"
	INCDIR	"wp:inclibs/"
	INCLUDE	"exec_lib.i"
	include	"graphics_libv39.i"
	
exec = 4

*
* Usage : setlightpen( struct lpmovement );
*

	XDEF	_setlightpen
_setlightpen:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)

	move.l	$4.w,a6
	lea	gfx_name(pc),a1
	moveq.l	#0,d0
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,gfx_base

	bsr	install_old
	bsr	setnewfunc

	move.l	gfx_base(pc),a6
	move.l	gb_ActiView(a6),a1
	jsr	_LVOLoadView(a6)

	move.l	8(a5),a4
	bsr	install_50hz
	bsr	install_TimA
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	rts

gfx_base:	dc.l	0
gfx_name:	dc.b	"graphics.library",0
		even

FUNC = _LVOLoadView

setnewfunc:
	move.l	gfx_base(pc),a1
	move.w	#FUNC,a0
	lea	newfunc(pc),a2
	move.l	a2,d0
	jsr	_LVOSetFunction(a6)
	move.l	d0,oldfunc
	rts

removefunc:
	move.l	$4,a6
	move.l	oldfunc,d0
	move.l	gfx_base(pc),a1
	move.w	#FUNC,a0
	jsr	_LVOSetFunction(a6)	
	moveq	#0,d0
dummy:	rts

newfunc:
	movem.l	d0/a0-a2/a6,-(sp)
	move.l	4.w,a6
	jsr	_LVOForbid(a6)
	move.w	#$100,d1
	move.l	8(a1),a2
	cmp.l	#0,a2
	beq.s	.no_cop1
	move.l	4(a2),a2
	bsr	change_copper
.no_cop1:
	move.l	4(a1),a2
	cmp.l	#0,a2
	beq.s	.no_cop2
	move.l	4(a2),a2
	bsr	change_copper
.no_cop2
	move.l	4.w,a6
	jsr	_LVOPermit(a6)
	movem.l	(sp)+,d0/a0-a2/a6
	dc.w	$4ef9
oldfunc:	dc.l	dummy

change_copper:
	move.w	(a2)+,d0
	cmp.w	d1,d0
	bne.s	.no_con0
	bset	#3,1(a2)
.no_con0:
	addq.l	#2,a2
	cmp.w	#$ffff,d0
	bne.s	change_copper
	rts

install_50hz:
	lea	lp_int_50hz(a4),a1
	move.l	a4,IS_DATA(a1)
	move.b	#2,LN_TYPE(a1)
	move.b	#15,LN_PRI(a1)
	lea	proc_50hz(pc),a0
	move.l	a0,IS_CODE(a1)
	lea	p50hzname(pc),a0
	move.l	a0,LN_NAME(a1)
	moveq	#5,d0
	move.l	$4,a6
	jsr	_LVOAddIntServer(a6)
	rts

install_TimA:
	lea	lp_int_TimA(a4),a1
	move.l	a4,IS_DATA(a1)
	move.b	#2,LN_TYPE(a1)
	move.b	#$64,LN_PRI(a1)
	lea	proc_TimA(pc),a0
	move.l	a0,IS_CODE(a1)
	lea	TimAname(pc),a0
	move.l	a0,LN_NAME(a1)
	moveq	#13,d0
	move.l	$4,a6
	jsr	_LVOAddIntServer(a6)
	rts

proc_TimA:
	movem.l	d1/a1,-(sp)
	lea	test(pc),a1
	move.l	$dff004,(a1)
	move.l	(a1),d1
	cmp.l	$dff004,d1
	beq.s	.gotit
	clr.l	(a1)
.gotit:
	moveq	#0,d0
	movem.l	(sp)+,d1/a1
	rts


	rts
*
* Usage : removelightpen( struct lpmovement );
*
	XDEF	_removelightpen
_removelightpen:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	bsr	remove_old
	bsr	removefunc
	move.l	8(a5),a4

	lea.l	lp_int_50hz(a4),a1
	moveq	#5,d0
	move.l	$4,a6
	jsr	_LVORemIntServer(a6)

	lea.l	lp_int_TimA(a4),a1
	moveq	#13,d0
	move.l	$4,a6
	jsr	_LVORemIntServer(a6)

	move.l	$4.w,a6
	move.l	gfx_base(pc),a1
	jsr	_LVOCloseLibrary(a6)
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	rts

func:
	dc.w	$4e7a,$0801
	rte

remove_old:
	movem.l	a5/a6,-(a7)
	move.l	$4.w,a6
	lea	func(pc),a5
	jsr	_LVOSupervisor(a6)
	move.l	d0,a0
	move.l	old2(pc),$6c(a0)
	movem.l	(a7)+,a5/a6
	rts
	
install_old:
	movem.l	a5/a6,-(a7)
	move.l	$4.w,a6
	lea	func(pc),a5
	jsr	_LVOSupervisor(a6)
	move.l	d0,a0
	move.l	$6c(a0),d0
	move.l	d0,old2
	lea	proc2(pc),a1
	move.l	a1,$6c(a0)
	movem.l	(a7)+,a5/a6
	rts

proc2:
	btst	#5,$dff01f
	beq.s	.no
	move.b	#$7f,$bfdd00
	move.b	#$81,$bfdd00

	move.b	#$36,$bfd500		; $35
	move.b	#$6b,$bfd400		; $1f
	move.b	#$89,$bfde00
.no:	dc.w	$4ef9
old2:	dc.l	dummy

test:	dc.l	0

proc_50hz:
	lea	$dff000,a0
	move.l	test(pc),d0
	beq	.notrig
	move.l	d0,d1

	and.l	#$0001ff00,d1
	lsr.l	#8,d1
	cmp.w	#411,d1
	bgt	.notrig

	and.l	#$ff,d0
	add.w	d0,d0

	cmp.w	#$88,d0
	bge	.ysub
	cmp.w	#$40,d0
	bcc	.nos
	add.w	#$13e+$88,d0
.ysub:
	sub.w	#$88,d0
.nos:
	add.w	d0,d0
	sub.w	lp_offset_x(a1),d0
	bpl	.okie
	moveq	#0,d0
.okie:

	sub.w	#41,d1
	add.w	d1,d1
	sub.w	lp_offset_y(a1),d1
	bpl	.okie2
	moveq	#0,d1
.okie2:
	add.l	lp_oldy(a1),d1
	lsr.l	#1,d1
	cmp.l	lp_oldy(a1),d1
	bne	.sigme

	add.l	lp_oldx(a1),d0
	lsr.l	#1,d0
	cmp.l	lp_oldx(a1),d0
	beq	.notrig

.sigme:
	move.l	d0,lp_x(a1)
	move.l	d1,lp_y(a1)	

	move.l	d0,lp_oldx(a1)
	move.l	d1,lp_oldy(a1)	

	move.l	$4.w,a6
	move.l	lp_signal(a1),d0
	move.l	lp_task(a1),a1
	jsr	_LVOSignal(a6)
.notrig:
	moveq	#0,d0
	rts

p50hzname:	dc.b	"50Hz LP-interupt",0
TimAname:	dc.b	"Cia-interupt",0
	even

