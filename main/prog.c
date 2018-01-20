#include "nb:pre.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Screen *scriptScreen;
extern struct Library *medialinkLibBase;
extern struct ObjectInfo ObjectRecord;
extern char *scriptCommands[];
extern struct Funcs *scriptFuncs[];
extern char *dateconv_monthNames[];
extern UWORD chip gui_pattern[];

/**** gadgets ****/

extern struct GadgetRecord Prog_GR[];
extern struct GadgetRecord ProgBackup_GR[];

/**** functions ****/

/******** Build_program_Requester() ********/

BOOL Build_program_Requester(	struct Window *onWindow, int top,
															struct ScriptNodeRecord *from_node )
{
int row;
SNRPTR this_node, selected_node;
struct ScriptNodeRecord work_node;
struct Window *window;
BOOL retVal=FALSE;

	if ( from_node==NULL )
	{
		/**** select object that was dbl clicked on ****/

		this_node = (SNRPTR)WhichObjectWasClicked(top, &row);
		if (this_node==NULL)
			return(FALSE);

		if (	this_node->nodeType >= TALK_GOTO ||
					/*this_node->nodeType==TALK_STARTSER ||*/
					this_node->nodeType==TALK_STARTPAR	)
			return(FALSE);

		DeselectAllButThisOne(this_node);

		/**** take last selected object in list as template ****/

		selected_node = NULL;
		for(this_node=(SNRPTR)ObjectRecord.firstObject;
				this_node->node.ln_Succ;
				this_node=(SNRPTR)this_node->node.ln_Succ)
		{
			if ( 	(this_node->miscFlags & OBJ_SELECTED) &&
						this_node->nodeType < TALK_GOTO &&
						/* this_node->nodeType!=TALK_STARTSER && */
						this_node->nodeType!=TALK_STARTPAR )
				selected_node = this_node;
		}
	}
	else
		selected_node = from_node;

	if (selected_node == NULL)
	{
		DisplayBeep(scriptScreen);
		return(FALSE);	
	}

	CopyMem(selected_node, &work_node, sizeof(struct ScriptNodeRecord));

	Prog_GR[0].x2 = scriptScreen->Width;
	Prog_GR[0].y2 = 77;
	Prog_GR[1].x2 = scriptScreen->Width-1;
	Prog_GR[1].y2 = 76;

	if (CPrefs.ScriptScreenModes & LACE)
	{
		Prog_GR[0].y2 *= 2;
		Prog_GR[1].y2 *= 2;
	}

	Prog_GR[3].x1 = ProgBackup_GR[1].x1;
	Prog_GR[3].y1 = ProgBackup_GR[1].y1;
	Prog_GR[3].x2 = ProgBackup_GR[1].x2;
	Prog_GR[3].y2 = ProgBackup_GR[1].y2;

	Prog_GR[4].x1 = ProgBackup_GR[2].x1;
	Prog_GR[4].y1 = ProgBackup_GR[2].y1;
	Prog_GR[4].x2 = ProgBackup_GR[2].x2;
	Prog_GR[4].y2 = ProgBackup_GR[2].y2;

	window = UA_OpenRequesterWindow(onWindow,Prog_GR,STDCOLORS);
	if (!window)
	{
		UA_WarnUser(197);
		return(FALSE);
	}

	retVal = Monitor_DateProgramming(window,&work_node);

	if ( retVal )
	{
		for(this_node=(SNRPTR)ObjectRecord.firstObject;
				this_node->node.ln_Succ;
				this_node=(SNRPTR)this_node->node.ln_Succ)
		{
			if (	(this_node->miscFlags & OBJ_SELECTED ) &&
						this_node->nodeType < TALK_GOTO &&
						/* this_node->nodeType!=TALK_STARTSER && */
						this_node->nodeType!=TALK_STARTPAR )
			{
				this_node->duration 						= work_node.duration;
				this_node->dayBits							= work_node.dayBits;
				this_node->Start.HHMMSS.Days 		= work_node.Start.HHMMSS.Days;
				this_node->Start.HHMMSS.Minutes	= work_node.Start.HHMMSS.Minutes;
				this_node->Start.HHMMSS.Ticks		= work_node.Start.HHMMSS.Ticks;
				this_node->End.HHMMSS.Days			= work_node.End.HHMMSS.Days;
				this_node->End.HHMMSS.Minutes		= work_node.End.HHMMSS.Minutes;
				this_node->End.HHMMSS.Ticks			= work_node.End.HHMMSS.Ticks;
				this_node->startendMode					= work_node.startendMode;

				SetByteBit(&this_node->miscFlags, OBJ_NEEDS_REFRESH);
			}
		}
	}

