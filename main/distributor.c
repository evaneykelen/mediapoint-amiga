#include "nb:pre.h"

#define DO_DISTRI 3
#define DO_CENTER 4
#define DISTRI_LR	0
#define DISTRI_TB	1
#define CENTER_H	0
#define CENTER_V	1

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Library *medialinkLibBase;
extern UWORD chip gui_pattern[];
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;

/**** gadgets ****/

extern struct GadgetRecord Distributor_GR[];

/**** vectors ****/

extern struct VectorRecord Distri_LeftAndRight_VR[];
extern struct VectorRecord Distri_TopAndBottom_VR[];
extern struct VectorRecord Center_Horizontally_VR[];
extern struct VectorRecord Center_Vertically_VR[];

/**** functions ****/

/******** Distributor() ********/

void Distributor(void)
{
struct Window *window, *aWin;
BOOL loop=TRUE, retval=FALSE, mouseMoved, drawIt, done[MAXEDITWINDOWS];
int ID, i, j;
WORD min,max,new,x,y, ex,ey, pos, start;
int minWdw, maxWdw, startWdw, numActive;
ULONG signals;
struct IntuiMessage *message;
static int action=DO_DISTRI, distri=DISTRI_LR, center=CENTER_H;
BOOL disabled=FALSE;
WORD prevX[MAXEDITWINDOWS],prevY[MAXEDITWINDOWS];
WORD prevW[MAXEDITWINDOWS],prevH[MAXEDITWINDOWS];
WORD newX[MAXEDITWINDOWS],newY[MAXEDITWINDOWS];
WORD newW[MAXEDITWINDOWS],newH[MAXEDITWINDOWS];
BOOL list[MAXEDITWINDOWS];
float add;

	PrepareRedrawAll(prevX,prevY,prevW,prevH,newX,newY,newW,newH,list);

	numActive = NumActiveEditWindows();

	if (numActive<=2)	// no left/right/top/bottom, wel center
	{
		action=DO_CENTER;
		disabled=TRUE;
	}

	/**** open a window ****/

	window = UA_OpenRequesterWindow(pageWindow,Distributor_GR,STDCOLORS);
	if (!window)
	{
		UA_WarnUser(185);
		return;
	}

	/**** render gadget ****/

	UA_DrawGadgetList(window, Distributor_GR);

	if ( UA_IsWindowOnLacedScreen(window) )
	{
		UA_DoubleVectorDimensions(Distri_LeftAndRight_VR);
		UA_DoubleVectorDimensions(Distri_TopAndBottom_VR);
		UA_DoubleVectorDimensions(Center_Horizontally_VR);
		UA_DoubleVectorDimensions(Center_Vertically_VR);
	}

	UA_InvertButton(window, &Distributor_GR[action]);

	UA_SetCycleGadgetToVal(window, &Distributor_GR[5], distri);
	UA_SetCycleGadgetToVal(window, &Distributor_GR[6], center);

	if (action==DO_DISTRI)
	{
		if (distri==DISTRI_LR)
			UA_DrawVectorList(window, Distri_LeftAndRight_VR);
		else
			UA_DrawVectorList(window, Distri_TopAndBottom_VR);
		UA_DisableButton(window, &Distributor_GR[6], gui_pattern);
	}
	else
	{
		if (center==CENTER_H)
			UA_DrawVectorList(window, Center_Horizontally_VR);
		else
			UA_DrawVectorList(window, Center_Vertically_VR);
		UA_DisableButton(window, &Distributor_GR[5], gui_pattern);
	}

	if (disabled)
	{
		UA_DisableButton(window, &Distributor_GR[3], gui_pattern);
		UA_DisableButton(window, &Distributor_GR[4], gui_pattern);
		UA_DisableButton(window, &Distributor_GR[5], gui_pattern);
	}

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, Distributor_GR, &CED);
			switch(ID)
			{
				case DO_DISTRI:
					if (action!=DO_DISTRI)
					{
						action=DO_DISTRI;
						UA_InvertButton(window, &Distributor_GR[3]);
						UA_InvertButton(window, &Distributor_GR[4]);
						UA_ClearButton(window, &Distributor_GR[7], AREA_PEN);
						if (distri==DISTRI_LR)
							UA_DrawVectorList(window, Distri_LeftAndRight_VR);
						else
							UA_DrawVectorList(window, Distri_TopAndBottom_VR);
						UA_EnableButton(window, &Distributor_GR[5]);
						UA_DisableButton(window, &Distributor_GR[6], gui_pattern);
						UA_SetCycleGadgetToVal(window, &Distributor_GR[5], distri);
					}
					break;

				case DO_CENTER:
					if (action!=DO_CENTER)
					{
						action=DO_CENTER;
						UA_InvertButton(window, &Distributor_GR[3]);
						UA_InvertButton(window, &Distributor_GR[4]);
						UA_ClearButton(window, &Distributor_GR[7], AREA_PEN);
						if (center==CENTER_H)
							UA_DrawVectorList(window, Center_Horizontally_VR);
						else
							UA_DrawVectorList(window, Center_Vertically_VR);
						UA_EnableButton(window, &Distributor_GR[6]);
						UA_DisableButton(window, &Distributor_GR[5], gui_pattern);
						UA_SetCycleGadgetToVal(window, &Distributor_GR[6], center);
					}
					break;

				case 5:
					UA_ProcessCycleGadget(window, &Distributor_GR[ID], &CED);
					UA_SetValToCycleGadgetVal(&Distributor_GR[ID], &distri);
					UA_ClearButton(window, &Distributor_GR[7], AREA_PEN);
					if (distri==DISTRI_LR)
						UA_DrawVectorList(window, Distri_LeftAndRight_VR);
					else
						UA_DrawVectorList(window, Distri_TopAndBottom_VR);
					break;

				case 6:
					UA_ProcessCycleGadget(window, &Distributor_GR[ID], &CED);
					UA_SetValToCycleGadgetVal(&Distributor_GR[ID], &center);
					UA_ClearButton(window, &Distributor_GR[7], AREA_PEN);
					if (center==CENTER_H)
						UA_DrawVectorList(window, Center_Horizontally_VR);
					else
						UA_DrawVectorList(window, Center_Vertically_VR);
					break;

				case 8:	/* Cancel */
do_cancel:
					UA_HiliteButton(window, &Distributor_GR[8]);
					loop=FALSE;
					retval=FALSE;
					break;

				case 9:	/* OK */
do_ok:
					UA_HiliteButton(window, &Distributor_GR[9]);
					loop=FALSE;
					retval=TRUE;
					break;
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE) /* cancel */
				goto do_cancel;
			else if (CED.Code==RAW_RETURN) /* OK */
				goto do_ok;
		}
	}

	if (disabled)
	{
		UA_EnableButton(window, &Distributor_GR[3]);
		UA_EnableButton(window, &Distributor_GR[4]);
	}

	UA_EnableButton(window, &Distributor_GR[5]);
	UA_EnableButton(window, &Distributor_GR[6]);

	if ( UA_IsWindowOnLacedScreen(window) )
	{
		UA_HalveVectorDimensions(Distri_LeftAndRight_VR);
		UA_HalveVectorDimensions(Distri_TopAndBottom_VR);
		UA_HalveVectorDimensions(Center_Horizontally_VR);
		UA_HalveVectorDimensions(Center_Vertically_VR);
	}

	UA_CloseRequesterWindow(window,STDCOLORS);

	/**** do the actual distributing ****/

	if ( !retval )
		return;	// cancel

	if (action==DO_DISTRI)	// DISTRIBUTE
	{
		min=9999;
		max=-1;
		new=0;
		minWdw=0;
		maxWdw=0;

		/**** find minimum and maximum positions ****/

		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if (EditSupportList[i]!=NULL && EditSupportList[i]->Active)
			{
				new++;
				if (distri==DISTRI_LR)
				{
					if ( EditWindowList[i]->X < min )
					{
						min = EditWindowList[i]->X;
						minWdw = i;
					}
					if ( (EditWindowList[i]->X+EditWindowList[i]->Width) > max )
					{
						max = EditWindowList[i]->X+EditWindowList[i]->Width;
						maxWdw = i;
					}
				}
				else
				{
					if ( EditWindowList[i]->Y < min )
					{
						min = EditWindowList[i]->Y;
						minWdw = i;
					}
					if ( (EditWindowList[i]->Y+EditWindowList[i]->Height) > max )
					{
						max = EditWindowList[i]->Y+EditWindowList[i]->Height;
						maxWdw = i;
					}
				}
			}
		}

		if (min==9999 || max==-1)
			return;

		if (minWdw==maxWdw)
			return;

		/**** calculate total width/height ****/

		add=0.0;
		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if (EditSupportList[i]!=NULL && EditSupportList[i]->Active)
			{
				if (distri==DISTRI_LR)
					add += (float)EditWindowList[i]->Width;
				else
					add += (float)EditWindowList[i]->Height;
			}
		}

		/**** calculate distance between objects (can be negative!) ****/

		if (new!=1)
			add = (float)(((max-min)-add) / (new-1));
		else
			add = 0.0;

		DrawAllHandles(LEAVE_ACTIVE);

		if ( distri==DISTRI_LR )
			pos = min;// + EditWindowList[minWdw]->Width + (int)add;
		else
			pos = min;// + EditWindowList[minWdw]->Height + (int)add;

		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			done[i] = FALSE;
			list[i] = FALSE;
		}
		//done[minWdw]=TRUE;
		//done[maxWdw]=TRUE;

		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			start=9999;
			startWdw=-1;
			for(j=0; j<MAXEDITWINDOWS; j++)
			{
				if ( EditSupportList[j]!=NULL && EditSupportList[j]->Active && !done[j] )
				{
					if ( distri==DISTRI_LR )
					{
						if ( EditWindowList[j]->X < start )
						{
							start = EditWindowList[j]->X;
							startWdw = j;
						}
					}
					else
					{
						if ( EditWindowList[j]->Y < start )
						{
							start = EditWindowList[j]->Y;
							startWdw = j;
						}
					}
				}
			}
			if ( startWdw!=-1 )
			{
				ex = EditWindowList[startWdw]->X;
				ey = EditWindowList[startWdw]->Y;
				if ( distri==DISTRI_LR )
				{
					EditWindowList[startWdw]->X = pos;
					pos += (EditWindowList[startWdw]->Width + (int)add);
				}
				else
				{
					EditWindowList[startWdw]->Y = pos;
					pos += (EditWindowList[startWdw]->Height + (int)add);
				}
				done[startWdw] = TRUE;
				CheckBoundingBox(	&EditWindowList[startWdw]->X, &EditWindowList[startWdw]->Y,
													EditWindowList[startWdw]->Width, EditWindowList[startWdw]->Height);
				list[ startWdw ] = TRUE;

				CorrectEW(EditWindowList[startWdw]);

				prevX[ startWdw ] = ex;
				prevY[ startWdw ] = ey;
				prevW[ startWdw ] = EditWindowList[startWdw]->Width;
				prevH[ startWdw ] = EditWindowList[startWdw]->Height;

				newX[ startWdw ] = EditWindowList[startWdw]->X;
				newY[ startWdw ] = EditWindowList[startWdw]->Y;
				newW[ startWdw ] = EditWindowList[startWdw]->Width;
				newH[ startWdw ] = EditWindowList[startWdw]->Height;
			}
		}

		RedrawAllOverlapWdwListPrev(prevX,prevY,prevW,prevH, newX,newY,newW,newH, list);

		DrawAllHandles(LEAVE_ACTIVE);
	}
	else	// CENTER
	{
		aWin = GetActiEditWin();
		if (aWin!=NULL)
			UA_SwitchMouseMoveOn(aWin);
		x = pageWindow->MouseX;
		y = pageWindow->MouseY;

		if (center==CENTER_H)
		{
			DrawHoriBars();
			SafeSetWriteMask(pageWindow->RPort, 0x3);
			SetDrMd(pageWindow->RPort, JAM2 | COMPLEMENT);
			Move(pageWindow->RPort, x, 0);
			Draw(pageWindow->RPort, x, pageWindow->Height-1);
		}
		else
		{
			DrawVertBars();
			SafeSetWriteMask(pageWindow->RPort, 0x3);
			SetDrMd(pageWindow->RPort, JAM2 | COMPLEMENT);
			Move(pageWindow->RPort, 0, y);
			Draw(pageWindow->RPort, pageWindow->Width-1, y);
		}

		loop=TRUE;

		while(loop)
		{
			signals = Wait(SIGNALMASK);
			if (signals & SIGNALMASK)
			{
				mouseMoved=FALSE;
				while(message = (struct IntuiMessage *)GetMsg(capsPort))
				{
					CED.Class		= message->Class;
					CED.Code		= message->Code;
					CED.MouseX	= pageWindow->MouseX;
					CED.MouseY	= pageWindow->MouseY;
					ReplyMsg((struct Message *)message);
					switch(CED.Class)
					{
						case IDCMP_RAWKEY:
							if (CED.Code == RAW_ESCAPE)
							{
								drawIt=FALSE;
								loop=FALSE;
							}
							break;
						case IDCMP_MOUSEBUTTONS:
							if (CED.Code == SELECTUP)
							{
								drawIt=TRUE;
								loop=FALSE;
							}
							break;
						case IDCMP_MOUSEMOVE:
							mouseMoved=TRUE;
							break;
					}
				}
				if (mouseMoved)
				{
					if (center==CENTER_H)
					{
						SafeSetWriteMask(pageWindow->RPort, 0x3);
						SetDrMd(pageWindow->RPort, JAM2 | COMPLEMENT);
						Move(pageWindow->RPort, x, 0);
						Draw(pageWindow->RPort, x, pageWindow->Height-1);
					}
					else
					{
						SafeSetWriteMask(pageWindow->RPort, 0x3);
						SetDrMd(pageWindow->RPort, JAM2 | COMPLEMENT);
						Move(pageWindow->RPort, 0, y);
						Draw(pageWindow->RPort, pageWindow->Width-1, y);
					}
					x = CED.MouseX;
					y = CED.MouseY;
					if (center==CENTER_H)
					{
						SafeSetWriteMask(pageWindow->RPort, 0x3);
						SetDrMd(pageWindow->RPort, JAM2 | COMPLEMENT);
						Move(pageWindow->RPort, x, 0);
						Draw(pageWindow->RPort, x, pageWindow->Height-1);
					}
					else
					{
						SafeSetWriteMask(pageWindow->RPort, 0x3);
						SetDrMd(pageWindow->RPort, JAM2 | COMPLEMENT);
						Move(pageWindow->RPort, 0, y);
						Draw(pageWindow->RPort, pageWindow->Width-1, y);
					}
				}
			}
		}

		if (center==CENTER_H)
		{
			DrawHoriBars();
			SafeSetWriteMask(pageWindow->RPort, 0x3);
			SetDrMd(pageWindow->RPort, JAM2 | COMPLEMENT);
			Move(pageWindow->RPort, x, 0);
			Draw(pageWindow->RPort, x, pageWindow->Height-1);
		}
		else
		{
			DrawVertBars();
			SafeSetWriteMask(pageWindow->RPort, 0x3);
			SetDrMd(pageWindow->RPort, JAM2 | COMPLEMENT);
			Move(pageWindow->RPort, 0, y);
			Draw(pageWindow->RPort, pageWindow->Width-1, y);
		}

		SafeSetWriteMask(pageWindow->RPort, 0xff);

		if (aWin!=NULL)
			UA_SwitchMouseMoveOff(aWin);

		for(i=0; i<MAXEDITWINDOWS; i++)
			list[i] = FALSE;

		if (drawIt)
		{
			DrawAllHandles(LEAVE_ACTIVE);
			for(i=0; i<MAXEDITWINDOWS; i++)
			{
				if ( EditSupportList[i]!=NULL && EditSupportList[i]->Active )
				{
					if ( center==CENTER_H )
					{
						new = x - (EditWindowList[i]->Width/2);
						ex = EditWindowList[i]->X;
						ey = EditWindowList[i]->Y;
						CheckBoundingBox(	&new, &ey, EditWindowList[i]->Width,
															EditWindowList[i]->Height);
						EditWindowList[i]->X = new;
					}
					else
					{
						new = y - (EditWindowList[i]->Height/2);
						ex = EditWindowList[i]->X;
						ey = EditWindowList[i]->Y;
						CheckBoundingBox(	&ex, &new, EditWindowList[i]->Width,
															EditWindowList[i]->Height);
						EditWindowList[i]->Y = new;
					}

					list[ i ] = TRUE;

					CorrectEW(EditWindowList[i]);

					prevX[ i ] = ex;
					prevY[ i ] = ey;
					prevW[ i ] = EditWindowList[i]->Width;
					prevH[ i ] = EditWindowList[i]->Height;

					newX[ i ] = EditWindowList[i]->X;
					newY[ i ] = EditWindowList[i]->Y;
					newW[ i ] = EditWindowList[i]->Width;
					newH[ i ] = EditWindowList[i]->Height;
				}
			}

			RedrawAllOverlapWdwListPrev(prevX,prevY,prevW,prevH, newX,newY,newW,newH, list);

			DrawAllHandles(LEAVE_ACTIVE);
		}
	}
}

