#include "nb:pre.h"
#include "setup.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"
#include "structs.h"

#define VERSI0N "\0$VER: 1.1"
static UBYTE *vers = VERSI0N;

/**** internal function declarations ****/

BOOL MonitorUser(	struct Window *window, PROCESSINFO *ThisPI,
									struct Genlock_record *gl_rec,
									struct UserApplicInfo *UAI);
void SetGLButtons(struct Window *window, struct BitMap *clipBM,
									struct Genlock_record *gl_rec, WORD sx[], WORD sy, WORD sh);

/**** external function declarations ****/

extern void GetVarsFromPI(struct Genlock_record *gl_rec, PROCESSINFO *ThisPI);
extern void PutVarsToPI(struct Genlock_record *gl_rec, PROCESSINFO *ThisPI);

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
struct Genlock_record gl_rec;
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
		UA_DoubleGadgetDimensions(GL_GR);

	/*** translate the GUI ****/

	UA_TranslateGR(GL_GR, msgs);

	/**** open the window ****/

	capsport = (struct MsgPort *)FindPort(MEDIALINKPORT);
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	UAI.windowX				= -1;	/* -1 means center on screen */
	UAI.windowY				= -1;	/* -1 means center on screen */
	UAI.windowWidth		= GL_GR[0].x2;
	UAI.windowHeight	= GL_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(&UAI);

	gl_rec.window = UAI.userWindow;

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, GL_GR);

	/**** monitor events ****/

	MonitorUser(UAI.userWindow, ThisPI, &gl_rec, &UAI);

	/**** close the window ****/

	UA_CloseWindow(&UAI);

	capsport->mp_SigTask = oldTask;
}

/******** MonitorUser() ********/

BOOL MonitorUser(	struct Window *window, PROCESSINFO *ThisPI,
									struct Genlock_record *gl_rec, struct UserApplicInfo *UAI)
{
BOOL loop=TRUE, retVal;
struct EventData CED;
int ID,i;
UWORD *mypattern1, sx[5], sy, sh;
struct BitMap clipBM;

	/**** do chip mem releated stuff ****/

	InitBitMap(&clipBM, 3, CLIP_W, CLIP_H);
	for(i=0; i<3; i++)
		clipBM.Planes[i] = NULL;
	for(i=0; i<3; i++)
	{
		clipBM.Planes[i] =  (PLANEPTR)AllocRaster(CLIP_W,CLIP_H);
		if ( !clipBM.Planes[i] )
		{
			for(i=0; i<3; i++)
				if ( clipBM.Planes[i] )
					FreeRaster(clipBM.Planes[i],CLIP_W,CLIP_H);
			return(FALSE);
		}
	}
	CopyMem(plane0, clipBM.Planes[0], PLANESIZE);
	CopyMem(plane1, clipBM.Planes[1], PLANESIZE);
	CopyMem(plane2, clipBM.Planes[2], PLANESIZE);

	mypattern1 = (UWORD *)AllocMem(4L, MEMF_CHIP);
	if (mypattern1==NULL)
		return(FALSE);
	mypattern1[0] = 0x5555;
	mypattern1[1] = 0xaaaa;

	/**** parse string ****/

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
		GetVarsFromPI(gl_rec, ThisPI);
	else
	{
		gl_rec->mode = 2;
		gl_rec->alwaysLaced = TRUE;
	}

	/**** set buttons ****/

	sx[0] = 0;
	sx[1] = 61;
	sx[2] = 122;
	sx[3] = 183;
	sx[4] = 244;

	if (!UA_IsUAScreenLaced(UAI))
	{
		sy = 0;
		sh = 24;
	}
	else
	{
		sy = 23;
		sh = 48;
	}

	SetGLButtons(window, &clipBM, gl_rec, sx, sy, sh);

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window, &CED);
		if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, GL_GR, &CED);
			switch(ID)
			{
				case 3:	// OK
do_ok:
					UA_HiliteButton(window, &GL_GR[3]);
					retVal=TRUE;
					loop=FALSE;
					break;

				case 4:	// Cancel
do_cancel:
					UA_HiliteButton(window, &GL_GR[4]);
					retVal=FALSE;
					loop=FALSE;
					break;

				case 5:	// Preview
					UA_InvertButton(window, &GL_GR[ID]);
					UA_SetSprite(window, SPRITE_BUSY);
					PerformActions(gl_rec,window,NULL);
					UA_SetSprite(window, SPRITE_NORMAL);
					UA_InvertButton(window, &GL_GR[ID]);
					break;

				case 9:	// Mode
					UA_ProcessCycleGadget(window, &GL_GR[ID], &CED);
					UA_SetValToCycleGadgetVal(&GL_GR[ID], &gl_rec->mode);
					SetGLButtons(window, &clipBM, gl_rec, sx, sy, sh);
					break;

				case 15:	// Always laced
					if ( gl_rec->alwaysLaced )
						gl_rec->alwaysLaced = FALSE;
					else
						gl_rec->alwaysLaced = TRUE;
					SetGLButtons(window, &clipBM, gl_rec, sx, sy, sh);
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

	// Clean up

	for(i=0; i<3; i++)
		if ( clipBM.Planes[i] )
			FreeRaster(clipBM.Planes[i],CLIP_W,CLIP_H);

	FreeMem(mypattern1, 4L);

	// Store settings

	if ( retVal )
		PutVarsToPI(gl_rec, ThisPI);

	// Get out

	return(retVal);
}

/******* SetGLButtons() ********/

void SetGLButtons(struct Window *window, struct BitMap *clipBM,
									struct Genlock_record *gl_rec, WORD sx[], WORD sy, WORD sh)
{
	// mode cycle

	UA_SetCycleGadgetToVal(window, &GL_GR[9], gl_rec->mode);

	// mode image

	UA_DrawGadget(window, &GL_GR[10]);
	BltBitMapRastPort(clipBM,
										sx[gl_rec->mode], sy,
										window->RPort,
										GL_GR[16].x1, GL_GR[16].y1,
										GL_GR[16].x2-GL_GR[16].x1+1, sh, 0xc0);

	// always laced

	UA_ClearButton(window, &GL_GR[15], AREA_PEN);
	if ( gl_rec->alwaysLaced )
		UA_InvertButton(window, &GL_GR[15]);
}

/******** E O F ********/
