//	File		:	setup.c
//	Uses		:	pre.h minc:types.h Errors.h process.h
//	Date		:	-92 29-04-93
//	Author	:	E. van Eykelen
//	Desc.		:	setup the Resource GUI
//

#include "nb:pre.h"
#include "demo:gen/support_protos.h"
#include "setup.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include <stat.h>	// used for stat() to get file size
#include "lightpen.h"

#define VERSI0N "\0$VER: 1.0"
static UBYTE *vers = VERSI0N;

/**** internal function declarations ****/

void XappSetup(PROCESSINFO *ThisPI);
BOOL MonitorUser(struct Window *window, PROCESSINFO *ThisPI);
void GetExtraData(PROCESSINFO *ThisPI, struct lp *lp );
void PutExtraData(PROCESSINFO *ThisPI, struct lp *lp );
int GetFileSize(STRPTR path);
void MakeNiceNumber(char *in, char *out);

/**** globals ****/

struct Library *medialinkLibBase		= NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase							= NULL;

UBYTE **msgs;
struct RendezVousRecord *rvrec;
struct MsgPort *capsport;

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

	if (UA_HostScreenPresent(&UAI))
		UAI.windowModes = 1;	/* open on the MediaLink screen */
	else
		UAI.windowModes = 3;	/* open on the first (frontmost) screen */

	/**** double the dimensions of gadgets etc. if screen is laced ****/

	if (UA_IsUAScreenLaced(&UAI))
		UA_DoubleGadgetDimensions(Light_GR);

	UA_TranslateGR(Light_GR, msgs);

	capsport = (struct MsgPort *)FindPort(MEDIALINKPORT);
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	UAI.windowX				= -1;	/* -1 means center on screen */
	UAI.windowY				= -1;	/* -1 means center on screen */
	UAI.windowWidth		= Light_GR[0].x2;
	UAI.windowHeight	= Light_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(&UAI);

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, Light_GR);
	Light_GR[5].type=BUTTON_GADGET;

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
int ID;
long signal,SigLp;

UWORD *mypattern1;
int x,y;
struct lp lp;

	lp.offset_x = 30;
	lp.offset_y = 0;

	mypattern1 = (UWORD *)AllocMem(4L, MEMF_CHIP);
	if (mypattern1==NULL)
		return(FALSE);
	mypattern1[0] = 0x5555;
	mypattern1[1] = 0xaaaa;

	/**** get arguments from extraData ****/

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
	{
		GetExtraData( ThisPI, &lp );
	}

	UA_SetStringGadgetToVal(window, &Light_GR[10], lp.offset_x );
	UA_SetStringGadgetToVal(window, &Light_GR[11], lp.offset_y );

	/**** event handler ****/

	if( init_mouse( &lp ) )
	{
		setlightpen(&lp);
		SigLp = lp.signal;

		while(loop)
		{
			signal = UA_doStandardWaitExtra(window,&CED, SigLp );

			if( signal & SigLp )			// a lightpen movement ??
			{
				set_mouse( &lp );
			}
			else
			if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
			{
				ID = UA_CheckGadgetList(window, Light_GR, &CED);
				switch(ID)
				{
					case 10:
						UA_ProcessStringGadget(window,Light_GR,&Light_GR[ID],&CED);
						UA_SetValToStringGadgetVal(&Light_GR[ID], &x);
						if ( x<-100 || x>100 )
						{
							GiveMessage(window, msgs[Msg_X_4-1], -100, 100); // "Enter a value between...
							UA_SetStringGadgetToVal(window, &Light_GR[10], lp.offset_x );
						}
						else
							lp.offset_x = (WORD)x;
						break;

					case 11:
						UA_ProcessStringGadget(window,Light_GR,&Light_GR[ID],&CED);
						UA_SetValToStringGadgetVal(&Light_GR[ID], &y);
						if ( y<-100 || y>100 )
						{
							GiveMessage(window, msgs[Msg_X_4-1], -100, 100); // "Enter a value between...
							UA_SetStringGadgetToVal(window, &Light_GR[11], lp.offset_y );
						}
						else
							lp.offset_y = (WORD)y;
						break;

					case 12:	// OK
do_ok:
						UA_HiliteButton(window, &Light_GR[12]);
						loop=FALSE;
						retVal=TRUE; 
						break;

					case 13:	// Cancel
do_cancel:
						UA_HiliteButton(window, &Light_GR[13]);
						loop=FALSE;
						retVal=FALSE; 
						break;

				}
			}
			else if (CED.Class==IDCMP_RAWKEY)
			{
				switch( CED.Code )
				{
					case RAW_ESCAPE:
						goto do_cancel;
					case RAW_RETURN:
						goto do_ok;
					case RAW_CURSORUP:
						if ( lp.offset_y < 100 )
							lp.offset_y++;
						break;
					case RAW_CURSORDOWN:
						if ( lp.offset_y > -100 )
							lp.offset_y--;
						break;
					case RAW_CURSORRIGHT:
						if ( lp.offset_x > -100 )
							lp.offset_x--;
						break;
					case RAW_CURSORLEFT:
						if ( lp.offset_x < 100 )
							lp.offset_x++;
						break;
				}
				if( x != lp.offset_x )
				{
					UA_SetStringGadgetToVal(window, &Light_GR[10], lp.offset_x );
					set_mouse( &lp );
				}
				if( y != lp.offset_y )
				{
					UA_SetStringGadgetToVal(window, &Light_GR[11], lp.offset_y );
					set_mouse( &lp );
				}
				x = lp.offset_x;
				y = lp.offset_y;
			}
		}
		removelightpen(&lp);
		free_mouse( &lp );
	}

	FreeMem(mypattern1, 4L);

	if ( retVal )
		PutExtraData(ThisPI, &lp );

	ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[0] = 2;

	return(retVal);
}

/******** GetExtraData() ********/

void GetExtraData(PROCESSINFO *ThisPI, struct lp *lp)
{
int x,y;

	sscanf(	ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"%d %d", &x, &y );
	lp->offset_x = x;
	lp->offset_y = y;
}

/******** PutExtraData() ********/

void PutExtraData(PROCESSINFO *ThisPI, struct lp *lp )
{
	sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData,
					"%d %d",
					lp->offset_x,
					lp->offset_y );
}

/******** E O F ********/
