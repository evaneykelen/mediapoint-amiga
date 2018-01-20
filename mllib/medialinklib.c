#include "mllib_includes.h"
#include "pascal:include/fiff.h"

/**** defines ****/

#define KEYBUFSIZE 20L
#define DONTCARE_OUTSIDE 1
#define CARE_OUTSIDE 2

#define RAW_SPACE				0x40
#define RAW_BACKSPACE		0x41
#define RAW_TAB					0x42
#define RAW_ENTER				0x43
#define RAW_RETURN			0x44
#define RAW_ESCAPE			0x45
#define RAW_DELETE			0x46
#define RAW_CURSORUP		0x4C
#define RAW_CURSORDOWN	0x4D
#define RAW_CURSORRIGHT	0x4E
#define RAW_CURSORLEFT	0x4F
#define RAW_HELP				0x5F
#define RAW_F10					0x59

/**** globals ****/

struct MsgPort *capsport;

/**** functions ****/

/*******************************************************************/
/*
 *   PUBLIC FUNCTIONS
 *
 *******************************************************************/

/******** LIBUA_InitStruct() ********/

void __saveds __asm LIBUA_InitStruct(register __a0 struct UserApplicInfo *UAI)
{
	UAI->userScreen				= NULL;
	UAI->medialinkScreen	= NULL;
	UAI->userWindow				= NULL;
	UAI->screenWidth			= 0;
	UAI->screenHeight			= 0;
	UAI->screenDepth			= 0;
	UAI->screenModes			= 0;
	UAI->small_TF					= NULL;
	UAI->large_TF					= NULL;
	UAI->windowX					= 0;
	UAI->windowY					= 0;
	UAI->windowWidth			= 0;
	UAI->windowHeight			= 0;
	UAI->windowModes			= 0;
	UAI->IB								= NULL;
	UAI->wflg							= 0L;
}

/******** LIBUA_OpenScreen() ********/

void __saveds __asm LIBUA_OpenScreen(register __a0 struct UserApplicInfo *UAI)
{
ULONG modes;
struct MsgPort *port;
struct List *list;
struct Node *node;
struct RendezVousRecord *rvrec;
struct TagItem Tags[20];
BOOL query;
Tag ti_Tag;
ULONG ti_Data;
struct Rectangle dclip;
int pos;

	/**** find port ****/

	if ( capsport != NULL )
	{
		port = (struct MsgPort *)FindPort(ML_RENDEZ_VOUS);
		if (port == NULL)
			return;
		list = &(port->mp_MsgList);
		node = list->lh_Head;
		rvrec = (struct RendezVousRecord *)node->ln_Name;
	}

#if 0
	if (	((rvrec->capsprefs->ScriptScreenModes & SUPER_KEY) == SUPER_KEY) ||
				((rvrec->capsprefs->ScriptScreenModes & SUPERLACE_KEY) == SUPERLACE_KEY) )
	{
		if ( rvrec->capsprefs->scriptMonitorID==DBLPAL_MONITOR_ID )
			modes = PAL_MONITOR_ID | HIRES;
		else if ( rvrec->capsprefs->scriptMonitorID==DBLNTSC_MONITOR_ID )
			modes = NTSC_MONITOR_ID | HIRES;
		else
			modes = rvrec->capsprefs->scriptMonitorID | HIRES;
	}
	else
		modes = rvrec->capsprefs->scriptMonitorID | HIRES;
#endif

	modes = rvrec->capsprefs->scriptMonitorID | rvrec->capsprefs->ScriptScreenModes;

	if ( ModeNotAvailable(modes) )
		modes = rvrec->capsprefs->scriptMonitorID;

	if ( ModeNotAvailable(modes) )
	{
		if ( rvrec->capsprefs->PagePalNtsc==PAL_MODE )
			modes = PAL_MONITOR_ID | HIRES;
		else
			modes = NTSC_MONITOR_ID | HIRES;
	}

	query = QueryOverscan(modes, &dclip, OSCAN_TEXT);

	if ( query )
	{
		ti_Tag = SA_DClip;
		ti_Data = (ULONG)&dclip;

		if ( (dclip.MaxX-dclip.MinX+1) > rvrec->capsprefs->ScriptScreenWidth )
		{
			pos = ((dclip.MaxX - dclip.MinX + 1) - rvrec->capsprefs->ScriptScreenWidth) / 2;
			dclip.MinX += pos;
			dclip.MaxX = dclip.MinX + rvrec->capsprefs->ScriptScreenWidth - 1; 
		}
	}
	else
	{
		ti_Tag = TAG_END;
		ti_Data = 0L;
	}

	Tags[ 0].ti_Tag		=	SA_Depth;
	Tags[ 0].ti_Data	=	UAI->screenDepth;
	Tags[ 1].ti_Tag		=	SA_Font;
	Tags[ 1].ti_Data	=	(ULONG)&UAI->small_TA;
	Tags[ 2].ti_Tag		=	SA_DisplayID;
	Tags[ 2].ti_Data	=	modes;
	Tags[ 3].ti_Tag		=	SA_ShowTitle;
	Tags[ 3].ti_Data	=	FALSE;
	Tags[ 4].ti_Tag		=	SA_DetailPen;
	Tags[ 4].ti_Data	=	0;
	Tags[ 5].ti_Tag		=	SA_BlockPen;
	Tags[ 5].ti_Data	=	1;
	Tags[ 6].ti_Tag		=	SA_Type;
	Tags[ 6].ti_Data	=	CUSTOMSCREEN;
	Tags[ 7].ti_Tag		=	SA_Title;
	Tags[ 7].ti_Data	=	NULL;
	Tags[ 8].ti_Tag		=	SA_Quiet;
	Tags[ 8].ti_Data	=	TRUE;
	Tags[ 9].ti_Tag		= ti_Tag;
	Tags[ 9].ti_Data	= ti_Data;
	Tags[10].ti_Tag		=	TAG_END;

	UAI->userScreen = OpenScreenTagList(NULL,Tags);
}

/******** LIBUA_OpenWindow() ********/

void __saveds __asm LIBUA_OpenWindow(register __a0 struct UserApplicInfo *UAI)
{
struct NewWindow NW;
struct Window *window;
struct Screen *screen;
LONG Lock;
struct MsgPort *port;
struct List *list;
struct Node *node;
struct RendezVousRecord *rvrec;

	/**** find port ****/

	if ( capsport != NULL )
	{
		port = (struct MsgPort *)FindPort(ML_RENDEZ_VOUS);
		if (port == NULL)
			return;
		list = &(port->mp_MsgList);
		node = list->lh_Head;
		rvrec = (struct RendezVousRecord *)node->ln_Name;
	}

	/**** fill new window struct ****/

	NW.Width				= UAI->windowWidth;
	NW.Height				= UAI->windowHeight;
	NW.DetailPen		= 7;
	NW.BlockPen			= 1;
	NW.Title				= NULL;
	NW.Flags				= UAI->wflg | WFLG_RMBTRAP;
	if ( capsport==NULL )
		NW.IDCMPFlags	= IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY |
										IDCMP_GADGETDOWN | IDCMP_GADGETUP;	// | IDCMP_NEWSIZE;
	else
		NW.IDCMPFlags	= 0L;	// else a message port is created
	NW.Type					= CUSTOMSCREEN;	// may be changed below !
	NW.FirstGadget	= NULL;
	NW.CheckMark		= NULL;
	if (UAI->windowModes == 1)				// medialink SCREEN
		NW.Screen			= UAI->medialinkScreen;
	else if (UAI->windowModes == 2)		// USER SCREEN
		NW.Screen			= UAI->userScreen;
	else if (UAI->windowModes == 3)		// FIRST SCREEN
	{
		Lock = LockIBase(0L);
		NW.Screen			= UAI->IB->FirstScreen;
		UnlockIBase(Lock);
		screen = NW.Screen;
		if ( screen && screen->Title && strncmp(screen->Title, "Workbench", 9) == 0)
			NW.Type			= WBENCHSCREEN;
	}

	screen = NW.Screen;

	if (UAI->windowX==-1)
		NW.LeftEdge = (screen->Width-UAI->windowWidth)/2;
	else
		NW.LeftEdge = UAI->windowX;

	if (UAI->windowY==-1)
		NW.TopEdge = (screen->Height-UAI->windowHeight)/2;
	else
		NW.TopEdge = UAI->windowY;

	NW.BitMap				= NULL;
	NW.MinWidth			= NW.Width;
	NW.MinHeight		= NW.Height;
	NW.MaxWidth			= NW.Width;
	NW.MaxHeight		= NW.Height;

	/**** open window ****/

	window = (struct Window *)OpenWindow(&NW);
	UAI->userWindow = window;

	/**** set up the communications port ****/

	if (window!=NULL)
	{
		if ( capsport != NULL )
		{
			window->UserPort = rvrec->capsport;
			ModifyIDCMP(window, IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY |
													IDCMP_GADGETDOWN | IDCMP_GADGETUP);	// | IDCMP_NEWSIZE);
		}

		my_SetAPen(window, AREA_PEN);
		if ( UAI->wflg & WFLG_GIMMEZEROZERO )
			RectFill(window->RPort,0,0,window->Width-1,window->Height-1);
		else
			RectFill(	window->RPort,
								window->BorderLeft, window->BorderTop,
								window->Width-window->BorderRight-1,
								window->Height-window->BorderBottom-1);

		if (window->WScreen->ViewPort.Modes & LACE)
			SetFont(window->RPort, UAI->large_TF);
		else
			SetFont(window->RPort, UAI->small_TF);
	}
}

/******** LIBUA_CloseScreen() ********/

void __saveds __asm LIBUA_CloseScreen(register __a0 struct UserApplicInfo *UAI)
{
	if (UAI->userScreen != NULL)
	{
		CloseScreen(UAI->userScreen);
		UAI->userScreen = NULL;
	}
}

/******** LIBUA_CloseWindow() ********/

void __saveds __asm LIBUA_CloseWindow(register __a0 struct UserApplicInfo *UAI)
{
	if (UAI->userWindow != NULL)
	{
		if ( capsport!=NULL )
			LIBUA_CloseWindowSafely(UAI->userWindow);
		else
			CloseWindow(UAI->userWindow);
		UAI->userWindow = NULL;
	}
}

/******** LIBUA_HostScreenPresent() ********/

BOOL __saveds __asm LIBUA_HostScreenPresent(register __a0 struct UserApplicInfo *UAI)
{
struct Screen *screen;
int j;
LONG Lock;

	/**** scan all screens ****/

	Lock = LockIBase(0L);
	screen = UAI->IB->FirstScreen;
	j=0;
	while(j<32)
	{
		if (screen->UserData!=NULL && (strcmp(screen->UserData, "ScriptEditor")==0) )
			break;
		screen = screen->NextScreen;
		if (screen==NULL)
			break;
	}
	UnlockIBase(Lock);

	if (screen==NULL)
		return(FALSE);

	/**** fill UAI with data ****/

	UAI->medialinkScreen = screen;

	return(TRUE);
}

/******** DrawGadget() ********/

void __saveds __asm LIBUA_DrawGadget(	register __a0 struct Window *window,
																			register __a1 struct GadgetRecord *GR)
{
	switch(GR->type)
	{
		case BUTTON_GADGET: UA_DrawButton(window, GR);
												break;
		case DATE_GADGET:
		case TIME_GADGET:
		case INTEGER_GADGET:
		case STRING_GADGET:
		case SPECIAL_STRING_GADGET:
												UA_DrawString(window, GR);
												break;
		case CYCLE_GADGET:	UA_DrawCycle(window, GR);
												break;
		case RADIO_GADGET:	UA_DrawRadio(window, GR);
												break;
		case CHECK_GADGET:	UA_DrawCheck(window, GR);
												break;
		case HIBOX_REGION:	LIBUA_DrawTwoColorBox(window, GR);
												break;
		case LOBOX_REGION:	LIBUA_DrawTwoColorBox(window, GR);
												break;
		case BORDER_REGION: UA_DrawABorder(window, GR);
												break;
		case DBL_BORDER_REGION:
												UA_DrawABorder(window, GR);
												break;
		case TEXT_REGION:		LIBUA_DrawText(window, GR, NULL);
												break;
		case TEXT_LEFT:			LIBUA_DrawSpecialGadgetText(window, GR, GR->txt, SPECIAL_TEXT_LEFT);
												break;
		case TEXT_RIGHT:		LIBUA_DrawSpecialGadgetText(window, GR, GR->txt, SPECIAL_TEXT_RIGHT);
												break;
		case DOTTED_LINE:
		case LO_LINE:				UA_DrawALine(window, GR);
												break;
		case COMBOBOX_REGION:
												LIBUA_DrawComboBox(window, GR);
												break;
	}
}

