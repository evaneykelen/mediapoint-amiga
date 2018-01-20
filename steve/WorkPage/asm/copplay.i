*
* Try to create a asynchrone coplist player with the aid of a 50hz interrupt
* The interrupt plays the data in the info struct until a terminator is found
* When a list is played a signal is used to inform the creator that a new
* list can be made. The pointer for that list is stored at a place 
* in the info struct. The list that is staying there can be freed by the creator
* The pointer changes have to be made within a semaphore to avoid hangups etc.
*
* So the structure should contain
*
*
	RSRESET
cpp_block:			rs.w	0
cpp_size:			rs.w	1
cpp_init:			rs.w	1
cpp_gfxbase:		rs.l	1
cpp_storep:			rs.l	1
cpp_playp:			rs.l	1
cpp_playedp:		rs.l	1
cpp_view:			rs.l	1
cpp_vextra:		rs.l	1
cpp_loadview:	rs.l		1
cpp_task:			rs.l	1
cpp_signum:			rs.l	1
cpp_sig:				rs.l	1
cpp_lists			rs.b	360			; 30 lists
cpp_endlist:		rs.w	1
cpp_SIZEOF:			rs.w	0

