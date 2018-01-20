*
* File	: playcdxl.s
* Uses	: ab_cdxl.i pan.i anhd.i
* Date	: 14 january 1993 
* Author: ing. C. Lieshout
* Desc	: Try to read an play a cdxl file
*
* Update: Enlarge only vertical when resolution is too big for horizontal 
* Date	: 21 oct 1993 


	INCDIR "include:"

	INCLUDE "exec/execbase.i"
	INCLUDE "exec/memory.i"
	INCDIR	"include:"
	INCLUDE "graphics/gfxbase.i"
	INCLUDE "graphics/displayinfo.i"
	INCLUDE	"dos/dos.i"
	INCLUDE "graphics/view.i"
	INCLUDE "graphics/rastport.i"
	INCLUDE "graphics/displayinfo.i"
	INCLUDE "graphics/videocontrol.i"
	INCLUDE "hardware/custom.i"
	INCLUDE "intuition/preferences.i"

	INCDIR	"wp:inclibs/"
	INCLUDE "exec_lib.i"
	INCLUDE "dos_lib.i"
	INCLUDE "graphics_libv39.i"
	INCLUDE "intuition_lib.i"

	INCDIR	"cdxl:asm/"
	INCLUDE	"anhd.i"
	INCLUDE	"ab_cdxl.i"
	INCLUDE	"pan.i"
	INCDIR	"wp:asm/"
	INCLUDE	"proces.i"
	INCLUDE	"system.i"

DEBUG = 0
XAPP = 1
MON_ID = DBLPAL_MONITOR_ID
;MON_ID = SUPER72_MONITOR_ID

LINEWAIT = 0
OFFSET = 0
CALC_CDTV = 0
speed= 12
LOOP = 0
DISKANIM = 1
BIGGER = 1

ROTS = -1

LIBV39 = 39

LIBV36 = 36

exec=4

mode_old=1005
key = $bfec01

MEMSIZE = 230000
TABLE_SIZE = 263000

;	bra	start

	IFNE	DEBUG
	XREF	KPutFmt
	XREF	KPutStr
	PRINTT	"De debug info staat aan dus je moet nog met debug.lib linken"
	ENDC

	IFEQ XAPP
top
	bset	#1,$bfe001
	lea	datablock(pc),a3
	move.l	#12,ab_frame_speed(a3)	; initial speed
	move.b	#0,ab_calc_cdtv(a3)	; calc from speed
	move.l	#-1,ab_num_rot(a3)
	subq	#1,d0
	beq.W	input_error
zoeken:
	cmp.b	#$20,(a0)+
	bne.s	gevonden
	dbra	d0,zoeken
	bra.W	input_error
gevonden:
	subq.l	#1,a0
	move.l	a0,ab_filenaam(a3)
zet:	cmp.b	#10,(a0)
	beq.w	zetnul
	cmp.b	#' ',(a0)
	beq.s	zetnul2

	addq.l	#1,a0
	bra.s	zet
zetnul2:
	move.b	#0,(a0)	; maak van de spatie na de ab_filenaam(a3) een nul
	addq.l	#1,a0	; a0 naar volgende commando

get_nextpar:
	tst.b	(a0)
	beq.w	no_in
	cmp.b	#'l',(a0)
	bne.b	no_loop
	addq.l	#1,a0
no_loop:
	cmp.b	#"c",(a0)
	bne.b	no_cdcalc
	move.b	#1,ab_calc_cdtv(a3)
	addq.l	#1,a0
	bra.b	get_nextpar
no_cdcalc:
	cmp.b	#"i",(a0)
	bne.b	no_info
	move.b	#1,ab_info(a3)
	addq.l	#1,a0
	bra.b	get_nextpar
no_info:
	cmp.b	#'o',(a0)
	bne	no_offset
	bsr	get_number
	move.w	d0,ab_mod_normal(a3)
no_offset:
	cmp.b	#"s",(a0)
	bne.b	no_speed

	bsr	get_number
	move.l	d0,ab_frame_speed(a3)
	move.l	d0,ab_overallspeed(a3)
no_speed:

	cmp.b	#"r",(a0)
	bne.b	no_rot
	bsr	get_number
	move.l	d0,ab_num_rot(a3)
	move.l	d0,ab_num_rot(a3)
no_rot:
	cmp.b	#"x",(a0)
	bne.b	no_x_co
	bsr	get_number
	move.w	d0,ab_dx(a3)
no_x_co:
	cmp.b	#"y",(a0)
	bne.b	no_y_co
	bsr	get_number
	move.w	d0,ab_dy(a3)
no_y_co:
	cmp.b	#'d',(a0)
	bne.b	no_disk4
	move.b	#1,ab_diskanim(a3)

no_disk4:
	cmp.b	#'b',(a0)
	bne.b	no_doub
	move.b	#$1,ab_double(a3)

no_doub:
	cmp.b	#0,(a0)
	beq	no_in
	addq.l	#1,a0
	bra.w	get_nextpar

	bra.w	no_in
zetnul:
	move.b	#0,(a0)
	bra.w	no_in

get_number:
	moveq	#1,d2
	moveq	#0,d0
rep_number:
	addq.l	#1,a0
	cmp.b	#'-',(a0)
	bne	no_minus
	neg.l	d2
	bra	rep_number
no_minus:
	moveq	#0,d1
	move.b	(a0),d1
	cmp.b	#'0',d1
	blt.b	exit_gn
	cmp.b	#'9',d1
	bgt.b	exit_gn
	sub.l	#'0',d1
	mulu	#10,d0
	add.l	d1,d0
	bra	rep_number
exit_gn:
	mulu	d2,d0
	rts

start:
	bset	#1,$bfe001
	lea	datablock(pc),a3
	lea	filename(pc),a0
	move.w	#0,ab_dx(a3)
	move.w	#0,ab_dy(a3)
	move.l	a0,ab_filenaam(a3)
	move.b	#CALC_CDTV,ab_calc_cdtv(a3)
	
	move.b	#BIGGER,ab_double(a3)
	move.b	#DISKANIM,ab_diskanim(a3)
	move.l	#speed,ab_overallspeed(a3)
	move.l	#speed,ab_frame_speed(a3)
	move.w  #OFFSET,ab_mod_normal(a3)
	move.l	#ROTS,ab_num_rot(a3)
	move.l	#320,ab_lenx(a3)
	move.l	#256,ab_leny(a3)
	move.w	#5,ab_planes(a3)
	move.l	#40,ab_breedte_x(a3)
	move.l	#51200,ab_unpacked_size(a3)

no_in:
	move.l	#0,ab_frames(a3)

	lea	tellerstart(pc),a0
	move.l	ab_overallspeed(a3),(a0)
	move.w	#640,ab_columns(a3)
	move.w	#256,ab_rows(a3)

	lea	ab_view1(a3),a0
	move.l	a0,ab_wviewblock1(a3)
	lea	ab_view2(a3),a0
	move.l	a0,ab_wviewblock2(a3)
	move.l	a7,ab_easy_exit(a3)

	bsr.w	openlibs

; reserveer geheugen voor tweede scherm

	tst.l	ab_unpackedsecond(a3)
	bne.b	already_assigned_second

	move.l	$4.w,a6
	move.l	#MEMSIZE,d0
	move.l	#$10002,d1
	jsr	_LVOAllocMem(a6)
	move.l	#1,d7
	move.l	d0,ab_unpackedsecond(a3)
	beq.w	exit
already_assigned_second:
	move.l	#$10002,d1
	move.l	$4.w,a6
	move.l	#MEMSIZE,d0
	jsr	_LVOAllocMem(a6)
	move.l	d0,ab_unpacked(a3)
	beq.w	exit

	move.l	#$10000,d1
	move.l	$4.w,a6
	move.l	#TABLE_SIZE,d0
	jsr	_LVOAllocMem(a6)
	move.l	d0,ab_low_hi(a3)

	move.l	ab_unpacked(a3),ab_frame_hidden(a3)
	move.l	ab_unpackedsecond(a3),ab_frame_show(a3)

	move.l	#MON_ID,ab_monid(a3)

	bsr.w	read_whole_file

	lea	tags(pc),a0		; get this from mlsystem struct
	move.l	a0,ab_tags(a3)

	bsr.w	setprefs
	bsr	filter_monitor_id
	bsr	create_low_to_high_tabel

	move.l	ab_graphbase(a3),a6
	move.l	gb_ActiView(a6),ab_oldview(a3)

	move.l	ab_wviewblock2(a3),ab_viewblock_visible(a3)
	move.l	ab_wviewblock1(a3),ab_viewblock_hidden(a3)

	bsr.w	setsam
	bne.w	no_anim

	move.l	sigmask(pc),ab_waitmask(a3)
	move.l	#0,ab_frame_counter(a3)
	move.l	#0,ab_frame_pointer(a3)

	bsr	get_pan_info


	bsr	create_viewports

	tst.w	ab_audio_size(a3)
	bne	no_50h

	bsr	calc_speed	
	bsr.w	install_50h

	bra	no_s1
no_50h:
	bsr	init_sound

no_s1:
	tst.b	ab_diskanim(a3)
	beq	ww

ww1:
	cmp.b	#1,ab_double(a3)
	bne	no_double7

	bsr	get_next_cdxl_frame_disk
	bsr	change_view
	bra	double7
no_double7:

	bsr	change_view
	
	bsr	get_next_cdxl_frame_disk_chip

double7:
	tst.b	ab_quit(a3)
	bne	no_anim

	btst	#6,$bfe001
	bne.b	ww1
	bra	no_anim

ww:
	bsr	get_next_cdxl_frame
	bsr	change_view

	tst.b	ab_quit(a3)
	bne	no_anim

	btst	#6,$bfe001
	bne.b	ww

 no_anim:
	moveq	#0,d7
	bra.w	exit

input_error:
	rts
	ENDC

init_user_coplist3:
;	move.l	#0,ab_ucop_pointer(a3)
;	rts

	moveq	#ucl_SIZEOF,d0
	move.l	#MEMF_CLEAR,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)

	move.l	d0,ab_ucop_pointer(a3)
	beq.w	no_ucop_mem3

	moveq	#0,d0
	move.w	ab_leny(a3),d0
	add.w	d0,d0
	move.w	d0,d6
	add.w	#10,d0
	move.l	ab_ucop_pointer(a3),a0
	move.l	ab_graphbase(a3),a6
	jsr	_LVOUCopperListInit(a6)

	moveq	#0,d7
rep_ucop:
	move.l	d7,d0
	move.w	#LINEWAIT,d1
	move.l	ab_ucop_pointer(a3),a1
	jsr	_LVOCWait(a6)
	move.l	ab_ucop_pointer(a3),a1
	jsr	_LVOCBump(a6)

	move.l	#$108,d0
	move.w	ab_mod_back_even(a3),d1

	move.l	ab_ucop_pointer(a3),a1
	jsr	_LVOCMove(a6)
	move.l	ab_ucop_pointer(a3),a1
	jsr	_LVOCBump(a6)

	move.l	#$10a,d0
	move.w	ab_mod_back_odd(a3),d1

	move.l	ab_ucop_pointer(a3),a1
	jsr	_LVOCMove(a6)
	move.l	ab_ucop_pointer(a3),a1
	jsr	_LVOCBump(a6)

	addq.l	#1,d7
