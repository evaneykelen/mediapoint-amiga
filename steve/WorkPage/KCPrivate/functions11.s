; fasterplot door Cees factor van 718 naar 242 3*
; 26 juni 1991
;
; fastplot and fastcricle
;
; uit kickstart juli 1988
;
execbase=4
openlib=-408
closelib=-414

openscreen=-198
closescreen=-66

phi	=	0
phiy	=	2
phixy	=	4
x1	=	6
y1	=	8
x	=	10
y	=	12
teller	=	14

startx	=	0
starty	=	2
verpixel=	4
versize	=	6
destx	=	8
desty	=	10
oldx	=	12
stopx	=	14
stopy	=	16


start:
	lea.l	varbase(pc),a5	; a5 wijst naar vars
	lea.l	mt(pc),a4	; a4 wijst naar mul-tabel
	bsr.L	init_mul_tabel
run:
	bsr.L	openstuff
	cmp.w	#0,d0
	bne.s	exit

	move.l	bpnt(pc),a1	; a1 wijst naar bitplane

;	bsr.L	circletest

	bsr.L	testdraw

;	bsr.s	testvergroot

;	bsr.s	testvul

ww1:	btst	#6,$bfe001
	bne.s	ww1

	bsr.L	closestuff
exit:
	rts

testvul:
	move.l	#160,d0
	move.l	#100,d1
	move.l	#90,d2
	bsr.L	fastcircle

	move.l	#195,d0
	move.l	#140,d1
	move.l	#30,d2
	bsr.L	fastcircle
;	move.l	#160,d0
;	move.l	#50,d1
;	move.l	#20,d2
;	bsr.L	fastcircle
;	move.l	#230,d0
;	move.l	#40,d1
;	move.l	#3,d2
;	bsr.L	fastcircle

;	move.l	#160,d0
;	move.l	#100,d1
;	move.l	#40,d2
;	bsr.L	fastcircle
;
;	move.l	#160,d0
;	move.l	#150,d1
;	move.l	#40,d2
;	bsr.L	fastcircle
;	move.w	#50,d0
;	move.w	#50,d1
;	move.w	#250,d2
;	move.w	#170,d3
;	bsr.L	draw

	lea.l	mt(pc),a4
	move.l	bpnt(pc),a1	; a1 wijst naar bitplane

ww2:	btst	#6,$bfe001
	bne.s	ww2

	bsr	start_clock
	move.l	#160,d0
	move.l	#100,d1
	bsr.s	vulnew
	bsr	stop_clock
	rts

vulnew:
	add.w	#1,d1

ga_links:			; boven je is het vrij
	sub.w	#1,d1		; herstel
rep_links:

	bsr	getplot
	bne	no_links_meer	
	bsr	fastplot

	add.w	#1,d1
	bsr	getplot
	bne	no_boven_links	; boven is al wit

	movem.l	d0/d1,-(a7)	; als zwart dan vul dit op
	bsr	vulnew
	movem.l	(a7)+,d0/d7

no_boven_links:
	add.w	#1,d0
	bra	ga_links

no_links_meer:
	rts


*
* Probeer een area fill te maken 
* oorspronkelijk door Cees op de QL gemaakt
*
*
* Vul het gebied vanaf d0,d1
*
vulet:
	move.w	#1,d6
	move.w	#1,d7
	bsr.s	vul1
	add.w	#1,d0

	move.w	#-1,d7
	move.w	#-1,d6
	bsr.s	vul1
	clr.l	d0
err:	rts

vul1:
	movem.l	d0/d1,-(a7)

vy1:	move.l	#0,d4
vx1:
	add.w	d7,d0		; kijk op de volgende regel
	bsr.L	getplot
	bne.s	geen

	bsr.L	fastplot	; zet punt	
	add.w	d6,d1

	bsr.L	getplot
	bne.s	nkleur		; is het zwart onthoud deze in d4

	move.w	d0,d4

	bra.s	wkleur

