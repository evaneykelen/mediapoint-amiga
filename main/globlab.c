#include "nb:pre.h"
#include "nb:keys.h"

/**** externals ****/

extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct MsgPort *capsPort;
extern struct Library *medialinkLibBase;
extern struct Window *pageWindow;
extern struct Window *scriptWindow;
extern struct ObjectInfo ObjectRecord;
extern UWORD chip gui_pattern[];
extern UBYTE **msgs;
extern struct Gadget PropSlider1;

/**** static globals ****/

static struct PropInfo PI2 = { AUTOKNOB | FREEVERT | PROPBORDERLESS, 0,0,0,0, };
static struct Image Im2 = { 0,0,0,0,0,NULL,0x0000,0x0000,NULL };
static struct Gadget PropSlider2 =
{
	NULL, 0,0,0,0, NULL, GACT_RELVERIFY | GACT_IMMEDIATE, GTYP_PROPGADGET,
	&Im2, NULL, NULL, NULL, (struct PropInfo *)&PI2, 2, NULL
};

/**** gadgets ****/

extern struct GadgetRecord LabelWdw_GR[];
extern struct GadgetRecord GlobalEventsWdw_GR[];
extern struct GadgetRecord KeyList_GR[];

/**** functions ****/

/******** Monitor_GlobalLabels() ********/

