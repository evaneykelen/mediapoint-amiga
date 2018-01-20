#include "nb:pre.h"
#include "setup.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "structs.h"
#include "protos.h"
#include <graphics/videocontrol.h>
#include "demo:gen/support_protos.h"

#define VERSI0N "\0$VER: 1.2"
static UBYTE *vers = VERSI0N;

/**** internal function declarations ****/

BOOL MonitorUser(	struct Window *window, PROCESSINFO *ThisPI,
									struct Neptun_record *nep_rec,
									struct UserApplicInfo *UAI);
void DrawNepButtons(struct Window *window, struct Neptun_record *nep_rec);
void CheckOtherButtons(	struct Window *window, struct Neptun_record *nep_rec,
												struct EventData *CED );
void GenlockOn(struct Screen *screen);
void GenlockOff(struct Screen *screen);
BOOL CheckDuration(char *str);
void WaitForUser(struct Window *window);

/**** external function declarations ****/

extern void GetVarsFromPI(struct Neptun_record *nep_rec, PROCESSINFO *ThisPI);
extern void PutVarsToPI(struct Neptun_record *nep_rec, PROCESSINFO *ThisPI);
BOOL PerformActions(struct Neptun_record *nep_rec, PROCESSINFO *ThisPI);

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
struct Neptun_record nep_rec;
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
		UA_DoubleGadgetDimensions(NEP_GR);
		UA_DoubleGadgetDimensions(NEP_Page1_GR);
		UA_DoubleGadgetDimensions(NEP_Page2_GR);
		UA_DoubleGadgetDimensions(NEP_Page3_GR);
	}

	UA_TranslateGR(NEP_GR, msgs);
	UA_TranslateGR(NEP_Page1_GR, msgs);
	UA_TranslateGR(NEP_Page2_GR, msgs);
	UA_TranslateGR(NEP_Page3_GR, msgs);

	/**** open the window ****/

	capsport = (struct MsgPort *)FindPort(MEDIALINKPORT);
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	UAI.windowX				= -1;
	UAI.windowY				= -1;
	UAI.windowWidth		= NEP_GR[0].x2;
	UAI.windowHeight	= NEP_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(&UAI);

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, NEP_GR);

	/**** monitor events ****/

	MonitorUser(UAI.userWindow, ThisPI, &nep_rec, &UAI);

	/**** close the window ****/

	UA_CloseWindow(&UAI);

	capsport->mp_SigTask = oldTask;
}

/******** MonitorUser() ********/

BOOL MonitorUser(	struct Window *window, PROCESSINFO *ThisPI,
									struct Neptun_record *nep_rec, struct UserApplicInfo *UAI)
{
BOOL loop=TRUE, retVal;
struct EventData CED;
int ID,i;
UWORD *mypattern1;
struct Window *panel;
TEXT str[256];
BOOL norexx=FALSE;

	mypattern1 = (UWORD *)AllocMem(4L, MEMF_CHIP);
	if (mypattern1==NULL)
		return(FALSE);
	mypattern1[0] = 0x5555;
	mypattern1[1] = 0xaaaa;

	/**** parse string ****/

	Forbid();
	if ( !FindPort("Neptun") )
	{
		Permit();
		sprintf(str,msgs[Msg_NoRexxHost-1],"Neptun");
		GiveMessage(window,str);
		UA_DisableButton(window, &NEP_GR[4], mypattern1);	// preview
		norexx=TRUE;
	}
	Permit();

