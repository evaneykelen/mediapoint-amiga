/*******************************************************************/
/*
 *
 *  T H I S   I S   A L S O   A   P L A Y E R   M O D U L E !
 *
 *
 */

#include "nb:pre.h"

#include "minc:types.h"
#include "minc:defs.h"
#include "minc:errors.h"
#include "minc:process.h"
#include "minc:ge.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct Window *pageWindow;
extern struct Library *medialinkLibBase;
extern UWORD chip gui_pattern[];
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern char *pageCommands[];
extern UBYTE **msgs;   
extern struct PageFuncs pageFuncs[];
extern struct ObjectInfo ObjectRecord;
extern struct List usedxapplist;

/**** gadgets ****/

extern struct GadgetRecord Interactive_GR[];
extern struct GadgetRecord VarDec_GR[];
extern struct GadgetRecord ExpDec_GR[];
extern struct GadgetRecord LabelWdw_GR[];
extern struct GadgetRecord KeyList_GR[];

/**** structs ****/

struct small_IA
{
	WORD	bx, by, bwidth, bheight;
	UBYTE jumpType;
	UBYTE	renderType;
	UBYTE	audioCue;
	WORD 	keyCode;
	WORD	rawkeyCode;
	TEXT  buttonName[50];
	TEXT  assignment[75];
};

/**** functions ****/

#ifndef USED_FOR_PLAYER

/******** MakeInteractiveButton() ********/

