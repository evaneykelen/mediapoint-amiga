#include "nb:pre.h"

#define PAROFFSET 5

/**** externals ****/

extern struct CapsPrefs CPrefs;
extern struct Window *scriptWindow;
extern struct ObjectInfo ObjectRecord;
extern int objectXPosList[];
extern int objectYPosList[];
extern int standardXPosList[];
extern int standardYPosList[];
extern BOOL ToolEnabledList[];	// index is type number
extern struct RastPort xappRP;
extern struct RastPort xappRP_2;
extern struct BitMap gfxBitMap;
extern UWORD chip gui_pattern[];
extern UWORD *lookUpList_Eff;
extern struct BitMap effBM;
extern UBYTE **msgs;
extern ULONG effDoubled;

/**** gadgets ****/

extern struct GadgetRecord Script_GR[];

/**** functions ****/

/******** DrawObjectList() ********/
/*
 * if allParts is FALSE only the (highlighted) names are refreshed.
 * if force is TRUE, the list is refresed without looking to
 * the static, allParts will be taken into account.
 * if start==-1 then the last topEntry value will be used.
 */

void DrawObjectList(int start, BOOL allParts, BOOL force)
{
SNRPTR this_node;
int i, j, k, y, end, par, disp;
static int prevStart=-1001;	/* magic number */
int bits[] = { 1,2,4,8,16,32,64 };
struct DateStamp ds;
TEXT str[128];
int xx, yy;
int effbm_h, effbm_h2, effbm_y1, effbm_y2;
BOOL inRoot=FALSE;

	if ( effDoubled==1 )
	{
		effbm_h=16;
		effbm_h2=16; //18;
		effbm_y1=1;
		effbm_y2=0;
	}
	else
	{
		effbm_h=8;
		effbm_h2=9;
		effbm_y1=0;
		effbm_y2=3;
	}

	if ( ObjectRecord.scriptSIR.listType == -1 )
	{
		ObjectRecord.scriptSIR.listType = 
					FindParentType( &(ObjectRecord.scriptSIR), ObjectRecord.objList);
		if ( ObjectRecord.scriptSIR.listType == -1 )
		{
			inRoot=TRUE;
			ObjectRecord.scriptSIR.listType = TALK_STARTSER;	// root is serial
		}
	}

	if ( ObjectRecord.scriptSIR.listType == TALK_STARTPAR )
		par=PAROFFSET;
	else
		par=0;

	DateStamp(&ds);

	if (start==-1)
	{
		start=prevStart;
		if (start==-1001)
			start=0;
		prevStart=start;
	}
	else if (start==prevStart && !force)
		return;	/* don't draw because last draw was at same height */
	else
		prevStart=start;	/* don't moan, draw! */

	if (ObjectRecord.objList->lh_TailPred == (struct Node *)ObjectRecord.objList)
		return;

	this_node=(struct ScriptNodeRecord *)ObjectRecord.firstObject;

	if (start>0)
	{
		for(i=0; i<start; i++)
			this_node = (struct ScriptNodeRecord *)(this_node->node.ln_Succ);
	}

	y = Script_GR[0].y1+2;
	if ( CPrefs.ScriptScreenModes & LACE )
		y += 1;

	SetDrMd(scriptWindow->RPort, JAM1);
	SetBPen(scriptWindow->RPort, AREA_PEN);

	if ( ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS )
	{
		if (CPrefs.showDays)
			end=283;
		else
			end=309;
	}
	else
		end=299;

	for(i=start, j=0; i<start+ObjectRecord.maxObjects; i++, j++)
	{
		SetAPen(scriptWindow->RPort, AREA_PEN);
		if ( this_node->miscFlags & OBJ_BEINGDRAGGED )
			RectFill(scriptWindow->RPort, Script_GR[0].x1+2, y+20*j-1, 415, y+20*j+17);
		else
		{
			if (allParts)
			{
				/**** copy object icon ****/

				if (this_node->nodeType == TALK_USERAPPLIC)
					ClipBlit(	&xappRP_2,
										(LONG)objectXPosList[this_node->numericalArgs[MAX_PARSER_ARGS-1]],
										(LONG)objectYPosList[this_node->numericalArgs[MAX_PARSER_ARGS-1]],
										scriptWindow->RPort,
										9, y+20*j, (LONG)ICONWIDTH, (LONG)ICONHEIGHT, 0xc0);			
				else
					ClipBlit(	&xappRP_2,
										(LONG)standardXPosList[this_node->nodeType],
										(LONG)standardYPosList[this_node->nodeType],
										scriptWindow->RPort,
										9, y+20*j, (LONG)ICONWIDTH, (LONG)ICONHEIGHT, 0xc0);

				if ( this_node->nodeType < TALK_GOTO )	//&& this_node->nodeType!=TALK_STARTSER )
				{
					if ( ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS )
					{
						if (ObjectRecord.scriptSIR.listType==TALK_STARTPAR)
							printDuration_4(this_node, y+20*j);
						else
						{
							if (CPrefs.showDays)
							{
								RectFill(scriptWindow->RPort, 307,y+20*j, 307+GFX_PROG_W-3,y+20*j+GFX_PROG_H-3);

								if ( this_node->nodeType==TALK_STARTSER )
									BltBitMapRastPort(&gfxBitMap,GFX_PROG_X+1,GFX_PROG_Y+1,
																		scriptWindow->RPort,307,y+20*j,
																		GFX_PROG_W-2,10, 0xc0);
								else if ( this_node->nodeType!=TALK_STARTPAR )
									BltBitMapRastPort(&gfxBitMap,GFX_PROG_X+1,GFX_PROG_Y+1,
																		scriptWindow->RPort,307,y+20*j,
																		GFX_PROG_W-2,GFX_PROG_H-2, 0xc0);
/*
								else
									RectFill(scriptWindow->RPort, 307,y+20*j, 307+GFX_PROG_W-3,y+20*j+GFX_PROG_H-3);
*/

								printDuration_1(this_node, y+20*j);
							}
							else if ( this_node->nodeType == TALK_STARTSER )
								RectFill(scriptWindow->RPort, end, y+20*j-1, 415, y+20*j+17);
							else
								printDuration_2(this_node, y+20*j);
						}
					}
					else	// TIMECODE_MIDI or TIMECODE_SMPTE
						printDuration_3(this_node, y+20*j);
				}
				else
					RectFill(scriptWindow->RPort, end, y+20*j-1, 415, y+20*j+17);

				if (	CPrefs.showDays && this_node->dayBits != -1 &&
							this_node->dayBits != 127 &&
							ObjectRecord.scriptSIR.timeCodeFormat == TIMEFORMAT_HHMMSS)
				{
					SetAfPt(scriptWindow->RPort, gui_pattern, 1);
					for(k=0; k<7; k++)
					{
						if ( !(this_node->dayBits & bits[k]) )
							RectFill(	scriptWindow->RPort, 307+16*k, y+20*j, 307+16*k+12, y+20*j+6);
					}
					SetAfPt(scriptWindow->RPort, NULL, 0);
				}

				if (	CPrefs.showDays &&
							ObjectRecord.scriptSIR.timeCodeFormat == TIMEFORMAT_HHMMSS &&
							this_node->Start.HHMMSS.Days!=-1 &&
							ObjectRecord.scriptSIR.listType != TALK_STARTPAR )	// has a date program
				{
					disp = DoObjectDateCheckV2(&ds,this_node);
					if ( disp==1 )			// past
						BltBitMapRastPort(&gfxBitMap,GFX_PAST_X,GFX_PAST_Y,
															scriptWindow->RPort,307,y+20*j+10,13,7, 0xc0);
					else if ( disp==2 )	// future
						BltBitMapRastPort(&gfxBitMap,GFX_FUTU_X,GFX_FUTU_Y,
															scriptWindow->RPort,307,y+20*j+10,13,7, 0xc0);
					else if ( disp==3 )	// day not selected
						BltBitMapRastPort(&gfxBitMap,GFX_DONT_X,GFX_DONT_Y,
															scriptWindow->RPort,307,y+20*j+10,13,7, 0xc0);
				}
			} // END OF if(allParts) 

			/**** clear object name area ****/

			if ( !allParts && !(this_node->miscFlags & OBJ_NEEDS_REFRESH) )
				;
			else
			{
				UnSetByteBit(&this_node->miscFlags, OBJ_NEEDS_REFRESH);
	
				/*** clear objectName area ****/
	
				RectFill(scriptWindow->RPort, ICONWIDTH+12, y+20*j, end-1+23, y+16+20*j);

				/**** render objectName ****/

				if (this_node->miscFlags & OBJ_SELECTED)
				{
					SetAPen(scriptWindow->RPort, HI_PEN);
					SetDrMd(scriptWindow->RPort, JAM1);
					RectFill(scriptWindow->RPort, ICONWIDTH+12, y+20*j, end-4, y+16+20*j);
				}

				Move(scriptWindow->RPort, ICONWIDTH+12, y+12+20*j -par);
				GetObjectName(this_node, str, 15, end-(ICONWIDTH+12)-16);
				if (strcmpi(str,msgs[Msg_Untitled-1])==0 || strcmpi(str,msgs[Msg_Comment-1])==0)
				{
					if (this_node->miscFlags & OBJ_SELECTED)
						SetAPen(scriptWindow->RPort, BGND_PEN);
					else
						SetAPen(scriptWindow->RPort, HI_PEN);
				}
				else
						SetAPen(scriptWindow->RPort, LO_PEN);
				Text(scriptWindow->RPort, str, strlen(str));
			}

			/**** if in root and if root contains already a global event then ****/
			/**** disable tool. ****/

			if (this_node->nodeType == TALK_GLOBALEVENT)
			{
				DisableTool(TALK_GLOBALEVENT);
				ShowToolIcons(scriptWindow, -2);
			}

			if (this_node->nodeType == TALK_INPUTSETTINGS)
			{
				DisableTool(TALK_INPUTSETTINGS);
				ShowToolIcons(scriptWindow, -2);
			}

			if (this_node->nodeType == TALK_TIMECODE)
			{
				DisableTool(TALK_TIMECODE);
				ShowToolIcons(scriptWindow, -2);
			}

			if ( inRoot && this_node->nodeType == TALK_VARS)
			{
				DisableTool(TALK_VARS);
				ShowToolIcons(scriptWindow, -2);
			}

			if ( this_node->nodeType == TALK_ANIM || this_node->nodeType == TALK_PAGE )
			{
				xx = ( (lookUpList_Eff[this_node->numericalArgs[2]]) % 20) * 32;
				yy = ( (lookUpList_Eff[this_node->numericalArgs[2]]) / 20) * effbm_h;
				RenderEffectIcon(scriptWindow->RPort,xx,yy+effbm_y1,end,y+20*j+effbm_y2,21,effbm_h2);
/*
				BltBitMapRastPort(&effBM, xx,yy+effbm_y1,
													scriptWindow->RPort, end, y+20*j+effbm_y2, 21, effbm_h2, 0xc0);
*/
			}
		}

		/**** get next object ****/

		this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ;

		if (this_node->node.ln_Succ==NULL)
			break;
	}

	if ( ObjectRecord.scriptSIR.listType == TALK_STARTPAR )
		DrawParBars(start);
}

