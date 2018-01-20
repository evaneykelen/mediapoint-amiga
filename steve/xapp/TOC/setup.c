#include "nb:pre.h"
#include "setup.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"
#include "structs.h"
#include <libraries/toccata.h>
#include <clib/toccata_protos.h>
#include <pragmas/toccata_pragmas.h>
#include <utility/tagitem.h>
#include <clib/graphics_protos.h>
#include "math.h"

#define VERSI0N "\0$VER: 1.1"
#define MEMSIZE 800
static UBYTE *vers = VERSI0N;

void __saveds __asm CB_Level( register __d0 ULONG Left, register __d1 ULONG Right );

/**** globals ****/

struct Library *medialinkLibBase		= NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase							= NULL;
struct ToccataBase *ToccataBase			= NULL;

UBYTE **msgs;
struct RendezVousRecord *rvrec;
struct MsgPort *capsport;

BOOL toccataPresent = TRUE;

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
		UA_DoubleGadgetDimensions(TOC_GR);

	UA_TranslateGR(TOC_GR, msgs);
	UA_TranslateGR(Record_GR, msgs);

	UAI.windowX				= -1;	/* -1 means center on screen */
	UAI.windowY				= -1;	/* -1 means center on screen */
	UAI.windowWidth		= TOC_GR[0].x2;
	UAI.windowHeight	= TOC_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(&UAI);

	/**** render all gadgets ****/

	UA_DrawGadgetListRange(UAI.userWindow, TOC_GR,  1, 7);
	UA_DrawGadgetListRange(UAI.userWindow, TOC_GR, 41, 42);

	/**** monitor events ****/

	ToccataBase = ObtainToccata(UAI.userWindow);
	if ( !ToccataBase )
		toccataPresent = FALSE;

	MonitorUser(UAI.userWindow, ThisPI);

	ReleaseToccata(ToccataBase);
	
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
struct Toccata_Record toc_rec;
struct TagItem tocTags[] = 
{
	{ TT_Level, (LONG)CB_Level	},
	{ TAG_DONE									},
};
ULONG *ptrs[10];
struct RastPort rastport;

	CopyMem(window->RPort,&rastport,sizeof(struct RastPort));	// used by level indicator

	ptrs[0] = (ULONG *)TOC_GR;
	ptrs[1] = (ULONG *)&toc_rec;
	ptrs[2] = (ULONG *)&rastport;
	ptrs[3] = NULL;
	window->UserData = (UBYTE *)&ptrs[0];

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
	FRR.opts			= DIR_OPT_MAUD | DIR_OPT_NOINFO;
	FRR.multiple	= FALSE;

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] == '\0' )
	{
		/**** MY OWN DEFAULTS ****/

		toc_rec.mode					= MODE_PLAY;

		toc_rec.playPath[0]		= '\0';
		toc_rec.playLoops			= 1;
		toc_rec.playFadeIn		= 0;

		toc_rec.recordPath[0]	= '\0';

		toc_rec.misc					= MISC_STOP;
		toc_rec.stopFadeOut		= 0;
		toc_rec.fadeInSecs		= 0;
		toc_rec.fadeOutSecs		= 0;
		toc_rec.setVolSecs		= 0;
		toc_rec.setVolTo			= 0;

		/**** TOCCATA HARDWARE/SOFTWARE DEFAULTS ****/

		if ( toccataPresent )
			GetToccataDefaults(&toc_rec);
	}
	else
		GetExtraData(ThisPI,&toc_rec);

	DrawTocPage(window,&toc_rec,mypattern1);

	if ( toc_rec.mode==MODE_RECORD && toccataPresent )
		T_StartLevel(tocTags);

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);
		if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, TOC_GR, &CED);
			switch(ID)
			{
				case 3:	// OK
do_ok:
					UA_HiliteButton(window, &TOC_GR[3]);
					loop=FALSE;
					retVal=TRUE; 
					break;

				case 4:	// Cancel
do_cancel:
					UA_HiliteButton(window, &TOC_GR[4]);
					loop=FALSE;
					retVal=FALSE; 
					break;

				case 5: // Preview
					UA_HiliteButton(window, &TOC_GR[ID]);
					if ( toc_rec.mode==MODE_RECORD && toccataPresent )
						T_StopLevel();
					if ( toccataPresent )
						SetToccataDefaults(&toc_rec);
					DoPreview(window,&toc_rec,&CED,mypattern1,toccataPresent,ToccataBase);
					if ( toc_rec.mode==MODE_RECORD && toccataPresent )
						T_StartLevel(tocTags);
					break;

				case 7:	// Modes
					if ( toc_rec.mode==MODE_RECORD && toccataPresent )
						T_StopLevel();
					UA_ProcessCycleGadget(window, &TOC_GR[ID], &CED);
					UA_SetValToCycleGadgetVal(&TOC_GR[ID], &toc_rec.mode);
					UA_ClearButton(window, &TOC_GR[2], AREA_PEN);
					DrawTocPage(window,&toc_rec,mypattern1);
					if ( toc_rec.mode==MODE_RECORD && toccataPresent )
						T_StartLevel(tocTags);
					break;
			}
			if (loop)
				CheckOtherButtons(window,&toc_rec,&FRR,mypattern1,&CED);
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE)
				goto do_cancel;
			else if (CED.Code==RAW_RETURN)
				goto do_ok;
		}
	}

	if ( toccataPresent )
		T_StopLevel();

	FreeMem(mypattern1, 4L);

	if ( retVal )
		PutExtraData(ThisPI, &toc_rec);

	return(retVal);
}

/******** DrawTocPage() ********/

