
	RSRESET
vi_block:		rs.w	0
vi_view:			rs.b	v_SIZEOF
vi_viewport:	rs.b	vp_SIZEOF
vi_rasinfo:		rs.b	ri_SIZEOF
vi_RastPort:	rs.b	rp_SIZEOF
vi_bitmap:		rs.b	bm_SIZEOF
vi_SIZEOF:		rs.w	0

* ab_data for the anim xapp

	RSRESET
ab_packed:			rs.l	1
ab_packed_size:	rs.l	1
ab_easy_exit:		rs.l	1
ab_double:			rs.l	1
ab_mod_offset:	rs.l	1
ab_ucop_pointer:	rs.l	1
ab_frame_counter:	rs.l	1
ab_msgpointer:		rs.l	1
ab_sig_ptoc:		rs.l	1
ab_mlsysstruct:	rs.l	1
ab_waitmask:		rs.l	1
ab_dosbase:			rs.l	1
ab_graphbase:		rs.l	1
ab_intbase:			rs.l	1
ab_conhandle:		rs.l	1
ab_filehandle:		rs.l	1
ab_filenaam:		rs.l	1
ab_unpackedsecond:	rs.l	1
ab_unpacked:		rs.l	1
ab_unpacked_size:	rs.l	1
ab_color_count:	rs.l	1
ab_buffer:			rs.b	12
ab_lenx:				rs.w	1
ab_leny:				rs.w	1
ab_breedte_x:		rs.l	1
ab_planes:			rs.w	1
ab_mode:				rs.w	1
ab_hires:			rs.b	1
ab_interlace:		rs.b	1
ab_loaded:			rs.b	1
ab_nonleaved:		rs.b	1
ab_quit:				rs.b	1
ab_more_cpu:		rs.b	1
ab_diskanim:		rs.b	1
ab_Xoff:				rs.b	1
ab_Yoff:				rs.b	1
ab_libversion:		rs.b	1
ab_aa_present:		rs.b	1
ab_v39_present:	rs.b	1
ab_info:				rs.b	1
ab_lead:				rs.b	1
ab_calc_cdtv:		rs.b	1
						rs.b	1
ab_columns:			rs.w	1
ab_rows:				rs.w	1

ab_frames:					rs.l	1
ab_num_rot:					rs.l	1
ab_rot_store:				rs.l	1
ab_col_pointer:			rs.l	1
ab_overallspeed:			rs.l	1
ab_oldview:					rs.l	1
ab_w_bitpointer:			rs.l	1
ab_frame_show:				rs.l	1
ab_frame_hidden:			rs.l	1

ab_frame_pointer:			rs.l	1

ab_filelock:				rs.l	1
ab_filesize:				rs.l	1
ab_image_size:				rs.l	1
ab_skip_size:				rs.l	1
ab_pansize:					rs.l	1
ab_num_cols:				rs.l	1
ab_low_hi:					rs.l	1
ab_loadmem:					rs.l	1
ab_loadmem_size:			rs.l	1
ab_loadbuf1:				rs.l	1
ab_loadbuf2:				rs.l	1
ab_ptype:					rs.b	1
ab_pinfo:					rs.b	1
ab_sigmode:					rs.b	1
ab_speed_override:		rs.b	1
ab_old_audio_0:			rs.l	1
ab_old_audio_1:			rs.l	1
ab_audio_size:				rs.w	1
ab_audio_skip:				rs.l	1

ab_dx:						rs.w	1
ab_dy:						rs.w	1
ab_view_visible:			rs.l	1
ab_view_hidden:			rs.l	1
ab_wview1:					rs.l	1
ab_wview2:					rs.l	1
ab_view1:					rs.b	vi_SIZEOF
ab_view2:					rs.b	vi_SIZEOF
ab_frame_speed:	rs.l	1
ab_SIZEOF:			rs.w	0
