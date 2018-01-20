#ifndef MEDIAPOINT_FIFF_H
#define MEDIAPOINT_FIFF_H

/*- fiff.h -----------------------------------------------------------
  Include file for the MediaPoint Fast IFF ToolBox Routines.

  $VER: mediapoint/pascal/include/fiff.h 01.016 (17.01.94)
--------------------------------------------------------------------*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef GRAPHICS_GFX_H
#include <graphics/gfx.h>
#endif

#ifndef GRAPHICS_VIEW_H
#include <graphics/view.h>
#endif

#ifndef MEDIAPOINT_BUFREAD_H
#include <pascal:include/bufread.h>
#endif

#define FILETYPE_ERROR		0
#define FILETYPE_UNKNOWN	1
#define FILETYPE_PICTURE	2
#define FILETYPE_ANIM			3
#define FILETYPE_SCRIPT		4
#define FILETYPE_PAGE			5
#define FILETYPE_TEXT			6		// now only ASCII, future IFF too
#define FILETYPE_MUSIC		7		// future
#define FILETYPE_ICON			8
#define	FILETYPE_MP_TEXT	9		// starts with ^
#define FILETYPE_THUMB		10		// what should I do with this ?

#define FORM 0x464f524d
#define ILBM 0x494c424d
#define BMHD 0x424d4844
#define CMAP 0x434d4150
#define BODY 0x424f4459
#define CAMG 0x43414d47
#define ANIM 0x414E494D
#define DPAN 0x4450414E
#define SVX  0x38535658
#define VHDR 0x56484452
#define MThd 0x4D546864
#define ACBM 0x4143424D
#define ABIT 0x41424954
#define MAUD 0x4D415544

#define IFF_ERROR_OK									0
#define IFF_ERROR_BAD_FORM						1
#define IFF_ERROR_BAD_ILBM						2
#define IFF_ERROR_BAD_BMHD						3
#define IFF_ERROR_BAD_CMAP						4
#define IFF_ERROR_BAD_CAMG						5
#define IFF_ERROR_BAD_BODY						6
#define IFF_ERROR_DOS_FAILURE					7		/* dos error code in ErrorInfo */
#define IFF_ERROR_NO_MEMORY						8
#define IFF_ERROR_DECODE_FAILURE			9
#define IFF_ERROR_PARSE_FAILURE				10
#define IFF_ERROR_ID_NOT_FOUND				11
#define IFF_ERROR_UNKNOWN_COMPRESSION	12
#define IFF_ERROR_NO_PLANES						13	/* no bitmap data to save */
#define	IFF_ERROR_NOT_IFF							14

/* these defines will be OR'ed into the ErrorInfo integer */

#define IFF_ERRINFO_X_CLIPPED		1		/* iff clipped during decoding */
#define IFF_ERRINFO_X_UNDERSCAN	2		/* iff smaller than supplied bitmap */
#define IFF_ERRINFO_Y_CLIPPED		4		/* iff clipped during decoding */
#define IFF_ERRINFO_Y_UNDERSCAN	8		/* iff smaller than supplied bitmap */
#define IFF_ERRINFO_STENCIL			16	/* stencil skipped during decoding */
#define IFF_ERRINFO_PLANES			32	/* planes skipped during decoding */

typedef struct {
	UWORD w, h;
	WORD	x, y;
	UBYTE	nPlanes;
	UBYTE	masking;
	UBYTE	compression;
	UBYTE	pad1;
	UWORD	transparentColor;
	UBYTE	xAspect, yAspect;
	WORD	pageWidth, pageHeight;
} BitMapHeader;

#define	cmpNone			0
#define	cmpByteRun1	1

#define	mskNone									0
#define	mskHasMask							1
#define	mskHasTransparentColor	2
#define	mskLasso								3


struct IFF_FRAME {
	BitMapHeader BMH;
	struct	ColorMap *colorMap;
	ULONG		viewModes;
	int			Error;
	int			ErrorInfo;
	VOID		*bufFileHandle;
	UBYTE		*iffMemory;						/* pointer to allocremembered block */
};



#ifdef ANIM_IN_FIFF

/* AnimList */

struct AnimList {
	struct MinList al_FrameList;				/* linked list of animation frames */
	struct IFF_FRAME al_IFFFrame;				/* information about ILBM data */
	struct BufFileInfo *al_BufFileInfo;	/* pointer to buffered animfile */
	UWORD  al_LoadSuccess;							/* FALSE if failure, otherwise TRUE */
	};

/* AnimFrame */

#define NT_RAMFRAME		255
#define NT_DISKFRAME	254
#define NT_ILBMFRAME	253

struct AnimFrame {
	struct Node af_Node;					/* node of the linked list */
																/* af_Node.ln_Type is NT_<frametype> */
	struct AnimHead *af_Header;		/* PTR to animhead block. may be used */
																/* multiple for different frames */
	struct ColorMap *af_ColorMap;	/* ptr to colormap / NULL if no CMAP */
	BYTE	*af_Offset;							/* abs ptr in file / rel ptr in file */
																/* abs address for ILBM (NT_ILBMFRAME) */
   ULONG	af_SizeOfData;				/* size of data which af_Offset indicates */
	};

/* AnimHeader */

struct AnimHead {
	UBYTE	operation;		/* compression type */
	UBYTE	mask;					/* for XOR compression */
	UWORD	w, h;					/* idem */
	WORD	x, y;					/* idem */
	ULONG	abstime;			/* currently unused */
	ULONG	reltime;			/* time in jiffies from previous frame */
	UBYTE	interleave;		/* 0 = 2 frames. 1 = animbrush (no timing info) */
	UBYTE	pad0;
	ULONG	bits;					/* option bits for XOR compression */
	UBYTE	pad[16];			/* future expansion */
	};

#define PLT_RAM	0			/* preload as many as possible into RAM */
#define PLT_DISK	1		/* load all frames while playing */

#endif /* ANIM_IN_FIFF */



/* These routines must be called from the fiff.o ToolBox */

extern void FastInitIFFFrame( struct IFF_FRAME * );
extern ULONG FastOpenIFF( struct IFF_FRAME *, STRPTR );
extern void FastCloseIFF( struct IFF_FRAME * );
extern void FastWriteIFF( struct IFF_FRAME *, STRPTR, struct BitMap24 *, struct BitMap24 * );
extern void FastParseChunk( struct IFF_FRAME *, ULONG );
extern void FastDecodeBody( struct IFF_FRAME *, struct BitMap24 *, APTR );
extern ULONG FastScanFileType( STRPTR );

#endif /* MEDIAPOINT_FIFF_H */
