;
;***                  ***
;*** MediaLink Player ***  file: MLP.asm
;***                  ***  type: object code
;***   Mike Kennedy   ***   asm: DevPac v2.08
;***                  ***

; The following is the source code to the 6 MediaLink Player functions
; which are: SetupPlayer(), KillPlayer(), ReadModule(), KillModule(),
; PlayTune(), and StopTune()... Assemble into linkable object code,
; and link with the XaPP.  I'm sorry, but I do not know anything about
; fd files or proto's so I'll leave that up to you.  All these functions
; adhere to the LatticeC v5.1 assembly interface rules...

DEBUG = 0

	XREF	KPutStr
	xdef	_SetupPlayer	;returns 0 on success
	xdef	_KillPlayer	;n/a
	xdef    _ReadModule      ;returns modptr or null = failure
	xdef	_KillModule      ;n/a
	xdef	_PlayTune	;null = failure
	xdef	_StopTune	;n/a
	xdef   _FakeRun	; Added 10-12-1993 KC.

_ExecBase		equ	4
_LVOOpenLibrary		equ	-552
_LVOCloseLibrary	equ	-414
_LVOAllocMem		equ	-198
_LVOFreeMem		equ	-210
_LVOLock                equ	-84
_LVOUnlock              equ     -90
_LVOExamine		equ     -102
_LVOOpen		equ	-30
_LVOClose		equ	-36
_LVORead		equ	-42
_LVOWrite		equ	-48
_LVOOutput		equ	-60
_LVORawDoFmt		equ	-522
_LVOOpenDevice		equ     -$1bc
_LVOCloseDevice		equ     -$1c2
_LVOOpenResource	equ	-$1f2
_LVOAddICRVector	equ	-$6
_LVORemICRVector	equ	-$c

PALTimerSpeed		equ	14187		; 14187.58
NTSCTimerSpeed		equ	14318		; 14318.18
PALAccur		equ	244
NTSCAccur		equ	795

	section	functions,code
;----------------------------------------------------------------------------
_ReadModule;(char *modname)			;Returns NULL on failure
		movem.l	a2-a6/d2-d6,-(sp)
		move.l	_ExecBase,a6            ;returns ptr to MODStruct
		lea	_DosLibrary,a1
		clr.l	d0
		jsr	_LVOOpenLibrary(a6)
		move.l	d0,_DosBase
		tst.l	d0
		beq	NoFile

		move.l  _DosBase,a6
		move.l	44(sp),d1
		move.l	#-1,d2
		jsr	_LVOLock(a6)
		move.l	d0,FileLock
		beq	NoFile

		move.l	d0,d1
		move.l	#FileInfoBlock,d2
		jsr	_LVOExamine(a6)

		move.l	FileLock,d1
		jsr	_LVOUnlock(a6)

		lea	FileInfoBlock,a0
		move.l	124(a0),d0
		move.l	d0,FileLength

		move.l	_ExecBase,a6
		add.l	#24,d0			; Added 4 KC (27-04-93)
		move.l	#2,d1
		jsr	_LVOAllocMem(a6)
		move.l	d0,FileBuffer
		beq	NoMem

		move.l	d0,a0
                move.l	FileLength,(a0)

		move.l	_DosBase,a6
		move.l	44(sp),d1
		move.l	#1005,d2
		jsr	_LVOOpen(a6)
		move.l	d0,FileHandle
		tst.l	d0
		beq	NoFile2

		move.l	FileHandle,d1
		move.l	FileBuffer,d2
		add.l	#20,d2
		move.l	FileLength,d3
		move.l	d3,d4
		jsr	_LVORead(a6)
		cmp.l	d4,d0
		bne	NoFile3

		move.l	FileHandle,d1
		jsr	_LVOClose(a6)

	        move.l	FileBuffer,a3
	        lea	20(a3),a0
		bsr	CheckTrack
		tst.l	d0
		bne.s	c0
		lea	_TRAK,a0
		bra     Found
c0		bsr	CheckDSS
		tst.l	d0
		bne.s	c1
		lea	_DSS,a0
		bra     Found