;	add.l	#50,d7
	move.l	d7,d0
	move.w	#LINEWAIT,d1
	move.l	ab_ucop_pointer(a3),a1
	jsr	_LVOCWait(a6)
	move.l	ab_ucop_pointer(a3),a1
	jsr	_LVOCBump(a6)

	move.l	#$108,d0
	move.w	ab_mod_normal_even(a3),d1
	move.l	ab_ucop_pointer(a3),a1
	jsr	_LVOCMove(a6)
	move.l	ab_ucop_pointer(a3),a1
	jsr	_LVOCBump(a6)

	move.l	#$10a,d0
	move.w	ab_mod_normal_odd(a3),d1
	move.l	ab_ucop_pointer(a3),a1
	jsr	_LVOCMove(a6)
	move.l	ab_ucop_pointer(a3),a1
	jsr	_LVOCBump(a6)
	addq.l	#1,d7

	cmp.w	d6,d7
	ble.w	rep_ucop

tijd1:
	move.l	#10000,d0
	move.l	#255,d1
	move.l	ab_ucop_pointer(a3),a1
	jsr	_LVOCWait(a6)

	move.l	ab_ucop_pointer(a3),a1
	jsr	_LVOCBump(a6)
no_ucop_mem3:
	rts


FREQ_NTSC = 3579545
FREQ_PAL = 3546895				; depends on pal or ntsc
CDXL_SPEED = 153600

init_sound:
	move.w	ab_audio_size(a3),d0
	beq.w	no_audio
	lsr.l	#1,d0			; words	
	move.w	d0,$dff0a4		; length
	move.w	d0,$dff0b4		; length

	move.l	ab_frame_hidden(a3),d0
	add.l	ab_audio_skip(a3),d0
	lea	startsample0(pc),a0
	move.l	d0,(a0)
	move.l	d0,$dff0a0		; audio data
	move.l	d0,$dff0b0		; audio data

	tst.b	ab_calc_cdtv(a3)
	bne.b	calc_cdtv2
	
	move.w	ab_audio_size(a3),d0		; user selected speed
	move.l	ab_frame_speed(a3),d1		; don't use cdtv speed
	addq.l	#1,d1
	bne.b	ok_fr
	moveq	#12,d1
ok_fr:
	mulu	d1,d0
	move.l	#FREQ_PAL,d1

	IFNE	XAPP
	move.l	ab_mlsysstruct(a3),a1
	move.l	ml_miscFlags(a1),d2
	btst	#0,d2
	bne	.pal
	move.l	#FREQ_NTSC,d1
.pal:
	ENDC

	divu	d0,d1
	move.w	d1,$dff0a6	
	move.w	d1,$dff0b6	
	bsr.w	calc_speed	
	bra.b	go_on_calc

calc_cdtv2:
	move.l	ab_pansize(a3),d0
	mulu	#23,d0
	move.w	ab_audio_size(a3),d1
	divu	d1,d0
	move.w	d0,$dff0a6
	move.w	d0,$dff0b6
go_on_calc:
	move.w	#64,$dff0a8
	move.w	#64,$dff0b8
	bsr.w	install_audio_0
	move.w	#$8203,$dff096
	move.w	#$8080,$dff09a
no_audio:
	rts

clear_sound:
	move.w   #$0080,$dff09a
	move.w   #$0003,$dff096	

	move.w	#$0,$dff0a8			; turn volume down ??
	move.w	#$0,$dff0b8

	tst.l	ab_old_audio_0(a3)
	beq.b	no_audio0
	bsr.w	remove_audio_0
	move.w	#$0,$dff0a8			; turn volume down ??
	move.w	#$0,$dff0b8
no_audio0:
	rts
ca:
	move.w   #$0003,$dff096	
	rts
	
get_next_cdxl_frame:

	move.l	ab_packed(a3),a0	; base cdxl file
	move.l	ab_frame_pointer(a3),d0
	add.l	d0,a0

	bsr.w	copy_cdxl

	move.l	ab_packed(a3),a1	; base cdxl file
	move.l	ab_frame_pointer(a3),d0
	add.l	d0,a1
	lea.l	pan_SIZEOF(a1),a1
	move.l	ab_viewblock_hidden(a3),a5
	lea	vi_viewport(a5),a0
	move.l	ab_num_cols(a3),d0
	move.l	ab_graphbase(a3),a6
	jsr	_LVOLoadRGB4(a6)

	move.l	ab_frame_pointer(a3),d1
	move.l	ab_pansize(a3),d0
	add.l	d0,d1
	cmp.l	ab_filesize(a3),d1
	blt.b	no_big_file
	moveq	#0,d1
	bsr.w	check_rots
no_big_file:
	move.l	d1,ab_frame_pointer(a3)
	rts

get_next_cdxl_frame_disk:
	move.l	ab_loadbuf1(a3),a0
	bsr.w	copy_cdxl
	move.l	ab_frame_pointer(a3),d1
	move.l	ab_pansize(a3),d0
	add.l	d0,d1
	cmp.l	ab_filesize(a3),d1
	blt.b	no_big_file2

 	move.l	ab_filehandle(a3),d1
	moveq	#0,d2
	moveq	#OFFSET_BEGINNING,d3
	move.l	ab_dosbase(a3),a6
	jsr	_LVOSeek(a6)
	moveq	#0,d1
	bsr.b	check_rots
no_big_file2:
	move.l	d1,ab_frame_pointer(a3)
	bsr.w	read_one_pan
	rts

*
* Load the next frame directly in the chip mem
*
get_next_cdxl_frame_disk_chip:
	move.l	ab_frame_pointer(a3),d1
	move.l	ab_pansize(a3),d0
	add.l	d0,d1
	cmp.l	ab_filesize(a3),d1
	blt.b	no_big_file3

	move.l	ab_filehandle(a3),d1
	moveq	#0,d2
	moveq	#OFFSET_BEGINNING,d3
	move.l	ab_dosbase(a3),a6
	jsr	_LVOSeek(a6)
	moveq	#0,d1
	bsr.b	check_rots
no_big_file3:
	move.l	d1,ab_frame_pointer(a3)

	bsr.w	read_one_pan_chip

	move.l	ab_frame_hidden(a3),a1		; base cdxl frame
	lea.l	pan_SIZEOF(a1),a1
	move.l	ab_viewblock_hidden(a3),a5
	lea	vi_viewport(a5),a0
	move.l	ab_num_cols(a3),d0
	move.l	ab_graphbase(a3),a6
	jsr	_LVOLoadRGB4(a6)
	rts

check_rots:
	cmp.l	#-1,ab_num_rot(a3)
	beq.w	no_big_file
	cmp.l	#1,ab_num_rot(a3)
	beq.b	zero1
	subq.l	#1,ab_num_rot(a3)
	bra.b	subit1
zero1:
	move.b	#$ff,ab_quit(a3)
subit1:
	rts

copy_cdxl:
	cmp.b	#1,ab_double(a3)
	beq.b	copy_cdxl_larger

	move.l	ab_frame_hidden(a3),a1
	move.l	ab_pansize(a3),d7
	lsr.l	#2,d7
rep_cop2:
	move.l	(a0)+,(a1)+
	subq.l	#1,d7
	bne.b	rep_cop2
	rts

copy_cdxl_larger:
	move.l	ab_frame_hidden(a3),a1

	move.l	ab_skip_size(a3),d7
	lsr.l	#1,d7
	subq.w	#1,d7
rep_cop21:
	move.w	(a0)+,(a1)+
	dbf	d7,rep_cop21
	
	lea	low_hi_tabel(pc),a2

	move.l	ab_low_hi(a3),d7
	bne.b	copy_larg_larger

	move.l	ab_image_size(a3),d7
	lsr.l	#1,d7
	subq.w	#1,d7
rep_cop22:
	moveq	#0,d6
	move.b	(a0)+,d6
	add.w	d6,d6
	move.w	0(a2,d6.w),(a1)+
	dbf	d7,rep_cop22

copy_audio:
	tst.w	ab_audio_size(a3)
	beq.b	no_audio2
	move.w	ab_audio_size(a3),d7
	lsr.w	#2,d7
	subq.w	#1,d7
rep_cop23:
	move.l	(a0)+,(a1)+
	dbf	d7,rep_cop23
no_audio2:
	rts

*
* Use a word lookup table ( should be faster )
*
copy_larg_larger:
	move.l	d7,a2
	move.l	ab_image_size(a3),d7
	lsr.l	#2,d7
	subq.w	#1,d7
rep_cop22_l:
	moveq	#0,d6
	move.w	(a0)+,d6
;	lsl.l	#2,d6
;	move.l	0(a2,d6.l*4),(a1)+
	dc.l	$22f26c00			; only >= 68020
	dbf	d7,rep_cop22_l
	bra.b	copy_audio

*
* maak een conversie tabel voor hires naar lowres
*
create_low_to_high_tabel:
	lea	low_hi_tabel(pc),a0
	move.w	#256,d0
	moveq	#0,d1
rep_create2:
	moveq	#0,d2

	btst	#0,d1
	beq.b	no_seth0
	or.w	#%11,d2
no_seth0:
	btst	#1,d1
	beq.b	no_seth1
	or.w	#%1100,d2
no_seth1:
	btst	#2,d1
	beq.b	no_seth2
	or.w	#%110000,d2
no_seth2:
	btst	#3,d1
	beq.b	no_seth3
	or.w	#%11000000,d2
no_seth3:

	btst	#4,d1
	beq.b	no_seth4
	or.w	#%1100000000,d2
no_seth4:
	btst	#5,d1
	beq.b	no_seth5
	or.w	#%110000000000,d2
no_seth5:
	btst	#6,d1
	beq.b	no_seth6
	or.w	#%11000000000000,d2
no_seth6:
	btst	#7,d1
	beq.b	no_seth7
	or.w	#%1100000000000000,d2
no_seth7:
	move.w	d2,(a0)+
	addq	#1,d1
	dbf	d0,rep_create2
	bsr.w	create_super_table
	rts

low_hi_tabel:
	ds.w	258

create_super_table:
	move.l	ab_low_hi(a3),d0
	beq.b	no_table

	lea	low_hi_tabel(pc),a1
	move.l	d0,a0
	moveq	#0,d0

super_table:		
	move.l	d0,d1
	and.w	#$ff00,d1
	lsr.w	#7,d1
	move.w	0(a1,d1.w),d2
	move.w	d2,(a0)+
;	move.w	#$f0f0,(a0)+
	move.l	d0,d1
	and.w	#$ff,d1
	add.w	d1,d1
	move.w	0(a1,d1.w),d2
	move.w	d2,(a0)+
;	move.w	#$f0f0,(a0)+

	addq.l	#1,d0
	cmp.l	#$10000,d0
	blt.b	super_table
no_table:
	rts


get_pan_info:
	move.b	#$ff,ab_nonleaved(a3)	; standaard non leaved
	move.l	ab_packed(a3),a0

	tst.b	ab_diskanim(a3)
	beq.b	no_disk1

	move.l	ab_loadbuf1(a3),a0
	cmp.b	#1,ab_double(a3)
	beq.b	no_disk1
	move.l	ab_frame_hidden(a3),a0
