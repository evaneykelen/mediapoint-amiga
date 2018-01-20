#include "nb:pre.h"
#include "setup.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"
#include "setup.h"
#include "structs.h"
#include "sample.h"
#include "loadsamp.h"

#define VERSI0N "\0$VER: 1.3"
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

void preview_sample(	struct Sample_record *sr, struct GadgetRecord *GR,
											struct Window *waitWindow, struct EventData *CED,
											ULONG sigbit );

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

	if (UA_HostScreenPresent(&UAI))
		UAI.windowModes = 1;	/* open on the MediaLink screen */
	else
		UAI.windowModes = 3;	/* open on the first (frontmost) screen */

	/**** double the dimensions of gadgets etc. if screen is laced ****/

	if (UA_IsUAScreenLaced(&UAI))
	{
		UA_DoubleGadgetDimensions(Sample1_GR);
		UA_DoubleGadgetDimensions(Sample2_GR);
		UA_DoubleGadgetDimensions(Sample3_GR);
		UA_DoubleGadgetDimensions(Sample4_GR);
		UA_DoubleGadgetDimensions(Sample5_GR);
	}

	UA_TranslateGR(Sample1_GR, msgs);
	UA_TranslateGR(Sample2_GR, msgs);
	UA_TranslateGR(Sample3_GR, msgs);
	UA_TranslateGR(Sample4_GR, msgs);
	UA_TranslateGR(Sample5_GR, msgs);

	/**** open the window ****/

	capsport = (struct MsgPort *)FindPort(MEDIALINKPORT);
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	UAI.windowX				= -1;	/* -1 means center on screen */
	UAI.windowY				= -1;	/* -1 means center on screen */
	UAI.windowWidth		= Sample1_GR[0].x2;
	UAI.windowHeight	= Sample1_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(&UAI);

	/**** monitor events ****/

	MonitorUser(UAI.userWindow, ThisPI);

	/**** close the window ****/

	{
		UWORD *led;
		led = 0xbfe000;
		*led &= ~2; // switch led on again
	}

	UA_CloseWindow(&UAI);

	capsport->mp_SigTask = oldTask;
}

/******** MonitorUser() ********/

