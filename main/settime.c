#include "nb:pre.h"

/**** defines ****/

#define HMST_SER	1
#define HMST_PAR	2
#define HMST_PRG	3
#define TC_SER		4

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Window *scriptWindow;
extern struct Library *medialinkLibBase;
extern struct ObjectInfo ObjectRecord;
extern BOOL blockScript;
extern struct Gadget ScriptSlider2;
extern struct TextFont *smallFont;
extern struct TextFont *largeFont;
extern UWORD chip gui_pattern[];
extern LONG topEntry1;

/**** static globals ****/

static BYTE days[10];

/**** gadgets ****/

extern struct GadgetRecord SetTime1_GR[];
extern struct GadgetRecord SetTime2_GR[];
extern struct GadgetRecord SetTime3_GR[];
extern struct GadgetRecord SetTime4_GR[];
extern struct GadgetRecord Script_GR[];

/**** functions ****/

/******** SetTime() ********/

void SetTime(void)
{
struct Window *window, *activeWindow;
BOOL loop, retVal;
int mode, i, dummy, numObj;
struct GadgetRecord *GR;
struct ScriptNodeRecord *this_node, *my_node;
LONG *undoDuration=NULL;
PUNCHDATA *undoPunchData_Start=NULL, *undoPunchData_End=NULL;

	retVal = FALSE;
	loop   = TRUE;
	for(i=0; i<7; i++)
		days[i] = -1;

	/**** open a window ****/

	if ( ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS )
	{
		if (CPrefs.showDays)
		{
			mode = HMST_PRG;
			GR = SetTime3_GR;
		}
		else
		{
			mode = HMST_SER;
			GR = SetTime1_GR;
		}
		if ( ObjectRecord.scriptSIR.listType==TALK_STARTPAR )
		{
			mode = HMST_PAR;
			GR = SetTime2_GR;
		}
	}
	else
	{
		mode = TC_SER;
		GR = SetTime4_GR;
	}

	window = UA_OpenRequesterWindow(scriptWindow, GR, STDCOLORS);
	if (!window)
	{
		UA_WarnUser(-1);
		return;
	}

	blockScript	= TRUE;

	/**** render gadget ****/

	UA_DrawGadgetList(window, GR);

	/**** disable script buttons ****/

	DisableAllEventIcons();
	UA_DisableButton(scriptWindow, &Script_GR[4], gui_pattern);	// Play
	UA_DisableButton(scriptWindow, &Script_GR[7], gui_pattern);	// Edit
	UA_DisableButton(scriptWindow, &Script_GR[8], gui_pattern);	// Show
	if ( CPrefs.userLevel > 2 )
		UA_DisableButton(scriptWindow, &Script_GR[5], gui_pattern);	// Parent

	OffGadget(&ScriptSlider2, scriptWindow, NULL);

	/**** render arrows ****/

	RenderDelayArrows(window,GR,mode);

	numObj = ObjectRecord.numObjects;

	/**** allocate undo buffer ****/

	if ( mode == HMST_SER || mode == HMST_PRG )
	{
		undoDuration = (LONG *)AllocMem(numObj * sizeof(LONG), MEMF_CLEAR | MEMF_ANY);
		if ( undoDuration )
		{
			i=0;
			for(	my_node=(SNRPTR)ObjectRecord.firstObject; my_node->node.ln_Succ;
						my_node=(SNRPTR)my_node->node.ln_Succ	)
			{
				undoDuration[i] = my_node->duration;
				i++;
			}
		}
	}
	else	// HMST_PAR and TC_SER
	{
		undoPunchData_Start	= (PUNCHDATA *)AllocMem(numObj*sizeof(PUNCHDATA), MEMF_CLEAR | MEMF_ANY);
		undoPunchData_End		= (PUNCHDATA *)AllocMem(numObj*sizeof(PUNCHDATA), MEMF_CLEAR | MEMF_ANY);
		if ( undoPunchData_Start && undoPunchData_End )
		{
			i=0;
			for(	my_node=(SNRPTR)ObjectRecord.firstObject; my_node->node.ln_Succ;
						my_node=(SNRPTR)my_node->node.ln_Succ	)
			{
				CopyMem(&my_node->Start,	&undoPunchData_Start[i],	sizeof(PUNCHDATA));
				CopyMem(&my_node->End,		&undoPunchData_End[i],		sizeof(PUNCHDATA));
				i++;
			}
		}
	}

	/**** event handler ****/

	this_node = GetFirstSelObj();

	while(loop)
	{
		doStandardWait(window);

		Forbid();
		activeWindow = IntuitionBase->ActiveWindow;
		Permit();

		/**** process script editor window events ****/

		if ( activeWindow==scriptWindow )
		{
			if ( CED.Class==IDCMP_MOUSEBUTTONS )
			{
				if ( doScriptMouseButtons(&dummy) )
					this_node = (struct ScriptNodeRecord *)CountNumSelected(ObjectRecord.firstObject,&i);
			}
			else if (CED.Class==IDCMP_RAWKEY)
			{
				dokeyScrolling();
				DoSelAll();
			}
			CED.Class = NULL;
		}

		switch(mode)
		{
			case HMST_SER:
				Check_HMST_SER_Buttons(window, GR, &loop, &retVal, this_node);
				break;
			case HMST_PAR:
				Check_HMST_PAR_Buttons(window, GR, &loop, &retVal, this_node);
				break;
			case HMST_PRG:
				Check_HMST_PRG_Buttons(window, GR, &loop, &retVal, this_node);
				break;
			case TC_SER:
				Check_TC_SER_Buttons(window, GR, &loop, &retVal, this_node);
				break;
		}
	}

	UA_CloseRequesterWindow(window,STDCOLORS);

	blockScript	= FALSE;

	/**** disable script buttons ****/

	EnableAllEventIcons();
	if (CPrefs.ScriptScreenModes & LACE)
		SetFont(scriptWindow->RPort, largeFont);
	UA_EnableButton(scriptWindow, &Script_GR[4]);	// Play
	if ( CPrefs.userLevel > 2 )
		UA_EnableButton(scriptWindow, &Script_GR[5]);	// Parent

	SetFont(scriptWindow->RPort, smallFont);
	ScriptSlider2On();

	for(i=0; i<=13; i++)
		UA_EnableButtonQuiet( &SetTime1_GR[i] );
	for(i=0; i<=22; i++)
		UA_EnableButtonQuiet( &SetTime2_GR[i] );
	for(i=0; i<=21; i++)
		UA_EnableButtonQuiet( &SetTime3_GR[i] );
	for(i=0; i<=22; i++)
		UA_EnableButtonQuiet( &SetTime4_GR[i] );

	/**** free memory ****/

	if ( !retVal )	// undo all duration changes
	{
		if ( mode == HMST_SER || mode == HMST_PRG )
		{
			i=0;
			for(	my_node=(SNRPTR)ObjectRecord.firstObject; my_node->node.ln_Succ;
						my_node=(SNRPTR)my_node->node.ln_Succ	)
			{
				my_node->duration = undoDuration[i];
				i++;
			}
		}
		else	// HMST_PAR and TC_SER
		{
			i=0;
			for(	my_node=(SNRPTR)ObjectRecord.firstObject; my_node->node.ln_Succ;
						my_node=(SNRPTR)my_node->node.ln_Succ	)
			{
				CopyMem(&undoPunchData_Start[i],	&my_node->Start,	sizeof(PUNCHDATA));
				CopyMem(&undoPunchData_End[i],		&my_node->End,		sizeof(PUNCHDATA));
				i++;
			}
		}
		DrawObjectList(-1, TRUE, TRUE);	// only redraw object name part
	}

	if ( mode == HMST_SER || mode == HMST_PRG )
	{
		if ( undoDuration )
			FreeMem(undoDuration, numObj*sizeof(LONG));
	}
	else	// HMST_PAR and TC_SER
	{
		if ( undoPunchData_Start )
			FreeMem(undoPunchData_Start, numObj*sizeof(PUNCHDATA));
		if ( undoPunchData_End )
			FreeMem(undoPunchData_End, numObj*sizeof(PUNCHDATA));
	}

	FixDurationAndDays(ObjectRecord.firstObject);
}

