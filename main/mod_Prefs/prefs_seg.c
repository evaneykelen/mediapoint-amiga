#include "nb:pre.h"
#include "prefs_gadgets.h"
#include "protos.h"

#define VERSION				"\0$VER: 2.4"
#define PLAYER_UNITS	"devs:player-units"
#define PLAYER_DEV		"devs:player.device"

/**** globals ****/

struct RendezVousRecord *rvrec;
struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *medialinkLibBase;
struct Library *ConsoleDevice;
static UBYTE *vers = VERSION;
struct EventData CED;
BOOL PDINFO;
UWORD chip mypattern1[] = { 0x5555, 0xaaaa };
struct FileListInfo FLI;
struct FileReqRecord FRR;
UBYTE **msgs;
ULONG *languagesAvailable;
int lan;
struct MsgPort *capsport;
 
/**** disable CTRL-C break ****/

int CXBRK(void) { return(0); }
void chkabort(void) { return; }

/**** functions ****/

/******** main() ********/

int main(int argc, char **argv)
{
struct MsgPort *port;
struct Node *node;
struct List *list;
struct Task *oldTask;

	/**** find the mother ship ****/

	port = (struct MsgPort *)FindPort(ML_RENDEZ_VOUS);
	if (port == NULL)
		return(0);

	/**** meddle with task pointer ****/

	capsport = (struct MsgPort *)FindPort(MEDIALINKPORT);
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	/**** link with it ****/

	list = &(port->mp_MsgList);
	node = list->lh_Head;
	rvrec = (struct RendezVousRecord *)node->ln_Name;

	/**** drain it ****/

	IntuitionBase 			= (struct IntuitionBase *)rvrec->intuition;
	GfxBase 						= (struct GfxBase *)rvrec->graphics;
	medialinkLibBase		= (struct Library *)rvrec->medialink;
	ConsoleDevice				= (struct Library *)rvrec->console;
	msgs								= (UBYTE **)rvrec->msgs;
	languagesAvailable	= (ULONG *)rvrec->aPtr;

	/**** translate gadgets ****/

	UA_TranslateGR(SelMon_GR,msgs);

	/**** set up FRR ****/

	FRR.path			= NULL; 
	FRR.fileName	= NULL;
	FRR.title			= NULL;
	FRR.opts			= DIR_OPT_ALL;
	FRR.multiple	= FALSE;

	/**** play around ****/

	if ( doYourThing() )
		rvrec->returnCode = TRUE;
	else
		rvrec->returnCode = FALSE;

	/**** and leave the show ****/

	capsport->mp_SigTask = oldTask;

	return(0);
}

/******** doYourThing() ********/

BOOL doYourThing(void)
{
struct UserApplicInfo UAI;
BOOL retval;

	/**** initialize the User Application Info structure ***/

	UA_InitStruct(&UAI);
	UAI.IB = IntuitionBase;

	/**** open a window ****/

	UAI.windowModes = 2;
	if (rvrec->ehi->activeScreen == STARTSCREEN_PAGE)
		UAI.userScreen = rvrec->pagescreen;
	else if (rvrec->ehi->activeScreen == STARTSCREEN_SCRIPT)
		UAI.userScreen = rvrec->scriptscreen;
	else
		return(FALSE);

	/**** double the dimensions of gadgets etc. if screen is laced ****/

	if ( UA_IsUAScreenLaced(&UAI) )
	{
		UA_DoubleGadgetDimensions(Prefs_GR);
		UA_DoubleGadgetDimensions(Prefs_1_GR);
		UA_DoubleGadgetDimensions(Prefs_3_GR);
		UA_DoubleGadgetDimensions(Prefs_4_GR);
		UA_DoubleGadgetDimensions(Prefs_5_GR);
	}

	UAI.windowX			 	= -1;
	UAI.windowY			 	= -1;
	UAI.windowWidth	 	= Prefs_GR[0].x2;
	UAI.windowHeight 	= Prefs_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;

	/**** set the right font for this window ****/

	UAI.small_TF = rvrec->smallfont;
	UAI.large_TF = rvrec->largefont;

	UA_OpenWindow(&UAI);
	if ( rvrec->ehi->activeScreen == STARTSCREEN_PAGE )
		UA_SetMenuColors(rvrec,UAI.userWindow);

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, Prefs_GR);

	/**** process events ****/

	retval = MonitorUser(UAI.userWindow);

	if ( rvrec->ehi->activeScreen == STARTSCREEN_PAGE )
		UA_ResetMenuColors(rvrec,UAI.userWindow);

	/**** close the window ****/

	UA_CloseWindow(&UAI);

	return(retval);
}

