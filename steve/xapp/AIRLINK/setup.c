#include "nb:pre.h"
#include "setup.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "structs.h"
#include "demo:gen/support_protos.h"

#define VERSI0N "\0$VER: 1.0"
static UBYTE *vers = VERSI0N;

/**** internal function declarations ****/

BOOL MonitorUser(	struct Window *window, PROCESSINFO *ThisPI,
									struct AirLink_record *al_rec,
									struct UserApplicInfo *UAI);
BOOL GetAirLinkButtons(struct Window *window, struct AirLink_record *al_rec, struct PopUpRecord *PUR);
BOOL PushButton(struct AirLink_record *al_rec);
void GetArgListMaxWidth(char *oriStr, int *maxw, int *num);
void CreateListFromArgList(char *oriStr, UBYTE *list, int maxw);

/**** external function declarations ****/

extern void GetVarsFromPI(struct AirLink_record *al_rec, PROCESSINFO *ThisPI);
extern void PutVarsToPI(struct AirLink_record *al_rec, PROCESSINFO *ThisPI);

/**** globals ****/

struct Library *medialinkLibBase		= NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase							= NULL;
struct Library *IconBase						= NULL;

UBYTE **msgs;
struct RendezVousRecord *rvrec;
struct MsgPort *capsport;

UBYTE *butlist=NULL;

/**** disable CTRL-C break ****/

void CXBRK(void) { }
void chkabort(void) { }

/**** functions ****/

/******** XappSetup() ********/

void XappSetup(PROCESSINFO *ThisPI)
{
struct UserApplicInfo UAI;
struct AirLink_record al_rec;
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
		UA_DoubleGadgetDimensions(AIR_GR);

	UA_TranslateGR(AIR_GR, msgs);

	/**** open the window ****/

	capsport = (struct MsgPort *)FindPort(MEDIALINKPORT);
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	UAI.windowX				= -1;	/* -1 means center on screen */
	UAI.windowY				= -1;	/* -1 means center on screen */
	UAI.windowWidth		= AIR_GR[0].x2;
	UAI.windowHeight	= AIR_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(&UAI);

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, AIR_GR);
	UA_DrawSpecialGadgetText(UAI.userWindow, &AIR_GR[10], msgs[Msg_Air_3-1], SPECIAL_TEXT_BEFORE_STRING);

	/**** monitor events ****/

	butlist=(UBYTE *)AllocMem(4096L,MEMF_CLEAR);
	if (butlist)
	{
		MonitorUser(UAI.userWindow, ThisPI, &al_rec, &UAI);
		FreeMem(butlist,4096L);
	}

	/**** close the window ****/

	UA_CloseWindow(&UAI);

	capsport->mp_SigTask = oldTask;
}

/******** MonitorUser() ********/

BOOL MonitorUser(	struct Window *window, PROCESSINFO *ThisPI,
									struct AirLink_record *al_rec, struct UserApplicInfo *UAI)
{
BOOL loop=TRUE, retVal;
struct EventData CED;
int ID,i;
UWORD *mypattern1;
struct PopUpRecord PUR;
TEXT str[128];

	mypattern1 = (UWORD *)AllocMem(4L, MEMF_CHIP);
	if (mypattern1==NULL)
		return(FALSE);
	mypattern1[0] = 0x5555;
	mypattern1[1] = 0xaaaa;

	PUR.window = NULL;
	PUR.GR = PopUp_GR;
	PUR.ptr = NULL;
	PUR.active = 0;
	PUR.number = 1;
	PUR.width = 19;
	PUR.fit = 0;
	PUR.top = 0;

	/**** parse string ****/

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
	{
		GetVarsFromPI(al_rec, ThisPI);
		//UA_PrintPopUpChoice2(window, &AIR_GR[10], al_rec->buttonName);
	}
	else
	{
		strcpy(al_rec->portName, "AIR");
		al_rec->buttonName[0] = '\0';
		al_rec->active = 0;

		UA_PrintPopUpChoice(window, &AIR_GR[10], &PUR);
		UA_DisableButton(window, &AIR_GR[10], mypattern1);
	}

	/**** check if some AIR is around -- if not disable all gadgets ****/

