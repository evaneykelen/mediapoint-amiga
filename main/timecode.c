#include "nb:pre.h"

/**** externals ****/

extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct MsgPort *capsPort;
extern struct Library *medialinkLibBase;
extern struct ObjectInfo ObjectRecord;
extern UWORD chip gui_pattern[];
extern UBYTE **msgs;   

/**** gadgets ****/

extern struct GadgetRecord TimeCodeWdw_GR[];

/**** functions ****/

/******** Monitor_TimeCode() ********/

void Monitor_TimeCode(struct Window *window)
{
int i,ID,source, format, rate;
TEXT offset[32]; 
BOOL loop=TRUE, retval, convert, SendOut;
int a,b,c,d,cache;

	UA_DrawSpecialGadgetText(window, &TimeCodeWdw_GR[2], msgs[Msg_TimingSource-1], SPECIAL_TEXT_LEFT);
	UA_DrawSpecialGadgetText(window, &TimeCodeWdw_GR[3], msgs[Msg_TimingRate-1], SPECIAL_TEXT_LEFT);
	UA_DrawSpecialGadgetText(window, &TimeCodeWdw_GR[4], msgs[Msg_TimingType-1], SPECIAL_TEXT_LEFT);

	source	= ObjectRecord.scriptSIR.timeCodeSource;
	format	= ObjectRecord.scriptSIR.timeCodeFormat;
	rate		= ObjectRecord.scriptSIR.timeCodeRate;
	SendOut	= ObjectRecord.scriptSIR.timeCodeOut;

	sprintf(offset, "%02d:%02d:%02d:%02d",
					ObjectRecord.scriptSIR.Offset.TimeCode.HH,
					ObjectRecord.scriptSIR.Offset.TimeCode.MM,
					ObjectRecord.scriptSIR.Offset.TimeCode.SS,
					ObjectRecord.scriptSIR.Offset.TimeCode.FF);

	UA_SetStringGadgetToString(window, &TimeCodeWdw_GR[15], offset);

	/**** highlight radio buttons ****/

	if ( source == TIMESOURCE_INTERNAL)
		UA_InvertButton(window, &TimeCodeWdw_GR[5]);
	else	/* EXTERNAL */
		UA_InvertButton(window, &TimeCodeWdw_GR[6]);

	if ( format == TIMEFORMAT_HHMMSS ) 
		UA_InvertButton(window, &TimeCodeWdw_GR[7]);
	else if ( format == TIMEFORMAT_MIDI )
		UA_InvertButton(window, &TimeCodeWdw_GR[8]);
	else if ( format == TIMEFORMAT_MLTC )
		UA_InvertButton(window, &TimeCodeWdw_GR[9]);
	else if ( format == TIMEFORMAT_CUSTOM )
	{
		UA_InvertButton(window, &TimeCodeWdw_GR[11]);
		UA_SetStringGadgetToString(window, &TimeCodeWdw_GR[12], CPrefs.customTimeCode);
	}

	if ( rate == TIMERATE_25FPS)
		UA_InvertButton(window, &TimeCodeWdw_GR[13]);
	else if ( rate == TIMERATE_30FPS)
		UA_InvertButton(window, &TimeCodeWdw_GR[14]);

	if (SendOut)
		UA_InvertButton(window, &TimeCodeWdw_GR[16]);	/* Send time code out */

	/**** disable buttons ****/

	if ( source == TIMESOURCE_EXTERNAL )
		UA_DisableButton(window, &TimeCodeWdw_GR[7], gui_pattern);	/* HH:MM:SS:T */

	if ( format == TIMEFORMAT_HHMMSS ) 
	{
		UA_DisableButton(window, &TimeCodeWdw_GR[13], gui_pattern);	// 25 fps
		UA_DisableButton(window, &TimeCodeWdw_GR[14], gui_pattern);	// 30 fps
		UA_DisableButton(window, &TimeCodeWdw_GR[15], gui_pattern);	// offset string
	}

	if ( format == TIMEFORMAT_HHMMSS ) // || format == TIMEFORMAT_SMPTE )
		UA_DisableButton(window, &TimeCodeWdw_GR[16], gui_pattern);	// Send time code out

	a = CPrefs.scriptTiming;
	UA_SetCycleGadgetToVal(window, &TimeCodeWdw_GR[17], a);

	// START PRELOAD

	if ( CPrefs.objectPreLoading==10 || CPrefs.objectPreLoading==20 )
		a=1;	// no
	else
		a=0;	// yes
	UA_SetCycleGadgetToVal(window, &TimeCodeWdw_GR[18], a);
	if ( CPrefs.objectPreLoading==10 || CPrefs.objectPreLoading==30 )
	{
		cache=1;
		UA_InvertButton(window, &TimeCodeWdw_GR[22]);
	}
	else
		cache=0;
	
	// END PRELOAD

	a = CPrefs.playOptions;
	if ( a==3 )
		a=0;
	else
		a=1;
	UA_SetCycleGadgetToVal(window, &TimeCodeWdw_GR[19], a);

	UA_SetCycleGadgetToVal(window, &TimeCodeWdw_GR[10], CPrefs.bufferOptions);

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
						ID = UA_CheckGadgetList(window, TimeCodeWdw_GR, &CED);
						switch(ID)
						{
							case 5:
							case 6:
								UA_ClearButton(window, &TimeCodeWdw_GR[5], AREA_PEN);
								UA_ClearButton(window, &TimeCodeWdw_GR[6], AREA_PEN);
								if (ID==5)
								{
									source=TIMESOURCE_INTERNAL;
									UA_EnableButton(window, &TimeCodeWdw_GR[7]); /* HHMMSS */
									if (format==TIMEFORMAT_HHMMSS)
										UA_InvertButton(window, &TimeCodeWdw_GR[7]);
								}
								else
								{
									source=TIMESOURCE_EXTERNAL;
									if (format==TIMEFORMAT_HHMMSS)
										UA_InvertButton(window, &TimeCodeWdw_GR[7]);
									UA_DisableButton(window, &TimeCodeWdw_GR[7], gui_pattern); /* HHMMSS */
									if (format==TIMEFORMAT_HHMMSS)
									{
										format=TIMEFORMAT_MIDI;
										UA_InvertButton(window, &TimeCodeWdw_GR[8]);
										UA_EnableButton(window, &TimeCodeWdw_GR[13]);	// 25 fps
										UA_EnableButton(window, &TimeCodeWdw_GR[14]);	// 30 fps
										if (rate==TIMERATE_25FPS)
											UA_InvertButton(window, &TimeCodeWdw_GR[13]);
										else if (rate==TIMERATE_30FPS)
											UA_InvertButton(window, &TimeCodeWdw_GR[14]);
										UA_EnableButton(window, &TimeCodeWdw_GR[15]); /* offset string */
										UA_EnableButton(window, &TimeCodeWdw_GR[16]); /* Send time code out */
										if (SendOut)
											UA_InvertButton(window, &TimeCodeWdw_GR[16]);
									}
								}
								UA_InvertButton(window, &TimeCodeWdw_GR[ID]);
								break;								

							case 7:
							case 8:
							case 9:
							case 11:
								UA_EnableButton(window, &TimeCodeWdw_GR[16]);	/* Send time code out */
								UA_ClearButton(window, &TimeCodeWdw_GR[ 7], AREA_PEN);
								UA_ClearButton(window, &TimeCodeWdw_GR[ 8], AREA_PEN);
								UA_ClearButton(window, &TimeCodeWdw_GR[ 9], AREA_PEN);
								UA_ClearButton(window, &TimeCodeWdw_GR[11], AREA_PEN);
								if ( source == TIMESOURCE_EXTERNAL)
									UA_DisableButton(window, &TimeCodeWdw_GR[7], gui_pattern);	/* HHMMSS */
								if (ID==7)
								{
									format=TIMEFORMAT_HHMMSS;
									UA_DisableButton(window, &TimeCodeWdw_GR[13], gui_pattern);	// 25 fps
									UA_DisableButton(window, &TimeCodeWdw_GR[14], gui_pattern);	// 30 fps
									UA_DisableButton(window, &TimeCodeWdw_GR[15], gui_pattern);	// offset string
								}
								else
								{
									UA_EnableButton(window, &TimeCodeWdw_GR[13]);	// 25 fps
									UA_EnableButton(window, &TimeCodeWdw_GR[14]);	// 30 fps
									if (rate==TIMERATE_25FPS)
										UA_InvertButton(window, &TimeCodeWdw_GR[13]);
									else if (rate==TIMERATE_30FPS)
										UA_InvertButton(window, &TimeCodeWdw_GR[14]);
									UA_EnableButton(window, &TimeCodeWdw_GR[15]); /* offset string */
									UA_EnableButton(window, &TimeCodeWdw_GR[16]); /* Send time code out */
									if (SendOut)
										UA_InvertButton(window, &TimeCodeWdw_GR[16]);
								}
								if (ID==7)
								{
									format=TIMEFORMAT_HHMMSS;
									UA_DisableButton(window, &TimeCodeWdw_GR[16], gui_pattern);	// Send time code out
								}
								else if (ID==8)
									format=TIMEFORMAT_MIDI;
/*
								else if (ID==9)
								{
									format=TIMEFORMAT_SMPTE;
									UA_DisableButton(window, &TimeCodeWdw_GR[16], gui_pattern);	// Send time code out
								}
*/
								else if (ID==9)
									format=TIMEFORMAT_MLTC;
								else if (ID==11)
									format=TIMEFORMAT_CUSTOM;
								UA_InvertButton(window, &TimeCodeWdw_GR[ID]);
								break;								

							case 12:
								UA_ProcessStringGadget(window, TimeCodeWdw_GR, &TimeCodeWdw_GR[12], &CED);
								break;

							case 13:
							case 14:
								UA_ClearButton(window, &TimeCodeWdw_GR[13], AREA_PEN);
								UA_ClearButton(window, &TimeCodeWdw_GR[14], AREA_PEN);
								if (ID==13)
									rate=TIMERATE_25FPS;
								else if (ID==14)
									rate=TIMERATE_30FPS;
								UA_InvertButton(window, &TimeCodeWdw_GR[ID]);
								break;

							case 15:	/* offset string */
								UA_ProcessStringGadget(window, TimeCodeWdw_GR, &TimeCodeWdw_GR[ID], &CED);
								CheckEnteredTimeCode(rate, &TimeCodeWdw_GR[ID]);
								UA_SetStringToGadgetString(&TimeCodeWdw_GR[ID], offset);
								UA_SetStringGadgetToString(window, &TimeCodeWdw_GR[ID], offset);
								break;				

							case 16:
								UA_InvertButton(window, &TimeCodeWdw_GR[ID]);
								if (SendOut)
									SendOut=FALSE;
								else
									SendOut=TRUE;
								break;

							case 10:
							case 17:
							case 18:
							case 19:
								UA_ProcessCycleGadget(window, &TimeCodeWdw_GR[ID], &CED);
								break;

							case 20:	/* OK */
do_ok:
								UA_HiliteButton(window, &TimeCodeWdw_GR[20]);
								loop=FALSE;
								retval=TRUE;
								break;

							case 21:	/* Cancel */
do_cancel:
								UA_HiliteButton(window, &TimeCodeWdw_GR[21]);
								loop=FALSE;
								retval=FALSE;
								break;

							case 22:	// cache
								UA_InvertButton(window, &TimeCodeWdw_GR[22]);
								if ( cache )
									cache=0;
								else
									cache=1;
								break;
						}
						break;
				}
				break;

			case IDCMP_RAWKEY:
				if (CED.Code==RAW_ESCAPE)	/* cancel */
					goto do_cancel;
				else if (CED.Code==RAW_RETURN)	/* OK */
					goto do_ok;
				break;
		}
	}

	UA_EnableButton(window, &TimeCodeWdw_GR[7]);
	for(i=13; i<17; i++)
		UA_EnableButton(window, &TimeCodeWdw_GR[i]);

	convert=FALSE;
	if (retval)
	{
		/**** warn user for possible data loss ****/

		if (ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS &&
				format!=TIMEFORMAT_HHMMSS)
		{
			convert = UA_OpenGenericWindow(	window, TRUE, TRUE,
																			msgs[Msg_Discard-1], msgs[Msg_Cancel-1],
																			EXCLAMATION_ICON, msgs[Msg_SwitchStd2TC-1],
																			FALSE, NULL );
			if (convert)
				switchTimeCode(2);
			else
				retval=FALSE;
		}
		else if ( (ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_MIDI ||
							/*ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_SMPTE ||*/
							ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_MLTC ||
							ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_CUSTOM ) &&
							format==TIMEFORMAT_HHMMSS)
		{
			convert = UA_OpenGenericWindow(	window, TRUE, TRUE,
																			msgs[Msg_Discard-1], msgs[Msg_Cancel-1],
																			EXCLAMATION_ICON,	msgs[Msg_SwitchTC2Std-1],
																			FALSE,NULL );
			if (convert)
				switchTimeCode(1);
			else
				retval=FALSE;
		}

		/**** store entered data ****/

		if (retval)
		{
			ObjectRecord.scriptSIR.timeCodeSource = source;
			ObjectRecord.scriptSIR.timeCodeFormat = format;
			ObjectRecord.scriptSIR.timeCodeRate		= rate;
			ObjectRecord.scriptSIR.timeCodeOut		= SendOut;

			UA_SetStringToGadgetString(&TimeCodeWdw_GR[15], offset);

			timeStringtoTimeCode(offset, &a, &b, &c, &d);
			ObjectRecord.scriptSIR.Offset.TimeCode.HH = (BYTE)a;
			ObjectRecord.scriptSIR.Offset.TimeCode.MM = (BYTE)b;
			ObjectRecord.scriptSIR.Offset.TimeCode.SS = (BYTE)c;
			ObjectRecord.scriptSIR.Offset.TimeCode.FF = (BYTE)d;

			UA_SetValToCycleGadgetVal(&TimeCodeWdw_GR[17], &a);
			CPrefs.scriptTiming = a;

			// START PRELOAD

			UA_SetValToCycleGadgetVal(&TimeCodeWdw_GR[18], &a);
			if (a==0 && cache)				// yes and cache
				a=30;
			else if (a==0 && !cache)	// yes and no cache
				a=40;
			else if (a==1 && cache)		// no and cache
				a=10;
			else											// no and no cache
				a=20;
			CPrefs.objectPreLoading = a;

			// END PRELOAD

			UA_SetValToCycleGadgetVal(&TimeCodeWdw_GR[19], &a);
			if (a==0)
				a=3;	// automatic, formerly known as auto+manual
			else
				a=2;	// manual
			CPrefs.playOptions = a;

			UA_SetValToCycleGadgetVal(&TimeCodeWdw_GR[10], &a);
			CPrefs.bufferOptions = a;

			UA_SetStringToGadgetString(&TimeCodeWdw_GR[12],CPrefs.customTimeCode);

			/**** set numericalArgs[0] to defer or continue ****/
			UpdateDeferCont(&(ObjectRecord.scriptSIR));
		}
	}
}

