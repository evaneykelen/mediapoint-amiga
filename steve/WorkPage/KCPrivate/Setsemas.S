	XDEF	_set_sema
_set_sema:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a3

	move.l	db_graphbase(a3),a6
	move.l	gb_ActiViewCprSemaphore(a6),a0
	move.l	$4,a6
	jsr	_LVOObtainSemaphore(a6)
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

	XDEF	_release_sema

_release_sema:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a3

	move.l	db_graphbase(a3),a6
	move.l	gb_ActiViewCprSemaphore(a6),a0
	move.l	$4,a6
	jsr	_LVOReleaseSemaphore(a6)
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts
