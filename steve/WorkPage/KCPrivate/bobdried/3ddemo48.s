scrollspeed=0
s_y=-300
breedte=29

dep=150
*delta_x=15
*delta_y=25

sprite_br=2
sprite_ho=$e

spee=1
spe=1
ixo=0
iyo=0
izo=1200
acur=6
xnul=160
ynul=100

* sprite routines door Cees Lieshout
* gemaakt in 1988 1989 en verder
*
max_x=352
num_planes=5

mod_sprite=40		; 320/8 - sprite_bre*2
sprite_size=898		; hoogte*64 + breedte

shapes_size=10000

mem_size=122000
plane_size=10000
extra_mem=40000
breedte_rast=max_x
breedte_b=breedte_rast/8
ciaapra =$bfe001
scr_size=20000
openlibrary=-408
execbase = 4
freemem=-210
allocmem=-198

start:
	move.l	execbase,a6
	lea	grname(pc),a1
	clr.l	d0
	jsr	openlibrary(a6)
	lea	gfxbase(pc),a4
	move.l	d0,(a4)
	tst.l	d0
	beq.S	nodemo
	bsr.S	demo

nodemo:

	moveq	#0,d0
	rts

demo:
	move.l	#mem_size,d0
	move.l	#$10002,d1
	move.l	execbase,a6
	jsr	allocmem(a6)
	move.l	d0,mem_gebied
	beq.L	mem_error
	move.l	d0,mem_cop

	add.l	#1000,d0
	move.l	d0,mem_cop2
	add.l	#1000,d0
	add.l	#5000,d0
	move.l	d0,mem_screen
	add.l	#num_planes*plane_size,d0
	move.l	d0,mem_screen2

	move.l	#extra_mem,d0
	move.l	#$10000,d1
	move.l	execbase,a6
	jsr	allocmem(a6)
	move.l	d0,mem_oppoints

	move.l	d0,oppoints
	beq.L	mem_error2

	move.l	d0,opwaarpoints
*	add.l	#extra_mem/2,opwaarpoints

	add.l	#2000,opwaarpoints
	move.l	d0,waar_patroon
	add.l	#4000,waar_patroon

	move.l	#shapes_size,d0
	move.l	#$10002,d1
	move.l	execbase,a6
	jsr	allocmem(a6)
	move.l	d0,mem_shapes
	beq.L	mem_error1


	move.w	#$4000,$dff09a

	move.w	#$01e0,$dff096


	move.l	#$dff180,a0
	move.l	#31,d7
repclol:
	move.w	#0,(a0)+
	dbf	d7,repclol

	move.l	mem_cop,a0
	move.l	mem_screen,d2
	bsr.L	maak_cop

	move.l	mem_cop2,a0
	move.l	mem_screen2,d2
	bsr.L	maak_cop

	move.l	mem_cop,$dff080

	movem.l	d0-d7/a0-a6,-(a7)
	bsr.L	maak_masks
	movem.l	(a7)+,d0-d7/a0-a6


	move.w	#$83c0,$dff096

door:
	bsr.L	scroll
	btst	#6,$bfe001
	beq.S	stoppen

	bsr.L	patroontjes
	btst	#10,$dff016
	beq.S	stoppen

	bra.S	door

stoppen:
	move.w	#$c000,$dff09a
	lea	gfxbase(pc),a4
	move.l	(a4),a6
	clr.w	$dff088
	move.l	38(a6),$dff080
	move.w	#$83e0,$dff096

	move.l	execbase,a6
	move.l	mem_shapes,a1
	move.l	#shapes_size,d0
	jsr	freemem(a6)

mem_error1:	
	move.l	execbase,a6
	move.l	mem_oppoints,a1
	move.l	#extra_mem,d0
	jsr	freemem(a6)

mem_error2:
	move.l	execbase,a6
	move.l	mem_gebied,a1
	move.l	#mem_size,d0
	jsr	freemem(a6)
mem_error:
	clr.l	d0
	rts



setspeed:
	move.l	#anglexinc,a1
	move.l	#2,d1
rep_cin:
	moveq	#0,d0
	move.w	$dff006,d0
*	lsr.w	#8,d0
	andi.w	#$f,d0
	move.l	d0,(a1)+
	dbf	d1,rep_cin
	rts

vlakje:	
	move.w	#280,zo
	clr.w	transx
	clr.w	transy
	clr.w	transz
	move.w	#40,welke_z

	move.l	#-60,start_x
	move.l	#-40,start_y
	move.l	#0,aantal_x
	move.l	#0,aantal_y
	clr.l	aantalpoints
	move.l	oppoints,a0
	move.l	#5,d7
	
vrep:	move.l	d7,-(a7)
	bsr.L	init_points
	move.l	(a7)+,d7
	sub.w	#10,welke_z
	add.l	#1,aantal_y
	add.l	#1,aantal_x
	dbf	d7,vrep


vww:	bsr.L	fade_in
	bsr.L	all_sprites
	btst	#6,$bfe001
	bne.S	vww
	rts

**********************************
scroll:
	move.l	mem_oppoints,oppoints

	clr.l	aantalpoints
	clr.l	scroll_teller
	clr.l	anglexinc+4
	move.l	#5,anglexinc

	clr.l	anglexinc+8

	move.l	#170,anglex
	move.l	#0,angley
	move.l	#0,angley

	clr.w	transx
	clr.w	transy
	move.w	#400,transz
	move.w	#-100,transx

	move.l	#90,anglez
	move.b	#1,modulbit
	move.w	#1280,zo
	bsr.L	maakpatroon

	move.w	#10,modulus

	move.l	waar_patroon,welk_patroon
	move.w	#0,welke_z

	move.l	#-60,start_x
	move.l	#s_y,start_y
	move.l	#5,aantal_x
	move.l	#breedte,aantal_y
	move.l	oppoints,a0

	bsr.L	sinit_points
	bsr.L	all_sprites

	move.l	#19,d4
sww99:
	movem.l	d0-d7/a0-a6,-(a7)
	sub.w	#20,transz
	add.w	#5,transx
	sub.w	#50,zo

	bsr.L	fade_in

	bsr.L	all_sprites
	movem.l	(a7)+,d0-d7/a0-a6

	dbf	d4,sww99

*	clr.l	anglexinc
*	clr.l	anglexinc+4
*	clr.l	anglexinc+8

	clr.w	transx
	clr.w	transy
	clr.w	transz

*	move.l	#5,anglexinc+4

	move.l	#5,change_teller
*	move.w	#270,zo

*	move.w	#80,modulus

	move.l	#0,sinusangle

	move.l	#5,tempo_angle
	clr.l	tempo_x
sww5:
	lea	$dff000,a0

	move.l	tempo_angle,d0
	add.l	d0,sinusangle
	cmpi.l	#359,sinusangle
	bls.S	snopo_sin
	move.l	#0,sinusangle
snopo_sin:

	clr.l	aantalpoints

	move.l	#-60,start_x
	move.l	#s_y,start_y
	move.l	#5,aantal_x
	move.l	#breedte,aantal_y
	move.l	oppoints,a0

	bsr.L	sinit_points
	bsr.L	all_sprites


	add.l	#6,welk_patroon
	move.l	eind_patroon,d0
	cmpi.l	welk_patroon,d0
	bhi.S	snopo12
	move.l	waar_patroon,welk_patroon
	bra.L	sstopmaar
snopo12:

	cmpi.b	#1,modulbit
*	bne	snopo_modu2

	move.w	mod_inc,d0
	add.w	d0,modulus
	cmpi.w	#0,modulus
	bgt.S	snopo_modu1
	neg.w	mod_inc

snopo_modu1:
	cmpi.w	#30,modulus
	bls.S	snopo_modu2
	neg.w	mod_inc

snopo_modu2:

	sub.l	#1,change_teller
	bne.S	sno_chan

	move.l	#10,change_teller
	add.l	#1,tempo_angle
	cmpi.l	#20,tempo_angle
	bls.S	sno_chan

	move.l	#anglexinc,a1
	move.l	#2,d1
srep_cin:
	moveq	#0,d0
	move.w	$dff006,d0
*	lsr.w	#8,d0
	andi.w	#$7,d0
	move.l	d0,(a1)+
	dbf	d1,srep_cin

*	neg.b	modulbit
*	move.w	#0,modulus

	move.l	#10,tempo_angle
sno_chan:
	btst	#10,$dff016
	bne.S	sno_patchange

	move.l	#90,anglez

	clr.l	anglexinc
	clr.l	anglexinc+4
	clr.l	anglexinc+8

	move.l	#10,anglex
*	clr.l	anglex
	clr.l	angley
*	clr.l	anglez
	move.b	#-1,modulbit


sno_patchange:
	btst	#10,$dff016
	bne.L	sww5

sstopmaar:
	move.l	#6,anglexinc
	move.l	#8,anglexinc+4
	move.l	#13,anglexinc+8
	move.l	#10,d7
sstop3:	move.l	d7,-(a7)
	add.w	#10,transz
	add.w	#10,transx
	bsr.L	all_sprites
	move.l	(a7)+,d7
	dbf	d7,sstop3

	move.l	#19,d7
sstopmaar2:
	add.w	#50,zo
	move.l	d7,-(a7)
	add.w	#10,transz
	add.w	#20,transx

	bsr.L	all_sprites
	move.l	d7,d3
	
	bsr.L	fade_out

	move.l	(a7)+,d7
	dbf	d7,sstopmaar2
	rts

*********************************************************
patroontjes:
	clr.l	tijdteller
	move.l	mem_oppoints,oppoints
	move.w	#1250,zo
	clr.w	transx
	clr.w	transy
	clr.w	transz

	move.w	#0,modulus
	move.l	patroonlijst,welk_patroon
	move.l	#patroonlijst+4,currentpatroon
	move.w	#0,welke_z

	move.l	#-60,start_x
	move.l	#-100,start_y
	move.l	#7,aantal_x
	move.l	#7,aantal_y
	clr.l	aantalpoints
	move.l	oppoints,a0

	bsr.L	pinit_points
	bsr.L	all_sprites

	move.l	#19,d4
