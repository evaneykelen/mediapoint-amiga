#include "nb:pre.h"
#include <devices/cdtv.h>
#include "cdtv.h"
#include "setup2.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"
#include "demo:gen/support_protos.h"

#define VERSI0N "\0$VER: 1.0"          
static UBYTE *vers = VERSI0N;

/**** external function declarations ****/

extern void GetVarsFromPI(struct CDTV_record *CDTV_rec, PROCESSINFO *ThisPI);
extern void PutVarsToPI(struct CDTV_record *CDTV_rec, PROCESSINFO *ThisPI);

/**** globals ****/

struct Library *medialinkLibBase		= NULL;
struct Library *DiskfontBase				= NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase							= NULL;
struct Library *IconBase						= NULL;

UBYTE **msgs;
struct RendezVousRecord *rvrec;
struct MsgPort *capsport;

/**** disable CTRL-C break ****/

void CXBRK(void) { }
void chkabort(void) {  }

/**** functions ****/

/******** XappSetup() ********/

void XappSetup(PROCESSINFO *ThisPI)
{
struct UserApplicInfo UAI;
struct CDTV_record CDTV_rec;
UBYTE *chipMem;
struct MsgPort *port;
struct Node *node;
struct List *list;
struct Task *oldTask;

	/**** find the mother ship ****/

	port = (struct MsgPort *)FindPort(ML_RENDEZ_VOUS);
	if (port == NULL)
		return;

	/**** link with it ****/

	list	= &(port->mp_MsgList);
	node	= list->lh_Head;
	rvrec	= (struct RendezVousRecord *)node->ln_Name;

	/**** drain it ****/

	msgs = (UBYTE **)rvrec->msgs;

	/**** open standard libraries ****/

	IntuitionBase 		= (struct IntuitionBase *)rvrec->intuition;
	GfxBase 					= (struct GfxBase *)rvrec->graphics;
	medialinkLibBase	= (struct Library *)rvrec->medialink;

	/**** parse .info file for special settings ****/

	GetInfoFile("CD32", ThisPI->pi_Arguments.ar_Worker.aw_MLSystem->xappPath,
							CDTV_rec.devName, &(CDTV_rec.portNr), &(CDTV_rec.baudRate));

	/**** open special libraries or devices ****/

	if ( !Open_SerialPort(&CDTV_rec) )
		return;

	/**** initialize the User Application Info structure ***/

	UA_InitStruct(&UAI);
	UAI.IB = IntuitionBase;

	/**** copy image to chip memory ****/

	chipMem = (UBYTE *)AllocMem(sizeof(ImageDataControler), MEMF_CHIP);
	if (chipMem==NULL)
	{
		Close_SerialPort(&CDTV_rec);
		return;
	}
	CopyMem(ImageDataControler, chipMem, sizeof(ImageDataControler));
	ImageControler.ImageData = (SHORT *)chipMem;	

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
		UA_DoubleGadgetDimensions(CDTV_GR);

	UA_TranslateGR(CDTV_GR, msgs);

	UA_AdjustGadgetCoords(Controler_GR, CDTV_GR[25].x1-3, CDTV_GR[25].y1-2);

	/**** open the window ****/

	capsport = (struct MsgPort *)FindPort(MEDIALINKPORT);
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	UAI.windowX				= -1;	/* -1 means center on screen */
	UAI.windowY				= -1;	/* -1 means center on screen */
	UAI.windowWidth		= CDTV_GR[0].x2;
	UAI.windowHeight	= CDTV_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(&UAI);

	CDTV_rec.window = UAI.userWindow;

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, CDTV_GR);
	DrawImage(UAI.userWindow->RPort, (struct Image *)&ImageControler,
						CDTV_GR[25].x1, CDTV_GR[25].y1);

	/**** monitor events ****/

	MonitorUser(UAI.userWindow, ThisPI, &CDTV_rec);

	/**** close the window ****/

	UA_CloseWindow(&UAI);

	/**** free chip mem ****/

	FreeMem(chipMem, sizeof(ImageDataControler));

	/**** close specials libraries or devices ****/

	Close_SerialPort(&CDTV_rec);

	capsport->mp_SigTask = oldTask;
}