/******** printDuration_1() ********/
/*
 * prints duration of HH:MM:SS:T with days
 *
 */

void printDuration_1(struct ScriptNodeRecord *this_node, int y)
{
LONG seconds=0;
int h,m,s,t;
TEXT str[30];

	if (this_node->nodeType==TALK_STARTSER)
		return;

	if (this_node->duration==-1)
		this_node->duration	= DEFAULT_DELAY*10;	/* 10 seconds */

	seconds = this_node->duration;
	h = seconds/36000;
	seconds -= (h*36000);
	m = seconds/600;
	seconds -= (m*600);
	s = seconds/10;
	seconds -= (s*10);
	t = seconds;
	seconds = this_node->duration;

	sprintf(str, "%02d:%02d:%02d:%01d", h,m,s,t);

	SetAPen(scriptWindow->RPort, AREA_PEN);
	RectFill(scriptWindow->RPort, 333, y+10, 415, y+16);
	SetAPen(scriptWindow->RPort, LO_PEN);

	Move(scriptWindow->RPort, 331, y+16);
	Text(scriptWindow->RPort, str, 10);

	SetAPen(scriptWindow->RPort, AREA_PEN);
}

/******** printDuration_2() ********/
/*
 * prints duration of HH:MM:SS:T without days
 *
 */

