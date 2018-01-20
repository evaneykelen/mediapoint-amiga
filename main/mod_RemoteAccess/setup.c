#include "nb:pre.h"
#include "setup.h"
#include "protos.h"
#include "structs.h"
#include "msm:protos.h"
#include "msm:structs.h"
#include "mra:ECP/structs.h"
#include <dos/dostags.h>
#include "demo:gen/wait50hz.h"

#define VERSION	"\0$VER: 1.0"

extern void DDMMYY_2_DDMMMYYYY(STRPTR, STRPTR);
/**** next calls in file mra:ECP/cdf.c ****/
extern BOOL Parse_CDF_File(struct CDF_Record *);
extern BOOL Write_CDF_File(struct CDF_Record *, STRPTR);
extern void Init_CDF_Record(struct CDF_Record *);

/**** globals ****/

static UBYTE *vers = VERSION;
UBYTE **msgs;
struct RendezVousRecord *rvrec;
struct EventData CED;
struct MsgPort *capsport;
struct UserApplicInfo UAI;
TEXT reportStr[256];
struct Process *process;

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *medialinkLibBase;

static struct PropInfo PI1 = { AUTOKNOB | FREEVERT | PROPBORDERLESS, 0,0,0,0, };
static struct Image Im1 = { 0,0,0,0,0,NULL,0x0000,0x0000,NULL };
struct Gadget PropSlider1 =
{
	NULL, 0,0,0,0, NULL, GACT_RELVERIFY | GACT_IMMEDIATE, GTYP_PROPGADGET,
	&Im1, NULL, NULL, NULL, (struct PropInfo *)&PI1, 1, NULL
};

static struct PropInfo PI2 = { AUTOKNOB | FREEVERT | PROPBORDERLESS, 0,0,0,0, };
static struct Image Im2 = { 0,0,0,0,0,NULL,0x0000,0x0000,NULL };
static struct Gadget PropSlider2 =
{
	NULL, 0,0,0,0, NULL, GACT_RELVERIFY | GACT_IMMEDIATE, GTYP_PROPGADGET,
	&Im2, NULL, NULL, NULL, (struct PropInfo *)&PI2, 2, NULL
};

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

/*
	if ( rvrec->aLong == XFER_DOWNLOAD )
	else if ( rvrec->aLong == XFER_UPLOAD )
*/

	/**** drain it ****/

	IntuitionBase 		= (struct IntuitionBase *)rvrec->intuition;
	GfxBase 					= (struct GfxBase *)rvrec->graphics;
	medialinkLibBase	= (struct Library *)rvrec->medialink;
	msgs							= (UBYTE **)rvrec->msgs;

	/**** play around ****/

	doYourThing();

	/**** delete temporary files ****/

	//DeleteFile(BIGFILE);
	//DeleteFile(TEMPSCRIPT);

	/**** and leave the show ****/

	capsport->mp_SigTask = oldTask;

	exit(0);
}

/******** doYourThing() ********/

