
	RSRESET
db_datablock:		rs.w	0
db_easy_exit:		rs.l	1
db_filename_pointer:	rs.l	1
db_oldview:		rs.l	1
db_dosbase:		rs.l	1
db_graphbase:		rs.l	1
db_mpmmubase:		rs.l	1
db_egsbase:		rs.l	1
db_egsblitbase:	rs.l	1
db_mpmmu_pointer:	rs.l	1
db_egsscreen_ptr:	rs.l	1
db_egs_width:	rs.l	1
db_egs_height:	rs.l	1
db_egs_offsetx:	rs.l	1
db_egs_offsety:	rs.l	1
db_newsize:		rs.l	1
db_newmem:		rs.l	1
db_unpacked:		rs.l	1
db_unpacked_size:	rs.l	1
db_waittofs:	rs.w	1
db_libversion:	rs.w	1
db_inactive_fileblok:	rs.l	1
db_Xoff:		rs.w	1
db_Yoff:		rs.w	1
db_colums:		rs.w	1
db_rows:		rs.w	1
db_skip_planes:		rs.w	1
db_colmap:		rs.l	1
db_data_pointer:	rs.l	1
db_dest_pointer:	rs.l	1
db_crng_found:	rs.b	1
db_drng_found:	rs.b	1
db_colcyc_point:	rs.l	1
db_colrate:	rs.w	1
db_colflags:	rs.w	1
db_collow:	rs.w	1
db_colhigh:	rs.w	1
db_end_dr:	rs.l	1
db_col_tab:	rs.l	1
db_waarcolors1:	rs.l	1

db_aa_present:	rs.b	1
db_nostop:		rs.b	1

* twee volledige view structuren later zullen deze buiten het programma 
* gemaakt moeten worden ???????
*
db_view1:		rs.b	v_SIZEOF
db_viewport1:		rs.b	vp_SIZEOF
db_rasinfo1:		rs.b	ri_SIZEOF
db_bitmap1:		rs.b	bm_SIZEOF

db_fileblok1:		rs.b	vb_SIZEOF	; data van de file
db_SIZEOF		rs.w	0
