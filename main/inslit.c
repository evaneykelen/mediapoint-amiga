#include "nb:pre.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Library *medialinkLibBase;
extern struct Gadget PropSlider1;
extern UBYTE **msgs;   

/**** gadgets ****/

extern struct GadgetRecord Literals_GR[];

/**** functions ****/

/******** InsertLiteral() ********/

UWORD InsertLiteral(void)
{
struct Window *window;
BOOL loop;
int i,ID,x,y,w,h,pos,lace,topEntry,numCmds,line,box;
TEXT str[2];
struct GadgetRecord *GR_ptr;
struct ScrollRecord SR;
BOOL dblClicked, retval;
UBYTE selectionList[20];
TEXT datestr[200];

	topEntry=0;
	numCmds=16;	// !!!!!!!!!!!!!!!!   see also selectionList size !!!!!!!!!!!!!!  
	loop = TRUE;
	box=-1;
	line=-1;

	for(i=0; i<20; i++)
		selectionList[i] = 0;

	GR_ptr = (struct GadgetRecord *)AllocMem(sizeof(struct GadgetRecord)*100,MEMF_CLEAR|MEMF_ANY);
	if (GR_ptr==NULL)
		return(0);

	/**** open a window ****/

	window = UA_OpenRequesterWindow(pageWindow,Literals_GR,STDCOLORS);
	if (!window)
	{
		FreeMem(GR_ptr,sizeof(struct GadgetRecord)*100);
		UA_WarnUser(-1);
		return(0);
	}

	/**** render gadget ****/

	UA_DrawGadgetList(window, Literals_GR);

	w = Literals_GR[3].x2 - Literals_GR[3].x1;
	h = Literals_GR[3].y2 - Literals_GR[3].y1;

	PropSlider1.LeftEdge	= Literals_GR[5].x1+4;
	PropSlider1.TopEdge		= Literals_GR[5].y1+2;
	PropSlider1.Width			= Literals_GR[5].x2-Literals_GR[5].x1-7;
	PropSlider1.Height		= Literals_GR[5].y2-Literals_GR[5].y1-3;

	if ( UA_IsWindowOnLacedScreen(pageWindow) )
	{
		PropSlider1.TopEdge	+= 2;
		PropSlider1.Height	-= 4;
		lace=2;
	}
	else
		lace=1;

	for(y=0; y<8; y++)
	{
		for(x=0; x<12; x++)
		{
			ClipBlit(	window->RPort,
								Literals_GR[3].x1, Literals_GR[3].y1,
								window->RPort,
								Literals_GR[3].x1+25*x, Literals_GR[3].y1+(h+4*lace)*y,
								w+1,h+1,0xc0);
			GR_ptr[x+y*12].x1 = Literals_GR[3].x1+25*x;
			GR_ptr[x+y*12].y1 = Literals_GR[3].y1+(h+4*lace)*y;
			GR_ptr[x+y*12].x2 = GR_ptr[x+y*12].x1+w;
			GR_ptr[x+y*12].y2 = GR_ptr[x+y*12].y1+h;
			GR_ptr[x+y*12].type = BUTTON_GADGET;
			GR_ptr[x+y*12+1].x1 = -1;
		}
	}

	SetAPen(window->RPort,LO_PEN);
	SetDrMd(window->RPort,JAM1);

	str[1]='\0';

	for(y=0; y<8; y++)
	{
		for(x=0; x<12; x++)
		{
			str[0] = 160+x+y*12;
			pos = TextLength(window->RPort,str,1) + 1;
			pos = (w-pos)/2;
			Move(	window->RPort,
						Literals_GR[3].x1+25*x+pos,
						Literals_GR[3].y1+(h+4*lace)*y + window->RPort->TxBaseline + lace);
			Text(window->RPort,str,1L);
		}
	}

	SR.GR							= &Literals_GR[4];
	SR.window					= window;
	SR.list						= msgs[ Msg_LiteralList-1 ];
	SR.sublist				= NULL;
	SR.selectionList	= selectionList;
	SR.entryWidth			= 12;
	SR.numDisplay			= 5;
	SR.numEntries			= numCmds;

	InitPropInfo(	(struct PropInfo *)PropSlider1.SpecialInfo,
								(struct Image *)PropSlider1.GadgetRender);
	AddGadget(window, &PropSlider1, -1L);

	UA_SetPropSlider(window,&PropSlider1,SR.numEntries,SR.numDisplay,topEntry);
	UA_PrintStandardList(NULL, -1, NULL);	// init static
	UA_PrintStandardList(&SR, topEntry, NULL);

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		dblClicked=FALSE;
		if (CED.extraClass == DBLCLICKED)
			dblClicked=TRUE;

		if ( CED.extraClass==IDCMP_GADGETDOWN || CED.extraClass==IDCMP_GADGETUP )
		{
			UA_ScrollStandardList(&SR, &topEntry, &PropSlider1, NULL, &CED);
		}
		else if ( CED.Class==IDCMP_MOUSEBUTTONS && ( CED.Code==SELECTDOWN || dblClicked) )
		{
			ID = UA_CheckGadgetList(window, Literals_GR, &CED);
			switch(ID)
			{
				case 2:	// Cancel
do_cancel:
					UA_HiliteButton(window, &Literals_GR[2]);
					loop=FALSE;
					retval=FALSE;
					break;

				case 6:	// OK
do_ok:
					UA_HiliteButton(window, &Literals_GR[6]);
					loop=FALSE;
					retval=TRUE;
					break;

				case 4:	// scroll list
					line = UA_SelectStandardListLine(&SR, topEntry, FALSE, &CED, FALSE, FALSE);
					if ( line != -1 )
					{
						line += topEntry;
						if ( box != -1 )
							UA_InvertButton(window, &GR_ptr[box]);
						box=-1;
						if ( dblClicked )
						{
							loop=FALSE;
							retval=TRUE;
						}
						GetDateString(datestr,line+1);
						if ( datestr[0] != '\0' )
						{
							UA_ShortenString(window->RPort, datestr, (Literals_GR[7].x2-Literals_GR[7].x1)-16);
							UA_ClearButton(window, &Literals_GR[7], AREA_PEN);
							Literals_GR[7].type = BUTTON_GADGET;
							UA_DrawText(window, &Literals_GR[7], datestr);
							Literals_GR[7].type = LOBOX_REGION;
						}
						else
							UA_ClearButton(window, &Literals_GR[7], AREA_PEN);
					}
					break;
			}

			ID = UA_CheckGadgetList(window, GR_ptr, &CED);
			if (ID!=-1)
			{
				line=-1;
				if ( box != -1 )
					UA_InvertButton(window, &GR_ptr[box]);
				box=ID;
				UA_InvertButton(window, &GR_ptr[ID]);
				for(i=0; i<20; i++)
					selectionList[i] = 0;
				UA_PrintStandardList(NULL, -1, NULL);	// init static
				UA_PrintStandardList(&SR, topEntry, NULL);
				if ( dblClicked )
				{
					loop=FALSE;
					retval=TRUE;
				}
				sprintf(datestr, "%c", box+160);
				UA_ClearButton(window, &Literals_GR[7], AREA_PEN);
				Literals_GR[7].type = BUTTON_GADGET;
				UA_DrawText(window, &Literals_GR[7], datestr);
				Literals_GR[7].type = LOBOX_REGION;
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_RETURN)
				goto do_ok;
			if (CED.Code==RAW_ESCAPE)
				goto do_cancel;
		}
	}

	UA_CloseRequesterWindow(window,STDCOLORS);

	FreeMem(GR_ptr,sizeof(struct GadgetRecord)*100);

	if ( retval )
	{
		if ( line!=-1 )
			return( (UWORD)(line+1001) );
		else if ( box!=-1 )
			return( (UWORD)(box+160) );
	}

	return(0);
}

/******** E O F ********/