void DrawTocPage(	struct Window *window, struct Toccata_Record *toc_rec,
									UWORD *mypattern1 )
{
TEXT str[256];
int i;

	UA_SetCycleGadgetToVal(window, &TOC_GR[7], toc_rec->mode);

	if ( toc_rec->mode == MODE_PLAY )
	{
		TOC_GR[8].type = HIBOX_REGION;
		UA_DrawGadgetListRange(window, TOC_GR, 8, 11);
		TOC_GR[8].type = BUTTON_GADGET;

		TOC_GR[25].type = INVISIBLE_GADGET;
		TOC_GR[25].color = 2;
		UA_ClearButton(window, &TOC_GR[25], AREA_PEN);
		TOC_GR[25].type = BUTTON_GADGET;
		TOC_GR[25].color = 0;

		UA_DrawSpecialGadgetText(window, &TOC_GR[9], "%", SPECIAL_TEXT_AFTER_STRING);
		UA_DrawSpecialGadgetText(window, &TOC_GR[11], msgs[Msg_X_I_6-1], SPECIAL_TEXT_AFTER_STRING);

		if ( toc_rec->playPath[0]=='\0' )
			strcpy(str, msgs[Msg_X_5-1]);
		else
			strcpy(str, toc_rec->playPath);
		UA_ShortenString(window->RPort, str, TOC_GR[8].x2-TOC_GR[8].x1-16);
		UA_ClearButton(window, &TOC_GR[8], AREA_PEN);
		UA_DrawText(window, &TOC_GR[8], str);

		if ( !toccataPresent )
			UA_DisableButton(window, &TOC_GR[5], mypattern1);	// preview
		else
		{
			if ( toc_rec->playPath[0]=='\0' )
				UA_DisableButton(window, &TOC_GR[5], mypattern1);	// preview
			else if ( UA_IsGadgetDisabled(&TOC_GR[5]) )
				UA_EnableButton(window, &TOC_GR[5]);
		}

		UA_SetStringGadgetToVal(window, &TOC_GR[9], toc_rec->playVolume);

		//UA_SetCycleGadgetToVal(window, &TOC_GR[10], toc_rec->playLoops);

		UA_SetCycleGadgetToVal(window, &TOC_GR[11], toc_rec->playFadeIn);
	}
	else if ( toc_rec->mode == MODE_RECORD )
	{
		TOC_GR[12].type = HIBOX_REGION;
		TOC_GR[24].type = HIBOX_REGION;
		UA_DrawGadgetListRange(window, TOC_GR, 12, 27);
		TOC_GR[12].type = BUTTON_GADGET;
		TOC_GR[24].type = BUTTON_GADGET;

		if ( toc_rec->recordPath[0]=='\0' )
			strcpy(str, msgs[Msg_X_5-1]);
		else
			strcpy(str, toc_rec->recordPath);
		UA_ShortenString(window->RPort, str, TOC_GR[12].x2-TOC_GR[12].x1-16);
		UA_ClearButton(window, &TOC_GR[12], AREA_PEN);
		UA_DrawText(window, &TOC_GR[12], str);

		if ( !toccataPresent )
		{
			UA_DisableButton(window, &TOC_GR[ 5], mypattern1);	// preview
			UA_DisableButton(window, &TOC_GR[25], mypattern1);	// record
		}
		else
		{
			if ( toc_rec->recordPath[0]=='\0' )
			{
				UA_DisableButton(window, &TOC_GR[ 5], mypattern1);	// play
				UA_DisableButton(window, &TOC_GR[25], mypattern1);	// record
			}
			else if ( UA_IsGadgetDisabled(&TOC_GR[5]) )
			{
				UA_EnableButton(window, &TOC_GR[ 5]);
				UA_EnableButton(window, &TOC_GR[25]);
			}
		}

		for(i=13; i<=22; i++)
			UA_DrawGadget(window, &TOC_GR[i]);

		UA_InvertButton(window, &TOC_GR[13 + toc_rec->recordCompression]); 

		if ( toc_rec->stereo )
			UA_InvertButton(window, &TOC_GR[17]); 

		UA_InvertButton(window, &TOC_GR[18 + toc_rec->input]); 

		if ( toc_rec->gain )
			UA_InvertButton(window, &TOC_GR[22]); 

		UA_SetCycleGadgetToVal(window, &TOC_GR[23], toc_rec->freq);

		UA_DrawSliderNotches(window, &TOC_GR[24], 2, 16, LO_PEN);
		UA_SetSliderGadg(window, &TOC_GR[24], toc_rec->level, 16, NULL, 0);
	}
	else if ( toc_rec->mode == MODE_MISC )
	{
		for(i=35; i<=40; i++)
			UA_EnableButtonQuiet(&TOC_GR[i]);

		UA_DrawGadgetListRange(window, TOC_GR, 28, 40);

		TOC_GR[25].type = INVISIBLE_GADGET;
		TOC_GR[25].color = 2;
		UA_ClearButton(window, &TOC_GR[25], AREA_PEN);
		TOC_GR[25].type = BUTTON_GADGET;
		TOC_GR[25].color = 0;

		if ( !toccataPresent )
			UA_DisableButton(window, &TOC_GR[5], mypattern1);	// preview
		else
		{
			if ( toc_rec->playPath[0]=='\0' )
				UA_DisableButton(window, &TOC_GR[5], mypattern1);	// preview
			else if ( UA_IsGadgetDisabled(&TOC_GR[5]) )
				UA_EnableButton(window, &TOC_GR[5]);
		}

		UA_DrawSpecialGadgetText(window, &TOC_GR[35], msgs[Msg_X_I_6-1], SPECIAL_TEXT_AFTER_STRING);
		UA_DrawSpecialGadgetText(window, &TOC_GR[36], msgs[Msg_X_I_6-1], SPECIAL_TEXT_AFTER_STRING);
		UA_DrawSpecialGadgetText(window, &TOC_GR[37], msgs[Msg_X_I_6-1], SPECIAL_TEXT_AFTER_STRING);
		UA_DrawSpecialGadgetText(window, &TOC_GR[38], "%", SPECIAL_TEXT_AFTER_STRING);
		UA_DrawSpecialGadgetText(window, &TOC_GR[39], "", SPECIAL_TEXT_BEFORE_STRING);
		UA_DrawSpecialGadgetText(window, &TOC_GR[39], "%", SPECIAL_TEXT_AFTER_STRING);
		UA_DrawSpecialGadgetText(window, &TOC_GR[40], msgs[Msg_X_I_6-1], SPECIAL_TEXT_AFTER_STRING);

		for(i=28; i<=31; i++)
			UA_DrawGadget(window, &TOC_GR[i]);

		UA_InvertButton(window, &TOC_GR[28 + toc_rec->misc]); 

		UA_SetCycleGadgetToVal(window, &TOC_GR[35], toc_rec->stopFadeOut);
		UA_SetCycleGadgetToVal(window, &TOC_GR[36], toc_rec->fadeInSecs);
		UA_SetCycleGadgetToVal(window, &TOC_GR[37], toc_rec->fadeOutSecs);

		UA_SetStringGadgetToVal(window, &TOC_GR[38], toc_rec->setVolFrom);
		UA_SetStringGadgetToVal(window, &TOC_GR[39], toc_rec->setVolTo);
		UA_SetCycleGadgetToVal(window, &TOC_GR[40], toc_rec->setVolSecs);

		if ( toc_rec->misc==MISC_STOP )
		{
			UA_DisableButton(window, &TOC_GR[36], mypattern1);
			UA_DisableButton(window, &TOC_GR[37], mypattern1);
			UA_DisableButton(window, &TOC_GR[38], mypattern1);
			UA_DisableButton(window, &TOC_GR[39], mypattern1);
			UA_DisableButton(window, &TOC_GR[40], mypattern1);
		}
		else if ( toc_rec->misc==MISC_FADEIN )
		{
			UA_DisableButton(window, &TOC_GR[35], mypattern1);
			UA_DisableButton(window, &TOC_GR[37], mypattern1);
			UA_DisableButton(window, &TOC_GR[38], mypattern1);
			UA_DisableButton(window, &TOC_GR[39], mypattern1);
			UA_DisableButton(window, &TOC_GR[40], mypattern1);
		}
		else if ( toc_rec->misc==MISC_FADEOUT )
		{
			UA_DisableButton(window, &TOC_GR[35], mypattern1);
			UA_DisableButton(window, &TOC_GR[36], mypattern1);
			UA_DisableButton(window, &TOC_GR[38], mypattern1);
			UA_DisableButton(window, &TOC_GR[39], mypattern1);
			UA_DisableButton(window, &TOC_GR[40], mypattern1);
		}
		else if ( toc_rec->misc==MISC_SETVOL )
		{
			UA_DisableButton(window, &TOC_GR[35], mypattern1);
			UA_DisableButton(window, &TOC_GR[36], mypattern1);
			UA_DisableButton(window, &TOC_GR[37], mypattern1);
		}
	}
}

