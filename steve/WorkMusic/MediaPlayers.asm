;
;***                  ***
;*** MediaLink Player ***  file: MediaPlayers.asm
;***                  ***  type: include code
;***   Mike Kennedy   ***   asm: DevPac v2.08
;***                  ***
;
; The following is the source codes to the various replay routines...
; Right now it includes : SoundTracker15, SoundTracker31, NoiseTracker,
; ProTracker, JamCracker, SoundMonitor, Future Composer v1.3/1.4, DSS,
; & MarkII.  This can be easily expanded in the future.
;

;***************************************
;*** PRO/NOISE/SOUNDTRACKER ROUTINE! ***
;***************************************

shutoff	move.l	#15,$dff096
	clr.l	$dff0a8
	clr.l	$dff0b8
	clr.l	$dff0c8
	clr.l	$dff0d8
	bset	#1,$bfe001
	rts

DMAWait = 300 ; Set this as low as possible without losing low notes.

n_note		EQU	0  ; W
n_cmd		EQU	2  ; W
n_cmdlo		EQU	3  ; B
n_start		EQU	4  ; L
n_length	EQU	8  ; W
n_loopstart	EQU	10 ; L
n_replen	EQU	14 ; W
n_period	EQU	16 ; W
n_finetune	EQU	18 ; B
n_volume	EQU	19 ; B
n_dmabit	EQU	20 ; W
n_toneportdirec	EQU	22 ; B
n_toneportspeed	EQU	23 ; B
n_wantedperiod	EQU	24 ; W
n_vibratocmd	EQU	26 ; B
n_vibratopos	EQU	27 ; B
n_tremolocmd	EQU	28 ; B
n_tremolopos	EQU	29 ; B
n_wavecontrol	EQU	30 ; B
n_glissfunk	EQU	31 ; B
n_sampleoffset	EQU	32 ; B
n_pattpos	EQU	33 ; B
n_loopcount	EQU	34 ; B
n_funkoffset	EQU	35 ; B
n_wavestart	EQU	36 ; L
n_reallength	EQU	40 ; W

mt_init	move.l	_CurrentMod,a0
	lea	20(a0),a0
	MOVE.L	A0,mt_SongDataPtr
	MOVE.L	A0,A1
	LEA	952(A1),A1
	MOVEQ	#127,D0
	MOVEQ	#0,D1
mtloop	MOVE.L	D1,D2
	SUBQ.W	#1,D0
mtloop2	MOVE.B	(A1)+,D1
	CMP.B	D2,D1
	BGT.S	mtloop
	DBRA	D0,mtloop2
	ADDQ.B	#1,D2

	LEA	mt_SampleStarts(PC),A1
	ASL.L	#8,D2
	ASL.L	#2,D2
	ADD.L	#1084,D2
	ADD.L	A0,D2
	MOVE.L	D2,A2
	MOVEQ	#30,D0
mtloop3	CLR.L	(A2)
	MOVE.L	A2,(A1)+
	MOVEQ	#0,D1
	MOVE.W	42(A0),D1
	ASL.L	#1,D1
	ADD.L	D1,A2
	ADD.L	#30,A0
	DBRA	D0,mtloop3

	OR.B	#2,$BFE001
	MOVE.B	#6,mt_speed
	CLR.B	mt_counter
	CLR.B	mt_SongPos
	CLR.W	mt_PatternPos
mt_end	CLR.W	$DFF0A8
	CLR.W	$DFF0B8
	CLR.W	$DFF0C8
	CLR.W	$DFF0D8
	MOVE.W	#$F,$DFF096
	RTS

mt_music
	MOVEM.L	D0-D4/A0-A6,-(SP)
	ADDQ.B	#1,mt_counter
	MOVE.B	mt_counter(PC),D0
	CMP.B	mt_speed(PC),D0
	BLO.S	mt_NoNewNote
	CLR.B	mt_counter
	TST.B	mt_PattDelTime2
	BEQ.S	mt_GetNewNote
	BSR.S	mt_NoNewAllChannels
	BRA	mt_dskip

mt_NoNewNote
	BSR.S	mt_NoNewAllChannels
	BRA	mt_NoNewPosYet

mt_NoNewAllChannels
	LEA	$DFF0A0,A5
	LEA	mt_chan1temp(PC),A6
	BSR	mt_CheckEfx
	LEA	$DFF0B0,A5
	LEA	mt_chan2temp(PC),A6
	BSR	mt_CheckEfx
	LEA	$DFF0C0,A5
	LEA	mt_chan3temp(PC),A6
	BSR	mt_CheckEfx
	LEA	$DFF0D0,A5
	LEA	mt_chan4temp(PC),A6
	BRA	mt_CheckEfx

mt_GetNewNote
	MOVE.L	mt_SongDataPtr(PC),A0
	LEA	12(A0),A3
	LEA	952(A0),A2	;pattpo
	LEA	1084(A0),A0	;patterndata
	MOVEQ	#0,D0
	MOVEQ	#0,D1
	MOVE.B	mt_SongPos(PC),D0
	MOVE.B	(A2,D0.W),D1
	ASL.L	#8,D1
	ASL.L	#2,D1
	ADD.W	mt_PatternPos(PC),D1
	CLR.W	mt_DMACONtemp

	LEA	$DFF0A0,A5
	LEA	mt_chan1temp(PC),A6
	BSR.S	mt_PlayVoice
	LEA	$DFF0B0,A5
	LEA	mt_chan2temp(PC),A6
	BSR.S	mt_PlayVoice
	LEA	$DFF0C0,A5
	LEA	mt_chan3temp(PC),A6
	BSR.S	mt_PlayVoice
	LEA	$DFF0D0,A5
	LEA	mt_chan4temp(PC),A6
	BSR.S	mt_PlayVoice
	BRA	mt_SetDMA

mt_PlayVoice
	TST.L	(A6)
	BNE.S	mt_plvskip
	BSR	mt_PerNop
mt_plvskip
	MOVE.L	(A0,D1.L),(A6)
	ADDQ.L	#4,D1
	MOVEQ	#0,D2
	MOVE.B	n_cmd(A6),D2
	AND.B	#$F0,D2
	LSR.B	#4,D2
	MOVE.B	(A6),D0
	AND.B	#$F0,D0
	OR.B	D0,D2
	TST.B	D2
	BEQ	mt_SetRegs
	MOVEQ	#0,D3
	LEA	mt_SampleStarts(PC),A1
	MOVE	D2,D4
	SUBQ.L	#1,D2
	ASL.L	#2,D2
	MULU	#30,D4
	MOVE.L	(A1,D2.L),n_start(A6)
	MOVE.W	(A3,D4.L),n_length(A6)
	MOVE.W	(A3,D4.L),n_reallength(A6)
	MOVE.B	2(A3,D4.L),n_finetune(A6)
	MOVE.B	3(A3,D4.L),n_volume(A6)
	MOVE.W	4(A3,D4.L),D3 ; Get repeat
	TST.W	D3
	BEQ.S	mt_NoLoop
	MOVE.L	n_start(A6),D2	; Get start
	ASL.W	#1,D3
	ADD.L	D3,D2		; Add repeat
	MOVE.L	D2,n_loopstart(A6)
	MOVE.L	D2,n_wavestart(A6)
	MOVE.W	4(A3,D4.L),D0	; Get repeat
	ADD.W	6(A3,D4.L),D0	; Add replen
	MOVE.W	D0,n_length(A6)
	MOVE.W	6(A3,D4.L),n_replen(A6)	; Save replen
	MOVEQ	#0,D0
	MOVE.B	n_volume(A6),D0
	MOVE.W	D0,8(A5)	; Set volume
	BRA.S	mt_SetRegs

mt_NoLoop
	MOVE.L	n_start(A6),D2
	ADD.L	D3,D2
	MOVE.L	D2,n_loopstart(A6)
	MOVE.L	D2,n_wavestart(A6)
	MOVE.W	6(A3,D4.L),n_replen(A6)	; Save replen
	MOVEQ	#0,D0
	MOVE.B	n_volume(A6),D0
	MOVE.W	D0,8(A5)	; Set volume
mt_SetRegs
	MOVE.W	(A6),D0
	AND.W	#$0FFF,D0
	BEQ	mt_CheckMoreEfx	; If no note
	MOVE.W	2(A6),D0
	AND.W	#$0FF0,D0
	CMP.W	#$0E50,D0
	BEQ.S	mt_DoSetFineTune
	MOVE.B	2(A6),D0
	AND.B	#$0F,D0
	CMP.B	#3,D0	; TonePortamento
	BEQ.S	mt_ChkTonePorta
	CMP.B	#5,D0
	BEQ.S	mt_ChkTonePorta
	CMP.B	#9,D0	; Sample Offset
	BNE.S	mt_SetPeriod
	BSR	mt_CheckMoreEfx
	BRA.S	mt_SetPeriod

mt_DoSetFineTune
	BSR	mt_SetFineTune
	BRA.S	mt_SetPeriod

mt_ChkTonePorta
	BSR	mt_SetTonePorta
	BRA	mt_CheckMoreEfx

mt_SetPeriod
	MOVEM.L	D0-D1/A0-A1,-(SP)
	MOVE.W	(A6),D1
	AND.W	#$0FFF,D1
	LEA	mt_PeriodTable(PC),A1
	MOVEQ	#0,D0
	MOVEQ	#36,D7
mt_ftuloop
	CMP.W	(A1,D0.W),D1
	BHS.S	mt_ftufound
	ADDQ.L	#2,D0
	DBRA	D7,mt_ftuloop
mt_ftufound
	MOVEQ	#0,D1
	MOVE.B	n_finetune(A6),D1
	MULU	#36*2,D1
	ADD.L	D1,A1
	MOVE.W	(A1,D0.W),n_period(A6)
	MOVEM.L	(SP)+,D0-D1/A0-A1

	MOVE.W	2(A6),D0
	AND.W	#$0FF0,D0
	CMP.W	#$0ED0,D0 ; Notedelay
	BEQ	mt_CheckMoreEfx

	MOVE.W	n_dmabit(A6),$DFF096
	BTST	#2,n_wavecontrol(A6)
	BNE.S	mt_vibnoc
	CLR.B	n_vibratopos(A6)
mt_vibnoc
	BTST	#6,n_wavecontrol(A6)
	BNE.S	mt_trenoc
	CLR.B	n_tremolopos(A6)
mt_trenoc
	MOVE.L	n_start(A6),(A5)	; Set start
	MOVE.W	n_length(A6),4(A5)	; Set length
	MOVE.W	n_period(A6),D0
	MOVE.W	D0,6(A5)		; Set period
	MOVE.W	n_dmabit(A6),D0
	OR.W	D0,mt_DMACONtemp
	BRA	mt_CheckMoreEfx

mt_SetDMA
	MOVE.W	#DMAWAIT,D0
mt_WaitDMA
	DBRA	D0,mt_WaitDMA
	MOVE.W	mt_DMACONtemp(PC),D0
	OR.W	#$8000,D0
	MOVE.W	D0,$DFF096
	MOVE.W	#DMAWAIT,D0
mt_WaitDMA2
	DBRA	D0,mt_WaitDMA2

	LEA	$DFF000,A5
	LEA	mt_chan4temp(PC),A6
	MOVE.L	n_loopstart(A6),$D0(A5)
	MOVE.W	n_replen(A6),$D4(A5)
	LEA	mt_chan3temp(PC),A6
	MOVE.L	n_loopstart(A6),$C0(A5)
	MOVE.W	n_replen(A6),$C4(A5)
	LEA	mt_chan2temp(PC),A6
	MOVE.L	n_loopstart(A6),$B0(A5)
	MOVE.W	n_replen(A6),$B4(A5)
	LEA	mt_chan1temp(PC),A6
	MOVE.L	n_loopstart(A6),$A0(A5)
	MOVE.W	n_replen(A6),$A4(A5)

mt_dskip
	ADD.W	#16,mt_PatternPos
	MOVE.B	mt_PattDelTime,D0
	BEQ.S	mt_dskc
	MOVE.B	D0,mt_PattDelTime2
	CLR.B	mt_PattDelTime
mt_dskc	TST.B	mt_PattDelTime2
	BEQ.S	mt_dska
	SUBQ.B	#1,mt_PattDelTime2
	BEQ.S	mt_dska
	SUB.W	#16,mt_PatternPos
mt_dska	TST.B	mt_PBreakFlag
	BEQ.S	mt_nnpysk
	SF	mt_PBreakFlag
	MOVEQ	#0,D0
	MOVE.B	mt_PBreakPos(PC),D0
	CLR.B	mt_PBreakPos
	LSL.W	#4,D0
	MOVE.W	D0,mt_PatternPos
mt_nnpysk
	CMP.W	#1024,mt_PatternPos
	BLO.S	mt_NoNewPosYet
mt_NextPosition
	MOVEQ	#0,D0
	MOVE.B	mt_PBreakPos(PC),D0
	LSL.W	#4,D0
	MOVE.W	D0,mt_PatternPos
	CLR.B	mt_PBreakPos
	CLR.B	mt_PosJumpFlag
	ADDQ.B	#1,mt_SongPos
	AND.B	#$7F,mt_SongPos
	MOVE.B	mt_SongPos(PC),D1
	MOVE.L	mt_SongDataPtr(PC),A0
	CMP.B	950(A0),D1
	BLO.S	mt_NoNewPosYet
	CLR.B	mt_SongPos
mt_NoNewPosYet
	TST.B	mt_PosJumpFlag
	BNE.S	mt_NextPosition
	MOVEM.L	(SP)+,D0-D4/A0-A6
	RTS

mt_CheckEfx
	BSR	mt_UpdateFunk
	MOVE.W	n_cmd(A6),D0
	AND.W	#$0FFF,D0
	BEQ.S	mt_PerNop
	MOVE.B	n_cmd(A6),D0
	AND.B	#$0F,D0
	BEQ.S	mt_Arpeggio
	CMP.B	#1,D0
	BEQ	mt_PortaUp
	CMP.B	#2,D0
	BEQ	mt_PortaDown
	CMP.B	#3,D0
	BEQ	mt_TonePortamento
	CMP.B	#4,D0
	BEQ	mt_Vibrato
	CMP.B	#5,D0
	BEQ	mt_TonePlusVolSlide
	CMP.B	#6,D0
	BEQ	mt_VibratoPlusVolSlide
	CMP.B	#$E,D0
	BEQ	mt_E_Commands
SetBack	MOVE.W	n_period(A6),6(A5)
	CMP.B	#7,D0
	BEQ	mt_Tremolo
	CMP.B	#$A,D0
	BEQ	mt_VolumeSlide
mt_Return2
	RTS

mt_PerNop
	MOVE.W	n_period(A6),6(A5)
	RTS

mt_Arpeggio
	MOVEQ	#0,D0
	MOVE.B	mt_counter(PC),D0
	DIVS	#3,D0
	SWAP	D0
	CMP.W	#0,D0
	BEQ.S	mt_Arpeggio2
	CMP.W	#2,D0
	BEQ.S	mt_Arpeggio1
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	LSR.B	#4,D0
	BRA.S	mt_Arpeggio3

mt_Arpeggio1
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#15,D0
	BRA.S	mt_Arpeggio3

mt_Arpeggio2
	MOVE.W	n_period(A6),D2
	BRA.S	mt_Arpeggio4

mt_Arpeggio3
	ASL.W	#1,D0
	MOVEQ	#0,D1
	MOVE.B	n_finetune(A6),D1
	MULU	#36*2,D1
	LEA	mt_PeriodTable(PC),A0
	ADD.L	D1,A0
	MOVEQ	#0,D1
	MOVE.W	n_period(A6),D1
	MOVEQ	#36,D7
mt_arploop
	MOVE.W	(A0,D0.W),D2
	CMP.W	(A0),D1
	BHS.S	mt_Arpeggio4
	ADDQ.L	#2,A0
	DBRA	D7,mt_arploop
	RTS

mt_Arpeggio4
	MOVE.W	D2,6(A5)
	RTS

mt_FinePortaUp
	TST.B	mt_counter
	BNE.S	mt_Return2
	MOVE.B	#$0F,mt_LowMask
mt_PortaUp
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	AND.B	mt_LowMask(PC),D0
	MOVE.B	#$FF,mt_LowMask
	SUB.W	D0,n_period(A6)
	MOVE.W	n_period(A6),D0
	AND.W	#$0FFF,D0
	CMP.W	#113,D0
	BPL.S	mt_PortaUskip
	AND.W	#$F000,n_period(A6)
	OR.W	#113,n_period(A6)
mt_PortaUskip
	MOVE.W	n_period(A6),D0
	AND.W	#$0FFF,D0
	MOVE.W	D0,6(A5)
	RTS

mt_FinePortaDown
	TST.B	mt_counter
	BNE	mt_Return2
	MOVE.B	#$0F,mt_LowMask
mt_PortaDown
	CLR.W	D0
	MOVE.B	n_cmdlo(A6),D0
	AND.B	mt_LowMask(PC),D0
	MOVE.B	#$FF,mt_LowMask
	ADD.W	D0,n_period(A6)
	MOVE.W	n_period(A6),D0
	AND.W	#$0FFF,D0
	CMP.W	#856,D0
	BMI.S	mt_PortaDskip
	AND.W	#$F000,n_period(A6)
	OR.W	#856,n_period(A6)
mt_PortaDskip
	MOVE.W	n_period(A6),D0
	AND.W	#$0FFF,D0
	MOVE.W	D0,6(A5)
	RTS

mt_SetTonePorta
	MOVE.L	A0,-(SP)
	MOVE.W	(A6),D2
	AND.W	#$0FFF,D2
	MOVEQ	#0,D0
	MOVE.B	n_finetune(A6),D0
	MULU	#36*2,D0
	LEA	mt_PeriodTable(PC),A0
	ADD.L	D0,A0
	MOVEQ	#0,D0
mt_StpLoop
	CMP.W	(A0,D0.W),D2
	BHS.S	mt_StpFound
	ADDQ.W	#2,D0
	CMP.W	#36*2,D0
	BLO.S	mt_StpLoop
	MOVEQ	#35*2,D0
mt_StpFound
	MOVE.B	n_finetune(A6),D2
	AND.B	#8,D2
	BEQ.S	mt_StpGoss
	TST.W	D0
	BEQ.S	mt_StpGoss
	SUBQ.W	#2,D0
mt_StpGoss
	MOVE.W	(A0,D0.W),D2
	MOVE.L	(SP)+,A0
	MOVE.W	D2,n_wantedperiod(A6)
	MOVE.W	n_period(A6),D0
	CLR.B	n_toneportdirec(A6)
	CMP.W	D0,D2
	BEQ.S	mt_ClearTonePorta
	BGE	mt_Return2
	MOVE.B	#1,n_toneportdirec(A6)
	RTS

mt_ClearTonePorta
	CLR.W	n_wantedperiod(A6)
	RTS

mt_TonePortamento
	MOVE.B	n_cmdlo(A6),D0
	BEQ.S	mt_TonePortNoChange
	MOVE.B	D0,n_toneportspeed(A6)
	CLR.B	n_cmdlo(A6)