pww99:
	movem.l	d0-d7/a0-a6,-(a7)
	sub.w	#50,zo

	bsr.L	fade_in
pnof:
	bsr.L	all_sprites
	movem.l	(a7)+,d0-d7/a0-a6


	btst	#6,ciaapra
	beq.L	pstopmaar

	dbf	d4,pww99

*	move.w	#270,zo
	move.l	#21,d7
pww100:
	move.l	d7,-(a7)

	clr.w	welke_sprite
	clr.l	aantalpoints

*	add.w	#1,modulus

	move.l	#-60,start_x
	move.l	#-100,start_y
	move.l	#7,aantal_x
	move.l	#7,aantal_y
	move.l	oppoints,a0

	bsr.L	pinit_points

	bsr.L	all_sprites

	move.l	(a7)+,d7
	btst	#6,ciaapra
	beq.L	pstopmaar

	dbf	d7,pww100

*	move.w	#80,modulus

	move.l	#0,sinusangle

	clr.w	modulus
	move.l	#20,tempo_angle
	clr.l	tempo_x
pww5:
	lea	$dff000,a0
	move.l	tempo_angle,d0
	add.l	d0,sinusangle
	cmpi.l	#359,sinusangle
	bls.S	pnopo_sin
	sub.l	#360,sinusangle
*	move.l	#0,sinusangle
pnopo_sin:

	clr.l	aantalpoints

	move.l	#-60,start_x
	move.l	#-100,start_y
	move.l	#7,aantal_x
	move.l	#7,aantal_y
	move.l	oppoints,a0

	bsr.L	pinit_points
	bsr.L	all_sprites


	move.w	welke_z,d0
	add.w	wel_inc,d0

	cmp.w	#-80,d0
	bgt.S	pnopo_zw
	neg.w	wel_inc
pnopo_zw:	
	cmpi.w	#80,d0
	blt.S	pnopo_zw2
	neg.w	wel_inc
pnopo_zw2:
	move.w	d0,welke_z

	cmpi.b	#1,modulbit
	bne.S	pnopo_modu2

	move.w	mod_inc,d0
	add.w	d0,modulus
	tst.w	modulus
	bpl.S	pnopo_modu1
	move.w	#1,mod_inc
*	neg.w	mod_inc
pnopo_modu1:
	cmpi.w	#30,modulus
	blt.S	pnopo_modu2
	neg.w	mod_inc
pnopo_modu2:

	sub.l	#1,change_teller
	bne.S	pno_chan

	move.l	#20,change_teller
	add.l	#1,tempo_angle
	cmpi.l	#23,tempo_angle
	bne.S	pnopo5

	neg.b	modulbit
pnopo5:
	cmpi.l	#25,tempo_angle
	bls.S	pno_chan

	bsr.L	setspeed
	neg.b	modulbit

	move.w	#0,modulus
	move.l	#20,tempo_angle

	bra.S	pver

pno_chan:
	btst	#10,$dff016
	bne.S	pno_patchange
pver:
	bsr.L	fade_o
	move.l	currentpatroon,a4
	move.l	(a4)+,d0
	bne.S	pnopo12

*	add.l	#1,tijdteller
*	cmpi.l	#2,tijdteller
	bra.L	pstopmaar3

	move.l	#patroonlijst,a4
	move.l	(a4)+,d0	
pnopo12:
	move.l	a4,currentpatroon
	move.l	d0,welk_patroon
	clr.l	aantalpoints

	move.l	#-60,start_x
	move.l	#-100,start_y
	move.l	#7,aantal_x
	move.l	#7,aantal_y
	move.l	oppoints,a0
	clr.w	welke_z
	bsr.S	pinit_points

	bsr.L	fade_i
pno_patchange:
	btst	#6,ciaapra
	bne.L	pww5

pstopmaar:
	move.l	#6,anglexinc
	move.l	#8,anglexinc+4
	move.l	#13,anglexinc+8
	move.l	#10,d7
pstop3:	move.l	d7,-(a7)
	add.w	#20,transz
	bsr.L	all_sprites
	move.l	(a7)+,d7
	dbf	d7,pstop3

	move.l	#19,d7
pstopmaar2:
	add.w	#50,zo
	move.l	d7,-(a7)
	add.w	#10,transz
	bsr.L	all_sprites
	move.l	d7,d3
	andi.l	#$3,d3
	cmpi.b	#2,d3
*	bne	noo

	bsr.L	fade_out
pnoo:

	move.l	(a7)+,d7
	dbf	d7,pstopmaar2
pstopmaar3:

	rts

pinit_points:
	move.l	sinusangle,d4
	move.l	aantal_y,d7
	move.l	start_y,d1
	move.l	welk_patroon,a4

prepy:	move.l	aantal_x,d6
	move.l	start_x,d0
	move.l	d4,d3
prepx:	

	move.w	d0,(a0)
	add.l	#30,d3
	cmpi.l	#359,d3
	bls.S	pnopo_sin2
	sub.l	#360,d3
pnopo_sin2:
	tst.b	(a4)
	beq.S	pno_patr
	lea.l	2(a0),a0
	move.l	d3,d2
	lsl.l	#1,d2
	lea.l	sinustabel,a1
	move.w	0(a1,d2.l),d2
	mulu	modulus,d2
	asr.w	#acur,d2
	move.w	d1,(a0)+
	cmpi.b	#10,(a4)
	bls.S	pno_zadd
	add.w	welke_z,d2
pno_zadd:
	move.w	d2,(a0)+

*	move.w	welke_z,(a0)+

	moveq	#0,d2
	move.b	(a4),d2
	cmpi.b	#10,d2
	bls.S	pno_po10
	sub.b	#10,d2
pno_po10:
	sub.b	#1,d2
	move.w	d2,(a0)+
	add.l	#1,aantalpoints
pno_patr:
	add.w	mdelta_x,d0
	lea.l	1(a4),a4
	dbf	d6,prepx
	add.w	mdelta_y,d1
	add.l	#20,d4
	dbf	d7,prepy
	move.w	#1000,(a0)+
	move.w	#1000,(a0)+
	move.w	#1000,(a0)+
	move.w	#1000,(a0)+
	rts

fade_i:	move.l	#15,d1
floop:	move.l	d1,-(a7)
	bsr.S	fade_in
	sub.w	#10,transz
	bsr.L	all_sprites
	move.l	(a7)+,d1
	dbf	d1,floop
	rts

fade_o:	move.l	#15,d1

floop2:	move.l	d1,-(a7)
	bsr.S	fade_out
	add.w	#10,transz
	bsr.L	all_sprites
	move.l	(a7)+,d1
	dbf	d1,floop2
	rts


fade_in:
	movem.l	d4-d7/a1/a2,-(a7)
	move.l	#$dff180,a0
	move.l	#32,d0
	lea.l	color,a1
	move.l	a1,a2
	add.l	#64,a2
repcols:
	move.w	(a1),d6
	move.w	(a2),d7
	move.w	d7,d5
	move.w	#1,d4
	move.l	#2,d3
rep_rgb:
	movem.l	d6/d7,-(a7)
	andi.w	#$f,d6
	andi.w	#$f,d7
	cmp.w	d6,d7
	beq.S	noaddcol
	add.w	d4,d5
noaddcol:
	movem.l	(a7)+,d6/d7
	lsr.w	#4,d6
	lsr.w	#4,d7
	lsl.w	#4,d4		
	dbf	d3,rep_rgb
	move.w	d5,(a2)+
	lea.l	2(a1),a1
	move.w	d5,(a0)+
	dbf	d0,repcols

	movem.l	(a7)+,d4-d7/a1/a2
	rts

fade_out:
	move.l	#$dff180,a0

	move.l	#31,d0
	lea.l	color,a2
	add.l	#64,a2
repcols2:
	move.w	(a2),d6
	move.w	d6,d5
	move.w	#1,d4
	move.l	#2,d3
rep_rgb2:
	movem.l	d6/d7,-(a7)
	andi.w	#$f,d6
	tst.w	d6
	beq.S	noaddcol2
	sub.w	d4,d5
noaddcol2:
	movem.l	(a7)+,d6/d7
	lsr.w	#4,d6
	lsl.w	#4,d4		
	dbf	d3,rep_rgb2
	move.w	d5,(a2)+
	move.w	d5,(a0)+
	dbf	d0,repcols2

	rts



maak_cop:
	add.l	#breedte_b*16+2,d2

	move.l	#num_planes-1,d0
	move.w	#$00e0,d1
do_scr:
	swap	d2
	move.w	d1,(a0)+
	move.w	d2,(a0)+
	swap	d2
	add.w	#2,d1
	move.w	d1,(a0)+
	move.w	d2,(a0)+
	add.w	#2,d1
	add.l	#plane_size,d2
	dbf	d0,do_scr	

	move.w	#$0180,d1
	move.l	#31,d2
	lea	color,a1
rep_col:
*	move.w	(a1)+,d3
*	move.w	d1,(a0)+
*	move.w	d3,(a0)+
	add.w	#2,d1
	dbf	d2,rep_col	

	move.l	#$008e3881,(a0)+
	move.l	#$009000c1,(a0)+
	move.l	#$01040020,(a0)+
	move.l	#$00920038,(a0)+
	move.l	#$009400d0,(a0)+

	move.l	#breedte_b,d0
	sub.l	#40,d0
	move.w	#$0108,(a0)+
	move.w	d0,(a0)+
	move.w	#$010a,(a0)+
	move.w	d0,(a0)+
	move.l	#$01020000,(a0)+
	move.l	#$0f0ffffe,(a0)+

	move.w	#$0100,(a0)+
	move.w	#num_planes,d1
	lsl.w	#4,d1
	or.w	#2,d1
	lsl.w	#8,d1
	move.w	d1,(a0)+
	move.l	#$fffffffe,(a0)+
	clr.l	d0
	rts

maak_masks:
	lea.l	shape_lijst_lijst,a0

meer_lijst1:
	move.l	(a0),a1
	cmpi.l	#0,a1
	beq.S	no_more_lijst1