/******** DrawGadgetList() ********/

void __saveds __asm LIBUA_DrawGadgetList(	register __a0 struct Window *window,
																					register __a1 struct GadgetRecord *GR)
{
int i=0;

	while(GR[i].x1 != -1)
	{
		LIBUA_DrawGadget(window, &GR[i]);
		i++;
	}
}

/******** CheckGadget() ********/

int __saveds __asm LIBUA_CheckGadget(	register __a0 struct Window *window,
																			register __a1 struct GadgetRecord *FirstGR,
																			register __a2 struct GadgetRecord *GR,
																			register __a3 struct EventData *ED)
{
int len, extra_x, extra_y;

	if (GR->type < 10)
	{
		extra_x=0;
		extra_y=0;
		if (GR->type == RADIO_GADGET || GR->type == CHECK_GADGET)
		{
			if (GR->txt != NULL)
			{
				len = strlen(GR->txt);
				extra_x = TextLength(window->RPort, GR->txt, len);
				extra_y = 3;
			}
		}
		if (	(ED->MouseX > GR->x1) && (ED->MouseX < (GR->x2+extra_x)) &&
					(ED->MouseY > (GR->y1-extra_y)) && (ED->MouseY < (GR->y2+extra_y)) )
		{
			return(UA_GetGadgetID(window, FirstGR, GR, NULL));
		}
	}

	return(-1);
}

/******** CheckGadgetList() ********/

int __saveds __asm LIBUA_CheckGadgetList(	register __a0 struct Window *window,
																					register __a1 struct GadgetRecord *FirstGR,
																					register __a2 struct EventData *ED)
{
int i=0;
struct GadgetRecord *GR_ptr;
int len, extra_x, extra_y;

	GR_ptr = FirstGR;
	while(GR_ptr->x1 != -1)
	{
		if (GR_ptr->type < 10)
		{
			extra_x=0;
			extra_y=0;
			if (GR_ptr->type == RADIO_GADGET || GR_ptr->type == CHECK_GADGET)
			{
				if (GR_ptr->txt != NULL)
				{
					len = strlen(GR_ptr->txt);
					extra_x = TextLength(window->RPort, GR_ptr->txt, len);
					extra_y = 3;
				}
			}
			if (	(ED->MouseX > GR_ptr->x1) &&
						(ED->MouseX < (GR_ptr->x2+extra_x)) &&
						(ED->MouseY > GR_ptr->y1) &&
						(ED->MouseY < GR_ptr->y2) )
			{
				return(UA_GetGadgetID(window, FirstGR, &FirstGR[i], NULL));
			}
		}
		i++;
		GR_ptr = &FirstGR[i];
	}

	return(-1);
}

/******** ProcessCycleGadget() ********/
/*
 * returns +1 for up and -1 for down
 *
 */

int __saveds __asm LIBUA_ProcessCycleGadget(	register __a0 struct Window *window,
																							register __a1 struct GadgetRecord *GR,
																							register __a2 struct EventData *ED)
{
int up_x1, up_y1, up_x2, up_y2;
int left_x1, left_y1, left_x2, left_y2;
int right_x1, right_y1, right_x2, right_y2;
int half_width, retval=0, hold=0, add=1, mayadd, i, tick;
struct CycleRecord *CR_ptr;
STRPTR CR_textptr;
ULONG signals, signalMask, IDCMP_Flags;
BOOL loop, cycling=TRUE;
struct IntuiMessage *message;

	LIBUA_HiliteButton(window, GR);

	half_width = (GR->x2-(GR->x1+18))/2;

	up_x1 = GR->x1;
	up_y1 = GR->y1;
	up_x2 = GR->x1+17;
	up_y2 = GR->y2;

	left_x1 = GR->x1+18;
	left_y1 = GR->y1;
	left_x2 = GR->x1+18+half_width+1;
	left_y2 = GR->y2;

	right_x1 = GR->x1+18+half_width;
	right_y1 = GR->y1;
	right_x2 = GR->x2;
	right_y2 = GR->y2;

	IDCMP_Flags = window->IDCMPFlags;
	IDCMP_Flags |= IDCMP_INTUITICKS;
	ModifyIDCMP(window, IDCMP_Flags);

	CR_ptr = (struct CycleRecord *)GR->ptr;

	if ( CR_ptr->number>20 )
		mayadd = 2;
	else
		mayadd = 1;

	while(cycling)
	{
		if (	((ED->MouseX > up_x1) && (ED->MouseX < up_x2) &&
					(ED->MouseY > up_y1) &&	(ED->MouseY < up_y2)) ||
					((ED->MouseX > right_x1) && (ED->MouseX < right_x2) &&
					(ED->MouseY > right_y1) && (ED->MouseY < right_y2)) )
		{
			if (ED->Qualifier&IEQUALIFIER_LSHIFT || ED->Qualifier&IEQUALIFIER_RSHIFT)
			{
				CR_ptr->active = CR_ptr->active - add;	
				retval=-1;
				if (CR_ptr->active<0)
				{
					CR_ptr->active=CR_ptr->number-1;
				}

if ((CR_ptr->active==CR_ptr->number-1) && hold>8)
{
	CR_ptr->active=0;
	cycling=FALSE;
	loop=FALSE;
}

			}
			else
			{
				CR_ptr->active = CR_ptr->active + add;
				retval=1;
				if (CR_ptr->active>=CR_ptr->number)
				{
					CR_ptr->active=0;
				}

if ((CR_ptr->active==0) && hold>8)
{
	CR_ptr->active=CR_ptr->number-1;
	cycling=FALSE;
	loop=FALSE;
}

			}
			if ( CR_ptr->ptr )
			{
				CR_textptr = CR_ptr->ptr;
				UA_DrawCycleText(window, GR, &CR_textptr[CR_ptr->active*CR_ptr->width]);
			}
		}
		else if (	(ED->MouseX > left_x1) && (ED->MouseX < left_x2) &&
							(ED->MouseY > left_y1) &&	(ED->MouseY < left_y2) )
		{
			CR_ptr->active = CR_ptr->active - add;
			retval=-1;
			if (CR_ptr->active<0)
			{
				CR_ptr->active=CR_ptr->number-1;
			}

if ((CR_ptr->active==CR_ptr->number-1) && hold>8)
{
	CR_ptr->active=0;
	cycling=FALSE;
	loop=FALSE;
}

			if ( CR_ptr->ptr )
			{
				CR_textptr = CR_ptr->ptr;
				UA_DrawCycleText(window, GR, &CR_textptr[CR_ptr->active*CR_ptr->width]);
			}
		}

		signalMask = (1L << capsport->mp_SigBit);

		if ( CR_ptr->ptr )
		{
			LIBUA_InvertButton(window, GR);
			loop=TRUE;
		}
		else
		{
			loop=FALSE;
			cycling=FALSE;
		}

		if ( hold>0 && mayadd==1 )
			for(i=0; i<12; i++)
				WaitTOF();

		tick=0;
		while(loop)
		{
			signals = Wait(signalMask);
			if (signals & signalMask)
			{
				while(message = (struct IntuiMessage *)GetMsg(capsport))
				{
					ED->Class	= message->Class;
					ED->Code	= message->Code;
					ReplyMsg((struct Message *)message);
						
					if ( (*((char *)0xbfe001))&64 )
					{
						loop=FALSE;
						cycling=FALSE;
					}

					if ( ED->Class==IDCMP_MOUSEBUTTONS && ED->Code==SELECTUP )
					{
						loop=FALSE;
						cycling=FALSE;
					}
					else if ( ED->Class==IDCMP_INTUITICKS )
					{
						hold++;
						if ( hold>3 )
						{
							loop=FALSE;
							if (hold>8)
								add=mayadd;
						}
					}
				}
			}
		}

		if ( CR_ptr->ptr )
			LIBUA_InvertButton(window, GR);
	}

	IDCMP_Flags = IDCMP_Flags & ~IDCMP_INTUITICKS;
	ModifyIDCMP(window, IDCMP_Flags);

	return(retval);
}

/******** HiliteButton() ********/

void __saveds __asm LIBUA_HiliteButton(	register __a0 struct Window *window,
																				register __a1 struct GadgetRecord *GR)
{
int i;

	if (GR->type==RADIO_GADGET)
	{
		LIBUA_InvertRadioButton(window, GR);
		for(i=0; i<10; i++)
			WaitTOF();
		LIBUA_InvertRadioButton(window, GR);
	}
	else if (GR->type==CHECK_GADGET)
	{
		LIBUA_InvertCheckButton(window, GR);
		for(i=0; i<10; i++)
			WaitTOF();
		LIBUA_InvertCheckButton(window, GR);
	}
	else if (GR->type==CYCLE_GADGET)	/* faster highlighting */
	{
		LIBUA_InvertRadioButton(window, GR);
		for(i=0; i<2; i++)
			WaitTOF();
		LIBUA_InvertRadioButton(window, GR);
	}
	else
	{
		LIBUA_InvertButton(window, GR);
		for(i=0; i<10; i++)
			WaitTOF();
		LIBUA_InvertButton(window, GR);
	}
}

/******** InvertButton() ********/

void __saveds __asm LIBUA_InvertButton(	register __a0 struct Window *window,
																				register __a1 struct GadgetRecord *GR)
{
	if (GR->type==RADIO_GADGET)
		LIBUA_InvertRadioButton(window,GR);
	else if (GR->type==CHECK_GADGET)
		LIBUA_InvertCheckButton(window,GR);
	else
	{
		SafeSetWriteMask(window->RPort, 0x3);
		SetDrMd(window->RPort, COMPLEMENT | JAM2);

		if (window->WScreen->ViewPort.Modes & LACE)
			MyRectFill(window->RPort, (LONG)GR->x1+2, (LONG)GR->y1+2,
																(LONG)GR->x2-2, (LONG)GR->y2-2);
		else
			MyRectFill(window->RPort, (LONG)GR->x1+2, (LONG)GR->y1+1,
															(LONG)GR->x2-2, (LONG)GR->y2-1);

		SetDrMd(window->RPort, JAM2);
		SafeSetWriteMask(window->RPort, 0xff);
	}
}

/******** HiliteRadioButton() ********/

void __saveds __asm LIBUA_HiliteRadioButton(register __a0 struct Window *window,
																						register __a1 struct GadgetRecord *GR)
{
int i;
	LIBUA_InvertRadioButton(window, GR);
	for(i=0; i<10; i++)
		WaitTOF();
	LIBUA_InvertRadioButton(window, GR);
}

/******** InvertRadioButton() ********/