mt_TonePortNoChange
	TST.W	n_wantedperiod(A6)
	BEQ	mt_Return2
	MOVEQ	#0,D0
	MOVE.B	n_toneportspeed(A6),D0
	TST.B	n_toneportdirec(A6)
	BNE.S	mt_TonePortaUp
mt_TonePortaDown
	ADD.W	D0,n_period(A6)
	MOVE.W	n_wantedperiod(A6),D0
	CMP.W	n_period(A6),D0
	BGT.S	mt_TonePortaSetPer
	MOVE.W	n_wantedperiod(A6),n_period(A6)
	CLR.W	n_wantedperiod(A6)
	BRA.S	mt_TonePortaSetPer

mt_TonePortaUp
	SUB.W	D0,n_period(A6)
	MOVE.W	n_wantedperiod(A6),D0
	CMP.W	n_period(A6),D0
	BLT.S	mt_TonePortaSetPer
	MOVE.W	n_wantedperiod(A6),n_period(A6)
	CLR.W	n_wantedperiod(A6)

mt_TonePortaSetPer
	MOVE.W	n_period(A6),D2
	MOVE.B	n_glissfunk(A6),D0
	AND.B	#$0F,D0
	BEQ.S	mt_GlissSkip
	MOVEQ	#0,D0
	MOVE.B	n_finetune(A6),D0
	MULU	#36*2,D0
	LEA	mt_PeriodTable(PC),A0
	ADD.W	D0,A0
	MOVEQ	#0,D0
mt_GlissLoop
	CMP.W	(A0,D0.W),D2
	BHS.S	mt_GlissFound			
	ADDQ.W	#2,D0
	CMP.W	#36*2,D0
	BLO.S	mt_GlissLoop
	MOVEQ	#35*2,D0
mt_GlissFound
	MOVE.W	(A0,D0.W),D2
mt_GlissSkip
	MOVE.W	D2,6(A5) ; Set period
	RTS

mt_Vibrato
	MOVE.B	n_cmdlo(A6),D0
	BEQ.S	mt_Vibrato2
	MOVE.B	n_vibratocmd(A6),D2
	AND.B	#$0F,D0
	BEQ.S	mt_vibskip
	AND.B	#$F0,D2
	OR.B	D0,D2
mt_vibskip
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$F0,D0
	BEQ.S	mt_vibskip2
	AND.B	#$0F,D2
	OR.B	D0,D2
mt_vibskip2
	MOVE.B	D2,n_vibratocmd(A6)
mt_Vibrato2
	MOVE.B	n_vibratopos(A6),D0
	LEA	mt_VibratoTable(PC),A4
	LSR.W	#2,D0
	AND.W	#$001F,D0
	MOVEQ	#0,D2
	MOVE.B	n_wavecontrol(A6),D2
	AND.B	#$03,D2
	BEQ.S	mt_vib_sine
	LSL.B	#3,D0
	CMP.B	#1,D2
	BEQ.S	mt_vib_rampdown
	MOVE.B	#255,D2
	BRA.S	mt_vib_set
mt_vib_rampdown
	TST.B	n_vibratopos(A6)
	BPL.S	mt_vib_rampdown2
	MOVE.B	#255,D2
	SUB.B	D0,D2
	BRA.S	mt_vib_set
mt_vib_rampdown2
	MOVE.B	D0,D2
	BRA.S	mt_vib_set
mt_vib_sine
	MOVE.B	0(A4,D0.W),D2
mt_vib_set
	MOVE.B	n_vibratocmd(A6),D0
	AND.W	#15,D0
	MULU	D0,D2
	LSR.W	#7,D2
	MOVE.W	n_period(A6),D0
	TST.B	n_vibratopos(A6)
	BMI.S	mt_VibratoNeg
	ADD.W	D2,D0
	BRA.S	mt_Vibrato3
mt_VibratoNeg
	SUB.W	D2,D0
mt_Vibrato3
	MOVE.W	D0,6(A5)
	MOVE.B	n_vibratocmd(A6),D0
	LSR.W	#2,D0
	AND.W	#$003C,D0
	ADD.B	D0,n_vibratopos(A6)
	RTS

mt_TonePlusVolSlide
	BSR	mt_TonePortNoChange
	BRA	mt_VolumeSlide

mt_VibratoPlusVolSlide
	BSR.S	mt_Vibrato2
	BRA	mt_VolumeSlide

mt_Tremolo
	MOVE.B	n_cmdlo(A6),D0
	BEQ.S	mt_Tremolo2
	MOVE.B	n_tremolocmd(A6),D2
	AND.B	#$0F,D0
	BEQ.S	mt_treskip
	AND.B	#$F0,D2
	OR.B	D0,D2
mt_treskip
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$F0,D0
	BEQ.S	mt_treskip2
	AND.B	#$0F,D2
	OR.B	D0,D2
mt_treskip2
	MOVE.B	D2,n_tremolocmd(A6)
mt_Tremolo2
	MOVE.B	n_tremolopos(A6),D0
	LEA	mt_VibratoTable(PC),A4
	LSR.W	#2,D0
	AND.W	#$001F,D0
	MOVEQ	#0,D2
	MOVE.B	n_wavecontrol(A6),D2
	LSR.B	#4,D2
	AND.B	#$03,D2
	BEQ.S	mt_tre_sine
	LSL.B	#3,D0
	CMP.B	#1,D2
	BEQ.S	mt_tre_rampdown
	MOVE.B	#255,D2
	BRA.S	mt_tre_set
mt_tre_rampdown
	TST.B	n_vibratopos(A6)
	BPL.S	mt_tre_rampdown2
	MOVE.B	#255,D2
	SUB.B	D0,D2
	BRA.S	mt_tre_set
mt_tre_rampdown2
	MOVE.B	D0,D2
	BRA.S	mt_tre_set
mt_tre_sine
	MOVE.B	0(A4,D0.W),D2
mt_tre_set
	MOVE.B	n_tremolocmd(A6),D0
	AND.W	#15,D0
	MULU	D0,D2
	LSR.W	#6,D2
	MOVEQ	#0,D0
	MOVE.B	n_volume(A6),D0
	TST.B	n_tremolopos(A6)
	BMI.S	mt_TremoloNeg
	ADD.W	D2,D0
	BRA.S	mt_Tremolo3
mt_TremoloNeg
	SUB.W	D2,D0
mt_Tremolo3
	BPL.S	mt_TremoloSkip
	CLR.W	D0
mt_TremoloSkip
	CMP.W	#$40,D0
	BLS.S	mt_TremoloOk
	MOVE.W	#$40,D0
mt_TremoloOk
	MOVE.W	D0,8(A5)
	MOVE.B	n_tremolocmd(A6),D0
	LSR.W	#2,D0
	AND.W	#$003C,D0
	ADD.B	D0,n_tremolopos(A6)
	RTS

mt_SampleOffset
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	BEQ.S	mt_sononew
	MOVE.B	D0,n_sampleoffset(A6)
mt_sononew
	MOVE.B	n_sampleoffset(A6),D0
	LSL.W	#7,D0
	CMP.W	n_length(A6),D0
	BGE.S	mt_sofskip
	SUB.W	D0,n_length(A6)
	LSL.W	#1,D0
	ADD.L	D0,n_start(A6)
	RTS
mt_sofskip
	MOVE.W	#$0001,n_length(A6)
	RTS

mt_VolumeSlide
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	LSR.B	#4,D0
	TST.B	D0
	BEQ.S	mt_VolSlideDown
mt_VolSlideUp
	ADD.B	D0,n_volume(A6)
	CMP.B	#$40,n_volume(A6)
	BMI.S	mt_vsuskip
	MOVE.B	#$40,n_volume(A6)
mt_vsuskip
	MOVE.B	n_volume(A6),D0
	MOVE.W	D0,8(A5)
	RTS

mt_VolSlideDown
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
mt_VolSlideDown2
	SUB.B	D0,n_volume(A6)
	BPL.S	mt_vsdskip
	CLR.B	n_volume(A6)
mt_vsdskip
	MOVE.B	n_volume(A6),D0
	MOVE.W	D0,8(A5)
	RTS

mt_PositionJump
	MOVE.B	n_cmdlo(A6),D0
	SUBQ.B	#1,D0
	MOVE.B	D0,mt_SongPos
mt_pj2	CLR.B	mt_PBreakPos
	ST 	mt_PosJumpFlag
	RTS

mt_VolumeChange
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	CMP.B	#$40,D0
	BLS.S	mt_VolumeOk
	MOVEQ	#$40,D0
mt_VolumeOk
	MOVE.B	D0,n_volume(A6)
	MOVE.W	D0,8(A5)
	RTS

mt_PatternBreak
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	MOVE.L	D0,D2
	LSR.B	#4,D0
	MULU	#10,D0
	AND.B	#$0F,D2
	ADD.B	D2,D0
	CMP.B	#63,D0
	BHI.S	mt_pj2
	MOVE.B	D0,mt_PBreakPos
	ST	mt_PosJumpFlag
	RTS

mt_SetSpeed
	MOVE.B	3(A6),D0
	BEQ	mt_Return2
	CLR.B	mt_counter
	MOVE.B	D0,mt_speed
	RTS

mt_CheckMoreEfx
	BSR	mt_UpdateFunk
	MOVE.B	2(A6),D0
	AND.B	#$0F,D0
	CMP.B	#$9,D0
	BEQ	mt_SampleOffset
	CMP.B	#$B,D0
	BEQ	mt_PositionJump
	CMP.B	#$D,D0
	BEQ.S	mt_PatternBreak
	CMP.B	#$E,D0
	BEQ.S	mt_E_Commands
	CMP.B	#$F,D0
	BEQ.S	mt_SetSpeed
	CMP.B	#$C,D0
	BEQ	mt_VolumeChange
	BRA	mt_PerNop

mt_E_Commands
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$F0,D0
	LSR.B	#4,D0
	BEQ.S	mt_FilterOnOff
	CMP.B	#1,D0
	BEQ	mt_FinePortaUp
	CMP.B	#2,D0
	BEQ	mt_FinePortaDown
	CMP.B	#3,D0
	BEQ.S	mt_SetGlissControl
	CMP.B	#4,D0
	BEQ	mt_SetVibratoControl
	CMP.B	#5,D0
	BEQ	mt_SetFineTune
	CMP.B	#6,D0
	BEQ	mt_JumpLoop
	CMP.B	#7,D0
	BEQ	mt_SetTremoloControl
	CMP.B	#9,D0
	BEQ	mt_RetrigNote
	CMP.B	#$A,D0
	BEQ	mt_VolumeFineUp
	CMP.B	#$B,D0
	BEQ	mt_VolumeFineDown
	CMP.B	#$C,D0
	BEQ	mt_NoteCut
	CMP.B	#$D,D0
	BEQ	mt_NoteDelay
	CMP.B	#$E,D0
	BEQ	mt_PatternDelay
	CMP.B	#$F,D0
	BEQ	mt_FunkIt
	RTS

mt_FilterOnOff
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#1,D0
	ASL.B	#1,D0
	AND.B	#$FD,$BFE001
	OR.B	D0,$BFE001
	RTS	

mt_SetGlissControl
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
	AND.B	#$F0,n_glissfunk(A6)
	OR.B	D0,n_glissfunk(A6)
	RTS

mt_SetVibratoControl
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
	AND.B	#$F0,n_wavecontrol(A6)
	OR.B	D0,n_wavecontrol(A6)
	RTS

mt_SetFineTune
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
	MOVE.B	D0,n_finetune(A6)
	RTS

mt_JumpLoop
	TST.B	mt_counter
	BNE	mt_Return2
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
	BEQ.S	mt_SetLoop
	TST.B	n_loopcount(A6)
	BEQ.S	mt_jumpcnt
	SUBQ.B	#1,n_loopcount(A6)
	BEQ	mt_Return2
mt_jmploop	MOVE.B	n_pattpos(A6),mt_PBreakPos
	ST	mt_PBreakFlag
	RTS

mt_jumpcnt
	MOVE.B	D0,n_loopcount(A6)
	BRA.S	mt_jmploop

mt_SetLoop
	MOVE.W	mt_PatternPos(PC),D0
	LSR.W	#4,D0
	MOVE.B	D0,n_pattpos(A6)
	RTS

mt_SetTremoloControl
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
	LSL.B	#4,D0
	AND.B	#$0F,n_wavecontrol(A6)
	OR.B	D0,n_wavecontrol(A6)
	RTS

mt_RetrigNote
	MOVE.L	D1,-(SP)
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
	BEQ.S	mt_rtnend
	MOVEQ	#0,D1
	MOVE.B	mt_counter(PC),D1
	BNE.S	mt_rtnskp
	MOVE.W	(A6),D1
	AND.W	#$0FFF,D1
	BNE.S	mt_rtnend
	MOVEQ	#0,D1
	MOVE.B	mt_counter(PC),D1
mt_rtnskp
	DIVU	D0,D1
	SWAP	D1
	TST.W	D1
	BNE.S	mt_rtnend
mt_DoRetrig
	MOVE.W	n_dmabit(A6),$DFF096	; Channel DMA off
	MOVE.L	n_start(A6),(A5)	; Set sampledata pointer
	MOVE.W	n_length(A6),4(A5)	; Set length
	MOVE.W	#DMAWAIT,D0
mt_rtnloop1
	DBRA	D0,mt_rtnloop1
	MOVE.W	n_dmabit(A6),D0
	BSET	#15,D0
	MOVE.W	D0,$DFF096
	MOVE.W	#DMAWAIT,D0
mt_rtnloop2
	DBRA	D0,mt_rtnloop2
	MOVE.L	n_loopstart(A6),(A5)
	MOVE.L	n_replen(A6),4(A5)
mt_rtnend
	MOVE.L	(SP)+,D1
	RTS

mt_VolumeFineUp
	TST.B	mt_counter
	BNE	mt_Return2
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$F,D0
	BRA	mt_VolSlideUp

mt_VolumeFineDown
	TST.B	mt_counter
	BNE	mt_Return2
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
	BRA	mt_VolSlideDown2

mt_NoteCut
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
	CMP.B	mt_counter(PC),D0
	BNE	mt_Return2
	CLR.B	n_volume(A6)
	MOVE.W	#0,8(A5)
	RTS

mt_NoteDelay
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
	CMP.B	mt_counter,D0
	BNE	mt_Return2
	MOVE.W	(A6),D0
	BEQ	mt_Return2
	MOVE.L	D1,-(SP)
	BRA	mt_DoRetrig

mt_PatternDelay
	TST.B	mt_counter
	BNE	mt_Return2
	MOVEQ	#0,D0
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
	TST.B	mt_PattDelTime2
	BNE	mt_Return2
	ADDQ.B	#1,D0
	MOVE.B	D0,mt_PattDelTime
	RTS

mt_FunkIt
	TST.B	mt_counter
	BNE	mt_Return2
	MOVE.B	n_cmdlo(A6),D0
	AND.B	#$0F,D0
	LSL.B	#4,D0
	AND.B	#$0F,n_glissfunk(A6)
	OR.B	D0,n_glissfunk(A6)
	TST.B	D0
	BEQ	mt_Return2
mt_UpdateFunk
	MOVEM.L	D1/A0,-(SP)
	MOVEQ	#0,D0
	MOVE.B	n_glissfunk(A6),D0
	LSR.B	#4,D0
	BEQ.S	mt_funkend
	LEA	mt_FunkTable(PC),A0
	MOVE.B	(A0,D0.W),D0
	ADD.B	D0,n_funkoffset(A6)
	BTST	#7,n_funkoffset(A6)
	BEQ.S	mt_funkend
	CLR.B	n_funkoffset(A6)

	MOVE.L	n_loopstart(A6),D0
	MOVEQ	#0,D1
	MOVE.W	n_replen(A6),D1
	ADD.L	D1,D0
	ADD.L	D1,D0
	MOVE.L	n_wavestart(A6),A0
	ADDQ.L	#1,A0
	CMP.L	D0,A0
	BLO.S	mt_funkok
	MOVE.L	n_loopstart(A6),A0
mt_funkok
	MOVE.L	A0,n_wavestart(A6)
	MOVEQ	#-1,D0
	SUB.B	(A0),D0
	MOVE.B	D0,(A0)
mt_funkend
	MOVEM.L	(SP)+,D1/A0
	RTS


mt_FunkTable dc.b 0,5,6,7,8,10,11,13,16,19,22,26,32,43,64,128

mt_VibratoTable
	dc.b   0, 24, 49, 74, 97,120,141,161
	dc.b 180,197,212,224,235,244,250,253
	dc.b 255,253,250,244,235,224,212,197
	dc.b 180,161,141,120, 97, 74, 49, 24

mt_PeriodTable
; Tuning 0, Normal
	dc.w	856,808,762,720,678,640,604,570,538,508,480,453
	dc.w	428,404,381,360,339,320,302,285,269,254,240,226
	dc.w	214,202,190,180,170,160,151,143,135,127,120,113
; Tuning 1
	dc.w	850,802,757,715,674,637,601,567,535,505,477,450
	dc.w	425,401,379,357,337,318,300,284,268,253,239,225
	dc.w	213,201,189,179,169,159,150,142,134,126,119,113
; Tuning 2
	dc.w	844,796,752,709,670,632,597,563,532,502,474,447
	dc.w	422,398,376,355,335,316,298,282,266,251,237,224
	dc.w	211,199,188,177,167,158,149,141,133,125,118,112
; Tuning 3
	dc.w	838,791,746,704,665,628,592,559,528,498,470,444
	dc.w	419,395,373,352,332,314,296,280,264,249,235,222
	dc.w	209,198,187,176,166,157,148,140,132,125,118,111
; Tuning 4
	dc.w	832,785,741,699,660,623,588,555,524,495,467,441
	dc.w	416,392,370,350,330,312,294,278,262,247,233,220
	dc.w	208,196,185,175,165,156,147,139,131,124,117,110
; Tuning 5
	dc.w	826,779,736,694,655,619,584,551,520,491,463,437
	dc.w	413,390,368,347,328,309,292,276,260,245,232,219
	dc.w	206,195,184,174,164,155,146,138,130,123,116,109
; Tuning 6
	dc.w	820,774,730,689,651,614,580,547,516,487,460,434
	dc.w	410,387,365,345,325,307,290,274,258,244,230,217
	dc.w	205,193,183,172,163,154,145,137,129,122,115,109
; Tuning 7
	dc.w	814,768,725,684,646,610,575,543,513,484,457,431
	dc.w	407,384,363,342,323,305,288,272,256,242,228,216
	dc.w	204,192,181,171,161,152,144,136,128,121,114,108
; Tuning -8
	dc.w	907,856,808,762,720,678,640,604,570,538,508,480
	dc.w	453,428,404,381,360,339,320,302,285,269,254,240
	dc.w	226,214,202,190,180,170,160,151,143,135,127,120
; Tuning -7
	dc.w	900,850,802,757,715,675,636,601,567,535,505,477
	dc.w	450,425,401,379,357,337,318,300,284,268,253,238
	dc.w	225,212,200,189,179,169,159,150,142,134,126,119
