#include "nb:pre.h"
#include "setup.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"
#include "structs.h"
#include "math.h"
//#include <rexx/storage.h>
//#include <rexx/rxslib.h>
//#include <rexx/errors.h>
//#include "to:rexx4/simplerexx.h"

#define VERSI0N "\0$VER: 1.0"
#define MEMSIZE 512
static UBYTE *vers = VERSI0N;

/**** globals ****/

struct Library *medialinkLibBase		= NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase							= NULL;

UBYTE **msgs;
struct RendezVousRecord *rvrec;
struct MsgPort *capsport;
struct Window *globalWindow;

/**** disable CTRL-C break ****/

void CXBRK(void) { }
void chkabort(void) { }

/**** functions ****/

/******** XappSetup() ********/

void XappSetup(PROCESSINFO *ThisPI)
{
struct UserApplicInfo UAI;
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

	capsport = (struct MsgPort *)FindPort(MEDIALINKPORT);
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	if (UA_HostScreenPresent(&UAI))
		UAI.windowModes = 1;	/* open on the MediaLink screen */
	else
		UAI.windowModes = 3;	/* open on the first (frontmost) screen */

	/**** double the dimensions of gadgets etc. if screen is laced ****/

	if (UA_IsUAScreenLaced(&UAI))
		UA_DoubleGadgetDimensions(PAR_GR);

	UA_TranslateGR(PAR_GR, msgs);

	UAI.windowX				= -1;	/* -1 means center on screen */
	UAI.windowY				= -1;	/* -1 means center on screen */
	UAI.windowWidth		= PAR_GR[0].x2;
	UAI.windowHeight	= PAR_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(&UAI);
	globalWindow = UAI.userWindow;

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow,PAR_GR);

	/**** monitor events ****/

	MonitorUser(UAI.userWindow, ThisPI);
	
	/**** close the window ****/

	UA_CloseWindow(&UAI);

	capsport->mp_SigTask = oldTask;
}

/******** MonitorUser() ********/

BOOL MonitorUser(struct Window *window, PROCESSINFO *ThisPI)
{
BOOL loop=TRUE, retVal;
struct EventData CED;
int i, ID, h, y, numFrames, frame;
struct FileReqRecord FRR;
TEXT path[SIZE_PATH], filename[SIZE_FILENAME], str[SIZE_FULLPATH], cmd[256];
UWORD *mypattern1;
struct PAR_Record par_rec;
char *args[17];
struct Gadget *g;
ULONG body,pot;
struct PropInfo *propInfo;
WORD startX, endX, y1, y2;
int frameRate;
ULONG ll,hh,mm,ss,ff;
BOOL NOPAR=FALSE;

	/**** init vars ****/

	numFrames = 0;
	frame = 1;

	mypattern1 = (UWORD *)AllocMem(4L, MEMF_CHIP);
	if (mypattern1==NULL)
		return(FALSE);
	mypattern1[0] = 0x5555;
	mypattern1[1] = 0xaaaa;

	for(i=0; i<=6; i++)
		args[i] = (char *)AllocMem(256L,MEMF_CLEAR|MEMF_ANY);

	if ( rvrec->capsprefs->PlayerPalNtsc == PAL_MODE )
		frameRate = 25;
	else
		frameRate = 30;

	/**** set up FRR ****/

	strcpy(path, rvrec->capsprefs->sample_Path);
	filename[0] = '\0';

	FRR.path			= path;
	FRR.fileName	= filename;
	FRR.opts			= DIR_OPT_ALL | DIR_OPT_NOINFO;
	FRR.multiple	= FALSE;
	FRR.title			= msgs[Msg_S16_8-1];

	/**** set or read settings ****/

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] == '\0' )
	{
		par_rec.path[0] = '\0';
		strcpy(par_rec.in, "00:00:00:00");
		strcpy(par_rec.out, "00:00:00:00");
		strcpy(par_rec.cue, "00:00:00:00");
		par_rec.studio16Cue	= FALSE;
		par_rec.startFrame	= 0;
		par_rec.endFrame		= 0;
		par_rec.startPix		= PAR_GR[22].x1-2;
		par_rec.endPix			= PAR_GR[22].x2-13;
	}
	else
	{
		GetExtraData(ThisPI,&par_rec);
		UA_SplitFullPath(par_rec.path,path,filename);
	}

	/**** set buttons ****/

	if ( par_rec.path[0]=='\0' )
		strcpy(str, msgs[Msg_X_5-1]);
	else
		strcpy(str, par_rec.path);
	UA_ShortenString(window->RPort, str, PAR_GR[8].x2-PAR_GR[8].x1-16);
	UA_ClearButton(window, &PAR_GR[8], AREA_PEN);
	UA_DrawText(window, &PAR_GR[8], str);

	UA_SetStringGadgetToString(window, &PAR_GR[12], par_rec.cue);
	if ( !par_rec.studio16Cue )
		UA_DisableButton(window, &PAR_GR[12], mypattern1);	// studio 16 cue time
	else
		UA_InvertButton(window, &PAR_GR[11]);

	/**** render cassette buttons ****/

	if ( UA_IsWindowOnLacedScreen(window) )
	{
		h = GFX_CR_H_L;
		y = GFX_CR_Y_L;
	}
	else
	{
		h = GFX_CR_H_NL;
		y = GFX_CR_Y_NL;
	}

	UA_PutImageInRastPort(rvrec->gfxBM, GFX_CR_STOP_X, y, GFX_W, window->RPort,
												PAR_GR[13].x1+((PAR_GR[13].x2-PAR_GR[13].x1+1)-GFX_CR_W)/2,
												PAR_GR[13].y1+((PAR_GR[13].y2-PAR_GR[13].y1+1)-h)/2,
												GFX_CR_W, h);

	UA_PutImageInRastPort(rvrec->gfxBM, GFX_CR_FWD_X, y, GFX_W, window->RPort,
												PAR_GR[14].x1+((PAR_GR[14].x2-PAR_GR[14].x1+1)-GFX_CR_W)/2,
												PAR_GR[14].y1+((PAR_GR[14].y2-PAR_GR[14].y1+1)-h)/2,
												GFX_CR_W, h);

	UA_PutImageInRastPort(rvrec->gfxBM, GFX_CR_STEPREW_X, y, GFX_W, window->RPort,
												PAR_GR[15].x1+((PAR_GR[15].x2-PAR_GR[15].x1+1)-GFX_CR_W)/2,
												PAR_GR[15].y1+((PAR_GR[15].y2-PAR_GR[15].y1+1)-h)/2,
												GFX_CR_W, h);

	UA_PutImageInRastPort(rvrec->gfxBM, GFX_CR_STEPFWD_X, y, GFX_W, window->RPort,
												PAR_GR[16].x1+((PAR_GR[16].x2-PAR_GR[16].x1+1)-GFX_CR_W)/2,
												PAR_GR[16].y1+((PAR_GR[16].y2-PAR_GR[16].y1+1)-h)/2,
												GFX_CR_W, h);

	UA_PutImageInRastPort(rvrec->gfxBM, GFX_CR_FASTREW_X, y, GFX_W, window->RPort,
												PAR_GR[17].x1+((PAR_GR[17].x2-PAR_GR[17].x1+1)-GFX_CR_W)/2,
												PAR_GR[17].y1+((PAR_GR[17].y2-PAR_GR[17].y1+1)-h)/2,
												GFX_CR_W, h);

	UA_PutImageInRastPort(rvrec->gfxBM, GFX_CR_FASTFWD_X, y, GFX_W, window->RPort,
												PAR_GR[18].x1+((PAR_GR[18].x2-PAR_GR[18].x1+1)-GFX_CR_W)/2,
												PAR_GR[18].y1+((PAR_GR[18].y2-PAR_GR[18].y1+1)-h)/2,
												GFX_CR_W, h);

	UA_PutImageInRastPort(rvrec->gfxBM, GFX_CR_PAUSE_X, y, GFX_W, window->RPort,
												PAR_GR[19].x1+((PAR_GR[19].x2-PAR_GR[19].x1+1)-GFX_CR_W)/2,
												PAR_GR[19].y1+((PAR_GR[19].y2-PAR_GR[19].y1+1)-h)/2,
												GFX_CR_W, h);