void printDuration_2(struct ScriptNodeRecord *this_node, int y)
{
LONG seconds=0;
int h,m,s,t;
TEXT str[30];

	if (this_node->nodeType==TALK_STARTSER)
		return;

	if (this_node->duration==-1)
		this_node->duration	= DEFAULT_DELAY*10;	/* 10 seconds */

	seconds = this_node->duration;
	h = seconds/36000;
	seconds -= (h*36000);
	m = seconds/600;
	seconds -= (m*600);
	s = seconds/10;
	seconds -= (s*10);
	t = seconds;
	seconds = this_node->duration;

	sprintf(str, "%02d:%02d:%02d:%01d", h,m,s,t);

	SetAPen(scriptWindow->RPort, AREA_PEN);
	RectFill(scriptWindow->RPort, 333, y+4, 415, y+10);
	SetAPen(scriptWindow->RPort, LO_PEN);

	Move(scriptWindow->RPort, 331, y+10);
	Text(scriptWindow->RPort, str, 10);

	SetAPen(scriptWindow->RPort, AREA_PEN);
}

/******** printDuration_3() ********/
/*
 * prints duration of HH:MM:SS:FF
 *
 */

void printDuration_3(struct ScriptNodeRecord *this_node, int y)
{
TEXT str[30];

	if (this_node->nodeType==TALK_STARTSER)
		return;

	if (this_node->Start.TimeCode.HH==-1)
	{
		this_node->Start.TimeCode.HH=0;
		this_node->Start.TimeCode.MM=0;
		this_node->Start.TimeCode.SS=0;
		this_node->Start.TimeCode.FF=0;
	}

	if (this_node->End.TimeCode.HH==-1)
	{
		this_node->End.TimeCode.HH=0;
		this_node->End.TimeCode.MM=0;
		this_node->End.TimeCode.SS=0;
		this_node->End.TimeCode.FF=0;
	}

	sprintf(str, "%02d:%02d:%02d:%02d",
					this_node->Start.TimeCode.HH,
					this_node->Start.TimeCode.MM,
					this_node->Start.TimeCode.SS,
					this_node->Start.TimeCode.FF);																			

	SetAPen(scriptWindow->RPort, AREA_PEN);
	RectFill(scriptWindow->RPort, 323, y, 415, y+6);
	RectFill(scriptWindow->RPort, 323, y+10, 415, y+16);
	SetAPen(scriptWindow->RPort, LO_PEN);

	Move(scriptWindow->RPort, 321, y+6);
	Text(scriptWindow->RPort, str, 11);

	sprintf(str, "%02d:%02d:%02d:%02d",
					this_node->End.TimeCode.HH,
					this_node->End.TimeCode.MM,
					this_node->End.TimeCode.SS,
					this_node->End.TimeCode.FF);																			

	Move(scriptWindow->RPort, 321, y+16);
	Text(scriptWindow->RPort, str, 11);

	SetAPen(scriptWindow->RPort, AREA_PEN);
}