/******** MonitorUser() ********/

BOOL MonitorUser(	struct Window *window, PROCESSINFO *ThisPI,
									struct CDTV_record *CDTV_rec)
{
BOOL loop=TRUE, retVal;
struct EventData CED;
int ID, track, numTracks, t2=1;
UWORD *mypattern1;
TEXT buf[64], str1[16], str2[16];
struct CDTV_record local_rec;
struct PopUpRecord PUR;

	mypattern1 = (UWORD *)AllocMem(4L, MEMF_CHIP);
	if (mypattern1==NULL)
		return(FALSE);
	mypattern1[0] = 0x5555;
	mypattern1[1] = 0xaaaa;

	/**** init CDTV_rec ****/

	CDTV_rec->IOReq1 = NULL;
	CDTV_rec->IOReq2 = NULL;
	CDTV_rec->IOPort = NULL;

	/**** parse string ****/

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
		GetVarsFromPI(CDTV_rec, ThisPI);
	else
	{
		CDTV_rec->action			= 0;			// play song
		CDTV_rec->command			= 3;			// fade out fast
		CDTV_rec->song				= 1;
		CDTV_rec->from				= 1;
		CDTV_rec->to					= 2;
		strcpy(CDTV_rec->start, "00:05:00");
		strcpy(CDTV_rec->end,   "00:10:00");
		CDTV_rec->fadeIn			= FALSE;
		CDTV_rec->control			= 0;			// serial
	}

	track = CDTV_rec->song;

	/**** pop-up ****/

	PUR.window = NULL;
	PUR.GR = PopUp_GR;
	PUR.ptr = msgs[Msg_CDTV_8-1];
	PUR.active = CDTV_rec->command;	//CDTV_cmds_CR.active;
	PUR.number = CDTV_cmds_CR.number;
	PUR.width = CDTV_cmds_CR.width;
	PUR.fit = 0;
	PUR.top = 0;

	/**** set buttons ****/

	SetButtons(CDTV_rec, window, mypattern1, &PUR);

	UA_DisableButton(window, &Controler_GR[2], mypattern1);
	UA_DisableButton(window, &Controler_GR[6], mypattern1);
	UA_DisableButton(window, &Controler_GR[7], mypattern1);

	/**** treat radio buttons separately ****/

	UA_InvertButton(window, &CDTV_GR[CDTV_rec->action+4]);

	if ( CDTV_rec->fadeIn )
		UA_InvertButton(window, &CDTV_GR[14]);

	/**** get info from CDTV ****/

	UA_SetSprite(window, SPRITE_BUSY);

	if ( SendStringViaSer(CDTV_rec, "!GI\n") )
	{
		if (GetStringFromSer(CDTV_rec,buf,GI_LENGTH))
		{
			sscanf(buf,"[%02d] [%02d] %s %s",&track,&numTracks,str1,str2);
			if ( numTracks!=0 )
			{
				sprintf(buf, "%d", numTracks);
				UA_DrawSpecialGadgetText(window, &CDTV_GR[2], buf, SPECIAL_TEXT_CENTER);
			}
			if ( CheckTimeCode(str2) )
				UA_DrawSpecialGadgetText(window, &CDTV_GR[3], str2, SPECIAL_TEXT_CENTER);
		}
	}

	UA_SetSprite(window, SPRITE_NORMAL);