BOOL Monitor_GlobalLabels(struct Window *window)
{
int topEntry, numEntries, i, j;
int ID, displayLines, displayFactor, line=0;
BOOL loop=TRUE, retval, val, definedList[MAX_GLOBAL_EVENTS];
struct ScriptEventRecord *SER;
TEXT keyName[41], labelName[41], printStr[101];
struct Window *reqWindow;
BOOL dblClicked=FALSE;

	UA_DrawSpecialGadgetText(window, &GlobalEventsWdw_GR[2], msgs[Msg_AvailableKeys-1], SPECIAL_TEXT_LEFT);
	UA_DrawSpecialGadgetText(window, &GlobalEventsWdw_GR[3], msgs[Msg_AssignedLabels-1], SPECIAL_TEXT_LEFT);

	UA_DisableButton(window, &GlobalEventsWdw_GR[7], gui_pattern); /* label name */
	UA_DisableButton(window, &GlobalEventsWdw_GR[10], gui_pattern); /* delete label name */

	/**** init vars ****/

	for(i=0; i<NUMKEYS; i++)
		keySERs[i] = NULL;

	/**** remember which SERs are defined and which not,	****/
	/**** this is for easy clean-up												****/

	for(i=0; i<MAX_GLOBAL_EVENTS; i++)
	{
		if ( ObjectRecord.scriptSIR.globallocalEvents[i] != NULL )
			definedList[i] = TRUE;
		else
			definedList[i] = FALSE;
	}

	/**** first get the current global events settings ****/

	for(i=0; i<NUMKEYS; i++)
	{
		SER = ObjectRecord.scriptSIR.globallocalEvents[i];
		if (SER != NULL)
		{
			if (SER->keyCode != -1)
			{
				for(j=0; j<NUMKEYS; j++)
				{
					if ( keyListCodes[j] == SER->keyCode )
						break;
				}
				keySERs[j] = (struct ScriptEventRecord *)SER;
			}
			else if (SER->rawkeyCode != -1)
			{
				for(j=0; j<NUMKEYS; j++)
				{
					if ( keyListRawCodes[j] == SER->rawkeyCode )
						break;
				}
				keySERs[j] = (struct ScriptEventRecord *)SER;
			}
		}
	}

	/**** init window and scroll bar ****/

	displayLines = 7;

	if ( UA_IsWindowOnLacedScreen(window) )
		displayFactor=24;
	else
		displayFactor=12;

	PropSlider1.LeftEdge	= GlobalEventsWdw_GR[5].x1+4;
	PropSlider1.TopEdge		= GlobalEventsWdw_GR[5].y1+2;
	PropSlider1.Width			= GlobalEventsWdw_GR[5].x2-GlobalEventsWdw_GR[5].x1-7;
	PropSlider1.Height		= GlobalEventsWdw_GR[5].y2-GlobalEventsWdw_GR[5].y1-3;

	if ( UA_IsWindowOnLacedScreen(window) )
	{
		PropSlider1.TopEdge	+= 2;
		PropSlider1.Height	-= 4;
	}

	//UA_DrawGadgetList(window, GlobalEventsWdw_GR);

	InitPropInfo((struct PropInfo *)PropSlider1.SpecialInfo, (struct Image *)PropSlider1.GadgetRender);
	AddGadget(window, &PropSlider1, -1L);

	topEntry=0L;
	numEntries=NUMKEYS;	/* keys.h */
	UA_InitPropSlider(window, &PropSlider1, numEntries, displayLines, topEntry);

	DisplayGlobalList(window, &GlobalEventsWdw_GR[4], -1, displayLines, displayFactor, numEntries);

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if (CED.extraClass == DBLCLICKED)
			dblClicked=TRUE;
		else
			dblClicked=FALSE;

		switch(CED.Class)
		{
			case IDCMP_MOUSEBUTTONS:
				if ( CED.extraClass==IDCMP_GADGETDOWN || CED.extraClass==IDCMP_GADGETUP )
				{
					doGlobalListSlider(	window,
															NUMKEYS, displayLines, &topEntry,
															&GlobalEventsWdw_GR[4], &PropSlider1,
															displayFactor);
					CED.Code = 0;
					break;
				}
				/**** process buttons ****/
				switch(CED.Code)
				{
					case SELECTDOWN:
						ID = UA_CheckGadgetList(window, GlobalEventsWdw_GR, &CED);
						switch(ID)
						{
							case 4:	/* list area */
								UA_EnableButton(window, &GlobalEventsWdw_GR[7]); /* label name */
								line = SelectLine(&GlobalEventsWdw_GR[4], displayFactor,
																	NUMKEYS, displayLines);
								if (line!=-1)
								{
									HighLightLabelLine(	window, &GlobalEventsWdw_GR[4],
																			displayFactor, line);
									UA_ClearButton(window, &GlobalEventsWdw_GR[6], AREA_PEN);

									if ( strlen(keyListTexts[topEntry+line])>1 )
										stccpy(keyName, keyListTexts[topEntry+line], strlen(keyListTexts[topEntry+line])-3);
									else
										stccpy(keyName, keyListTexts[topEntry+line], 40);

									UA_DrawSpecialGadgetText(window, &GlobalEventsWdw_GR[6], keyName, SPECIAL_TEXT_CENTER);
									if (keySERs[topEntry+line]!=NULL)
									{
										UA_EnableButton(window, &GlobalEventsWdw_GR[10]); /* delete label name */

										stccpy(printStr, keySERs[topEntry+line]->labelName, 100);
										UA_ShortenString(scriptWindow->RPort, printStr, (GlobalEventsWdw_GR[7].x2-GlobalEventsWdw_GR[7].x1)-16);

										UA_DrawSpecialGadgetText(window, &GlobalEventsWdw_GR[7], printStr, SPECIAL_TEXT_CENTER);
									}
									else
										UA_DisableButton(window, &GlobalEventsWdw_GR[10], gui_pattern); /* delete label name */

									if (dblClicked)
										goto do_label;
								}
								break;

							case 7:	/* label name */
do_label:
								if (!dblClicked)
									UA_HiliteButton(window, &GlobalEventsWdw_GR[7]);
								reqWindow = Open_A_Request_Window(window, LabelWdw_GR);
								if ( reqWindow )
								{
									val = MonitorPickLabel(reqWindow,labelName);
									if (val)
										AddGlobalEvent(topEntry+line, labelName);
									Close_A_Request_Window(reqWindow);
								}
								if (val)
									DisplayGlobalList(window, &GlobalEventsWdw_GR[4], -2,
																		displayLines, displayFactor, numEntries);
								PropSlider1.LeftEdge	= GlobalEventsWdw_GR[5].x1+4;
								PropSlider1.TopEdge		= GlobalEventsWdw_GR[5].y1+2;
								PropSlider1.Width			= GlobalEventsWdw_GR[5].x2-GlobalEventsWdw_GR[5].x1-7;
								PropSlider1.Height		= GlobalEventsWdw_GR[5].y2-GlobalEventsWdw_GR[5].y1-3;

								if ( UA_IsWindowOnLacedScreen(window) )
								{
									PropSlider1.TopEdge	+= 2;
									PropSlider1.Height	-= 4;
								}

								UA_SetPropSlider(window, &PropSlider1, numEntries, displayLines, topEntry);

								if (val)
								{
									UA_ClearButton(window, &GlobalEventsWdw_GR[7], AREA_PEN);
									stccpy(printStr, labelName, 100);
									UA_ShortenString(scriptWindow->RPort, printStr, (GlobalEventsWdw_GR[7].x2-GlobalEventsWdw_GR[7].x1)-16);
									UA_DrawSpecialGadgetText(window, &GlobalEventsWdw_GR[7], printStr, SPECIAL_TEXT_CENTER);
								}
								break;

							case 8:	/* OK */
do_ok:
								UA_HiliteButton(window, &GlobalEventsWdw_GR[8]);
								retval=TRUE;
								loop=FALSE;
								break;

							case 9:	/* Cancel */
do_cancel:
								UA_HiliteButton(window, &GlobalEventsWdw_GR[9]);
								retval=FALSE;
								loop=FALSE;
								break;

							case 10: /* delete label */
								UA_HiliteButton(window, &GlobalEventsWdw_GR[ID]);
								for(i=0; i<MAX_GLOBAL_EVENTS; i++)
								{
									if ( ObjectRecord.scriptSIR.globallocalEvents[i] == keySERs[topEntry+line] )
									{
										FreeMem(ObjectRecord.scriptSIR.globallocalEvents[i], sizeof(struct ScriptEventRecord) );
										ObjectRecord.scriptSIR.globallocalEvents[i] = NULL;
										keySERs[topEntry+line] = NULL;
										break;
									}
								}
								UA_ClearButton(window, &GlobalEventsWdw_GR[7], AREA_PEN);
								UA_DisableButton(window, &GlobalEventsWdw_GR[10], gui_pattern); /* delete label name */
								DisplayGlobalList(window, &GlobalEventsWdw_GR[4], -2,
																	displayLines, displayFactor, numEntries);
								break;
						}
						break;
				}
				break;

			case IDCMP_RAWKEY:
				if (CED.Code==RAW_ESCAPE)	/* cancel */
					goto do_cancel;
				else if (CED.Code==RAW_RETURN && !UA_IsGadgetDisabled(&GlobalEventsWdw_GR[8]))	/* OK */
					goto do_ok;
				break;
		}
	}

	if (!retval)	/* CANCEL */
	{
		for(i=0; i<MAX_GLOBAL_EVENTS; i++)
		{
			if (!definedList[i] && ObjectRecord.scriptSIR.globallocalEvents[i] != NULL)
			{
				FreeMem(ObjectRecord.scriptSIR.globallocalEvents[i], sizeof(struct ScriptEventRecord) );
				ObjectRecord.scriptSIR.globallocalEvents[i] = NULL;
			}
		}
	}

	UA_EnableButton(window, &GlobalEventsWdw_GR[7]); /* label name */
	UA_EnableButton(window, &GlobalEventsWdw_GR[10]); /* delete label name */

	return(retval);
}