; Tuning -6
	dc.w	894,844,796,752,709,670,632,597,563,532,502,474
	dc.w	447,422,398,376,355,335,316,298,282,266,251,237
	dc.w	223,211,199,188,177,167,158,149,141,133,125,118
; Tuning -5
	dc.w	887,838,791,746,704,665,628,592,559,528,498,470
	dc.w	444,419,395,373,352,332,314,296,280,264,249,235
	dc.w	222,209,198,187,176,166,157,148,140,132,125,118
; Tuning -4
	dc.w	881,832,785,741,699,660,623,588,555,524,494,467
	dc.w	441,416,392,370,350,330,312,294,278,262,247,233
	dc.w	220,208,196,185,175,165,156,147,139,131,123,117
; Tuning -3
	dc.w	875,826,779,736,694,655,619,584,551,520,491,463
	dc.w	437,413,390,368,347,328,309,292,276,260,245,232
	dc.w	219,206,195,184,174,164,155,146,138,130,123,116
; Tuning -2
	dc.w	868,820,774,730,689,651,614,580,547,516,487,460
	dc.w	434,410,387,365,345,325,307,290,274,258,244,230
	dc.w	217,205,193,183,172,163,154,145,137,129,122,115
; Tuning -1
	dc.w	862,814,768,725,684,646,610,575,543,513,484,457
	dc.w	431,407,384,363,342,323,305,288,272,256,242,228
	dc.w	216,203,192,181,171,161,152,144,136,128,121,114

mt_chan1temp	dc.l	0,0,0,0,0,$00010000,0,  0,0,0,0
mt_chan2temp	dc.l	0,0,0,0,0,$00020000,0,  0,0,0,0
mt_chan3temp	dc.l	0,0,0,0,0,$00040000,0,  0,0,0,0
mt_chan4temp	dc.l	0,0,0,0,0,$00080000,0,  0,0,0,0

mt_SampleStarts	dc.l	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
		dc.l	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

mt_SongDataPtr	dc.l 0

mt_speed	dc.b 6
mt_counter	dc.b 0
mt_SongPos	dc.b 0
mt_PBreakPos	dc.b 0
mt_PosJumpFlag	dc.b 0
mt_PBreakFlag	dc.b 0
mt_LowMask	dc.b 0
mt_PattDelTime	dc.b 0
mt_PattDelTime2	dc.b 0,0

mt_PatternPos	dc.w 0
mt_DMACONtemp	dc.w 0

;***************************************
;***  END OF PRO/NOISE/SOUNDTRACKER  ***
;***************************************

;*******************************************
;*** FUTURE COMPOSER V1.3 REPLAY ROUTINE ***
;*******************************************
FC13END_MUSIC:	CLR.W	lbW000748
	CLR.L	$DFF0A6
	CLR.L	$DFF0B6
	CLR.L	$DFF0C6
	CLR.L	$DFF0D6
	MOVE.W	#15,$DFF096
	BCLR	#1,$BFE001
	RTS

FC13INIT_MUSIC:	MOVE.W	#1,lbW000748
	BSET	#1,$BFE001
	move.l	_CurrentMod,a0
	lea	20(a0),a0
	LEA	$64(A0),A1
	MOVE.L	A1,lbL00088A
	MOVE.L	A0,A1
	ADD.L	8(A0),A1
	MOVE.L	A1,lbL00088E
	MOVE.L	A0,A1
	ADD.L	$10(A0),A1
	MOVE.L	A1,lbL000892
	MOVE.L	A0,A1
	ADD.L	$18(A0),A1
	MOVE.L	A1,lbL000896
	MOVE.L	4(A0),D0
	DIVU	#13,D0
	LEA	$28(A0),A1
	LEA	lbL00094E(PC),A2
	MOVEQ	#9,D1
lbC0000CA	MOVE.W	(A1)+,(A2)+
	MOVE.L	(A1)+,(A2)+
	ADDQ.W	#4,A2
	DBRA	D1,lbC0000CA

	MOVEQ	#0,D2
	MOVE.L	A0,D1
	ADD.L	$20(A0),D1
	SUB.L	#lbL000B84,D1
	LEA	lbL00094A(PC),A0
	MOVE.L	D1,(A0)+
	MOVEQ	#8,D3
lbC0000EA	MOVE.W	(A0),D2
	ADD.L	D2,D1
	ADD.L	D2,D1
	ADDQ.W	#6,A0
	MOVE.L	D1,(A0)+
	DBRA	D3,lbC0000EA

	MOVE.L	lbL00088A(PC),A0
	MOVEQ	#0,D2
	MOVE.B	12(A0),D2
	BNE.S	lbC000108
	MOVE.B	#3,D2
lbC000108	MOVE.W	D2,lbW000744
	MOVE.W	D2,lbW000746
	CLR.W	lbW00074E
	MOVE.W	#15,$DFF096
	MOVE.W	#$780,$DFF09A
	MOVEQ	#0,D7
	MULU	#13,D0
	MOVEQ	#3,D6
	LEA	lbL000752(PC),A0
	LEA	lbL00089A(PC),A1
	LEA	lbL00087A(PC),A2
lbC00013E	MOVE.L	A1,10(A0)
	MOVE.L	A1,$12(A0)
	CLR.L	14(A0)
	CLR.B	$2D(A0)
	CLR.B	$2F(A0)
	CLR.W	8(A0)
	CLR.L	$30(A0)
	MOVE.B	#1,$17(A0)
	MOVE.B	#1,$18(A0)
	CLR.B	$19(A0)
	CLR.L	$1A(A0)
	CLR.W	$1E(A0)
	MOVEQ	#0,D3
	MOVE.W	(A2)+,D1
	MOVE.W	(A2)+,D3
	DIVU	#3,D3
	MOVE.B	D3,$20(A0)
	MULU	#3,D3
	AND.L	#$FF,D3
	AND.L	#$FF,D1
	ADD.L	#$DFF0A0,D1
	MOVE.L	D1,A6
	MOVE.L	#0,(A6)
	MOVE.W	#$100,4(A6)
	MOVE.W	#0,6(A6)
	MOVE.W	#0,8(A6)
	MOVE.L	D1,$3C(A0)
	CLR.W	$40(A0)
	MOVE.L	lbL00088A(PC),(A0)
	MOVE.L	lbL00088A(PC),$34(A0)
	ADD.L	D0,$34(A0)
	ADD.L	D3,$34(A0)
	ADD.L	D7,(A0)
	ADD.L	D3,(A0)
	MOVE.W	#13,6(A0)
	MOVE.L	(A0),A3
	MOVE.B	(A3),D1
	AND.L	#$FF,D1
	LSL.W	#6,D1
	MOVE.L	lbL00088E(PC),A4
	ADD.W	D1,A4
	MOVE.L	A4,$22(A0)
	CLR.L	$26(A0)
	MOVE.B	#1,$21(A0)
	MOVE.B	#2,$2A(A0)
	MOVE.B	1(A3),$2C(A0)
	MOVE.B	2(A3),$16(A0)
	CLR.B	$2B(A0)
	CLR.B	$2D(A0)
	CLR.W	$38(A0)
	ADD.W	#$4A,A0
	DBRA	D6,lbC00013E

	RTS

FC13PLAY_MUSIC	LEA	lbL000734(PC),A6
	TST.W	lbW000748
	BNE.S	lbC00022A
	RTS

lbC00022A	SUBQ.W	#1,lbW000744
	BNE.S	lbC00025A
	MOVE.W	lbW000746(PC),lbW000744
	LEA	lbL000752(PC),A0
	BSR	lbC00039E
	LEA	lbL00079C(PC),A0
	BSR	lbC00039E
	LEA	lbL0007E6(PC),A0
	BSR	lbC00039E
	LEA	lbL000830(PC),A0
	BSR	lbC00039E
lbC00025A	CLR.W	lbW00074E
	LEA	lbL000752(PC),A0
	BSR	lbC0004B8
	MOVE.W	D0,(A6)+
	MOVE.W	D1,(A6)+
	LEA	lbL00079C(PC),A0
	BSR	lbC0004B8
	MOVE.W	D0,(A6)+
	MOVE.W	D1,(A6)+
	LEA	lbL0007E6(PC),A0
	BSR	lbC0004B8
	MOVE.W	D0,(A6)+
	MOVE.W	D1,(A6)+
	LEA	lbL000830(PC),A0
	BSR	lbC0004B8
	MOVE.W	D0,(A6)+
	MOVE.W	D1,(A6)+
	LEA	lbL000734(PC),A6
	MOVE.W	lbW00074E(PC),D0
	OR.W	#$8000,D0
	MOVE.W	D0,-(SP)
	MOVEQ	#0,D1
	MOVE.L	lbL000796(PC),D2
	MOVE.W	lbW000792(PC),D1
	ADD.L	D1,D2
	MOVE.L	lbL0007E0(PC),D3
	MOVE.W	lbW0007DC(PC),D1
	ADD.L	D1,D3
	MOVE.L	lbL00082A(PC),D4
	MOVE.W	lbW000826(PC),D1
	ADD.L	D1,D4
	MOVE.L	lbL000874(PC),D5
	MOVE.W	lbW000870(PC),D1
	ADD.L	D1,D5
	MOVE.W	lbW000794(PC),D0
	MOVE.W	lbW0007DE(PC),D1
	MOVE.W	lbW000828(PC),D6
	MOVE.W	lbW000872(PC),D7
	MOVE.W	(SP)+,$DFF096
	LEA	lbL000752(PC),A0
	TST.W	$48(A0)
	BEQ	lbC000306
	SUBQ.W	#1,$48(A0)
	CMP.W	#1,$48(A0)
	BNE.S	lbC000306
	CLR.W	$48(A0)
	MOVE.L	D2,$DFF0A0
	MOVE.W	D0,$DFF0A4
lbC000306	LEA	lbL00079C(PC),A0
	TST.W	$48(A0)
	BEQ.S	lbC00032C
	SUBQ.W	#1,$48(A0)
	CMP.W	#1,$48(A0)
	BNE.S	lbC00032C
	CLR.W	$48(A0)
	MOVE.L	D3,$DFF0B0
	MOVE.W	D1,$DFF0B4
lbC00032C	LEA	lbL0007E6(PC),A0
	TST.W	$48(A0)
	BEQ.S	lbC000352
	SUBQ.W	#1,$48(A0)
	CMP.W	#1,$48(A0)
	BNE.S	lbC000352
	CLR.W	$48(A0)
	MOVE.L	D4,$DFF0C0
	MOVE.W	D6,$DFF0C4
lbC000352	LEA	lbL000830(PC),A0
	TST.W	$48(A0)
	BEQ.S	lbC000378
	SUBQ.W	#1,$48(A0)
	CMP.W	#1,$48(A0)
	BNE.S	lbC000378
	CLR.W	$48(A0)
	MOVE.L	D5,$DFF0D0
	MOVE.W	D7,$DFF0D4
lbC000378	LEA	$DFF0A6,A5
	MOVE.W	(A6)+,(A5)
	MOVE.W	(A6)+,2(A5)
	MOVE.W	(A6)+,$10(A5)
	MOVE.W	(A6)+,$12(A5)
	MOVE.W	(A6)+,$20(A5)
	MOVE.W	(A6)+,$22(A5)
	MOVE.W	(A6)+,$30(A5)
	MOVE.W	(A6)+,$32(A5)
	RTS

lbC00039E	MOVEQ	#0,D5
	MOVE.L	$22(A0),A1
	ADD.W	$28(A0),A1
	CMP.W	#$40,$28(A0)
	BNE.S	lbC000410
	MOVE.L	(A0),A2
	ADD.W	6(A0),A2
	CMP.L	$34(A0),A2
	BNE.S	lbC0003C2
	MOVE.W	D5,6(A0)
	MOVE.L	(A0),A2
lbC0003C2	MOVEQ	#0,D1
	ADDQ.B	#1,lbB000750
	CMP.B	#4,lbB000750
	BNE.S	lbC0003EC
	MOVE.B	D5,lbB000750
	MOVE.B	-1(A1),D1
	BEQ.S	lbC0003EC
	MOVE.W	D1,lbW000744
	MOVE.W	D1,lbW000746
lbC0003EC	MOVE.B	(A2),D1
	MOVE.B	1(A2),$2C(A0)
	MOVE.B	2(A2),$16(A0)
	MOVE.W	D5,$28(A0)
	LSL.W	#6,D1
	ADD.L	lbL00088E(PC),D1
	MOVE.L	D1,$22(A0)
	ADD.W	#13,6(A0)
	MOVE.L	D1,A1
lbC000410	MOVE.B	1(A1),D1
	MOVE.B	(A1)+,D0
	BNE.S	lbC000420
	AND.W	#$C0,D1
	BEQ.S	lbC000438
	BRA.S	lbC000424

lbC000420	MOVE.W	D5,$38(A0)
lbC000424	MOVE.B	D5,$2F(A0)
	MOVE.B	(A1),$1F(A0)
	BTST	#7,D1
	BEQ.S	lbC000438
	MOVE.B	2(A1),$2F(A0)
lbC000438	AND.W	#$7F,D0
	BEQ.S	lbC0004B2
	MOVE.B	D0,8(A0)
	MOVE.B	(A1),9(A0)
	MOVE.B	$20(A0),D2
	MOVEQ	#0,D3
	BSET	D2,D3
	OR.W	D3,lbW00074E
	MOVE.W	D3,$DFF096
	MOVE.B	(A1),D1
	AND.W	#$3F,D1
	ADD.B	$16(A0),D1
	MOVE.L	lbL000896(PC),A2
	LSL.W	#6,D1
	ADD.W	D1,A2
	MOVE.W	D5,$10(A0)
	MOVE.B	(A2),$17(A0)
	MOVE.B	(A2)+,$18(A0)
	MOVE.B	(A2)+,D1
	AND.W	#$FF,D1
	MOVE.B	(A2)+,$1B(A0)
	MOVE.B	#$40,$2E(A0)
	MOVE.B	(A2)+,D0
	MOVE.B	D0,$1C(A0)
	MOVE.B	D0,$1D(A0)
	MOVE.B	(A2)+,$1E(A0)
	MOVE.L	A2,10(A0)
	MOVE.L	lbL000892(PC),A2
	LSL.W	#6,D1
	ADD.W	D1,A2
	MOVE.L	A2,$12(A0)
	MOVE.W	D5,$32(A0)
	MOVE.B	D5,$1A(A0)
	MOVE.B	D5,$19(A0)
lbC0004B2	ADDQ.W	#2,$28(A0)
	RTS

lbC0004B8	MOVEQ	#0,D7
lbC0004BA	TST.B	$1A(A0)
	BEQ.S	lbC0004C8
	SUBQ.B	#1,$1A(A0)
	BRA	lbC000616

lbC0004C8	MOVE.L	$12(A0),A1
	ADD.W	$32(A0),A1
lbC0004D0	CMP.B	#$E1,(A1)
	BEQ	lbC000616
	CMP.B	#$E0,(A1)
	BNE.S	lbC0004F0
	MOVE.B	1(A1),D0
	AND.W	#$3F,D0
	MOVE.W	D0,$32(A0)
	MOVE.L	$12(A0),A1
	ADD.W	D0,A1
lbC0004F0	CMP.B	#$E2,(A1)
	BNE.S	lbC000566
	MOVEQ	#0,D0
	MOVEQ	#0,D1
	MOVE.B	$20(A0),D1
	BSET	D1,D0
	OR.W	D0,lbW00074E
	MOVE.W	D0,$DFF096
	MOVE.B	1(A1),D0
	AND.W	#$FF,D0
	LEA	lbL00094A(PC),A4
	ADD.W	D0,D0
	MOVE.W	D0,D1
	ADD.W	D1,D1
	ADD.W	D1,D1
	ADD.W	D1,D0
	ADD.W	D0,A4
	MOVE.L	$3C(A0),A3
	MOVE.L	(A4),D1
	ADD.L	#lbL000B84,D1
	MOVE.L	D1,(A3)
	MOVE.L	D1,$44(A0)
	MOVE.W	4(A4),4(A3)
	MOVE.L	6(A4),$40(A0)
	SWAP	D1
	MOVE.W	#3,$48(A0)
	TST.W	D1
	BNE.S	lbC000554
	MOVE.W	#2,$48(A0)
lbC000554	CLR.W	$10(A0)
	MOVE.B	#1,$17(A0)
	ADDQ.W	#2,$32(A0)
	BRA	lbC000606

lbC000566	CMP.B	#$E4,(A1)
	BNE.S	lbC0005BA
	MOVE.B	1(A1),D0
	AND.W	#$FF,D0
	LEA	lbL00094A(PC),A4
	ADD.W	D0,D0
	MOVE.W	D0,D1
	ADD.W	D1,D1
	ADD.W	D1,D1
	ADD.W	D1,D0
	ADD.W	D0,A4
	MOVE.L	$3C(A0),A3
	MOVE.L	(A4),D1
	ADD.L	#lbL000B84,D1
	MOVE.L	D1,(A3)
	MOVE.L	D1,$44(A0)
	MOVE.W	4(A4),4(A3)
	MOVE.L	6(A4),$40(A0)
	SWAP	D1
	MOVE.W	#3,$48(A0)
	TST.W	D1
	BNE.S	lbC0005B4
	MOVE.W	#2,$48(A0)
lbC0005B4	ADDQ.W	#2,$32(A0)
	BRA.S	lbC000606

lbC0005BA	CMP.B	#$E7,(A1)
	BNE.S	lbC0005DC
	MOVE.B	1(A1),D0
	AND.W	#$FF,D0
	LSL.W	#6,D0
	MOVE.L	lbL000892(PC),A1
	ADD.W	D0,A1
	MOVE.L	A1,$12(A0)
	MOVE.W	D7,$32(A0)
	BRA	lbC0004D0

lbC0005DC	CMP.B	#$E8,(A1)
	BNE.S	lbC0005F0
	MOVE.B	1(A1),$1A(A0)
	ADDQ.W	#2,$32(A0)
	BRA	lbC0004BA

lbC0005F0	CMP.B	#$E3,(A1)
	BNE.S	lbC000606
	ADDQ.W	#3,$32(A0)
	MOVE.B	1(A1),$1B(A0)
	MOVE.B	2(A1),$1C(A0)
lbC000606	MOVE.L	$12(A0),A1
	ADD.W	$32(A0),A1
	MOVE.B	(A1),$2B(A0)
	ADDQ.W	#1,$32(A0)
lbC000616	TST.B	$19(A0)
	BEQ.S	lbC000622
	SUBQ.B	#1,$19(A0)
	BRA.S	lbC000670

lbC000622	SUBQ.B	#1,$17(A0)
	BNE.S	lbC000670
	MOVE.B	$18(A0),$17(A0)
lbC00062E	MOVE.L	10(A0),A1
	ADD.W	$10(A0),A1
	MOVE.B	(A1),D0
	CMP.B	#$E8,D0
	BNE.S	lbC00064A
	ADDQ.W	#2,$10(A0)
	MOVE.B	1(A1),$19(A0)
	BRA.S	lbC000616

