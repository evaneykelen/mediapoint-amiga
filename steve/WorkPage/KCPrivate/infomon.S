	incdir	"wp:inclibs/"
	include	"exec_lib.i"
	include	"graphics_libv39.i"
	incdir	"include:"
	include	"dos/dos.i"
	include	"graphics/view.i"
	include	"graphics/monitor.i"
	include	"graphics/displayinfo.i"

	XREF    KPutFmt
	XREF    KPutStr

FUNC = _LVOLoadView
MON_ID = DBLPAL_MONITOR_ID

start:
	lea	gname(pc),a1
	moveq	#0,d0
	move.l	$4.w,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,gbase
	beq	.err
	move.l	#MON_ID,d0
	move.l	#0,a1
	move.l	gbase(pc),a6
	jsr	_LVOOpenMonitor(a6)
	move.l	d0,monitor
	beq	.nomon
	move.l	d0,a0
	move.l	ms_ratiov(a0),d0
	move.l	ms_ratioh(a0),d1

	move.l	ms_Special(a0),a1
	move.w	spm_vsync(a1),d0
	move.w	spm_vsync+2(a1),d1
	
	move.l	#MON_ID,d2
	move.w	#0,a0
	lea	data(pc),a1
	move.l	#DTAG_MNTR,d1
	move.l	#dis_SIZEOF,d0
	move.l	gbase(pc),a6
	jsr	_LVOGetDisplayInfoData(a6)
	lea	data(pc),a1
	move.w	mtr_TotalRows(a1),d0
	move.w	mtr_MouseTicks+2(a1),d0

	move.l	monitor(pc),a0
	jsr	_LVOCloseMonitor(a6)
.nomon:
	move.l	gbase(pc),a1
	jsr	_LVOCloseLibrary(a6)
.err:
	moveq	#0,d0
	rts

dummy:	rts

gbase:		dc.l	0
monitor:	dc.l	0
gname:	dc.b	"graphics.library",0
	even
data:	blk.b	1000,0