/******** Check_HMST_SER_Buttons() ********/

void Check_HMST_SER_Buttons(	struct Window *window, struct GadgetRecord *GR,
															BOOL *loop, BOOL *retVal,
															struct ScriptNodeRecord *this_node )
{
int i,ID;
TEXT str[20];

	if ( this_node && this_node->nodeType < TALK_GOTO )	//&& this_node->nodeType!=TALK_STARTSER )
	{
		if ( UA_IsGadgetDisabled(&GR[3]) )
		{
			for(i=3; i<=11; i++)
				UA_EnableButton(window, &GR[i]);
			RenderDelayArrows(window,GR,HMST_SER);
		}
		if ( this_node->duration == -1 )
			this_node->duration = DEFAULT_DELAY*10;
		secondsToDuration(this_node->duration, str);
		UA_SetStringGadgetToString(window, &GR[11], str);
	}
	else
		for(i=3; i<=11; i++)
			UA_DisableButton(window, &GR[i], gui_pattern);

	if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
	{
		ID = UA_CheckGadgetList(window, GR, &CED);
		switch(ID)
		{
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
				UA_InvertButton(window, &GR[ID]);
				MonitorArrows(window, HMST_SER, ID-3, &GR[11]);
				UA_SetStringToGadgetString(&GR[11], str);
				SetTiming(str, NULL, 128);
				UA_InvertButton(window, &GR[ID]);
				break;

			case 11:	// time gadget
				UA_ProcessStringGadget(window, GR, &GR[ID], &CED);
				CheckEnteredTime(&GR[ID]);
				UA_DrawGadget(window, &GR[ID]);
				UA_SetStringToGadgetString(&GR[ID],str);
				SetTiming(str, NULL, 128);
				break;

			case 12:	// OK
do_ok:
				UA_HiliteButton(window, &GR[12]);
				*loop=FALSE;
				*retVal=TRUE;
				break;

			case 13:	// Cancel
do_cancel:
				UA_HiliteButton(window, &GR[13]);
				*loop=FALSE;
				*retVal=FALSE;
				break;
		}
	}
	else if (CED.Class==IDCMP_RAWKEY)
	{
		if (CED.Code==RAW_ESCAPE)				// cancel
			goto do_cancel;
		else if (CED.Code==RAW_RETURN)	// OK
			goto do_ok;
		DoSelAll();
	}
}