	CopyMem(CDTV_rec, &local_rec, sizeof(struct CDTV_record));

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window, &CED);
		if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			/**** CDTV window gadgets ****/

			ID = UA_CheckGadgetList(window, CDTV_GR, &CED);
			switch(ID)
			{
				case 4:	// play song
				case 5:	// play from...to
				case 6:	// play start...end
				case 7:	// direct command
					if ( CDTV_rec->action != ID-4 )
					{
						UA_InvertButton(window, &CDTV_GR[CDTV_rec->action+4]);
						CDTV_rec->action=ID-4;								
						UA_InvertButton(window, &CDTV_GR[CDTV_rec->action+4]);
						SetButtons(CDTV_rec, window, mypattern1, &PUR);
					}
					break;

				case 8:		// song
				case 9:		// from
				case 10:	// to
				case 11:	// start
				case 12:	// end
					UA_ProcessStringGadget(window, CDTV_GR, &CDTV_GR[ID], &CED);
					if (ID==8)
						UA_SetValToStringGadgetVal(&CDTV_GR[ID], &CDTV_rec->song);
					else if (ID==9)
						UA_SetValToStringGadgetVal(&CDTV_GR[ID], &CDTV_rec->from);
					else if (ID==10)
						UA_SetValToStringGadgetVal(&CDTV_GR[ID], &CDTV_rec->to);
					else if (ID==11)
						UA_SetStringToGadgetString(&CDTV_GR[ID], CDTV_rec->start);
					else if (ID==12)
						UA_SetStringToGadgetString(&CDTV_GR[ID], CDTV_rec->end);
					break;

				case 13:	// command
					UA_InvertButton(window, &CDTV_GR[ID]);
					if ( UA_OpenPopUpWindow(window, &CDTV_GR[ID], &PUR) )
					{
						UA_Monitor_PopUp(&PUR);
						UA_ClosePopUpWindow(&PUR);
						CDTV_rec->command = PUR.active;
					}
					UA_InvertButton(window, &CDTV_GR[ID]);
					UA_PrintPopUpChoice(window, &CDTV_GR[ID], &PUR);
					break;

				case 14:
					UA_InvertButton(window, &CDTV_GR[ID]);
					if ( CDTV_rec->fadeIn )
						CDTV_rec->fadeIn=FALSE;
					else
						CDTV_rec->fadeIn=TRUE;
					break;

				case 15:	// new cd
					UA_InvertButton(window, &CDTV_GR[ID]);
					UA_ClearButton(window, &CDTV_GR[2], AREA_PEN);
					UA_ClearButton(window, &CDTV_GR[3], AREA_PEN);
					UA_ClearButton(window, &CDTV_GR[26], AREA_PEN);
					if ( SendSerCmd(CDTV_rec, DO_STOP) && SendStringViaSer(CDTV_rec, "!CD\n") )
					{
						t2=1;
						if ( SendStringViaSer(CDTV_rec, "!GI\n") )
						{
							if (GetStringFromSer(CDTV_rec,buf,GI_LENGTH))
							{	
								sscanf(buf,"[%02d] [%02d] %s %s",&track,&numTracks,str1,str2);
								if ( numTracks!=0 )
								{
									sprintf(buf, "%d", numTracks);
									UA_DrawSpecialGadgetText(window, &CDTV_GR[2], buf, SPECIAL_TEXT_CENTER);
								}
								if ( CheckTimeCode(str2) )
								{
									UA_DrawSpecialGadgetText(window, &CDTV_GR[3], str2, SPECIAL_TEXT_CENTER);
								}
							}
						}
					}
					UA_InvertButton(window, &CDTV_GR[ID]);
					break;

				case 17:	// OK
do_ok:
					UA_HiliteButton(window, &CDTV_GR[17]);
					loop=FALSE;
					retVal=TRUE; 
					break;

				case 18:	// Preview
					UA_InvertButton(window, &CDTV_GR[ID]);
					UA_ClearButton(window, &CDTV_GR[26], AREA_PEN);
					ControlCDTV(CDTV_rec);
					Delay(4L);
					UA_InvertButton(window, &CDTV_GR[ID]);
					break;

				case 19:	// Cancel
do_cancel:
					UA_HiliteButton(window, &CDTV_GR[19]);
					loop=FALSE;
					retVal=FALSE; 
					break;
			}

			/**** Controler gadgets ****/

			ID =  UA_CheckGadgetList(window, Controler_GR, &CED);
			switch(ID)
			{
				case 1:	// Stop
					UA_InvertButton(window, &Controler_GR[ID]);
					SendSerCmd(CDTV_rec, DO_STOP);
					UA_ClearButton(window, &CDTV_GR[26], AREA_PEN);
					Delay(4L);
					UA_InvertButton(window, &Controler_GR[ID]);
					break;

				case 3:	// Play Fwd
				case 4:	// Play prev
				case 5:	// Play next
					UA_InvertButton(window, &Controler_GR[ID]);
					if ( SendStringViaSer(CDTV_rec, "!GI\n") )	// Get Info
					{
						if (GetStringFromSer(CDTV_rec,buf,GI_LENGTH))
						{
							track = t2;	// remember it now...
							sscanf(buf,"[%02d] [%02d] %s %s",&t2,&numTracks,str1,str2);
							if ( numTracks!=0 )
							{
								if ( t2==0 )
									t2 = track;
								if ( ID==4 )			// Play prev
									t2--;
								else if ( ID==5 )	// Play Next
									t2++;	
								if ( t2<1 )
									t2=1;
								if ( t2>numTracks )
									t2=numTracks;
								local_rec.song = t2;
								local_rec.fadeIn = FALSE;
								SendSerCmd(&local_rec, DO_PLAYTRACK);
								// Try to obtain info on track
								sprintf(str1,"!GT %d\n",t2);
								if ( SendStringViaSer(CDTV_rec, str1) )	// Get Track
								{
									if (GetStringFromSer(CDTV_rec,buf,GT_LENGTH))
									{
										sscanf(buf,"%s %s",str1,str2);
										if ( CheckTimeCode(str1) && CheckTimeCode(str2) )
										{
											sprintf(str2,"%d - %s",t2,str1);
											UA_ClearButton(window, &CDTV_GR[26], AREA_PEN);
											UA_DrawSpecialGadgetText(window, &CDTV_GR[26], str2, SPECIAL_TEXT_CENTER);
										}
									}
								}
							}
						}
					}
					Delay(4L);
					UA_InvertButton(window, &Controler_GR[ID]);
					break;

				case 8:	// Pause
					UA_InvertButton(window, &Controler_GR[ID]);
					if ( SendStringViaSer(CDTV_rec, "!GI\n") )	// Get Info
					{
						if (GetStringFromSer(CDTV_rec,buf,GI_LENGTH))
						{
							sscanf(buf,"[%02d] [%02d] %s %s",&track,&numTracks,str1,str2);
							if ( numTracks!=0 )
							{
								SendSerCmd(&local_rec, DO_PAUSE);
								// Print where we are
								if ( CheckTimeCode(str1) && CheckTimeCode(str2) )
								{
									sprintf(str2,"%d - %s",track,str1);
									UA_ClearButton(window, &CDTV_GR[26], AREA_PEN);
									UA_DrawSpecialGadgetText(window, &CDTV_GR[26], str2, SPECIAL_TEXT_CENTER);
								}
							}
						}
					}
					Delay(4L);
					UA_InvertButton(window, &Controler_GR[ID]);
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

	SendSerCmd(CDTV_rec, DO_STOP);

	FreeMem(mypattern1, 4L);

	if ( retVal )
		PutVarsToPI(CDTV_rec, ThisPI);

	return(retVal);
}

