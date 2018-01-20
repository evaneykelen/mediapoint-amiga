/*****************************************************/
/*
 * Captain's log:
 *
 * Star date: Sunday 26-Sep-93
 *						Send this file to Chris.
 *
 *						Monday 27-Sep-93 00:48:56 
 *						Realized I should keep a log of changes I make to this file
 *						as Chris and I probably work on the same files.
 *
 *            Monday 27-Sep-93 00:51:46   
 *						Modified the event handler. It now uses UA_doStandardWait().
 *						Removed some fat associated with this change.
 *
 *						Saturday 02-Oct-93 15:42:59
 *						Modified setup.c, brings MP screen to front after doing a preview.
 *						Modiefied toaster.c (used Chris' version, adapted few things).
 *						
 */

#include "nb:pre.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "gen:support_protos.h"
#include "structs.h"
#include "setup.h"

#define TR_W	528
#define TR_H	42

/**** external functions ****/

extern void GetVarsFromPI(struct Toaster_record *trec, PROCESSINFO *ThisPI);
extern void PutVarsToPI(struct Toaster_record *trec, PROCESSINFO *ThisPI);
extern BOOL PerformActions(struct Toaster_record *trec, STRPTR errorStr);

/**** function declarations ****/

BOOL MonitorUser(	struct Window *window, PROCESSINFO *ThisPI,
									struct UserApplicInfo *UAI );
void RenderToasterPage(	int page, struct Window *window,
												struct Toaster_record *trec, UWORD *mypattern1 );
void CheckPage1(struct Window *window, struct EventData *CED,
								struct Toaster_record *trec, UWORD *mypattern1);
void CheckPage2(struct Window *window, struct EventData *CED,
								struct Toaster_record *trec, UWORD *mypattern1);
void CheckPage3(struct Window *window, struct EventData *CED,
								struct Toaster_record *trec, UWORD *mypattern1);

/**** globals ****/

struct Library *medialinkLibBase		= NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase							= NULL;

UBYTE **msgs;
struct RendezVousRecord *rvrec;
struct MsgPort *capsport;
struct BitMap bm;

struct View *oldview;

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

	port = (struct MsgPort *)FindPort("MP rendez-vous");
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

	capsport = (struct MsgPort *)FindPort("MediaPointPort");
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	if (UA_HostScreenPresent(&UAI))
		UAI.windowModes = 1;	/* open on the MediaLink screen */
	else
		UAI.windowModes = 3;	/* open on the first (frontmost) screen */

	/**** double the dimensions of gadgets etc. if screen is laced ****/

	if (UA_IsUAScreenLaced(&UAI))
	{
		UA_DoubleGadgetDimensions(Toaster_1_GR);
		UA_DoubleGadgetDimensions(Toaster_2_GR);
		UA_DoubleGadgetDimensions(Toaster_3_GR);
		UA_DoubleGadgetDimensions(Toaster_4_GR);
		UA_DoubleGadgetDimensions(Toaster_5_GR);
		UA_DoubleGadgetDimensions(Toaster_6_GR);
	}

	/**** open the window ****/

	UAI.windowX				= -1;
	UAI.windowY				= -1;
	UAI.windowWidth		= Toaster_1_GR[0].x2;
	UAI.windowHeight	= Toaster_1_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(&UAI);

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, Toaster_1_GR);

	/**** monitor events ****/

	MonitorUser(UAI.userWindow, ThisPI, &UAI);

	/**** close the window ****/

	UA_CloseWindow(&UAI);

	capsport->mp_SigTask = oldTask;
}

/******** MonitorUser() ********/

BOOL MonitorUser(	struct Window *window, PROCESSINFO *ThisPI,
									struct UserApplicInfo *UAI)
{
BOOL loop=TRUE, retVal;
struct EventData CED;
int ID;
UWORD *mypattern1;
struct Toaster_record trec;
TEXT errorStr[100];

	/**** init vars, alloc memory ****/