	i=0;
	strcpy(str,"AIR");
	do
	{
		if (FindPort(str))
		{
			if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] == '\0' )
				strcpy(al_rec->portName,str);
			i=-1;
			break;
		}
		sprintf(str,"AIR.%02d",i);
		i++;
	}
	while(i<=32);

	if (i!=-1)	// no AIR around
	{
		GiveMessage(window, msgs[Msg_Air1-1]);
		UA_DisableButton(window, &AIR_GR[ 6], mypattern1);
		UA_DisableButton(window, &AIR_GR[ 8], mypattern1);
		UA_DisableButton(window, &AIR_GR[ 9], mypattern1);
		UA_DisableButton(window, &AIR_GR[10], mypattern1);
	}
	else
	{
		UA_SetStringGadgetToString(window, &AIR_GR[8], al_rec->portName);

		UA_EnableButton(window, &AIR_GR[10]);
		if ( GetAirLinkButtons(window,al_rec,&PUR) )
		{
			if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
			{
				if ( al_rec->active < PUR.number )
					PUR.active = al_rec->active;
			}

			UA_PrintPopUpChoice(window, &AIR_GR[10], &PUR);
			stccpy(al_rec->buttonName,PUR.ptr+PUR.active*PUR.width,255);
		}
		else
		{	
			PUR.ptr = NULL;
			UA_PrintPopUpChoice(window, &AIR_GR[10], &PUR);
			UA_DisableButton(window, &AIR_GR[10], mypattern1);
		}						
	}

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window, &CED);
		if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, AIR_GR, &CED);
			switch(ID)
			{
				case 4:		// OK
do_ok:
					UA_HiliteButton(window, &AIR_GR[4]);
					retVal=TRUE;
					loop=FALSE;
					break;

				case 5:		// Cancel
do_cancel:
					UA_HiliteButton(window, &AIR_GR[5]);
					retVal=FALSE;
					loop=FALSE;
					break;

				case 6:		// Preview
					UA_InvertButton(window, &AIR_GR[ID]);
					UA_SetSprite(window, SPRITE_BUSY);
					if ( !PushButton(al_rec) )
						GiveMessage(window, msgs[Msg_Air1-1]);
					UA_SetSprite(window, SPRITE_NORMAL);
					UA_InvertButton(window, &AIR_GR[ID]);
					break;

				case 8:		// Port name
					UA_ProcessStringGadget(window, AIR_GR, &AIR_GR[ID], &CED);
					UA_SetStringToGadgetString(&AIR_GR[ID], al_rec->portName);
					ID=9;	// go on to next case...

				case 9:		// Get button names
					UA_HiliteButton(window, &AIR_GR[ID]);
					UA_EnableButton(window, &AIR_GR[10]);
					if ( GetAirLinkButtons(window,al_rec,&PUR) )
					{
						UA_PrintPopUpChoice(window, &AIR_GR[10], &PUR);
						stccpy(al_rec->buttonName,PUR.ptr+PUR.active*PUR.width,255);
					}
					else
					{
						PUR.ptr = NULL;
						UA_PrintPopUpChoice(window, &AIR_GR[10], &PUR);
						UA_DisableButton(window, &AIR_GR[10], mypattern1);
					}						
					break;

				case 10:	// Do...
					UA_InvertButton(window, &AIR_GR[ID]);
					if ( UA_OpenPopUpWindow(window, &AIR_GR[ID], &PUR) )
					{
						UA_Monitor_PopUp(&PUR);
						UA_ClosePopUpWindow(&PUR);
						stccpy(al_rec->buttonName,PUR.ptr+PUR.active*PUR.width,255);
					}
					UA_InvertButton(window, &AIR_GR[ID]);
					UA_PrintPopUpChoice(window, &AIR_GR[10], &PUR);
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
	{
		al_rec->active=PUR.active;
		PutVarsToPI(al_rec, ThisPI);
	}

	return(retVal);
}

/******** GetAirLinkButtons() ********/

BOOL GetAirLinkButtons(struct Window *window, struct AirLink_record *al_rec, struct PopUpRecord *PUR)
{
char *tmp;
int maxw,num;
char str[256];

	if ( !FindPort(al_rec->portName) )
	{
		sprintf(str,msgs[Msg_Air2-1],al_rec->portName);
		GiveMessage(window,str);
		return(FALSE);
	}

	tmp = (UBYTE *)AllocMem(4096L,MEMF_CLEAR);
	if (!tmp)
		return(FALSE);

	if ( !UA_IssueRexxCmd_V2("MP_AIR",al_rec->portName,"Get_gadget_names",tmp,NULL) )
	{
		FreeMem(tmp,4096L);
		return(FALSE);
	}

	GetArgListMaxWidth(tmp,&maxw,&num);

	if ( maxw==0 || num==0 )
	{
		FreeMem(tmp,4096L);
		return(FALSE);
	}

	maxw++;
	CreateListFromArgList(tmp,butlist,maxw);

	PUR->ptr = butlist;
	PUR->active = 0;
	PUR->number = num;
	PUR->width = maxw;
	PUR->fit = 0;
	PUR->top = 0;

	FreeMem(tmp,4096L);

	return(TRUE);
}

/******** GetArgListMaxWidth() ********/

void GetArgListMaxWidth(char *oriStr, int *maxw, int *num)
{
int numChars, argNum, len, numSeen;
char *strPtr;

	strPtr = oriStr;
	len = strlen(strPtr);
	argNum=0;
	numSeen=0;
	*maxw=0;
	*num=0;
	while(1)
	{
		if (*strPtr==0 || *strPtr==0x0a)	// 0x0a is specific for this case!!!!!!!!!!!
			break;
		numChars = stcarg(strPtr, " 	,");	// space, tab, comma
		if (numChars>=1)
		{
			if ( *maxw < numChars )
				*maxw = numChars;
			argNum++;
		}
		strPtr += numChars+1;
		numSeen += numChars+1;
		if (numSeen>len)
			break;		
	}
	*num = argNum;
}

/******** CreateListFromArgList() ********/

void CreateListFromArgList(char *oriStr, UBYTE *list, int maxw)
{
int numChars, argNum, len, numSeen;
char *strPtr;

	strPtr = oriStr;
	len = strlen(strPtr);
	argNum=0;
	numSeen=0;
	while(1)
	{
		if (*strPtr==0 || *strPtr==0x0a)	// 0x0a is specific for this case!!!!!!!!!!!
			break;
		numChars = stcarg(strPtr, " 	,");	// space, tab, comma
		if (numChars>=1)
		{
			stccpy(list+argNum*maxw, strPtr, numChars+1);
			argNum++;
		}
		strPtr += numChars+1;
		numSeen += numChars+1;
		if (numSeen>len)
			break;		
	}
}

/******** PushButton() ********/

BOOL PushButton(struct AirLink_record *al_rec)
{
char tmp[256];

	if ( !FindPort(al_rec->portName) )
		return(FALSE);

	sprintf(tmp,"output IR<%s>",al_rec->buttonName);

	return( UA_IssueRexxCmd_V2("MP_AIR",al_rec->portName,tmp,NULL,NULL) );
}

/******** E O F ********/
