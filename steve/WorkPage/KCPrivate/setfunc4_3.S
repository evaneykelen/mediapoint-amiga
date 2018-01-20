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
	movem.l	d0-d1/a0-a1,-(a7)

;	cmp.l	#112,d0
;	bge	.no_p

	bsr	check_task
	beq	.no_p

	lea	string(pc),a0
	lea	data(pc),a1
	move.l	d0,data
	jsr     KPutFmt
.no_p
	movem.l	(a7)+,d0-d1/a0-a1
	dc.w	$4ef9

oldfunc:	dc.l	dummy

*
* Am I the task worth watching ?
*
check_task:
	movem.l	d0/d1/a0/a1,-(a7)
	move.w	#0,a1
	jsr	_LVOFindTask(a6)
	tst.l	d0
	beq	.no
	move.l	d0,a0
	move.l	LN_NAME(a0),a0
	cmp.l	#'medi',(a0)
	beq	.do
.no:
	moveq	#0,d0	
	movem.l	(a7)+,d0/d1/a0/a1
	rts

.do:
	moveq	#1,d0
	movem.l	(a7)+,d0/d1/a0/a1
	rts

gbase:		dc.l	0
data:		dc.l	0,0,0
gname:	dc.b	"graphics.library",0
string:	dc.b	"a-%ld",10,0
