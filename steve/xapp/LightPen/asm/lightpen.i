
; Struct for the lightpen interface
; Date 21 june 1994

	RSRESET
lp_struct:		rs.w	0
lp_x:					rs.l	1
lp_y:					rs.l	1
lp_oldx:				rs.l	1
lp_oldy:				rs.l	1
lp_offset_x:	rs.w	1
lp_offset_y:	rs.w	1
lp_signal:		rs.l	1
lp_signum:		rs.l	1
lp_task:			rs.l	1
lp_int_50hz:	rs.b	IS_SIZE
lp_int_TimA:	rs.b	IS_SIZE
lp_SIZEOF:		rs.w	0