void __saveds __asm LIBUA_InvertRadioButton(register __a0 struct Window *window,
																						register __a1 struct GadgetRecord *GR)
{
	SafeSetWriteMask(window->RPort, 0x3);
	SetDrMd(window->RPort, COMPLEMENT | JAM2);

	if (window->WScreen->ViewPort.Modes & LACE)
	{
		Move(window->RPort, (LONG)GR->x1+5, (LONG)GR->y1+3);
		Draw(window->RPort, (LONG)GR->x2-5, (LONG)GR->y1+3);
		MyRectFill(window->RPort, (LONG)GR->x1+4, (LONG)GR->y1+4,
															(LONG)GR->x2-4, (LONG)GR->y2-4);
		Move(window->RPort, (LONG)GR->x1+5, (LONG)GR->y2-3);
		Draw(window->RPort, (LONG)GR->x2-5, (LONG)GR->y2-3);
	}
	else
	{
		Move(window->RPort, (LONG)GR->x1+5, (LONG)GR->y1+2);
		Draw(window->RPort, (LONG)GR->x2-5, (LONG)GR->y1+2);
		MyRectFill(window->RPort, (LONG)GR->x1+4, (LONG)GR->y1+3,
															(LONG)GR->x2-4, (LONG)GR->y2-3);
		Move(window->RPort, (LONG)GR->x1+5, (LONG)GR->y2-2);
		Draw(window->RPort, (LONG)GR->x2-5, (LONG)GR->y2-2);
	}

	SetDrMd(window->RPort, JAM2);
	SafeSetWriteMask(window->RPort, 0xff);
}

/******** InvertCheckButton() ********/

void __saveds __asm LIBUA_InvertCheckButton(register __a0 struct Window *window,
																						register __a1 struct GadgetRecord *GR)
{
int add=0;

	SafeSetWriteMask(window->RPort, 0x3);
	SetDrMd(window->RPort, COMPLEMENT | JAM2);

	if ( GR->type==BUTTON_GADGET )	// especially made for 'check box-like' button gadgets
		add=1;

	if (window->WScreen->ViewPort.Modes & LACE)
		MyRectFill(window->RPort, (LONG)GR->x1+4-add, (LONG)GR->y1+4,
															(LONG)GR->x2-4+add, (LONG)GR->y2-4);
	else
		MyRectFill(window->RPort, (LONG)GR->x1+4-add, (LONG)GR->y1+2,
															(LONG)GR->x2-4+add, (LONG)GR->y2-2);

	SetDrMd(window->RPort, JAM2);
	SafeSetWriteMask(window->RPort, 0xff);
}

/******** ClearButton() ********/

void __saveds __asm LIBUA_ClearButton(register __a0 struct Window *window,
																			register __a1 struct GadgetRecord *GR,
																			register __d0 int pen)
{
	SetDrMd(window->RPort, JAM2);
	my_SetAPen(window, ChoosePen(window,GR,pen));

	if (GR->type==DATE_GADGET || GR->type==TIME_GADGET ||
			GR->type==INTEGER_GADGET || GR->type==STRING_GADGET ||
			GR->type==SPECIAL_STRING_GADGET )
		UA_ClearStringGadget(window, GR, AREA_PEN);
	else if (GR->type==TEXT_REGION)
	{
		if (window->WScreen->ViewPort.Modes & LACE)
			MyRectFill(window->RPort, (LONG)GR->x1, (LONG)GR->y1+2,
																(LONG)GR->x2, (LONG)GR->y2-2);
		else
			MyRectFill(window->RPort, (LONG)GR->x1, (LONG)GR->y1+1,
																(LONG)GR->x2, (LONG)GR->y2-1);
	}
	else if (GR->type==RADIO_GADGET)
	{
		MyRectFill(window->RPort, (LONG)GR->x1, (LONG)GR->y1, (LONG)GR->x2, (LONG)GR->y2);
		UA_DrawRadio(window,GR);
	}
	else if (GR->type==INVISIBLE_GADGET)
		MyRectFill(window->RPort, (LONG)GR->x1, (LONG)GR->y1,	(LONG)GR->x2, (LONG)GR->y2);
	else
	{
		if (window->WScreen->ViewPort.Modes & LACE)
			MyRectFill(window->RPort, (LONG)GR->x1+2, (LONG)GR->y1+2,
															(LONG)GR->x2-2, (LONG)GR->y2-2);
		else
			MyRectFill(window->RPort, (LONG)GR->x1+2, (LONG)GR->y1+1,
															(LONG)GR->x2-2, (LONG)GR->y2-1);
	}
}

/******** ClearCycleButton() ********/

void __saveds __asm LIBUA_ClearCycleButton(	register __a0 struct Window *window,
																						register __a1 struct GadgetRecord *GR,
																						register __d0 int pen)
{
int oldx1;

	SetDrMd(window->RPort, JAM2);
	my_SetAPen(window, ChoosePen(window,GR,pen));

	oldx1 = GR->x1;
	GR->x1 = GR->x1 + 18 + 2;

	if (window->WScreen->ViewPort.Modes & LACE)
		MyRectFill(window->RPort, (LONG)GR->x1, (LONG)GR->y1+2,
														(LONG)GR->x2-2, (LONG)GR->y2-2);
	else
		MyRectFill(window->RPort, (LONG)GR->x1, (LONG)GR->y1+1,
														(LONG)GR->x2-2, (LONG)GR->y2-1);

	GR->x1 = oldx1;
}

/******** SetValToCycleGadgetVal() ********/
/* returns 'active' number of cycle gadget *NOT* ptr to contents
 */

void __saveds __asm LIBUA_SetValToCycleGadgetVal(	register __a0 struct GadgetRecord *GR,
																									register __a1 int *value)
{
struct CycleRecord *CR_ptr;

	*value=0;
	CR_ptr = (struct CycleRecord *)GR->ptr;
	if (CR_ptr != NULL)
		*value = CR_ptr->active;
}

/******** SetCycleGadgetToVal() ********/

void __saveds __asm LIBUA_SetCycleGadgetToVal(register __a0 struct Window *window,
																							register __a1 struct GadgetRecord *GR,
																							register __d0 int value)
{
struct CycleRecord *CR_ptr;

	CR_ptr = (struct CycleRecord *)GR->ptr;
	if (CR_ptr != NULL)
	{
		if ( CR_ptr->active == value )
			return;
		CR_ptr->active = value;
	}
	UA_DrawCycle(window, GR);
}

/******** GetRadioGadgetVal() ********/

int __saveds __asm LIBUA_GetRadioGadgetVal(register __a0 struct GadgetRecord *GR)
{
struct RadioRecord *RR_ptr;

	RR_ptr = (struct RadioRecord *)GR->ptr;
	return(RR_ptr->active);
}

/******** DisableButton() ********/

void __saveds __asm LIBUA_DisableButton(register __a0 struct Window *window,
																				register __a1 struct GadgetRecord *GR,
																				register __a2 UWORD *pattern)
{
	if (GR->type > 100)
		return;	// already disabled

	my_SetAPen(window, AREA_PEN);
	SetDrMd(window->RPort, JAM1);

	SetAfPt(window->RPort, pattern, 1);
	MyRectFill(window->RPort, (LONG)GR->x1, (LONG)GR->y1, (LONG)GR->x2, (LONG)GR->y2);
	SetAfPt(window->RPort, NULL, 0);
	GR->type += 100;
}

/******** EnableButton() ********/

void __saveds __asm LIBUA_EnableButton(	register __a0 struct Window *window,
																				register __a1 struct GadgetRecord *GR)
{
	if (GR->type > 100)
		GR->type -= 100;

	if (GR->type==DATE_GADGET || GR->type==TIME_GADGET ||
			GR->type==INTEGER_GADGET || GR->type==STRING_GADGET ||
			GR->type==SPECIAL_STRING_GADGET )
	{
		UA_ClearStringGadget(window, GR, AREA_PEN);
		UA_DrawString(window, GR);
	}
	else
	{
		LIBUA_ClearButton(window, GR, AREA_PEN);
		LIBUA_DrawGadget(window, GR);
	}
}

/******** EnableButtonQuiet() ********/

void __saveds __asm LIBUA_EnableButtonQuiet(register __a0 struct GadgetRecord *GR)
{
	if (GR->type > 100)
		GR->type -= 100;
}

/******** DisableButtonQuiet() ********/

void __saveds __asm LIBUA_DisableButtonQuiet(register __a0 struct GadgetRecord *GR)
{
	if (GR->type > 100)
		return;	// already disabled
	GR->type += 100;
}

/******** AdjustGadgetCoords() ********/

void __saveds __asm LIBUA_AdjustGadgetCoords(	register __a0 struct GadgetRecord *GR,
																							register __d0 int xoffset,
																							register __d1 int yoffset)
{
int i=0;

	while(GR[i].x1 != -1)
	{
		GR[i].x1 += xoffset; 
		GR[i].x2 += xoffset; 
		GR[i].y1 += yoffset; 
		GR[i].y2 += yoffset;
		i++;
	}
}

/******** DisableRangeOfButtons() ********/

void __saveds __asm LIBUA_DisableRangeOfButtons(register __a0 struct Window *window,
																								register __a1 struct GadgetRecord *GR,
																								register __d0 int num,
																								register __a2 UWORD *pattern)
{
int i;

	for(i=0; i<num; i++)
		LIBUA_DisableButton(window, &GR[i], pattern);
}

/******** EnableRangeOfButtons() ********/

void __saveds __asm LIBUA_EnableRangeOfButtons(	register __a0 struct Window *window,
																								register __a1 struct GadgetRecord *GR,
																								register __d0 int num)
{
int i;

	for(i=0; i<num; i++)
		LIBUA_EnableButton(window, &GR[i]);
}

/******** SetStringGadgetToString() ********/

void __saveds __asm LIBUA_SetStringGadgetToString(register __a0 struct Window *window,
																									register __a1 struct GadgetRecord *GR,
																									register __a2 STRPTR str)
{
struct StringRecord *SR_ptr;

	SR_ptr = (struct StringRecord *)GR->ptr;
	if (SR_ptr != NULL)
	{
		if ( !strcmp(SR_ptr->buffer,str) )
			return;
		stccpy(SR_ptr->buffer, str, SR_ptr->maxLen+1);
		LIBUA_DrawStringText(window, GR, SR_ptr->buffer);
	}
}

/******** SetStringToGadgetString() ********/

void __saveds __asm LIBUA_SetStringToGadgetString(register __a0 struct GadgetRecord *GR,
																									register __a1 STRPTR str)
{
struct StringRecord *SR_ptr;

	SR_ptr = (struct StringRecord *)GR->ptr;
	if (SR_ptr != NULL)
		stccpy(str, SR_ptr->buffer, SR_ptr->maxLen+1);
}

/******** WipeStringGadget() ********/

void __saveds __asm LIBUA_WipeStringGadget(register __a0 struct GadgetRecord *GR)
{
struct StringRecord *SR_ptr;
int i;

	SR_ptr = (struct StringRecord *)GR->ptr;
	if (SR_ptr != NULL)
	{
		for(i=0; i<SR_ptr->maxLen; i++)
			SR_ptr->buffer[i] = '\0';
	}
}

/******** IsGadgetDisabled() ********/

BOOL __saveds __asm LIBUA_IsGadgetDisabled(register __a0 struct GadgetRecord *GR)
{
	if ( GR->type < 100 )
		return(FALSE);
	else
		return(TRUE);
}

/******** DrawTwoColorBox() ********/

void __saveds __asm LIBUA_DrawTwoColorBox(register __a0 struct Window *window,
																					register __a1 struct GadgetRecord *GR)
{
	if (	GR->type == HIBOX_REGION || GR->type == BORDER_REGION ||
				GR->type == DBL_BORDER_REGION )
		my_SetAPen(window, ChoosePen(window,GR,LO_PEN));
	else
		my_SetAPen(window, ChoosePen(window,GR,HI_PEN));

	Move(window->RPort, GR->x1+1, GR->y2);
	Draw(window->RPort, GR->x2, GR->y2);
	Draw(window->RPort, GR->x2, GR->y1);
	Move(window->RPort, GR->x2-1, GR->y1+1);
	Draw(window->RPort, GR->x2-1, GR->y2-1);

	if (window->WScreen->ViewPort.Modes & LACE)
	{
		Move(window->RPort, GR->x1+2, GR->y2-1);
		Draw(window->RPort, GR->x2-2, GR->y2-1);
	}

	if (	GR->type == HIBOX_REGION || GR->type == BORDER_REGION ||
				GR->type == DBL_BORDER_REGION )
		my_SetAPen(window, ChoosePen(window,GR,HI_PEN));
	else
		my_SetAPen(window, ChoosePen(window,GR,LO_PEN));

	Move(window->RPort, GR->x1, GR->y2);
	Draw(window->RPort, GR->x1, GR->y1);
	Draw(window->RPort, GR->x2-1, GR->y1);
	Move(window->RPort, GR->x1+1, GR->y1+1);
	Draw(window->RPort, GR->x1+1, GR->y2-1);

	if (window->WScreen->ViewPort.Modes & LACE)
	{
		Move(window->RPort, GR->x1+2, GR->y1+1);
		Draw(window->RPort, GR->x2-2, GR->y1+1);
	}

	LIBUA_ClearButton(window,GR,AREA_PEN);

	if (GR->txt != NULL)
		LIBUA_DrawText(window, GR, GR->txt);
}