c1		lea	20(a3),a0
		bsr	CheckFC13
		tst.l	d0
		bne.s	c2
		lea	_FC13,a0
		bra.s   Found
c2		bsr	CheckFC14
		tst.l	d0
		bne.s	c3
		lea	_FC14,a0
		bra.s   Found
c3		bsr	CheckJC
		tst.l	d0
		bne.s	c4
		lea	_JAM,a0
		bra.s   Found
c4		bsr	CheckMark2
		tst.l	d0
		bne.s	c5
		lea	_MK2,a0
		bra.s   Found
c5		bsr	CheckSM
		tst.l	d0
		bne.s	c6
		lea	_SM2,a0
		bra.s   Found
c6		bsr	CheckST15
		tst.l	d0
		beq.s	c7
		lea	_ST15,a0
		bra.s   Found
c7	      	lea	20(a3),a0
		bsr	CheckST31
		tst.l	d0
		bne.s	c8
		lea	_TRAK,a0
		bra.s   Found
c8		bra.s	NoFile2
		rts

Found		lea	4(a3),a1
		moveq	#3,d0
floop		move.l	(a0)+,(a1)+
		dbra	d0,floop
		move.l	a3,d0
		movem.l	(sp)+,a2-a6/d2-d6
		rts
NoFile3		move.l	FileHandle,d1
		jsr	_LVOClose(a6)
NoFile2		move.l	_ExecBase,a6
		move.l	FileBuffer,a1
		move.l	FileLength,d0
		add.l	#24,d0			; added 24 KC (04-05-93)
		jsr	_LVOFreeMem(a6)
NoMem
NoFile		clr.l	d0
		movem.l	(sp)+,a2-a6/d2-d6
		rts

_KillModule	;(long MODStruct)			;returns nothing...
               	move.l	4(sp),a1
		movem.l	a6,-(sp)
		move.l	_ExecBase,a6
		move.l	(a1),d0
		add.l	#24,d0			; added 4 KC (27-04-93)
		jsr	_LVOFreeMem(a6)
		movem.l	(sp)+,a6
		rts

_DosLibrary	dc.b	'dos.library',0
_DosBase        dc.l	0
                cnop	0,4
FileInfoBlock	ds.b	260
FileLock	dc.l    0
FileHandle	dc.l    0
FileLength	dc.l    0
FileBuffer	dc.l    0
;----------------------------------------------------------------------------
_PlayTune;(long MODPtr)				;returns 0 on success
		move.l	4(sp),a0
		movem.l	a2-a6/d2-d6,-(sp)
		move.l	a0,_CurrentMod
		lea	8(a0),a0
		move.l	(a0),a0
		jsr	(a0)
		move.l	#0,_OffsetAdjust
		bsr	StartTimer
		movem.l	(sp)+,a2-a6/d2-d6
		rts

_StopTune	movem.l	a2-a6/d2-d6,-(sp)       ;no return
		bsr	StopTimer
		move.l	_CurrentMod,a0
		move.l	16(a0),a0
		jsr	(a0)
		movem.l	(sp)+,a2-a6/d2-d6
		rts

; 1 on the 244 times add one by the time value
; NTSC 795 

_Interrupt	movem.l	d1-d7/a0-a6,-(sp)
		move.w	CIATimerSpeed(pc),d0
		addq.l	#1,_OffsetAdjust
		move.l	_AccurValue(pc),d1
		move.l	_OffsetAdjust(pc),d2
		cmp.l	d1,d2			; PAL 244
		blt	no_add1
		addq.l	#1,d1
		cmp.l	d1,d2			; PAL 245
		blt	no_add2

		move.l	#1,_OffsetAdjust
		bra	do_add1
no_add2:
		sub.w	#100,d0
;		move.w	#$f00,$dff180
do_add1:
		move.l	_wTimer(pc),a2
		move.l	4(a2),a0
		move.b	d0,(a0)
		move.l	8(a2),a0
		lsr.w	#8,d0
		move.b	d0,(a0)
		
