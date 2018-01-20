#include "nb:pre.h"
#include "setup.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"
#include "structs.h"
#include "math.h"

#define VERSI0N "\0$VER: 2.1"
#define MEMSIZE 800
static UBYTE *vers = VERSI0N;

/**** globals ****/

struct Library *medialinkLibBase		= NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase							= NULL;

UBYTE **msgs;
struct RendezVousRecord *rvrec;
struct MsgPort *capsport;

/**** disable CTRL-C break ****/

void CXBRK(void) { }
void chkabort(void) { }

/**** functions ****/

/******** XappSetup() ********/

void XappSetup(PROCESSINFO *ThisPI)
{
struct UserApplicInfo UAI;
struct MsgPort *port;
struct Node *node;
struct List *list;
struct Task *oldTask;

	/**** find the mother ship ****/

	port = (struct MsgPort *)FindPort(ML_RENDEZ_VOUS);
	if (port == NULL)
		return;

	/**** link with it ****/

	list = &(port->mp_MsgList);
	node = list->lh_Head;
	rvrec = (struct RendezVousRecord *)node->ln_Name;

	/**** drain it ****/

	msgs = (UBYTE **)rvrec->msgs;

	/**** open standard libraries ****/

	IntuitionBase 		= (struct IntuitionBase *)rvrec->intuition;
	GfxBase 					= (struct GfxBase *)rvrec->graphics;
	medialinkLibBase	= (struct Library *)rvrec->medialink;

	/**** initialize the User Application Info structure ***/

	UA_InitStruct(&UAI);
	UAI.IB = IntuitionBase;

	/**** open and load the medialink font ***/

	UAI.small_TA.ta_Name	= (UBYTE *)"fonts:mediapoint.font";
	UAI.small_TA.ta_YSize = 10;
	UAI.small_TA.ta_Style = FS_NORMAL;
	UAI.small_TA.ta_Flags = FPF_DESIGNED;
	UAI.small_TF = rvrec->smallfont;

	UAI.large_TA.ta_Name	= (UBYTE *)"fonts:mediapoint.font";
	UAI.large_TA.ta_YSize = 20;
	UAI.large_TA.ta_Style = FS_NORMAL;
	UAI.large_TA.ta_Flags = FPF_DESIGNED;
	UAI.large_TF = rvrec->largefont;

	/**** open a window ****/

	capsport = (struct MsgPort *)FindPort(MEDIALINKPORT);
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	if (UA_HostScreenPresent(&UAI))
		UAI.windowModes = 1;	/* open on the MediaLink screen */
	else
		UAI.windowModes = 3;	/* open on the first (frontmost) screen */

	/**** double the dimensions of gadgets etc. if screen is laced ****/

	if (UA_IsUAScreenLaced(&UAI))
		UA_DoubleGadgetDimensions(Studio_GR);

	UA_TranslateGR(Studio_GR, msgs);

	UAI.windowX				= -1;	/* -1 means center on screen */
	UAI.windowY				= -1;	/* -1 means center on screen */
	UAI.windowWidth		= Studio_GR[0].x2;
	UAI.windowHeight	= Studio_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(&UAI);

	/**** render all gadgets ****/

	UA_DrawGadgetListRange(UAI.userWindow, Studio_GR, 1, 9);

	/**** monitor events ****/

	MonitorUser(UAI.userWindow, ThisPI);
	
	/**** close the window ****/

	UA_CloseWindow(&UAI);

	capsport->mp_SigTask = oldTask;
}

/******** MonitorUser() ********/