/******** printDuration_4() ********/
/*
 * prints duration of HH:MM:SS:T of a parallel branch
 *
 */

void printDuration_4(struct ScriptNodeRecord *this_node, int y)
{
LONG seconds=0;
int h,m,s,t;
TEXT str[30];

	if (this_node->Start.ParHMSTOffset==-1)
	{
		this_node->Start.ParHMSTOffset = 10;
		this_node->End.ParHMSTOffset = 10;
	}
	
	SetAPen(scriptWindow->RPort, AREA_PEN);
	RectFill(scriptWindow->RPort, 333, y,    415, y+6);
	RectFill(scriptWindow->RPort, 333, y+10, 415, y+16);
	SetAPen(scriptWindow->RPort, LO_PEN);

	seconds = this_node->Start.ParHMSTOffset;
	h = seconds/36000;
	seconds -= (h*36000);
	m = seconds/600;
	seconds -= (m*600);
	s = seconds/10;
	seconds -= (s*10);
	t = seconds;
	seconds = this_node->duration;

	sprintf(str, "%02d:%02d:%02d:%01d", h,m,s,t);

	Move(scriptWindow->RPort, 331, y+6);
	Text(scriptWindow->RPort, str, 10);

	seconds = this_node->End.ParHMSTOffset;
	h = seconds/36000;
	seconds -= (h*36000);
	m = seconds/600;
	seconds -= (m*600);
	s = seconds/10;
	seconds -= (s*10);
	t = seconds;
	seconds = this_node->duration;

	sprintf(str, "%02d:%02d:%02d:%01d", h,m,s,t);

	Move(scriptWindow->RPort, 331, y+16);
	Text(scriptWindow->RPort, str, 10);

	SetAPen(scriptWindow->RPort, AREA_PEN);
}