lbC00064A	CMP.B	#$E1,D0
	BEQ.S	lbC000670
	CMP.B	#$E0,D0
	BNE.S	lbC000668
	MOVE.B	1(A1),D0
	AND.L	#$3F,D0
	SUBQ.B	#5,D0
	MOVE.W	D0,$10(A0)
	BRA.S	lbC00062E

lbC000668	MOVE.B	(A1),$2D(A0)
	ADDQ.W	#1,$10(A0)
lbC000670	MOVE.B	$2B(A0),D0
	BMI.S	lbC00067E
	ADD.B	8(A0),D0
	ADD.B	$2C(A0),D0
lbC00067E	AND.W	#$7F,D0
	LEA	lbL0008A2(PC),A1
	ADD.W	D0,D0
	MOVE.W	D0,D1
	ADD.W	D0,A1
	MOVE.W	(A1),D0
	MOVE.B	$2E(A0),D7
	TST.B	$1E(A0)
	BEQ.S	lbC00069E
	SUBQ.B	#1,$1E(A0)
	BRA.S	lbC0006F4

lbC00069E	MOVE.B	D1,D5
	MOVE.B	$1C(A0),D4
	ADD.B	D4,D4
	MOVE.B	$1D(A0),D1
	TST.B	D7
	BPL.S	lbC0006B4
	BTST	#0,D7
	BNE.S	lbC0006DA
lbC0006B4	BTST	#5,D7
	BNE.S	lbC0006C8
	SUB.B	$1B(A0),D1
	BCC.S	lbC0006D6
	BSET	#5,D7
	MOVEQ	#0,D1
	BRA.S	lbC0006D6

lbC0006C8	ADD.B	$1B(A0),D1
	CMP.B	D4,D1
	BCS.S	lbC0006D6
	BCLR	#5,D7
	MOVE.B	D4,D1
lbC0006D6	MOVE.B	D1,$1D(A0)
lbC0006DA	LSR.B	#1,D4
	SUB.B	D4,D1
	BCC.S	lbC0006E4
	SUB.W	#$100,D1
lbC0006E4	ADD.B	#$A0,D5
	BCS.S	lbC0006F2
lbC0006EA	ADD.W	D1,D1
	ADD.B	#$18,D5
	BCC.S	lbC0006EA
lbC0006F2	ADD.W	D1,D0
lbC0006F4	EOR.B	#1,D7
	MOVE.B	D7,$2E(A0)
	MOVEQ	#0,D1
	MOVE.B	$2F(A0),D1
	BEQ.S	lbC000714
	CMP.B	#$1F,D1
	BLS.S	lbC000710
	AND.W	#$1F,D1
	NEG.W	D1
lbC000710	SUB.W	D1,$38(A0)
lbC000714	ADD.W	$38(A0),D0
	CMP.W	#$70,D0
	BHI.S	lbC000722
	MOVE.W	#$71,D0
lbC000722	CMP.W	#$6B0,D0
	BLS.S	lbC00072C
	MOVE.W	#$6B0,D0
lbC00072C	MOVEQ	#0,D1
	MOVE.B	$2D(A0),D1
	RTS
lbL000734	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbW000744	dc.w	0
lbW000746	dc.w	0
lbW000748	dc.w	0
	dc.w	0
	dc.w	0
lbW00074E	dc.w	0
lbB000750	dc.b	0
	dc.b	0
lbL000752	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbW000792	dc.w	0
lbW000794	dc.w	0
lbL000796	dc.l	0
	dc.w	0
lbL00079C	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbW0007DC	dc.w	0
lbW0007DE	dc.w	0
lbL0007E0	dc.l	0
	dc.w	0
lbL0007E6	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbW000826	dc.w	0
lbW000828	dc.w	0
lbL00082A	dc.l	0
	dc.w	0
lbL000830	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0
lbW000870	dc.w	0
lbW000872	dc.w	0
lbL000874	dc.l	0
	dc.w	0
lbL00087A	dc.l	0
	dc.l	$100003
	dc.l	$200006
	dc.l	$300009
lbL00088A	dc.l	0
lbL00088E	dc.l	0
lbL000892	dc.l	0
lbL000896	dc.l	0
lbL00089A	dc.l	$1000000
	dc.l	$E1
lbL0008A2	dc.l	$6B00650
	dc.l	$5F405A0
	dc.l	$54C0500
	dc.l	$4B80474
	dc.l	$43403F8
	dc.l	$3C0038A
	dc.l	$3580328
	dc.l	$2FA02D0
	dc.l	$2A60280
	dc.l	$25C023A
	dc.l	$21A01FC
	dc.l	$1E001C5
	dc.l	$1AC0194
	dc.l	$17D0168
	dc.l	$1530140
	dc.l	$12E011D
	dc.l	$10D00FE
	dc.l	$F000E2
	dc.l	$D600CA
	dc.l	$BE00B4
	dc.l	$AA00A0
	dc.l	$97008F
	dc.l	$87007F
	dc.l	$780071
	dc.l	$710071
	dc.l	$710071
	dc.l	$710071
	dc.l	$710071
	dc.l	$710071
	dc.l	$710071
	dc.l	$D600CA0
	dc.l	$BE80B40
	dc.l	$A980A00
	dc.l	$97008E8
	dc.l	$86807F0
	dc.l	$7800714
	dc.l	$1AC01940
	dc.l	$17D01680
	dc.l	$15301400
	dc.l	$12E011D0
	dc.l	$10D00FE0
	dc.l	$F000E28
lbL00094A	dc.l	0
lbL00094E	dc.l	0
	dc.l	$10000
	dc.l	0
	dc.l	1
	dc.l	0
	dc.l	0
	dc.l	$10000
	dc.l	0
	dc.l	1
	dc.l	0
	dc.l	0
	dc.l	$10000
	dc.l	0
	dc.l	1
	dc.l	0
	dc.l	0
	dc.l	$10000
	dc.l	0
	dc.l	1
	dc.l	0
	dc.l	0
	dc.l	$10000
	dc.l	0
	dc.l	1
	dc.l	0
	dc.l	$100000
	dc.l	$100000
	dc.l	$200010
	dc.l	$10
	dc.l	$40
	dc.l	$100000
	dc.l	$100000
	dc.l	$600010
	dc.l	$10
	dc.l	$80
	dc.l	$100000
	dc.l	$100000
	dc.l	$A00010
	dc.l	$10
	dc.l	$C0
	dc.l	$100000
	dc.l	$100000
	dc.l	$E00010
	dc.l	$10
	dc.l	$100
	dc.l	$100000
	dc.l	$100000
	dc.l	$1200010
	dc.l	$10
	dc.l	$140
	dc.l	$100000
	dc.l	$100000
	dc.l	$1600010
	dc.l	$10
	dc.l	$180
	dc.l	$100000
	dc.l	$100000
	dc.l	$1A00010
	dc.l	$10
	dc.l	$1C0
	dc.l	$100000
	dc.l	$100000
	dc.l	$1E00010
	dc.l	$10
	dc.l	$200
	dc.l	$100000
	dc.l	$100000
	dc.l	$2200010
	dc.l	$10
	dc.l	$240
	dc.l	$100000
	dc.l	$100000
	dc.l	$2600010
	dc.l	$10
	dc.l	$280
	dc.l	$100000
	dc.l	$100000
	dc.l	$2A00010
	dc.l	$10
	dc.l	$2C0
	dc.l	$100000
	dc.l	$100000
	dc.l	$2E00010
	dc.l	$10
	dc.l	$300
	dc.l	$100000
	dc.l	$100000
	dc.l	$3200010
	dc.l	$10
	dc.l	$340
	dc.l	$100000
	dc.l	$100000
	dc.l	$3600010
	dc.l	$10
	dc.l	$380
	dc.l	$100000
	dc.l	$100000
	dc.l	$3A00010
	dc.l	$10
	dc.l	$3C0
	dc.l	$100000
	dc.l	$100000
	dc.l	$3E00010
	dc.l	$10
	dc.l	$400
	dc.l	$80000
	dc.l	$80000
	dc.l	$4100008
	dc.l	8
	dc.l	$420
	dc.l	$80000
	dc.l	$80000
	dc.l	$4300008
	dc.l	8
	dc.l	$440
	dc.l	$80000
	dc.l	$80000
	dc.l	$4500008
	dc.l	8
	dc.l	$460
	dc.l	$80000
	dc.l	$80000
	dc.l	$4700008
	dc.l	8
	dc.l	$480
	dc.l	$100000
	dc.l	$100000
	dc.l	$4A00008
	dc.l	8
	dc.l	$4B0
	dc.l	$100000
	dc.l	$100000
	dc.l	$4D00010
	dc.l	$10
	dc.l	$4F0
	dc.l	$80000
	dc.l	$80000
	dc.l	$5000008
	dc.l	8
	dc.l	$510
	dc.l	$180000
	dc.w	$18
lbL000B84	dc.l	$C0C0D0D8
	dc.l	$E0E8F0F8
	dc.l	$F8F0E8
	dc.l	$E0D8D0C8
	dc.l	$3F372F27
	dc.l	$1F170F07
	dc.l	$FF070F17
	dc.l	$1F272F37
	dc.l	$C0C0D0D8
	dc.l	$E0E8F0F8
	dc.l	$F8F0E8
	dc.l	$E0D8D0C8
	dc.l	$C0372F27
	dc.l	$1F170F07
	dc.l	$FF070F17
	dc.l	$1F272F37
	dc.l	$C0C0D0D8
	dc.l	$E0E8F0F8
	dc.l	$F8F0E8
	dc.l	$E0D8D0C8
	dc.l	$C0B82F27
	dc.l	$1F170F07
	dc.l	$FF070F17
	dc.l	$1F272F37
	dc.l	$C0C0D0D8
	dc.l	$E0E8F0F8
	dc.l	$F8F0E8
	dc.l	$E0D8D0C8
	dc.l	$C0B8B027
	dc.l	$1F170F07
	dc.l	$FF070F17
	dc.l	$1F272F37
	dc.l	$C0C0D0D8
	dc.l	$E0E8F0F8
	dc.l	$F8F0E8
	dc.l	$E0D8D0C8
	dc.l	$C0B8B0A8
	dc.l	$1F170F07
	dc.l	$FF070F17
	dc.l	$1F272F37
	dc.l	$C0C0D0D8
	dc.l	$E0E8F0F8
	dc.l	$F8F0E8
	dc.l	$E0D8D0C8
	dc.l	$C0B8B0A8
	dc.l	$A0170F07
	dc.l	$FF070F17
	dc.l	$1F272F37
	dc.l	$C0C0D0D8
	dc.l	$E0E8F0F8
	dc.l	$F8F0E8
	dc.l	$E0D8D0C8
	dc.l	$C0B8B0A8
	dc.l	$A0980F07
	dc.l	$FF070F17
	dc.l	$1F272F37
	dc.l	$C0C0D0D8
	dc.l	$E0E8F0F8
	dc.l	$F8F0E8
	dc.l	$E0D8D0C8
	dc.l	$C0B8B0A8
	dc.l	$A0989007
	dc.l	$FF070F17
	dc.l	$1F272F37
	dc.l	$C0C0D0D8
	dc.l	$E0E8F0F8
	dc.l	$F8F0E8
	dc.l	$E0D8D0C8
	dc.l	$C0B8B0A8
	dc.l	$A0989088
	dc.l	$FF070F17
	dc.l	$1F272F37
	dc.l	$C0C0D0D8
	dc.l	$E0E8F0F8
	dc.l	$F8F0E8
	dc.l	$E0D8D0C8
	dc.l	$C0B8B0A8
	dc.l	$A0989088
	dc.l	$80070F17
	dc.l	$1F272F37
	dc.l	$C0C0D0D8
	dc.l	$E0E8F0F8
	dc.l	$F8F0E8
	dc.l	$E0D8D0C8
	dc.l	$C0B8B0A8
	dc.l	$A0989088
	dc.l	$80880F17
	dc.l	$1F272F37
	dc.l	$C0C0D0D8
	dc.l	$E0E8F0F8
	dc.l	$F8F0E8
	dc.l	$E0D8D0C8
	dc.l	$C0B8B0A8
	dc.l	$A0989088
	dc.l	$80889017
	dc.l	$1F272F37
	dc.l	$C0C0D0D8
	dc.l	$E0E8F0F8
	dc.l	$F8F0E8
	dc.l	$E0D8D0C8
	dc.l	$C0B8B0A8
	dc.l	$A0989088
	dc.l	$80889098
	dc.l	$1F272F37
	dc.l	$C0C0D0D8
	dc.l	$E0E8F0F8
	dc.l	$F8F0E8
	dc.l	$E0D8D0C8
	dc.l	$C0B8B0A8
	dc.l	$A0989088
	dc.l	$80889098
	dc.l	$A0272F37
	dc.l	$C0C0D0D8
	dc.l	$E0E8F0F8
	dc.l	$F8F0E8
	dc.l	$E0D8D0C8
	dc.l	$C0B8B0A8
	dc.l	$A0989088
	dc.l	$80889098
	dc.l	$A0A82F37
	dc.l	$C0C0D0D8
	dc.l	$E0E8F0F8
	dc.l	$F8F0E8
	dc.l	$E0D8D0C8
	dc.l	$C0B8B0A8
	dc.l	$A0989088
	dc.l	$80889098
	dc.l	$A0A8B037
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$817F7F7F
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81817F7F
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$8181817F
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$817F7F7F
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81817F7F
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$8181817F
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$817F7F7F
	dc.l	$7F7F7F7F
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81817F7F
	dc.l	$7F7F7F7F
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$8181817F
	dc.l	$7F7F7F7F
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$7F7F7F7F
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$81818181
	dc.l	$817F7F7F
	dc.l	$80808080
	dc.l	$80808080
	dc.l	$80808080
	dc.l	$80808080
	dc.l	$80808080
	dc.l	$80808080
	dc.l	$80808080
	dc.l	$80807F7F
	dc.l	$80808080
	dc.l	$80808080
	dc.l	$80808080
	dc.l	$80808080
	dc.l	$80808080
	dc.l	$80808080
	dc.l	$80808080
	dc.l	$8080807F
	dc.l	$80808080
	dc.l	$80808080
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$80808080
	dc.l	$8080807F
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$80808080
	dc.l	$80807F7F
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$80808080
	dc.l	$807F7F7F
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$80808080
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$8080807F
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$80807F7F
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$80807F7F
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$7F7F7F7F
	dc.l	$80809098
	dc.l	$A0A8B0B8
	dc.l	$C0C8D0D8
	dc.l	$E0E8F0F8
	dc.l	$81018
	dc.l	$20283038
	dc.l	$40485058
	dc.l	$6068707F
	dc.l	$8080A0B0
	dc.l	$C0D0E0F0
	dc.l	$102030
	dc.l	$40506070
	dc.l	$4545797D
	dc.l	$7A777066
	dc.l	$6158534D
	dc.l	$2C201812
	dc.l	$4DBD3CD
	dc.l	$C6BCB5AE
	dc.l	$A8A39D99
	dc.l	$938E8B8A
	dc.l	$4545797D
	dc.l	$7A777066
	dc.l	$5B4B4337
	dc.l	$2C201812
	dc.l	$4F8E8DB
	dc.l	$CFC6BEB0
	dc.l	$A8A49E9A
	dc.l	$95948D83
	dc.l	$4060
	dc.l	$7F604020
	dc.l	$E0C0A0
	dc.l	$80A0C0E0
	dc.l	$4060
	dc.l	$7F604020
	dc.l	$E0C0A0
	dc.l	$80A0C0E0
	dc.l	$80809098
	dc.l	$A0A8B0B8
	dc.l	$C0C8D0D8
	dc.l	$E0E8F0F8
	dc.l	$81018
	dc.l	$20283038
	dc.l	$40485058
	dc.l	$6068707F
	dc.l	$8080A0B0
	dc.l	$C0D0E0F0
	dc.l	$102030
	dc.l	$40506070

;********************************************
;*** END OF FUTURE COMPOSER V1.3 REPLAYER ***
;********************************************

;******************************************
;*** JAMCRACKER PRO REPLAY ROUTINE v1.0 ***
;******************************************

DMAWAIT		EQU	300	;Change to suit
		RSRESET		;Instrument info structure
it_name		RS.B	31
it_flags		RS.B	1
it_size		RS.L	1
it_address	RS.L	1
it_sizeof		RS.W	0
		RSRESET		;Pattern info structure
pt_size		RS.W	1
pt_address	RS.L	1
pt_sizeof		RS.W	0
		RSRESET		;Note info structure
nt_period		RS.B	1
nt_instr		RS.B	1
nt_speed		RS.B	1
nt_arpeggio	RS.B	1
nt_vibrato	RS.B	1
nt_phase		RS.B	1
nt_volume		RS.B	1
nt_porta		RS.B	1
nt_sizeof		RS.W	0
		RSRESET		;Voice info structure
pv_waveoffset	RS.W	1
pv_dmacon		RS.W	1
pv_custbase	RS.L	1
pv_inslen		RS.W	1
pv_insaddress	RS.L	1
pv_peraddress	RS.L	1
pv_pers		RS.W	3
pv_por		RS.W	1
pv_deltapor	RS.W	1
pv_porlevel	RS.W	1
pv_vib		RS.W	1
pv_deltavib	RS.W	1
pv_vol		RS.W	1
pv_deltavol	RS.W	1
pv_vollevel	RS.W	1
pv_phase		RS.W	1
pv_deltaphase	RS.W	1
pv_vibcnt		RS.B	1
pv_vibmax		RS.B	1
pv_flags		RS.B	1
pv_sizeof		RS.W	0

pp_init:
	move.l	_CurrentMod,a0
	lea	20(a0),a0
		addq.w	#4,a0
		move.w	(a0)+,d0
		move.w	d0,d1
		move.l	a0,instable
		mulu	#it_sizeof,d0
		add.w	d0,a0

		move.w	(a0)+,d0
		move.w	d0,d2
		move.l	a0,patttable
		mulu	#pt_sizeof,d0
		add.w	d0,a0

		move.w	(a0)+,d0
		move.w	d0,songlen
		move.l	a0,songtable
		add.w	d0,d0
		add.w	d0,a0

		move.l	patttable(PC),a1
		move.w	d2,d0
		subq.w	#1,d0
.l0:		move.l	a0,pt_address(a1)
		move.w	(a1),d3		;pt_size
		mulu	#nt_sizeof*4,d3
		add.w	d3,a0
		addq.w	#pt_sizeof,a1
		dbra	d0,.l0

		move.l	instable(PC),a1
		move.w	d1,d0
		subq.w	#1,d0
.l1:		move.l	a0,it_address(a1)
		move.l	it_size(a1),d2
		add.l	d2,a0
		add.w	#it_sizeof,a1
		dbra	d0,.l1

		move.l	songtable(PC),pp_songptr
		move.w	songlen(PC),pp_songcnt
		move.l	pp_songptr(PC),a0
		move.w	(a0),d0
		mulu	#pt_sizeof,d0
		add.l	patttable(PC),d0
		move.l	d0,a0
		move.l	a0,pp_pattentry
		move.b	pt_size+1(a0),pp_notecnt
		move.l	pt_address(a0),pp_address
		move.b	#6,pp_wait
		move.b	#1,pp_waitcnt
		clr.w	pp_nullwave
		move.w	#$000F,$DFF096

		lea	pp_variables(PC),a0
		lea	$DFF0A0,a1
		moveq	#1,d1
		move.w	#$80,d2
		moveq	#4-1,d0
