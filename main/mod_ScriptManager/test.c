#include "nb:pre.h"
#include "gadgets.h"
#include "protos.h"
#include "structs.h"

#define VERSION	"\0$VER: MediaPoint ScriptManager 1.1"
#define LOGFILE "ram:logfile"

/**** globals ****/

static UBYTE *vers = VERSION;
UBYTE **msgs;
struct RendezVousRecord *rvrec;
struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *medialinkLibBase;
struct UserApplicInfo UAI;
struct EventData CED;
UWORD chip mypattern1[] = { 0x5555, 0xaaaa };
struct MsgPort *capsport;
BYTE copyWhat = 0;
struct PropInfo PI = { AUTOKNOB | FREEVERT | PROPBORDERLESS, 0,0,0,0, };
struct Image Im = { 0,0,0,0,0,NULL,0x0000,0x0000,NULL };
struct Gadget PropSlider =
{
	NULL, 0,0,0,0, NULL, GACT_RELVERIFY | GACT_IMMEDIATE, GTYP_PROPGADGET,
	&Im, NULL, NULL, NULL, (struct PropInfo *)&PI, 2, NULL
};
struct GadgetRecord DiskReq_GR[] =
{
  7,  77,  89,  90, 0, NULL, Msg_OK,			BUTTON_GADGET, NULL,
230,  77, 312,  90, 0, NULL, Msg_Cancel,	BUTTON_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};
FILE *logfile = NULL;
int numErrors=0;
int diskNr=2;
TEXT oriDest[SIZE_FULLPATH];

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

	oriDest[0] = '\0';

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

	/**** play around ****/

	doYourThing();

	/**** and leave the show ****/

	capsport->mp_SigTask = oldTask;

	exit(0);
}

/******** doYourThing() ********/

BOOL doYourThing(void)
{
BOOL retval;

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
	}

	UA_TranslateGR(SM1_GR, msgs);
	UA_TranslateGR(SM2_GR, msgs);
	UA_TranslateGR(SM3_GR, msgs);
	UA_TranslateGR(SM4_GR, msgs);
	UA_TranslateGR(SM5_GR, msgs);

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

	logfile = (FILE *)fopen(LOGFILE,"w");
	
	retval = MonitorUser(UAI.userWindow);

	if ( logfile )
	{
		fclose( logfile );
		if ( GetFileSize(LOGFILE)==0 )	// shoot to kill
			DeleteFile(LOGFILE);
	}

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
			DoWorkInProgress(window, 3, NULL);
	}

	return(retval);
}

/******** MonitorCreateRunTime() ********/

BOOL MonitorCreateRunTime(struct Window *window)
{
BOOL loop=TRUE, retval;
int ID, choice;
//TEXT path[SIZE_FULLPATH], filename[SIZE_FILENAME], temp[256];
TEXT path[SIZE_FULLPATH], filename[SIZE_FULLPATH], temp[256];
struct FileReqRecord FRR;

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
strcpy(oriDest,path);	// remember e.g. DF0: -> turn in afterwards into e.g. Empty:
								if ( strnicmp(path,"ram",3) )	// don't convert ram:	
									UA_TurnAssignIntoDir(path);
								if ( path[ (strlen(path)-1) ] == '/' )
									path[ (strlen(path)-1) ] = '\0';
								UA_ClearButton(window, &SM3_GR[1], AREA_PEN);
								strcpy(temp,path);
								UA_ShortenString(window->RPort, temp, (SM3_GR[1].x2-SM3_GR[1].x1)-16);
								UA_DrawText(window, &SM3_GR[1], temp);
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
		if ( choice==0 )
			copyWhat = COPY_SCRIPTDATA;	// global - define
		else
			copyWhat = COPY_ALLDATA;		// global - define

		DoWorkInProgress(window, 1, path);
	}

	return(retval);
}

/******** MonitorMissingFiles() ********/

BOOL MonitorMissingFiles(struct Window *window)
{
BOOL loop=TRUE, retval;
int ID,i,topEntry,line;
//TEXT path[SIZE_FULLPATH], filename[SIZE_FILENAME], temp[256];
TEXT path[SIZE_FULLPATH], filename[SIZE_FULLPATH], temp[256];
struct FileListInfo *FLI;
UBYTE *scanDevs[50];
struct ScrollRecord SR;
UBYTE selectionList[50];
struct FileReqRecord FRR;

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

				case 2:	// page list area
					line = UA_SelectStandardListLine(&SR,topEntry,TRUE,&CED,FALSE,FALSE);
					if ( line!= -1 )
					{
					}
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
		// open deep scan stuff
		OpenDiskList();
		if ( !OpenScanMem() )
			CloseDiskList();

		PlaceInList(path);
		for(i=0; i<FLI->numDevices; i++)
			if ( selectionList[i] )
				PlaceInList(scanDevs[i]);
	
		DoWorkInProgress(window, 2, NULL);

		// close deep scan stuff
		CloseScanMem();
		CloseDiskList();
	}

	return(retval);
}