	FixDurationAndDays(ObjectRecord.firstObject);

	UA_CloseRequesterWindow(window,STDCOLORS);

	return( retVal );
}

/******** Monitor_DateProgramming() ********/

BOOL Monitor_DateProgramming(	struct Window *window, 
															struct ScriptNodeRecord *selected_node)
{
BOOL loop=TRUE, retVal=FALSE;
TEXT tmp[256];
int i, ID, updown, a,b,c;
int val1, val2;
struct ScriptNodeRecord keptNode;

	CopyMem(selected_node, &keptNode, sizeof(struct ScriptNodeRecord));

	/**** fill gadgets with texts BEFORE they are rendered ****/

	// NEW NEW

	if (	selected_node->Start.HHMMSS.Days == -1 ||
				selected_node->startendMode == -1 ) /* object has no full prog yet */
	{
		selected_node->startendMode = 1;	/* CYCLICAL */
	}

	// NEW NEW

	createStartEndDay(selected_node->Start.HHMMSS.Days, tmp, 0);	/* 0=start */
	UA_SetStringGadgetToString(window, &Prog_GR[2], tmp);	/* start day */

	createStartEndDay(selected_node->End.HHMMSS.Days, tmp, 1);	/* 1=end */
	UA_SetStringGadgetToString(window, &Prog_GR[3], tmp);	/* end day */

	createStartEndTime(selected_node->Start.HHMMSS.Minutes, selected_node->Start.HHMMSS.Ticks, tmp);
	UA_SetStringGadgetToString(window, &Prog_GR[4], tmp);	/* start time */

	createStartEndTime(selected_node->End.HHMMSS.Minutes, selected_node->End.HHMMSS.Ticks, tmp);
	UA_SetStringGadgetToString(window, &Prog_GR[5], tmp);	/* end time */

	setStartEndCalender(window, selected_node->Start.HHMMSS.Days, selected_node->End.HHMMSS.Days);

	if ( selected_node->startendMode == 2 ) /* CONTINUOUS */
	{
		Prog_GR[3].y1 = ProgBackup_GR[2].y1;
		Prog_GR[3].y2 = ProgBackup_GR[2].y2;

		Prog_GR[4].y1 = ProgBackup_GR[1].y1;
		Prog_GR[4].y2 = ProgBackup_GR[1].y2;
	}
	else
	{
		Prog_GR[3].y1 = ProgBackup_GR[1].y1;
		Prog_GR[3].y2 = ProgBackup_GR[1].y2;

		Prog_GR[4].y1 = ProgBackup_GR[2].y1;
		Prog_GR[4].y2 = ProgBackup_GR[2].y2;
	}

	CheckEnteredDate(window, &Prog_GR[2], 7);
	CheckEnteredDate(window, &Prog_GR[3], 11);

	if (selected_node->duration == -1)
		selected_node->duration = DEFAULT_DELAY*10;

	secondsToDuration(selected_node->duration, tmp);
	UA_SetStringGadgetToString(window, &Prog_GR[24], tmp);	/* duration */

	/**** render gadgets ****/

	UA_ClearButton(window, &ProgBackup_GR[3], AREA_PEN);
	UA_DrawGadgetList(window, Prog_GR);

	/**** modify gadgets after they are rendered ****/

	if (selected_node->dayBits == -1)
	{
		for(i=15; i<22; i++)
			UA_InvertCheckButton(window, &Prog_GR[i]);	// NEW RADIO
		selected_node->dayBits = 127;
	}
	else
	{
		if ( selected_node->dayBits & 1 )
			UA_InvertCheckButton(window, &Prog_GR[15]);// NEW RADIO
		if ( selected_node->dayBits & 2 )
			UA_InvertCheckButton(window, &Prog_GR[16]);// NEW RADIO
		if ( selected_node->dayBits & 4 )
			UA_InvertCheckButton(window, &Prog_GR[17]);// NEW RADIO
		if ( selected_node->dayBits & 8 )
			UA_InvertCheckButton(window, &Prog_GR[18]);// NEW RADIO
		if ( selected_node->dayBits & 16 )
			UA_InvertCheckButton(window, &Prog_GR[19]);// NEW RADIO
		if ( selected_node->dayBits & 32 )
			UA_InvertCheckButton(window, &Prog_GR[20]);// NEW RADIO
		if ( selected_node->dayBits & 64 )
			UA_InvertCheckButton(window, &Prog_GR[21]);// NEW RADIO
	}