no_add1:	move.l	_CurrentMod,a0
		move.l  12(a0),a0

;		move.w	#$f0f,$dff180
		jsr	(a0)
;		move.w	#$000,$dff180
		movem.l	(sp)+,d1-d7/a0-a6
		moveq	#0,d0
		rts

_FakeRun:
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	_CurrentMod(pc),d0		; remember old module
	move.l	d0,-(a7)
	lea	fakemod(pc),a0
	move.l	a0,_CurrentMod
	bsr	OpenTimer
	bsr	StartTimer
	bsr	StopTimer
	move.l	(a7)+,d0
	move.l	d0,_CurrentMod				; restore old module
	movem.l	(a7)+,d0-d7/a0-a6
	moveq	#0,d0
	rts
*
* Fake interrupt function
*
fake_init:
;	move.w	#$00f,$dff180
	rts	

_CurrentMod	dc.l	0
_OffsetAdjust:	dc.l	0
_AccurValue:	dc.l	0

;----------------------------------------------------------------------------
_SetupPlayer	movem.l	a2-a6/d2-d6,-(sp)       ;returns 0 on success
		bsr	AllocAudio
		tst.l	d0
		bne.s	AudioErr
		bsr	OpenTimer
		tst.l	d0
		beq.s	TimerError
		clr.l	d0
		movem.l	(sp)+,a2-a6/d2-d6
		rts
TimerError	bsr	KillAudio
AudioErr	move	#1,d0
		movem.l	(sp)+,a2-a6/d2-d6
		rts

AllocAudio	movem.l	a6,-(sp)		;returns NULL if successful
		clr.l	d0
		clr.l	d1
		lea	IOAudio(pc),a1
		move.b	#$7F,9(a1)
		lea	AudioName(pc),a0
		move.l	_ExecBase,a6
		jsr	_LVOOpenDevice(a6)
		movem.l	(sp)+,a6
		rts

_KillPlayer					;no return
KillAudio	movem.l	a6,-(sp)
		lea	IOAudio(pc),a1
		move.l	_ExecBase,a6
		jsr	_LVOCloseDevice(a6)
		movem.l	(sp)+,a6
		rts

AudioName	dc.b	'audio.device',0,0

IOAudio
ioa_Request
io_Message
mn_Node
ln_Succ		dc.l	0
ln_Pred		dc.l	0
ln_Type		dc.b    0
ln_Pri		dc.b    $7f
ln_Name		dc.l	0
mn_ReplyPort	dc.l	0
mn_Length	dc.w	0
io_Device	dc.l	0
io_Unit		dc.l	0
io_Command	dc.w	0
io_Flags	dc.b	0
io_Error	dc.b	0
ioa_AllocKey	dc.w	0
ioa_Data	dc.l    TempDat
ioa_Length	dc.l	1
ioa_Period	dc.w	0
ioa_Volume	dc.w	0
ioa_Cycles	dc.w	0
ioa_WriteMsg	ds.l    5
TempDat		dc.w	$f00

;----------------------------------------------------------------------------
OpenTimer       movem.l	a6,-(sp)		;returns 0 on failure
		move.l	_ExecBase,a6
		lea	CIABRsrc,a1
		clr.l	d0
		jsr	_LVOOpenResource(a6)
		move.l	d0,CIABBase
		movem.l	(sp)+,a6
		rts

StartTimer	movem.l	a2/a6,-(sp)		;returns 0 on success
		move.l	_ExecBase,a6
		cmpi.b	#60,530(a6)
		beq.s   NTSC
		move.w	#PALTimerSpeed,CIATimerSpeed
		move.l	#PALAccur,_AccurValue
		bra.s	GTcont
NTSC		move.w	#NTSCTimerSpeed,CIATimerSpeed
		move.l	#NTSCAccur,_AccurValue
GTcont		lea	TimerRegisters,a2