BOOL MonitorUser(struct Window *window, PROCESSINFO *ThisPI)
{
BOOL loop=TRUE, retVal;
struct EventData CED;
int ID;
struct FileReqRecord FRR;
TEXT path[SIZE_PATH], filename[SIZE_FILENAME];
UWORD *mypattern1;
struct Studio_Record studio_rec;

	mypattern1 = (UWORD *)AllocMem(4L, MEMF_CHIP);
	if (mypattern1==NULL)
		return(FALSE);
	mypattern1[0] = 0x5555;
	mypattern1[1] = 0xaaaa;

	/**** set up FRR ****/

	strcpy(path, rvrec->capsprefs->sample_Path);
	filename[0] = '\0';

	FRR.path			= path;
	FRR.fileName	= filename;
	FRR.opts			= DIR_OPT_ALL | DIR_OPT_NOINFO;
	FRR.multiple	= FALSE;

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] == '\0' )
	{
		studio_rec.mode					= MODE_PLAY;

		studio_rec.playPath1[0]	= '\0';
		studio_rec.playPath2[0]	= '\0';
		studio_rec.playPath3[0]	= '\0';
		studio_rec.playPath4[0]	= '\0';
		studio_rec.playVolume		= 100;
		studio_rec.playFadeIn		= 0;

		studio_rec.misc					= MISC_STOP;
		studio_rec.stopFadeOut	= 0;
		studio_rec.fadeInSecs		= 0;
		studio_rec.fadeOutSecs	= 0;
		studio_rec.setVolFrom		= 100;
		studio_rec.setVolTo			= 50;
		studio_rec.setVolSecs		= 0;
	}
	else
	{
		GetExtraData(ThisPI,&studio_rec);
		if ( studio_rec.playPath1[0] )
			UA_SplitFullPath(studio_rec.playPath1, path, filename);
	}

	DrawTocPage(window,&studio_rec,mypattern1);

	/**** Check if Studio Arexx host is around ****/

	if ( !FindPort("Studio16.1") )
		GiveMessage(window, msgs[Msg_S16_7-1]);

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);
		if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, Studio_GR, &CED);
			switch(ID)
			{
				case 5:	// OK
do_ok:
					UA_HiliteButton(window, &Studio_GR[5]);
					loop=FALSE;
					retVal=TRUE; 
					break;

				case 6:	// Cancel
do_cancel:
					UA_HiliteButton(window, &Studio_GR[6]);
					loop=FALSE;
					retVal=FALSE; 
					break;

				case 7: // Preview
					UA_HiliteButton(window, &Studio_GR[ID]);
					UA_SetSprite(window, SPRITE_BUSY);
					DoPreview(window,&studio_rec,&CED,mypattern1);
					UA_SetSprite(window, SPRITE_NORMAL);
					break;

				case 9:	// Modes
					UA_ProcessCycleGadget(window, &Studio_GR[ID], &CED);
					UA_SetValToCycleGadgetVal(&Studio_GR[ID], &studio_rec.mode);
					UA_ClearButton(window, &Studio_GR[4], AREA_PEN);
					DrawTocPage(window,&studio_rec,mypattern1);
					break;
			}
			if (loop)
				CheckOtherButtons(window,&studio_rec,&FRR,mypattern1,&CED);
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE)
				goto do_cancel;
			else if (CED.Code==RAW_RETURN)
				goto do_ok;
		}
	}

	FreeMem(mypattern1, 4L);

	if ( retVal )
		PutExtraData(ThisPI, &studio_rec);

	SilenceInTheStudio();

	return(retVal);
}

/******** DrawTocPage() ********/

