#include "nb:pre.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *scriptWindow;
extern struct Library *medialinkLibBase;
extern struct ObjectInfo ObjectRecord;

/**** gadgets ****/

extern struct GadgetRecord TC_Tweaker_GR[];
extern struct GadgetRecord TC_Tweaker1_GR[];
extern struct GadgetRecord TC_Tweaker2_GR[];

/**** functions ****/

/******** TimeCodeTweaker() ********/
/*
 * For TC but also for hmst parallel branches
 *
 */

void TimeCodeTweaker(void)
{
struct Window *window;
BOOL loop, retVal;
ULONG sj, ej, dj, oj, swap, ori_sj, ori_ej;
int ID, numSel, numTot;
struct ScriptNodeRecord *start, *end, *this_node;
TEXT tc[20];
BYTE hh,mm,ss,ff;
int h,m,s,f;
struct GadgetRecord *strGR;

	retVal = FALSE;
	loop   = TRUE;

	/**** first count number of (selected) nodes ****/

	numTot=0;
	numSel=0;
	for(	this_node=(struct ScriptNodeRecord *)ObjectRecord.firstObject;
				this_node->node.ln_Succ;
				this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ	)
	{
		numTot++;
		if ( this_node->miscFlags & OBJ_SELECTED )
			numSel++;
	}		

	if ( numTot < 3 || numSel < 3 )	// makes no sense
		return;

	numTot = numSel;	// I LOVE CLUTCHES

	/**** open a window ****/

	window = UA_OpenRequesterWindow(scriptWindow, TC_Tweaker_GR, STDCOLORS);
	if (!window)
	{
		UA_WarnUser(-1);
		return;
	}

	/**** double/halve gadgets ****/

	if ( window->WScreen->ViewPort.Modes & LACE )
	{
		UA_DoubleGadgetDimensions(TC_Tweaker1_GR);
		UA_DoubleGadgetDimensions(TC_Tweaker2_GR);
	}

	/**** render gadgets ****/

	UA_DrawGadgetList(window, TC_Tweaker_GR);

	if ( ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS )
		strGR = TC_Tweaker2_GR;
	else
		strGR = TC_Tweaker1_GR;
	UA_DrawGadgetList(window, strGR);

	/**** find first and last selected node ****/

	start=end=NULL;

	for(	this_node=(struct ScriptNodeRecord *)ObjectRecord.firstObject;
				this_node->node.ln_Succ;
				this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ	)
	{
		if ( this_node->miscFlags & OBJ_SELECTED )
		{
			if ( start==NULL )
				start = this_node;
			end = this_node;
		}
	}		

	if ( ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS )
	{
		secondsToDuration(start->Start.ParHMSTOffset, tc);
		UA_SetStringGadgetToString(window, &strGR[0], tc);

		secondsToDuration(end->End.ParHMSTOffset, tc);
		UA_SetStringGadgetToString(window, &strGR[1], tc);

		sj = start->Start.ParHMSTOffset;
		ej = end->End.ParHMSTOffset;

		dj = ej-sj;	
		dj /= numTot;
		secondsToDuration(dj,tc);
		UA_SetStringGadgetToString(window, &strGR[2], tc);

		UA_SetStringGadgetToString(window, &strGR[3], "00:00:00:0");
		oj = 0;
	}
	else	// timecode
	{
		sprintf(tc, "%02d:%02d:%02d:%02d",
						start->Start.TimeCode.HH,	start->Start.TimeCode.MM,
						start->Start.TimeCode.SS, start->Start.TimeCode.FF);
		UA_SetStringGadgetToString(window, &strGR[0], tc);

		sprintf(tc, "%02d:%02d:%02d:%02d",
						end->End.TimeCode.HH,	end->End.TimeCode.MM,
						end->End.TimeCode.SS, end->End.TimeCode.FF);
		UA_SetStringGadgetToString(window, &strGR[1], tc);

		sj = TC2Jiffies(start->Start.TimeCode.HH,	start->Start.TimeCode.MM,
										start->Start.TimeCode.SS, start->Start.TimeCode.FF);
		ej = TC2Jiffies(end->End.TimeCode.HH,	end->End.TimeCode.MM,
										end->End.TimeCode.SS, end->End.TimeCode.FF);

		dj = ej-sj;	
		dj /= numTot;
		Jiffies2TC(dj,&hh,&mm,&ss,&ff);
		sprintf(tc, "%02d:%02d:%02d:%02d", hh, mm, ss, ff);
		UA_SetStringGadgetToString(window, &strGR[2], tc);

		UA_SetStringGadgetToString(window, &strGR[3], "00:00:00:00");
		oj = 0;
	}

	ori_sj = sj;
	ori_ej = ej;

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, strGR, &CED);
			switch(ID)
			{
				case 0:
				case 1:
				case 2:
				case 3:
					UA_ProcessStringGadget(window, strGR, &strGR[ID], &CED);
					if ( ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS )
						CheckEnteredTime(&strGR[ID]);
					else
						CheckEnteredTimeCode(ObjectRecord.scriptSIR.timeCodeRate, &strGR[ID]);
					UA_SetStringToGadgetString(&strGR[ID], tc);
					UA_SetStringGadgetToString(window, &strGR[ID], tc);

					if ( ID==0 || ID==1 )	// start, end
					{
						UA_SetStringToGadgetString(&strGR[0], tc);
						if ( ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS )
							DurationStringToSeconds(tc, &sj);
						else
						{
							timeStringtoTimeCode(tc,&h,&m,&s,&f);
							sj = TC2Jiffies((BYTE)h,(BYTE)m,(BYTE)s,(BYTE)f);
						}

						UA_SetStringToGadgetString(&strGR[1], tc);
						if ( ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS )
							DurationStringToSeconds(tc, &ej);
						else
						{
							timeStringtoTimeCode(tc,&h,&m,&s,&f);
							ej = TC2Jiffies((BYTE)h,(BYTE)m,(BYTE)s,(BYTE)f);
						}
	
						if ( ej < sj )	// swap start and end if start smaller than end
						{
							swap = sj;
							sj = ej;
							ej = swap;

							if ( ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS )
							{
								secondsToDuration(sj,tc);
								UA_SetStringGadgetToString(window, &strGR[0], tc);

								secondsToDuration(ej,tc);
								UA_SetStringGadgetToString(window, &strGR[1], tc);
							}
							else
							{
								Jiffies2TC(sj,&hh,&mm,&ss,&ff);
								sprintf(tc, "%02d:%02d:%02d:%02d", hh, mm, ss, ff);
								UA_SetStringGadgetToString(window, &strGR[0], tc);

								Jiffies2TC(ej,&hh,&mm,&ss,&ff);
								sprintf(tc, "%02d:%02d:%02d:%02d", hh, mm, ss, ff);
								UA_SetStringGadgetToString(window, &strGR[1], tc);
							}
						}

						dj = ej-sj;	
						dj /= numTot;

						if ( ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS )
						{
							secondsToDuration(dj,tc);
							UA_SetStringGadgetToString(window, &strGR[2], tc);
						}
						else
						{
							Jiffies2TC(dj,&hh,&mm,&ss,&ff);
							sprintf(tc, "%02d:%02d:%02d:%02d", hh, mm, ss, ff);
							UA_SetStringGadgetToString(window, &strGR[2], tc);
						}

						ori_sj = sj;
						ori_ej = ej;
					}
					else if ( ID==2 )	// delta
					{
						if ( ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS )
						{
							UA_SetStringToGadgetString(&strGR[2], tc);
							DurationStringToSeconds(tc, &dj);

							UA_SetStringToGadgetString(&strGR[0], tc);
							DurationStringToSeconds(tc, &sj);

							sj += (dj * numTot);	// add delta to start, PUT it in end!

							secondsToDuration(sj,tc);
							UA_SetStringGadgetToString(window, &strGR[1], tc);
							ori_ej = sj;
						}
						else
						{
							UA_SetStringToGadgetString(&strGR[2], tc);
							timeStringtoTimeCode(tc,&h,&m,&s,&f);
							dj = TC2Jiffies((BYTE)h,(BYTE)m,(BYTE)s,(BYTE)f);

							UA_SetStringToGadgetString(&strGR[0], tc);
							timeStringtoTimeCode(tc,&h,&m,&s,&f);
							sj = TC2Jiffies((BYTE)h,(BYTE)m,(BYTE)s,(BYTE)f);

							sj += (dj * numTot);	// add delta to start, PUT it in end!

							Jiffies2TC(sj,&hh,&mm,&ss,&ff);
							sprintf(tc, "%02d:%02d:%02d:%02d", hh, mm, ss, ff);
							UA_SetStringGadgetToString(window, &strGR[1], tc);
							ori_ej = sj;
						}
					}
					else if ( ID==3 )	// offset
					{
						if ( ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS )
						{
							UA_SetStringToGadgetString(&strGR[3], tc);
							DurationStringToSeconds(tc, &oj);

							sj = ori_sj + oj;
							ej = ori_ej + oj;						

							secondsToDuration(sj,tc);
							UA_SetStringGadgetToString(window, &strGR[0], tc);

							secondsToDuration(ej,tc);
							UA_SetStringGadgetToString(window, &strGR[1], tc);
						}
						else
						{
							UA_SetStringToGadgetString(&strGR[3], tc);
							timeStringtoTimeCode(tc,&h,&m,&s,&f);
							oj = TC2Jiffies((BYTE)h,(BYTE)m,(BYTE)s,(BYTE)f);

							sj = ori_sj + oj;
							ej = ori_ej + oj;						

							Jiffies2TC(sj,&hh,&mm,&ss,&ff);
							sprintf(tc, "%02d:%02d:%02d:%02d", hh, mm, ss, ff);
							UA_SetStringGadgetToString(window, &strGR[0], tc);

							Jiffies2TC(ej,&hh,&mm,&ss,&ff);
							sprintf(tc, "%02d:%02d:%02d:%02d", hh, mm, ss, ff);
							UA_SetStringGadgetToString(window, &strGR[1], tc);
						}
					}
					break;
			}

			ID = UA_CheckGadgetList(window, TC_Tweaker_GR, &CED);
			switch(ID)
			{
				case 4:	// OK
do_ok:
					UA_HiliteButton(window, &TC_Tweaker_GR[4]);
					loop=FALSE;
					retVal=TRUE;
					break;

				case 5:	// Cancel
do_cancel:
					UA_HiliteButton(window, &TC_Tweaker_GR[5]);
					loop=FALSE;
					retVal=FALSE;
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

	/**** double/halve gadgets ****/

	if ( window->WScreen->ViewPort.Modes & LACE )
	{
		UA_HalveGadgetDimensions(TC_Tweaker1_GR);
		UA_HalveGadgetDimensions(TC_Tweaker2_GR);
	}

	UA_CloseRequesterWindow(window,STDCOLORS);

	if ( retVal )
	{
		if ( ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS )
		{
			/**** get start ****/

			UA_SetStringToGadgetString(&strGR[0], tc);
			DurationStringToSeconds(tc, &sj);

			/**** get delta ****/

			UA_SetStringToGadgetString(&strGR[2], tc);
			DurationStringToSeconds(tc, &dj);
		}
		else
		{
			/**** get start ****/

			UA_SetStringToGadgetString(&strGR[0], tc);
			timeStringtoTimeCode(tc,&h,&m,&s,&f);
			sj = TC2Jiffies((BYTE)h,(BYTE)m,(BYTE)s,(BYTE)f);

			/**** get delta ****/

			UA_SetStringToGadgetString(&strGR[2], tc);
			timeStringtoTimeCode(tc,&h,&m,&s,&f);
			dj = TC2Jiffies((BYTE)h,(BYTE)m,(BYTE)s,(BYTE)f);
		}

		for(	this_node=(struct ScriptNodeRecord *)ObjectRecord.firstObject;
					this_node->node.ln_Succ;
					this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ	)
		{
			if ( this_node->miscFlags & OBJ_SELECTED )
			{
				if ( ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS )
				{
					this_node->Start.ParHMSTOffset = sj;
					sj += dj;
					this_node->End.ParHMSTOffset = sj;
				}
				else
				{
					Jiffies2TC(sj,&hh,&mm,&ss,&ff);

					this_node->Start.TimeCode.HH = hh;
					this_node->Start.TimeCode.MM = mm;
					this_node->Start.TimeCode.SS = ss;
					this_node->Start.TimeCode.FF = ff;

					sj += dj;

					Jiffies2TC(sj,&hh,&mm,&ss,&ff);

					this_node->End.TimeCode.HH = hh;
					this_node->End.TimeCode.MM = mm;
					this_node->End.TimeCode.SS = ss;
					this_node->End.TimeCode.FF = ff;
				}
			}
		}

		// set end by hand, due to rounding-off errors

		/**** get end ****/

		if ( ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS )
		{
			UA_SetStringToGadgetString(&strGR[1], tc);
			DurationStringToSeconds(tc, &ej);
			end->End.ParHMSTOffset = ej;
		}
		else
		{
			UA_SetStringToGadgetString(&strGR[1], tc);
			timeStringtoTimeCode(tc,&h,&m,&s,&f);
			ej = TC2Jiffies((BYTE)h,(BYTE)m,(BYTE)s,(BYTE)f);

			Jiffies2TC(ej,&hh,&mm,&ss,&ff);

			end->End.TimeCode.HH = hh;
			end->End.TimeCode.MM = mm;
			end->End.TimeCode.SS = ss;
			end->End.TimeCode.FF = ff;
		}
	}
}

