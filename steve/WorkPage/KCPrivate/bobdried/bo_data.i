; structure for the scroll xapp

	RSRESET
vi_block:		rs.w	0
vi_view:		rs.b	v_SIZEOF
vi_viewport:		rs.b	vp_SIZEOF
vi_rasinfo:		rs.b	ri_SIZEOF
vi_bitmap:		rs.b	bm_SIZEOF
vi_ColorMap:	rs.l	1
vi_vpextra:		rs.l	1
vi_vextra:		rs.l	1
vi_mem:			rs.l	1
vi_SIZEOF:		rs.w	0

	RSRESET
cb_data:		rs.w	0
cb_shiftdest:	rs.l	1
cb_Alastword:	rs.w	1
cb_max_x:	rs.w	1
cb_max_y:	rs.w	1
cb_tags:		rs.l	1
cb_rows:		rs.w	1
cb_columns:	rs.w	1
cb_viewx:	rs.w	1
cb_viewy:	rs.w	1
cb_monid:	rs.l	1
cb_monitorspec:	rs.l	1
cb_or_vmode_mask:	rs.w	1
cb_dimquery:	rs.b	dim_SIZEOF
cb_and_vmode_mask:	rs.w	1
cb_filenamepointer:	rs.l	1
cb_filename:		rs.l	1
cb_frame_pointer:	rs.l	0
cb_linesize:		rs.l	1
cb_colors:		rs.l	1
cb_coltable:		rs.l	1
cb_oldview:		rs.l	1
cb_dosbase:		rs.l	1
cb_graphbase:		rs.l	1
cb_intbase:		rs.l	1
cb_fontbase:		rs.l	1
cb_crawl_rec:		rs.l	1
cb_fontname:		rs.l	1

cb_active_view:	rs.l	1
cb_inactive_view:	rs.l	1
cb_starty:		rs.w	1
cb_delta:		rs.w	1
cb_breedte_x:		rs.l	1
cb_planes:		rs.w	1
cb_leny:		rs.w	1
cb_lenx:		rs.w	1
cb_lenviewx:		rs.w	1
cb_lenviewy:		rs.w	1
cb_mode:		rs.w	1
cb_init_rasty:	rs.w	1
cb_rastx:		rs.w	1
cb_rasty:		rs.w	1
cb_rast_secx:		rs.w	1
cb_rast_secy:		rs.w	1
cb_baseline:		rs.w	1
cb_speed:		rs.w	1
cb_offset_y:		rs.w	1
cb_bitmapsize:	rs.l	1
cb_bitmapheight:	rs.l	1
cb_middle:			rs.l	1
cb_middle_gap:			rs.l	1
cb_intergap:			rs.l	1
cb_allign:			rs.w	1

cb_screenmem:		rs.l	1
cb_memsize:		rs.l	1
cb_plane_size:		rs.l	1

cb_fontpointer:	rs.l	1
cb_fontsize:		rs.w	1
cb_fontheight:	rs.w	1
cb_fontname_buffer:	rs.b	100
cb_fontchange:	rs.b	1
cb_red:			rs.b	1
cb_green:		rs.b	1
cb_blue:			rs.b	1
cb_current_y:		rs.l	1

cb_filehandle:	rs.l	1
cb_filesize:		rs.l	1
cb_filelock:		rs.l	1
cb_filemem:		rs.l	1
cb_filememsize:	rs.l	1
cb_config_size:	rs.l	1

cb_charpointer:	rs.l	1
cb_text_data:		rs.l	1
cb_waitmask:		rs.l	1
cb_loaded:		rs.b	1
cb_v39_present:	rs.b	1
cb_aa_present:	rs.b	1
cb_from_file:		rs.b	1
cb_colormap:		rs.l	1
cb_rastport:		rs.b	rp_SIZEOF
cb_fontattr:		rs.b	ta_SIZEOF
cb_viewblock1:	rs.b	vi_SIZEOF
cb_viewblock2:	rs.b	vi_SIZEOF
cb_SIZEOF		rs.w	0
