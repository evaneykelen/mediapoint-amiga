*
* File	: artgraph.s
* Uses	: art_data.i
* Date	: 1 february 1993 
* Author: ing. C. Lieshout
* Desc	: Try to display some art idea
*

	INCDIR "include:"
	INCLUDE "exec/types.i"
	INCLUDE "exec/execbase.i"
	INCLUDE "exec/funcdef.i"
	INCLUDE	"exec/memory.i"
	INCLUDE "graphics/gfxbase.i"

	INCLUDE	"hardware/custom.i"
	INCLUDE	"hardware/blit.i"
	INCLUDE	"hardware/dmabits.i"
	
	INCLUDE "graphics/view.i"
	INCLUDE "graphics/gfx.i"
	INCLUDE "graphics/rastport.i"
	INCLUDE	"graphics/text.i"
	INCLUDE	"graphics/videocontrol.i"
	INCLUDE "intuition/intuitionbase.i"
        INCLUDE "intuition/preferences.i"
        INCLUDE	"libraries/dos.i"

	INCDIR	"wp:inclibs/"
	INCLUDE	"diskfont_lib.i"
	INCLUDE	"intuition_lib.i"
	INCLUDE	"dos_lib.i"
	INCLUDE "exec_lib.i"
	INCLUDE "graphics_libv39.i"

	INCDIR	"art:asm/"
	INCLUDE	"art_data.i"
	INCDIR	"wp:asm/"
	INCLUDE "proces.i"
	INCLUDE	"system.i"

MON_ID = DEFAULT_MONITOR_ID!HIRESLACE_KEY
;MON_ID = PAL_MONITOR_ID!HIRESLACE_KEY

DEBUG = 0
; Default values when there is no viewport

DEF_MODE = V_LACE+V_HIRES
DEF_PLANES = 1
REP_LINES = 0

BOUND = 10
NLINES = 20
HO = 580
BR = 738
LIBV39 = 39

POINT_MEM_SIZE = 1000		; Space for 60 points ( 2*(p*8) + 4 = 1000 )

	IFNE	DEBUG
	XREF	KPutFmt
	XREF	KPutStr
	PRINTT	"De debug info staat aan dus je moet nog met debug.lib linken"
	ENDC

start:
	bsr	get_prefs
	rts
	even

get_prefs:
	move.b	#0,Xoff
	move.b	#0,Yoff
	lea	intname(pc),a1
	moveq	#0,d0
	move.l	$4,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,intbase
	beq	no_int
	move.l	d0,a6

	move.l	intbase(pc),a6
	move.l	#PAL_MONITOR_ID+HIRES_KEY,a0
	move.l	#3,d0				//OSCAN_MAX
	lea	qu_data(pc),a1
	jsr	_LVOQueryOverscan(a6)

	lea	qu_data(pc),a1

	IFNE DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
	lea	ra_MinX(a1)
	lea	dbstr1(pc),a0
	jsr	KPutStr
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	
	move.w	ra_MinY(a1),d0		; adjust this to a minimum ?????
	move.w	ra_MinX(a1),d0

	move.l	intbase(pc),a1
	move.l	$4,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	intbase
no_int:
	rts

intname:	dc.b	'intuition.library',0
	even
Xoff:	dc.w	0
Yoff:	dc.w	0
intbase:	dc.l	0
qu_data:	blk.l	400,0
	END