	if ( !norexx )
		GetNepVersion(nep_rec);

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
		GetVarsFromPI(nep_rec, ThisPI);
	else
	{
		nep_rec->page = 0;

		// page 1
		nep_rec->computer1				= FALSE;
		nep_rec->video1						= FALSE;
		nep_rec->overlay1					= FALSE;

		// page 2	
		nep_rec->video2						= FALSE;
		nep_rec->fadein_fadeout1	= 0;

		nep_rec->computer2				= FALSE;
		nep_rec->fadein_fadeout2	= 0;

		// page 3
		nep_rec->video3						= FALSE;
		strcpy(nep_rec->duration1, "02.00");
		nep_rec->from1						= FALSE;
		nep_rec->to1							= FALSE;
		nep_rec->pct1t						= 100;

		nep_rec->computer3				= FALSE;
		strcpy(nep_rec->duration2, "02.00");
		nep_rec->from2						= FALSE;
		nep_rec->to2							= FALSE;
		nep_rec->pct2t						= 100;

		if ( !norexx )
		{
			for(i=1; i<=5; i++)
				GetNepValues(nep_rec,i);
		}
	}

	DrawNepButtons(window,nep_rec);

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window, &CED);
		if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, NEP_GR, &CED);
			switch(ID)
			{
				case 3:		// OK
do_ok:
					UA_HiliteButton(window, &NEP_GR[3]);
					retVal=TRUE;
					loop=FALSE;
					break;

				case 4:		// Preview
					UA_InvertButton(window, &NEP_GR[ID]);
					panel = UA_OpenMessagePanel(window,msgs[Msg_Nep_Click-1]);
					GenlockOn(window->WScreen);
					PerformActions(nep_rec,ThisPI);
					if (panel)
						WaitForUser(panel);
					GenlockOff(window->WScreen);
					if (panel)
						UA_CloseMessagePanel(panel);
					UA_InvertButton(window, &NEP_GR[ID]);
					break;

				case 5:		// Cancel
do_cancel:
					UA_HiliteButton(window, &NEP_GR[5]);
					retVal=FALSE;
					loop=FALSE;
					break;

				case 9:		// page
					UA_ProcessCycleGadget(window, &NEP_GR[9], &CED);
					UA_SetValToCycleGadgetVal(&NEP_GR[9],&nep_rec->page);
					UA_ClearButton(window, &NEP_GR[2], AREA_PEN);
					DrawNepButtons(window,nep_rec);
					break;
			}
			if ( ID==-1 && loop && !norexx )
				CheckOtherButtons(window, nep_rec, &CED);
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
		PutVarsToPI(nep_rec, ThisPI);

	return(retVal);
}

/******** DrawNepButtons() ********/