/******** DrawText() ********/

void __saveds __asm LIBUA_DrawText(	register __a0 struct Window *window,
																		register __a1 struct GadgetRecord *GR,
																		register __a2 STRPTR str)
{
int x,y,len;
TEXT buff[256];

	if (str==NULL)
	{
		if (GR->txt == NULL)
			return;
		stccpy(buff, GR->txt, 256);
	}
	else
		stccpy(buff, str, 256);

	x=y=0;
	len = strlen(buff);

	y = GR->y2 - GR->y1 - window->RPort->TxHeight;
	y = y/2;
	y += GR->y1 + window->RPort->TxBaseline + 1;

	if (GR->type == RADIO_GADGET || GR->type == CHECK_GADGET)
		x = GR->x2 + 4;
	else if (	GR->type == STRING_GADGET || GR->type == DATE_GADGET ||
						GR->type == TIME_GADGET || GR->type == INTEGER_GADGET ||
						GR->type == LOBOX_REGION || GR->type == HIBOX_REGION ||
						GR->type==SPECIAL_STRING_GADGET )
		x = GR->x1 - TextLength(window->RPort, buff, len) - 4;
	else if (	GR->type == CYCLE_GADGET || GR->type == BUTTON_GADGET ||
						GR->type == TEXT_REGION)
	{
		x = GR->x2 - GR->x1 - TextLength(window->RPort, buff, len);
		x = x/2;
		x += GR->x1;
	}

	if ( GR->type == BUTTON_GADGET || GR->type == CYCLE_GADGET )
		my_SetAPen(window, ChooseTextPen(window,GR));
	else
		SetAPen(window->RPort, 1);

	SetDrMd(window->RPort, JAM1);
	Move(window->RPort, (LONG)x, (LONG)y);
	Text(window->RPort, buff, len);
}

/******** GetKeys() ********/

void __saveds __asm LIBUA_GetKeys(register __a0 USHORT *raw,
																	register __a1 char *key,
																	register __a2 int *numkeys,
																	register __a3 struct EventData *ED)
{
LONG numchars;
UBYTE buffer[KEYBUFSIZE];
int i;

	for(i=0; i<KEYBUFSIZE; i++)
		buffer[i] = 0x20;

	*raw=0;
	*key=0;
	numchars = UA_DeadKeyConvert(&buffer[0], (LONG)KEYBUFSIZE, 0L, ED);
	if (numchars > 0L)
	{
		*raw = ED->Code;
		if (ED->Code==RAW_ENTER || ED->Code==RAW_RETURN || ED->Code==RAW_ESCAPE)
				numchars=2;	/* because all other raw keys have more chars */
		*key = buffer[0];
	}
	else
	{
		*raw = NULL;
		*key = NULL;
		*numkeys = 0;
	}
	*numkeys = numchars;
}

/******** SetStringGadgetToVal() ********/

void __saveds __asm LIBUA_SetStringGadgetToVal(	register __a0 struct Window *window,
																								register __a1 struct GadgetRecord *GR,
																								register __d0 int value)
{
struct StringRecord *SR_ptr;
TEXT str[256];

	SR_ptr = (struct StringRecord *)GR->ptr;
	if (SR_ptr != NULL)
	{
		sprintf((STRPTR)str, "%d", value);
		if ( !strcmp(SR_ptr->buffer,str) )
			return;
		stccpy((STRPTR)SR_ptr->buffer, (STRPTR)str, SR_ptr->maxLen+1);
		LIBUA_DrawStringText(window, GR, SR_ptr->buffer);
	}
}

/******** SetValToStringGadgetVal() ********/

void __saveds __asm LIBUA_SetValToStringGadgetVal(register __a0 struct GadgetRecord *GR,
																									register __a1 int *value)
{
struct StringRecord *SR_ptr;

	*value=0;
	SR_ptr = (struct StringRecord *)GR->ptr;
	if (SR_ptr != NULL)
		sscanf((char *)SR_ptr->buffer, (char *)"%d", (int *)value);
}

/******** ProcessStringGadget() ********/

BOOL __saveds __asm LIBUA_ProcessStringGadget(register __a0 struct Window *window,
																							register __a1 struct GadgetRecord *FirstGR,
																							register __a2 struct GadgetRecord *GR,
																							register __a3 struct EventData *ED)
{
BOOL loop, keyOK, tabbed;
ULONG signals, signalMask;
USHORT raw;
char key;
int numkeys, cursorpos, maxLen, i;
TEXT tempbuff[256], swapbuff[256], oldch[2], ch[2];
struct StringRecord *SR_ptr;
struct IntuiMessage *message = NULL;
/**** NEW ****/
struct StringInfo SI;
struct Gadget G;
struct StringExtend SE;
UBYTE *txt, *undo;
ULONG class;
USHORT code;
struct MsgPort *port;
struct List *list;
struct Node *node;
struct RendezVousRecord *rvrec = NULL;
BOOL helpKey=FALSE;

	/**** find port ****/

	if ( capsport )
	{
		port = (struct MsgPort *)FindPort(ML_RENDEZ_VOUS);
		if (port)
		{
			list = &(port->mp_MsgList);
			node = list->lh_Head;
			rvrec = (struct RendezVousRecord *)node->ln_Name;
		}
		else
			rvrec = NULL;
	}

	signalMask = (1L << capsport->mp_SigBit);

	SR_ptr = (struct StringRecord *)GR->ptr;

	i=strlen(SR_ptr->buffer)-1;
	while(i>=0)
	{
		if ( SR_ptr->buffer[i]==' ' )
			SR_ptr->buffer[i]='\0';
		else
			break;
		i--;
	}

	UA_ZeroString(tempbuff, 256);
	stccpy(tempbuff, SR_ptr->buffer, 256);

	if ( *SR_ptr->buffer == ' ' )
		UA_ZeroString(SR_ptr->buffer, SR_ptr->maxLen-1);

	/**** START NEW: use intuition under >36 to handle string editting ****/

	if (GfxBase->LibNode.lib_Version > 36 && GR->type==SPECIAL_STRING_GADGET)
	{
		txt = (UBYTE *)AllocMem(SR_ptr->maxLen+10, MEMF_ANY | MEMF_CLEAR);
		if (txt==NULL)
			return(FALSE);

		undo = (UBYTE *)AllocMem(SR_ptr->maxLen+10, MEMF_ANY | MEMF_CLEAR);
		if (undo==NULL)
		{
			FreeMem(txt,SR_ptr->maxLen+10);
			return(FALSE);
		}

		strcpy(txt, tempbuff);

		AddStrGad(window, GR, txt, undo, SR_ptr->maxLen, &SI, &G, &SE);
		SI.BufferPos = strlen(txt);	
		ActivateGadget(&G, window, NULL);

		while(message = (struct IntuiMessage *)GetMsg(capsport))
			ReplyMsg((struct Message *)message);

		loop=TRUE;
		while(loop)
		{
			signals = Wait(signalMask);
			while( message = (struct IntuiMessage *)GetMsg(capsport) )
			{
				class = message->Class;
				code = message->Code;
				ReplyMsg((struct Message *)message);

				if (	class==IDCMP_GADGETUP ||
							(class==IDCMP_MOUSEBUTTONS && code==SELECTDOWN) )
					loop=FALSE;
				if ( code==RAW_HELP )
					helpKey=TRUE;
			}
		}

		stccpy(SR_ptr->buffer, txt, SR_ptr->maxLen);
		RemoveStrGad(window, &G);

		FreeMem(undo, SR_ptr->maxLen+10);
		FreeMem(txt, SR_ptr->maxLen+10);

		return(helpKey);
	}

	/**** END NEW: use intuition under >=36 to handle string editting ****/

	cursorpos=0;
	maxLen = SR_ptr->maxLen;
	oldch[1]='\0';
	ch[1]='\0';
	tabbed=FALSE;
	loop=TRUE;

	UA_PrtStrTxt(window, GR, tempbuff, cursorpos);
	/* needed here because PrtStrTxt selects and de-selects */
	/* cursor again */
	UA_RenderCursor(window, GR, tempbuff, cursorpos);
	cursorpos = UA_CalcCursorPosFromMousePos(	window, FirstGR, GR, tempbuff,
																						DONTCARE_OUTSIDE, ED);
	if (cursorpos==-1)
		loop=FALSE;
	else
		UA_RenderCursor(window, GR, tempbuff, cursorpos);

	//signalMask = (1L << capsport->mp_SigBit);

	while(loop)
	{
		signals = Wait(signalMask);
		if (signals & signalMask)
		{
			//while(message = (struct IntuiMessage *)GetMsg(window->UserPort))
			while(message = (struct IntuiMessage *)GetMsg(capsport))
			{
				ED->Class			= message->Class;
				ED->Code			= message->Code;
				ED->Qualifier	= message->Qualifier;
				if (ED->Class==IDCMP_RAWKEY)
					ED->IAddress = (APTR)message->IAddress;
				else
					ED->IAddress = NULL;
				ED->Seconds		= message->Seconds;
				ED->Micros		= message->Micros;
				ED->MouseX		= message->MouseX;
				ED->MouseY		= message->MouseY;

				ReplyMsg((struct Message *)message);

				switch(ED->Class)
				{
					case IDCMP_MOUSEBUTTONS:
						if (ED->Code==SELECTDOWN)
						{
							UA_RenderCursor(window, GR, tempbuff, cursorpos);
							cursorpos = UA_CalcCursorPosFromMousePos(	window, FirstGR, GR, tempbuff, CARE_OUTSIDE, ED);
							if (cursorpos==-1)
							{
								loop=FALSE;
								break;
							}
							UA_RenderCursor(window, GR, tempbuff, cursorpos);
						}
						break;

					case IDCMP_RAWKEY:
						LIBUA_GetKeys(&raw, &key, &numkeys, ED);
						if (numkeys>1)	/* special keys */
						{
							switch(raw)
							{
								case RAW_ESCAPE:
								case RAW_ENTER:
								case RAW_RETURN:
									loop=FALSE;
									break;

								case RAW_CURSORLEFT:
									if (ED->Qualifier&IEQUALIFIER_LSHIFT || ED->Qualifier&IEQUALIFIER_RSHIFT)
										cursorpos=0;
									else
										if (cursorpos>0)
											--cursorpos;
									UA_PrtStrTxt(window, GR, tempbuff, cursorpos);
									break;

								case RAW_CURSORRIGHT:
									if (ED->Qualifier&IEQUALIFIER_LSHIFT || ED->Qualifier&IEQUALIFIER_RSHIFT)
										cursorpos=strlen(tempbuff);
									else if (cursorpos<strlen(tempbuff))
										cursorpos++;
									if (cursorpos > maxLen-1)
										cursorpos--;
									UA_PrtStrTxt(window, GR, tempbuff, cursorpos);
									break;

								case RAW_F1:
								case RAW_F2:
								case RAW_F3:
								case RAW_F4:
								case RAW_F5:
								case RAW_F6:
									if ( GR->type==TIME_GADGET )
									{
										if (rvrec && raw==RAW_F1)
											stccpy(tempbuff, rvrec->capsprefs->F1_TIMECODE_STR, SR_ptr->maxLen+1);
										else if (rvrec && raw==RAW_F2)
											stccpy(tempbuff, rvrec->capsprefs->F2_TIMECODE_STR, SR_ptr->maxLen+1);
										else if (rvrec && raw==RAW_F3)
											stccpy(tempbuff, rvrec->capsprefs->F3_TIMECODE_STR, SR_ptr->maxLen+1);
										else if (rvrec && raw==RAW_F4)
											stccpy(tempbuff, rvrec->capsprefs->F4_TIMECODE_STR, SR_ptr->maxLen+1);
										else if (rvrec && raw==RAW_F5)
											stccpy(tempbuff, rvrec->capsprefs->F5_TIMECODE_STR, SR_ptr->maxLen+1);
										else if (rvrec && raw==RAW_F6)
											stccpy(tempbuff, rvrec->capsprefs->F6_TIMECODE_STR, SR_ptr->maxLen+1);
										cursorpos=0;
										UA_PrtStrTxt(window, GR, tempbuff, cursorpos);
									}
									break;
							}
						}
						else if (numkeys==1) /* normal keys, backspace, delete, A-X */
						{
							if (key=='x' && ED->Qualifier&IEQUALIFIER_RCOMMAND)	/* Amiga-X */
							{
								UA_ZeroString(tempbuff, 256);
								tempbuff[0] = ' ';
								cursorpos=0;					
								UA_PrtStrTxt(window, GR, tempbuff, cursorpos);
							}
							else if (raw==RAW_BACKSPACE)
							{
								if ( ED->Qualifier&IEQUALIFIER_LSHIFT || ED->Qualifier&IEQUALIFIER_RSHIFT )
								{
									stccpy(swapbuff, &tempbuff[cursorpos], 256);
									UA_ZeroString(tempbuff, 256);
									stccpy(tempbuff, swapbuff, 256);
									cursorpos=0;
								}
								else
								{
									if (cursorpos>0)
									{
										if (tempbuff[cursorpos] == '\0')	/* cursor rightmost */
										{
											--cursorpos;
											tempbuff[cursorpos] = '\0';
										}
										else	/* cursor in middle */
										{
											--cursorpos;
											stccpy(&tempbuff[cursorpos], &tempbuff[cursorpos+1], 256);
										}
									}
								}
								UA_PrtStrTxt(window, GR, tempbuff, cursorpos);
							}
							else if (raw==RAW_DELETE)	/* delete */
							{
								if ( ED->Qualifier&IEQUALIFIER_LSHIFT || ED->Qualifier&IEQUALIFIER_RSHIFT )
								{
									for(i=cursorpos; i<256; i++)
										tempbuff[i] = '\0';
								}
								else
									stccpy(&tempbuff[cursorpos], &tempbuff[cursorpos+1], 256);
								UA_PrtStrTxt(window, GR, tempbuff, cursorpos);
							}
							else if (raw==RAW_TAB)
							{
								tabbed=TRUE;
								loop=FALSE;
								break;
							}
							else
							{
								if (cursorpos<maxLen && strlen(tempbuff)<maxLen)
								{
									ch[0] = key;
									if (	(TextLength(window->RPort, tempbuff, strlen(tempbuff)) +
												TextLength(window->RPort, &ch[0], 1L)) < 
												(GR->x2-GR->x1-6) )
									{
										keyOK=TRUE;
										switch(GR->type)
										{
											case DATE_GADGET:
												if (key<'0' || key>'9')
													if (key!='/' && key!='-' && key!='.' && key!=' ')
														keyOK=FALSE;
												break;
											case INTEGER_GADGET:
												if (key<'0' || key>'9')
													if (key!='-')
														keyOK=FALSE;
												break;
											case TIME_GADGET:
												if (key<'0' || key>'9')
													if (key!=':' && key!='.')
														keyOK=FALSE;
												break;
										}
										if (keyOK)
										{
											if (tempbuff[cursorpos] != '\0')
											{
												stccpy(swapbuff, &tempbuff[cursorpos], 256);
												stccpy(&tempbuff[cursorpos+1], swapbuff, 256);
											}
											tempbuff[cursorpos] = key;
											if ( (cursorpos+1) != maxLen)
												cursorpos++;
											UA_PrtStrTxt(window, GR, tempbuff, cursorpos);
										}
										else
											DisplayBeep(NULL);
									}
								}
								else if (strlen(tempbuff)==maxLen) /* overstrike */
								{
									ch[0] = key;
									oldch[0] = tempbuff[cursorpos];
									if ((TextLength(window->RPort, tempbuff, strlen(tempbuff)) +
											TextLength(window->RPort, &ch[0], 1L) -
											TextLength(window->RPort, &oldch[0], 1L)) <
											(GR->x2-GR->x1-6) )
									{
										keyOK=TRUE;
										switch(GR->type)
										{
											case DATE_GADGET:
												if (key<'0' || key>'9')
													if (key!='/' && key!='-' && key!='.' && key!=' ')
														keyOK=FALSE;
												break;
											case INTEGER_GADGET:
												if (key<'0' || key>'9')
													keyOK=FALSE;
												break;
											case TIME_GADGET:
												if (key<'0' || key>'9')
													if (key!=':' && key!='.')
														keyOK=FALSE;
												break;
										}
										if (keyOK)
										{
											tempbuff[cursorpos]=key;
											if ( (cursorpos+1) != maxLen)
												cursorpos++;
											UA_PrtStrTxt(window, GR, tempbuff, cursorpos);
										}
										else
											DisplayBeep(NULL);
									}
								}
							}
						}
						break;
				}	/* switch */
			}	/* while (message) */
		} /* if (signals) */
	} /* while(loop) */

	if (tempbuff[0] == '\0')
		tempbuff[0] = ' ';

	SR_ptr = (struct StringRecord *)GR->ptr;
	if (SR_ptr->buffer != NULL)
	{
		stccpy(SR_ptr->buffer, tempbuff, SR_ptr->maxLen+1); //256);
		i=strlen(SR_ptr->buffer)-1;
		while(i>=0)
		{
			if ( SR_ptr->buffer[i]==' ' )
				SR_ptr->buffer[i]='\0';
			else
				break;
			i--;
		}
	}
	UA_DrawString(window, GR);

	return(tabbed);
}

