* File	: soundplay.s
* Uses	: soundstructone.i
* Date	: 2 february 1993 ( updated 7 - 07 -1993 )
* Author: ing. C. Lieshout
* Desc	: Try to read samples from disk
*	: This program needs to be compiled with psamp.c

	INCDIR	"include:"
	INCLUDE	"exec/interrupts.i"
	INCLUDE	"hardware/custom.i"
	INCDIR	"sample:oneshot/asm/"
	INCLUDE	"soundstructone.i"
	INCDIR	"wp:inclibs/"
	INCLUDE	"exec_lib.i"
	
	
exec = 4

DEBUG = 0

	IF	DEBUG
	XREF	KPutFmt
	XREF	KPutStr
	PRINTT	"De debug info staat aan dus je moet nog met debug.lib linken"
	ENDC

	xdef	_ddif
_ddif:
	divu	d1,d0
	rts
	
	xdef	_play_sound
_play_sound:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	lea	$dff000,a1
	move.l	8(a5),a0

	cmp.w	#2,si_channel(a0)
	beq	chan2

	move.w	#$0003,dmacon(a1)
	move.l	si_sounddata(a0),a2
	move.l	a2,aud0(a1)
	move.l	a2,aud1(a1)
	move.b	si_type(a0),d0
	and.b	#SI_STEREO,d0
	beq	no_stereo
	move.l	si_soundlength(a0),d0
	add.l	d0,a2
	move.l	a2,aud1(a1)
no_stereo:

	move.w	#64,aud0+ac_vol(a1)
	move.w	#64,aud1+ac_vol(a1)

	move.l	si_soundlength(a0),d0		; SIZE
	lsr.l	#1,d0
	move.w	d0,aud0+ac_len(a1)
	move.w	d0,aud1+ac_len(a1)

	move.w	si_period(a0),aud0+ac_per(a1)
	move.w	si_period(a0),aud1+ac_per(a1)

	move.w	#$00ff,adkcon(a5)

	lea	si_int_audio(a0),a1
	move.l	a0,IS_DATA(a1)

	bsr.w	install_audio_0

	move.w	#$0080,$dff09c
	move.w	#$8080,$dff09a
	move.w	#$0080,$dff09c
	move.w	#$8203,$dff096

	bset	#1,$bfe001
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	rts

chan2:
	move.w	#$000c,dmacon(a1)
	move.l	si_sounddata(a0),a2
	move.l	a2,aud2(a1)
	move.l	a2,aud3(a1)
	move.b	si_type(a0),d0
	and.b	#SI_STEREO,d0
	beq	no_stereo_c2
	move.l	si_soundlength(a0),d0
	add.l	d0,a2
	move.l	a2,aud1(a1)
no_stereo_c2:
	move.w	#64,aud2+ac_vol(a1)
	move.w	#64,aud3+ac_vol(a1)
	move.l	si_soundlength(a0),d0		; SIZE
	lsr.l	#1,d0
	move.w	d0,aud2+ac_len(a1)
	move.w	d0,aud3+ac_len(a1)

	move.w	si_period(a0),aud2+ac_per(a1)
	move.w	si_period(a0),aud3+ac_per(a1)

	move.w	#$00ff,adkcon(a5)

	bsr.w	install_audio_2

	move.w	#$8200,$dff09a
	move.w	#$0200,$dff09c
	move.w	#$820c,$dff096

	bset	#1,$bfe001
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	rts

	xdef	_exit_sound
_exit_sound:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a0
	lea	$dff000,a5
	cmp.w	#2,si_channel(a0)
	beq	exit_c2
	move.w	#0,aud0+ac_vol(a5)
	move.w	#0,aud1+ac_vol(a5)
	move.w	#$0080,$dff09a
	bsr	remove_audio_0
	move.w	#$0003,dmacon(a5)
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	rts

exit_c2:
	move.w	#0,aud2+ac_vol(a5)
	move.w	#0,aud3+ac_vol(a5)
	move.w	#$0200,$dff09a
	bsr	remove_audio_2
	move.w	#$000c,dmacon(a5)
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	rts

send_signal:
	movem.l	a0-a2/a6/d0-d2,-(a7)
	move.l	exec,a6
	move.l	si_audiosig(a1),d0
	move.l	si_task(a1),a1
	jsr	_LVOSignal(a6)
	movem.l	(a7)+,a0-a2/a6/d0-d2
	rts

install_audio_0:
	move.l	a0,-(a7)
	lea	si_int_audio(a0),a1
	move.l	#0,LN_SUCC(a1)
	move.l	#0,LN_PRED(a1)
	move.l	a0,IS_DATA(a1)
	move.b	#2,LN_TYPE(a1)
	move.b	#100,LN_PRI(a1)
	lea	proc_audio_0(pc),a0
	move.l	a0,IS_CODE(a1)
	lea	proc0name(pc),a0
	move.l	a0,LN_NAME(a1)
	
	moveq	#7,d0
	move.l	$4,a6
	jsr	_LVOSetIntVector(a6)
	move.l	(a7)+,a0
	move.l	d0,si_oldaudio(a0)
	rts

remove_audio_0:
	moveq	#7,d0
	move.l	si_oldaudio(a0),a1
	move.l	exec,a6
	jsr	_LVOSetIntVector(a6)
	rts

proc_audio_0:
	move.w	#$0080,$dff09c
	bsr	send_signal
	move.w	#$f00,$dff180
	rts

install_audio_2:
	move.l	a0,-(a7)
	lea	si_int_audio(a0),a1
	move.l	a0,IS_DATA(a1)
	move.b	#2,LN_TYPE(a1)
	move.b	#100,LN_PRI(a1)
	lea	proc_audio_2(pc),a0
	move.l	a0,IS_CODE(a1)
	lea	proc2name(pc),a0
	move.l	a0,LN_NAME(a1)
	moveq	#9,d0
	move.l	$4,a6
	jsr	_LVOSetIntVector(a6)
	move.l	(a7)+,a0
	move.l	d0,si_oldaudio(a0)
	rts

remove_audio_2:
	moveq	#9,d0
	move.l	si_oldaudio(a0),a1
	move.l	exec,a6
	jsr	_LVOSetIntVector(a6)
	rts

proc_audio_2:
	move.w	#$0200,$dff09c
	bsr	send_signal
	rts

proc0name:	dc.b	"Audio0 interupt",0
proc2name:	dc.b	"Audio2 interupt",0
	even