	mypattern1 = (UWORD *)AllocMem(4L, MEMF_CHIP);
	if (mypattern1==NULL)
		return(FALSE);
	mypattern1[0] = 0x5555;
	mypattern1[1] = 0xaaaa;

	InitBitMap(&bm,2,TR_W,TR_H);
	bm.Planes[0] = (PLANEPTR)AllocRaster(TR_W, TR_H);
	bm.Planes[1] = (PLANEPTR)AllocRaster(TR_W, TR_H);

	CopyMem(plane0, bm.Planes[0], sizeof(plane0));
	CopyMem(plane1, bm.Planes[1], sizeof(plane1));

	/**** parse string ****/

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
		GetVarsFromPI(&trec, ThisPI);
	else
	{
		trec.cmd = 0;							// start with transition page
		trec.previewSource = 5;		// 1 (1,2,3,4,DV1,DV2,DV3)
		trec.transitionBank	= 0;	// own set (own,A,B,C,D,E,F,G,H,I)
		trec.transitionSpeed = 2;	// fast (slow,med,fast)
		trec.transitionCol = 0;		// 
		trec.transitionRow = 0;		// 
		trec.transitionOwn = 0;		// -1 if col/row is used

		trec.from	= 0;						// DV1 (DV1,DV2)
		trec.saveFrameStore[0] = '\0';
		trec.FS_Number = 1;				// frame.001

		trec.loadFrameStore[0] = '\0';
		trec.Into_Number = 1;			// frame.001

		trec.frameStorePath[0] = '\0';
	}

	/**** set buttons ****/

	UA_SetCycleGadgetToVal(window, &Toaster_1_GR[7], trec.cmd);

	RenderToasterPage( trec.cmd, window, &trec, mypattern1 );

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);
		if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			if ( trec.cmd==0 )
				CheckPage1(window,&CED,&trec,mypattern1);
			else if ( trec.cmd==1 )
				CheckPage2(window,&CED,&trec,mypattern1);
			else if ( trec.cmd==2 )
				CheckPage3(window,&CED,&trec,mypattern1);

			ID = UA_CheckGadgetList(window, Toaster_1_GR, &CED);
			switch(ID)
			{
				case 4:		// OK
do_ok:
					UA_HiliteButton(window, &Toaster_1_GR[4]);
					retVal=TRUE;
					loop=FALSE;
					break;

				case 5:		// Preview
					UA_InvertButton(window, &Toaster_1_GR[ID]);
					UA_SetSprite(window, SPRITE_BUSY);

oldview = GfxBase->ActiView;
WBenchToFront();

					if ( !PerformActions(&trec,errorStr) )
						GiveMessage(window, errorStr);

GfxBase->ActiView = oldview;
ScreenToFront(window->WScreen);
MakeScreen(window->WScreen);
RethinkDisplay();

					UA_SetSprite(window, SPRITE_NORMAL);
					Delay(4L);
					UA_InvertButton(window, &Toaster_1_GR[ID]);
					break;

				case 6:		// Cancel
do_cancel:
					UA_HiliteButton(window, &Toaster_1_GR[6]);
					retVal=FALSE;
					loop=FALSE;
					break;

				case 7:	// cmd
					UA_ProcessCycleGadget(window, &Toaster_1_GR[ID], &CED);
					UA_SetValToCycleGadgetVal(&Toaster_1_GR[ID], &trec.cmd);
					RenderToasterPage( trec.cmd, window, &trec, mypattern1 );
					break;
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE)				// Cancel
				goto do_cancel;
			else if (CED.Code==RAW_RETURN)	// OK
				goto do_ok;
		}
	}

	FreeRaster(bm.Planes[0], TR_W, TR_H);
	FreeRaster(bm.Planes[1], TR_W, TR_H);
	FreeMem(mypattern1, 4L);

	if ( retVal )
		PutVarsToPI(&trec, ThisPI);

	return(retVal);
}

/******** RenderToasterPage() ********/

void RenderToasterPage(	int page, struct Window *window,
												struct Toaster_record *trec, UWORD *mypattern1 )
{
TEXT str[256];
int i;