/******** GetExtraData() ********/

void GetExtraData(PROCESSINFO *ThisPI, struct Toccata_Record *toc_rec)
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
					sscanf(tmp, "%d", &toc_rec->mode);
					break;
				case 1:
					RemoveQuotes(tmp);
					strcpy(toc_rec->playPath, tmp);
					break;
				case 2:
					sscanf(tmp, "%d", &toc_rec->playVolume);
					break;
				case 3:
					sscanf(tmp, "%d", &toc_rec->playLoops);
					break;
				case 4:
					sscanf(tmp, "%d", &toc_rec->playFadeIn);
					break;
				case 5:
					RemoveQuotes(tmp);
					strcpy(toc_rec->recordPath, tmp);
					break;
				case 6:
					sscanf(tmp, "%d", &toc_rec->recordCompression);
					break;
				case 7:
					sscanf(tmp, "%d", &toc_rec->stereo);
					break;
				case 8:
					sscanf(tmp, "%d", &toc_rec->input);
					break;
				case 9:
					sscanf(tmp, "%d", &toc_rec->gain);
					break;
				case 10:
					sscanf(tmp, "%d", &toc_rec->freq);
					break;
				case 11:
					sscanf(tmp, "%d", &toc_rec->level);
					break;
				case 12:
					sscanf(tmp, "%d", &toc_rec->misc);
					break;
				case 13:
					sscanf(tmp, "%d", &toc_rec->stopFadeOut);
					break;
				case 14:
					sscanf(tmp, "%d", &toc_rec->fadeInSecs);
					break;
				case 15:
					sscanf(tmp, "%d", &toc_rec->fadeOutSecs);
					break;
				case 16:
					sscanf(tmp, "%d", &toc_rec->setVolFrom);
					break;
				case 17:
					sscanf(tmp, "%d", &toc_rec->setVolTo);
					break;
				case 18:
					sscanf(tmp, "%d", &toc_rec->setVolSecs);
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

void PutExtraData(PROCESSINFO *ThisPI, struct Toccata_Record *toc_rec)
{
UBYTE scrStr1[SIZE_FULLPATH+10], scrStr2[SIZE_FULLPATH+10];

	StrToScript(toc_rec->playPath, scrStr1);
	StrToScript(toc_rec->recordPath, scrStr2);

	sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"%d \\\"%s\\\" %d %d %d \\\"%s\\\" %d %d %d %d %d %d %d %d %d %d %d %d %d",
					toc_rec->mode,
					scrStr1,
					toc_rec->playVolume,
					toc_rec->playLoops,
					toc_rec->playFadeIn,
					scrStr2,
					toc_rec->recordCompression,
					toc_rec->stereo,
					toc_rec->input,
					toc_rec->gain,
					toc_rec->freq,
					toc_rec->level,
					toc_rec->misc,
					toc_rec->stopFadeOut,
					toc_rec->fadeInSecs,
					toc_rec->fadeOutSecs,
					toc_rec->setVolFrom,
					toc_rec->setVolTo,
					toc_rec->setVolSecs );
}

/******** ObtainToccata() ********/

struct ToccataBase *ObtainToccata(struct Window *window)
{
struct ToccataBase *lib;

	lib = (struct ToccataBase *)OpenLibrary("toccata.library",0L);
	if ( !lib )
	{
		if ( window )
			GiveMessage(window, msgs[Msg_TOC_21-1]);
	}
	else
	{
		if ( !lib->tb_HardInfo )	// library present but no card
		{
			if ( window )
				GiveMessage(window, msgs[Msg_TOC_22-1]);
			CloseLibrary((struct Library *)lib);
			lib=NULL;	// sorry: no party today
		}
	}

	return(lib);
}

/******** ReleaseToccata() ********/

void ReleaseToccata(struct ToccataBase *lib)
{
	if ( lib )
		CloseLibrary((struct Library *)lib);
}

/******** GetToccataDefaults() ********/