.l2:		move.w	#0,8(a1)
		move.w	d2,(a0)		;pv_waveoffset
		move.w	d1,pv_dmacon(a0)
		move.l	a1,pv_custbase(a0)
		move.l	#pp_periods,pv_peraddress(a0)
		move.w	#1019,pv_pers(a0)
		clr.w	pv_pers+2(a0)
		clr.w	pv_pers+4(a0)
		clr.l	pv_por(a0)
		clr.w	pv_porlevel(a0)
		clr.l	pv_vib(a0)
		clr.l	pv_vol(a0)
		move.w	#$40,pv_vollevel(a0)
		clr.l	pv_phase(a0)
		clr.w	pv_vibcnt(a0)
		clr.b	pv_flags(a0)
		add.w	#pv_sizeof,a0
		add.w	#$10,a1
		add.w	d1,d1
		add.w	#$40,d2
		dbra	d0,.l2

		bset	#1,$BFE001

		rts

pp_end:		moveq	#0,d0
		lea	$DFF000,a0
		move.w	d0,$A8(a0)
		move.w	d0,$B8(a0)
		move.w	d0,$C8(a0)
		move.w	d0,$D8(a0)
		move.w	#$000F,$96(a0)
		bclr	#1,$BFE001
		rts

pp_play:		lea	$DFF000,a6
		subq.b	#1,pp_waitcnt
		bne.s	.l0
		bsr	pp_nwnt
		move.b	pp_wait(PC),pp_waitcnt

.l0:		lea	pp_variables(PC),a1
		bsr.s	pp_uvs
		lea	pp_variables+pv_sizeof(PC),a1
		bsr.s	pp_uvs
		lea	pp_variables+2*pv_sizeof(PC),a1
		bsr.s	pp_uvs
		lea	pp_variables+3*pv_sizeof(PC),a1

pp_uvs:		move.l	pv_custbase(a1),a0

.l0:		move.w	pv_pers(a1),d0
		bne.s	.l1
		bsr	pp_rot
		bra.s	.l0
.l1:		add.w	pv_por(a1),d0
		tst.w	pv_por(a1)
		beq.s	.l1c
		bpl.s	.l1a
		cmp.w	pv_porlevel(a1),d0
		bge.s	.l1c
		bra.s	.l1b
.l1a:		cmp.w	pv_porlevel(a1),d0
		ble.s	.l1c
.l1b:		move.w	pv_porlevel(a1),d0

.l1c:		add.w	pv_vib(a1),d0
		cmp.w	#135,d0
		bge.s	.l1d
		move.w	#135,d0
		bra.s	.l1e
.l1d:		cmp.w	#1019,d0
		ble.s	.l1e
		move.w	#1019,d0
.l1e:		move.w	d0,6(a0)
		bsr	pp_rot

		move.w	pv_deltapor(a1),d0
		add.w	d0,pv_por(a1)
		cmp.w	#-1019,pv_por(a1)
		bge.s	.l3
		move.w	#-1019,pv_por(a1)
		bra.s	.l5
.l3:		cmp.w	#1019,pv_por(a1)
		ble.s	.l5
		move.w	#1019,pv_por(a1)

.l5:		tst.b	pv_vibcnt(a1)
		beq.s	.l7
		move.w	pv_deltavib(a1),d0
		add.w	d0,pv_vib(a1)
		subq.b	#1,pv_vibcnt(a1)
		bne.s	.l7
		neg.w	pv_deltavib(a1)
		move.b	pv_vibmax(a1),pv_vibcnt(a1)

.l7:		move.w	pv_dmacon(a1),d0
		move.w	pv_vol(a1),8(a0)
		move.w	pv_deltavol(a1),d0
		add.w	d0,pv_vol(a1)
		tst.w	pv_vol(a1)
		bpl.s	.l8
		clr.w	pv_vol(a1)
		bra.s	.la
.l8:		cmp.w	#$40,pv_vol(a1)
		ble.s	.la
		move.w	#$40,pv_vol(a1)

.la:		btst	#1,pv_flags(a1)
		beq.s	.l10
		tst.w	pv_deltaphase(a1)
		beq.s	.l10
		bpl.s	.sk
		clr.w	pv_deltaphase(a1)
.sk:		move.l	pv_insaddress(a1),a0
		move.w	(a1),d0		;pv_waveoffset
		neg.w	d0
		lea	(a0,d0.w),a2
		move.l	a2,a3
		move.w	pv_phase(a1),d0
		lsr.w	#2,d0
		add.w	d0,a3

		moveq	#$40-1,d0
.lb:		move.b	(a2)+,d1
		ext.w	d1
		move.b	(a3)+,d2
		ext.w	d2
		add.w	d1,d2
		asr.w	#1,d2
		move.b	d2,(a0)+
		dbra	d0,.lb

		move.w	pv_deltaphase(a1),d0
		add.w	d0,pv_phase(a1)
		cmp.w	#$100,pv_phase(a1)
		blt.s	.l10
		sub.w	#$100,pv_phase(a1)

.l10:		rts

pp_rot:		move.w	pv_pers(a1),d0
		move.w	pv_pers+2(a1),pv_pers(a1)
		move.w	pv_pers+4(a1),pv_pers+2(a1)
		move.w	d0,pv_pers+4(a1)
		rts

pp_nwnt:		move.l	pp_address(PC),a0
		add.l	#4*nt_sizeof,pp_address
		subq.b	#1,pp_notecnt
		bne.s	.l5

.l0:		addq.l	#2,pp_songptr
		subq.w	#1,pp_songcnt
		bne.s	.l1
		move.l	songtable(PC),pp_songptr
		move.w	songlen(PC),pp_songcnt
.l1:		move.l	pp_songptr(PC),a1
		move.w	(a1),d0
		mulu	#pt_sizeof,d0
		add.l	patttable(PC),d0
		move.l	d0,a1
		move.b	pt_size+1(a1),pp_notecnt
		move.l	pt_address(a1),pp_address

.l5:		clr.w	pp_tmpdmacon
		lea	pp_variables(PC),a1
		bsr	pp_nnt
		addq.w	#nt_sizeof,a0
		lea	pp_variables+pv_sizeof(PC),a1
		bsr	pp_nnt
		addq.w	#nt_sizeof,a0
		lea	pp_variables+2*pv_sizeof(PC),a1
		bsr	pp_nnt
		addq.w	#nt_sizeof,a0
		lea	pp_variables+3*pv_sizeof(PC),a1
		bsr	pp_nnt

		move.w	pp_tmpdmacon(PC),$96(a6)

		move.w	#DMAWAIT-1,d0
.loop1:		dbra	d0,.loop1

		lea	pp_variables(PC),a1
		bsr.s	pp_scr
		lea	pp_variables+pv_sizeof(PC),a1
		bsr.s	pp_scr
		lea	pp_variables+2*pv_sizeof(PC),a1
		bsr.s	pp_scr
		lea	pp_variables+3*pv_sizeof(PC),a1
		bsr.s	pp_scr

		bset	#7,pp_tmpdmacon
		move.w	pp_tmpdmacon(PC),$96(a6)

		move.w	#DMAWAIT-1,d0
.loop2:		dbra	d0,.loop2

		move.l	pp_variables+pv_insaddress(PC),$A0(a6)
		move.w	pp_variables+pv_inslen(PC),$A4(a6)
		move.l	pp_variables+pv_sizeof+pv_insaddress(PC),$B0(a6)
		move.w	pp_variables+pv_sizeof+pv_inslen(PC),$B4(a6)
		move.l	pp_variables+2*pv_sizeof+pv_insaddress(PC),$C0(a6)
		move.w	pp_variables+2*pv_sizeof+pv_inslen(PC),$C4(a6)
		move.l	pp_variables+3*pv_sizeof+pv_insaddress(PC),$D0(a6)
		move.w	pp_variables+3*pv_sizeof+pv_inslen(PC),$D4(a6)

		rts

pp_scr:		move.w	pp_tmpdmacon(PC),d0
		and.w	pv_dmacon(a1),d0
		beq.s	.l5

		move.l	pv_custbase(a1),a0
		move.l	pv_insaddress(a1),(a0)
		move.w	pv_inslen(a1),4(a0)
		move.w	pv_pers(a1),6(a0)
		btst	#0,pv_flags(a1)
		bne.s	.l5
		move.l	#pp_nullwave,pv_insaddress(a1)
		move.w	#1,pv_inslen(a1)

.l5:		rts

pp_nnt:		move.b	(a0),d1		;nt_period
		beq	.l5

		and.l	#$000000FF,d1
		add.w	d1,d1
		add.l	#pp_periods-2,d1
		move.l	d1,a2

		btst	#6,nt_speed(a0)
		beq.s	.l2
		move.w	(a2),pv_porlevel(a1)
		bra.s	.l5

.l2:		move.w	pv_dmacon(a1),d0
		or.w	d0,pp_tmpdmacon

		move.l	a2,pv_peraddress(a1)
		move.w	(a2),pv_pers(a1)
		move.w	(a2),pv_pers+2(a1)
		move.w	(a2),pv_pers+4(a1)

		clr.w	pv_por(a1)

		move.b	nt_instr(a0),d0
		ext.w	d0
		mulu	#it_sizeof,d0
		add.l	instable(PC),d0
		move.l	d0,a2
		tst.l	it_address(a2)
		bne.s	.l1
		move.l	#pp_nullwave,pv_insaddress(a1)
		move.w	#1,pv_inslen(a1)
		clr.b	pv_flags(a1)
		bra.s	.l5

.l1:		move.l	it_address(a2),a3
		btst	#1,it_flags(a2)
		bne.s	.l0a
		move.l	it_size(a2),d0
		lsr.l	#1,d0
		move.w	d0,pv_inslen(a1)
		bra.s	.l0
.l0a:		move.w	(a1),d0		;pv_waveoffset
		add.w	d0,a3
		move.w	#$20,pv_inslen(a1)
.l0:		move.l	a3,pv_insaddress(a1)
		move.b	it_flags(a2),pv_flags(a1)
		move.w	pv_vollevel(a1),pv_vol(a1)

.l5:		move.b	nt_speed(a0),d0
		and.b	#$0F,d0
		beq.s	.l6
		move.b	d0,pp_wait

.l6:		move.l	pv_peraddress(a1),a2
		move.b	nt_arpeggio(a0),d0
		beq.s	.l9
		cmp.b	#$FF,d0
		bne.s	.l7
		move.w	(a2),pv_pers(a1)
		move.w	(a2),pv_pers+2(a1)
		move.w	(a2),pv_pers+4(a1)
		bra.s	.l9

.l7:		and.b	#$0F,d0
		add.b	d0,d0
		ext.w	d0
		move.w	(a2,d0.w),pv_pers+4(a1)
		move.b	nt_arpeggio(a0),d0
		lsr.b	#4,d0
		add.b	d0,d0
		ext.w	d0
		move.w	(a2,d0.w),pv_pers+2(a1)
		move.w	(a2),pv_pers(a1)

.l9:		move.b	nt_vibrato(a0),d0
		beq.s	.ld
		cmp.b	#$FF,d0
		bne.s	.la
		clr.l	pv_vib(a1)
		clr.b	pv_vibcnt(a1)
		bra.s	.ld
.la:		clr.w	pv_vib(a1)
		and.b	#$0F,d0
		ext.w	d0
		move.w	d0,pv_deltavib(a1)
		move.b	nt_vibrato(a0),d0
		lsr.b	#4,d0
		move.b	d0,pv_vibmax(a1)
		lsr.b	#1,d0
		move.b	d0,pv_vibcnt(a1)

.ld:		move.b	nt_phase(a0),d0
		beq.s	.l10
		cmp.b	#$FF,d0
		bne.s	.le
		clr.w	pv_phase(a1)
		move.w	#$FFFF,pv_deltaphase(a1)
		bra.s	.l10
.le:		and.b	#$0F,d0
		ext.w	d0
		move.w	d0,pv_deltaphase(a1)
		clr.w	pv_phase(a1)

.l10:		move.b	nt_volume(a0),d0
		bne.s	.l10a
		btst	#7,nt_speed(a0)
		beq.s	.l16
		bra.s	.l11a
.l10a:		cmp.b	#$FF,d0
		bne.s	.l11
		clr.w	pv_deltavol(a1)
		bra.s	.l16
.l11:		btst	#7,nt_speed(a0)
		beq.s	.l12
.l11a:		move.b	d0,pv_vol+1(a1)
		move.b	d0,pv_vollevel+1(a1)
		clr.w	pv_deltavol(a1)
		bra.s	.l16
.l12:		bclr	#7,d0
		beq.s	.l13
		neg.b	d0
.l13:		ext.w	d0
		move.w	d0,pv_deltavol(a1)

.l16:		move.b	nt_porta(a0),d0
		beq.s	.l1a
		cmp.b	#$FF,d0
		bne.s	.l17
		clr.l	pv_por(a1)
		bra.s	.l1a
.l17:		clr.w	pv_por(a1)
		btst	#6,nt_speed(a0)
		beq.s	.l17a
		move.w	pv_porlevel(a1),d1
		cmp.w	pv_pers(a1),d1
		bgt.s	.l17c
		neg.b	d0
		bra.s	.l17c

.l17a:		bclr	#7,d0
		bne.s	.l18
		neg.b	d0
		move.w	#135,pv_porlevel(a1)
		bra.s	.l17c

.l18:		move.w	#1019,pv_porlevel(a1)
.l17c:		ext.w	d0
.l18a:		move.w	d0,pv_deltapor(a1)

.l1a:		rts

	*** Data section ***

pp_periods:	DC.W	1019,962,908,857,809,763,720,680,642,606,572,540
		DC.W	509,481,454,428,404,381,360,340,321,303,286,270
		DC.W	254,240,227,214,202,190,180,170,160,151,143,135
		DC.W	135,135,135,135,135,135,135,135,135
		DC.W	135,135,135,135,135,135

songlen:		DS.W	1
songtable:	DS.L	1
instable:		DS.L	1
patttable:	DS.L	1

pp_wait:		DS.B	1
pp_waitcnt:	DS.B	1
pp_notecnt:	DS.B	1
		DS.B	1
pp_address:	DS.L	1
pp_songptr:	DS.L	1
pp_songcnt:	DS.W	1
pp_pattentry:	DS.L	1
pp_tmpdmacon:	DS.W	1
pp_variables:	DS.B	4*48
pp_nullwave:	DS.W	1


;**************************************
;*** END OF JAMCRACKER PRO REPLAYER ***
;**************************************

;****************************************
;*** SOUNDMONITOR V2.0 REPLAY ROUTINE ***
;****************************************

bpend:	move.w #15,$dff096
	move.l	#0,bpstep
	move.b	#0,tr
	move.b	#1,bpcount
	move.b	#6,bpdelay
	move.b	#1,arpcount
	move.b	#1,bprepcount
	rts

bpinit:		lea samples(pc),a0
	move.l	_CurrentMod,a1
	lea	20(a1),a1
		clr.b numtables
		cmpi.w #'V.',26(a1)
		bne.s bpnotv2
		cmpi.b #'2',28(a1)
		bne.s bpnotv2
		move.b 29(a1),numtables
bpnotv2:	move.l #512,d0
		move.w 30(a1),d1
		moveq.l #1,d2
		mulu #4,d1
		subq.w #1,d1
findhighest:	cmp.w (a1,d0),d2
		bge.s nothigher
		move.w (a1,d0),d2
nothigher:	addq.l #4,d0
		dbra d1,findhighest
		move.w 30(a1),d1
		mulu #16,d1
		move.l #512,d0
		mulu #48,d2
		add.l d2,d0
		add.l d1,d0
		add.l a1,d0	;***
		move.l d0,tables
		moveq.l #0,d1
		move.b numtables,d1
		lsl.l #6,d1
		add.l d1,d0
		move.l #14,d1
		add.l #32,a1
initloop:	move.l d0,(a0)+
		cmpi.b #$ff,(a1)
		beq.s bpissynth
		move.w 24(a1),d2
		mulu #2,d2
		add.l d2,d0
bpissynth:	add.l #32,a1
		dbra d1,initloop
		rts
bpmusic:	bsr bpsynth
		subq.b #1,arpcount
		moveq.l #3,d0
		lea bpcurrent(pc),a0
		move.l #$dff0a0,a1
bploop1:	move.b 12(a0),d4
		ext.w d4
		add.w d4,(a0)
		tst.b $1e(a0)
		bne.s bplfo
		move.w (a0),6(a1)
bplfo:		move.l 4(a0),(a1)
		move.w 8(a0),4(a1)
		tst.b 11(a0)
		bne.s bpdoarp
		tst.b 13(a0)
		beq.s not2
bpdoarp:	tst.b arpcount
		bne.s not0
		move.b 11(a0),d3
		move.b 13(a0),d4
		and.w #240,d4
		and.w #240,d3
		lsr.w #4,d3
		lsr.w #4,d4
		add.w d3,d4
		add.b 10(a0),d4
		bsr bpplayarp
		bra.s not2
not0:		cmpi.b #1,arpcount
		bne.s not1
		move.b 11(a0),d3
		move.b 13(a0),d4
		and.w #15,d3
		and.w #15,d4
		add.w d3,d4
		add.b 10(a0),d4
		bsr bpplayarp
		bra.s not2
not1:		move.b 10(a0),d4
		bsr bpplayarp
not2:		lea $10(a1),a1
		lea $20(a0),a0
		dbra d0,bploop1
		tst.b arpcount
		bne.s arpnotzero
		move.b #3,arpcount
arpnotzero:	subq.b #1,bpcount
		beq.s bpskip1
		rts
bpskip1:	move.b bpdelay,bpcount
bpplay:		bsr.s bpnext
		move.w dma,$dff096
		;move.l #$1f4,d0
bpxx:		;dbra d0,bpxx
		moveq.l #3,d0
		move.l #$dff0a0,a1
		moveq #1,d1
		lea bpcurrent(pc),a2
		lea bpbuffer(pc),a5
bploop2:	btst #15,(a2)
		beq.s bpskip7
		bsr bpplayit
bpskip7:	asl.w #1,d1
		lea $10(a1),a1
		lea $20(a2),a2
		lea $24(a5),a5
		dbra d0,bploop2
		rts
bpnext:		clr.w dma
	move.l	_CurrentMod,a0
	lea	20(a0),a0
		move.l #$dff0a0,a3
		moveq.l #3,d0
		moveq #1,d7
		lea bpcurrent(pc),a1