BOOL MonitorUser(struct Window *window, PROCESSINFO *ThisPI)
{
BOOL loop=TRUE, retVal;
struct EventData CED;
int ID;
UWORD *mypattern1;
struct Sample_record sample_rec;
struct FileReqRecord FRR;
TEXT path[SIZE_FULLPATH], filename[SIZE_FULLPATH];

	/**** init vars, alloc memory ****/

	mypattern1 = (UWORD *)AllocMem(4L, MEMF_CHIP);
	if (mypattern1==NULL)
		return(FALSE);
	mypattern1[0] = 0x5555;
	mypattern1[1] = 0xaaaa;

	sample_rec.action				= SAMPLE_PLAY;
	sample_rec.filename[0]	= '\0';
	sample_rec.loops				= DEFAULT_LOOPS;
	sample_rec.volume				= DEFAULT_VOL;
	sample_rec.freq					= DEFAULT_FREQ;
	sample_rec.playFadeIn		= DEFAULT_FADE;
	sample_rec.balance			= DEFAULT_BALANCE;
	sample_rec.fadeOut			= DEFAULT_FADEINOUT;
	sample_rec.fadeIn				= DEFAULT_FADEINOUT;
	sample_rec.setVolume		= DEFAULT_SETVOL;
	sample_rec.trackPlay		= 0;
	sample_rec.trackStop		= 0;
	sample_rec.trackFadeIn	= 0;
	sample_rec.trackFadeOut	= 0;
	sample_rec.trackSetVol	= 0;
	sample_rec.playFromDisk	= 0;
	sample_rec.filter				= 0;	// filter is off
	sample_rec.soundBox			= 0;	// soundBox is off by default

	/**** set up FRR ****/

	strcpy(path, rvrec->capsprefs->sample_Path);
	filename[0] = '\0';

	FRR.path							= path;
	FRR.fileName					= filename;
	FRR.title							= msgs[Msg_S16_8-1]; // "Select a sample file:"
	FRR.opts							= DIR_OPT_SAMPLE | DIR_OPT_NOINFO;
	FRR.multiple					= FALSE;

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
	{
		GetVarsFromPI(&sample_rec, ThisPI);
		if ( sample_rec.action==SAMPLE_PLAY )	// path available
			UA_SplitFullPath(sample_rec.filename, path, filename);
	}

	/**** event handler ****/

	DrawPage((BYTE)sample_rec.action,window,&sample_rec,mypattern1);
	SetButtons((BYTE)sample_rec.action, window, &sample_rec, mypattern1);

	while(loop)
	{
		UA_doStandardWait(window,&CED);
		if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, Sample1_GR, &CED);
			switch(ID)
			{
				case 2:		// action cycle
					UA_ProcessCycleGadget(window, &Sample1_GR[ID], &CED);
					UA_SetValToCycleGadgetVal(&Sample1_GR[ID], &sample_rec.action); 
					sample_rec.action++;
					DrawPage((BYTE)sample_rec.action,window,&sample_rec,mypattern1);
					SetButtons((BYTE)sample_rec.action, window, &sample_rec, mypattern1);
					break;

				case 8:		// Preview
					UA_HiliteButton(window, &Sample1_GR[ID]);
					UA_ClearButton(window, &Sample1_GR[ID], AREA_PEN);
					UA_DrawText(window, &Sample1_GR[ID], msgs[Msg_Stop-1]);

					preview_sample( &sample_rec, Sample1_GR, window, &CED, (1L<<rvrec->capsport->mp_SigBit) );

					UA_ClearButton(window, &Sample1_GR[ID], AREA_PEN);
					UA_DrawText(window, &Sample1_GR[ID], msgs[Msg_Play-1]);
					break;

				case 9:		// OK
do_ok:
					UA_HiliteButton(window, &Sample1_GR[9]);
					retVal=TRUE;
					loop=FALSE;
					break;

				case 10:	// Cancel
do_cancel:
					UA_HiliteButton(window, &Sample1_GR[10]);
					retVal=FALSE;
					loop=FALSE;
					break;
			}
			CheckOtherButtons(window, sample_rec.action, &CED, &sample_rec, &FRR, mypattern1);
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE)				// cancel
				goto do_cancel;
			else if (CED.Code==RAW_RETURN)	// OK
				goto do_ok;
		}
	}

	if ( retVal )
	{
		PutVarsToPI(&sample_rec, ThisPI);

		if ( sample_rec.action == SAMPLE_PLAY )
			ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[0] = 2;
		else
			ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[0] = 1;
	}

	FreeMem(mypattern1, 4L);

	return(retVal);
}

/******** DrawPage() ********/