void DrawTocPage(	struct Window *window, struct Studio_Record *studio_rec,
									UWORD *mypattern1 )
{
TEXT str[SIZE_FULLPATH], *str2;
int i;

	UA_SetCycleGadgetToVal(window, &Studio_GR[9], studio_rec->mode);

	if ( studio_rec->mode == MODE_PLAY )
	{
		UA_DrawGadgetListRange(window, Studio_GR, 10, 16);
		UA_DrawGadgetListRange(window, Studio_GR, 30, 33);
		UA_DrawSpecialGadgetText(window, &Studio_GR[15], "%", SPECIAL_TEXT_AFTER_STRING);
		UA_DrawSpecialGadgetText(window, &Studio_GR[16], msgs[Msg_X_I_6-1], SPECIAL_TEXT_AFTER_STRING);
		for(i=0; i<4; i++)
		{
			if ( i==0 )
				str2 = studio_rec->playPath1;
			else if ( i==1 )
				str2 = studio_rec->playPath2;
			else if ( i==2 )
				str2 = studio_rec->playPath3;
			else if ( i==3 )
				str2 = studio_rec->playPath4;
			if ( str2[0]=='\0' )
				strcpy(str, msgs[Msg_X_5-1]);
			else
				strcpy(str, str2);
			UA_ShortenString(window->RPort, str, Studio_GR[11+i].x2-Studio_GR[11+i].x1-16);
			UA_ClearButton(window, &Studio_GR[11+i], AREA_PEN);
			UA_DrawText(window, &Studio_GR[11+i], str);
		}
		if (	studio_rec->playPath1[0]=='\0' && studio_rec->playPath2[0]=='\0' &&
					studio_rec->playPath3[0]=='\0' && studio_rec->playPath4[0]=='\0' )
			UA_DisableButton(window, &Studio_GR[7], mypattern1);	// preview
		else if ( UA_IsGadgetDisabled(&Studio_GR[7]) )
			UA_EnableButton(window, &Studio_GR[7]);
		UA_SetStringGadgetToVal(window, &Studio_GR[15], studio_rec->playVolume);
		UA_SetCycleGadgetToVal(window, &Studio_GR[16], studio_rec->playFadeIn);
	}
	else if ( studio_rec->mode == MODE_MISC )
	{
		for(i=24; i<=29; i++)
			UA_EnableButtonQuiet(&Studio_GR[i]);
		UA_DrawGadgetListRange(window, Studio_GR, 17, 29);
		if (	studio_rec->playPath1[0]=='\0' && studio_rec->playPath2[0]=='\0' &&
					studio_rec->playPath3[0]=='\0' && studio_rec->playPath4[0]=='\0' )
			UA_DisableButton(window, &Studio_GR[7], mypattern1);	// preview
		else if ( UA_IsGadgetDisabled(&Studio_GR[7]) )
			UA_EnableButton(window, &Studio_GR[7]);
		UA_DrawSpecialGadgetText(window, &Studio_GR[24], msgs[Msg_X_I_6-1], SPECIAL_TEXT_AFTER_STRING);
		UA_DrawSpecialGadgetText(window, &Studio_GR[25], msgs[Msg_X_I_6-1], SPECIAL_TEXT_AFTER_STRING);
		UA_DrawSpecialGadgetText(window, &Studio_GR[26], msgs[Msg_X_I_6-1], SPECIAL_TEXT_AFTER_STRING);
		UA_DrawSpecialGadgetText(window, &Studio_GR[29], msgs[Msg_X_I_6-1], SPECIAL_TEXT_AFTER_STRING);
		UA_DrawSpecialGadgetText(window, &Studio_GR[27], "%", SPECIAL_TEXT_AFTER_STRING);
		UA_DrawSpecialGadgetText(window, &Studio_GR[28], "%", SPECIAL_TEXT_AFTER_STRING);
		UA_DrawSpecialGadgetText(window, &Studio_GR[28], "", SPECIAL_TEXT_BEFORE_STRING);
		UA_InvertButton(window, &Studio_GR[17 + studio_rec->misc]); 
		UA_SetCycleGadgetToVal(window, &Studio_GR[24], studio_rec->stopFadeOut);
		UA_SetCycleGadgetToVal(window, &Studio_GR[25], studio_rec->fadeInSecs);
		UA_SetCycleGadgetToVal(window, &Studio_GR[26], studio_rec->fadeOutSecs);
		UA_SetStringGadgetToVal(window, &Studio_GR[27], studio_rec->setVolFrom);
		UA_SetStringGadgetToVal(window, &Studio_GR[28], studio_rec->setVolTo);
		UA_SetCycleGadgetToVal(window, &Studio_GR[29], studio_rec->setVolSecs);
		if ( studio_rec->misc==MISC_STOP )
		{
			UA_DisableButton(window, &Studio_GR[25], mypattern1);
			UA_DisableButton(window, &Studio_GR[26], mypattern1);
			UA_DisableButton(window, &Studio_GR[27], mypattern1);
			UA_DisableButton(window, &Studio_GR[28], mypattern1);
			UA_DisableButton(window, &Studio_GR[29], mypattern1);
		}
		else if ( studio_rec->misc==MISC_FADEIN )
		{
			UA_DisableButton(window, &Studio_GR[24], mypattern1);
			UA_DisableButton(window, &Studio_GR[26], mypattern1);
			UA_DisableButton(window, &Studio_GR[27], mypattern1);
			UA_DisableButton(window, &Studio_GR[28], mypattern1);
			UA_DisableButton(window, &Studio_GR[29], mypattern1);
		}
		else if ( studio_rec->misc==MISC_FADEOUT )
		{
			UA_DisableButton(window, &Studio_GR[24], mypattern1);
			UA_DisableButton(window, &Studio_GR[25], mypattern1);
			UA_DisableButton(window, &Studio_GR[27], mypattern1);
			UA_DisableButton(window, &Studio_GR[28], mypattern1);
			UA_DisableButton(window, &Studio_GR[29], mypattern1);
		}
		else if ( studio_rec->misc==MISC_SETVOL )
		{
			UA_DisableButton(window, &Studio_GR[24], mypattern1);
			UA_DisableButton(window, &Studio_GR[25], mypattern1);
			UA_DisableButton(window, &Studio_GR[26], mypattern1);
		}
	}
}

