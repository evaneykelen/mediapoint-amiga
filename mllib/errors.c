#include "mllib_includes.h"

#define VERSI0N "\0$VER: 1.3"          
static UBYTE *vers = VERSI0N;

/**** defines ****/

#define MAXARGSTRLEN 512	// see also globalallocs.c

/**** externals ****/

extern struct MsgPort *capsport;

/**** globals ****/

struct GadgetRecord Warn_GR[] =
{
  0,   0, 320,  95, 0, NULL,	0,												DIMENSIONS, NULL,
  0,	 0,	319,  94, 0, NULL,	0,												DBL_BORDER_REGION, NULL,
  7,  77,  89,  90, 0, NULL,	0,												BUTTON_GADGET, NULL,	// LEFT
230,  77, 312,  90, 0, NULL,	0,												BUTTON_GADGET, NULL,	// RIGHT
-1
};

/**** functions ****/

/*******************************************************************/
/*
 *   PUBLIC FUNCTIONS
 *
 *******************************************************************/

/******** WarnUser() ********/

void __saveds __asm LIBUA_WarnUser(	register __d0 int EC )
{
TEXT errorStr[256];
struct Window *wdw;

	Forbid();
	wdw = IntuitionBase->ActiveWindow;
	Permit();

	sprintf(errorStr, "Internal error %d", EC);

	if (wdw != NULL)
		LIBUA_OpenGenericWindow(wdw, TRUE, FALSE, "OK", NULL, QUESTION_ICON,
														errorStr, TRUE, Warn_GR);
}

/******** OpenGenericWindow() ********/
/*
 * if icon==0 no icon is rendered.
 * if icon==1 a question mark is rendered.
 * if icon==2 a exclamation point is rendered.
 *
 * ok -> show OK button, cancel -> show Cancel button
 * 
 * if key == TRUE --> respond to return and escape key pressings.
 *
 *	#define NO_ICON 					0
 *	#define QUESTION_ICON 		1
 *	#define EXCLAMATION_ICON	2
 *
 */

BOOL __saveds __asm LIBUA_OpenGenericWindow(register __a0	struct Window *window,
																						register __d0 BOOL ok,
																						register __d1 BOOL cancel,
																						register __a1 STRPTR okText,
																						register __a2 STRPTR cancelText,
																						register __d2 int icon,
																						register __a3 STRPTR messageText,
																						register __d3 BOOL key,
																						register __a5 struct GadgetRecord *GR)
{
int ID,pos;
struct Window *reqWindow;
struct Screen *screen;
BOOL lace=FALSE;
struct EventData CED;
struct MsgPort *port;
struct List *list;
struct Node *node;
struct RendezVousRecord *rvrec;

	/**** find port ****/

	if ( capsport != NULL )
	{
		port = (struct MsgPort *)FindPort(ML_RENDEZ_VOUS);
		if (port == NULL)
			return(FALSE);
		list = &(port->mp_MsgList);
		node = list->lh_Head;
		rvrec = (struct RendezVousRecord *)node->ln_Name;
	}

	screen = window->WScreen;
	if (screen==NULL)
		return(FALSE);

	if ( GR==NULL )
		GR = Warn_GR;

	ScreenToFront(screen);

	if ( screen->ViewPort.Modes & LACE )
		lace=TRUE;

	reqWindow = (struct Window *)LIBUA_OpenRequesterWindow(window, GR, STDCOLORS);
	if ( reqWindow==NULL)
		return(FALSE);

	LIBUA_DrawGadget(reqWindow, &GR[1]);	// border

	if (ok)
	{
		LIBUA_DrawGadget(reqWindow, &GR[2]);		// left button (OK)
		LIBUA_DrawText(reqWindow, &GR[2], okText);
		if (key)
			LIBUA_DrawDefaultButton(reqWindow, &GR[2]);
	}

	if (cancel)
	{
		LIBUA_DrawGadget(reqWindow, &GR[3]);		// right button (OK)
		LIBUA_DrawText(reqWindow, &GR[3], cancelText);
	}

	pos=0;

	if ( reqWindow->RPort->BitMap->Depth!=1 )
	{
		if ( icon==NO_ICON )
		{
			pos = 17;
		}
		else if ( icon==QUESTION_ICON )
		{
			pos = 57;
			if ( reqWindow->WScreen->ViewPort.Modes & LACE )
			{
				LIBUA_PutImageInRastPort(	rvrec->gfxBM, GFX_LARGE_QUEST_X, GFX_LARGE_QUEST_Y, GFX_W,
																	reqWindow->RPort, 10, 5,
																	GFX_LARGE_QUEST_W, GFX_LARGE_QUEST_H);
			}
			else
			{
				LIBUA_PutImageInRastPort(	rvrec->gfxBM, GFX_SMALL_QUEST_X, GFX_SMALL_QUEST_Y, GFX_W,
																	reqWindow->RPort, 10, 5,
																	GFX_SMALL_QUEST_W, GFX_SMALL_QUEST_H);
			}
		}
		else if (icon==EXCLAMATION_ICON)
		{
			pos = 57;
			if ( reqWindow->WScreen->ViewPort.Modes & LACE )
			{
				LIBUA_PutImageInRastPort(	rvrec->gfxBM, GFX_LARGE_EXCLA_X, GFX_LARGE_EXCLA_Y, GFX_W,
																	reqWindow->RPort, 10, 5,
																	GFX_LARGE_EXCLA_W, GFX_LARGE_EXCLA_H);
			}
			else
			{
				LIBUA_PutImageInRastPort(	rvrec->gfxBM, GFX_SMALL_EXCLA_X, GFX_SMALL_EXCLA_Y, GFX_W,
																	reqWindow->RPort, 10, 5,
																	GFX_SMALL_EXCLA_W, GFX_SMALL_EXCLA_H);
			}
		}
	}

	LIBUA_printSeveralLines(reqWindow, &GR[0],
													pos, reqWindow->RPort->TxBaseline+5,
													GR[0].x2-15, GR[0].y2-5, messageText);

	/**** normal case event handler ****/

	while(1)
	{
		LIBUA_doStandardWait( reqWindow, &CED );
		if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = LIBUA_CheckGadgetList(reqWindow, GR, &CED);
			switch(ID)
			{
				case 2:
do_ok:
					if (ok)
					{
						LIBUA_HiliteButton(reqWindow, &GR[2]);
						LIBUA_CloseRequesterWindow(reqWindow, STDCOLORS);
						return(TRUE);
					}
					break;

				case 3:
do_cancel:
					if (cancel)
					{
						LIBUA_HiliteButton(reqWindow, &GR[3]);
						LIBUA_CloseRequesterWindow(reqWindow, STDCOLORS);
						return(FALSE);
					}
					break;
			}
		}
		else if (key && CED.Class==IDCMP_RAWKEY)
		{
			if (cancel && CED.Code==RAW_ESCAPE)
				goto do_cancel;
			else if (ok && CED.Code==RAW_RETURN)
				goto do_ok;
		}
	}
}

