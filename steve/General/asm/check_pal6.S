*
* File	: checkpal.s
* Uses	:
* Date	: 29 june 1993 
* Author: ing. C. Lieshout
* Desc	: Try to find out wether the system is NTSC or PAL
*	: From the amiga mail page V - 10 volume II ( C-version )

OWN = 0

	IF OWN
	INCDIR	"include:"
	include	"exec/types.i"
	include	"exec/funcdef.i"
	include	"exec/exec_lib.i"
	include	"graphics/gfxbase.i"
	INCDIR	"wp:inclibs/"
	include	"graphics_libv39.i"
	include	"intuition_lib.i"
	ENDC

	XDEF	_check_pal
*
* Needs Gfx-base in a6, intbase in a5
*
check_pal:
_check_pal:
	moveq	#0,d7
	if OWN
	moveq	#0,d0
	lea	gname(pc),a1
	move.l	$4,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,a5

	moveq	#0,d0
	lea	intname(pc),a1
	move.l	$4,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,a6
	exg	a5,a6
	ENDC

	cmp.l	#0,a6
	beq	.no_graph_base
	cmp.l	#0,a5
	beq	.no_intbase
	
	cmp.w  #36,LIB_VERSION(a6)
        blt	.no_v36
	lea	pubname(pc),a0
	exg	a5,a6
	jsr	_LVOLockPubScreen(a6)
	exg	a5,a6
	move.l	d0,d7
	beq	.no_v36
	move.l	d7,a0
	lea	44(a0),a0			; sc_ViewPort(a0),a0
	jsr	_LVOGetVPModeID(a6)
	move.l	d0,d6				; modeid	
	cmp.l	#INVALID_ID,d0
	beq	.no_v36
	and.l	#MONITOR_ID_MASK,d0
	cmp.l	#NTSC_MONITOR_ID,d0
	beq	.oke_m
	cmp.l	#PAL_MONITOR_ID,d0
	beq	.oke_m	
	move.l	#LORES_KEY,d6			; set mode at lores
.oke_m:

	move.l	#0,a0
	lea	dimdat(pc),a1
	move.l	#dis_Resolution,d0
	move.l	#DTAG_DISP,d1
	move.l	d6,d2
	jsr	_LVOGetDisplayInfoData(a6)

	lea	dimdat(pc),a1
	moveq	#0,d0
	move.l	dis_PropertyFlags(a1),d1
	and.l	#DIPF_IS_PAL,d1
	beq	.no_pal
	bra	.pal_found
.no_v36:
	moveq	#0,d0
	move.w	gb_DisplayFlags(a6),d0
	and.w	#PAL,d0
	beq	.no_pal
.pal_found:
	moveq	#1,d0
.no_pal:
	move.l	d0,-(a7)
	tst.l	d7
	beq	.no_psc
	move.l	d7,a1
	lea	pubname(pc),a0
	exg	a5,a6
	jsr	_LVOUnlockPubScreen(a6)
	exg	a5,a6
.no_psc:
	move.l	(a7)+,d0
	
.no_intbase:
	IF OWN
	move.l	d0,-(a7)
	move.l	a5,a1
	move.l	$4,a6
	jsr	_LVOCloseLibrary(a6)
	move.l	(a7)+,d0

	move.l	d0,-(a7)
	move.l	a6,a1
	move.l	$4,a6
	jsr	_LVOCloseLibrary(a6)
	move.l	(a7)+,d0
	ENDC

	rts

.no_graph_base:
	moveq	#-1,d0
	rts

dimdat:	blk.b	dis_Resolution,0
	IF OWN
gname:	dc.b	"graphics.library",0
intname:	dc.b	"intuition.library",0
	ENDC
pubname:	dc.b	"Workbench",0