/******** MonitorUser() ********/

BOOL MonitorUser(struct Window *window)
{
BOOL loop=TRUE, retval;
int ID,screen=1,maxScreens=4,oldLan;
struct CapsPrefs *copy_CPrefs;
UBYTE *undoHomeDirs, *undoHomePaths;

	/**** allocate memory for undo ****/

	copy_CPrefs = (struct CapsPrefs *)AllocMem(sizeof(struct CapsPrefs), MEMF_ANY);
	if (copy_CPrefs==NULL)
		return(FALSE);
	CopyMem(rvrec->capsprefs, copy_CPrefs, sizeof(struct CapsPrefs));

	undoHomeDirs = (UBYTE *)AllocMem(10*SIZE_FILENAME, MEMF_ANY | MEMF_CLEAR);
	if (undoHomeDirs==NULL)
		return(FALSE);
	CopyMem(rvrec->homeDirs, undoHomeDirs, 10*SIZE_FILENAME);

	undoHomePaths = (UBYTE *)AllocMem(10*SIZE_FULLPATH, MEMF_ANY | MEMF_CLEAR);
	if (undoHomePaths==NULL)
		return(FALSE);
	CopyMem(rvrec->homePaths, undoHomePaths, 10*SIZE_FULLPATH);

	oldLan = rvrec->capsprefs->lanCode;
	lan = LanToCycle(rvrec->capsprefs->lanCode);

	RenderPrefsWindow(screen, window);

	PDINFO = GetPlayerDeviceInfo();
	if ( !PDINFO )
		maxScreens--;

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		switch(CED.Class)
		{
			case IDCMP_MOUSEBUTTONS:
				if (CED.Code==SELECTDOWN)
				{
					ID = UA_CheckGadgetList(window, Prefs_GR, &CED);
					switch(ID)
					{
						case 2:	// previous
							UA_HiliteButton(window, &Prefs_GR[ID]);
							if (screen==1)
								screen=maxScreens;
							else
								screen--;
							RenderPrefsWindow(screen, window);
							break;

						case 3:	// next
							UA_HiliteButton(window, &Prefs_GR[ID]);
							if (screen==maxScreens)
								screen=1;
							else
								screen++;
							RenderPrefsWindow(screen, window);
							break;

						case 4:	// OK
do_ok:
							UA_HiliteButton(window, &Prefs_GR[4]);
							loop=FALSE;
							retval=TRUE;
							break;

						case 5:	// Cancel
do_cancel:
							UA_HiliteButton(window, &Prefs_GR[5]);
							loop=FALSE;
							retval=FALSE;
							break;
					}

					if ( loop )
					{
						if (screen==1)
							ID = UA_CheckGadgetList(window, Prefs_1_GR, &CED);
						else if (screen==2)
							ID = UA_CheckGadgetList(window, Prefs_3_GR, &CED);
						else if (screen==3)
							ID = UA_CheckGadgetList(window, Prefs_4_GR, &CED);
						else if (screen==4)
							ID = UA_CheckGadgetList(window, Prefs_5_GR, &CED);
						MonitorPrefsScreen(screen, ID, window);
					}
				}
				break;

			case IDCMP_RAWKEY:
				if ( CED.Code==RAW_RETURN )
					goto do_ok;
				else if ( CED.Code==RAW_ESCAPE )
					goto do_cancel;
				break;
		}
	}

	if (!retval)
	{
		CopyMem(copy_CPrefs, rvrec->capsprefs, sizeof(struct CapsPrefs));
		CopyMem(undoHomeDirs, rvrec->homeDirs, 10*SIZE_FILENAME);
		CopyMem(undoHomePaths, rvrec->homePaths, 10*SIZE_FULLPATH);
		if (rvrec->ehi->activeScreen == STARTSCREEN_SCRIPT)
			TakeNextColor(window);	// restore color set
	}
	else
	{
		if ( PDINFO )
			PutPlayerDeviceInfo();
	}

	FreeMem(undoHomeDirs, 10*SIZE_FILENAME);
	FreeMem(undoHomePaths, 10*SIZE_FULLPATH);
	FreeMem(copy_CPrefs, sizeof(struct CapsPrefs));

	return(retval);
}

/******** RenderPrefsWindow() ********/

