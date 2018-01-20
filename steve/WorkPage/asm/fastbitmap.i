* Fast bitmap structures

*
* structure for holding background or other temporary bitmaps
* there is no shifting all words are remembered
*
	RSRESET
fbm_struct:	rs.w	0
fbm_width:	rs.w	1		; pixels
fbm_width_b:rs.w	1		; words
fbm_smask:	rs.w	1		; start mask
fbm_emask:	rs.w	1		; end mask
fbm_height:	rs.w	1
fbm_depth:	rs.w	1
fbm_size:		rs.l	1
fbm_planes:	rs.l	8
fbm_SIZEOF:	rs.w	0

*
* Structure for movement with 16 images one pixel shifted to the right
*
	RSRESET
mfbm_struct:	rs.w	0
mfbm_width:		rs.w	1		; pixels
mfbm_width_b:	rs.w	1		; words
mfbm_height:	rs.w	1
mfbm_depth:		rs.w	1
mfbm_size:		rs.l	1
mfbm_flags:		rs.l	1
mfbm_shifts:	rs.l	16
mfbm_SIZEOF:	rs.w	0

MFBM_MASKED = $1				; There is a mask interleaved
MFBM_INTERLEAVED = $2		; I am interleaved ( now always )