/******** DoWorkInProgress() ********/
/*
 * mode 1 = create run-time
 * mode 2 = fix
 * mode 3 = calc size
 *
 */

void DoWorkInProgress(struct Window *window, BYTE mode, STRPTR path)
{
BOOL loop;
int ID;
TEXT report[256];
struct Process *process;
struct Window *wdw;

	UA_ClearButton(window, &SM1_GR[4], AREA_PEN);	// clear whole window
	UA_DrawGadgetList(window, SM5_GR);

	UA_DisableButton(window, &SM5_GR[1], mypattern1);

	if ( mode==1 )
	{
		//Report("Creating run-time script...");
		Report( msgs[Msg_SM_22-1] );
		CreateRunTime(rvrec->aPtr,path);
	}
	else if ( mode==2 )
	{
		Forbid();
		process = (struct Process *)FindTask(NULL);
		Permit();
		wdw = process->pr_WindowPtr;
		process->pr_WindowPtr = -1;		// disable those stupid 'insert volume ...' messages

		//Report("Searching for missing files...");
		Report( msgs[Msg_SM_23-1] );
		FindMissingFiles(rvrec->aPtr);

		process->pr_WindowPtr = wdw;	// enable those stupid 'insert volume ...' messages
	}
	else if ( mode==3 )
	{
		CalcRunTimeSize(rvrec->aPtr,"apenoot:");
	}
	
	if ( numErrors == 1 )
	{
		sprintf(report, msgs[Msg_SM_34-1], numErrors);
		Report( report );	// 1 file couldn't be found

		Report( msgs[Msg_SM_35-1] );	// in ram:logfile staat...
	}
	else if ( numErrors > 1 )
	{
		sprintf(report, msgs[Msg_SM_33-1], numErrors);
		Report( report );	// 2 files couldn't be found

		Report( msgs[Msg_SM_35-1] );	// in ram:logfile staat...
	}

	Report( msgs[Msg_SM_24-1] );	// Ready

	/**** event handler ****/

	UA_EnableButton(window, &SM5_GR[1]);

	loop=TRUE;
	while(loop)
	{
		UA_doStandardWait(window,&CED);
		if ( CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN )
		{
			ID = UA_CheckGadgetList(window, SM5_GR, &CED);
			if (ID==1)
			{
do_ok:
				UA_HiliteButton(window, &SM5_GR[1]);
				loop=FALSE;
			}
		}
		else if ( CED.Class==IDCMP_RAWKEY && CED.Code==RAW_RETURN )
			goto do_ok;
	}

	if ( mode==1 || mode==3 )
		rvrec->returnCode = FALSE;
	else
		rvrec->returnCode = TRUE;		// only fixed script must be reloaded
}

/******** FindMissingFiles() ********/

BOOL FindMissingFiles(STRPTR scriptPath)
{
//TEXT path[SIZE_FULLPATH], name[SIZE_FILENAME];
TEXT path[SIZE_FULLPATH], name[SIZE_FULLPATH];
BOOL retval;

	// hunt for files

	SplitFullPath(scriptPath, path, name);
	retval = ParseAScriptFile(path, name, MODE_FIXSCRIPT, NULL, NULL, NULL);

	return( retval );
}

/******** CreateRunTime() ********/

BOOL CreateRunTime(STRPTR scriptPath, STRPTR destPath)
{
//TEXT path[SIZE_FULLPATH], name[SIZE_FILENAME], dest[SIZE_FULLPATH];
TEXT path[SIZE_FULLPATH], name[SIZE_FULLPATH], dest[SIZE_FULLPATH];
LONG size, largest, free, total;
struct FileLock *lock1, *lock2;
BOOL retval=TRUE;

	size=0;
	largest=0;
	strcpy(dest,destPath);

	CreateLongDir(dest);	// create dir on destination

	SplitFullPath(scriptPath, path, name);

	lock1 = (struct FileLock *)Lock(path,ACCESS_READ);
	lock2 = (struct FileLock *)Lock(destPath,ACCESS_READ);

	if ( lock1 && lock2 )
	{
		if ( SameLock((BPTR)lock1,(BPTR)lock2) == LOCK_SAME )
		{
			UnLock((BPTR)lock1);
			UnLock((BPTR)lock2);
			Message( msgs[Msg_OverItself-1] );
			return(TRUE);
		}
		UnLock((BPTR)lock1);
		UnLock((BPTR)lock2);
	}

	// GET FILE SIZE AND LARGEST SIZE

	retval = ParseAScriptFile(path, name, MODE_CALCSIZE, NULL, &size, &largest);	// get size and largest
	if ( retval )
	{
		if ( !DoSystemFiles(1, &size, NULL, NULL, NULL) )				// get total system size
			return(TRUE);

		// Copy script so that there's room later

		CopyFile(scriptPath, dest, NULL, TRUE, NULL,NULL, COPY_TWICE);

		// CHECK IF LARGEST FITS ON DESTINATION
	
		free = GetFreeSpace(dest);

		if ( largest > free )
		{
			Message( msgs[Msg_SM_36-1] );	// file larger than dest -- fatal!
			return(TRUE);
		}

		if ( size > free )
			Message( msgs[Msg_SM_37-1], oriDest );	// meerdere disks nodig

		// COPY SYSTEM FILES

		total = size;
		size = 0;

		if ( !DoSystemFiles(2, &size, &total, dest, NULL) )							// copy system files
			return( FALSE );
		if ( !DoSystemFiles(3, &size, &total, dest, name) )							// copy xapps
			return( FALSE );

		ParseAScriptFile(path, name, MODE_RUNTIME, dest, &size, &total);	// copy files
	}

	Report("100");

	return( TRUE );
}