void GetToccataDefaults(struct Toccata_Record *toc_rec)
{
struct TagItem tocTags[32];
LONG l[32], val;
int i;
UBYTE perc_to_db[] =
{
 0, 
 0,  0,  0,  1,  1,  1,  1,  1,  1,  1, 
 2,  2,  2,  2,  2,  2,  3,  3,  3,  3, 
 3,  3,  4,  4,  4,  4,  4,  5,  5,  5, 
 5,  5,  6,  6,  6,  6,  6,  7,  7,  7, 
 7,  8,  8,  8,  8,  9,  9,  9,  9, 10, 
10, 10, 10, 11, 11, 11, 12, 12, 12, 13, 
13, 13, 14, 14, 15, 15, 15, 16, 16, 17, 
17, 18, 18, 19, 19, 20, 20, 21, 22, 22, 
23, 24, 25, 25, 26, 27, 28, 29, 31, 32, 
33, 35, 37, 39, 42, 45, 49, 54, 64 
};

	tocTags[0].ti_Tag		= PAT_MixAux1Left;
	tocTags[0].ti_Data	= (LONG)&l[0];

	tocTags[1].ti_Tag		= PAT_MixAux1Right;
	tocTags[1].ti_Data	= (LONG)&l[1];

	tocTags[2].ti_Tag		= PAT_MixAux2Left;
	tocTags[2].ti_Data	= (LONG)&l[2];

	tocTags[3].ti_Tag		= PAT_MixAux2Right;
	tocTags[3].ti_Data	= (LONG)&l[3];

	tocTags[4].ti_Tag		= PAT_InputVolumeLeft;
	tocTags[4].ti_Data	= (LONG)&l[4];

	tocTags[5].ti_Tag		= PAT_InputVolumeRight;
	tocTags[5].ti_Data	= (LONG)&l[5];

	tocTags[6].ti_Tag		= PAT_OutputVolumeLeft;
	tocTags[6].ti_Data	= (LONG)&l[6];

	tocTags[7].ti_Tag		= PAT_OutputVolumeRight;
	tocTags[7].ti_Data	= (LONG)&l[7];

	tocTags[8].ti_Tag		= PAT_LoopbackVolume;
	tocTags[8].ti_Data	= (LONG)&l[8];

	tocTags[9].ti_Tag		= PAT_Input;
	tocTags[9].ti_Data	= (LONG)&l[9];

	tocTags[10].ti_Tag	= PAT_MicGain;
	tocTags[10].ti_Data	= (LONG)&l[10];

	tocTags[11].ti_Tag	= PAT_Mode;
	tocTags[11].ti_Data	= (LONG)&l[11];

	tocTags[12].ti_Tag	= PAT_Frequency;
	tocTags[12].ti_Data	= (LONG)&l[12];

	tocTags[13].ti_Tag	= TAG_DONE;

	T_GetPart( tocTags );

	// Translate hardware register values to GUI values

	// input from hardware ranges from 0...-64 db (full volume to muted)
	val = l[6] + 64;					// l ranges from 0...-64	-> +64 -> 64...0
	for(i=0; i<100; i++)
	{
		if ( perc_to_db[i] >= val )	// eg if val==64 -> i==99;
		{
			val = i+1;
			break;
		}
	}			
	toc_rec->playVolume = (int)val;
	toc_rec->setVolFrom = (int)val;

	val = l[11];
	val = val & TMODE_MASK;
	if ( val==TMODE_LINEAR_8 )
		toc_rec->recordCompression = 0;
	else if ( val==TMODE_LINEAR_16 )
		toc_rec->recordCompression = 1;
	else if ( val==TMODE_ALAW )
		toc_rec->recordCompression = 2;
	else if ( val==TMODE_ULAW )
		toc_rec->recordCompression = 3;

	val = l[11];
	if ( val & TMODEF_STEREO )
		toc_rec->stereo = 1;
	else
		toc_rec->stereo = 0;

	val = l[9];
	if ( val==TINPUT_Line )
		toc_rec->input = 0;
	else if ( val==TINPUT_Aux1 )
		toc_rec->input = 1;
	else if ( val==TINPUT_Mic )
		toc_rec->input = 2;
	else if ( val==TINPUT_Mix )
		toc_rec->input = 3;

	val = l[10];
	if ( val )
		toc_rec->gain = 1;
	else
		toc_rec->gain = 0;

	val = l[12];
	if ( val==5513 )
		toc_rec->freq = 0;
	else if ( val==6615 )
		toc_rec->freq = 1;
	else if ( val==8000 )
		toc_rec->freq = 2;
	else if ( val==9600 )
		toc_rec->freq = 3;
	else if ( val==11025 )
		toc_rec->freq = 4;
	else if ( val==16000 )
		toc_rec->freq = 5;
	else if ( val==18900 )
		toc_rec->freq = 6;
	else if ( val==22050 )
		toc_rec->freq = 7;
	else if ( val==27429 )
		toc_rec->freq = 8;
	else if ( val==32000 )
		toc_rec->freq = 9;
	else if ( val==33075 )
		toc_rec->freq = 10;
	else if ( val==37800 )
		toc_rec->freq = 11;
	else if ( val==44100 )
		toc_rec->freq = 12;
	else if ( val==48000 )
		toc_rec->freq = 13;

	val = l[4];
	toc_rec->level = val;
}

/******** SetToccataDefaults() ********/

void SetToccataDefaults(struct Toccata_Record *toc_rec)
{
struct TagItem tocTags[32];
LONG val;
UBYTE perc_to_db[] =
{
 0, 
 0,  0,  0,  1,  1,  1,  1,  1,  1,  1, 
 2,  2,  2,  2,  2,  2,  3,  3,  3,  3, 
 3,  3,  4,  4,  4,  4,  4,  5,  5,  5, 
 5,  5,  6,  6,  6,  6,  6,  7,  7,  7, 
 7,  8,  8,  8,  8,  9,  9,  9,  9, 10, 
10, 10, 10, 11, 11, 11, 12, 12, 12, 13, 
13, 13, 14, 14, 15, 15, 15, 16, 16, 17, 
17, 18, 18, 19, 19, 20, 20, 21, 22, 22, 
23, 24, 25, 25, 26, 27, 28, 29, 31, 32, 
33, 35, 37, 39, 42, 45, 49, 54, 64 
};

	// Translate GUI values to hardware register values

	if ( toc_rec->mode==MODE_PLAY )
		val = toc_rec->playVolume;
	else
		val = toc_rec->setVolFrom;

	// val goes from 0..100

	if ( val==0 )
		val = 64;	// mute
	else
		val = perc_to_db[ 100-val ];	// 100-100=0->0 dB and 100-1=99->64 dB
	val = val * -1;

	tocTags[0].ti_Tag	= PAT_OutputVolumeLeft;
	tocTags[0].ti_Data = val;

	tocTags[1].ti_Tag	= PAT_OutputVolumeRight;
	tocTags[1].ti_Data = val;

	if ( toc_rec->recordCompression==0 )
		val = TMODE_LINEAR_8;
	else if ( toc_rec->recordCompression==1 )
		val = TMODE_LINEAR_16;
	else if ( toc_rec->recordCompression==2 )
		val = TMODE_ALAW;
	else if ( toc_rec->recordCompression==3 )
		val = TMODE_ULAW;

	if ( toc_rec->stereo==1 )
		val |= TMODEF_STEREO;

	tocTags[2].ti_Tag	= PAT_Mode;
	tocTags[2].ti_Data = val;

	if ( toc_rec->input==0 )
		val = TINPUT_Line;
	else if ( toc_rec->input==1 )
		val = TINPUT_Aux1;
	else if ( toc_rec->input==2 )
		val = TINPUT_Mic;
	else if ( toc_rec->input==3 )
		val = TINPUT_Mix;

	tocTags[3].ti_Tag	= PAT_Input;
	tocTags[3].ti_Data = val;

	if ( toc_rec->gain==1 )
		val = TRUE;
	else
		val = FALSE;

	tocTags[4].ti_Tag	= PAT_MicGain;
	tocTags[4].ti_Data = val;

	if ( toc_rec->freq==0 )
		val = 5513;
	else if ( toc_rec->freq==1 )
		val = 6615;
	else if ( toc_rec->freq==2 )
		val = 8000;
	else if ( toc_rec->freq==3 )
		val = 9600;
	else if ( toc_rec->freq==4 )
		val = 11025;
	else if ( toc_rec->freq==5 )
		val = 16000;
	else if ( toc_rec->freq==6 )
		val = 18900;
	else if ( toc_rec->freq==7 )
		val = 22050;
	else if ( toc_rec->freq==8 )
		val = 27429;
	else if ( toc_rec->freq==9 )
		val = 32000;
	else if ( toc_rec->freq==10 )
		val = 33075;
	else if ( toc_rec->freq==11 )
		val = 37800;
	else if ( toc_rec->freq==12 )
		val = 44100;
	else if ( toc_rec->freq==13 )
		val = 48000;

	tocTags[5].ti_Tag = PAT_Frequency;
	tocTags[5].ti_Data = val;

	val = toc_rec->level;
	tocTags[6].ti_Tag	= PAT_InputVolumeLeft;
	tocTags[6].ti_Data = val;

	tocTags[7].ti_Tag	= PAT_InputVolumeRight;
	tocTags[7].ti_Data = val;

	tocTags[8].ti_Tag = TAG_DONE;

	T_SetPart( tocTags );
}