/******** DrawStringText() ********/

void __saveds __asm LIBUA_DrawStringText(	register __a0 struct Window *window,
																					register __a1 struct GadgetRecord *GR,
																					register __a2 STRPTR str)
{
int y;
TEXT buff[256];

	if (str==NULL)
		return;
	UA_ClearStringGadget(window, GR, AREA_PEN);

	if ( str[0]=='\0' )
		return;

	stccpy(buff, str, 255);
	LIBUA_ShortenString(window->RPort, buff, (GR->x2-GR->x1)-16);

	my_SetAPen(window, ChooseTextPen(window,GR));

	y = GR->y2 - GR->y1 - window->RPort->TxHeight;
	y = y/2;
	y += GR->y1 + window->RPort->TxBaseline + 1;

	SetDrMd(window->RPort, JAM1);
	Move(window->RPort, (LONG)GR->x1+4, (LONG)y); // per 15-5-93 GR->y1+window->RPort->TxBaseline+1);
	Text(window->RPort, buff, strlen(buff));
}

/********	DrawSpecialGadgetText() ********/

void __saveds __asm LIBUA_DrawSpecialGadgetText(register __a0 struct Window *window,
																								register __a1 struct GadgetRecord *GR,
																								register __a2 STRPTR str,
																								register __d0 int mode)
{
int x,y;

	if (str==NULL)
			return;

	x=-1;
	if (mode == SPECIAL_TEXT_TOP)	/* place text on top of box but left justified. */
	{															/* e.g. used in format requester to label cycle gadgets */
		x = GR->x1 - 2;
		y = GR->y1 - window->RPort->TxHeight + window->RPort->TxBaseline;
	}
	else if (mode == SPECIAL_TEXT_RIGHT)	/* center text vertically, right justified. */
	{
		x = GR->x2-TextLength(window->RPort, str, strlen(str));
		y = GR->y2 - GR->y1 - window->RPort->TxHeight;
		y = y/2;
		y += GR->y1 + window->RPort->TxBaseline;
	}
	else if (mode == SPECIAL_TEXT_LEFT)	/* center text vertically, left justified. */
	{
		x = GR->x1+2;
		y = GR->y2 - GR->y1 - window->RPort->TxHeight;
		y = y/2;
		y += GR->y1 + window->RPort->TxBaseline;
	}
	else if (mode == SPECIAL_TEXT_CENTER)
	{
		x = GR->x2 - GR->x1 - TextLength(window->RPort, str, strlen(str));
		x = x/2;
		x += GR->x1;

		y = GR->y2 - GR->y1 - window->RPort->TxHeight;
		y = y/2;
		y += (GR->y1 + window->RPort->TxBaseline + 1);
	}
	else if (mode == SPECIAL_TEXT_AFTER_STRING)
	{
		x = GR->x2 + 4;
		y = GR->y1 + window->RPort->TxBaseline + 2;
	}
	else if (mode == SPECIAL_TEXT_BEFORE_STRING)
	{
		x = GR->x1 - TextLength(window->RPort, str, strlen(str)) - 4;
		y = GR->y1 + window->RPort->TxBaseline + 2;
	}

	if (x!=-1)
	{
		my_SetAPen(window, ChoosePen(window,GR,LO_PEN));
		SetDrMd(window->RPort, JAM1);
		Move(window->RPort, (LONG)x, (LONG)y);
		Text(window->RPort, str, strlen(str));
	}
}

/******** GetDepthOfWindowScreen() ********/
/*
 * return 1 if window has no screen (whenever that happens I don't know)
 */

int __saveds __asm LIBUA_GetDepthOfWindowScreen(register __a0 struct Window *window)
{
//struct Screen *screen;
//struct BitMap *bitmap;

	if ( window->WScreen != NULL )
	{
		return( window->RPort->BitMap->Depth );
/*
		screen=window->WScreen;
		if ( &(screen->BitMap) != NULL)
		{
			bitmap=&(screen->BitMap);
			return((int)bitmap->Depth);
		}
*/
	}
	return(1);
}

/******** DoubleGadgetDimensions() ********/

void __saveds __asm LIBUA_DoubleGadgetDimensions(register __a0 struct GadgetRecord *GR)
{
int i=0;

	while(GR[i].x1 != -1)
	{
		GR[i].y1 *= 2; 
		GR[i].y2 *= 2; 
		i++;
	}
}

/******** HalveGadgetDimensions() ********/

void __saveds __asm LIBUA_HalveGadgetDimensions(register __a0 struct GadgetRecord *GR)
{
int i=0;

	while(GR[i].x1 != -1)
	{
		GR[i].y1 /= 2; 
		GR[i].y2 /= 2; 
		i++;
	}
}

/******** IsWindowOnLacedScreen() ********/

BOOL __saveds __asm LIBUA_IsWindowOnLacedScreen(register __a0 struct Window *window)
{
struct Screen *screen;
struct ViewPort *viewport;

	if ( window->WScreen != NULL )
	{
		screen=window->WScreen;
		if ( &(screen->ViewPort) != NULL)
		{
			viewport=&(screen->ViewPort);
			if (viewport->Modes & LACE)
				return(TRUE);
			else
				return(FALSE);
		}
	}
	return(FALSE);
}

/******** IsUAScreenLaced() ********/

BOOL __saveds __asm LIBUA_IsUAScreenLaced(register __a0 struct UserApplicInfo *UAI)
{
struct Screen *screen=NULL;
struct ViewPort *viewport;
LONG Lock;

	if (UAI->windowModes == 1)				/* medialink SCREEN */
		screen	= UAI->medialinkScreen;
	else if (UAI->windowModes == 2)		/* USER SCREEN */
		screen	= UAI->userScreen;
	else if (UAI->windowModes == 3)		/* FIRST SCREEN */
	{
		Lock = LockIBase(0L);
		screen	= UAI->IB->FirstScreen;
		UnlockIBase(Lock);
	}

	if ( screen != NULL )
	{
		viewport=&(screen->ViewPort);
		if (viewport->Modes & LACE)
			return(TRUE);
		else
			return(FALSE);
	}
	return(FALSE);
}

