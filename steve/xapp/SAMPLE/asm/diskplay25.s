* File	: diskplay.s
* Uses	: disk_data.i
* Date	: 2 february 1993 ( updated 7 - 07 -1993 )
* Author: ing. C. Lieshout
* Desc	: Try to read samples from disk
*	: This program needs to be compiled with psamp.c

	INCDIR	"include:"
	INCLUDE	"exec/interrupts.i"
	INCLUDE	"hardware/custom.i"
	INCDIR	"sample:asm/"
;	INCLUDE	"disk_data.i"
	INCLUDE	"soundstruct.i"
	INCDIR	"wp:inclibs/"
	INCLUDE	"exec_lib.i"
	
exec = 4

DEBUG = 0

	IF	DEBUG
	XREF	KPutFmt
	XREF	KPutStr
	PRINTT	"De debug info staat aan dus je moet nog met debug.lib linken"
	ENDC

	xdef	_change_sound
_change_sound:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	lea	$dff000,a1
	move.l	8(a5),a0
	cmp.w	#2,si_channel(a0)
	beq	chan2c
	move.w	si_vol_right(a0),aud0+ac_vol(a1)
	move.w	si_vol_left(a0),aud1+ac_vol(a1)
	move.w	si_period(a0),aud0+ac_per(a1)
	move.w	si_period(a0),aud1+ac_per(a1)
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	rts
chan2c:
	move.w	si_vol_left(a0),aud2+ac_vol(a1)
	move.w	si_vol_right(a0),aud3+ac_vol(a1)
	move.w	si_period(a0),aud2+ac_per(a1)
	move.w	si_period(a0),aud3+ac_per(a1)
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
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
	lea	si_chipdat(a0),a2
	move.l	cd_chip2(a2),aud0(a1)
	move.l	cd_chip2(a2),aud1(a1)
	move.b	si_type(a0),d0
	and.b	#SI_STEREO,d0
	beq	no_stereo
	move.l	cd_chipS2(a2),aud1(a1)
no_stereo:

	move.w	si_vol_right(a0),aud0+ac_vol(a1)
	move.w	si_vol_left(a0),aud1+ac_vol(a1)

	move.l	si_chipsize(a0),d0		; SIZE
	lsr.w	#1,d0
	move.w	d0,aud0+ac_len(a1)
	move.w	d0,aud1+ac_len(a1)

	move.w	si_period(a0),aud0+ac_per(a1)
	move.w	si_period(a0),aud1+ac_per(a1)

	move.w	#$00ff,adkcon(a5)

	lea	si_int_audio(a0),a1
	move.l	a0,IS_DATA(a1)

	bsr.w	install_audio_0

	move.w	#$8080,$dff09a
	move.w	#$0080,$dff09c
	move.w	#$8203,$dff096

	bset	#1,$bfe001
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	rts

chan2:
	move.w	#$000c,dmacon(a1)
	lea	si_chipdat(a0),a2
	move.l	cd_chip2(a2),aud2(a1)
	move.l	cd_chip2(a2),aud3(a1)
	move.b	si_type(a0),d0
	and.b	#SI_STEREO,d0
	beq	no_stereo_c2
	move.l	cd_chipS2(a2),aud1(a1)
no_stereo_c2:
	move.w	si_vol_left(a0),aud2+ac_vol(a1)
	move.w	si_vol_right(a0),aud3+ac_vol(a1)
	move.l	si_chipsize(a0),d0		; SIZE
	lsr.w	#1,d0
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
	tst.w	si_sigtest(a1)
	beq	.no_sig
	movem.l	a0-a2/a6/d0-d2,-(a7)
	move.l	exec,a6
	move.l	si_audiosig(a1),d0
	move.l	si_task(a1),a1
	jsr	_LVOSignal(a6)
	movem.l	(a7)+,a0-a2/a6/d0-d2
.no_sig:
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

	lea	si_chipdat(a1),a0

	move.l	cd_chip1(a0),$dff0a0	; zet audio start naar andere buffer
	move.l	cd_chip1(a0),$dff0b0

	move.l	cd_chip1(a0),d0
	move.l	cd_chip2(a0),cd_chip1(a0)
	move.l	d0,cd_chip2(a0)

	move.b	si_type(a1),d0
	and.b	#SI_STEREO,d0
	beq	no_stereo_samp0
	move.l	cd_chipS1(a0),$dff0b0
	move.l	cd_chipS1(a0),d0
	move.l	cd_chipS2(a0),cd_chipS1(a0)
	move.l	d0,cd_chipS2(a0)
no_stereo_samp0:
	bsr	send_signal
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

	lea	si_chipdat(a1),a0

	move.l	cd_chip1(a0),$dff0c0	; zet audio start naar andere buffer
	move.l	cd_chip1(a0),$dff0d0

	move.l	cd_chip1(a0),d0
	move.l	cd_chip2(a0),cd_chip1(a0)
	move.l	d0,cd_chip2(a0)

	move.b	si_type(a1),d0
	and.b	#SI_STEREO,d0
	beq	no_stereo_samp2
	move.l	cd_chipS1(a0),$dff0b0
	move.l	cd_chipS1(a0),d0
	move.l	cd_chipS2(a0),cd_chipS1(a0)
	move.l	d0,cd_chipS2(a0)
