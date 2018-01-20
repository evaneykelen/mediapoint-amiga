#include "nb:pre.h"

#include "minc:types.h"
#include "minc:defs.h"
#include "minc:errors.h"
#include "minc:process.h"
#include "minc:ge.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *scriptWindow;
extern struct Library *medialinkLibBase;
extern UWORD chip gui_pattern[];
extern struct Gadget PropSlider1;
extern UBYTE **msgs;   

/**** static globals ****/

static UBYTE **buttonList;

/**** static globals ****/

static struct PropInfo PI2 = { AUTOKNOB | FREEVERT | PROPBORDERLESS, 0,0,0,0, };
static struct Image Im2 = { 0,0,0,0,0,NULL,0x0000,0x0000,NULL };
static struct Gadget PropSlider2 =
{
	NULL, 0,0,0,0, NULL, GACT_RELVERIFY | GACT_IMMEDIATE, GTYP_PROPGADGET,
	&Im2, NULL, NULL, NULL, (struct PropInfo *)&PI2, 2, NULL
};

/**** gadgets ****/

extern struct GadgetRecord LocalEvents_GR[];

/**** functions ****/

/******** MonitorLocalEvents() ********/

BOOL MonitorLocalEvents(struct ScriptInfoRecord *SIR)
{
struct Window *window;
BOOL loop, retVal;
int i, j, k, l, ID, numPages, numJumps, numDisp=8, topEntry1, topEntry2, line;
SNRPTR this_node;
struct ScriptNodeRecord temp_node;
struct List *list;
struct ScriptEventRecord *ser, **serlist;
struct PageEventRecord *per;
UBYTE **pageList, *page, *selectionList, **jumpList, *jumpSelList,
			**undoLabelSNR;
struct Gadget *g;
BOOL dblClicked;
struct ScrollRecord SR1, SR2;
int selectedPage, selectedJump;

	SetSpriteOfActWdw(SPRITE_BUSY);

	Validate_All_LE(SIR,FALSE);	// link LOCALEVENTS to labels

	/**** init vars ****/

	retVal 				= FALSE;
	loop   				= TRUE;

	list 					= NULL;
	ser 					= NULL;
	per 					= NULL;
	page 					= NULL;
	pageList 			= NULL;
	selectionList = NULL;
	jumpList 			= NULL;
	buttonList		= NULL;
	jumpSelList		= NULL;
	undoLabelSNR	= NULL;

	numPages 			= 0;
	numJumps		 	= 0;
	topEntry1 		= 0L;
	topEntry2 		= 0L;

	selectedPage = -1;
	selectedJump = -1;

	/**** calculate number of PAGEs with Local Events attached ****/

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] != NULL)
		{
			list = SIR->allLists[i];
			if (list->lh_TailPred != (struct Node *)list)
			{
				for(this_node=(SNRPTR)list->lh_Head;
						this_node->node.ln_Succ;
						this_node=(SNRPTR)this_node->node.ln_Succ)
				{
					if (	this_node->nodeType==TALK_PAGE &&
								this_node->numericalArgs[15]==1 &&
								this_node->list!=NULL )
						numPages++;
				}
			}
		}
	}
	
	if ( numPages==0 )
	{
		SetSpriteOfActWdw(SPRITE_NORMAL);
		Message( msgs[ Msg_LocalEvents_4-1 ] );
		return(TRUE);
	}

	/**** allocate list of pointers to page names ****/

	pageList = (UBYTE **)AllocMem(sizeof(UBYTE *)*numPages,MEMF_CLEAR|MEMF_ANY);
	if ( pageList==NULL )
	{
		SetSpriteOfActWdw(SPRITE_NORMAL);
		UA_WarnUser(-1);
		return(FALSE);
	}

	undoLabelSNR = (UBYTE **)AllocMem(sizeof(UBYTE *)*(numPages*MAX_LOCAL_EVENTS),
																		MEMF_CLEAR|MEMF_ANY);
	if ( undoLabelSNR==NULL )
	{
		SetSpriteOfActWdw(SPRITE_NORMAL);
		UA_WarnUser(-1);
		return(FALSE);
	}

	selectionList = (UBYTE *)AllocMem(numPages,MEMF_CLEAR|MEMF_ANY);
	if ( selectionList==NULL )
	{
		SetSpriteOfActWdw(SPRITE_NORMAL);
		UA_WarnUser(-1);
		return(FALSE);
	}

	jumpSelList = (UBYTE *)AllocMem(MAX_LOCAL_EVENTS,MEMF_CLEAR|MEMF_ANY);
	if ( jumpSelList==NULL )
	{
		SetSpriteOfActWdw(SPRITE_NORMAL);
		UA_WarnUser(-1);
		return(FALSE);
	}

	/**** get all names of pages with local events ****/

	j=0;
	k=0;
	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] != NULL)
		{
			list = SIR->allLists[i];
			if (list->lh_TailPred != (struct Node *)list)
			{
				for(this_node=(SNRPTR)list->lh_Head;
						this_node->node.ln_Succ;
						this_node=(SNRPTR)this_node->node.ln_Succ)
				{
					if (	this_node->nodeType==TALK_PAGE &&
								this_node->numericalArgs[15]==1 &&
								this_node->list!=NULL )
					{
						pageList[j] = this_node->objectName;

						per = (struct PageEventRecord *)(this_node->list->lh_Head);
						serlist = (struct ScriptEventRecord **)per->er_LocalEvents;

						for(l=0; l<MAX_LOCAL_EVENTS; l++)
						{
							if ( serlist[l] != NULL )
							{
								undoLabelSNR[k] = (UBYTE *)serlist[l]->labelSNR;
								k++;
							}
							else
								break;
						}	

						j++;
					}
				}
			}
		}
	}

	SetSpriteOfActWdw(SPRITE_NORMAL);

	/**** open a window ****/

	window = UA_OpenRequesterWindow(scriptWindow,LocalEvents_GR,STDCOLORS);
	if (!window)
	{
		UA_WarnUser(-1);
		return(FALSE);
	}

	/**** render gadgets and scroll bar ****/

	PropSlider1.LeftEdge	= LocalEvents_GR[7].x1+4;
	PropSlider1.TopEdge		= LocalEvents_GR[7].y1+2;
	PropSlider1.Width			= LocalEvents_GR[7].x2-LocalEvents_GR[7].x1-7;
	PropSlider1.Height		= LocalEvents_GR[7].y2-LocalEvents_GR[7].y1-3;

	PropSlider2.LeftEdge	= LocalEvents_GR[9].x1+4;
	PropSlider2.TopEdge		= LocalEvents_GR[9].y1+2;
	PropSlider2.Width			= LocalEvents_GR[9].x2-LocalEvents_GR[9].x1-7;
	PropSlider2.Height		= LocalEvents_GR[9].y2-LocalEvents_GR[9].y1-3;

	if ( UA_IsWindowOnLacedScreen(window) )
	{
		PropSlider1.TopEdge	+= 2;
		PropSlider1.Height	-= 4;

		PropSlider2.TopEdge	+= 2;
		PropSlider2.Height	-= 4;
	}

	UA_DrawGadgetList(window, LocalEvents_GR);

	InitPropInfo(	(struct PropInfo *)PropSlider1.SpecialInfo,
								(struct Image *)PropSlider1.GadgetRender);
	AddGadget(window, &PropSlider1, -1L);
	InitPropInfo(	(struct PropInfo *)PropSlider2.SpecialInfo,
								(struct Image *)PropSlider2.GadgetRender);
	AddGadget(window, &PropSlider2, -1L);

	UA_SetPropSlider(window, &PropSlider1, numPages, numDisp, topEntry1);
	UA_SetPropSlider(window, &PropSlider2, numPages, numDisp, topEntry1);

	//UA_DisableButton(window, &LocalEvents_GR[10], gui_pattern);	// edit
	UA_DisableButton(window, &LocalEvents_GR[13], gui_pattern);	// pick
	OffGadget(&PropSlider2, window, NULL);	// jump list slider

	/**** init scroll record ****/

	SR1.GR							= &LocalEvents_GR[6];
	SR1.window					= window;
	SR1.list						= NULL;
	SR1.sublist					= NULL;
	SR1.selectionList		= selectionList;
	SR1.entryWidth			= -1;
	SR1.numDisplay			= numDisp;
	SR1.numEntries			= numPages;

	UA_PrintStandardList(NULL,-1,NULL);	// init static
	UA_PrintStandardList(&SR1,topEntry1,pageList);

	SR2.GR							= &LocalEvents_GR[8];
	SR2.window					= window;
	SR2.list						= NULL;
	SR2.sublist					= NULL;
	SR2.selectionList		= jumpSelList;
	SR2.entryWidth			= -1;
	SR2.numDisplay			= numDisp;
	SR2.numEntries			= numJumps;

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		dblClicked=FALSE;
		if (CED.extraClass == DBLCLICKED)
			dblClicked=TRUE;

		if ( CED.extraClass==IDCMP_GADGETDOWN || CED.extraClass==IDCMP_GADGETUP )
		{
			g = (struct Gadget *)CED.IAddress;
			if (g)
			{
				ID = g->GadgetID;
				if ( ID==1 )
				{
					UA_PrintStandardList(NULL,-1,NULL);	// init static
					UA_ScrollStandardList(&SR1,&topEntry1,&PropSlider1,pageList,&CED);
				}
				else if (ID==2)
				{
					if ( jumpList )
					{
						UA_PrintStandardList(NULL,-1,NULL);	// init static
						UA_ScrollStandardList(&SR2,&topEntry2,&PropSlider2,buttonList,&CED);
					}
				}
			}
		}
		else if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, LocalEvents_GR, &CED);
			switch(ID)
			{
				case 6:	// page list area
					line = UA_SelectStandardListLine(&SR1,topEntry1,FALSE,&CED,FALSE,FALSE);
					selectedPage = line;
					if ( line != -1 )
					{
						if ( jumpList )	// free previous jump list
							FreeMem(jumpList,sizeof(UBYTE *)*numJumps);

						if ( buttonList )	// free previous button list
							FreeMem(buttonList,sizeof(UBYTE *)*numJumps);

						UA_ClearButton(window, &LocalEvents_GR[8], AREA_PEN);

						jumpList = CreateJumpList(topEntry1+line, &numJumps, SIR);
						if ( jumpList )
						{
							SR2.numEntries	= numJumps;
							SR2.sublist			= jumpList;
							OnGadget(&PropSlider2, window, NULL);	// jump list slider
UA_SetPropSlider(window, &PropSlider2, numJumps, numDisp, topEntry2);
							//UA_EnableButton(window, &LocalEvents_GR[10]);								// edit
							UA_DisableButton(window, &LocalEvents_GR[13], gui_pattern);	// pick
							for(i=0; i<MAX_LOCAL_EVENTS; i++)
								jumpSelList[i] = 0;							
							UA_PrintStandardList(NULL,-1,NULL);	// init static
							UA_PrintStandardList(&SR2,topEntry2,buttonList);
						}
					}
					break;

				case 8:	// jump list area
					line = UA_SelectStandardListLine(&SR2,topEntry2,FALSE,&CED,FALSE,FALSE);
					selectedJump = line;
					if ( line !=- 1 )
					{
						UA_EnableButton(window, &LocalEvents_GR[13]);	// pick
						if ( dblClicked )
							goto do_pick;
					}
					break;

#if 0
				case 10:	// Edit
do_edit:
					UA_HiliteButton(window, &LocalEvents_GR[10]);
					break;
#endif

				case 11:	// OK
do_ok:
					UA_HiliteButton(window, &LocalEvents_GR[11]);
					loop=FALSE;
					retVal=TRUE;
					break;

				case 12:	// Cancel
do_cancel:
					UA_HiliteButton(window, &LocalEvents_GR[12]);
					loop=FALSE;
					retVal=FALSE;
					break;

				case 13:	// Pick
do_pick:
					UA_HiliteButton(window, &LocalEvents_GR[13]);
					temp_node.extraData=NULL;

					if (	!strcmp( jumpList[selectedJump+topEntry2], msgs[ Msg_GotoPrevPage-1 ] ) ||
								!strcmp( jumpList[selectedJump+topEntry2], msgs[ Msg_GotoNextPage-1 ] ) )
					{
						Message( msgs[ Msg_DestCantBeChanged-1 ] );
					}
					else
					{
						standAlonePickLabel(scriptWindow,&temp_node);
						if ( temp_node.extraData )
						{
							ModifyJumpList(	SIR, &temp_node, numPages,
															selectedPage+topEntry1, selectedJump+topEntry2 );
							UA_PrintStandardList(NULL,-1,NULL);	// init static
							UA_PrintStandardList(&SR2,topEntry2,buttonList);
						}
					}
					break;
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE)				// cancel
				goto do_cancel;
			else if (CED.Code==RAW_RETURN)	// OK
				goto do_ok;
		}
	}

	if ( !retVal )	// put back old labelSNR and find label names again
	{
		k=0;
		for(i=0; i<CPrefs.MaxNumLists; i++)
		{
			if (SIR->allLists[i] != NULL)
			{
				list = SIR->allLists[i];
				if (list->lh_TailPred != (struct Node *)list)
				{
					for(this_node=(SNRPTR)list->lh_Head; this_node->node.ln_Succ;
							this_node=(SNRPTR)this_node->node.ln_Succ)
					{
						if (	this_node->nodeType==TALK_PAGE && this_node->numericalArgs[15]==1 &&
									this_node->list!=NULL )
						{
							per = (struct PageEventRecord *)(this_node->list->lh_Head);
							serlist = (struct ScriptEventRecord **)per->er_LocalEvents;

							for(l=0; l<MAX_LOCAL_EVENTS; l++)
							{
								if ( serlist[l] != NULL )
								{
									serlist[l]->labelSNR = (SNRPTR)undoLabelSNR[k];
									strcpy(serlist[l]->labelName, serlist[l]->labelSNR->objectName);
									k++;
								}
								else
									break;
							}	
						}
					}
				}
			}
		}
	}

	UA_EnableButton(window, &LocalEvents_GR[13]);	// pick

	UA_CloseRequesterWindow(window,STDCOLORS);