/******** DrawTimeCodes() ********/

void DrawTimeCodes(int start)
{
struct ScriptNodeRecord *this_node;
int i, j, k, y, end, disp;
int bits[] = { 1,2,4,8,16,32,64 };
struct DateStamp ds;

	DateStamp(&ds);

	if (ObjectRecord.objList->lh_TailPred == (struct Node *)ObjectRecord.objList)
		return;

	this_node=(struct ScriptNodeRecord *)ObjectRecord.firstObject;

	if (start>0)
	{
		for(i=0; i<start; i++)
			this_node = (struct ScriptNodeRecord *)(this_node->node.ln_Succ);
	}

	y = Script_GR[0].y1+2;
	if ( CPrefs.ScriptScreenModes & LACE )
		y += 1;

	SetDrMd(scriptWindow->RPort, JAM1);
	SetAPen(scriptWindow->RPort, AREA_PEN);
	SetBPen(scriptWindow->RPort, 0);

	if ( ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS )
	{
		if (CPrefs.showDays)
			end=306;
		else
			end=332;
	}
	else
		end=322;

	for(i=start, j=0; i<start+ObjectRecord.maxObjects; i++, j++)
	{
		if ( this_node->nodeType < TALK_GOTO )	//&& this_node->nodeType!=TALK_STARTSER )
		{
			if ( this_node->miscFlags & OBJ_NEEDS_REFRESH )
			{
				if ( ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS )
				{
					if (ObjectRecord.scriptSIR.listType==TALK_STARTPAR)
						printDuration_4(this_node, y+20*j);
					else
					{
						if (CPrefs.showDays)
						{
							RectFill(scriptWindow->RPort, 307,y+20*j, 307+GFX_PROG_W-3,y+20*j+GFX_PROG_H-3);

							if ( this_node->nodeType==TALK_STARTSER )
								BltBitMapRastPort(&gfxBitMap,GFX_PROG_X+1,GFX_PROG_Y+1,
																	scriptWindow->RPort,307,y+20*j,
																	GFX_PROG_W-2,10, 0xc0);
							else if ( this_node->nodeType!=TALK_STARTPAR )
								BltBitMapRastPort(&gfxBitMap,GFX_PROG_X+1,GFX_PROG_Y+1,
																	scriptWindow->RPort,307,y+20*j,
																	GFX_PROG_W-2,GFX_PROG_H-2, 0xc0);
//							else
//								RectFill(scriptWindow->RPort, 307,y+20*j, 307+GFX_PROG_W-3,y+20*j+GFX_PROG_H-3);

							printDuration_1(this_node, y+20*j);
						}
						else if ( this_node->nodeType == TALK_STARTSER )
								RectFill(scriptWindow->RPort, end, y+20*j-1, 415, y+20*j+17);
						else
							printDuration_2(this_node, y+20*j);
					}
				}
				else	/* TIMECODE_MIDI or TIMECODE_SMPTE */
					printDuration_3(this_node, y+20*j);
			}
		}
		else
			RectFill(scriptWindow->RPort, end, y+20*j-1, 415, y+20*j+17);

		if ( this_node->miscFlags & OBJ_NEEDS_REFRESH )
		{
			if (	CPrefs.showDays && this_node->dayBits != -1 &&
						this_node->dayBits != 127 &&
						ObjectRecord.scriptSIR.timeCodeFormat == TIMEFORMAT_HHMMSS)
			{
				SetAfPt(scriptWindow->RPort, gui_pattern, 1);
				for(k=0; k<7; k++)
				{
					if ( !(this_node->dayBits & bits[k]) )
						RectFill(	scriptWindow->RPort, 307+16*k, y+20*j, 307+16*k+12, y+20*j+6);
				}
				SetAfPt(scriptWindow->RPort, NULL, 0);
			}

			if (	CPrefs.showDays &&
						ObjectRecord.scriptSIR.timeCodeFormat == TIMEFORMAT_HHMMSS &&
						this_node->Start.HHMMSS.Days!=-1 &&
						ObjectRecord.scriptSIR.listType != TALK_STARTPAR )	// has a date program
			{
				disp = DoObjectDateCheckV2(&ds,this_node);
				if ( disp==1 )			// past
					BltBitMapRastPort(&gfxBitMap,GFX_PAST_X,GFX_PAST_Y,
														scriptWindow->RPort,307,y+20*j+10,13,7, 0xc0);
				else if ( disp==2 )	// future
					BltBitMapRastPort(&gfxBitMap,GFX_FUTU_X,GFX_FUTU_Y,
														scriptWindow->RPort,307,y+20*j+10,13,7, 0xc0);
				else if ( disp==3 )	// day not selected
					BltBitMapRastPort(&gfxBitMap,GFX_DONT_X,GFX_DONT_Y,
														scriptWindow->RPort,307,y+20*j+10,13,7, 0xc0);
			}
		}

		UnSetByteBit(&this_node->miscFlags, OBJ_NEEDS_REFRESH);

		/**** get next object ****/

		this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ;

		if (this_node->node.ln_Succ==NULL)
			break;
	}

	if ( ObjectRecord.scriptSIR.listType == TALK_STARTPAR )
		DrawParBars(start);
}

