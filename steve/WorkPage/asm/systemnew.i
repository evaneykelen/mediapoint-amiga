; MLSystem asm include file
; 2 - 11 - 1992

	IFND	GRAPHICS_GFX_I
	INCLUDE	graphics/gfx.i
	ENDC

	IFND	GRAPHICS_COPPER_I
	INCLUDE	graphics/copper.i
	ENDC

	RSRESET
eff_nums		rs.b	0
en_number:	rs.w	1
en_speed:		rs.w	1
en_chunk:		rs.w	1
en_vari:		rs.w	1
en_type:		rs.w	1
en_SIZEOF:	rs.w	0

MAXSCREENBUFS equ 3

	RSRESET
DISPLAY		rs.b	0
dp_View		rs.b	v_SIZEOF
dp_ViewPort	rs.b	vp_SIZEOF
dp_RasInfo	rs.b	ri_SIZEOF
dp_RastPort	rs.b	rp_SIZEOF
dp_BitMap	rs.b	bm_SIZEOF
dp_ColorMap	rs.l	1
dp_vpextra	rs.l	1
dp_vextra		rs.l	1
dp_SIZEOF	rs.w	0
	RSRESET

SCREENBUFFER	rs.b	0
sbu_Base			rs.l	1
sbu_Size			rs.l	1
sbu_Viewed		rs.w	1
sbu_InUse			rs.w	1
sbu_Display		rs.b	dp_SIZEOF
sbu_SIZEOF		rs.w	0
	RSRESET


	RSRESET
MLSYSTEM		rs.w	0
ml_buffers	rs.b	MAXSCREENBUFS*sbu_SIZEOF
ml_monitor_ID	rs.l	1
ml_miscFlags	rs.l	1
ml_extend		rs.l	8
ml_taglist		rs.l	1
ml_VIList		rs.l	1
ml_sema_trans:	rs.b	SS_SIZE
ml_sema_music:	rs.b		SS_SIZE
ml_text:			rs.b	150
ml_refreshRate:	rs.w	1
ml_SIZEOF		rs.w	0

