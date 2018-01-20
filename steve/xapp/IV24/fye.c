#include "nb:pre.h"
#include "fye.h"
#include "fyedefs.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "gen:support_protos.h"
#include "protos.h"
#include <fye/fye.h>
#include <fye/fyebase.h>
#include <clib/fye_protos.h>
#include <pragmas/fye.h>

#define VERSI0N "\0$VER: MediaPoint IV-24 xapp 1.0"          
static UBYTE *vers = VERSI0N;

/**** defines ****/

#define STP (char *)	// scalar to pointer

/**** globals ****/

struct Library *medialinkLibBase		= NULL;
//struct Library *DiskfontBase				= NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase							= NULL;
UBYTE **msgs;

/**** functions ****/

/******** XappSetup() ********/

void XappSetup(PROCESSINFO *ThisPI)
{
struct UserApplicInfo UAI;
struct MsgPort *port;
struct Node *node;
struct List *list;
struct RendezVousRecord *rvrec;

	/**** find the mother ship ****/

	port = (struct MsgPort *)FindPort(ML_RENDEZ_VOUS);
	if (port == NULL)
		return;

//KPrintF("IV 1\n");

	/**** link with it ****/

	list = &(port->mp_MsgList);
	node = list->lh_Head;
	rvrec = (struct RendezVousRecord *)node->ln_Name;

	/**** drain it ****/

	msgs = (UBYTE **)rvrec->msgs;

	/**** open standard libraries ****/

/*	if ( !OpenAllLibs() )
		return; */

//KPrintF("IV 2\n");

	IntuitionBase 		= (struct IntuitionBase *)rvrec->intuition;
	GfxBase 					= (struct GfxBase *)rvrec->graphics;
	medialinkLibBase	= (struct Library *)rvrec->medialink;

	/**** initialize the User Application Info structure ***/

	UA_InitStruct(&UAI);
	UAI.IB = IntuitionBase;

//KPrintF("IV 3\n");

	/**** open and load the medialink font ***/

	//OpenUserApplicationFonts(&UAI, "fonts:mediapoint.font", 10, 20);

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

//KPrintF("IV 4\n");

	if (UA_HostScreenPresent(&UAI))
		UAI.windowModes = 1;	/* open on the MediaLink screen */
	else
		UAI.windowModes = 3;	/* open on the first (frontmost) screen */

	/**** double the dimensions of gadgets etc. if screen is laced ****/

	if (UA_IsUAScreenLaced(&UAI))
	{
		UA_DoubleGadgetDimensions(FYE_GR);
		UA_DoubleGadgetDimensions(A1_GR);
		UA_DoubleGadgetDimensions(A2_GR);
		UA_DoubleGadgetDimensions(A3_GR);
		UA_DoubleGadgetDimensions(A4_GR);
		UA_DoubleGadgetDimensions(A5_GR);
	}

	TranslateGR(FYE_GR, msgs);
	TranslateGR(A1_GR, msgs);
	TranslateGR(A2_GR, msgs);
	TranslateGR(A3_GR, msgs);
	TranslateGR(A4_GR, msgs);
	TranslateGR(A5_GR, msgs);

//KPrintF("IV 5\n");

	UAI.windowX			 = -1;	/* -1 means center on screen */
	UAI.windowY			 = -1;	/* -1 means center on screen */
	UAI.windowWidth	 = FYE_GR[0].x2;
	UAI.windowHeight = FYE_GR[0].y2;
	UA_OpenWindow(&UAI);

	/**** render all gadgets ****/

//KPrintF("IV 6\n");

	UA_DrawGadgetList(UAI.userWindow, FYE_GR);

	/**** monitor events ****/

	MonitorUser(UAI.userWindow, ThisPI);

	/**** close the window ****/

	UA_CloseWindow(&UAI);

	/**** close the medialink font ****/

	//CloseUserApplicationFonts(&UAI);	/* function not in lib */

	/**** close all the libraries ****/

	//CloseAllLibs();
}

/******** MonitorUser() ********/

