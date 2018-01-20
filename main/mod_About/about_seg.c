#include "nb:pre.h"
#include "about_seg.h"

#define VERSION	"\0$VER: 2.3"

/**** function declarations ****/

void doYourThing(void);
void MonitorUser(struct Window *window);
void PrintPage(struct Window *window, int page);

/**** globals ****/

static UBYTE *vers = VERSION;
UBYTE **msgs;
struct RendezVousRecord *rvrec;
struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *medialinkLibBase;
struct BitMap logoBM;
struct MsgPort *capsport;

/**** gadgets ****/

struct GadgetRecord about_GR[] =
{
  0,   0, 320, 200, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	319, 199, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
140, 182, 222, 195, 0, NULL, Msg_More,									BUTTON_GADGET, NULL,
230, 182, 312, 195, 0, NULL, Msg_OK,										BUTTON_GADGET, NULL,
  4,  42, 314, 178, 2, NULL, 0,													INVISIBLE_GADGET, NULL,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

struct GadgetRecord about_frames_GR[] =
{
  9,  57, 312,  68, 0, NULL, 0,	  											TEXT_REGION, NULL,
  9,  69, 312,  80, 0, NULL, 0,													TEXT_REGION, NULL,
  9,  81, 312,  92, 0, NULL, 0,													TEXT_REGION, NULL,
  9,  93, 312, 104, 0, NULL, 0,													TEXT_REGION, NULL, 
  9, 105, 312, 116, 0, NULL, 0,													TEXT_REGION, NULL,
  9, 117, 312, 128, 0, NULL, 0,													TEXT_REGION, NULL,
  9, 129, 312, 140, 0, NULL, 0,													TEXT_REGION, NULL,
  9, 141, 312, 152, 0, NULL, 0,													TEXT_REGION, NULL,
  9, 153, 312, 164, 0, NULL, 0,													TEXT_REGION, NULL,
  9, 165, 312, 176, 0, NULL, 0,													TEXT_REGION, NULL,
-1
};

/**** disable CTRL-C break ****/

void CXBRK(void) { }
void chkabort(void) { }

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

	UA_TranslateGR(about_GR,msgs);

	/**** play around ****/

	doYourThing();

	/**** and leave the show ****/

	capsport->mp_SigTask = oldTask;

	exit(0);
}

/******** doYourThing() ********/

void doYourThing(void)
{
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
		return;

	/**** double the dimensions of gadgets etc. if screen is laced ****/

	if ( UA_IsUAScreenLaced(&UAI) )
	{
		UA_DoubleGadgetDimensions(about_GR);
		UA_DoubleGadgetDimensions(about_frames_GR);
	}

	UAI.windowX				= -1;	/* -1 means center on screen */
	UAI.windowY				= -1;	/* -1 means center on screen */
	UAI.windowWidth		= about_GR[0].x2;
	UAI.windowHeight	= about_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;

	/**** set the right font for this window ****/

	UAI.small_TF = rvrec->smallfont;
	UAI.large_TF = rvrec->largefont;

	UA_OpenWindow(&UAI);
	UA_SetMenuColors(rvrec,UAI.userWindow);

	/**** render all gadgets ****/

	InitBitMap(&logoBM, 3, BM_WIDTH, BM_HEIGHT);
	logoBM.Planes[0] = (PLANEPTR)plane1;
	logoBM.Planes[1] = (PLANEPTR)plane2;
	logoBM.Planes[2] = (PLANEPTR)plane3;
	if ( UAI.userWindow->RPort->BitMap->Depth!=1 )
		BltBitMapRastPort(&logoBM,0,0,UAI.userWindow->RPort,13,6,BM_WIDTH,BM_HEIGHT,0xc0);

	UA_DrawGadgetList(UAI.userWindow, about_GR);

	PrintPage(UAI.userWindow, 1);
	
	MonitorUser(UAI.userWindow);

	UA_ResetMenuColors(rvrec,UAI.userWindow);

	/**** close the window ****/

	UA_CloseWindow(&UAI);
}

/******** MonitorUser() ********/

void MonitorUser(struct Window *window)
{
BOOL loop;
int ID, page=2;
struct EventData CED;

	/**** event handler ****/

	loop=TRUE;
	while(loop)
	{
		UA_doStandardWait(window,&CED);

		switch(CED.Class)
		{
			case IDCMP_MOUSEBUTTONS:
				if (CED.Code==SELECTDOWN)
				{
					ID = UA_CheckGadgetList(window, about_GR, &CED);
					switch(ID)
					{
						case 2: /* More */
							UA_HiliteButton(window, &about_GR[ID]);
							PrintPage(window, page);
							page++;
							if (page>4)
								page=1;
							break;

						case 3:	/* OK */
							UA_HiliteButton(window, &about_GR[ID]);
							loop=FALSE;
							break;
					}
				}
				break;

			case IDCMP_RAWKEY:
				if ( CED.Code==RAW_RETURN )
				{
					UA_HiliteButton(window, &about_GR[3]);
					loop=FALSE;
				}
				break;
		}
	}
}

/******** PrintPage() ********/