/******** MonitorPickLabel() ********/

BOOL MonitorPickLabel(struct Window *window,STRPTR labelName)
{
int topEntry, numEntries;
int i, ID, displayLines, line;
BOOL loop=TRUE, retval;
UBYTE *labelList[1000];	//MAX_GLOBAL_EVENTS];
UBYTE selectionList[1000];	//MAX_GLOBAL_EVENTS];
SNRPTR this_node;
struct List *list;
TEXT printStr[100];
BOOL dblClicked=FALSE;
struct ScrollRecord SR;

	//for(i=0; i<MAX_GLOBAL_EVENTS; i++)
	for(i=0; i<1000; i++)
		selectionList[i] = 0;

	/**** count number of labels, fill array of pointer to them ****/

	numEntries = 0;
	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		list = (struct List *)ObjectRecord.scriptSIR.allLists[i];
		if (list != NULL)
		{
			if (list->lh_TailPred != (struct Node *)list)
			{
				for(this_node=(SNRPTR)list->lh_Head;
						this_node->node.ln_Succ;
						this_node=(SNRPTR)this_node->node.ln_Succ)
				{
					if (this_node->nodeType == TALK_LABEL ||
							this_node->nodeType == TALK_STARTSER ||
							this_node->nodeType == TALK_STARTPAR)
					{
						//if (numEntries<MAX_GLOBAL_EVENTS)
						if (numEntries<1000)
						{
							if (this_node->objectName[0]!='\0')
							{
								labelList[numEntries] = &this_node->objectName[0];
								numEntries++;
							}
						}
					}
				}
			}
		}
	}

	if (numEntries==0)
	{
		UA_WarnUser(180);
		return(FALSE);
	}

	/**** init window and scroll bar ****/

	displayLines = 7;

	PropSlider2.LeftEdge	= LabelWdw_GR[4].x1+4;
	PropSlider2.TopEdge		= LabelWdw_GR[4].y1+2;
	PropSlider2.Width			= LabelWdw_GR[4].x2-LabelWdw_GR[4].x1-7;
	PropSlider2.Height		= LabelWdw_GR[4].y2-LabelWdw_GR[4].y1-3;

	if ( UA_IsWindowOnLacedScreen(window) )
	{
		PropSlider2.TopEdge	+= 2;
		PropSlider2.Height	-= 4;
	}

	UA_DrawGadgetList(window, LabelWdw_GR);

	InitPropInfo((struct PropInfo *)PropSlider2.SpecialInfo, (struct Image *)PropSlider2.GadgetRender);
	AddGadget(window, &PropSlider2, -1L);

	topEntry=0L;
	UA_InitPropSlider(window, &PropSlider2, numEntries,	displayLines, topEntry);

	/**** init scroll record ****/

	SR.GR							= &LabelWdw_GR[3];
	SR.window					= window;
	SR.list						= NULL;
	SR.sublist				= NULL;
	SR.selectionList	= &selectionList[0];
	SR.entryWidth			= -1;
	SR.numDisplay			= displayLines;
	SR.numEntries			= numEntries;

	UA_PrintStandardList(NULL,-1,NULL);	// init static
	UA_PrintStandardList(&SR,topEntry,labelList);

	UA_DisableButton(window, &LabelWdw_GR[6], gui_pattern); /* OK */

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if (CED.extraClass == DBLCLICKED)
			dblClicked=TRUE;
		else
			dblClicked=FALSE;

		switch(CED.Class)
		{
			case IDCMP_MOUSEBUTTONS:
				if ( CED.extraClass==IDCMP_GADGETDOWN || CED.extraClass==IDCMP_GADGETUP )
				{
					UA_ScrollStandardList(&SR,&topEntry,&PropSlider2,labelList,&CED);
					CED.Code = 0;
					break;
				}

				/**** process buttons ****/

				switch(CED.Code)
				{
					case SELECTDOWN:
						ID = UA_CheckGadgetList(window, LabelWdw_GR, &CED);
						switch(ID)
						{
							case 3:	/* list area */
								line = UA_SelectStandardListLine(&SR,topEntry,FALSE,&CED,FALSE,FALSE);
								if (line!=-1)
								{
									UA_ClearButton(window, &LabelWdw_GR[5], AREA_PEN);
									stccpy(printStr, labelList[topEntry+line], 100);
									UA_ShortenString(scriptWindow->RPort, printStr, (LabelWdw_GR[5].x2-LabelWdw_GR[5].x1)-16);
									UA_DrawSpecialGadgetText(window, &LabelWdw_GR[5], printStr, SPECIAL_TEXT_CENTER);
									stccpy(labelName, labelList[topEntry+line], 40);
									UA_EnableButton(window, &LabelWdw_GR[6]); /* OK */
									if (dblClicked)
										goto do_ok;
								}
								break;

							case 6:	/* OK */
do_ok:
								if (!dblClicked)
									UA_HiliteButton(window, &LabelWdw_GR[6]);
								retval=TRUE;
								loop=FALSE;
								break;

							case 7:	/* Cancel */
do_cancel:
								UA_HiliteButton(window, &LabelWdw_GR[7]);
								retval=FALSE;
								loop=FALSE;
								break;
						}
						break;
				}
				break;

			case IDCMP_RAWKEY:
				if (CED.Code==RAW_ESCAPE)	/* cancel */
					goto do_cancel;
				else if (CED.Code==RAW_RETURN && !UA_IsGadgetDisabled(&LabelWdw_GR[6]))	/* OK */
					goto do_ok;
				break;
		}
	}

	UA_EnableButton(window, &LabelWdw_GR[6]); /* OK */

	return(retval);
}

