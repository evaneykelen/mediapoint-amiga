
*
* Set the two temporary viewports with start vars for 
* cop_down1 en cop_down2
*
* 
cop_init_new:
	move.b	#$1,db_easy_out_on(a3)
	cmp.l	#1,db_variation(a3)
	beq	no_eas10
	move.b	#$0,db_easy_out_on(a3)
no_eas10:
	move.w	#0,db_tmode(a3)
	move.l	#0,db_varwtime(a3)
	lea	teller(pc),a0
	move.l	db_varwtime(a3),off_teller(a0)
	move.l	db_varwtime(a3),off_tel_start(a0)
	clr.w	db_cop_inactive_test(a3)	; There is no top viewport 
	move.w	#TRUE,db_cop_active_test(a3)	; active is on
	
	move.l	db_active_viewblok(a3),a5
	move.l	db_inactive_viewblok(a3),a4
	bsr	check_cop_modes

	bsr	copy_view2
	bsr	switch_temp_views
	bsr	copy_view3

	bsr	init_user_coplist

; So now there are copies of the two viewports in itviewp an atviewp

	move.l	atviewp(pc),a1
	move.w	vp_DyOffset(a1),d0	; DEBUG

	move.l	itview(pc),a0		; Put the first in de view struct
	move.l	a1,v_ViewPort(a0)
	move.l	db_ucop_pointer(a3),vp_UCopIns(a1)

	move.l	atview(pc),a0		; Put the first in de view struct
	move.l	a1,v_ViewPort(a0)

	move.l	itviewp(pc),a1

	move.w	vp_DyOffset(a1),db_final_dy_inactive(a3)
	move.w	vp_DHeight(a1),db_final_height(a3)
	move.w	#0,vp_DHeight(a1)		; start with height 0

	move.w	db_inactive_min(a3),vp_DyOffset(a1)
	move.w	db_active_min(a3),d7
	rts


*
* Scroll down with two different viewports
* and a user copper list
*
cop_down1:
	
	bsr	cop_init_new

	move.l	db_wstore_moves2(a3),db_copstore(a3)

rep_cop_down1:					; Push the active down
	move.l	atviewp(pc),a1
	move.l	db_active_inc(a3),d0
	add.w	d0,vp_DyOffset(a1)
	move.w	vp_DyOffset(a1),d1

; Is there room on top for the inactive	

	move.w	d7,d2

	add.w	db_gapsize(a3),d2
	add.w	d0,d2
	cmp.w	d1,d2
	bgt	no_in_between1

	add.w	d0,d7

	move.w	#TRUE, db_cop_inactive_test(a3)

	move.l	itviewp(pc),a1
	move.w	vp_DyOffset(a1),d3
	move.l	db_inactive_viewblok(a3),a4

	move.l	db_inactive_inc(a3),d0
	add.w	d0,vp_DyOffset(a1)
	move.w	vp_DyOffset(a1),d0

	move.w	db_final_dy_inactive(a3),d4
	cmp.w	d4,d0
	ble	no_in_between1
	sub.w	d4,d0
	move.w	d4,vp_DyOffset(a1)
	
	add.w	d0,vp_DHeight(a1)
	move.w	vp_DHeight(a1),d0
	cmp.w	db_final_height(a3),d0
	ble	no_in_between1
	move.w	db_final_height(a3),vp_DHeight(a1)

no_in_between1:

	bsr	create_cop_view

	move.l	db_active_viewblok(a3),a4
	tst.b	vb_interlace(a4)
	bne	cd1_int3
	cmp.w	#267,d7
	bge	exit_cop_down1
cd1_int3:
	cmp.w	#267*2,d7
	bge	exit_cop_down1

	bra	rep_cop_down1

exit_cop_down1:

	bsr	show_cops

	bsr	showpicture

	bsr	clear_cop_lists
	
	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)

	bsr	free_view_ucop
	bsr	switch_temp_views
	bsr	free_view_ucop
	rts

*
* Scroll down with two different viewports
* and a user copper list
*
cop_down2:

	bsr	cop_init_new
	move.l	db_wstore_moves2(a3),db_copstore(a3)

	move.l	itviewp(pc),a0
	move.l	vp_RasInfo(a0),a2
	move.w	db_final_height(a3),ri_RyOffset(a2)
	move.l	db_active_inc(a3),d0
	sub.w	#1,ri_RyOffset(a2)

rep_cop_down2:					; Push the active down

	move.l	atviewp(pc),a1

	move.l	db_active_inc(a3),d0
	move.w	vp_DyOffset(a1),d1
	add.w	d0,vp_DyOffset(a1)

; Is there room on top for the inactive	

	move.w	d7,d2
;	add.w	d0,d2
	add.w	db_gapsize(a3),d2

	cmp.w	d1,d2
	bgt	no_inactive2

;	move.l	db_inactive_inc(a3),d0

	add.w	d0,d7
	move.w	#TRUE, db_cop_inactive_test(a3)

; Now check the bottom offset 

	cmp.w	db_bottom_offset(a3),d7
	blt	no_inactive2

	move.l	itviewp(pc),a1
	move.w	vp_DyOffset(a1),d3
	move.l	db_inactive_viewblok(a3),a4

	move.l	db_inactive_inc(a3),d0
	sub.w	d0,ri_RyOffset(a2)
	add.w	d0,vp_DHeight(a1)

	move.w	vp_DHeight(a1),d6
	move.w	ri_RyOffset(a2),d5
	bpl	no_pro12
	move.w	#0,ri_RyOffset(a2)
no_pro12:

	move.w	db_final_height(a3),d5
	cmp.w	db_final_height(a3),d6
	ble	no_inactive2

	move.w	#0,ri_RyOffset(a2)
	move.w	db_final_height(a3),vp_DHeight(a1)

	move.l	db_inactive_inc(a3),d0
	add.w	d0,vp_DyOffset(a1)
	move.w	vp_DyOffset(a1),d0
	move.w	db_final_dy_inactive(a3),d4
	cmp.w	d4,d0
	ble	no_inactive2
	sub.w	d4,d0
	move.w	d4,vp_DyOffset(a1)

no_inactive2:

	bsr	create_cop_view

	move.w	db_terminate(a3),d6
	cmp.w	db_terminate(a3),d7
	bge	exit_cop_down2

	bra	rep_cop_down2

exit_cop_down2:

	bsr	show_cops

	move.l	db_inactive_viewblok(a3),a5
	move.l	vb_rasinfow(a5),a2
	move.w	#0,ri_RyOffset(a2)

	bsr	showpicture
	bsr	clear_cop_lists

	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)

	bsr	free_view_ucop
	bsr	switch_temp_views
	bsr	free_view_ucop
bt:
	rts

exit_cop_down11:

	bsr	proces_file
	bsr	showpicture

	move.l	db_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)

	bsr	free_view_ucop
	bsr	switch_temp_views
	bsr	free_view_ucop
	rts