/*
	UA_PutImageInRastPort(rvrec->gfxBM, GFX_CR_LOOP_X, y, GFX_W, window->RPort,
												PAR_GR[20].x1+((PAR_GR[20].x2-PAR_GR[20].x1+1)-GFX_CR_W)/2,
												PAR_GR[20].y1+((PAR_GR[20].y2-PAR_GR[20].y1+1)-h)/2,
												GFX_CR_W, h);
*/

	/**** Check if PAR host is around ****/

	if ( !FindPort("DDR") )
	{
		NOPAR=TRUE;
		GiveMessage(window, msgs[Msg_NoARexxPort-1], "PAR");
	}
	else
	{
		if ( par_rec.path[0] )
		{
/*
			DoArexxCmd("PAUSE ON",NULL,0);
			DoArexxCmd("STARTFRAME 1",NULL,0);
			sprintf(cmd,"PLAY '%s'",par_rec.path);
			DoArexxCmd(cmd,NULL,0);
			DoArexxCmd("STOP",NULL,0);
			DoArexxCmd("PAUSE OFF",NULL,0);
*/
			sprintf(cmd,"FILE '%s'",par_rec.path);
			DoArexxCmd(cmd,NULL,0);
		}

		// read # frames

		if ( DoArexxCmd("FRAMES",str,SIZE_FULLPATH) )
			sscanf(str,"%d",&numFrames);

		// read pause state

		if ( DoArexxCmd("PAUSE",str,SIZE_FULLPATH) )
		{
			if ( !strnicmp(str,"ON",2) )
				UA_InvertButton(window, &PAR_GR[19]);
		}

/*
		// read loop state

		if ( DoArexxCmd("LOOP",str,SIZE_FULLPATH) )
		{
			if ( !strnicmp(str,"ON",2) )
				UA_InvertButton(window, &PAR_GR[20]);
		}
*/

		// Print frame number

		PrintFrameNr(window,numFrames);
		if ( DoArexxCmd("JUMP",str,SIZE_FULLPATH) )
			sscanf(str,"%d",&frame);
	}

	/**** init slider ****/

	PropSlider.LeftEdge	= PAR_GR[22].x1+4;
	PropSlider.TopEdge	= PAR_GR[22].y1+2;
	PropSlider.Width		= PAR_GR[22].x2-PAR_GR[22].x1-7;
	PropSlider.Height		= PAR_GR[22].y2-PAR_GR[22].y1-3;
	if ( UA_IsWindowOnLacedScreen(window) )
	{
		PropSlider.TopEdge	+= 2;
		PropSlider.Height		-= 4;
	}
	propInfo = (struct PropInfo *)PropSlider.SpecialInfo;
	InitPropInfo(propInfo, (struct Image *)PropSlider.GadgetRender);
	AddGadget(window, &PropSlider, -1L);

	if ( numFrames > 1 )
	{
		body = (MAXBODY)/numFrames;
		pot = (((ULONG)MAXPOT)*(frame-1))/(numFrames-1);
		NewModifyProp(&PropSlider, window, NULL, AUTOKNOB|FREEHORIZ|PROPBORDERLESS,
									pot,			/* horizPot */ 
									0L,				/* vertPot */
									body,			/* horizBody */
									MAXBODY,	/* vertBody */
									1L);
		RefreshGList(&PropSlider, window, NULL, 1);
	}

	/**** render limits ****/

	SetAPen(window->RPort,LO_PEN);
	SetDrMd(window->RPort,JAM1);

	y1 = PAR_GR[22].y2;
	if ( UA_IsWindowOnLacedScreen(window) )
		y2 = y1 + 14; 
	else
		y2 = y1 + 7; 

	if ( numFrames>0 )
	{
		if ( par_rec.startFrame==0 )
			par_rec.startFrame = 1;
	
		if ( par_rec.endFrame==0 )
			par_rec.endFrame = numFrames;
	}

	startX = par_rec.startPix;
	Move(window->RPort, startX, y1+window->RPort->TxBaseline);
	Text(window->RPort, "\0", 1L);

	endX = par_rec.endPix;
	Move(window->RPort, endX,   y1+window->RPort->TxBaseline);
	Text(window->RPort, "\0", 1L);

	if ( numFrames > 0 )
		SetBackSlider(window,propInfo,&par_rec,numFrames,(MAXBODY)/numFrames);

	/**** print in and out ****/

	UA_ClearButton(window, &PAR_GR[ 9], AREA_PEN);
	if ( numFrames > 0 )
		UA_DrawSpecialGadgetText(window, &PAR_GR[ 9], par_rec.in, SPECIAL_TEXT_CENTER);

	UA_ClearButton(window, &PAR_GR[10], AREA_PEN);
	if ( numFrames > 0 )
		UA_DrawSpecialGadgetText(window, &PAR_GR[10], par_rec.out, SPECIAL_TEXT_CENTER);

	/**** disable slider if needed ****/

	if ( numFrames <=1 )
		OffGadget(&PropSlider,window,NULL);

	/**** if no par ghost all buttons ****/

	if ( NOPAR )
	{
		OffGadget(&PropSlider,window,NULL);
		UA_DisableButton(window, &PAR_GR[6], mypattern1);	// preview
		for(i=8; i<=21; i++)
			UA_DisableButton(window, &PAR_GR[i], mypattern1);
	}

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if ( CED.extraClass==IDCMP_GADGETDOWN || CED.extraClass==IDCMP_GADGETUP )
		{
			if ( numFrames>0 )
			{
				g = (struct Gadget *)CED.IAddress;
				if (g && g->GadgetID==1 )
					DoSlider(window, g, (MAXBODY)/numFrames, propInfo, numFrames, &par_rec);
			}
		}
		else if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = -1;
			if (	numFrames>0 && CED.MouseX >= (startX-1) && CED.MouseX <= (startX+13) &&
						CED.MouseY >= (y1+1) && CED.MouseY <= (y2+1) )
			{
				DragArrow(window,&startX,endX,y1,y2,1,numFrames,frameRate,&par_rec);
				SetBackSlider(window,propInfo,&par_rec,numFrames,(MAXBODY)/numFrames);

				frame = (((ULONG)propInfo->HorizPot)*(numFrames-1)+MAXPOT/2)/MAXPOT;
				frame++;
				sprintf(cmd,"JUMP %d",frame);
				DoArexxCmd(cmd,NULL,0);

				PrintFrameNr(window,numFrames);
			}
			else if (	numFrames>0 && CED.MouseX >= (endX-1) && CED.MouseX <= (endX+13) &&
								CED.MouseY >= (y1+1) && CED.MouseY <= (y2+1) )
			{
				DragArrow(window,&endX,startX,y1,y2,2,numFrames,frameRate,&par_rec);
				SetBackSlider(window,propInfo,&par_rec,numFrames,(MAXBODY)/numFrames);

				frame = (((ULONG)propInfo->HorizPot)*(numFrames-1)+MAXPOT/2)/MAXPOT;
				frame++;
				sprintf(cmd,"JUMP %d",frame);
				DoArexxCmd(cmd,NULL,0);

				PrintFrameNr(window,numFrames);
			}
			else
				ID = UA_CheckGadgetList(window, PAR_GR, &CED);
			switch(ID)
			{
				case 4:		// OK
do_ok:
					UA_HiliteButton(window, &PAR_GR[4]);
					loop=FALSE;
					retVal=TRUE; 
					break;

				case 5:		// Cancel
do_cancel:
					UA_HiliteButton(window, &PAR_GR[5]);
					loop=FALSE;
					retVal=FALSE; 
					break;

				case 6: 	// Preview
					UA_HiliteButton(window, &PAR_GR[ID]);
					UA_SetSprite(window, SPRITE_BUSY);

					Preview(&par_rec);

#if 0
					if ( par_rec.path[0] )
					{
						sprintf(cmd,"PROJECT '%s'",par_rec.path);
						DoArexxCmd(cmd,NULL,0);
					}

					sprintf(cmd,"JUMP %d",par_rec.startFrame);
					DoArexxCmd(cmd,NULL,0);

					DoArexxCmd("PLAY",NULL,0);

					//DoArexxCmd("WAIT",NULL,0);
#endif

					UA_SetSprite(window, SPRITE_NORMAL);
					break;

				case 8:		// open a file
					UA_InvertButton(window, &PAR_GR[ID]);
					if ( UA_OpenAFile(window, &FRR, mypattern1) )
					{
						UA_MakeFullPath(FRR.path, FRR.fileName, par_rec.path);
						strcpy(str, par_rec.path);
						UA_ShortenString(window->RPort, str, PAR_GR[ID].x2-PAR_GR[ID].x1-16);
						UA_ClearButton(window, &PAR_GR[ID], AREA_PEN);
						UA_DrawText(window, &PAR_GR[ID], str);

						sprintf(str,"FILE '%s'",par_rec.path);
						DoArexxCmd(str,NULL,0);

						if ( DoArexxCmd("FRAMES",str,SIZE_FULLPATH) )
							sscanf(str,"%d",&numFrames);

						if ( numFrames <=1 )
							OffGadget(&PropSlider,window,NULL);
						else
							OnGadget(&PropSlider,window,NULL);

						par_rec.startFrame = 1;
						par_rec.endFrame = numFrames;

						SetDrMd(window->RPort,JAM1);
						SetAPen(window->RPort,AREA_PEN);
						Move(window->RPort, startX, y1+window->RPort->TxBaseline);
						Text(window->RPort, "\0", 1L);
						Move(window->RPort, endX,   y1+window->RPort->TxBaseline);
						Text(window->RPort, "\0", 1L);
						SetAPen(window->RPort,LO_PEN);
						startX = PAR_GR[22].x1-2;
						Move(window->RPort, startX, y1+window->RPort->TxBaseline);
						Text(window->RPort, "\0", 1L);
						endX = PAR_GR[22].x2-13;
						Move(window->RPort, endX,   y1+window->RPort->TxBaseline);
						Text(window->RPort, "\0", 1L);

						ll = par_rec.startFrame;					
						hh = ll / (60*60*frameRate);
						ll = ll - (hh * (60*60*frameRate));
						mm = ll / (60*frameRate);
						ll = ll - (mm * (60*frameRate));
						ss = ll / frameRate;
						ff = ll - (ss * frameRate);
						sprintf(par_rec.in, "%02d:%02d:%02d:%02d", hh,mm,ss,ff);
						UA_ClearButton(window, &PAR_GR[ 9], AREA_PEN);
						if ( numFrames > 0 )
							UA_DrawSpecialGadgetText(window, &PAR_GR[ 9], par_rec.in, SPECIAL_TEXT_CENTER);

						ll = par_rec.endFrame;					
						hh = ll / (60*60*frameRate);
						ll = ll - (hh * (60*60*frameRate));
						mm = ll / (60*frameRate);
						ll = ll - (mm * (60*frameRate));
						ss = ll / frameRate;
						ff = ll - (ss * frameRate);
						sprintf(par_rec.out, "%02d:%02d:%02d:%02d", hh,mm,ss,ff);
						UA_ClearButton(window, &PAR_GR[10], AREA_PEN);
						if ( numFrames > 0 )
							UA_DrawSpecialGadgetText(window, &PAR_GR[10], par_rec.out, SPECIAL_TEXT_CENTER);

						PrintFrameNr(window,numFrames);
					}
					else
						UA_InvertButton(window, &PAR_GR[ID]);
					break;

				case 11:	// cue on/off
					UA_InvertButton(window, &PAR_GR[ID]);
					if ( par_rec.studio16Cue )
					{
						par_rec.studio16Cue = FALSE;
						UA_DisableButton(window, &PAR_GR[12], mypattern1);	// studio 16 cue time
						DoArexxCmd("SMPTE OFF",NULL,0);
					}
					else
					{
						par_rec.studio16Cue = TRUE;
						UA_EnableButton(window, &PAR_GR[12]);	// studio 16 cue time
						sprintf(str,"SMPTE %s",par_rec.cue);
						DoArexxCmd(str,NULL,0);
						DoArexxCmd("SMPTE ON",NULL,0);
					}
					break;	

				case 12:	// set cue
					UA_ProcessStringGadget(window, PAR_GR, &PAR_GR[ID], &CED);
					UA_SetStringToGadgetString(&PAR_GR[ID], par_rec.cue);
					PAR_CheckEnteredTimeCode(frameRate, par_rec.cue);
					UA_SetStringGadgetToString(window, &PAR_GR[ID], par_rec.cue);
					sprintf(str,"SMPTE %s",par_rec.cue);
					DoArexxCmd(str,NULL,0);
					break;

				case 13:	// Stop
					if ( numFrames>0 )
					{
						UA_InvertButton(window, &PAR_GR[ID]);
						DoArexxCmd("STOP",NULL,0);
						UA_InvertButton(window, &PAR_GR[ID]);
						PrintFrameNr(window,numFrames);
					}
					break;

				case 14:	// Play
					if ( numFrames>0 )
					{
						UA_InvertButton(window, &PAR_GR[ID]);

						frame = (((ULONG)propInfo->HorizPot)*(numFrames-1)+MAXPOT/2)/MAXPOT;
						frame++;
						if ( frame==numFrames )
							frame=0;
						sprintf(cmd,"JUMP %d",frame);
						DoArexxCmd(cmd,NULL,0);

						PrintFrameNr(window,numFrames);
						if ( numFrames>0 )
							DoArexxCmd("PLAY",NULL,0);
						UA_InvertButton(window, &PAR_GR[ID]);

						if ( DoArexxCmd("PAUSE",str,SIZE_FULLPATH) )
						{
							UA_ClearButton(window, &PAR_GR[19], AREA_PEN);
							UA_PutImageInRastPort(rvrec->gfxBM, GFX_CR_PAUSE_X, y, GFX_W, window->RPort,
																		PAR_GR[19].x1+((PAR_GR[19].x2-PAR_GR[19].x1+1)-GFX_CR_W)/2,
																		PAR_GR[19].y1+((PAR_GR[19].y2-PAR_GR[19].y1+1)-h)/2,
																		GFX_CR_W, h);
							if ( !strnicmp(str,"ON",2) )
								UA_InvertButton(window, &PAR_GR[19]);
						}
					}
					break;

				case 15:	// Step rew
					UA_InvertButton(window, &PAR_GR[ID]);
					if ( numFrames>0 )
					{
						if ( DoArexxCmd("JUMP",str,SIZE_FULLPATH) )
						{
							sscanf(str,"%d",&frame);
							if ( frame>1 && frame<=numFrames )
							{
								frame--;
								sprintf(cmd,"JUMP %d",frame);
								DoArexxCmd(cmd,NULL,0);
							}
						}
					}
					UA_InvertButton(window, &PAR_GR[ID]);
					PrintFrameNr(window,numFrames);
					break;

				case 16:	// Step fwd
					UA_InvertButton(window, &PAR_GR[ID]);
					if ( numFrames>0 )
					{
						if ( DoArexxCmd("JUMP",str,SIZE_FULLPATH) )
						{
							sscanf(str,"%d",&frame);
							if ( frame<numFrames )
							{
								frame++;
								sprintf(cmd,"JUMP %d",frame);
								DoArexxCmd(cmd,NULL,0);
							}
						}
					}
					UA_InvertButton(window, &PAR_GR[ID]);
					PrintFrameNr(window,numFrames);
					break;

				case 17:	// Fast rew
					UA_InvertButton(window, &PAR_GR[ID]);
					if ( numFrames>0 )
					{
						frame=1;
						sprintf(cmd,"JUMP %d",frame);
						DoArexxCmd(cmd,NULL,0);
					}
					UA_InvertButton(window, &PAR_GR[ID]);
					PrintFrameNr(window,numFrames);
					break;

				case 18:	// Fast fwd
					UA_InvertButton(window, &PAR_GR[ID]);
					if ( numFrames>0 )
					{
						frame=numFrames;
						sprintf(cmd,"JUMP %d",frame);
						DoArexxCmd(cmd,NULL,0);
					}
					UA_InvertButton(window, &PAR_GR[ID]);
					PrintFrameNr(window,numFrames);
					break;

				case 19:	// Pause
					if ( numFrames>0 )
					{
						if ( DoArexxCmd("PAUSE",str,SIZE_FULLPATH) )
						{
							UA_ClearButton(window, &PAR_GR[ID], AREA_PEN);
							UA_PutImageInRastPort(rvrec->gfxBM, GFX_CR_PAUSE_X, y, GFX_W, window->RPort,
																		PAR_GR[19].x1+((PAR_GR[19].x2-PAR_GR[19].x1+1)-GFX_CR_W)/2,
																		PAR_GR[19].y1+((PAR_GR[19].y2-PAR_GR[19].y1+1)-h)/2,
																		GFX_CR_W, h);
							if ( !strnicmp(str,"OFF",3) )
							{
								DoArexxCmd("PAUSE ON",NULL,0);
								UA_InvertButton(window, &PAR_GR[ID]);
								PrintFrameNr(window,numFrames);
							}
							else
								DoArexxCmd("PAUSE OFF",NULL,0);
						}
					}
					break;

/*
				case 20:	// Loop
					if ( numFrames>0 )
					{
						if ( DoArexxCmd("LOOP",str,SIZE_FULLPATH) )
						{
							UA_ClearButton(window, &PAR_GR[ID], AREA_PEN);
							UA_PutImageInRastPort(rvrec->gfxBM, GFX_CR_LOOP_X, y, GFX_W, window->RPort,
																		PAR_GR[20].x1+((PAR_GR[20].x2-PAR_GR[20].x1+1)-GFX_CR_W)/2,
																		PAR_GR[20].y1+((PAR_GR[20].y2-PAR_GR[20].y1+1)-h)/2,
																		GFX_CR_W, h);
							if ( !strnicmp(str,"OFF",3) )
							{
								DoArexxCmd("LOOP ON",NULL,0);
								UA_InvertButton(window, &PAR_GR[ID]);
							}
							else
								DoArexxCmd("LOOP OFF",NULL,0);
						}
					}
					break;
*/
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

	for(i=0; i<=6; i++)
		FreeMem(args[i],256L);

	if ( retVal )
	{
		par_rec.startPix = startX;
		par_rec.endPix = endX;
		PutExtraData(ThisPI, &par_rec);
	}

	return(retVal);
}

