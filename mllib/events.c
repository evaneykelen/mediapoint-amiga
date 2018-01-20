#include "mllib_includes.h"

extern struct MsgPort *capsport;

/**** functions ****/

/*******************************************************************/
/*
 *   PUBLIC FUNCTIONS
 *
 *******************************************************************/

/******** doStandardWait() ********/

void __saveds __asm LIBUA_doStandardWait(	register __a0 struct Window *waitWindow,
																					register __a1 struct EventData *CED)
{
ULONG signals, sigMask;

	sigMask = 1L << capsport->mp_SigBit;
	signals = Wait(sigMask);
	if ( (signals & sigMask) && (waitWindow->Flags & WFLG_WINDOWACTIVE) )
	{
		HandleIDCMP(CED, capsport);
		if (CED->Class == IDCMP_MOUSEBUTTONS && CED->Code==SELECTDOWN )
		{
			if ( LIBUA_CheckIfDragged(waitWindow,CED) )
				CED->Class = 0;
		}
	}
	else
	{
		CheckIfDepthWasClicked();
		CED->Class = 0;
		ActivateWindow(waitWindow);
	}
}

/******** doStandardWaitExtra() ********/

ULONG __saveds __asm LIBUA_doStandardWaitExtra(	register __a0 struct Window *waitWindow,
																								register __a1 struct EventData *CED,
																								register __d0 ULONG extraFlags )
{
ULONG signals, sigMask;

	sigMask = 1L << capsport->mp_SigBit;
	sigMask |= extraFlags;
	signals = Wait(sigMask);

	if ( signals & (1L << capsport->mp_SigBit) )
	{
		if ( waitWindow->Flags & WFLG_WINDOWACTIVE )
		{
			HandleIDCMP(CED, capsport);
			if (CED->Class == IDCMP_MOUSEBUTTONS && CED->Code==SELECTDOWN )
				LIBUA_CheckIfDragged(waitWindow,CED);
		}
		else
			CheckIfDepthWasClicked();

		//return(0L);
	}

	return( signals );
}

/******** CheckIfDragged() ********/

BOOL __saveds __asm LIBUA_CheckIfDragged(	register __a0 struct Window *window,
																					register __a1 struct EventData *CED )
{
struct Window *activeWindow;
ULONG signals, sigMask;
BOOL loop=TRUE, mouseMoved=FALSE, retval=FALSE;
struct IntuiMessage *message;
SHORT oriCX, oriCY, DiffX, DiffY, prevX, prevY, winX, winY, winW, winH, snif;
struct EventData copy_of_CED;

	Forbid();
	activeWindow = IntuitionBase->ActiveWindow;
	Permit();

	if ( activeWindow == window )
	{
		sigMask = 1L << capsport->mp_SigBit;

		if ( LIBUA_IsWindowOnLacedScreen(activeWindow) )
			snif=10;
		else
			snif=5;

		CopyMem(CED,&copy_of_CED,sizeof(struct EventData));

		if (	CED->MouseX>=0 && CED->MouseX<=activeWindow->Width &&
					CED->MouseY>=0 && CED->MouseY<=snif )
		{
			oriCX = activeWindow->MouseX;	//WScreen->MouseX;
			oriCY = activeWindow->MouseY;	//WScreen->MouseY;

			DiffX = oriCX - activeWindow->LeftEdge;
			DiffY = oriCY - activeWindow->TopEdge;

			prevX = activeWindow->LeftEdge;
			prevY = activeWindow->TopEdge;

			winX = prevX;
			winY = prevY;
			winW = activeWindow->Width;
			winH = activeWindow->Height;

			LIBUA_SwitchMouseMoveOn(activeWindow);

			DrawMarqueeBox(&activeWindow->WScreen->RastPort,winX,winY,winX+winW-1,winY+winH-1);

			/**** drag window ****/

			while(loop)
			{
				signals = Wait(sigMask);
				if (signals & sigMask)
				{
					mouseMoved=FALSE;
					while(message = (struct IntuiMessage *)GetMsg(capsport))
					{
						CED->Class		= message->Class;
						CED->Code			= message->Code;
						CED->MouseX		= activeWindow->MouseX;	//WScreen->MouseX;
						CED->MouseY		= activeWindow->MouseY;	//WScreen->MouseY;
						ReplyMsg((struct Message *)message);
	
						switch(CED->Class)
						{
							case IDCMP_MOUSEBUTTONS:
								if (CED->Code == SELECTUP)
									loop=FALSE;
								break;

							case IDCMP_MOUSEMOVE:
								mouseMoved=TRUE;
								break;
						}
					}
					if (mouseMoved)
					{
						DrawMarqueeBox(&activeWindow->WScreen->RastPort,winX,winY,winX+winW-1,winY+winH-1);
						winX = CED->MouseX - DiffX;
						winY = CED->MouseY - DiffY;
						if (winX < 0)
							winX = 0;
						if (winY < 0)
							winY = 0;
						if ( (winX + winW) >= activeWindow->WScreen->Width )
							winX = activeWindow->WScreen->Width-winW;
						if ( (winY + winH) >= activeWindow->WScreen->Height)
							winY = activeWindow->WScreen->Height-winH;
						DrawMarqueeBox(&activeWindow->WScreen->RastPort,winX,winY,winX+winW-1,winY+winH-1);
					}
				}
			}

			DrawMarqueeBox(&activeWindow->WScreen->RastPort,winX,winY,winX+winW-1,winY+winH-1);

			LIBUA_SwitchMouseMoveOff(activeWindow);

			MoveWindow(activeWindow, winX-prevX, winY-prevY);
			retval=TRUE;
		}

		CopyMem(&copy_of_CED,CED,sizeof(struct EventData));

		return(retval);
	}
	else
		return(FALSE);
}