/******** CheckBoundingBox() ********/

void CheckBoundingBox(WORD *x, WORD *y, WORD w, WORD h)
{
	if (*x < 0)
		*x=0;

	if (*y < 0)
		*y=0;

	if ( (*x + w) > CPrefs.PageScreenWidth)		// 0+640
		*x = CPrefs.PageScreenWidth - w;				// 640-640

	if ( (*y + h) > CPrefs.PageScreenHeight)
		*y = CPrefs.PageScreenHeight - h;

	if (*x < 0)
		*x=0;

	if (*y < 0)
		*y=0;
}

/******** DrawHoriBars() ********/

void DrawHoriBars(void)
{
int i,pos;

	SetDrPt(pageWindow->RPort, 0x3333);	// 0011001100110011
	SetDrMd(pageWindow->RPort, JAM1 | COMPLEMENT);
	SafeSetWriteMask(pageWindow->RPort, 0x3);

	for(i=0; i<=5; i++)
	{
		pos = (pageWindow->Width/6)*i;
		Move(pageWindow->RPort, pos, 0);
		Draw(pageWindow->RPort, pos, pageWindow->Height-1);
	}
	Move(pageWindow->RPort, pageWindow->Width-1, 0);
	Draw(pageWindow->RPort, pageWindow->Width-1, pageWindow->Height-1);

	SetDrPt(pageWindow->RPort, 0xffff);
	SetDrMd(pageWindow->RPort, JAM1);
	SafeSetWriteMask(pageWindow->RPort, 0xff);
}

/******** DrawVertBars() ********/

void DrawVertBars(void)
{
int i,pos;

	SetDrPt(pageWindow->RPort, 0x3333);	// 0011001100110011
	SetDrMd(pageWindow->RPort, JAM1 | COMPLEMENT);
	SafeSetWriteMask(pageWindow->RPort, 0x3);

	for(i=0; i<=5; i++)
	{
		pos = (pageWindow->Height/6)*i;
		Move(pageWindow->RPort, 0, pos);
		Draw(pageWindow->RPort, pageWindow->Width-1, pos);
	}
	Move(pageWindow->RPort, 0, pageWindow->Height-1);
	Draw(pageWindow->RPort, pageWindow->Width-1, pageWindow->Height-1);

	SetDrPt(pageWindow->RPort, 0xffff);
	SetDrMd(pageWindow->RPort, JAM1);
	SafeSetWriteMask(pageWindow->RPort, 0xff);
}

/******** E O F ********/