void RenderPrefsWindow(int screen, struct Window *window)
{
TEXT buf[512];
int value;
struct CycleRecord *CR;
TEXT monitorName[50];

	UA_ClearButton(window, &Prefs_GR[6], AREA_PEN);

	if (screen==1)
	{
		Prefs_1_GR[4].type = HIBOX_REGION;
		Prefs_1_GR[5].type = HIBOX_REGION;
		Prefs_1_GR[6].type = HIBOX_REGION;

		UA_DrawGadgetList(window, Prefs_1_GR);

		Prefs_1_GR[4].type = BUTTON_GADGET;
		Prefs_1_GR[5].type = BUTTON_GADGET;
		Prefs_1_GR[6].type = BUTTON_GADGET;
	}
	else if (screen==2)
		UA_DrawGadgetList(window, Prefs_3_GR);
	else if (screen==3)
		UA_DrawGadgetList(window, Prefs_4_GR);
	else if (screen==4)
		UA_DrawGadgetList(window, Prefs_5_GR);

	if (screen==1)
	{
		UA_SetCycleGadgetToVal(window, &Prefs_1_GR[0], rvrec->capsprefs->userLevel-1);

		UA_SetCycleGadgetToVal(window, &Prefs_1_GR[1], rvrec->capsprefs->colorSet);

		UA_SetCycleGadgetToVal(window, &Prefs_1_GR[2], lan);

		if ( rvrec->capsprefs->WorkBenchOn )
			UA_SetCycleGadgetToVal(window, &Prefs_1_GR[3], 0);	// on
		else
			UA_SetCycleGadgetToVal(window, &Prefs_1_GR[3], 1);	// off

		// START NEW

		UA_ClearButton(window, &Prefs_1_GR[4], AREA_PEN);
		if ( rvrec->capsprefs->scriptMonName[0]!='\0' )
		{
			strcpy(monitorName,rvrec->capsprefs->scriptMonName);
			UA_ShortenString(window->RPort, monitorName, Prefs_1_GR[4].x2 - Prefs_1_GR[4].x1 - 16);
		}
		else
			strcpy(monitorName,msgs[Msg_P_DefaultMon-1]);
		UA_DrawSpecialGadgetText(window, &Prefs_1_GR[4], monitorName, SPECIAL_TEXT_CENTER);

		UA_ClearButton(window, &Prefs_1_GR[5], AREA_PEN);
		if ( rvrec->capsprefs->pageMonName[0]!='\0' )
		{
			strcpy(monitorName,rvrec->capsprefs->pageMonName);
			UA_ShortenString(window->RPort, monitorName, Prefs_1_GR[5].x2 - Prefs_1_GR[5].x1 - 16);
		}
		else
			strcpy(monitorName,msgs[Msg_P_DefaultMon-1]);
		UA_DrawSpecialGadgetText(window, &Prefs_1_GR[5], monitorName, SPECIAL_TEXT_CENTER);

		UA_ClearButton(window, &Prefs_1_GR[6], AREA_PEN);
		if ( rvrec->capsprefs->playerMonName[0]!='\0' )
		{
			strcpy(monitorName,rvrec->capsprefs->playerMonName);
			UA_ShortenString(window->RPort, monitorName, Prefs_1_GR[6].x2 - Prefs_1_GR[6].x1 - 16);
		}
		else
			strcpy(monitorName,msgs[Msg_P_DefaultMon-1]);
		UA_DrawSpecialGadgetText(window, &Prefs_1_GR[6], monitorName, SPECIAL_TEXT_CENTER);

		UA_SetCycleGadgetToVal(window, &Prefs_1_GR[7], rvrec->capsprefs->thumbnailSize-1);

		if ( rvrec->capsprefs->ThumbnailScreenDepth < 4 )
			rvrec->capsprefs->ThumbnailScreenDepth = 4;
		UA_SetCycleGadgetToVal(window, &Prefs_1_GR[8], rvrec->capsprefs->ThumbnailScreenDepth-4);
		CR = (struct CycleRecord *)Prefs_1_GR[8].ptr;
		if ( !rvrec->capsprefs->AA_available )
		{
			CR->number = 1;	// limit thumbnails to 16 colors
			rvrec->capsprefs->ThumbnailScreenDepth = 4;	// 16 colors
			UA_SetCycleGadgetToVal(window, &Prefs_1_GR[8], 0);
		}

		// END NEW
	}
	else if (screen==2)
	{
		UA_SetStringGadgetToString(window, &Prefs_3_GR[1], rvrec->capsprefs->F1_TIMECODE_STR);
		UA_SetStringGadgetToString(window, &Prefs_3_GR[2], rvrec->capsprefs->F2_TIMECODE_STR);
		UA_SetStringGadgetToString(window, &Prefs_3_GR[3], rvrec->capsprefs->F3_TIMECODE_STR);
		UA_SetStringGadgetToString(window, &Prefs_3_GR[4], rvrec->capsprefs->F4_TIMECODE_STR);
		UA_SetStringGadgetToString(window, &Prefs_3_GR[5], rvrec->capsprefs->F5_TIMECODE_STR);
		UA_SetStringGadgetToString(window, &Prefs_3_GR[6], rvrec->capsprefs->F6_TIMECODE_STR);
	}
	else if (screen==3)
	{
		UA_DrawSpecialGadgetText(window, &Prefs_4_GR[4], msgs[Msg_Descrip-1], SPECIAL_TEXT_TOP);
		UA_DrawSpecialGadgetText(window, &Prefs_4_GR[5], msgs[Msg_Descrip-1], SPECIAL_TEXT_TOP);

		UA_DrawSpecialGadgetText(window, &Prefs_4_GR[6], msgs[Msg_Path-1], SPECIAL_TEXT_TOP);
		UA_DrawSpecialGadgetText(window, &Prefs_4_GR[7], msgs[Msg_Path-1], SPECIAL_TEXT_TOP);

		UA_SetValToCycleGadgetVal(&Prefs_4_GR[2], &value);
		UA_ClearButton(window, &Prefs_4_GR[4], AREA_PEN);
		UA_DrawSpecialGadgetText(	window, &Prefs_4_GR[4],
															msgs[Msg_DefaultList-1]+value*21,
															SPECIAL_TEXT_LEFT);

		if (value==0)
			stccpy(buf, rvrec->capsprefs->import_picture_Path, SIZE_PATH);
		else if (value==1)
			stccpy(buf, rvrec->capsprefs->anim_Path, SIZE_PATH);
		else if (value==2)
			stccpy(buf, rvrec->capsprefs->music_Path, SIZE_PATH);
		else if (value==3)
			stccpy(buf, rvrec->capsprefs->sample_Path, SIZE_PATH);
		else if (value==4)
			stccpy(buf, rvrec->capsprefs->import_text_Path, SIZE_PATH);
		else if (value==5)
			stccpy(buf, rvrec->capsprefs->document_Path, SIZE_PATH);
		else if (value==6)
			stccpy(buf, rvrec->capsprefs->script_Path, SIZE_PATH);

		UA_ShortenStringFront(window->RPort, buf, Prefs_4_GR[6].x2 - Prefs_4_GR[6].x1 - 16);
		UA_ClearButton(window, &Prefs_4_GR[6], AREA_PEN);
		UA_DrawSpecialGadgetText(window, &Prefs_4_GR[6], buf, SPECIAL_TEXT_LEFT);

		UA_SetValToCycleGadgetVal(&Prefs_4_GR[3], &value);
		UA_SetStringGadgetToString(	window, &Prefs_4_GR[5],
																rvrec->homeDirs+value*SIZE_FILENAME);

		strcpy(buf, rvrec->homePaths+value*SIZE_FULLPATH);
		UA_ShortenStringFront(window->RPort, buf, Prefs_4_GR[7].x2 - Prefs_4_GR[7].x1 - 16);
		UA_ClearButton(window, &Prefs_4_GR[7], AREA_PEN);
		UA_DrawSpecialGadgetText(window, &Prefs_4_GR[7], buf, SPECIAL_TEXT_LEFT);
	}
	else if (screen==4)
	{
		if ( rvrec->capsprefs->PDevice.playerName[0]=='\0' )
			UA_DrawSpecialGadgetText(window, &Prefs_5_GR[1], msgs[Msg_NoDeviceSelected-1], SPECIAL_TEXT_LEFT);
		else
			UA_DrawSpecialGadgetText(window, &Prefs_5_GR[1], rvrec->capsprefs->PDevice.playerName, SPECIAL_TEXT_LEFT);
		if (rvrec->capsprefs->PDevice.baudRate != -1)
		{
			if (rvrec->capsprefs->PDevice.baudRate==1200)
				value=0;
			else if (rvrec->capsprefs->PDevice.baudRate==2400)
				value=1;
			else if (rvrec->capsprefs->PDevice.baudRate==4800)
				value=2;
			else if (rvrec->capsprefs->PDevice.baudRate==9600)
				value=3;
			UA_SetCycleGadgetToVal(window, &Prefs_5_GR[2], value);
		}
		UA_SetStringGadgetToString(window, &Prefs_5_GR[3], rvrec->capsprefs->PDevice.deviceName);
		UA_SetCycleGadgetToVal(window, &Prefs_5_GR[4], rvrec->capsprefs->PDevice.unit);
	}
}