/******** GetExtraData() ********/

void GetExtraData(PROCESSINFO *ThisPI, struct PAR_Record *par_rec)
{
int numChars, argNum, len, numSeen;
char *strPtr;
char tmp[MEMSIZE], scrStr[MEMSIZE];

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] == '\0' )
		return;

	ScriptToStr(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData, scrStr);

	strPtr = scrStr;
	len = strlen(strPtr);
	argNum=0;
	numSeen=0;

	while(1)
	{
		if (*strPtr==0)
			break;
		numChars = stcarg(strPtr, " 	,");	// space, tab, comma
		if ( numChars>=1 && numChars<MEMSIZE )
		{
			stccpy(tmp, strPtr, numChars+1);
			switch(argNum)
			{
				case 0:
					RemoveQuotes(tmp);
					strcpy(par_rec->path, tmp);
					break;

				case 1:
					sscanf(tmp, "%s", par_rec->in);
					break;

				case 2:
					sscanf(tmp, "%s", par_rec->out);
					break;

				case 3:
					sscanf(tmp, "%s", par_rec->cue);
					break;

				case 4:
					sscanf(tmp, "%d", &par_rec->studio16Cue);
					break;

				case 5:
					sscanf(tmp, "%d", &par_rec->startFrame);
					break;

				case 6:
					sscanf(tmp, "%d", &par_rec->endFrame);
					break;

				case 7:
					sscanf(tmp, "%d", &par_rec->startPix);
					break;

				case 8:
					sscanf(tmp, "%d", &par_rec->endPix);
					break;
			}
			argNum++;
		}
		strPtr += numChars+1;
		numSeen += numChars+1;
		if (numSeen>len)
			break;		
	}
}