	UA_ClearButton(window, &Toaster_1_GR[8], AREA_PEN);

	switch( trec->cmd )
	{
		case 0:	// transitions page
			UA_EnableButton(window, &Toaster_1_GR[5]);
			UA_DrawGadgetList(window, Toaster_2_GR);
			UA_SetCycleGadgetToVal(window, &Toaster_2_GR[0], trec->previewSource);			
			UA_SetCycleGadgetToVal(window, &Toaster_2_GR[1], trec->transitionBank);			
			UA_SetCycleGadgetToVal(window, &Toaster_2_GR[2], trec->transitionSpeed);			
			if ( trec->transitionBank > 0 )	// small transition boxes
			{
				UA_DrawGadgetList(window, Toaster_3_GR);
				UA_InvertButton(window, &Toaster_3_GR[ trec->transitionRow*8+trec->transitionCol ]);
				for(i=0; i<8; i++)
				{
					sprintf(str,"œœœœ%d",i+1);	// micro spaces (special to MP font)
					UA_DrawSpecialGadgetText(window, &Toaster_3_GR[i], str, SPECIAL_TEXT_TOP);
				}
				for(i=0; i<4; i++)
				{
					sprintf(str,"%d",i+1);
					UA_DrawSpecialGadgetText(window, &Toaster_3_GR[i*8], str, SPECIAL_TEXT_BEFORE_STRING);
				}
			}
			else
			{
				UA_DrawGadgetList(window, Toaster_4_GR);
				BltBitMapRastPort(&bm,0,0,window->RPort,
											Toaster_4_GR[0].x1,Toaster_4_GR[0].y1,TR_W,TR_H,0xc0);
				UA_InvertButton(window, &Toaster_4_GR[ trec->transitionOwn ]);
			}
			break;			

		case 1:	// load page
			UA_DisableButton(window, &Toaster_1_GR[5], mypattern1);
			UA_DrawGadgetList(window, Toaster_6_GR);
			if ( trec->loadFrameStore[0] )
			{
				strcpy(str, trec->loadFrameStore);
				UA_ShortenStringFront(window->RPort, str, (Toaster_6_GR[0].x2-Toaster_6_GR[0].x1)-16);
				UA_ClearButton(window, &Toaster_6_GR[0], AREA_PEN);
				UA_DrawSpecialGadgetText(window, &Toaster_6_GR[0], str, SPECIAL_TEXT_CENTER);
			}
			//UA_SetCycleGadgetToVal(window, &Toaster_6_GR[0], trec->from);		
			//UA_SetStringGadgetToVal(window, &Toaster_6_GR[2], trec->Into_Number);
			break;

		case 2:	// save page
			UA_DisableButton(window, &Toaster_1_GR[5], mypattern1);
			UA_DrawGadgetList(window, Toaster_5_GR);
			if ( trec->saveFrameStore[0] )
			{
				strcpy(str, trec->saveFrameStore);
				UA_ShortenStringFront(window->RPort, str, (Toaster_5_GR[1].x2-Toaster_5_GR[1].x1)-16);
				UA_ClearButton(window, &Toaster_5_GR[1], AREA_PEN);
				UA_DrawSpecialGadgetText(window, &Toaster_5_GR[1], str, SPECIAL_TEXT_CENTER);
			}
			UA_SetCycleGadgetToVal(window, &Toaster_5_GR[0], trec->from);
			//UA_SetStringGadgetToVal(window, &Toaster_5_GR[3], trec->FS_Number);
			break;
	}
}

/******** CheckPage1() ********/

void CheckPage1(struct Window *window, struct EventData *CED,
								struct Toaster_record *trec, UWORD *mypattern1)
{
int ID,bank;

