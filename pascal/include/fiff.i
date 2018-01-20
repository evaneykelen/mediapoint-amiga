	IFND	MEDIAPOINT_FIFF_I
MEDIAPOINT_FIFF_I	SET	1
**
**	$VER: mediapoint/pascal/fiff.i 01.010 (17.08.93)
**	Includes Release 39.108
**
**	Contains structures and coded definitions.
**
**	(C) Copyright 1992-1993 B.O.M.B. AudioVisual Entertainment
**	    All Rights Reserved
**

	IFND	EXEC_LISTS_I
	INCLUDE	"exec/lists.i"
	ENDC

   STRUCTURE BitMapHeader,0
	UWORD	bmh_w
	UWORD	bmh_h
	WORD	bmh_x
	WORD	bmh_y
	UBYTE	bmh_nPlanes
	UBYTE	bmh_masking
	UBYTE	bmh_compression
	UBYTE	bmh_pad1
	UWORD	bmh_transparentColor
	UBYTE	bmh_xAspect
	UBYTE	bmh_yAspect
	WORD	bmh_pageWidth
	WORD	bmh_pageHeight
	LABEL	bmh_SIZEOF
	

   STRUCTURE IFF_FRAME,0
	STRUCT	iff_BMH,bmh_SIZEOF
	APTR	iff_colorMap
	ULONG	iff_viewModes
	LONG	iff_Error
	LONG	iff_ErrorInfo
	APTR	iff_BufFileHandle
	APTR	iff_Memory
	LABEL	iff_SIZEOF

	IFD	ANIM_IN_FIFF

   STRUCTURE ANIMLIST,0
	STRUCT	al_FrameList,MLH_SIZE
	STRUCT	al_IFFFrame,iff_SIZE
	ULONG	al_BufFileInfo
	UWORD	al_LoadSuccess
	LABEL	al_SIZE

   STRUCTURE ANIMHEAD,0
	UBYTE	ah_operation	; compression method (ANIM 5)
	UBYTE	ah_mask		; XOR compression
	UWORD	ah_w		; 	<>
	UWORD	ah_h		; 	<>
	UWORD	ah_x		; 	<>
	UWORD	ah_y		; 	<>
	ULONG	ah_abstime	; currently unused
	ULONG	ah_reltime	; relative frame time in jiffies (1/60th sec)
	UBYTE	ah_interleave	; 0 = normal, 1 = animbrushes
	UBYTE	ah_pad0		;
	ULONG	ah_bits		; option bits
	STRUCT	ah_pad,16	;
	LABEL	ah_SIZE

   STRUCTURE ANIMFRAME,0
	STRUCT	af_Node,LN_SIZE
	APTR	af_Header	; ptr to ANIMHEAD block, may be the same for
				; multiple frames
	APTR	af_ColorMap	; ptr to colormap for this frame (if any)
	APTR	af_Offset	; ptr to data of DLTA chunk, relative
				; fileoffset if previous frame from disk,
				; absolute if previous frame in RAM
				; FORM ILBM mem address if ILBM (1st) frame
	ULONG	af_SizeOfData	; size of data af_Offset points to
	LABEL	af_SIZE

	ENDC	; ANIM_IN_FIFF

IFF_ERROR_OK			equ	0
IFF_ERROR_BAD_FORM		equ	1
IFF_ERROR_BAD_ILBM		equ	2
IFF_ERROR_BAD_BMHD		equ	3
IFF_ERROR_BAD_CMAP		equ	4
IFF_ERROR_BAD_CAMG		equ	5
IFF_ERROR_BAD_BODY		equ	6
IFF_ERROR_DOS_FAILURE		equ	7	; error code in ErrorInfo
IFF_ERROR_NO_MEMORY		equ	8
IFF_ERROR_DECODE_FAILURE	equ	9
IFF_ERROR_PARSE_FAILURE		equ	10
IFF_ERROR_ID_NOT_FOUND		equ	11
IFF_ERROR_UNKNOWN_COMPRESSION	equ	12
IFF_ERROR_NO_PLANES		equ	13
IFF_ERROR_NOT_IFF		equ	14

IFF_ERRINFO_X_CLIPPED		equ	1	; width ilbm clipped
IFF_ERRINFO_X_UNDERSCAN		equ	2	; width ilbm too small
IFF_ERRINFO_Y_CLIPPED		equ	4	; height ilbm clipped
IFF_ERRINFO_Y_UNDERSCAN		equ	8	; height ilbm too small
IFF_ERRINFO_STENCIL		equ	16	; stencil skipped
IFF_ERRINFO_PLANES		equ	32	; planes skipped

BITN_IFF_ERRINFO_X_CLIPPED	equ	0
BITN_IFF_ERRINFO_X_UNDERSCAN	equ	1
BITN_IFF_ERRINFO_Y_CLIPPED	equ	2
BITN_IFF_ERRINFO_Y_UNDERSCAN	equ	3
BITN_IFF_ERRINFO_STENCIL	equ	4
BITN_IFF_ERRINFO_PLANES		equ	5

GETSIZE_ERR_DOS			equ	-1	; errorcode iffGetFileSize
GETSIZE_ERR_MEM			equ	-2	; errorcode iffGetFileSize

FILETYPE_ERROR		equ	0
FILETYPE_UNKNOWN	equ	1
FILETYPE_PICTURE	equ	2
FILETYPE_ANIM		equ	3
FILETYPE_SCRIPT		equ	4
FILETYPE_PAGE		equ	5
FILETYPE_TEXT		equ	6		; now only ASCII, future IFF too
FILETYPE_MUSIC		equ	7		; future
FILETYPE_ICON		equ	8
FILETYPE_MP_TEXT	equ	9
;FILETYPE_THUMB		equ	10		; what should I do with this?

FORM			equ	$464f524d
ILBM			equ	$494c424d
BMHD			equ	$424d4844
CMAP			equ	$434d4150
BODY			equ	$424f4459
CAMG			equ	$43414d47
ANHD			equ	$414E4844
DLTA			equ	$444C5441
ANIM			equ	$414E494D
DPAN			equ	$4450414E
SVX			equ	$38535658
VHDR			equ	$56484452
MThd			equ	$4D546864
ACBM			equ	$4143424D
ABIT			equ	$41424954
MAUD			equ	$4D415544

cmpNone			equ	0
cmpByteRun1		equ	1

mskNone			equ	0
mskHasMask		equ	1
mskHasTransparentColor	equ	2
mskLasso		equ	3

	IFD	ANIM_IN_FIFF

PLT_RAM			equ	0	; PreLoadType defines for Anims
PLT_DISK		equ	1

NT_RAMFRAME		equ	255
NT_DISKFRAME		equ	254
NT_ILBMFRAME		equ	253

	ENDC	; ANIM_IN_FIFF

	ENDC	; MEDIAPOINT_FIFF_I
