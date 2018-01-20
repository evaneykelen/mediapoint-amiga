#include "nb:pre.h"
#include "setup.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"
#include "structs.h"

#define VERSI0N "\0$VER: MediaPoint All4One xapp 0.1"
static UBYTE *vers = VERSI0N;

/**** internal function declarations ****/

BOOL MonitorUser(	struct Window *window, PROCESSINFO *ThisPI,
									struct VuPort_record *vp_rec,
									struct UserApplicInfo *UAI);

/**** external function declarations ****/

extern void GetVarsFromPI(struct VuPort_record *vp_rec, PROCESSINFO *ThisPI);
extern void PutVarsToPI(struct VuPort_record *vp_rec, PROCESSINFO *ThisPI);
void GetInfoFile(	PROCESSINFO *ThisPI, STRPTR appName, STRPTR devName, int *portNr,
									int *baudRate);

/**** globals ****/

struct Library *medialinkLibBase		= NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase							= NULL;
struct Library *IconBase						= NULL;

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
struct VuPort_record vp_rec;
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

	/**** parse .info file for special settings ****/

	GetInfoFile(ThisPI, "All4One", vp_rec.devName, &(vp_rec.portNr), &(vp_rec.baudRate));

	/**** open special libraries or devices ****/

	if ( !Open_SerialPort((struct standard_record *)&vp_rec,8,8,1) )
		return;

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
		UA_DoubleGadgetDimensions(VP_GR);

	//UA_TranslateGR(VP_GR, msgs);

	/**** open the window ****/

	capsport = (struct MsgPort *)FindPort(MEDIALINKPORT);
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	UAI.windowX				= -1;	/* -1 means center on screen */
	UAI.windowY				= -1;	/* -1 means center on screen */
	UAI.windowWidth		= VP_GR[0].x2;
	UAI.windowHeight	= VP_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(&UAI);

	vp_rec.window = UAI.userWindow;

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, VP_GR);

	/**** monitor events ****/

	MonitorUser(UAI.userWindow, ThisPI, &vp_rec, &UAI);

	/**** close the window ****/

	UA_CloseWindow(&UAI);

	/**** close specials libraries or devices ****/

	Close_SerialPort((struct standard_record *)&vp_rec);

	capsport->mp_SigTask = oldTask;
}

/******** MonitorUser() ********/

BOOL MonitorUser(	struct Window *window, PROCESSINFO *ThisPI,
									struct VuPort_record *vp_rec, struct UserApplicInfo *UAI)
{
BOOL loop=TRUE, retVal;
struct EventData CED;
int ID;
UWORD *mypattern1;

	mypattern1 = (UWORD *)AllocMem(4L, MEMF_CHIP);
	if (mypattern1==NULL)
		return(FALSE);
	mypattern1[0] = 0x5555;
	mypattern1[1] = 0xaaaa;

	/**** parse string ****/

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
		GetVarsFromPI(vp_rec, ThisPI);
	else
	{
		vp_rec->command = 0;	// play
		vp_rec->unit = 0;			// unit 1
	}

	/**** set buttons ****/

	UA_SetCycleGadgetToVal(window, &VP_GR[2], vp_rec->command);
	UA_SetCycleGadgetToVal(window, &VP_GR[3], vp_rec->unit);

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window, &CED);
		if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, VP_GR, &CED);
			switch(ID)
			{
				case 2:	// command
					UA_ProcessCycleGadget(window, &VP_GR[ID], &CED);
					UA_SetValToCycleGadgetVal(&VP_GR[ID], &vp_rec->command);
					break;

				case 3:	// unit#
					UA_ProcessCycleGadget(window, &VP_GR[ID], &CED);
					UA_SetValToCycleGadgetVal(&VP_GR[ID], &vp_rec->unit);
					break;

				case 4:	// OK
do_ok:
					UA_HiliteButton(window, &VP_GR[4]);
					retVal=TRUE;
					loop=FALSE;
					break;

				case 5:	// Cancel
do_cancel:
					UA_HiliteButton(window, &VP_GR[5]);
					retVal=FALSE;
					loop=FALSE;
					break;

				case 6:	// Preview
					UA_InvertButton(window, &VP_GR[ID]);
					UA_SetSprite(window, SPRITE_BUSY);
					if (!PerformActions(vp_rec))
						GiveMessage(window, msgs[Msg_X_3-1]); // "Communication with device aborted."
					UA_SetSprite(window, SPRITE_NORMAL);
					UA_InvertButton(window, &VP_GR[ID]);
					break;
			}
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
		PutVarsToPI(vp_rec, ThisPI);

	return(retVal);
}

/******** E O F ********/
