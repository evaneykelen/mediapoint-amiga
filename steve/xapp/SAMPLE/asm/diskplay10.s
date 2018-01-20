* File	: diskplay.s
* Uses	: disk_data.i
* Date	: 2 february 1993 
* Author: ing. C. Lieshout
* Desc	: Try to read samples from disk
*	: This program needs to be compiled with fsound.c

	INCDIR	"wsa:asm/"
	INCLUDE	"disk_data.i"
	INCDIR	"wp:inclibs/"
	INCLUDE	"exec_lib.i"
	
exec = 4

DEBUG = 0

	IF	DEBUG
	XREF	KPutFmt
	XREF	KPutStr
	PRINTT	"De debug info staat aan dus je moet nog met debug.lib linken"
	ENDC


	xdef	_do_sound
_do_sound:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	lea	$dff000,a1
	move.w	#$00ff,adkcon(a1)

	move.l	8(a5),d0		; period
	move.w	d0,audoper(a1)
	move.w	d0,aud1per(a1)

	move.l	12(a5),d0		; volume
	move.w	d0,audovol(a1)
	move.w	d0,aud1vol(a1)
	move.w	d0,volume_0
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	rts

	xdef	_fade_sound
_fade_sound:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.w	#0,action
	move.l	#0,teller
	move.l	#4,tellerstart
	move.w	#64,tvolume
	move.w	#1,action	
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	rts

	IF DEBUG
dbstr1:	dc.b	"Init sound",10,0
	even
	ENDC
	
	xdef	_set_sound_vars
_set_sound_vars:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	lea	$dff000,a1
	move.w	#$000f,dmacon(a1)

	move.l	20(a5),a0
	move.l	(a0),audolc(a1)
	move.l	(a0),aud1lc(a1)

	move.l	a0,structpoint

;	move.l	d0,second_0

	move.w	#0,audovol(a1)
	move.w	#0,aud1vol(a1)
	move.l	16(a5),d0		; SIZE
	lsr.w	#1,d0
	move.w	d0,audolen(a1)
	move.w	d0,aud1len(a1)

	move.l	8(a5),d0		; period
	move.w	d0,audoper(a1)
	move.w	d0,aud1per(a1)

	move.w	#$00ff,adkcon(a5)

	bsr.w	install_audio_0
	bsr	install_50h

	IF DEBUG
	movem.l	a0-a6/d0-d7,-(a7)
;	lea	8(a5),a1
	lea	dbstr1(pc),a0
	jsr	KPutStr
	movem.l	(a7)+,a0-a6/d0-d7
	ENDC

	move.w	#$8203,$dff096
	move.w	#$8080,$dff09a
	move.w	#$0080,$dff09c

	bset	#1,$bfe001
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	rts

	xdef	_exit_sound
_exit_sound:
	movem.l	d0-d7/a0-a6,-(a7)
	lea	$dff000,a5
	move.w	#$0080,$dff09a
	bsr	remove_50h
	bsr	remove_audio_0
	move.w	#$000f,dmacon(a5)
	movem.l	(a7)+,d0-d7/a0-a6
	rts

send_signal:
	movem.l	a0-a2/a6/d0-d2,-(a7)
	move.l	exec,a6
	move.l	task(pc),a1
	move.l	sigmask(pc),d0
	jsr	_LVOSignal(a6)
	movem.l	(a7)+,a0-a2/a6/d0-d2
	rts

	xdef	_setsam
_setsam:
	link	a5,#0
	move.l	8(a5),sigmask
	movem.l	d0-d7/a0-a5,-(a7)
	sub.l	a1,a1
	move.l	exec,a6
	jsr	_LVOFindTask(a6)
	move.l	d0,task
	movem.l	(a7)+,d0-d7/a0-a5
	unlk	a5
	rts

	xdef	_setfade
_setfade:
	link	a5,#0
	move.l	8(a5),sigfade
	movem.l	d0-d7/a0-a5,-(a7)
	sub.l	a1,a1
	move.l	exec,a6
	jsr	_LVOFindTask(a6)
	move.l	d0,taskfade
	movem.l	(a7)+,d0-d7/a0-a5
	unlk	a5
	rts
	
