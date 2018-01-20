#include "nb:pre.h"
#include "print_gadgets.h"
#include "protos.h"

#define VERSION	"\0$VER: 2.3"

/**** functions declarations ****/

void main(int argc, char **argv);
BOOL doYourThing(void);
BOOL MonitorUser(struct Window *window);

/**** globals ****/

struct RendezVousRecord *rvrec;
struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *medialinkLibBase;
struct Library *ConsoleDevice;
static UBYTE *vers = VERSION;
struct EventData CED;
UWORD chip mypattern1[] = { 0x5555, 0xaaaa };
struct Screen *theScreen;
struct EditWindow *EW;
struct Window *printWindow;
UBYTE **msgs;
struct MsgPort *capsport;

char *scriptCommands[] = {
"ANIM", "AREXX", "XAPP", "DOS", "STARTSER", "ENDSER", "STARTPAR", "ENDPAR",
"SOUND", "START", "END", "DURATION", "PROGRAM", "SCRIPTTALK", "PAGE", "COUNTER",
"GOTO", "DATA", "MAIL", "GLOBALEVENT", "TIMECODE", "LABEL", "NOP", "LOCALEVENT",
"VARIABLES", "INPUTSETTINGS",
NULL }; /* ALWAYS END WITH NULL! */

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

	/**** drain it ****/

	IntuitionBase 		= (struct IntuitionBase *)rvrec->intuition;
	GfxBase 					= (struct GfxBase *)rvrec->graphics;
	medialinkLibBase	= (struct Library *)rvrec->medialink;
	ConsoleDevice			= (struct Library *)rvrec->console;
	msgs							= (UBYTE **)rvrec->msgs;

	/**** translate gadgets ****/

	UA_TranslateGR(Print_GR,msgs);

	/**** play around ****/

	if ( doYourThing() )
		rvrec->returnCode = TRUE;
	else
		rvrec->returnCode = FALSE;

	/**** and leave the show ****/

	capsport->mp_SigTask = oldTask;

	exit(0);
}

/******** doYourThing() ********/

BOOL doYourThing(void)
{
BOOL retval;
struct UserApplicInfo UAI;

	/**** initialize the User Application Info structure ***/

	UA_InitStruct(&UAI);
	UAI.IB = IntuitionBase;

	/**** open a window ****/

	UAI.windowModes = 2;	/* open on MY screen */

	if (rvrec->ehi->activeScreen == STARTSCREEN_PAGE)
		UAI.userScreen = rvrec->pagescreen;
	else if (rvrec->ehi->activeScreen == STARTSCREEN_SCRIPT)
		UAI.userScreen = rvrec->scriptscreen;
	else
		return(FALSE);

	theScreen = UAI.userScreen;

	/**** double the dimensions of gadgets etc. if screen is laced ****/

	if ( UA_IsUAScreenLaced(&UAI) )
		UA_DoubleGadgetDimensions(Print_GR);

	UAI.windowX			 	= -1;	/* -1 means center on screen */
	UAI.windowY			 	= -1;	/* -1 means center on screen */
	UAI.windowWidth	 	= Print_GR[0].x2;
	UAI.windowHeight 	= Print_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;

	/**** set the right font for this window ****/

	UAI.small_TF = rvrec->smallfont;
	UAI.large_TF = rvrec->largefont;

	UA_OpenWindow(&UAI);
	UA_SetMenuColors(rvrec,UAI.userWindow);

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, Print_GR);

	if ( rvrec->returnCode == 1 )	// indicates print script	or page
		UA_DrawText(UAI.userWindow, &Print_GR[2], msgs[Msg_PrintScript-1]);
	else
		UA_DrawText(UAI.userWindow, &Print_GR[2], msgs[Msg_PrintPage-1]);

	/**** get window list ****/

//	Message("%d %d", rvrec->EW[0]->X, rvrec->EW[0]->Y);

//	Message("%d %d", rvrec->EW[1]->X, rvrec->EW[1]->Y);

/***
	i=0;
	while ( *(EW+i) != NULL )
	{
		printf("%d %d\n", *(EW[i]->X, EW[i]->Y);
		i++;
	}
***/

	/**** process events ****/
	
	retval = MonitorUser(UAI.userWindow);
	UA_ResetMenuColors(rvrec,UAI.userWindow);

	/**** close the window ****/

	UA_CloseWindow(&UAI);

	return(retval);
}

/******** MonitorUser() ********/

