*
* When the computer has a 68030 it can be faster to blit with the 68030
*
try_to_fast_cpu:

	move.l	db_inactive_fileblok(a3),a4
	lsr.l	#1,d7
	move.l	vb_breedte_x(a5),d3
	mulu	d7,d3
	add.l	d3,a2

	move.l	vb_breedte_x(a4),d3
	mulu	d7,d3
	add.l	d3,a1
	
	lsr.w	#1,d2
	move.l	db_shiftdest(a3),d4
	subq.w	#1,d2
rep_cpu_ver1:
	move.w	d2,d6
rep_cpu_hor1:
	move.l	-2(a1),d5
	lsr.l	d4,d5
	move.w	d5,(a2)+
	addq.l	#2,a1
	dbf	d6,rep_cpu_hor1
	add.l	d1,a1
	add.l	d0,a2
	subq.l	#1,d7
	bne.s	rep_cpu_ver1

;	move.l	db_graphbase(a3),a6
;	jsr	_LVODisownBlitter(a6)	; geef bliter weer vrij

	rts
