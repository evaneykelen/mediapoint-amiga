read_whole_file:
	move.l	db_mlmmubase(a3),a6
	move.l	vb_filenaam(a5),a1		; naam van de file mnmmulib
	jsr	_LVOMLMMU_FindMemBlk(a6)
	move.l	d0,vb_packed(a5)
	bne	file_loaded2
	
	moveq	#3,d7
	move.l	vb_filenaam(a5),d1
	move.l	#mode_old,d2
	move.l	db_dosbase(a3),a6
	jsr	_LVOOpen(a6)
	move.l	d0,vb_filehandle(a5)
	beq.w	exit
	move.l	d0,d1
	lea.l	vb_breedte_x(a5),a2
	move.l	a2,d2
	moveq	#8,d3
	move.l	db_dosbase(a3),a6
	jsr	_LVORead(a6)
	lea.l	vb_breedte_x(a5),a1
	moveq	#1,d7
	cmp.l	#'FORM',(a1)+
	bne.w	exit
	move.l	(a1),d0
	add.l	#8,d0
	move.l	d0,vb_packed_size(a5)

	moveq	#2,d7
	move.l	#$10000,d1
	or.l	#MEMF_STAY,d1
	move.l	db_mlmmubase(a3),a6
	move.l	vb_filenaam(a5),a1		; naam van de file mnmmulib
	jsr	_LVOMLMMU_AllocMem(a6)

	move.l	d0,vb_packed(a5)
	beq.w	exit
	move.l	d0,a0

check_load:

* test of de file al in het geheugen zit

	move.l	vb_packed(a5),a1
	jsr	_LVOMLMMU_GetMemStat(a6)	; haal status van memblock op

	and.l	#MTF_INIT,d0
	bne	file_loaded

	move.l	vb_packed(a5),a1
	jsr	_LVOMLMMU_OwnMemBlk(a6)
	tst.l	d0
	beq	check_load

	move.l	vb_filehandle(a5),d1
	moveq	#0,d2				; seek beginning
	moveq	#-1,d3
	move.l	db_dosbase(a3),a6
	jsr	_LVOSeek(a6)		

	move.l	vb_packed_size(a5),d3
	move.l	vb_packed(a5),d2
	move.l	vb_filehandle(a5),d1
	move.l	db_dosbase(a3),a6
	jsr	_LVORead(a6)		

	move.l	db_mlmmubase(a3),a6
	move.l	vb_packed(a5),a1
	move.l	#MTF_INIT,d0
	or.l	#MTF_SETCLR,d0
	jsr	_LVOMLMMU_SetMemStat(a6)	; file nu wel geladen

	move.l	vb_packed(a5),a1
	jsr	_LVOMLMMU_DisOwnMemBlk(a6)

* hele file nu in het geheugen 

file_loaded:
	bsr.b	close_file
file_loaded2:
	moveq	#0,d0
	rts

close_file:
	move.l	db_dosbase(a3),a6
	move.l	vb_filehandle(a5),d1
	jsr	_LVOClose(a6)
	clr.l	vb_filehandle(a5)
	rts
