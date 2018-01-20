	incdir	"wp:inclibs/"
	include	"exec_lib.i"
	include	"graphics_libv39.i"
	incdir	"include:"
	include	"dos/dos.i"
	include	"graphics/view.i"

	XREF    KPutFmt
	XREF    KPutStr

FUNC = _LVOLoadView

start:
	lea	gname(pc),a1
	moveq	#0,d0
	move.l	$4.w,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,gbase
	beq	.err
	move.l	d0,a1
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
	move.l	gbase(pc),a1
	move.w	#FUNC,a0
	jsr	_LVOSetFunction(a6)	

	move.l	gbase(pc),a1
	jsr	_LVOCloseLibrary(a6)
.err:
	moveq	#0,d0
	rts

dummy:	rts

newfunc:
	movem.l	d0-d7/a0-a6,-(a7)
	move.l  a1,data
	move.l	v_ViewPort(a1),a1
	move.l	a1,data+4
	move.l	vp_RasInfo(a1),a1
	move.l	ri_BitMap(a1),a1
	moveq	#0,d0
	move.b	bm_Depth(a1),d0
	move.w	d0,data+8
	lea     data(pc),a1
	lea     string(pc),a0
	jsr     KPutFmt
	movem.l	(a7)+,d0-d7/a0-a6
	dc.w	$4ef9

oldfunc:	dc.l	dummy

gbase:		dc.l	0
data:		dc.l	0,0,0
gname:	dc.b	"graphics.library",0
string:	dc.b	"loadview %lx,%lx,%d",10,0