BOOL MakeInteractiveButton(void)
{
struct Window *window, *reqWindow;
BOOL loop, retVal, done;
int ID, key, raw, len, wdw, numWdw, hitWdw, i, j, val;
TEXT tmp[50];
struct small_IA *small_IA_List[MAXEDITWINDOWS];

	/**** init vars ****/

	retVal = FALSE;
	loop   = TRUE;

	wdw = FirstActiveEditWindow();
	if (wdw==-1)
		wdw=0;	// take first. safe 'cause menus don't allow a MakeInteractiveButton with no windows around!
	numWdw=0;
	for(i=0; i<MAXEDITWINDOWS; i++)
		if (EditWindowList[i]!=NULL)
			numWdw++;

	for(i=0; i<MAXEDITWINDOWS; i++)
		small_IA_List[i] = NULL;

	for(i=0; i<numWdw; i++)
	{
		small_IA_List[i] = (struct small_IA *)AllocMem(sizeof(struct small_IA), MEMF_CLEAR | MEMF_ANY);
		if ( small_IA_List[i]==NULL )	
		{
			UA_WarnUser(-1);
			return(FALSE);
		}
	}

	j=0;
	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i]!=NULL )
		{
			small_IA_List[j]->bx					= EditWindowList[i]->bx;
			small_IA_List[j]->by					= EditWindowList[i]->by;
			small_IA_List[j]->bwidth			= EditWindowList[i]->bwidth;
			small_IA_List[j]->bheight			= EditWindowList[i]->bheight;
			small_IA_List[j]->jumpType		= EditWindowList[i]->jumpType;
			small_IA_List[j]->renderType	= EditWindowList[i]->renderType;
			small_IA_List[j]->audioCue		= EditWindowList[i]->audioCue;
			small_IA_List[j]->keyCode			= EditWindowList[i]->keyCode;
			small_IA_List[j]->rawkeyCode	= EditWindowList[i]->rawkeyCode;
			strcpy(small_IA_List[j]->buttonName,EditWindowList[i]->buttonName);
			strcpy(small_IA_List[j]->assignment,EditWindowList[i]->assignment);
			j++;
		}
	}

	/**** double or halve gadgets ****/

	if ( UA_IsWindowOnLacedScreen(pageWindow) )
	{
		if ( Interactive_GR[0].x1 == 0 )	// not doubled
		{
			UA_DoubleGadgetDimensions(Interactive_GR);
			Interactive_GR[0].x1 = 1;
		}
		if ( KeyList_GR[0].x1 == 0 )	// not doubled
		{
			UA_DoubleGadgetDimensions(KeyList_GR);
			KeyList_GR[0].x1 = 1;
		}
	}
	else
	{
		if ( Interactive_GR[0].x1 == 1 )	// doubled
		{
			UA_HalveGadgetDimensions(Interactive_GR);
			Interactive_GR[0].x1 = 0;
		}
		if ( KeyList_GR[0].x1 == 1 )	// doubled
		{
			UA_HalveGadgetDimensions(KeyList_GR);
			KeyList_GR[0].x1 = 0;
		}
	}

	/**** open a window ****/

	window = UA_OpenRequesterWindow(pageWindow,Interactive_GR,STDCOLORS);
	if (!window)
	{
		UA_WarnUser(-1);
		return(FALSE);
	}

	/**** render gadget ****/

	UA_DrawGadgetList(window, Interactive_GR);

	SetAPen(window->RPort, LO_PEN);
	SetDrMd(window->RPort, JAM1);
	if ( UA_IsWindowOnLacedScreen(window) )
		val=3;
	else
		val=2;

	PrintHorizText(window, &Interactive_GR[16], 3, "–\0");
	PrintHorizText(window, &Interactive_GR[17], 3, "—\0");

	Interactive_GR[10].type = BUTTON_GADGET;
	DrawIAButtons(wdw, window, EditWindowList[wdw]);

	if (numWdw==1)
	{
		UA_DisableButton(window, &Interactive_GR[14], gui_pattern);	// apply to all
		UA_DisableButton(window, &Interactive_GR[16], gui_pattern);	// next wdw
		UA_DisableButton(window, &Interactive_GR[17], gui_pattern);	// prev wdw
	}

	DrawThickBorder(wdw);

	/**** monitor user ****/

	Interactive_GR[19].type=BUTTON_GADGET;

	while(loop)
	{
		doStandardWait(window);

		if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
			DetermineClickEvent(&hitWdw, FALSE);

		if ( numWdw>1 && CED.Class==IDCMP_RAWKEY && CED.Code==RAW_TAB )
		{
			if ( CED.Qualifier&IEQUALIFIER_LSHIFT || CED.Qualifier&IEQUALIFIER_RSHIFT )
				goto prev_wdw;
			else
				goto next_wdw;
		}

		if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, Interactive_GR, &CED);

			if ( !(window->Flags & WFLG_WINDOWACTIVE) )	// clicked outside
			{
				ID=-1;
				if (hitWdw!=-1)	// choose new edit window
				{
					DrawThickBorder(wdw);
					wdw=hitWdw;
					DrawThickBorder(wdw);
					DrawIAButtons(wdw, window, EditWindowList[wdw]);
				}
			}

			switch(ID)
			{
				case 2:		// OK
do_ok:
					UA_HiliteButton(window, &Interactive_GR[2]);
					loop=FALSE;
					retVal=TRUE;
					break;

				case 3:		// Cancel
do_cancel:
					UA_HiliteButton(window, &Interactive_GR[3]);
					loop=FALSE;
					retVal=FALSE;
					break;

				case 6:		// goto
					UA_ProcessCycleGadget(window, &Interactive_GR[ID], &CED);
					UA_SetValToCycleGadgetVal(&Interactive_GR[ID], &val);
					EditWindowList[wdw]->jumpType = val;
					DrawIAButtons(wdw, window, EditWindowList[wdw]);
					break;

				case 7:		// button name
					UA_ProcessStringGadget(window,Interactive_GR,&Interactive_GR[ID],&CED);
					UA_SetStringToGadgetString(&Interactive_GR[ID],EditWindowList[wdw]->buttonName);
					break;

				case 8:		// highlight
					UA_ProcessCycleGadget(window, &Interactive_GR[ID], &CED);
					UA_SetValToCycleGadgetVal(&Interactive_GR[ID], &val);
					UnSetByteBit(	&EditWindowList[wdw]->renderType,
												RENDERTYPE_NONE|
												RENDERTYPE_INVERT|RENDERTYPE_BOX|RENDERTYPE_FATBOX|
												RENDERTYPE_STAY );
					if ( val==0 )
						EditWindowList[wdw]->renderType |= RENDERTYPE_NONE;
					else if ( val==1 )
						EditWindowList[wdw]->renderType |= RENDERTYPE_INVERT;
					else if ( val==2 )
						EditWindowList[wdw]->renderType |= RENDERTYPE_BOX;
					else if ( val==3 )
						EditWindowList[wdw]->renderType |= RENDERTYPE_FATBOX;
					else if ( val==4 )
						EditWindowList[wdw]->renderType |= ( RENDERTYPE_INVERT | RENDERTYPE_STAY );
					else if ( val==5 )
						EditWindowList[wdw]->renderType |= ( RENDERTYPE_BOX | RENDERTYPE_STAY );
					else if ( val==6 )
						EditWindowList[wdw]->renderType |= ( RENDERTYPE_FATBOX | RENDERTYPE_STAY );
					break;

				case 9:		// audio cue
					UA_ProcessCycleGadget(window, &Interactive_GR[ID], &CED);
					UA_SetValToCycleGadgetVal(&Interactive_GR[ID], &val);
					EditWindowList[wdw]->audioCue = val;
					break;

				case 10:	// key
					UA_HiliteButton(window, &Interactive_GR[ID]);
					reqWindow = Open_A_Request_Window(window, KeyList_GR);
					if ( reqWindow )
					{
						if ( MonitorKeyList(reqWindow,&key,&raw) )
						{
							Close_A_Request_Window(reqWindow);
							KeyToKeyName(key,raw,tmp);
							len = strlen(tmp);
							if ( raw!=-1 && len >= 4 )
								tmp[ len-4 ] = '\0';	// remove _KEY

							EditWindowList[wdw]->keyCode		= key;
							EditWindowList[wdw]->rawkeyCode	= raw;

							UA_ClearButton(window, &Interactive_GR[10], AREA_PEN);
							UA_DrawText(window, &Interactive_GR[10], tmp);
						}
						else
							Close_A_Request_Window(reqWindow);
					}
					break;

				case 11:	// delete key
					UA_HiliteButton(window, &Interactive_GR[ID]);
					UA_ClearButton(window, &Interactive_GR[10], AREA_PEN);
					EditWindowList[wdw]->keyCode		= -1;
					EditWindowList[wdw]->rawkeyCode	= -1;
					break;

				case 14:	// all window
					UA_HiliteButton(window, &Interactive_GR[ID]);
					DoAllForInteractive(wdw);
					break;

				case 16:	// next window
next_wdw:
					UA_HiliteButton(window, &Interactive_GR[16]);
					DrawThickBorder(wdw);
					wdw++;
					if (wdw==numWdw)
						wdw=0;
					DrawThickBorder(wdw);
					DrawIAButtons(wdw, window, EditWindowList[wdw]);
					break;

				case 17:	// previous window
prev_wdw:
					UA_HiliteButton(window, &Interactive_GR[17]);
					DrawThickBorder(wdw);
					wdw--;
					if (wdw<0)
						wdw=numWdw-1;
					DrawThickBorder(wdw);
					DrawIAButtons(wdw, window, EditWindowList[wdw]);
					break;

				case 18:	// detect (aka follow)
					UA_ProcessCycleGadget(window, &Interactive_GR[ID], &CED);
					UA_SetValToCycleGadgetVal(&Interactive_GR[ID], &val);
					if ( val==0 )				// yes
						SetByteBit(&EditWindowList[wdw]->renderType, RENDERTYPE_AUTO);
					else if ( val==1 )	// no
						UnSetByteBit(&EditWindowList[wdw]->renderType, RENDERTYPE_AUTO);
					break;

				case 19:	// magic, hidden special button
					UA_InvertButton(window, &Interactive_GR[19]);
					InvertByteBit(&EditWindowList[wdw]->renderType, RENDERTYPE_IMMEDIATE);
					break;

				case 20:	// assignment
					UA_ProcessStringGadget(window,Interactive_GR,&Interactive_GR[ID],&CED);
					UA_SetStringToGadgetString(&Interactive_GR[ID],EditWindowList[wdw]->assignment);
					{
					char var1[50], var2[50], varcont[50];
					int value, type, error;
						error = ParseDeclaration(	NULL, EditWindowList[wdw]->assignment,
																			var1, var2, &value, varcont, &type, TRUE );
						if ( error==0 )
						{
							if ( type==0 )			// VAR = VAR
								sprintf(EditWindowList[wdw]->assignment,"%s = %s", var1, var2);
							else if ( type==1 )	// VAR = VALUE
								sprintf(EditWindowList[wdw]->assignment,"%s = %d", var1, value);
							else if ( type==2 )	// VAR = "STRING"
								sprintf(EditWindowList[wdw]->assignment,"%s = \"%s\"", var1, varcont);
							UA_SetStringGadgetToString(window,&Interactive_GR[20],EditWindowList[wdw]->assignment);
						}
						else
							ProcessParseError(error);
					}
					break;
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE)				// cancel
				goto do_cancel;
			else if (CED.Code==RAW_RETURN)	// OK
				goto do_ok;
		}
	}

	Interactive_GR[19].type=INVISIBLE_GADGET;

	DrawThickBorder(wdw);

	for(i=7; i<12; i++)
		UA_EnableButtonQuiet(&Interactive_GR[i]);
	UA_EnableButtonQuiet(&Interactive_GR[18]);
	UA_EnableButtonQuiet(&Interactive_GR[20]);

	UA_EnableButtonQuiet(&Interactive_GR[14]);	// apply to all
	UA_EnableButtonQuiet(&Interactive_GR[16]);	// next wdw
	UA_EnableButtonQuiet(&Interactive_GR[17]);	// prev wdw

	UA_CloseRequesterWindow(window,STDCOLORS);

	Interactive_GR[10].type = HIBOX_REGION;

	if ( !retVal) 
	{
		j=0;
		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if ( EditWindowList[i]!=NULL )
			{
				EditWindowList[i]->bx						= small_IA_List[j]->bx;
				EditWindowList[i]->by						= small_IA_List[j]->by;
				EditWindowList[i]->bwidth				= small_IA_List[j]->bwidth;
				EditWindowList[i]->bheight			= small_IA_List[j]->bheight;
				EditWindowList[i]->jumpType			= small_IA_List[j]->jumpType;
				EditWindowList[i]->renderType		= small_IA_List[j]->renderType;
				EditWindowList[i]->audioCue			= small_IA_List[j]->audioCue;
				EditWindowList[i]->keyCode			= small_IA_List[j]->keyCode;
				EditWindowList[i]->rawkeyCode		= small_IA_List[j]->rawkeyCode;
				strcpy(EditWindowList[i]->buttonName,small_IA_List[j]->buttonName);
				strcpy(EditWindowList[i]->assignment,small_IA_List[j]->assignment);
				j++;
			}
		}
	}
	else	// OK
	{
		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if ( EditWindowList[i]!=NULL && EditWindowList[i]->jumpType!=0 )
			{
				EditWindowList[i]->bx				= EditWindowList[i]->X;
				EditWindowList[i]->by				= EditWindowList[i]->Y;
				EditWindowList[i]->bwidth		= EditWindowList[i]->Width;
				EditWindowList[i]->bheight	= EditWindowList[i]->Height;
			}
		}

		// warn user for equal button names -- BTW this is not harmful

		done=FALSE;
		for(j=0; j<MAXEDITWINDOWS; j++)
		{
			for(i=0; i<MAXEDITWINDOWS; i++)
			{
				if (	i!=j && !done && EditWindowList[j] && EditWindowList[i] &&
							EditWindowList[i]->jumpType!=0 && EditWindowList[j]->jumpType!=0 )
				{
					if ( !stricmp(EditWindowList[j]->buttonName,EditWindowList[i]->buttonName) )
					{
						done=TRUE;
						Message( msgs[Msg_ButtonsSameName-1] );
					}
				}
			}
		}
	}

	for(i=0; i<numWdw; i++)
		if ( small_IA_List[i] != NULL )
			FreeMem( small_IA_List[i], sizeof(struct small_IA));

	return(retVal);
}

