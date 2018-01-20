
	incdir	"wp:inclibs/"
	include	"exec_lib.i"
	include	"mpmmu_lib.i"

start:
	lea	mpname(pc),a1
	moveq	#0,d0
	move.l	$4.w,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,mpbase
	lea	name(pc),a1
	move.l	d0,a6
	moveq	#0,d0
	jsr	_LVOMLMMU_FindMemBlk(a6)

	move.l	mpbase(pc),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	moveq	#0,d0
	rts

mpname:	dc.b	"nb:system/mpmmu.library",0

name:	dc.b	"hd3:data/pictures/8bit/3493picr",0
	even

mpbase:	dc.l	0

