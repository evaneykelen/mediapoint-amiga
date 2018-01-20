#include "nb:pre.h"
#include "nb:xapp_names.h"

#define LOOKUPLISTSIZE 256
#define IN_SCRIPT	1
#define IN_PAGE		2

/**** externals ****/

extern ULONG allocFlags;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct MsgPort *capsPort;
extern struct Library *medialinkLibBase;
extern struct Window *pageWindow;
extern struct Window *scriptWindow;
extern struct ObjectInfo ObjectRecord;
extern UWORD chip gui_pattern[];
extern TEXT *dir_system;
extern struct BitMap effBM;
extern struct eventHandlerInfo EHI;
extern struct Gadget ScriptSlider2;
extern struct MenuRecord **script_MR;
extern LONG topEntry1, topEntry2;
extern struct TextFont *smallFont;
extern struct TextFont *largeFont;

/**** globals ****/

UWORD *lookUpList_Eff		= NULL;
UWORD *lookUpList_Brush	= NULL;
BOOL blockScript=FALSE;

/**** static globals ****/

static struct PropInfo PI1 = { AUTOKNOB | FREEVERT | PROPBORDERLESS, 0,0,0,0, };
static struct Image Im1 = { 0,0,0,0,0,NULL,0x0000,0x0000,NULL };
struct Gadget PropSlider1 =
{
	NULL, 0,0,0,0, NULL, GACT_RELVERIFY | GACT_IMMEDIATE, GTYP_PROPGADGET,
	&Im1, NULL, NULL, NULL, (struct PropInfo *)&PI1, 1,NULL
};

int NUMEFFECTS = 0;	// see also fasteffect.c
int NUMBRUSHES = 0;	// see also effect.c & fasteffect.c

UBYTE *effectNames		= NULL, *brushNames		= NULL;
UWORD *effectNumbers	= NULL, *brushNumbers	= NULL;

static ULONG effectNamesSize, brushNamesSize, effectBMHeight;
ULONG effDoubled=0;

static UBYTE **effectList	= NULL;		// array of ptrs
UBYTE **brushList	= NULL;		// array of ptrs

/**** gadgets ****/

extern struct GadgetRecord PLS_ChooseEffect_GR[];
extern struct GadgetRecord Script_GR[];
extern struct GadgetRecord ChooseEffect_GR[];

/**** functions ****/

/******** Monitor_Effect() ********/
/*
 * if type==1 (IN_SCRIPT) this req. is used for the Script Editor.
 * if type==2 (IN_PAGE) this req. is used for the PLS.
 * 
 * The req. in the script editor can do things in the object list like scrolling,
 * etc., select effects for 1 or more files etc. etc.
 *
 * The req. in the PLS can only go in with an effect, select one or choose no effect
 * and go out again.
 *
 */