/******** KeyToKeyName() ********/

void KeyToKeyName(int key, int raw, STRPTR keyName)
{
	if (key != -1)
		sprintf(keyName, "%c", toupper(key));
	else if (raw != -1)
	{
		switch(raw)
		{
			case TALK_HELP_KC: 				strcpy(keyName, TALK_HELP_KT); 				break;
			case TALK_ESC_KC: 				strcpy(keyName, TALK_ESC_KT); 				break;
			case TALK_F1_KC: 					strcpy(keyName, TALK_F1_KT); 					break;
			case TALK_F2_KC: 					strcpy(keyName, TALK_F2_KT); 					break;
			case TALK_F3_KC: 					strcpy(keyName, TALK_F3_KT); 					break;
			case TALK_F4_KC: 					strcpy(keyName, TALK_F4_KT); 					break;
			case TALK_F5_KC: 					strcpy(keyName, TALK_F5_KT); 					break;
			case TALK_F6_KC: 					strcpy(keyName, TALK_F6_KT); 					break;
			case TALK_F7_KC: 					strcpy(keyName, TALK_F7_KT); 					break;
			case TALK_F8_KC: 					strcpy(keyName, TALK_F8_KT); 					break;
			case TALK_F9_KC: 					strcpy(keyName, TALK_F9_KT); 					break;
			case TALK_F10_KC: 				strcpy(keyName, TALK_F10_KT); 				break;
			case TALK_CURSORUP_KC: 		strcpy(keyName, TALK_CURSORUP_KT); 		break;
			case TALK_CURSORDOWN_KC: 	strcpy(keyName, TALK_CURSORDOWN_KT); 	break;
			case TALK_CURSORLEFT_KC: 	strcpy(keyName, TALK_CURSORLEFT_KT); 	break;
			case TALK_CURSORRIGHT_KC:	strcpy(keyName, TALK_CURSORRIGHT_KT); break;
			case TALK_TAB_KC: 				strcpy(keyName, TALK_TAB_KT); 				break;
			case TALK_DEL_KC: 				strcpy(keyName, TALK_DEL_KT); 				break;
			case TALK_BACKSPACE_KC: 	strcpy(keyName, TALK_BACKSPACE_KT); 	break;
			case TALK_RETURN_KC: 			strcpy(keyName, TALK_RETURN_KT); 			break;
			case TALK_SPACE_KC: 			strcpy(keyName, TALK_SPACE_KT); 			break;

			case TALK_OPEN_BRACKET_KC:	strcpy(keyName, TALK_OPEN_BRACKET_KT);	break;
			case TALK_CLOSE_BRACKET_KC:	strcpy(keyName, TALK_CLOSE_BRACKET_KT);	break;
			case TALK_STAR_KC:				strcpy(keyName, TALK_STAR_KT);				break;
			case TALK_PLUS_KC:				strcpy(keyName, TALK_PLUS_KT);				break;

			default: 									strcpy(keyName, TALK_ESC_KT); 				break;
		}
	}
	else
		keyName[0]='\0';
}