/******** PutExtraData() ********/

void PutExtraData(PROCESSINFO *ThisPI, struct PAR_Record *par_rec)
{
UBYTE scrStr[SIZE_FULLPATH+10];

	StrToScript(par_rec->path, scrStr);

	sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"\\\"%s\\\" %s %s %s %d %d %d %d %d",
					par_rec->path,
					par_rec->in,
					par_rec->out,
					par_rec->cue,
					par_rec->studio16Cue,
					par_rec->startFrame,
					par_rec->endFrame,
					par_rec->startPix,
					par_rec->endPix);
}

/******** DoArexxCmd() ********/

BOOL DoArexxCmd(STRPTR cmd, STRPTR ret, int max)
{
BOOL retval=FALSE;
char RString[256];

	if ( ret )
	{
		sprintf(RString,"options results\naddress DDR %s\nsay result",cmd);
		retval = UA_IssueRexxCmd_V1("MP_PAR",RString,ret,TRUE,max);
	}
	else
	{
		sprintf(RString,"address DDR %s",cmd);
		retval = UA_IssueRexxCmd_V1("MP_PAR",RString,NULL,FALSE,0);
	}

	return(retval);
}

/******** PrintFrameNr() ********/

void PrintFrameNr(struct Window *window, int numFrames)
{
TEXT str[10];
ULONG body,pot;
int frame=0;

	if ( DoArexxCmd("JUMP",str,9) )
	{
		UA_ClearButton(window, &PAR_GR[21], AREA_PEN);
		if ( str[0] != '0' )
		{
			sscanf(str,"%d",&frame);
			sprintf(str,"%d",frame);
			UA_DrawSpecialGadgetText(window, &PAR_GR[21], str, SPECIAL_TEXT_CENTER);
		}
		if ( frame>0 && numFrames>1 )
		{	
			// START - SET SLIDER
			body = (MAXBODY)/numFrames;
			pot = (((ULONG)MAXPOT)*(frame-1))/(numFrames-1);
			NewModifyProp(&PropSlider, window, NULL, AUTOKNOB|FREEHORIZ|PROPBORDERLESS,
										pot,			/* horizPot */ 
										0L,				/* vertPot */
										body,			/* horizBody */
										MAXBODY,	/* vertBody */
										1L);
			RefreshGList(&PropSlider, window, NULL, 1);
			// END - SET SLIDER
		}
	}
}