; is het zwart kijk of er op deze regel een zwarte was
; en begin te plotten

nkleur:
	tst.w	d4
	beq.s	wkleur

	movem.l	d0/d7,-(a7)
	move.w	d4,d0
	bsr.s	klein
	neg.w	d7
	bsr.s	vul1
	movem.l	(a7)+,d0/d7
	move.w	#0,d4
wkleur:
	sub.w	d6,d1
	bra.s	vx1
geen:
	tst.w	d4
	beq.s	vuit

	add.w	d6,d1
	move.w	d4,d0
	bsr.s	klein
	neg.w	d7
	bra.s	vy1
vuit:
	movem.l	(a7)+,d0/d1
	rts

klein:
	move.l	d0,-(a7)
rkl:	bsr.L	getplot
	bne.s	sklein
	bsr.L	fastplot
	add.w	d7,d0
	bra.s	rkl
sklein:
	move.l	(a7)+,d0
	rts

testvergroot:
	move.w	#300,d0
	move.w	#100,d1
	move.w	#320,d2
	move.w	#120,d3
	bsr.L	draw

	move.w	#298,d0
	move.w	#98,d1
	move.w	#10,d2
	move.w	#4,d3
rep_v:
	movem.w	d0-d3,-(a7)
	bsr.s	vergroot
	movem.w	(a7)+,d0-d3
wacht:
	btst	#6,$bfe001
	beq.s	nokke
	move.w	$dff00c,d7
	btst	#1,d7
	bne.s	rechts
	btst	#9,d7
	bne.s	links
	move.w	d7,d6
	lsr.w	#1,d6
	eor.w	d7,d6
	btst	#0,d6
	bne.s	achter
	btst	#8,d6
	bne.s	voor
	bra.s	wacht

rechts:
	btst	#7,$bfe001
	bne.s	no_but
	addq.w	#1,d3
	bra.s	rep_v
no_but:
	addq.w	#1,d0
	bra.s	rep_v
links:
	btst	#7,$bfe001
	bne.s	no_but2
	cmp.w	#1,d3
	beq.s	wacht
	subq.w	#1,d3
	bra.s	rep_v
no_but2:

	cmp.w	#1,d0
	beq.s	wacht
	subq.w	#1,d0
	bra.s	rep_v
achter:
	subq.w	#1,d1
	bra.s	rep_v
voor:	addq.w	#1,d1
	bra.s	rep_v
nokke:
	rts

* vanaf x,y d0,d1
* in d2 het aantal pixels
* in d3 de vergroting de vergroting komt op positie 0,0
*
vergroot:
	move.l	a5,-(a7)	
	lea.l	vergbase(pc),a5
	move.w	d0,startx(a5)
	move.w	d1,starty(a5)
	move.w	d0,stopx(a5)
	move.w	d1,stopy(a5)
	add.w	d2,stopx(a5)
	add.w	d2,stopy(a5)
	move.w	d2,verpixel(a5)
	move.w	d3,versize(a5)
	clr.w	oldx(a5)
	clr.w	destx(a5)
	clr.w	desty(a5)

vergr_y:
	move.w	oldx(a5),destx(a5)
	move.w	startx(a5),d0
vergr_x:
	bsr.L	getplot
	beq.s	nopixel
	bsr.s	PUTblok
	bra.s	welpixel
nopixel:
	bsr.s	CLSblok
welpixel:
	btst	#6,$bfe001
	beq.s	nn

	addq.w	#1,d0
	cmp.w	stopx(a5),d0
	bne.s	vergr_x

	move.w	versize(a5),d2
	add.w	d2,desty(a5)
	addq.w	#1,d1
	cmp.w	stopy(a5),d1
	bne.s	vergr_y
nn:
	move.l	(a7)+,a5
	rts

* zet een blok vanaf destx(a5),desty(a5) met versize pixels
*
PUTblok:
	movem.w	d0/d1,-(a7)

	move.w	destx(a5),d0
	move.w	desty(a5),d1

	move.w	versize(a5),d2
	subq.w	#1,d2