/******** AddGlobalEvent() ********/

void AddGlobalEvent(int line, STRPTR labelName)
{
struct ScriptEventRecord *SER;
int i;

	if ( keySERs[line] != NULL )
	{
		stccpy(keySERs[line]->labelName, labelName, MAX_PARSER_CHARS);
		keySERs[line]->labelSNR = (SNRPTR)FindLabel(&ObjectRecord.scriptSIR, labelName);
		return;
	}
	else
	{
		/**** if no matching SER can be found, create a new one ****/

		SER = (struct ScriptEventRecord *)
				AllocMem(sizeof(struct ScriptEventRecord), MEMF_ANY | MEMF_CLEAR);
		if (SER!=NULL)
		{
			for(i=0; i<MAX_GLOBAL_EVENTS; i++)
			{
				if ( ObjectRecord.scriptSIR.globallocalEvents[i] == NULL)	/* free */
				{
					ObjectRecord.scriptSIR.globallocalEvents[i] = (struct ScriptEventRecord *)SER;
	
					SER->keyCode = -1;
					SER->rawkeyCode = -1;

					if ( keyListCodes[line] != -1 )
						SER->keyCode = keyListCodes[line];
					else
						SER->rawkeyCode = keyListRawCodes[line];

					stccpy(SER->labelName, labelName, MAX_PARSER_CHARS);

					SER->labelSNR = (SNRPTR)FindLabel(&ObjectRecord.scriptSIR, labelName);

					/**** needed for scrolling ****/
					keySERs[line] = (struct ScriptEventRecord *)SER;

					return;
				}
			}
		}
	}
}