/******** CheckOtherButtons() ********/

void CheckOtherButtons(	struct Window *window, struct Studio_Record *studio_rec,
												struct FileReqRecord *FRR, UWORD *mypattern1,
												struct EventData *CED )
{
TEXT fullPath[SIZE_FULLPATH], *str2;
int ID;

	if ( studio_rec->mode==MODE_PLAY )
	{
		ID = UA_CheckGadgetList(window, &Studio_GR[0], CED);

		switch( ID )
		{
			case 11:	// sample path
			case 12:	// sample path
			case 13:	// sample path
			case 14:	// sample path
				UA_HiliteButton(window, &Studio_GR[ID]);
				FRR->title = msgs[Msg_S16_8-1];
				if ( UA_OpenAFile(window, FRR, mypattern1) )
				{
					UA_MakeFullPath(FRR->path, FRR->fileName, fullPath);
					if ( ID==11 )
						str2 = studio_rec->playPath1;
					else if ( ID==12 )
						str2 = studio_rec->playPath2;
					else if ( ID==13 )
						str2 = studio_rec->playPath3;
					else if ( ID==14 )
						str2 = studio_rec->playPath4;
					strcpy(str2, fullPath);
					UA_ShortenString(window->RPort, fullPath, Studio_GR[ID].x2-Studio_GR[ID].x1-16);
					UA_ClearButton(window, &Studio_GR[ID], AREA_PEN);
					UA_DrawText(window, &Studio_GR[ID], fullPath);
					//DrawTocPage(window,studio_rec,mypattern1);
					if (studio_rec->playPath1[0]=='\0' && studio_rec->playPath2[0]=='\0' &&
							studio_rec->playPath3[0]=='\0' && studio_rec->playPath4[0]=='\0' )
						UA_DisableButton(window, &Studio_GR[7], mypattern1);	// preview
					else if ( UA_IsGadgetDisabled(&Studio_GR[7]) )
						UA_EnableButton(window, &Studio_GR[7]);
				}
				break;

			case 15:	// volume
				UA_ProcessStringGadget(window, Studio_GR, &Studio_GR[ID], CED);
				UA_SetValToStringGadgetVal(&Studio_GR[ID], &studio_rec->playVolume);
				if ( studio_rec->playVolume<0 || studio_rec->playVolume>100 )
				{
					studio_rec->playVolume=100;
					GiveMessage(window, msgs[Msg_X_4-1], 0, 100); // "Enter a value between...
					UA_SetStringGadgetToVal(window, &Studio_GR[ID], 100);
				}
				break;

			case 16:	// fade in 'n' seconds
				UA_ProcessCycleGadget(window, &Studio_GR[ID], CED);
				UA_SetValToCycleGadgetVal(&Studio_GR[ID], &studio_rec->playFadeIn);
				break;

			case 30:	// delete sample path
			case 31:
			case 32:
			case 33:
				UA_HiliteButton(window, &Studio_GR[ID]);
				if ( ID==30 )
					studio_rec->playPath1[0] = '\0';
				else if ( ID==31 )
					studio_rec->playPath2[0] = '\0';
				else if ( ID==32 )
					studio_rec->playPath3[0] = '\0';
				else if ( ID==33 )
					studio_rec->playPath4[0] = '\0';
				UA_ClearButton(window, &Studio_GR[ID-19], AREA_PEN);	// 11 etc.
				UA_DrawText(window, &Studio_GR[ID-19], msgs[Msg_X_5-1]);
				if (studio_rec->playPath1[0]=='\0' && studio_rec->playPath2[0]=='\0' &&
						studio_rec->playPath3[0]=='\0' && studio_rec->playPath4[0]=='\0' )
					UA_DisableButton(window, &Studio_GR[7], mypattern1);	// preview
				else if ( UA_IsGadgetDisabled(&Studio_GR[7]) )
					UA_EnableButton(window, &Studio_GR[7]);
				break;
		}
	}
	else if ( studio_rec->mode==MODE_MISC )
	{
		ID = UA_CheckGadgetList(window, &Studio_GR[17], CED);
		if ( ID!=-1 )
			ID+=17;

		switch( ID )
		{
			case 17:
			case 18:
			case 19:
			case 20:
				if ( studio_rec->misc!=ID-17 )
				{
					UA_InvertButton(window, &Studio_GR[studio_rec->misc+17]);
					studio_rec->misc = ID-17;
					UA_InvertButton(window, &Studio_GR[studio_rec->misc+17]);
					DrawTocPage(window,studio_rec,mypattern1);
				}
				break;

			case 24:
				UA_ProcessCycleGadget(window, &Studio_GR[ID], CED);
				UA_SetValToCycleGadgetVal(&Studio_GR[ID], &studio_rec->stopFadeOut);
				break;

			case 25:
				UA_ProcessCycleGadget(window, &Studio_GR[ID], CED);
				UA_SetValToCycleGadgetVal(&Studio_GR[ID], &studio_rec->fadeInSecs);
				break;

			case 26:
				UA_ProcessCycleGadget(window, &Studio_GR[ID], CED);
				UA_SetValToCycleGadgetVal(&Studio_GR[ID], &studio_rec->fadeOutSecs);
				break;

			case 27:
				UA_ProcessStringGadget(window, Studio_GR, &Studio_GR[ID], CED);
				UA_SetValToStringGadgetVal(&Studio_GR[ID], &studio_rec->setVolFrom);
				if ( studio_rec->setVolFrom<0 || studio_rec->setVolFrom>100 )
				{
					studio_rec->setVolFrom=100;
					GiveMessage(window, msgs[Msg_X_4-1], 0, 100); // "Enter a value between...
					UA_SetStringGadgetToVal(window, &Studio_GR[ID], 100);
				}
				break;

			case 28:
				UA_ProcessStringGadget(window, Studio_GR, &Studio_GR[ID], CED);
				UA_SetValToStringGadgetVal(&Studio_GR[ID], &studio_rec->setVolTo);
				if ( studio_rec->setVolTo<0 || studio_rec->setVolTo>100 )
				{
					studio_rec->setVolTo=100;
					GiveMessage(window, msgs[Msg_X_4-1], 0, 100); // "Enter a value between...
					UA_SetStringGadgetToVal(window, &Studio_GR[ID], 100);
				}
				break;

			case 29:
				UA_ProcessCycleGadget(window, &Studio_GR[ID], CED);
				UA_SetValToCycleGadgetVal(&Studio_GR[ID], &studio_rec->setVolSecs);
				break;
		}
	}
}