BOOL doYourThing(void)
{
BOOL retval;
//struct Process *process;

	/**** initialize the User Application Info structure ***/

	UA_InitStruct(&UAI);
	UAI.IB = IntuitionBase;

	/**** open a window ****/

	UAI.windowModes = 2;
	UAI.userScreen = rvrec->scriptscreen;

	/**** double the dimensions of gadgets etc. if screen is laced ****/

	if ( UA_IsUAScreenLaced(&UAI) )
	{
		UA_DoubleGadgetDimensions(RA1_GR);
		UA_DoubleGadgetDimensions(RA1_A_GR);
		UA_DoubleGadgetDimensions(RA1_B_GR);
		// RA2_GR tru RA8_GR are doubled on window open
	}

	UA_TranslateGR(RA1_GR, msgs);
	UA_TranslateGR(RA1_A_GR, msgs);
	UA_TranslateGR(RA1_B_GR, msgs);
	UA_TranslateGR(RA2_GR, msgs);
	UA_TranslateGR(RA3_GR, msgs);
	UA_TranslateGR(RA4_GR, msgs);
	UA_TranslateGR(RA5_GR, msgs);
	UA_TranslateGR(RA6_GR, msgs);
	UA_TranslateGR(RA7_GR, msgs);
	UA_TranslateGR(RA8_GR, msgs);

	UAI.windowX			 	= -1;
	UAI.windowY			 	= -1;
	UAI.windowWidth	 	= RA1_GR[0].x2;
	UAI.windowHeight 	= RA1_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;

	/**** set the right font for this window ****/

	UAI.small_TF = rvrec->smallfont;
	UAI.large_TF = rvrec->largefont;

	UA_OpenWindow(&UAI);

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
BOOL loop, retval, optionsModified, useIt, sessionModified, dblClicked;
int ID, numScripts, numDests, numDisp, selScr, selDest, topEntry1, topEntry2, line, i, swap;
struct SessionRecord session_rec;
struct ScrollRecord SR1, SR2;
UBYTE **scriptList, **swapList, **ecpList, **cdfList, *selList1, *selList2;
struct Gadget *g;
struct FileReqRecord FRR;
TEXT temp[SIZE_FULLPATH], path1[SIZE_FULLPATH], fileName1[SIZE_FULLPATH];
TEXT path2[SIZE_FULLPATH], fileName2[SIZE_FULLPATH];
TEXT date[16], time[16];
struct ScriptListNode *SLN;
struct DestListNode *DLN;

	/**** set vars ****/

	loop = TRUE;
	optionsModified = FALSE;
	sessionModified = FALSE;

	session_rec.sessionName[0] = '\0';
	UA_MakeFullPath(rvrec->aPtrThree, "Session_Drawer", session_rec.sessionPath);
	//session_rec.sessionPath[0] = '\0';
	session_rec.upload_all_files = 0;
	session_rec.delayed_upload = 0;
	session_rec.skip_system_files = 0;
	session_rec.upload_multiple_scripts = 0;

	/**** render all gadgets ****/

	UA_DrawGadgetList(window, RA1_GR);
	UA_DrawDefaultButton(window, &RA1_GR[10]);

	/**** read config ****/

	if ( !GetRemoteAccessConfig(rvrec->aPtrThree, &session_rec) )
		return(FALSE);

	/**** read session ****/

	GetSession(&session_rec,window,temp);

	/**** alloc memory ****/

	scriptList = (UBYTE **)AllocMem(sizeof(UBYTE *)*MAXSCRIPTS,MEMF_CLEAR|MEMF_ANY);
	if ( !scriptList )
		return(FALSE);

	swapList = (UBYTE **)AllocMem(sizeof(UBYTE *)*MAXSCRIPTS,MEMF_CLEAR|MEMF_ANY);
	if ( !swapList )
	{
		FreeMem(scriptList, sizeof(UBYTE *)*MAXSCRIPTS);
		return(FALSE);
	}

	ecpList = (UBYTE **)AllocMem(sizeof(UBYTE *)*MAXSCRIPTS,MEMF_CLEAR|MEMF_ANY);
	if ( !ecpList )
	{
		FreeMem(scriptList, sizeof(UBYTE *)*MAXSCRIPTS);
		FreeMem(swapList, sizeof(UBYTE *)*MAXSCRIPTS);
		return(FALSE);
	}

	cdfList = (UBYTE **)AllocMem(sizeof(UBYTE *)*MAXSCRIPTS,MEMF_CLEAR|MEMF_ANY);
	if ( !cdfList )
	{
		FreeMem(scriptList, sizeof(UBYTE *)*MAXSCRIPTS);
		FreeMem(swapList, sizeof(UBYTE *)*MAXSCRIPTS);
		FreeMem(ecpList, sizeof(UBYTE *)*MAXSCRIPTS);
		return(FALSE);
	}

	selList1 = (UBYTE *)AllocMem(MAXSCRIPTS,MEMF_CLEAR|MEMF_ANY);
	if ( !selList1 )
	{
		FreeMem(scriptList, sizeof(UBYTE *)*MAXSCRIPTS);
		FreeMem(swapList, sizeof(UBYTE *)*MAXSCRIPTS);
		FreeMem(ecpList, sizeof(UBYTE *)*MAXSCRIPTS);
		FreeMem(cdfList, sizeof(UBYTE *)*MAXSCRIPTS);
		return(FALSE);
	}

	selList2 = (UBYTE *)AllocMem(MAXSCRIPTS,MEMF_CLEAR|MEMF_ANY);
	if ( !selList2 )
	{
		FreeMem(scriptList, sizeof(UBYTE *)*MAXSCRIPTS);
		FreeMem(swapList, sizeof(UBYTE *)*MAXSCRIPTS);
		FreeMem(ecpList, sizeof(UBYTE *)*MAXSCRIPTS);
		FreeMem(cdfList, sizeof(UBYTE *)*MAXSCRIPTS);
		FreeMem(selList1, MAXSCRIPTS);
		return(FALSE);
	}

	/**** render rest of gadgets ****/

	// ======================================================================
	// WHEN GOING FROM UPLOAD MULTIPLE TO UPLOAD SINGLE, WE JUMP BACK TO HERE
	// ======================================================================

again:

	// START - INIT SESSION
	if ( UA_IsWindowOnLacedScreen(window) )
		numDisp = 16;
	else
		numDisp = 8;
	topEntry1 = 0;
	topEntry2 = 0;
	selScr = 0;
	selDest = 0;
	numScripts = 0;
	numDests = 0;
	for(i=0;i<MAXSCRIPTS;i++)
	{
		selList1[i]=0;
		selList2[i]=0;
		scriptList[i]=NULL;
		swapList[i]=NULL;
		ecpList[i]=NULL;
		cdfList[i]=NULL;
	}
	selList1[0] = 1;
	selList2[0] = 1;
	CalcValues(&session_rec, selScr, &numScripts, &numDests, scriptList, swapList, ecpList, cdfList);
	// END - INIT SESSION

	if ( session_rec.upload_multiple_scripts )
	{
		UA_EnableButtonQuiet(&RA1_A_GR[ 9]);
		UA_EnableButtonQuiet(&RA1_A_GR[10]);
		UA_EnableButtonQuiet(&RA1_A_GR[11]);
		UA_EnableButtonQuiet(&RA1_A_GR[12]);
		UA_EnableButtonQuiet(&RA1_A_GR[13]);
		UA_DrawGadgetList(window, RA1_A_GR);

		// INIT PROP SLIDER 1
		PropSlider1.LeftEdge	= RA1_A_GR[5].x1+4;
		PropSlider1.TopEdge		= RA1_A_GR[5].y1+2;
		PropSlider1.Width			= RA1_A_GR[5].x2-RA1_A_GR[5].x1-7;
		PropSlider1.Height		= RA1_A_GR[5].y2-RA1_A_GR[5].y1-3;
		if ( UA_IsWindowOnLacedScreen(window) )
		{
			PropSlider1.TopEdge	+= 2;
			PropSlider1.Height	-= 4;
		}
		InitPropInfo(	(struct PropInfo *)PropSlider1.SpecialInfo,
									(struct Image *)PropSlider1.GadgetRender);
		AddGadget(window, &PropSlider1, -1L);

		// INIT PROP SLIDER 2
		PropSlider2.LeftEdge	= RA1_A_GR[7].x1+4;
		PropSlider2.TopEdge		= RA1_A_GR[7].y1+2;
		PropSlider2.Width			= RA1_A_GR[7].x2-RA1_A_GR[7].x1-7;
		PropSlider2.Height		= RA1_A_GR[7].y2-RA1_A_GR[7].y1-3;
		if ( UA_IsWindowOnLacedScreen(window) )
		{
			PropSlider2.TopEdge	+= 2;
			PropSlider2.Height	-= 4;
		}
		InitPropInfo(	(struct PropInfo *)PropSlider2.SpecialInfo,
									(struct Image *)PropSlider2.GadgetRender);
		AddGadget(window, &PropSlider2, -1L);

		// INIT SCROLL RECORD

		SR1.GR						= &RA1_A_GR[4];
		SR1.window				= window;
		SR1.list					= NULL;
		SR1.sublist				= swapList;
		SR1.selectionList	= selList1;
		SR1.entryWidth		= -1;
		SR1.numDisplay		= numDisp;
		SR1.numEntries		= numScripts;

		SR2.GR						= &RA1_A_GR[6];
		SR2.window				= window;
		SR2.list					= NULL;
		SR2.sublist				= cdfList;
		SR2.selectionList	= selList2;
		SR2.entryWidth		= -1;
		SR2.numDisplay		= numDisp;
		SR2.numEntries		= numDests;

		UA_PrintStandardList(NULL,-1,NULL);	// init static
		UA_PrintStandardList(&SR1,0,scriptList);
		UA_SetPropSlider(window, &PropSlider1, numScripts, numDisp, 0);

		UA_PrintStandardList(NULL,-1,NULL);	// init static
		UA_PrintStandardList(&SR2,0,ecpList);
		UA_SetPropSlider(window, &PropSlider2, numDests, numDisp, 0);

		// Ghost 

		DoButtonGhosting(window,numScripts,numDests);
	}
	else
	{
		UA_EnableButtonQuiet(&RA1_B_GR[1]);
		UA_EnableButtonQuiet(&RA1_B_GR[2]);
		UA_EnableButtonQuiet(&RA1_B_GR[4]);
		UA_DrawGadgetList(window, RA1_B_GR);

		UA_DrawSpecialGadgetText(window, &RA1_B_GR[0], msgs[Msg_RA_Script-1], SPECIAL_TEXT_TOP);
		UA_DrawSpecialGadgetText(window, &RA1_B_GR[1], msgs[Msg_RA_ECP-1], SPECIAL_TEXT_TOP);
		UA_DrawSpecialGadgetText(window, &RA1_B_GR[2], msgs[Msg_RA_CDF-1], SPECIAL_TEXT_TOP);
	
		UA_ClearButton(window, &RA1_B_GR[0], AREA_PEN);
		UA_ClearButton(window, &RA1_B_GR[1], AREA_PEN);
		UA_ClearButton(window, &RA1_B_GR[2], AREA_PEN);
		UA_ClearButton(window, &RA1_B_GR[3], AREA_PEN);	

		if ( scriptList[0] )
		{
			GetEntry(&session_rec, 0, -1, path1, fileName1, NULL, NULL, &swap, 1);

			UA_MakeFullPath(path1, fileName1, temp);
			UA_ShortenString(window->RPort, temp, (RA1_B_GR[0].x2-RA1_B_GR[0].x1)-16);
			UA_DrawText(window, &RA1_B_GR[0], temp);
		}
		else
		{
			swap = TRUE;
			UA_DisableButton(window, &RA1_B_GR[1], mypattern1);	// ECP
			UA_DisableButton(window, &RA1_B_GR[2], mypattern1);	// CDF
			UA_DisableButton(window, &RA1_B_GR[4], mypattern1);	// Edit CDF
		}

		if ( swap )
			UA_InvertButton(window, &RA1_B_GR[3]);

		if ( ecpList[0] )
		{
			GetEntry(&session_rec, 0, 0, path1, fileName1, path2, fileName2, NULL, 2);

			strcpy(temp, fileName1);
			UA_ShortenString(window->RPort, temp, (RA1_B_GR[1].x2-RA1_B_GR[1].x1)-16);
			UA_DrawText(window, &RA1_B_GR[1], temp);

			strcpy(temp, fileName2);
			UA_ShortenString(window->RPort, temp, (RA1_B_GR[2].x2-RA1_B_GR[2].x1)-16);
			UA_DrawText(window, &RA1_B_GR[2], temp);
		}
	}

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		dblClicked=FALSE;
		if (CED.extraClass == DBLCLICKED)
			dblClicked=TRUE;

		if ( CED.extraClass==IDCMP_GADGETDOWN || CED.extraClass==IDCMP_GADGETUP )
		{
			g = (struct Gadget *)CED.IAddress;
			if (g)
			{
				ID = g->GadgetID;
				if ( ID==1 )
				{
					UA_PrintStandardList(NULL,-1,NULL);	// init static
					UA_ScrollStandardList(&SR1,&topEntry1,&PropSlider1,scriptList,&CED);
				}
				else if ( ID==2 )
				{
					UA_PrintStandardList(NULL,-1,NULL);	// init static
					UA_ScrollStandardList(&SR2,&topEntry2,&PropSlider2,ecpList,&CED);
				}
			}
		}
		else if ( CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN )
		{
			/*************************************************************************/
			/**** GENERIC BUTTONS  ***************************************************/
			/*************************************************************************/

			ID = UA_CheckGadgetList(window, RA1_GR, &CED);
			switch(ID)
			{
				case 6:		// Load session
					UA_HiliteButton(window, &RA1_GR[ID]);
					useIt = TRUE;
					if ( sessionModified )
					{
						useIt = UA_OpenGenericWindow(	window, TRUE, TRUE, msgs[Msg_Yes-1], msgs[Msg_No-1],
																					QUESTION_ICON, msgs[Msg_RA_Modi1-1], TRUE, NULL );
					}
					if ( useIt )
					{
						FRR.path			= session_rec.sessionPath;
						FRR.fileName	= session_rec.sessionName;
						FRR.opts			= DIR_OPT_ALL | DIR_OPT_NOINFO;
						FRR.multiple	= FALSE;
						FRR.title			= msgs[Msg_RA_Select_a_session-1];
						if ( UA_OpenAFile(window, &FRR, mypattern1) )
						{
							sessionModified = FALSE;	// because we just loaded it
							GetSession(&session_rec,window,temp);
							if ( session_rec.upload_multiple_scripts )
							{
								RemoveGadget(window,&PropSlider1);
								RemoveGadget(window,&PropSlider2);
							}
							strcpy(session_rec.sessionName, FRR.fileName);
							strcpy(session_rec.sessionPath, FRR.path);
							optionsModified = TRUE;
							goto again;
						}
					}
					break;

				case 7:		// Save session
					UA_HiliteButton(window, &RA1_GR[ID]);
					FRR.path			= session_rec.sessionPath;
					FRR.fileName	= session_rec.sessionName;
					FRR.opts			= DIR_OPT_ALL | DIR_OPT_NOINFO;
					FRR.multiple	= FALSE;
					FRR.title			= msgs[Msg_RA_Save_this_session_as-1];
					if ( UA_SaveAFile(window, &FRR, mypattern1) )
					{
						sessionModified = FALSE;	// because we just saved it
						strcpy(temp, FRR.fileName);
						UA_ShortenString(window->RPort, temp, (RA1_GR[8].x2-RA1_GR[8].x1)-16);
						UA_ClearButton(window, &RA1_GR[8], AREA_PEN);
						UA_DrawSpecialGadgetText(window, &RA1_GR[8], temp, SPECIAL_TEXT_CENTER);
						// Save session file
						UA_MakeFullPath(FRR.path, FRR.fileName, temp);
						if ( !SaveSession(&session_rec, temp) )
							Message(msgs[Msg_RA_SessionSaveError-1]);
						UA_OpenAFile(NULL,&FRR,NULL);	// special trick to reset dir caching
					}
					break;

				case 9:		// Options
					UA_HiliteButton(window, &RA1_GR[ID]);
					ID = session_rec.upload_multiple_scripts;
					optionsModified = SetOptions(window, &session_rec);
					if ( ID != session_rec.upload_multiple_scripts )
					{
						if ( ID )	// we're going from multiple to single script
						{
							RemoveGadget(window,&PropSlider1);
							RemoveGadget(window,&PropSlider2);
						}
						UA_ClearButton(window, &RA1_GR[12], AREA_PEN);
						goto again;
					}
					break;

				case 10:	// Upload
do_upload:
					UA_HiliteButton(window, &RA1_GR[10]);
					useIt = TRUE;
					/**** warn if there's no script ****/
					if ( session_rec.scriptList.lh_TailPred == (struct Node *)&session_rec.scriptList )
						Message(msgs[Msg_RA_NoScripts-1]);
					else
					{
						/**** warn for modified session ****/
						if ( sessionModified )
							useIt = UA_OpenGenericWindow(	window, TRUE, TRUE, msgs[Msg_Yes-1], msgs[Msg_No-1],
																						QUESTION_ICON, msgs[Msg_RA_Modi2-1], TRUE, NULL );
						if ( useIt )
						{
							loop=FALSE;
							retval=TRUE;
						}
					}
					break;

				case 11:	// Cancel
do_cancel:
					UA_HiliteButton(window, &RA1_GR[11]);
					loop=FALSE;
					retval=FALSE;
					break;
			}

			/**************************************************************************/
			/**** MULTIPLE SCRIPTS  ***************************************************/
			/**************************************************************************/

			if ( ID==-1 && session_rec.upload_multiple_scripts )
			{
				ID = UA_CheckGadgetList(window, RA1_A_GR, &CED);
				switch(ID)
				{
					case 4:		// Script list
						line = UA_SelectStandardListLine(&SR1,topEntry1,FALSE,&CED,FALSE,FALSE);
						if ( line != -1 )
						{
							// START - UPDATE DEST LIST
							selScr = line + topEntry1;
							selDest = 0;
							for(i=0;i<MAXSCRIPTS;i++)
							{
								selList2[i]=0;
								scriptList[i]=NULL;
								swapList[i]=NULL;
								ecpList[i]=NULL;
								cdfList[i]=NULL;
							}
							CalcValues(	&session_rec, selScr, &numScripts, &numDests,
													scriptList, swapList, ecpList, cdfList );
							if ( numScripts > 0 )
							{
								SR2.numEntries = numDests;
								topEntry2=0;
								selList2[0]=1;
								UA_PrintStandardList(NULL,-1,NULL);	// init static
								UA_ClearButton(window, &RA1_A_GR[6], AREA_PEN);
								UA_PrintStandardList(&SR2,0,ecpList);
								UA_SetPropSlider(window, &PropSlider2, numDests, numDisp, 0);
							}
							// END - UPDATE DEST LIST
	
							DoButtonGhosting(window,numScripts,numDests);

							if ( dblClicked )
								goto do_script_edit;
						}
						break;
	
					case 6:		// Dest list
						line = UA_SelectStandardListLine(&SR2,topEntry2,FALSE,&CED,FALSE,FALSE);
						if ( line != -1 )
						{
							selDest = line + topEntry2;

							if ( dblClicked )
								goto do_dest_edit;
						}
						break;
	
					case 8:		// Add
						UA_HiliteButton(window, &RA1_A_GR[ID]);
						if ( SelectScript(window,&session_rec,numScripts,scriptList) )
						{
							sessionModified = TRUE;
							// START - UPDATE BOTH LISTS
							for(i=0;i<MAXSCRIPTS;i++)
							{
								selList1[i]=0;
								selList2[i]=0;
								scriptList[i]=NULL;
								swapList[i]=NULL;
								ecpList[i]=NULL;
								cdfList[i]=NULL;
							}
							CalcValues(	&session_rec, numScripts, &numScripts, &numDests,
													scriptList, swapList, ecpList, cdfList );
							if ( numScripts > 0 )
							{
								selScr = numScripts-1;
								selDest = 0;
								SR1.numEntries = numScripts;
								SR2.numEntries = numDests;
								if (numScripts>numDisp && topEntry1!=(numScripts-numDisp))
									topEntry1 = numScripts-numDisp;
								topEntry2=0;
								selList1[selScr]=1;
								selList2[selDest]=1;
								UA_PrintStandardList(NULL,-1,NULL);	// init static
								UA_PrintStandardList(&SR1,topEntry1,scriptList);
								UA_SetPropSlider(window, &PropSlider1, numScripts, numDisp, topEntry1);
								UA_PrintStandardList(NULL,-1,NULL);	// init static
								UA_ClearButton(window, &RA1_A_GR[6], AREA_PEN);
								UA_PrintStandardList(&SR2,topEntry2,ecpList);
								UA_SetPropSlider(window, &PropSlider2, numDests, numDisp, topEntry2);
							}
							// END - UPDATE BOTH LISTS
				
							DoButtonGhosting(window,numScripts,numDests);
						}
						break;
	
					case 9:		// Delete
						UA_HiliteButton(window, &RA1_A_GR[ID]);
						DeleteScriptItem(&session_rec, selScr, selDest);
						sessionModified = TRUE;
						// START - UPDATE BOTH LISTS
						for(i=0;i<MAXSCRIPTS;i++)
						{
							selList1[i]=0;
							selList2[i]=0;
							scriptList[i]=NULL;
							swapList[i]=NULL;
							ecpList[i]=NULL;
							cdfList[i]=NULL;
						}
						if ( selScr>0 && selScr>=(numScripts-1) )
							selScr--;
						CalcValues(	&session_rec, selScr, &numScripts, &numDests,
												scriptList, swapList, ecpList, cdfList );
						selDest = 0;
						SR1.numEntries = numScripts;
						SR2.numEntries = numDests;
						if ( (selScr >= topEntry1) && (selScr < (topEntry1+numDisp)) )
						{
							if (topEntry1>0)
								topEntry1--;
						}
						else if ( selScr < topEntry1 )
							topEntry1 = selScr;
						topEntry2=0;
						if ( numScripts > 0 )
						{
							selList1[selScr]=1;
							selList2[selDest]=1;
							UA_PrintStandardList(NULL,-1,NULL);	// init static
							UA_ClearButton(window, &RA1_A_GR[4], AREA_PEN);
							UA_PrintStandardList(&SR1,topEntry1,scriptList);
							UA_SetPropSlider(window, &PropSlider1, numScripts, numDisp, topEntry1);
							UA_PrintStandardList(NULL,-1,NULL);	// init static
							UA_ClearButton(window, &RA1_A_GR[6], AREA_PEN);
							UA_PrintStandardList(&SR2,topEntry2,ecpList);
							UA_SetPropSlider(window, &PropSlider2, numDests, numDisp, topEntry2);
						}
						// END - UPDATE BOTH LISTS
						DoButtonGhosting(window,numScripts,numDests);
						if ( numScripts==0 )
							UA_ClearButton(window, &RA1_A_GR[4], AREA_PEN);
						if ( numDests==0 )
							UA_ClearButton(window, &RA1_A_GR[6], AREA_PEN);
						break;
	
					case 10:	// Edit
do_script_edit:
						UA_HiliteButton(window, &RA1_A_GR[10]);
						if ( SelectScript(window,&session_rec,selScr,scriptList) )
						{
							sessionModified = TRUE;
							CalcValues(	&session_rec, selScr, &numScripts, &numDests,
													scriptList, swapList, ecpList, cdfList );
							UA_PrintStandardList(NULL,-1,NULL);	// init static
							UA_PrintStandardList(&SR1,topEntry1,scriptList);
						}
						break;
	
					case 11:	// Add
						UA_HiliteButton(window, &RA1_A_GR[ID]);
						if ( SelectECPandCDF(window,&session_rec,selScr,numDests,ecpList) )
						{
							sessionModified = TRUE;
							// START - UPDATE DEST LIST
							for(i=0;i<MAXSCRIPTS;i++)
							{
								selList2[i]=0;
								ecpList[i]=NULL;
								cdfList[i]=NULL;
							}
							CalcValues(	&session_rec, selScr, &numScripts, &numDests,
													scriptList, swapList, ecpList, cdfList );
							selDest = numDests-1;
							SR2.numEntries = numDests;
							if (numDests>numDisp && topEntry2!=(numDests-numDisp))
								topEntry2 = numDests-numDisp;
							selList2[selDest]=1;
							UA_PrintStandardList(NULL,-1,NULL);	// init static
							UA_PrintStandardList(&SR2,topEntry2,ecpList);
							UA_SetPropSlider(window, &PropSlider2, numDests, numDisp, topEntry2);
							// END - UPDATE BOTH LISTS
						}
						DoButtonGhosting(window,numScripts,numDests);
						break;
	
					case 12:	// Delete
						UA_HiliteButton(window, &RA1_A_GR[ID]);
						DeleteDestItem(&session_rec, selScr, selDest);
						sessionModified = TRUE;
						// START - UPDATE BOTH LISTS
						for(i=0;i<MAXSCRIPTS;i++)
						{
							selList2[i]=0;
							ecpList[i]=NULL;
							cdfList[i]=NULL;
						}
						if ( selDest>0 && selDest>=(numDests-1) )
							selDest--;
						CalcValues(	&session_rec, selScr, &numScripts, &numDests,
												scriptList, swapList, ecpList, cdfList );
						SR2.numEntries = numDests;
						if ( (selDest >= topEntry2) && (selDest < (topEntry2+numDisp)) )
						{
							if (topEntry2>0)
								topEntry2--;
						}
						else if ( selDest < topEntry2 )
							topEntry2 = selDest;
						if ( numDests > 0 )
						{
							selList2[selDest]=1;
							UA_PrintStandardList(NULL,-1,NULL);	// init static
							UA_ClearButton(window, &RA1_A_GR[6], AREA_PEN);
							UA_PrintStandardList(&SR2,topEntry2,ecpList);
							UA_SetPropSlider(window, &PropSlider2, numDests, numDisp, topEntry2);
						}
						// END - UPDATE BOTH LISTS
						DoButtonGhosting(window,numScripts,numDests);
						if ( numDests==0 )
							UA_ClearButton(window, &RA1_A_GR[6], AREA_PEN);
						break;
	
					case 13:	// Edit
do_dest_edit:
						UA_HiliteButton(window, &RA1_A_GR[13]);
						if ( SelectECPandCDF(window,&session_rec,selScr,selDest,ecpList) )
						{
							sessionModified = TRUE;
							CalcValues(	&session_rec, selScr, &numScripts, &numDests,
													scriptList, swapList, ecpList, cdfList );
							UA_PrintStandardList(NULL,-1,NULL);	// init static
							UA_PrintStandardList(&SR2,topEntry2,ecpList);
						}
						break;
				}
			}

			/***********************************************************************/
			/**** SINGLE SCRIPT  ***************************************************/
			/***********************************************************************/

			if ( ID==-1 && !session_rec.upload_multiple_scripts )
			{
				ID = UA_CheckGadgetList(window, RA1_B_GR, &CED);
				switch(ID)
				{
					case 0:		// Script
					case 3:		// Swap
						useIt = FALSE;
						if ( scriptList[0] )
							GetEntry(&session_rec, 0, -1, path1, fileName1, NULL, NULL, &swap, 1);
						else
						{
							strcpy(path1, rvrec->capsprefs->script_Path);
							fileName1[0] = '\0';
							swap = TRUE;							
						}
						if ( ID==0 )			// Select Script file
						{
							FRR.path			= path1;
							FRR.fileName	= fileName1;
							FRR.opts			= DIR_OPT_SCRIPTS;
							FRR.multiple	= FALSE;
							FRR.title			= msgs[Msg_SelectAScript-1];
							UA_InvertButton(window, &RA1_B_GR[0]);
							if ( UA_OpenAFile(window, &FRR, mypattern1) )
							{
								useIt = TRUE;
								UA_MakeFullPath(path1, fileName1, temp);
								UA_ShortenString(window->RPort, temp, (RA1_B_GR[0].x2-RA1_B_GR[0].x1)-16);
								UA_ClearButton(window, &RA1_B_GR[0], AREA_PEN);
								UA_DrawText(window, &RA1_B_GR[0], temp);
							}
							else
								UA_InvertButton(window, &RA1_B_GR[0]);
						}
						else if ( ID==3 )	// Swap immediately
						{
							useIt = TRUE;
							UA_ClearButton(window, &RA1_B_GR[3], AREA_PEN);
							if ( swap )
								swap=FALSE;
							else
							{
								swap=TRUE;
								UA_InvertButton(window, &RA1_B_GR[3]);
							}			
						}
						if ( useIt )
						{
							sessionModified = TRUE;
							if ( scriptList[0] )
								UpdateEntry(&session_rec, 0, -1, path1, fileName1, NULL, NULL, swap, 1);
							else
							{
								SLN = (struct ScriptListNode *)AllocMem(sizeof(struct ScriptListNode),MEMF_CLEAR|MEMF_ANY);
								if (SLN)
								{
									AddTail(&session_rec.scriptList, (struct Node *)SLN);
									NewList(&SLN->destList);
									strcpy(SLN->scriptPath, path1);
									strcpy(SLN->scriptName, fileName1);
									SLN->swap = swap;

									UA_EnableButton(window, &RA1_B_GR[1]);	// ECP
									UA_EnableButton(window, &RA1_B_GR[2]);	// CDF
									UA_EnableButton(window, &RA1_B_GR[4]);	// Edit CDF
								}
							}
							CalcValues(	&session_rec, 0, &numScripts, &numDests,
													scriptList, swapList, ecpList, cdfList );
						}
						break;

					case 1:		// ECP
					case 2:		// CDF
						useIt = FALSE;
						if ( ecpList[0] )
							GetEntry(&session_rec, 0, 0, path1, fileName1, path2, fileName2, NULL, 2);
						else
						{
							UA_MakeFullPath(rvrec->aPtrThree, "ECP_Drawer", path1);
							fileName1[0] = '\0';
							UA_MakeFullPath(rvrec->aPtrThree, "CDF_Drawer", path2);
							fileName2[0] = '\0';
						}
						if ( ID==1 )
						{
							FRR.path			= path1;
							FRR.fileName	= fileName1;
							FRR.opts			= DIR_OPT_ALL | DIR_OPT_NOINFO;
							FRR.multiple	= FALSE;
							FRR.title			= msgs[Msg_RA_SelectECP-1];
							UA_InvertButton(window, &RA1_B_GR[1]);
							if ( UA_OpenAFile(window, &FRR, mypattern1) )
							{
								useIt = TRUE;
								strcpy(temp, fileName1);
								UA_ShortenString(window->RPort, temp, (RA1_B_GR[1].x2-RA1_B_GR[1].x1)-16);
								UA_ClearButton(window, &RA1_B_GR[1], AREA_PEN);
								UA_DrawText(window, &RA1_B_GR[1], temp);
							}
							else
								UA_InvertButton(window, &RA1_B_GR[1]);
						}
						else if ( ID==2 )
						{
							FRR.path			= path2;
							FRR.fileName	= fileName2;
							FRR.opts			= DIR_OPT_ALL | DIR_OPT_NOINFO;
							FRR.multiple	= FALSE;
							FRR.title			= msgs[Msg_RA_SelectCDF-1];
							UA_InvertButton(window, &RA1_B_GR[2]);
							if ( UA_OpenAFile(window, &FRR, mypattern1) )
							{
								useIt = TRUE;
								strcpy(temp, fileName2);
								UA_ShortenString(window->RPort, temp, (RA1_B_GR[2].x2-RA1_B_GR[2].x1)-16);
								UA_ClearButton(window, &RA1_B_GR[2], AREA_PEN);
								UA_DrawText(window, &RA1_B_GR[2], temp);
							}
							else
								UA_InvertButton(window, &RA1_B_GR[2]);
						}
						if ( useIt )
						{
							sessionModified = TRUE;
							if ( ecpList[0] )
								UpdateEntry(&session_rec, 0, 0, path1, fileName1, path2, fileName2, -1, 2);
							else
							{
								SLN = (struct ScriptListNode *)session_rec.scriptList.lh_Head;	// first node
								DLN = (struct DestListNode *)AllocMem(sizeof(struct DestListNode),MEMF_CLEAR|MEMF_ANY);
								if (DLN)
								{
									AddTail(&SLN->destList, (struct Node *)DLN);
									strcpy(DLN->ecpPath, path1);
									strcpy(DLN->ecpName, fileName1);
									strcpy(DLN->cdfPath, path2);
									strcpy(DLN->cdfName, fileName2);
								}
							}
							CalcValues(	&session_rec, 0, &numScripts, &numDests,
													scriptList, swapList, ecpList, cdfList );
						}
						break;

					case 4:	// Edit CDF
						UA_InvertButton(window, &RA1_B_GR[ID]);
						if ( ecpList[0] )
							GetEntry(&session_rec, 0, 0, path1, fileName1, path2, fileName2, NULL, 2);
						else
						{
							UA_MakeFullPath(rvrec->aPtrThree, "CDF_Drawer", path2);
							fileName2[0] = '\0';
						}
						if ( EditCDF(window,&session_rec,path2,fileName2) )
						{
							// UPDATE VISUALLY
							strcpy(temp, fileName2);
							UA_ShortenString(window->RPort, temp, (RA1_B_GR[2].x2-RA1_B_GR[2].x1)-16);
							UA_ClearButton(window, &RA1_B_GR[2], AREA_PEN);
							UA_DrawText(window, &RA1_B_GR[2], temp);
							// UPDATE INTERNALLY
							sessionModified = TRUE;
							if ( ecpList[0] )
								UpdateEntry(&session_rec, 0, 0, path1, fileName1, path2, fileName2, -1, 2);
							CalcValues(&session_rec, 0, &numScripts, &numDests, scriptList, swapList, ecpList, cdfList);
						}
						UA_InvertButton(window, &RA1_B_GR[ID]);
						break;
				}
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if ( CED.Code==RAW_RETURN )
				goto do_upload;
			else if ( CED.Code==RAW_ESCAPE )
				goto do_cancel;
		}
	}

	/**** free memory ****/

	FreeMem(scriptList, sizeof(UBYTE *)*MAXSCRIPTS);
	FreeMem(swapList, sizeof(UBYTE *)*MAXSCRIPTS);
	FreeMem(ecpList, sizeof(UBYTE *)*MAXSCRIPTS);
	FreeMem(cdfList, sizeof(UBYTE *)*MAXSCRIPTS);
	FreeMem(selList1, MAXSCRIPTS);
	FreeMem(selList2, MAXSCRIPTS);

	/**** Upload was chosen... ****/

	if ( retval )
	{
		UA_DisableButton(window, &RA1_GR[10], mypattern1);	// Upload
		UA_DisableButton(window, &RA1_GR[11], mypattern1);	// Cancel

		SetRemoteAccessConfig(rvrec->aPtrThree, &session_rec);	// save silently

		/**** delayed or immediate uploading? ****/

		useIt = TRUE;
		if ( session_rec.delayed_upload )
		{
			ID = SetCountdown(window,&session_rec,date,time);
			if ( ID==1 )	// Delayed
				useIt = WaitOnCountdown(window,&session_rec,date,time);
			else if ( ID==2 )
				useIt = TRUE;
			else
				useIt = FALSE;
		}

		/**** do the upload ****/

		if ( useIt )
			MonitorUpload(window,&session_rec);
	}

	/**** free nodes with scripts and destinations ****/

	FreeScriptAndDestLists(&session_rec);

	return(retval);
}