/******** MonitorPrefsScreen() ********/

void MonitorPrefsScreen(int screen, int ID, struct Window *window)
{
int value,i,width,height;
TEXT filename[SIZE_FILENAME], buf[512];
BOOL *MeddledWithPaths, laced;
ULONG modeID;
TEXT monitorName[50];

	filename[0] = '\0';
	buf[0] = '\0';

	if (screen==1)
	{
		switch(ID)
		{
			case 0:
			case 1:
			case 2:
			case 3:
				UA_ProcessCycleGadget(window, &Prefs_1_GR[ID], &CED);
				UA_SetValToCycleGadgetVal(&Prefs_1_GR[ID], &value);

				if (ID==0)
				{
					rvrec->capsprefs->userLevel = value+1;
				}
				else if (ID==1)
				{
					rvrec->capsprefs->colorSet = value;
					TakeNextColor(window);
				}
				else if (ID==2)
				{
					lan = value;	// lan is a global, keeps track of changes
					rvrec->capsprefs->lanCode = CycleToLan(lan);
				}
				else if (ID==3)
				{
					if (value==0)
						rvrec->capsprefs->WorkBenchOn	=	TRUE;
					else
						rvrec->capsprefs->WorkBenchOn	=	FALSE;
				}
				break;

			case 4:
			case 5:
			case 6:
				if ( ID==4 )	// script
				{
					modeID = rvrec->capsprefs->scriptMonitorID;
					strcpy(monitorName, rvrec->capsprefs->scriptMonName);
					laced = TRUE;
					width = 640;
					height = -1;
				}
				else if ( ID==5 )
				{
					modeID = rvrec->capsprefs->pageMonitorID;
					strcpy(monitorName, rvrec->capsprefs->pageMonName);
					laced = FALSE;
					width = 640;
					height = 580;
				}
				else if ( ID==6 )
				{
					modeID = rvrec->capsprefs->playerMonitorID;
					strcpy(monitorName, rvrec->capsprefs->playerMonName);
					laced = FALSE;
					width = -1;
					height = -1;
				}
				UA_InvertButton(window, &Prefs_1_GR[ID]);
				if ( SelectMonitor(window,&modeID,laced,width,height,monitorName) )
				{
					UA_InvertButton(window, &Prefs_1_GR[ID]);

					UA_ShortenString(window->RPort, monitorName, Prefs_1_GR[ID].x2 - Prefs_1_GR[ID].x1 - 16);
					UA_ClearButton(window, &Prefs_1_GR[ID], AREA_PEN);
					UA_DrawSpecialGadgetText(window, &Prefs_1_GR[ID], monitorName, SPECIAL_TEXT_CENTER);

					if ( ID==4 )
					{
						rvrec->capsprefs->scriptMonitorID = modeID;
						strcpy(rvrec->capsprefs->scriptMonName, monitorName);
					}
					else if ( ID==5 )
					{
						rvrec->capsprefs->pageMonitorID = modeID;
						strcpy(rvrec->capsprefs->pageMonName, monitorName);
					}
					else if ( ID==6 )
					{
						rvrec->capsprefs->playerMonitorID = modeID;
						strcpy(rvrec->capsprefs->playerMonName, monitorName);
					}
				}
				else
					UA_InvertButton(window, &Prefs_1_GR[ID]);
				break;

			case 7:
			case 8:
				UA_ProcessCycleGadget(window, &Prefs_1_GR[ID], &CED);
				UA_SetValToCycleGadgetVal(&Prefs_1_GR[ID], &value);
				if (ID==7)
					rvrec->capsprefs->thumbnailSize = value+1;
				else if (ID==8)
					rvrec->capsprefs->ThumbnailScreenDepth = value+4;
				break;
		}
	}
	else if (screen==2)
	{
		switch(ID)
		{
			case 1:	// time codes
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
				UA_ProcessStringGadget(window, Prefs_3_GR, &Prefs_3_GR[ID], &CED);
				ValidateTimeCode(&Prefs_3_GR[ID], window);
				if (ID==1)
					UA_SetStringToGadgetString(	&Prefs_3_GR[ID],
																			rvrec->capsprefs->F1_TIMECODE_STR);
				else if (ID==2)
					UA_SetStringToGadgetString(	&Prefs_3_GR[ID],
																			rvrec->capsprefs->F2_TIMECODE_STR);
				else if (ID==3)
					UA_SetStringToGadgetString(	&Prefs_3_GR[ID],
																			rvrec->capsprefs->F3_TIMECODE_STR);
				else if (ID==4)
					UA_SetStringToGadgetString(	&Prefs_3_GR[ID],
																			rvrec->capsprefs->F4_TIMECODE_STR);
				else if (ID==5)
					UA_SetStringToGadgetString(	&Prefs_3_GR[ID],
																			rvrec->capsprefs->F5_TIMECODE_STR);
				else if (ID==6)
					UA_SetStringToGadgetString(	&Prefs_3_GR[ID],
																			rvrec->capsprefs->F6_TIMECODE_STR);
				break;
		}
	}
	else if (screen==3)
	{
		switch(ID)
		{
			case 2:	// cycle tru default paths
				UA_ProcessCycleGadget(window, &Prefs_4_GR[2], &CED);
				// print description
				UA_ClearButton(window, &Prefs_4_GR[4], AREA_PEN);
				UA_SetValToCycleGadgetVal(&Prefs_4_GR[2], &value);
				UA_DrawSpecialGadgetText(	window, &Prefs_4_GR[4],
																	msgs[Msg_DefaultList-1]+value*21,
																	SPECIAL_TEXT_LEFT);
				// print path
				if (value==0)
					stccpy(buf, rvrec->capsprefs->import_picture_Path, SIZE_PATH);
				else if (value==1)
					stccpy(buf, rvrec->capsprefs->anim_Path, SIZE_PATH);
				else if (value==2)
					stccpy(buf, rvrec->capsprefs->music_Path, SIZE_PATH);
				else if (value==3)
					stccpy(buf, rvrec->capsprefs->sample_Path, SIZE_PATH);
				else if (value==4)
					stccpy(buf, rvrec->capsprefs->import_text_Path, SIZE_PATH);
				else if (value==5)
					stccpy(buf, rvrec->capsprefs->document_Path, SIZE_PATH);
				else if (value==6)
					stccpy(buf, rvrec->capsprefs->script_Path, SIZE_PATH);
				UA_ShortenStringFront(window->RPort, buf, Prefs_4_GR[6].x2 - Prefs_4_GR[6].x1 - 16);
				UA_ClearButton(window, &Prefs_4_GR[6], AREA_PEN);
				UA_DrawSpecialGadgetText(window, &Prefs_4_GR[6], buf, SPECIAL_TEXT_LEFT);
				break;

			case 3:	// cycle tru home paths
				UA_ProcessCycleGadget(window, &Prefs_4_GR[3], &CED);
				// print description
				UA_SetValToCycleGadgetVal(&Prefs_4_GR[3], &value);
				UA_SetStringGadgetToString(	window, &Prefs_4_GR[5],
																		rvrec->homeDirs+value*SIZE_FILENAME);
				strcpy(buf, rvrec->homePaths+value*SIZE_FULLPATH);
				UA_ShortenStringFront(window->RPort, buf, Prefs_4_GR[7].x2 - Prefs_4_GR[7].x1 - 16);
				UA_ClearButton(window, &Prefs_4_GR[7], AREA_PEN);
				UA_DrawSpecialGadgetText(window, &Prefs_4_GR[7], buf, SPECIAL_TEXT_LEFT);
				break;

			case 5:	// enter home description
				UA_ProcessStringGadget(window, Prefs_4_GR, &Prefs_4_GR[5], &CED);
				UA_SetValToCycleGadgetVal(&Prefs_4_GR[3], &value);
				UA_SetStringToGadgetString(	&Prefs_4_GR[5],
																		rvrec->homeDirs+value*SIZE_FILENAME );
				break;

			case 6:	// choose default path
				MeddledWithPaths = (BOOL *)rvrec->aPtrTwo;
				*MeddledWithPaths = TRUE;
				UA_HiliteButton(window, &Prefs_4_GR[6]);
				UA_SetValToCycleGadgetVal(&Prefs_4_GR[2], &value);
				if (value==0)
					strcpy(buf, rvrec->capsprefs->import_picture_Path);
				else if (value==1)
					strcpy(buf, rvrec->capsprefs->anim_Path);
				else if (value==2)
					strcpy(buf, rvrec->capsprefs->music_Path);
				else if (value==3)
					strcpy(buf, rvrec->capsprefs->sample_Path);
				else if (value==4)
					strcpy(buf, rvrec->capsprefs->import_text_Path);
				else if (value==5)
					strcpy(buf, rvrec->capsprefs->document_Path);
				else if (value==6)
					strcpy(buf, rvrec->capsprefs->script_Path);
				FRR.path			= buf; 
				FRR.fileName	= filename;
				FRR.title			= msgs[Msg_SelectPath-1];
				FRR.opts			= DIR_OPT_ONLYDIRS;
				if ( UA_OpenAFile(window, &FRR, mypattern1) )
				{
					if (value==0)
						stccpy(rvrec->capsprefs->import_picture_Path, buf, SIZE_PATH);
					else if (value==1)
						stccpy(rvrec->capsprefs->anim_Path, buf, SIZE_PATH);
					else if (value==2)
						stccpy(rvrec->capsprefs->music_Path, buf, SIZE_PATH);
					else if (value==3)
						stccpy(rvrec->capsprefs->sample_Path, buf, SIZE_PATH);
					else if (value==4)
						stccpy(rvrec->capsprefs->import_text_Path, buf, SIZE_PATH);
					else if (value==5)
						stccpy(rvrec->capsprefs->document_Path, buf, SIZE_PATH);
					else if (value==6)
						stccpy(rvrec->capsprefs->script_Path, buf, SIZE_PATH);
					UA_ShortenStringFront(window->RPort, buf, Prefs_4_GR[6].x2 - Prefs_4_GR[6].x1 - 16);
					UA_ClearButton(window, &Prefs_4_GR[6], AREA_PEN);
					UA_DrawSpecialGadgetText(window, &Prefs_4_GR[6], buf, SPECIAL_TEXT_LEFT);
				}
				break;

			case 7:	// choose home path
				UA_HiliteButton(window, &Prefs_4_GR[7]);
				UA_SetValToCycleGadgetVal(&Prefs_4_GR[3], &value);
				strcpy(buf, rvrec->homePaths+value*SIZE_FULLPATH);
				FRR.path			= buf; 
				FRR.fileName	= filename;
				FRR.title			= msgs[Msg_SelectPath-1];
				FRR.opts			= DIR_OPT_ONLYDIRS;
				if ( UA_OpenAFile(window, &FRR, mypattern1) )
				{
					strcpy(rvrec->homePaths+value*SIZE_FULLPATH, buf);
					UA_ShortenStringFront(window->RPort, buf, Prefs_4_GR[7].x2 - Prefs_4_GR[7].x1 - 16);
					UA_ClearButton(window, &Prefs_4_GR[7], AREA_PEN);
					UA_DrawSpecialGadgetText(window, &Prefs_4_GR[7], buf, SPECIAL_TEXT_LEFT);
				}
				break;
		}
	}
	else if (screen==4)
	{
		switch(ID)
		{
			case 1:
				UA_HiliteButton(window, &Prefs_5_GR[ID]);
				stccpy(buf, "devs:players/", 100);

				FRR.path			= buf; 
				FRR.fileName	= rvrec->capsprefs->PDevice.playerName;
				FRR.title			= msgs[Msg_SelectDevice-1];
				FRR.opts			= DIR_OPT_ALL;
				if ( UA_OpenAFile(window, &FRR, mypattern1) )
				{
					stccpy(buf, rvrec->capsprefs->PDevice.playerName, 100);
					UA_ShortenStringFront(window->RPort, buf, Prefs_5_GR[ID].x2 - Prefs_5_GR[ID].x1 - 16);
					UA_ClearButton(window, &Prefs_5_GR[ID], AREA_PEN);
					UA_DrawSpecialGadgetText(window, &Prefs_5_GR[ID], buf, SPECIAL_TEXT_LEFT);
					PDINFO=TRUE;
				}
				break;

			case 2:
				UA_ProcessCycleGadget(window, &Prefs_5_GR[ID], &CED);
				UA_SetValToCycleGadgetVal(&Prefs_5_GR[ID], &value);
				if (value==0)
					rvrec->capsprefs->PDevice.baudRate=1200;
				else if (value==1)
					rvrec->capsprefs->PDevice.baudRate=2400;
				else if (value==2)
					rvrec->capsprefs->PDevice.baudRate=4800;
				else if (value==3)
					rvrec->capsprefs->PDevice.baudRate=9600;
				break;

			case 3:
				UA_ProcessStringGadget(window, Prefs_5_GR, &Prefs_5_GR[ID], &CED);
				UA_SetStringToGadgetString(	&Prefs_5_GR[ID],
																		rvrec->capsprefs->PDevice.deviceName);
				break;

			case 4:
				UA_ProcessCycleGadget(window, &Prefs_5_GR[ID], &CED);
				UA_SetValToCycleGadgetVal(&Prefs_5_GR[ID], &value);
				rvrec->capsprefs->PDevice.unit = value;
				break;
		}
	}
}