BOOL MonitorUser(struct Window *window, PROCESSINFO *ThisPI)
{
ULONG signals, signalMask;
struct IntuiMessage *message;
BOOL loop=TRUE, retVal;
struct EventData CED;
int ID;
struct IV24_actions IV24;
UWORD *mypattern1, *waitPtr;
UBYTE *ed;
ULONG edSize;

	/**** make a copy of extra data ****/

	edSize = 	*ThisPI->pi_Arguments.ar_Worker.aw_ExtraDataSize;
	ed = (UBYTE *)AllocMem(edSize,MEMF_ANY|MEMF_CLEAR);
	if (ed==NULL)
		return(FALSE);
	CopyMem(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData, ed, edSize);

	/**** alloc chip mem ****/

	mypattern1 = (UWORD *)AllocMem(4L, MEMF_CHIP);
	if (mypattern1==NULL)
		return(FALSE);
	mypattern1[0] = 0x5555;
	mypattern1[1] = 0xaaaa;

	waitPtr = (UWORD *)AllocMem(sizeof(WaitPointer), MEMF_CHIP);
	if (waitPtr==NULL)
		return(FALSE);
	CopyMem(WaitPointer, waitPtr, sizeof(WaitPointer));

	if ( !CheckBoard(window) )
		UA_DisableButton(window, &FYE_GR[4], mypattern1);	// preview

	/**** init struct ****/

	IV24.action						= ACTION_PIP;

	IV24.pip_size					= 3;									// 0...3
	IV24.pip_action				= PIP_ACTION_PLACE;
	IV24.pip_display_mode	= 1;									// 0...5 (1=fs amiga, video in pip)
	IV24.pip_close				= FALSE;
	IV24.pip_x						= 40;
	IV24.pip_y						= 40;
	IV24.pip_speed				= 2;									// gets multiplied by 10

	IV24.fsv_display_mode	= 0;									// 0...3 (0=fs video)

	IV24.fileName[0]			= '\0';
	IV24.delay						= 5;									// 5 seconds

	IV24.comp_output[0] 	= MODE_EXTERN;
	IV24.comp_output[1] 	= SET_INTERN;
	IV24.comp_output[2] 	= SET_POSITIVE;
	IV24.rgb_output[0] 		= MODE_AMIGA;
	IV24.rgb_output[1] 		= SET_INTERN;
	IV24.rgb_output[2] 		= SET_POSITIVE;
	IV24.misc_output[0]		= SCAN_HISCAN;
	IV24.misc_output[1]		= COLORS_16M;
	IV24.misc_output[2]		= 0;									// no bypass

	IV24.comp_fade				= COMPFADE_AMIGA;			// fade from no amiga to full over video (genlocked)
	IV24.comp_from				= 0;
	IV24.comp_to					= 255;

	/**** parse string ****/

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
		GetExtraData(ThisPI,&IV24);

	/**** set gadgets ****/

	RenderPage(window, &IV24);

	/**** event handler ****/

	signalMask = (1L << window->UserPort->mp_SigBit);

	while(loop)
	{
		signals = Wait(signalMask);
		if (signals & signalMask)
		{
			while( message = (struct IntuiMessage *)GetMsg(window->UserPort) )
			{
				CED.Class			= message->Class;
				CED.Code			= message->Code;
				CED.MouseX 		= message->MouseX;
				CED.MouseY 		= message->MouseY;
				ReplyMsg((struct Message *)message);
				//RestoreQual(&CED);

				if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code!=MENUDOWN && CED.Code==SELECTDOWN)
					SimulateDragBar(window,&CED);

				if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
				{
					if ( IV24.action==ACTION_PIP )
					{
						ID = UA_CheckGadgetList(window, A1_GR, &CED);
						ProcessPage1(window, ID, &IV24, &CED);
					}
					else if ( IV24.action==ACTION_FSV )
					{
						ID = UA_CheckGadgetList(window, A2_GR, &CED);
						ProcessPage2(window, ID, &IV24, &CED);
					}
					else if ( IV24.action==ACTION_FB )
					{
						ID = UA_CheckGadgetList(window, A3_GR, &CED);
						ProcessPage3(window, ID, &IV24, &CED, mypattern1, waitPtr);
					}
					else if ( IV24.action==ACTION_CP )
					{
						ID = UA_CheckGadgetList(window, A4_GR, &CED);
						ProcessPage4(window, ID, &IV24, &CED);
					}
					else if ( IV24.action==ACTION_CK )
					{
						ID = UA_CheckGadgetList(window, A5_GR, &CED);
						ProcessPage5(window, ID, &IV24, &CED);
					}
					ID = UA_CheckGadgetList(window, FYE_GR, &CED);
					switch(ID)
					{
						case 4:		// Preview
							UA_InvertButton(window, &FYE_GR[ID]);
							SetSprite(window, SPRITE_BUSY, waitPtr);
							PutExtraData(ThisPI,&IV24);
							if ( !XappDoIt(ThisPI,&IV24) )
								GiveMessage(window, msgs[Msg_IV_MISC_1]); // "The IV-24 doesn't respond."
							SetSprite(window, SPRITE_NORMAL, waitPtr);
							UA_InvertButton(window, &FYE_GR[ID]);
							break;

						case 5:		// OK
do_ok:
							UA_HiliteButton(window, &FYE_GR[5]);
							loop=FALSE;
							retVal=TRUE; 
							break;

						case 6:		// Cancel
do_cancel:
							UA_HiliteButton(window, &FYE_GR[6]);
							loop=FALSE;
							retVal=FALSE; 
							break;

						case 10:	// action
							UA_ProcessCycleGadget(window, &FYE_GR[ID], &CED);
							UA_SetValToCycleGadgetVal(&FYE_GR[ID], &IV24.action);
							IV24.action+=1;
							RenderPage(window, &IV24);
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
		}
	}

	if ( retVal )
		PutExtraData(ThisPI,&IV24);
	else
		CopyMem(ed, ThisPI->pi_Arguments.ar_Worker.aw_ExtraData, edSize);

	FreeMem(ed,edSize);

	FreeMem(waitPtr, sizeof(WaitPointer));
	FreeMem(mypattern1, 4L);

	return(retVal);
}

#if 0
/******** RestoreQual() ********/

void RestoreQual(struct EventData *CED)
{
	if ( CED->Class == IDCMP_RAWKEY )
	{
		if ( CED->Code >= 0x60 )
		{
			switch(CED->Code)
			{
				case 0x66:
					CED->Qualifier = IEQUALIFIER_LCOMMAND;
					break;
				case 0x60:
					CED->Qualifier = IEQUALIFIER_LSHIFT;
					break;
				case 0x63:
					CED->Qualifier = IEQUALIFIER_CONTROL;
					break;
				case 0x64:
					CED->Qualifier = IEQUALIFIER_LALT;
					break;
				case 0xe0:
				case 0xe3:
				case 0xe4:
				case 0xe6:
					CED->Qualifier = 0;
				default:
					CED->Qualifier = 0;
			}
		}
		else
			CED->Qualifier = 0;
	}
}
#endif

/******** RenderPage() ********/

void RenderPage(struct Window *window, struct IV24_actions *IV24)
{
TEXT temp[SIZE_FULLPATH];

	/**** clean window ****/

	UA_ClearButton(window, &FYE_GR[8], AREA_PEN);
	UA_ClearButton(window, &FYE_GR[9], AREA_PEN);

	if ( IV24->action==ACTION_PIP )
		UA_DrawGadgetList(window,A1_GR);
	else if ( IV24->action==ACTION_FSV )
		UA_DrawGadgetList(window,A2_GR);
	else if ( IV24->action==ACTION_FB )
	{
		A3_GR[0].type = HIBOX_REGION;
		UA_DrawGadgetList(window,A3_GR);
		A3_GR[0].type = BUTTON_GADGET;
	}
	else if ( IV24->action==ACTION_CP )
		UA_DrawGadgetList(window,A4_GR);
	else if ( IV24->action==ACTION_CK )
		UA_DrawGadgetList(window,A5_GR);

	UA_SetCycleGadgetToVal(window, &FYE_GR[10], IV24->action-1);

	if ( IV24->action==ACTION_PIP )
	{
		UA_SetCycleGadgetToVal(window, &A1_GR[0], IV24->pip_size);
		UA_SetCycleGadgetToVal(window, &A1_GR[1], IV24->pip_action);
		UA_SetCycleGadgetToVal(window, &A1_GR[2], IV24->pip_display_mode);
		UA_SetCycleGadgetToVal(window, &A1_GR[3], IV24->pip_speed);
		if ( IV24->pip_close )
			UA_InvertButton(window, &A1_GR[4]);
		UA_SetStringGadgetToVal(window, &A1_GR[5], IV24->pip_x);
		UA_SetStringGadgetToVal(window, &A1_GR[6], IV24->pip_y);
		DisplayModeInfo(window,IV24);
	}
	else if ( IV24->action==ACTION_FSV )
	{
		UA_SetCycleGadgetToVal(window, &A2_GR[0], IV24->fsv_display_mode);
		DisplayModeInfo(window,IV24);
	}
	else if ( IV24->action==ACTION_FB )
	{
		if ( IV24->fileName[0] != '\0' )
		{
			strcpy(temp, IV24->fileName);
			UA_ShortenString(window->RPort, temp, (A3_GR[0].x2-A3_GR[0].x1)-16);
			UA_DrawText(window, &A3_GR[0], temp);
		}
		UA_SetStringGadgetToVal(window, &A3_GR[1], IV24->delay);
	}
	else if ( IV24->action==ACTION_CP )
	{
		UA_SetCycleGadgetToVal(window, &A4_GR[0], IV24->comp_output[0]-1);
		UA_SetCycleGadgetToVal(window, &A4_GR[1], IV24->comp_output[1]-1);
		UA_SetCycleGadgetToVal(window, &A4_GR[2], IV24->comp_output[2]-1);

		UA_SetCycleGadgetToVal(window, &A4_GR[3], IV24->rgb_output[0]-1);
		UA_SetCycleGadgetToVal(window, &A4_GR[4], IV24->rgb_output[1]-1);
		UA_SetCycleGadgetToVal(window, &A4_GR[5], IV24->rgb_output[2]-1);

		UA_SetCycleGadgetToVal(window, &A4_GR[6], IV24->misc_output[0]-1);
		UA_SetCycleGadgetToVal(window, &A4_GR[7], IV24->misc_output[1]-1);

		if ( IV24->misc_output[2] )
			UA_InvertButton(window, &A4_GR[8]);
	}
	else if ( IV24->action==ACTION_CK )
	{
		UA_SetCycleGadgetToVal(window, &A5_GR[0], IV24->comp_fade-1);
		UA_SetStringGadgetToVal(window, &A5_GR[1], IV24->comp_from);
		UA_SetStringGadgetToVal(window, &A5_GR[2], IV24->comp_to);
	}
}

/******** ProcessPage1() ********/
/*
 * PIP
 *
 */

void ProcessPage1(struct Window *window, int ID, struct IV24_actions *IV24,
									struct EventData *CED)
{
BOOL touched=FALSE;

	switch(ID)
	{
		case 0: // PIP size
		case 1:	// PIP action
		case 2:	// PIP mode
		case 3:	// PIP speed
			UA_ProcessCycleGadget(window, &A1_GR[ID], CED);
			touched=TRUE;
			break;

		case 4:	// close PIP when done
			UA_InvertButton(window, &A1_GR[ID]);
			if ( IV24->pip_close==FALSE )
				IV24->pip_close=TRUE;
			else
				IV24->pip_close=FALSE;
			break;

		case 5:
		case 6:
			UA_ProcessStringGadget(window, A1_GR, &A1_GR[ID], CED);
			touched=TRUE;
			break;
	}

	if (touched)	// sigh...got to drag info from cycles, strings etc.
	{
		/**** get settings ****/

		UA_SetValToCycleGadgetVal(&A1_GR[0], &IV24->pip_size);
		UA_SetValToCycleGadgetVal(&A1_GR[1], &IV24->pip_action);
		UA_SetValToCycleGadgetVal(&A1_GR[2], &IV24->pip_display_mode);
		UA_SetValToCycleGadgetVal(&A1_GR[3], &IV24->pip_speed);

		UA_SetValToStringGadgetVal(&A1_GR[5], &IV24->pip_x);
		UA_SetValToStringGadgetVal(&A1_GR[6], &IV24->pip_y);

		if (IV24->pip_x<30 || IV24->pip_x>500)
		{
			IV24->pip_x = 40;
			UA_SetStringGadgetToVal(window, &A1_GR[5], IV24->pip_x);
			GiveMessage(window, msgs[Msg_X_4], 30, 500); // "Enter a value between 30 and 500."
		}

		if (IV24->pip_y<28 || IV24->pip_y>605)
		{
			IV24->pip_y = 40;
			UA_SetStringGadgetToVal(window, &A1_GR[6], IV24->pip_y);
			GiveMessage(window, msgs[Msg_X_4], 28, 605); // "Enter a value between 28 and 605."
		}

		if (ID==2)
			DisplayModeInfo(window, IV24);
	}
}

/******** ProcessPage2() ********/
/*
 * FSV
 *
 */

void ProcessPage2(struct Window *window, int ID, struct IV24_actions *IV24,
									struct EventData *CED)
{
	switch(ID)
	{
		case 0:
			UA_ProcessCycleGadget(window, &A2_GR[ID], CED);
			UA_SetValToCycleGadgetVal(&A2_GR[ID], &IV24->fsv_display_mode);
			DisplayModeInfo(window, IV24);
			break;
	}
}

/******** ProcessPage3() ********/
/*
 * Framebuffer
 *
 */

void ProcessPage3(struct Window *window, int ID, struct IV24_actions *IV24,
									struct EventData *CED, UWORD *mypattern1, UWORD *waitPtr)
{
TEXT path[SIZE_FULLPATH];
TEXT temp[SIZE_FULLPATH];

	switch(ID)
	{
		case 0:	// file
			UA_HiliteButton(window, &A3_GR[ID]);
			strcpy(path,IV24->fileName);
			if ( PickPicture(window,path,mypattern1,waitPtr) )
			{
				strcpy(IV24->fileName,path);
				strcpy(temp,path);
				UA_ShortenString(window->RPort, temp, (A3_GR[0].x2-A3_GR[0].x1)-16);
				UA_ClearButton(window, &A3_GR[0], AREA_PEN);
				UA_DrawText(window, &A3_GR[0], temp);
			}
			break;

		case 1:	// delay
			UA_ProcessStringGadget(window, A3_GR, &A3_GR[ID], CED);
			UA_SetValToStringGadgetVal(&A3_GR[ID], &IV24->delay);
			break;
	}
}

/******** ProcessPage4() ********/

void ProcessPage4(struct Window *window, int ID, struct IV24_actions *IV24,
									struct EventData *CED)
{
BOOL touched=FALSE;
int value,i;

	switch(ID)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			UA_ProcessCycleGadget(window, &A4_GR[ID], CED);
			touched=TRUE;
			break;

		case 8:	// bypass
			UA_InvertButton(window, &A4_GR[ID]);
			if ( IV24->misc_output[2]==FALSE )
				IV24->misc_output[2] = TRUE;
			else
				IV24->misc_output[2] = FALSE;
			break;
	}

	if (touched)	// sigh...got to drag info from cycles, strings etc.
	{
		for(i=0; i<3; i++)
		{
			UA_SetValToCycleGadgetVal(&A4_GR[i], &value);
			IV24->comp_output[i] = value+1;

			UA_SetValToCycleGadgetVal(&A4_GR[3+i], &value);
			IV24->rgb_output[i] = value+1;
		}

		for(i=0; i<2; i++)
		{
			UA_SetValToCycleGadgetVal(&A4_GR[6+i], &value);
			IV24->misc_output[i] = value+1;
		}
	}
}

/******** ProcessPage5() ********/

void ProcessPage5(struct Window *window, int ID, struct IV24_actions *IV24,
									struct EventData *CED)
{
int value;

	switch(ID)
	{
		case 0:
			UA_ProcessCycleGadget(window, &A5_GR[ID], CED);
			UA_SetValToCycleGadgetVal(&A5_GR[ID], &value);
			IV24->comp_fade = value+1;
			break;

		case 1:
			UA_ProcessStringGadget(window, A5_GR, &A5_GR[ID], CED);
			UA_SetValToStringGadgetVal(&A5_GR[ID], &IV24->comp_from);
			if (IV24->comp_from<0 || IV24->comp_from>255)
			{
				IV24->comp_from = 0;
				UA_SetStringGadgetToVal(window, &A5_GR[ID], IV24->comp_from);
				GiveMessage(window, msgs[Msg_X_4], 0, 255);	// "Enter a value between 0 and 255."
			}
			break;

		case 2:
			UA_ProcessStringGadget(window, A5_GR, &A5_GR[ID], CED);
			UA_SetValToStringGadgetVal(&A5_GR[ID], &IV24->comp_to);
			if (IV24->comp_to<0 || IV24->comp_to>255)
			{
				IV24->comp_to = 255;
				UA_SetStringGadgetToVal(window, &A5_GR[ID], IV24->comp_to);
				GiveMessage(window, msgs[Msg_X_4], 0, 255);	// "Enter a value between 0 and 255."
			}
			break;
	}
}

/******** DisplayModeInfo() ********/

void DisplayModeInfo(struct Window *window, struct IV24_actions *IV24)
{
	UA_ClearButton(window, &FYE_GR[ 9], AREA_PEN);
	UA_ClearButton(window, &FYE_GR[11], AREA_PEN);
	UA_ClearButton(window, &FYE_GR[12], AREA_PEN);

	if ( IV24->action == ACTION_PIP )
	{
		if ( IV24->pip_display_mode==0 )
		{
			UA_DrawSpecialGadgetText(window, &FYE_GR[11], msgs[Msg_IV_PM_1], SPECIAL_TEXT_CENTER);
			UA_DrawSpecialGadgetText(window, &FYE_GR[12], msgs[Msg_IV_PM_2], SPECIAL_TEXT_CENTER);
		}
		else if ( IV24->pip_display_mode==1 )
		{
			UA_DrawSpecialGadgetText(window, &FYE_GR[11], msgs[Msg_IV_PM_3], SPECIAL_TEXT_CENTER);
			UA_DrawSpecialGadgetText(window, &FYE_GR[12], msgs[Msg_IV_PM_4], SPECIAL_TEXT_CENTER);
		}
		else if ( IV24->pip_display_mode==2 )
		{
			UA_DrawSpecialGadgetText(window, &FYE_GR[11], msgs[Msg_IV_PM_5], SPECIAL_TEXT_CENTER);
			UA_DrawSpecialGadgetText(window, &FYE_GR[12], msgs[Msg_IV_PM_6], SPECIAL_TEXT_CENTER);
		}
		else if ( IV24->pip_display_mode==3 )
		{
			UA_DrawSpecialGadgetText(window, &FYE_GR[11], msgs[Msg_IV_PM_7], SPECIAL_TEXT_CENTER);
			UA_DrawSpecialGadgetText(window, &FYE_GR[12], msgs[Msg_IV_PM_8], SPECIAL_TEXT_CENTER);
		}
		else if ( IV24->pip_display_mode==4 )
		{
			UA_DrawSpecialGadgetText(window, &FYE_GR[11], msgs[Msg_IV_PM_9], SPECIAL_TEXT_CENTER);
			UA_DrawSpecialGadgetText(window, &FYE_GR[12], msgs[Msg_IV_PM_10], SPECIAL_TEXT_CENTER);
		}
		else if ( IV24->pip_display_mode==5 )
		{
			UA_DrawSpecialGadgetText(window, &FYE_GR[11], msgs[Msg_IV_PM_11], SPECIAL_TEXT_CENTER);
			UA_DrawSpecialGadgetText(window, &FYE_GR[12], msgs[Msg_IV_PM_12], SPECIAL_TEXT_CENTER);
		}
	}
	else if ( IV24->action == ACTION_FSV )
	{
		if ( IV24->fsv_display_mode==0 )
			UA_DrawSpecialGadgetText(window, &FYE_GR[11], msgs[Msg_IV_PM_3], SPECIAL_TEXT_CENTER);
		else if ( IV24->fsv_display_mode==1 )
			UA_DrawSpecialGadgetText(window, &FYE_GR[11], msgs[Msg_IV_PM_1], SPECIAL_TEXT_CENTER);
		else if ( IV24->fsv_display_mode==2 )
			UA_DrawSpecialGadgetText(window, &FYE_GR[11], msgs[Msg_IV_PM_7], SPECIAL_TEXT_CENTER);
		else if ( IV24->fsv_display_mode==3 )
			UA_DrawSpecialGadgetText(window, &FYE_GR[11], msgs[Msg_IV_PM_5], SPECIAL_TEXT_CENTER);
	}		
}

/******** CheckBoard() ********/

BOOL CheckBoard(struct Window *window)
{
struct FyeBase *FyeBase;
ULONG	error;

	if ((FyeBase=(struct FyeBase *)OpenLibrary("fye.library",0))==NULL)
	{
		GiveMessage(window, msgs[Msg_IV_MISC_2]); // "The fye.library is not available."
		return(FALSE);
	}

	if (error=ObtainFyeBoard())
	{
		GiveMessage(window, msgs[Msg_IV_MISC_3]); // "The IV-24 card is not available."
		CloseLibrary((struct Library *)FyeBase);
		return(FALSE);
	}

	ReleaseFyeBoard();
	CloseLibrary((struct Library *)FyeBase);
	return(TRUE);
}

/******** E O F ********/
