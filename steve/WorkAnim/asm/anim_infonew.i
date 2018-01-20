
* Anim info structuur

	RSRESET
ani_info:				rs.w	0
ani_form:				rs.l	1
ani_size:				rs.l	1
ani_filepos:			rs.l	1
ani_type:				rs.w	1
ani_reltime:			rs.l	0
ani_lenx:				rs.w	1
ani_leny:				rs.w	1
ani_planes:			rs.w	0
ani_operation:		rs.b	1
ani_mask:				rs.b	1
ani_interleave:		rs.b	0
ani_masking:			rs.b	1
ani_bits:				rs.b	0
ani_compression:	rs.b	1
ani_cmap:				rs.l	1
ani_chunkpos:		rs.l	1
ani_chunksize:		rs.l	1
ani_camghires:		rs.b	1
ani_camglace:		rs.b	1
ani_camgmode:		rs.w	1
ani_next:				rs.l	1
ani_SIZEOF:			rs.w	0

BMHD_type = 1<<0
ANHD_type = 1<<1
CAMG_type = 1<<2
DLTA_type = 1<<3
BODY_type = 1<<4