/******** CalcRunTimeSize() ********/

BOOL CalcRunTimeSize(STRPTR scriptPath, STRPTR destPath)
{
//TEXT path[SIZE_FULLPATH], name[SIZE_FILENAME], report[256];
TEXT path[SIZE_FULLPATH], name[SIZE_FULLPATH], report[256];
LONG size1, size2, largest;

	size1=0;
	size2=0;
	largest=0;

	SplitFullPath(scriptPath, path, name);
	ParseAScriptFile(path, name, MODE_CALCSIZE, destPath, &size1, &largest);

	if ( !DoSystemFiles(1, &size2, NULL, NULL, NULL) )
		return( FALSE );

	//sprintf(report, "Total size of data files: %d Kb", size1/1024);
	sprintf(report,msgs[Msg_SM_25-1],size1/1024);
	Report(report);

	//sprintf(report, "Total size of system files: %d Kb", size2/1024);
	sprintf(report,msgs[Msg_SM_26-1],size2/1024);
	Report(report);

	//sprintf(report, "Largest file: %d Kb", largest/1024);
	sprintf(report,msgs[Msg_SM_27-1],largest/1024);
	Report(report);

	return( TRUE );
}

/******** DoSystemFiles() ********/
/*
 * mode 1 = get file sizes
 * mode 2 = get file sizes AND copy system files 
 * mode 3 = get file sizes AND copy xapps 
 *
 */

BOOL DoSystemFiles(BYTE mode, LONG *size, LONG *largest, STRPTR dest, STRPTR scrName)
{
TEXT f[SIZE_FULLPATH], str[SIZE_FULLPATH];
LONG l;
FILE *fp;

	if ( copyWhat == COPY_SCRIPTDATA )
		return(TRUE);

	// CREATE STARTUP-SEQUENCE

	if ( dest && mode==2 )
	{
		// write out startup-sequence

		if ( scrName )
		{
			UA_MakeFullPath(dest,"s",str);
			CreateLongDir(str);
			UA_MakeFullPath(str,"startup-sequence",f);
			fp = fopen(f,"w");
			if (fp)
			{
				fprintf(fp, "c/rexxmast >NIL:\n");
				fprintf(fp, "player %s\n", scrName);
				fclose(fp);
			}
		}

		// Create Scripts directory

		UA_MakeFullPath(dest,"Scripts",str);
		CreateLongDir(str);

		// Create Xapps directory

		UA_MakeFullPath(dest,"Xapps",str);
		CreateLongDir(str);
	}

	// MISC SYSTEM FILES

	strcpy(f,"system/MediaPoint.config");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "System", FALSE, size,largest, COPY_ONCE) )	// DF0:System/
			return(FALSE);
	}

	// MISCELLANEOUS STUFF

	strcpy(f,"sys:system/RexxMast");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "c", FALSE, size,largest, COPY_ONCE) )	// DF0:C/
			return(FALSE);
	}

	strcpy(f,"fonts:MediaPoint.font");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==3)
	{
		if ( !CopyFile(f, dest, "fonts", FALSE, size,largest, COPY_ONCE) )	// DF0:fonts/
			return(FALSE);
	}

	strcpy(f,"fonts:MediaPoint/10");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==3)
	{
		if ( !CopyFile(f, dest, "fonts/MediaPoint", FALSE, size,largest, COPY_ONCE) )	// DF0:fonts/MediaPoint/
			return(FALSE);
	}

	strcpy(f,"fonts:MediaPoint/20");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==3)
	{
		if ( !CopyFile(f, dest, "fonts/MediaPoint", FALSE, size,largest, COPY_ONCE) )	// DF0:fonts/MediaPoint/
			return(FALSE);
	}

	strcpy(f,"Player");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, NULL, FALSE, size,largest, COPY_ONCE) )	// DF0:
			return(FALSE);
	}

	// LANGUAGE FILES

	sprintf(f,"system/texts.%s",rvrec->capsprefs->lanExtension);
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "System", TRUE, size,largest, COPY_ONCE) )	// DF0:System/
			return(FALSE);
	}