/******** FindString() ********/
/*
 * returns -1 on not found, char pos on found
 *
 */

int __saveds __asm LIBUA_FindString(register __a0 STRPTR OrgString,
																		register __a1 STRPTR CheckString)
{
int i, Length, MaxLength;

	MaxLength = strlen(OrgString);

	i = 0;
	Length = strlen(CheckString);
	MaxLength -= Length;
	while(i < (MaxLength+1))
	{
		if(!strncmp(&OrgString[i], CheckString, Length))
			return(i);
		i++;
	}

	return(-1);
}

/******** ValidatePath() ********/
/*
 * Makes from 'anim:' the string 'anim:' and
 * makes from 'anim:test/pics' and 'anim:test/pics/' the
 * string 'anim:test/pics/'. For once and for all:
 *
 * A FILENAME MUST ALWAYS BE DIRECTLY APPENDABLE TO A PATH !!!!!!!!!!!!!
 *            ++++           ++++++++   
 */

void __saveds __asm LIBUA_ValidatePath(register __a0 STRPTR path)
{
int len;
	len = strlen(path);
	if ( path[len-1]==':' || path[len-1]=='/' )
		return;	/* already OK */
	else
	{
		if ( LIBUA_FindString(path, ":")==-1 )
			strcat(path, ":");
		else
			strcat(path, "/");
	}
}

/******** TurnAssignIntoDir() ********/

void __saveds __asm LIBUA_TurnAssignIntoDir(register __a0 STRPTR ass)
{
struct FileLock *fileLock;

	fileLock = (struct FileLock *)Lock((STRPTR)ass,(LONG)ACCESS_READ);
	if (fileLock != NULL)
	{
		if (my_getpath((BPTR)fileLock, ass)!=0)
			stccpy(ass, "SYS:", 10);
		UnLock((BPTR)fileLock);
	}
	LIBUA_ValidatePath(ass);
}

/******** MakeFullPath() ********/
/*
 * Makes from 'anim:' and 'ape' the string 'anim:ape' and
 * makes from 'anim:test/pics/hires' and 'ape' the
 * string 'anim:test/pics/hires/ape'. Even more, the string
 * 'anim:test/pics/hires/' (note the slash) and 'ape' is correctly
 * converted (no double slashes). This seems trivial but this routine
 * should be used instead of trying to connect them yourself.
 *
 */

void __saveds __asm LIBUA_MakeFullPath(	register __a0 STRPTR path,
																				register __a1 STRPTR name,
																				register __a2 STRPTR answer)
{
int len;

	len = strlen(path);
	if ( path[len-1]==':' || path[len-1]=='/' )
		sprintf(answer, "%s%s", path, name);
	else if (path!=NULL && path[0]!='\0')
		sprintf(answer, "%s/%s", path, name);
	else
		strcpy(answer, name);	// survived strcpy
}

/******** SplitFullPath() ********/
/*
 * If fullPath doesn't contain a ':'	-> path becomes current path ( getcd() )
 *																		-> filename becomes fullPath
 *
 * If fullPath is shorter than 3 characters:
 *																		-> path and filename become '\0'
 *
 * If fullPath is enclosed by '"':		-> double quotes are removed before parsing
 *
 * After stripping, path will get the UA_ValidatePath treatment
 *
 */

void __saveds __asm LIBUA_SplitFullPath(	register __a0 STRPTR fullPath,
																					register __a1 STRPTR path,
																					register __a2 STRPTR filename	)
{
int i,j,pos,len;

	path[0] = '\0';
	filename[0] = '\0';

	if ( !fullPath )
		return;

	// check if a volume name is present

	if ( LIBUA_FindString(fullPath, ":")==-1 )
	{
		strcpy(path,"RAM:");
		stccpy(filename, fullPath, SIZE_FILENAME);

		// remove trailing and ending double quotes
		if ( filename[0] == '\"' )
		{
			len = strlen(filename)-2;
			for(i=0; i<len; i++)
				filename[i] = filename[i+1];
			filename[len] = '\0';
		}

		if ( filename[0]=='@' || filename[0]=='\0' )
			path[0] = '\0';

		return;
	}

	// check minimal length

	if (strlen(fullPath)<3)
		return;

	// remove trailing and ending double quotes

	if ( fullPath[0] == '\"' )
	{
		len = strlen(fullPath)-2;
		for(i=0; i<len; i++)
			fullPath[i] = fullPath[i+1];
		fullPath[len] = '\0';
	}

	// split path and filename

	pos=-1;
	len=strlen(fullPath);
	for(j=len-1; j>0; j--)
	{
		if ( fullPath[j] == ':' || fullPath[j] == '/' )
		{
			pos=j;
			break;
		}
	}

	if ( pos != -1 )
	{
		stccpy(path, fullPath, pos+2);
		stccpy(filename, &fullPath[j+1], len-pos+1);

		LIBUA_ValidatePath(path);
	}

	if ( filename[0]=='@' || filename[0]=='\0' )
		path[0] = '\0';
}

/******** DrawSliderNotches() ********/
/*
 * type: 1           |
 *             | | | | | | |		-> num=7
 *
 *       2     | | | | | | |
 *
 */

void __saveds __asm LIBUA_DrawSliderNotches(	register __a0	struct Window *window,
																							register __a1 struct GadgetRecord *GR,
																							register __d0 int type,
																							register __d1 int num,
																							register __d2 int pen  )
{
int i,j,k,kw;

	kw = (GR->x2-GR->x1) / num;

	my_SetAPen(window,ChoosePen(window,GR,pen));
	SetDrMd(window->RPort,JAM1);

	if (window->WScreen->ViewPort.Modes & LACE)
		j=2;
	else
		j=1;

	for(i=0; i<num; i++)
	{
		k=0;
		if ( type==1 && i==(num/2) )	// draw taller notch
			k=j;

		Move(window->RPort, GR->x1+6+i*kw, GR->y1-1);				// from -1 ...
		Draw(window->RPort, GR->x1+6+i*kw, GR->y1-(j+k));		// to -1/-2

		Move(window->RPort, GR->x1+6+i*kw+1, GR->y1-1);			// from -1 ...
		Draw(window->RPort, GR->x1+6+i*kw+1, GR->y1-(j+k));	// to -1/-2

		Move(window->RPort, GR->x1+6+i*kw, GR->y2+1);				// from 1 ...
		Draw(window->RPort, GR->x1+6+i*kw, GR->y2+(j+k));		// to 1/2

		Move(window->RPort, GR->x1+6+i*kw+1, GR->y2+1);			// from 1 ...
		Draw(window->RPort, GR->x1+6+i*kw+1, GR->y2+(j+k));	// to 1/2
	}
}

/******** DrawDefaultButton() ********/

void __saveds __asm LIBUA_DrawDefaultButton(	register __a0	struct Window *window,
																							register __a1 struct GadgetRecord *GR )
{
	my_SetAPen(window, ChoosePen(window,GR,LO_PEN));

	Move(window->RPort, GR->x1-1, GR->y1-1);
	Draw(window->RPort, GR->x2+1,	GR->y1-1);
	Draw(window->RPort, GR->x2+1,	GR->y2+1);
	Draw(window->RPort, GR->x1-1, GR->y2+1);
	Draw(window->RPort, GR->x1-1, GR->y1-1);

	if ( window->WScreen->ViewPort.Modes & LACE )
	{
		Move(window->RPort, GR->x1-1, GR->y1-2);
		Draw(window->RPort, GR->x2+1,	GR->y1-2);

		Move(window->RPort, GR->x1-1, GR->y2+2);
		Draw(window->RPort, GR->x2+1,	GR->y2+2);
	}

	Move(window->RPort, GR->x1-2, GR->y1);
	Draw(window->RPort, GR->x1-2,	GR->y2);

	Move(window->RPort, GR->x2+2, GR->y1);
	Draw(window->RPort, GR->x2+2,	GR->y2);
}

/******** TranslateGR() ********/

void __saveds __asm LIBUA_TranslateGR(	register __a0 struct GadgetRecord *GR,
																				register __a1 UBYTE **msgs )
{
struct GadgetRecord *nextGR;
struct CycleRecord *CR;
int i;
BOOL wasDisabled;

	nextGR = GR;
	while(nextGR)
	{
		i=0;
		while( nextGR[i].x1 != -1 )
		{
			if ( nextGR[i].type>100 )
			{
				nextGR[i].type-=100;
				wasDisabled=TRUE;
			}
			else
				wasDisabled=FALSE;

			if ( nextGR[i].type == PREV_GR )
			{
				nextGR = nextGR[i].ptr;
				break;
			}
			if ( nextGR[i].lanCode != 0 )
				nextGR[i].txt = msgs[ nextGR[i].lanCode-1 ];
			if ( nextGR[i].type == CYCLE_GADGET && nextGR[i].ptr != NULL )
			{
				CR = (struct CycleRecord *)nextGR[i].ptr;
				if ( CR->lanCode != 0 )
					CR->ptr = msgs[ CR->lanCode-1 ];
			}

			if ( wasDisabled )
				nextGR[i].type+=100;

			i++;
		}
	}
}

/******** PutCapsPort() ********/

void __saveds __asm LIBUA_PutCapsPort(register __a0 struct MsgPort *port)
{
	capsport = port;
}

/******** CloseWindowSafely() ********/

void __saveds __asm LIBUA_CloseWindowSafely(register __a0 struct Window *window)
{
	Forbid();
	StripIntuiMessages(window->UserPort, window);
	window->UserPort = NULL;
	ModifyIDCMP(window, 0L);
	Permit();
	CloseWindow(window);
}

/******** GetRightPen() ********/

int __saveds __asm LIBUA_GetRightPen(	register __a0 struct Window *window,
																			register __a1 struct GadgetRecord *GR,
																			register __d0 long pen )
{
	return( ChoosePen(window,GR,pen) );
}

/******** DrawGadgetListRange() ********/

void __saveds __asm LIBUA_DrawGadgetListRange(register __a0 struct Window *window,
																							register __a1 struct GadgetRecord *GR,
																							register __d0 int first,
																							register __d1 int last )
{
int i=0;

	for(i=first; i<=last; i++)
	{
		if (GR[i].x1 != -1)
			LIBUA_DrawGadget(window, &GR[i]);
		else
			return;
	}
}

/******** DrawComboBox() ********/

void __saveds __asm LIBUA_DrawComboBox(	register __a0 struct Window *window,
																				register __a1 struct GadgetRecord *GR )
{
struct GadgetRecord *titleGR;
int len;

	titleGR = GR->ptr;
	len = TextLength(window->RPort, titleGR->txt, strlen(titleGR->txt));

	SetDrMd(window->RPort, JAM1);
	SetAPen(window->RPort, LO_PEN);

	Move(window->RPort, titleGR->x1-5, GR->y1 );
	Draw(window->RPort, GR->x1, GR->y1);
	Draw(window->RPort, GR->x1, GR->y2);
	Draw(window->RPort, GR->x2, GR->y2);
	Draw(window->RPort, GR->x2, GR->y1);
	Draw(window->RPort, titleGR->x1+(titleGR->x2-titleGR->x1)+len+5+4, GR->y1);

	Move(window->RPort, GR->x1+1, GR->y1);
	Draw(window->RPort, GR->x1+1, GR->y2);

	Move(window->RPort, GR->x2-1, GR->y2);
	Draw(window->RPort, GR->x2-1, GR->y1);

	if (window->WScreen->ViewPort.Modes & LACE)
	{
		Move(window->RPort, titleGR->x1-5, GR->y1+1 );
		Draw(window->RPort, GR->x1, GR->y1+1);

		Move(window->RPort, GR->x1, GR->y2+1);
		Draw(window->RPort, GR->x2, GR->y2+1);

		Move(window->RPort, GR->x2, GR->y1+1);
		Draw(window->RPort, titleGR->x1+(titleGR->x2-titleGR->x1)+len+5+4, GR->y1+1);
	}
}