no_disk1:
	move.l	pan_Size(a0),ab_pansize(a3)
	move.w	pan_XSize(a0),ab_lenx(a3)
	cmp.b	#1,ab_double(a3)
	bne.b	no_BIGGER2
	move.w	pan_XSize(a0),d0

	cmp.w	#200,d0
	ble	.oke_b
	move.b	#2,ab_double(a3)		; switch to bigger vertical
	bra	no_BIGGER2
.oke_b:
	add.w	d0,pan_XSize(a0)
	add.w	d0,ab_lenx(a3)
no_BIGGER2:
	moveq	#0,d0
	move.w	pan_XSize(a0),d0
	add.l	#15,d0
	lsr.l	#4,d0
	lsl.l	#1,d0
	move.l	d0,ab_breedte_x(a3)		; word align it
	move.w	pan_YSize(a0),ab_leny(a3)
	move.b	pan_Type(a0),ab_ptype(a3)
	move.b	pan_Info(a0),ab_pinfo(a3)
	move.w	pan_AudioSize(a0),ab_audio_size(a3)
	moveq	#0,d0
	move.b	pan_PixelSize(a0),d0
	move.w	d0,ab_planes(a3)

	move.l	ab_breedte_x(a3),d1
	mulu	d0,d1
	mulu	ab_leny(a3),d1
	move.l	d1,ab_image_size(a3)

	moveq	#0,d0
	move.w	pan_ColorMapSize(a0),d0
	lsr.w	#1,d0
	move.l	d0,ab_num_cols(a3)
	move.w	pan_ColorMapSize(a0),d0
	add.l	#pan_SIZEOF,d0
	move.l	d0,ab_skip_size(a3)
	add.l	d1,d0
	move.l	d0,ab_audio_skip(a3)		; where is the audio

	move.b	pan_Info(a0),d0
	and.b	#PIV_MASK,d0
	beq	no_le
	cmp.b	#PIV_HAM,D0
	bgt.b	no_ham1
	move.w	#V_HAM,ab_mode(a3)
no_ham1:
	cmp.b	#PIV_AVM,d0
	bne.b	no_dctv
	move.b	#$ff,ab_hires(a3)
	move.w	#V_HIRES,ab_mode(a3)
no_dctv:
	move.b	pan_Info(a0),d0
	and.b	#PIF_LINES,d0
	beq	no_le
	move.b	#0,ab_nonleaved(a3)
no_le:
	rts

*
* The OS does some mighty strange things with the modulo values
* so find out what the system wants by just trying
*
get_difmodulo:
	move.b	ab_double(a3),d7
	move.l	d7,-(a7)
	move.l	ab_frame_hidden(a3),a0
	lea.l	pan_SIZEOF(a0),a0
	move.l	a0,ab_col_pointer(a3)
	move.l	ab_viewblock_hidden(a3),a5		; change hidden
	bsr.w	maak_viewport
	lea	vi_view(a5),a0
	bsr.b	get_modulos
	move.l	(a7)+,d7
	move.b	d7,ab_double(a3)
	rts
*
* Get from de copperlist the modulo value
* a0 points to the view
*
get_modulos:
	cmp.l		#0,a0
	beq.b		exit_putcopy
	move.l		v_LOFCprList(a0),a0
	cmp.l		#0,a0
	beq.b		exit_putcopy
	move.w		crl_MaxCount(a0),d2		; aantal elementen
	move.l		crl_start(a0),a0		; eigenlijke copperlist
	subq.l		#1,d2
mrep_putcop:
	move.w		(a0)+,d0
	cmp.w		#$ffff,d0
	beq.b		exit_putcopy

	cmp.w		#$108,d0
	bne.b		no_putmod8
	move.w		(a0),ab_mod_normal_even(a3)
;	bra.b		exit_getmodulo
no_putmod8:
	cmp.w		#$10a,d0
	bne.b		no_putmoda
	move.w		(a0),ab_mod_normal_odd(a3)
	bra.b		exit_getmodulo
no_putmoda:	
	addq.l		#2,a0
	dbf		d2,mrep_putcop
exit_getmodulo:
	move.w		ab_mod_normal_even(a3),d0
	move.w		ab_mod_normal_odd(a3),d0

;	move.w		#8,ab_mod_normal_odd(a3)

	move.l		ab_breedte_x(a3),d1
	neg.w		d1
	add.w		ab_mod_normal_even(a3),d1

	move.l		ab_breedte_x(a3),d0
	tst.b		ab_dbl(a3)
	beq	.noad
	add.w		d0,d1
.noad
	move.w		d1,ab_mod_back_even(a3)

;	move.l		ab_breedte_x(a3),d1
;	neg.w		d1
;	add.w		ab_mod_normal_odd(a3),d1
;	move.l		ab_breedte_x(a3),d0
;	add.w		d0,d1
;	add.w		#8,d1

	move.w		d1,ab_mod_back_odd(a3)

exit_putcopy:
	rts
	
create_viewports:
	tst.b	ab_double(a3)
	beq.b	no_double6
	bsr.w	get_difmodulo
	bsr.w	init_user_coplist3
	move.l	ab_loadbuf1(a3),a0
	tst.b	ab_diskanim(a3)
	bne.b	double6
no_double6:
	move.l	ab_frame_hidden(a3),a0
double6:
	lea.l	pan_SIZEOF(a0),a0
	move.l	a0,ab_col_pointer(a3)

	move.l	ab_viewblock_hidden(a3),a5		; change hidden

	move.l	ab_frame_hidden(a3),ab_w_bitpointer(a3)
	move.l	ab_skip_size(a3),d0
	add.l	d0,ab_w_bitpointer(a3)
	bsr.w	maak_viewport			; create new non leaved view

	move.l	ab_viewblock_visible(a3),a5		; change hidden

	move.l	ab_frame_show(a3),ab_w_bitpointer(a3)
	move.l	ab_skip_size(a3),d0
	add.l	d0,ab_w_bitpointer(a3)
	bsr.w	maak_viewport			; create new non leaved view
	rts

no_disk2:
	lea.l	pan_SIZEOF(a0),a0
	move.l	a0,ab_col_pointer(a3)
	move.l	ab_viewblock_hidden(a3),a5		; change hidden
	move.l	ab_frame_hidden(a3),ab_w_bitpointer(a3)
	bsr.w	maak_viewport			; create new non leaved view
	move.l	ab_viewblock_visible(a3),a5		; change visible
	move.l	ab_frame_show(a3),ab_w_bitpointer(a3)
	bsr.w	maak_viewport			; create new non leaved view
	rts

*
* Convert from non-interleaved to interleaved
*
convert_non_int:
	tst.b	ab_double(a3)
	bne.w	convert_non_int_double

	movem.l	d0-d7/a0-a6,-(a7)

	tst.b	ab_nonleaved(a3)
	beq	copy_non_leaved

	move.b	#$0,ab_nonleaved(a3)	; set the nonleaved byte

	move.w	ab_planes(a3),d7
	cmp.w	#1,d7
	beq.w	no_convert

	move.l	ab_breedte_x(a3),d6
	move.l	d6,d5
	mulu	d7,d6			; total width in bytes of
					; interleaved line
	move.l	ab_frame_hidden(a3),a0	; data to
	move.l	ab_frame_show(a3),a1	; data from
	add.l	ab_skip_size(a3),a1

	move.l	a0,a5			; temporary from
	move.l	a0,a6			; temporary from

	move.w	ab_planes(a3),d4	; width picture
	subq.w	#1,d4
rep_h2:

	move.l	a6,a5
	move.w	ab_leny(a3),d7		; height picture
	subq.w	#1,d7
rep_pl2:
	move.l	ab_breedte_x(a3),d5	; no planes
	move.l	a5,a0
	subq.w	#1,d5
rep_b2:
	move.b	(a1)+,(a0)+
	dbf	d5,rep_b2		; copy one line

	add.l	d6,a5			; goto next line
	
	dbf	d7,rep_pl2

	add.l	ab_breedte_x(a3),a6	; goto next plane
	dbf	d4,rep_h2

create_converted:

	move.l	ab_frame_show(a3),a0
	lea.l	pan_SIZEOF(a0),a0
	move.l	a0,ab_col_pointer(a3)

	move.l	ab_sysframe(a3),a5
	lea	sbu_Display+dp_View(a5),a1
	lea	sbu_Display+dp_ViewPort(a5),a0

	bsr.w	free_view_port			; free old lists etc.

	move.l	ab_frame_hidden(a3),ab_w_bitpointer(a3)

	bsr.w	maak_viewport_global		; create new non leaved view

	move.l	ab_graphbase(a3),a6
	move.l	ab_viewblock_visible(a3),a1
	lea	vi_viewport(a1),a0
	jsr	_LVOWaitBOVP(a6)

	move.l	ab_sysframe(a3),a5
	lea	sbu_Display+dp_View(a5),a1
	jsr	_LVOLoadView(a6)

	tst.b	ab_interlace(a3)
	beq.b	no_convert
	move.l	ab_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)

no_convert:
	movem.l	(a7)+,d0-d7/a0-a6

no_conv_needed1:
	rts

copy_non_leaved:

	move.w	ab_planes(a3),d7
	cmp.w	#1,d7
	beq.w	no_convert

	move.l	ab_breedte_x(a3),d6
	move.l	d6,d5
	mulu	d7,d6			; total width in bytes of
					; interleaved line
	move.l	ab_frame_hidden(a3),a0	; data to
	move.l	ab_frame_show(a3),a1	; data from
	add.l	ab_skip_size(a3),a1

	move.w	ab_planes(a3),d4	; width picture
	subq.w	#1,d4
.rep_h2:
	move.w	ab_leny(a3),d7		; height picture
	subq.w	#1,d7
.rep_pl2:
	move.l	ab_breedte_x(a3),d5	; no planes
	subq.w	#1,d5
.rep_b2:
	move.b	(a1)+,(a0)+
	dbf	d5,.rep_b2		; copy one line
	dbf	d7,.rep_pl2
	dbf	d4,.rep_h2
	bra	create_converted
	
convert_non_int_double:
	movem.l	d0-d7/a0-a6,-(a7)

	clr.l	ab_ucop_pointer(a3)	; we don't want that now

	move.w	ab_planes(a3),d7

	move.l	ab_breedte_x(a3),d6
	move.l	d6,d5
	mulu	d7,d6			; total width in bytes of
					; interleaved line
	move.l	ab_frame_hidden(a3),a0	; data to
	move.l	ab_frame_show(a3),a1	; data from
	add.l	ab_skip_size(a3),a1

	move.l	a0,a5			; temporary from
	move.l	a0,a6			; temporary from

	move.w	ab_planes(a3),d4	; no planes
	subq.w	#1,d4
drep_h2:

	move.l	a6,a5
	move.w	ab_leny(a3),d7		; height picture
	subq.w	#1,d7
drep_pl2:
	move.l	ab_breedte_x(a3),d5	; width picture
	move.l	a5,a0
	move.l	a0,a2
	add.l	d6,a2
	subq.w	#1,d5
drep_b2:
	move.b	(a1),(a0)+
	move.b	(a1)+,(a2)+
	dbf	d5,drep_b2		; copy one line
	add.l	d6,a5			; goto next line
	add.l	d6,a5
	dbf	d7,drep_pl2
	add.l	ab_breedte_x(a3),a6	; goto next plane
	dbf	d4,drep_h2

	move.b	#$0,ab_nonleaved(a3)	; set the nonleaved byte
	bra.w	create_converted

	IFNE	XAPP

	XDEF	_get_varsize
