 
* aw_data for the transitions XAPP anim in window play

	RSRESET
aw_data:				rs.w	0
aw_bltcoords:		rs.l	6
							rs.l	2					; debug dummy
aw_memory:			rs.l	1
aw_memsize:			rs.l	1
aw_packed:			rs.l	1
aw_packed_size:	rs.l	1
aw_fastback:		rs.l	1
aw_fastbacksize:	rs.l	1
aw_frame_counter:	rs.l	1
aw_dpan_count:		rs.l	1
aw_relfpointer:	rs.l	1
aw_frames:			rs.l	1
aw_msgpointer:		rs.l	1
aw_sig_ptoc:		rs.l	1
aw_mlsysstruct:	rs.l	1
aw_waitmask:		rs.l	1
aw_dosbase:			rs.l	1
aw_graphbase:		rs.l	1
aw_intbase:			rs.l	1
aw_mlmmubase:		rs.l	1
aw_filehandle:		rs.l	1
aw_filenaam:		rs.l	1
aw_buffer:			rs.b	12
aw_lenx:				rs.w	1
aw_leny:				rs.w	1
aw_breedte_x:		rs.l	1
aw_planes:			rs.w	1
aw_screendepth:	rs.w	1
aw_hires:			rs.b	1
aw_interlace:		rs.b	1
aw_mode:				rs.w	1
aw_compression:	rs.b	1
aw_masking:			rs.b	1
aw_loaded:			rs.b	1
aw_diskloaded:		rs.b	1
aw_nonleaved:		rs.b	1
aw_bits:				rs.b	1
aw_lize:				rs.b	1
aw_quit:				rs.b	1
aw_more_cpu:		rs.b	1
aw_show:				rs.b	1
aw_acompression:	rs.b	1
aw_ainterleaved:	rs.b	1
aw_bmhd_changed:	rs.b	1
aw_bmhd_found:		rs.b	1
aw_diskanim:		rs.b	1
aw_XORmode:			rs.b	1
aw_background:		rs.b	1
aw_stay_on:					rs.b	1

aw_temp_formpointer:		rs.l	1
aw_temp_formdisksize:	rs.l	1
aw_global_diskoffset:	rs.l	1

aw_num_rot:					rs.l	1
aw_rot_store:				rs.l	1
aw_head_frame_list:		rs.l	1
aw_temp_frame_list:		rs.l	1
aw_col_pointer:			rs.l	1
aw_overallspeed:			rs.l	1
aw_old_speed:				rs.l	1
aw_w_bitpointer:			rs.l	1

aw_rowmulti:				rs.l	1
aw_frame_pointer:			rs.l	1

aw_view_visible:			rs.l	1
aw_view_hidden:			rs.l	1
aw_wview1:					rs.l	1
aw_wview2:					rs.l	1
aw_bitmap_animsize:		rs.l	1
aw_bitmap_pscr1:			rs.l	1
aw_bitmap_pscr2:			rs.l	1
aw_bitmap_pa1:				rs.l	1
aw_bitmap_pa2:				rs.l	1

aw_bitmapscreen1:		rs.b	bm_SIZEOF
aw_bitmapscreen2:		rs.b	bm_SIZEOF
aw_bitmapanim1:		rs.b	bm_SIZEOF
aw_bitmapanim2:		rs.b	bm_SIZEOF

aw_reltime:			rs.l	1
aw_temp_formsize:	rs.l	1
aw_FORM_counter:	rs.l	1
aw_frame_speed:	rs.l	1
aw_multabel:		rs.l	581
aw_SIZEOF:			rs.w	0