/******** DrawIAButtons() ********/

void DrawIAButtons(int wdw, struct Window *window, struct EditWindow *ew)
{
int i;
TEXT str[20];
int jt, rt, ac;

	sprintf(str, "%d", (int)wdw+1);
	UA_PrintInBox(	window, &Interactive_GR[15],
									Interactive_GR[15].x1, Interactive_GR[15].y1,
									Interactive_GR[15].x2, Interactive_GR[15].y2,
									str, PRINT_CENTERED );

	if ( ew->jumpType!=0 )
	{
		for(i=7; i<12; i++)
		{
			if ( UA_IsGadgetDisabled(&Interactive_GR[i]) )
			{
				if ( i==10 ) Interactive_GR[10].type = HIBOX_REGION;
				UA_EnableButton(window, &Interactive_GR[i]);
			}
		}
		Interactive_GR[10].type = BUTTON_GADGET;

		if ( UA_IsGadgetDisabled(&Interactive_GR[18]) )
			UA_EnableButton(window, &Interactive_GR[18]);

		if ( UA_IsGadgetDisabled(&Interactive_GR[20]) )
			UA_EnableButton(window, &Interactive_GR[20]);

		if ( ew->buttonName[0]=='\0' )
			sprintf(ew->buttonName,"%s%d", msgs[Msg_InterActButton-1], (int)ew->DrawSeqNum);
		UA_SetStringGadgetToString(window,&Interactive_GR[7],ew->buttonName);

		jt = ew->jumpType;
		rt = ew->renderType;
		if ( ew->renderType & RENDERTYPE_NONE )
			rt=0;
		if ( ew->renderType & RENDERTYPE_INVERT )
			rt=1;
		if ( ew->renderType & RENDERTYPE_BOX )
			rt=2;
		if ( ew->renderType & RENDERTYPE_FATBOX )
			rt=3;
		if ( (ew->renderType & RENDERTYPE_INVERT) && (ew->renderType & RENDERTYPE_STAY) )
			rt=4;
		if ( (ew->renderType & RENDERTYPE_BOX) && (ew->renderType & RENDERTYPE_STAY) )
			rt=5;
		if ( (ew->renderType & RENDERTYPE_FATBOX) && (ew->renderType & RENDERTYPE_STAY) )
			rt=6;
		ac = ew->audioCue;

		UA_SetCycleGadgetToVal(window, &Interactive_GR[6], jt);
		UA_SetCycleGadgetToVal(window, &Interactive_GR[8], rt);
		UA_SetCycleGadgetToVal(window, &Interactive_GR[9], ac);

		if ( ew->renderType & RENDERTYPE_AUTO )
			UA_SetCycleGadgetToVal(window, &Interactive_GR[18], 0);
		else
			UA_SetCycleGadgetToVal(window, &Interactive_GR[18], 1);

		UA_ClearButton(window, &Interactive_GR[19], AREA_PEN);
		if ( ew->renderType & RENDERTYPE_IMMEDIATE )
			UA_InvertButton(window, &Interactive_GR[19]);

		KeyToKeyName(ew->keyCode, ew->rawkeyCode, str);
		i = strlen(str);
		if ( ew->rawkeyCode!=-1 && i >= 4 )
			str[ i-4 ] = '\0';	// remove _KEY
		UA_ClearButton(window, &Interactive_GR[10], AREA_PEN);
		UA_DrawText(window, &Interactive_GR[10], str);

		UA_SetStringGadgetToString(window,&Interactive_GR[20],ew->assignment);
	}
	else
	{
		UA_SetCycleGadgetToVal(window, &Interactive_GR[6], 0);
		for(i=7; i<12; i++)
			UA_DisableButton(window, &Interactive_GR[i], gui_pattern);
		UA_DisableButton(window, &Interactive_GR[18], gui_pattern);
		UA_DisableButton(window, &Interactive_GR[20], gui_pattern);
	}
}

/******** DoAllForInteractive() ********/

void DoAllForInteractive(int wdw)
{
int i;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i]!=NULL && i!=wdw )
		{
			EditWindowList[i]->bx							= EditWindowList[wdw]->bx;
			EditWindowList[i]->by							= EditWindowList[wdw]->by;
			EditWindowList[i]->bwidth					= EditWindowList[wdw]->bwidth;
			EditWindowList[i]->bheight				= EditWindowList[wdw]->bheight;
			EditWindowList[i]->jumpType				= EditWindowList[wdw]->jumpType;
			EditWindowList[i]->renderType			= EditWindowList[wdw]->renderType;
			EditWindowList[i]->audioCue				= EditWindowList[wdw]->audioCue;
		}
	}
}

#endif

/******** GetLEInfo() ********/
/*
 * Reads BUTTON statements from a PageTalk document.
 *
 * Creates new entries in node->list (globallocalEvents[]) list.
 *
 */

BOOL GetLEInfo( struct ScriptNodeRecord *this_node, STRPTR fullPath,
								struct ScriptInfoRecord *SIR )
{
struct PageInfoRecord PIR;
struct ParseRecord *PR;
TEXT buffer[MAXSCANDEPTH];
int instruc, line;
BOOL validPage=TRUE;
BOOL headerPassed=FALSE;
int i,hole,buttonsFound=0;
struct EditWindow tempEW;
struct ScriptEventRecord *ser;
struct PageEventRecord *per;
struct PageEventRecord *work_node;
struct PageEventRecord *next_node;