free_pagelist:

	if ( pageList )
		FreeMem(pageList,sizeof(UBYTE *)*numPages);

	if ( undoLabelSNR )
		FreeMem(undoLabelSNR,sizeof(UBYTE *)*(numPages*MAX_LOCAL_EVENTS));

	if ( jumpList )
		FreeMem(jumpList,sizeof(UBYTE *)*numJumps);

	if ( buttonList )
		FreeMem(buttonList,sizeof(UBYTE *)*numJumps);

	if ( selectionList )
		FreeMem(selectionList,numPages);

	if ( jumpSelList )
		FreeMem(jumpSelList,MAX_LOCAL_EVENTS);

	return(retVal);
}

/******** CreateJumpList() ********/
/*
 * Looks up which node we want to see a jump list of and creates a jump list.
 *
 */

UBYTE **CreateJumpList(int line, int *numJumps, struct ScriptInfoRecord *SIR)
{
int i, j;
SNRPTR this_node;
struct List *list;
struct ScriptEventRecord *ser, **serlist;
struct PageEventRecord *per;
UBYTE **jumpList;

	per = NULL;
	serlist = NULL;

	j=0;
	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] != NULL)
		{
			list = SIR->allLists[i];
			if (list->lh_TailPred != (struct Node *)list)
			{
				for(this_node=(SNRPTR)list->lh_Head; this_node->node.ln_Succ;
						this_node=(SNRPTR)this_node->node.ln_Succ)
				{
					if (	this_node->nodeType==TALK_PAGE && this_node->numericalArgs[15]==1 &&
								this_node->list!=NULL )
					{
						if (j==line)
						{
							per = (struct PageEventRecord *)(this_node->list->lh_Head);
							serlist = (struct ScriptEventRecord **)per->er_LocalEvents;
							goto brk_fast;
						}
						else
							j++;
					}
				}
			}
		}
	}		