/******** standAlonePickLabel() ********/

void standAlonePickLabel(struct Window *onWindow, SNRPTR this_node)
{
TEXT labelName[41];
struct Window *window;

	window = Open_A_Request_Window(onWindow, LabelWdw_GR);
	if (window)
	{
		if ( MonitorPickLabel(window,labelName) )
		{
			stccpy(this_node->objectName, labelName, MAX_OBJECTNAME_CHARS);
			this_node->extraData = (UBYTE *)FindLabel(&ObjectRecord.scriptSIR, labelName);
			if (this_node->extraData==NULL)
				UA_WarnUser(181);
		}
		Close_A_Request_Window(window);
	}
}

/******** DisplayGlobalList() ********/
/*
 * Initialize static by calling with topEntry == -1 !
 * -2 draws always.
 *
 */

void DisplayGlobalList(	struct Window *window, struct GadgetRecord *GR,
												int topEntry, int lines, int displayFactor,
												int numEntries)
{
int i,len;
static int lastTop = 0;
TEXT printStr[100];

	if (topEntry==lastTop)
		return;

	if (topEntry==-1)
	{
		lastTop = 0;
		topEntry = 0;
	}
	else if (topEntry == -2)
		topEntry = lastTop;

	if (numEntries<lines)
		lines=numEntries;

	SetBPen(window->RPort, UA_GetRightPen(window,GR,AREA_PEN));

	for(i=0; i<lines; i++)
	{
		SetAPen(window->RPort, UA_GetRightPen(window,GR,AREA_PEN));
		RectFill(window->RPort, GR->x1+2, GR->y1+2+displayFactor*i, GR->x2-2, GR->y1+window->RPort->TxBaseline+4+displayFactor*i);
		Move(window->RPort, GR->x1+3, GR->y1+window->RPort->TxBaseline+2+displayFactor*i);
		SetAPen(window->RPort, LO_PEN);
		if (strlen(keyListTexts[topEntry+i])>1)
			len=4;
		else
			len=0;
		Text(window->RPort, keyListTexts[topEntry+i], strlen(keyListTexts[topEntry+i])-len);
		if (keySERs[topEntry+i]!=NULL)
		{
			//Move(window->RPort, GR->x1+148, GR->y1+window->RPort->TxBaseline+2+displayFactor*i);
			Move(window->RPort, GR->x1+((GR->x2-GR->x1)/2), GR->y1+window->RPort->TxBaseline+2+displayFactor*i);
			stccpy(printStr, keySERs[topEntry+i]->labelName, 100);
			//UA_ShortenString(scriptWindow->RPort, printStr, 148-16);
			UA_ShortenString(scriptWindow->RPort, printStr, ((GR->x2-GR->x1)/2)-16);
			Text(window->RPort, printStr, strlen(printStr));
		}
	}

	lastTop = topEntry;
}