more_masks:
	move.l	(a1)+,d6
	beq.S	no_more_masks

	move.l	d6,a6
	move.l	20(a6),a3

	move.l	#5,d5
rep_m_pl:
	move.l	(a6)+,a4
	cmpi.l	#0,a4
	beq.S	no_shapm
	move.l	a3,a5
	clr.l	d7
change_sp_ho:
	move.l	8(a0),d7
	sub.l	#1,d7
rep_h:
	clr.l	d6

change_sp_br:
	move.l	4(a0),d6
	sub.l	#1,d6
rep_b:	move.w	(a4)+,d0
	or.w	d0,(a5)+
	dbf	d6,rep_b
	dbf	d7,rep_h
no_shapm:
	dbf	d5,rep_m_pl
	bra.S	more_masks
no_more_masks:
	lea.l	12(a0),a0

	bra.S	meer_lijst1
no_more_lijst1:

	move.l	a0,testy
	clr.l	d0


	move.l	#sh26,d0
	cmpi.l	#$7ffff,d0
	bls.S	no_move

*** probeer de shapes naar het slowram te verhuizen

	lea.l	shape_lijst_lijst,a0

meer_lijst2:
	move.l	(a0),a1
	cmpi.l	#0,a1
	beq.S	no_more_lijst2

	lea.l	12(a0),a0
	move.l	mem_shapes,d0
	move.l	#sh26,d1

rep_verhuis:
	move.l	(a1)+,a2
	move.l	#num_planes,d7
rep_ver_planes:
	move.l	(a2),d2
	beq.S	niet_verhuis
	sub.l	d1,d2
	add.l	d0,d2
	move.l	d2,(a2)
niet_verhuis:
	lea.l	4(a2),a2
	dbf	d7,rep_ver_planes
	tst.l	(a1)
	bne.S	rep_verhuis
	bra.S	meer_lijst2
no_more_lijst2:

** verhuis de eigenlijke informatie
	lea.l	sh26,a0
	lea.l	color,a1
	move.l	mem_shapes,a2
rep_verhuis2:
	move.b	(a0)+,(a2)+
	cmp.l	a0,a1
	bhi.S	rep_verhuis2

no_move:
	rts

all_sprites:
	bsr.S	put_all_sprites

wl:	move.w	$dff006,d1
	lsr.w	#8,d1
	cmp.w	#20,d1
	bhi.S	wl

	move.l	mem_cop,$dff080
	clr.w	$dff088
	move.l	mem_cop,d0
	move.l	mem_cop2,mem_cop
	move.l	d0,mem_cop2
	move.l	mem_screen,d0
	move.l	mem_screen2,mem_screen
	move.l	d0,mem_screen2
	rts

put_all_sprites:
	move.l	mem_screen,d1
	add.l	#breedte_b*16+2,d1

	move.l	#num_planes-1,d7
repclear:

wab1:	btst	#14,$dff002
	bne.S	wab1
	move.w	d1,$dff056
	swap	d1
	move.w	d1,$dff054
	swap	d1
	move.w	#$0100,$dff040
	move.w	#$0,$dff042
	move.w	#4,$dff066
	move.w	#12820,$dff058
	add.l	#plane_size,d1
	dbf	d7,repclear

	lea.l	$dff000,a0
	bsr.L	draai_obj
	bsr.L	redraw

	bsr.L	bubblesort

	move.l	opwaarpoints,a2

	btst	#10,$dff016
	bne.W	reppll
*	add.l	#1,anglexinc
*	and.l	#$f,anglexinc

reppll:	move.w	(a2)+,d6
	cmp.w	#1000,d6
	beq.S	no_more_put
	move.w	(a2)+,d7
	cmpi.w	#336,d6
	bhi.S	niet_plaatsen
	cmpi.w	#216,d7
	bhi.S	niet_plaatsen

	move.w	2(a2),d5

	move.l	welke_shape_lijst,a3
	lsl.l	#2,d5
	lea.l	0(a3,d5),a3
	move.l	(a3),waar_shape


*	move.l	#shape_l1,waar_shape

	bsr.S	put_sprite

niet_plaatsen:
	addq.l	#4,a2

	bra.S	reppll
no_more_put:	
	rts


put_sprite:
	move.l	a2,-(a7)
	move.w	d7,d0
	mulu	#breedte_b,d0

	move.l	mem_screen,a2
	add.l	d0,a2
	move.w	d6,d1
	lsr.w	#4,d1
	move.w	d1,d2
	ext.l	d1
	lsl.l	#1,d1
	add.l	d1,a2	
	move.l	a2,waar_screen
	lsl.w	#4,d2
	sub.w	d2,d6

	lsl.w	#8,d6
	lsl.w	#4,d6
	or.w	#$0dfc,d6

	move.l	waar_shape,a4
	move.l	20(a4),waar_mask

ww9:	btst	#14,$dff002
	bne.S	ww9

change_sp_mod:
	move.w	#mod_sprite,$66(a0)
	move.w	#mod_sprite,$62(a0)

	move.w	#0,$64(a0)
*	move.w	#4,$62(a0)

	move.w	#0,$42(a0)
	move.w	#$ffff,$44(a0)
	move.w	#$ffff,$46(a0)

	move.l	#num_planes-1,d4
rep_pl:	move.l	(a4)+,d2

ww2:	btst	#14,$dff002
	bne.S	ww2

	move.l	waar_screen,d1
	bsr.S	plaats_mask

	tst.l	d2
	beq.S	no_plane

	move.l	waar_screen,d1

ww3:	btst	#14,$dff002
	bne.S	ww3

	move.l	d1,$54(a0)
	move.l	d1,$4c(a0)
	move.l	d2,$50(a0)	
	move.w	d6,$40(a0)
	move.w	#sprite_size,$58(a0)	
tijd6:
no_plane:
	add.l	#plane_size,waar_screen
	dbf	d4,rep_pl
no_put:
ww90:	btst	#14,$dff002
	bne.S	ww90

	move.l	(a7)+,a2
	rts	

plaats_mask:
	movem.l	d2/d6,-(a7)
	move.l	waar_mask,d2
	move.l	d1,$54(a0)
	move.l	d1,$4c(a0)
	move.l	d2,$50(a0)	
	and.w	#$ff00,d6
	or.w	#%00001100,d6
	move.w	d6,$40(a0)
change_sp_size2:
	move.w	#sprite_size,$58(a0)	
	movem.l	(a7)+,d2/d6
	rts

grname:		dc.b	"graphics.library",0
	even
mem_gebied:	dc.l	0
gfxbase:	dc.l	0	
mem_screen:	dc.l	0
mem_cop:	dc.l	0
mem_screen2:	dc.l	0
mem_cop2:	dc.l	0
mem_shapes:	dc.l	0
mem_clear:	dc.l	0
waar_shape:	dc.l	0
waar_screen:	dc.l	0
waar_mask:	dc.l	0

welke_shape_lijst:	dc.l	shape_lijst

shape_lijst_lijst:	dc.l	shape_lijst2,5,$36
			dc.l	shape_lijst,2,$f
			dc.l	0


shape_lijst2:	dc.l	shape_l21,shape_l22,shape_l23,shape_l24,0
shape_lijst:	dc.l	shape_l1,shape_l2,shape_l3,shape_l4,0

shape_l1:	dc.l	sh11,sh12,sh13,0,0,sh16
shape_l2:	dc.l	sh11,sh12,sh13,sh16,0,sh16
shape_l3:	dc.l	sh11,sh12,sh13,0,sh16,sh16
shape_l4:	dc.l	sh11,sh12,sh13,sh16,sh16,sh16

shape_l21:	dc.l	sh21,sh22,sh23,sh24,0,sh26
shape_l22:	dc.l	sh21,sh22,sh23,sh24,sh26,sh26
shape_l23:	dc.l	sh21,sh22,sh23,sh24,0,sh26
shape_l24:	dc.l	sh21,sh22,sh23,sh24,sh26,sh26


sh26:	blk.w	300,0
sh16:	blk.w	40,0

* Breedte (in words)= $0001

* Hoogte = $000f

sh11:	dc.w	$06c0,$0000
	dc.w	$1980,$0000
	dc.w	$2090,$0000
	dc.w	$40e8,$0000
	dc.w	$0068,$0000
	dc.w	$0c7c,$0000
	dc.w	$40c4,$0000
	dc.w	$0140,$0000
	dc.w	$9fc4,$0000
	dc.w	$5088,$0000
	dc.w	$6038,$0000
	dc.w	$1070,$0000
	dc.w	$3680,$0000
	dc.w	$0f80,$0000

sh12:	dc.w	$00c0,$0000
	dc.w	$0610,$0000
	dc.w	$1f10,$0000
	dc.w	$3f08,$0000
	dc.w	$7f88,$0000
	dc.w	$7f8c,$0000
	dc.w	$3f04,$0000
	dc.w	$3e80,$0000
	dc.w	$8004,$0000
	dc.w	$4008,$0000
	dc.w	$6038,$0000
	dc.w	$3070,$0000
	dc.w	$3fe0,$0000
	dc.w	$0fc0,$0000

sh13:	dc.w	$0f00,$0000
	dc.w	$3fe0,$0000
	dc.w	$3fe0,$0000
	dc.w	$7ff0,$0000
	dc.w	$7ff0,$0000
	dc.w	$fff0,$0000
	dc.w	$fff8,$0000
	dc.w	$fffc,$0000
	dc.w	$7ff8,$0000
	dc.w	$3ff0,$0000
	dc.w	$1fc0,$0000
	dc.w	$0f80,$0000
	dc.w	$0000,$0000
	dc.w	$0000,$0000


* Breedte (in words)= $0004

* Hoogte = $0036