void DrawNepButtons(struct Window *window, struct Neptun_record *nep_rec)
{
struct GadgetRecord *GR;

	UA_SetCycleGadgetToVal(window,&NEP_GR[9],nep_rec->page);

	if ( nep_rec->page==0 )
	{
		GR = NEP_Page1_GR;
		UA_DrawGadgetList(window,GR);

		UA_ClearButton(window, &GR[1], AREA_PEN);
		if ( nep_rec->video1 )
			UA_InvertButton(window, &GR[1]);
		UA_DrawGadget(window, &GR[2]);
		if ( nep_rec->normal_invert==0 )
			UA_InvertButton(window, &GR[2]);
		UA_DrawGadget(window, &GR[3]);
		if ( nep_rec->normal_invert==1 )
			UA_InvertButton(window, &GR[3]);

		UA_ClearButton(window, &GR[5], AREA_PEN);
		if ( nep_rec->computer1 )
			UA_InvertButton(window, &GR[5]);
		UA_DrawGadget(window, &GR[6]);
		if ( nep_rec->genlock_amiga==0 )
			UA_InvertButton(window, &GR[6]);
		UA_DrawGadget(window, &GR[7]);
		if ( nep_rec->genlock_amiga==1 )
			UA_InvertButton(window, &GR[7]);

		UA_ClearButton(window, &GR[9], AREA_PEN);
		if ( nep_rec->overlay1 )
			UA_InvertButton(window, &GR[9]);
		UA_DrawGadget(window, &GR[10]);
		if ( nep_rec->normal_alpha==0 )
			UA_InvertButton(window, &GR[10]);
		UA_DrawGadget(window, &GR[11]);
		if ( nep_rec->normal_alpha==1 )
			UA_InvertButton(window, &GR[11]);
	}
	else if ( nep_rec->page==1 )
	{
		GR = NEP_Page2_GR;
		UA_DrawGadgetList(window,GR);

		UA_ClearButton(window, &GR[1], AREA_PEN);
		if ( nep_rec->video2 )
			UA_InvertButton(window, &GR[1]);
		UA_DrawGadget(window, &GR[2]);
		if ( nep_rec->fadein_fadeout1==0 )
			UA_InvertButton(window, &GR[2]);
		UA_DrawGadget(window, &GR[3]);
		if ( nep_rec->fadein_fadeout1==1 )
			UA_InvertButton(window, &GR[3]);

		UA_ClearButton(window, &GR[5], AREA_PEN);
		if ( nep_rec->computer2 )
			UA_InvertButton(window, &GR[5]);
		UA_DrawGadget(window, &GR[6]);
		if ( nep_rec->fadein_fadeout2==0 )
			UA_InvertButton(window, &GR[6]);
		UA_DrawGadget(window, &GR[7]);
		if ( nep_rec->fadein_fadeout2==1 )
			UA_InvertButton(window, &GR[7]);
	}
	else if ( nep_rec->page==2 )
	{
		GR = NEP_Page3_GR;
		UA_DrawGadgetList(window,GR);
		UA_DrawSpecialGadgetText(window, &GR[ 2], msgs[Msg_X_I_6-1], SPECIAL_TEXT_AFTER_STRING);
		UA_DrawSpecialGadgetText(window, &GR[11], msgs[Msg_X_I_6-1], SPECIAL_TEXT_AFTER_STRING);
		UA_DrawSpecialGadgetText(window, &GR[ 4], "%", SPECIAL_TEXT_AFTER_STRING);
		UA_DrawSpecialGadgetText(window, &GR[ 7], "%", SPECIAL_TEXT_AFTER_STRING);
		UA_DrawSpecialGadgetText(window, &GR[13], "%", SPECIAL_TEXT_AFTER_STRING);
		UA_DrawSpecialGadgetText(window, &GR[16], "%", SPECIAL_TEXT_AFTER_STRING);

		UA_ClearButton(window, &GR[ 1], AREA_PEN);
		UA_ClearButton(window, &GR[ 3], AREA_PEN);
		UA_ClearButton(window, &GR[ 6], AREA_PEN);

		UA_ClearButton(window, &GR[10], AREA_PEN);
		UA_ClearButton(window, &GR[12], AREA_PEN);
		UA_ClearButton(window, &GR[15], AREA_PEN);

		// COMPUTER

		if ( nep_rec->video3 )
			UA_InvertButton(window, &GR[1]);
		UA_SetStringGadgetToString(window, &GR[2], nep_rec->duration1);
		if ( nep_rec->from1 )
			UA_InvertButton(window, &GR[3]);
		UA_SetStringGadgetToVal(window, &GR[4], nep_rec->pct1f);
		if ( nep_rec->to1 )
			UA_InvertButton(window, &GR[6]);
		UA_SetStringGadgetToVal(window, &GR[7], nep_rec->pct1t);

		// VIDEO

		if ( nep_rec->computer3 )
			UA_InvertButton(window, &GR[10]);
		UA_SetStringGadgetToString(window, &GR[11], nep_rec->duration2);
		if ( nep_rec->from2 )
			UA_InvertButton(window, &GR[12]);
		UA_SetStringGadgetToVal(window, &GR[13], nep_rec->pct2f);
		if ( nep_rec->to2 )
			UA_InvertButton(window, &GR[15]);
		UA_SetStringGadgetToVal(window, &GR[16], nep_rec->pct2t);
	}
}

/******** CheckOtherButtons() ********/