/******** ValidateTimeCode() ********/

void ValidateTimeCode(struct GadgetRecord *GR, struct Window *window)
{
struct StringRecord *SR_ptr;
int hours, mins, secs, frames, i;
TEXT tmp[16];
int maxFF=29;

	hours		= 0;
	mins		= 0;
	secs		= 0;
	frames	= 0;

	SR_ptr = (struct StringRecord *)GR->ptr;
	if (SR_ptr == NULL)
		return;

	stccpy(tmp, SR_ptr->buffer, 16);

	/**** remove - . etc ****/	
	for(i=0; i<strlen(tmp); i++)
		if ( isdigit(tmp[i]) == 0 )
			tmp[i]=' ';

	sscanf(tmp, "%d %d %d %d", &hours, &mins, &secs, &frames);

	if ( hours > 99 )
		hours = 99;

	if ( mins > 59 )
		mins = 59;

	if ( secs > 59 )
		secs = 59;

	if ( frames > maxFF )
		frames = maxFF;

	sprintf(SR_ptr->buffer, "%02d:%02d:%02d:%02d", hours, mins, secs, frames);

	UA_SetStringGadgetToString(window, GR, SR_ptr->buffer);
}

/******** TakeNextColor() ********/

void TakeNextColor(struct Window *window)
{
	LoadRGB4(	&(window->WScreen->ViewPort),
						&rvrec->paletteList[rvrec->capsprefs->colorSet*8], 8L);
}