BOOL MonitorUser(struct Window *window)
{
BOOL loop=TRUE, retval;
int ID, i, j, val;
UBYTE ORI_printerDest;
BOOL ORI_printerTextOnly;
int ORI_printerCopies;
BOOL multipleFiles=FALSE;

	ORI_printerDest			= rvrec->capsprefs->printerDest;
	ORI_printerTextOnly = rvrec->capsprefs->printerTextOnly;
	ORI_printerCopies		= rvrec->capsprefs->printerCopies;

	if ( rvrec->returnCode == 1 )	// print script
	{
		UA_InvertButton(window, &Print_GR[4]);
		UA_DisableButton(window, &Print_GR[5], mypattern1);
		UA_DisableButton(window, &Print_GR[6], mypattern1);
		UA_DisableButton(window, &Print_GR[7], mypattern1);
		UA_DisableButton(window, &Print_GR[9], mypattern1);		// text only
		UA_DisableButton(window, &Print_GR[14], mypattern1);	// multiple files
	}
	else	// print page
	{
		for(i=0, j=PRINTER_DEST_PRINTER; i<4; i++, j++)
			if (rvrec->capsprefs->printerDest == j)
				UA_InvertButton(window, &Print_GR[4+i]);
		if (rvrec->capsprefs->printerTextOnly)
			UA_InvertButton(window, &Print_GR[9]);
	}

	UA_SetStringGadgetToVal(window, &Print_GR[10], (int)rvrec->capsprefs->printerCopies);

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		switch(CED.Class)
		{
			case IDCMP_MOUSEBUTTONS:
				if (CED.Code==SELECTDOWN)
				{
					ID = UA_CheckGadgetList(window, Print_GR, &CED);
					switch(ID)
					{
						case 4:
						case 5:
						case 6:
						case 7:
							UA_InvertButton(window,
											&Print_GR[ 4 + rvrec->capsprefs->printerDest -1 ]);
							rvrec->capsprefs->printerDest = ID-3;
							UA_InvertButton(window,
											&Print_GR[ 4 + rvrec->capsprefs->printerDest -1 ]);
							break;

						case 9:
							UA_InvertButton(window, &Print_GR[9]);
							if (rvrec->capsprefs->printerTextOnly)
								rvrec->capsprefs->printerTextOnly=FALSE;
							else
								rvrec->capsprefs->printerTextOnly=TRUE;
							break;

						case 10:
							UA_ProcessStringGadget(window, Print_GR, &Print_GR[ID], &CED);
							UA_SetValToStringGadgetVal(&Print_GR[ID], &val);
							rvrec->capsprefs->printerCopies = val;
							break;

						case 11: /* Cancel */
do_cancel:
							UA_HiliteButton(window, &Print_GR[11]);
							loop=FALSE;
							retval=FALSE;
							break;

						case 12: /* OK */
do_ok:
							UA_HiliteButton(window, &Print_GR[12]);
							loop=FALSE;
							retval=TRUE;
							break;

						case 14:
							UA_InvertButton(window, &Print_GR[ID]);
							if (multipleFiles)
								multipleFiles=FALSE;
							else
								multipleFiles=TRUE;
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
		if ( rvrec->returnCode == 1 )	// print script
			PrintScript();
		else													// print page
			PrintPage();
	}

	if ( !retval )
	{
		rvrec->capsprefs->printerDest			= ORI_printerDest;
		rvrec->capsprefs->printerTextOnly	= ORI_printerTextOnly;
		rvrec->capsprefs->printerCopies		= ORI_printerCopies;
	}

	return(retval);
}

/******** GetObjInfo() ********/

void GetObjInfo(struct ScriptNodeRecord *this_node, STRPTR objectName)
{
	if (this_node->nodeType == TALK_AREXX || this_node->nodeType == TALK_DOS)
	{
		if (this_node->extraData != NULL && this_node->extraData[0] != '\0')
			sprintf(objectName, "\"%s\"", this_node->extraData);
		else
			stccpy(objectName, msgs[Msg_Comment-1], MAX_OBJECTNAME_CHARS);
	}
	else
	{
		if (this_node->objectName[0] != '\0')
		{
			if (this_node->nodeType == TALK_LABEL ||
					this_node->nodeType == TALK_STARTSER ||
					this_node->nodeType == TALK_STARTPAR ||
					this_node->nodeType == TALK_USERAPPLIC )
				sprintf(objectName, "\"%s\"", this_node->objectName);
			else
				stccpy(objectName, this_node->objectName, MAX_OBJECTNAME_CHARS);
		}
		else
		{
			if ( this_node->nodeType == TALK_USERAPPLIC )
				stccpy(objectName, msgs[Msg_Comment-1], MAX_OBJECTNAME_CHARS);
			else
				stccpy(objectName, msgs[Msg_Untitled-1], MAX_OBJECTNAME_CHARS);
		}
	}

	if (this_node->nodeType == TALK_NOP)
	{
		if (strcmpi(objectName, msgs[Msg_Untitled-1])==0)
			objectName[0] = '\0';
	}
}

/******** OpenPrintingWindow() ********/

BOOL OpenPrintingWindow(STRPTR printWhat)
{
TEXT mess[256];

	if ( theScreen->ViewPort.Modes & LACE )
		UA_DoubleGadgetDimensions(PrintWindow_GR);

	printWindow = UA_OpenRequesterWindow(theScreen->FirstWindow, PrintWindow_GR, USECOLORS);
	if (printWindow==NULL)
	{
		if ( theScreen->ViewPort.Modes & LACE )
			UA_HalveGadgetDimensions(PrintWindow_GR);
		UA_WarnUser(211);
		return(FALSE);
	}

	UA_DrawGadgetList(printWindow, PrintWindow_GR);
	UA_DrawText(printWindow, &PrintWindow_GR[2], msgs[Msg_Stop-1]);

	sprintf(mess, msgs[Msg_ScriptPrinting-1], printWhat);	// Now printing %s (script/doc)

	UA_printSeveralLines(	printWindow, &PrintWindow_GR[0],
												15, printWindow->RPort->TxBaseline+5,
												PrintWindow_GR[0].x2-15, PrintWindow_GR[0].y2-5, mess);

	if ( theScreen->ViewPort.Modes & LACE )
		UA_HalveGadgetDimensions(PrintWindow_GR);

	return(TRUE);
}

/******** ClosePrintingWindow() ********/

void ClosePrintingWindow(void)
{
	UA_CloseRequesterWindow(printWindow,USECOLORS);
}

/******** E O F ********/