_get_varsize:
	move.l	#ab_SIZEOF,d0
	rts
	ENDC

filter_monitor_id:
	move.b	#0,ab_dbl(a3)
	move.w	#0,ab_alternate_lowres(a3)
	move.l	ab_monid(a3),d1
	and.l	#MONITOR_ID_MASK,d1
	move.l	d1,ab_monid(a3)
	cmp.l	#SUPER72_MONITOR_ID,d1
	bne	.no72
	or.l	#SUPER_KEY,ab_monid(a3)
	move.w	#HIRES_KEY,ab_alternate_lowres(a3)
.no72:
	cmp.l	#EURO72_MONITOR_ID,d1
	bne	.noe72
	or.l	#SUPER_KEY,ab_monid(a3)
	move.w	#HIRES_KEY,ab_alternate_lowres(a3)
	move.b	#1,ab_dbl(a3)
.noe72:
	cmp.l	#VGA_MONITOR_ID,d1
	bne	.nov72
	or.l	#SUPER_KEY,ab_monid(a3)
	move.w	#HIRES_KEY,ab_alternate_lowres(a3)
	move.b	#1,ab_dbl(a3)
.nov72:
	cmp.l	#DBLPAL_MONITOR_ID,d1
	bne	.nodbl
	move.b	#1,ab_dbl(a3)
.nodbl:
	cmp.l	#DBLNTSC_MONITOR_ID,d1
	bne	.nodbl2
	move.b	#1,ab_dbl(a3)
.nodbl2:

	rts

	IFNE	XAPP

	XDEF	_pass_mlsystem
*
* haal de info uit de mlsystem struct
*
_pass_mlsystem:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	12(a5),a3
	move.l	a7,ab_easy_exit(a3)

	move.b	#$0,ab_loaded(a3)
	move.b	#$0,ab_hires(a3)
	move.b	#$0,ab_interlace(a3)
	move.w	#0,ab_mode(a3)

	move.l	8(a5),a0			; pointer naar mlsystem
	move.l	a0,ab_mlsysstruct(a3)
	move.l  ml_monitor_ID(a0),ab_monid(a3)
	move.l	ml_taglist(a0),ab_tags(a3)

	move.l	16(a5),a1

	move.l	a1,ab_msgpointer(a3)
	moveq	#0,d0
	move.b	MP_SIGBIT(a1),d0
	moveq	#1,d1
	lsl.l	d0,d1
	move.l	d1,ab_sig_ptoc(a3)

	bsr.w	openlibs

	bsr.w	setprefs
	bsr	filter_monitor_id

	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

	
	XDEF	_load_cdxl

_load_cdxl:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	12(a5),a3
	move.l	a7,ab_easy_exit(a3)

	move.b	#0,ab_double(a3)

	tst.b	ab_loaded(a3)		; file loaded ??
	bne.w	already_loaded

	move.l	8(a5),a0
	move.l	a0,ab_filenaam(a3)

	moveq	#0,d0			; filename
	move.l	16(a5),a1		; pointer to numargs

	move.b	#0,ab_sigmode(a3)
	move.w	0(a1),d0		; precize normal mode
	cmp.w	#2,d0
	bne.b	no_precize
	move.b	#1,ab_sigmode(a3)
no_precize:
	move.w	12(a1),d0		; x pos blitin
	move.w	d0,ab_dx(a3)
	move.w	14(a1),d0		; y pos blitin
	move.w	d0,ab_dy(a3)
	move.w	18(a1),d0		; CDXL speed override
	move.l	d0,ab_overallspeed(a3)
	move.w	20(a1),d0		; number of rotations
	move.l	d0,ab_num_rot(a3)
	move.l	d0,ab_rot_store(a3)

	move.w	22(a1),d0		; anim control bits

	btst	#0,d0
	beq.b	no_cd_calc
	move.b	#1,ab_calc_cdtv(a3)
no_cd_calc:
	btst	#2,d0
	beq.b	no_double10

	move.l	d0,-(a7)
	move.b	#$1,ab_double(a3)
	moveq	#MEMF_PUBLIC,d1
	move.l	$4.w,a6
	move.l	#TABLE_SIZE,d0
	jsr	_LVOAllocMem(a6)
	move.l	d0,ab_low_hi(a3)
	bsr.w	create_low_to_high_tabel
	move.l	(a7)+,d0
no_double10:

	btst	#1,d0
	beq.b	no_diskanim3
	move.b	#1,ab_diskanim(a3)

	move.b	#$ff,ab_loaded(a3)	; file loaded
	bra.b	already_loaded

no_diskanim3:

	bsr.w	read_whole_file

	move.b	#$ff,ab_loaded(a3)	; file loaded

	tst.b	ab_diskanim(a3)		; forced play from disk
	bne.b	already_loaded

	bsr.w	get_pan_info

already_loaded:

	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0			; succes
	rts

	XDEF	_do_cdxl

_do_cdxl:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a3

	move.l	a7,ab_easy_exit(a3)
	move.b	#0,ab_quit(a3)

	move.l	ab_rot_store(a3),ab_num_rot(a3)

	tst.b	ab_loaded(a3)
	beq.w	not_loaded

	move.l	#0,ab_frame_counter(a3)
	move.l	#0,ab_frame_pointer(a3)

	move.l	ab_mlsysstruct(a3),a0
	tst.w	sbu_Viewed(a0)
	bne.b	active_first

	move.l	sbu_Base(a0),ab_frame_hidden(a3)
	lea.l	sbu_SIZEOF(a0),a0
	move.l	sbu_Base(a0),ab_frame_show(a3)
	bra.b	inactive_first

active_first:

	move.l	sbu_Base(a0),ab_frame_show(a3)
	lea.l	sbu_SIZEOF(a0),a0
	move.l	sbu_Base(a0),ab_frame_hidden(a3)

inactive_first:

	lea	ab_view1(a3),a0
	move.l	a0,ab_wviewblock1(a3)
	lea	ab_view2(a3),a0
	move.l	a0,ab_wviewblock2(a3)

	move.l	ab_wviewblock2(a3),ab_viewblock_visible(a3)
	move.l	ab_wviewblock1(a3),ab_viewblock_hidden(a3)

	bsr	set_lace

	tst.b	ab_diskanim(a3)
	beq.b	mem_cdxl2

	bsr.w	read_whole_file			; partial load 

	bsr.w	get_pan_info			; retrieve info

mem_cdxl2:

	bsr.w	setsam
	tst.b	d0
	bne.w	not_loaded

	move.l	sigmask(pc),ab_waitmask(a3)

	bsr.w	create_viewports

	move.l	ab_overallspeed(a3),ab_frame_speed(a3)

	tst.w	ab_audio_size(a3)
	bne.b	no_50h

	lea	tellerstart(pc),a0
	move.l	ab_overallspeed(a3),(a0)

	bsr.w	calc_speed	
	bsr.w	install_50h


	bra.b	no_s1

no_50h:
	bsr.w	init_sound

no_s1:
	move.l	#0,ab_frame_counter(a3)
	move.l	#0,ab_frame_pointer(a3)

	tst.b	ab_diskanim(a3)
	beq.b	mem_cdxl1
rep_disk_cdxl:
	cmp.b	#1,ab_double(a3)
	bne.b	no_double72
	bsr.w	get_next_cdxl_frame_disk
	bsr.w	change_view
	bra.b	double72
no_double72:

	bsr.w	change_view
	tst.b	ab_quit(a3)
	bne.b	quit_cdxl
	
	bsr.w	get_next_cdxl_frame_disk_chip

double72:
	tst.b	ab_quit(a3)
	bne.b	quit_cdxl

	bra.b	rep_disk_cdxl

mem_cdxl1:

	bsr.w	get_next_cdxl_frame

	bsr.w	change_view

	tst.b	ab_quit(a3)
	bne.w	quit_cdxl

	bra.b	mem_cdxl1

quit_cdxl:

	bsr.w	clear_sound

	bsr.w	remove_50h
	bsr.w	freesam

	bsr.b	copy_local_view
	
	bsr.b	set_viewed

	move.l	ab_graphbase(a3),a6
	jsr	_LVOWaitTOF(a6)

not_loaded:
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

copy_local_view:
	move.l	ab_mlsysstruct(a3),a0
	move.l	ab_frame_hidden(a3),d0

	cmp.l	sbu_Base(a0),d0
	beq.b	inactive_first2
active_first2:
	lea.l	sbu_SIZEOF(a0),a0
inactive_first2:

	move.l	a0,ab_sysframe(a3)

	bsr.w	convert_non_int

	rts

set_viewed:
	move.l	ab_mlsysstruct(a3),a0
	move.w	#0,sbu_Viewed(a0)		; clear all viewed
	lea.l	sbu_SIZEOF(a0),a0
	move.w	#0,sbu_Viewed(a0)
	lea.l	sbu_SIZEOF(a0),a0
	move.w	#0,sbu_Viewed(a0)
	move.l	ab_sysframe(a3),a0
	move.w	#1,sbu_Viewed(a0)
	rts

	XDEF	_release_cdxl

_release_cdxl
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a3
	move.l	ab_monitorspec(a3),d0
	beq	.nomon
	move.l	d0,a0
	move.l	ab_graphbase(a3),a6
	jsr	_LVOCloseMonitor(a6)
.nomon:

	bsr.w	close_libs
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

	XDEF	_unload_cdxl
_unload_cdxl:
	link	a5,#0
	movem.l	d0-d7/a0-a6,-(a7)
	move.l	8(a5),a3

	tst.l	ab_filehandle(a3)
	beq.b	no_fopen
	bsr.w	close_file

no_fopen:
	tst.b	ab_diskanim(a3)
	bne.b	exit112
	tst.l	ab_packed(a3)
	beq.s	exit112
	move.l	ab_packed(a3),a1
	move.l	ab_packed_size(a3),d0
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)
	clr.l	ab_packed(a3)
exit112:
	tst.l	ab_loadmem(a3)
	beq.b	exit113
	move.l	ab_loadmem(a3),a1
	move.l	ab_loadmem_size(a3),d0
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)
	clr.l	ab_loadmem(a3)
exit113:
	tst.l	ab_low_hi(a3)
	beq.s	exit114
	move.l	ab_low_hi(a3),a1
	move.l	#TABLE_SIZE,d0
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)
	clr.l	ab_low_hi(a3)
exit114:
	lea	ab_view1(a3),a5
	bsr.w	freememory

	lea	ab_view2(a3),a5
	lea     vi_viewport(a5),a1
	move.l  #0,vp_UCopIns(a1)		; don't free this twice

	bsr.w	freememory

	move.b	#0,ab_loaded(a3)
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	moveq	#0,d0
	rts

	ENDC

*
* Retrieve the overscan settings
*
get_oscan_vals:
	move.l	ab_intbase(a3),a6
	move.l	ab_monid(a3),d0
	move.l	d0,d1
	and.l	#MONITOR_ID_MASK,d1
	cmp.l	#SUPER72_MONITOR_ID,d1
	bne	.no_mul
	or.l	#SUPER_KEY,d0
	or.l	#SUPER_KEY,ab_monid(a3)