#if 0
	strcpy(f,"system/texts.English");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "System", TRUE, size,largest, COPY_ONCE) )	// DF0:System/
			return(FALSE);
	}
#endif

	// MEDIAPOINT LIBRARIES

	strcpy(f,"system/mediapoint.library");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "System", TRUE, size,largest, COPY_ONCE) )	// DF0:System/
			return(FALSE);
	}

	strcpy(f,"system/mpmmu.library");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "System", TRUE, size,largest, COPY_ONCE) )	// DF0:System/
			return(FALSE);
	}

	// INPUT

	strcpy(f,"system/Input");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "System", TRUE, size,largest, COPY_ONCE) )	// DF0:System/
			return(FALSE);
	}

	// AREXX & DOS

	strcpy(f,"system/ARexx");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "System", TRUE, size,largest, COPY_ONCE) )	// DF0:System/
			return(FALSE);
	}

	strcpy(f,"system/DOS");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "System", TRUE, size,largest, COPY_ONCE) )	// DF0:System/
			return(FALSE);
	}

	// SERIAL & PARALLEL

	strcpy(f,"system/Serial");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "System", TRUE, size,largest, COPY_ONCE) )	// DF0:System/
			return(FALSE);
	}

	strcpy(f,"system/Parallel");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "System", TRUE, size,largest, COPY_ONCE) )	// DF0:System/
			return(FALSE);
	}

	// TIMERS

	strcpy(f,"system/HHMMSST_Timer");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "System", TRUE, size,largest, COPY_ONCE) )	// DF0:System/
			return(FALSE);
	}

	strcpy(f,"system/HHMMSSFF_Timer");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "System", TRUE, size,largest, COPY_ONCE) )	// DF0:System/
			return(FALSE);
	}

	strcpy(f,"system/MIDI_Timer");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "System", TRUE, size,largest, COPY_ONCE) )	// DF0:System/
			return(FALSE);
	}

	// ANIMATION & TRANSITIONS & MUSIC

	strcpy(f,"system/Animation");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "System", TRUE, size,largest, COPY_ONCE) )	// DF0:System/
			return(FALSE);
	}

	strcpy(f,"system/Transitions");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "System", TRUE, size,largest, COPY_ONCE) )	// DF0:System/
			return(FALSE);
	}

	strcpy(f,"system/Music");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "System", TRUE, size,largest, COPY_ONCE) )	// DF0:System/
			return(FALSE);
	}

	// XAPPS

	if ( XappInScript("CDTV") )
	{
		strcpy(f,"xapps/CDTV");
		if (mode==1)
		{
			l = GetFileSize(f);
			*size = *size + l;
		}
		else if (mode==3)
		{
			if ( !CopyFile(f, dest, "Xapps", TRUE, size,largest, COPY_ONCE) )	// DF0:Xapps/
				return(FALSE);
		}
	}

	if ( XappInScript("SAMPLE") )
	{
		strcpy(f,"xapps/Sample");
		if (mode==1)
		{
			l = GetFileSize(f);
			*size = *size + l;
		}
		else if (mode==3)
		{
			if ( !CopyFile(f, dest, "Xapps", TRUE, size,largest, COPY_ONCE) )	// DF0:Xapps/
				return(FALSE);
		}
	}

	if ( XappInScript("ION") )
	{
		strcpy(f,"xapps/Ion");
		if (mode==1)
		{
			l = GetFileSize(f);
			*size = *size + l;
		}
		else if (mode==3)
		{
			if ( !CopyFile(f, dest, "Xapps", TRUE, size,largest, COPY_ONCE) )	// DF0:Xapps/
				return(FALSE);
		}
	}

	if ( XappInScript("IV-24") )
	{
		strcpy(f,"xapps/IV-24");
		if (mode==1)
		{
			l = GetFileSize(f);
			*size = *size + l;
		}
		else if (mode==3)
		{
			if ( !CopyFile(f, dest, "Xapps", TRUE, size,largest, COPY_ONCE) )	// DF0:Xapps/
				return(FALSE);
		}

		strcpy(f,"libs:fye.library");
		if (mode==1)
		{
			l = GetFileSize(f);
			*size = *size + l;
		}
		else if (mode==3)
		{
			if ( !CopyFile(f, dest, "libs", FALSE, size,largest, COPY_ONCE) )	// DF0:LIBS/
				return(FALSE);
		}
	}

	if ( XappInScript("LASERDISC") )
	{
		strcpy(f,"xapps/LaserDisc");
		if (mode==1)
		{
			l = GetFileSize(f);
			*size = *size + l;
		}
		else if (mode==3)
		{
			if ( !CopyFile(f, dest, "Xapps", TRUE, size,largest, COPY_ONCE) )	// DF0:Xapps/
				return(FALSE);
		}

		strcpy(f,"devs:Player.Device");
		if (mode==1)
		{
			l = GetFileSize(f);
			*size = *size + l;
		}
		else if (mode==3)
		{
			if ( !CopyFile(f, dest, "devs", FALSE, size,largest, COPY_ONCE) )	// DF0:DEVS/
				return(FALSE);
		}

		strcpy(f,"devs:Player-Units");
		if (mode==1)
		{
			l = GetFileSize(f);
			*size = *size + l;
		}
		else if (mode==3)
		{
			if ( !CopyFile(f, dest, "devs", FALSE, size,largest, COPY_ONCE) )	// DF0:DEVS/
				return(FALSE);
		}

		if ( GetPlayerDevInfo(str) )
		{
			sprintf(f,"devs:Players/%s",str);
			if (mode==1)
			{
				l = GetFileSize(f);
				*size = *size + l;
			}
			else if (mode==3)
			{
				if ( !CopyFile(f, dest, "devs/Players", FALSE, size,largest, COPY_ONCE) )	// DF0:DEVS/PLAYERS/
					return(FALSE);
			}
		}
	}

	if ( XappInScript("STUDIO16") )
	{
		strcpy(f,"xapps/Studio16");
		if (mode==1)
		{
			l = GetFileSize(f);
			*size = *size + l;
		}
		else if (mode==3)
		{
			if ( !CopyFile(f, dest, "Xapps", TRUE, size,largest, COPY_ONCE) )	// DF0:Xapps/
				return(FALSE);
		}

		strcpy(f,"libs:studio.library");
		if (mode==1)
		{
			l = GetFileSize(f);
			*size = *size + l;
		}
		else if (mode==3)
		{
			if ( !CopyFile(f, dest, "libs", FALSE, size,largest, COPY_ONCE) )	// DF0:LIBS/
				return(FALSE);
		}
	}

	if ( XappInScript("MIDI") )
	{
		strcpy(f,"xapps/MIDI");
		if (mode==1)
		{
			l = GetFileSize(f);
			*size = *size + l;
		}
		else if (mode==3)
		{
			if ( !CopyFile(f, dest, "Xapps", TRUE, size,largest, COPY_ONCE) )	// DF0:Xapps/
				return(FALSE);
		}

		strcpy(f,"libs:camd.library");
		if (mode==1)
		{
			l = GetFileSize(f);
			*size = *size + l;
		}
		else if (mode==3)
		{
			if ( !CopyFile(f, dest, "libs", FALSE, size,largest, COPY_ONCE) )	// DF0:LIBS/
				return(FALSE);
		}

		strcpy(f,"libs:realtime.library");
		if (mode==1)
		{
			l = GetFileSize(f);
			*size = *size + l;
		}
		else if (mode==3)
		{
			if ( !CopyFile(f, dest, "libs", FALSE, size,largest, COPY_ONCE) )	// DF0:LIBS/
				return(FALSE);
		}
	}

	if ( XappInScript("CDXL") )
	{
		strcpy(f,"xapps/CDXL");
		if (mode==1)
		{
			l = GetFileSize(f);
			*size = *size + l;
		}
		else if (mode==3)
		{
			if ( !CopyFile(f, dest, "Xapps", TRUE, size,largest, COPY_ONCE) )	// DF0:Xapps/
				return(FALSE);
		}
	}

	if ( XappInScript("INTERLUDE") )
	{
		strcpy(f,"xapps/Interlude");
		if (mode==1)
		{
			l = GetFileSize(f);
			*size = *size + l;
		}
		else if (mode==3)
		{
			if ( !CopyFile(f, dest, "Xapps", TRUE, size,largest, COPY_ONCE) )	// DF0:Xapps/
				return(FALSE);
		}
	}

	// AMIGA SHARED LIBRARIES

	strcpy(f,"libs:diskfont.library");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "libs", FALSE, size,largest, COPY_ONCE) )	// DF0:LIBS/
			return(FALSE);
	}

	strcpy(f,"libs:mathtrans.library");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "libs", FALSE, size,largest, COPY_ONCE) )	// DF0:LIBS/
			return(FALSE);
	}

