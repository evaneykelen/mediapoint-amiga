SI_STEREO = $80

	RSRESET
si_soundinfo:		rs.w	0
si_sounddata:		rs.l	1
si_memsize:			rs.l	1
si_soundlength:	rs.l	1
si_period:			rs.w	1
si_channel:			rs.w	1
si_type:				rs.b	1
si_end:				rs.b	1
si_audiosig:		rs.l	1
si_audionum:		rs.l	1
si_task:				rs.l	1
						rs.l	1
si_int_audio:		rs.b	IS_SIZE
						rs.l	1
si_oldaudio:		rs.l	1
si_DOSBase:			rs.l	1
si_SIZEOF:			rs.w	0