/******** InitPropInfo() ********/
/*
 * This functions should eliminate the problems with prop gadgets
 * which are used in non-lace and lace environments and write over
 * the container in which they live.
 *
 */

void InitPropInfo(struct PropInfo *PropInfo, struct Image *IM)
{
	PropInfo->HorizPot = 0;
	PropInfo->HorizBody = 0;
	PropInfo->CHeight = 0;
	PropInfo->HPotRes = 0;

	IM->Height = 0;
	IM->Depth = 0;
	IM->ImageData = NULL;
	IM->PlanePick	= 0x0000;
	IM->PlaneOnOff = 0x0000;
	IM->NextImage = NULL;
}

/******** DoSlider() ********/

void DoSlider(struct Window *window, struct Gadget *g, ULONG body,
							struct PropInfo *PropInfo, UWORD numLevels, struct PAR_Record *par_rec)
{
ULONG signals, sigMask;
BOOL loop=TRUE;
struct IntuiMessage *message;
BOOL mouseMoved=FALSE;
struct EventData CED;
UWORD level;
TEXT str[50];

	UA_SwitchMouseMoveOn(window);
	sigMask = 1L << capsport->mp_SigBit;

	level = (((ULONG)PropInfo->HorizPot)*(numLevels-1)+MAXPOT/2)/MAXPOT;

