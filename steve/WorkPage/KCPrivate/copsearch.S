
	IFNE	0
*
* In a view a1
*
find_gen_and_switch:
	move.l		a1,a2
	move.l		v_LOFCprList(a2),a0
	move.l		v_SHFCprList(a2),a1

	cmp.l	#0,a1
	beq	exit_cc

	move.l		crl_Next(a1),a2
	move.w		crl_MaxCount(a0),d2		; aantal elementen
;	add.w		d2,d2
	move.l		crl_start(a0),a0		; eigenlijke copperlist
	move.l		crl_start(a1),a1

	subq.l		#1,d2
	move.w	d2,d7
.rep_copcop2:
	move.w	(a0)+,d0
	cmp.w	#$ffff,d0
	beq	hh
	cmp.w	#$100,d0
	bne	.no_diver1
	and.w	#$fefd,(a0)
.no_diver1:
	cmp.w	#$104,d0
	bne	.no_diver2
	and.w	#$81ff,(a0)
	or.w	#$0400,(a0)
.no_diver2:
	cmp.w	#$106,d0
	bne	.no_diver3
;	and.w	#$81ff,(a0)
	nop
	nop

.no_diver3:
	addq.l	#2,a0
	dbf	d2,.rep_copcop2
hh:

.rep_copcop2:
	move.w	(a1)+,d0
	cmp.w	#$ffff,d0
	beq	exit_copcopy2
	cmp.w	#$100,d0
	bne	.no_diver1
	and.w	#$fefd,(a1)
.no_diver1:
	cmp.w	#$104,d0
	bne	.no_diver2
	and.w	#$81ff,(a1)
	or.w	#$0400,(a1)
.no_diver2:
	addq.l	#2,a1
	dbf	d7,.rep_copcop2
exit_copcopy2:
exit_cc:
	rts
	ENDC