.no_mul:
	cmp.l	#VGA_MONITOR_ID,d1
	bne	.no_mul2
	or.l	#SUPER_KEY,d0
	or.l	#SUPER_KEY,ab_monid(a3)
.no_mul2:

	or.l	#HIRES_KEY,d0
	move.l	d0,a0
	move.l	#1,d0				//OSCAN_MAX
 	lea.l	ab_view1(a3),a1
	jsr	_LVOQueryOverscan(a6)
 	lea.l	ab_view1(a3),a1

	move.w	ra_MaxX(a1),d0
	move.w	ra_MaxY(a1),d1
	addq.w	#1,d0
	lsr.w	#1,d0
	lsr.w	#1,d1
	add.w	d0,d0
	add.w	d1,d1
	move.w	d0,ab_columns(a3)
	move.l	ab_monid(a3),d2
	and.w	#V_LACE,d2
	beq	.nol
	lsr.w	#1,d1
.nol:
	move.w	d1,ab_rows(a3)

	move.l	ab_monid(a3),d0
	or.l	#HIRES_KEY,d0
	move.l	d0,a0
	move.l	#4,d0				//OSCAN_MAX
 	lea.l	ab_view1(a3),a1
	jsr	_LVOQueryOverscan(a6)
 	lea.l	ab_view1(a3),a1

	move.w	ra_MaxY(a1),d0
	sub.w	ra_MinY(a1),d0
	add.w	d0,d0
	move.l	ab_monid(a3),d2
	and.w	#V_LACE,d2
	beq	.nol2
	lsr.w	#1,d0
.nol2:

	move.w	d0,ab_max_y(a3)

	move.w	ra_MaxX(a1),d0
	sub.w	ra_MinX(a1),d0
	addq.l	#7,d0
	lsr.l	#3,d0
	lsl.l	#3,d0
	move.w	d0,ab_max_x(a3)

	move.w	#0,a0
 	lea.l	ab_view1(a3),a1
	moveq	#mtr_SIZEOF,d0
	move.l	#DTAG_MNTR,d1
	move.l	ab_monid(a3),d2
	move.l	ab_graphbase(a3),a6
	jsr	_LVOGetDisplayInfoData(a6)
 	lea.l	ab_view1(a3),a1

	move.w	mtr_ViewPositionRange+2(a1),d0		; get minimal y pos
	move.w	mtr_ViewPosition+2(a1),d1		; get actual y pos
	move.w	d0,d2
	addq.w	#4,d0					; diff < 4 ?
	cmp.w	d0,d1
	bge	.minimum_oke
	move.l	d2,d1
	addq.w	#4,d1
.minimum_oke:
	move.w	mtr_ViewPosition(a1),ab_viewx(a3)
	move.w	d1,ab_viewy(a3)

	and.w	#$fffe,ab_viewx(a3)
	and.w	#$fffe,ab_viewy(a3)

	clr.b	ab_Xoff(a3)
	clr.b	ab_Yoff(a3)
	rts

set_lace:
	IFNE XAPP
	move.w	#0,ab_or_vmode_mask(a3)
	move.l	ab_mlsysstruct(a3),a0
	move.l	ml_miscFlags(a0),d0
	btst	#4,d0
	beq	.nolace
	ENDC
	move.w	#V_LACE,ab_or_vmode_mask(a3)	; set depending on LACE on/off
.nolace:
	rts

*
* Set depending on monitor the LACE bits
* Data should be somewhere in the mlsystem structure
*
* For test purpose also open the monitor here
*
set_and_or_vmode:
	move.l	ab_graphbase(a3),a6
	move.l	ab_monid(a3),d0
	move.w	#0,a1
	jsr	_LVOOpenMonitor(a6)
	move.l	d0,ab_monitorspec(a3)

	bsr	set_lace

	move.w	#0,ab_view_intoff(a3)

	move.l	ab_monid(a3),d0
	move.w	#~0,ab_and_vmode_mask(a3)
	move.l	d0,d1
	and.l	#DBLPAL_MONITOR_ID,d1		; what about super72 ?????
	cmp.l	#DBLPAL_MONITOR_ID,d1
	beq	.set
	move.l	d0,d1
	and.l	#DBLNTSC_MONITOR_ID,d1
	cmp.l	#DBLNTSC_MONITOR_ID,d1
	beq	.set
;	move.w	#~V_LACE,ab_and_vmode_mask(a3)
	rts
.set:	
	move.w	#1,ab_view_intoff(a3)
	move.w	#~V_LACE,ab_and_vmode_mask(a3)
	rts

setprefs:

	bsr	set_and_or_vmode
	bsr	get_oscan_vals

.y72:
	move.l	ab_graphbase(a3),a6
	move.w	LIB_VERSION(a6),d0
	move.b	d0,ab_libversion(a3)
	cmp.w	#LIBV39,d0
	blt.b	.no_v39_1
	move.b	#$ff,ab_v39_present(a3)
.no_v39_1:
	cmp.w	#LIBV36,d0
	blt.b	.no_v36				; er is geen chiprevbits

	move.b	gb_ChipRevBits0(a6),d0
	and.b	#SETCHIPREV_AA,d0
	cmp.b	#SETCHIPREV_AA,d0
	bne.b	.no_aa1
	move.b	#$ff,ab_aa_present(a3)
.no_aa1:

.no_v36:
	move.l	$4,a6
	move.w	AttnFlags(a6),d0
	and.l	#AFF_68020,d0
	beq.b	.no_cpu1
	move.b	#1,ab_more_cpu(a3)
.no_cpu1:
clear_view_mem:
	move.l	#220,d0
	lea	ab_view1(a3),a0
	lsr.l	#2,d0
	subq.w	#1,d0
.clr_view:
	move.l	#0,(a0)+
	dbf	d0,.clr_view
	rts

ACUR = 8
*
* Calculate the speed so that speeds of non jiffy size are possible
*
calc_speed:
	movem.l	a0/d0/d1,-(a7)
	move.l	ab_frame_speed(a3),d1
	addq.l	#1,d1
	moveq	#50,d0				; depends on pal or ntsc
	IFNE	XAPP
	move.l	ab_mlsysstruct(a3),a0
	move.l	ml_miscFlags(a0),d2
	btst	#0,d2
	bne	.pal
	moveq	#60,d0
.pal:
	ENDC
	tst.l	d1
	bne.b	oke_non_zero
	moveq	#12,d1
oke_non_zero:
	lsl.l	#ACUR,d0
	divu	d1,d0
	and.l	#$ffff,d0
set_wait:
	lea	teller(pc),a0
	move.l	d0,(a0)
	lea	tellerstart(pc),a0
	move.l	d0,(a0)
	movem.l	(a7)+,d0/d1/a0
	rts

setsam:
	lea	sigmask(pc),a0
	tst.l	(a0)
	bne.w	no_install

	move.l	$4.w,a6
	moveq	#-1,d0
	jsr	_LVOAllocSignal(a6)	

	cmp.l	#-1,d0
	beq.b	setsam_error
	lea	signum(pc),a0
	move.l	d0,(a0)
	move.l	d0,d1
	moveq	#$1,d0
	lsl.l	d1,d0	
	lea	sigmask(pc),a0
	move.l	d0,(a0)
		
	sub.l	a1,a1
	move.l	$4.w,a6
	jsr	_LVOFindTask(a6)
	lea	task(pc),a0
	move.l	d0,(a0)
	moveq	#0,d0
	rts

setsam_error:
	rts

freesam:
	move.l	signum(pc),d0
	cmp.l	#-1,d0
	beq.b	nofreesam
	move.l	$4.w,a6
	jsr	_LVOFreeSignal(a6)
nofreesam:
	lea	signum(pc),a0
	clr.l	(a0)
	lea	sigmask(pc),a0
	clr.l	(a0)
	rts

stuur_signaal:
	movem.l	d0/a1,-(a7)

	move.l	$4.w,a6
	move.l	task(pc),a1
	move.l	sigmask(pc),d0
	jsr	_LVOSignal(a6)
	movem.l	(a7)+,d0/a1
	rts

install_50h:
	lea	handle_installed(pc),a0
	tst.b	(a0)
	bne.b	no_install

	lea.l	intstruct50h(pc),a1

	lea	proc50hnaam(pc),a0
	move.l	a0,10(a1)

	lea	teller(pc),a0
	move.l	a0,14(a1)
	lea	proc_50h(pc),a0
	move.l	a0,18(a1)

	moveq	#5,d0
	move.l	$4.w,a6
	jsr	_LVOAddIntServer(a6)
	lea	handle_installed(pc),a0
	move.b	#$ff,(a0)
no_install:
	rts

remove_50h:
	lea	handle_installed(pc),a0
	tst.b	(a0)
	beq.b	no_remove
	lea.l	intstruct50h(pc),a1
	moveq	#5,d0
	move.l	$4.w,a6
	jsr	_LVORemIntServer(a6)
	lea	handle_installed(pc),a0
	move.b	#$00,(a0)
no_remove:
	rts

proc_50h:
	move.l	a0,-(a7)
	moveq	#1,d0
	lsl.l	#ACUR,d0
	sub.l	d0,off_teller(a1)
	addq.l	#2,off_timer(a1)
	move.l	off_teller(a1),d1
	lsr.l	#ACUR,d1
	tst.l	d1
	bne.b	no_send
	
	bsr.w	stuur_signaal
	move.l	off_tel_start(a1),d0
	add.l	d0,off_teller(a1)
no_send:
	move.l	(a7)+,a0
	moveq	#0,d0
	rts

proc50hnaam:	dc.b	"50Hz interupt",0
	even
		
intstruct50h:
	dc.l	0,0
	dc.b	2,15	; type en pri
	dc.l	0,0	; pointer naar naam en data
	dc.l	0

teller:		dc.l	0
tellerstart:	dc.l	0
task:		dc.l	0
signum:		dc.l	-1
sigmask:	dc.l	0
timer:		dc.l	0
handle_installed:	dc.b	0,0

off_teller = 0
off_tel_start = 4
off_task = 8
off_signum = 12
off_sigmask = 16
off_timer = 20


own_wait:
	movem.l	d0/d1/d6/a0/a1,-(a7)

wait_for_50:
	move.l	ab_waitmask(a3),d0
	or.l	ab_sig_ptoc(a3),d0
no_sigptoc:

	move.l	$4.w,a6
	jsr	_LVOWait(a6)

	movem.l	d0/d1/a0/a1,-(a7)
	move.l	$4.w,a6
	jsr	_LVOForbid(a6)
	movem.l	(a7)+,d0/d1/a0/a1

	move.l	d0,d1
	move.l	d0,d6
	and.l	ab_sig_ptoc(a3),d1
	beq.b	no_wacht1

; Er is een message van boven kijk wat zij doet en handel ernaar

search_list:
	move.l	ab_msgpointer(a3),a0		;get msgport ptr
	lea.l	MP_MSGLIST(a0),a1		;make ptr to msglist
	move.l	LH_HEAD(a1),d1			;get first 	

