#include "nb:pre.h"
#include "setup.h"
#include "protos.h"
#include "structs.h"

#define VERSION	"\0$VER: 1.4"
#define LOGFILE "ram:logfile"

/**** function declarations ****/

BOOL doYourThing(void);
BOOL MonitorUser(struct Window *window);
BOOL MonitorCreateRunTime(struct Window *window);
BOOL MonitorMissingFiles(struct Window *window);
BOOL MonitorCalcSize(struct Window *window);
void InitPropInfo(struct PropInfo *PI, struct Image *IM);
void Report(STRPTR report);
void WaitOnLastButton(struct Window *window);
void MyTurnAssignIntoDir(STRPTR ass);
int my_getpath(BPTR lock, char *path);

/**** globals ****/

static UBYTE *vers = VERSION;
UBYTE **msgs;
struct RendezVousRecord *rvrec;
struct EventData CED;
struct MsgPort *capsport;
FILE *logfile=NULL;
struct UserApplicInfo UAI;
TEXT reportStr[256];

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *medialinkLibBase;

/**** disable CTRL-C break ****/

int CXBRK(void) { return(0); }
void chkabort(void) { return; }

/**** functions ****/

/******** main() ********/

void main(int argc, char **argv)
{
struct MsgPort *port;
struct Node *node;
struct List *list;
struct Task *oldTask;

	/**** find the mother ship ****/

	port = (struct MsgPort *)FindPort(ML_RENDEZ_VOUS);
	if (port == NULL)
		exit(0);

	/**** meddle with task pointer ****/

	capsport = (struct MsgPort *)FindPort(MEDIALINKPORT);
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	/**** link with it ****/

	list = &(port->mp_MsgList);
	node = list->lh_Head;
	rvrec = (struct RendezVousRecord *)node->ln_Name;

	rvrec->returnCode = FALSE;

	/**** drain it ****/

	IntuitionBase 		= (struct IntuitionBase *)rvrec->intuition;
	GfxBase 					= (struct GfxBase *)rvrec->graphics;
	medialinkLibBase	= (struct Library *)rvrec->medialink;
	msgs							= (UBYTE **)rvrec->msgs;

	/**** open log file ****/

	logfile = (FILE *)fopen(LOGFILE,"w");

	/**** play around ****/

	doYourThing();

	/**** close log file -> delete it when it's empty ****/

	if ( logfile )
		fclose( logfile );

	if ( GetFileSize(LOGFILE)==0 )	// shoot to kill
		DeleteFile(LOGFILE);

	/**** delete temporary files ****/

	DeleteFile(BIGFILE);
	DeleteFile(TEMPSCRIPT);

	/**** and leave the show ****/

	capsport->mp_SigTask = oldTask;

	exit(0);
}

/******** doYourThing() ********/

BOOL doYourThing(void)
{
BOOL retval;
struct Process *process;

	/**** initialize the User Application Info structure ***/

	UA_InitStruct(&UAI);
	UAI.IB = IntuitionBase;

	/**** open a window ****/

	UAI.windowModes = 2;
	UAI.userScreen = rvrec->scriptscreen;

	/**** double the dimensions of gadgets etc. if screen is laced ****/

	if ( UA_IsUAScreenLaced(&UAI) )
	{
		UA_DoubleGadgetDimensions(SM1_GR);
		UA_DoubleGadgetDimensions(SM2_GR);
		UA_DoubleGadgetDimensions(SM3_GR);
		UA_DoubleGadgetDimensions(SM4_GR);
		UA_DoubleGadgetDimensions(SM5_GR);
		UA_DoubleGadgetDimensions(DiskReq_GR);
	}

	UA_TranslateGR(SM1_GR, msgs);
	UA_TranslateGR(SM2_GR, msgs);
	UA_TranslateGR(SM3_GR, msgs);
	UA_TranslateGR(SM4_GR, msgs);
	UA_TranslateGR(SM5_GR, msgs);
	// UA_TranslateGR(DiskReq_GR, msgs); done later

	UAI.windowX			 	= -1;
	UAI.windowY			 	= -1;
	UAI.windowWidth	 	= SM1_GR[0].x2;
	UAI.windowHeight 	= SM1_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;

	/**** set the right font for this window ****/

	UAI.small_TF = rvrec->smallfont;
	UAI.large_TF = rvrec->largefont;

	UA_OpenWindow(&UAI);

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, SM1_GR);
	UA_DrawGadgetList(UAI.userWindow, SM2_GR);

	/**** process events ****/

	Forbid();
	process = (struct Process *)FindTask(NULL);
	Permit();
	if ( process )
		process->pr_WindowPtr = UAI.userWindow;

	retval = MonitorUser(UAI.userWindow);

	if ( process )
		process->pr_WindowPtr = -1;

	/**** close the window ****/

	UA_CloseWindow(&UAI);

	return(retval);
}