	if ( selected_node->startendMode == 2) /* CONTINUOUS */
		UA_InvertButton(window, &Prog_GR[6]);

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		switch(CED.Class)
		{
			case IDCMP_MOUSEBUTTONS:
				switch(CED.Code)
				{
					case SELECTDOWN:
						ID = UA_CheckGadgetList(window, Prog_GR, &CED);
						switch(ID)
						{
							case 2:
								UA_ProcessStringGadget(window, Prog_GR, &Prog_GR[ID], &CED);
								CheckEnteredDate(window, &Prog_GR[ID], 7);
								UA_DrawGadget(window, &Prog_GR[2]);
								UA_DrawGadget(window, &Prog_GR[3]);
								UA_DrawGadget(window, &Prog_GR[7]);
								UA_DrawGadget(window, &Prog_GR[8]);
								UA_DrawGadget(window, &Prog_GR[9]);
								UA_DrawGadget(window, &Prog_GR[10]);
								break;

							case 3:
								UA_ProcessStringGadget(window, Prog_GR, &Prog_GR[ID], &CED);
								CheckEnteredDate(window, &Prog_GR[ID], 11);
								UA_DrawGadget(window, &Prog_GR[2]);
								UA_DrawGadget(window, &Prog_GR[3]);
								UA_DrawGadget(window, &Prog_GR[11]);
								UA_DrawGadget(window, &Prog_GR[12]);
								UA_DrawGadget(window, &Prog_GR[13]);
								UA_DrawGadget(window, &Prog_GR[14]);
								break;

							case 4:
							case 5:
								UA_ProcessStringGadget(window, Prog_GR, &Prog_GR[ID], &CED);
								CheckEnteredTime(&Prog_GR[ID]);
								UA_DrawGadget(window, &Prog_GR[4]);
								UA_DrawGadget(window, &Prog_GR[5]);
								break;

							case 6:	/* date/time relation */
								UA_InvertButton(window, &Prog_GR[ID]);
								if ( selected_node->startendMode == 1 ) /* CYCLICAL */
								{
									/* make it continuous */

									selected_node->startendMode = 2;

									Prog_GR[3].y1 = ProgBackup_GR[2].y1;
									Prog_GR[3].y2 = ProgBackup_GR[2].y2;

									Prog_GR[4].y1 = ProgBackup_GR[1].y1;
									Prog_GR[4].y2 = ProgBackup_GR[1].y2;
								}
								else
								{
									/* make it cyclical */

									selected_node->startendMode = 1;

									Prog_GR[3].y1 = ProgBackup_GR[1].y1;
									Prog_GR[3].y2 = ProgBackup_GR[1].y2;

									Prog_GR[4].y1 = ProgBackup_GR[2].y1;
									Prog_GR[4].y2 = ProgBackup_GR[2].y2;
								}
//								if ( CPrefs.ScriptScreenDepth>2 )
//									UA_ClearButton(window, &ProgBackup_GR[3], 4);
//								else
								UA_ClearButton(window, &ProgBackup_GR[3], AREA_PEN);
								UA_DrawGadget(window, &Prog_GR[3]);
								UA_DrawGadget(window, &Prog_GR[4]);
								break;

							case 7:
								updown = UA_ProcessCycleGadget(window, &Prog_GR[ID], &CED);
								if (updown==-1)
									decreaseDayName(window, 7, 8);
								if (updown==1)
									increaseDayName(window, 7, 8);
								UA_DrawGadget(window, &Prog_GR[7]);
								UA_DrawGadget(window, &Prog_GR[8]);
								break;

							case 8:
								updown = UA_ProcessCycleGadget(window, &Prog_GR[ID], &CED);
								if (updown==-1)
									decreaseDay(window, 7, 8);
								if (updown==1)
									increaseDay(window, 7, 8);
								UA_DrawGadget(window, &Prog_GR[7]);
								UA_DrawGadget(window, &Prog_GR[8]);
								break;

							case 9:
							case 10:
								UA_ProcessCycleGadget(window, &Prog_GR[ID], &CED);
								changeMonth(window, 7, 8);
								UA_DrawGadget(window, &Prog_GR[7]);
								UA_DrawGadget(window, &Prog_GR[8]);
								break;	

							case 11:
								updown = UA_ProcessCycleGadget(window, &Prog_GR[ID], &CED);
								if (updown==-1)
									decreaseDayName(window, 11, 12);
								if (updown==1)
									increaseDayName(window, 11, 12);
								UA_DrawGadget(window, &Prog_GR[11]);
								UA_DrawGadget(window, &Prog_GR[12]);
								break;

							case 12:
								updown = UA_ProcessCycleGadget(window, &Prog_GR[ID], &CED);
								if (updown==-1)
									decreaseDay(window, 11, 12);
								if (updown==1)
									increaseDay(window, 11, 12);
								UA_DrawGadget(window, &Prog_GR[11]);
								UA_DrawGadget(window, &Prog_GR[12]);
								break;

							case 13:
							case 14:
								UA_ProcessCycleGadget(window, &Prog_GR[ID], &CED);
								changeMonth(window, 11, 12);
								UA_DrawGadget(window, &Prog_GR[11]);
								UA_DrawGadget(window, &Prog_GR[12]);
								break;

							case 15:
							case 16:
							case 17:
							case 18:
							case 19:
							case 20:
							case 21:
								if ( CED.Qualifier&IEQUALIFIER_LSHIFT || CED.Qualifier&IEQUALIFIER_RSHIFT )
								{
									for(i=15; i<=21; i++)
									{
										UA_InvertCheckButton(window, &Prog_GR[i]);
										InvertByteBit(&selected_node->dayBits, (1<<(i-15)) );
									}
								}
								else
								{
									UA_InvertCheckButton(window, &Prog_GR[ID]);
									InvertByteBit(&selected_node->dayBits, (1<<(ID-15)) );
								}
								break;

							case 22:	/* OK */
do_ok:
								UA_HiliteButton(window, &Prog_GR[22]);
								retVal=TRUE;
								loop=FALSE;
								break;

							case 23:	/* Cancel */
do_cancel:
								UA_HiliteButton(window, &Prog_GR[23]);
								retVal=FALSE;
								loop=FALSE;
								break;

							case 24: /* duration */
								UA_ProcessStringGadget(window, Prog_GR, &Prog_GR[ID], &CED);
								CheckEnteredTime(&Prog_GR[ID]);
								UA_DrawGadget(window, &Prog_GR[ID]);
								break;
						}
						break;

					default:
						break;
				}
				break;

			case IDCMP_RAWKEY:
				if (CED.Code==RAW_ESCAPE)	// cancel
					goto do_cancel;
				else if (CED.Code==RAW_RETURN)	// OK
					goto do_ok;
				break;
		}
	}

	if ( retVal )
	{
		UA_SetValToCycleGadgetVal(&Prog_GR[8], &a);
		a++;
		UA_SetValToCycleGadgetVal(&Prog_GR[9], &b);
		UA_SetValToCycleGadgetVal(&Prog_GR[10], &c);
		c+=1990;
		sprintf(tmp, "%d-%s-%d", (int)a, dateconv_monthNames[b], (int)c);
		dateStringtoDays(tmp, (int *)&selected_node->Start.HHMMSS.Days);

		UA_SetValToCycleGadgetVal(&Prog_GR[12], &a);
		a++;
		UA_SetValToCycleGadgetVal(&Prog_GR[13], &b);
		UA_SetValToCycleGadgetVal(&Prog_GR[14], &c);
		c+=1990;
		sprintf(tmp, "%d-%s-%d", (int)a, dateconv_monthNames[b], (int)c);
		dateStringtoDays(tmp, (int *)&selected_node->End.HHMMSS.Days);

		/* hh:mm:ss:t */
		/* 0123456789 */

		UA_SetStringToGadgetString(&Prog_GR[4], tmp);
		tmp[2] = ':';
		tmp[5] = ':';
		tmp[8] = ':';
		timeStringtoMinutesAndTicks(tmp, &val1, &val2);
		selected_node->Start.HHMMSS.Minutes = (LONG)val1;
		selected_node->Start.HHMMSS.Ticks = (LONG)val2;

		UA_SetStringToGadgetString(&Prog_GR[5], tmp);
		tmp[2] = ':';
		tmp[5] = ':';
		tmp[8] = ':';
		timeStringtoMinutesAndTicks(tmp, &val1, &val2);
		selected_node->End.HHMMSS.Minutes = (LONG)val1;
		selected_node->End.HHMMSS.Ticks = (LONG)val2;

		UA_SetStringToGadgetString(&Prog_GR[24], tmp);
		DurationStringToSeconds(tmp, &selected_node->duration);
	}

	UA_EnableButton(window, &Prog_GR[24]);	// duration

	if ( !retVal )
	{
		CopyMem(&keptNode, selected_node, sizeof(struct ScriptNodeRecord));
	}

	return(retVal);
}

/******** E O F ********/