rep_P_y:
	move.w	versize(a5),d3
	subq.w	#1,d3
	move.w	destx(a5),d0
rep_P_x:
	bsr.L	fastplot
	addq.w	#1,d0
	dbf	d3,rep_P_x
	addq.w	#1,d1
	dbf	d2,rep_P_y
	move.w	d0,destx(a5)

	movem.w	(a7)+,d0/d1
	rts

* clear een blok vanaf destx(a5),desty(a5) met versize pixels
*
CLSblok:
	movem.w	d0/d1,-(a7)

	move.w	destx(a5),d0
	move.w	desty(a5),d1

	move.w	versize(a5),d2
	subq.w	#1,d2
rep_C_y:
	move.w	versize(a5),d3
	subq.w	#1,d3
	move.w	destx(a5),d0
rep_C_x:
	bsr.L	fastclear
	addq.w	#1,d0
	dbf	d3,rep_C_x
	addq.w	#1,d1
	dbf	d2,rep_C_y
	move.w	d0,destx(a5)

	movem.w	(a7)+,d0/d1
	rts


testdraw:
	bsr.L	start_clock
	move.l	#160,d0
	move.l	#100,d1
	move.l	#10,d2
	move.l	#200,d3
again:
	bsr.s	draw

	add.w	#1,d2
	cmp.w	#300,d2
	bne.s	again

again2:
	bsr.s	draw

	sub.w	#1,d3
	cmp.w	#10,d3
	bne.s	again2

again3:
	bsr.s	draw

	sub.w	#1,d2
	cmp.w	#10,d2
	bne.s	again3

again4:
	bsr.s	draw

	add.w	#1,d3
	cmp.w	#200,d3
	bne.s	again4

	bsr.L	stop_clock
	rts

*
* probeer een lijn te trekken
*
draw:
	movem.l	d0-d7/a2/a3,-(a7)
	move.l	#$10001,d4
;	move.l	#$10001,d5

	bsr.L	fastplot

	sub.w	d0,d2
	bcc.s	x2tox1
	neg.w	d2
	neg.w	d4
x2tox1:
	swap	d4
	sub.w	d1,d3
	bcc.s	y2toy1
	neg.w	d3
	neg.w	d4
y2toy1:
	move.w	d2,d7
	or.w	d3,d7
	beq.s	enddraw

	move.l	d4,d5

	cmp.w	d2,d3
	bcs.s	xbig

	and.l	#$0000ffff,d5

	exg	d2,d3
	bra.s	ybig

xbig:	and.l	#$ffff0000,d5
ybig:	moveq	#0,d7
	move.w	d2,d7
	subq.w	#1,d7
	move.w	d2,d6
	lsr.w	#1,d6
	move.w	d6,a2
	move.w	d3,a3

next:	move.w	a2,d3
	add.w	a3,d3
	bcs.s	diagonal
	cmp.w	d2,d3
	bcs.s	horivert

diagonal:
	sub.w	d2,d3
	move.w	d3,a2

	add.w	d4,d1
	swap	d4
	add.w	d4,d0		; PUTpoint
	swap	d4

	movem.w	d0/d1,-(a7)
	move.w	d0,d6
	lsr.w	#3,d6
	add.w	d1,d1
	add.w	(a4,d1.w),d6
	and.w	#$7,d0
	move.b	settabel2(pc,d0.w),d0
	bchg.b	d0,(a1,d6.l)
	movem.w	(a7)+,d0/d1

	dbf	d7,next
enddraw:
	movem.l	(a7)+,d0-d7/a2/a3
	rts

horivert:
	move.w	d3,a2
	add.w	d5,d1
	swap	d5
	add.w	d5,d0		; PUTpoint
	swap	d5

	movem.w	d0/d1,-(a7)
	move.w	d0,d6
	lsr.w	#3,d6
	add.w	d1,d1
	add.w	(a4,d1.w),d6
	and.w	#$7,d0
	move.b	settabel2(pc,d0.w),d0
	bchg.b	d0,(a1,d6.l)
	movem.w	(a7)+,d0/d1

	dbf	d7,next
	movem.l	(a7)+,d0-d7/a2/a3
	rts