/******** Check_HMST_PAR_Buttons() ********/

void Check_HMST_PAR_Buttons(	struct Window *window, struct GadgetRecord *GR,
															BOOL *loop, BOOL *retVal,
															struct ScriptNodeRecord *this_node )
{
int i,ID;
TEXT str[20];

	if ( this_node && this_node->nodeType < TALK_GOTO )	//&& this_node->nodeType!=TALK_STARTSER )
	{
		if ( UA_IsGadgetDisabled(&GR[3]) )
		{
			for(i= 3; i<=11; i++)
				UA_EnableButton(window, &GR[i]);
			for(i=12; i<=20; i++)
				UA_EnableButton(window, &GR[i]);
			RenderDelayArrows(window,GR,HMST_PAR);
		}
		if ( this_node->Start.ParHMSTOffset == -1 )
			this_node->Start.ParHMSTOffset = 0;
		if ( this_node->End.ParHMSTOffset == -1 )
			this_node->End.ParHMSTOffset = 0;
		secondsToDuration(this_node->Start.ParHMSTOffset, str);
		UA_SetStringGadgetToString(window, &GR[11], str);
		secondsToDuration(this_node->End.ParHMSTOffset, str);
		UA_SetStringGadgetToString(window, &GR[20], str);
	}
	else
	{
		for(i= 3; i<=11; i++)
			UA_DisableButton(window, &GR[i], gui_pattern);
		for(i=12; i<=20; i++)
			UA_DisableButton(window, &GR[i], gui_pattern);
	}

	if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
	{
		ID = UA_CheckGadgetList(window, GR, &CED);
		switch(ID)
		{
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
				UA_InvertButton(window, &GR[ID]);
				MonitorArrows(window, HMST_PAR, ID-3, &GR[11]);
				UA_SetStringToGadgetString(&GR[11], str);
				SetTiming(str, NULL, 128);
				UA_InvertButton(window, &GR[ID]);
				break;

			case 11:	// time gadget
				UA_ProcessStringGadget(window, GR, &GR[ID], &CED);
				CheckEnteredTime(&GR[ID]);
				UA_DrawGadget(window, &GR[ID]);
				UA_SetStringToGadgetString(&GR[ID],str);
				SetTiming(str, NULL, 128);
				break;

			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
			case 19:
				UA_InvertButton(window, &GR[ID]);
				MonitorArrows(window, HMST_PAR, ID-12, &GR[20]);
				UA_SetStringToGadgetString(&GR[20], str);
				SetTiming(NULL, str, 128);
				UA_InvertButton(window, &GR[ID]);
				break;

			case 20:	// time gadget
				UA_ProcessStringGadget(window, GR, &GR[ID], &CED);
				CheckEnteredTime(&GR[ID]);
				UA_DrawGadget(window, &GR[ID]);
				UA_SetStringToGadgetString(&GR[ID],str);
				SetTiming(NULL, str, 128);
				break;

			case 21:	// OK
do_ok:
				UA_HiliteButton(window, &GR[21]);
				*loop=FALSE;
				*retVal=TRUE;
				break;

			case 22:	// Cancel
do_cancel:
				UA_HiliteButton(window, &GR[22]);
				*loop=FALSE;
				*retVal=FALSE;
				break;
		}
	}
	else if (CED.Class==IDCMP_RAWKEY)
	{
		if (CED.Code==RAW_ESCAPE)				// cancel
			goto do_cancel;
		else if (CED.Code==RAW_RETURN)	// OK
			goto do_ok;
		DoSelAll();
	}
}