	if ( (level+1) < par_rec->startFrame )
		level = par_rec->startFrame-1;
	if ( (level+1) > par_rec->endFrame )
		level = par_rec->endFrame-1;

	UA_ClearButton(window, &PAR_GR[21], AREA_PEN);
	sprintf(str,"%d",level+1);
	UA_DrawSpecialGadgetText(window, &PAR_GR[21], str, SPECIAL_TEXT_CENTER);

	sprintf(str,"JUMP %d",level+1);
	DoArexxCmd(str,NULL,0);

	while(loop)
	{
		signals = Wait( sigMask );
		if (signals & sigMask)
		{
			mouseMoved=FALSE;
			while(message = (struct IntuiMessage *)GetMsg(capsport))
			{
				CED.Class	= message->Class;
				ReplyMsg((struct Message *)message);
				if ( CED.Class == IDCMP_MOUSEMOVE )
					mouseMoved=TRUE;
				else
					loop=FALSE;
			}
			if (mouseMoved)
			{
				if (g->Flags & GFLG_SELECTED)
				{
					level = (((ULONG)PropInfo->HorizPot)*(numLevels-1)+MAXPOT/2)/MAXPOT;

					if ( (level+1) < par_rec->startFrame )
						level = par_rec->startFrame-1;
					if ( (level+1) > par_rec->endFrame )
						level = par_rec->endFrame-1;

					UA_ClearButton(window, &PAR_GR[21], AREA_PEN);
					sprintf(str,"%d",level+1);
					UA_DrawSpecialGadgetText(window, &PAR_GR[21], str, SPECIAL_TEXT_CENTER);

					sprintf(str,"JUMP %d",level+1);
					DoArexxCmd(str,NULL,0);
 				}
				else
					loop=FALSE;
			}
		}
	}