brk_fast:

	if ( serlist==NULL )
		return(NULL);

	i=0;
	while( (serlist[i] != NULL) && (i < MAXEDITWINDOWS) )
		i++;

	if ( i==0 )
		return(NULL);

	// i is now number of local events this page has

	*numJumps = i;

	jumpList = (UBYTE **)AllocMem(sizeof(UBYTE *)*(*numJumps),MEMF_CLEAR|MEMF_ANY);
	if ( jumpList==NULL )
		return(NULL);

	buttonList = (UBYTE **)AllocMem(sizeof(UBYTE *)*(*numJumps),MEMF_CLEAR|MEMF_ANY);
	if ( buttonList==NULL )
		return(NULL);

	for(i=0; i<*numJumps; i++)
	{
		ser = (struct ScriptEventRecord *)serlist[i];						

		if ( ser->typeBits == TB_PREVPAGE )
		{
			jumpList[i] = msgs[ Msg_GotoPrevPage-1 ];
		}
		else if ( ser->typeBits == TB_NEXTPAGE )
		{
			jumpList[i] = msgs[ Msg_GotoNextPage-1 ];
		}
		else
		{
			if ( ser->labelSNR )
				jumpList[i] = ser->labelName;
			else
				jumpList[i] = msgs[ Msg_LocalEvents_8-1 ];	// unreferenced
		}

		buttonList[i] = ser->buttonName;
	}

	return(jumpList);
}

