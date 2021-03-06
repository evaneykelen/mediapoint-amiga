	incdir	"wp:inclibs/"
	include	"exec_lib.i"
	include	"graphics_libv39.i"
	incdir	"include:"
	include	"dos/dos.i"
	include	"graphics/view.i"

	XREF    KPutFmt
	XREF    KPutStr

FUNC = _LVOAllocMem

start:
	move.l	$4,a6
	move.l	$4,a1
	move.w	#FUNC,a0
	lea	newfunc(pc),a2
	move.l	a2,d0
	jsr	_LVOSetFunction(a6)
	move.l	d0,oldfunc

;.ww:	btst	#6,$bfe001
;	bne	.ww

	move.l	#SIGBREAKF_CTRL_C,d0
	jsr	_LVOWait(a6)

	move.l	oldfunc,d0
	move.l	$4.w,a6
	move.l	$4.w,a1
	move.w	#FUNC,a0
	jsr	_LVOSetFunction(a6)	
.err:
	moveq	#0,d0
	rts

dummy:	rts

newfunc:
	movem.l	d0-d7/a0-a6,-(a7)
	lea	string(pc),a0
	cmp.l	#48,d0
	bne	.no_p
	jsr     KPutStr
.no_p
	movem.l	(a7)+,d0-d7/a0-a6
	dc.w	$4ef9

oldfunc:	dc.l	dummy

gbase:		dc.l	0
data:		dc.l	0,0,0
gname:	dc.b	"graphics.library",0
string:	dc.b	"48",10,0