ww1:
		moveq	#0,d0
		move.b	$dff006,d0
		and.b	#$ff,d0
		cmp.b	#100,d0
		ble	ww1
		cmp.b	#120,d0
		bge	ww1

		move.l	CIABBase,a6
		clr.l	d0
		move.l	d0,TimerTemp
		lea	TimerInterrupt,a1
		jsr	_LVOAddICRVector(a6)
		tst.l	d0
		beq.s	SetTimer

ww2:
		moveq	#0,d0
		move.b	$dff006,d0
		and.b	#$ff,d0
		cmp.b	#100,d0
		ble	ww2
		cmp.b	#120,d0
		bge	ww2
		
		lea	12(a2),a2
		moveq	#1,d0
		move.l	d0,TimerTemp
		lea	TimerInterrupt,a1
		jsr	_LVOAddICRVector(a6)
		tst.l	d0
		beq.s	SetTimer
		movem.l	(sp)+,a2/a6
		rts

SetTimer	move.l	a2,_wTimer
		move.l	(a2),a0
		move.b	#%10001,(a0)
		move.w	CIATimerSpeed,d1
		move.l	4(a2),a0
		move.b	d1,(a0)
		move.l	8(a2),a0
		lsr.w	#8,d1
		move.b	d1,(a0)
		clr.l	d0
		movem.l	(sp)+,a2/a6
		rts

StopTimer	movem.l	a6,-(sp)		;no return value
		move.l	CIABBase,a6
		move.l	TimerTemp,d0
		lea	TimerInterrupt,a1
		jsr	_LVORemICRVector(a6)
		movem.l	(sp)+,a6
		rts


_wTimer		dc.l	0
CIATimerSpeed	dc.w	0
CIABBase	dc.l	0
CIABRsrc	dc.b	'ciab.resource',0
InterruptName	dc.b	'MLP_interrupt',0
  even

TimerInterrupt	dc.l	0
		dc.l	0
		dc.b	4
		dc.b	0
		dc.l	InterruptName
		dc.w	0
		dc.w	0
		dc.l	_Interrupt

TimerTemp	dc.l	0

TimerRegisters	dc.l	$bfde00,$bfd400,$bfd500
		dc.l	$bfdf00,$bfd600,$bfd700

;---------------------------------------------------------------------------
CheckTrack
		clr.l	d0
		cmp.l	#'M.K.',$438(a0)
		sne	d0
		rts
;--------------------
CheckDSS
		cmp.l	#'MMU2',(a0)
		bne.s	Chk7
		lea	$59C(a0),a1
		move.w	(a1)+,d0
		subq.w	#1,d0
		clr.l	d1
		move.l	d1,d2
Chk2		move.b	(a1)+,d1
		cmp.b	d1,d2
		bhi.s	Chk3
		move.b	d1,d2
Chk3		dbra	d0,Chk2
		addq.w	#1,d2
		moveq	#10,d0
		lsl.l	d0,d2
		add.l	#$61E,d2
		lea	10(a0),a1
		moveq	#$1E,d0
Chk4		clr.l	d1
		move.w	$22(a1),d1
		beq.s	Chk6
		add.l	d1,d1
		add.l	d1,d2
		clr.l	d1
		cmp.w	#1,$28(a1)
		beq.s	Chk5
		move.w	$28(a1),d1
		add.l	d1,d1
Chk5		add.l	d1,d2
		bclr	#0,$21(a1)
		add.l	$1E(a1),d2
Chk6		lea	$2E(a1),a1
		dbra	d0,Chk4
		move.l	$28(a5),d0
		move.l	d0,d1
		sub.l	#$40,d0
		add.l	#$40,d1
		cmp.l	d0,d2
		blt.s	Chk7
		cmp.l	d1,d2
		bgt.s	Chk7
		clr.l	d0
		bra.s	Chk8
Chk7		moveq	#1,d0
Chk8		rts
;-------------------
CheckFC13	clr.l	d0
		cmp.l	#'FC13',(a0)
	        beq.s	FC_ok
		cmp.l	#'SMOD',(a0)
		sne	d0
FC_ok		rts
;----------------------
CheckFC14	clr.l	d0
		cmpi.l	#"FC14",(a0)
		sne	d0
		rts