scan:	move.l	d1,a1
	move.l	LH_HEAD(a1),d1
	beq.b	end_of_list

	move.l	pd_Cmd(a1),d2	; pd command
	cmp.l	#DCC_DOTERM,d2
	beq.b	doterm			; let de C-prog handle the reply
	cmp.l	#DCC_DOSTOP,d2
	beq.b	doterm

	bra.b	scan

doterm:
	movem.l	d0/d1/a0/a1,-(a7)
	sub.l	a1,a1
	move.l	$4.w,a6
	jsr	_LVOFindTask(a6)
	move.l	d0,a1
	move.l	ab_sig_ptoc(a3),d0
	jsr	_LVOSignal(a6)		; restore signal
	movem.l	(a7)+,d0/d1/a0/a1

	move.b	#$ff,ab_quit(a3)
	move.l	$4.w,a6
	jsr	_LVOPermit(a6)

	movem.l	(a7)+,d0/d1/d6/a0/a1
	moveq	#-1,d0
	rts

no_wacht1:

end_of_list:
	move.l	$4.w,a6
	jsr	_LVOPermit(a6)

	and.l	ab_waitmask(a3),d6	; only when a 50hz signal is given
	beq	wait_for_50		; otherwise wait on

	bsr	check_key
	
	movem.l	(a7)+,d0/d1/d6/a0/a1
	moveq	#0,d0
	rts

check_key:
	move.b	$bfec01,d0
	not	d0
	ror.b	#1,d0
	cmp.b	#$60,d0
	bhi	no_key_ch1

	move.l	ab_dosbase(a3),a6
	movem.l	d1/a0/a1,-(a7)
	moveq	#1,d1
	jsr	_LVODelay(a6)		; give system time to check keys
	movem.l	(a7)+,d1/a1/a0
no_key_ch1
	moveq	#0,d0
	rts

change_view:
	addq.l		#1,ab_frames(a3)	; frames played

	bsr.w		own_wait

	tst.w		d0
	bne.b		endit

change_view2:
	move.l		ab_graphbase(a3),a6
	move.l		ab_viewblock_visible(a3),a1
	lea		vi_viewport(a1),a0
	jsr		_LVOWaitBOVP(a6)

	move.l		ab_graphbase(a3),a6
	move.l		ab_viewblock_hidden(a3),a1
	jsr		_LVOLoadView(a6)

	tst.b		ab_interlace(a3)
	beq.b		no_wait
	move.l		ab_graphbase(a3),a6
	jsr		_LVOWaitTOF(a6)
no_wait:

	move.l		ab_viewblock_visible(a3),d0
	move.l		ab_viewblock_hidden(a3),ab_viewblock_visible(a3)
	move.l		d0,ab_viewblock_hidden(a3)

	move.l		ab_frame_show(a3),d0
	move.l		ab_frame_hidden(a3),ab_frame_show(a3)
	move.l		d0,ab_frame_hidden(a3)

	move.l		ab_frame_show(a3),d0
	add.l		ab_audio_skip(a3),d0
	lea		startsample0(pc),a0
	move.l		d0,(a0)

	moveq		#0,d0
endit:
	rts
*
* set the bitplane data with a0 pointer to bitmap
*
set_planes:
	move.l		ab_w_bitpointer(a3),d0
	move.l		ab_breedte_x(a3),d1
	tst.b		ab_nonleaved(a3)
	beq.b		no_modu2
	mulu		ab_leny(a3),d1
no_modu2:
	move.l		d0,bm_Planes(a0)		; zet de bitplanes
	add.l		d1,d0
	move.l		d0,bm_Planes+4(a0)
	add.l		d1,d0
	move.l		d0,bm_Planes+8(a0)
	add.l		d1,d0
	move.l		d0,bm_Planes+12(a0)
	add.l		d1,d0
	move.l		d0,bm_Planes+16(a0)
	add.l		d1,d0
	move.l		d0,bm_Planes+20(a0)
	add.l		d1,d0
	move.l		d0,bm_Planes+24(a0)
	add.l		d1,d0
	move.l		d0,bm_Planes+28(a0)
	rts

*
* Set dx dy width and height of the viewport pointed at by a0
*
set_xy_viewport:
	move.w		ab_lenx(a3),vp_DWidth(a0)
	move.w		ab_leny(a3),d0
	tst.b		ab_double(a3)
	beq.b		.no_double4
	add.w		d0,d0
.no_double4:
	move.w		d0,vp_DHeight(a0)
	move.w		ab_mode(a3),vp_Modes(a0)
	clr.l		vp_Next(a0)

	move.w		ab_columns(a3),d1
	tst.b		ab_hires(a3)
	bne.b		.hi1
	lsr.w		#1,d1
.hi1:
	move.l		ab_breedte_x(a3),d0
	lsl.l		#3,d0
	cmp.w		ab_max_x(a3),d0
	ble		.oke1
	move.w		ab_max_x(a3),d0
.oke1:
	sub.w		d1,d0
	asr.w		#1,d0
	neg.w		d0
	move.w		d0,vp_DxOffset(a0)
	add.w		ab_dx(a3),d0
	move.w		d0,vp_DxOffset(a0)

	bsr		get_dy_offset
	move.w		d0,vp_DyOffset(a0)	
	rts

get_dy_offset:
	move.w	ab_rows(a3),d1
	move.w	ab_leny(a3),d0
	tst.b   ab_double(a3)
	beq.b   .no_double5
	add.w   d0,d0
.no_double5:

	lsl.w	#1,d1
	lsl.w	#1,d0

	tst.b	ab_interlace(a3)
	beq.w	.no_inter1
	lsr.w	#1,d0			; was already lace
.no_inter1:
	and.w	#$fffe,d0

	sub.w	d1,d0
	neg.w	d0

	asr.w	#1,d0

	tst.w	d0
	bge	.okie
	moveq	#0,d0
.okie:
	tst.b	ab_interlace(a3)
	bne.s	.inter5
	asr.w	#1,d0
	rts

.inter5:
	and.w	#$fffe,d0
	rts

maak_viewport:

	bsr		freememory

	move.l		ab_graphbase(a3),a6
	lea.l		vi_view(a5),a1
	jsr		_LVOInitView(a6)

	lea.l		vi_view(a5),a1
	move.w		ab_viewx(a3),v_DxOffset(a1)
	move.w		ab_viewy(a3),v_DyOffset(a1)

	lea.l		vi_view(a5),a1
	move.w		ab_mode(a3),d0
	or.w		ab_or_vmode_mask(a3),d0
	and.w		ab_and_vmode_mask(a3),d0
	move.w		d0,v_Modes(a1)
	lea.l		vi_viewport(a5),a0
	jsr		_LVOInitVPort(a6)

	lea.l		vi_view(a5),a0
	lea.l		vi_viewport(a5),a1
	move.l		a1,v_ViewPort(a0)

	lea		vi_bitmap(a5),a0
	move.w		ab_planes(a3),d0		; depth

	move.l		ab_breedte_x(a3),d1		; breedte in bytes * 8
	tst.b		ab_nonleaved(a3)
	bne.b		no_modu
	mulu		ab_planes(a3),d1
no_modu:
	lsl.l		#3,d1				; width
	move.w		ab_leny(a3),d2			; height
	jsr		_LVOInitBitMap(a6)

	lea		vi_bitmap(a5),a0

	bsr.w		set_planes

	lea.l		vi_rasinfo(a5),a0		; initialiseer rasinfo
	lea.l		vi_bitmap(a5),a1
	move.l		a1,ri_BitMap(a0)
	move.w		#0,ri_RxOffset(a0)
	move.w		#0,ri_RyOffset(a0)
	move.l		#0,ri_Next(a0)

	lea		vi_viewport(a5),a0
	lea		vi_rasinfo(a5),a1
	move.l		a1,vp_RasInfo(a0)

	clr.l		vp_Next(a0)

	bsr		set_xy_viewport

	moveq	#VIEW_EXTRA_TYPE,d0
	jsr	_LVOGfxNew(a6)
	tst.l	d0
	beq	exit_maakv

	move.l	d0,vi_vextra(a5)

	move.l	d0,a1
	lea.l	vi_view(a5),a0
	jsr	_LVOGfxAssociate(a6)

	lea.l	vi_view(a5),a0
	or.w	#EXTEND_VSTRUCT,v_Modes(a0)

	move.l	vi_vextra(a5),a0
	move.l	ab_monitorspec(a3),ve_Monitor(a0)

	moveq	#VIEWPORT_EXTRA_TYPE,d0
	jsr	_LVOGfxNew(a6)
	move.l	d0,vi_vpextra(a5)
	beq	exit_maakv

	move.l	d0,a1
	lea.l	vi_viewport(a5),a0
	jsr	_LVOGfxAssociate(a6)

	move.l	ab_tags(a3),a0
	move.l	vi_vpextra(a5),12(a0)

	move.w	#0,a0
	lea	ab_dimquery(a3),a1
	moveq	#dim_SIZEOF,d0
	move.l	#DTAG_DIMS,d1
	move.l	ab_monid(a3),d2
	or.w	ab_mode(a3),d2
	or.w	ab_or_vmode_mask(a3),d2
	jsr	_LVOGetDisplayInfoData(a6)

	lea	ab_dimquery(a3),a0
	move.l	vi_vpextra(a5),a1

	move.l	dim_MaxOScan(a0),vpe_DisplayClip(a1)
	move.l	dim_MaxOScan+4(a0),vpe_DisplayClip+4(a1)

	move.l	ab_monid(a3),d0
	or.w	ab_mode(a3),d0
        move.w 	ab_mode(a3),d3            
	and.w   #~$884,d3                 
	bne     .nolow2                   
	and.w   #~SUPER_KEY,d0            
	or.w    ab_alternate_lowres(a3),d0
.nolow2:
	jsr	_LVOFindDisplayInfo(a6)
	move.l	ab_tags(a3),a0
	move.l	d0,20(a0)		
no_v36_1:
	move.l		ab_num_cols(a3),d0
	jsr		_LVOGetColorMap(a6)
	tst.w		d0
	beq		exit_maakv
	
	move.l		d0,a0
	move.l		ab_tags(a3),a1
	lea.l		vi_viewport(a5),a2
	move.l		a2,4(a1)
	jsr		_LVOVideoControl(a6)

	lea		vi_viewport(a5),a0
	move.l		ab_col_pointer(a3),a1
	move.l		ab_num_cols(a3),d0
	jsr		_LVOLoadRGB4(a6)

	lea		vi_view(a5),a0
	lea		vi_viewport(a5),a1
	move.l		ab_ucop_pointer(a3),vp_UCopIns(a1)
	jsr		_LVOMakeVPort(a6)

	lea		vi_view(a5),a1
	jsr		_LVOMrgCop(a6)
exit_maakv:
	rts

tags:
	dc.l	VTAG_ATTACH_CM_SET,0
	dc.l	VTAG_VIEWPORTEXTRA_SET,0
	dc.l	VTAG_NORMAL_DISP_SET,0
	dc.l	VTAG_BORDERBLANK_CLR,0
	dc.l	VTAG_CHROMAKEY_SET,0
	dc.l	VTAG_CHROMA_PEN_CLR,0
	dc.l	VTAG_END_CM,0