sh21:	dc.w	$0000,$01e8,$8000,$0000,$0000
	dc.w	$0000,$1006,$d000,$0000,$0000
	dc.w	$0000,$9bfe,$3200,$0000,$0000
	dc.w	$0001,$e0bf,$9d00,$0000,$0000
	dc.w	$0003,$0080,$d340,$0000,$0000
	dc.w	$000e,$5df8,$19c0,$0000,$0000
	dc.w	$0031,$fffe,$0c68,$0000,$0000
	dc.w	$0007,$d13f,$8624,$0000,$0000
	dc.w	$001c,$004f,$cda0,$0000,$0000
	dc.w	$0074,$0203,$c9ec,$0000,$0000
	dc.w	$01f0,$7c42,$59e6,$0000,$0000
	dc.w	$02c0,$be81,$3833,$0000,$0000
	dc.w	$05a5,$f5e0,$7e3d,$8000,$0000
	dc.w	$0603,$ff80,$372d,$8000,$0000
	dc.w	$0307,$ff38,$4f1c,$c000,$0000
	dc.w	$0b0b,$ffe1,$0b0c,$4000,$0000
	dc.w	$0627,$ffee,$1716,$4000,$0000
	dc.w	$17bf,$fff9,$0fc2,$3000,$0000
	dc.w	$042f,$c7ec,$1feb,$0000,$0000
	dc.w	$040f,$83f2,$0fcf,$9000,$0000
	dc.w	$0d0f,$03a8,$0f67,$8800,$0000
	dc.w	$043f,$03be,$2643,$0000,$0000
	dc.w	$24af,$03bb,$27d3,$4800,$0000
	dc.w	$2725,$07a6,$00a7,$c000,$0000
	dc.w	$2681,$8e28,$14c5,$c800,$0000
	dc.w	$2782,$fee0,$5f62,$c800,$0000
	dc.w	$2700,$e500,$1e47,$e000,$0000
	dc.w	$21c1,$1e00,$04c5,$2800,$0000
	dc.w	$11f4,$0100,$1e63,$6000,$0000
	dc.w	$38c5,$0004,$2c63,$c000,$0000
	dc.w	$1af0,$8a28,$fc67,$8000,$0000
	dc.w	$0c3f,$85f5,$7cef,$c800,$0000
	dc.w	$2d1f,$dc3f,$78e7,$4800,$0000
	dc.w	$228b,$ffde,$f976,$0800,$0000
	dc.w	$05ae,$1ffd,$fbec,$8800,$0000
	dc.w	$3fe3,$87fa,$6aff,$1000,$0000
	dc.w	$1864,$0099,$e8fc,$2000,$0000
	dc.w	$0cbc,$0140,$09f0,$3000,$0000
	dc.w	$053d,$f004,$9f80,$2000,$0000
	dc.w	$079b,$fd46,$dd80,$2000,$0000
	dc.w	$0baa,$7ce9,$fc01,$4000,$0000
	dc.w	$05f0,$f7ff,$f402,$8000,$0000
	dc.w	$0034,$1f3d,$4042,$8000,$0000
	dc.w	$021d,$0208,$00fb,$0000,$0000
	dc.w	$010f,$8080,$27fc,$0000,$0000
	dc.w	$0065,$f6b9,$1ffc,$0000,$0000
	dc.w	$0070,$7fff,$fbd0,$0000,$0000
	dc.w	$006e,$397d,$43e4,$0000,$0000
	dc.w	$002b,$a462,$c800,$0000,$0000
	dc.w	$0000,$fc20,$0000,$0000,$0000
	dc.w	$0006,$7ebe,$c840,$0000,$0000
	dc.w	$0001,$8aaf,$fb00,$0000,$0000
	dc.w	$0000,$f817,$fc00,$0000,$0000
	dc.w	$0000,$3ec4,$2000,$0000,$0000

sh22:	dc.w	$0000,$0000,$8000,$0000,$0000
	dc.w	$0000,$0ff9,$1800,$0000,$0000
	dc.w	$0000,$7fff,$c200,$0000,$0000
	dc.w	$0001,$e0bf,$e100,$0000,$0000
	dc.w	$0007,$0000,$fc40,$0000,$0000
	dc.w	$000e,$0000,$1e00,$0000,$0000
	dc.w	$0010,$0000,$0f88,$0000,$0000
	dc.w	$0040,$2ec0,$07c4,$0000,$0000
	dc.w	$0003,$ffb0,$0dd0,$0000,$0000
	dc.w	$000b,$fffc,$01f0,$0000,$0000
	dc.w	$010f,$fffd,$81f8,$0000,$0000
	dc.w	$023f,$fffe,$c03c,$0000,$0000
	dc.w	$045f,$ffff,$803e,$0000,$0000
	dc.w	$05ff,$ffff,$c82e,$0000,$0000
	dc.w	$00ff,$ffff,$b01f,$0000,$0000
	dc.w	$08ff,$ffff,$f40f,$8000,$0000
	dc.w	$01ff,$ffff,$e817,$a000,$0000
	dc.w	$107f,$ffff,$f003,$c000,$0000
	dc.w	$03ff,$c7ff,$e00b,$f000,$0000
	dc.w	$23ff,$83ff,$f00f,$e000,$0000
	dc.w	$22ff,$03ff,$f007,$f000,$0000
	dc.w	$23ff,$03ff,$d803,$f800,$0000
	dc.w	$237f,$03ff,$d803,$f000,$0000
	dc.w	$20ff,$07ff,$f807,$f800,$0000
	dc.w	$217f,$8fff,$e8c5,$f000,$0000
	dc.w	$207f,$ffff,$a062,$f000,$0000
	dc.w	$60ff,$ffff,$e047,$f800,$0000
	dc.w	$603f,$ffff,$f8c5,$f000,$0000
	dc.w	$700b,$ffff,$e063,$f800,$0000
	dc.w	$383a,$fffb,$d063,$f800,$0000
	dc.w	$3a0f,$75d7,$0067,$f800,$0000
	dc.w	$3c00,$7a0a,$80ef,$f000,$0000
	dc.w	$1d00,$23c0,$80f7,$f000,$0000
	dc.w	$1e80,$0020,$0177,$f000,$0000
	dc.w	$1fa0,$0000,$03ef,$f000,$0000
	dc.w	$27e0,$0000,$02ff,$e000,$0000
	dc.w	$07e4,$0000,$00ff,$d000,$0000
	dc.w	$03fc,$0000,$09ff,$c000,$0000
	dc.w	$02fd,$f004,$9fff,$c000,$0000
	dc.w	$007f,$fd46,$dfff,$c000,$0000
	dc.w	$085f,$fce9,$fffe,$8000,$0000
	dc.w	$040f,$ffff,$fffd,$0000,$0000
	dc.w	$040b,$ffff,$ffbd,$0000,$0000
	dc.w	$0202,$ffff,$ff04,$0000,$0000
	dc.w	$0100,$7f7f,$d800,$0000,$0000
	dc.w	$00e0,$0946,$e000,$0000,$0000
	dc.w	$0070,$0000,$0000,$0000,$0000
	dc.w	$003e,$0000,$0004,$0000,$0000
	dc.w	$001f,$a000,$0000,$0000,$0000
	dc.w	$000f,$fc20,$0000,$0000,$0000
	dc.w	$0001,$febe,$c840,$0000,$0000
	dc.w	$0000,$7fff,$fb00,$0000,$0000
	dc.w	$0000,$07ff,$fe00,$0000,$0000
	dc.w	$0000,$013f,$f000,$0000,$0000

sh23:	dc.w	$0000,$03ff,$0000,$0000,$0000
	dc.w	$0000,$1fff,$e000,$0000,$0000
	dc.w	$0000,$ffff,$fc00,$0000,$0000
	dc.w	$0001,$e0bf,$fe00,$0000,$0000
	dc.w	$0007,$0000,$ff80,$0000,$0000
	dc.w	$000e,$0000,$1fe0,$0000,$0000
	dc.w	$0030,$0000,$0ff0,$0000,$0000
	dc.w	$0040,$0000,$07f8,$0000,$0000
	dc.w	$0000,$0000,$0dfc,$0000,$0000
	dc.w	$0000,$0000,$01fe,$0000,$0000
	dc.w	$0100,$0000,$01ff,$0000,$0000
	dc.w	$0200,$0000,$003f,$8000,$0000
	dc.w	$0400,$0000,$003f,$c000,$0000
	dc.w	$0400,$0000,$002f,$c000,$0000
	dc.w	$0000,$0000,$001f,$e000,$0000
	dc.w	$0800,$0000,$000f,$e000,$0000
	dc.w	$0000,$0000,$0017,$e000,$0000
	dc.w	$1000,$0000,$0003,$f000,$0000
	dc.w	$0000,$3800,$000b,$f000,$0000
	dc.w	$2000,$7c00,$000f,$f000,$0000
	dc.w	$2000,$fc00,$0007,$f800,$0000
	dc.w	$2000,$fc00,$0003,$f800,$0000
	dc.w	$2000,$fc00,$0003,$f800,$0000
	dc.w	$2000,$f800,$0007,$f800,$0000
	dc.w	$2000,$7000,$00c5,$f800,$0000
	dc.w	$2000,$0000,$0062,$f800,$0000
	dc.w	$6000,$0000,$0047,$f800,$0000
	dc.w	$6000,$0000,$00c5,$f800,$0000
	dc.w	$7000,$0000,$0063,$f800,$0000
	dc.w	$3800,$0000,$0063,$f800,$0000
	dc.w	$3a00,$0000,$0067,$f800,$0000
	dc.w	$3c00,$0000,$00ef,$f800,$0000
	dc.w	$3d00,$0000,$00f7,$f800,$0000
	dc.w	$3e80,$0000,$0177,$f800,$0000
	dc.w	$3fa0,$0000,$03ef,$f800,$0000
	dc.w	$1fe0,$0000,$02ff,$f800,$0000
	dc.w	$1fe4,$0000,$00ff,$f000,$0000
	dc.w	$1ffc,$0000,$09ff,$f000,$0000
	dc.w	$0ffd,$f004,$9fff,$e000,$0000
	dc.w	$0fff,$fd46,$dfff,$e000,$0000
	dc.w	$07ff,$fce9,$ffff,$e000,$0000
	dc.w	$03ff,$ffff,$ffff,$c000,$0000
	dc.w	$03ff,$ffff,$ffff,$c000,$0000
	dc.w	$01ff,$ffff,$ffff,$8000,$0000
	dc.w	$00ff,$ffff,$ffff,$0000,$0000
	dc.w	$001f,$ffff,$fffe,$0000,$0000
	dc.w	$000f,$ffff,$fffc,$0000,$0000
	dc.w	$0001,$ffff,$fff8,$0000,$0000
	dc.w	$0000,$5fff,$fff0,$0000,$0000
	dc.w	$0000,$03df,$ffe0,$0000,$0000
	dc.w	$0000,$0141,$3780,$0000,$0000
	dc.w	$0000,$0000,$0400,$0000,$0000
	dc.w	$0000,$0000,$0000,$0000,$0000
	dc.w	$0000,$0000,$0000,$0000,$0000