#if 0
	strcpy(f,"libs:mathffp.library");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "libs", FALSE, size,largest, COPY_ONCE) )	// DF0:LIBS/
			return(FALSE);
	}

	strcpy(f,"libs:icon.library");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "libs", FALSE, size,largest, COPY_ONCE) )	// DF0:LIBS/
			return(FALSE);
	}
#endif

	strcpy(f,"libs:locale.library");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "libs", FALSE, size,largest, COPY_ONCE) )	// DF0:LIBS/
			return(FALSE);
	}

	strcpy(f,"libs:rexxsyslib.library");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "libs", FALSE, size,largest, COPY_ONCE) )	// DF0:LIBS/
			return(FALSE);
	}

	// AMIGA SHARED DEVICES

	strcpy(f,"devs:serial.device");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "devs", FALSE, size,largest, COPY_ONCE) )	// DF0:DEVS/
			return(FALSE);
	}

	strcpy(f,"devs:parallel.device");
	if (mode==1)
	{
		l = GetFileSize(f);
		*size = *size + l;
	}
	else if (mode==2)
	{
		if ( !CopyFile(f, dest, "devs", FALSE, size,largest, COPY_ONCE) )	// DF0:DEVS/
			return(FALSE);
	}
}

/******** SplitFullPath() ********/
/*
 * If fullPath doesn't contain a ':'	-> path becomes current path ( getcd() )
 *																		-> filename becomes fullPath
 *
 * If fullPath is shorter than 3 characters:
 *																		-> path and filename become '\0'
 *
 * If fullPath is enclosed by '"':		-> double quotes are removed before parsing
 *
 * After stripping, path will get the UA_ValidatePath treatment
 *
 */