/******** CheckOtherButtons() ********/

void CheckOtherButtons(	struct Window *window, struct Toccata_Record *toc_rec,
												struct FileReqRecord *FRR, UWORD *mypattern1,
												struct EventData *CED )
{
TEXT fullPath[SIZE_FULLPATH];
int ID;

	if ( toc_rec->mode==MODE_PLAY )
	{
		ID = UA_CheckGadgetList(window, &TOC_GR[0], CED);

		switch( ID )
		{
			case 8:		// sample path
				UA_HiliteButton(window, &TOC_GR[ID]);
				FRR->title = msgs[Msg_S16_8-1];
				if ( toc_rec->playPath[0] )
					UA_SplitFullPath(toc_rec->playPath, FRR->path, FRR->fileName);
				if ( UA_OpenAFile(window, FRR, mypattern1) )
				{
					UA_MakeFullPath(FRR->path, FRR->fileName, fullPath);
					strcpy(toc_rec->playPath, fullPath);
					UA_ShortenString(window->RPort, fullPath, TOC_GR[8].x2-TOC_GR[8].x1-16);
					UA_ClearButton(window, &TOC_GR[8], AREA_PEN);
					UA_DrawText(window, &TOC_GR[8], fullPath);
					DrawTocPage(window,toc_rec,mypattern1);
				}
				break;

			case 9:		// volume
				UA_ProcessStringGadget(window, TOC_GR, &TOC_GR[ID], CED);
				UA_SetValToStringGadgetVal(&TOC_GR[ID], &toc_rec->playVolume);
				if ( toc_rec->playVolume<0 || toc_rec->playVolume>100 )
				{
					toc_rec->playVolume=100;
					GiveMessage(window, msgs[Msg_X_4-1], 0, 100); // "Enter a value between...
					UA_SetStringGadgetToVal(window, &TOC_GR[ID], 100);
				}
				break;
/*
			case 10:	// # loops
				UA_ProcessCycleGadget(window, &TOC_GR[ID], CED);
				UA_SetValToCycleGadgetVal(&TOC_GR[ID], &toc_rec->playLoops);
				break;
*/
			case 11:	// fade in 'n' seconds
				UA_ProcessCycleGadget(window, &TOC_GR[ID], CED);
				UA_SetValToCycleGadgetVal(&TOC_GR[ID], &toc_rec->playFadeIn);
				break;
		}
	}
	else if ( toc_rec->mode==MODE_RECORD )
	{
		ID = UA_CheckGadgetList(window, &TOC_GR[12], CED);
		if ( ID!=-1 )
			ID+=12;

		switch( ID )
		{
			case 12:	// sample path
				UA_HiliteButton(window, &TOC_GR[ID]);
				FRR->title = msgs[Msg_TOC_23-1];
				if ( toc_rec->recordPath[0] )
					UA_SplitFullPath(toc_rec->recordPath, FRR->path, FRR->fileName);
				else
				{
					strcpy(FRR->path,"RAM:");
					strcpy(FRR->fileName,"sample1");
				}
				if ( UA_SaveAFile(window, FRR, mypattern1) )
				{
					UA_MakeFullPath(FRR->path, FRR->fileName, fullPath);
					strcpy(toc_rec->recordPath, fullPath);
					if ( toc_rec->playPath[0]=='\0' )
						strcpy(toc_rec->playPath,toc_rec->recordPath);	// SmartStep
					UA_ShortenString(window->RPort, fullPath, TOC_GR[ID].x2-TOC_GR[ID].x1-16);
					UA_ClearButton(window, &TOC_GR[ID], AREA_PEN);
					UA_DrawText(window, &TOC_GR[ID], fullPath);
					DrawTocPage(window,toc_rec,mypattern1);
				}
				break;

			case 13:
			case 14:
			case 15:
			case 16:
				if ( toc_rec->recordCompression!=ID-13 )
				{
					UA_InvertButton(window, &TOC_GR[toc_rec->recordCompression+13]);
					toc_rec->recordCompression = ID-13;
					UA_InvertButton(window, &TOC_GR[toc_rec->recordCompression+13]);
				}
				break;

			case 17:
				UA_InvertButton(window, &TOC_GR[17]);
				if ( toc_rec->stereo )
					toc_rec->stereo = FALSE;
				else
					toc_rec->stereo = TRUE;
				break;

			case 18:
			case 19:
			case 20:
			case 21:
				if ( toc_rec->input!=ID-18 )
				{
					UA_InvertButton(window, &TOC_GR[toc_rec->input+18]);
					toc_rec->input = ID-18;
					UA_InvertButton(window, &TOC_GR[toc_rec->input+18]);
				}
				break;

			case 22:
				UA_InvertButton(window, &TOC_GR[22]);
				if ( toc_rec->gain )
					toc_rec->gain = FALSE;
				else
					toc_rec->gain = TRUE;
				break;

			case 23:
				UA_ProcessCycleGadget(window, &TOC_GR[ID], CED);
				UA_SetValToCycleGadgetVal(&TOC_GR[ID], &toc_rec->freq);
				break;

			case 24:
				UA_ProcessSliderGadg(window,&TOC_GR[ID],&toc_rec->level,16,NULL,CED,NULL,NULL,0,0);
				break;

			case 25:
				UA_HiliteButton(window, &TOC_GR[ID]);
				DoRecord(window,toc_rec,CED,mypattern1);
				break;
		}

		if ( ID!=-1 && toccataPresent )
			SetToccataDefaults(toc_rec);
	}
	else if ( toc_rec->mode==MODE_MISC )
	{
		ID = UA_CheckGadgetList(window, &TOC_GR[28], CED);
		if ( ID!=-1 )
			ID+=28;

		switch( ID )
		{
			case 28:
			case 29:
			case 30:
			case 31:
				if ( toc_rec->misc!=ID-28 )
				{
					UA_InvertButton(window, &TOC_GR[toc_rec->misc+28]);
					toc_rec->misc = ID-28;
					UA_InvertButton(window, &TOC_GR[toc_rec->misc+28]);
					DrawTocPage(window,toc_rec,mypattern1);
				}
				break;

			case 35:
				UA_ProcessCycleGadget(window, &TOC_GR[ID], CED);
				UA_SetValToCycleGadgetVal(&TOC_GR[ID], &toc_rec->stopFadeOut);
				break;

			case 36:
				UA_ProcessCycleGadget(window, &TOC_GR[ID], CED);
				UA_SetValToCycleGadgetVal(&TOC_GR[ID], &toc_rec->fadeInSecs);
				break;

			case 37:
				UA_ProcessCycleGadget(window, &TOC_GR[ID], CED);
				UA_SetValToCycleGadgetVal(&TOC_GR[ID], &toc_rec->fadeOutSecs);
				break;

			case 38:
				UA_ProcessStringGadget(window, TOC_GR, &TOC_GR[ID], CED);
				UA_SetValToStringGadgetVal(&TOC_GR[ID], &toc_rec->setVolFrom);
				if ( toc_rec->setVolFrom<0 || toc_rec->setVolFrom>100 )
				{
					toc_rec->setVolFrom=100;
					GiveMessage(window, msgs[Msg_X_4-1], 0, 100); // "Enter a value between...
					UA_SetStringGadgetToVal(window, &TOC_GR[ID], 100);
				}
				break;

			case 39:
				UA_ProcessStringGadget(window, TOC_GR, &TOC_GR[ID], CED);
				UA_SetValToStringGadgetVal(&TOC_GR[ID], &toc_rec->setVolTo);
				if ( toc_rec->setVolTo<0 || toc_rec->setVolTo>100 )
				{
					toc_rec->setVolTo=100;
					GiveMessage(window, msgs[Msg_X_4-1], 0, 100); // "Enter a value between...
					UA_SetStringGadgetToVal(window, &TOC_GR[ID], 100);
				}
				break;

			case 40:
				UA_ProcessCycleGadget(window, &TOC_GR[ID], CED);
				UA_SetValToCycleGadgetVal(&TOC_GR[ID], &toc_rec->setVolSecs);
				break;
		}
	}
}

