ww:
	lea	test(pc),a0
	move.l	$dff004,(a0)
	move.l	(a0),d0
	cmp.l	$dff004,d0
	bne	.nom
	move.w	#$f00,$dff180
.nom:
	btst	#6,$bfe001
	bne	ww
	rts
test:	dc.l	0