settabel2:
	dc.b	7,6,5,4,3,2,1,0

plottest:
	bsr.L	start_clock

	moveq	#1,d0
do_y:
	moveq	#1,d1
do_x:
	movem.w	d0/d1,-(a7)

	bsr.L	fastplot

	movem.w	(a7)+,d0/d1
	add.w	#1,d1
	cmp.w	#240,d1
	bne.s	do_x
	add.w	#1,d0
	cmp.w	#319,d0
	bne.s	do_y	

	bsr.L	stop_clock

	rts

init_mul_tabel:
	lea.l	mt(pc),a0
	moveq	#0,d0
	move.l	#511,d7
rep_mt:	move.w	d0,(a0)+
	add.w	#40,d0
	dbf	d7,rep_mt
	rts
	
mt:	blk.w	512,0

*
* a4 wijst naar de mul-tabel
*
fastplot:
	movem.l	d0/d1/d7,-(a7)
	move.w	d0,d7	
	lsr.w	#3,d7
	add.w	d1,d1
	add.w	(a4,d1.w),d7
	and.w	#$7,d0
	move.b	settabel(pc,d0.w),d0
	bset.b	d0,(a1,d7.l)
	movem.l	(a7)+,d0/d1/d7
	rts

fastclear:
	movem.l	d0/d1,-(a7)
	move.w	d0,d7	
	lsr.w	#3,d7
	add.w	d1,d1
	add.w	(a4,d1.w),d7
	and.w	#$7,d0
	move.b	settabel(pc,d0.w),d0
	bclr.b	d0,(a1,d7.l)
	movem.l	(a7)+,d0/d1
	rts

getplot:
	movem.l	d0/d1/d7,-(a7)
	move.w	d0,d7	
	lsr.w	#3,d7
	add.w	d1,d1
	add.w	(a4,d1.w),d7
	and.w	#$7,d0
	move.b	settabel(pc,d0.w),d0
	btst.b	d0,(a1,d7.l)
	movem.l	(a7)+,d0/d1/d7
	rts

settabel:
	dc.b	7,6,5,4,3,2,1,0


circletest:
	bsr.s	start_clock

	move.l	#1,teller(a5)
h1:
	move.l	#1,d2

; hier kan je een eigen routine in bouwen

h0:
	move.l 	#160,d0
	move.l	#128,d1
	bsr.L	fastcircle
	add.l	teller(a5),d2
	cmp.l	#120,d2
	blt.s	h0
	move.b	$bfe001,d0
	and.b	#64,d0
	beq.s	uit

	move.l	bpnt(pc),a0
	move.l	#10240,d0

cl3:
	move.b	#0,(a0)+
	dbra	d0,cl3

	add.l	#1,teller(a5)
	cmp.l	#10,teller(a5)
	bgt.s	uit
	bra.s	h1
uit:
	bsr.s	stop_clock
	rts
start_clock:
	clr.l	d0
	move.b	$bfea01,d0
	lsl.l	#8,d0
	or.b	$bfe901,d0
	lsl.l	#8,d0
	or.b	$bfe801,d0
	move.l	d0,starttime
	rts

stop_clock:
	clr.l	d0
	move.b	$bfea01,d0
	lsl.l	#8,d0
	or.b	$bfe901,d0
	lsl.l	#8,d0
	or.b	$bfe801,d0
	sub.l	starttime,d0
	add.l	d0,d0
	move.l	d0,clockje
	rts

starttime:	dc.l	0
clockje:	dc.l	0


openstuff:
	move.l	execbase,a6
	lea	intname(pc),a1
	move.l	#0,d0
	jsr	openlib(a6)
	move.l	d0,intbase

	move.l	intbase(pc),a6
	lea	newscreen(pc),a0
	jsr	openscreen(a6)
	cmp.l	#0,d0
	bne.s	set2