void SplitFullPath(STRPTR fullPath, STRPTR path, STRPTR filename)
{
int i,j,pos,len;

	path[0] = '\0';
	filename[0] = '\0';

	if ( !fullPath )
	{
		return;
	}

	// check if a volume name is present

	if ( UA_FindString(fullPath, ":")==-1 )
	{
		getcd(0, path);
		stccpy(filename, fullPath, SIZE_FILENAME);
		return;
	}

	// check minimal length

	if (strlen(fullPath)<3)
	{
		return;
	}

	// remove trailing and ending double quotes

	if ( fullPath[0] == '\"' )
	{
		len = strlen(fullPath)-2;
		for(i=0; i<len; i++)
			fullPath[i] = fullPath[i+1];
		fullPath[len] = '\0';
	}

	// split path and filename

	pos=-1;
	len=strlen(fullPath);
	for(j=len-1; j>0; j--)
	{
		if ( fullPath[j] == ':' || fullPath[j] == '/' )
		{
			pos=j;
			break;
		}
	}

	if ( pos != -1 )
	{
		stccpy(path, fullPath, pos+2);
		stccpy(filename, &fullPath[j+1], len-pos+1);

		UA_ValidatePath(path);
	}
}

/******** RemoveQuotes() ********/

void RemoveQuotes(STRPTR str)
{
int i,len;

	/* remove trailing and ending ' or " */
	len = strlen(str)-2;
	if (len>0)
	{
		for(i=0; i<len; i++)
			str[i] = str[i+1];
		str[len] = '\0';
	}
	else
		str[0] = '\0';
}

/******** XappInScript() ********/