/******** SetButtons() ********/

void SetButtons(struct CDTV_record *CDTV_rec, struct Window *window, UWORD *mypattern1,
								struct PopUpRecord *PUR)
{
BOOL disable[30];
int i;

	for(i=0; i<30; i++)
		disable[i] = FALSE;	

	for(i=8; i<15; i++)
		UA_EnableButton(window, &CDTV_GR[i]);

	if ( CDTV_rec->fadeIn )
		UA_InvertButton(window, &CDTV_GR[14]);	// fade

	switch( CDTV_rec->action )	// play song, play from...to etc.
	{
		case 0:	// play song
			disable[ 9] = TRUE;
			disable[10] = TRUE;
			disable[11] = TRUE;
			disable[12] = TRUE;
			disable[13] = TRUE;
			UA_SetStringGadgetToVal(window, &CDTV_GR[8], CDTV_rec->song);
			break;

		case 1:	// play from...to
			disable[ 8] = TRUE;
			disable[11] = TRUE;
			disable[12] = TRUE;
			disable[13] = TRUE;
			UA_SetStringGadgetToVal(window, &CDTV_GR[9], CDTV_rec->from);
			UA_SetStringGadgetToVal(window, &CDTV_GR[10], CDTV_rec->to);
			break;

		case 2:	// play start...end
			disable[ 8] = TRUE;
			disable[ 9] = TRUE;
			disable[10] = TRUE;
			disable[13] = TRUE;
			UA_SetStringGadgetToString(window, &CDTV_GR[11], CDTV_rec->start);
			UA_SetStringGadgetToString(window, &CDTV_GR[12], CDTV_rec->end);
			break;

		case 3:	// do command
			disable[ 8] = TRUE;
			disable[ 9] = TRUE;
			disable[10] = TRUE;
			disable[11] = TRUE;
			disable[12] = TRUE;
			disable[14] = TRUE;	// fade
			UA_PrintPopUpChoice(window, &CDTV_GR[13], PUR);
			break;
	}