	/**** init vars ****/

	PIR.ew = &tempEW;	// provide a temporary storage for BUTTON commands
	instruc = -1;
	line=0;
	hole=0;

	if ( this_node->numericalArgs[15] == 2 )	// IFF pic
		goto kill_list;	// see below

	/**** alloc mem for localevent list 																****/
	/**** this ONLY happens when a page gets buttons after the script		****/
	/**** was loaded else this is done by scripttalk.c									****/

	if ( this_node->list == NULL )
	{
		this_node->list = (struct List *)AllocMem(sizeof(struct List), MEMF_ANY | MEMF_CLEAR);
		if ( this_node->list != NULL )
		{
			NewList(this_node->list);
			per = (struct PageEventRecord *)AllocMem(	sizeof(struct PageEventRecord),
																								MEMF_ANY | MEMF_CLEAR);
			if (per != NULL)
			{
				AddTail((struct List *)this_node->list, (struct Node *)per);
				per->er_Header.ph_NodeType = TALK_LOCALEVENT;
				for(i=0; i<MAX_LOCAL_EVENTS; i++)
					per->er_LocalEvents[i] = NULL;
			}
		}
		per = (struct PageEventRecord *)(this_node->list->lh_Head);
	}
	else
		per = (struct PageEventRecord *)(this_node->list->lh_Head);

	/**** SCAN PAGETALK DOCUMENT ****/

	PR = (struct ParseRecord *)OpenParseFile(pageCommands, fullPath);
	if (PR!=NULL)
	{
		while(instruc != PARSE_STOP)
		{
			instruc = GetParserLine((struct ParseRecord *)PR, buffer);
			if (instruc == PARSE_INTERPRET)
			{
				passOneParser((struct ParseRecord *)PR, buffer);
				if (passTwoParser((struct ParseRecord *)PR))
				{
					if (line==0 && PR->commandCode!=TALK_PAGETALK)
					{
						Message(msgs[Msg_DocUnreadable-1]);
						validPage=FALSE;
						instruc=PARSE_STOP;
					}
					else
					{
						if ( PR->commandCode == TALK_PAGETALK )
							headerPassed = TRUE;
						PR->sourceLine = line+1;

						if ( PR->commandCode == TALK_BUTTON )
						{
							PerfFunc((struct GenericFuncs *)pageFuncs, PR, (struct ScriptInfoRecord *)&PIR);

							if ( !per->er_LocalEvents[hole] )
							{
								per->er_LocalEvents[hole] = (struct ScriptEventRecord *)
																AllocMem(	sizeof(struct ScriptEventRecord),
																					MEMF_ANY | MEMF_CLEAR );
								if ( !per->er_LocalEvents[hole] )
									return(FALSE);
							}

							if ( per->er_LocalEvents[hole] )
							{
								ser = per->er_LocalEvents[hole];
								hole++;

								buttonsFound++;

								stccpy(ser->buttonName, PIR.ew->buttonName, 49);
								ser->labelSNR = FindLabel(SIR, ser->labelName);

								stccpy(ser->assignment, PIR.ew->assignment, 74);

								ser->keyCode		= PIR.ew->keyCode;
								ser->rawkeyCode	= PIR.ew->rawkeyCode;
								ser->x					= PIR.ew->bx;
								ser->y					= PIR.ew->by;
								ser->width			= PIR.ew->bwidth;
								ser->height			= PIR.ew->bheight;
								ser->renderType	= PIR.ew->renderType;
								ser->audioCue		= PIR.ew->audioCue;
								ser->typeBits		= 0;
	
								if ( PIR.ew->jumpType == JUMPTYPE_GOTO )
									ser->typeBits = TB_ISGOTO;
								else if ( PIR.ew->jumpType == JUMPTYPE_GOSUB )
									ser->typeBits = TB_ISGOSUB;
								else if ( PIR.ew->jumpType == JUMPTYPE_PREV )
									ser->typeBits = TB_ISGOPREV;
								else if ( PIR.ew->jumpType == JUMPTYPE_NEXT )
									ser->typeBits = TB_ISGONEXT;
								else if ( PIR.ew->jumpType == JUMPTYPE_PREVPAGE )
									ser->typeBits = TB_PREVPAGE;
								else if ( PIR.ew->jumpType == JUMPTYPE_NEXTPAGE )
									ser->typeBits = TB_NEXTPAGE;
							}
						}

						if (PR->commandCode == PRINTERROR_CODE)
						{
							validPage=FALSE;
							instruc=PARSE_STOP;
						}
					}
				}
				else
				{
					if (line>0 && PR->commandCode==-1)	/* command not valid */
					{
						sprintf(buffer, msgs[Msg_IllegalCommandInLine-1], line+1);
						printError(PR, buffer);
						validPage=FALSE;
						instruc=PARSE_STOP;
					}
				}
			}
			line++;
		}
		CloseParseFile(PR);
	}
	else	// file can't be opened
	{
		return(FALSE);
	}

	if (!headerPassed || !validPage )
		goto kill_list;

	if ( buttonsFound==0 )
	{
kill_list:

		if ( this_node->list != NULL )
		{
			work_node = (struct PageEventRecord *)(this_node->list->lh_Head);
			per = (struct PageEventRecord *)(this_node->list->lh_Head);

			for(i=0; i<MAX_LOCAL_EVENTS; i++)
			{
				if ( per->er_LocalEvents[i] )
					FreeMem(per->er_LocalEvents[i],sizeof(struct ScriptEventRecord));
			}

			while(next_node = (struct PageEventRecord *)(work_node->er_Header.ph_Node.ln_Succ))
			{
				FreeMem(work_node, sizeof(struct PageEventRecord));
				work_node = next_node;
			}
	
			if (this_node->list!=NULL)
				FreeMem(this_node->list, sizeof(struct List));

			this_node->list = NULL;
		}
	}
}

#ifndef USED_FOR_PLAYER

/******** ModifyLEInfo() ********/