;---------------------------
CheckJC		clr.l	d0
		cmp.l	#'BeEp',(a0)
		sne	d0
		rts
;--------------------
CheckMark2	clr.l	d0
		cmpi.l  #".ZAD",$348(a0)
		sne	d0
		cmpi.l  #"S89.",$34C(a0)
        	sne	d0
		rts
;--------------------
CheckSM		clr.l	d0
		cmp.w	#'V.',$1a(a0)
		sne	d0
		rts
;--------------------
CheckST15	move.l	a0,a2
		add.l	#4,a2
ST155BC		tst.b	$1d7(a0)
		beq	ST1569A
		lea	$1d8(a0),a1
		cmp.b	#$ff,$1d7(a0)
		beq	ST1569A
		cmp.b	#0,$1d6(a0)
		beq	ST1569A
		cmp.b	#$80,$1d6(a0)
		bhi	ST1569A
		clr.l	d0
		move.w	#$800,d1
		move.b	$1d6(a0),d0
		move.b	d0,d1
ST155F2		cmp.b	(a1,d0.l),d1
		bcs	ST1569A
		dbra	d0,ST155F2
		lea	$2a(a0),a1
		clr.l	d0
		clr.l	d1
		clr.l	d2
		clr.l	d3
ST15610		move.w	(a1,d0.w),d1
		tst.w	d1
		beq.s	ST15620
		tst.w	2(a1,d0.w)
		beq.s	ST15620
		addq.w	#1,d3
ST15620		add.l	d1,d2
		and.w	#$8000,d1
		bne.s	ST1569A
		move.w	4(a1,d0.w),d1
		and.w	#$8000,d1
		bne.s	ST1569A
		move.w	6(a1,d0.w),d1
		and.w	#$8000,d1
		bne.s	ST1569A
		add.w	#$1e,d0
		cmp.w	#$1c2,d0
		bne.s	ST15610
		tst.l	d2
		beq.s	ST1569A
		cmp.l	#$40000,d2
		bhi.s	ST1569A
		cmp.w	#2,d3
		bls.s	ST1569A
		lea	$2c(a0),a1
		clr.l	d0
ST1565E		cmp.b	#0,(a1,d0.l)
		bne.s	ST1569A
		add.w	#$1e,d0
		cmp.w	#$1c2,d0
		bne.s	ST1565E
		lea	$2d(a0),a1
		clr.l	d0
		clr.l	d1
ST15678		cmp.b	#$40,(a1,d0.l)
		bhi.s	ST1569A
		tst.b	(a1,d0.l)
		beq.s	ST15688
		addq.l	#1,d1
ST15688		add.w	#$1e,d0
		cmp.w	#$1c2,d0
		bne.s	ST15678
		cmp.w	#2,d1
		bls.s	ST1569A
		bra.s	ST156A6
ST1569A		addq.l	#2,a0
		cmp.l	a2,a0
		bne	ST155BC
		clr.l	d0
        	rts
ST156A6		move.l	#1,d0
	        rts
;--------------------
CheckST31	bsr     Check31
		clr.l	d0
		tst.l	ST23ADDRESS
	        seq	d0
		rts
Check31		tst.b	$3b6(a0)
		beq	ST256
		cmp.b	#$80,$3b6(a0)
		bhi	ST256
		moveq	#$14,d0
		clr.l	d1
		clr.l	d2
		moveq	#$1e,d7
ST1b4		cmp.w	#$4000,$16(a0,d0.l)
		bhi	ST256
		move.w	$1a(a0,d0.l),d1
		lsr.w	#1,d1
		add.w	$1c(a0,d0.l),d1
		bvs	ST256
		cmp.w	#$4000,d1
		bhi	ST256
		tst.w	$16(a0,d0.l)
		beq.s	ST20a
		move.w	$18(a0,d0.l),d1
		cmp.w	#$40,d1
		bhi	ST256
		cmp.w	#1,$1c(a0,d0.l)
		beq.s	ST1f6
		cmp.w	#$80,$1c(a0,d0.l)
		bcs	ST256