/******** Check_HMST_PRG_Buttons() ********/

void Check_HMST_PRG_Buttons(	struct Window *window, struct GadgetRecord *GR,
															BOOL *loop, BOOL *retVal,
															struct ScriptNodeRecord *this_node	)
{
int i,ID;
TEXT str[20];

	if ( this_node && this_node->nodeType < TALK_GOTO )	//&& this_node->nodeType!=TALK_STARTSER )
	{
		if ( UA_IsGadgetDisabled(&GR[3]) )
		{
			for(i=3; i<=19; i++)
				UA_EnableButton(window, &GR[i]);
			RenderDelayArrows(window,GR,HMST_SER);
			for(i=0; i<7; i++)
				days[i]=-1;
		}

		if ( this_node->duration == -1 )
			this_node->duration = DEFAULT_DELAY*10;
		secondsToDuration(this_node->duration, str);
		UA_SetStringGadgetToString(window, &GR[11], str);

		if ( days[0]==-1 )	// first time
		{ 
			for(i=0; i<7; i++)
			{
				if ( this_node->dayBits & (1<<i) )
				{
					days[i]=TRUE;
					UA_InvertCheckButton(window, &GR[12+i]);
				}
				else
					days[i]=FALSE;
			}
		}
		else
		{
			for(i=0; i<7; i++)
			{
				if ( (this_node->dayBits & (1<<i)) && !days[i] )
					UA_InvertCheckButton(window, &GR[12+i]);	// must be on, was off
				else if ( !(this_node->dayBits & (1<<i)) && days[i] )
					UA_InvertCheckButton(window, &GR[12+i]);	// must be off, was on

				if ( this_node->dayBits & (1<<i) )
					days[i]=TRUE;
				else
					days[i]=FALSE;
			}
		}
	}
	else
		for(i=3; i<=19; i++)
			UA_DisableButton(window, &GR[i], gui_pattern);

	if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
	{
		ID = UA_CheckGadgetList(window, GR, &CED);
		switch(ID)
		{
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
				UA_InvertButton(window, &GR[ID]);
				MonitorArrows(window, HMST_PRG, ID-3, &GR[11]);
				UA_SetStringToGadgetString(&GR[11], str);
				SetTiming(str, NULL, 128);
				UA_InvertButton(window, &GR[ID]);
				break;

			case 11:	// time gadget
				UA_ProcessStringGadget(window, GR, &GR[ID], &CED);
				CheckEnteredTime(&GR[ID]);
				UA_DrawGadget(window, &GR[ID]);
				UA_SetStringToGadgetString(&GR[ID],str);
				SetTiming(str, NULL, 128);
				break;

			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
				if ( CED.Qualifier&IEQUALIFIER_LSHIFT || CED.Qualifier&IEQUALIFIER_RSHIFT )
				{
					for(i=12; i<=18; i++)
					{
						UA_InvertCheckButton(window, &GR[i]);
						InvertByteBit(&this_node->dayBits, (1<<(i-12)) );
						if (this_node->dayBits & (1<<(i-12)))
							days[i-12] = TRUE;
						else
							days[i-12] = FALSE;
					}
				}
				else
				{
					UA_InvertCheckButton(window, &GR[ID]);
					InvertByteBit(&this_node->dayBits, (1<<(ID-12)) );
					if (this_node->dayBits & (1<<(ID-12)))
						days[ID-12] = TRUE;
					else
						days[ID-12] = FALSE;
				}
				SetTiming(NULL,NULL,this_node->dayBits);
				break;

			case 19:	// Scheduling
				UA_InvertButton(window, &GR[ID]);
				if ( this_node )
				{
					if ( Build_program_Requester(scriptWindow,0,this_node) )
					{
						secondsToDuration(this_node->duration, str);
						UA_SetStringGadgetToString(window, &GR[11], str);
						SetTiming(str, NULL, this_node->dayBits);
					}
				}
				UA_InvertButton(window, &GR[ID]);
				break;

			case 20:	// OK
do_ok:
				UA_HiliteButton(window, &GR[20]);
				*loop=FALSE;
				*retVal=TRUE;
				break;

			case 21:	// Cancel
do_cancel:
				UA_HiliteButton(window, &GR[21]);
				*loop=FALSE;
				*retVal=FALSE;
				break;
		}
	}
	else if (CED.Class==IDCMP_RAWKEY)
	{
		if (CED.Code==RAW_ESCAPE)				// cancel
			goto do_cancel;
		else if (CED.Code==RAW_RETURN)	// OK
			goto do_ok;
		DoSelAll();
	}
}

