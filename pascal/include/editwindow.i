	IFND	MEDIAPOINT_EDITWINDOW_I
MEDIAPOINT_EDITWINDOW_I	SET	1
**
**	$VER: mediapoint/pascal/include/editwindow.i 01.008 (02.11.93)
**
**	Contains structure.
**
**	(C) Copyright 1992-1993 B.O.M.B. AudioVisual Entertainment
**	    All Rights Reserved
**


  STRUCTURE EditWindow,0
	STRUCT	ew_frameNode,MLN_SIZE
	UWORD	ew_FirstChar
	UWORD	ew_LastChar
	UBYTE	ew_antiAliasLevel
	UBYTE	ew_justification
	APTR	ew_rastPort
	APTR	ew_teInfo
	APTR	ew_undoBM		; null or ptr to bitmap

	WORD	ew_xSpacing
	WORD	ew_ySpacing
	UWORD	ew_slantAmount
	WORD	ew_slantValue
	UWORD	ew_underLineHeight
	WORD	ew_underLineOffset

	UBYTE	ew_shadowDepth
	UBYTE	ew_shadowPen
	UBYTE	ew_shadowType
	UBYTE	ew_shadowDirection

	UBYTE	ew_wdw_shadowDepth
	UBYTE	ew_wdw_shadowDirection
	UBYTE	ew_wdw_shadowPen

	ALIGNWORD
	WORD	ew_X
	WORD	ew_Y
	WORD	ew_Width
	WORD	ew_Height
	WORD	ew_TopMargin
	WORD	ew_BottomMargin
	WORD	ew_LeftMargin
	WORD	ew_RightMargin
	UBYTE	ew_Border
	ALIGNWORD
	STRUCT	ew_BorderColor,4*2
	WORD	ew_BorderWidth
	UWORD	ew_BackFillType
	UWORD	ew_BackFillColor
 	WORD	ew_PhotoOffsetX
	WORD	ew_PhotoOffsetY
	WORD	ew_PatternNum
	UWORD	ew_DrawSeqNum

	APTR	ew_charFont;
	UBYTE	ew_underlineColor;
	UBYTE	ew_charStyle;
	UBYTE	ew_charColor;

	UBYTE	ew_flags;

	STRUCT	ew_in1,3*2
	STRUCT	ew_in2,3*2
	STRUCT	ew_in3,3*2
	STRUCT	ew_out1,3*2
	STRUCT	ew_out2,3*2
	STRUCT	ew_out3,3*2
	STRUCT	ew_inDelay,3*4
	STRUCT	ew_outDelay,3*4

	STRUCT	ew_crawl_fontName,50
	WORD	ew_crawl_fontSize
	UBYTE	ew_crawl_speed
	UBYTE	ew_crawl_flags
	APTR	ew_crawl_text
	UWORD	ew_crawl_length
	UBYTE	ew_crawl_color

	WORD	ew_bx
	WORD	ew_by
	WORD	ew_bwidth
	WORD	ew_bheight
	UBYTE	ew_jumpType
	UBYTE	ew_renderType
	UBYTE	ew_audioCue
	WORD	ew_keyCode
	WORD	ew_rawkeyCode
	STRUCT	ew_buttonName,50	
	LABEL	ew_SIZE


	ENDC	; MEDIAPOINT_EDITWINDOW_I