/******** TC2Jiffies() ********/

ULONG TC2Jiffies(BYTE HH, BYTE MM, BYTE SS, BYTE FF)
{
ULONG jiffies=0L, h, m, s;

	if ( CPrefs.PlayerPalNtsc==PAL_MODE )	// PAL
	{
		h = 60*60*25;	// 90000 ('frames per hour')
		m = 60*25;		// 1500 fpm ('frames per minute')
		s = 25;				// 25 fps
	}
	else	// NTSC
	{
		h = 60*60*30;	// 10800 fph ('frames per hour')
		m = 60*30;		// 1800 fpm ('frames per minute')
		s = 30;				// 30 fps
	}

	jiffies =	 HH * h;
	jiffies += (MM * m );
	jiffies += (SS * s );
	jiffies += FF;

	return(jiffies);
}

/******** Jiffies2TC() ********/

void Jiffies2TC(LONG jiffies, BYTE *HH, BYTE *MM, BYTE *SS, BYTE *FF)
{
ULONG h, m, s;

	if ( CPrefs.PlayerPalNtsc==PAL_MODE )	// PAL
	{
		h = 60*60*25;	// 90000 ('frames per hour')
		m = 60*25;		// 1500 fpm ('frames per minute')
		s = 25;				// 25 fps
	}
	else	// NTSC
	{
		h = 60*60*30;	// 10800 fph ('frames per hour')
		m = 60*30;		// 1800 fpm ('frames per minute')
		s = 30;				// 30 fps
	}

	*HH = jiffies / h;
	jiffies -= (*HH * h);

	*MM = jiffies / m;
	jiffies -= (*MM * m);

	*SS = jiffies / s;
	jiffies -= (*SS * s);

	*FF = jiffies;
}

/******** E O F ********/