BOOL Monitor_Effect(WORD *p1, WORD *p2, WORD *p3, int type)
{
int numEntries, topEntry, line, thickMax, thickPos, val, ID, i, dummy, numDisp;
UBYTE *selectionList;
WORD effect, speed, thick;
struct Window *window, *activeWindow;
WORD *cancelEffNr	= NULL;
WORD *cancelSpeed	= NULL;
WORD *cancelThick	= NULL;
struct ScriptNodeRecord *this_node;
BOOL loop=TRUE, retval=FALSE, dblClicked=FALSE;
struct GadgetRecord *GR;
static int topEntry_1=0, topEntry_2=0;
struct ScrollRecord SR;

	if ( type==1 )
		topEntry = topEntry_1;
	else
		topEntry = topEntry_2;

	/**** init vars ****/

	blockScript=FALSE;

	if ( type == IN_SCRIPT )
	{
		numEntries = NUMEFFECTS;
		numDisp = 13;
		blockScript=TRUE;
	}
	else
	{
		numEntries = NUMBRUSHES;
		numDisp = 10;
		//SetStandardColors(pageWindow);
	}

	line=-1;
	cancelEffNr=NULL;
	cancelSpeed=NULL;
	cancelThick=NULL;

	if ( type == IN_SCRIPT )
		GR = ChooseEffect_GR;
	else
		GR = PLS_ChooseEffect_GR;

	/**** scan page xapp for effect graphics and effect names ****/

	if ( !TestBit(allocFlags, EFFECTINFO_FLAG) )
	{
		if ( !GetInfoFromPageXaPP() )
		{
			blockScript=FALSE;
			return(FALSE);
		}
	}

	/**** disable some script screen buttons ****/

	if ( type == IN_SCRIPT )
	{
		DisableAllEventIcons();
		UA_DisableButton(scriptWindow, &Script_GR[4], gui_pattern);	// Play
		UA_DisableButton(scriptWindow, &Script_GR[7], gui_pattern);	// edit
		UA_DisableButton(scriptWindow, &Script_GR[8], gui_pattern);	// show
		if ( CPrefs.userLevel > 2 )
			UA_DisableButton(scriptWindow, &Script_GR[5], gui_pattern);	// Parent
		OffGadget(&ScriptSlider2, scriptWindow, NULL);
	}

	/**** open requester ****/

	if ( type == IN_SCRIPT )
		window = scriptWindow;
	else
		window = pageWindow;

	window = OpenEffectWindow(window, GR, numEntries, topEntry, numDisp);
	if ( !window )
	{
		blockScript=FALSE;
		return(FALSE);
	}

	selectionList = (UBYTE *)AllocMem(numEntries, MEMF_ANY | MEMF_CLEAR);
	if (selectionList==NULL)
	{
		UA_CloseRequesterWindow(window,STDCOLORS);
		UA_WarnUser(105);
		blockScript=FALSE;
		return(FALSE);
	}

	/**** see if/which effect is selected ****/

	if ( type == IN_SCRIPT )
	{
		effect	= -1;
		speed		= 0;
		thick		= 0;

		for(this_node=ObjectRecord.firstObject; this_node->node.ln_Succ;
				this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
		{
			if (	this_node->miscFlags & OBJ_SELECTED &&
						(this_node->nodeType == TALK_ANIM || this_node->nodeType == TALK_PAGE) )
			{
				if ( this_node->numericalArgs[2] != -1 )
				{
					effect	= this_node->numericalArgs[2];
					speed		= this_node->numericalArgs[3];
					thick		= this_node->numericalArgs[4];
					break;
				}
			}
		}

		if ( effect != -1 )
		{
			*( selectionList + lookUpList_Eff[ effect ] ) = 1;
			line = lookUpList_Eff[ effect ] - topEntry;
		}
	}
	else
	{
		effect	= *p1;	// can be -1 !
		speed		= *p2;
		thick		= *p3;

		if ( effect==-1 )
			UA_DisableButton(window, &GR[4], gui_pattern);	// OK

		if ( effect != -1 )
		{
			*( selectionList + lookUpList_Brush[ effect ] ) = 1;
			line = lookUpList_Brush[ effect ] - topEntry;
		}
	}

	/**** init scroll record ****/

	SR.GR							= &GR[2];
	SR.window					= window;
	SR.list						= NULL;
	SR.sublist				= NULL;
	SR.selectionList	= &selectionList[0];
	SR.entryWidth			= -1;
	SR.numDisplay			= numDisp;
	SR.numEntries			= numEntries;

	GR[2].x1 = GR[2].x1 + 25; 
	UA_PrintStandardList(NULL,-1,NULL);	// init static

	if ( type == IN_SCRIPT )
	{
		if (CPrefs.ScriptScreenModes & LACE)
			DoubleEffBM(TRUE);
		else
			DoubleEffBM(FALSE);
	}
	else
	{
		if (CPrefs.PageScreenModes & LACE)
			DoubleEffBM(TRUE);
		else
			DoubleEffBM(FALSE);
	}

	if ( type == IN_SCRIPT )
		UA_PrintStandardList(&SR,topEntry,effectList);
	else
		UA_PrintStandardList(&SR,topEntry,brushList);

	PrintIconList(&GR[2], window, topEntry, numDisp);

	/**** init speed and thick gadgets ****/

	if ( effect == -1 )	// no effect selected yet, ghost speed and chunck
	{
		UA_SetSliderGadg(window, &GR[7], 0, 20, &GR[9], 1);			// speed
		UA_SetSliderGadg(window, &GR[8], 0, 10, &GR[10], 1);		// chunck
		UA_DisableButton(window, &GR[7], gui_pattern);					// speed
		UA_DisableButton(window, &GR[8], gui_pattern);					// chunck
	}
	else
	{
		/********** SET SLIDERS ************/
		UA_SetSliderGadg(window, &GR[7], speed-1, 20, &GR[9], 1);
		GetThickMax(effect, &thickMax);	// returns 1,2,3 or 4
		ConvertThickToChunck(thick, &thickPos, &thickMax);
		thick = thickPos;
		
		if ( thickPos!=-1 )
			UA_SetSliderGadg(window, &GR[8], thick, thickMax, &GR[10], 1);
		else	// this is a no chunck effect
		{
			UA_SetSliderGadg(window, &GR[8], 0, 10, &GR[10], 1);	// chunck
			UA_DisableButton(window, &GR[8], gui_pattern);				// chunck
		}
		/********** SLIDERS SET ************/
	}

	/**** allocate undo buffer ****/

	if ( type == IN_SCRIPT )
	{
		//GetNumObjects();	// fills ObjectRecord
		cancelEffNr	= (WORD *)AllocMem(ObjectRecord.numObjects*2, MEMF_CLEAR);
		cancelSpeed	= (WORD *)AllocMem(ObjectRecord.numObjects*2, MEMF_CLEAR);
		cancelThick	= (WORD *)AllocMem(ObjectRecord.numObjects*2, MEMF_CLEAR);
		if ( cancelEffNr==NULL || cancelSpeed==NULL || cancelThick==NULL ) //|| cancelVari==NULL )
		{
			if ( cancelEffNr!=NULL )
				FreeMem(cancelEffNr,ObjectRecord.numObjects*2);
			if ( cancelSpeed!=NULL )
				FreeMem(cancelSpeed,ObjectRecord.numObjects*2);
			if ( cancelThick!=NULL )
				FreeMem(cancelThick,ObjectRecord.numObjects*2);
			UA_CloseRequesterWindow(window,STDCOLORS);
			UA_WarnUser(-1);
			blockScript=FALSE;
			return(FALSE);	
		}

		/**** save all transitions to undo buffer ****/

		for(i=0,this_node=ObjectRecord.firstObject; this_node->node.ln_Succ;
				i++,this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
		{
			cancelEffNr[i]	= this_node->numericalArgs[2];
			cancelSpeed[i]	= this_node->numericalArgs[3];
			cancelThick[i]	= this_node->numericalArgs[4];
		}
	}

	/**** monitor user ****/

	while(loop)
	{
		doStandardWait(window);

		if (CED.extraClass == DBLCLICKED)
			dblClicked=TRUE;
		else
			dblClicked=FALSE;

		Forbid();
		activeWindow = IntuitionBase->ActiveWindow;
		Permit();

		/**** process script editor window events ****/

		if ( (type==IN_SCRIPT) && activeWindow==scriptWindow )
		{
			if (CED.Class==IDCMP_MOUSEBUTTONS)
			{
				if ( doScriptMouseButtons(&dummy) )
				{
					for(i=0; i<numEntries; i++)	// clear list
						*(selectionList+i) = 0;
					UA_PrintStandardList(NULL,-1,NULL);	// init static
					this_node = (struct ScriptNodeRecord *)CountNumSelected(ObjectRecord.firstObject,&i);
					if (this_node && i>0)
					{
						effect	= this_node->numericalArgs[2];
						speed		= this_node->numericalArgs[3];
						thick		= this_node->numericalArgs[4];
						*(selectionList + lookUpList_Eff[ effect ] ) = 1;	// hilite effect
						line = lookUpList_Eff[ effect ] - topEntry;				// calculate selected line
					}
					UA_PrintStandardList(&SR,topEntry,effectList);
					if ( UA_IsGadgetDisabled(&GR[7]) )
						UA_EnableButton(window, &GR[7]);	// speed
					if ( UA_IsGadgetDisabled(&GR[8]) )
						UA_EnableButton(window, &GR[8]);	// chunck
					/********** SET SLIDERS ************/
					UA_SetSliderGadg(window, &GR[7], speed-1, 20, &GR[9], 1);
					GetThickMax(effect, &thickMax);	// returns 1,2,3 or 4
					ConvertThickToChunck(thick, &thickPos, &thickMax);
					thick = thickPos;
					if ( thickPos!=-1 )
						UA_SetSliderGadg(window, &GR[8], thick, thickMax, &GR[10], 1);
					else	// this is a no chunck effect
					{
						UA_SetSliderGadg(window, &GR[8], 0, 10, &GR[10], 1);	// chunck
						UA_DisableButton(window, &GR[8], gui_pattern);				// chunck
					}
					/********** SLIDERS SET ************/
				}
			}
			else if (CED.Class==IDCMP_RAWKEY)
			{
				dokeyScrolling();
				DoSelAll();
			}
			CED.Class = NULL;
		}
		else if ( (type==IN_PAGE) && activeWindow!=window )
			CED.Class = NULL;

		/**** process effect window events ****/

		if (CED.Class==IDCMP_MOUSEBUTTONS)
		{
			if ( CED.extraClass==IDCMP_GADGETDOWN || CED.extraClass==IDCMP_GADGETUP )
			{
				if ( type==IN_SCRIPT )
					ScrollEffectList(	&GR[2], window, NULL, selectionList, -1,
														numEntries, &topEntry, numDisp, &PropSlider1, effectList, &SR);
				else
					ScrollEffectList(	&GR[2], window, NULL, selectionList, -1,
														numEntries, &topEntry, numDisp, &PropSlider1, brushList, &SR);
				CED.Code = 0;
			}

			/**** process buttons ****/

			if (CED.Code==SELECTDOWN)
			{
				GR[2].x1 = GR[2].x1 - 25; 
				ID = UA_CheckGadgetList(window, GR, &CED);
				GR[2].x1 = GR[2].x1 + 25; 
				switch(ID)
				{
					case 2:	// scroll area
						line = UA_SelectStandardListLine(&SR,topEntry,FALSE,&CED,FALSE,FALSE);
						if (line!=-1)
						{
							if ( UA_IsGadgetDisabled(&GR[4]) )
								UA_EnableButton(window, &GR[4]);	// OK
							if ( UA_IsGadgetDisabled(&GR[7]) )
								UA_EnableButton(window, &GR[7]);	// speed
							if ( UA_IsGadgetDisabled(&GR[8]) )
								UA_EnableButton(window, &GR[8]);	// chunck

							if ( type==IN_SCRIPT )
								effect = *( effectNumbers + (5*(topEntry+line)) );
							else
								effect = *( brushNumbers + (5*(topEntry+line)) );

							/**** effect is now known: derive all other vars from it ****/

							/**** speed ****/

							speed = GetSpeed(effect);
							UA_SetSliderGadg(window, &GR[7], speed-1, 20, &GR[9], 1);

							/**** chunck size ****/

							thick = GetThick(effect);
							GetThickMax(effect, &thickMax);	// returns 1,2,3 or 4
							ConvertThickToChunck(thick, &thickPos, &thickMax);
							thick = thickPos;

							if ( thick!=-1 )
								UA_SetSliderGadg(window, &GR[8], thick, thickMax, &GR[10], 1);
							else	// this is a no chunck effect
							{
								UA_SetSliderGadg(window, &GR[8], 0, 10, &GR[10], 1);	// chunck
								UA_DisableButton(window, &GR[8], gui_pattern);				// chunck
							}

							/**** effect, speed and thick are now set ****/

							if ( type==IN_SCRIPT )
							{
								SetAllObj(effect, speed, thick); //, vari);
								DrawTransitionBlocks(topEntry1);
							}

							if (dblClicked)
								goto do_ok;
						}
						else
							effect = -1;
						break;

					case 4:	// OK
do_ok:
						UA_HiliteButton(window, &GR[4]);
						retval=TRUE;
						loop=FALSE;
						break;

					case 5:	// Cancel
do_cancel:
						UA_HiliteButton(window, &GR[5]);
						retval=FALSE;
						loop=FALSE;
						break;

					case 13:	// no effect
						UA_HiliteButton(window, &GR[13]);
						retval=TRUE;
						loop=FALSE;
						effect	= -1;
						speed		= 20;
						thick		= 0;
						break;

					case 7:	// speed
						val = speed-1;
						UA_ProcessSliderGadg(window, &GR[ID], &val, 20, &GR[9], &CED, NULL,NULL,0,1);
						speed = val+1;
						if ( type==IN_SCRIPT )
							SetAllObj(effect,speed,thick);
						break;

					case 8:	// chunck size
						val = thick;
						UA_ProcessSliderGadg(window, &GR[ID], &val, thickMax, &GR[10], &CED, NULL,NULL,0,1);
						thick = val;
						if ( type==IN_SCRIPT )
							SetAllObj(effect,speed,thick);
						break;
				}
			}
		}
		else if (CED.Class==IDCMP_RAWKEY )
		{
			if (CED.Code==RAW_ESCAPE)	// cancel
				goto do_cancel;
			else if ( CED.Code==RAW_RETURN && !UA_IsGadgetDisabled(&GR[4]) )	// OK
				goto do_ok;
			else if ( type==IN_SCRIPT )
				DoSelAll();
		}
	}

	blockScript=FALSE;

	UA_EnableButton(window, &GR[4]);	// OK
	UA_EnableButton(window, &GR[7]);	// speed
	UA_EnableButton(window, &GR[8]);	// chunck

	UA_CloseRequesterWindow(window,STDCOLORS);

	if (retval)	// OK
	{
		if ( type==IN_SCRIPT )
			DrawTransitionBlocks(topEntry1);
		else
		{
			*p1 = effect;	// can be -1 !
			*p2 = speed;
			*p3 = thick;	

			GetThickMax(effect, &thickMax);	// returns 1,2,3 or 4
			ConvertChunckToThick(thick, &val, thickMax);	// NEW
			*p3=val;																			// NEW
			//if ( thickMax==3 || thickMax==4 )						// OLD
			//	*p3 = *p3 + 1;														// OLD
		}
	}
	else	// cancel
	{
		if ( type==IN_SCRIPT )
		{
			/**** save all transitions ****/
			i=0;
			for(i=0,this_node=ObjectRecord.firstObject; this_node->node.ln_Succ;
					i++,this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
			{
				SetByteBit(&this_node->miscFlags, OBJ_NEEDS_REFRESH);
				this_node->numericalArgs[2] = cancelEffNr[i];
				this_node->numericalArgs[3] = cancelSpeed[i];
				this_node->numericalArgs[4] = cancelThick[i];
			}
			//FreeMem(cancelEffNr,ObjectRecord.numObjects*2);
			//FreeMem(cancelSpeed,ObjectRecord.numObjects*2);
			//FreeMem(cancelThick,ObjectRecord.numObjects*2);
			DrawTransitionBlocks(topEntry1);
		}
	}

	if ( type==IN_SCRIPT )
	{
		FreeMem(cancelEffNr,ObjectRecord.numObjects*2);
		FreeMem(cancelSpeed,ObjectRecord.numObjects*2);
		FreeMem(cancelThick,ObjectRecord.numObjects*2);
	}

	FreeMem(selectionList, numEntries);

	GR[2].x1 = GR[2].x1 - 25; 

	if ( type==IN_SCRIPT )
	{
		EnableAllEventIcons();

		if (CPrefs.ScriptScreenModes & LACE)
			SetFont(scriptWindow->RPort, largeFont);

		UA_EnableButton(scriptWindow, &Script_GR[4]);	// Play
		if ( CPrefs.userLevel > 2 )
			UA_EnableButton(scriptWindow, &Script_GR[5]);	// Parent

		SetFont(scriptWindow->RPort, smallFont);

		ScriptSlider2On();
	}

	if ( type==1 )
		topEntry_1 = topEntry;
	else
		topEntry_2 = topEntry;

	return(retval);
}

/******** PrintIconList() ********/

void PrintIconList(	struct GadgetRecord *GR, struct Window *window,
										int top, int displayLines)
{
int i,x,y,lineHeight,add=0,h,h2;

	if ( EHI.activeScreen == STARTSCREEN_PAGE )
		add=NUMEFFECTS;

	if ( effDoubled==1 )
	{
		h=16;
		h2=18;
	}
	else
	{
		h=8;
		h2=9;
	}

	lineHeight = (GR->y2 - GR->y1) / displayLines;
	for(i=0; i<displayLines; i++)
	{
		x = ((top+i+add) % 20) * 32;
		y = ((top+i+add) / 20) * h;
		RenderEffectIcon(window->RPort,x,y,GR->x1-20,GR->y1+i*lineHeight+2,21,h2);
	}
}

/******** ScrollEffectList() ********/

void ScrollEffectList(	struct GadgetRecord *GR, struct Window *window,
												UBYTE *list, UBYTE *selectionList,
												int entryWidth, int numEntries, int *top,
												int numDisplay, struct Gadget *g, TEXT *ptrlist[],
												struct ScrollRecord *SR)
{
ULONG signals;
BOOL loop=TRUE, mouseMoved=FALSE;
struct IntuiMessage *message;
LONG f;

	if ( CED.Qualifier & IEQUALIFIER_LSHIFT || CED.Qualifier & IEQUALIFIER_RSHIFT )
	{
		f = ( (CED.MouseY - g->TopEdge) * numEntries) / g->Height;
		if ( f < 0 )
			f = 0;
		*top = f;
		if ( (*top+numDisplay) > numEntries )
			*top = numEntries-numDisplay;
		UA_SetPropSlider(window, g, numEntries, numDisplay, *top);
		UA_PrintStandardList(SR, *top, ptrlist);
		PrintIconList(GR, window, *top, numDisplay);
		return;
	}

	UA_GetPropSlider(window, g, numEntries, numDisplay, top);
	UA_PrintStandardList(SR,*top,ptrlist);
	PrintIconList(GR, window, *top, numDisplay);

	UA_SwitchMouseMoveOn(window);

	while(loop)
	{
		signals = Wait(SIGNALMASK);
		if (signals & SIGNALMASK)
		{
			mouseMoved=FALSE;
			while(message = (struct IntuiMessage *)GetMsg(capsPort))
			{
				CED.Class	= message->Class;
				ReplyMsg((struct Message *)message);
				if ( CED.Class == IDCMP_MOUSEMOVE )
					mouseMoved=TRUE;
				else
					loop=FALSE;
			}
			if (mouseMoved)
			{
				if (g->Flags & GFLG_SELECTED)
				{
					UA_GetPropSlider(window, g, numEntries, numDisplay, top);
					UA_PrintStandardList(SR,*top,ptrlist);
					PrintIconList(GR, window, *top, numDisplay);
					loop=TRUE;
				}
				else
					loop=FALSE;
			}
		}
	}

	UA_SwitchMouseMoveOff(window);
}

/******** GetEffectPos() ********/

void GetEffectPos(int effNr, WORD *x, WORD *y)
{
int i,h;

	if ( effDoubled==1 )
		h=16;
	else
		h=8;

	*x=-1;
	*y=-1;

	if ( EHI.activeScreen == STARTSCREEN_SCRIPT )
	{
		for(i=0; i<NUMEFFECTS; i++)
		{
			if ( *(effectNumbers+(5*i)) == effNr )
			{
				*x = (i % 20) * 32;
				*y = (i / 20) * h;
				return;
			}
		}	
	}
	else
	{
		for(i=0; i<NUMBRUSHES; i++)
		{
			if ( *(brushNumbers+(5*i)) == effNr )
			{
				*x = ((i+NUMEFFECTS) % 20) * 32;
				*y = ((i+NUMEFFECTS) / 20) * h;
				return;
			}
		}	
	}
}

/******** GetThickMax() ********/

void GetThickMax(int effNr, int *max)
{
int i;

	*max=-1;

	for(i=0; i<NUMEFFECTS; i++)
	{
		if ( EHI.activeScreen == STARTSCREEN_SCRIPT )
		{
			if ( *(effectNumbers+(5*i)) == effNr )
			{
				*max = (int)*(effectNumbers+(5*i)+4);
				return;
			}
		}
		else
		{
			if ( *(brushNumbers+(5*i)) == effNr )
			{
				*max = (int)*(brushNumbers+(5*i)+4);
				return;
			}
		}
	}	
}

/******** GetSpeed() ********/

int GetSpeed(int effNr)
{
int i,speed=0;

	if ( EHI.activeScreen == STARTSCREEN_SCRIPT )
	{
		for(i=0; i<NUMEFFECTS; i++)
			if ( *(effectNumbers+(5*i)) == effNr )
				return ( (int)*(effectNumbers+(5*i)+1) );
	}
	else
	{
		for(i=0; i<NUMBRUSHES; i++)
			if ( *(brushNumbers+(5*i)) == effNr )
				return ( (int)*(brushNumbers+(5*i)+1) );
	}

	return(speed);
}

/******** GetThick() ********/

int GetThick(int effNr)
{
int i,thick=0;

	if ( EHI.activeScreen == STARTSCREEN_SCRIPT )
	{
		for(i=0; i<NUMEFFECTS; i++)
			if ( *(effectNumbers+(5*i)) == effNr )
				return ( (int)*(effectNumbers+(5*i)+2) );
	}
	else
	{
		for(i=0; i<NUMBRUSHES; i++)
			if ( *(brushNumbers+(5*i)) == effNr )
				return ( (int)*(brushNumbers+(5*i)+2) );
	}

	return(thick);
}

/******** ConvertThickToChunk() ********/
/*
 *	thickMax is 1:	1,2,4,8,16
 *							2:	1..10
 *							3:	no thick
 *							4:	1,2
 *
 *	out: thickMax is real max
 */

void ConvertThickToChunck(int inThick, int *outThick, int *thickMax)
{
	switch(*thickMax)
	{
		case 1:
			if (inThick<=1) *outThick=0;
			else if (inThick==2) *outThick=1;
			else if (inThick==4) *outThick=2;
			else if (inThick==8) *outThick=3;
			else if (inThick>=16) *outThick=4;
			*thickMax=5;
			break;

		case 2:
			*outThick=inThick;
			*thickMax=10;
			break;

		case 3:
			*outThick=-1;
			break;

		case 4:
			if (inThick<=1) *outThick=0;
			else if (inThick>=2) *outThick=1;
			*thickMax=2;
			break;
	}
}

/******** ConvertChunkToThick() ********/
/*
 *	thickMax is 1:	1,2,4,8,16
 *							2:	1..10
 *							3:	no thick
 *							4:	1,2
 */

void ConvertChunckToThick(int inChunck, int *outThick, int thickMax)
{
	switch(thickMax)
	{
		case 1:
			if (inChunck==0)			*outThick=1;
			else if (inChunck==1)	*outThick=2;
			else if (inChunck==2)	*outThick=4;
			else if (inChunck==3)	*outThick=8;
			else if (inChunck==4)	*outThick=16;
			break;

		case 2:
			*outThick = inChunck;
			break;

		case 3:
			*outThick=0;
			break;

		case 4:
			*outThick = inChunck + 1;
			break;
	}
}

/******** GetInfoFromPageXaPP() ********/

BOOL GetInfoFromPageXaPP(void)
{
TEXT fullPath[SIZE_FULLPATH];
struct ScriptNodeRecord this_node;
LONG *tl;
WORD h;
int i,j;

	effectNames		= NULL;
	brushNames		= NULL;
	effectNumbers	= NULL;
	brushNumbers	= NULL;

	//for(i=0; i<2; i++)
	effBM.Planes[0] = NULL;

	/**** create e.g. "work:medialink/xapps/system/music" ****/

	UA_MakeFullPath(dir_system, TRANSITIONS_XAPP, fullPath);

	this_node.numericalArgs[2]=-2;	// tells XaPP to return info

	if ( !InitXaPP(fullPath, &this_node, TRUE) )	// true means tiny
	{
		UA_WarnUser(106);
		return(FALSE);
	}

	/**** data is now stored in this_node ****/

	NUMEFFECTS = this_node.numericalArgs[8];
	NUMBRUSHES = this_node.numericalArgs[9];

	h = ( (NUMEFFECTS+NUMBRUSHES) / 20 ) * 11;

	h = h * 2;	// room for interlace
	effDoubled=0;

	effectBMHeight = h;

	InitBitMap(&effBM, 1, 640, h);

	//for(i=0; i<2; i++)
	//{

	effBM.Planes[0] = (PLANEPTR)AllocRaster(640,h);
	if ( effBM.Planes[0]==NULL )
	{
		UA_WarnUser(107);
		return(FALSE);
	}

	//	}

	/**** copy plane 0 ****/

	tl = (LONG *)&this_node.numericalArgs[0];
	tl = (LONG *)*tl;
	CopyMem(tl, effBM.Planes[0], 80*h);	// depends on width being 640!!!

#if 0
	/**** copy plane 1 ****/

	tl = (LONG *)&this_node.numericalArgs[2];
	tl = (LONG *)*tl;
	CopyMem(tl, effBM.Planes[1], 80*h);	// depends on width being 640!!!
#endif

	/**** alloc effectNames ****/

	effectNamesSize = this_node.numericalArgs[10];
	effectNames = (UBYTE *)AllocMem(effectNamesSize, MEMF_ANY);
	if (effectNames==NULL)
	{
		UA_WarnUser(108);
		return(FALSE);
	}

	/**** alloc effectNumbers ****/

	effectNumbers = (UWORD *)AllocMem(10*NUMEFFECTS, MEMF_ANY);	// 5 words per effect
	if (effectNumbers==NULL)
	{
		UA_WarnUser(109);
		return(FALSE);
	}

	/**** copy effect names ****/

	tl = (LONG *)&this_node.numericalArgs[4];
	tl = (LONG *)*tl;
	CopyMem(tl, effectNames, effectNamesSize);

	/**** copy effect numbers ****/

	tl = (LONG *)&this_node.numericalArgs[12];
	tl = (LONG *)*tl;
	CopyMem(tl, effectNumbers, 10*NUMEFFECTS);

	/**** alloc brushNames ****/

	brushNamesSize = this_node.numericalArgs[11];
	brushNames = (UBYTE *)AllocMem(brushNamesSize, MEMF_ANY);
	if (brushNames==NULL)
	{
		UA_WarnUser(110);
		return(FALSE);
	}

	/**** alloc brushNumbers ****/

	brushNumbers = (UWORD *)AllocMem(10*NUMBRUSHES, MEMF_ANY);	// 5 words per effect
	if (brushNumbers==NULL)
	{
		UA_WarnUser(111);
		return(FALSE);
	}

	/**** copy brush names ****/

	tl = (LONG *)&this_node.numericalArgs[6];
	tl = (LONG *)*tl;
	CopyMem(tl, brushNames, brushNamesSize);

	/**** copy brush numbers ****/

	tl = (LONG *)&this_node.numericalArgs[14];
	tl = (LONG *)*tl;
	CopyMem(tl, brushNumbers, 10*NUMBRUSHES);

	/**** alloc effectList ****/

	effectList = (UBYTE **)AllocMem(sizeof(UBYTE *)*NUMEFFECTS, MEMF_ANY);
	if (effectList==NULL)
	{
		UA_WarnUser(112);
		return(FALSE);
	}

	/**** alloc brushList ****/

	brushList = (UBYTE **)AllocMem(sizeof(UBYTE *)*NUMBRUSHES, MEMF_ANY);
	if (brushList==NULL)
	{
		UA_WarnUser(113);
		return(FALSE);
	}

	/**** fill effectList ****/

	i=0;
	j=0;
	while(i<NUMEFFECTS)
	{
		effectList[i] = effectNames+j;
		while( *(effectNames+j) != '\0' )
			j++;
		j++;
		i++;
	}

	/**** fill brushList ****/

	i=0;
	j=0;
	while(i<NUMBRUSHES)
	{
		brushList[i] = brushNames+j;
		while( *(brushNames+j) != '\0' )
			j++;
		j++;
		i++;
	}

	/**** allocate fast lookup table ****/

	lookUpList_Eff = (UWORD *)AllocMem(sizeof(UWORD)*LOOKUPLISTSIZE, MEMF_ANY | MEMF_CLEAR);
	if ( lookUpList_Eff==NULL )
	{
		UA_WarnUser(114);
		return(FALSE);
	}

	/**** store e.g. in [46] that the effect bitmap is at 0 ****/

	for(i=0; i<NUMEFFECTS; i++)
	{
		lookUpList_Eff[ effectNumbers[i*5] ] = i;
	}
	/**** allocate fast lookup table ****/

	lookUpList_Brush = (UWORD *)AllocMem(sizeof(UWORD)*LOOKUPLISTSIZE, MEMF_ANY | MEMF_CLEAR);
	if ( lookUpList_Brush==NULL )
	{
		UA_WarnUser(115);
		return(FALSE);
	}

	/**** store e.g. in [46] that the effect bitmap is at 0 ****/

	for(i=0; i<NUMBRUSHES; i++)
	{
		lookUpList_Brush[ brushNumbers[i*5] ] = i;
	}

	SetBit(&allocFlags, EFFECTINFO_FLAG);

	return(TRUE);
}

/******** FreeInfoFromPageXaPP() ********/

void FreeInfoFromPageXaPP(void)
{
	if ( !TestBit(allocFlags, EFFECTINFO_FLAG) )
		return;

	FreeMem(lookUpList_Eff, sizeof(UWORD)*LOOKUPLISTSIZE);
	FreeMem(lookUpList_Brush, sizeof(UWORD)*LOOKUPLISTSIZE);

	//for(i=0; i<2; i++)

	if ( effBM.Planes[0] )
		FreeRaster(effBM.Planes[0], 640, effectBMHeight);

	FreeMem(effectNames, effectNamesSize);
	FreeMem(effectNumbers, 10*NUMEFFECTS);

	FreeMem(brushNames, brushNamesSize);
	FreeMem(brushNumbers, 10*NUMBRUSHES);

	FreeMem(effectList, sizeof(UBYTE *)*NUMEFFECTS);
	FreeMem(brushList, sizeof(UBYTE *)*NUMBRUSHES);
}

/******** OpenEffectWindow() ********/

struct Window *OpenEffectWindow(struct Window *onWindow, struct GadgetRecord *GR,
																int numEntries, int topEntry, int numDisp)
{
struct Window *window;
//UBYTE *ptr;

	window = UA_OpenRequesterWindow(onWindow,GR,STDCOLORS);
	if (!window)
	{
		UA_WarnUser(104);
		return(NULL);
	}

	/**** init window and scroll bar ****/

	PropSlider1.LeftEdge	= GR[3].x1+4;
	PropSlider1.TopEdge		= GR[3].y1+2;
	PropSlider1.Width			= GR[3].x2-GR[3].x1-7;
	PropSlider1.Height		= GR[3].y2-GR[3].y1-3;

	if ( UA_IsWindowOnLacedScreen(onWindow) )
	{
		PropSlider1.TopEdge	+= 2;
		PropSlider1.Height	-= 4;
	}

	UA_DrawGadgetList(window, GR);

	InitPropInfo(	(struct PropInfo *)PropSlider1.SpecialInfo,
								(struct Image *)PropSlider1.GadgetRender);
	AddGadget(window, &PropSlider1, -1L);

	UA_SetPropSlider(window, &PropSlider1, numEntries, numDisp, topEntry);

	return(window);
}

/******** SetAllObj() ********/

void SetAllObj(WORD effect, WORD speed, WORD thick) //, WORD vari)
{
struct ScriptNodeRecord *this_node;
int thickMax, val;

	GetThickMax(effect,&thickMax);	// returns 1,2,3 or 4
	ConvertChunckToThick(thick, &val, thickMax);

	/**** put effect data in all currently selected objects ****/

	for(this_node=ObjectRecord.firstObject; this_node->node.ln_Succ;
			this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
	{
		if (	(this_node->miscFlags & OBJ_SELECTED) &&
					(this_node->nodeType == TALK_ANIM || this_node->nodeType == TALK_PAGE) )
		{
			SetByteBit(&this_node->miscFlags, OBJ_NEEDS_REFRESH);
			this_node->numericalArgs[2] = effect;
			this_node->numericalArgs[3] = speed;
			this_node->numericalArgs[4] = val; //thick;
		}
	}
}

/******** DoSelAll() ********/

void DoSelAll(void)
{
WORD ascii;

	if ( CED.Class==IDCMP_RAWKEY )
	{
		ascii = RawKeyToASCII(CED.Code);

		if ( ascii=='a' )
		{
			if ( CED.Qualifier&IEQUALIFIER_RCOMMAND )	//| CED.Qualifier&IEQUALIFIER_LCOMMAND )
			{
				SelectAllObjects();
				DrawObjectList(-1, TRUE, TRUE);
				if ( ObjectRecord.objList != ObjectRecord.scriptSIR.allLists[0] )
				{
					EnableMenu(script_MR[EDIT_MENU], EDIT_CUT);
					EnableMenu(script_MR[EDIT_MENU], EDIT_COPY);
				}
				EnableMenu(script_MR[EDIT_MENU], EDIT_CLEAR);
			}
		}

		switch(CED.Code)
		{
			case RAW_F1:
				SetTiming(CPrefs.F1_TIMECODE_STR, NULL, 128);
				break;
			case RAW_F2:
				SetTiming(CPrefs.F2_TIMECODE_STR, NULL, 128);
				break;
			case RAW_F3:
				SetTiming(CPrefs.F3_TIMECODE_STR, NULL, 128);
				break;
			case RAW_F4:
				SetTiming(CPrefs.F4_TIMECODE_STR, NULL, 128);
				break;
			case RAW_F5:
				SetTiming(CPrefs.F5_TIMECODE_STR, NULL, 128);
				break;
			case RAW_F6:
				SetTiming(CPrefs.F6_TIMECODE_STR, NULL, 128);
				break;
		}
	}
}

/******** DoubleEffBM() ********/

void DoubleEffBM(BOOL makeDouble)
{
int srcH, dstH, ys, yd;

	if ( makeDouble && effDoubled==0 )	// not scaled to double yet, make it twice the height
	{
		effDoubled=1;
		srcH = effectBMHeight/2;
		dstH = effectBMHeight;
	}
	else if ( !makeDouble && effDoubled==1 )	// scaled to double, make it halve the height
	{
		effDoubled=0;
		srcH = effectBMHeight;
		dstH = effectBMHeight/2;
	}
	else
		return;	// job done before

	if ( effDoubled==1 )	// make it twice the height
	{
		yd = dstH-1;
		for(ys=srcH-1; ys>=0; ys--)
		{
			BltBitMap(&effBM, 0, ys, &effBM, 0, yd,   640, 1, 0xc0, 0xff, NULL);
			BltBitMap(&effBM, 0, ys, &effBM, 0, yd-1, 640, 1, 0xc0, 0xff, NULL);
			yd -= 2;
		}
	}
	else if ( effDoubled==0 )	// make it halve the height
	{
		yd = 1;
		for(ys=2; ys<srcH-2; ys+=2)
		{
			BltBitMap(&effBM, 0, ys, &effBM, 0, yd, 640, 1, 0xc0, 0xff, NULL);
			yd++;
		}
	}
}

/******** E O F ********/