install_audio_0:
	move.l	#intstruct0,a1
	move.l	#7,d0
	move.l	exec,a6
	jsr	_LVOSetIntVector(a6)
	move.l	d0,old_audio_0
	rts

remove_audio_0:
	move.l	#7,d0
	move.l	old_audio_0,a1
	move.l	exec,a6
	jsr	_LVOSetIntVector(a6)
	rts

proc_audio_0:
	move.w	#$0080,$dff09c
	
	move.l	structpoint(pc),a0
	move.l	(a0),$dff0a0		; zet audio start naar andere buffer
	move.l	(a0),$dff0b0

	move.l	(a0),d0
	move.l	4(a0),(a0)
	move.l	d0,4(a0)

	bsr	send_signal

no_change_0:
	rts

proc0naam:	dc.b	"audio0 interupt",0
	even
		
old_audio_0:	dc.l	0

intstruct0:
	dc.l	0,0
	dc.b	2,100			; type en pri
	dc.l	proc0naam,0		; pointer naar naam en data
	dc.l	proc_audio_0

install_50h:
	lea	handle_installed(pc),a0
	tst.b	(a0)
	bne.b	no_install

	lea.l	intstruct50h(pc),a1

	lea	proc50hnaam(pc),a0
	move.l	a0,10(a1)
	lea	teller(pc),a0
	move.l	a0,14(a1)
	lea	proc_50h(pc),a0
	move.l	a0,18(a1)

	moveq	#5,d0
	move.l	$4,a6
	jsr	_LVOAddIntServer(a6)
	lea	handle_installed(pc),a0
	move.b	#$ff,(a0)
no_install:
	rts

remove_50h:
	lea	handle_installed(pc),a0
	tst.b	(a0)
	beq.b	no_remove
	lea.l	intstruct50h(pc),a1
	moveq	#5,d0
	move.l	$4,a6
	jsr	_LVORemIntServer(a6)
	lea	handle_installed(pc),a0
	move.b	#$00,(a0)
no_remove:
	rts

proc_50h:
	move.l	a0,-(a7)
* if you dont want any fading

	tst.w	off_action(a1)
	beq	no_send
	add.l	#32,off_teller(a1)
	move.l	off_tel_start(a1),d0
	cmp.l	off_teller(a1),d0
	bgt.w	no_send
	sub.w	#32,off_volume(a1)
	tst.w	off_volume(a1)
	bmi	volume_zero
	move.w	off_volume(a1),$dff0a8
	move.w	off_volume(a1),$dff0b8
	clr.l	off_teller(a1)
no_send:
	move.l	(a7)+,a0
	moveq	#0,d0
	rts

volume_zero:
	move.w	#0,$dff0a8
	move.w	#0,$dff0b8
	move.w	#0,off_action(a1)			; switch off fade
	movem.l	a0-a2/a6/d0-d2,-(a7)
	move.l	exec,a6
	move.l	taskfade(pc),a1
	move.l	sigfade(pc),d0
	jsr	_LVOSignal(a6)
	movem.l	(a7)+,a0-a2/a6/d0-d2
	move.l	(a7)+,a0
	moveq	#0,d0
	rts

proc50hnaam:	dc.b	"50Hz interupt",0
	even
		
intstruct50h:
	dc.l	0,0
	dc.b	2,15	; type en pri
	dc.l	0,0	; pointer naar naam en data
	dc.l	0

teller:		dc.l	0
tellerstart:	dc.l	4
tvolume:	dc.w	0
action:		dc.w	0
handle_installed:	dc.b	0,0

off_teller = 0
off_tel_start = 4
off_volume = 8
off_action = 10

;first_0:	dc.l	0
;second_0:	dc.l	0
structpoint:	dc.l	0
volume_0:	dc.w	0
task:		dc.l	0
sigmask:	dc.l	0
taskfade:	dc.l	0
sigfade:	dc.l	0