	ID = UA_CheckGadgetList(window, Toaster_2_GR, CED);
	switch(ID)
	{
		case 0:	// Preview Source selector
			UA_ProcessCycleGadget(window, &Toaster_2_GR[ID], CED);
			UA_SetValToCycleGadgetVal(&Toaster_2_GR[ID], &trec->previewSource);
			break;

		case 1:	// Transition Bank Selector
			UA_ProcessCycleGadget(window, &Toaster_2_GR[ID], CED);
			bank = trec->transitionBank;
			UA_SetValToCycleGadgetVal(&Toaster_2_GR[ID], &trec->transitionBank);
			if ( bank==0 || trec->transitionBank==0 )	// time for a refreshment
				RenderToasterPage(trec->cmd,window,trec,mypattern1);
			break;

		case 2:	// Transition Speed selector
			UA_ProcessCycleGadget(window, &Toaster_2_GR[ID], CED);
			UA_SetValToCycleGadgetVal(&Toaster_2_GR[ID], &trec->transitionSpeed);
			break;
	}

	if ( trec->transitionBank > 0 )	// small transitions
	{
		ID = UA_CheckGadgetList(window, Toaster_3_GR, CED);
		if ( ID != -1 )
		{
			UA_InvertButton(window, &Toaster_3_GR[ trec->transitionRow*8+trec->transitionCol ]);
			trec->transitionCol = ID % 8;
			trec->transitionRow = ID / 8;
			UA_InvertButton(window, &Toaster_3_GR[ trec->transitionRow*8+trec->transitionCol ]);
		}
	}
	else
	{
		ID = UA_CheckGadgetList(window, Toaster_4_GR, CED);
		if ( ID != -1 )
		{
			UA_InvertButton(window, &Toaster_4_GR[ trec->transitionOwn ]);
			trec->transitionOwn = ID;
			UA_InvertButton(window, &Toaster_4_GR[ trec->transitionOwn ]);
		}
	}
}

/******** CheckPage2() ********/
/*
 * Load framestore
 *
 */

void CheckPage2(struct Window *window, struct EventData *CED,
								struct Toaster_record *trec, UWORD *mypattern1)
{
int ID;
struct FileReqRecord FRR;
TEXT path[SIZE_FULLPATH], filename[SIZE_FILENAME], errorStr[100];

	ID = UA_CheckGadgetList(window, Toaster_6_GR, CED);
	switch(ID)
	{
/*
		case 0:	// From selector
			UA_ProcessCycleGadget(window, &Toaster_6_GR[ID], CED);
			UA_SetValToCycleGadgetVal(&Toaster_6_GR[ID], &trec->from);
			break;
*/

		case 1:	// Load
			UA_InvertButton(window, &Toaster_6_GR[ID]);

			/**** set up FRR (File Request Record) ****/

			if ( trec->frameStorePath[0] )
				strcpy(path, trec->frameStorePath);
			else
				strcpy(path, rvrec->capsprefs->import_picture_Path);
			filename[0] = '\0';

			FRR.path							= path;
			FRR.fileName					= filename;
			FRR.title							= (UBYTE *)"Select a Toaster frame:";
			FRR.opts							= DIR_OPT_ALL;	// Chris: tell me if toaster frames
			FRR.multiple					= FALSE;				// are FORM/ILBM, there's a filter for such files.

			/**** pop up the file requester ****/

			if ( UA_OpenAFile(window, &FRR, mypattern1) )
			{
				strcpy(trec->frameStorePath, path);	//UA_MakeFullPath(path, filename, trec->frameStorePath);
				strcpy(trec->loadFrameStore, filename);
				/**** put path+file names on screen ****/
				strcpy(path,filename);
				UA_ShortenStringFront(window->RPort, path, (Toaster_6_GR[0].x2-Toaster_6_GR[0].x1)-16);
				UA_ClearButton(window, &Toaster_6_GR[0], AREA_PEN);
				UA_DrawSpecialGadgetText(window, &Toaster_6_GR[0], path, SPECIAL_TEXT_CENTER);

				/**** use ARexx send stuff to actually load the frame ****/

oldview = GfxBase->ActiView;
WBenchToFront();

				if ( !PerformActions(trec,errorStr) )
					GiveMessage(window, errorStr);

GfxBase->ActiView = oldview;
ScreenToFront(window->WScreen);
MakeScreen(window->WScreen);
RethinkDisplay();
			}

			UA_InvertButton(window, &Toaster_6_GR[ID]);
			break;

#if 0
		case 2:	// 'Into' framestore integer gadget
			UA_ProcessStringGadget(window, Toaster_6_GR, &Toaster_6_GR[ID], CED);
			UA_SetValToStringGadgetVal(&Toaster_6_GR[ID], &trec->Into_Number);
			break;
#endif
	}
}

