* the PAN cdxl structure

	RSRESET
panh_chunk:		rs.w	0
panh_Type:		rs.b	1
panh_Info:		rs.b	1
panh_Size:		rs.l	1
panh_SIZEOF:		rs.w	0

	RSRESET
pan_chunk:		rs.w	0
pan_Type:		rs.b	1
pan_Info:		rs.b	1
pan_Size:		rs.l	1
pan_Back:		rs.l	1
pan_Frame:		rs.l	1
pan_XSize:		rs.w	1
pan_YSize:		rs.w	1
pan_Res:		rs.b	1
pan_PixelSize:	rs.b	1
pan_ColorMapSize:	rs.w	1
pan_AudioSize:	rs.w	1
pan_PadBytes:		rs.b	8
pan_SIZEOF:		rs.w	0

; PanFrame.Info: Video Types

PIV_MASK     = $0f
PIV_STANDARD = 0
PIV_HAM      = 1
PIV_YUV      = 2
PIV_AVM      = 3

; PanFrame.Info: Audio type

PIA_MASK   = $10
PIA_MONO   = $00
PIA_STEREO = $10

; PanFrame.Info: Pixel Value Orientation

PIF_MASK      = $E0
PIB_CHUNKY    = 5
PIB_BITBYTE   = 6
PIB_PLANELINE = 7
PIF_PLANES    = $00
PIF_CHUNKY    = $20
PIF_BYTE      = $40
PIF_LINES     = $80
PIF_BYTELINES = $c0
