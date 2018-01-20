#include "nb:pre.h"

#define VERSION	"\0$VER: 1.1"
#define NUMSTRGADS	16

/**** function declarations ****/

void doYourThing(void);
void MonitorUser(struct Window *window);

/**** globals ****/

static UBYTE *vers = VERSION;
UBYTE **msgs;
struct RendezVousRecord *rvrec;
struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *medialinkLibBase;
struct MsgPort *capsport;
UWORD chip mypattern1[] = { 0x5555, 0xaaaa };

/**** gadgets ****/

struct StringRecord bgm_01_SR = { 3, "    " };
struct StringRecord bgm_02_SR = { 3, "    " };
struct StringRecord bgm_03_SR = { 3, "    " };
struct StringRecord bgm_04_SR = { 3, "    " };
struct StringRecord bgm_05_SR = { 3, "    " };
struct StringRecord bgm_06_SR = { 3, "    " };
struct StringRecord bgm_07_SR = { 3, "    " };
struct StringRecord bgm_08_SR = { 3, "    " };
struct StringRecord bgm_09_SR = { 3, "    " };
struct StringRecord bgm_10_SR = { 3, "    " };
struct StringRecord bgm_11_SR = { 3, "    " };
struct StringRecord bgm_12_SR = { 3, "    " };
struct StringRecord bgm_13_SR = { 3, "    " };
struct StringRecord bgm_14_SR = { 3, "    " };
struct StringRecord bgm_15_SR = { 3, "    " };
struct StringRecord bgm_16_SR = { 3, "    " };

struct StringRecord bgm_realmax_SR = { 3, "    " };

struct GadgetRecord bgm_GR[] =
{
  0,   0, 320, 142, 0, NULL, 0,													DIMENSIONS, NULL,
  0,	 0,	319, 141, 0, NULL, 0,													DBL_BORDER_REGION, NULL,
  7,   1, 312,  10, 0, "Bar Graphics Maker", 0,					TEXT_REGION, NULL,
  7,  12, 312,  12, 0, NULL, 0,													LO_LINE, NULL,
  7,  90, 312,  90, 0, NULL, 0,													LO_LINE, NULL,
  7, 119, 312, 119, 0, NULL, 0,													LO_LINE, NULL,
 10, 124,  92, 137, 0, "OK",	0,												BUTTON_GADGET, NULL,
227, 124, 309, 137, 0, "Cancel", 0,											BUTTON_GADGET, NULL,
 36,  23,  75,  36, 1, "1", 0,													INTEGER_GADGET, (struct GadgetRecord *)&bgm_01_SR,
111,  23, 150,  36, 1, "2", 0,													INTEGER_GADGET, (struct GadgetRecord *)&bgm_02_SR,
186,  23, 225,  36, 1, "3", 0,													INTEGER_GADGET, (struct GadgetRecord *)&bgm_03_SR,
261,  23, 300,  36, 1, "4", 0,													INTEGER_GADGET, (struct GadgetRecord *)&bgm_04_SR,
 36,  38,  75,  51, 1, "5", 0,													INTEGER_GADGET, (struct GadgetRecord *)&bgm_05_SR,
111,  38, 150,  51, 1, "6", 0,													INTEGER_GADGET, (struct GadgetRecord *)&bgm_06_SR,
186,  38, 225,  51, 1, "7", 0,													INTEGER_GADGET, (struct GadgetRecord *)&bgm_07_SR,
261,  38, 300,  51, 1, "8", 0,													INTEGER_GADGET, (struct GadgetRecord *)&bgm_08_SR,
 36,  53,  75,  66, 1, "9", 0,													INTEGER_GADGET, (struct GadgetRecord *)&bgm_09_SR,
111,  53, 150,  66, 1, "10", 0,													INTEGER_GADGET, (struct GadgetRecord *)&bgm_10_SR,
186,  53, 225,  66, 1, "11", 0,													INTEGER_GADGET, (struct GadgetRecord *)&bgm_11_SR,
261,  53, 300,  66, 1, "12", 0,													INTEGER_GADGET, (struct GadgetRecord *)&bgm_12_SR,
 36,  68,  75,  81, 1, "13", 0,													INTEGER_GADGET, (struct GadgetRecord *)&bgm_13_SR,
111,  68, 150,  81, 1, "14", 0,													INTEGER_GADGET, (struct GadgetRecord *)&bgm_14_SR,
186,  68, 225,  81, 1, "15", 0,													INTEGER_GADGET, (struct GadgetRecord *)&bgm_15_SR,
261,  68, 300,  81, 1, "16", 0,													INTEGER_GADGET, (struct GadgetRecord *)&bgm_16_SR,
236,  98, 275, 111, 1, "Real maximum:", 0,							INTEGER_GADGET, (struct GadgetRecord *)&bgm_realmax_SR,
0,0,0,0,0,NULL,0,PREV_GR,(struct GadgetRecord *)NULL,	// 'last one'
-1
};

/**** disable CTRL-C break ****/

