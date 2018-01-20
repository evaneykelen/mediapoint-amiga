*
* File	: bltbitmapfm.s
* Uses	:
* Date	: 22 august 1993 
* Author: ing. C. Lieshout
* Desc	: blit bitmap from fast

	
	INCDIR	"include:"
	INCLUDE "graphics/gfx.i"

	RSRESET
bfm_block:		rs.w	0
bfm_restval:		rs.l	1
bfm_addr_offset_source:	rs.l	1
bfm_addr_offset_dest:	rs.l	1
bfm_totalx:		rs.w	1
bfm_totaly:		rs.w	1
bfm_totalx_w:		rs.w	1
bfm_modsource:		rs.l	1
bfm_moddest:		rs.l	1
bfm_depth:		rs.w	1
bfm_endmask:		rs.w	1
bfm_SIZEOF:		rs.w	0

	XDEF _BltBitMapFM
*
* BltBitMap(SrcBitMap, SrcX, SrcY, DstBitMap, 
*            A0         D0:16 D1:16 A1         
* DstX, DstY, SizeX, SizeY, Minterm, Mask [, TempA]) 
* D2:16 D3:16 D4:16  D5:16  D6:8     D7:8   [A2]
*
_BltBitMapFM:
	link	a5,#-bfm_SIZEOF

	movem.l	d1-d7/a0-a6,-(a7)

	sub.l	#bfm_SIZEOF,a5

	move.w	d5,bfm_totaly(a5)
	beq	.no_blit
	sub.w	#1,bfm_totaly(a5)		; for dbf

; check depth's

	moveq	#0,d6
	move.b	bm_Depth(a1),d7
	cmp.b	bm_Depth(a0),d7
	ble	.oke_depth
	move.b	bm_Depth(a0),d7			; use the smallest depth
.oke_depth:
	move.w	d7,bfm_depth(a5)
	beq	.no_blit
	sub.w	#1,bfm_depth(a5)

; calculate source start word

	move.w	d0,d7
	and.w	#$fff0,d7			; only copy words
	sub.w	d7,d0
	move.w	d0,bfm_restval(a5)
	lsr.l	#3,d7
	move.l	d7,bfm_addr_offset_source(a5)		; save offset x

	move.w	d4,d7
	add.w	d0,d7				; total nr of pixels to copy
	move.w	d7,bfm_totalx(a5)

	move.w	d7,d6
	moveq	#-1,d4
	and.w	#$f,d6
	beq	.no_shift
	move.l	#$ffff0000,d4
	lsr.l	d6,d4
.no_shift:
	move.w	d4,bfm_endmask(a5)	

	ext.l	d7
	add.l	#15,d7
	lsr.l	#4,d7

	move.w	d7,bfm_totalx_w(a5)		; words to copy
	beq	.no_blit

	add.l	d7,d7				; copy range in bytes
	move.w	bm_BytesPerRow(a0),d6
	ext.l	d6
	sub.l	d7,d6
	move.l	d6,bfm_modsource(a5)

	move.w	bm_BytesPerRow(a1),d6
	ext.l	d6
	sub.l	d7,d6
	move.l	d6,bfm_moddest(a5)		; should be zero ?

	move.w	bm_BytesPerRow(a0),d7
	mulu	d1,d7
	add.l	d7,bfm_addr_offset_source(a5)		; add y offset

; calculate destination offset

	moveq	#0,d7
	move.w	d2,d7
	and.w	#$fff0,d7			; force to words
	lsr.l	#3,d7				; bytes
	move.l	d7,bfm_addr_offset_dest(a5)	; save offset 

	move.w	bm_BytesPerRow(a1),d7
	mulu	d3,d7
	add.l	d7,bfm_addr_offset_dest(a5)	; add y offset


	move.l	bfm_modsource(a5),d0
	move.l	bfm_moddest(a5),d1
	move.w	bfm_totalx_w(a5),d7
	move.w	d7,d6
	and.b	#1,d7
	beq	.long_mode
	cmp.w	#1,bfm_totalx_w(a5)
	beq	.single_word_mode

	move.w	bfm_totalx_w(a5),d7
	lsr.w	#1,d7
	move.w	d7,bfm_totalx_w(a5)
	sub.w	#1,bfm_totalx_w(a5)		; for the dbf
	lea.l	bm_Planes(a0),a3		; source pointer planes
	lea.l	bm_Planes(a1),a4		; dest pointer planes
	move.w	bfm_depth(a5),d7
.lwrep_planes:
	move.l	(a3)+,a2
	move.l	(a4)+,a6
	add.l	bfm_addr_offset_source(a5),a2		; start source
	add.l	bfm_addr_offset_dest(a5),a6		; start dest
	move.w	bfm_totaly(a5),d6
.lwrep_lines:	
	move.w	bfm_totalx_w(a5),d5
.lwrep_words:
	move.l	(a2)+,(a6)+
	dbf	d5,.lwrep_words
	move.w	(a2)+,(a6)+			; add the uneven word
;	and.w	d4,-2(a6)
	add.l	d0,a2				; add modulo's
	add.l	d1,a6
	dbf	d6,.lwrep_lines
	dbf	d7,.lwrep_planes

.no_blit:	
	movem.l	(a7)+,d1-d7/a0-a6
	unlk	a5
	rts

.single_word_mode:
	lea.l	bm_Planes(a0),a3		; source pointer planes
	lea.l	bm_Planes(a1),a4		; dest pointer planes
	move.w	bfm_depth(a5),d7
.swrep_planes:
	move.l	(a3)+,a2
	move.l	(a4)+,a6
	add.l	bfm_addr_offset_source(a5),a2		; start source
	add.l	bfm_addr_offset_dest(a5),a6		; start dest
	move.w	bfm_totaly(a5),d6
.swrep_lines:	
	move.w	(a2)+,(a6)+			; move single word
;	and.w	d4,-2(a6)
	add.l	d0,a2				; add modulo's
	add.l	d1,a6
	dbf	d6,.swrep_lines
	dbf	d7,.swrep_planes
	bra	.no_blit
	
.long_mode:
	move.w	bfm_totalx_w(a5),d7
	lsr.w	#1,d7
	move.w	d7,bfm_totalx_w(a5)
	sub.w	#1,bfm_totalx_w(a5)		; for the dbf
	lea.l	bm_Planes(a0),a3		; source pointer planes
	lea.l	bm_Planes(a1),a4		; dest pointer planes
	move.w	bfm_depth(a5),d7
.lrep_planes:
	move.l	(a3)+,a2
	move.l	(a4)+,a6
	add.l	bfm_addr_offset_source(a5),a2		; start source
	add.l	bfm_addr_offset_dest(a5),a6		; start dest
	move.w	bfm_totaly(a5),d6
.lrep_lines:	
	move.w	bfm_totalx_w(a5),d5
.lrep_words:
	move.l	(a2)+,(a6)+
;	move.l	#$f0f0f0f0,(a6)+
	dbf	d5,.lrep_words
;	and.w	d4,-2(a6)
	add.l	d0,a2				; add modulo's
	add.l	d1,a6
	dbf	d6,.lrep_lines
	dbf	d7,.lrep_planes
	bra	.no_blit