sh24:	dc.w	$0000,$0000,$0000,$0000,$0000
	dc.w	$0000,$0000,$0000,$0000,$0000
	dc.w	$0000,$0000,$0000,$0000,$0000
	dc.w	$0000,$1f40,$0000,$0000,$0000
	dc.w	$0000,$ffff,$0000,$0000,$0000
	dc.w	$0001,$ffff,$e000,$0000,$0000
	dc.w	$000f,$ffff,$f000,$0000,$0000
	dc.w	$003f,$ffff,$f800,$0000,$0000
	dc.w	$007f,$ffff,$f200,$0000,$0000
	dc.w	$00ff,$ffff,$fe00,$0000,$0000
	dc.w	$00ff,$ffff,$fe00,$0000,$0000
	dc.w	$01ff,$ffff,$ffc0,$0000,$0000
	dc.w	$03ff,$ffff,$ffc0,$0000,$0000
	dc.w	$03ff,$ffff,$ffd0,$0000,$0000
	dc.w	$07ff,$ffff,$ffe0,$0000,$0000
	dc.w	$07ff,$ffff,$fff0,$0000,$0000
	dc.w	$0fff,$ffff,$ffe8,$0000,$0000
	dc.w	$0fff,$ffff,$fffc,$0000,$0000
	dc.w	$1fff,$ffff,$fff4,$0000,$0000
	dc.w	$1fff,$ffff,$fff0,$0000,$0000
	dc.w	$1fff,$ffff,$fff8,$0000,$0000
	dc.w	$1fff,$ffff,$fffc,$0000,$0000
	dc.w	$1fff,$ffff,$fffc,$0000,$0000
	dc.w	$1fff,$ffff,$fff8,$0000,$0000
	dc.w	$1fff,$ffff,$ff3a,$0000,$0000
	dc.w	$1fff,$ffff,$ff9d,$0000,$0000
	dc.w	$1fff,$ffff,$ffb8,$0000,$0000
	dc.w	$1fff,$ffff,$ff3a,$0000,$0000
	dc.w	$0fff,$ffff,$ff9c,$0000,$0000
	dc.w	$07ff,$ffff,$ff9c,$0000,$0000
	dc.w	$05ff,$ffff,$ff98,$0000,$0000
	dc.w	$03ff,$ffff,$ff10,$0000,$0000
	dc.w	$02ff,$ffff,$ff08,$0000,$0000
	dc.w	$017f,$ffff,$fe88,$0000,$0000
	dc.w	$005f,$ffff,$fc10,$0000,$0000
	dc.w	$001f,$ffff,$fd00,$0000,$0000
	dc.w	$001b,$ffff,$ff00,$0000,$0000
	dc.w	$0003,$ffff,$f600,$0000,$0000
	dc.w	$0002,$0ffb,$6000,$0000,$0000
	dc.w	$0000,$02b9,$2000,$0000,$0000
	dc.w	$0000,$0316,$0000,$0000,$0000
	dc.w	$0000,$0000,$0000,$0000,$0000
	dc.w	$0000,$0000,$0000,$0000,$0000
	dc.w	$0000,$0000,$0000,$0000,$0000
	dc.w	$0000,$0000,$0000,$0000,$0000
	dc.w	$0000,$0000,$0000,$0000,$0000
	dc.w	$0000,$0000,$0000,$0000,$0000
	dc.w	$0000,$0000,$0000,$0000,$0000
	dc.w	$0000,$0000,$0000,$0000,$0000
	dc.w	$0000,$0000,$0000,$0000,$0000
	dc.w	$0000,$0000,$0000,$0000,$0000
	dc.w	$0000,$0000,$0000,$0000,$0000
	dc.w	$0000,$0000,$0000,$0000,$0000
	dc.w	$0000,$0000,$0000,$0000,$0000



color:	dc.w	$0000,$0400,$0500,$0700,$0900,$0c00,$0f00,$0fdd
	dc.w	$0000,$04a0,$05a0,$07a0,$09a0,$0ca0,$0fa0,$0fdd
	dc.w	$0000,$040a,$050a,$070a,$090a,$0c0a,$0f0a,$0fdd
	dc.w	$0000,$048f,$058f,$078f,$098f,$0c8f,$0f8f,$0fdd
	blk.w	34,0

col1:	dc.w	$0000,$0400,$0500,$0700,$0900,$0c00,$0f00,$0fdd
	dc.w	$0000,$04a0,$05a0,$07a0,$09a0,$0ca0,$0fa0,$0fdd
	dc.w	$0000,$040a,$050a,$070a,$090a,$0c0a,$0f0a,$0fdd
	dc.w	$0000,$048f,$058f,$078f,$098f,$0c8f,$0f8f,$0fdd

col2:	dc.w	$0000,$0200,$0300,$0400,$0500,$0600,$0700,$0800
	dc.w	$0900,$0a00,$0b00,$0c00,$0fff,$0fff,$0fff,$0fff
	dc.w	$0000,$020f,$030f,$040f,$050f,$060f,$070f,$080f
	dc.w	$090f,$0a0f,$0b0f,$0c0f,$0fff,$0fff,$0fff,$0fff



init_points:
	move.l	sinusangle,d4
	move.l	aantal_y,d7
	move.l	start_y,d1
repy:	move.l	aantal_x,d6
	move.l	start_x,d0
	move.l	d4,d3
repx:	move.w	d0,(a0)+
	add.l	#30,d3
	cmpi.l	#359,d3
	bls.S	nopo_sin2
	sub.l	#360,d3
nopo_sin2:
	move.l	d3,d2
	lsl.l	#1,d2
	lea.l	sinustabel,a1
	move.w	0(a1,d2.l),d2
	mulu	modulus,d2

*	mulu	#70,d2
	asr.w	#acur,d2

	move.w	d1,(a0)+
*	move.w	d2,(a0)+

	move.w	welke_z,(a0)+

	move.w	welke_sprite,(a0)+
*	move.w	#0,(a0)+


	add.w	mdelta_x,d0
	add.l	#1,aantalpoints
	dbf	d6,repx
	add.w	mdelta_y,d1
	add.l	#20,d4
	dbf	d7,repy

	move.w	#1000,(a0)+
	move.w	#1000,(a0)+
	move.w	#1000,(a0)+
	move.w	#1000,(a0)+
	rts


bubblesort:
	movem.l	d0-d7/a0-a2,-(a7)
	move.l	opwaarpoints,a0
	move.l	aantalpoints,d2
	lsl.l	#3,d2		;upper
	clr.l	d1		;lower
	move.l	d2,d7
	bsr.S	quicks
	movem.l	(a7)+,d0-d7/a0-a2
	rts


quicks:	cmp.l	d2,d1
	bge.S	uitquic
	movem.l	d1/d2,-(a7)
	addq.l	#8,d2
	bsr.S	partit
	movem.l	(a7)+,d1/d2

	movem.l	d1/d2/d3,-(a7)
	move.l	d3,d2
	tst.l	d2
	beq.S	noq
	subq.l	#8,d2
	bsr.S	quicks
noq:	movem.l	(a7)+,d1/d2/d3
	movem.l	d1/d2/d3,-(a7)
	move.l	d3,d1
	addq.l	#8,d1
	bsr.S	quicks
	movem.l	(a7)+,d1/d2/d3
uitquic:
	rts

partit:
	move.w	4(a0,d1.l),d4
	move.l	0(a0,d1.l),a1
	move.l	4(a0,d1.l),a2	
	move.l	d1,d5
inc_d5:	cmp.l	d7,d5
	bge.S	dec_d2
	addq.l	#8,d5
	cmp.w	4(a0,d5.l),d4
	bgt.S	inc_d5
dec_d2:	tst.l	d2
	beq.S	edec_d2
	subq.l	#8,d2
	cmp.w	4(a0,d2.l),d4
	blt.S	dec_d2
edec_d2:
	cmp.l	d2,d5
	bge.S	einc_d5
	move.l	4(a0,d5.l),d6
	move.l	4(a0,d2.l),4(a0,d5.l)
	move.l	d6,4(a0,d2.l)

	move.l	0(a0,d5.l),d6
	move.l	0(a0,d2.l),0(a0,d5.l)
	move.l	d6,0(a0,d2.l)

	bra.S	inc_d5
einc_d5:
	move.l	4(a0,d2.l),4(a0,d1.l)
	move.l	0(a0,d2.l),0(a0,d1.l)
	
	move.l	a1,0(a0,d2.l)
	move.l	a2,4(a0,d2.l)
	move.l	d2,d3
	rts



draai_obj:
	lea	anglex,a1
	lea	anglexinc,a2

	moveq	#2,d1
repinc:	move.l	(a1),d0
	add.l	(a2),d0
	move.l	d0,(a1)
	cmpi.l	#359,d0
	bls.s	nopangle
	sub.l	#360,d0
	move.l	d0,(a1)
*	clr.l	(a1)
nopangle:
	addq.l	#4,a1
	addq.l	#4,a2
	dbf	d1,repinc
	rts

redraw:	
	move.l	oppoints,a2
	move.l	opwaarpoints,a3
repdraw:
	cmpi.w	#1000,(a2)
	beq.S	no_more_points

	bsr.S	project1

	move.w	d0,(a3)+
	move.w	d1,(a3)+
	move.w	d2,(a3)+

	move.w	6(a2),(a3)+

	add.l	#8,a2
	bra.s	repdraw