int CXBRK(void) { return(0); }
void chkabort(void) { return; } //{ return(0); }

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

	//UA_TranslateGR(about_GR,msgs);

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
		UA_DoubleGadgetDimensions(bgm_GR);

	UAI.windowX				= -1;	/* -1 means center on screen */
	UAI.windowY				= -1;	/* -1 means center on screen */
	UAI.windowWidth		= bgm_GR[0].x2;
	UAI.windowHeight	= bgm_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;

	/**** set the right font for this window ****/

	UAI.small_TF = rvrec->smallfont;
	UAI.large_TF = rvrec->largefont;

	UA_OpenWindow(&UAI);
	UA_SetMenuColors(rvrec,UAI.userWindow);

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, bgm_GR);

	/**** monitor user ****/
	
	MonitorUser(UAI.userWindow);

	/**** close the window ****/

	UA_ResetMenuColors(rvrec,UAI.userWindow);
	UA_CloseWindow(&UAI);
}

/******** MonitorUser() ********/

void MonitorUser(struct Window *window)
{
BOOL loop,retval,tabbed;
int ID, i, j, maxgad, firstact, pixheight;
struct EventData CED;
float pct;
int values[NUMSTRGADS], realmax;
WORD oh;

	/**** set buttons ****/

	j=0;
	firstact=-1;
	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( rvrec->ES[i] && rvrec->ES[i]->Active )
		{
			if ( firstact==-1 )
				firstact=i;
			j++;
		}
	}

	if ( j>NUMSTRGADS )
		maxgad=NUMSTRGADS;
	else
		maxgad=NUMSTRGADS-j;	
	for(i=0; i<maxgad; i++)
		UA_DisableButton(window, &bgm_GR[8+(NUMSTRGADS-maxgad)+i], mypattern1);

	if ( firstact != -1 )
	{
		pixheight = rvrec->EW[firstact]->Height;
		j=0;
		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if ( rvrec->ES[i] && rvrec->ES[i]->Active )
			{
				pct = ((float)rvrec->EW[i]->Height / (float)pixheight) * 100.0;
				UA_SetStringGadgetToVal(window, &bgm_GR[8+j], (int)pct);
				values[j]=(int)pct;
				j++;
			}
		}
	}

	UA_SetStringGadgetToString(window, &bgm_GR[24], "100");
	realmax=100;

	/**** event handler ****/

	loop=TRUE;
	retval=FALSE;
	while(loop)
	{
		UA_doStandardWait(window,&CED);

		switch(CED.Class)
		{
			case IDCMP_MOUSEBUTTONS:
				if (CED.Code==SELECTDOWN)
				{
					ID = UA_CheckGadgetList(window, bgm_GR, &CED);

					if ( firstact != -1 )
					{
						if ( ID>=8 && ID<=23 )
						{
again:
							tabbed = UA_ProcessStringGadget(window, bgm_GR, &bgm_GR[ID], &CED);
							UA_SetValToStringGadgetVal(&bgm_GR[ID], &values[ID-8]);
							if ( values[ID-8]<0 )
								values[ID-8]=0;
							if ( values[ID-8]>100 )
								values[ID-8]=100;
							UA_SetStringGadgetToVal(window, &bgm_GR[ID], values[ID-8]);
							if ( tabbed )
							{
								if ((ID-8)>=(NUMSTRGADS-maxgad-1) )
									ID=8;
								else
									ID++;
								goto again;
							}
						}
					}

					switch(ID)
					{
do_ok:
						case 6:		// OK
							UA_HiliteButton(window, &bgm_GR[6]);
							retval=TRUE;
							loop=FALSE;
							break;

do_cancel:
						case 7:		// Cancel
							UA_HiliteButton(window, &bgm_GR[7]);
							loop=FALSE;
							break;

						case 24:	// realmax
							UA_ProcessStringGadget(window, bgm_GR, &bgm_GR[24], &CED);
							UA_SetValToStringGadgetVal(&bgm_GR[24], &realmax);
							if ( realmax <= 10 )
								realmax = 10;
							UA_SetStringGadgetToVal(window, &bgm_GR[24], realmax);
							break;
					}

				}
				break;

			case IDCMP_RAWKEY:
				if ( CED.Code==RAW_RETURN )
					goto do_ok;
				else if ( CED.Code==RAW_ESCAPE )
					goto do_cancel;
				break;
		}
	}

	if ( retval && firstact!=-1 )
	{
		rvrec->returnCode = TRUE;

		j=0;
		oh = rvrec->EW[firstact]->Y + rvrec->EW[firstact]->Height;
		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if ( rvrec->ES[i] && rvrec->ES[i]->Active )
			{
				if ( values[j]==0 )
					values[j]=1;			
				pct = ((float)values[j] / (float)realmax) * (FLOAT)pixheight;
//printser("oh=%d pct=%d\n",oh,(int)pct);
				rvrec->EW[i]->Height = (int)pct;
				rvrec->EW[i]->Y = oh - rvrec->EW[i]->Height;
				j++;
			}
		}
	}
}

/******** E O F ********/