void PrintPage(struct Window *window, int page)
{
double mem;
TEXT buf[100];
struct Library *lib;
int vers, rev;

	UA_ClearButton(window, &about_GR[4], AREA_PEN);

	if (page==1)
	{
		UA_DrawSpecialGadgetText(window, &about_frames_GR[4], msgs[Msg_About_a1-1], SPECIAL_TEXT_LEFT);
		UA_DrawSpecialGadgetText(window, &about_frames_GR[5], msgs[Msg_About_a2-1], SPECIAL_TEXT_LEFT);
		UA_DrawSpecialGadgetText(window, &about_frames_GR[6], msgs[Msg_About_a3-1], SPECIAL_TEXT_LEFT);
		UA_DrawSpecialGadgetText(window, &about_frames_GR[7], msgs[Msg_About_a4-1], SPECIAL_TEXT_LEFT);
		UA_DrawSpecialGadgetText(window, &about_frames_GR[8], msgs[Msg_About_a5-1], SPECIAL_TEXT_LEFT);
		UA_DrawSpecialGadgetText(window, &about_frames_GR[9], msgs[Msg_About_a6-1], SPECIAL_TEXT_LEFT);
	}
	else if (page==2)
	{
		UA_DrawSpecialGadgetText(window, &about_frames_GR[0], msgs[Msg_About_b1-1], SPECIAL_TEXT_CENTER);
		UA_DrawSpecialGadgetText(window, &about_frames_GR[2], msgs[Msg_About_b2-1], SPECIAL_TEXT_CENTER);
		UA_DrawSpecialGadgetText(window, &about_frames_GR[3], msgs[Msg_About_b3-1], SPECIAL_TEXT_CENTER);
		UA_DrawSpecialGadgetText(window, &about_frames_GR[4], msgs[Msg_About_b4-1], SPECIAL_TEXT_CENTER);
		UA_DrawSpecialGadgetText(window, &about_frames_GR[5], msgs[Msg_About_b5-1], SPECIAL_TEXT_CENTER);
		UA_DrawSpecialGadgetText(window, &about_frames_GR[6], msgs[Msg_About_b6-1], SPECIAL_TEXT_CENTER);
		UA_DrawSpecialGadgetText(window, &about_frames_GR[7], msgs[Msg_About_b7-1], SPECIAL_TEXT_CENTER);
		UA_DrawSpecialGadgetText(window, &about_frames_GR[8], msgs[Msg_About_b8-1], SPECIAL_TEXT_CENTER);
		UA_DrawSpecialGadgetText(window, &about_frames_GR[9], msgs[Msg_About_b9-1], SPECIAL_TEXT_CENTER);
	}
	else if (page==3)
	{
		UA_DrawSpecialGadgetText(window, &about_frames_GR[0], msgs[Msg_Translation-1], SPECIAL_TEXT_CENTER);
		UA_DrawSpecialGadgetText(window, &about_frames_GR[2], msgs[Msg_Translator-1], SPECIAL_TEXT_CENTER);
		UA_DrawSpecialGadgetText(window, &about_frames_GR[4], msgs[Msg_AboutSupport-1], SPECIAL_TEXT_CENTER);
		UA_DrawSpecialGadgetText(window, &about_frames_GR[6], msgs[Msg_AboutSup1-1], SPECIAL_TEXT_CENTER);
		UA_DrawSpecialGadgetText(window, &about_frames_GR[7], msgs[Msg_AboutSup2-1], SPECIAL_TEXT_CENTER);
	}
	else if (page==4)
	{
		mem = ((double)AvailMem(MEMF_CHIP))/1024000.0;
		sprintf(buf, "%s %3.2f %s", msgs[Msg_About_c1-1], mem, msgs[Msg_About_c6-1]);
		UA_DrawSpecialGadgetText(window, &about_frames_GR[0], buf, SPECIAL_TEXT_LEFT);

		mem = ((double)AvailMem(MEMF_FAST))/1024000.0;
		sprintf(buf, "%s %3.2f %s", msgs[Msg_About_c2-1], mem, msgs[Msg_About_c6-1]);
		UA_DrawSpecialGadgetText(window, &about_frames_GR[1], buf, SPECIAL_TEXT_LEFT);

		sprintf(buf, "%s %s", msgs[Msg_About_c3-1], msgs[Msg_BetaNr-1]);
		UA_DrawSpecialGadgetText(window, &about_frames_GR[3], buf, SPECIAL_TEXT_LEFT);

		sprintf(buf, "%s %d.%d", msgs[Msg_About_c4-1], medialinkLibBase->lib_Version, medialinkLibBase->lib_Revision);
		UA_DrawSpecialGadgetText(window, &about_frames_GR[4], buf, SPECIAL_TEXT_LEFT);

		vers=0;
		rev=0;
		lib = (struct Library *)OpenLibrary("mpmmu.library",0L);
		if (lib)
		{
			vers = lib->lib_Version;
			rev = lib->lib_Revision;
			CloseLibrary(lib);
		}
		sprintf(buf, "%s %d.%d", msgs[Msg_About_c5-1], vers, rev);
		UA_DrawSpecialGadgetText(window, &about_frames_GR[5], buf, SPECIAL_TEXT_LEFT);
	}
}

/******** E O F ********/