/******** Check_TC_SER_Buttons() ********/

void Check_TC_SER_Buttons(	struct Window *window, struct GadgetRecord *GR,
														BOOL *loop, BOOL *retVal,
														struct ScriptNodeRecord *this_node )
{
int i,ID;
TEXT str[20];

	if ( this_node && this_node->nodeType < TALK_GOTO )	//&& this_node->nodeType!=TALK_STARTSER )
	{
		if ( UA_IsGadgetDisabled(&GR[3]) )
		{
			for(i= 3; i<=11; i++)
				UA_EnableButton(window, &GR[i]);
			for(i=12; i<=20; i++)
				UA_EnableButton(window, &GR[i]);
			RenderDelayArrows(window,GR,TC_SER);
		}
		HoursMinsSecsFramesToString(str,
																this_node->Start.TimeCode.HH,
																this_node->Start.TimeCode.MM,
																this_node->Start.TimeCode.SS,
																this_node->Start.TimeCode.FF);
		UA_SetStringGadgetToString(window, &GR[11], str);
		HoursMinsSecsFramesToString(str,
																this_node->End.TimeCode.HH,
																this_node->End.TimeCode.MM,
																this_node->End.TimeCode.SS,
																this_node->End.TimeCode.FF);
		UA_SetStringGadgetToString(window, &GR[20], str);
	}
	else
	{
		for(i= 3; i<=11; i++)
			UA_DisableButton(window, &GR[i], gui_pattern);
		for(i=12; i<=20; i++)
			UA_DisableButton(window, &GR[i], gui_pattern);
	}

	if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
	{
		ID = UA_CheckGadgetList(window, GR, &CED);
		switch(ID)
		{
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
				UA_InvertButton(window, &GR[ID]);
				MonitorArrows(window, TC_SER, ID-3, &GR[11]);
				UA_SetStringToGadgetString(&GR[11], str);
				SetTiming(str, NULL, 128);
				UA_InvertButton(window, &GR[ID]);
				break;

			case 11:	// time gadget
				UA_ProcessStringGadget(window, GR, &GR[ID], &CED);
				//CheckEnteredTime(&GR[ID]);
				CheckEnteredTimeCode(ObjectRecord.scriptSIR.timeCodeRate,&GR[ID]);
				UA_DrawGadget(window, &GR[ID]);
				UA_SetStringToGadgetString(&GR[ID],str);
				SetTiming(str, NULL, 128);
				break;

			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
			case 19:
				UA_InvertButton(window, &GR[ID]);
				MonitorArrows(window, TC_SER, ID-12, &GR[20]);
				UA_SetStringToGadgetString(&GR[20], str);
				SetTiming(NULL, str, 128);
				UA_InvertButton(window, &GR[ID]);
				break;

			case 20:	// time gadget
				UA_ProcessStringGadget(window, GR, &GR[ID], &CED);
				//CheckEnteredTime(&GR[ID]);
				CheckEnteredTimeCode(ObjectRecord.scriptSIR.timeCodeRate,&GR[ID]);
				UA_DrawGadget(window, &GR[ID]);
				UA_SetStringToGadgetString(&GR[ID],str);
				SetTiming(NULL, str, 128);
				break;

			case 21:	// OK
do_ok:
				UA_HiliteButton(window, &GR[21]);
				*loop=FALSE;
				*retVal=TRUE;
				break;

			case 22:	// Cancel
do_cancel:
				UA_HiliteButton(window, &GR[22]);
				*loop=FALSE;
				*retVal=FALSE;
				break;
		}
	}
	else if (CED.Class==IDCMP_RAWKEY)
	{
		if (CED.Code==RAW_ESCAPE)				// cancel
			goto do_cancel;
		else if (CED.Code==RAW_RETURN)	// OK
			goto do_ok;
		DoSelAll();
	}
}

