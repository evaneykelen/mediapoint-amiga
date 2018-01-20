#include "nb:pre.h"

STATIC void GetEffectValues(WORD *p1, WORD *p2, WORD *p3, int pos);

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Screen *pageScreen;
extern struct Window *scriptWindow;
extern struct Screen *scriptScreen;
extern struct Library *medialinkLibBase;
extern struct ObjectInfo ObjectRecord;
extern ULONG effDoubled;
extern int NUMEFFECTS;
extern int NUMBRUSHES;
extern UBYTE *effectNames, *brushNames;
extern UWORD *effectNumbers, *brushNumbers;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;

/**** gadgets ****/

extern struct GadgetRecord FastEffect_GR[];

/**** functions ****/

BOOL ChooseEffectFast(void)
{
struct Window *window;
BOOL loop=TRUE, retval=FALSE;
int ID, i;
WORD x,y,lace=1,w,h,r,c,ex,ey,eh,eh2,p1,p2,p3;

	if ( NumActiveEditWindows()==0 )
		return(FALSE);

	/**** open a window ****/

	window = UA_OpenRequesterWindow(pageWindow, FastEffect_GR, STDCOLORS);
	if (!window)
	{
		UA_WarnUser(-1);
		return(FALSE);
	}

	/**** render gadgets ****/

	UA_DrawGadgetList(window, FastEffect_GR);

	/**** render no effect cross ****/

	if ( UA_IsWindowOnLacedScreen(window) )
		i=5;
	else
		i=3;
	PrintHorizText(window,&FastEffect_GR[3],i,"š\0");

	/**** calculate handy values ****/

	if ( UA_IsWindowOnLacedScreen(window) )
		lace=2;
	w = 24;	//141/6;
	h = 10*lace;	//(69*lace)/7;

	/**** render effect blocks ****/

	if (CPrefs.PageScreenModes & LACE)
		DoubleEffBM(TRUE);
	else
		DoubleEffBM(FALSE);

	ID=NUMEFFECTS;
	if ( effDoubled==1 )
	{
		eh=16;
		eh2=18;
	}
	else
	{
		eh=8;
		eh2=9;
	}
	for(r=0; r<7; r++)
	{
		for(c=0; c<6; c++)
		{
			ex = (ID % 20) * 32;
			ey = (ID / 20) * eh;
			ID++;
			x = c*w;
			y = r*h;
			RenderEffectIcon(window->RPort,ex,ey,x+7,y+3*lace,21,eh2);
			if ( ID-NUMEFFECTS == NUMBRUSHES )
				break;
		}
	}

	/**** highlight current effect ****/

	p1=-2;	// uninitialized
	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditSupportList[i] && EditSupportList[i]->Active )
		{
			if ( p1==-2 )
				p1 = EditWindowList[i]->in1[0];
			else if ( p1!=-2 )
			{
				if ( EditWindowList[i]->in1[0]!=p1 )
					p1=-3;	// this indicates that not all windows carry same effect
			}
		}
	}
	if ( p1>=0 )
	{
		x = p1 % 6;
		y = p1 / 6;
		if ( x+6*y < NUMBRUSHES )
		{
			SetDrMd(window->RPort,COMPLEMENT|JAM2);
			RectFill(window->RPort, 7+x*w, 3*lace+y*h, 7+x*w+w-4, 3*lace+y*h+h-2*lace-1+lace);
			SetDrMd(window->RPort,JAM1);
		}
	}
	else
		p1=-1;

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, FastEffect_GR, &CED);
			switch(ID)
			{
				case 2:	// Cancel
do_cancel:
					UA_HiliteButton(window, &FastEffect_GR[2]);
					loop=FALSE;
					retval=FALSE;
					break;
				case 3:	// no effect
					UA_HiliteButton(window, &FastEffect_GR[ID]);
					p1=-1;
					p2=0;
					p3=0;
					goto no_effect;
					break;
			}
			if ( loop )
			{
				if ( CED.MouseX>7 && CED.MouseX<147 && CED.MouseY>3*lace && CED.MouseY<71*lace )
				{
					x = CED.MouseX-7;
					y = CED.MouseY-3*lace;
					x = x / w;
					y = y / h;
					if ( x+6*y < NUMBRUSHES )
					{
						SetDrMd(window->RPort,COMPLEMENT|JAM2);
						RectFill(window->RPort, 7+x*w, 3*lace+y*h, 7+x*w+w-4, 3*lace+y*h+h-2*lace-1+lace);
						Delay(4L);
						SetDrMd(window->RPort,JAM1);
						GetEffectValues(&p1, &p2, &p3, x+6*y);
						if ( p1 != -1 )
						{
no_effect:
							for(i=0; i<MAXEDITWINDOWS; i++)
							{
								if ( EditSupportList[i] && EditSupportList[i]->Active )
								{
									EditWindowList[i]->in1[0] = p1;
									EditWindowList[i]->in2[0] = p2;
									EditWindowList[i]->in3[0] = p3;
									EditWindowList[i]->in1[1] = -1; EditWindowList[i]->in1[2] = -1;
									EditWindowList[i]->in2[1] = -1; EditWindowList[i]->in2[2] = -1;
									EditWindowList[i]->in3[1] = -1; EditWindowList[i]->in3[2] = -1;
									EditWindowList[i]->out1[0] = -1; EditWindowList[i]->out1[1] = -1; EditWindowList[i]->out1[2] = -1;
                  EditWindowList[i]->out2[0] = -1; EditWindowList[i]->out2[1] = -1; EditWindowList[i]->out2[2] = -1;
                  EditWindowList[i]->out3[0] = -1; EditWindowList[i]->out3[1] = -1; EditWindowList[i]->out3[2] = -1;
								}
							}
							loop=FALSE;
							retval=TRUE;
						}
					}
				}			
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE)	// cancel
				goto do_cancel;
		}
	}

	UA_CloseRequesterWindow(window,STDCOLORS);

	return(retval);
}

/******** GetEffectValues() ********/

STATIC void GetEffectValues(WORD *p1, WORD *p2, WORD *p3, int pos)
{
int thickMax, thickPos;
WORD effect, speed, thick;

	effect = *( brushNumbers + 5*pos );
	speed = GetSpeed(effect);
	thick = GetThick(effect);
	GetThickMax(effect, &thickMax);	// returns 1,2,3 or 4
	ConvertThickToChunck(thick, &thickPos, &thickMax);
	thick = thickPos;
	*p1 = effect;	// can be -1 !
	*p2 = speed;
	*p3 = thick;	
	GetThickMax(effect, &thickMax);	// returns 1,2,3 or 4
	if ( thickMax==3 || thickMax==4 )
		*p3 = *p3 + 1;
}

/******** E O F ********/