/******** OpenMessagePanel() ********/

struct Window * __saveds __asm LIBUA_OpenMessagePanel(register __a0	struct Window *window,
																											register __a1 STRPTR messageText)
{
struct Window *reqWindow;
struct Screen *screen;

	screen = window->WScreen;
	if (screen==NULL || messageText==NULL)
		return(NULL);

	ScreenToFront(screen);

	reqWindow = (struct Window *)LIBUA_OpenRequesterWindow(window, Warn_GR, STDCOLORS);
	if ( reqWindow==NULL)
		return(NULL);

	LIBUA_DrawGadget(reqWindow, &Warn_GR[1]);	// border

	LIBUA_printSeveralLines(reqWindow, &Warn_GR[0],
													15, reqWindow->RPort->TxBaseline+5,
													Warn_GR[0].x2-15, Warn_GR[0].y2-5, messageText);

	return(reqWindow);
}

/******** CloseMessagePanel() ********/

void __saveds __asm LIBUA_CloseMessagePanel(register __a0 struct Window *window)
{
	LIBUA_CloseRequesterWindow(window, STDCOLORS);
}

/******** PutImageInRastPort() ********/

void __saveds __asm LIBUA_PutImageInRastPort(	register __a0 struct BitMap *gfxBitMap,
																							register __d0 WORD srcX,
																							register __d1 WORD srcY,
																							register __d2 WORD srcW,
																							register __a1 struct RastPort *dstRp,
																							register __d3 WORD dstX,
																							register __d4 WORD dstY,
																							register __d5 WORD dstW,
																							register __d6 WORD dstH )
{
int pen;

	SetAPen(dstRp,1L);	// black
	BltTemplate(	gfxBitMap->Planes[0]+(srcW/8)*srcY+(srcX/8), srcX%16, srcW/8,
								dstRp, dstX, dstY, dstW, dstH);

	if ( dstRp->BitMap->Depth==1 )
		pen=0;	
	else
		pen=2;	// white

	SetAPen(dstRp,pen);
	BltTemplate(	gfxBitMap->Planes[1]+(srcW/8)*srcY+(srcX/8), srcX%16, srcW/8,
								dstRp, dstX, dstY, dstW, dstH);
}

/******** E O F ********/