/******** DrawTransitionBlocks() ********/

void DrawTransitionBlocks(int start)
{
struct ScriptNodeRecord *this_node;
int i, j, xx, yy, y, end;
int effbm_h, effbm_h2, effbm_y1, effbm_y2;

	if ( effDoubled==1 )
	{
		effbm_h=16;
		effbm_h2=16;//18;
		effbm_y1=1;
		effbm_y2=0;
	}
	else
	{
		effbm_h=8;
		effbm_h2=9;
		effbm_y1=0;
		effbm_y2=3;
	}

	if (ObjectRecord.objList->lh_TailPred == (struct Node *)ObjectRecord.objList)
		return;

	this_node=(struct ScriptNodeRecord *)ObjectRecord.firstObject;

	if (start>0)
	{
		for(i=0; i<start; i++)
			this_node = (struct ScriptNodeRecord *)(this_node->node.ln_Succ);
	}

	if ( ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS )
	{
		if (CPrefs.showDays)
			end=306;
		else
			end=332;
	}
	else
		end=322;

	end -= 23;

	y = Script_GR[0].y1+2;
	if ( CPrefs.ScriptScreenModes & LACE )
		y += 1;

	for(i=start, j=0; i<start+ObjectRecord.maxObjects; i++, j++)
	{
		if ( this_node->nodeType == TALK_ANIM || this_node->nodeType == TALK_PAGE )
		{
			xx = ( (lookUpList_Eff[this_node->numericalArgs[2]]) % 20) * 32;
			yy = ( (lookUpList_Eff[this_node->numericalArgs[2]]) / 20) * effbm_h;
			RenderEffectIcon(scriptWindow->RPort,xx,yy+effbm_y1,end,y+20*j+effbm_y2,21,effbm_h2);
/*
			BltBitMapRastPort(&effBM, xx,yy+effbm_y1,
												scriptWindow->RPort, end, y+20*j+effbm_y2,	21, effbm_h2, 0xc0);
*/
		}

		/**** get next object ****/

		this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ;

		if (this_node->node.ln_Succ==NULL)
			break;
	}
}