no_stereo_samp2:
	bsr	send_signal
	rts

proc0name:	dc.b	"Audio0 interupt",0
proc2name:	dc.b	"Audio2 interupt",0
	even

*
* When fade_in is started a 50HZ interrupt is started which signals
* when the fade in completed ( task si_task, signal si_sig_fade )
*
	XDEF	_fade_in
_fade_in:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a0
	cmp.w	#2,si_channel(a0)
	beq	ch2
	bsr	install_50hz0	
	bra	end_fi
ch2:
	bsr	install_50hz2
end_fi:
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	rts

install_50hz0:
	lea	si_int_fade(a0),a1
	move.l	a0,IS_DATA(a1)
	move.b	#2,LN_TYPE(a1)
	move.b	#15,LN_PRI(a1)
	lea	proc_50hz0(pc),a0
	move.l	a0,IS_CODE(a1)
	lea	p50hzname1(pc),a0
	move.l	a0,LN_NAME(a1)
	moveq	#5,d0
	move.l	$4,a6
	jsr	_LVOAddIntServer(a6)
	rts

install_50hz2:
	lea	si_int_fade(a0),a1
	move.l	a0,IS_DATA(a1)
	move.b	#2,LN_TYPE(a1)
	move.b	#15,LN_PRI(a1)
	lea	proc_50hz2(pc),a0
	move.l	a0,IS_CODE(a1)
	lea	p50hzname2(pc),a0
	move.l	a0,LN_NAME(a1)
	moveq	#5,d0
	move.l	$4,a6
	jsr	_LVOAddIntServer(a6)
	rts

	XDEF	_remove_fade
_remove_fade:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a0
	lea.l	si_int_fade(a0),a1
	moveq	#5,d0
	move.l	$4,a6
	jsr	_LVORemIntServer(a6)
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	rts

proc_50hz2:
proc_50hz0:
	movem.l	a0/a5,-(a7)
	move.l	a1,a0
	moveq	#0,d1
	move.w	si_inc_right(a0),d0
	add.w	d0,si_vol_temp_right(a0)
	move.w	si_inc_left(a0),d0
	add.w	d0,si_vol_temp_left(a0)

	move.w	si_vol_temp_right(a0),d0
	lsr.w	#4,d0
	tst.w	si_inc_right(a0)
	bmi	np_no_ar

	cmp.w	si_vol_right(a0),d0
	blt	p_no_ar
	move.w	si_vol_right(a0),d0
	lsl.l	#4,d0
	move.w	d0,si_vol_temp_right(a0)
	addq.w	#1,d1
p_no_ar:
	bra	fp_no_ar

np_no_ar:
	cmp.w	si_vol_right(a0),d0
	bgt	p_no_ar
	move.w	si_vol_right(a0),d0
	lsl.l	#4,d0
	move.w	d0,si_vol_temp_right(a0)
	addq.w	#1,d1
fp_no_ar:

	move.w	si_vol_temp_right(a0),d0
	lsr.w	#4,d0
	tst.w	si_inc_right(a0)
	bmi	np_no_al
	
	cmp.w	si_vol_left(a0),d0
	blt	p_no_al
	move.w	si_vol_left(a0),d0
	lsl.l	#4,d0
	move.w	d0,si_vol_temp_left(a0)
	addq.w	#1,d1
p_no_al:
	bra	fp_no_al

np_no_al:	
	cmp.w	si_vol_left(a0),d0
	bgt	p_no_al
	move.w	si_vol_left(a0),d0
	lsl.l	#4,d0
	move.w	d0,si_vol_temp_left(a0)
	addq.w	#1,d1

fp_no_al:
	cmp.w	#2,d1
	bne	p_no_finish

	movem.l	a0-a2/a6/d0-d2,-(a7)	// signal the task that you are ready
	move.l	4,a6
	move.l	si_fadesig(a0),d0
	move.l	si_task(a0),a1
	jsr	_LVOSignal(a6)
	movem.l	(a7)+,a0-a2/a6/d0-d2

p_no_finish:
	lea	$dff000,a5
	add.l	#aud0,a5
	cmp.w	#2,si_channel(a0)
	bne	p_no_ch2		
	add.l	#ac_SIZEOF*2,a5
	move.w	si_vol_temp_right(a0),d0
	lsr.w	#4,d0
	move.w	d0,ac_SIZEOF+ac_vol(a5)
	move.w	si_vol_temp_left(a0),d0
	lsr.w	#4,d0
	move.w	d0,ac_vol(a5)
	bra	tijd1
p_no_ch2:
	move.w	si_vol_temp_right(a0),d0
	lsr.w	#4,d0
	move.w	d0,ac_vol(a5)
	move.w	si_vol_temp_left(a0),d0
	lsr.w	#4,d0
	move.w	d0,ac_SIZEOF+ac_vol(a5)

tijd1:
;	move.w	#$f0f,$dff180
	movem.l	(a7)+,a0/a5
	moveq	#0,d0
	rts

p50hzname1:	dc.b	"50Hz-1 interupt",0
p50hzname2:	dc.b	"50Hz-2 interupt",0
	even
