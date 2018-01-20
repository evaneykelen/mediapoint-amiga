#include "nb:pre.h"
#include "protos.h"

#define MONLISTWIDTH 80

/**** externals ****/

extern struct RendezVousRecord *rvrec;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *medialinkLibBase;
extern struct EventData CED;
extern UWORD chip mypattern1[];
extern UBYTE **msgs;
extern struct MsgPort *capsport;

/**** static globals ****/

static struct PropInfo PI = { AUTOKNOB | FREEVERT | PROPBORDERLESS, 0,0,0,0, };
static struct Image Im = { 0,0,0,0,0,NULL,0x0000,0x0000,NULL };
static struct Gadget PropSlider =
{
	NULL, 0,0,0,0, NULL, GACT_RELVERIFY | GACT_IMMEDIATE, GTYP_PROPGADGET,
	&Im, NULL, NULL, NULL, (struct PropInfo *)&PI, 2, NULL
};
 
/**** gadgets ****/

extern struct GadgetRecord SelMon_GR[];

/**** functions ****/

/******** SelectMonitor() ********/

BOOL SelectMonitor(	struct Window *onWindow, ULONG *monitorID, BOOL laced,
										WORD width, WORD height, STRPTR monName )
{
int numMons, numDisp=12, topEntry=0, ID, i, j, listPos;
struct ScrollRecord SR;
struct Window *window;
UBYTE *monList;
ULONG *IDS;
UBYTE selectionList[100];
BOOL dblClicked,loop,retVal=TRUE;

	monList = (UBYTE *)AllocMem(MONLISTWIDTH*100,MEMF_ANY);
	if ( !monList )
		return(FALSE);

	IDS = (ULONG *)AllocMem(100*sizeof(ULONG),MEMF_ANY);
	if ( !IDS )
	{
		FreeMem(monList,MONLISTWIDTH*100);
		return(FALSE);
	}

	listPos = -1;
	if ( laced )
		numMons = MakeMonitorList(monList,IDS,100,laced,width,height,*monitorID,monName,&listPos);
	else
		numMons = MakeMonitorList(monList,IDS,100,laced,width,height,0L,monName,&listPos);
	if ( numMons==0 )
	{
		FreeMem(IDS,100*sizeof(ULONG));
		FreeMem(monList,MONLISTWIDTH*100);
		return(FALSE);
	}

	for(i=0; i<100; i++)
		selectionList[i] = 0;

	if ( listPos>=0 && listPos<100 )
		selectionList[ listPos ] = 1;

	window = UA_OpenRequesterWindow(onWindow, SelMon_GR, USECOLORS);
	if ( !window )
	{
		FreeMem(IDS,100*sizeof(ULONG));
		FreeMem(monList,MONLISTWIDTH*100);
		return(FALSE);
	}

	/**** render gadgets and scroll bar ****/

	PropSlider.LeftEdge	= SelMon_GR[5].x1+4;
	PropSlider.TopEdge	= SelMon_GR[5].y1+2;
	PropSlider.Width		= SelMon_GR[5].x2-SelMon_GR[5].x1-7;
	PropSlider.Height		= SelMon_GR[5].y2-SelMon_GR[5].y1-3;

	if ( UA_IsWindowOnLacedScreen(window) )
	{
		PropSlider.TopEdge += 2;
		PropSlider.Height -= 4;
	}

	UA_DrawGadgetList(window,SelMon_GR);

	InitPropInfo(	(struct PropInfo *)PropSlider.SpecialInfo,
								(struct Image *)PropSlider.GadgetRender);
	AddGadget(window, &PropSlider, -1L);

	UA_SetPropSlider(window, &PropSlider, numMons, numDisp, topEntry);

	/**** init scroll record ****/

	SR.GR							= &SelMon_GR[4];
	SR.window					= window;
	SR.list						= monList;
	SR.sublist				= NULL;
	SR.selectionList	= selectionList;
	SR.entryWidth			= MONLISTWIDTH;
	SR.numDisplay			= numDisp;
	SR.numEntries			= numMons;

	if (window->WScreen->ViewPort.Modes & LACE)
		SetFont(window->RPort, rvrec->tiny_largefont);
	else
		SetFont(window->RPort, rvrec->tiny_smallfont);

	UA_PrintStandardList(NULL,-1,NULL);	// init static
	UA_PrintStandardList(&SR,topEntry,NULL);

	if (window->WScreen->ViewPort.Modes & LACE)
		SetFont(window->RPort, rvrec->largefont);
	else
		SetFont(window->RPort, rvrec->smallfont);

	/**** monitor user ****/

	loop=TRUE;

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		dblClicked=FALSE;
		if (CED.extraClass == DBLCLICKED)
			dblClicked=TRUE;

		if ( CED.extraClass==IDCMP_GADGETDOWN || CED.extraClass==IDCMP_GADGETUP )
		{
			if (window->WScreen->ViewPort.Modes & LACE)
				SetFont(window->RPort, rvrec->tiny_largefont);
			else
				SetFont(window->RPort, rvrec->tiny_smallfont);

			UA_ScrollStandardList(&SR,&topEntry,&PropSlider,NULL,&CED);

			if (window->WScreen->ViewPort.Modes & LACE)
				SetFont(window->RPort, rvrec->largefont);
			else
				SetFont(window->RPort, rvrec->smallfont);
		}
		else if ( CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN )
		{
			ID = UA_CheckGadgetList(window, SelMon_GR, &CED);
			switch(ID)
			{
				case 4:	// list
					if (window->WScreen->ViewPort.Modes & LACE)
						SetFont(window->RPort, rvrec->tiny_largefont);
					else
						SetFont(window->RPort, rvrec->tiny_smallfont);
					UA_SelectStandardListLine(&SR,topEntry,FALSE,&CED,FALSE,FALSE);
					if (window->WScreen->ViewPort.Modes & LACE)
						SetFont(window->RPort, rvrec->largefont);
					else
						SetFont(window->RPort, rvrec->smallfont);
					if ( dblClicked )
						goto do_ok;
					break;

				case 6:	// OK
do_ok:
					UA_HiliteButton(window, &SelMon_GR[6]);
					loop=FALSE;
					retVal=TRUE;
					break;

				case 7:	// Cancel
do_cancel:
					UA_HiliteButton(window, &SelMon_GR[7]);
					loop=FALSE;
					retVal=FALSE;
					break;
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE)
				goto do_cancel;
			else if (CED.Code==RAW_RETURN && !UA_IsGadgetDisabled(&SelMon_GR[6]))
				goto do_ok;
		}
	}

	/**** exit ****/

	UA_CloseRequesterWindow(window,USECOLORS);

	if ( retVal )
	{
		for(i=0; i<100; i++)
		{
			if ( selectionList[i] == 1 )
			{
				for(j=0; j<MONLISTWIDTH; j++)
				{
					if ( *(monList+(MONLISTWIDTH*i)+j) == ',' )
						break;
					else
					{
						monName[j] = *(monList+(MONLISTWIDTH*i)+j);
						monName[j+1] = '\0';
					}
				}
				*monitorID = *(IDS+i);
				break;				
			}
		}
	}

	FreeMem(IDS,100*sizeof(ULONG));
	FreeMem(monList,MONLISTWIDTH*100);

	return( retVal );
}

/******** E O F ********/