/******** DrawParBars() ********/

void DrawParBars(int start)
{
int i, j, delta, y, w;
struct ScriptNodeRecord *this_node;
float pps,x1,x2;
ULONG start_jiffies, end_jiffies;

	delta=0;
	for(	this_node=ObjectRecord.firstObject; this_node->node.ln_Succ;
				this_node=(SNRPTR)this_node->node.ln_Succ	)
	{
		if ( ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS )
		{
			if ( this_node->End.ParHMSTOffset > delta )
				delta = this_node->End.ParHMSTOffset;
		}
		else
		{
			end_jiffies = TC2Jiffies(	this_node->End.TimeCode.HH,
																this_node->End.TimeCode.MM,
																this_node->End.TimeCode.SS,
																this_node->End.TimeCode.FF );
			if ( end_jiffies > delta )
				delta = end_jiffies;
		}
	}

	if ( delta==0 )
		return;

	if (ObjectRecord.objList->lh_TailPred == (struct Node *)ObjectRecord.objList)
		return;

	this_node=(struct ScriptNodeRecord *)ObjectRecord.firstObject;
	for(i=0; i<start; i++)
		this_node = (struct ScriptNodeRecord *)(this_node->node.ln_Succ);

	y = Script_GR[0].y1+2;
	if ( CPrefs.ScriptScreenModes & LACE )
		y += 1;

	if ( ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS )
	{
		if (CPrefs.showDays)
			w=231;
		else
			w=257;
	}
	else
		w=247;

	pps = (float)w / (float)delta;

	SetDrMd(scriptWindow->RPort,JAM1);

	for(i=start, j=0; i<start+ObjectRecord.maxObjects; i++, j++)
	{
		/**** draw white bar with black outlining ****/

		SetAPen(scriptWindow->RPort,HI_PEN);
		RectFill(	scriptWindow->RPort,
							ICONWIDTH+12, y+20*j +12, ICONWIDTH+12+w, y+20*j +15 );
		SetAPen(scriptWindow->RPort,LO_PEN);
		Move(scriptWindow->RPort, ICONWIDTH+12, 	y+20*j +11);
		Draw(scriptWindow->RPort, ICONWIDTH+12+w, y+20*j +11);
		Move(scriptWindow->RPort, ICONWIDTH+12, 	y+20*j +16);
		Draw(scriptWindow->RPort, ICONWIDTH+12+w, y+20*j +16);

		/**** draw black bar ****/

		if ( ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS )
		{
			if ( this_node->Start.ParHMSTOffset < this_node->End.ParHMSTOffset )
			{
				x1 = (float)this_node->Start.ParHMSTOffset * pps;
				x2 = (float)this_node->End.ParHMSTOffset * pps;
				if ( (x2-x1) > 3 )
				{
					SetAPen(scriptWindow->RPort,LO_PEN);
					RectFill(	scriptWindow->RPort,
										ICONWIDTH+12+(int)x1, y+20*j +12, ICONWIDTH+12+(int)x2, y+20*j +15 );
				}
			}
		}
		else
		{
			start_jiffies	= TC2Jiffies(	this_node->Start.TimeCode.HH, this_node->Start.TimeCode.MM,
																	this_node->Start.TimeCode.SS, this_node->Start.TimeCode.FF );
			end_jiffies		= TC2Jiffies(	this_node->End.TimeCode.HH, this_node->End.TimeCode.MM,
																	this_node->End.TimeCode.SS, this_node->End.TimeCode.FF );
			if ( start_jiffies < end_jiffies )
			{
				x1 = (float)start_jiffies * pps;
				x2 = (float)end_jiffies * pps;
				if ( ICONWIDTH+12+(int)x2 > (ICONWIDTH+12+w-2) )
					x2 = w;
				if ( (x2-x1) > 3 )
				{
					SetAPen(scriptWindow->RPort,LO_PEN);
					RectFill(	scriptWindow->RPort,
										ICONWIDTH+12+(int)x1, y+20*j +12, ICONWIDTH+12+(int)x2, y+20*j +15 );
				}
			}
		}
		
		this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ;
		if (this_node->node.ln_Succ==NULL)
			break;
	}		
}