void CheckOtherButtons(	struct Window *window, struct Neptun_record *nep_rec,
												struct EventData *CED )
{
int ID,val;
TEXT str[10];

	if ( nep_rec->page==0 )
	{
		ID = UA_CheckGadgetList(window, NEP_Page1_GR, CED);
		switch(ID)
		{
			case  1: 	nep_rec->video1 = ( nep_rec->video1 ? 0 : 1); break;
			case  2:	if ( nep_rec->normal_invert )
									nep_rec->normal_invert = ( nep_rec->normal_invert ? 0 : 1);
								if ( nep_rec->video1==0 )
									nep_rec->video1=1;
								break;
			case  3:	if ( !nep_rec->normal_invert )
									nep_rec->normal_invert = ( nep_rec->normal_invert ? 0 : 1);
								if ( nep_rec->video1==0 )
									nep_rec->video1=1;
								break;

			case  5:	nep_rec->computer1 = ( nep_rec->computer1 ? 0 : 1); break;
			case  6:	if ( nep_rec->genlock_amiga )
									nep_rec->genlock_amiga = ( nep_rec->genlock_amiga ? 0 : 1);
								if ( nep_rec->computer1==0 )
									nep_rec->computer1=1;
								break;
			case  7:	if ( !nep_rec->genlock_amiga )
									nep_rec->genlock_amiga = ( nep_rec->genlock_amiga ? 0 : 1);
								if ( nep_rec->computer1==0 )
									nep_rec->computer1=1;
								break;

			case  9: 	nep_rec->overlay1 = ( nep_rec->overlay1 ? 0 : 1); break;
			case 10:	if ( nep_rec->normal_alpha )
									nep_rec->normal_alpha = ( nep_rec->normal_alpha ? 0 : 1);
								if ( nep_rec->overlay1==0 )
									nep_rec->overlay1=1;
								break;
			case 11:	if ( !nep_rec->normal_alpha )
									nep_rec->normal_alpha = ( nep_rec->normal_alpha ? 0 : 1);
								if ( nep_rec->overlay1==0 )
									nep_rec->overlay1=1;
								break;
		}
	}
	else if ( nep_rec->page==1 )
	{
		ID = UA_CheckGadgetList(window, NEP_Page2_GR, CED);
		switch(ID)
		{
			case  1: 	nep_rec->video2 = ( nep_rec->video2 ? 0 : 1); break;
			case  2:	if ( nep_rec->fadein_fadeout1 )
									nep_rec->fadein_fadeout1 = ( nep_rec->fadein_fadeout1 ? 0 : 1);
								if ( nep_rec->video2==0 )
									nep_rec->video2=1;
								break;
			case  3:	if ( !nep_rec->fadein_fadeout1 )
									nep_rec->fadein_fadeout1 = ( nep_rec->fadein_fadeout1 ? 0 : 1);
								if ( nep_rec->video2==0 )
									nep_rec->video2=1;
								break;

			case  5: 	nep_rec->computer2 = ( nep_rec->computer2 ? 0 : 1); break;
			case  6:	if ( nep_rec->fadein_fadeout2 )
									nep_rec->fadein_fadeout2 = ( nep_rec->fadein_fadeout2 ? 0 : 1);
								if ( nep_rec->computer2==0 )
									nep_rec->computer2=1;
								break;
			case  7:	if ( !nep_rec->fadein_fadeout2 )
									nep_rec->fadein_fadeout2 = ( nep_rec->fadein_fadeout2 ? 0 : 1);
								if ( nep_rec->computer2==0 )
									nep_rec->computer2=1;
								break;
		}
	}
	else if ( nep_rec->page==2 )
	{
		ID = UA_CheckGadgetList(window, NEP_Page3_GR, CED);
		switch(ID)
		{
			case  1: 	nep_rec->video3 = ( nep_rec->video3 ? 0 : 1);
								if ( nep_rec->video3 )
								{
									if ( nep_rec->from1==0 )
										nep_rec->from1=1;
									if ( nep_rec->to1==0 )
										nep_rec->to1=1;
								}
								break;
			case 10: 	nep_rec->computer3 = ( nep_rec->computer3 ? 0 : 1);
								if ( nep_rec->computer3 )
								{
									if ( nep_rec->from2==0 )
										nep_rec->from2=1;
									if ( nep_rec->to2==0 )
										nep_rec->to2=1;
								}
								break;

			case  2:
			case 11:	UA_ProcessStringGadget(window,NEP_Page3_GR,&NEP_Page3_GR[ID],CED);
								UA_SetStringToGadgetString(&NEP_Page3_GR[ID],str);
								if ( !CheckDuration(str) )
									GiveMessage(window, msgs[Msg_Nep_Dur-1], "00.02", "20.00"); // "Enter a value between...
								else if (ID==2)
									strcpy(nep_rec->duration1,str);
								else if (ID==11)
									strcpy(nep_rec->duration2,str);
								if ( ID==2 )
								{
									if ( nep_rec->video3==0 )
										nep_rec->video3=1;
								}
								if ( ID==11 )
								{
									if ( nep_rec->computer3==0 )
										nep_rec->computer3=1;
								}
								break;

			case  3: 	nep_rec->from1 = ( nep_rec->from1 ? 0 : 1);
								if ( nep_rec->video3==0 )
									nep_rec->video3=1;
								break;
			case  6: 	nep_rec->to1 = ( nep_rec->to1 ? 0 : 1);
								if ( nep_rec->video3==0 )
									nep_rec->video3=1;
								break;

			case 12: 	nep_rec->from2 = ( nep_rec->from2 ? 0 : 1);
								if ( nep_rec->computer3==0 )
									nep_rec->computer3=1;
								break;
			case 15: 	nep_rec->to2 = ( nep_rec->to2 ? 0 : 1);
								if ( nep_rec->computer3==0 )
									nep_rec->computer3=1;
								break;

			case  4:
			case  7:
			case 13:
			case 16:	UA_ProcessStringGadget(window,NEP_Page3_GR,&NEP_Page3_GR[ID],CED);
								UA_SetValToStringGadgetVal(&NEP_Page3_GR[ID],&val);
								if ( val<0 || val>100 )
									GiveMessage(window, msgs[Msg_X_4-1], 0, 100); // "Enter a value between...
								else if ( ID==4 )
									nep_rec->pct1f = val;									
								else if ( ID==7 )
									nep_rec->pct1t = val;									
								else if ( ID==13 )
									nep_rec->pct2f = val;									
								else if ( ID==16 )
									nep_rec->pct2t = val;									
								if (ID==4 || ID==7)
								{
									if ( nep_rec->video3==0 )
										nep_rec->video3=1;
									if ( ID==4 && nep_rec->from1==0 )
										nep_rec->from1=1;
									if ( ID==7 && nep_rec->to1==0 )
										nep_rec->to1=1;
								}
								if (ID==13 || ID==16)
								{
									if ( nep_rec->computer3==0 )
										nep_rec->computer3=1;
									if ( ID==13 && nep_rec->from2==0 )
										nep_rec->from2=1;
									if ( ID==16 && nep_rec->to2==0 )
										nep_rec->to2=1;
								}
								break;

			case  5:	UA_HiliteButton(window, &NEP_Page3_GR[ID]);
								GetNepValues(nep_rec,4);
								if ( nep_rec->video3==0 )
									nep_rec->video3=1;
								if ( nep_rec->from1==0 )
									nep_rec->from1=1;
								break;
			case  8:	UA_HiliteButton(window, &NEP_Page3_GR[ID]);
								GetNepValues(nep_rec,6);
								if ( nep_rec->video3==0 )
									nep_rec->video3=1;
								if ( nep_rec->to1==0 )
									nep_rec->to1=1;
								break;

			case 14:	UA_HiliteButton(window, &NEP_Page3_GR[ID]);
								GetNepValues(nep_rec,5);
								if ( nep_rec->computer3==0 )
									nep_rec->computer3=1;
								if ( nep_rec->from2==0 )
									nep_rec->from2=1;
								break;
			case 17:	UA_HiliteButton(window, &NEP_Page3_GR[ID]);
								GetNepValues(nep_rec,7);
								if ( nep_rec->computer3==0 )
									nep_rec->computer3=1;
								if ( nep_rec->to2==0 )
									nep_rec->to2=1;
								break;
		}
	}