err0:
	jsr	closestuff2
	move.l	#255,d0
	rts

set2:
	move.l	d0,screen
	move.l	screen(pc),a0
	move.l	192(a0),a0
	move.l	a0,bpnt
	move.l	#10240,d0

cl1:
	move.b	#0,(a0)+
	dbra	d0,cl1


	move.w	#0,d0

	rts

closestuff:
	move.l	intbase(pc),a6
	move.l	screen(pc),a0
	jsr	closescreen(a6)

closestuff2:
	move.l	execbase,a6
	move.l	intbase(pc),a1
	jsr	closelib(a6)
	rts

*
* Teken een cirkel met diameter d2 met midden d0,d1
*


fastcircle:
	movem.l	d0/d1/d2/d3,-(sp)

	move.w	d0,x(a5)
	move.w	d1,y(a5)
	move.w	#0,phi(a5)
	move.w	d2,x1(a5)
	move.w	#0,y1(a5)
circl1:
	move.w	phi(a5),d3
	add.w	y1(a5),d3
	add.w	y1(a5),d3
	addq.w	#1,d3
	move.w	d3,phiy(a5)
	sub.w	x1(a5),d3
	sub.w	x1(a5),d3
	addq.w	#1,d3
	move.w	d3,phixy(a5)

	move.w	x(a5),d0
	add.w	x1(a5),d0
	move.w	y(a5),d1
	add.w	y1(a5),d1
	bsr.L	fastplot


	move.w	x(a5),d0
	sub.w	x1(a5),d0
	move.w	y(a5),d1
	add.w	y1(a5),d1
	bsr.L	fastplot

	move.w	x(a5),d0
	add.w	x1(a5),d0
	move.w	y(a5),d1
	sub.w	y1(a5),d1
	bsr.L	fastplot

	move.w	x(a5),d0
	sub.w	x1(a5),d0
	move.w	y(a5),d1
	sub.w	y1(a5),d1
	bsr.L	fastplot

	move.w	x(a5),d0
	add.w	y1(a5),d0
	move.w	y(a5),d1
	add.w	x1(a5),d1
	bsr.L	fastplot

	move.w	x(a5),d0
	sub.w	y1(a5),d0
	move.w	y(a5),d1
	add.w	x1(a5),d1
	bsr.L	fastplot

	move.w	x(a5),d0
	add.w	y1(a5),d0
	move.w	y(a5),d1
	sub.w	x1(a5),d1
	bsr.L	fastplot

	move.w	x(a5),d0
	sub.w	y1(a5),d0
	move.w	y(a5),d1
	sub.w	x1(a5),d1
	bsr.L	fastplot

	move.w	phiy(a5),d0
	move.w	d0,phi(a5)
	addq.w	#1,y1(a5)
	move.w	phixy(a5),d0
	bsr.s	toabs
	move.w	d0,d1
	move.w	phiy(a5),d0
	bsr.s	toabs
	cmp.w	d0,d1
	bge.s	circl3
	move.w	phixy(a5),d0
	move.w	d0,phi(a5)
	subq.w	#1,x1(a5)

circl3:
	move.w	y1(a5),d0
	cmp.w	x1(a5),d0
	ble.L	circl1
	movem.l	(sp)+,d0/d1/d2/d3
	rts

toabs:
	cmp.w	#0,d0
	bmi.s	toa1
	rts

toa1:
	neg.w	d0
	rts

; data sectie

intname:	dc.b	"intuition.library",0,0
	even
intbase:	blk.l	1

newscreen:
	dc.w	0
	dc.w	0
	dc.w	320
	dc.w	256
	dc.w	1
	dc.b	0
	dc.b	1
	dc.w	0
	dc.w	15
	dc.l	0
	dc.l	0
	dc.l	0
	dc.l	0

screen:	blk.l	1
bpnt:	blk.l	1

varbase:	blk.w	12,0
vergbase:	blk.w	20,0