/******** GetExtraData() ********/

void GetExtraData(PROCESSINFO *ThisPI, struct Studio_Record *studio_rec)
{
int numChars, argNum, len, numSeen;
char *strPtr;
char tmp[MEMSIZE], scrStr[MEMSIZE];

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] == '\0' )
		return;

	ScriptToStr(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData, scrStr);

	strPtr = scrStr;
	len = strlen(strPtr);
	argNum=0;
	numSeen=0;

	while(1)
	{
		if (*strPtr==0)
			break;
		numChars = stcarg(strPtr, " 	,");	// space, tab, comma
		if ( numChars>=1 && numChars<MEMSIZE )
		{
			stccpy(tmp, strPtr, numChars+1);
			switch(argNum)
			{
				case 0:
					sscanf(tmp, "%d", &studio_rec->mode);
					break;
				case 1:
					RemoveQuotes(tmp);
					strcpy(studio_rec->playPath1, tmp);
					break;
				case 2:
					RemoveQuotes(tmp);
					strcpy(studio_rec->playPath2, tmp);
					break;
				case 3:
					RemoveQuotes(tmp);
					strcpy(studio_rec->playPath3, tmp);
					break;
				case 4:
					RemoveQuotes(tmp);
					strcpy(studio_rec->playPath4, tmp);
					break;
				case 5:
					sscanf(tmp, "%d", &studio_rec->playVolume);
					break;
				case 6:
					sscanf(tmp, "%d", &studio_rec->playFadeIn);
					break;
				case 7:
					sscanf(tmp, "%d", &studio_rec->misc);
					break;
				case 8:
					sscanf(tmp, "%d", &studio_rec->stopFadeOut);
					break;
				case 9:
					sscanf(tmp, "%d", &studio_rec->fadeInSecs);
					break;
				case 10:
					sscanf(tmp, "%d", &studio_rec->fadeOutSecs);
					break;
				case 11:
					sscanf(tmp, "%d", &studio_rec->setVolFrom);
					break;
				case 12:
					sscanf(tmp, "%d", &studio_rec->setVolTo);
					break;
				case 13:
					sscanf(tmp, "%d", &studio_rec->setVolSecs);
					break;
			}
			argNum++;
		}
		strPtr += numChars+1;
		numSeen += numChars+1;
		if (numSeen>len)
			break;		
	}
}