*
* Create a viewport in the global ml data pointed at by ab_sysframe
*
maak_viewport_global:
	move.l		ab_graphbase(a3),a6
	move.l		ab_sysframe(a3),a5
	lea		sbu_Display(a5),a5
	bsr		maak_viewport
	rts

	IFNE	0	
	lea		sbu_Display+dp_View(a5),a1
	jsr		_LVOInitView(a6)

	lea		sbu_Display+dp_ViewPort(a5),a0
	jsr		_LVOInitVPort(a6)

	lea		sbu_Display+dp_View(a5),a0
	lea		sbu_Display+dp_ViewPort(a5),a1
	move.l		a1,v_ViewPort(a0)
	move.w		ab_mode(a3),v_Modes(a0)
	or.w		#V_LACE,v_Modes(a0)

	lea		sbu_Display+dp_BitMap(a5),a0
	move.w		ab_planes(a3),d0			; depth

	move.l		ab_breedte_x(a3),d1		; breedte in bytes * 8
	tst.b		ab_nonleaved(a3)
	bne.b		no_modu3
	mulu		ab_planes(a3),d1
no_modu3:
	lsl.l		#3,d1				; width
	move.w		ab_leny(a3),d2			; height
	jsr		_LVOInitBitMap(a6)

	lea		sbu_Display+dp_BitMap(a5),a0
	bsr.w		set_planes
	
	lea		sbu_Display+dp_RasInfo(a5),a0

	lea		sbu_Display+dp_BitMap(a5),a1
	move.l		a1,ri_BitMap(a0)
	move.w		#0,ri_RxOffset(a0)
	move.w		#0,ri_RyOffset(a0)
	move.l		#0,ri_Next(a0)

	lea		sbu_Display+dp_ViewPort(a5),a0
	lea		sbu_Display+dp_RasInfo(a5),a1
	move.l		a1,vp_RasInfo(a0)

	bsr.w		set_xy_viewport

	move.l		ab_num_cols(a3),d0
	jsr		_LVOGetColorMap(a6)
	lea		sbu_Display+dp_ViewPort(a5),a0
	move.l		d0,vp_ColorMap(a0)
	beq.w		exit_maakv

	lea		sbu_Display+dp_ViewPort(a5),a0
	move.l		ab_col_pointer(a3),a1

	move.l		ab_num_cols(a3),d0
	jsr		_LVOLoadRGB4(a6)

	lea		sbu_Display+dp_View(a5),a0
	lea		sbu_Display+dp_ViewPort(a5),a1
	move.l          ab_ucop_pointer(a3),vp_UCopIns(a1)
	jsr		_LVOMakeVPort(a6)

	lea		sbu_Display+dp_View(a5),a1
	jsr		_LVOMrgCop(a6)

	rts
	ENDC

reset_oldview:
	tst.l		ab_oldview(a3)
	beq.b		no_oldview
	move.l		ab_graphbase(a3),a6
	move.l		ab_oldview(a3),a1
	jsr		_LVOLoadView(a6)
	jsr		_LVOWaitTOF(a6)
no_oldview:
	rts

*
* Free view a0 en viewport a1
*
free_view_port:
	movem.l		d6/d7,-(a7)
	move.l		a0,d7
	move.l		a1,d6
	bra.b		free_in

freememory:
	movem.l		d6/d7,-(a7)
	lea		vi_view(a5),a1
	lea		vi_viewport(a5),a0
	move.l		a0,d7
	move.l		a1,d6
	
free_in:
	move.l		ab_graphbase(a3),a6
	move.l		d7,a0
	jsr		_LVOGfxLookUp(a6)
	move.l		d0,vi_vpextra(a5)
	move.l		d6,a0
	jsr		_LVOGfxLookUp(a6)
	move.l		d0,vi_vextra(a5)

	move.l		d7,a0
	tst.l		vp_ColorMap(a0)
	beq.b		.no_colormem

	move.l		vp_ColorMap(a0),a0
	jsr		_LVOFreeColorMap(a6)
	move.l		d7,a0
	clr.l		vp_ColorMap(a0)
.no_colormem:

	move.l		d7,a0
	jsr		_LVOFreeVPortCopLists(a6)

	move.l		d6,a0
	tst.l		v_LOFCprList(a0)
	beq.b		.no_lofmem

	move.l		v_LOFCprList(a0),a0
	jsr		_LVOFreeCprList(a6)
	lea		vi_view(a5),a0
	move.l		d6,a0
	clr.l		v_LOFCprList(a0)
.no_lofmem:

	move.l		d6,a0
	tst.l		v_SHFCprList(a0)
	beq.b		.no_shfmem

	move.l		v_SHFCprList(a0),a0
	jsr		_LVOFreeCprList(a6)
	move.l		d6,a0
	clr.l		v_SHFCprList(a0)
.no_shfmem:

.no_colormap:
	tst.l		vi_vpextra(a5)
	beq		.no_vpextra
	move.l		vi_vpextra(a5),a0
	jsr		_LVOGfxFree(a6)
	clr.l		vi_vpextra(a5)
.no_vpextra:
	tst.l		vi_vextra(a5)
	beq		.no_vextra
	move.l		vi_vextra(a5),a0
	jsr		_LVOGfxFree(a6)
	clr.l		vi_vextra(a5)
.no_vextra:
	movem.l		(a7)+,d7/d6
	moveq		#0,d0
	rts

get_file_size:
	move.l	ab_filenaam(a3),d1
	moveq	#-2,d2
	move.l	ab_dosbase(a3),a6
	jsr	_LVOLock(a6)
	move.l	d0,ab_filelock(a3)
	beq.w	exit
	move.l	d0,d1
	lea.l	ab_view1(a3),a0
	addq.l	#8,a0
	move.l	a0,d2
	and.l	#$fffffffc,d2
	move.l	d2,ab_frame_pointer(a3)
	move.l	ab_dosbase(a3),a6
	jsr	_LVOExamine(a6)
	move.l	ab_frame_pointer(a3),a2
	move.l	124(a2),ab_filesize(a3)
	move.l	ab_filelock(a3),d1
	move.l	ab_dosbase(a3),a6
	jsr	_LVOUnLock(a6)

	bsr.w	clear_view_mem

	rts

check_CDXL:
	moveq	#panh_SIZEOF,d3
	lea	ab_buffer(a3),a0
	move.l	a0,d2
	move.l	ab_filehandle(a3),d1
	move.l	ab_dosbase(a3),a6
	jsr	_LVORead(a6)		
	lea	ab_buffer(a3),a0
	move.l	panh_Size(a0),ab_pansize(a3)	
	move.l	ab_filesize(a3),d1
	moveq	#1,d7
	move.l	ab_pansize(a3),d0
	tst.w	d0
	beq.b	exitcd
.ch:
	swap	d0
	tst.w	d0
	beq	.no_gg
	swap	d0
	lsr.l	#1,d0
	lsr.l	#1,d1
	bra	.ch
.no_gg:
	swap	d0
.y_gg:
	divu	d0,d1
	swap	d1
	tst.w	d1
	bne.b	exitcd

	IFNE	XAPP
	move.l	ab_mlsysstruct(a3),a0
	move.l	ml_buffers+sbu_Size(a0),d0
	ELSE
	move.l	#MEMSIZE,d0
	ENDC
	move.l	ab_pansize(a3),d1
	cmp.l	d0,d1
	bgt	exitcd

 	move.l	ab_filehandle(a3),d1
	moveq	#0,d2
	moveq	#OFFSET_BEGINNING,d3
	move.l	ab_dosbase(a3),a6
	jsr	_LVOSeek(a6)
 	rts

exitcd:

	bra.w	exit

read_whole_file:
	bsr.w	get_file_size

	tst.l	ab_filehandle(a3)
	beq.b	no_fopen2
	bsr.w	close_file
no_fopen2:
	moveq	#3,d7
	move.l	ab_filenaam(a3),d1
	move.l	#mode_old,d2
	move.l	ab_dosbase(a3),a6
	jsr	_LVOOpen(a6)
	move.l	d0,ab_filehandle(a3)
	beq.w	exit

	tst.b	ab_diskanim(a3)
	bne.b	allocate_read_bufs

	move.l	ab_filesize(a3),ab_packed_size(a3)
	move.l	ab_filesize(a3),d0
	moveq	#2,d7
	move.l	#$10000,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
	move.l	d0,ab_packed(a3)

	IFNE XAPP
	beq.b	disk_close_file
	ELSE
	beq.b	allocate_read_bufs
	ENDC

	bsr.w	check_CDXL
		
	move.l	ab_packed_size(a3),d3
	move.l	ab_packed(a3),d2
	move.l	ab_filehandle(a3),d1
	move.l	ab_dosbase(a3),a6
	jsr	_LVORead(a6)		

* hele file nu in het geheugen 

	bsr.w	close_file
	moveq	#0,d0
	rts

disk_close_file:
	move.b	#1,ab_diskanim(a3)
	bsr.w	close_file
	moveq	#0,d0
	rts

*
* Can't read the whole file in at once or I don't want to
* Use diskanim instead
* allocate two buffers for loading
*
allocate_read_bufs:
	moveq	#panh_SIZEOF,d3
	lea	ab_buffer(a3),a0
	move.l	a0,d2
	move.l	ab_filehandle(a3),d1
	move.l	ab_dosbase(a3),a6
	jsr	_LVORead(a6)		
	lea	ab_buffer(a3),a0
	move.l	panh_Size(a0),ab_pansize(a3)	
	move.l	ab_filesize(a3),d1
	moveq	#1,d7
	move.l	ab_pansize(a3),d0
	beq.w	exitcd
	divu	d0,d1
	swap	d1
	tst.w	d1
	bne.w	exitcd
	move.l	panh_Size(a0),d0
	add.l	d0,d0
	move.l	d0,ab_loadmem_size(a3)		; for two buffers
	moveq	#MEMF_PUBLIC,d1
	move.l	$4.w,a6
	jsr	_LVOAllocMem(a6)
	moveq	#2,d7
	move.l	d0,ab_loadmem(a3)
	beq.w	exit
	move.l	d0,ab_loadbuf1(a3)
	add.l	ab_pansize(a3),d0
	move.l	d0,ab_loadbuf2(a3)
	move.l	ab_filehandle(a3),d1
	moveq	#0,d2
	moveq	#OFFSET_BEGINNING,d3
	move.l	ab_dosbase(a3),a6
	jsr	_LVOSeek(a6)

;	bsr	read_one_pan

	bsr.b	read_one_pan_chip

	move.b	#1,ab_diskanim(a3)
	rts

*
* Read one pan frame from the file
* in the buffer pointer a0
* filehandle is valid
*
read_one_pan:
	move.l	ab_pansize(a3),d3
	move.l	ab_loadbuf1(a3),d2
	move.l	ab_filehandle(a3),d1
	move.l	ab_dosbase(a3),a6
	jsr	_LVORead(a6)
	rts

read_one_pan_with_copy:
	bsr.b	read_one_pan
;	move.l	ab_loadbuf1(a3),a0
;	bsr	copy_cdxl
	rts

read_one_pan_chip:
	cmp.b	#1,ab_double(a3)
	beq.b	read_one_pan_with_copy
	
	move.l	ab_pansize(a3),d3
	move.l	ab_frame_hidden(a3),d2
	move.l	ab_filehandle(a3),d1
	move.l	ab_dosbase(a3),a6
	jsr	_LVORead(a6)
	rts