/******** GetPlayerDeviceInfo() ********/

BOOL GetPlayerDeviceInfo(void)
{
FILE *fp;
int dummy;

	/* 0 Sony_1200_9600 9600 serial.device 3 */

	fp = fopen(PLAYER_DEV, "r");
	if (fp==NULL)
		return(FALSE);
	else
		fclose(fp);

	fp = fopen(PLAYER_UNITS, "r");
	if (fp==NULL)
		return(TRUE);	// there's a device but no devs:player_units

	fscanf(	fp, "%d %s %d %s %d\n",
					&dummy,
					rvrec->capsprefs->PDevice.playerName,
					&rvrec->capsprefs->PDevice.baudRate,
					rvrec->capsprefs->PDevice.deviceName,
					&rvrec->capsprefs->PDevice.unit );
	fclose(fp);

	return(TRUE);
}

/******** PutPlayerDeviceInfo() ********/

BOOL PutPlayerDeviceInfo(void)
{
FILE *fp;

	/* 0 Sony_1200_9600 9600 serial.device 3 */

	fp = fopen(PLAYER_UNITS, "w");
	if (fp==NULL)
		return(FALSE);
	fprintf(fp, "%d %s %d %s %d\n",
					0,
					rvrec->capsprefs->PDevice.playerName,
					rvrec->capsprefs->PDevice.baudRate,
					rvrec->capsprefs->PDevice.deviceName,
					rvrec->capsprefs->PDevice.unit );
	fclose(fp);

	return(TRUE);
}