bploop3:	moveq.l #0,d1
		move.w bpstep,d1
		lsl.w #4,d1
		move.l d0,d2
		lsl.l #2,d2
		add.l d2,d1
		add.l #512,d1
		move.w (a0,d1),d2
		move.b 2(a0,d1),st
		move.b 3(a0,d1),tr
		subq.w #1,d2
		mulu #48,d2
		moveq.l #0,d3
		move.w 30(a0),d3
		lsl.w #4,d3
		add.l d2,d3
		move.l #$00000200,d4
		move.b bppatcount,d4
		add.l d3,d4
		move.l d4,a2
		add.l a0,a2
		moveq.l #0,d3
		move.b (a2),d3
		tst.b d3
		bne.s bpskip4
		bra bpoptionals
bpskip4:	clr.w 12(a1)
		move.b 1(a2),d4
		and.b #15,d4
		cmpi.b #10,d4
		bne.s bp_do1
		move.b 2(a2),d4
		and.b #240,d4
		bne.s bp_not1
bp_do1:		add.b tr,d3
		ext.w d3
bp_not1:	move.b d3,10(a1)
		lea bpper(pc),a4
		lsl.w #1,d3
		move.w -2(a4,d3.w),(a1)
		bset #15,(a1)
		move.b #$ff,2(a1)
		moveq #0,d3
		move.b 1(a2),d3
		lsr.b #4,d3
		and.b #15,d3
		tst.b d3
		bne.s bpskip5
		move.b 3(a1),d3
bpskip5: 	move.b 1(a2),d4
		and.b #15,d4
		cmpi.b #10,d4
		bne.s bp_do2
		move.b 2(a2),d4
		and.b #15,d4
		bne.s bp_not2
bp_do2:		add.b st,d3
bp_not2:	cmpi.w #1,8(a1)
		beq.s bpsamplechange
		cmp.b 3(a1),d3
		beq.s bpoptionals
bpsamplechange:	move.b d3,3(a1)
		or.w d7,dma
bpoptionals: 	moveq.l #0,d3
		moveq.l #0,d4
		move.b 1(a2),d3
		and.b #15,d3
		move.b 2(a2),d4
		cmpi.b #0,d3
		bne.s notopt0
		move.b d4,11(a1)
notopt0:	cmpi.b #1,d3
		bne.s bpskip3
		move.w d4,8(a3)
		move.b d4,2(a1)
bpskip3:	cmpi.b #2,d3
		bne.s bpskip9
		move.b d4,bpcount
		move.b d4,bpdelay
bpskip9:	cmpi.b #3,d3
		bne.s bpskipa
		tst.b d4
		bne.s bpskipb
		bset #1,$bfe001
		bra.s bpskip2
bpskipb:	bclr #1,$bfe001
bpskipa:	cmpi.b #4,d3
		bne.s noportup
		sub.w d4,(a1)
		clr.b 11(a1)
noportup:	cmpi.b #5,d3
		bne.s noportdn
		add.w d4,(a1)
		clr.b 11(a1)
noportdn:	cmpi.b #6,d3
		bne.s notopt6
		move.b d4,bprepcount
notopt6:	cmpi.b #7,d3
		bne.s notopt7
		subq.b #1,bprepcount
		beq.s notopt7
		move.w d4,bpstep
notopt7:	cmpi.b #8,d3
		bne.s notopt8
		move.b d4,12(a1)
notopt8:	cmpi.b #9,d3
		bne.s notopt9
		move.b d4,13(a1)
notopt9:
bpskip2:	lea $10(a3),a3
		lea $20(a1),a1
		asl.w #1,d7
		dbra d0,bploop3
		addq.b #3,bppatcount
		cmpi.b #48,bppatcount
		bne.s bpskip8
		move.b #0,bppatcount
		addq.w #1,bpstep
	move.l	_CurrentMod,a0
	lea	20(a0),a0
		move.w 30(a0),d1
		cmp.w bpstep,d1
		bne.s bpskip8
		move.w #0,bpstep
bpskip8:	rts
bpplayit:	bclr #15,(a2)
		tst.l (a5)
		beq.s noeg1
		moveq #0,d3
		move.l (a5),a4
		moveq #7,d7
eg1loop:	move.l 4(a5,d3.w),(a4)+
		addq.w #4,d3
		dbra d7,eg1loop
noeg1:		move.w (a2),6(a1)
		moveq.l #0,d7
		move.b 3(a2),d7
		move.l d7,d6
		lsl.l #5,d7
	move.l	_CurrentMod,a3
	lea	20(a3),a3
		cmpi.b #$ff,(a3,d7.w)
		beq.s bpplaysynthetic
		clr.l (a5)
		clr.b $1a(a2)
		clr.w $1e(a2)
		add.l #24,d7
		lsl.l #2,d6
		move.l #samples,a4
		move.l -4(a4,d6),d4
		beq.s bp_nosamp
		move.l d4,(a1)
		move.w (a3,d7),4(a1)
		move.b 2(a2),9(a1)
		cmpi.b #$ff,2(a2)
		bne.s skipxx
		move.w 6(a3,d7),8(a1)
skipxx: 	move.w 4(a3,d7),8(a2)
		moveq.l #0,d6
		move.w 2(a3,d7),d6
		add.l d6,d4
		move.l d4,4(a2)
		cmpi.w #1,8(a2)
		bne.s bpskip6
bp_nosamp:	move.l #null,4(a2)
		bra.s bpskip10
bpskip6:	move.w 8(a2),4(a1)
		move.l 4(a2),(a1)
bpskip10:	or.w #$8000,d1
		move.w d1,$dff096
		rts
bpplaysynthetic:move.b #$1,$1a(a2)
		clr.w $e(a2)
		clr.w $10(a2)
		clr.w $12(a2)
		move.w 22(a3,d7.w),$14(a2)
		addq.w #1,$14(a2)
		move.w 14(a3,d7.w),$16(a2)
		addq.w #1,$16(a2)
		move.w #1,$18(a2)
		move.b 17(a3,d7.w),$1d(a2)
		move.b 9(a3,d7.w),$1e(a2)
		move.b 4(a3,d7.w),$1f(a2)
		move.b 19(a3,d7.w),$1c(a2)
		move.l tables,a4
		moveq.l #0,d3
		move.b 1(a3,d7.w),d3
		lsl.l #6,d3
		add.l d3,a4
		move.l a4,(a1)
		move.l a4,4(a2)
		move.w 2(a3,d7.w),4(a1)
		move.w 2(a3,d7.w),8(a2)
		tst.b 4(a3,d7.w)
		beq.s bpadsroff
		move.l tables,a4
		moveq.l #0,d3
		move.b 5(a3,d7.w),d3
		lsl.l #6,d3
		add.l d3,a4
		moveq #0,d3
		move.b (a4),d3
		add.b #128,d3
		lsr.w #2,d3
		cmpi.b #$ff,2(a2)
		bne.s bpskip99
		move.b 25(a3,d7.w),2(a2)
bpskip99:	moveq #0,d4
		move.b 2(a2),d4
		mulu d4,d3
		lsr.w #6,d3
		move.w d3,8(a1)
		bra.s bpflipper
bpadsroff:	move.b 2(a2),9(a1)
		cmpi.b #$ff,2(a2)
		bne.s bpflipper
		move.b 25(a3,d7.w),9(a1)
bpflipper:	move.l 4(a2),a4
		move.l a4,(a5)
		moveq #0,d3
		moveq #7,d4
eg2loop:	move.l (a4,d3.w),4(a5,d3.w)
		addq.w #4,d3
		dbra d4,eg2loop
		tst.b 17(a3,d7.w)
		beq bpskip10
		tst.b 19(a3,d7.w)
		beq bpskip10
		moveq.l #0,d3
		move.b 19(a3,d7.w),d3
		lsr.l #3,d3
		move.b d3,$1c(a2)
		subq.l #1,d3
eg3loop:	neg.b (a4)+
		dbra d3,eg3loop
		bra bpskip10
bpplayarp:	lea bpper(pc),a4
		ext.w d4
		asl.w #1,d4
		move.w -2(a4,d4.w),6(a1)
		rts
bpsynth:	move.l #3,d0
		lea bpcurrent(pc),a2
		lea $dff0a0,a1
	move.l	_CurrentMod,a3
	lea	20(a3),a3
		lea bpbuffer(pc),a5
bpsynthloop:	tst.b $1a(a2)
		beq.s bpnosynth
		bsr.s bpyessynth
bpnosynth:	lea $24(a5),a5
		lea $20(a2),a2
		lea $10(a1),a1
		dbra d0,bpsynthloop
		rts
bpyessynth:	moveq #0,d7
		move.b 3(a2),d7
		lsl.w #5,d7
		tst.b $1f(a2)
		beq.s bpendadsr
		subq.w #1,$18(a2)
		bne.s bpendadsr
		moveq.l #0,d3
		move.b 8(a3,d7.w),d3
		move.w d3,$18(a2)
		move.l tables,a4
		move.b 5(a3,d7.w),d3
		lsl.l #6,d3
		add.l d3,a4
		move.w $12(a2),d3
		moveq #0,d4
		move.b (a4,d3.w),d4
		add.b #128,d4
		lsr.w #2,d4
		moveq #0,d3
		move.b 2(a2),d3
		mulu d3,d4
		lsr.w #6,d4
		move.w d4,8(a1)
		addq.w #1,$12(a2)
		move.w 6(a3,d7.w),d4
		cmp.w $12(a2),d4
		bne.s bpendadsr
		clr.w $12(a2)
		cmpi.b #1,$1f(a2)
		bne.s bpendadsr
		clr.b $1f(a2)
bpendadsr:	tst.b $1e(a2)
		beq.s bpendlfo
		subq.w #1,$16(a2)
		bne.s bpendlfo
		moveq.l #0,d3
		move.b 16(a3,d7.w),d3
		move.w d3,$16(a2)
		move.l tables,a4
		move.b 10(a3,d7.w),d3
		lsl.l #6,d3
		add.l d3,a4
		move.w $10(a2),d3
		moveq.l #0,d4
		move.b (a4,d3.w),d4
		ext.w d4
		ext.l d4
		moveq.l #0,d5
		move.b 11(a3,d7.w),d5
		tst.b d5
		beq.s bpnotx
		divs d5,d4
bpnotx:		move.w (a2),d5
		add.w d4,d5
		move.w d5,6(a1)
		addq.w #1,$10(a2)
		move.w 12(a3,d7.w),d3
		cmp.w $10(a2),d3
		bne.s bpendlfo
		clr.w $10(a2)
		cmpi.b #1,$1e(a2)
		bne.s bpendlfo
		clr.b $1e(a2)
bpendlfo:	tst.b $1d(a2)
		beq bpendeg
		subq.w #1,$14(a2)
		bne bpendeg
		tst.l (a5)
		beq.s bpendeg
		moveq.l #0,d3
		move.b 24(a3,d7.w),d3
		move.w d3,$14(a2)
		move.l tables,a4
		move.b 18(a3,d7.w),d3
		lsl.l #6,d3
		add.l d3,a4
		move.w $e(a2),d3
		moveq.l #0,d4
		move.b (a4,d3.w),d4
		move.l (a5),a4
		add.b #128,d4
		lsr.l #3,d4
		moveq.l #0,d3
		move.b $1c(a2),d3
		move.b d4,$1c(a2)
		add.l d3,a4
		move.l a5,a6
		add.l d3,a6
		addq.l #4,a6
		cmp.b d3,d4
		beq.s bpnexteg
		bgt bpishigh
bpislow:	sub.l d4,d3
		subq.l #1,d3
bpegloop1a:	move.b -(a6),d4
		move.b d4,-(a4)
		dbra d3,bpegloop1a
		bra.s bpnexteg
bpishigh:	sub.l d3,d4
		subq.l #1,d4
bpegloop1b:	move.b (a6)+,d3
		neg.b d3
		move.b d3,(a4)+
		dbra d4,bpegloop1b
bpnexteg:	addq.w #1,$e(a2)
		move.w 20(a3,d7.w),d3
		cmp.w $e(a2),d3
		bne.s bpendeg
		clr.w $e(a2)
		cmpi.b #1,$1d(a2)
		bne.s bpendeg
		clr.b $1d(a2)
bpendeg:	rts

null:		dc.w 0
bpcurrent:	dc.w 0,0
		dc.l null
		dc.w 1
		dc.b 0,0,0,0
		dc.w 0,0,0
		dc.w 0,0,0
		dc.b 0,0
		dc.b 0,0
		dc.b 0,0

		dc.w 0,0
		dc.l null
		dc.w 1,0,0
		dc.w 0,0,0,0,0,0,0,0,0

		dc.w 0,0
		dc.l null
		dc.w 1,0,0
		dc.w 0,0,0,0,0,0,0,0,0

		dc.w 0,0
		dc.l null
		dc.w 1,0,0
		dc.w 0,0,0,0,0,0,0,0,0

bpstep:		dc.w 0
bppatcount:	dc.b 0
st:		dc.b 0
tr:		dc.b 0
bpcount:	dc.b 1
bpdelay:	dc.b 6
arpcount:	dc.b 1
bprepcount:	dc.b 1
numtables:	dc.b 0
		even
dma:		dc.w 0
tables:		dc.l 0

bpbuffer:	dcb.b 144,0
		dc.w 6848,6464,6080,5760,5440,5120,4832,4576,4320,4064,3840,3616
		dc.w 3424,3232,3040,2880,2720,2560,2416,2288,2160,2032,1920,1808
		dc.w 1712,1616,1520,1440,1360,1280,1208,1144,1080,1016,0960,0904

bpper:		dc.w 0856,0808,0760,0720,0680,0640,0604,0572,0540,0508,0480,0452
		dc.w 0428,0404,0380,0360,0340,0320,0302,0286,0270,0254,0240,0226
		dc.w 0214,0202,0190,0180,0170,0160,0151,0143,0135,0127,0120,0113
		dc.w 0107,0101,0095,0090,0085,0080,0076,0072,0068,0064,0060,0057

samples:	ds.l 15
;************************************
;*** END OF SOUNDMONITOR REPLAYER ***
;************************************

;*************************************************
;*** OLD SOUNDTRACKER 15 SAMPLE REPLAY ROUTINE ***
;*************************************************

start_muzak:
	move.w	#6,speed+2
	move.l	_CurrentMod,a0
	lea	20(a0),a0
	move.l	a0,muzakoffset

init0:	move.l	muzakoffset,a0
	add.l	#472,a0
	move.l	#$80,d0
	clr.l	d1
init1:	move.l	d1,d2
	subq.w	#1,d0
init2:	move.b	(a0)+,d1
	cmp.b	d2,d1
	bgt.s	init1
	dbf	d0,init2
	addq.b	#1,d2

init3:	move.l	muzakoffset,a0
	lea	pointers,a1
	mulu	#1024,d2
	add.l	#600,d2
	add.l	a0,d2
	move.l	#15-1,d0
init4:	move.l	d2,(a1)+
	clr.l	d1
	move.w	42(a0),d1
	lsl.l	#1,d1
	add.l	d1,d2
	add.l	#30,a0
	dbf	d0,init4

init5:	move.w	#$0,$dff0a8
	move.w	#$0,$dff0b8
	move.w	#$0,$dff0c8
	move.w	#$0,$dff0d8
	clr.w	timpos
	clr.l	trkpos
	clr.l	patpos
	rts

stop_muzak:
	move.w	#$0,$dff0a8
	move.w	#$0,$dff0b8
	move.w	#$0,$dff0c8
	move.w	#$0,$dff0d8
	move.w	#$f,$dff096
	rts

replay_muzak:
	movem.l	d0-d7/a0-a6,-(a7)
	addq.w	#1,timpos
speed:	cmp.w	#6,timpos
	beq	replaystep

chaneleffects:
	lea	datach0,a6
	tst.b	3(a6)
	beq.s	ceff1
	lea	$dff0a0,a5
	bsr.s	ceff5
ceff1:	lea	datach1,a6
	tst.b	3(a6)
	beq.s	ceff2
	lea	$dff0b0,a5
	bsr.s	ceff5
ceff2:	lea	datach2,a6
	tst.b	3(a6)
	beq.s	ceff3
	lea	$dff0c0,a5
	bsr.s	ceff5
ceff3:	lea	datach3,a6
	tst.b	3(a6)
	beq.s	ceff4
	lea	$dff0d0,a5
	bsr.s	ceff5
ceff4:	movem.l	(a7)+,d0-d7/a0-a6
	rts

ceff5:	move.b	2(a6),d0
	and.b	#$0f,d0
	tst.b	d0
	beq.s	arpreggiato
	cmp.b	#1,d0
	beq	pitchup
	cmp.b	#2,d0
	beq	pitchdown
	cmp.b	#12,d0
	beq	setvol
	cmp.b	#14,d0
	beq	setfilt
	cmp.b	#15,d0
	beq	setspeed
	rts

arpreggiato:
	cmp.w	#1,timpos
	beq.s	arp1
	cmp.w	#2,timpos
	beq.s	arp2
	cmp.w	#3,timpos
	beq.s	arp3
	cmp.w	#4,timpos
	beq.s	arp1
	cmp.w	#5,timpos
	beq.s	arp2
	rts

arp1:	clr.l	d0
	move.b	3(a6),d0
	lsr.b	#4,d0
	bra.s	arp4
arp2:	clr.l	d0
	move.b	3(a6),d0
	and.b	#$0f,d0
	bra.s	arp4
arp3:	move.w	16(a6),d2
	bra.s	arp6
arp4:	lsl.w	#1,d0
	clr.l	d1
	move.w	16(a6),d1
	lea	notetable,a0
arp5:	move.w	(a0,d0.w),d2
	cmp.w	(a0),d1
	beq.s	arp6
	addq.l	#2,a0
	bra.s	arp5
arp6:	move.w	d2,6(a5)
	rts

pitchdown:
	bsr	newrou
	clr.l	d0
	move.b	3(a6),d0
	and.b	#$0f,d0
	add.w	d0,(a4)
	cmp.w	#$358,(a4)
	bmi.s	ok1
	move.w	#$358,(a4)
ok1:	move.w	(a4),6(a5)
	rts

pitchup:bsr	newrou
	clr.l	d0
	move.b	3(a6),d0
	and.b	#$0f,d0
	sub.w	d0,(a4)
	cmp.w	#$71,(a4)
	bpl.s	ok2
	move.w	#$71,(a4)
ok2:	move.w	(a4),6(a5)
	rts

setvol:	move.b	3(a6),8(a5)
	rts

setfilt:move.b	3(a6),d0
	and.b	#$01,d0
	asl.b	#$01,d0
	and.b	#$fd,$bfe001
	or.b	d0,$bfe001
	rts

setspeed:
	clr.l	d0
	move.b	3(a6),d0
	and.b	#$0f,d0
	move.w	d0,speed+2
	rts

newrou:	cmp.l	#datach0,a6
	bne.s	fuck1
	lea	voi1,a4
	rts
fuck1:	cmp.l	#datach1,a6
	bne.s	fuck2
	lea	voi2,a4
	rts
fuck2:	cmp.l	#datach2,a6
	bne.s	fuck3
	lea	voi3,a4
	rts
fuck3:	lea	voi4,a4
	rts