	if ( ID != -1 )
		DrawNepButtons(window,nep_rec);
}

/******** GenlockOn() ********/

void GenlockOn(struct Screen *screen)
{
struct TagItem ti[4];

	ti[0].ti_Tag	= VTAG_BORDERNOTRANS_CLR;
	ti[0].ti_Data	= NULL;
	ti[1].ti_Tag	= VTAG_CHROMAKEY_SET;
	ti[1].ti_Data	= NULL;
	ti[2].ti_Tag	= VTAG_CHROMA_PEN_SET;
	ti[2].ti_Data = 0;
	ti[3].ti_Tag	= VTAG_END_CM;
	ti[3].ti_Data = NULL;
	if ( VideoControl(screen->ViewPort.ColorMap, ti)==NULL )
	{
		MakeScreen(screen);
		RethinkDisplay();
	}
}

/******** GenlockOff() ********/

void GenlockOff(struct Screen *screen)
{
struct TagItem ti[4];

	ti[0].ti_Tag	= VTAG_BORDERNOTRANS_SET;
	ti[0].ti_Data	= NULL;
	ti[1].ti_Tag	= VTAG_CHROMAKEY_SET;
	ti[1].ti_Data	= NULL;
	ti[2].ti_Tag	= VTAG_CHROMA_PEN_CLR;
	ti[2].ti_Data = 0;
	ti[3].ti_Tag	= VTAG_END_CM;
	ti[3].ti_Data = NULL;
	if ( VideoControl(screen->ViewPort.ColorMap, ti)==NULL )
	{
		MakeScreen(screen);
		RethinkDisplay();
	}
}

