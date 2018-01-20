#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <graphics/videocontrol.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/macros.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <string.h>	// for setmem()
#include "stdio.h"
#include "disp_def.h"

/**** DEFINES ****/

#define RC_OK						0	/* No error */
#define RC_READ_ERROR		1	/* Error while reading file */
#define RC_CANT_FIND		2	/* Can't find file */
#define RC_NO_MEM				3	/* Not enough memory for operation */
#define RC_NO_CDDEVICE	4	/* Could not open cd/cdtv device */
#define RC_NO_WIN				5	/* Could not open window */
#define RC_NO_SC				6	/* Could not open screen */
#define RC_BAD_TRACK		7	/* Bad MPEG Track specification */
#define RC_FAILED				8	/* Something failed */

IMPORT struct Library *GfxBase;
IMPORT struct Library *IntuitionBase;
//IMPORT struct Library *GameBase;
IMPORT struct Custom far custom;

/**** FUNC DEFS ****/

VOID CloseDisplay( DISP_DEF * disp_def );
VOID CopRoutine( VOID );	// In CopRoutine.a

/**** GLOBALS ****/

struct Interrupt CopInt;
struct UCopList	*old_ucl, ucl;
BOOL CopIntAdded;
BYTE CopSigBit = -1;
ULONG CopSignal;
STATIC USHORT chip InvisiblePointer[]= {0x0000,0x0000,0x0000,0x0000,0x0000,0x0000};

/**** Data that gets passed into our copper interrupt routine. ****/

typedef	struct IntData
{
	struct Task *Task;
	ULONG SigMask;
} INTDATA;

INTDATA CopIntData;

/******** SetUpIntData() ********/
/*
 * Set up the data that will get sent into the copper interrupt.
 *
 */

BOOL SetUpIntData( INTDATA * intdata )
{
    if ( ( CopSigBit = AllocSignal(-1) ) != -1 ) {
	CopSignal = intdata->SigMask = (1L << CopSigBit);
	intdata->Task = FindTask( NULL );
	return( TRUE );
    }

    return( FALSE );

} // SetUpIntData()


/******** AddCopInt() ********/
/*
 * Add a copper interrupt that will signal us to call ChangeVPBitMap().
 *
 */

VOID AddCopInt( DISP_DEF * disp_def, struct UCopList * ucl, struct Interrupt * copint, INTDATA * intdata )
{
    setmem( ucl, sizeof ( *ucl ), 0 );

    CINIT ( ucl, 50 );
    CMOVE ( ucl, custom.intena, (INTF_SETCLR|INTF_COPER) );
    CWAIT ( ucl, 0, 0 );
//    CWAIT ( ucl, disp_def->Height, 0 );
    CMOVE ( ucl, custom.intreq, (INTF_SETCLR|INTF_COPER) );
    CEND ( ucl );

    old_ucl = disp_def->vp->UCopIns;
    disp_def->vp->UCopIns = ucl;

    copint->is_Node.ln_Type = NT_INTERRUPT;
    copint->is_Node.ln_Pri = 100;
    copint->is_Node.ln_Name = "";
    copint->is_Data = intdata;
    copint->is_Code = CopRoutine;

    AddIntServer( INTB_COPER, copint );

    CopIntAdded = TRUE;

    if ( disp_def->Flags & DISP_SCREEN ) {
	RethinkDisplay();
    }

} // AddCopInt()

/******** GetModeID() ********/
/*
 * Check that the ModeID can handle doublebuffering.
 * If not, then find the best ModeID that can.
 *
 */

ULONG GetModeID( DISP_DEF * disp_def )
{
    struct DisplayInfo	disp;
    ULONG		ModeID = disp_def->ModeID;

    if ( GetDisplayInfoData(NULL, (UBYTE *)&disp, sizeof(struct DisplayInfo), DTAG_DISP, ModeID) ) {

	if ( ( disp_def->Flags & DISP_DBUF) && !(disp.PropertyFlags & DIPF_IS_DBUFFER) ) {

	    ModeID = BestModeID(BIDTAG_NominalWidth, disp_def->Width,
	    BIDTAG_NominalHeight, disp_def->Height,
	    BIDTAG_Depth, disp_def->Depth,
	    /* Keep the DIPF_ properties of the original
	     * ModeID, but ensure we have DBUFFER
	     * properties too.
	     */
	    BIDTAG_SourceID, ModeID,
	    BIDTAG_DIPFMustHave, DIPF_IS_DBUFFER,
	    TAG_END);
	}
    }

    return( disp_def->ModeID = ModeID );

} // GetModeID()

/******** ScrWinOpen() ********/
/*
 * Open a screen and a window as defined by disp_def.
 *
 */