/******** ModifyJumpList() ********/

void ModifyJumpList(struct ScriptInfoRecord *SIR,
										struct ScriptNodeRecord *temp_node, int numPages,
										int selectedPage, int selectedJump)
{
int i, j;
SNRPTR this_node;
struct List *list;
struct ScriptEventRecord *ser, **serlist;
struct PageEventRecord *per;

	per = NULL;
	serlist = NULL;

	/**** find which page is selected ****/

	j=0;
	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] != NULL)
		{
			list = SIR->allLists[i];
			if (list->lh_TailPred != (struct Node *)list)
			{
				for(this_node=(SNRPTR)list->lh_Head; this_node->node.ln_Succ;
						this_node=(SNRPTR)this_node->node.ln_Succ)
				{
					if (	this_node->nodeType==TALK_PAGE &&
								this_node->numericalArgs[15]==1 && this_node->list!=NULL )
					{
						if ( j==selectedPage )	//line)
						{
							per = (struct PageEventRecord *)(this_node->list->lh_Head);
							serlist = (struct ScriptEventRecord **)per->er_LocalEvents;
							goto brk_fast;
						}
						else
							j++;
					}
				}
			}
		}
	}		

brk_fast:

	if ( serlist==NULL )
		return;

	ser = serlist[ selectedJump ];	//line];

	if ( ser )
	{
		strcpy(ser->labelName, temp_node->objectName);
		ser->labelSNR = (SNRPTR)temp_node->extraData;
	}
}

/******** E O F ********/