/******** doGlobalListSlider() ********/

void doGlobalListSlider(struct Window *window,
												int numEntries, int numDisplay, int *topEntry,
												struct GadgetRecord *GR, struct Gadget *g, int displayFactor)
{
ULONG signals;
BOOL loop=TRUE;
struct IntuiMessage *message;
BOOL mouseMoved=FALSE;
LONG f;

	if ( CED.Qualifier & IEQUALIFIER_LSHIFT || CED.Qualifier & IEQUALIFIER_RSHIFT )
	{
		f = ( (CED.MouseY - g->TopEdge) * numEntries) / g->Height;
		if ( f < 0 )
			f = 0;
		*topEntry = f;
		if ( (*topEntry+numDisplay) > numEntries )
			*topEntry = numEntries-numDisplay;
		UA_SetPropSlider(window, g, numEntries, numDisplay, *topEntry);
		DisplayGlobalList(window, GR, *topEntry, numDisplay, displayFactor, numEntries);
		return;
	}

	UA_GetPropSlider(window, g, numEntries, numDisplay, topEntry);

	DisplayGlobalList(window, GR, *topEntry, numDisplay, displayFactor, numEntries);

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
				if (PropSlider1.Flags & GFLG_SELECTED)
				{
					UA_GetPropSlider(window, g, numEntries, numDisplay, topEntry);
					DisplayGlobalList(window, GR, *topEntry, numDisplay, displayFactor, numEntries);
					loop=TRUE;
				}
				else
					loop=FALSE;
			}
		}
	}

	UA_SwitchMouseMoveOff(window);
}

/******** SelectLine() ********/
/*
 * displayFactor is height of individual lines
 *
 */

int SelectLine(	struct GadgetRecord *GR, int displayFactor,
								int numEntries, int numDisplay)
{
int line;

	line = (CED.MouseY-GR->y1) / displayFactor;
	if (numEntries<numDisplay)
	{
		if (line-1>=(numEntries-1))
			return(-1);
	}
	else if (line<0 || line>=numDisplay)
		return(-1);
	return(line);
}

/******** HighLightLabelLine() ********/
/*
 * displayFactor is height of individual lines
 *
 */

void HighLightLabelLine(struct Window *window,
												struct GadgetRecord *GR, int displayFactor, int line)
{
	SafeSetWriteMask(window->RPort, 0x7);
	SetDrMd(window->RPort, JAM2 | COMPLEMENT);