	UA_SwitchMouseMoveOff(window);

	SetBackSlider(window, PropInfo, par_rec, numLevels, body);
}

/******** DragArrow() ********/

void DragArrow(	struct Window *window, WORD *startX, WORD endX, WORD y1, WORD y2,
								BYTE mode, int numFrames, int frameRate, struct PAR_Record *par_rec)
{
ULONG signals, sigMask;
BOOL loop=TRUE;
struct IntuiMessage *message;
BOOL mouseMoved=FALSE;
struct EventData CED;
WORD diff;
LONG pix,t,tb;
ULONG l,h,m,s,f;
TEXT str[20];

	SetAPen(window->RPort,HI_PEN);
	SetDrMd(window->RPort,JAM1);

	Move(window->RPort, *startX, y1+window->RPort->TxBaseline);
	Text(window->RPort, "\0", 1L);

	UA_SwitchMouseMoveOn(window);
	sigMask = 1L << capsport->mp_SigBit;

	diff = *startX - window->MouseX;

	while(loop)
	{
		signals = Wait( sigMask );
		if (signals & sigMask)
		{
			mouseMoved=FALSE;
			while(message = (struct IntuiMessage *)GetMsg(capsport))
			{
				CED.Class	= message->Class;
				ReplyMsg((struct Message *)message);
				if ( CED.Class == IDCMP_MOUSEMOVE )
					mouseMoved=TRUE;
				else
					loop=FALSE;
			}
			if (mouseMoved)
			{
				SetAPen(window->RPort,AREA_PEN);
				Move(window->RPort, *startX, y1+window->RPort->TxBaseline);
				Text(window->RPort, "\0", 1L);

				if ( mode==1 )
				{	
					*startX = window->MouseX + diff;
					if ( *startX > (endX-14) )
						*startX = endX-14;
					if ( *startX < PAR_GR[22].x1-2 )
						*startX = PAR_GR[22].x1-2;
				}
				else if ( mode==2 )
				{
					*startX = window->MouseX + diff;
					if ( *startX > PAR_GR[22].x2-13 )
						*startX = PAR_GR[22].x2-13;
					if ( *startX < endX+14 )
						*startX = endX+14;
				}

				pix = *startX - PAR_GR[22].x1-2;
				tb = ( PAR_GR[22].x2-13 ) - ( PAR_GR[22].x1-2 ) + 1;
				if ( pix<=0 || tb==0 )
					t = 0;
				else if ( *startX == PAR_GR[22].x2-13 )
					t = numFrames-1;
				else
					t = ( pix * numFrames ) / tb;

				if ( mode==1 )
					par_rec->startFrame = t+1;
				else if ( mode==2 )
					par_rec->endFrame = t+1;

				l = t+1;					
				h = l / (60*60*frameRate);
				l = l - (h * (60*60*frameRate));
				m = l / (60*frameRate);
				l = l - (m * (60*frameRate));
				s = l / frameRate;
				f = l - (s * frameRate);
				if ( mode==1 )
				{
					sprintf(par_rec->in, "%02d:%02d:%02d:%02d", h,m,s,f);
					UA_ClearButton(window, &PAR_GR[ 9], AREA_PEN);
					UA_DrawSpecialGadgetText(window, &PAR_GR[ 9], par_rec->in, SPECIAL_TEXT_CENTER);
				}
				else if ( mode==2 )
				{
					sprintf(par_rec->out, "%02d:%02d:%02d:%02d", h,m,s,f);
					UA_ClearButton(window, &PAR_GR[10], AREA_PEN);
					UA_DrawSpecialGadgetText(window, &PAR_GR[10], par_rec->out, SPECIAL_TEXT_CENTER);
				}

				SetAPen(window->RPort,HI_PEN);
				Move(window->RPort, *startX, y1+window->RPort->TxBaseline);
				Text(window->RPort, "\0", 1L);
			}
		}
	}

	SetAPen(window->RPort,LO_PEN);
	Move(window->RPort, *startX, y1+window->RPort->TxBaseline);
	Text(window->RPort, "\0", 1L);

	UA_SwitchMouseMoveOff(window);

	sprintf(str,"STARTFRAME %d",par_rec->startFrame);
	DoArexxCmd(str,NULL,0);
	sprintf(str,"ENDFRAME %d",par_rec->endFrame);
	DoArexxCmd(str,NULL,0);
}