/*******************************************************************/
/*
 *   PRIVATE FUNCTIONS
 *
 *******************************************************************/

/********* StripIntuiMessages() ********/

void StripIntuiMessages(struct MsgPort *mp, struct Window *window)
{
struct IntuiMessage *msg;
struct Node *succ;

	msg = (struct IntuiMessage *)mp->mp_MsgList.lh_Head;
	while(succ=msg->ExecMessage.mn_Node.ln_Succ)
	{
		if (msg->IDCMPWindow == (struct Window *)window)
		{
			Remove((struct Node *)msg);
			ReplyMsg((struct Message *)msg);
		}
		msg = (struct IntuiMessage *)succ;
	}
}

/******** DrawButton() ********/

void UA_DrawButton(struct Window *window, struct GadgetRecord *GR)
{
int x, y, len;

	SetDrMd(window->RPort, JAM1);

	/* draw right and bottom lines */
	my_SetAPen(window, ChoosePen(window,GR,LO_PEN));
	Move(window->RPort, GR->x1+1, GR->y2);
	Draw(window->RPort, GR->x2,		GR->y2);
	Draw(window->RPort, GR->x2,		GR->y1);
	Move(window->RPort, GR->x2-1, GR->y1+1);
	Draw(window->RPort, GR->x2-1, GR->y2-1);

	if (window->WScreen->ViewPort.Modes & LACE)
	{
		Move(window->RPort, GR->x1+2, GR->y2-1);
		Draw(window->RPort, GR->x2-1,	GR->y2-1);
	}

	/* draw left and top lines */
	my_SetAPen(window, ChoosePen(window,GR,HI_PEN));
	Move(window->RPort, GR->x1, 	GR->y2);
	Draw(window->RPort, GR->x1, 	GR->y1);
	Draw(window->RPort, GR->x2-1, GR->y1);
	Move(window->RPort, GR->x1+1, GR->y1+1);
	Draw(window->RPort, GR->x1+1, GR->y2-1);

	if (window->WScreen->ViewPort.Modes & LACE)
	{
		Move(window->RPort, GR->x1+2, GR->y1+1);
		Draw(window->RPort, GR->x2-1,	GR->y1+1);
	}

	LIBUA_ClearButton(window,GR,AREA_PEN);

	/* draw text if available */

	if (GR->txt != NULL && GR->type==BUTTON_GADGET)
	{
		len = strlen(GR->txt);
		//if (len==1)
		//{
		//	x = GR->x1+4;
		//}
		//else
		//{
			x = GR->x2 - GR->x1 - TextLength(window->RPort, GR->txt, len);
			x = x/2;
			x += GR->x1;
		//}
		y = GR->y2 - GR->y1 - window->RPort->TxHeight;
		y = y/2;
		y += GR->y1 + window->RPort->TxBaseline + 1;

		my_SetAPen(window, ChooseTextPen(window,GR));
		SetDrMd(window->RPort, JAM1);

		Move(window->RPort, (LONG)x, (LONG)y);
		Text(window->RPort, GR->txt, len);

		if ( *GR->txt=='O' && *(GR->txt+1)=='K' )
			LIBUA_DrawDefaultButton(window,GR);
	}
}

/******** DrawCycle() ********/

void UA_DrawCycle(struct Window *window, struct GadgetRecord *GR)
{
int oldx1, oldx2, x, y, len;
struct CycleRecord *CR_ptr;
STRPTR CR_textptr;

	SetDrMd(window->RPort, JAM1);

	CR_ptr = (struct CycleRecord *)GR->ptr;

	oldx1 = GR->x1;
	oldx2 = GR->x2;

	/* draw left part of cycle gadget */

	GR->x2 = GR->x1 + 17;
	UA_DrawButton(window, GR);

	GR->x2 = oldx2;

	/* draw right part of cycle gadget */

	GR->x1 = GR->x1 + 18;
	UA_DrawButton(window, GR);

	GR->x1 = oldx1;

	/* draw @ in left part of cycle gadget */

	x = 17- TextLength(window->RPort, "ƒ", 1L);
	x = x/2;
	x += GR->x1+1; /* + 5; */
	y = GR->y2 - GR->y1-window->RPort->TxHeight;
	y = y/2;
	y += GR->y1 + window->RPort->TxBaseline + 1;

	my_SetAPen(window, ChooseTextPen(window,GR));
	SetDrMd(window->RPort, JAM1);
	Move(window->RPort, (LONG)x, (LONG)y);
	Text(window->RPort, "ƒ", 1L);

	if (CR_ptr != NULL)
	{
		CR_textptr = CR_ptr->ptr;
		if ( CR_ptr->ptr )
			UA_DrawCycleText(window, GR, &CR_textptr[CR_ptr->active*CR_ptr->width]);
	}	

	if (GR->txt != NULL)
	{
		len = strlen(GR->txt);
		x = GR->x1 - TextLength(window->RPort, GR->txt, len) - 4;
		y = GR->y1 + window->RPort->TxBaseline + 2;	// was +3

		my_SetAPen(window, ChoosePen(window,GR,TEXT_PEN));
		SetDrMd(window->RPort, JAM1);
		Move(window->RPort, (LONG)x, (LONG)y);
		Text(window->RPort, GR->txt, len);
	}
}

/******** DrawRadio() ********/

void UA_DrawRadio(struct Window *window, struct GadgetRecord *GR)
{
	SetDrMd(window->RPort, JAM1);

	my_SetAPen(window, ChoosePen(window,GR,HI_PEN));
	Move(window->RPort, GR->x1, GR->y1+2);
	Draw(window->RPort, GR->x1, GR->y2-2);
	Move(window->RPort, GR->x1+2, GR->y2);
	Draw(window->RPort, GR->x1+2, GR->y2-1);
	Draw(window->RPort, GR->x1+1, GR->y2-1);
	Draw(window->RPort, GR->x1+1, GR->y1+1);
	Draw(window->RPort, GR->x1+2, GR->y1+1);
	Draw(window->RPort, GR->x1+2, GR->y1);
	Draw(window->RPort, GR->x2-3, GR->y1);

	if (window->WScreen->ViewPort.Modes & LACE)
	{
		Move(window->RPort, GR->x1+2, GR->y1+2);
		Draw(window->RPort, GR->x1+3, GR->y1+1);
		Draw(window->RPort, GR->x2-3, GR->y1+1);
		Draw(window->RPort, GR->x2-2, GR->y1+2);

		Move(window->RPort, GR->x1+2, GR->y2-2);
		Draw(window->RPort, GR->x1+2, GR->y2-2);
	}

	my_SetAPen(window, ChoosePen(window,GR,LO_PEN));
	Move(window->RPort, GR->x2, GR->y1+2);
	Draw(window->RPort, GR->x2, GR->y2-2);
	Move(window->RPort, GR->x1+3, GR->y2);
	Draw(window->RPort, GR->x2-2, GR->y2);
	Draw(window->RPort, GR->x2-2, GR->y2-1);
	Draw(window->RPort, GR->x2-1, GR->y2-1);
	Draw(window->RPort, GR->x2-1, GR->y1+1);
	Draw(window->RPort, GR->x2-2, GR->y1+1);
	Draw(window->RPort, GR->x2-2, GR->y1);

	if (window->WScreen->ViewPort.Modes & LACE)
	{
		Move(window->RPort, GR->x1+3, GR->y2-1);
		Draw(window->RPort, GR->x2-3, GR->y2-1);
		Draw(window->RPort, GR->x2-2, GR->y2-2);
	}

	my_SetAPen(window, ChoosePen(window,GR,AREA_PEN));
	if (window->WScreen->ViewPort.Modes & LACE)
	{
		Move(window->RPort, (LONG)GR->x1+3, (LONG)GR->y1+2);			// 5  3
		Draw(window->RPort, (LONG)GR->x2-3, (LONG)GR->y1+2);			// 5  3
		MyRectFill(window->RPort, (LONG)GR->x1+2, (LONG)GR->y1+3,		// 4  4
															(LONG)GR->x2-2, (LONG)GR->y2-3);	// 4  4
		Move(window->RPort, (LONG)GR->x1+3, (LONG)GR->y2-2);			// 5  3
		Draw(window->RPort, (LONG)GR->x2-3, (LONG)GR->y2-2);			// 5  3
	}
	else
	{
		Move(window->RPort, (LONG)GR->x1+3, (LONG)GR->y1+1);			// 5  2
		Draw(window->RPort, (LONG)GR->x2-3, (LONG)GR->y1+1);			// 5  2
		MyRectFill(window->RPort, (LONG)GR->x1+2, (LONG)GR->y1+2,		// 4  3
															(LONG)GR->x2-2, (LONG)GR->y2-2);	// 4  3
		Move(window->RPort, (LONG)GR->x1+3, (LONG)GR->y2-1);			// 5  2
		Draw(window->RPort, (LONG)GR->x2-3, (LONG)GR->y2-1);			// 5  2
	}

	LIBUA_DrawText(window, GR, NULL);	/* NULL means use gadget text */
}

/******** DrawCheck() ********/

void UA_DrawCheck(struct Window *window, struct GadgetRecord *GR)
{
	UA_DrawButton(window, GR);
	LIBUA_DrawText(window, GR, NULL);	/* NULL means use gadget text */
}

/******** DrawSimpleBox() ********/

void UA_DrawSimpleBox(struct Window *window, struct GadgetRecord *GR)
{
	SetDrMd(window->RPort, JAM1);

	my_SetAPen(window, ChoosePen(window,GR,LO_PEN));
	Move(window->RPort, GR->x1, GR->y1);
	Draw(window->RPort, GR->x2, GR->y1);
	Draw(window->RPort, GR->x2, GR->y2);
	Draw(window->RPort, GR->x1, GR->y2);
	Draw(window->RPort, GR->x1, GR->y1);

	if (window->WScreen->ViewPort.Modes & LACE)
	{
		Move(window->RPort, GR->x1+1, GR->y1+1);
		Draw(window->RPort, GR->x2-1, GR->y1+1);

		Move(window->RPort, GR->x1, GR->y2+1);	/* YES, PLUS ONE ! */
		Draw(window->RPort, GR->x2, GR->y2+1);	/* YES, PLUS ONE ! */
	}
}

/******** DrawABorder() ********/

void UA_DrawABorder(struct Window *window, struct GadgetRecord *GR)
{
int oldx1, oldx2;

	if (GR->type == BORDER_REGION)
		UA_DrawSimpleBox(window, GR);
	else if (GR->type == DBL_BORDER_REGION)
	{
		UA_DrawSimpleBox(window, GR);
		oldx1 = GR->x1;
		oldx2 = GR->x2;
		GR->x1 += 1;
		GR->x2 -= 1;
		UA_DrawSimpleBox(window, GR);
		GR->x1 = oldx1;
		GR->x2 = oldx2;
	}
}

/******** DrawABox() ********/

void UA_DrawABox(struct Window *window, int pen, int x1, int y1, int x2, int y2)
{
	my_SetAPen(window, pen);
	SetDrMd(window->RPort, JAM1);
	Move(window->RPort, x1, y1);
	Draw(window->RPort, x2, y1);
	Draw(window->RPort, x2, y2);
	Draw(window->RPort, x1, y2);
	Draw(window->RPort, x1, y1+1);
}

/******** DrawCycleText() ********/

void UA_DrawCycleText(struct Window *window, struct GadgetRecord *GR, STRPTR str)
{
int oldx1;

	oldx1 = GR->x1;
	GR->x1 = GR->x1 + 18 + 2;
	LIBUA_ClearButton(window, GR, AREA_PEN);
	LIBUA_DrawText(window, GR, str);
	GR->x1 = oldx1;
}

/******** GetGadgetID() ********/

int UA_GetGadgetID(	struct Window *window, struct GadgetRecord *FirstGR,
										struct GadgetRecord *GR, STRPTR str)
{
int i=0;
struct GadgetRecord *GR_ptr;

	GR_ptr = FirstGR;