/******** PrintHorizText() ********/

void PrintHorizText(struct Window *window, struct GadgetRecord *GR, int y, STRPTR str)
{
int x, len;

	SetAPen(window->RPort, LO_PEN);
	SetDrMd(window->RPort, JAM1);

	len = TextLength(window->RPort, str, strlen(str));
	x = GR->x2 - GR->x1;
	x = (x-len)/2;
	x = GR->x1 + x;
	x++;

	if ( !(window->WScreen->ViewPort.Modes & LACE) )
		y--;

	Move(window->RPort, x, GR->y1 + y + window->RPort->TxBaseline);
	Text(window->RPort, str, strlen(str));
}

/******** SetTiming() ********/

void SetTiming(STRPTR str1, STRPTR str2, UBYTE dayBits)
{
struct ScriptNodeRecord *this_node;
int h,m,s,f;

	for(	this_node=(SNRPTR)ObjectRecord.firstObject; this_node->node.ln_Succ;
				this_node=(SNRPTR)this_node->node.ln_Succ	)
	{
		if ( this_node->miscFlags & OBJ_SELECTED )
		{
			PutDateIntoIt(this_node);	// make -1 dates OK else dol.c gets confused

			if ( this_node->nodeType < TALK_GOTO )	//&& this_node->nodeType!=TALK_STARTSER )
			{
				this_node->miscFlags |= OBJ_NEEDS_REFRESH;

				if ( ObjectRecord.scriptSIR.timeCodeFormat == TIMEFORMAT_HHMMSS )
				{
					if ( ObjectRecord.scriptSIR.listType==TALK_STARTPAR )
					{
						if (str1)
							DurationStringToSeconds(str1, &this_node->Start.ParHMSTOffset);
						if (str2)
							DurationStringToSeconds(str2, &this_node->End.ParHMSTOffset);
					}
					else if ( str1 )
						DurationStringToSeconds(str1, &this_node->duration);

					if ( dayBits!=128 )
						this_node->dayBits = dayBits;
				}
				else	// timecode
				{
					if ( str1 )
					{
						timeStringtoTimeCode(str1, &h, &m, &s, &f);
						this_node->Start.TimeCode.HH = h;
						this_node->Start.TimeCode.MM = m;
						this_node->Start.TimeCode.SS = s;
						this_node->Start.TimeCode.FF = f;
					}
					if ( str2 )
					{
						timeStringtoTimeCode(str2, &h, &m, &s, &f);
						this_node->End.TimeCode.HH = h;
						this_node->End.TimeCode.MM = m;
						this_node->End.TimeCode.SS = s;
						this_node->End.TimeCode.FF = f;
					}
				}
			}
		}
	}
	DrawTimeCodes(topEntry1);
}

/********	RenderDelayArrows() ********/

void RenderDelayArrows(struct Window *window, struct GadgetRecord *GR, int mode)
{
int i,add,a,b,c,d;

	if ( CPrefs.ScriptScreenModes & LACE )
		add=3;
	else
		add=2;

	if ( mode == HMST_SER || mode == HMST_PRG )
	{
		a=3;
		b=7;
		c=0;
		d=0;
	}
	else if ( mode == HMST_PAR || mode == TC_SER )
	{
		a=3;
		b=7;
		c=12;
		d=16;
	}

	for(i=0; i<=3; i++)
	{
		PrintHorizText(window, &GR[a+i], 3, "–\0");
		PrintHorizText(window, &GR[b+i], 3, "—\0");
		if ( c>0 )
			PrintHorizText(window, &GR[c+i], 3, "–\0");
		if ( d>0 )
			PrintHorizText(window, &GR[d+i], 3, "—\0");
	}
}

/******** MonitorArrows() ********/