/*******************************************************************/
/*
 *   PRIVATE FUNCTIONS
 *
 *******************************************************************/

/******** HandleIDCMP() ********/

void HandleIDCMP(struct EventData *CED, struct MsgPort *port)
{
struct IntuiMessage *message;
struct IntuiMessage localMsg;
static ULONG prev_Seconds = 0L;
static ULONG prev_Micros  = 0L;

	CED->menuNum 			= 0;
	CED->itemNum 			= 0;
	CED->itemFlags 		= 0;
	CED->Class 				= 0;
	CED->extraClass 	= 0;
	CED->Code 				= 0;
	CED->Qualifier 		= 0;
	CED->IAddress 		= NULL;
	CED->Seconds			= 0;
	CED->Micros 			= 0;
	CED->MouseX 			= 1;
	CED->MouseY 			= 1;

	while(message = (struct IntuiMessage *)GetMsg(port))
	{
		/**** make a working copy of the message port ****/

		CopyMem(message, &localMsg, sizeof(struct IntuiMessage));

		/**** reply to sender ****/

		ReplyMsg((struct Message *)message);

		/**** copy interesting fields ****/

		CED->Class				= localMsg.Class;
		CED->Code					= localMsg.Code;
		CED->Qualifier		= localMsg.Qualifier;
		if (CED->Class==IDCMP_RAWKEY ||
				CED->Class==IDCMP_GADGETDOWN ||
				CED->Class==IDCMP_GADGETUP)
			CED->IAddress		= (APTR)localMsg.IAddress;
		CED->Seconds		= localMsg.Seconds;
		CED->Micros			= localMsg.Micros;
		CED->MouseX			= localMsg.MouseX;
		CED->MouseY			= localMsg.MouseY;

		/**** check if user double clicked ****/

		if ( CED->Class == IDCMP_MOUSEBUTTONS && CED->Code == SELECTDOWN )
		{
			if ( MyDoubleClick(prev_Seconds, prev_Micros, CED) )
				CED->extraClass = DBLCLICKED;
			else
			{
				prev_Seconds = CED->Seconds;
				prev_Micros  = CED->Micros;
			}
		}

		if (CED->Class != IDCMP_MOUSEBUTTONS)
		{
			prev_Seconds = 0L;
			prev_Micros  = 0L;
		}

		if (CED->Class==IDCMP_GADGETDOWN || CED->Class==IDCMP_GADGETUP)
		{
			CED->extraClass = CED->Class;
			CED->Class = IDCMP_MOUSEBUTTONS;
		}
	}
}

/******** MyDoubleClick() ********/

BOOL MyDoubleClick(	ULONG prev_Seconds, ULONG prev_Micros,
										struct EventData *CED )
{
static SHORT MouseX=-1;
static SHORT MouseY=-1;
 
	if (MouseX==-1)
	{
		MouseX = CED->MouseX;
		MouseY = CED->MouseY;
		return(FALSE);
	}

	if (	AbsWORD((WORD)MouseX,(WORD)CED->MouseX)>4 ||
				AbsWORD((WORD)MouseY,(WORD)CED->MouseY)>4 )
	{
		MouseX = CED->MouseX;
		MouseY = CED->MouseY;
		return(FALSE);
	}

	if ( DoubleClick(prev_Seconds, prev_Micros, CED->Seconds, CED->Micros) )
		return(TRUE);

	return(FALSE);
}

/******** AbsWORD() ********/

int AbsWORD(int a, int b)
{
WORD c;

	c = a - b;
	if (c<0)
		c*=-1;
	return(c);
}

/******** DrawMarqueeBox() ********/

void DrawMarqueeBox(struct RastPort *rp, WORD x1, WORD y1, WORD x2, WORD y2)
{
	SetAPen(rp, 1L);
	SetDrMd(rp, JAM2|COMPLEMENT);
	//SafeSetWriteMask(rp, 0x7);

	Move(rp, (LONG)x1, (LONG)y1);
	Draw(rp, (LONG)x2, (LONG)y1);
	Draw(rp, (LONG)x2, (LONG)y2);
	Draw(rp, (LONG)x1, (LONG)y2);
	Draw(rp, (LONG)x1, (LONG)y1);

	SetDrMd(rp, JAM1);
	//SafeSetWriteMask(rp, 0xff);
}

/******** CheckIfDepthWasClicked() ********/

void CheckIfDepthWasClicked(void)
{
struct Window *window;
struct RasInfo *RI;

	Forbid();
	window = IntuitionBase->ActiveWindow;
	Permit();

	if (	window &&
				window->WScreen->MouseX > (window->WScreen->Width-20) &&
				window->WScreen->MouseY < MHEIGHT )
	{
		RI = window->WScreen->ViewPort.RasInfo;
		RI->RyOffset = 0;
		ScrollVPort( &(window->WScreen->ViewPort) );

		OpenWorkBench();
		ScreenToBack(window->WScreen);
	}
}

/******** E O F ********/