/******** FindNameInList() ********/

BOOL FindNameInList(STRPTR oriPath, STRPTR name, int type)
{
	// STUB FOR 'MSM:PARSE3.C'
	return(TRUE);
}

/******** GetRemoteAccessConfig() ********/

BOOL GetRemoteAccessConfig(STRPTR path, struct SessionRecord *session_rec)
{
TEXT fullPath[SIZE_FULLPATH];
FILE *fp;

	UA_MakeFullPath(path,RA_CONFIG,fullPath);
	fp = fopen(fullPath,"r");
	if ( fp )
	{
		fgets(fullPath, SIZE_FULLPATH-1, fp);
		if ( !feof(fp) )
		{
			fullPath[ strlen(fullPath)-1 ] = '\0';
			RemoveQuotes(fullPath);
			UA_SplitFullPath(fullPath, session_rec->sessionPath, session_rec->sessionName);
		}
		if ( !feof(fp) )
			fscanf(fp,"%s %d\n",fullPath,&session_rec->upload_all_files);
		if ( !feof(fp) )
			fscanf(fp,"%s %d\n",fullPath,&session_rec->delayed_upload);
		if ( !feof(fp) )
			fscanf(fp,"%s %d\n",fullPath,&session_rec->skip_system_files);
		if ( !feof(fp) )
			fscanf(fp,"%s %d\n",fullPath,&session_rec->upload_multiple_scripts);
		fclose(fp);
		return(TRUE);
	}	
	else
		return(FALSE);
}