ST1f6		move.w	$1a(a0,d0.l),d1
		lsr.w	#1,d1
		add.w	$1c(a0,d0.l),d1
		bvs	ST256
		cmp.w	$16(a0,d0.l),d1
		bhi.s	ST256
ST20a		move.w	$16(a0,d0.l),d1
		add.l	d1,d2
		add.l	#$1e,d0
		dbra	d7,ST1b4
		lsl.l	#1,d2
		cmp.l	#$4000,d2
		bls.s	ST256
		clr.l	d0
		move.b	$3b6(a0),d0
		sub.b	#1,d0
		move.w	d0,d1
		add.w	#$3b8,d1
ST234		cmp.b	#$3f,(a0,d1.l)
		bhi	ST256
		subq.w	#1,d1
		dbra	d0,ST234
		move.l	a0,ST23ADDRESS
		move.l	d2,ST23TEMP
		move.l	ST23TEMP,d1
		add.l	#$438,d1
		add.l	#$3b8,a0
		clr.l	d0
		moveq	#$7f,d7
ST2c2		cmp.b	(a0)+,d0
		bhi.s	ST2ca
		move.b	-1(a0),d0
ST2ca		dbra	d7,ST2c2
		addq.w	#1,d0
		lsl.l	#8,d0
		lsl.l	#2,d0
		add.l	d0,d1
		add.l	#4,d1
		move.l	d1,ST23LENGTH
		rts
ST256		clr.l	ST23ADDRESS
		clr.l	ST23LENGTH
		clr.l	ST23TEMP
		rts
ST23ADDRESS	dc.l	0
ST23LENGTH	dc.l	0
ST23TEMP	dc.l	0
;--------------------

;Digital Sound Studio
_DSS	dc.b	'DSS!'
	dc.l	InitDSSMOD
	dc.l	PlayDSSMOD
	dc.l	StopDSSMOD
	dc.b	'Digital Sound Studio',0
	even
;Future Composer v1.3
_FC13	dc.b	'FC13'
	dc.l	FC13INIT_MUSIC
	dc.l	FC13PLAY_MUSIC
	dc.l	FC13END_MUSIC
	dc.b	'Future Composer v1.3',0
	even
;Future Composer v1.4
_FC14	dc.b	'FC14'
	dc.l	FC14INIT_MUSIC
	dc.l	FC14PLAY_MUSIC
	dc.l	FC14END_MUSIC
	dc.b	'Future Composer v1.4',0
	even
;JamCracker Pro
_JAM	dc.b	'JAMC'
	dc.l	pp_init
	dc.l	pp_play
	dc.l	pp_end
	dc.b	'JamCracker Pro',0
	even
;MarkII
_MK2	dc.b	'MRK2'
	dc.l	InitMark2
	dc.l	PlayMark2
	dc.l	StopMark2
	dc.b	'MarkII',0
	even
;SoundMonitor v2.0
_SM2	dc.b	'SM20'
	dc.l	bpinit
	dc.l	bpmusic
	dc.l	bpend
	dc.b	'SoundMonitor v2.0',0
	even
;Old SoundTracker 15
_ST15	dc.b	'ST15'
	dc.l	start_muzak
	dc.l	replay_muzak
	dc.l	stop_muzak
	dc.b	'Old SoundTracker15',0
	even
;SoundTracker/NoiseTracker/ProTracker/StarTrekker
_TRAK	dc.b	'TRAK'
	dc.l	mt_init
	dc.l	mt_music
	dc.l	mt_end
	dc.b	'31 Inst. Tracker',0
	even
; Fake init stub for the first run 
; For some reason the interrupt is starting somewhere else the first time
; Added 10-12-1993 KC.
fakemod:	dc.b	'    '
	dc.l	fake_init
	dc.l	fake_init
	dc.l	fake_init
	dc.b	' '
	even

;---------------------------------------------------------------------------
	section	players,code_c
	include wm:mediaplayers.asm
