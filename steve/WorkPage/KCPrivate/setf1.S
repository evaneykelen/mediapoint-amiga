
	INCDIR	wp:inclibs/
	INCLUDE	"exec_lib.i"

	XREF	KPutStr

	XDEF	_install_watch
_install_watch:
	move.l	$4.w,a6
	jsr	_LVOForbid(A6)
	move.l	a6,a1
	move.l	#-198,a0
	move.l	#func,d0
	jsr	_LVOSetFunction(a6)
	move.l	d0,jf+2
	move.l	$4.w,a6
	jsr	_LVOPermit(a6)
	rts

	XDEF	_remove_watch
_remove_watch:
	move.l	$4.w,a6
	jsr	_LVOForbid(A6)
	move.l	a6,a1
	move.l	#-198,a0
	move.l	(jf+2)(pc),d0
	jsr	_LVOSetFunction(a6)
	move.l	d0,func+2
	move.l	$4.w,a6
	jsr	_LVOPermit(a6)
	rts

dbstr1:	dc.b	"a",0

func:
	movem.l	a0-a6/d0-d7,-(a7)

	move.l	$4.w,a6
	jsr	_LVOForbid(A6)

	move.w	#$f00,$dff180		
	lea	dbstr1(pc),a0
	jsr	KPutStr

	move.l	$4.w,a6
	jsr	_LVOPermit(a6)

	movem.l	(a7)+,a0-a6/d0-d7

jf:	jmp	0