	RectFill(	window->RPort, GR->x1+2, GR->y1+2+displayFactor*line,
						GR->x2-2, GR->y1+2+window->RPort->TxBaseline+3+displayFactor*line);

	Delay(4L);

	RectFill(	window->RPort, GR->x1+2, GR->y1+2+displayFactor*line,
						GR->x2-2, GR->y1+2+window->RPort->TxBaseline+3+displayFactor*line);

	SetDrMd(window->RPort, JAM1);
	SafeSetWriteMask(window->RPort, 0xff);
}

/******** MonitorKeyList() ********/
/*
 * Used in PLS to select local event keys.
 *
 */

BOOL MonitorKeyList(struct Window *window, int *key, int *raw)
{
int topEntry, numEntries, i;
int ID, displayLines, displayFactor, line=0;
BOOL loop=TRUE, retval,dblClicked=FALSE;

	/**** init vars ****/

	*key = -1;
	*raw = -1;

	for(i=0; i<NUMKEYS; i++)
		keySERs[i] = NULL;	// keys.h

	/**** init window and scroll bar ****/

	displayLines = 7;
	if ( UA_IsWindowOnLacedScreen(window) )
		displayFactor=24;
	else
		displayFactor=12;

	PropSlider1.LeftEdge	= KeyList_GR[4].x1 + 4;
	PropSlider1.TopEdge		= KeyList_GR[4].y1 + 2;
	PropSlider1.Width			= KeyList_GR[4].x2 - KeyList_GR[4].x1 - 7;
	PropSlider1.Height		= KeyList_GR[4].y2 - KeyList_GR[4].y1 - 3;

	if ( UA_IsWindowOnLacedScreen(window) )
	{
		PropSlider1.TopEdge	+= 2;
		PropSlider1.Height	-= 4;
	}

	UA_DrawGadgetList(window, KeyList_GR);

	InitPropInfo(	(struct PropInfo *)PropSlider1.SpecialInfo,
								(struct Image *)PropSlider1.GadgetRender);
	AddGadget(window, &PropSlider1, -1L);

	topEntry=0L;
	numEntries=NUMKEYS;	/* keys.h */
	UA_InitPropSlider(window, &PropSlider1, numEntries, displayLines, topEntry);

	DisplayGlobalList(window, &KeyList_GR[3], -1, displayLines, displayFactor, numEntries);

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if (CED.extraClass == DBLCLICKED)
			dblClicked=TRUE;
		else
			dblClicked=FALSE;

		switch(CED.Class)
		{
			case IDCMP_MOUSEBUTTONS:
				if ( CED.extraClass==IDCMP_GADGETDOWN || CED.extraClass==IDCMP_GADGETUP )
				{
					doGlobalListSlider(	window, NUMKEYS, displayLines, &topEntry,
															&KeyList_GR[3], &PropSlider1, displayFactor);
					CED.Code = 0;
					break;
				}
				switch(CED.Code)
				{
					case SELECTDOWN:
						ID = UA_CheckGadgetList(window, KeyList_GR, &CED);
						switch(ID)
						{
							case 2:	// Cancel
do_cancel:
								UA_HiliteButton(window, &KeyList_GR[2]);
								retval=FALSE;
								loop=FALSE;
								break;

							case 3:	// list area
								line = SelectLine(&KeyList_GR[3], displayFactor, NUMKEYS, displayLines);
								if (line!=-1)
								{
									HighLightLabelLine(window, &KeyList_GR[3], displayFactor, line);

Delay(4L);

									if ( keyListCodes[topEntry+line] != -1 )
										*key = keyListCodes[topEntry+line];
									else
										*raw = keyListRawCodes[topEntry+line];
/*
									if (dblClicked)
									{
										retval=TRUE;
										loop=FALSE;
									}
*/
retval=TRUE;
loop=FALSE;
								}
								break;
						}
						break;
				}
				break;

			case IDCMP_RAWKEY:
				if (CED.Code==RAW_ESCAPE)	// cancel
					goto do_cancel;
				break;
		}
	}

	return(retval);
}

/******** E O F ********/
