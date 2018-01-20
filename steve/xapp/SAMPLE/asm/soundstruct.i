SI_NORMAL = $01
SI_FAST = $02
SI_DISK = $04
SI_SEQ = $08
SI_FIBO = $10
SI_RAW = $20
SI_LOOPING = $40
SI_STEREO = $80

	RSRESET
cd_chipdat:		rs.w	0
cd_chip1:		rs.l	1
cd_chip2:		rs.l	1
cd_chipS1:		rs.l	1
cd_chipS2:		rs.l	1
cd_mem:			rs.l	1
cd_memS			rs.l	1
cd_SIZEOF:		rs.w	0

	RSRESET
si_soundinfo:		rs.w	0
si_sounddata:		rs.l	1
si_sondlength:		rs.l	1
si_soundoffset:	rs.l	1
si_fp:				rs.l	1
si_start_offset:	rs.l	1
si_play_offset:	rs.l	1
si_seq_data:		rs.l	1
si_seq_offset:		rs.l	1
si_seq_length:		rs.l	1
si_seq_play:	rs.l	1
si_loop:				rs.w	1
si_period:			rs.w	1
si_loops:			rs.w	1
si_channel:			rs.w	1
si_type:				rs.b	1
si_end:				rs.b	1
si_blockcount:		rs.w	1
si_sigtest:		rs.w	1
si_audiosig:		rs.l	1
si_audionum:		rs.l	1
si_fadesig:			rs.l	1
si_fadenum:			rs.l	1
si_task:				rs.l	1
si_chipsize:		rs.l	1
si_chipdat:			rs.b	cd_SIZEOF
si_dummy:			rs.l	1
si_int_audio:		rs.b	IS_SIZE
si_int_fade:		rs.b	IS_SIZE
si_oldaudio:		rs.l	1
si_DOSBase:			rs.l	1
si_filter				rs.w	1
si_vol_right:		rs.w	1
si_vol_left:		rs.w	1
si_vol_temp_right:rs.w	1
si_vol_temp_left:	rs.w	1
si_inc_right:		rs.w	1
si_inc_left:		rs.w	1
si_SIZEOF:			rs.w	0