BOOL XappInScript(STRPTR xappName)
{
struct ScriptInfoRecord *SIR;
struct ScriptNodeRecord *this_node;
int i;

	SIR = &(rvrec->ObjRec->scriptSIR);
	for(i=0; i<rvrec->capsprefs->MaxNumLists; i++)
	{
		if ( SIR->allLists[i] )
		{
			for(this_node=(struct ScriptNodeRecord *)SIR->allLists[i]->lh_Head;
					this_node->node.ln_Succ;
					this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
			{
				if ( 	this_node->nodeType == TALK_USERAPPLIC &&
							!strcmpi(this_node->objectPath,xappName) )
					return(TRUE);
			}
		}
	}

	return(FALSE);
}

/******** GetFreeSpace() ********/

LONG GetFreeSpace(STRPTR dest)
{
int error;
struct InfoData info;

	if ( !strnicmp(dest,"ram",3) )	// this is RAM: or RAM DISK:
	{
		return( (LONG)AvailMem(MEMF_PUBLIC) );
	}
	else
	{
		error = getdfs(dest,&info);	
		if ( error!=0 )
		{
			return(0);
		}
		else
			return( (info.id_NumBlocks - info.id_NumBlocksUsed) * info.id_BytesPerBlock ); 
	}
}

/******** GetPlayerDevInfo() ********/

BOOL GetPlayerDevInfo(STRPTR playerName)
{
FILE *fp;
int dummy1;
TEXT dummy2[100];

	/* 0 Sony_1200_9600 9600 serial.device 3 */

	fp = fopen("devs:player-units", "r");
	if (fp==NULL)
		return(FALSE);
	fscanf(	fp, "%d %s %d %s %d\n", &dummy1, playerName, &dummy1, dummy2, &dummy1 );
	fclose(fp);

	return(TRUE);
}

/******** CopyFile() ********/

BOOL CopyFile(STRPTR sourceFile, STRPTR destDevice, STRPTR destPath, BOOL alsoInfo,
							LONG *size, LONG *largest, BOOL copyMode)
{
TEXT fullPath[SIZE_FULLPATH], cmdStr[256], report[256], suffix[10];
//TEXT filename[SIZE_FILENAME], path[SIZE_FULLPATH];
TEXT filename[SIZE_FULLPATH], path[SIZE_FULLPATH];
int error;
LONG diskSize, fileSize, timeSrc, timeDst;
struct FileLock *FL;
BOOL ok_to_copy = TRUE;

	if ( destPath )
		UA_MakeFullPath(destDevice, destPath, fullPath);
	else
		strcpy(fullPath, destDevice);

	if ( strnicmp(sourceFile,"ram",3) )
		UA_TurnAssignIntoDir(sourceFile);
	if ( sourceFile[ (strlen(sourceFile)-1) ] == '/' )
		sourceFile[ (strlen(sourceFile)-1) ] = '\0';

	// check if disk is full

	diskSize = GetFreeSpace(destDevice);
	fileSize = GetFileSize(sourceFile);

	if ( diskSize < fileSize )
	{
		if ( !WaitForDisk(UAI.userWindow,destDevice) )
			return(FALSE);

		if ( diskNr==2 )	
			destDevice[ strlen(destDevice)-1 ] = '\0';	// sloop : eraf
		else if ( diskNr>2 )
			destDevice[ strlen(destDevice)-3 ] = '\0';	// sloop 02:,03: etc. eraf
		sprintf(suffix,"%02d",diskNr);
		strcat(destDevice,suffix);
		if ( !Relabel(oriDest,destDevice) )
			Report("Relabel() failed");

		UA_ValidatePath(destDevice);
		if ( destDevice[ (strlen(destDevice)-1) ] == '/' )
			destDevice[ (strlen(destDevice)-1) ] = '\0';

		diskNr++;

		if ( !DoSystemFiles(3, NULL, NULL, destDevice, NULL) )	// copy xapps, size&largest on null else they are counted twice
			return( FALSE );

		if ( destPath )
			UA_MakeFullPath(destDevice, destPath, fullPath);
		else
			strcpy(fullPath, destDevice);
	}

	// create directory (if non-existing)

	CreateLongDir(fullPath);

	// copy file

	SplitFullPath(sourceFile, path, filename); // split 'from:file' into 'from:' and 'file'
	UA_MakeFullPath(fullPath, filename, path); // create 'dest:file'

	if ( filename[0]=='\0' )
		return(TRUE);

	FL = NULL;
	if ( copyMode == COPY_ONCE )
	{
		FL = (struct FileLock *)Lock(path,ACCESS_READ);
		if ( FL )
			ok_to_copy = FALSE;	// mode says copy once and file apperently already exists
	}

	timeSrc = getft(sourceFile);
	timeDst = getft(path);

	if ( timeDst!=-1 && timeSrc!=-1 && (timeDst > timeSrc) )
	{
		//sprintf(report,"%s not copied to %s because %s is of a later date",sourceFile,path,path);
		//Report(report);
		ok_to_copy = FALSE;
	}

	if ( ok_to_copy )
	{
		sprintf(report,msgs[Msg_SM_29-1],sourceFile,fullPath);
		Report(report);

		sprintf(cmdStr, "copy \"%s\" to \"%s\"",sourceFile,fullPath);
		error = ExecuteCommand(cmdStr);
		if ( !error )
		{
			//sprintf(report,msgs[Msg_SM_28-1],sourceFile,fullPath);
			sprintf(report,msgs[Msg_SM_28-1],filename,fullPath);
			Report(report);
			numErrors++;
			if ( logfile )
			{
				fprintf(logfile,report);
				fprintf(logfile,"\n");
			}
		}

		if ( alsoInfo )
		{
			sprintf(cmdStr, "copy \"%s.info\" to \"%s\"",sourceFile,fullPath);
			ExecuteCommand(cmdStr);
		}

		if ( size && largest )
		{
			*size = *size + fileSize;
			if ( *largest>0 )
			{
				sprintf(report, "%d", (*(size)*100) / *(largest));	// n %
				Report(report);
			}
		}
	}

	if ( ( copyMode == COPY_ONCE ) && FL )
		UnLock((BPTR)FL);

	return(TRUE);
}

/******** CreateLongDir() ********/
/*
 * newPath = "rad:alpha/bravo/charlie"
 *
 */
 
void CreateLongDir(STRPTR newPath)
{
int xx[16],i;
char str1[256], str2[256];
struct FileLock *FL;
TEXT report[256];

	strcpy(str1,newPath);
	stspfp(str1,xx);
	i=0;
	str2[0] = '\0';

	while( xx[i] != -1 )
	{
		if ( strlen(str2) > 0 )
			strcat(str2,"/");
		strcat(str2,&str1[ xx[i] ]);

		FL = (struct FileLock *)CreateDir( str2 );
		if (FL)
		{
			UnLock((BPTR)FL);

			//sprintf(report, "Creating directory %s", newPath);
			sprintf(report,msgs[Msg_SM_30-1],newPath);
			Report(report);
		}
		
		i++;
	}
}

/******** ExecuteCommand() ********/

BOOL ExecuteCommand(STRPTR cmd)
{
#if 0
	DOSHandle =	(struct FileHandle *)Open("NIL:", (LONG)MODE_NEWFILE);
	if ( DOSHandle )
	{
		retVal = (BOOL)Execute((UBYTE *)cmd, (BPTR)NULL, (BPTR)DOSHandle);
		Close((BPTR)DOSHandle);
	}
#endif

	if ( system(cmd) != 0 )
		return(FALSE);
	else
		return(TRUE);

#if 0
	return(retVal);
#endif
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
TEXT reportStr[256];

	strcpy(reportStr,report);

	//KPrintF(reportStr);	KPrintF("\r\n");

	window = UAI.userWindow;

	if (window->WScreen->ViewPort.Modes & LACE)
		SetFont(window->RPort, rvrec->tiny_largefont);
	else
		SetFont(window->RPort, rvrec->tiny_smallfont);

	if ( isdigit( reportStr[0] ) )
	{
		SetAPen(window->RPort, BGND_PEN);
		sscanf(reportStr,"%d",&pct);
		if (pct)
		{
			if (pct>100)
				pct=100;
	
			w = SM5_GR[2].x2 - SM5_GR[2].x1 - 6;

			x = w * pct;
			x /= 100;

			if ( ( SM5_GR[2].x1 + x ) > ( SM5_GR[2].x1 + w ) )
				x = w; //SM5_GR[2].x2-3;

			RectFill(	window->RPort,
								SM5_GR[2].x1+3, 	SM5_GR[2].y1+2,
								SM5_GR[2].x1+3+x,	SM5_GR[2].y2-2 );
		}
	}
	else
	{
		SetAPen(window->RPort, LO_PEN);
		SetBPen(window->RPort, AREA_PEN);

		ScrollRaster(	window->RPort, 0, window->RPort->TxBaseline+5,
									SM5_GR[0].x1+2, SM5_GR[0].y1+2,
									SM5_GR[0].x2-2, SM5_GR[0].y2-2); 

		UA_ShortenString(window->RPort, reportStr, (SM5_GR[0].x2-SM5_GR[0].x1)-32);
		Move(window->RPort, SM5_GR[0].x1+5, SM5_GR[0].y2-window->RPort->TxBaseline);
		Text(window->RPort, reportStr, strlen(reportStr));
	}

	if (window->WScreen->ViewPort.Modes & LACE)
		SetFont(window->RPort, rvrec->largefont);
	else
		SetFont(window->RPort, rvrec->smallfont);
}

/******** WaitForDisk() ********/

BOOL WaitForDisk(struct Window *window, STRPTR destDevice)
{
struct Window *reqwdw;
BOOL loop=TRUE, retval=FALSE;
TEXT str[512];
int ID;

	sprintf(str, msgs[Msg_SM_31-1], oriDest);

	reqwdw = UA_OpenMessagePanel( window, str );
	UA_TranslateGR(DiskReq_GR, msgs);
	UA_DrawGadgetList(reqwdw, DiskReq_GR);
	UA_SwitchFlagsOn(reqwdw,IDCMP_DISKINSERTED);

	while(loop)
	{
		UA_doStandardWait(reqwdw,&CED);
		switch(CED.Class)
		{
			case IDCMP_MOUSEBUTTONS:
				if (CED.Code==SELECTDOWN)
				{
					ID = UA_CheckGadgetList(reqwdw, DiskReq_GR, &CED);
					if ( ID==0 )
					{
						UA_HiliteButton(reqwdw, &DiskReq_GR[ID]);		
						retval=TRUE;
						loop=FALSE;
					}
					else
					{
						UA_HiliteButton(reqwdw, &DiskReq_GR[ID]);		
						retval=FALSE;
						loop=FALSE;
					}
				}
				break;

			case IDCMP_DISKINSERTED:
					retval=TRUE;
					loop=FALSE;
				break;
		}
	}

	UA_SwitchFlagsOff(reqwdw,IDCMP_DISKINSERTED);
	UA_CloseMessagePanel( reqwdw );

	return( retval );
}

/******** E O F ********/