/******** CheckPage3() ********/
/*
 * Save framestore
 *
 */

void CheckPage3(struct Window *window, struct EventData *CED,
								struct Toaster_record *trec, UWORD *mypattern1)
{
int ID,len;
struct FileReqRecord FRR;
TEXT path[SIZE_FULLPATH], filename[SIZE_FILENAME], index[5], errorStr[100];

	ID = UA_CheckGadgetList(window, Toaster_5_GR, CED);
	switch(ID)
	{
		case 0:	// From selector
			UA_ProcessCycleGadget(window, &Toaster_5_GR[ID], CED);
			UA_SetValToCycleGadgetVal(&Toaster_5_GR[ID], &trec->from);
			break;

		case 2:	// Save
			UA_InvertButton(window, &Toaster_5_GR[ID]);

			/**** set up FRR (File Request Record) ****/

			if ( trec->saveFrameStore[0] )
			{
				strcpy(path, trec->frameStorePath);
				strcpy(filename, trec->saveFrameStore);
			}
			else
			{
				strcpy(path, rvrec->capsprefs->import_picture_Path);
				filename[0] = '\0';
			}

#if 0
			len = strlen(filename);
			if ( len > 4 )
			{
				if (	filename[ len-4 ] == '.' &&
							isdigit( filename[ len-3 ] ) &&
							isdigit( filename[ len-2 ] ) &&
							isdigit( filename[ len-1 ] ) )
				{
					filename[ len-4 ] = '\0';	// chop index off
					sprintf(index, ".%03d", trec->FS_Number);
					strcat(filename,index);
				}
			}
			else
				sprintf(filename, "frame.%03d", trec->FS_Number);	// default name
#endif

			FRR.path							= path;
			FRR.fileName					= filename;
			FRR.title							= (UBYTE *)"Save this frame as:";
			FRR.opts							= DIR_OPT_ALL;	// Chris: tell me if toaster frames
			FRR.multiple					= FALSE;				// are FORM/ILBM, there's a filter for such files.

			/**** pop up the file requester ****/

			if ( UA_SaveAFile(window, &FRR, mypattern1) )
			{
				strcpy(trec->frameStorePath, path);	//UA_MakeFullPath(path, filename, trec->frameStorePath);
				strcpy(trec->saveFrameStore,filename);
				/**** put path+file names on screen ****/
				strcpy(path,filename);
				UA_ShortenStringFront(window->RPort, path, (Toaster_5_GR[1].x2-Toaster_5_GR[1].x1)-16);
				UA_ClearButton(window, &Toaster_5_GR[1], AREA_PEN);
				UA_DrawSpecialGadgetText(window, &Toaster_5_GR[1], path, SPECIAL_TEXT_CENTER);

				/**** use ARexx send stuff to actually save the frame ****/

oldview = GfxBase->ActiView;
WBenchToFront();

				if ( !PerformActions(trec,errorStr) )
					GiveMessage(window, errorStr);

GfxBase->ActiView = oldview;
ScreenToFront(window->WScreen);
MakeScreen(window->WScreen);
RethinkDisplay();
			}

			UA_InvertButton(window, &Toaster_5_GR[ID]);
			break;

#if 0
		case 3:	// framestore number integer gadget
			UA_ProcessStringGadget(window, Toaster_5_GR, &Toaster_5_GR[ID], CED);
			UA_SetValToStringGadgetVal(&Toaster_5_GR[ID], &trec->FS_Number);
			break;
#endif
	}
}

/******** E O F ********/