BOOL ModifyLEInfo(	struct ScriptNodeRecord *this_node, STRPTR fullPath,
										struct ScriptInfoRecord *SIR )
{
struct PageInfoRecord PIR;
struct ParseRecord *PR;
TEXT buffer[MAXSCANDEPTH];
int instruc, line;
BOOL validPage=TRUE;
BOOL headerPassed=FALSE;
int i,j,hole,buttonsFound=0;
struct EditWindow tempEW;
struct ScriptEventRecord *ser, *kept_ser_list[MAX_LOCAL_EVENTS], *ser2;
struct PageEventRecord *per, *per2;
struct PageEventRecord *work_node;
struct PageEventRecord *next_node;
struct List *list;
struct ScriptNodeRecord *snr, *snr2;

	/**** init vars ****/

	PIR.ew = &tempEW;	// provide a temporary storage for BUTTON commands
	instruc = -1;
	line=0;
	hole=0;

	if ( this_node->numericalArgs[15] == 2 )	// IFF pic
		goto kill_list;	// see below

	/**** alloc mem for localevent list 																****/
	/**** this ONLY happens when a page gets buttons after the script		****/
	/**** was loaded else this is done by scripttalk.c									****/

	if ( this_node->list == NULL )
	{
		this_node->list = (struct List *)AllocMem(sizeof(struct List), MEMF_ANY | MEMF_CLEAR);
		if ( this_node->list != NULL )
		{
			NewList(this_node->list);
			per = (struct PageEventRecord *)AllocMem(	sizeof(struct PageEventRecord),
																								MEMF_ANY | MEMF_CLEAR);
			if (per != NULL)
			{
				AddTail((struct List *)this_node->list, (struct Node *)per);
				per->er_Header.ph_NodeType = TALK_LOCALEVENT;
				for(i=0; i<MAX_LOCAL_EVENTS; i++)
					per->er_LocalEvents[i] = NULL;
			}
		}
		per = (struct PageEventRecord *)(this_node->list->lh_Head);
	}
	else
		per = (struct PageEventRecord *)(this_node->list->lh_Head);

	for(i=0; i<MAX_LOCAL_EVENTS; i++)
		kept_ser_list[i] = NULL;

	for(i=0; i<MAX_LOCAL_EVENTS; i++)
		kept_ser_list[i] = per->er_LocalEvents[i];

	for(i=0; i<MAX_LOCAL_EVENTS; i++)
		per->er_LocalEvents[i] = NULL;

	/**** SCAN PAGETALK DOCUMENT ****/

	PR = (struct ParseRecord *)OpenParseFile(pageCommands, fullPath);
	if (PR!=NULL)
	{
		while(instruc != PARSE_STOP)
		{
			instruc = GetParserLine((struct ParseRecord *)PR, buffer);
			if (instruc == PARSE_INTERPRET)
			{
				passOneParser((struct ParseRecord *)PR, buffer);
				if (passTwoParser((struct ParseRecord *)PR))
				{
					if (line==0 && PR->commandCode!=TALK_PAGETALK)
					{
						Message(msgs[Msg_DocUnreadable-1]);
						validPage=FALSE;
						instruc=PARSE_STOP;
					}
					else
					{
						if ( PR->commandCode == TALK_PAGETALK )
							headerPassed = TRUE;
						PR->sourceLine = line+1;

						if ( PR->commandCode == TALK_BUTTON )
						{
							PerfFunc((struct GenericFuncs *)pageFuncs, PR, (struct ScriptInfoRecord *)&PIR);

							if ( !per->er_LocalEvents[hole] )
							{
								per->er_LocalEvents[hole] = (struct ScriptEventRecord *)
																AllocMem(	sizeof(struct ScriptEventRecord),
																					MEMF_ANY | MEMF_CLEAR );
								if ( !per->er_LocalEvents[hole] )
									return(FALSE);
							}

							if ( per->er_LocalEvents[hole] )
							{
								ser = per->er_LocalEvents[hole];
								hole++;

								buttonsFound++;

								stccpy(ser->buttonName, PIR.ew->buttonName, 49);
								stccpy(ser->assignment, PIR.ew->assignment, 74);

								ser->keyCode		= PIR.ew->keyCode;
								ser->rawkeyCode	= PIR.ew->rawkeyCode;
								ser->x					= PIR.ew->bx;
								ser->y					= PIR.ew->by;
								ser->width			= PIR.ew->bwidth;
								ser->height			= PIR.ew->bheight;
								ser->renderType	= PIR.ew->renderType;
								ser->audioCue		= PIR.ew->audioCue;
								ser->typeBits		= 0;

								if ( PIR.ew->jumpType == JUMPTYPE_GOTO )
									ser->typeBits = TB_ISGOTO;
								else if ( PIR.ew->jumpType == JUMPTYPE_GOSUB )
									ser->typeBits = TB_ISGOSUB;
								else if ( PIR.ew->jumpType == JUMPTYPE_PREV )
									ser->typeBits = TB_ISGOPREV;
								else if ( PIR.ew->jumpType == JUMPTYPE_NEXT )
									ser->typeBits = TB_ISGONEXT;
								else if ( PIR.ew->jumpType == JUMPTYPE_PREVPAGE )
									ser->typeBits = TB_PREVPAGE;
								else if ( PIR.ew->jumpType == JUMPTYPE_NEXTPAGE )
									ser->typeBits = TB_NEXTPAGE;
							}
						}

						if (PR->commandCode == PRINTERROR_CODE)
						{
							validPage=FALSE;
							instruc=PARSE_STOP;
						}
					}
				}
				else
				{
					if (line>0 && PR->commandCode==-1)	/* command not valid */
					{
						sprintf(buffer, msgs[Msg_IllegalCommandInLine-1], line+1);
						printError(PR, buffer);
						validPage=FALSE;
						instruc=PARSE_STOP;
					}
				}
			}
			line++;
		}
		CloseParseFile(PR);
	}
	else	// file can't be opened
	{
		return(FALSE);
	}

	if (!headerPassed || !validPage )
		goto kill_list;

