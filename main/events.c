#include "nb:pre.h"

/**** defines ****/

#define KEYBUFSIZE 20

/**** externals ****/

extern struct CapsPrefs CPrefs;
extern struct EventData CED;
extern struct eventHandlerInfo EHI;
extern struct MenuRecord **page_MR;
extern struct MenuRecord **script_MR;
extern struct MsgPort *capsPort;
extern struct Window *pageWindow;
extern struct Window *scriptWindow;
extern TEXT pageScreenTitle[255];
extern UWORD palettes[];
extern struct Screen *pageScreen;
extern UWORD chip gui_pattern[];
extern struct Library *medialinkLibBase;
extern UBYTE **msgs;   
extern BOOL blockScript;
extern struct RendezVousRecord rvrec;

/**** gadgets ****/

extern struct GadgetRecord Script_GR[];

/**** functions ****/

/******** HandleIDCMP() ********/

void HandleIDCMP(struct Window *handleWindow, BOOL dragBar, WORD *ascii)
{
struct IntuiMessage *message;
int menu, item;
struct IntuiMessage localMsg;
static ULONG prev_Seconds = 0L;
static ULONG prev_Micros  = 0L;
struct RasInfo *RI;

	CED.menuNum 		= 0;
	CED.itemNum 		= 0;
	CED.itemFlags 	= 0;
	CED.Class 			= 0;
	CED.extraClass 	= 0;
	CED.Code 				= 0;
	CED.Qualifier 	= 0;
	CED.IAddress 		= NULL;
	CED.Seconds			= 0;
	CED.Micros 			= 0;
	CED.MouseX 			= 1;
	CED.MouseY 			= 1;

	while(message = (struct IntuiMessage *)GetMsg(capsPort))
	{
		CopyMem(message, &localMsg, sizeof(struct IntuiMessage));
		ReplyMsg((struct Message *)message);

		/**** copy interesting fields ****/

		CED.Class				= localMsg.Class;
		CED.Code				= localMsg.Code;
		CED.Qualifier		= localMsg.Qualifier;
		if (CED.Class==IDCMP_RAWKEY || CED.Class==IDCMP_GADGETDOWN ||
				CED.Class==IDCMP_GADGETUP)
			CED.IAddress		= (APTR)localMsg.IAddress;
		CED.Seconds			= localMsg.Seconds;
		CED.Micros			= localMsg.Micros;
		CED.MouseX			= localMsg.MouseX;
		CED.MouseY			= localMsg.MouseY;

		if (EHI.activeScreen==STARTSCREEN_PAGE && handleWindow==pageWindow)
		{
			RI = pageScreen->ViewPort.RasInfo;
			CED.MouseY += RI->RyOffset;
		}

		/**** check if user double clicked ****/

		if ( CED.Class == IDCMP_MOUSEBUTTONS && CED.Code == SELECTDOWN )
		{
			if ( MyDoubleClick(prev_Seconds, prev_Micros, CED.Seconds, CED.Micros) )
				CED.extraClass = DBLCLICKED;
			prev_Seconds = CED.Seconds;
			prev_Micros  = CED.Micros;
		}
		if (CED.Class != IDCMP_MOUSEBUTTONS)
		{
			prev_Seconds = 0L;
			prev_Micros  = 0L;
		}

		if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code != MENUDOWN)
		{
			if ( dragBar && CED.Code==SELECTDOWN )
				UA_CheckIfDragged(handleWindow,&CED);

			CheckIfDepthWasClicked(handleWindow);
		}
		else
		{
			/**** menu item key short cuts ****/ 

			if( CED.Class==IDCMP_RAWKEY )
			{
				if ( CED.Qualifier & IEQUALIFIER_CONTROL )	// MAGIC KEYS
				{
					if ( CED.Code==RAW_ENTER )
					{
						MemWatcher();
						CED.Class=0;
						CED.Code=0;
					}
					else if (	CED.Code == RAW_HELP )
					{
						if ( CPrefs.fastMenus )
							CPrefs.fastMenus=FALSE;
						else
							CPrefs.fastMenus=TRUE;
						CED.Class=0;
						CED.Code=0;
					}
				}
				else if ( CED.Qualifier&IEQUALIFIER_RCOMMAND )
				{
					if ( !blockScript )
					{
						if (EHI.activeScreen == STARTSCREEN_PAGE)
							GetMenuEquivalent(&menu, &item, &localMsg, page_MR);
						else
							GetMenuEquivalent(&menu, &item, &localMsg, script_MR);
						if (menu!=-1 && item!=-1)
						{
							CED.menuNum = menu;
							CED.itemNum = item;
							CED.Class = IDCMP_MENUPICK;
							CED.Qualifier=0;
						}
					}
				}

				//if ( ascii )
				//	*ascii = RawKeyToASCII(CED.Code);
			}

			/**** choosing an item from a menu ****/

	 		else if (	CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==MENUDOWN && !blockScript )
			{
				if (EHI.activeScreen == STARTSCREEN_PAGE)
				{
					ScreenAtNormalPos();
					UA_SetMenuColors(&rvrec,pageWindow);
					Monitor_Menu(pageWindow, &menu, &item, page_MR);
					UA_ResetMenuColors(&rvrec,pageWindow);
				}
				else
				{
					//SetStandardColors(scriptWindow);
					Monitor_Menu(scriptWindow, &menu, &item, script_MR);
					//ResetStandardColors(scriptWindow);
				}
				if (menu!=-1 && item!=-1)
				{
					CED.menuNum = menu;
					CED.itemNum = item;
					CED.Class = IDCMP_MENUPICK;
				}
			}

			/**** clicks on an intuition gadget ****/

			else if (CED.Class==IDCMP_GADGETDOWN || CED.Class==IDCMP_GADGETUP)
			{
				CED.extraClass = CED.Class;
				CED.Class = IDCMP_MOUSEBUTTONS;
			}
		}		
	}
}

