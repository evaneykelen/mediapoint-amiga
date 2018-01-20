#include "nb:pre.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Screen *pageScreen;
extern struct Library *medialinkLibBase;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct EditSupport **Clipboard_SL;
extern struct EditSupport **Undo_SL;
extern struct Screen **DA_Screens;
extern struct TextFont *largeFont;
extern struct TextFont *textFont;
extern UBYTE **msgs;   
extern struct ColorMap *undoCM;
extern int lastUndoableAction;
extern UWORD chip gui_pattern[];
extern struct EditWindow backEW;
extern struct EditSupport backES;

/**** globals ****/

BOOL fromTextEdit=FALSE;

/**** gadgets ****/

extern struct GadgetRecord ImportType_GR[];

/**** functions ****/

/******** ProcessImport() ********/

void ProcessImport(void)
{
int retval, remapIt;
BOOL resizeIt;

	/**** get user's input (import what and how) ****/

	CopyCMtoCM(pageScreen->ViewPort.ColorMap, undoCM);

	retval = DoImport(&resizeIt, &remapIt);

	/**** do actual import of picture, screen or text ****/

	DrawAllHandles(LEAVE_ACTIVE);

	if (retval==0)				// background
		ImportABackground(resizeIt, remapIt, NULL, NULL);
	else if (retval==1)		// picture
		ImportAPicture(resizeIt, remapIt);
	else if (retval==2)		// screen
		ImportAScreen(resizeIt, remapIt, NULL);	// NULL means no screen supplied as in datatypes
	else if (retval==3)		// text
		ImportAText();
	else if (retval==4)		// datatype
		ImportADataType(resizeIt, remapIt);

	DrawAllHandles(LEAVE_ACTIVE);
}

/******** DoImport() ********/

int DoImport(BOOL *resizeIt, int *remapIt)
{
struct Window *window;
BOOL loop, retVal, dblClicked=FALSE;
int import_choice, prev_choice, ID;
static BOOL resize=FALSE;
//static BOOL remap=FALSE;
int i, numActive, numWdw;

	numActive=0;	
	numWdw=0;

	/**** count number of active window ****/

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditSupportList[i]!=NULL)
		{
			numWdw++;
			if ( EditSupportList[i]->Active )
				numActive++;
		}
	}

	/**** init vars ****/

	// START SMARTSTEP

	if ( backES.photoOpts & SIZE_PHOTO || backES.photoOpts & MOVE_PHOTO )
		import_choice = 4;	// import a pic, there's already a bgnd
	else if (	!(backES.photoOpts & SIZE_PHOTO || backES.photoOpts & MOVE_PHOTO) )
	{
		if ( numActive!=0 )
			import_choice = 4;	// although there's no bgnd, there IS a wdw active
		else
			import_choice = 3;
	}
	else
		import_choice = 3;

	if ( fromTextEdit )
		import_choice = 6;
	fromTextEdit = FALSE;

	// END SMARTSTEP

	retVal = FALSE;
	loop   = TRUE;

	/**** open a window ****/

	window = UA_OpenRequesterWindow(pageWindow,ImportType_GR,STDCOLORS);
	if (!window)
	{
		UA_WarnUser(150);
		return(-1);
	}

	/**** render gadget ****/

	UA_DrawGadgetList(window, ImportType_GR);

	if (GfxBase->LibNode.lib_Version < 39)
		UA_DisableButton(window, &ImportType_GR[7], gui_pattern);	// datatype

	if (numWdw==MAXEDITWINDOWS && numActive==0)	// max # windows reached
	{
		UA_DisableButton(window, &ImportType_GR[4], gui_pattern);	// pict
		UA_DisableButton(window, &ImportType_GR[5], gui_pattern);	// scr
		UA_DisableButton(window, &ImportType_GR[6], gui_pattern);	// text
		UA_DisableButton(window, &ImportType_GR[7], gui_pattern);	// datatype
	}

	UA_InvertButton(window, &ImportType_GR[import_choice]);
	prev_choice = import_choice;	// used for de-hiliting radio buttons

	if (resize)
		UA_InvertButton(window, &ImportType_GR[10]);

	//if (remap)
	//	UA_InvertButton(window, &ImportType_GR[11]);

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if (CED.extraClass == DBLCLICKED)
			dblClicked=TRUE;
		else
			dblClicked=FALSE;

		if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, ImportType_GR, &CED);
			switch(ID)
			{
				case 3:	// background
				case 4:	// picture
				case 5:	// screen
				case 6:	// text
				case 7:	// datatype
					UA_InvertButton(window, &ImportType_GR[prev_choice]);
					import_choice = ID; 
					prev_choice = import_choice;
					UA_InvertButton(window, &ImportType_GR[import_choice]);
					if ( dblClicked )
						goto do_ok;
					break;

				case 8:		// OK
do_ok:
					UA_HiliteButton(window, &ImportType_GR[8]);
					loop=FALSE;
					retVal=TRUE;
					break;

				case 9:
do_cancel:
					UA_HiliteButton(window, &ImportType_GR[9]);
					loop=FALSE;
					retVal=FALSE;
					break;

				case 10:
					UA_InvertButton(window, &ImportType_GR[ID]);
					if (resize==FALSE)
						resize=TRUE;
					else
						resize=FALSE;
					break;

#if 0
				case 11:
					UA_InvertButton(window, &ImportType_GR[ID]);
					if (remap==FALSE)
						remap=TRUE;
					else
						remap=FALSE;
					break;
#endif
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

	UA_CloseRequesterWindow(window,STDCOLORS);

	UA_EnableButtonQuiet( &ImportType_GR[4] );
	UA_EnableButtonQuiet( &ImportType_GR[5] );
	UA_EnableButtonQuiet( &ImportType_GR[6] );
	UA_EnableButtonQuiet( &ImportType_GR[7] );

	*resizeIt = resize;
	*remapIt = FALSE;	//remap;

	if (retVal)
		return( import_choice-3 );

	return(-1);
}

/******** E O F ********/