no_more_points:
	move.w	#1000,(a3)+
	move.w	#1000,(a3)+
	move.w	#1000,(a3)+
	move.w	#1000,(a3)+
	clr.l	d0
	rts


project1:
	movem.l	d3-d7/a3/a2,-(a7)
	moveq	#0,d0
	moveq	#0,d1
	moveq	#0,d2
	move.w	(a2),d0
	move.w	2(a2),d1
	move.w	4(a2),d2

	add.w	transx,d0
	add.w	transy,d1
	add.w	transz,d2
	
* d0*2 om de schaal tussen y en z wat te herstelen

*	asl.w	#1,d0

	bsr.S	rotx
	bsr.S	roty
	bsr.S	rotz

*	sub.w	xo,d0
*	sub.w	yo,d1
	sub.w	zo,d2

	bsr.S	perspective3

	add.w	#xnul,d0
	add.w	#ynul,d1
	movem.l	(a7)+,d3-d7/a3/a2
	rts

rotz:	move.l	anglez,angle
	bsr.S	rot
	rts

roty:	move.l	angley,angle
	exg	d1,d2
	bsr.S	rot
	exg	d1,d2
	rts

rotx:	move.l	anglex,angle
	exg	d0,d2
	bsr.S	rot
	exg	d0,d2
	rts

perspective3:
	move.l	d3,-(a7)
	move.w	#64*dep,d3
	cmpi.w	#0,d2
	bne.S	nop_nul
	move.l	#1,d2
nop_nul:
*	neg.l	d2
	divs	d2,d3
	muls	d3,d1
	muls	d3,d0
	asr.w	#acur,d1
	asr.w	#acur,d0
	move.l	(a7)+,d3
	rts

rot:	move.l	d2,-(a7)
	movem.l	d0/d1,-(a7)

	move.l	angle,d4
	lsl.l	#1,d4
	lea.l	costabel(pc),a3
	muls	0(a3,d4.l),d0
	move.l	angle,d4
	lsl.l	#1,d4
	lea.l	sinustabel(pc),a3
	muls	0(a3,d4.l),d1
	sub.w	d1,d0
	move.w	d0,d6

	movem.l	(a7)+,d0/d1

	move.l	angle,d4
	lsl.l	#1,d4
	lea.l	sinustabel(pc),a3
	muls	0(a3,d4.l),d0
	move.l	angle,d4
	lsl.l	#1,d4
	lea.l	costabel(pc),a3
	muls	0(a3,d4.l),d1
	add.w	d0,d1
	move.w	d6,d0
	asr.w	#acur,d0
	asr.w	#acur,d1
	move.l	(a7)+,d2
	rts

sinustabel:
	dc.w	$0000,$0001,$0002,$0003,$0004,$0005,$0006,$0007
	dc.w	$0008,$000A,$000B,$000C,$000D,$000E,$000F,$0010
	dc.w	$0011,$0012,$0013,$0014,$0015,$0016,$0018,$0019
	dc.w	$001A,$001B,$001C,$001D,$001E,$001F,$0020,$0021
	dc.w	$0021,$0022,$0023,$0024,$0025,$0026,$0027,$0028
	dc.w	$0029,$002A,$002A,$002B,$002C,$002D,$002E,$002E
	dc.w	$002F,$0030,$0031,$0031,$0032,$0033,$0033,$0034
	dc.w	$0035,$0035,$0036,$0036,$0037,$0038,$0038,$0039
	dc.w	$0039,$003A,$003A,$003A,$003B,$003B,$003C,$003C
	dc.w	$003C,$003D,$003D,$003D,$003E,$003E,$003E,$003E
	dc.w	$003F,$003F,$003F,$003F,$003F,$003F,$003F,$003F
	dc.w	$003F,$003F,$003F,$003F
costabel:
	dc.w	$003F,$003F,$003F,$003F
	dc.w	$003F,$003F,$003F,$003F,$003E,$003E,$003E,$003E
	dc.w	$003E,$003D,$003D,$003D,$003C,$003C,$003C,$003B
	dc.w	$003B,$003A,$003A,$0039,$0039,$0038,$0038,$0037
	dc.w	$0037,$0036,$0036,$0035,$0034,$0034,$0033,$0032
	dc.w	$0032,$0031,$0030,$0030,$002F,$002E,$002D,$002D
	dc.w	$002C,$002B,$002A,$0029,$0028,$0028,$0027,$0026
	dc.w	$0025,$0024,$0023,$0022,$0021,$0020,$001F,$001E
	dc.w	$001D,$001C,$001B,$001A,$0019,$0018,$0017,$0016
	dc.w	$0015,$0014,$0013,$0012,$0011,$0010,$000F,$000D
	dc.w	$000C,$000B,$000A,$0009,$0008,$0007,$0006,$0005
	dc.w	$0004,$0002,$0001,$0000,$0000,$FFFF,$FFFE,$FFFD
	dc.w	$FFFC,$FFFA,$FFF9,$FFF8,$FFF7,$FFF6,$FFF5,$FFF4
	dc.w	$FFF3,$FFF2,$FFF1,$FFEF,$FFEE,$FFED,$FFEC,$FFEB
	dc.w	$FFEA,$FFE9,$FFE8,$FFE7,$FFE6,$FFE5,$FFE4,$FFE3
	dc.w	$FFE2,$FFE1,$FFE0,$FFDF,$FFDE,$FFDD,$FFDC,$FFDB
	dc.w	$FFDA,$FFDA,$FFD9,$FFD8,$FFD7,$FFD6,$FFD5,$FFD4
	dc.w	$FFD4,$FFD3,$FFD2,$FFD1,$FFD1,$FFD0,$FFCF,$FFCE
	dc.w	$FFCE,$FFCD,$FFCC,$FFCC,$FFCB,$FFCB,$FFCA,$FFC9
	dc.w	$FFC9,$FFC8,$FFC8,$FFC7,$FFC7,$FFC6,$FFC6,$FFC5
	dc.w	$FFC5,$FFC5,$FFC4,$FFC4,$FFC3,$FFC3,$FFC3,$FFC3
	dc.w	$FFC2,$FFC2,$FFC2,$FFC2,$FFC1,$FFC1,$FFC1,$FFC1
	dc.w	$FFC1,$FFC1,$FFC1,$FFC1,$FFC1,$FFC1,$FFC1,$FFC1
	dc.w	$FFC1,$FFC1,$FFC1,$FFC1,$FFC1,$FFC1,$FFC1,$FFC1
	dc.w	$FFC2,$FFC2,$FFC2,$FFC2,$FFC3,$FFC3,$FFC3,$FFC4
	dc.w	$FFC4,$FFC4,$FFC5,$FFC5,$FFC5,$FFC6,$FFC6,$FFC7
	dc.w	$FFC7,$FFC8,$FFC8,$FFC9,$FFC9,$FFCA,$FFCB,$FFCB
	dc.w	$FFCC,$FFCD,$FFCD,$FFCE,$FFCF,$FFCF,$FFD0,$FFD1
	dc.w	$FFD1,$FFD2,$FFD3,$FFD4,$FFD5,$FFD5,$FFD6,$FFD7
	dc.w	$FFD8,$FFD9,$FFDA,$FFDB,$FFDC,$FFDC,$FFDD,$FFDE
	dc.w	$FFDF,$FFE0,$FFE1,$FFE2,$FFE3,$FFE4,$FFE5,$FFE6
	dc.w	$FFE7,$FFE8,$FFE9,$FFEA,$FFEB,$FFEC,$FFEE,$FFEF
	dc.w	$FFF0,$FFF1,$FFF2,$FFF3,$FFF4,$FFF5,$FFF6,$FFF7
	dc.w	$FFF8,$FFFA,$FFFB,$FFFC,$FFFD,$FFFE,$FFFF,$0000
	dc.w	$0000,$0002,$0003,$0004,$0005,$0006,$0007,$0008
	dc.w	$0009,$000A,$000C,$000D,$000E,$000F,$0010,$0011
	dc.w	$0012,$0013,$0014,$0015,$0016,$0017,$0018,$0019
	dc.w	$001A,$001B,$001C,$001D,$001E,$001F,$0020,$0021
	dc.w	$0022,$0023,$0024,$0025,$0026,$0027,$0028,$0029
	dc.w	$0029,$002A,$002B,$002C,$002D,$002D,$002E,$002F
	dc.w	$0030,$0030,$0031,$0032,$0033,$0033,$0034,$0035
	dc.w	$0035,$0036,$0036,$0037,$0037,$0038,$0038,$0039
	dc.w	$0039,$003A,$003A,$003B,$003B,$003C,$003C,$003C
	dc.w	$003D,$003D,$003D,$003E,$003E,$003E,$003E,$003F
	dc.w	$003F,$003F,$003F,$003F,$003F,$003F,$003F,$003F
	dc.w	$003F,$0040,$0040

dosname:	dc.b	"dos.library",0
	even
conhandle:	dc.l	0
aantal_x:	dc.l	0
aantal_y:	dc.l	0
start_x:	dc.l	0
start_y:	dc.l	0
mdelta_x:	dc.w	15
mdelta_y:	dc.w	25

transx:	dc.w	0
transy:	dc.w	0
transz:	dc.w	0

angle:	dc.l	0
anglex:	dc.l	0
angley:	dc.l	0
anglez:	dc.l	0

wel_inc:	dc.w	5

welke_z:	dc.w	0
welke_sprite:	dc.w	0

xo:	dc.w	ixo
yo:	dc.w	iyo
zo:	dc.w	izo
modulus:	dc.w	0
mod_inc:	dc.w	1
tempo_angle:	dc.l	0
tempo_x:	dc.l	0
tempo_y:	dc.l	0
tijdteller:	dc.l	0
change_teller:	dc.l	20
anglexinc:	dc.l	3,4,5,0

cirhoek:	dc.l	0
sinusangle:	dc.l	0
sinusangle2:	dc.l	0
aantalpoints:	dc.l	0
mem_oppoints:	dc.l	0
oppoints:	dc.l	0
opwaarpoints:	dc.l	0
welk_patroon:	dc.l	0