/******** DoPreview() ********/

void DoPreview(	struct Window *window, struct Toccata_Record *toc_rec,
								struct EventData *CED, UWORD *mypattern1, BOOL toccataPresent,
								struct ToccataBase *ToccataBase )
{
ULONG tocSig=0L, sigs;
LONG tocNum, val;
BOOL loop=TRUE;
int i,w,j;
struct TagItem tocTags[] =
{
	{ TT_FileName, NULL		},
	{ TT_ErrorTask, NULL	},
	{ TT_ErrorMask, NULL	},
	{ TT_Window, NULL			},
	{ TAG_DONE						},
};
struct TagItem tocTags2[10];
UBYTE perc_to_db[] =
{
 0, 
 0,  0,  0,  1,  1,  1,  1,  1,  1,  1, 
 2,  2,  2,  2,  2,  2,  3,  3,  3,  3, 
 3,  3,  4,  4,  4,  4,  4,  5,  5,  5, 
 5,  5,  6,  6,  6,  6,  6,  7,  7,  7, 
 7,  8,  8,  8,  8,  9,  9,  9,  9, 10, 
10, 10, 10, 11, 11, 11, 12, 12, 12, 13, 
13, 13, 14, 14, 15, 15, 15, 16, 16, 17, 
17, 18, 18, 19, 19, 20, 20, 21, 22, 22, 
23, 24, 25, 25, 26, 27, 28, 29, 31, 32, 
33, 35, 37, 39, 42, 45, 49, 54, 64 
};
struct GfxBase *GfxBase;

