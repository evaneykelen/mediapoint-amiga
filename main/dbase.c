#include "nb:pre.h"
#include "dbase.h"

#define RAWBUFSIZE 2000	// see also ptread.c and parsetext.c

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Screen *pageScreen;
extern struct Library *medialinkLibBase;
extern UBYTE **msgs;   

/**** static globals ****/

static struct PropInfo PI2 = { AUTOKNOB | FREEVERT | PROPBORDERLESS, 0,0,0,0, };
static struct Image Im2 = { 0,0,0,0,0,NULL,0x0000,0x0000,NULL };
static struct Gadget PropSlider =
{
	NULL, 0,0,0,0, NULL, GACT_RELVERIFY | GACT_IMMEDIATE, GTYP_PROPGADGET,
	&Im2, NULL, NULL, NULL, (struct PropInfo *)&PI2, 2, NULL
};

/**** gadgets ****/

extern struct GadgetRecord DBase_GR[];

/**** functions ****/

/******** ImportDBaseFile() ********/

BOOL ImportDBaseFile(STRPTR path, UBYTE *buffer)
{
struct Window *window;
BOOL loop, retVal;
int ID,numDisp,numLines,topEntry,line,rows_cols,first,last,i,size,numFields;
UBYTE *fieldsList[32];
struct ScrollRecord SR;
struct DBaseRecord dbase_rec;
TEXT str[10];

	/**** init vars ****/

	retVal = FALSE;
	loop = TRUE;
	numDisp = 4;
	topEntry = 0;
	rows_cols = 0;	// 0=rows 1=cols

	/**** do dBASE related stuff ****/

	if ( !InspectDBaseFile(&dbase_rec,path) )
		return(FALSE);	// errors are already processed - just leave quick

	first = 1;
	last = dbase_rec.numRecords;
	numLines = dbase_rec.numFields;

	numFields = numLines;
	if ( numFields>32 )
		numFields=32;		// WE ALLOW A MAX OF 32 FIELDS
	for(i=0; i<numFields; i++)
		fieldsList[i] = dbase_rec.fields+i*15;

	/**** open a window ****/

	window = UA_OpenRequesterWindow(pageWindow, DBase_GR, STDCOLORS);
	if (!window)
	{
		CloseDBaseFile( &dbase_rec );
		UA_WarnUser(-1);
		return(FALSE);
	}

	/**** render gadget ****/

	UA_DrawGadgetList(window, DBase_GR);

	/**** set buttons ****/

	UA_InvertButton(window, &DBase_GR[14+rows_cols]);	// inverts 14 or 15
	UA_SetStringGadgetToVal(window, &DBase_GR[7], first);
	UA_SetStringGadgetToVal(window, &DBase_GR[8], last);
	sprintf(str,"%d",dbase_rec.numRecords);
	UA_PrintInBox(	window, &DBase_GR[9],
									DBase_GR[9].x1, DBase_GR[9].y1, DBase_GR[9].x2, DBase_GR[9].y2,
									str, PRINT_CENTERED);

	/**** init list ****/

	PropSlider.LeftEdge	= DBase_GR[12].x1+4;
	PropSlider.TopEdge	= DBase_GR[12].y1+2;
	PropSlider.Width		= DBase_GR[12].x2-DBase_GR[12].x1-7;
	PropSlider.Height		= DBase_GR[12].y2-DBase_GR[12].y1-3;
	if ( UA_IsWindowOnLacedScreen(window) )
	{
		PropSlider.TopEdge += 2;
		PropSlider.Height	-= 4;
	}
	InitPropInfo((struct PropInfo *)PropSlider.SpecialInfo, (struct Image *)PropSlider.GadgetRender);
	AddGadget(window, &PropSlider, -1L);

	/**** init scroll record ****/

	SR.GR							= &DBase_GR[11];
	SR.window					= window;
	SR.list						= NULL;
	SR.sublist				= NULL;
	SR.selectionList	= NULL;
	SR.entryWidth			= -1;
	SR.numDisplay			= numDisp;
	SR.numEntries			= numLines;

	UA_SetPropSlider(window, &PropSlider, numLines, numDisp, topEntry);

	UA_PrintNewList(NULL,-1,NULL,FALSE);	// init static
	UA_PrintNewList(&SR,topEntry,fieldsList,FALSE);

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if ( CED.extraClass==IDCMP_GADGETDOWN || CED.extraClass==IDCMP_GADGETUP )
		{
			UA_ScrollNewList(&SR,&topEntry,&PropSlider,fieldsList,&CED);
		}
		else if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, DBase_GR, &CED);
			switch(ID)
			{
				case 5:		// OK
do_ok:
					UA_HiliteButton(window, &DBase_GR[5]);
					loop=FALSE;
					retVal=TRUE;
					break;

				case 6:		// Cancel
do_cancel:
					UA_HiliteButton(window, &DBase_GR[6]);
					loop=FALSE;
					retVal=FALSE;
					break;

				case 7:		// first
					UA_ProcessStringGadget(window, DBase_GR, &DBase_GR[ID], &CED);
					UA_SetValToStringGadgetVal(&DBase_GR[ID], &first);
					if ( first<1 || first>last )
					{
						Message(msgs[Msg_X_4-1], 1, last); // "Enter a value between...
						first=1;
					}
					UA_SetStringGadgetToVal(window, &DBase_GR[ID], first);
					break;

				case 8:		// last
					UA_ProcessStringGadget(window, DBase_GR, &DBase_GR[ID], &CED);
					UA_SetValToStringGadgetVal(&DBase_GR[ID], &last);
					if ( last<first || last>dbase_rec.numRecords )
					{
						Message(msgs[Msg_X_4-1], first, dbase_rec.numRecords);
						last=dbase_rec.numRecords;
					}
					UA_SetStringGadgetToVal(window, &DBase_GR[ID], last);
					break;

				case 11:	// fields list
					line = UA_SelectNewListLine(&SR,topEntry,&CED);
					if ( line != -1 )
					{
						line += topEntry;
						if ( *( fieldsList[line] + 1 ) == 0x21 )
							*( fieldsList[line] + 1 ) = 0x20;
						else
							*( fieldsList[line] + 1 ) = 0x21;
						UA_PrintNewList(NULL,-1,NULL,FALSE);	// init static
						UA_PrintNewList(&SR,topEntry,fieldsList,TRUE);
					}
					break;

				case 14:	// rows
					if ( rows_cols==1 )
					{
						UA_InvertButton(window, &DBase_GR[14]);
						UA_InvertButton(window, &DBase_GR[15]);
						rows_cols=0;
					}
					break;

				case 15:	// cols
					if ( rows_cols==0 )
					{
						UA_InvertButton(window, &DBase_GR[14]);
						UA_InvertButton(window, &DBase_GR[15]);
						rows_cols=1;
					}
					break;
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE)	// cancel
				goto do_cancel;
			else if (CED.Code==RAW_RETURN && !UA_IsGadgetDisabled(&DBase_GR[5]))	// OK
				goto do_ok;
		}
	}

	UA_CloseRequesterWindow(window,STDCOLORS);

	/**** do dBASE related stuff ****/

	if ( retVal )	
	{
		dbase_rec.firstRecord = first-1;
		dbase_rec.lastRecord = last-1;
		if ( rows_cols==0 )
			dbase_rec.orientation = REPORT_IN_ROWS;
		else
			dbase_rec.orientation = REPORT_IN_COLS;
		ID=0;
		for(i=0; i<numFields; i++)
		{
			if ( *( fieldsList[i] + 1 ) == 0x20 )	// check button off -> don't use this field
				dbase_rec.usedFields[i]=TRUE;
			else
				ID++;	// count # of selected fields: if 0 then do nakko (see below)
		}

		if ( ID>0 && OpenDBaseFile( &dbase_rec, path, TRUE ) )
		{
			size = strlen(dbase_rec.report);
			if ( size > RAWBUFSIZE )
				size = RAWBUFSIZE;
			stccpy(buffer, dbase_rec.report, size);
		}
		else
			retVal = FALSE;
	}

	CloseDBaseFile( &dbase_rec );

	return(retVal);
}

/******** InspectDBaseFile() ********/

BOOL InspectDBaseFile(struct DBaseRecord *dbase_rec, STRPTR path)
{
BOOL retval;

	retval = OpenDBaseFile( dbase_rec, path, FALSE );

	switch( dbase_rec->error )
	{
		case DBASE_NO_ERROR:
			break;
		case DBASE_FILE_ERROR:
			Message(msgs[Msg_DB_Error_1-1]);	// FILE ERROR
			break;
		case DBASE_EMPTY_ERROR:
			Message(msgs[Msg_DB_Error_2-1]);	// FILE EMPTY
			break;
		case DBASE_MEMORY_ERROR:
			Message(msgs[Msg_DB_Error_3-1]);	// MEMORY ERROR
			break;
		default:
			Message(msgs[Msg_DB_Error_4-1]);	// UNEXPECTED ERROR
			break;
	}

	return(retval);
}

/******** E O F ********/