install_audio_0:
	lea.l	intstruct0(pc),a1
	lea	proc0name(pc),a0
	move.l	a0,10(a1)
	lea	startsample0(pc),a0
	move.l	a0,14(a1)
	lea	proc_audio_0(pc),a0
	move.l	a0,18(a1)
	
	moveq	#7,d0
	move.l	$4.w,a6
	jsr	_LVOSetIntVector(a6)
	move.l	d0,ab_old_audio_0(a3)
	rts

remove_audio_0:
	moveq	#7,d0
	move.l	ab_old_audio_0(a3),a1
	move.l	$4.w,a6
	jsr	_LVOSetIntVector(a6)
	clr.l	ab_old_audio_0(a3)
	rts

proc_audio_0:
	move.l	startsample0(pc),$dff0a0 ; zet audio start naar andere buffer
	move.l	startsample0(pc),$dff0b0

	bsr.w	stuur_signaal

	move.w	#$0080,$dff09c
	rts

proc0name:	dc.b	"audio 0 int",0
	even

intstruct0:
	dc.l	0,0
	dc.b	2,100			; type en pri
	dc.l	0,0			; pointer naar naam en data
	dc.l	0

startsample0:	dc.l	0
stopsample0:	dc.l	0	
waarsample0:	dc.l	0
volume0:	dc.w	64
periode0:	dc.w	128
loopbit0:	dc.b	0,0
onoff0:		dc.b	0,0

close_file:
	move.l	ab_dosbase(a3),a6
	move.l	ab_filehandle(a3),d1
	jsr	_LVOClose(a6)
	clr.l	ab_filehandle(a3)
	rts

	
openlibs:
	clr.l	ab_packed(a3)
	clr.l	ab_unpacked(a3)
	clr.l	ab_filehandle(a3)
	clr.l	ab_graphbase(a3)
	clr.l	ab_intbase(a3)

	clr.l	ab_dosbase(a3)
	move.l	$4.w,a6

	lea	dosnam(pc),a1
	moveq	#0,d0
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,ab_dosbase(a3)
	beq.b	exit

	lea	graphname(pc),a1
	moveq	#0,d0
	move.l	$4.w,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,ab_graphbase(a3)
	beq.s	exit

	lea	intnam(pc),a1
	moveq	#0,d0
	move.l	$4.w,a6
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,ab_intbase(a3)
	beq.s	exit

	move.l	ab_dosbase(a3),a6
	jsr	_LVOOutput(a6)
	move.l	d0,ab_conhandle(a3)

*	beq.s	exit
	rts

close_libs:
	tst.l	ab_graphbase(a3)
	beq.s	no_gbase
	move.l	ab_graphbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	ab_graphbase(a3)

no_gbase:
	tst.l	ab_intbase(a3)
	beq.s	no_intb
	move.l	ab_intbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	ab_intbase(a3)
no_intb:
	tst.l	ab_dosbase(a3)
	beq.b	no_dosb
	move.l	ab_dosbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	ab_dosbase(a3)
no_dosb:
	rts
	
exit:	
	IFNE XAPP
	move.l	ab_easy_exit(a3),a7
	lea	error(pc),a0
	move.l	d7,(a0)
	movem.l	(a7)+,d0-d7/a0-a6
	unlk	a5
	move.l	error(pc),d0
	rts
error:	dc.l	0

	ELSE

	bsr	clear_sound

	bsr.w	remove_50h

	bsr.w	freesam

	bsr.w	reset_oldview
	lea	ab_view1(a3),a5
	bsr.w	freememory
	lea	ab_view2(a3),a5
	lea     vi_viewport(a5),a1
	move.l  #0,vp_UCopIns(a1)		; don't free this twice
	bsr.w	freememory

	move.w   #$0001,$dff096			; ???

	move.l	ab_easy_exit(a3),a7
exit11:	
	tst.l	ab_loadmem(a3)
	beq	exit111
	move.l	ab_loadmem(a3),a1
	move.l	ab_loadmem_size(a3),d0
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)
	clr.l	ab_loadmem(a3)
exit111:
	tst.l	ab_unpackedsecond(a3)
	beq.s	exit12
	move.l	ab_unpackedsecond(a3),a1
	move.l	#MEMSIZE,d0
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)
	clr.l	ab_unpackedsecond(a3)
exit12:
	tst.l	ab_unpacked(a3)
	beq.s	exit1
	move.l	ab_unpacked(a3),a1
	move.l	#MEMSIZE,d0
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)
	clr.l	ab_unpacked(a3)

exit1:	tst.l	ab_packed(a3)
	beq.s	exit21
	move.l	ab_packed(a3),a1
	move.l	ab_packed_size(a3),d0
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)
	clr.l	ab_packed(a3)

exit21:
	tst.l	ab_low_hi(a3)
	beq.s	exit22
	move.l	ab_low_hi(a3),a1
	move.l	#TABLE_SIZE,d0
	move.l	$4.w,a6
	jsr	_LVOFreeMem(a6)
	clr.l	ab_low_hi(a3)

exit22:	tst.l	ab_filehandle(a3)
	beq.s	exit2
	bsr.w	close_file
	
exit2:
	tst.l	ab_graphbase(a3)
	beq.s	exit3
	move.l	ab_graphbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	ab_graphbase(a3)

exit3:
	tst.l	ab_intbase(a3)
	beq.s	exit4
	move.l	ab_intbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	ab_intbase(a3)

exit4:	
	tst.l	ab_dosbase(a3)
	beq.s	exit5

	tst.b	ab_info(a3)
	beq.b	no_info2
	tst.l	ab_conhandle(a3)
	beq.b	no_info2

	movem.l	d0-d7/a0-a6,-(a7)
	move.l	timer(pc),d0
	bsr.w	printsecs
	bsr.w	printframes
	bsr.b	print_persec
	movem.l	(a7)+,d0-d7/a0-a6

no_info2:
	tst.l	d7
	beq.s	exit51
	subq.l	#1,d7
	move.l	ab_dosbase(a3),a6
	lea.l	foutmelding(pc),a1
	lsl.l	#2,d7
	add.l	d7,a1
	move.l	(a1),d2
	move.l	ab_conhandle(a3),d1

	move.l	ab_conhandle(a3),d1
	beq.w	no_con_open

	move.l	#FOUT_LENGTE,d3
	jsr	_LVOWrite(a6)

exit51:
	move.l	ab_dosbase(a3),a1
	move.l	$4.w,a6
	jsr	_LVOCloseLibrary(a6)
	clr.l	ab_dosbase(a3)
exit5:
	moveq	#0,d0
	rts

veelvoud:
	move.l	#1,d5
	move.l	d4,d1
rephon:	cmp.l	d0,d1
	bhi.b	geenho
	add.l	d4,d1
	addq.l	#1,d5
	cmpi.l	#10,d5
	bne.b	rephon
geenho:	subq.l	#1,d5
	sub.l	d4,d1
	sub.l	d1,d0

	bsr.w	putin
	rts


; print timer / frames decimaal

print_persec:
	move.l	ab_frames(a3),d0
	mulu	#100,d0
	move.l	timer(pc),d1
	beq.b	no_div
	divu	d1,d0
	move.b	#1,ab_lead(a3)
	lea.l	ab_view1(a3),a1
	move.l	#"Fr /",(a1)+
	move.l	#" s: ",(a1)+
	and.l	#$ffff,d0
	bra.b	printdec
no_div:
	rts	

printframes:
	move.b	#1,ab_lead(a3)
	move.l	ab_frames(a3),d0
	lea.l	ab_view1(a3),a1
	move.l	#"Fram",(a1)+
	move.l	#"es: ",(a1)+
	bra.b	printdec
	
printsecs:
	move.b	#0,ab_lead(a3)
	move.l	timer(pc),d0
	lea.l	ab_view1(a3),a1
	move.l	#"Secs",(a1)+
	move.l	#"  : ",(a1)+

printdec:
	move.l	#10000000,d4
	bsr.w	veelvoud
	move.l	#1000000,d4
	bsr.w	veelvoud
	move.l	#100000,d4
	bsr.w	veelvoud
	move.l	#10000,d4
	bsr.w	veelvoud
		
	lea.l	tienvoud(pc),a4

	move.l	#3,d7
repdec:	move.l	(a4)+,d6
	tst.b	ab_lead(a3)
	bne.b	no_point
	cmp.l	#10,d6
	bne.b	no_point
	move.b	#'.',(a1)+
no_point:
	move.l	d0,d5
	divu	d6,d5
	bsr.b	putin
	swap	d5
	clr.l	d0
	move.w	d5,d0
	dbf	d7,repdec
	move.b	#$a,(a1)+
	lea	ab_view1(a3),a2
	move.l	a1,d3
	sub.l	a2,d3

	move.l	a2,d2
	move.l	ab_conhandle(a3),d1
	move.l	ab_dosbase(a3),a6
	jsr	_LVOWrite(a6)
	move.b	#0,ab_aa_present(a3)
	rts

putin:	move.l	d5,-(a7)
	tst.b	ab_aa_present(a3)
	bne.b	nospa
	tst.b	d5
	bne.b	nospa
;	move.b	#" ",(a1)+
	move.l	(a7)+,d5
	rts
nospa:	move.b	#$f,ab_aa_present(a3)
	add.b	#"0",d5
	move.b	d5,(a1)+
	move.l	(a7)+,d5
	rts


tienvoud:	dc.l	1000,100,10,1

no_con_open:
	move.l	d2,a0
	move.l	#FOUT_LENGTE-3,d3

	lea	ab_view1(a3),a1
	move.b	#0,(a1)+
	move.b	#20,(a1)+
	move.b	#20,(a1)+
c_fout:	move.b	(a0)+,(a1)+
	dbf	d3,c_fout

	move.l	$4.w,a6
	lea.l	intnam(pc),a1
	jsr	-408(a6)
	tst.l	d0
	beq.b	err_intui
	move.l	d0,a6
	clr.l	d0
	lea	ab_view1(a3),a0
	move.l	#40,d1
	jsr	-90(a6)

	move.l	a6,a1
	move.l	4,a6
	jsr	-414(a6)
err_intui
	bra.w	exit51

	even

FOUT_LENGTE=21

foutmelding:
	dc.l	fout0,fout1,fout2,fout3,fout4,fout5

fout0:		dc.b	"Not an CDXL file    ",10,0
fout1:		dc.b	"Not enough memory   ",10,0
fout2:		dc.b	"File not found      ",10,0
fout3:		dc.b	"Unknown anim mode   ",0
fout4:		dc.b	" ",0
fout5:		dc.b	" ",0

	even

;filename:	dc.b	'cprogs:cdxl/callnow.xl',0
;filename:	dc.b	"hd2:cdxl/coke2.cdxl",0
;filename:	dc.b	"hd2:cdxl/blade.xl",0
filename:	dc.b	"hd2:cdxl/bluesbrothers.xl",0
;filename:	dc.b	"hd2:cdxl/star1.cdxl",0
	even
	ENDC

dosnam:		dc.b	'dos.library',0
graphname:	dc.b	'graphics.library',0
intnam:		dc.b	'intuition.library',0
	even

	IFEQ XAPP
datablock:
	blk.b	ab_SIZEOF,0

	ENDC