	if (str==NULL)	/* compare co-ordinates */
	{
		while(GR_ptr->x1 != -1)
		{
			if (	GR_ptr->x1 == GR->x1 &&
						GR_ptr->y1 == GR->y1 &&
						GR_ptr->x2 == GR->x2 &&
						GR_ptr->y2 == GR->y2 )
			{
				return(i);
			}
			i++;
			GR_ptr = &FirstGR[i];
		}
	}
	else
	{
		while(GR_ptr->x1 != -1)
		{
			if ( !strcmp(GR_ptr->txt, str) )
				return(i+100);
			i++;
			GR_ptr = &FirstGR[i];
		}
	}
	return(-1);
}

/******** ClearStringGadget() ********/

void UA_ClearStringGadget(struct Window *window,
													struct GadgetRecord *GR, int pen)
{
	SetDrMd(window->RPort, JAM2);
	my_SetAPen(window, ChoosePen(window,GR,pen));
	MyRectFill(window->RPort, (LONG)GR->x1+4, (LONG)GR->y1+2,
														(LONG)GR->x2-4, (LONG)GR->y2-2);
}

/******** DeadKeyConvert() ********/

LONG UA_DeadKeyConvert(	UBYTE *kbuffer, LONG kbsize, struct KeyMap *kmap,
												struct EventData *ED)
{
static struct InputEvent ievent = { NULL, IECLASS_RAWKEY, 0, 0, 0 };

	if (ED->Class != IDCMP_RAWKEY)
		return(-2);
	ievent.ie_Code = ED->Code;
	ievent.ie_Qualifier = ED->Qualifier;
	ievent.ie_position.ie_addr = *((APTR*)ED->IAddress);
	return(RawKeyConvert(&ievent, kbuffer, kbsize, 0L));
}

/******** ZeroString() ********/

void UA_ZeroString(STRPTR str, int len)
{
int i;
	for(i=0; i<len; i++)
		str[i]='\0';
}

/******** CalcCursorPosFromMousePos() ********/

int UA_CalcCursorPosFromMousePos(	struct Window *window,
																	struct GadgetRecord *FirstGR,
																	struct GadgetRecord *GR,
																	STRPTR str, int mode,
																	struct EventData *ED)
{
int i,ID;

	ID = LIBUA_CheckGadget(window, FirstGR, GR, ED);
	if (ID==-1)	/* outside */
	{
		if (mode == DONTCARE_OUTSIDE)
			return(0);
		else
			return(-1);
	}

	if ( *str == '\0' || *str == ' ' )
		return(0);

	for(i=0; i<strlen(str); i++)
	{
		if ( TextLength(window->RPort, str, i+1) >= (ED->MouseX-GR->x1-4) )
		{
			return(i);
		}
	}
	if (ED->MouseX>GR->x1-4)
	{
		ID = strlen(str);

		if ( GR->type!=SPECIAL_STRING_GADGET && GR->type!=STRING_GADGET )
			ID--;

		if (ID<0)
			ID=0;

		return(ID);
	}
	else
		return(0);
}

/******** RenderCursor() ********/

void UA_RenderCursor(	struct Window *window, struct GadgetRecord *GR,
											STRPTR str, int pos)
{
int x1,y1,x2,y2;

	if (pos<0)
		return;
	x1 = GR->x1+4 + TextLength(window->RPort, str, pos);
	y1 = GR->y1+2;
	x2 = x1 + TextLength(window->RPort, &str[pos], 1L);
	if (x2 > (GR->x2-2))
		x2=GR->x2-2;
	y2 = GR->y2-2;
	if (x1>=x2)
	{
		x2=GR->x2-2;
		x1=x2-1;
	}
	if (x1 > GR->x2-4)
	{
		x1=GR->x2-4;
		x2=GR->x2-2;
	}
	SafeSetWriteMask(window->RPort, 0x3);
	SetDrMd(window->RPort, JAM2 | COMPLEMENT);
	RectFill(window->RPort, (LONG)x1, (LONG)y1, (LONG)x2, (LONG)y2);
	SetDrMd(window->RPort, JAM2);
	SafeSetWriteMask(window->RPort, 0xff);
}

/******** PrtStrTxt() ********/

void UA_PrtStrTxt(struct Window *window, struct GadgetRecord *GR,
									STRPTR str, int pos)
{
int y;

	if (str==NULL || pos<0)
		return;
	UA_RenderCursor(window, GR, str, pos);
	UA_ClearStringGadget(window, GR, AREA_PEN);

	my_SetAPen(window, ChooseTextPen(window,GR));

	y = GR->y2 - GR->y1 - window->RPort->TxHeight;
	y = y/2;
	y += GR->y1 + window->RPort->TxBaseline + 1;

	SetDrMd(window->RPort, JAM1);
	Move(window->RPort, (LONG)GR->x1+4, (LONG)y); // per 15-5-93 GR->y1+window->RPort->TxBaseline+1);
	Text(window->RPort, str, strlen(str));
	UA_RenderCursor(window, GR, str, pos);
}

/******** DrawString() ********/

void UA_DrawString(struct Window *window, struct GadgetRecord *GR)
{
int x, y, len;
struct StringRecord *SR_ptr;

	SetDrMd(window->RPort, JAM1);

	my_SetAPen(window, ChoosePen(window,GR,HI_PEN));
	UA_DrawABox(window, ChoosePen(window,GR,HI_PEN), GR->x1, GR->y1, GR->x2-2, GR->y2-1);
	Move(window->RPort, GR->x1+1, GR->y1+1);
	Draw(window->RPort, GR->x1+1, GR->y2-2);
	Move(window->RPort, GR->x2-3, GR->y1+1);
	Draw(window->RPort, GR->x2-3, GR->y2-2);

	my_SetAPen(window, ChoosePen(window,GR,LO_PEN));
	Move(window->RPort, GR->x1+2, GR->y2-2);
	Draw(window->RPort, GR->x1+2, GR->y1+1);
	Draw(window->RPort, GR->x2-4, GR->y1+1);
	Move(window->RPort, GR->x1+3, GR->y1+2);
	Draw(window->RPort, GR->x1+3, GR->y2-2);
	Move(window->RPort, GR->x1, GR->y2);
	Draw(window->RPort, GR->x2, GR->y2);
	Draw(window->RPort, GR->x2, GR->y1);
	Move(window->RPort, GR->x2-1, GR->y1);
	Draw(window->RPort, GR->x2-1, GR->y2-1);

	UA_ClearStringGadget(window,GR,AREA_PEN);

	if (GR->txt != NULL)
	{
		len = strlen(GR->txt);
		x = GR->x1 - TextLength(window->RPort, GR->txt, len) - 4;
		y = GR->y1 + window->RPort->TxBaseline + 2;	// was +1

		my_SetAPen(window, ChooseTextPen(window,GR));

		SetDrMd(window->RPort, JAM1);
		Move(window->RPort, (LONG)x, (LONG)y);
		Text(window->RPort, GR->txt, len);
	}

	SR_ptr = (struct StringRecord *)GR->ptr;
	if (SR_ptr != NULL)
	{
		if (SR_ptr->buffer != NULL)
			LIBUA_DrawStringText(window, GR, SR_ptr->buffer);
	}
}

/******** DrawALine() ********/

void UA_DrawALine(struct Window *window, struct GadgetRecord *GR)
{
	SetDrMd(window->RPort, JAM1);
	if (GR->type == LO_LINE || GR->type == DOTTED_LINE)
		my_SetAPen(window, ChoosePen(window,GR,LO_PEN));
	if (GR->type == DOTTED_LINE)
		SetDrPt(window->RPort, 0xaaaa);
	Move(window->RPort, (LONG)GR->x1, (LONG)GR->y1);
	Draw(window->RPort, (LONG)GR->x2, (LONG)GR->y2);
	if (window->WScreen->ViewPort.Modes & LACE)
	{
		Move(window->RPort, (LONG)GR->x1, (LONG)GR->y1+1);
		Draw(window->RPort, (LONG)GR->x2, (LONG)GR->y2+1);
	}
	SetDrPt(window->RPort, 0xffff);
}

/******** my_SetAPen() ********/

void my_SetAPen(struct Window *window, long pen)
{
	SetDrMd(window->RPort, JAM1);
	SetAPen(window->RPort, pen);

#if 0
#if 0

#define HI_PEN 		2L	/* pen color to draw top and left lines of buttons */
#define LO_PEN		1L	/* pen color to draw bottom and right lines of buttons */
#define TEXT_PEN	1L	/* pen color to draw text inside buttons */
#define AREA_PEN	3L	/* pen color to draw back of requesters, gadgets etc. */
#define BGND_PEN	0L

#endif

	SetDrMd(window->RPort,JAM1);	// NEW NEW NEW

	if ( window->UserData!=NULL )
	{
		/**** UserData[]  0        1      2      3          ****/
		/****             AREA_PEN LO_PEN HI_PEN TEXT_PEN   ****/

		if (pen==HI_PEN)
			pen = window->UserData[2];
		else if (pen==LO_PEN)
			pen = window->UserData[1];
		else if (pen==TEXT_PEN)
			pen = window->UserData[3];
		else if (pen==AREA_PEN)
			pen = window->UserData[0];
		else if (pen==BGND_PEN)
			pen = 0;
		else
			pen = window->UserData[0];

		SetAPen(window->RPort, pen);
	}
	else
	{
/*
		if ( window->RPort->BitMap->Depth==1 )
		{
			if (pen==AREA_PEN)
				pen = 0L;
			else
				pen = 1L;
			SetAPen(window->RPort, pen);
		}
		else
*/
		SetAPen(window->RPort, pen);
	}
#endif
}

/******** my_getpath() ********/

int my_getpath(BPTR lock, char *path)
{
int error=-1;

	if (GfxBase->LibNode.lib_Version >= 36)
	{
		if ( NameFromLock(lock, path, SIZE_PATH) )
			error=0;
	}
	else
	{
		strcpy(path, "SYS:");	
		error=0;
	}
	return(error);
}

/******** ChoosePen() ********/

int ChoosePen(struct Window *window, struct GadgetRecord *GR, int pen)
{
	if ( pen==BGND_PEN )
		return(0);	

	// NB LO_PEN is TEXT_PEN

	if ( window->RPort->BitMap->Depth==1 )
	{
		if ( pen==LO_PEN || pen==HI_PEN )
			return(1);	
		else if ( pen==AREA_PEN )
			return(0);	
	}
	else if ( window->RPort->BitMap->Depth==2 )
	{
		if ( pen==LO_PEN )
			return(1);	
		else if ( pen==HI_PEN )
			return(2);	
		else if ( pen==AREA_PEN )
			return(3);	
	}
	else
	{
		if ( pen==LO_PEN )
			return(1);	

		if ( GR->color==0 )					// dark color set
		{
			if ( pen==HI_PEN )
				return(6);	
			else if ( pen==AREA_PEN )
				return(7);	
		}
		else if ( GR->color==1 )		// light color set
		{
			if ( pen==HI_PEN )
				return(4);	
			else if ( pen==AREA_PEN )
				return(5);	
		}
		else if ( GR->color==2 )		// old-style color set
		{
			if ( pen==HI_PEN )
				return(2);	
			else if ( pen==AREA_PEN )
				return(3);	
		}
	}

	return(0);
}

/******** ChooseTextPen() ********/

int ChooseTextPen(struct Window *window, struct GadgetRecord *GR)
{
	if ( window->RPort->BitMap->Depth==1 )
		return(1);	
	else if ( window->RPort->BitMap->Depth==2 )
		return(1);	
	else
	{
		if ( GR->color==0 )					// dark color set
			return(2);	
		else if ( GR->color==1 )		// light color set
			return(1);	
		else if ( GR->color==2 )		// old-style color set
		return(1);	
	}

	return(0);
}

/******** MyRectFill() ********/

void MyRectFill(struct RastPort *rp, WORD x1, WORD y1, WORD x2, WORD y2)
{
	if ( x2<x1 || y2<y1 )
		return;
	RectFill(rp,x1,y1,x2,y2);
}

/******** E O F ********/