	if ( disable[13] )
		UA_PrintPopUpChoice(window, &CDTV_GR[13], PUR);

	for(i=8; i<15; i++)
		if ( disable[i] )
			UA_DisableButton(window, &CDTV_GR[i], mypattern1);
}

/******** CheckTimeCode() ********/
/*
 * [99:59:74]
 *
 */

BOOL CheckTimeCode(STRPTR str)
{
	if (	str[0]>='[' &&
				str[1]>='0' && str[2]<='9' &&
				str[3]>=':' &&
				str[4]>='0' && str[5]<='9' &&
				str[6]>=':' &&
				str[7]>='0' && str[8]<='9' &&
				str[9]>=']' )
	{
		str[0]=str[1];
		str[1]=str[2];
		str[3]=str[4];
		str[4]=str[5];
		str[6]=str[7];
		str[7]=str[8];
		str[2]=':';
		str[5]=':';
		str[8]='\0';
		return(TRUE);
	}
	else
		return(FALSE);
}

/******** ControlCDTV() ********/

BOOL ControlCDTV(struct CDTV_record *CDTV_rec)
{
	if ( CDTV_rec->action==0 )
		SendSerCmd(CDTV_rec, DO_PLAYTRACK);
	else if ( CDTV_rec->action==1 )
		SendSerCmd(CDTV_rec, DO_PLAYFROMTO);
	else if ( CDTV_rec->action==2 )
		SendSerCmd(CDTV_rec, DO_PLAYSTARTEND);
	else if ( CDTV_rec->action==3 )
	{
		if ( CDTV_rec->command==0 )
			SendSerCmd(CDTV_rec, DO_FADE_IN_SLOW);
		if ( CDTV_rec->command==1 )
			SendSerCmd(CDTV_rec, DO_FADE_IN_FAST);
		if ( CDTV_rec->command==2 )
			SendSerCmd(CDTV_rec, DO_FADE_OUT_SLOW);
		if ( CDTV_rec->command==3 )
			SendSerCmd(CDTV_rec, DO_FADE_OUT_FAST);
		if ( CDTV_rec->command==4 )
			SendSerCmd(CDTV_rec, DO_MUTE_ON);
		if ( CDTV_rec->command==5 )
			SendSerCmd(CDTV_rec, DO_MUTE_OFF);
		if ( CDTV_rec->command==6 )
			SendSerCmd(CDTV_rec, DO_PAUSE);
		if ( CDTV_rec->command==7 )
			SendSerCmd(CDTV_rec, DO_STOP);
		if ( CDTV_rec->command==8 )
			SendSerCmd(CDTV_rec, DO_SINGLESTEP);
		if ( CDTV_rec->command==9 )
			SendSerCmd(CDTV_rec, DO_HALFSPEED);
		if ( CDTV_rec->command==10 )
			SendSerCmd(CDTV_rec, DO_QUARTERSPEED);
		if ( CDTV_rec->command==11 )
			SendSerCmd(CDTV_rec, DO_VIDEO_NO_AUDIO);
		if ( CDTV_rec->command==12 )
			SendSerCmd(CDTV_rec, DO_SLOMO_OFF);
	}

	return(TRUE);
}

/******** E O F ********/