	for(i=0; i<MAX_LOCAL_EVENTS; i++)
	{
		if ( kept_ser_list[i] )
		{
			for(j=0; j<MAX_LOCAL_EVENTS; j++)
			{
				if ( per->er_LocalEvents[j] )
				{
					if ( !strcmpi(	per->er_LocalEvents[j]->buttonName,
													kept_ser_list[i]->buttonName ) )
					{
						per->er_LocalEvents[j]->labelSNR = FindLabel(SIR, kept_ser_list[i]->labelName);
						strcpy( per->er_LocalEvents[j]->labelName, kept_ser_list[i]->labelName );
					}
				}
			}
		}
	}

	if ( buttonsFound==0 )
	{
kill_list:
		if ( this_node->list != NULL )
		{
			work_node = (struct PageEventRecord *)(this_node->list->lh_Head);
			per = (struct PageEventRecord *)(this_node->list->lh_Head);

			for(i=0; i<MAX_LOCAL_EVENTS; i++)
				if ( per->er_LocalEvents[i] )
					FreeMem(per->er_LocalEvents[i],sizeof(struct ScriptEventRecord));

			while(next_node = (struct PageEventRecord *)(work_node->er_Header.ph_Node.ln_Succ))
			{
				FreeMem(work_node, sizeof(struct PageEventRecord));
				work_node = next_node;
			}
	
			if (this_node->list!=NULL)
				FreeMem(this_node->list, sizeof(struct List));

			this_node->list = NULL;
		}
	}
	else	// update others too
	{
		per = (struct PageEventRecord *)(this_node->list->lh_Head);

		for(i=0; i<CPrefs.MaxNumLists; i++)
		{
			if (SIR->allLists[i])
			{
				list = SIR->allLists[i];
				if (list->lh_TailPred != (struct Node *)list)
				{
					for(snr=(SNRPTR)list->lh_Head; snr->node.ln_Succ;	snr=(SNRPTR)snr->node.ln_Succ)
					{
						if ( snr != this_node )
						{
							if (	!strcmpi(this_node->objectPath, snr->objectPath) &&
										!strcmpi(this_node->objectName, snr->objectName) )
							{
								per2 = (struct PageEventRecord *)(snr->list->lh_Head);
								for(hole=0; hole<MAX_LOCAL_EVENTS; hole++)
								{
									if ( per->er_LocalEvents[hole] )
									{
										ser = per->er_LocalEvents[hole];
										if ( per2->er_LocalEvents[hole] )
										{
											ser2 = per2->er_LocalEvents[hole];
											snr2 = ser2->labelSNR;
											CopyMem(ser, ser2, sizeof(struct ScriptEventRecord));											
											ser2->labelSNR = snr2;
										}
									}	
								}
							}
						}
					}
				}
			}
		}
	}
}

/******** UpdateLEInfo() ********/

BOOL UpdateLEInfo(	struct ScriptNodeRecord *this_node, STRPTR fullPath,
										struct ScriptInfoRecord *SIR )
{
struct PageEventRecord *per;
int i;

	if ( this_node->list )
	{
		per = (struct PageEventRecord *)(this_node->list->lh_Head);
		for(i=0; i<MAX_LOCAL_EVENTS; i++)
		{
			if ( per->er_LocalEvents[i] )
			{
				FreeMem(per->er_LocalEvents[i], sizeof(struct ScriptEventRecord));
				per->er_LocalEvents[i] = NULL;
			}
		}
	}

	GetLEInfo(this_node,fullPath,SIR);

	return(TRUE);
}

#endif

/******** GetAllLEInfos() ********/

void GetAllLEInfos(struct ScriptInfoRecord *SIR)
{
int i;
SNRPTR this_node;
struct List *list;
TEXT fullPath[SIZE_FULLPATH];

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] != NULL)
		{
			list = SIR->allLists[i];
			if (list->lh_TailPred != (struct Node *)list)
			{
				for(this_node=(SNRPTR)list->lh_Head;
						this_node->node.ln_Succ;
						this_node=(SNRPTR)this_node->node.ln_Succ)
				{
					if (	this_node->nodeType==TALK_PAGE && this_node->numericalArgs[15]==1 &&
								this_node->list )
					{
						UA_MakeFullPath(this_node->objectPath, this_node->objectName, fullPath);
						GetLEInfo(this_node,fullPath,SIR);
					}
				}
			}
		}
	}
}

/******** Validate_All_LE() ********/
/*
 * Returns # of pages with interactivity
 *
 */