currentpatroon:	dc.l	0

patroonlijst:	dc.l	patroon9,patroon8,patroon6,patroon7
		dc.l	patroon1,patroon2,patroon3,patroon4
		dc.l	patroon5,0

modulbit:	dc.b	1


patroon6:	dc.b	2,13,2,13,2,13,2,13
		dc.b	13,2,13,2,13,2,13,2
		dc.b	2,13,2,13,2,13,2,13
		dc.b	13,2,13,2,13,2,13,2
		dc.b	2,13,2,13,2,13,2,13
		dc.b	13,2,13,2,13,2,13,2
		dc.b	2,13,2,13,2,13,2,13
		dc.b	13,2,13,2,13,2,13,2

patroon7:	dc.b	1,1,1,1,1,1,1,1
		dc.b	1,2,2,2,2,2,2,1
		dc.b	1,2,3,3,3,3,2,1
		dc.b	1,2,3,4,4,3,2,1
		dc.b	1,2,3,4,4,3,2,1
		dc.b	1,2,3,3,3,3,2,1
		dc.b	1,2,2,2,2,2,2,1
		dc.b	1,1,1,1,1,1,1,1

patroon8:	dc.b	4,4,4,4,4,4,4,11
		dc.b	4,11,11,11,11,11,11,11
		dc.b	4,4,4,4,4,4,4,11
		dc.b	4,11,11,11,11,11,11,11
		dc.b	4,4,4,4,4,4,4,11
		dc.b	4,11,11,11,11,11,11,11
		dc.b	4,4,4,4,4,4,4,11
		dc.b	4,11,11,11,11,11,11,11

patroon9:	dc.b	1,1,1,0,0,2,2,2
		dc.b	1,1,1,0,0,2,2,2
		dc.b	1,1,1,0,0,2,2,2
		dc.b	0,0,0,14,13,0,0,0
		dc.b	0,0,0,12,11,0,0,0
		dc.b	3,3,3,0,0,4,4,4
		dc.b	3,3,3,0,0,4,4,4
		dc.b	3,3,3,0,0,4,4,4

patroon1:	dc.b	1,1,1,1,1,1,1,1
		dc.b	1,1,1,1,1,1,1,1
		dc.b	1,1,13,13,13,13,1,1
		dc.b	1,1,13,12,12,13,1,1
		dc.b	1,1,13,12,12,13,1,1
		dc.b	1,1,13,13,13,13,1,1
		dc.b	1,1,1,1,1,1,1,1
		dc.b	1,1,1,1,1,1,1,1

patroon2:	dc.b	1,1,1,1,1,1,1,1
		dc.b	1,14,14,14,14,14,14,1
		dc.b	1,14,1,1,1,1,14,1
		dc.b	1,14,1,1,1,1,14,1
		dc.b	1,14,1,1,1,1,14,1
		dc.b	1,14,1,1,1,1,14,1
		dc.b	1,14,14,14,14,14,14,1
		dc.b	1,1,1,1,1,1,1,1

patroon3:	dc.b	0,0,0,0,0,0,0,0
		dc.b	1,0,0,0,1,4,4,0
		dc.b	1,0,0,1,4,0,0,0
		dc.b	1,0,1,0,4,0,0,0
		dc.b	1,1,1,0,4,0,0,0
		dc.b	1,0,1,0,4,0,0,0
		dc.b	1,0,0,1,0,4,4,0	
		dc.b	0,0,0,0,0,0,0,0

patroon4:	dc.b	13,13,1,1,1,1,13,13
		dc.b	1,13,13,1,1,13,13,1
		dc.b	1,1,13,13,13,13,1,1
		dc.b	1,1,1,13,13,1,1,1
		dc.b	1,1,13,13,13,13,1,1
		dc.b	1,13,13,1,1,13,13,1
		dc.b	13,13,1,1,1,1,13,13
		dc.b	0,0,0,0,0,0,0,0

patroon5:	dc.b	1,1,1,1,11,11,11,11
		dc.b	1,1,1,1,11,11,11,11
		dc.b	1,1,1,1,11,11,11,11
		dc.b	1,1,1,1,11,11,11,11
		dc.b	11,11,11,11,1,1,1,1
		dc.b	11,11,11,11,1,1,1,1
		dc.b	11,11,11,11,1,1,1,1	
		dc.b	11,11,11,11,1,1,1,1

	even
maakpatroon:
	move.l	waar_patroon,a0
	move.l	a0,a5
	add.l	#extra_mem,a5
	sub.l	#4000,a5
	clr.l	d5

	lea.l	tekst,a1
	lea.l	letters,a2
replet:
	cmp.l	a0,a5
	bls.L	finito2

	clr.l	d0
	move.b	(a1)+,d0
	beq.S	finito
	cmp.b	#" ",d0
	bne.S	no_spatie
	add.l	#6*5,a0

	bra.S	replet
no_spatie:
	add.l	#1,d5
	cmpi.l	#4,d5
	bls.S	mnopo
	moveq	#1,d5
mnopo:
	sub.l	#"a",d0
	lsl.l	#3,d0
	lea.l	0(a2,d0.w),a3
	move.l	(a3),a4
	move.l	4(a3),d7
	sub.l	#1,d7
repcopy:
	move.l	#5,d6
repcopy4:
	tst.b	(a4)+
	beq.S	niets2
	move.b	d5,(a0)+
	bra.S	niets3
niets2:
	move.b	#0,(a0)+
niets3:
	dbf	d6,repcopy4

*	move.w	(a4)+,(a0)+
*	move.w	(a4)+,(a0)+
*	move.w	(a4)+,(a0)+

	dbf	d7,repcopy
	add.l	#6,a0
	bra.S	replet	
	
finito:	
	move.l	a0,eind_patroon

	move.l	#breedte,d3
	lea.l	tekst,a1
	lea.l	letters,a2
replet2:
	cmp.l	a0,a5
	bls.S	finito2

	clr.l	d0
	move.b	(a1)+,d0
	beq.S	finito2
	cmp.b	#" ",d0
	bne.S	no_spatie2
	add.l	#6*5,a0
	bra.S	replet2
no_spatie2:
	add.l	#1,d5
	cmpi.l	#4,d5
	bls.S	mnopo2
	moveq	#1,d5
mnopo2:

	sub.l	#"a",d0
	lsl.l	#3,d0
	lea.l	0(a2,d0.w),a3
	move.l	(a3),a4
	move.l	4(a3),d7
	sub.l	#1,d7
repcopy2:

	move.l	#5,d6
repcopy5:
	tst.b	(a4)+
	beq.S	niets4
	move.b	d5,(a0)+
	bra.S	niets5
niets4:
	move.b	#0,(a0)+
niets5:
	dbf	d6,repcopy5

	sub.l	#1,d3
	bmi.S	finito2
	dbf	d7,repcopy2
	add.l	#6,a0
	bra.S	replet2
	
finito2:	
	rts


sinit_points:
	move.l	sinusangle,d4
	move.l	aantal_y,d7
	move.l	start_y,d1
	move.l	welk_patroon,a4

srepy:	move.l	aantal_x,d6
	move.l	start_x,d0
	move.l	d4,d3
srepx:	

	move.w	d0,(a0)
	add.l	#1,d3
	cmpi.l	#359,d3
	bls.S	snopo_sin2
	sub.l	#360,d3
snopo_sin2:
	tst.b	(a4)
	beq.S	sno_patr
	lea.l	2(a0),a0
	move.l	d3,d2
	lsl.l	#1,d2
	lea.l	sinustabel,a1
	move.w	0(a1,d2.l),d2
	mulu	modulus,d2
	asr.w	#acur,d2
	move.w	d1,(a0)+
	cmpi.b	#10,(a4)
	bls.S	sno_zadd
	add.w	welke_z,d2
sno_zadd:
	move.w	d2,(a0)+

*	move.w	welke_z,(a0)+

	moveq	#0,d2
	move.b	(a4),d2
	cmpi.b	#10,d2
	bls.S	sno_po10
	sub.b	#10,d2
sno_po10:
	sub.b	#1,d2
	move.w	d2,(a0)+
	add.l	#1,aantalpoints
sno_patr:
	add.w	#25,d0
	lea.l	1(a4),a4
	dbf	d6,srepx
	add.w	#20,d1
	add.l	#20,d4
	dbf	d7,srepy
	move.w	#1000,(a0)+
	move.w	#1000,(a0)+
	move.w	#1000,(a0)+
	move.w	#1000,(a0)+
	rts

scroll_teller:	dc.l	0
waar_patroon:	dc.l	0
eind_patroon:	dc.l	0

tekst:		dc.b	"    hallo dit is een test van bolletjes"
		dc.b	" scroll"
		dc.b	" als je dit goed wil lezen druk dan rechter"
		dc.b	" muisknop in",0

	even
letters:	dc.l	leta,3
		dc.l	letb,3
		dc.l	letc,3
		dc.l	letd,3
		dc.l	lete,3
		dc.l	letf,3
		dc.l	letg,3
		dc.l	leth,3
		dc.l	leti,1
		dc.l	letj,3
		dc.l	letk,3
		dc.l	letl,3
		dc.l	letm,5
		dc.l	letn,3
		dc.l	leto,3
		dc.l	letp,3
		dc.l	letq,3
		dc.l	letr,3
		dc.l	lets,3
		dc.l	lett,3
		dc.l	letu,3
		dc.l	letv,5
		dc.l	letw,5
		dc.l	letx,3
		dc.l	lety,3
		dc.l	letz,4
	

leta:
	dc.b	1,1,1,1,0,0
	dc.b	0,1,0,0,1,0
	dc.b	1,1,1,1,0,0

letb:
	dc.b	1,1,1,1,1,0
	dc.b	1,0,1,0,1,0
	dc.b	1,1,1,1,0,0

letc:
	dc.b	1,1,1,1,1,0
	dc.b	1,0,0,0,1,0
	dc.b	1,0,0,0,1,0

letd:	dc.b	1,1,1,1,1,0
	dc.b	1,0,0,0,1,0
	dc.b	1,1,1,1,0,0