/******** LanToCycle() ********/

int LanToCycle(int lanCode)
{
	switch(lanCode)
	{
		case  1:	return( 2); break;
		case  2:	return( 6); break;
		case  3:	return( 1); break;
		case  4:	return( 4); break;
		case  5:	return( 3); break;
		case  6:	return( 5); break;
		case  7:	return( 8); break;
		case  8:	return( 0); break;
		case  9:	return( 9); break;
		case 10:	return( 7); break;
	}
}

/******** CycleToLan() ********/

int CycleToLan(int cycle)
{
	switch(cycle)
	{
		case  2:	return( 1); break;
		case  6:	return( 2); break;
		case  1:	return( 3); break;
		case  4:	return( 4); break;
		case  3:	return( 5); break;
		case  5:	return( 6); break;
		case  8:	return( 7); break;
		case  0:	return( 8); break;
		case  9:	return( 9); break;
		case  7:	return(10); break;
	}
}

/******** InitPropInfo() ********/
/*
 * This functions should eliminate the problems with prop gadgets
 * which are used in non-lace and lace environments and write over
 * the container in which they live.
 *
 */

void InitPropInfo(struct PropInfo *PI, struct Image *IM)
{
	PI->VertPot = 0;
	PI->VertBody = 0;
	PI->CHeight = 0;
	PI->VPotRes = 0;

	IM->Height = 0;
	IM->Depth = 0;
	IM->ImageData = NULL;
	IM->PlanePick	= 0x0000;
	IM->PlaneOnOff = 0x0000;
	IM->NextImage = NULL;
}

/******** E O F ********/
