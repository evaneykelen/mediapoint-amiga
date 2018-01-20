* File	: wait50hz.s
* Uses	: wait50hz.i
* Date	: 18 july 1993
* Author: ing. C. Lieshout
* Desc	: Functions to install a 50hz interrupt

	INCDIR	"include:"
	INCLUDE	"exec/interrupts.i"
	INCLUDE	"hardware/custom.i"

	INCDIR  "cprogs:serial/asm/"
	INCLUDE "waitsec.i"
	INCDIR	"wp:inclibs/"
	INCLUDE	"exec_lib.i"
	
exec = 4

*
* Usage : set50hz( struct wjif *wj, long value );
*

	XDEF	_install50hz
_install50hz:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a4
	move.l	12(a5),hz_endval(a4)
	clr.l	hz_val(a4)

	move.l	4,a6
	sub.l	a1,a1
	jsr	_LVOFindTask(a6)
	move.l	d0,hz_task(a4)
	bsr	install_50hz
already_inst:
	movem.l	(a7)+,d0-d7/a0-a6
	moveq	#1,d0
	unlk	a5
	rts

hz50_fail:
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

install_50hz:
	lea	hz_int_50hz(a4),a1
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

	XDEF	_set50hz
_set50hz:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a4
	move.l	12(a5),hz_endval(a4)
	clr.l	hz_val(a4)
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	rts

*
* Usage : remove50hz( struct wjif *wj );
*
	XDEF	_remove50hz
_remove50hz:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a4
	lea.l	hz_int_50hz(a4),a1
	moveq	#5,d0
	move.l	$4,a6
	jsr	_LVORemIntServer(a6)
	move.l	hz_signum(a4),d0
no_50hz:
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	rts

proc_50hz:
	tst.l	hz_on(a1)
	beq	p_no_finish
	add.l	#1,hz_val(a1)
	move.l	hz_val(a1),d0
	cmp.l	hz_endval(a1),d0
	blt	p_no_finish
	clr.l	hz_val(a1)
	move.l	4,a6
	move.l	hz_signal(a1),d0
	move.l	hz_task(a1),a1
	jsr	_LVOSignal(a6)	// signal the task that you are ready
p_no_finish:
	moveq	#0,d0
	rts

p50hzname:	dc.b	"50Hz modem timeout",0
	even