int Validate_All_LE(struct ScriptInfoRecord *SIR, BOOL makeGOTOS)
{
int i,j,num;
SNRPTR this_node, node;
struct List *list;
struct ScriptEventRecord *ser;
struct PageEventRecord *per;
struct ScriptNodeRecord *dest, *snr1, *snr2;

	num=0;

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] != NULL)
		{
			list = SIR->allLists[i];
			if (list->lh_TailPred != (struct Node *)list)
			{
				for(this_node=(SNRPTR)list->lh_Head;
						this_node->node.ln_Succ;
						this_node=(SNRPTR)this_node->node.ln_Succ)
				{
					if (	this_node->nodeType==TALK_PAGE &&
								this_node->numericalArgs[15]==1 &&
								this_node->list!=NULL )
					{
						j=0;
						per = (struct PageEventRecord *)(this_node->list->lh_Head);

						while( per->er_LocalEvents[j] && (j < MAX_LOCAL_EVENTS) )
						{
							num++;

							ser = (struct ScriptEventRecord *)per->er_LocalEvents[j];						
							ser->labelSNR = (struct ScriptNodeRecord *)FindLabel(SIR, ser->labelName);
							if ( ser->labelSNR==NULL )
							{
								node = (struct ScriptNodeRecord *)ObjectRecord.scriptSIR.allLists[0]->lh_TailPred;
								ser->labelSNR = node;
								strcpy(ser->labelName, node->objectName);
							}

							if ( makeGOTOS )
							{
								dest = ser->labelSNR;	// remember destination

								snr1 = (struct ScriptNodeRecord *)AllocMem(
											sizeof(struct ScriptNodeRecord),MEMF_CLEAR | MEMF_ANY);
								if ( snr1 )
								{
									ser->labelSNR = snr1;

									snr1->node.ln_Name	= NULL;
									snr1->node.ln_Type	= 100;	// arbitrary type identifier
									snr1->node.ln_Pri = 0;
									snr1->list = NULL;
									snr1->nodeType = TALK_GOTO;
									if ( ser->typeBits == TB_ISGOTO )
										snr1->numericalArgs[0] = TG_GOTO;
									else if ( ser->typeBits == TB_ISGOSUB )
										snr1->numericalArgs[0] = TG_GOSUB;
									else if ( ser->typeBits == TB_ISGOPREV )
										snr1->numericalArgs[0] = TG_SPPREVGOSUB;
									else if ( ser->typeBits == TB_ISGONEXT )
										snr1->numericalArgs[0] = TG_SPNEXTGOSUB;

									strcpy(snr1->objectName,"TEMP NAME 1");
									snr1->extraData = (UBYTE *)dest;
									snr1->ParentSNR = FindParentNode(SIR,list);
									snr1->ProcInfo = NULL;
								}

								snr2 = (struct ScriptNodeRecord *)AllocMem(
											sizeof(struct ScriptNodeRecord),MEMF_CLEAR | MEMF_ANY);
								if ( snr2 )
								{
									snr2->node.ln_Name	= NULL;
									snr2->node.ln_Type	= 100;	// arbitrary type identifier
									snr2->node.ln_Pri = 0;
									snr2->list = NULL;
									snr2->nodeType = TALK_GOTO;
									if ( ser->typeBits == TB_ISGOTO )
										snr2->numericalArgs[0] = TG_GOTO;
									else if ( ser->typeBits == TB_ISGOSUB )
										snr2->numericalArgs[0] = TG_GOSUB;
									else if ( ser->typeBits == TB_ISGOPREV )
										snr2->numericalArgs[0] = TG_SPPREVGOSUB;
									else if ( ser->typeBits == TB_ISGONEXT )
										snr2->numericalArgs[0] = TG_SPNEXTGOSUB;

									strcpy(snr2->objectName,"TEMP NAME 2");
									snr2->extraData = (UBYTE *)dest;
									snr2->ParentSNR = FindParentNode(SIR,list);
									snr2->ProcInfo = NULL;
								}

								snr1->node.ln_Pred = this_node->node.ln_Pred;
								snr1->node.ln_Succ = (struct Node *)snr2;

								snr2->node.ln_Pred = (struct Node *)snr1;
								snr2->node.ln_Succ = this_node->node.ln_Succ;
							}

							j++;
						}
					}
				}
			}
		}
	}

	return( num );
}

/******** Free_All_LE() ********/

void Free_All_LE( struct ScriptInfoRecord *SIR )
{
int i,j;
SNRPTR this_node;
struct List *list;
struct ScriptEventRecord *ser;
struct PageEventRecord *per;
struct ScriptNodeRecord *snr1, *snr2;

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] != NULL)
		{
			list = SIR->allLists[i];
			if (list->lh_TailPred != (struct Node *)list)
			{
				for(this_node=(SNRPTR)list->lh_Head; this_node->node.ln_Succ;
						this_node=(SNRPTR)this_node->node.ln_Succ)
				{
					if (	this_node->nodeType==TALK_PAGE &&
								this_node->numericalArgs[15]==1 &&
								this_node->list!=NULL )
					{
						j=0;
						per = (struct PageEventRecord *)(this_node->list->lh_Head);
						while( per->er_LocalEvents[j] && (j < MAX_LOCAL_EVENTS) )
						{
							ser = (struct ScriptEventRecord *)per->er_LocalEvents[j];						
							snr1 = ser->labelSNR;
							ser->labelSNR = NULL;

							snr2 = (SNRPTR)snr1->node.ln_Succ;
							if ( snr1 )
								FreeMem(snr1, sizeof(struct ScriptNodeRecord));
							if ( snr2 )
								FreeMem(snr2, sizeof(struct ScriptNodeRecord));

							j++;
						}
					}
				}
			}
		}
	}
}

#ifndef USED_FOR_PLAYER

/******** Copy_LE_Info() ********/

void Copy_LE_Info(struct ScriptNodeRecord *oldSNR, struct ScriptNodeRecord *newSNR)
{
struct PageEventRecord *per;
struct ScriptEventRecord **old_serlist, **new_serlist;
int i;

	newSNR->list = (struct List *)AllocMem(sizeof(struct List), MEMF_ANY | MEMF_CLEAR);
	if ( newSNR->list != NULL )
	{
		NewList(newSNR->list);
		per = (struct PageEventRecord *)AllocMem(	sizeof(struct PageEventRecord),
																							MEMF_ANY | MEMF_CLEAR);
		if (per != NULL)
		{
			AddTail((struct List *)newSNR->list, (struct Node *)per);
			per->er_Header.ph_NodeType = TALK_LOCALEVENT;
			for(i=0; i<MAX_LOCAL_EVENTS; i++)
				per->er_LocalEvents[i] = NULL;
		}
	}

	per = (struct PageEventRecord *)(oldSNR->list->lh_Head);
	old_serlist = (struct ScriptEventRecord **)per->er_LocalEvents;

	per = (struct PageEventRecord *)(newSNR->list->lh_Head);
	new_serlist = (struct ScriptEventRecord **)per->er_LocalEvents;

	for(i=0; i<MAX_LOCAL_EVENTS; i++)
	{
		if ( old_serlist[i] != NULL )
		{
			new_serlist[i] = (struct ScriptEventRecord *)AllocMem(	sizeof(struct ScriptEventRecord),
																															MEMF_ANY | MEMF_CLEAR);
			if ( new_serlist[i] != NULL )
			{
				CopyMem(old_serlist[i], new_serlist[i], sizeof(struct ScriptEventRecord));
			}
		}
		else
			break;
	}	
}

#endif

/******** IsXappUsed() ********/

BOOL IsXappUsed(STRPTR xappname)
{
struct Node *node;

	for(node=usedxapplist.lh_Head; node->ln_Succ; node=node->ln_Succ)
		if ( !stricmp(node->ln_Name,xappname) )
			return(TRUE);

	return(FALSE);
}

/******** E O F ********/