/******** SetRemoteAccessConfig() ********/

BOOL SetRemoteAccessConfig(STRPTR path, struct SessionRecord *session_rec)
{
TEXT fullPath[SIZE_FULLPATH];
FILE *fp;

	UA_MakeFullPath(path,RA_CONFIG,fullPath);
	fp = fopen(fullPath,"w");
	if ( fp )
	{
		UA_MakeFullPath(session_rec->sessionPath, session_rec->sessionName, fullPath);
		fprintf(fp,"\"%s\"\n",fullPath);
		fprintf(fp,"upload_all_files %d\n", session_rec->upload_all_files);
		fprintf(fp,"delayed_upload %d\n", session_rec->delayed_upload);
		fprintf(fp,"skip_system_files %d\n", session_rec->skip_system_files);
		fprintf(fp,"upload_multiple_scripts %d\n", session_rec->upload_multiple_scripts);
		fclose(fp);
		return(TRUE);
	}	
	else
		return(FALSE);
}

/******** CalcValues() ********/

void CalcValues(struct SessionRecord *session_rec, int active,
								int *numScripts, int *numDests,
								UBYTE **scriptList, UBYTE **swapList, UBYTE **ecpList, UBYTE **cdfList)
{
struct ScriptListNode *sln_work_node, *sln_next_node;
struct DestListNode *dln_work_node, *dln_next_node;
int i,j;

	if ( session_rec->scriptList.lh_TailPred == (struct Node *)&session_rec->scriptList )
	{
		*numScripts = 0;
		*numDests = 0;
		return;
	}

	i=0;
	sln_work_node = (struct ScriptListNode *)session_rec->scriptList.lh_Head;	// first node
	while(sln_next_node = (struct ScriptListNode *)(sln_work_node->node.ln_Succ))
	{
		scriptList[i] = sln_work_node->scriptName;

		if ( sln_work_node->swap )
			swapList[i] = msgs[Msg_RA_Immediately-1];	//swap_immediately;
		else
			swapList[i] = msgs[Msg_RA_Later-1];	//swap_later;

		if ( i==active )
		{
			// DEST LIST
			j=0;
			dln_work_node = (struct DestListNode *)sln_work_node->destList.lh_Head;	// first node
			if ( sln_work_node->destList.lh_TailPred != (struct Node *)&sln_work_node->destList )
			{
				while(dln_next_node = (struct DestListNode *)(dln_work_node->node.ln_Succ))
				{
					ecpList[j] = dln_work_node->ecpName;
					cdfList[j] = dln_work_node->cdfName;
					j++;
					dln_work_node = dln_next_node;
				}
			}
			*numDests = j;
		}

		// SCRIPT LIST
		sln_work_node = sln_next_node;
		i++;
		*numScripts = i;
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

/******** SelectScript() ********/

BOOL SelectScript(struct Window *onWindow, struct SessionRecord *session_rec,
									int selScript, UBYTE **scriptList)
{
struct Window *window;
BOOL loop=TRUE, retval;
struct FileReqRecord FRR;
TEXT path[SIZE_FULLPATH], fileName[SIZE_FULLPATH], temp[SIZE_FULLPATH];
int ID, swap;
struct ScriptListNode *SLN;

	/**** open window ****/

	window = UA_OpenRequesterWindow(onWindow, RA5_GR, STDCOLORS);
	if ( !window )
		return(FALSE);
	UA_DrawGadgetList(window, RA5_GR);
	UA_DrawSpecialGadgetText(window, &RA5_GR[7], msgs[Msg_RA_Script-1], SPECIAL_TEXT_TOP);

	/**** set up FRR ****/

	strcpy(path, rvrec->capsprefs->script_Path);
	fileName[0] = '\0';

	FRR.path			= path;
	FRR.fileName	= fileName;
	FRR.opts			= DIR_OPT_SCRIPTS;
	FRR.multiple	= FALSE;
	FRR.title			= msgs[Msg_SelectAScript-1];

	/**** set buttons to values ****/

	if ( scriptList[selScript] )
	{
		GetEntry(session_rec, selScript, -1, path, fileName, NULL, NULL, &swap, 1);
		UA_MakeFullPath(path, fileName, temp);
		UA_ShortenString(window->RPort, temp, (RA5_GR[7].x2-RA5_GR[7].x1)-16);
		UA_DrawText(window, &RA5_GR[7], temp);
	}
	else
	{
		UA_DisableButton(window, &RA5_GR[5], mypattern1);	// OK
		swap = TRUE;
	}

	if ( swap )
		UA_InvertButton(window, &RA5_GR[8]);

	/**** event handler ****/	
	
	while(loop)
	{
		UA_doStandardWait(window,&CED);
		if ( CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN )
		{
			ID = UA_CheckGadgetList(window, RA5_GR, &CED);
			switch(ID)
			{
				case 5:	// OK
do_ok:
					UA_HiliteButton(window, &RA5_GR[5]);
					loop=FALSE;
					retval=TRUE;
					break;

				case 6:	// Cancel
do_cancel:
					UA_HiliteButton(window, &RA5_GR[6]);
					loop=FALSE;
					retval=FALSE;
					break;

				case 7:	// Select script
					UA_InvertButton(window, &RA5_GR[7]);
					if ( UA_OpenAFile(window, &FRR, mypattern1) )
					{
						UA_MakeFullPath(path, fileName, temp);
						UA_ShortenString(window->RPort, temp, (RA5_GR[7].x2-RA5_GR[7].x1)-16);
						UA_ClearButton(window, &RA5_GR[7], AREA_PEN);
						UA_DrawText(window, &RA5_GR[7], temp);
						UA_EnableButton(window, &RA5_GR[5]);	// OK
					}
					else
						UA_InvertButton(window, &RA5_GR[7]);
					break;

				case 8:	// Swap immediately
					UA_InvertButton(window, &RA5_GR[8]);
					if ( swap )
						swap=FALSE;
					else
						swap=TRUE;
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

	UA_CloseRequesterWindow(window, STDCOLORS);

	if ( retval )
	{
		if ( scriptList[selScript] )
		{
			UpdateEntry(session_rec, selScript, -1, path, fileName, NULL, NULL, swap, 1);
		}
		else
		{
			SLN = (struct ScriptListNode *)AllocMem(sizeof(struct ScriptListNode),MEMF_CLEAR|MEMF_ANY);
			if (SLN)
			{
				AddTail(&session_rec->scriptList, (struct Node *)SLN);
				NewList(&SLN->destList);
				strcpy(SLN->scriptPath, path);
				strcpy(SLN->scriptName, fileName);
				SLN->swap = swap;
			}
		}
	}

	UA_EnableButtonQuiet(&RA5_GR[5]);

	return(retval);
}

/******** SelectECPandCDF() ********/

BOOL SelectECPandCDF(	struct Window *onWindow, struct SessionRecord *session_rec,
											int selScr, int selDest, UBYTE **ecpList )
{
struct Window *window;
BOOL loop=TRUE, retval;
struct FileReqRecord FRR;
TEXT path1[SIZE_FULLPATH], fileName1[SIZE_FULLPATH], temp[SIZE_FULLPATH];
TEXT path2[SIZE_FULLPATH], fileName2[SIZE_FULLPATH];
TEXT tmp_path2[SIZE_FULLPATH], tmp_fileName2[SIZE_FULLPATH];
int ID, i;
struct DestListNode *DLN;
struct ScriptListNode *sln_work_node, *sln_next_node;

	/**** open window ****/

	window = UA_OpenRequesterWindow(onWindow, RA6_GR, STDCOLORS);
	if ( !window )
		return(FALSE);
	UA_DrawGadgetList(window, RA6_GR);
	UA_DrawSpecialGadgetText(window, &RA6_GR[7], msgs[Msg_RA_ECP-1], SPECIAL_TEXT_TOP);
	UA_DrawSpecialGadgetText(window, &RA6_GR[8], msgs[Msg_RA_CDF-1], SPECIAL_TEXT_TOP);

	/**** set up FRR ****/

	UA_MakeFullPath(rvrec->aPtrThree, "ECP_Drawer", path1);
	fileName1[0] = '\0';

	UA_MakeFullPath(rvrec->aPtrThree, "CDF_Drawer", path2);
	fileName2[0] = '\0';

	FRR.opts = DIR_OPT_ALL | DIR_OPT_NOINFO;
	FRR.multiple = FALSE;

	/**** set buttons to values ****/

	if ( ecpList[selDest] )
	{
		GetEntry(session_rec, selScr, selDest, path1, fileName1, path2, fileName2, NULL, 2);

		strcpy(temp, fileName1);
		UA_ShortenString(window->RPort, temp, (RA6_GR[7].x2-RA6_GR[7].x1)-16);
		UA_DrawText(window, &RA6_GR[7], temp);

		strcpy(temp, fileName2);
		UA_ShortenString(window->RPort, temp, (RA6_GR[8].x2-RA6_GR[8].x1)-16);
		UA_DrawText(window, &RA6_GR[8], temp);
	}
	else
	{
		UA_DisableButton(window, &RA6_GR[5], mypattern1);	// OK
		UA_DisableButton(window, &RA6_GR[9], mypattern1);	// Edit CDF
	}
	
	/**** event handler ****/	
	
	while(loop)
	{
		UA_doStandardWait(window,&CED);
		if ( CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN )
		{
			ID = UA_CheckGadgetList(window, RA6_GR, &CED);
			switch(ID)
			{
				case 5:	// OK
do_ok:
					UA_HiliteButton(window, &RA6_GR[5]);
					loop=FALSE;
					retval=TRUE;
					break;

				case 6:	// Cancel
do_cancel:
					UA_HiliteButton(window, &RA6_GR[6]);
					loop=FALSE;
					retval=FALSE;
					break;

				case 7:	// Select ECP
					UA_InvertButton(window, &RA6_GR[ID]);
					FRR.path = path1;
					FRR.fileName = fileName1;
					FRR.title	= msgs[Msg_RA_SelectECP-1];
					if ( UA_OpenAFile(window, &FRR, mypattern1) )
					{
						strcpy(temp, fileName1);
						UA_ShortenString(window->RPort, temp, (RA6_GR[ID].x2-RA6_GR[ID].x1)-16);
						UA_ClearButton(window, &RA6_GR[ID], AREA_PEN);
						UA_DrawText(window, &RA6_GR[ID], temp);
						if ( fileName1[0] && fileName2[0] )
						{
							UA_EnableButton(window, &RA6_GR[5]);	// OK
							UA_EnableButton(window, &RA6_GR[9]);	// Edit CDF
						}
					}
					else
						UA_InvertButton(window, &RA6_GR[ID]);
					break;

				case 8:	// Select CDF
					UA_InvertButton(window, &RA6_GR[ID]);
					FRR.path = path2;
					FRR.fileName = fileName2;
					FRR.title	= msgs[Msg_RA_SelectCDF-1];
					if ( UA_OpenAFile(window, &FRR, mypattern1) )
					{
						strcpy(temp, fileName2);
						UA_ShortenString(window->RPort, temp, (RA6_GR[ID].x2-RA6_GR[ID].x1)-16);
						UA_ClearButton(window, &RA6_GR[ID], AREA_PEN);
						UA_DrawText(window, &RA6_GR[ID], temp);
						if ( fileName1[0] && fileName2[0] )
						{
							UA_EnableButton(window, &RA6_GR[5]);	// OK
							UA_EnableButton(window, &RA6_GR[9]);	// Edit CDF
						}
					}
					else
						UA_InvertButton(window, &RA6_GR[ID]);
					break;

				case 9:	// Edit CDF
					UA_InvertButton(window, &RA6_GR[ID]);
					strcpy(tmp_path2,path2);
					strcpy(tmp_fileName2,fileName2);
					if ( EditCDF(window,session_rec,tmp_path2,tmp_fileName2) )
					{
						strcpy(path2,tmp_path2);
						strcpy(fileName2,tmp_fileName2);
						strcpy(temp, fileName2);
						UA_ShortenString(window->RPort, temp, (RA6_GR[8].x2-RA6_GR[8].x1)-16);
						UA_ClearButton(window, &RA6_GR[8], AREA_PEN);
						UA_DrawText(window, &RA6_GR[8], temp);
					}
					UA_InvertButton(window, &RA6_GR[ID]);
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

	UA_CloseRequesterWindow(window, STDCOLORS);

	if ( retval )
	{
		if ( ecpList[selDest] )
		{
			UpdateEntry(session_rec, selScr, selDest, path1, fileName1, path2, fileName2, -1, 2);
		}
		else
		{
			i=0;
			ID=FALSE;
			sln_work_node = (struct ScriptListNode *)session_rec->scriptList.lh_Head;	// first node
			while(sln_next_node = (struct ScriptListNode *)(sln_work_node->node.ln_Succ))
			{
				if ( i==selScr )
				{
					ID=TRUE;
					break;
				}
				else
				{
					sln_work_node = sln_next_node;
					i++;
				}
			}
			if (ID)
			{
				DLN = (struct DestListNode *)AllocMem(sizeof(struct DestListNode),MEMF_CLEAR|MEMF_ANY);
				if (DLN)
				{
					AddTail(&sln_work_node->destList, (struct Node *)DLN);
					strcpy(DLN->ecpPath, path1);
					strcpy(DLN->ecpName, fileName1);
					strcpy(DLN->cdfPath, path2);
					strcpy(DLN->cdfName, fileName2);
				}
			}
		}
	}

	UA_EnableButtonQuiet(&RA6_GR[5]);	// OK
	UA_EnableButtonQuiet(&RA6_GR[9]);	// Edit CDF

	return(retval);
}

/******** UpdateEntry() ********/
/*
 * mode = 1  ->  update script list entry
 * mode = 2  ->  update dest list entry
 *
 */

void UpdateEntry(	struct SessionRecord *session_rec, int selScript, int selDest,
									STRPTR s1, STRPTR s2, STRPTR s3, STRPTR s4, int val, int mode )
{
struct ScriptListNode *sln_work_node, *sln_next_node;
struct DestListNode *dln_work_node, *dln_next_node;
int i,j;

	i=0;
	sln_work_node = (struct ScriptListNode *)session_rec->scriptList.lh_Head;	// first node
	while(sln_next_node = (struct ScriptListNode *)(sln_work_node->node.ln_Succ))
	{
		if ( i==selScript )
		{
			if ( mode==1 )
			{
				strcpy(sln_work_node->scriptPath, s1);
				strcpy(sln_work_node->scriptName, s2);
				sln_work_node->swap = val;
			}
			// DEST LIST
			j=0;
			dln_work_node = (struct DestListNode *)sln_work_node->destList.lh_Head;	// first node
			while(dln_next_node = (struct DestListNode *)(dln_work_node->node.ln_Succ))
			{
				if ( j==selDest && mode==2 )
				{
					strcpy(dln_work_node->ecpPath, s1);
					strcpy(dln_work_node->ecpName, s2);
					strcpy(dln_work_node->cdfPath, s3);
					strcpy(dln_work_node->cdfName, s4);
				}				
				j++;
				dln_work_node = dln_next_node;
			}
		}
		// SCRIPT LIST
		sln_work_node = sln_next_node;
		i++;
	}
}

/******** GetEntry() ********/
/*
 * mode = 1  ->  get script list entry
 * mode = 2  ->  get dest list entry
 *
 */

void GetEntry(struct SessionRecord *session_rec, int selScript, int selDest,
							STRPTR s1, STRPTR s2, STRPTR s3, STRPTR s4, int *val, int mode )
{
struct ScriptListNode *sln_work_node, *sln_next_node;
struct DestListNode *dln_work_node, *dln_next_node;
int i,j;

	i=0;
	sln_work_node = (struct ScriptListNode *)session_rec->scriptList.lh_Head;	// first node
	while(sln_next_node = (struct ScriptListNode *)(sln_work_node->node.ln_Succ))
	{
		if ( i==selScript )
		{
			if ( mode==1 )
			{
				strcpy(s1, sln_work_node->scriptPath);
				strcpy(s2, sln_work_node->scriptName);
				*val = sln_work_node->swap;
			}
			// DEST LIST
			j=0;
			dln_work_node = (struct DestListNode *)sln_work_node->destList.lh_Head;	// first node
			while(dln_next_node = (struct DestListNode *)(dln_work_node->node.ln_Succ))
			{
				if ( j==selDest && mode==2 )
				{
					strcpy(s1, dln_work_node->ecpPath);
					strcpy(s2, dln_work_node->ecpName);
					strcpy(s3, dln_work_node->cdfPath);
					strcpy(s4, dln_work_node->cdfName);
				}				
				j++;
				dln_work_node = dln_next_node;
			}
		}
		// SCRIPT LIST
		sln_work_node = sln_next_node;
		i++;
	}
}

/******** DeleteScriptItem() ********/

void DeleteScriptItem(struct SessionRecord *session_rec, int selScr, int selDest)
{
struct ScriptListNode *sln_work_node, *sln_next_node;
struct DestListNode *dln_work_node, *dln_next_node;
int i;

	i=0;
	sln_work_node = (struct ScriptListNode *)(session_rec->scriptList.lh_Head);	/* first node */
	while(sln_next_node = (struct ScriptListNode *)(sln_work_node->node.ln_Succ))
	{
		if ( i==selScr )
		{
			dln_work_node = (struct DestListNode *)(sln_work_node->destList.lh_Head);	/* first node */
			while(dln_next_node = (struct DestListNode *)(dln_work_node->node.ln_Succ))
			{
				Remove((struct Node *)dln_work_node);
				FreeMem(dln_work_node,sizeof(struct DestListNode));
				dln_work_node = dln_next_node;
			}
			Remove((struct Node *)sln_work_node);
			FreeMem(sln_work_node,sizeof(struct ScriptListNode));

			return;	// FUNCTION EXITS HERE WHEN DONE
		}
		sln_work_node = sln_next_node;
		i++;
	}
}

/******** DeleteDestItem() ********/

void DeleteDestItem(struct SessionRecord *session_rec, int selScr, int selDest)
{
struct ScriptListNode *sln_work_node, *sln_next_node;
struct DestListNode *dln_work_node, *dln_next_node;
int i,j;

	i=0;
	sln_work_node = (struct ScriptListNode *)(session_rec->scriptList.lh_Head);	/* first node */
	while(sln_next_node = (struct ScriptListNode *)(sln_work_node->node.ln_Succ))
	{
		if ( i==selScr )
		{
			j=0;
			dln_work_node = (struct DestListNode *)(sln_work_node->destList.lh_Head);	/* first node */
			while(dln_next_node = (struct DestListNode *)(dln_work_node->node.ln_Succ))
			{
				if ( j==selDest )
				{
					Remove((struct Node *)dln_work_node);
					FreeMem(dln_work_node,sizeof(struct DestListNode));
					return;
				}
				dln_work_node = dln_next_node;
				j++;
			}
		}
		sln_work_node = sln_next_node;
		i++;
	}
}

/******** DoButtonGhosting() ********/

void DoButtonGhosting(struct Window *window, int numScripts, int numDests)
{
	if ( numScripts>0 )	// Unghost 
	{
		if ( UA_IsGadgetDisabled(&RA1_A_GR[ 9]) )
		{
			UA_EnableButton(window, &RA1_A_GR[ 9]);	// Delete
			UA_EnableButton(window, &RA1_A_GR[10]);	// Edit
			OnGadget(&PropSlider1, window, NULL);
		}
	}
	else								// Ghost
	{
		if ( !UA_IsGadgetDisabled(&RA1_A_GR[ 9]) )
		{
			UA_DisableButton(window, &RA1_A_GR[ 9], mypattern1);	// Delete
			UA_DisableButton(window, &RA1_A_GR[10], mypattern1);	// Edit
			OffGadget(&PropSlider1, window, NULL);
		}
	}

	if ( numDests>0 )		// Unghost 
	{
		if ( UA_IsGadgetDisabled(&RA1_A_GR[12]) )
		{
			UA_EnableButton(window, &RA1_A_GR[12]);	// Delete
			UA_EnableButton(window, &RA1_A_GR[13]);	// Edit
			OnGadget(&PropSlider2, window, NULL);
		}
	}
	else								// Ghost
	{
		if ( !UA_IsGadgetDisabled(&RA1_A_GR[12]) )
		{
			UA_DisableButton(window, &RA1_A_GR[12], mypattern1);	// Delete
			UA_DisableButton(window, &RA1_A_GR[13], mypattern1);	// Edit
			OffGadget(&PropSlider2, window, NULL);
		}
	}

	if ( numScripts==0 )
	{
		if ( !UA_IsGadgetDisabled(&RA1_A_GR[11]) )
			UA_DisableButton(window, &RA1_A_GR[11], mypattern1);	// Add
	}
	else
	{
		if ( UA_IsGadgetDisabled(&RA1_A_GR[11]) )
			UA_EnableButton(window, &RA1_A_GR[11]);	// Add
	}
}

/******** SetOptions() ********/

BOOL SetOptions(struct Window *onWindow, struct SessionRecord *session_rec)
{
struct Window *window;
BOOL loop=TRUE, retval;
int ID, upload_all_files, delayed_upload, skip_system_files, upload_multiple_scripts;

	/**** open window ****/

	window = UA_OpenRequesterWindow(onWindow, RA3_GR, STDCOLORS);
	if ( !window )
		return(FALSE);
	UA_DrawGadgetList(window, RA3_GR);

	/**** set buttons to values ****/

	upload_all_files				= session_rec->upload_all_files;
	delayed_upload					= session_rec->delayed_upload;
	skip_system_files				= session_rec->skip_system_files;
	upload_multiple_scripts	= session_rec->upload_multiple_scripts;

	if ( upload_all_files )
		UA_InvertButton(window, &RA3_GR[7]);

	if ( delayed_upload )
		UA_InvertButton(window, &RA3_GR[8]);

	if ( skip_system_files )
		UA_InvertButton(window, &RA3_GR[9]);

	if ( upload_multiple_scripts )
		UA_InvertButton(window, &RA3_GR[10]);

	/**** event handler ****/	
	
	while(loop)
	{
		UA_doStandardWait(window,&CED);
		if ( CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN )
		{
			ID = UA_CheckGadgetList(window, RA3_GR, &CED);
			switch(ID)
			{
				case 5:	// OK
do_ok:
					UA_HiliteButton(window, &RA3_GR[5]);
					loop=FALSE;
					retval=TRUE;
					break;

				case 6:	// Cancel
do_cancel:
					UA_HiliteButton(window, &RA3_GR[6]);
					loop=FALSE;
					retval=FALSE;
					break;

				case 7:
					UA_InvertButton(window, &RA3_GR[ID]);
					if ( upload_all_files )
						upload_all_files=FALSE;
					else
						upload_all_files=TRUE;
					break;

				case 8:
					UA_InvertButton(window, &RA3_GR[ID]);
					if ( delayed_upload )
						delayed_upload=FALSE;
					else
						delayed_upload=TRUE;
					break;

				case 9:
					UA_InvertButton(window, &RA3_GR[ID]);
					if ( skip_system_files )
						skip_system_files=FALSE;
					else
						skip_system_files=TRUE;
					break;

				case 10:
					UA_InvertButton(window, &RA3_GR[ID]);
					if ( upload_multiple_scripts )
						upload_multiple_scripts=FALSE;
					else
						upload_multiple_scripts=TRUE;
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

	UA_CloseRequesterWindow(window, STDCOLORS);

	if ( retval )
	{
		session_rec->upload_all_files					= upload_all_files;
		session_rec->delayed_upload						= delayed_upload;
		session_rec->skip_system_files				= skip_system_files;
		session_rec->upload_multiple_scripts	= upload_multiple_scripts;
	}

	return(retval);
}

/******** GetSession() ********/

void GetSession(struct SessionRecord *session_rec, struct Window *window, STRPTR temp)
{
	UA_ClearButton(window, &RA1_GR[8], AREA_PEN);

	if ( !ParseSessionFile(session_rec) )
		Message(msgs[Msg_RA_SessionReadError-1]);
	else
	{
		strcpy(temp, session_rec->sessionName);
		UA_ShortenString(window->RPort, temp, (RA1_GR[8].x2-RA1_GR[8].x1)-16);
		UA_DrawSpecialGadgetText(window, &RA1_GR[8], temp, SPECIAL_TEXT_CENTER);
	}
}

/******** SetCountdown() ********/

int SetCountdown(	struct Window *onWindow, struct SessionRecord *session_rec,
									STRPTR date, STRPTR time )
{
struct Window *window;
BOOL loop=TRUE, retval;
int ID, day, month, year, hours, mins, secs, tenths=0, radio;

	/**** open window ****/

	window = UA_OpenRequesterWindow(onWindow, RA7_GR, STDCOLORS);
	if ( !window )
		return(0);
	UA_DrawGadgetList(window, RA7_GR);

	/**** set buttons to values ****/

	UA_InvertButton(window, &RA7_GR[8]);
	radio=0;

	SystemDate(&day, &month, &year);
	DayMonthYearToString(date,day,month,year);
	UA_SetStringGadgetToString(window, &RA7_GR[10], date);

	SystemTime(&hours, &mins, &secs);
	hours++;
	if ( hours>23 )
		hours=0;
	HoursMinsSecsTenthsToString(time, hours, mins, secs, tenths);
	time[8]='\0';
	UA_SetStringGadgetToString(window, &RA7_GR[11], time);

	/**** event handler ****/	
	
	while(loop)
	{
		UA_doStandardWait(window,&CED);
		if ( CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN )
		{
			ID = UA_CheckGadgetList(window, RA7_GR, &CED);
			switch(ID)
			{
				case 5:	// OK
do_ok:
					UA_HiliteButton(window, &RA7_GR[5]);
					loop=FALSE;
					retval=TRUE;
					break;

				case 6:	// Cancel
do_cancel:
					UA_HiliteButton(window, &RA7_GR[6]);
					loop=FALSE;
					retval=FALSE;
					break;

				case 8:	// Delayed
					if ( radio==1 )
					{
						radio=0;
						UA_InvertButton(window, &RA7_GR[8]);
						UA_InvertButton(window, &RA7_GR[9]);
					}						
					break;

				case 9:	// Immediately
					if ( radio==0 )
					{
						radio=1;
						UA_InvertButton(window, &RA7_GR[8]);
						UA_InvertButton(window, &RA7_GR[9]);
					}						
					break;

				case 10:
					UA_ProcessStringGadget(window, RA7_GR, &RA7_GR[ID], &CED);
					CheckEnteredDate(window, &RA7_GR[ID], -1);
					UA_DrawGadget(window, &RA7_GR[ID]);
					UA_SetStringToGadgetString(&RA7_GR[ID],date);
					break;

				case 11:
					UA_ProcessStringGadget(window, RA7_GR, &RA7_GR[ID], &CED);
					CheckEnteredTime(&RA7_GR[ID]);
					{
						struct StringRecord *SR_ptr;
						SR_ptr = (struct StringRecord *)RA7_GR[ID].ptr;
						SR_ptr->buffer[8]='\0';
					}
					UA_DrawGadget(window, &RA7_GR[ID]);
					UA_SetStringToGadgetString(&RA7_GR[ID],time);
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

	UA_CloseRequesterWindow(window, STDCOLORS);

	if ( retval )
	{
		if ( radio==0 )	// Delayed
			return(1);

		if ( radio==1 )	// Immediately
			return(2);
	}

	return(0);
}

/******** WaitOnCountdown() ********/

BOOL WaitOnCountdown(	struct Window *onWindow, struct SessionRecord *session_rec,
											STRPTR date, STRPTR time )
{
struct Window *window;
BOOL loop=TRUE, retval=TRUE;
int ID;
struct wjif WJIF;
ULONG hz_signal=0,signals;
TEXT dateStr[16], timeStr[16], str[50];
int day, month, year, hours, mins, secs, tenths=0, days1, days2, mins1, mins2, ticks1, ticks2;
ULONG l1,l2,l3,l4;

	DDMMYY_2_DDMMMYYYY(date,date);
	dateStringtoDays(date,&days1);

	strcat(time,":0");
	timeStringtoMinutesAndTicks(time, &mins1, &ticks1);

	/**** open window ****/

	window = UA_OpenRequesterWindow(onWindow, RA4_GR, STDCOLORS);
	if ( !window )
		return(FALSE);
	UA_DrawGadgetList(window, RA4_GR);

	/**** event handler ****/	
	
	WJIF.signum = 0;
	hz_signal = set50hz(&WJIF, 50);

	while(loop)
	{
		signals = UA_doStandardWaitExtra(window,&CED,hz_signal);

		if ( signals & hz_signal )
		{
			if ( hz_signal != 0 )
			{
				remove50hz( &WJIF );
				WJIF.signum = 0;
				hz_signal = set50hz(&WJIF, 50);
			}

			SystemDate(&day, &month, &year);
			DayMonthYearToString(dateStr,day,month,year);
			DDMMYY_2_DDMMMYYYY(dateStr,dateStr);
			dateStringtoDays(dateStr,&days2);

			SystemTime(&hours, &mins, &secs);
			HoursMinsSecsTenthsToString(timeStr, hours, mins, secs, tenths);
			timeStringtoMinutesAndTicks(timeStr, &mins2, &ticks2);

			l1 = (ULONG)( (days1-days2)*(24*36000) );
			l2 = (ULONG)( ((mins1/60) - (mins2/60)) * 36000 );
			l3 = (ULONG)( ((mins1%60) - (mins2%60)) * 600 );
			l4 = (ULONG)( ((ticks1/50) - (ticks2/50)) * 10 );

			if ( (LONG)(l1+l2+l3+l4) >= 0L )
			{
				secondsToDuration(l1+l2+l3+l4, str);
				//str[8]='\0';	// chop off tenths
				str[ strlen(str)-2 ] = '\0';	// chop off tenths

				UA_ClearButton(window, &RA4_GR[8], AREA_PEN);
				UA_DrawSpecialGadgetText(window, &RA4_GR[8], str, SPECIAL_TEXT_CENTER);
			}	
			else
			{
				loop=FALSE;
				UA_ClearButton(window, &RA4_GR[8], AREA_PEN);
			}
		}
		else
		{
			if ( CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN )
			{
				ID = UA_CheckGadgetList(window, RA4_GR, &CED);
				switch(ID)
				{
					case 5:	// Abort
do_abort:
						UA_HiliteButton(window, &RA4_GR[5]);
						retval=FALSE;
						loop=FALSE;
						break;
				}
			}
			else if (CED.Class==IDCMP_RAWKEY)
			{
				if ( CED.Code==RAW_ESCAPE )
					goto do_abort;
			}
		}
	}

	UA_CloseRequesterWindow(window, STDCOLORS);

	if ( hz_signal != 0 )
		remove50hz( &WJIF );

	return(retval);
}

/******** EditCDF() ********/

BOOL EditCDF(struct Window *onWindow, struct SessionRecord *session_rec, STRPTR path, STRPTR fileName)
{
struct Window *window;
BOOL loop=TRUE, retval, modifiedCDF;
int ID,val;
struct FileReqRecord FRR;
struct CDF_Record CDF_rec;
TEXT password1[16], password2[16];

	/**** init and read CDF file ****/

	Init_CDF_Record(&CDF_rec);
	strcpy(CDF_rec.ConnectionClass,"1");	// made -1 by init, we don't want that for the gui
	UA_MakeFullPath(path, fileName, CDF_rec.CDF_Path);
	if ( !Parse_CDF_File(&CDF_rec) )
		return(FALSE);

	/**** open window ****/

	window = UA_OpenRequesterWindow(onWindow, RA8_GR, STDCOLORS);
	if ( !window )
		return(FALSE);
	UA_DrawGadgetList(window, RA8_GR);

	UA_DrawSpecialGadgetText(window, &RA8_GR[ 8], msgs[Msg_CDF_2-1], SPECIAL_TEXT_TOP);
	UA_DrawSpecialGadgetText(window, &RA8_GR[15], msgs[Msg_CDF_5-1], SPECIAL_TEXT_TOP);
	UA_DrawSpecialGadgetText(window, &RA8_GR[16], msgs[Msg_CDF_6-1], SPECIAL_TEXT_TOP);
	UA_DrawSpecialGadgetText(window, &RA8_GR[17], msgs[Msg_Device-1], SPECIAL_TEXT_TOP);
	UA_DrawSpecialGadgetText(window, &RA8_GR[20], msgs[Msg_CDF_14-1], SPECIAL_TEXT_TOP);

	UA_DrawDefaultButton(window, &RA8_GR[5]);

	/**** start - set button contents ****/

	sscanf(CDF_rec.ConnectionClass,"%d",&val);	// cc >= 1 !!
	UA_SetCycleGadgetToVal(window,&RA8_GR[8],val-1);

	sscanf(CDF_rec.BaudRate,"%d",&val);
	UA_SetStringGadgetToVal(window,&RA8_GR[11],val);

	sscanf(CDF_rec.UnitNumber,"%d",&val);
	UA_SetCycleGadgetToVal(window,&RA8_GR[12],val);

	if ( !strcmpi(CDF_rec.HandShaking, "RTS/CTS") ) { val=0; } else { val=1; }
	UA_SetCycleGadgetToVal(window,&RA8_GR[13],val);

	sscanf(CDF_rec.BufferSize,"%d",&val);
	UA_SetStringGadgetToVal(window,&RA8_GR[14],val);

	UA_SetStringGadgetToString(window,&RA8_GR[15],CDF_rec.DestinationPath);
	UA_SetStringGadgetToString(window,&RA8_GR[16],CDF_rec.PassWord);
	UA_SetStringGadgetToString(window,&RA8_GR[17],CDF_rec.SerialDevice);
	UA_SetStringGadgetToString(window,&RA8_GR[18],CDF_rec.ReplyString);
	UA_SetStringGadgetToString(window,&RA8_GR[19],CDF_rec.DialPrefix);
	UA_SetStringGadgetToString(window,&RA8_GR[20],CDF_rec.PhoneNr);

	/**** end - set button contents ****/

	DisableCDFButtons(window,RA8_GR,&CDF_rec);

	/**** event handler ****/	

	modifiedCDF = FALSE;
	
	while(loop)
	{
		UA_doStandardWait(window,&CED);
		if ( CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN )
		{
			ID = UA_CheckGadgetList(window, RA8_GR, &CED);
			switch(ID)
			{
				case 5:		// OK
do_ok:
					UA_HiliteButton(window, &RA8_GR[5]);
					if ( modifiedCDF )
					{
						if ( UA_OpenGenericWindow(window, TRUE, TRUE, msgs[Msg_Yes-1], msgs[Msg_No-1],
																			QUESTION_ICON, msgs[Msg_CDF_15-1], TRUE, NULL ) )
						{
							loop=FALSE;
							retval=TRUE;
						}
					}
					else
					{
						loop=FALSE;
						retval=TRUE;
					}				
					break;

				case 6:		// Cancel
do_cancel:
					UA_HiliteButton(window, &RA8_GR[6]);
					loop=FALSE;
					retval=FALSE;
					break;

				case 8:		// Connection class
					UA_ProcessCycleGadget(window, &RA8_GR[ID], &CED);
					UA_SetValToCycleGadgetVal(&RA8_GR[ID],&val);
					sprintf(CDF_rec.ConnectionClass,"%d",val+1);
					DisableCDFButtons(window,RA8_GR,&CDF_rec);
					modifiedCDF=TRUE;
					break;

				case 9:		// Load CDF
					UA_HiliteButton(window, &RA8_GR[ID]);
					FRR.path			= path;
					FRR.fileName	= fileName;
					FRR.opts			= DIR_OPT_ALL | DIR_OPT_NOINFO;
					FRR.multiple	= FALSE;
					FRR.title			= msgs[Msg_RA_SelectCDF-1];
					if ( UA_OpenAFile(window, &FRR, mypattern1) )
					{
						Init_CDF_Record(&CDF_rec);
						strcpy(CDF_rec.ConnectionClass,"1");	// made -1 by init, we don't want that for the gui
						UA_MakeFullPath(path, fileName, CDF_rec.CDF_Path);
						if ( !Parse_CDF_File(&CDF_rec) )
						{
							loop=FALSE;
							retval=FALSE;
						}
						else
						{
							/**** enable all buttons ****/

							for(val=11; val<=20; val++)
								UA_EnableButtonQuiet(&RA8_GR[val]);

							/**** start - set button contents ****/

							sscanf(CDF_rec.ConnectionClass,"%d",&val);	// cc >= 1 !!
							UA_SetCycleGadgetToVal(window,&RA8_GR[8],val-1);

							sscanf(CDF_rec.BaudRate,"%d",&val);
							UA_SetStringGadgetToVal(window,&RA8_GR[11],val);

							sscanf(CDF_rec.UnitNumber,"%d",&val);
							UA_SetCycleGadgetToVal(window,&RA8_GR[12],val);

							if ( !strcmpi(CDF_rec.HandShaking, "RTS/CTS") ) { val=0; } else { val=1; }
							UA_SetCycleGadgetToVal(window,&RA8_GR[13],val);

							sscanf(CDF_rec.BufferSize,"%d",&val);
							UA_SetStringGadgetToVal(window,&RA8_GR[14],val);

							UA_SetStringGadgetToString(window,&RA8_GR[15],CDF_rec.DestinationPath);
							UA_SetStringGadgetToString(window,&RA8_GR[16],CDF_rec.PassWord);
							UA_SetStringGadgetToString(window,&RA8_GR[17],CDF_rec.SerialDevice);
							UA_SetStringGadgetToString(window,&RA8_GR[18],CDF_rec.ReplyString);
							UA_SetStringGadgetToString(window,&RA8_GR[19],CDF_rec.DialPrefix);
							UA_SetStringGadgetToString(window,&RA8_GR[20],CDF_rec.PhoneNr);

							/**** end - set button contents ****/

							/**** disable (some) buttons ****/

							DisableCDFButtons(window,RA8_GR,&CDF_rec);
							modifiedCDF=FALSE;
						}
					}
					break;

				case 10:	// Save CDF
					UA_HiliteButton(window, &RA8_GR[ID]);

					/**** start - get button contents ****/

					UA_SetValToCycleGadgetVal(&RA8_GR[8],&val);
					sprintf(CDF_rec.ConnectionClass,"%d",val+1);	// cc >= 1 !!

					UA_SetValToStringGadgetVal(&RA8_GR[11],&val);
					sprintf(CDF_rec.BaudRate,"%d",val);

					UA_SetValToCycleGadgetVal(&RA8_GR[12],&val);
					sprintf(CDF_rec.UnitNumber,"%d",val);

					UA_SetValToCycleGadgetVal(&RA8_GR[13],&val);
					if ( val==0 )
						strcpy(CDF_rec.HandShaking, "RTS/CTS");
					else if ( val==1 )
						strcpy(CDF_rec.HandShaking, "NONE");

					UA_SetValToStringGadgetVal(&RA8_GR[14],&val);
					sprintf(CDF_rec.BufferSize,"%d",val);

					UA_SetStringToGadgetString(&RA8_GR[15],CDF_rec.DestinationPath);
					UA_SetStringToGadgetString(&RA8_GR[16],CDF_rec.PassWord);
					UA_SetStringToGadgetString(&RA8_GR[17],CDF_rec.SerialDevice);
					UA_SetStringToGadgetString(&RA8_GR[18],CDF_rec.ReplyString);
					UA_SetStringToGadgetString(&RA8_GR[19],CDF_rec.DialPrefix);
					UA_SetStringToGadgetString(&RA8_GR[20],CDF_rec.PhoneNr);

					/**** end - get button contents ****/

					FRR.path			= path;
					FRR.fileName	= fileName;
					FRR.opts			= DIR_OPT_ALL | DIR_OPT_NOINFO;
					FRR.multiple	= FALSE;
					FRR.title			= msgs[Msg_CDF_12-1];
					if ( UA_SaveAFile(window, &FRR, mypattern1) )
					{
						UA_MakeFullPath(path, fileName, CDF_rec.CDF_Path);
						if ( !Write_CDF_File(&CDF_rec, CDF_rec.CDF_Path) )
						{
							loop=FALSE;
							retval=FALSE;
						}
						else
						{
							SaveCDFIcon(CDF_rec.CDF_Path);
							modifiedCDF=FALSE;
						}
					}
					break;

				case 11:	// Baudrate
				case 14:	// Buffer size
				case 15:	// Default path
				case 16:	// Password
				case 17:	// Serial Device
				case 18:	// Reply string
				case 19:	// Dial prefix
				case 20:	// Phone nr
					UA_ProcessStringGadget(window, RA8_GR, &RA8_GR[ID], &CED);
					modifiedCDF=TRUE;
					if ( ID==16 )
					{
						UA_SetStringToGadgetString(&RA8_GR[16], password1);
						if ( strlen(password1) < 8 )
						{
							strcpy(password2,"********");
							strncpy(password2,password1,strlen(password1));
							UA_SetStringGadgetToString(window, &RA8_GR[16], password2);
						}
						else if ( strlen(password1) > 8 )
						{
							password1[8] = '\0';
							UA_SetStringGadgetToString(window, &RA8_GR[16], password1);
						}
					}
					break;

				case 12:	// Unit
				case 13:	// Handshaking
					UA_ProcessCycleGadget(window, &RA8_GR[ID], &CED);
					modifiedCDF=TRUE;
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

	UA_CloseRequesterWindow(window, STDCOLORS);

	for(val=11; val<=20; val++)
		UA_EnableButtonQuiet(&RA8_GR[val]);

	return(retval);
}

/******** DisableCDFButtons() ********/

void DisableCDFButtons(	struct Window *window, struct GadgetRecord *GR,
												struct CDF_Record *CDF_rec )
{
int cc;
ULONG flags;

	// CC_SERIALCABLE=1, CC_PARALLELCABLE, CC_SCSI, CC_MODEM, CC_NETWORK, CC_DATABROADCAST,
	// CC_INFRARED, CC_EMAIL, CC_ISDN, CC_LEASED_LINE

	// CDF_CONNECTIONCLASS, CDF_SERIALDEVICE, CDF_UNITNUMBER, CDF_BAUDRATE, CDF_HANDSHAKING,
	// CDF_BUFFERSIZE, CDF_DIALPREFIX, CDF_PHONENR, CDF_PASSWORD, CDF_DESTINATIONPATH,
	// CDF_REPLYSTRING,

	flags = 0L;
	sscanf(CDF_rec->ConnectionClass,"%d",&cc);
	
	if ( cc==CC_SERIALCABLE )
	{
		flags = (
						(  1L << (CDF_SERIALDEVICE+1) ) |
						(  1L << (CDF_UNITNUMBER+1) ) |
						(  1L << (CDF_BAUDRATE+1) ) |
						(  1L << (CDF_HANDSHAKING+1) ) |
						(  1L << (CDF_BUFFERSIZE+1) ) |
						(  1L << (CDF_PASSWORD+1) ) 
						);
	}
	else if ( cc==CC_MODEM )
	{
		flags = (
						(  1L << (CDF_SERIALDEVICE+1) ) |
						(  1L << (CDF_UNITNUMBER+1) ) |
						(  1L << (CDF_BAUDRATE+1) ) |
						(  1L << (CDF_HANDSHAKING+1) ) |
						(  1L << (CDF_BUFFERSIZE+1) ) |
						(  1L << (CDF_DIALPREFIX+1) ) |
						(  1L << (CDF_PHONENR+1) ) |
						(  1L << (CDF_PASSWORD+1) ) |
						(  1L << (CDF_REPLYSTRING+1) )
						);
	}
	else if ( cc==CC_NETWORK )
	{
		flags = (
						(  1L << (CDF_DESTINATIONPATH+1) )
						);
	}
	else if ( cc==CC_LEASED_LINE )
	{
		flags = (
						(  1L << (CDF_SERIALDEVICE+1) ) |
						(  1L << (CDF_UNITNUMBER+1) ) |
						(  1L << (CDF_BAUDRATE+1) ) |
						(  1L << (CDF_HANDSHAKING+1) ) |
						(  1L << (CDF_BUFFERSIZE+1) ) |
						(  1L << (CDF_PASSWORD+1) ) |
						(  1L << (CDF_REPLYSTRING+1) )
						);
	}

	if ( flags & (1L << (CDF_SERIALDEVICE+1)) )
		UA_EnableButton(window, &GR[17]);
	else
		UA_DisableButton(window, &GR[17], mypattern1);

	if ( flags & (1L << (CDF_UNITNUMBER+1)) )
		UA_EnableButton(window, &GR[12]);
	else
		UA_DisableButton(window, &GR[12], mypattern1);

	if ( flags & (1L << (CDF_BAUDRATE+1)) )
		UA_EnableButton(window, &GR[11]);
	else
		UA_DisableButton(window, &GR[11], mypattern1);

	if ( flags & (1L << (CDF_HANDSHAKING+1)) )
		UA_EnableButton(window, &GR[13]);
	else
		UA_DisableButton(window, &GR[13], mypattern1);

	if ( flags & (1L << (CDF_BUFFERSIZE+1)) )
		UA_EnableButton(window, &GR[14]);
	else
		UA_DisableButton(window, &GR[14], mypattern1);

	if ( flags & (1L << (CDF_DIALPREFIX+1)) )
		UA_EnableButton(window, &GR[19]);
	else
		UA_DisableButton(window, &GR[19], mypattern1);

	if ( flags & (1L << (CDF_PHONENR+1)) )
		UA_EnableButton(window, &GR[20]);
	else
		UA_DisableButton(window, &GR[20], mypattern1);

	if ( flags & (1L << (CDF_PASSWORD+1)) )
		UA_EnableButton(window, &GR[16]);
	else
		UA_DisableButton(window, &GR[16], mypattern1);

	if ( flags & (1L << (CDF_DESTINATIONPATH+1)) )
		UA_EnableButton(window, &GR[15]);
	else
		UA_DisableButton(window, &GR[15], mypattern1);

	if ( flags & (1L << (CDF_REPLYSTRING+1)) )
		UA_EnableButton(window, &GR[18]);
	else
		UA_DisableButton(window, &GR[18], mypattern1);
}

/******** E O F ********/