/******** CheckDuration() ********/

BOOL CheckDuration(char *str)
{
ULONG l;

	if ( strlen(str)!=5 )
		return(FALSE);

	if (	isdigit(str[0]) && isdigit(str[1]) &&
				str[2]=='.' &&
				isdigit(str[3]) && isdigit(str[4]) )
	{
		str[0]-=0x30;
		str[1]-=0x30;
		str[3]-=0x30;
		str[4]-=0x30;

		l = str[0]*1000;
		l += (str[1]*100);
		l += (str[3]*10);
		l += (str[4]);
		l *= 50;
		l = l / 100;

		str[0]+=0x30;
		str[1]+=0x30;
		str[3]+=0x30;
		str[4]+=0x30;

		if ( l<1 || l>1000 )
			return(FALSE);
		else
			return(TRUE);
	}

	return(FALSE);
}

/******** WaitForUser() ********/

void WaitForUser(struct Window *window)
{
struct IntuiMessage *message;
struct EventData CED;

	while(message = (struct IntuiMessage *)GetMsg(capsport))
		ReplyMsg((struct Message *)message);

	CED.Class=0L;
	CED.Code=0;
	while(1)
	{
		UA_doStandardWait(window,&CED);
		if (	(CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN) ||
					(CED.Class==MOUSEBUTTONS && CED.Code==MENUDOWN) ||
					(CED.Class==IDCMP_RAWKEY && CED.Code!=0) )
			break;
	}

	while(message = (struct IntuiMessage *)GetMsg(capsport))
		ReplyMsg((struct Message *)message);
}

/******** E O F ********/