/******** PAR_CheckEnteredTimeCode() ********/

void PAR_CheckEnteredTimeCode(int frameRate, STRPTR str)
{
int hours, mins, secs, frames, i;
TEXT tmp[16];

	hours		= 0;
	mins		= 0;
	secs		= 0;
	frames	= 0;

	stccpy(tmp, str, 16);

	/**** remove - . etc ****/	

	for(i=0; i<strlen(tmp); i++)
		if ( isdigit(tmp[i]) == 0 )
			tmp[i]=' ';

	sscanf(tmp, "%d %d %d %d", &hours, &mins, &secs, &frames);

	PAR_CheckHMSF(&hours, &mins, &secs, &frames, frameRate);

	sprintf(str, "%02d:%02d:%02d:%02d", hours, mins, secs, frames);
}

/******** PAR_CheckHMSF() ********/

void PAR_CheckHMSF(int *h, int *m, int *s, int *f, int frameRate)
{
int maxFF;

	if ( frameRate==25 )
		maxFF=23;
	else if ( frameRate==30 )
		maxFF=29;

	if ( *h < 0 )
		*h = 0;
	if ( *h > 99 )
		*h = 99;

	if ( *m < 0 )
		*m = 0;
	if ( *m > 59 )
		*m = 59;

	if ( *s < 0 )
		*s = 0;
	if ( *s > 59 )
		*s = 59;

	if ( *f < 0 )
		*f = 0;
	if ( *f > maxFF )
		*f = maxFF;
}

/******** SetBackSlider() ********/

void SetBackSlider(	struct Window *window, struct PropInfo *PropInfo,
										struct PAR_Record *par_rec, UWORD numLevels, ULONG body )
{
BOOL doit=FALSE;
UWORD level;
ULONG pot;
TEXT str[50];

	/**** set slider of prop slider between arrows ****/

	doit=FALSE;
	level = (((ULONG)PropInfo->HorizPot)*(numLevels-1)+MAXPOT/2)/MAXPOT;

	if ( (level+1) < par_rec->startFrame )
	{
		level = par_rec->startFrame-1;
		doit=TRUE;
	}
	else if ( (level+1) > par_rec->endFrame )
	{
		level = par_rec->endFrame-1;
		doit=TRUE;
	}
	if ( doit )
	{
		// START - SET SLIDER
		pot = (((ULONG)MAXPOT)*(level))/(numLevels-1);
		NewModifyProp(&PropSlider, window, NULL, AUTOKNOB|FREEHORIZ|PROPBORDERLESS,
									pot,			/* horizPot */ 
									0L,				/* vertPot */
									body,			/* horizBody */
									MAXBODY,	/* vertBody */
									1L);
		RefreshGList(&PropSlider, window, NULL, 1);
		// END - SET SLIDER

		UA_ClearButton(window, &PAR_GR[21], AREA_PEN);
		sprintf(str,"%d",level+1);
		UA_DrawSpecialGadgetText(window, &PAR_GR[21], str, SPECIAL_TEXT_CENTER);
	}
}

/******** E O F ********/