void DrawPage(BYTE page, struct Window *window, struct Sample_record *sample_rec,
							UWORD *mypattern1)
{
	UA_EnableButtonQuiet(&Sample1_GR[8]);

	Sample1_GR[11].type = INVISIBLE_GADGET;
	Sample1_GR[11].color = 2;
	UA_ClearButton(window, &Sample1_GR[11], AREA_PEN);
	Sample1_GR[11].type = HIBOX_REGION;
	Sample1_GR[11].color = 1;
	UA_ClearButton(window, &Sample1_GR[12], AREA_PEN);

	UA_DrawGadget(window,&Sample1_GR[ 1]);
	UA_DrawGadget(window,&Sample1_GR[ 2]);
	UA_DrawGadget(window,&Sample1_GR[ 8]);
	UA_DrawGadget(window,&Sample1_GR[ 9]);
	UA_DrawGadget(window,&Sample1_GR[10]);

	if ( sample_rec->filename[0]=='\0' || page != SAMPLE_PLAY )
		UA_DisableButton(window, &Sample1_GR[8], mypattern1);

	switch(page)
	{
		case SAMPLE_PLAY:
			Sample1_GR[11].type = HIBOX_REGION;
			Sample1_GR[16].type = INVISIBLE_GADGET;
			UA_DrawGadgetList(window, Sample1_GR);
			Sample1_GR[11].type = BUTTON_GADGET;
			Sample1_GR[16].type = BUTTON_GADGET;
			UA_DrawSpecialGadgetText(window, &Sample1_GR[6], msgs[Msg_X_I_6-1], SPECIAL_TEXT_AFTER_STRING);
			UA_DrawSliderNotches(window, &Sample1_GR[7], 1, 11, LO_PEN);
			UA_DrawSpecialGadgetText(window, &Sample1_GR[4], "%", SPECIAL_TEXT_AFTER_STRING);
			break;

		case SAMPLE_STOP:
			UA_DrawGadgetList(window, Sample5_GR);
			break;

		case SAMPLE_FADEOUT:
			UA_DrawGadgetList(window, Sample2_GR);
			UA_DrawSpecialGadgetText(window, &Sample2_GR[0], msgs[Msg_X_I_6-1], SPECIAL_TEXT_AFTER_STRING);
			break;

		case SAMPLE_FADEIN:
			UA_DrawGadgetList(window, Sample3_GR);
			UA_DrawSpecialGadgetText(window, &Sample3_GR[0], msgs[Msg_X_I_6-1], SPECIAL_TEXT_AFTER_STRING);
			break;

		case SAMPLE_SETVOL:
			UA_DrawGadgetList(window, Sample4_GR);
			UA_DrawSpecialGadgetText(window, &Sample4_GR[0], "%", SPECIAL_TEXT_AFTER_STRING);
			break;
	}
}

/******** SetButtons() ********/

void SetButtons(BYTE page, struct Window *window, struct Sample_record *sample_rec,
								UWORD *mypattern1)
{
TEXT name[SIZE_FULLPATH];
UWORD *led;

	led=0xbfe000;

	UA_SetCycleGadgetToVal(window, &Sample1_GR[2], page-1);

	switch(page)
	{
		case SAMPLE_PLAY:
//KPrintF("[%s]\n",sample_rec->filename);
			if ( sample_rec->filename[0]=='@' )
				strcpy(name, msgs[Msg_VarPath_5-1]);
			else if ( sample_rec->filename[0]=='\0' )
				strcpy(name, msgs[Msg_NoFileSelectedYet-1]);
			else
			{
				strcpy(name, sample_rec->filename);
				UA_ShortenString(window->RPort, name, (Sample1_GR[11].x2-Sample1_GR[11].x1)-16);
			}
			UA_ClearButton(window, &Sample1_GR[11], AREA_PEN);
			UA_DrawText(window, &Sample1_GR[11], name);
			UA_SetCycleGadgetToVal(window, &Sample1_GR[3], sample_rec->loops-1);
			UA_SetStringGadgetToVal(window, &Sample1_GR[4], sample_rec->volume);
			UA_SetStringGadgetToVal(window, &Sample1_GR[5], sample_rec->freq);
			UA_SetCycleGadgetToVal(window, &Sample1_GR[6], sample_rec->playFadeIn);
			UA_SetSliderGadg(window, &Sample1_GR[7], sample_rec->balance+5, 11, NULL, 0);
			UA_SetCycleGadgetToVal(window, &Sample1_GR[13], sample_rec->trackPlay);
			UA_ClearButton(window, &Sample1_GR[14], AREA_PEN);
			if ( sample_rec->playFromDisk )
				UA_InvertButton(window, &Sample1_GR[14]);
			UA_ClearButton(window, &Sample1_GR[15], AREA_PEN);
			if ( sample_rec->filter )
			{
				*led &= ~2;	// switch led on
				UA_InvertButton(window, &Sample1_GR[15]);
			}
			else
				*led |= 2;	// switch led off
			UA_ClearButton(window, &Sample1_GR[16], AREA_PEN);
			if ( sample_rec->soundBox )
				UA_InvertButton(window, &Sample1_GR[16]);
			break;

		case SAMPLE_STOP:
			UA_SetCycleGadgetToVal(window, &Sample5_GR[0], sample_rec->trackStop);
			break;

		case SAMPLE_FADEOUT:
			UA_SetCycleGadgetToVal(window, &Sample2_GR[0], sample_rec->fadeOut);
			UA_SetCycleGadgetToVal(window, &Sample2_GR[1], sample_rec->trackFadeOut);
			break;

		case SAMPLE_FADEIN:
			UA_SetCycleGadgetToVal(window, &Sample3_GR[0], sample_rec->fadeIn);
			UA_SetCycleGadgetToVal(window, &Sample3_GR[1], sample_rec->trackFadeIn);
			break;

		case SAMPLE_SETVOL:
			UA_SetStringGadgetToVal(window, &Sample4_GR[0], sample_rec->setVolume);
			UA_SetCycleGadgetToVal(window, &Sample4_GR[1], sample_rec->trackSetVol);
			break;
	}			

	if ( sample_rec->filename[0]=='@' )
	{
		if ( page==SAMPLE_PLAY )
			UA_DisableButton(window, &Sample1_GR[11], mypattern1);	// filename path
		UA_DisableButton(window, &Sample1_GR[8], mypattern1);	// preview
	}
}