lete:	dc.b	1,1,1,1,1,0
	dc.b	1,0,1,0,1,0
	dc.b	1,0,1,0,1,0

letf:	dc.b	1,1,1,1,1,0
	dc.b	0,0,1,0,1,0
	dc.b	0,0,0,0,1,0

letg:	dc.b	1,0,0,1,1,0
	dc.b	1,0,1,0,1,0
	dc.b	1,1,1,1,1,0

leth:	dc.b	1,1,1,1,1,0
	dc.b	0,0,1,0,0,0
	dc.b	1,1,1,1,1,0

leti:	dc.b	1,1,1,1,0,1

letj:	dc.b	0,1,0,0,0,0
	dc.b	1,0,0,0,0,0
	dc.b	0,1,1,1,0,1

letk:	dc.b	1,1,1,1,1,0
	dc.b	0,1,0,1,0,0
	dc.b	1,0,0,0,1,0

letl:	dc.b	1,1,1,1,1,0
	dc.b	1,0,0,0,0,0
	dc.b	1,0,0,0,0,0

letm:	dc.b	1,1,1,1,1,0
	dc.b	0,0,0,1,0,0
	dc.b	0,0,1,0,0,0
	dc.b	0,0,0,1,0,0
	dc.b	1,1,1,1,1,0

letn:	dc.b	1,1,1,1,1,0
	dc.b	0,0,0,0,1,0
	dc.b	1,1,1,1,0,0

leto:	dc.b	1,1,1,1,1,0
	dc.b	1,0,0,0,1,0
	dc.b	1,1,1,1,1,0

letp:	dc.b	1,1,1,1,1,0
	dc.b	0,0,1,0,1,0
	dc.b	0,0,1,1,1,0

letq:	dc.b	0,0,1,1,1,0
	dc.b	0,0,1,0,1,0
	dc.b	1,1,1,1,1,0

letr:	dc.b	1,1,1,1,1,0
	dc.b	0,1,1,0,1,0
	dc.b	1,0,1,1,1,0

lets:	dc.b	1,0,1,1,1,0
	dc.b	1,0,1,0,1,0
	dc.b	1,1,1,0,1,0

lett:	dc.b	0,0,0,0,1,0
	dc.b	1,1,1,1,1,0
	dc.b	0,0,0,0,1,0

letu:	dc.b	1,1,1,1,1,0
	dc.b	1,0,0,0,0,0
	dc.b	1,1,1,1,1,0

letv:	dc.b	0,0,0,1,1,0
	dc.b	0,1,1,0,0,0
	dc.b	1,0,0,0,0,0
	dc.b	0,1,1,0,0,0
	dc.b	0,0,0,1,1,0

letw:	dc.b	1,1,1,1,1,0
	dc.b	0,1,0,0,0,0
	dc.b	0,0,1,0,0,0
	dc.b	0,1,0,0,0,0
	dc.b	1,1,1,1,1,0

letx:	dc.b	1,1,0,1,1,0
	dc.b	0,1,1,1,0,0
	dc.b	1,1,0,1,1,0

lety:	dc.b	0,0,0,1,1,0
	dc.b	1,1,1,0,0,0
	dc.b	0,0,0,1,1,0

letz:	dc.b	1,0,0,0,1,0
	dc.b	1,0,1,1,1,0
	dc.b	1,1,1,0,1,0
	dc.b	1,0,0,0,1,0

points:
	dc.w	50,50,50,0
	dc.w	-50,-50,50,0
	dc.w	50,-50,50,1
	dc.w	-50,50,50,1

	dc.w	-50,-50,-50,1
	dc.w	50,-50,-50,0
	dc.w	-50,50,-50,0
	dc.w	50,50,-50,1

	dc.w	1000,1000,1000,0


	dc.w	-150,0,0,0
	dc.w	-75,0,0,1
	dc.w	0,0,0,0
	dc.w	75,0,0,1
	dc.w	150,0,0,0

	dc.w	1000,1000,1000,0

	dc.w	-90,30,0,0
	dc.w	-90,15,0,0
	dc.w	-90,0,0,0
	dc.w	-90,-15,0,0
	dc.w	-90,-30,0,0
	dc.w	-60,30,0,0
	dc.w	-75,30,0,0
	dc.w	-60,-30,0,0
	dc.w	-75,-30,0,0

	dc.w	-40,30,0,2
	dc.w	-25,30,0,2
	dc.w	-10,30,0,2
	dc.w	-40,15,0,2
	dc.w	-40,0,0,2
	dc.w	-30,0,0,2
	dc.w	-20,0,0,2
	dc.w	-40,-15,0,2
	dc.w	-40,-30,0,2
	dc.w	-25,-30,0,2
	dc.w	-10,-30,0,2

	dc.w	10,30,0,3
	dc.w	25,30,0,3
	dc.w	40,30,0,3
	dc.w	10,15,0,3
	dc.w	10,0,0,3
	dc.w	20,0,0,3
	dc.w	30,0,0,3
	dc.w	10,-15,0,3
	dc.w	10,-30,0,3
	dc.w	25,-30,0,3
	dc.w	40,-30,0,3

	dc.w	60,30,0,1
	dc.w	75,30,0,1
	dc.w	90,30,0,1
	dc.w	60,15,0,1
	dc.w	60,0,0,1
	dc.w	75,0,0,1
	dc.w	90,0,0,1
	dc.w	90,-15,0,1
	dc.w	90,-30,0,1
	dc.w	75,-30,0,1
	dc.w	60,-30,0,1
	dc.w	1000,1000,1000


cijferlijst:
	dc.l	negen,acht,zeven,zes,vijf,vier,drie,twee,een,nul

een:	dc.b	0,0,1,0,0,0,0,0
	dc.b	0,1,1,0,0,0,0,0
	dc.b	1,0,1,0,0,0,0,0
	dc.b	0,0,1,0,0,0,0,0
	dc.b	0,0,1,0,0,0,0,0
	dc.b	0,0,1,0,0,0,0,0
	dc.b	0,0,1,0,0,0,0,0
	dc.b	0,1,1,1,0,0,0,0

twee:	dc.b	0,0,1,1,0,0,0,0
	dc.b	0,1,0,0,1,0,0,0
	dc.b	1,0,0,0,1,0,0,0
	dc.b	0,0,0,1,0,0,0,0
	dc.b	0,0,0,1,0,0,0,0
	dc.b	0,0,1,0,0,0,0,0
	dc.b	0,0,1,0,0,0,0,0
	dc.b	1,1,1,1,1,0,0,0


drie:	dc.b	0,1,1,0,0,0,0,0
	dc.b	1,0,0,1,0,0,0,0
	dc.b	0,0,0,1,0,0,0,0
	dc.b	0,0,1,0,0,0,0,0
	dc.b	0,0,0,1,0,0,0,0
	dc.b	0,0,0,0,1,0,0,0
	dc.b	1,0,0,0,1,0,0,0
	dc.b	0,1,1,1,0,0,0,0

vier:	dc.b	0,1,0,0,1,0,0,0
	dc.b	0,1,0,0,1,0,0,0
	dc.b	1,0,0,0,1,0,0,0
	dc.b	1,0,0,0,1,0,0,0
	dc.b	1,1,1,1,1,0,0,0
	dc.b	0,0,0,0,1,0,0,0
	dc.b	0,0,0,0,1,0,0,0
	dc.b	0,0,0,0,1,0,0,0

vijf:	dc.b	1,1,1,1,1,0,0,0
	dc.b	1,0,0,0,0,0,0,0
	dc.b	1,0,0,0,0,0,0,0
	dc.b	1,1,1,1,0,0,0,0
	dc.b	0,0,0,0,1,0,0,0
	dc.b	0,0,0,0,1,0,0,0
	dc.b	1,0,0,0,1,0,0,0
	dc.b	0,1,1,1,0,0,0,0	

zes:	dc.b	0,0,1,0,0,0,0,0
	dc.b	0,1,0,0,0,0,0,0
	dc.b	0,1,0,0,0,0,0,0
	dc.b	1,0,1,1,0,0,0,0
	dc.b	1,1,0,0,1,0,0,0
	dc.b	1,0,0,0,1,0,0,0
	dc.b	1,0,0,0,1,0,0,0
	dc.b	0,1,1,1,0,0,0,0

zeven:	dc.b	1,1,1,1,1,0,0,0
	dc.b	0,0,0,0,1,0,0,0
	dc.b	0,0,0,1,0,0,0,0
	dc.b	0,0,0,1,0,0,0,0
	dc.b	0,0,1,0,0,0,0,0
	dc.b	0,0,1,0,0,0,0,0
	dc.b	0,1,0,0,0,0,0,0
	dc.b	0,1,0,0,0,0,0,0

acht:	dc.b	0,0,1,1,0,0,0,0
	dc.b	0,1,0,0,1,0,0,0
	dc.b	0,1,0,0,1,0,0,0
	dc.b	0,0,1,1,0,0,0,0
	dc.b	0,1,0,0,1,0,0,0
	dc.b	1,0,0,0,0,1,0,0
	dc.b	0,1,0,0,1,0,0,0
	dc.b	0,0,1,1,0,0,0,0

negen:	dc.b	0,1,1,1,0,0,0,0
	dc.b	1,0,0,0,1,0,0,0
	dc.b	1,0,0,1,1,0,0,0
	dc.b	0,1,1,0,1,0,0,0
	dc.b	0,0,0,0,1,0,0,0
	dc.b	0,0,0,1,0,0,0,0
	dc.b	0,0,0,1,0,0,0,0
	dc.b	0,0,1,0,0,0,0,0
nul:	dc.b	0,1,1,1,0,0,0,0
	dc.b	1,0,0,0,1,0,0,0
	dc.b	1,0,0,0,1,0,0,0
	dc.b	1,0,0,0,1,0,0,0
	dc.b	1,0,0,0,1,0,0,0
	dc.b	1,0,0,0,1,0,0,0
	dc.b	1,0,0,0,1,0,0,0
	dc.b	0,1,1,1,0,0,0,0
testy:	dc.l	0
