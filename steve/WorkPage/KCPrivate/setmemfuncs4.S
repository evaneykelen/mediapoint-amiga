	incdir	"wp:inclibs/"
	include	"exec_lib.i"
	include	"graphics_libv39.i"
	incdir	"include:"
	include	"dos/dos.i"
	include	"graphics/view.i"
	include	"exec/tasks.i"
	include	"exec/memory.i"

	IFNE 0
MEMSIZE = 1000000

start:
	move.l	$4.w,a6
	move.l	#MEMSIZE,d0
	move.l	#MEMF_PUBLIC,d1
	jsr	_LVOAllocMem(a6)
	move.l	d0,mem1
	beq	.nomem

	move.l	d0,a1
	add.l	#MEMSIZE-8,a1
	move.l	#-2,(a1)+
	move.l	#-2,(a1)

	move.l	$4.w,a6
	move.l	#MEMSIZE,d0
	move.l	#MEMF_PUBLIC,d1
	jsr	_LVOAllocMem(a6)
	move.l	d0,mem2
	beq	.nomem
	move.l	d0,a1
	add.l	#MEMSIZE-8,a1
	move.l	#-2,(a1)+
	move.l	#-2,(a1)

	move.l	mem1(pc),a0
	move.l	mem2(pc),a1

	bsr	_SetFunctions


	bsr	_ResetFunctions

	move.l	mem2(pc),a1
	move.l	#MEMSIZE,d0
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)

	move.l	mem1(pc),a1
	move.l	#MEMSIZE,d0
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)

.nomem:
	moveq	#0,d0
	rts
	ENDC
	
	XDEF	_SetFunctions
*
* In a0 the alloc and free buffers
*
_SetFunctions:
	movem.l	d0-d1/a0-a6,-(a7)
	move.l	a0,allocdata
	move.l	a1,freedata
	move.l	$4.w,a6				; patch allocmem
	jsr	_LVOForbid(a6)
	move.l	a6,a1
	move.w	#_LVOAllocMem,a0
	lea	ownallocmem(pc),a2
	move.l	a2,d0
	jsr	_LVOSetFunction(a6)
	move.l	d0,oldallocmem

	move.l	$4.w,a6				; path freemem
	move.l	a6,a1
	move.w	#_LVOFreeMem,a0
	lea	ownfreemem(pc),a2
	move.l	a2,d0
	jsr	_LVOSetFunction(a6)
	move.l	d0,oldfreemem
	jsr	_LVOPermit(a6)
	movem.l	(a7)+,d0-d1/a0-a6
	moveq	#0,d0
	rts

	XDEF	_ResetFunctions

_ResetFunctions:
	movem.l	d1/a0-a6,-(a7)
	move.l	$4.w,a6
	jsr	_LVOForbid(a6)

	move.l	oldallocmem(pc),d0
	move.l	a6,a1
	move.w	#_LVOAllocMem,a0
	jsr	_LVOSetFunction(a6)	

	move.l	oldfreemem(pc),d0
	move.l	$4.w,a6
	move.l	a6,a1
	move.w	#_LVOFreeMem,a0
	jsr	_LVOSetFunction(a6)	

	jsr	_LVOPermit(a6)
	lea	alloccount(pc),a0
	move.l	a0,d0
	movem.l	(a7)+,d1/a0-a6
	rts

ownallocmem:
	movem.l	d1-d7/a0-a6,-(a7)
	move.l	d0,d7
	move.l	d1,d6
	bsr	check_task
	beq	.nocheck
	addq.l	#1,alloccount
	add.l	d7,totalmem

; It's a task worth watching

	move.l	allocdata(pc),a3		; find free entry
	cmp.l	#-2,(a3)
	beq	.nocheck
	addq.l	#8,allocdata
	move.l	d7,4(a3)
	move.l	d7,d0
	move.l	d6,d1
	move.l	oldallocmem(pc),a4
	jsr	(a4)
	move.l	d0,(a3)
	movem.l	(a7)+,d1-d7/a0-a6
	rts

.nocheck:
	move.l	d7,d0
	move.l	d6,d1
	move.l	oldallocmem(pc),a4
	jsr	(a4)
	movem.l	(a7)+,d1-d7/a0-a6
	rts

ownfreemem:
	movem.l	d1-d7/a0-a6,-(a7)
	move.l	d0,d7
	move.l	a1,d6
	bsr	check_task
	beq	.nocheck
	addq.l	#1,freecount
	sub.l	d7,totalmem
; It's a task worth watching

	move.l	freedata(pc),a3		; find matching entry
	cmp.l	#-2,(a3)
	beq	.nocheck
	move.l	d6,(a3)
	move.l	d7,4(a3)
	addq.l	#8,freedata
	move.l	d7,d0
	move.l	d6,a1
	move.l	oldfreemem(pc),a4
	jsr	(a4)
	movem.l	(a7)+,d1-d7/a0-a6
	rts

.nocheck:
	move.l	d7,d0
	move.l	d6,a1
	move.l	oldfreemem(pc),a4
	jsr	(a4)
	movem.l	(a7)+,d1-d7/a0-a6
	rts
*
* Am I the task worth watching ?
*
check_task:
	move.w	#0,a1
	jsr	_LVOFindTask(a6)
	tst.l	d0
	beq	.no
	move.l	d0,a0
	move.l	LN_NAME(a0),a0
	move.l	(a0),name
	cmp.l	#'Medi',(a0)
	beq	.do
	cmp.l	#'MP c',(a0)
	beq	.do
.no:
	moveq	#0,d0	
	rts

.do:
	moveq	#1,d0
	rts

name:		dc.l	0
oldallocmem:	dc.l	dummy
oldfreemem:	dc.l	dummy
allocdata:	dc.l	0
freedata:	dc.l	0
mem1:		dc.l	0
mem2:		dc.l	0
alloccount:	dc.l	0
freecount:	dc.l	0
totalmem:	dc.l	0
dummy:	moveq	#0,d0
	rts