BOOL ScrWinOpen( DISP_DEF * disp_def )
{
    ULONG		flags,ErrorCode = 0;
    struct ColorSpec	initialcolors[] = 
			{
			    {0,0,0,0},
			    {-1,0,0,0}
			};

    GetModeID( disp_def );

    if ( disp_def->Flags & DISP_ALLOCBM ) {
	flags = BMF_CLEAR|BMF_DISPLAYABLE;

	if ( disp_def->Flags & DISP_INTERLEAVED )
	    flags |= BMF_INTERLEAVED;

	if (!(disp_def->bm[0] = (struct BitMap *)AllocBitMap( disp_def->Width, 
	 disp_def->Height, disp_def->Depth, flags, NULL )) ) {

	    disp_def->bm[1] = NULL;
	    return( RC_NO_MEM );
	}

	if ( disp_def->Flags & DISP_DBUF ) {

	    if (!(disp_def->bm[1] = (struct BitMap *)AllocBitMap( disp_def->Width, 
		disp_def->Height, disp_def->Depth, flags, NULL )) ) {

		FreeBitMap( disp_def->bm[0] );
		disp_def->bm[0] = NULL;

		return( RC_NO_MEM );
	    }
	}
    }

    if ( disp_def->screen = OpenScreenTags(NULL,

	SA_Top,		disp_def->Top,
	SA_Left,	disp_def->Left,
	SA_Width,	disp_def->Width,
	SA_Height,	disp_def->Height,
	SA_Depth,	disp_def->Depth,
	SA_BitMap,	disp_def->bm[0],
	SA_DisplayID,	disp_def->ModeID,
	SA_Interleaved,	(disp_def->Flags & DISP_INTERLEAVED) ? TRUE : FALSE,
	SA_ShowTitle,	FALSE,
	SA_Quiet,	TRUE,
	SA_Behind,	TRUE,
	SA_Colors,	initialcolors,
	SA_ErrorCode,	&ErrorCode,
	TAG_DONE) )
    {
	if ( disp_def->window = OpenWindowTags( NULL,
	    WA_Width,		disp_def->screen->Width,
	    WA_Height,		disp_def->screen->Height,
	    WA_IDCMP,		NULL,	//IDCMP_MOUSEBUTTONS,
	    WA_Flags,		BACKDROP | BORDERLESS | RMBTRAP,
	    WA_Activate,	TRUE,
	    WA_CustomScreen,	disp_def->screen,
	    TAG_DONE) ) {

	    if ( disp_def->Flags & DISP_NOPOINTER )
		SetPointer( disp_def->window, InvisiblePointer, 1, 1, 0, 0 );

	    disp_def->vp = &disp_def->screen->ViewPort;

	    if ( !(disp_def->Flags & DISP_DBUF) || (disp_def->dbuf = AllocDBufInfo( disp_def->vp )) ) {

		/*
		 * Add the copper interrupt.
		 */
		if ( !(disp_def->Flags & DISP_COP_INT) || SetUpIntData( &CopIntData ) ) {
		    if ( disp_def->Flags & DISP_COP_INT ) 
			AddCopInt( disp_def, &ucl, &CopInt, &CopIntData );

		    ScreenPosition( disp_def->screen, SPOS_ABSOLUTE, disp_def->Left, disp_def->Top, 0, 0);

		    ScreenToFront( disp_def->screen );

		    return( RC_OK );
		}
	    }
	    if ( disp_def->dbuf ) {
		FreeDBufInfo( disp_def->dbuf );
	    }
	    CloseWindow( disp_def->window );
	    disp_def->window = NULL;
	}

	CloseScreen( disp_def->screen );
	disp_def->screen = NULL;

	if ( disp_def->Flags & DISP_ALLOCBM ) {
	    FreeBitMap( disp_def->bm[0] );
	    FreeBitMap( disp_def->bm[1] );
	    disp_def->bm[0] = disp_def->bm[1] = NULL;
	}
	return( RC_NO_WIN );
    }

    if ( disp_def->Flags & DISP_ALLOCBM ) {
	FreeBitMap( disp_def->bm[0] );
	FreeBitMap( disp_def->bm[1] );
	disp_def->bm[0] = disp_def->bm[1] = NULL;
    }

    return( RC_NO_SC );

} // ScrWinOpen()

/******** OpenDisplay() ********/
/* 
 * Open a screen.
 *
 */

BOOL OpenDisplay( DISP_DEF * disp_def )
{
	return( ScrWinOpen( disp_def ) );
} // OpenDisplay()


/******** CloseDisplay() ********/
/* 
 *  Close screen.
 *
 */

VOID CloseDisplay( DISP_DEF * disp_def )
{

    if ( CopIntAdded ) {
	RemIntServer( INTB_COPER, &CopInt );
	disp_def->vp->UCopIns = old_ucl;
	RethinkDisplay();
	FreeCopList( ucl.FirstCopList );
	CopIntAdded = FALSE;
    }

    if ( CopSigBit != -1 ) {
	FreeSignal( CopSigBit );
	CopSigBit = -1;
    }

    if ( disp_def->dbuf )
	FreeDBufInfo( disp_def->dbuf );

    /* 
     * Close the screen and window.
     */
    if ( disp_def->Flags & DISP_SCREEN ) {
	if ( disp_def->window ) {
	    CloseWindow( disp_def->window );
	    disp_def->window = NULL;
	}

	if ( disp_def->screen ) {
	    CloseScreen( disp_def->screen );
	    disp_def->screen = NULL;
	}
    }

    /* 
     * If we allocated these bitmaps with AllocBitMap(),
     * free them with FreeBitMap().
     */
    if ( disp_def->Flags & DISP_ALLOCBM ) {
	if ( disp_def->bm[0] )
	    FreeBitMap( disp_def->bm[0] );

	if ( disp_def->bm[1] )
	    FreeBitMap( disp_def->bm[1] );
    }

} // CloseDisplay()