/******** PutExtraData() ********/

void PutExtraData(PROCESSINFO *ThisPI, struct Studio_Record *studio_rec)
{
UBYTE scrStr1[SIZE_FULLPATH+10], scrStr2[SIZE_FULLPATH+10];
UBYTE scrStr3[SIZE_FULLPATH+10], scrStr4[SIZE_FULLPATH+10];

	StrToScript(studio_rec->playPath1, scrStr1);
	StrToScript(studio_rec->playPath2, scrStr2);
	StrToScript(studio_rec->playPath3, scrStr3);
	StrToScript(studio_rec->playPath4, scrStr4);

	sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"%d \\\"%s\\\" \\\"%s\\\" \\\"%s\\\" \\\"%s\\\" %d %d %d %d %d %d %d %d %d",
					studio_rec->mode,
					scrStr1, scrStr2, scrStr3, scrStr4,
					studio_rec->playVolume, studio_rec->playFadeIn,
					studio_rec->misc, studio_rec->stopFadeOut,
					studio_rec->fadeInSecs, studio_rec->fadeOutSecs,
					studio_rec->setVolFrom, studio_rec->setVolTo, studio_rec->setVolSecs );
}

/******** DoPreview() ********/

void DoPreview(	struct Window *window, struct Studio_Record *studio_rec,
								struct EventData *CED, UWORD *mypattern1 )
{
TEXT errorStr[100];

	if ( !PerformActions(studio_rec) )	// errorStr) )
		GiveMessage(window, errorStr);
}

/******** E O F ********/