/******** MonitorUser() ********/

BOOL MonitorUser(struct Window *window)
{
BOOL loop=TRUE, retval;
int ID, choice;

	/**** event handler ****/

	choice=0;	// create run-time
	UA_InvertButton(window, &SM2_GR[choice]);

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		switch(CED.Class)
		{
			case IDCMP_MOUSEBUTTONS:
				if (CED.Code==SELECTDOWN)
				{
					ID = UA_CheckGadgetList(window, SM2_GR, &CED);
					switch(ID)
					{
						case 0:
						case 1:
						case 2:
							UA_InvertButton(window, &SM2_GR[choice]);
							choice=ID;						
							UA_InvertButton(window, &SM2_GR[choice]);
							break;

						case 3:
do_ok:
							UA_HiliteButton(window, &SM2_GR[3]);
							loop=FALSE;
							retval=TRUE;
							break;

						case 4:
do_cancel:
							UA_HiliteButton(window, &SM2_GR[4]);
							loop=FALSE;
							retval=FALSE;
							break;
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

	if ( retval )
	{
		if ( choice==0 )	
			retval = MonitorCreateRunTime(window);
		else if ( choice==1 )
			retval = MonitorMissingFiles(window);
		else if ( choice==2 )
			retval = MonitorCalcSize(window);
	}

	return(retval);
}

/******** MonitorCreateRunTime() ********/

BOOL MonitorCreateRunTime(struct Window *window)
{
BOOL loop=TRUE, retval, syst, network=FALSE;
int ID, choice, len;
TEXT path[SIZE_FULLPATH], filename[SIZE_FULLPATH], temp[SIZE_FULLPATH], scriptName[SIZE_FULLPATH];
struct FileReqRecord FRR;
TEXT report[256];

	UA_ClearButton(window, &SM1_GR[4], AREA_PEN);	// clear whole window

	UA_DrawGadgetList(window, SM3_GR);
	SM3_GR[ 1 ].type = BUTTON_GADGET;

	choice=0;	// script and data
	UA_InvertButton(window, &SM3_GR[choice+2]);

	UA_ClearButton(window, &SM3_GR[1], AREA_PEN);
	strcpy(path, "RAM:");
	UA_ShortenString(window->RPort, path, (SM3_GR[1].x2-SM3_GR[1].x1)-16);
	UA_DrawText(window, &SM3_GR[1], path);

	UA_DrawSpecialGadgetText(window, &SM3_GR[2], msgs[Msg_SM_7-1], SPECIAL_TEXT_TOP);

	/**** set up file requester stuff ****/

	FRR.path			= path;
	FRR.fileName	= filename;
	FRR.title			= msgs[Msg_SelectPath-1];
	FRR.opts			= DIR_OPT_ONLYDIRS;
	FRR.multiple	= FALSE;

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		switch(CED.Class)
		{
			case IDCMP_MOUSEBUTTONS:
				if (CED.Code==SELECTDOWN)
				{
					ID = UA_CheckGadgetList(window, SM3_GR, &CED);
					switch(ID)
					{
						case 1:	// path
							UA_HiliteButton(window, &SM3_GR[ID]);
							if ( UA_OpenAFile(window, &FRR, mypattern1) )
							{
								if ( path[ (strlen(path)-1) ] == '/' )
									path[ (strlen(path)-1) ] = '\0';
								UA_TurnAssignIntoDir(path);
								if ( path[ (strlen(path)-1) ] == '/' )
									path[ (strlen(path)-1) ] = '\0';

								if ( !strnicmp(path,"RAM DISK",8) )	// change RAM DISK: into RAM:
								{
									sprintf(temp,"RAM:%s",&path[9]);	// start to copy AFTER 'RAM DISK:'
									strcpy(path,temp);
								}

								UA_ClearButton(window, &SM3_GR[1], AREA_PEN);
								strcpy(temp,path);
								UA_ShortenString(window->RPort, temp, (SM3_GR[1].x2-SM3_GR[1].x1)-16);
								UA_DrawText(window, &SM3_GR[1], temp);

								// START -- Check if we copy this over a network --
						
								UA_MakeFullPath(path,"RUNS_script.1",temp);
								if ( !TheFileIsNotThere(temp) )
								{
									// There is a 'RUNS_xxx' file.
									network=TRUE;
									UA_MakeFullPath(path,"Script.2",scriptName);	// scriptName to make
									strcpy(temp,path);
									UA_MakeFullPath(temp,"Script2",path);	// Script 1 runs -> Copy to script 2
									// Update GUI
									UA_ClearButton(window, &SM3_GR[1], AREA_PEN);
									strcpy(temp,path);
									UA_ShortenString(window->RPort, temp, (SM3_GR[1].x2-SM3_GR[1].x1)-16);
									UA_DrawText(window, &SM3_GR[1], temp);
								}
								else
								{
									UA_MakeFullPath(path,"RUNS_script.2",temp);
									if ( !TheFileIsNotThere(temp) )
									{
										// There is a 'RUNS_xxx' file.
										network=TRUE;
										UA_MakeFullPath(path,"Script.1",scriptName);	// scriptName to make
										strcpy(temp,path);
										UA_MakeFullPath(temp,"Script1",path);	// Script 2 runs -> Copy to script 1
										// Update GUI
										UA_ClearButton(window, &SM3_GR[1], AREA_PEN);
										strcpy(temp,path);
										UA_ShortenString(window->RPort, temp, (SM3_GR[1].x2-SM3_GR[1].x1)-16);
										UA_DrawText(window, &SM3_GR[1], temp);
									}
								}
						
								// END -- Check if we copy this over a network --
						
							}
							break;

						case 2:
						case 3:
							UA_InvertButton(window, &SM3_GR[choice+2]);
							choice=ID-2;						
							UA_InvertButton(window, &SM3_GR[choice+2]);
							break;

						case 4:
do_ok:
							UA_HiliteButton(window, &SM3_GR[4]);
							loop=FALSE;
							retval=TRUE;
							break;

						case 5:
do_cancel:
							UA_HiliteButton(window, &SM3_GR[5]);
							loop=FALSE;
							retval=FALSE;
							break;
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

	if ( retval )
	{
		UA_ClearButton(window, &SM1_GR[4], AREA_PEN);	// clear whole window
		UA_DrawGadgetList(window, SM5_GR);
		UA_DisableButton(window, &SM5_GR[1], mypattern1);

		// Creating run-time script
		strcpy(report,msgs[Msg_SM_22-1]);
		Report(report);

		if ( choice==0 )
			syst = SYSTEMFILES_NO;
		else
			syst = SYSTEMFILES_YES;

		// rvrec->aPtr contains script name
		if ( network )
			retval = CreateBigFile(	rvrec->aPtr,BIGFILE,TEMPSCRIPT,
															MODE_CREATE_RUNTIME,path,syst,scriptName);
		else
			retval = CreateBigFile(	rvrec->aPtr,BIGFILE,TEMPSCRIPT,
															MODE_CREATE_RUNTIME,path,syst,NULL);
		if ( retval )
			retval = InterpretBigFile(BIGFILE, MODE_CREATE_RUNTIME, NULL, NULL, NULL, syst);
		else
		{
			// Some files can't be found
			strcpy(report,msgs[Msg_SM_39-1]);
			Report(report);
			// Try to fix it first
			strcpy(report,msgs[Msg_SM_40-1]);
			Report(report);
		}

		// Ready
		strcpy(report,msgs[Msg_SM_24-1]);
		Report(report);

		// Try to remove the .MP suffix

		SplitFullPath(rvrec->aPtr,temp,filename);
		UA_MakeFullPath(path,filename,temp);
		strcpy(path,temp);	// temp is oldname
		len = strlen(path);	
		if ( len>3 && !strnicmp( &path[len-3],".MP",3 ) )
			path[len-3] = '\0';	// path is newname
		Rename(temp,path);
		// Rename .info too 
		strcat(temp,".info");
		strcat(path,".info");
		Rename(temp,path);

		WaitOnLastButton(window);
	}

	return(retval);
}

/******** MonitorMissingFiles() ********/

BOOL MonitorMissingFiles(struct Window *window)
{
BOOL loop=TRUE, retval;
int ID,i,topEntry;
TEXT path[SIZE_FULLPATH], filename[SIZE_FULLPATH], temp[SIZE_FULLPATH];
struct FileListInfo *FLI;
UBYTE *scanDevs[50];
struct ScrollRecord SR;
UBYTE selectionList[50];
struct FileReqRecord FRR;
TEXT report[256];
struct Process *process;
struct Window *CLI_window;
TEXT cmdStr[512];

	for(i=0; i<50; i++)
	{
		scanDevs[i] = NULL;
		selectionList[i] = FALSE;
	}

	UA_ClearButton(window, &SM1_GR[4], AREA_PEN);	// clear whole window

	UA_DrawGadgetList(window, SM4_GR);
	SM4_GR[ 1 ].type = BUTTON_GADGET;

	getcd(0,path);

	UA_ClearButton(window, &SM4_GR[1], AREA_PEN);
	UA_ShortenString(window->RPort, path, (SM4_GR[1].x2-SM4_GR[1].x1)-16);
	UA_DrawText(window, &SM4_GR[1], path);

	UA_DrawSpecialGadgetText(window, &SM4_GR[2], msgs[Msg_SM_12-1], SPECIAL_TEXT_TOP);

	/**** get devices ****/

	if ( !rvrec->aPtrTwo )
		return(FALSE);
	else
		FLI = (struct FileListInfo *)rvrec->aPtrTwo;

	for(i=0; i<FLI->numDevices; i++)
		scanDevs[i] = FLI->deviceList+i*SIZE_FILENAME;

	/**** set up scroll bar ****/

	PropSlider.LeftEdge	= SM4_GR[3].x1+4;
	PropSlider.TopEdge	= SM4_GR[3].y1+2;
	PropSlider.Width		= SM4_GR[3].x2-SM4_GR[3].x1-7;
	PropSlider.Height		= SM4_GR[3].y2-SM4_GR[3].y1-3;

	if ( UA_IsWindowOnLacedScreen(window) )
	{
		PropSlider.TopEdge += 2;
		PropSlider.Height -= 4;
	}

	InitPropInfo(	(struct PropInfo *)PropSlider.SpecialInfo,
								(struct Image *)PropSlider.GadgetRender);
	AddGadget(window, &PropSlider, -1L);

	topEntry = 0;
	UA_SetPropSlider(window, &PropSlider, FLI->numDevices, 6, topEntry);

	/**** init scroll record ****/

	SR.GR							= &SM4_GR[2];
	SR.window					= window;
	SR.list						= NULL;
	SR.sublist				= NULL;
	SR.selectionList	= selectionList;
	SR.entryWidth			= -1;
	SR.numDisplay			= 6;
	SR.numEntries			= FLI->numDevices;

	UA_PrintStandardList(NULL,-1,NULL);	// init static
	UA_PrintStandardList(&SR,topEntry,scanDevs);

	/**** set up file requester stuff ****/

	FRR.path			= path;
	FRR.fileName	= filename;
	FRR.title			= msgs[Msg_SelectPath-1];
	FRR.opts			= DIR_OPT_ONLYDIRS;
	FRR.multiple	= FALSE;

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if ( CED.extraClass==IDCMP_GADGETDOWN || CED.extraClass==IDCMP_GADGETUP )
		{
			UA_ScrollStandardList(&SR,&topEntry,&PropSlider,scanDevs,&CED);
		}
		else if ( CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN )
		{
			ID = UA_CheckGadgetList(window, SM4_GR, &CED);
			switch(ID)
			{
				case 1:	// path
					UA_HiliteButton(window, &SM4_GR[ID]);
					if ( UA_OpenAFile(window, &FRR, mypattern1) )
					{
						if ( path[ (strlen(path)-1) ] == '/' )
							path[ (strlen(path)-1) ] = '\0';
						UA_ClearButton(window, &SM4_GR[1], AREA_PEN);
						strcpy(temp,path);
						UA_ShortenString(window->RPort, temp, (SM4_GR[1].x2-SM4_GR[1].x1)-16);
						UA_DrawText(window, &SM4_GR[1], temp);
					}
					break;

				case 2:	// device list area
					UA_SelectStandardListLine(&SR,topEntry,TRUE,&CED,FALSE,FALSE);
					break;

				case 4:
do_ok:
					UA_HiliteButton(window, &SM4_GR[4]);
					loop=FALSE;
					retval=TRUE;
					break;

					case 5:
do_cancel:
					UA_HiliteButton(window, &SM4_GR[5]);
					loop=FALSE;
					retval=FALSE;
					break;
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if ( CED.Code==RAW_RETURN )
				goto do_ok;
			else if ( CED.Code==RAW_ESCAPE )
				goto do_cancel;
		}
	}

	if ( retval )
	{
		UA_ClearButton(window, &SM1_GR[4], AREA_PEN);	// clear whole window
		UA_DrawGadgetList(window, SM5_GR);
		UA_DisableButton(window, &SM5_GR[1], mypattern1);

		// Disable system requesters
		Forbid();
		process = (struct Process *)FindTask(NULL);
		Permit();
		if ( process )
		{
			CLI_window = process->pr_WindowPtr;
			process->pr_WindowPtr = -1;
		}

		// Find missing files...
		strcpy(report,msgs[Msg_SM_23-1]);
		Report(report);

		OpenDiskList();
		if ( OpenScanMem() )
		{
			PlaceInList(path);
			for(i=0; i<FLI->numDevices; i++)
				if ( selectionList[i] )
					PlaceInList(scanDevs[i]);

			// rvrec->aPtr contains script name
			retval = CreateBigFile(	rvrec->aPtr,BIGFILE,TEMPSCRIPT,
															MODE_FIND_MISSING,NULL,SYSTEMFILES_NO,NULL );
			if ( retval )
			{
				retval = InterpretBigFile(BIGFILE, MODE_FIND_MISSING,
																	NULL, NULL, NULL, SYSTEMFILES_NO);
				if ( retval )
				{
					// Rename "ram:MP_TempScript" to eg "ram:my_script.fixed"
					SplitFullPath(rvrec->aPtr,path,filename);
					strcat(filename,".fixed");
					UA_MakeFullPath("ram:",filename,path);
					Rename(TEMPSCRIPT,path);	// oldname,newname

					// Copy .info file of original
					sprintf(temp,"%s.info",path);
					sprintf(path,"%s.info",rvrec->aPtr);
					sprintf(cmdStr, "copy \"%s\" to \"%s\"", path, temp);
					ExecuteCommand(cmdStr);
				}
			}

			CloseScanMem();
			CloseDiskList();
		}

		// Ready
		strcpy(report,msgs[Msg_SM_24-1]);
		Report(report);

		// Enable system requesters again
		if ( process )
			process->pr_WindowPtr = CLI_window;

		WaitOnLastButton(window);
	}

	if ( retval )
		rvrec->returnCode = TRUE;

	return(retval);
}

/******** MonitorCalcSize() ********/

BOOL MonitorCalcSize(struct Window *window)
{
BOOL retval;
int systemSize, dataSize, largest;
TEXT report[256];
struct Process *process;
struct Window *CLI_window;

	UA_ClearButton(window, &SM1_GR[4], AREA_PEN);	// clear whole window
	UA_DrawGadgetList(window, SM5_GR);
	UA_DisableButton(window, &SM5_GR[1], mypattern1);

	// Disable system requesters
	Forbid();
	process = (struct Process *)FindTask(NULL);
	Permit();
	if ( process )
	{
		CLI_window = process->pr_WindowPtr;
		process->pr_WindowPtr = -1;
	}

	// rvrec->aPtr contains script name
	retval = CreateBigFile(	rvrec->aPtr,BIGFILE,TEMPSCRIPT,
													MODE_CALC_SIZE,NULL, SYSTEMFILES_YES,NULL );
	if ( retval )
	{
		retval = InterpretBigFile(BIGFILE, MODE_CALC_SIZE,
															&systemSize, &dataSize, &largest, SYSTEMFILES_YES);
		if ( retval )
		{
			// "Total size of data files: %d Kb"
			sprintf(report,msgs[Msg_SM_25-1],dataSize/1024);
			Report(report);

			// "Total size of system files: %d Kb"
			sprintf(report,msgs[Msg_SM_26-1],systemSize/1024);
			Report(report);

			// "Largest file: %d Kb"
			sprintf(report,msgs[Msg_SM_27-1],largest/1024);
			Report(report);
		}
		else
		{
			// Some files can't be found
			strcpy(report,msgs[Msg_SM_39-1]);
			Report(report);
			// Try to fix it first
			strcpy(report,msgs[Msg_SM_40-1]);
			Report(report);
		}
	}

	// Ready
	strcpy(report,msgs[Msg_SM_24-1]);
	Report(report);

	// Enable system requesters again
	if ( process )
		process->pr_WindowPtr = CLI_window;

	WaitOnLastButton(window);

	return(retval);
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

/******** Report() ********/

void Report(STRPTR report)
{
struct Window *window;
int x,w,pct;
TEXT repstr[256];

	//printser(report);	printser("\n");

	strcpy(repstr,report);
	window = UAI.userWindow;

	if (window->WScreen->ViewPort.Modes & LACE)
		SetFont(window->RPort, rvrec->tiny_largefont);
	else
		SetFont(window->RPort, rvrec->tiny_smallfont);

	// update the status bar or print a text

	if ( isdigit( repstr[0] ) )
	{
		SetAPen(window->RPort, BGND_PEN);
		sscanf(repstr,"%d",&pct);
		if (pct)
		{
			if (pct>100)
				pct=100;
			w = SM5_GR[2].x2 - SM5_GR[2].x1 - 6;
			x = w * pct;
			x /= 100;
			if ( ( SM5_GR[2].x1 + x ) > ( SM5_GR[2].x1 + w ) )
				x = w;
			RectFill(window->RPort, SM5_GR[2].x1+3, SM5_GR[2].y1+2, SM5_GR[2].x1+3+x,	SM5_GR[2].y2-2);
		}
	}
	else
	{
		SetAPen(window->RPort, LO_PEN);
		SetBPen(window->RPort, AREA_PEN);
		ScrollRaster(	window->RPort, 0, window->RPort->TxBaseline+5,
									SM5_GR[0].x1+2, SM5_GR[0].y1+2,
									SM5_GR[0].x2-2, SM5_GR[0].y2-2); 
		UA_ShortenString(window->RPort, repstr, (SM5_GR[0].x2-SM5_GR[0].x1)-32);
		Move(window->RPort, SM5_GR[0].x1+5, SM5_GR[0].y2-window->RPort->TxBaseline);
		Text(window->RPort, repstr, strlen(repstr));
	}

	if (window->WScreen->ViewPort.Modes & LACE)
		SetFont(window->RPort, rvrec->largefont);
	else
		SetFont(window->RPort, rvrec->smallfont);
}

/******** WaitOnLastButton() ********/

void WaitOnLastButton(struct Window *window)
{
	if ( logfile )
	{
		fclose( logfile );
		logfile=NULL;
		if ( GetFileSize(LOGFILE)!=0 )
		{
			// "The file "ram:logfile" lists all missing files"
			strcpy(reportStr,msgs[Msg_SM_35-1]);
			Report(reportStr);
		}
	}

	UA_EnableButton(window, &SM5_GR[1]);
	while(1)
	{
		UA_doStandardWait(window,&CED);
		if ( CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN )
		{
			if ( UA_CheckGadgetList(window, SM5_GR, &CED) == 1 )
			{
do_ok:
				UA_HiliteButton(window, &SM5_GR[1]);
				return;
			}
		}
		else if ( CED.Class==IDCMP_RAWKEY && CED.Code==RAW_RETURN )
			goto do_ok;
	}
}

/******** E O F ********/