/******** DoObjectDateCheckV2() ********/
/*
 * ===============================================================================
 * IF THIS FUNCTION IS ADAPTED ALSO MODIFY COUNTERPART IN PH:PROCCONT.C !!!!!!!!!!
 * ===============================================================================
 *
 * return 0 if object will be shown
 * returns 1 if object is in past
 * returns 2 if object is in future
 * returns 3 if object is on this day but deselected OR ERROR
 *
 */

int DoObjectDateCheckV2(struct DateStamp *CurDate, struct ScriptNodeRecord *this_node)
{
LONG CurMinutesSince1978, ObjMinutesSince1978Start,ObjMinutesSince1978End;

	if ( this_node->dayBits & (1<<(CurDate->ds_Days % 7)) )
	{
		if ( this_node->startendMode == -1 )
			this_node->startendMode = ARGUMENT_CYCLICAL;
		
		if (this_node->startendMode == ARGUMENT_CYCLICAL)
		{
			if ((this_node->Start.HHMMSS.Minutes == 0) && (this_node->End.HHMMSS.Minutes == 0))
			{
				// Wrong date
				if ( this_node->Start.HHMMSS.Days > this_node->End.HHMMSS.Days )	
				{
					return(3);
				}

				// Is current day smaller than starting day?
				if ( CurDate->ds_Days < this_node->Start.HHMMSS.Days )	
				{
					return(2);	// obj in future
				}

				// Is current day larger than ending day?
				if ( CurDate->ds_Days > this_node->End.HHMMSS.Days )	
				{
					return(1);	// obj in past
				}
			}
			else
			{
				// Wrong date
				if ( this_node->Start.HHMMSS.Days > this_node->End.HHMMSS.Days )	
				{
					return(3);
				}

				// Wrong time
				if ( this_node->Start.HHMMSS.Minutes > this_node->End.HHMMSS.Minutes )	
				{
					return(3);
				}

				// Is current day smaller than starting day?
				if ( CurDate->ds_Days < this_node->Start.HHMMSS.Days )	
				{
					return(2);	// obj in future
				}

				// Is current day larger than ending day?
				if ( CurDate->ds_Days > this_node->End.HHMMSS.Days )	
				{
					return(1);	// obj in past
				}

				// Is current day OK but current time smaller than starting time?
				if ( CurDate->ds_Minute < this_node->Start.HHMMSS.Minutes )
				{
					return(2);	// obj in future
				}

				// Is current day OK but current time larger than ending time?
				if ( CurDate->ds_Minute > this_node->End.HHMMSS.Minutes )
				{
					return(1);	// obj in past
				}
			}
		}
		else if (this_node->startendMode == ARGUMENT_CONTINUOUS)
		{
			CurMinutesSince1978 = (CurDate->ds_Days*1440)+CurDate->ds_Minute;
			ObjMinutesSince1978Start = (this_node->Start.HHMMSS.Days*1440)+this_node->Start.HHMMSS.Minutes;
			ObjMinutesSince1978End = (this_node->End.HHMMSS.Days*1440)+this_node->End.HHMMSS.Minutes;

			if ( ObjMinutesSince1978Start > ObjMinutesSince1978End )
			{
				return(3);	// error in dates/times
			}

			if ( CurMinutesSince1978 < ObjMinutesSince1978Start )
			{
				return(2);	// obj in future
			}

			if ( CurMinutesSince1978 > ObjMinutesSince1978End )
			{
				return(1);	// obj in past
			}

/*
			if(!(
				  (ObjMinutesSince1978Start <= CurMinutesSince1978) &&
				  (ObjMinutesSince1978End >= CurMinutesSince1978) 
			  ) )
				this_node->miscFlags |= OBJ_OUTDATED;
*/
		}
		else
			Message("Scheduling error 1");
	}
	else
	{
		return(3);
	}

	return(0);
}

/******** E O F ********/