replaystep:			
	clr.w	timpos
	move.l	muzakoffset,a0
	move.l	a0,a3
	add.l	#12,a3		
	move.l	a0,a2
	add.l	#472,a2		
	add.l	#600,a0
	clr.l	d1
	move.l	trkpos,d0
	move.b	(a2,d0),d1
	mulu	#1024,d1
	add.l	patpos,d1
	clr.w	enbits
	lea	$dff0a0,a5	
	lea	datach0,a6
	bsr	chanelhandler
	lea	$dff0b0,a5
	lea	datach1,a6
	bsr	chanelhandler
	lea	$dff0c0,a5
	lea	datach2,a6
	bsr	chanelhandler
	lea	$dff0d0,a5	
	lea	datach3,a6
	bsr	chanelhandler
	move.l	#400,d0
rep1:	dbf	d0,rep1		
	move.l	#$8000,d0
	or.w	enbits,d0
	move.w	d0,$dff096
	cmp.w	#1,datach0+14
	bne.s	rep2
	clr.w	datach0+14
	move.w	#1,$dff0a4
rep2:	cmp.w	#1,datach1+14
	bne.s	rep3
	clr.w	datach1+14
	move.w	#1,$dff0b4
rep3:	cmp.w	#1,datach2+14
	bne.s	rep4
	clr.w	datach2+14
	move.w	#1,$dff0c4
rep4:	cmp.w	#1,datach3+14
	bne.s	rep5
	clr.w	datach3+14
	move.w	#1,$dff0d4

rep5:	add.l	#16,patpos
	cmp.l	#64*16,patpos
	bne	rep6
	clr.l	patpos
	addq.l	#1,trkpos
	clr.l	d0
	move.w	numpat,d0
	cmp.l	trkpos,d0
	bne	rep6
	clr.l	trkpos
rep6:	movem.l	(a7)+,d0-d7/a0-a6
	rts

chanelhandler:
	move.l	(a0,d1.l),(a6)
	addq.l	#4,d1
	clr.l	d2
	move.b	2(a6),d2		
	lsr.b	#4,d2
	beq.s	chan2
	move.l	d2,d4
	lsl.l	#2,d2
	mulu	#30,d4
	lea	pointers-4,a1
	move.l	(a1,d2.l),4(a6)		
	move.w	(a3,d4.l),8(a6)
	move.w	2(a3,d4.l),18(a6)

	move.l	d0,-(a7)
	move.b	2(a6),d0
	and.b	#$0f,d0
	cmp.b	#$0c,d0
	bne.s	ok3
	move.b	3(a6),8(a5)
	bra.s	ok4
ok3:	move.w	2(a3,d4.l),8(a5)
ok4:	move.l	(a7)+,d0

	clr.l	d3
	move.w	4(a3,d4),d3	
	add.l	4(a6),d3
	move.l	d3,10(a6)	
	move.w	6(a3,d4),14(a6)
	cmp.w	#1,14(a6)
	beq.s	chan2		
	move.l	10(a6),4(a6)
	move.w	6(a3,d4),8(a6)
chan2:	cmp.w	#0,(a6)
	beq.s	chan4
	move.w	22(a6),$dff096
	cmp.w	#0,14(a6)
	bne.s	chan3		
	move.w	#1,14(a6)
chan3:	bsr	newrou
	move.w	(a6),(a4)
	move.w	(a6),16(a6)
	move.l	4(a6),0(a5)
	move.w	8(a6),4(a5)
	move.w	(a6),6(a5)
	move.w	22(a6),d0
	or.w	d0,enbits
	move.w	18(a6),20(a6)
chan4:	rts

datach0:	dc.w	0,0,0,0,0,0,0,0,0,0,0,1
datach1:	dc.w	0,0,0,0,0,0,0,0,0,0,0,2
datach2:	dc.w	0,0,0,0,0,0,0,0,0,0,0,4
datach3:	dc.w	0,0,0,0,0,0,0,0,0,0,0,8
voi1:		dc.w	0
voi2:		dc.w	0
voi3:		dc.w	0
voi4:		dc.w	0
pointers:	dc.l	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
notetable:	dc.w	856,808,762,720,678,640,604,570
		dc.w	538,508,480,453,428,404,381,360
		dc.w	339,320,302,285,269,254,240,226
		dc.w	214,202,190,180,170,160,151,143
		dc.w	135,127,120,113,000
muzakoffset:	dc.l	0
trkpos:		dc.l	0
patpos:		dc.l	0
numpat:		dc.w	0
enbits:		dc.w	0
timpos:		dc.w	0

;****************************************
;*** END OF OLD SOUNDTRACKER REPLAYER ***
;****************************************

;*******************************************
;*** DIGITAL SOUND STUDIO REPLAY ROUTINE ***
;*******************************************
InitDSSMOD
	jsr	InitDSS
	jsr	ClearSampPtrs
	jsr	ClearAudRegs
	clr.w	DSS_w5BC
	clr.w	dss_Counter
	clr.w	DSS_w5C0
	clr.l	DSS_l5C2
	clr.w	DSS_w5C6
	clr.w	DSS_b5C8
	clr.w	DSS_w5CA
	clr.w	DSS_w5CC
	rts

StopDSSMOD
	jsr	ClearAudRegsDMA
	rts

InitDSS	move.l	_CurrentMod,a1
	lea	20(a1),a1
	lea	$59C(a1),a0
	move.w	(a0)+,d0
	subq.w	#1,d0
	moveq	#0,d1
	move.l	d1,d2
DSS_C1DE
	move.b	(a0)+,d2
	cmp.b	d2,d1
	bhi.s	DSS_C1E6
	move.b	d2,d1
DSS_C1E6
	dbra	d0,DSS_C1DE
	addq.w	#1,d1
	moveq	#10,d0
	lsl.l	d0,d1
	lea	$61E(a1),a0
	add.l	d1,a0
	lea	10(a1),a2
	lea	DSS_l6CC,a3
	moveq	#$1E,d0
DSS_C202
	clr.l	(a3)
	moveq	#0,d1
	move.w	$22(a2),d1
	beq.s	DSS_C22C
	move.l	a0,(a3)
	moveq	#0,d2
	cmp.w	#1,$28(a2)
	beq.s	DSS_C21E
	move.w	$28(a2),d2
	add.l	d2,d1
DSS_C21E
	add.l	d1,d1
	bclr	#0,$21(a2)
	add.l	$1E(a2),d1
	add.l	d1,a0
DSS_C22C
	addq.l	#4,a3
	add.l	#$2E,a2
	dbra	d0,DSS_C202
	rts

ClearSampPtrs
	lea	DSS_l6CC,a0
	moveq	#$1E,d0
DSS_C242
	move.l	(a0)+,d1
	beq.s	DSS_C250
	move.l	d1,a1
	moveq	#3,d2
DSS_C24A
	clr.b	(a1)+
	dbra	d2,DSS_C24A
DSS_C250
	dbra	d0,DSS_C242
	rts

ClearAudRegsDMA
	clr.w	$DFF0A8
	clr.w	$DFF0B8
	clr.w	$DFF0C8
	clr.w	$DFF0D8
	move.w	#15,$DFF096
	rts

ClearAudRegs
	clr.w	$DFF0A8
	clr.w	$DFF0B8
	clr.w	$DFF0C8
	clr.w	$DFF0D8
	rts

PlayDSSMOD
	move.l	_CurrentMod,a6
	lea	20(a6),a6
	move.w	8(a6),d0
	addq.w	#1,dss_Counter
	cmp.w	dss_Counter,d0
	bne.s	CheckdaDSS
	clr.w	dss_Counter
	bra	PlayDSS

CheckdaDSS
	lea	DSS_l5CE,a0
	lea	$DFF0A0,a1
	moveq	#3,d3
DSS_C2D0
	move.w	(a0),d0
	and.w	#$7FF,d0
	cmp.w	#$7FF,d0
	beq.s	DSS_C2E4
	tst.b	3(a0)
	beq.s	DSS_C2E4
	bsr.s	DSS_C2F4
DSS_C2E4
	add.l	#$18,a0
	add.w	#$10,a1
	dbra	d3,DSS_C2D0
	rts

DSS_C2F4
	move.b	2(a0),d0
	beq.s	DSS_C308
	cmp.b	#1,d0
	beq.s	DSS_C362
	cmp.b	#2,d0
	beq.s	DSS_C37C
	rts

DSS_C308
	moveq	#0,d0
	move.w	dss_Counter,d0
	cmp.w	#1,d0
	beq.s	DSS_C330
	cmp.w	#2,d0
	beq.s	DSS_C338
	cmp.w	#3,d0
	beq.s	DSS_C342
	cmp.w	#4,d0
	beq.s	DSS_C338
	cmp.w	#5,d0
	beq.s	DSS_C330
	rts

DSS_C330
	move.b	3(a0),d0
	lsr.b	#4,d0
	bra.s	DSS_C348

DSS_C338
	move.b	3(a0),d0
	and.b	#15,d0
	bra.s	DSS_C348

DSS_C342
	move.w	$12(a0),d2
	bra.s	DSS_C35C

DSS_C348
	add.w	d0,d0
	move.w	$12(a0),d1
	lea	DSS_l630,a2
DSS_C354
	move.w	0(a2,d0.w),d2
	cmp.w	(a2)+,d1
	bne.s	DSS_C354
DSS_C35C
	move.w	d2,6(a1)
	rts

DSS_C362
	moveq	#0,d0
	move.b	3(a0),d0
	sub.w	d0,$16(a0)
	cmp.w	#$71,$16(a0)
	bpl.s	DSS_C394
	move.w	#$71,$16(a0)
	bra.s	DSS_C394

DSS_C37C
	moveq	#0,d0
	move.b	3(a0),d0
	add.w	d0,$16(a0)
	cmp.w	#$6B0,$16(a0)
	bmi.s	DSS_C394
	move.w	#$6B0,$16(a0)
DSS_C394
	move.w	$16(a0),6(a1)
	rts

PlayDSS	lea	$61E(a6),a0
	lea	$59E(a6),a1
	move.w	DSS_w5C0,d0
	move.w	d0,DSS_w5CC
	moveq	#0,d1
	move.b	0(a1,d0.w),d1
	moveq	#10,d0
	lsl.l	d0,d1
	add.l	DSS_l5C2,d1
	clr.w	DSS_w5C6
	lea	$DFF0A0,a3
	lea	DSS_l5CE,a4
	moveq	#3,d7
DSS_C3D4
	bsr	DSS_C470
	add.w	#$10,a3
	add.l	#$18,a4
	dbra	d7,DSS_C3D4
	move.w	DSS_w5C6,d0
	or.w	#$8000,d0
	move.w	d0,$DFF096
	bsr	DSS_C4F8
	lea	DSS_l5CE,a0
	lea	$DFF0A0,a1
	moveq	#3,d0
DSS_C408
	move.l	12(a0),(a1)
	move.w	$10(a0),4(a1)
	add.l	#$18,a0
	add.w	#$10,a1
	dbra	d0,DSS_C408
	cmp.l	#$3F0,DSS_l5C2
	beq.s	DSS_C444
	add.l	#$10,DSS_l5C2
	tst.w	DSS_w5CA
	beq.s	DSS_C46E
	clr.w	DSS_w5CA
DSS_C444
	clr.l	DSS_l5C2
	move.w	DSS_w5CC,DSS_w5C0
	addq.w	#1,DSS_w5C0
	move.w	$59C(a6),d0
	move.w	DSS_w5C0,d1
	cmp.w	d0,d1
	bne.s	DSS_C46E
	clr.w	DSS_w5C0
DSS_C46E
	rts

DSS_C470
	lea	10(a6),a2
	move.l	0(a0,d1.l),0(a4)
	addq.l	#4,d1
	moveq	#0,d0
	move.b	0(a4),d0
	lsr.b	#3,d0
	tst.b	d0
	beq.s	DSS_C4CA
	lea	DSS_l6CC,a5
	subq.b	#1,d0
	move.l	d0,d3
	lsl.w	#2,d0
	mulu	#$2E,d3
	add.l	#$1E,d3
	add.l	d3,a2
	move.l	0(a5,d0.w),d4
	add.l	(a2)+,d4
	move.l	d4,6(a4)
	beq.s	DSS_C4CA
	move.w	(a2)+,10(a4)
	move.l	(a2)+,d5
	move.w	(a2)+,d2
	bne.s	DSS_C4BA
	moveq	#1,d2
	moveq	#0,d5
DSS_C4BA
	move.w	d2,$10(a4)
	add.l	6(a4),d5
	move.l	d5,12(a4)
	move.w	(a2),4(a4)
DSS_C4CA
	move.w	0(a4),d6
	and.w	#$7FF,d6
	beq.s	DSS_C528
	move.w	d6,$12(a4)
	move.w	$14(a4),d0
	move.w	d0,$DFF096
	bsr	DSS_C4F8
	cmp.w	#$7FF,d6
	bne.s	DSS_C50E
	clr.w	8(a3)
	or.w	d0,DSS_w5C6
	rts

DSS_C4F8
	moveq	#4,d3
DSS_C4FA
	move.b	$DFF006,d2
DSS_C500
	cmp.b	$DFF006,d2
	beq.s	DSS_C500
	dbra	d3,DSS_C4FA
	rts

DSS_C50E
	move.l	6(a4),(a3)
	beq.s	DSS_C528
	move.w	10(a4),4(a3)
	move.w	d6,6(a3)
	or.w	d0,DSS_w5C6
	move.w	d6,$16(a4)
DSS_C528
	move.b	2(a4),d0
	bsr.s	DSS_C546
	tst.b	d0
	beq.s	DSS_C544
	cmp.b	#4,d0
	beq.s	DSS_C56C
	cmp.b	#5,d0
	beq.s	DSS_C57E
	cmp.b	#6,d0
	beq.s	DSS_C590
DSS_C544
	rts

DSS_C546
	cmp.w	#3,d0
	bne.s	DSS_C554
	moveq	#0,d2
	move.b	3(a4),d2
	bra.s	DSS_C55C

DSS_C554
	tst.w	d6
	beq.s	DSS_C56A
	move.w	4(a4),d2
DSS_C55C
	sub.w	DSS_b5C8,d2
	bge.s	DSS_C566
	moveq	#0,d2
DSS_C566
	move.w	d2,8(a3)
DSS_C56A
	rts

DSS_C56C
	moveq	#$40,d2
	move.b	3(a4),d0
	sub.b	d0,d2
	blt.s	DSS_C57C
	move.b	d2,DSS_b5C9
DSS_C57C
	rts

DSS_C57E
	move.b	3(a4),d0
	beq.s	DSS_C58E
	clr.w	dss_Counter
	move.b	d0,9(a6)
DSS_C58E
	rts

DSS_C590
	move.w	#1,DSS_w5CA
	move.b	3(a4),d0
	cmp.b	#$FF,d0
	beq.s	DSS_C5AE
	subq.b	#2,d0
	ext.w	d0
	move.w	d0,DSS_w5CC
	rts

DSS_C5AE
	move.w	DSS_w5C0,DSS_w5CC
	rts

DSS_w5BC	dc.w	0
dss_Counter	dc.w	0
DSS_w5C0	dc.w	0
DSS_l5C2	dc.l	0
DSS_w5C6	dc.w	0
DSS_b5C8	dc.b	0
DSS_b5C9	dc.b	0
DSS_w5CA	dc.w	0
DSS_w5CC	dc.w	0

DSS_l5CE	dc.l	0,0,0,0,0
		dc.l	$10000
		dc.l	0,0,0,0,0
		dc.l	$20000
		dc.l	0,0,0,0,0
		dc.l	$40000
		dc.l	0,0,0,0,0
		dc.l	$80000
		dc.w	$6B0
DSS_l630	dc.l	$06B00650,$05F405A0,$054C0500,$04B80474,$043403F8
		dc.l	$03C0038A,$03580328,$02FA02D0,$02A60280,$025C023A
		dc.l	$021A01FC,$01E001C5,$01AC0194,$017D0168,$01530140
		dc.l	$012E011D,$010D00FE,$00F000E2,$00D600CA,$00BE00B4
		dc.l	$00AA00A0,$0097008F,$0087007F,$00780071,$00710071
		dc.l	$00710071,$00710071,$00710071,$00710071,$00710071
		dc.l	$00710071,$00710071,$00710071,$00710071,$00710071
		dc.l	$00710071,$00710071,$00710071,$00710071
DSS_l6CC	ds.l	31
;********************************************
;*** END OF DIGITAL SOUND STUDIO REPLAYER ***
;********************************************

;*******************************************
;*** FUTURE COMPOSER V1.4 REPLAY ROUTINE ***
;*******************************************
FC14END_MUSIC:
	clr.w FC14onoff
	bra	shutoff

FC14INIT_MUSIC:
	move.w #1,FC14onoff
	bset #1,$bfe001
	move.l	_CurrentMod,a0
	lea	20(a0),a0
	lea 180(a0),a1
	move.l a1,FC14SeqPoint
	move.l a0,a1
	add.l 8(a0),a1
	move.l a1,FC14PatPoint
	move.l a0,a1
	add.l 16(a0),a1
	move.l a1,FC14FRQPoint
	move.l a0,a1
	add.l 24(a0),a1
	move.l a1,FC14VOLPoint
	move.l 4(a0),d0
	divu #13,d0

	lea 40(a0),a1
	lea FC14SOUNDINFO+4(pc),a2
	moveq #10-1,d1
FC14initloop:
	move.w (a1)+,(a2)+
	move.l (a1)+,(a2)+
	adda.w #10,a2
	dbf d1,FC14initloop
	move.l a0,d1
	add.l 32(a0),d1
	lea FC14SOUNDINFO(pc),a3
	move.l d1,(a3)+
	moveq #9-1,d3
	moveq #0,d2
FC14initloop1:
	move.w (a3),d2
	add.l d2,d1
	add.l d2,d1
	addq.l #2,d1
	adda.w #12,a3
	move.l d1,(a3)+
	dbf d3,FC14initloop1

	lea 100(a0),a1
	lea FC14SOUNDINFO+160(pc),a2
	move.l a0,a3
	add.l 36(a0),a3

	moveq #80-1,d1
	moveq #0,d2
FC14initloop2:
	move.l a3,(a2)+
	move.b (a1)+,d2
	move.w d2,(a2)+
	clr.w (a2)+
	move.w d2,(a2)+
	addq.w #6,a2
	add.w d2,a3
	add.w d2,a3
	dbf d1,FC14initloop2

	move.l FC14SeqPoint(pc),a0
	moveq #0,d2
	move.b 12(a0),d2
	bne.s FC14speedok
	move.b #3,d2
FC14speedok:
	move.w d2,FC14respcnt
	move.w d2,FC14repspd
FC14INIT2:
	clr.w FC14audtemp
	move.w #$000f,$dff096
	move.w #$0780,$dff09a
	moveq #0,d7
	mulu #13,d0
	moveq #4-1,d6
	lea FC14V1data(pc),a0
	lea FC14silent(pc),a1
	lea FC14Chandata(pc),a2
