#include "nb:pre.h"
#include "pagesetup_gadgets.h"
#include "vectors.h"

#define VERSION	"\0$VER: 2.2"

/**** functions declarations ****/

void main(int argc, char **argv);
BOOL doYourThing(void);
BOOL MonitorUser(struct Window *window);
UBYTE GetWBPrinterPrefs(BOOL getonlySERPAR);

/**** externals ****/

extern UWORD chip mypattern1[];

/**** globals ****/

struct RendezVousRecord *rvrec;
struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *medialinkLibBase;
static UBYTE *vers = VERSION;
struct EventData CED;
UBYTE **msgs;
struct MsgPort *capsport;

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
	msgs							= (UBYTE **)rvrec->msgs;

	/**** translate gadgets ****/

	UA_TranslateGR(PageSetup_GR,msgs);

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
struct UserApplicInfo UAI;
BOOL retval;

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

	/**** double the dimensions of gadgets etc. if screen is laced ****/

	if ( UA_IsUAScreenLaced(&UAI) )
	{
		UA_DoubleGadgetDimensions(PageSetup_GR);
		UA_DoubleVectorDimensions(Portrait_VR);
		UA_DoubleVectorDimensions(Portrait4x4_VR);
		UA_DoubleVectorDimensions(Landscape_VR);
		UA_DoubleVectorDimensions(Landscape4x4_VR);
	}

	UAI.windowX			 	= -1;	/* -1 means center on screen */
	UAI.windowY			 	= -1;	/* -1 means center on screen */
	UAI.windowWidth	 	= PageSetup_GR[0].x2;
	UAI.windowHeight	= PageSetup_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;

	/**** set the right font for this window ****/

	UAI.small_TF = rvrec->smallfont;
	UAI.large_TF = rvrec->largefont;

	UA_OpenWindow(&UAI);
	UA_SetMenuColors(rvrec,UAI.userWindow);

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, PageSetup_GR);
	UA_SetCycleGadgetToVal(	UAI.userWindow, &PageSetup_GR[4],
													rvrec->capsprefs->printerScaleAndOri-1);
	if ( rvrec->capsprefs->printerQuality == PRINTER_QUALITY_DRAFT )
		UA_InvertButton(UAI.userWindow, &PageSetup_GR[7]);
	else if ( rvrec->capsprefs->printerQuality == PRINTER_QUALITY_LETTER )
		UA_InvertButton(UAI.userWindow, &PageSetup_GR[8]);

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
int ID, cycleVal;
UBYTE ORI_printerScaleAndOri, ORI_printerQuality;

	ORI_printerScaleAndOri = rvrec->capsprefs->printerScaleAndOri;
	ORI_printerQuality		 = rvrec->capsprefs->printerQuality;

	/**** render vector drawing ****/

	UA_SetValToCycleGadgetVal(&PageSetup_GR[4], &cycleVal);
	cycleVal++;
	if (cycleVal==PRINTER_ORI_LANDSCAPE)
		UA_DrawVectorList(window, Landscape_VR);
	else if (cycleVal==PRINTER_ORI_LANDSCAPE4X4)
		UA_DrawVectorList(window, Landscape4x4_VR);
	else if (cycleVal==PRINTER_ORI_PORTRAIT)
		UA_DrawVectorList(window, Portrait_VR);
	else if (cycleVal==PRINTER_ORI_PORTRAIT4X4)
		UA_DrawVectorList(window, Portrait4x4_VR);

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		switch(CED.Class)
		{
			case IDCMP_MOUSEBUTTONS:
				if (CED.Code==SELECTDOWN)
				{
					ID = UA_CheckGadgetList(window, PageSetup_GR, &CED);
					switch(ID)
					{
						case 4:
							UA_ProcessCycleGadget(window, &PageSetup_GR[4], &CED);
							UA_SetValToCycleGadgetVal(&PageSetup_GR[4], &cycleVal);
							cycleVal++;
							UA_ClearButton(window, &PageSetup_GR[5], AREA_PEN);
							if (cycleVal==PRINTER_ORI_LANDSCAPE)
								UA_DrawVectorList(window, Landscape_VR);
							else if (cycleVal==PRINTER_ORI_LANDSCAPE4X4)
								UA_DrawVectorList(window, Landscape4x4_VR);
							else if (cycleVal==PRINTER_ORI_PORTRAIT)
								UA_DrawVectorList(window, Portrait_VR);
							else if (cycleVal==PRINTER_ORI_PORTRAIT4X4)
								UA_DrawVectorList(window, Portrait4x4_VR);
							break;

						case 7:
							if (rvrec->capsprefs->printerQuality != PRINTER_QUALITY_DRAFT)
							{
								UA_InvertButton(window, &PageSetup_GR[7]);
								UA_InvertButton(window, &PageSetup_GR[8]);
								rvrec->capsprefs->printerQuality = PRINTER_QUALITY_DRAFT;
							}
							break;

						case 8:
							if (rvrec->capsprefs->printerQuality != PRINTER_QUALITY_LETTER)
							{
								UA_InvertButton(window, &PageSetup_GR[7]);
								UA_InvertButton(window, &PageSetup_GR[8]);
								rvrec->capsprefs->printerQuality = PRINTER_QUALITY_LETTER;
							}
							break;

						case 9: /* cancel */
do_cancel:
							UA_HiliteButton(window, &PageSetup_GR[9]);
							loop=FALSE;
							retval=FALSE;
							break;

						case 10:	// OK
do_ok:
							UA_HiliteButton(window, &PageSetup_GR[10]);
							loop=FALSE;
							retval=TRUE;
							break;

						case 12:	// get wb prefs
							UA_HiliteButton(window, &PageSetup_GR[12]);
							if (rvrec->capsprefs->printerQuality == PRINTER_QUALITY_DRAFT)
								UA_InvertButton(window, &PageSetup_GR[7]);
							else
								UA_InvertButton(window, &PageSetup_GR[8]);
							GetWBPrinterPrefs(FALSE);
							cycleVal = rvrec->capsprefs->printerScaleAndOri-1;
							UA_SetCycleGadgetToVal(window, &PageSetup_GR[4], cycleVal);
							UA_ClearButton(window, &PageSetup_GR[5], AREA_PEN);
							if (rvrec->capsprefs->printerScaleAndOri==PRINTER_ORI_LANDSCAPE)
								UA_DrawVectorList(window, Landscape_VR);
							else if (rvrec->capsprefs->printerScaleAndOri==PRINTER_ORI_PORTRAIT)
								UA_DrawVectorList(window, Portrait_VR);
							if (rvrec->capsprefs->printerQuality == PRINTER_QUALITY_DRAFT)
								UA_InvertButton(window, &PageSetup_GR[7]);
							else
								UA_InvertButton(window, &PageSetup_GR[8]);
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
		UA_SetValToCycleGadgetVal(&PageSetup_GR[4], &cycleVal);
		rvrec->capsprefs->printerScaleAndOri = cycleVal+1;
	}
	else
	{
		rvrec->capsprefs->printerScaleAndOri	= ORI_printerScaleAndOri;
		rvrec->capsprefs->printerQuality			= ORI_printerQuality;
	}

	return(retval);
}

/******** GetWBPrinterPrefs() ********/

UBYTE GetWBPrinterPrefs(BOOL getonlySERPAR)
{
struct Preferences *PrefBuffer;
UBYTE retval=2;

	PrefBuffer = (struct Preferences *)AllocMem(sizeof(struct Preferences),
																							(ULONG)MEMF_CLEAR);
	if (PrefBuffer == NULL)
	{
//		WarnUser(132);
		return(2);
	}

	GetPrefs(PrefBuffer, sizeof(struct Preferences));

	if (!getonlySERPAR)
	{
		if (PrefBuffer->PrintAspect == ASPECT_VERT)
			rvrec->capsprefs->printerScaleAndOri = PRINTER_ORI_PORTRAIT;
		else
			rvrec->capsprefs->printerScaleAndOri = PRINTER_ORI_LANDSCAPE;

		if (PrefBuffer->PrintQuality == DRAFT)
			rvrec->capsprefs->printerQuality = PRINTER_QUALITY_DRAFT;
		else if (PrefBuffer->PrintQuality == LETTER)
			rvrec->capsprefs->printerQuality = PRINTER_QUALITY_LETTER;
	}
	else
	{
		retval = PrefBuffer->PrinterPort;
		rvrec->capsprefs->paperLength = PrefBuffer->PaperLength;
	}
	
	FreeMem(PrefBuffer, sizeof(struct Preferences));

	return(retval);
}

/******** E O F ********/
