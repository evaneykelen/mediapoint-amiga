free_memory_view:
	move.l		db_graphbase(a3),a6
	move.l		vb_viewportw(a5),a0
	jsr		_LVOGfxLookUp(a6)
	move.l		d0,vb_vpextra(a5)
	move.l		vb_vieww(a5),a0
	jsr		_LVOGfxLookUp(a6)
	move.l		d0,vb_vextra(a5)

	move.l		vb_viewportw(a5),a0
	tst.l		vp_ColorMap(a0)
	beq.w		no_colormem

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
	tst.l	vb_vpextra(a5)
	beq	no_vp
	move.l	vb_vpextra(a5),a0

	IFNE GFXSNOOP
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.temp2(pc),a1
	move.l	a0,(a1)
	lea	.dbstr2(pc),a0
	jsr	KPutFmt
	bra	.tt2
.dbstr2:	dc.b	"free1 %d,%d",10,0
	even
.temp2:	dc.l	0
.tt2:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.l	db_graphbase(a3),a6
	jsr	_LVOGfxFree(a6)
	clr.l	vb_vpextra(a5)
no_vp:
	tst.l	vb_vextra(a5)
	beq	no_v
	move.l	vb_vextra(a5),a0

	IFNE GFXSNOOP
	movem.l	a0-a6/d0-d7,-(a7)
	lea	.temp2(pc),a1
	move.l	a0,(a1)
	lea	.dbstr2(pc),a0
	jsr	KPutFmt
	bra	.tt2
.dbstr2:	dc.b	"free2 %d,%d",10,0
	even
.temp2:	dc.l	0
.tt2:
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC
	move.l	db_graphbase(a3),a6
	jsr	_LVOGfxFree(a6)
	clr.l	vb_vextra(a5)
no_v:
	moveq		#0,d0
	rts