/******** CheckOtherButtons() ********/

void CheckOtherButtons(	struct Window *window, BYTE page, struct EventData *CED,
												struct Sample_record *sample_rec,
												struct FileReqRecord *FRR, UWORD *mypattern1 )
{
int ID;
SoundInfo sinfo;
UWORD *led;

	led = 0xbfe000;

	switch(page)
	{
		case SAMPLE_PLAY:
			Sample1_GR[7].type = BUTTON_GADGET;
			ID = UA_CheckGadgetList(window, Sample1_GR, CED);
			Sample1_GR[7].type = HIBOX_REGION;
			switch(ID)
			{
				case 3:	// loops
					UA_ProcessCycleGadget(window, &Sample1_GR[ID], CED);
					UA_SetValToCycleGadgetVal(&Sample1_GR[ID], &(sample_rec->loops));
					sample_rec->loops+=1;
					break; 

				case 4:	// volume
					UA_ProcessStringGadget(window, Sample1_GR, &Sample1_GR[ID], CED);
					UA_SetValToStringGadgetVal(&Sample1_GR[ID], &(sample_rec->volume));
					if ( sample_rec->volume<0 || sample_rec->volume>100 )
					{
						GiveMessage(window, msgs[Msg_X_4-1], 0, 100); // "Enter a value between 0 and 100."
						sample_rec->volume=100;
						UA_SetStringGadgetToVal(window, &Sample1_GR[4], sample_rec->volume);						
					}
					break; 

				case 5:	// freq.
					UA_ProcessStringGadget(window, Sample1_GR, &Sample1_GR[ID], CED);
					UA_SetValToStringGadgetVal(&Sample1_GR[ID], &(sample_rec->freq));
					break; 

				case 6:	// fade in
					UA_ProcessCycleGadget(window, &Sample1_GR[ID], CED);
					UA_SetValToCycleGadgetVal(&Sample1_GR[ID], &(sample_rec->playFadeIn));
					break; 

				case 7:	// balance
					sample_rec->balance += 5;	// go from -5...+5 to 0...10
					UA_ProcessSliderGadg(	window, &Sample1_GR[ID], &(sample_rec->balance), 11,
																NULL, CED, NULL,NULL,0,0 );
					sample_rec->balance -= 5;	// go from 0...10 to -5...+5
					break;

				case 11:	// filename
					UA_HiliteButton(window, &Sample1_GR[ID]);
					if ( UA_OpenAFile(window, FRR, mypattern1) )
					{
						UA_MakeFullPath(FRR->path, FRR->fileName, sample_rec->filename);
						UA_EnableButton(window, &Sample1_GR[8]);	// preview

						sinfo.type = SI_DISK | SI_INFO;
						if ( loadsoundfile( &sinfo, sample_rec->filename,FALSE,(1L<<rvrec->capsport->mp_SigBit)) )
						{
							if ( sinfo.period != 0 )
								sample_rec->freq = 3579546 / sinfo.period;						
							else
								sample_rec->freq = DEFAULT_FREQ;
							sample_rec->volume = (sinfo.vol_right*100)/64;

							if ( sample_rec->volume == 0 )
								sample_rec->volume = 100;

							freesound( &sinfo );
						}

						SetButtons(sample_rec->action, window, sample_rec, mypattern1);
					}
					break;

				case 13:	// track
					UA_ProcessCycleGadget(window, &Sample1_GR[ID], CED);
					UA_SetValToCycleGadgetVal(&Sample1_GR[ID], &(sample_rec->trackPlay));
					break; 

				case 14:	// play from disk
					UA_InvertButton(window, &Sample1_GR[ID]);
					if ( sample_rec->playFromDisk )
						sample_rec->playFromDisk=FALSE;
					else
						sample_rec->playFromDisk=TRUE;
					break; 

				case 15:	// filter
					UA_InvertButton(window, &Sample1_GR[ID]);
					if ( sample_rec->filter )
					{
						sample_rec->filter=FALSE;
						*led |= 2;	// switch led off
					}
					else
					{
						sample_rec->filter=TRUE;
						*led &= ~2;	// switch led on
					}
					break; 

				case 16:	// soundBox
					UA_InvertButton(window, &Sample1_GR[ID]);
					if ( sample_rec->soundBox )
						sample_rec->soundBox=FALSE;
					else
						sample_rec->soundBox=TRUE;
					break; 
			}
			break;

		case SAMPLE_STOP:
			ID = UA_CheckGadgetList(window, Sample5_GR, CED);
			switch(ID)
			{
				case 0:	// track
					UA_ProcessCycleGadget(window, &Sample5_GR[ID], CED);
					UA_SetValToCycleGadgetVal(&Sample5_GR[ID], &(sample_rec->trackStop));
					break; 
			}
			break;

		case SAMPLE_FADEOUT:
			ID = UA_CheckGadgetList(window, Sample2_GR, CED);
			switch(ID)
			{
				case 0:	// seconds
					UA_ProcessCycleGadget(window, &Sample2_GR[ID], CED);
					UA_SetValToCycleGadgetVal(&Sample2_GR[ID], &(sample_rec->fadeOut));
					break; 

				case 1:	// track
					UA_ProcessCycleGadget(window, &Sample2_GR[ID], CED);
					UA_SetValToCycleGadgetVal(&Sample2_GR[ID], &(sample_rec->trackFadeOut));
					break; 
			}
			break;

		case SAMPLE_FADEIN:
			ID = UA_CheckGadgetList(window, Sample3_GR, CED);
			switch(ID)
			{
				case 0:	// seconds
					UA_ProcessCycleGadget(window, &Sample3_GR[ID], CED);
					UA_SetValToCycleGadgetVal(&Sample3_GR[ID], &(sample_rec->fadeIn));
					break; 

				case 1:	// track
					UA_ProcessCycleGadget(window, &Sample3_GR[ID], CED);
					UA_SetValToCycleGadgetVal(&Sample3_GR[ID], &(sample_rec->trackFadeIn));
					break; 
			}
			break;

		case SAMPLE_SETVOL:
			ID = UA_CheckGadgetList(window, Sample4_GR, CED);
			switch(ID)
			{
				case 0:	// volume
					UA_ProcessStringGadget(window, Sample4_GR, &Sample4_GR[ID], CED);
					UA_SetValToStringGadgetVal(&Sample4_GR[ID], &(sample_rec->setVolume));
					if ( sample_rec->setVolume<0 || sample_rec->setVolume>100 )
					{
						GiveMessage(window, msgs[Msg_X_4-1], 0, 100); // "Enter a value between 0 and 100."
						sample_rec->setVolume=100;
						UA_SetStringGadgetToVal(window, &Sample4_GR[0], sample_rec->setVolume);						
					}
					break; 

				case 1:	// track
					UA_ProcessCycleGadget(window, &Sample4_GR[ID], CED);
					UA_SetValToCycleGadgetVal(&Sample4_GR[ID], &(sample_rec->trackSetVol));
					break; 
			}
	}
}

/******** E O F ********/