	// Create a signal mask different from capsport mask!
	// They might be the same because capsport comes from another planet...

	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0L);
	if ( !GfxBase )
	{
		CloseLibrary((struct Library *)GfxBase);
		return;
	}

	if ( window )
	{
		tocSig=0L;
		for(i=0; i<31; i++)
		{
			tocNum = AllocSignal( i );	
			if ( tocNum!=-1 )
			{
				tocSig = 1L << tocNum;
				if ( tocSig==(1L << capsport->mp_SigBit) )
					FreeSignal(tocNum);
				else
					break;
			}
		}

		UA_ClearButton(window, &TOC_GR[5], AREA_PEN);
		UA_DrawText(window, &TOC_GR[5], msgs[Msg_Stop-1]);
	}

	// START ALWAYS

	if ( toc_rec->mode==MODE_RECORD )
		tocTags[0].ti_Data	= (ULONG)toc_rec->recordPath;
	else
		tocTags[0].ti_Data	= (ULONG)toc_rec->playPath;

	// END ALWAYS

	if ( window )
	{
		tocTags[1].ti_Data	= (ULONG)FindTask(NULL);
		tocTags[2].ti_Data	= (ULONG)tocSig;
		tocTags[3].ti_Data	= (ULONG)window;
	}
	else
	{
		tocTags[1].ti_Tag		= TAG_DONE;
		tocTags[1].ti_Data	= 0L;
	}

	// START ALWAYS

	tocTags2[0].ti_Tag	= PAT_OutputVolumeLeft;
	tocTags2[1].ti_Tag	= PAT_OutputVolumeRight;
	tocTags2[2].ti_Tag	= TAG_DONE;

	if ( window )
		UA_SetSprite(window, SPRITE_BUSY);

	if (	toc_rec->mode==MODE_PLAY || toc_rec->mode==MODE_RECORD ||
				(window && toc_rec->mode==MODE_MISC) )
	{
		/**** SET VOL ****/
		if ( toc_rec->mode==MODE_PLAY || toc_rec->mode==MODE_RECORD )
		{
			if ( toc_rec->playFadeIn )	// start from 0 and go up
				val = 0;	// mute
			else
				val = toc_rec->playVolume;	// play at this level
		}
		else if ( toc_rec->mode==MODE_MISC )
		{
			if ( toc_rec->misc==MISC_STOP )
				val = toc_rec->playVolume;
			else if ( toc_rec->misc==MISC_FADEIN )
				val = 0;
			else if ( toc_rec->misc==MISC_FADEOUT )
				val = toc_rec->playVolume;
			else if ( toc_rec->misc==MISC_SETVOL )
				val = toc_rec->setVolFrom;
		}
		SetTheVol(ToccataBase,perc_to_db, val, tocTags2);

		/**** PLAY ****/
		if ( toccataPresent	)
			T_Playback( tocTags );

		/**** FADE ****/
		if (	(toc_rec->mode==MODE_PLAY || toc_rec->mode==MODE_RECORD)
					&& toc_rec->playFadeIn )	// start from 0 and go up
		{
			for(i=0; i<=toc_rec->playVolume; i++)
			{
				SetTheVol(ToccataBase,perc_to_db, i, tocTags2);
				for(w=0; w<toc_rec->playFadeIn/2; w++)
					WaitTOF();
			}
		}
	}
	if ( toc_rec->mode==MODE_MISC )
	{
		/**** STOP AND FADEOUT ****/
		if ( toc_rec->misc==MISC_STOP || toc_rec->misc==MISC_FADEOUT )
		{
			if (	( toc_rec->misc==MISC_STOP && toc_rec->stopFadeOut ) ||
						( toc_rec->misc==MISC_FADEOUT && toc_rec->fadeOutSecs ) )
			{
				if ( toc_rec->misc==MISC_STOP )
					j = toc_rec->stopFadeOut;
				else if ( toc_rec->misc==MISC_FADEOUT )
					j = toc_rec->fadeOutSecs;			
				for(i=toc_rec->playVolume; i>=0; i--)
				{
					SetTheVol(ToccataBase,perc_to_db, i, tocTags2);
					for(w=0; w<j/2; w++)
						WaitTOF();
				}
			}
			/**** set last volume value (in case of nod fade) ****/
			SetTheVol(ToccataBase,perc_to_db, 0, tocTags2);

			/**** STOP PART OF STOP ****/
			if ( toc_rec->misc==MISC_STOP )
				T_Stop(0L);
		}

		/**** FADE IN ****/
		if ( toc_rec->misc==MISC_FADEIN )
		{
			if ( toc_rec->fadeInSecs )
			{
				for(i=0; i<=toc_rec->playVolume; i++)
				{
					SetTheVol(ToccataBase,perc_to_db, i, tocTags2);
					for(w=0; w<toc_rec->fadeInSecs/2; w++)
						WaitTOF();
				}
			}
			SetTheVol(ToccataBase,perc_to_db, toc_rec->playVolume, tocTags2);
		}

		/**** FADING PART OF SET VOL ****/
		if ( toc_rec->misc==MISC_SETVOL )
		{
			if ( toc_rec->setVolSecs )
			{
				if ( toc_rec->setVolFrom > toc_rec->setVolTo )
				{
					for(i=toc_rec->setVolFrom; i>=toc_rec->setVolTo; i--)
					{
						SetTheVol(ToccataBase,perc_to_db, i, tocTags2);
						for(w=0; w<toc_rec->setVolSecs/2; w++)
							WaitTOF();
					}
				}
				else if ( toc_rec->setVolFrom < toc_rec->setVolTo )
				{
					for(i=toc_rec->setVolFrom; i<toc_rec->setVolTo; i++)
					{
						SetTheVol(ToccataBase,perc_to_db, i, tocTags2);
						for(w=0; w<toc_rec->setVolSecs/2; w++)
							WaitTOF();
					}
				}
			}
			SetTheVol(ToccataBase,perc_to_db, toc_rec->setVolTo, tocTags2);
		}
	}

	// END ALWAYS

	if ( toc_rec->mode==MODE_MISC && window )
		loop=FALSE;

	if ( window )
		UA_SetSprite(window, SPRITE_NORMAL);

	if ( window )
	{
		while( loop )
		{
			sigs = UA_doStandardWaitExtra( window, CED, tocSig );
			if ( sigs & (1L << capsport->mp_SigBit) )
			{
				if (CED->Class==MOUSEBUTTONS && CED->Code==SELECTDOWN)
				{
					if ( UA_CheckGadgetList(window, TOC_GR, CED) == 5 )
					{
						UA_HiliteButton(window, &TOC_GR[5]);
						loop = FALSE;
//KPrintF("IDCMP\n");
					}
				}
			}
			else if ( sigs & tocSig )
			{
				loop = FALSE;
//KPrintF("TOC\n");
			}
		}

		if ( toccataPresent )
			T_Stop( 0 );

		UA_ClearButton(window, &TOC_GR[5], AREA_PEN);
		UA_DrawText(window, &TOC_GR[5], msgs[Msg_Play-1]);

		if ( tocNum!=-1 )
			FreeSignal( tocNum );
	}

	CloseLibrary((struct Library *)GfxBase);
}

/******** DoRecord() ********/

void DoRecord(struct Window *window, struct Toccata_Record *toc_rec,
							struct EventData *CED, UWORD *mypattern1 )
{
struct Window *panel;
ULONG tocSig=0L, sigs;
LONG tocNum;
BOOL loop=TRUE, recording;
int ID,i;
struct TagItem tocTags[] =
{
	{ TT_FileName, NULL		},
	{ TT_ErrorTask, NULL	},
	{ TT_ErrorMask, NULL	},
	{ TT_Window, NULL			},
	{ TAG_DONE						},
};

	// Create a signal mask different from capsport mask!
	// They might be the same because capsport comes from another planet...

	tocSig=0L;
	for(i=0; i<31; i++)
	{
		tocNum = AllocSignal( i );	
		if ( tocNum!=-1 )
		{
			tocSig = 1L << tocNum;
			if ( tocSig==(1L << capsport->mp_SigBit) )
				FreeSignal(tocNum);
			else
				break;
		}
	}

