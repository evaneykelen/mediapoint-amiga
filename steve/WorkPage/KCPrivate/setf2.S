	incdir wp:inclibs/
	include exec_lib.i
	incdir include:
	include	dos/dos.i
	include exec/nodes.i
test:
	moveq	#0,d1
	move.l	#123456,d0
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
	tst.l	d0
	beq	no_mem
	move.l	d0,a1
	move.l	#123456,d0
	jsr	_LVOFreeMem(a6)	

no_mem:
	rts
start:
	bsr	install_watch
	move.l	#SIGBREAKF_CTRL_F,d0
	move.l	$4.w,a6
	jsr	_LVOWait(a6)
	bsr	remove_watch
	moveq	#0,d0
	rts

	XREF	KPutStr

install_watch:
	move.l	$4.w,a6
	jsr	_LVOForbid(A6)
	move.l	a6,a1
	move.l	#-198,a0
	move.l	#func,d0
	jsr	_LVOSetFunction(a6)
	move.l	d0,jf+2
	move.l	$4.w,a6
	jsr	_LVOPermit(a6)
	rts

remove_watch:
	move.l	$4.w,a6
	jsr	_LVOForbid(A6)
	move.l	a6,a1
	move.l	#-198,a0
	move.l	(jf+2)(pc),d0
	jsr	_LVOSetFunction(a6)
	move.l	d0,func+2
	move.l	$4.w,a6
	jsr	_LVOPermit(a6)
	rts

dbstr1:	dc.b	"a",10,0
	even

func:
	movem.l	a0-a6/d0-d7,-(a7)
	cmp.l	#100000,d0
	ble	no_print

	move.l	$4.w,a6
	jsr	_LVOForbid(A6)

	move.l	#0,a1
	jsr	_LVOFindTask(a6)
	
	move.w	#$f00,$dff180		
	move.l	LN_NAME(a0),a0
	jsr	KPutStr

	move.l	$4.w,a6
	jsr	_LVOPermit(a6)

no_print:
	movem.l	(a7)+,a0-a6/d0-d7

jf:	jmp	0

