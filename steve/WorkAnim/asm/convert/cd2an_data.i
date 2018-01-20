
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
ab_data:				rs.w	0
ab_packed:			rs.l	1
ab_packed_size:	rs.l	1
ab_easy_exit:		rs.l	1
ab_frame_counter:	rs.l	1
ab_dpan_count:		rs.l	1
ab_relfpointer:	rs.l	1
ab_frames:			rs.l	1
ab_msgpointer:		rs.l	1
ab_dosbase:			rs.l	1
ab_graphbase:		rs.l	1
ab_intbase:			rs.l	1
ab_conhandle:		rs.l	1
ab_filehandle:		rs.l	1
ab_filenaam:		rs.l	1
ab_filehandle_out:	rs.l	1
ab_filenaam_out:		rs.l	1
ab_unpackedsecond:	rs.l	1
ab_unpacked:		rs.l	1
ab_unpacked_size:	rs.l	1
ab_color_count:	rs.l	1
ab_colormapsize:	rs.l	1
ab_audiosize:		rs.l	1
ab_buffer:			rs.b	12
ab_lenx:				rs.w	1
ab_leny:				rs.w	1
ab_breedte_x:		rs.l	1
ab_planes:			rs.w	1
ab_picture:			rs.b	1
ab_continue:		rs.b	1
ab_info:				rs.b	1
ab_lead:				rs.b	1
ab_hires:			rs.b	1
ab_interlace:		rs.b	1
ab_mode:				rs.w	1
ab_compression:	rs.b	1
ab_masking:			rs.b	1
ab_loaded:			rs.b	1
ab_diskloaded:		rs.b	1
ab_nonleaved:		rs.b	1
ab_bits:				rs.b	1
ab_lize:				rs.b	1
ab_quit:				rs.b	1
ab_more_cpu:		rs.b	1
ab_show:				rs.b	1
ab_acompression:	rs.b	1
ab_ainterleaved:	rs.b	1
ab_cmap_changed:	rs.b	1
ab_bmhd_changed:	rs.b	1
ab_cmap_found:		rs.b	1
ab_bmhd_found:		rs.b	1
ab_looping:			rs.b	1
ab_diskanim:		rs.b	1
ab_XORmode:			rs.b	1
ab_Xoff:				rs.b	1
ab_Yoff:				rs.b	1
ab_libversion:		rs.b	1
ab_aa_present:		rs.b	1
ab_v39_present:	rs.b	1
ab_columns:			rs.w	1
ab_rows:				rs.w	1

ab_temp_formpointer:		rs.l	1
ab_temp_formdisksize:	rs.l	1
ab_global_diskoffset:	rs.l	1

ab_num_rot:					rs.l	1
ab_rot_store:				rs.l	1
ab_head_frame_list:		rs.l	1
ab_temp_frame_list:		rs.l	1
ab_col_pointer:			rs.l	1
ab_overallspeed:			rs.l	1
ab_old_speed:				rs.l	1
ab_oldview:					rs.l	1
ab_w_bitpointer:			rs.l	1
ab_frame_show:				rs.l	1
ab_frame_hidden:			rs.l	1

ab_rowmulti:				rs.l	1
ab_frame_pointer:			rs.l	1

ab_view_visible:			rs.l	1
ab_view_hidden:			rs.l	1
ab_wview1:					rs.l	1
ab_wview2:					rs.l	1
ab_view1:					rs.b	vi_SIZEOF
ab_view2:					rs.b	vi_SIZEOF

ab_reltime:			rs.l	1
ab_temp_formsize:	rs.l	1
ab_FORM_counter:	rs.l	1
ab_frame_speed:	rs.l	1
ab_multabel:		rs.l	581
ab_SIZEOF:			rs.w	0