FC14initloop3:
	move.l a1,10(a0)
	move.l a1,18(a0)
	clr.w 4(a0)
	move.w #$000d,6(a0)
	clr.w 8(a0)
	clr.l 14(a0)
	move.b #$01,23(a0)
	move.b #$01,24(a0)
	clr.b 25(a0)
	clr.l 26(a0)
	clr.w 30(a0)
	clr.l 38(a0)
	clr.w 42(a0)
	clr.l 44(a0)
	clr.l 48(a0)
	clr.w 56(a0)
	moveq #$00,d3
	move.w (a2)+,d1
	move.w (a2),d3
	divu #$0003,d3
	moveq #0,d4
	bset d3,d4
	move.w d4,32(a0)
	move.w (a2)+,d3
	andi.l #$00ff,d3
	andi.l #$00ff,d1
	lea $dff0a0,a6
	add.w d1,a6
	move.l #$0000,(a6)
	move.w #$0100,4(a6)
	move.w #$0000,6(a6)
	move.w #$0000,8(a6)
	move.l a6,60(a0)
	move.l FC14SeqPoint(pc),(a0)
	move.l FC14SeqPoint(pc),52(a0)
	add.l d0,52(a0)
	add.l d3,52(a0)
	add.l d7,(a0)
	add.l d3,(a0)
	move.l (a0),a3
	move.b (a3),d1
	andi.l #$00ff,d1
	lsl.w #6,d1
	move.l FC14PatPoint(pc),a4
	adda.w d1,a4
	move.l a4,34(a0)
	move.b 1(a3),44(a0)
	move.b 2(a3),22(a0)
	lea $4a(a0),a0
	dbf d6,FC14initloop3
	rts

FC14PLAY_MUSIC:
	lea FC14audtemp(pc),a5
	tst.w 8(a5)
	bne.s FC14music_on
	rts
FC14music_on:
	moveq #6,d6
	subq.w #1,4(a5)
	bne.s FC14nonewnote
	move.w 6(a5),4(a5)
	moveq #0,d5
	lea FC14V1data(pc),a0
	bsr FC14new_note
	lea FC14V2data(pc),a0
	bsr FC14new_note
	lea FC14V3data(pc),a0
	bsr FC14new_note
	lea FC14V4data(pc),a0
	bsr FC14new_note
FC14nonewnote:
	clr.w (a5)
	lea $dff000,a6
	lea FC14V1data(pc),a0
	bsr FC14effects
	move.l d0,$a6(a6)
	lea FC14V2data(pc),a0
	bsr FC14effects
	move.l d0,$b6(a6)
	lea FC14V3data(pc),a0
	bsr FC14effects
	move.l d0,$c6(a6)
	lea FC14V4data(pc),a0
	bsr FC14effects
	move.l d0,$d6(a6)
	lea FC14V1data(pc),a0
	move.l 68(a0),a1
	adda.w 64(a0),a1
	move.l 68+74(a0),a2
	adda.w 64+74(a0),a2
	move.l 68+148(a0),a3
	adda.w 64+148(a0),a3
	move.l 68+222(a0),a4
	adda.w 64+222(a0),a4
	move.w 66(a0),d1
	move.w 66+74(a0),d2
	move.w 66+148(a0),d3
	move.w 66+222(a0),d4
	moveq #2,d0
	moveq #0,d5
	move.w (a5),d7
	ori.w #$8000,d7
	move.w d7,$dff096
FC14chan1:
	lea FC14V1data+72(pc),a0
	move.w (a0),d7
	beq.s FC14chan2
	subq.w #1,(a0)
	cmp.w d0,d7
	bne.s FC14chan2
	move.w d5,(a0)
	move.l a1,$a0(a6)
	move.w d1,$a4(a6)
FC14chan2:
	lea FC14V2data+72(pc),a0
	move.w (a0),d7
	beq.s FC14chan3
	subq.w #1,(a0)
	cmp.w d0,d7
	bne.s FC14chan3
	move.w d5,(a0)
	move.l a2,$b0(a6)
	move.w d2,$b4(a6)
FC14chan3:
	lea FC14V3data+72(pc),a0
	move.w (a0),d7
	beq.s FC14chan4
	subq.w #1,(a0)
	cmp.w d0,d7
	bne.s FC14chan4
	move.w d5,(a0)
	move.l a3,$c0(a6)
	move.w d3,$c4(a6)
FC14chan4:
	lea FC14V4data+72(pc),a0
	move.w (a0),d7
	beq.s FC14endplay
	subq.w #1,(a0)
	cmp.w d0,d7
	bne.s FC14endplay
	move.w d5,(a0)
	move.l a4,$d0(a6)
	move.w d4,$d4(a6)
FC14endplay:
	rts

FC14new_note:
	move.l 34(a0),a1
	adda.w 40(a0),a1
	cmp.b #$49,(a1)
	beq.s FC14patend
	cmp.w #64,40(a0)
	bne FC14samepat
FC14patend:
	move.w d5,40(a0)
	move.l (a0),a2
	adda.w 6(a0),a2
	cmpa.l 52(a0),a2
	bne.s FC14notend
	move.w d5,6(a0)
	move.l (a0),a2
FC14notend:
	lea FC14spdtemp(pc),a3
	moveq #1,d1
	addq.b #1,(a3)
	cmpi.b #5,(a3)
	bne.s FC14nonewspd
	move.b d1,(a3)
	move.b 12(a2),d1
	beq.s FC14nonewspd
	move.w d1,2(a3)
	move.w d1,4(a3)
FC14nonewspd:
	move.b (a2)+,d1
	move.b (a2)+,44(a0)
	move.b (a2)+,22(a0)
	lsl.w d6,d1
	move.l FC14PatPoint(pc),a1
	add.w d1,a1
	move.l a1,34(a0)
	addi.w #$000d,6(a0)
FC14samepat:
	move.b 1(a1),d1
	move.b (a1)+,d0
	bne.s FC14ww1
	andi.w #%11000000,d1
	beq.s FC14noport
	bra.s FC14ww11
FC14ww1:
	move.w d5,56(a0)
FC14ww11:
	move.b d5,47(a0)
	btst #7,d1
	beq.s FC14noport
	move.b 2(a1),47(a0)
FC14noport:
	andi.w #$007f,d0
	beq FC14nextnote
	move.b d0,8(a0)
	move.b (a1),d1
	move.b d1,9(a0)
	move.w 32(a0),d3
	or.w d3,(a5)
	move.w d3,$dff096
	andi.w #$003f,d1
	add.b 22(a0),d1
	move.l FC14VOLPoint(pc),a2
	lsl.w d6,d1
	adda.w d1,a2
	move.w d5,16(a0)
	move.b (a2),23(a0)
	move.b (a2)+,24(a0)
	moveq #0,d1
	move.b (a2)+,d1
	move.b (a2)+,27(a0)
	move.b #$40,46(a0)
	move.b (a2),28(a0)
	move.b (a2)+,29(a0)
	move.b (a2)+,30(a0)
	move.l a2,10(a0)
	move.l FC14FRQPoint(pc),a2
	lsl.w d6,d1
	adda.w d1,a2
	move.l a2,18(a0)
	move.w d5,50(a0)
	move.b d5,25(a0)
	move.b d5,26(a0)
FC14nextnote:
	addq.w #2,40(a0)
	rts

FC14effects:
	moveq #0,d7
FC14testsustain:
	tst.b 26(a0)
	beq.s FC14sustzero
	subq.b #1,26(a0)
	bra FC14VOLUfx
FC14sustzero:
	move.l 18(a0),a1
	adda.w 50(a0),a1
FC14testeffects:
	cmpi.b #$e1,(a1)
	beq FC14VOLUfx
	move.b (a1),d0
	cmpi.b #$e0,d0
	bne.s FC14testnewsound
	move.b 1(a1),d1
	andi.w #$003f,d1
	move.w d1,50(a0)
	move.l 18(a0),a1
	adda.w d1,a1
	move.b (a1),d0
FC14testnewsound:
	cmpi.b #$e2,d0
	bne.s FC14testE4
	move.w 32(a0),d1
	or.w d1,(a5)
	move.w d1,$dff096
	moveq #0,d0
	move.b 1(a1),d0
	lea FC14SOUNDINFO(pc),a4
	lsl.w #4,d0
	adda.w d0,a4
	move.l 60(a0),a3
	move.l (a4)+,d1
	move.l d1,(a3)
	move.l d1,68(a0)
	move.w (a4)+,4(a3)
	move.l (a4),64(a0)
	move.w #$0003,72(a0)
	move.w d7,16(a0)
	move.b #$01,23(a0)
	addq.w #2,50(a0)
	bra FC14transpose
FC14testE4:
	cmpi.b #$e4,d0
	bne.s FC14testE9
	moveq #0,d0
	move.b 1(a1),d0
	lea FC14SOUNDINFO(pc),a4
	lsl.w #4,d0
	adda.w d0,a4
	move.l 60(a0),a3
	move.l (a4)+,d1
	move.l d1,(a3)
	move.l d1,68(a0)
	move.w (a4)+,4(a3)
	move.l (a4),64(a0)
	move.w #$0003,72(a0)
	addq.w #2,50(a0)
	bra FC14transpose
FC14testE9:
	cmpi.b #$e9,d0
	bne FC14testpatjmp
	move.w 32(a0),d1
	or.w d1,(a5)
	move.w d1,$dff096
	moveq #0,d0
	move.b 1(a1),d0
	lea FC14SOUNDINFO(pc),a4
	lsl.w #4,d0
	adda.w d0,a4
	move.l (a4),a2
	cmpi.l #"SSMP",(a2)+
	bne.s FC14nossmp
	lea 320(a2),a4
	moveq #0,d1
	move.b 2(a1),d1
	lsl.w #4,d1
	add.w d1,a2
	add.l (a2),a4
	move.l 60(a0),a3
	move.l a4,(a3)
	move.l 4(a2),4(a3)
	move.l a4,68(a0)
	move.l 6(a2),64(a0)
	move.w d7,16(a0)
	move.b #1,23(a0)
	move.w #3,72(a0)
FC14nossmp:
	addq.w #3,50(a0)
	bra.s FC14transpose
FC14testpatjmp:
	cmpi.b #$e7,d0
	bne.s FC14testpitchbend
	moveq #0,d0
	move.b 1(a1),d0
	lsl.w d6,d0
	move.l FC14FRQPoint(pc),a1
	adda.w d0,a1
	move.l a1,18(a0)
	move.w d7,50(a0)
	bra FC14testeffects
FC14testpitchbend:
	cmpi.b #$ea,d0
	bne.s FC14testnewsustain
	move.b 1(a1),4(a0)
	move.b 2(a1),5(a0)
	addq.w #3,50(a0)
	bra.s FC14transpose
FC14testnewsustain:
	cmpi.b #$e8,d0
	bne.s FC14testnewvib
	move.b 1(a1),26(a0)
	addq.w #2,50(a0)
	bra FC14testsustain
FC14testnewvib:
	cmpi.b #$e3,(a1)+
	bne.s FC14transpose
	addq.w #3,50(a0)
	move.b (a1)+,27(a0)
	move.b (a1),28(a0)
FC14transpose:
	move.l 18(a0),a1
	adda.w 50(a0),a1
	move.b (a1),43(a0)
	addq.w #1,50(a0)

FC14VOLUfx:
	tst.b 25(a0)
	beq.s FC14volsustzero
	subq.b #1,25(a0)
	bra FC14calcperiod
FC14volsustzero:
	tst.b 15(a0)
	bne.s FC14do_VOLbend
	subq.b #1,23(a0)
	bne.s FC14calcperiod
	move.b 24(a0),23(a0)
FC14volu_cmd:
	move.l 10(a0),a1
	adda.w 16(a0),a1
	move.b (a1),d0
FC14testvoluend:
	cmpi.b #$e1,d0
	beq.s FC14calcperiod
	cmpi.b #$ea,d0
	bne.s FC14testVOLsustain
	move.b 1(a1),14(a0)
	move.b 2(a1),15(a0)
	addq.w #3,16(a0)
	bra.s FC14do_VOLbend
FC14testVOLsustain:
	cmpi.b #$e8,d0
	bne.s FC14testVOLloop
	addq.w #2,16(a0)
	move.b 1(a1),25(a0)
	bra.s FC14calcperiod
FC14testVOLloop:
	cmpi.b #$e0,d0
	bne.s FC14setvolume
	move.b 1(a1),d0
	andi.w #$003f,d0
	subq.b #5,d0
	move.w d0,16(a0)
	bra.s FC14volu_cmd
FC14do_VOLbend:
	not.b 38(a0)
	beq.s FC14calcperiod
	subq.b #1,15(a0)
	move.b 14(a0),d1
	add.b d1,45(a0)
	bpl.s FC14calcperiod
	moveq #0,d1
	move.b d1,15(a0)
	move.b d1,45(a0)
	bra.s FC14calcperiod
FC14setvolume:
	move.b (a1),45(a0)
	addq.w #1,16(a0)
FC14calcperiod:
	move.b 43(a0),d0
	bmi.s FC14lockednote
	add.b 8(a0),d0
	add.b 44(a0),d0
FC14lockednote:
	moveq #$7f,d1
	and.l d1,d0
	lea FC14PERIODS(pc),a1
	add.w d0,d0
	move.w d0,d1
	adda.w d0,a1
	move.w (a1),d0
	move.b 46(a0),d7
	tst.b 30(a0)
	beq.s FC14vibrator
	subq.b #1,30(a0)
	bra.s FC14novibrato
FC14vibrator:
	moveq #5,d2
	move.b d1,d5
	move.b 28(a0),d4
	add.b d4,d4
	move.b 29(a0),d1
	tst.b d7
	bpl.s FC14vib1
	btst #0,d7
	bne.s FC14vib4
FC14vib1:
	btst d2,d7
	bne.s FC14vib2
	sub.b 27(a0),d1
	bcc.s FC14vib3
	bset d2,d7
	moveq #0,d1
	bra.s FC14vib3
FC14vib2:
	add.b 27(a0),d1
	cmp.b d4,d1
	bcs.s FC14vib3
	bclr d2,d7
	move.b d4,d1
FC14vib3:
	move.b d1,29(a0)
FC14vib4:
	lsr.b #1,d4
	sub.b d4,d1
	bcc.s FC14vib5
	subi.w #$0100,d1
FC14vib5:
	addi.b #$a0,d5
	bcs.s FC14vib7
FC14vib6:
	add.w d1,d1
	addi.b #$18,d5
	bcc.s FC14vib6
FC14vib7:
	add.w d1,d0
FC14novibrato:
	eori.b #$01,d7
	move.b d7,46(a0)
	not.b 39(a0)
	beq.s FC14pitchbend
	moveq #0,d1
	move.b 47(a0),d1
	beq.s FC14pitchbend
	cmpi.b #$1f,d1
	bls.s FC14portaup
FC14portadown:
	andi.w #$1f,d1
	neg.w d1
FC14portaup:
	sub.w d1,56(a0)
FC14pitchbend:
	not.b 42(a0)
	beq.s FC14addporta
	tst.b 5(a0)
	beq.s FC14addporta
	subq.b #1,5(a0)
	moveq #0,d1
	move.b 4(a0),d1
	bpl.s FC14pitchup
	ext.w d1
FC14pitchup:
	sub.w d1,56(a0)
FC14addporta:
	add.w 56(a0),d0
	cmpi.w #$0070,d0
	bhi.s FC14nn1
	move.w #$0071,d0
FC14nn1:
	cmpi.w #$0d60,d0
	bls.s FC14nn2
	move.w #$0d60,d0
FC14nn2:
	swap d0
	move.b 45(a0),d0
	rts



FC14V1data:  ds.b 64
FC14offset1: ds.b 02
FC14ssize1:  ds.b 02
FC14start1:  ds.b 06

FC14V2data:  ds.b 64
FC14offset2: ds.b 02
FC14ssize2:  ds.b 02
FC14start2:  ds.b 06

FC14V3data:  ds.b 64
FC14offset3: ds.b 02
FC14ssize3:  ds.b 02
FC14start3:  ds.b 06

FC14V4data:  ds.b 64
FC14offset4: ds.b 02
FC14ssize4:  ds.b 02
FC14start4:  ds.b 06

FC14audtemp: dc.w 0
FC14spdtemp: dc.w 0
FC14respcnt: dc.w 0
FC14repspd:  dc.w 0
FC14onoff:   dc.w 0

FC14Chandata: dc.l $00000000,$00100003,$00200006,$00300009
FC14SeqPoint: dc.l 0
FC14PatPoint: dc.l 0
FC14FRQPoint: dc.l 0
FC14VOLPoint: dc.l 0


FC14silent: dc.w $0100,$0000,$0000,$00e1

FC14PERIODS:
	dc.w $06b0,$0650,$05f4,$05a0,$054c,$0500,$04b8,$0474
	dc.w $0434,$03f8,$03c0,$038a,$0358,$0328,$02fa,$02d0
	dc.w $02a6,$0280,$025c,$023a,$021a,$01fc,$01e0,$01c5
	dc.w $01ac,$0194,$017d,$0168,$0153,$0140,$012e,$011d
	dc.w $010d,$00fe,$00f0,$00e2,$00d6,$00ca,$00be,$00b4
	dc.w $00aa,$00a0,$0097,$008f,$0087,$007f,$0078,$0071
	dc.w $0071,$0071,$0071,$0071,$0071,$0071,$0071,$0071
	dc.w $0071,$0071,$0071,$0071,$0d60,$0ca0,$0be8,$0b40
	dc.w $0a98,$0a00,$0970,$08e8,$0868,$07f0,$0780,$0714
	dc.w $1ac0,$1940,$17d0,$1680,$1530,$1400,$12e0,$11d0
	dc.w $10d0,$0fe0,$0f00,$0e28,$06b0,$0650,$05f4,$05a0
	dc.w $054c,$0500,$04b8,$0474,$0434,$03f8,$03c0,$038a
	dc.w $0358,$0328,$02fa,$02d0,$02a6,$0280,$025c,$023a
	dc.w $021a,$01fc,$01e0,$01c5,$01ac,$0194,$017d,$0168
	dc.w $0153,$0140,$012e,$011d,$010d,$00fe,$00f0,$00e2
	dc.w $00d6,$00ca,$00be,$00b4,$00aa,$00a0,$0097,$008f
	dc.w $0087,$007f,$0078,$0071

FC14SOUNDINFO:
	ds.b 10*16
	ds.b 80*16

;********************************************
;*** END OF FUTURE COMPOSER V1.4 REPLAYER ***
;********************************************

;*****************************
;*** MARKII REPLAY ROUTINE ***
;*****************************
PlayMark2
	move.l	_CurrentMod,a0
	lea	20(a0),a0
	moveq	#0,d0
	moveq	#1,d1
	jsr	(a0)
	rts

InitMark2
	move.l	_CurrentMod,a0
	lea	20(a0),a0
	moveq	#-1,d0
	jsr	(a0)
	rts

StopMark2
	move.l	_CurrentMod,a0
	lea	20(a0),a0
	moveq	#1,d0
	moveq	#1,d1
	jsr	(a0)
	bra	shutoff
	rts
;******************************
;*** END OF MARKII REPLAYER ***
;******************************
