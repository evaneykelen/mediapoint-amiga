* Fast bitmap structures

*
* structure for holding background or other temporary bitmaps
* there is no shifting all words are remembered
*
	RSRESET
fstore_struct:	rs.w	0
fstore_width:	rs.w	1		; pixels
fstore_width_w:rs.w	1		; words
fstore_height:	rs.w	1
fstore_depth:	rs.w	1
fstore_smask:	rs.w	1		; start mask
fstore_emask:	rs.w	1		; end mask
fstore_offset:	rs.l	1	; byte offset in bitmap
fstore_modulo:	rs.w	1
fstore_x:		rs.w	1		; x coordinate in source
fstore_y:		rs.w	1		; y coordinate for place back
fstore_size:	rs.l	1
fstore_planes:	rs.l	8
fstore_SIZEOF:	rs.w	0

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