void MonitorArrows(struct Window *window, int mode, int ID, struct GadgetRecord *GR)
{
TEXT str[20];
int h,m,s,t,hold=0,add;
struct IntuiMessage *message;
ULONG signals;
BOOL loop=TRUE;

	UA_SetStringToGadgetString(GR,str);

	if ( mode == HMST_SER || mode == HMST_PAR || mode == HMST_PRG )
	{
		timeStringtoDuration(str,&h,&m,&s,&t);
		if      (ID==0) h++; 
		else if (ID==1) m++; 
		else if (ID==2) s++; 
		else if (ID==3) t++; 
		else if (ID==4) h--; 
		else if (ID==5) m--; 
		else if (ID==6) s--; 
		else if (ID==7) t--; 
		CheckHMST_Cycle(&h, &m, &s, &t);
		HoursMinsSecsTenthsToString(str, h, m, s, t);
		UA_SetStringGadgetToString(window, GR, str);
	}
	else
	{
		timeStringtoTimeCode(str, &h, &m, &s, &t);
		if      (ID==0) h++; 
		else if (ID==1) m++; 
		else if (ID==2) s++; 
		else if (ID==3) t++; 
		else if (ID==4) h--; 
		else if (ID==5) m--; 
		else if (ID==6) s--; 
		else if (ID==7) t--; 
		CheckHMSF_Cycle(&h, &m, &s, &t, ObjectRecord.scriptSIR.timeCodeRate);
		HoursMinsSecsFramesToString(str, h, m, s, t);
		UA_SetStringGadgetToString(window, GR, str);
	}

	add=1;

	UA_SwitchFlagsOn(window, IDCMP_INTUITICKS);

	while(loop)
	{
		signals = Wait(SIGNALMASK);
		if (signals & SIGNALMASK)
		{
			while(message = (struct IntuiMessage *)GetMsg(capsPort))
			{
				CED.Class	= message->Class;
				CED.Code	= message->Code;
				ReplyMsg((struct Message *)message);
				switch(CED.Class)
				{
					case IDCMP_MOUSEBUTTONS:
						if (CED.Code == SELECTUP)
							loop=FALSE;
						break;

					case IDCMP_INTUITICKS:
						if ( hold>3 )
						{
							if      (ID==0) h+=add; 
							else if (ID==1) m+=add; 
							else if (ID==2) s+=add; 
							else if (ID==3) t+=add; 
							else if (ID==4) h-=add; 
							else if (ID==5) m-=add; 
							else if (ID==6) s-=add; 
							else if (ID==7) t-=add; 

							if ( mode == HMST_SER || mode == HMST_PAR || mode == HMST_PRG )
							{
								CheckHMST_Cycle(&h, &m, &s, &t);
								HoursMinsSecsTenthsToString(str, h, m, s, t);
								UA_SetStringGadgetToString(window, GR, str);
							}
							else
							{
								CheckHMSF_Cycle(&h, &m, &s, &t, ObjectRecord.scriptSIR.timeCodeRate);
								HoursMinsSecsFramesToString(str, h, m, s, t);
								UA_SetStringGadgetToString(window, GR, str);
							}

							hold++;
							if (hold>8)
								add=2;
						}
						else
							hold++;
						break;
				}
			}
		}
	}

	UA_SwitchFlagsOff(window, IDCMP_INTUITICKS);
}

/******** FixDurationAndDays() ********/

void FixDurationAndDays(struct ScriptNodeRecord *firstSNR)
{
struct ScriptNodeRecord *this_node;

	for(	this_node=(SNRPTR)firstSNR;
				this_node->node.ln_Succ;
				this_node=(SNRPTR)this_node->node.ln_Succ	)
	{
		if ( this_node->nodeType >= TALK_GOTO )
		{
			if (ObjectRecord.scriptSIR.timeCodeFormat == TIMEFORMAT_HHMMSS)
			{
				this_node->duration	= DEFAULT_DELAY*10;	// 10 seconds
				this_node->dayBits	= 127;
			}
			else
			{
				this_node->Start.TimeCode.HH = 00;
				this_node->Start.TimeCode.MM = 00;
				this_node->Start.TimeCode.SS = 00;
				this_node->Start.TimeCode.FF = 00;

				this_node->End.TimeCode.HH = 00;
				this_node->End.TimeCode.MM = 00;
				this_node->End.TimeCode.SS = 00;
				this_node->End.TimeCode.FF = 00;
			}
		}
	}
}

/******** E O F ********/