/******** GetMenuEquivalent() ********/

void GetMenuEquivalent(int *menu, int *item, struct IntuiMessage *msg, struct MenuRecord **MR)
{
USHORT key;
int i,j;
BOOL mustHaveShift=FALSE;

	if (	(CED.Qualifier & IEQUALIFIER_LSHIFT) ||
				(CED.Qualifier & IEQUALIFIER_RSHIFT) )
		mustHaveShift=TRUE;

	DoKeys(msg, &key);
	if (key > 0x5f)
		key -= 0x20;	/* convert lowercase typed key to uppercase, as stored in menu */

	if ( mustHaveShift )
	{
		if ( key==0x29 ) key=0x30;
		else if ( key==0x21 ) key=0x31;
		else if ( key==0x40 ) key=0x32;
		else if ( key==0x23 ) key=0x33;
		else if ( key==0x24 ) key=0x34;
		else if ( key==0x25 ) key=0x35;
		else if ( key==0x5e ) key=0x36;
		else if ( key==0x26 ) key=0x37;
		else if ( key==0x2a ) key=0x38;
		else if ( key==0x28 ) key=0x39;
	}

	if ( key==0x2f )	// convert '/' to '?'
		key=0x3f;

	if (key != 0)
	{
		for(j=0; j<NUMMENUS; j++)
		{
			for(i=0; i<16; i++)
			{
				if (	MR[j]->commandKey[i] == (int)key &&
							MR[j]->shifted[i] == mustHaveShift )
				{
					if ( MR[j]->disabled[i] )
					{
						*menu=-1;
						*item=-1;
					}
					else
					{
						*menu=j;
						*item=i;
					}
					return;
				}
			}
		}
	}
	*menu=-1;
	*item=-1;
	return;
}

/******** DoKeys() ********/

void DoKeys(struct IntuiMessage *msg, USHORT *key)
{
LONG numchars;
UBYTE buffer[KEYBUFSIZE+5];
int i;

	for(i=0; i<KEYBUFSIZE; i++)
		buffer[i] = 0x20;

	numchars = EventsDeadKeyConvert(msg, &buffer[0], KEYBUFSIZE, 0L);

	if (numchars > 0L)
		*key=(USHORT)buffer[0];
	else
		*key=0;
}

/******** EventsDeadKeyConvert() ********/

LONG EventsDeadKeyConvert(struct IntuiMessage *msg,
													UBYTE *kbuffer, LONG kbsize, struct KeyMap *kmap)
{
static struct InputEvent ievent = { NULL, IECLASS_RAWKEY, 0, 0, 0 };

	if (msg->Class != IDCMP_RAWKEY)
		return(-2);

	ievent.ie_Code = msg->Code;
	ievent.ie_Qualifier = msg->Qualifier;
	ievent.ie_position.ie_addr = *((APTR*)msg->IAddress);	/* one of the last surviving APTRs ? */

	return(RawKeyConvert(&ievent, kbuffer, kbsize, kmap));
}

/******** MyDoubleClick() ********/

BOOL MyDoubleClick(	ULONG prev_Seconds, ULONG prev_Micros,
										ULONG Seconds, ULONG Micros )
{
static SHORT MouseX=-1;
static SHORT MouseY=-1;

	if (MouseX==-1)
	{
		MouseX = CED.MouseX;
		MouseY = CED.MouseY;
		return(FALSE);
	}

	if (	AbsWORD((WORD)MouseX,(WORD)CED.MouseX)>16 ||
				AbsWORD((WORD)MouseY,(WORD)CED.MouseY)>4 )
	{
		MouseX = CED.MouseX;
		MouseY = CED.MouseY;
		return(FALSE);
	}

	return( DoubleClick(prev_Seconds, prev_Micros, Seconds, Micros) );
}

/******** doStandardWait() ********/
/*
 * This one allows for clicking outside window (used e.g. for windef, transitions and
 * interact windows.
 * 
 */

void doStandardWait(struct Window *waitWindow)
{
ULONG signals;

	signals = Wait(SIGNALMASK);
	if ( signals & SIGNALMASK )
		HandleIDCMP(waitWindow,TRUE,NULL);
}

/******** CheckIfDepthWasClicked() ********/

void CheckIfDepthWasClicked(struct Window *handleWindow)
{
	if (	handleWindow->MouseX > (handleWindow->Width-20) &&
				handleWindow->MouseY < MHEIGHT )
	{
		if (handleWindow==scriptWindow)
		{
			UA_HiliteButton(scriptWindow, &Script_GR[6]);
			OpenWorkBench();
			MyScreenToBack(handleWindow->WScreen);
		}
		else if (	handleWindow==pageWindow &&
							handleWindow->MouseX > (handleWindow->Width-2) &&
							handleWindow->MouseY < 2 )
		{
			OpenWorkBench();
			MyScreenToBack(handleWindow->WScreen);
		}
	}
}

/******** RawKeyToASCII() ********/

WORD RawKeyToASCII(USHORT Code)
{
UBYTE buffer[10];
struct InputEvent ie;

	ie.ie_Class = IECLASS_RAWKEY;
	ie.ie_SubClass = 0;
	ie.ie_Code = Code;
	ie.ie_Qualifier = CED.Qualifier;

	if ( MapRawKey(&ie,buffer,2,NULL)==1 )
		return( (WORD)buffer[0] );
	else
		return( 0 );
}

/******** E O F ********/
