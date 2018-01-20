; Different structures passed to the assembly by the parser

	RSRESET
eff_parse:		rs.w	0
eff_innum:		rs.l	1
eff_inspeed:	rs.l	1
eff_inthick:	rs.l	1
eff_outnum:		rs.l	1
eff_outspeed:	rs.l	1
eff_outthick:	rs.l	1
eff_delay1:		rs.l	1
eff_delay2:		rs.l	1
eff_SIZEOF:		rs.l	0

	RSRESET

winp_parse:		rs.w	0
winp_x:			rs.l	1
winp_y:			rs.l	1
winp_wx:			rs.l	1
winp_wy:			rs.l	1
winp_tbc:		rs.l	1
winp_rbc:		rs.l	1
winp_bbc:		rs.l	1
winp_lbc:		rs.l	1
winp_bw:			rs.l	1
winp_ic:			rs.l	1
winp_it:			rs.l	1
winp_flags:		rs.l	1
winp_patnr:		rs.l	1
winp_tm:			rs.l	1
winp_rm:			rs.l	1
winp_bm:			rs.l	1
winp_lm:			rs.l	1
winp_sdep:		rs.l	1
winp_sdir:		rs.l	1
winp_spen:		rs.l	1
winp_eff:		rs.b	eff_SIZEOF
winp_SIZEOF:	rs.l	0

	RSRESET
clipp_parse:		rs.w	0
clipp_path:			rs.l	1
clipp_x:				rs.l	1
clipp_y:				rs.l	1
clipp_width:		rs.l	1		
clipp_height:		rs.l	1
clipp_fps:			rs.l	1
clipp_loops:		rs.l	1
clipp_disk:			rs.l	1
clipp_eff:			rs.b	eff_SIZEOF
clipp_SIZEOF:		rs.l	0

	RSRESET
clipa_parse:		rs.w	0
clipa_path:			rs.l	1
clipa_x:				rs.l	1
clipa_y:				rs.l	1
clipa_width:		rs.l	1		
clipa_height:		rs.l	1
clipa_fps:			rs.l	1
clipa_loops:		rs.l	1
clipa_disk:			rs.l	1
clipa_eff:			rs.b	eff_SIZEOF
clipa_SIZEOF:		rs.l	0

	RSRESET
textp_parse:		rs.w	0
textp_ed:			rs.l	1
textp_eff:			rs.b	eff_SIZEOF
textp_SIZEOF:		rs.l	0