/******** switchTimeCode() ********/
/*
 * mode==1	-->	goto HHMMSS mode
 * mode==2	-->	goto MIDI/SMPTE mode
 *
 */

void switchTimeCode(int mode)
{
int i;
struct List *this_list;
struct ScriptNodeRecord *this_node;

//	ChangeSpriteImage(SPRITE_BUSY);

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (ObjectRecord.scriptSIR.allLists[i] != NULL)
		{
			this_list = ObjectRecord.scriptSIR.allLists[i];

			if (this_list->lh_TailPred != (struct Node *)this_list)	/* non empty */
			{
				for(this_node=(struct ScriptNodeRecord *)this_list->lh_Head;
						this_node->node.ln_Succ;
						this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
				{
					if (mode==1)	/* goto HHMMSS mode */
					{
						this_node->duration							= DEFAULT_DELAY*10;	/* 10 seconds */
						this_node->dayBits							= 127;
						this_node->Start.HHMMSS.Days		= -1;
						this_node->Start.HHMMSS.Minutes	= -1;
						this_node->Start.HHMMSS.Ticks		= -1;
						this_node->End.HHMMSS.Days			= -1;
						this_node->End.HHMMSS.Minutes		= -1;
						this_node->End.HHMMSS.Ticks			= -1;
					}
					else	/* goto MIDI/SMPTE mode */
					{
						this_node->duration							= -1;
						this_node->dayBits							= -1;
						this_node->Start.TimeCode.HH		= 0;
						this_node->Start.TimeCode.MM		= 0;
						this_node->Start.TimeCode.SS		= 0;
						this_node->Start.TimeCode.FF		= 0;
						this_node->End.TimeCode.HH			= 0;
						this_node->End.TimeCode.MM			= 0;
						this_node->End.TimeCode.SS			= 0;
						this_node->End.TimeCode.FF			= 0;
					}
				}
			}
		}
	}						

//	ChangeSpriteImage(SPRITE_NORMAL);
}
		
/******** E O F ********/