	panel = (struct Window *)UA_OpenRequesterWindow(window, Record_GR, USECOLORS);
	if ( panel )
	{
		UA_DrawGadgetList(panel,Record_GR);
		UA_DisableButton(panel, &Record_GR[4], mypattern1);	// Stop
		recording=FALSE;

		while( loop )
		{
			sigs = UA_doStandardWaitExtra( panel, CED, tocSig );
			if ( sigs & (1L << capsport->mp_SigBit) )
			{
				if (CED->Class==MOUSEBUTTONS && CED->Code==SELECTDOWN)
				{
					ID = UA_CheckGadgetList(panel, Record_GR, CED);
					if ( ID==2 )			// OK
					{
						UA_HiliteButton(panel, &Record_GR[2]);
						loop = FALSE;
					}
					else if ( ID==3 )	// Start recording
					{
						UA_HiliteButton(panel, &Record_GR[3]);							// Start
						UA_DisableButton(panel, &Record_GR[2], mypattern1);	// OK
						UA_DisableButton(panel, &Record_GR[3], mypattern1);	// Start
						UA_EnableButton(panel, &Record_GR[4]);							// Stop
						// START ACTUAL RECORDING
						tocTags[0].ti_Data = (LONG)toc_rec->recordPath;
						tocTags[1].ti_Data = (LONG)FindTask(NULL);
						tocTags[2].ti_Data = (LONG)tocSig;
						tocTags[3].ti_Data = (LONG)panel;
						if ( toccataPresent )
							T_Capture( tocTags );
						recording=TRUE;
					}
					else if ( ID==4 )	// Stop recording
					{
						if ( toccataPresent )
							T_Stop( 0 );
						recording=FALSE;
						UA_HiliteButton(panel, &Record_GR[4]);							// Stop
						UA_EnableButton(panel, &Record_GR[2]);							// OK
						UA_EnableButton(panel, &Record_GR[3]);							// Start
						UA_DisableButton(panel, &Record_GR[4], mypattern1);	// Stop
					}
				}
			}
			else if ( sigs & tocSig )
			{
				// TOCCATA SIGNALED US
				if ( recording )
				{
					recording=FALSE;
					if ( toccataPresent )
						T_Stop( 0 );
					UA_HiliteButton(panel, &Record_GR[4]);							// Stop
					UA_EnableButton(panel, &Record_GR[2]);							// OK
					UA_EnableButton(panel, &Record_GR[3]);							// Start
					UA_DisableButton(panel, &Record_GR[4], mypattern1);	// Stop
				}
			}
		}

		UA_CloseRequesterWindow(panel,USECOLORS);
	}

	if ( tocNum!=-1 )
		FreeSignal( tocNum );

	UA_EnableButtonQuiet(&Record_GR[2]);
	UA_EnableButtonQuiet(&Record_GR[3]);
	UA_EnableButtonQuiet(&Record_GR[4]);
}

/******* CB_Level() ********/
/*
 * This hook is called by the Toccata library
 *
 */

void __saveds __asm CB_Level( register __d0 ULONG Left, register __d1 ULONG Right )
{
int h;
struct GfxBase *GfxBase;
struct IntuitionBase *IntuitionBase;
struct Window *wdw;
struct GadgetRecord *TOC_GR;
struct Toccata_Record *toc_rec;
ULONG *ptrs;
UBYTE pen;
struct RastPort *rp;
 
	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0L);
	IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library",0L);

	wdw = IntuitionBase->ActiveWindow;
	if ( !wdw || !wdw->UserData )
		return;

	ptrs = (ULONG *)wdw->UserData;
	if ( ptrs[0] && ptrs[1] && ptrs[2] && !ptrs[3] )	// integrity check
	{
		TOC_GR = (struct GadgetRecord *)ptrs[0];
		toc_rec = (struct Toccata_Record *)ptrs[1];
		rp = (struct RastPort *)ptrs[2];
	}
	else
		return;

	/**** LEFT CHANNEL ****/

	if ( Left > 126 )
	{
		pen = 2;	// white
		Left = 126;
	}
	else
		pen = 1;	// black

	// Left goes from 0...126
	// Make it go from 0...100

	Left = ( Left * 100 ) / 126;

	h = TOC_GR[26].y2 - TOC_GR[26].y1 - 4;
	h = h * Left;
	h = h / 100;

	SetDrMd(rp,JAM1);

	SetAPen(rp,5L);
	RectFill(rp, TOC_GR[26].x1+3, TOC_GR[26].y1+2, TOC_GR[26].x2-3, TOC_GR[26].y2-2-h);

	SetAPen(rp,pen);
	RectFill(rp, TOC_GR[26].x1+3, TOC_GR[26].y2-2-h, TOC_GR[26].x2-3, TOC_GR[26].y2-2);

	/**** RIGHT CHANNEL ****/

	if ( Right > 126 )
	{
		pen = 2;	// white
		Right = 126;
	}
	else
		pen = 1;	// black

	// Right goes from 0...126
	// Make it go from 0...100

	Right = ( Right * 100 ) / 126;

	h = TOC_GR[27].y2 - TOC_GR[27].y1 - 4;
	h = h * Right;
	h = h / 100;

	SetDrMd(rp,JAM1);

	SetAPen(rp,5L);
	RectFill(rp, TOC_GR[27].x1+3, TOC_GR[27].y1+2, TOC_GR[27].x2-3, TOC_GR[27].y2-2-h);

	SetAPen(rp,pen);
	RectFill(rp, TOC_GR[27].x1+3, TOC_GR[27].y2-2-h, TOC_GR[27].x2-3, TOC_GR[27].y2-2);

	CloseLibrary((struct Library *)IntuitionBase);
	CloseLibrary((struct Library *)GfxBase);
}

/******** SetTheVol() ********/

void SetTheVol(struct ToccataBase *ToccataBase, UBYTE *perc_to_db, int val, struct TagItem *tags)
{
	if ( val==0 )
		val = 64;	// mute
	else
		val = perc_to_db[ 100-val ];	// 100-100=0->0 dB and 100-1=99->64 dB
	val = val * -1;
	tags[0].ti_Data	= val;
	tags[1].ti_Data	= val;
	T_SetPart( tags );
}

/******** E O F ********/

#if 0
	tag=0;
	sprintf(str,"PAT_MixAux1Left = %x\n", l[tag++]);
	KPrintF(str);
	sprintf(str,"PAT_MixAux1Right = %x\n", l[tag++]);
	KPrintF(str);
	sprintf(str,"PAT_MixAux2Left = %x\n", l[tag++]);
	KPrintF(str);
	sprintf(str,"PAT_MixAux2Right = %x\n", l[tag++]);
	KPrintF(str);
	sprintf(str,"PAT_InputVolumeLeft = %x\n", l[tag++]);
	KPrintF(str);
	sprintf(str,"PAT_InputVolumeRight = %x\n", l[tag++]);
	KPrintF(str);
	sprintf(str,"PAT_OutputVolumeLeft = %x\n", l[tag++]);
	KPrintF(str);
	sprintf(str,"PAT_OutputVolumeRight = %x\n", l[tag++]);
	KPrintF(str);
	sprintf(str,"PAT_LoopbackVolume = %x\n", l[tag++]);
	KPrintF(str);
	sprintf(str,"PAT_Input = %x\n", l[tag++]);
	KPrintF(str);
	sprintf(str,"PAT_MicGain = %x\n", l[tag++]);
	KPrintF(str);
	sprintf(str,"PAT_Mode = %x\n", l[tag++]);
	KPrintF(str);
	sprintf(str,"PAT_Frequency = %x\n", l[tag++]);
	KPrintF(str);
#endif
