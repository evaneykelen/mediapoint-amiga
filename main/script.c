#include "nb:pre.h"
#include "minc:types.h"
#include "minc:system.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"
#include <graphics/videocontrol.h>	// not in GST

extern struct List *LoadMLSegments(struct List *, struct FileLock *, ULONG);
extern void UnLoadMLSegments(struct List *SegList, ULONG);
extern BOOL playScript(struct ScriptInfoRecord *, UBYTE, BOOL, BOOL, BOOL);	// see ph:playscript.c
extern BOOL SpecialAllocMLSystem(void);
extern void SpecialFreeMLSystem(void);
extern void ProcessDeInitializer(void);

/**** defines ****/

#define HIT_OBJECT_LIST	0
#define HIT_XAPP_LIST		2
#define HIT_PLAY				4
#define HIT_PARENT			5
#define HIT_EDIT				7
#define HIT_SHOW				8

/**** externals ****/

extern ULONG allocFlags;
extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct Screen *scriptScreen;
extern struct Window *scriptWindow;
extern struct Window *pageWindow;
extern struct MenuRecord **script_MR;
extern struct Screen **DA_Screens;
extern struct Process *process;
extern struct Gadget ScriptSlider1;
extern struct Gadget ScriptSlider2;
extern ULONG numEntries1, numDisplay1, numEntries2, numDisplay2;
extern LONG topEntry1, topEntry2;
extern struct Library *medialinkLibBase;
extern struct Library *MLMMULibBase;
extern struct CapsPrefs CPrefs;
extern struct ObjectInfo ObjectRecord;
extern struct List *undoListPtr;
extern struct Document scriptDoc;
extern struct TextFont *smallFont;
extern struct TextFont *largeFont;
extern struct List *SegList;		// defined in proccont.c, initialised in Initsegment.c
extern UBYTE **msgs;   
extern struct List **clipLists;
extern struct List **undoLists;
extern struct ScriptNodeRecord *editSNR;
extern BOOL BlockAllInput;
extern struct Window *playWindow;
extern UWORD chip gui_pattern[];
extern BOOL blockScript;
extern TEXT MRO_Script[];
extern TEXT MRO_Page[];
extern struct Screen *playScreen;
extern struct RendezVousRecord rvrec;
extern TEXT newScript[];
extern char *scriptCommands[];
extern MLSYSTEM	*MLSystem;
extern struct BitMap gfxBitMap;
extern struct RastPort gfxRP;

/**** static globals ****/

STATIC demoMsgDone=FALSE;
STATIC UBYTE kept_mousePointer;
STATIC struct FileInfoBlock __aligned fib1;
STATIC BOOL gotDate=FALSE;

/**** globals ****/

struct ScriptNodeRecord *fromSNR=NULL;	// see also proccont.c

/**** gadgets ****/

extern struct GadgetRecord Script_GR[];

/**** functions ****/

/******** HandleScriptEvents() ********/
/*
 * output: 0 (also 0 if DO_OTHER), QUIT_MEDIALINK
 *
 */

int HandleScriptEvents(void)
{
int mode=0, editIt, numSel;
ULONG signals;
BOOL playable=FALSE;
struct ScriptNodeRecord *this_node;
struct IntuiMessage *message;
WORD ascii;
BOOL didASCII;

	while(message = (struct IntuiMessage *)GetMsg(capsPort))
		ReplyMsg((struct Message *)message);

	editSNR = NULL;

	EnableMenu(script_MR[SCREEN_MENU], SCREEN_PAGE);
	DisableMenu(script_MR[SCREEN_MENU], SCREEN_SCRIPT);

	ScreenToFront(scriptScreen);
	ActivateWindow(scriptWindow);

	playable = GhostPlay();

	if (CPrefs.ScriptScreenModes & LACE)
		DoubleEffBM(TRUE);
	else
		DoubleEffBM(FALSE);

	if ( scriptDoc.opened )
	{
		DrawObjectList(topEntry1, TRUE, TRUE);
		SetScriptMenus();
		//gotDate = GetScriptTime(&scriptDoc,&fib1);
	}
	//else
	//	gotDate = FALSE;

	/**** event handler ****/

	// DO THIS FOR THE DEMO VERSION

#ifdef USED_FOR_DEMO
	if ( !demoMsgDone )
	{
		demoMsgDone=TRUE;
		ShowDemoMsg();
	}
#endif

	// DO THIS IF YOU WANT TO SHOW A PERSONALIZED VERSION MESSAGE

	if ( !demoMsgDone )
	{
		demoMsgDone=TRUE;
		ShowPersonalized();
	}

	// DO THIS IF YOU WANT TO SHOW A BETA WARNING

	if ( !demoMsgDone )
	{
		demoMsgDone=TRUE;
		ShowBeta();
	}

	while(mode==0)
	{
		signals = Wait(SIGNALMASK);
		if (signals & SIGNALMASK)
		{
			HandleIDCMP(scriptWindow,FALSE,NULL);

			playable = GhostPlay();

			didASCII=FALSE;

			if (CED.Class == IDCMP_MENUPICK)
			{
				mode = CheckScriptMenu();
				playable = GhostPlay();
			}
			else if (CED.Class == IDCMP_RAWKEY)
			{
				// ascii keys not protected by scriptDoc.opened

				ascii = RawKeyToASCII(CED.Code);

				switch(ascii)
				{
					case 'o':	// open
						didASCII=TRUE;
						if ( !script_MR[FILE_MENU]->disabled[FILE_OPEN] )
						{
							CED.menuNum = FILE_MENU;
							CED.itemNum = FILE_OPEN;
							mode = CheckScriptMenu();
						}
						break;

					case '0':	// prefs
						didASCII=TRUE;
						if ( !script_MR[SCREEN_MENU]->disabled[SCREEN_PREFS] )
						{
							CED.menuNum = SCREEN_MENU;
							CED.itemNum = SCREEN_PREFS;
							CheckScriptMenu();
						}
						break;
				}

				// rawkeys not protected by scriptDoc.opened

				if ( !didASCII )
				{
					switch(CED.Code)
					{
						case 0x00:	// raw ~ (toggle script <==> page )
							if ( !script_MR[SCREEN_MENU]->disabled[SCREEN_PAGE] )
							{
								if (	(CED.Qualifier & IEQUALIFIER_LSHIFT ||
											CED.Qualifier & IEQUALIFIER_RSHIFT) &&
											!script_MR[FILE_MENU]->disabled[FILE_SAVE] )
								{
									CED.menuNum = FILE_MENU;
									CED.itemNum = FILE_SAVE;
									mode = CheckScriptMenu();
								}
								CED.menuNum = SCREEN_MENU;
								CED.itemNum = SCREEN_PAGE;
								mode = CheckScriptMenu();
							}
							break;

						case RAW_F10:	// UNDOCUMENTED !!!
							if ( CED.Qualifier & IEQUALIFIER_CONTROL )
							{
								UnLoadMLSegments(SegList, OTT_AFTERLOAD | OTT_PRELOAD);
								SegList = NULL;
								SegList = LoadMLSegments(SegList,CPrefs.appdirLock, OTT_PRELOAD);
								//ClipBlit(&gfxRP,0,0,scriptWindow->RPort,0,0,256,75,0xc0);
							}
							break;
					}
				}

				// ascii keys protected by scriptDoc.opened

				if (scriptDoc.opened)
				{
					switch(ascii)
					{
						case '1':	// goto page
							if ( !(CED.Qualifier & IEQUALIFIER_NUMERICPAD) )
							{
								didASCII=TRUE;
								if ( !script_MR[SCREEN_MENU]->disabled[SCREEN_PAGE] )
								{
									CED.menuNum = SCREEN_MENU;
									CED.itemNum = SCREEN_PAGE;
									mode = CheckScriptMenu();
								}
							}
							break;

						case 'e':	// edit
							didASCII=TRUE;
							numSel=0;
							this_node = CountNumSelected(ObjectRecord.firstObject, &numSel);
							if ( this_node && numSel==1 )
							{
								if ( this_node->nodeType==TALK_PAGE )
								{
									editSNR = this_node;
									ReallyDeselectAllButThisOne(this_node);	
									mode=DO_OTHER;
								}
							}
							break;

						case 's':	// show
							didASCII=TRUE;
							numSel=0;
							this_node = CountNumSelected(ObjectRecord.firstObject, &numSel);
							if ( this_node && numSel==1 )
							{
								UA_HiliteButton(scriptWindow, &Script_GR[HIT_SHOW]);
								do_small_play(this_node);
							}
							break;

						case 'i':	// info
							didASCII=TRUE;
							numSel=0;
							this_node = CountNumSelected(ObjectRecord.firstObject, &numSel);
							if ( this_node && numSel==1 )
								ShowXappInfo(this_node);
							break;

						case 'r':	// record
							didASCII=TRUE;
							CED.Qualifier = IEQUALIFIER_LSHIFT;	// fake a shift+click Play event
						case 'p':	// play
							didASCII=TRUE;
							if ( playable )
								DoPlay(FALSE);
							break;

						case 'f':	// play from
							didASCII=TRUE;
							if (	ObjectRecord.objList != ObjectRecord.scriptSIR.allLists[0] &&
										ObjectRecord.numObjects!=0 && playable )
								DoPlay(TRUE);
							break;

						case 't':	// transitions
							didASCII=TRUE;
							if (	!blockScript &&
										ObjectRecord.objList != ObjectRecord.scriptSIR.allLists[0] ) // root
								DoTransitions(1);
							break;

						case 'd':	// duration/delay
							didASCII=TRUE;
							if ( !blockScript )
								DoDuration(1);
							break;

						case 'u':	// undo
						case 'z':	// undo
							didASCII=TRUE;
							if ( !script_MR[EDIT_MENU]->disabled[EDIT_UNDO] )
							{
								CED.menuNum = EDIT_MENU;
								CED.itemNum = EDIT_UNDO;
								mode = CheckScriptMenu();
								playable = GhostPlay();
							}
							break;

						case 'x':	// cut
							didASCII=TRUE;
							if ( !script_MR[EDIT_MENU]->disabled[EDIT_CUT] )
							{
								CED.menuNum = EDIT_MENU;
								CED.itemNum = EDIT_CUT;
								mode = CheckScriptMenu();
								playable = GhostPlay();
							}
							break;

						case 'c':	// cut
							didASCII=TRUE;
							if ( !script_MR[EDIT_MENU]->disabled[EDIT_COPY] )
							{
								CED.menuNum = EDIT_MENU;
								CED.itemNum = EDIT_COPY;
								mode = CheckScriptMenu();
								playable = GhostPlay();
							}
							break;

						case 'v':	// paste
							didASCII=TRUE;
							if ( !script_MR[EDIT_MENU]->disabled[EDIT_PASTE] )
							{
								CED.menuNum = EDIT_MENU;
								CED.itemNum = EDIT_PASTE;
								mode = CheckScriptMenu();
								playable = GhostPlay();
							}
							break;

						case 'b':	// buttons
							didASCII=TRUE;
							if ( !script_MR[SMISC_MENU]->disabled[SMISC_LOCALEVENTS] )
							{
								CED.menuNum = SMISC_MENU;
								CED.itemNum = SMISC_LOCALEVENTS;
								CheckScriptMenu();
							}
							break;

						case 'm':	// script manager
							didASCII=TRUE;
							if ( !script_MR[SMISC_MENU]->disabled[SMISC_SCRIPTMANAGER] )
							{
								CED.menuNum = SMISC_MENU;
								CED.itemNum = SMISC_SCRIPTMANAGER;
								CheckScriptMenu();
							}
							break;
					}
				}

				// rawkeys protected by scriptDoc.opened

				if (scriptDoc.opened)
				{
					if ( !didASCII )
					{
						DoSelAll();	// also for F1...F6 keys

						switch(CED.Code)
						{
							case RAW_BACKSPACE:
								if ( !script_MR[EDIT_MENU]->disabled[EDIT_CLEAR] )
								{
									CED.menuNum = EDIT_MENU;
									CED.itemNum = EDIT_CLEAR;
									CheckScriptMenu();
									playable = GhostPlay();
								}
								break;
	
							case RAW_DELETE:
								if (	(ObjectRecord.objList != ObjectRecord.scriptSIR.allLists[0]) &&
											!script_MR[EDIT_MENU]->disabled[EDIT_CUT] )
								{
									CED.menuNum = EDIT_MENU;
									CED.itemNum = EDIT_CUT;
									CheckScriptMenu();
									playable = GhostPlay();
								}
								break;

							case RAW_F7:
								if (	( ObjectRecord.scriptSIR.timeCodeFormat!=TIMEFORMAT_HHMMSS ||
												ObjectRecord.scriptSIR.listType==TALK_STARTPAR ) &&
										ObjectRecord.objList != ObjectRecord.scriptSIR.allLists[0] )
								{
									TimeCodeTweaker();
									DrawObjectList(-1, TRUE, TRUE);	// only redraw object name part
								}
								break;

							case RAW_F8:
								this_node = CountNumSelected(ObjectRecord.firstObject, &numSel);
								if ( this_node && numSel>=1 )
									FixedVarPath();
								break;

							default:
								dokeyScrolling();
								break;
						}
					}
				}
			}
			else if (CED.Class == IDCMP_MOUSEBUTTONS)
			{
				if (scriptDoc.opened)
				{
					editIt=FALSE;
					doScriptMouseButtons(&editIt);
					if ( editIt )
						mode=DO_OTHER;
				}
			}
		}
	}

	if (mode==DO_OTHER)
	{
		SetSpriteOfActWdw(SPRITE_BUSY);
		return(0);
	}
	else if (mode==DO_QUIT)
		return(QUIT_MEDIALINK);
	else
		return(0);
}

/******** doScriptMouseButtons() ********/

BOOL doScriptMouseButtons(int *editIt)
{
int ID,dummy,pos,numSel;
SNRPTR this_node;
BOOL dragMove=TRUE, objHit=FALSE;

	if ( ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS )
	{
		if (CPrefs.showDays)
			pos=283;
		else
			pos=309;
	}
	else
		pos=299;

	if ( blockScript ) pos += (418-pos);

	if (CED.extraClass==IDCMP_GADGETDOWN || CED.extraClass==IDCMP_GADGETUP)
	{
		doScriptEdSlider();
	}
	else if (CED.Code==SELECTDOWN)
	{
		if ( !blockScript && CED.MouseX < 4 )	// clicked beside object window
		{
			if ( !UA_IsGadgetDisabled(&Script_GR[5]) )
				processScriptParent();
			ID=-1;
		}
		else
		{
			ID = UA_CheckGadgetList(scriptWindow, Script_GR, &CED);
			if ( blockScript && ID!=HIT_OBJECT_LIST )
				ID=-1;
		}

		switch(ID)
		{
			case HIT_OBJECT_LIST:
				if ( !blockScript && CED.MouseX < 47 )	// clicked on object icon
				{
					this_node = (struct ScriptNodeRecord *)WhichObjectWasClicked((int)topEntry1,&dummy);
					if ( this_node != NULL )
					{
						if ( CED.Qualifier & IEQUALIFIER_LSHIFT || CED.Qualifier & IEQUALIFIER_RSHIFT )
						{
							// Shift click means edit this page
							if (this_node->nodeType==TALK_PAGE)
							{
								*editIt = TRUE;
								editSNR = this_node;
								ReallyDeselectAllButThisOne(this_node);
							}
						}
						else
						{
							// return true means edit this page
							if ( processDblClick((int)topEntry1,this_node) )	// func name misleading!
							{
								if (this_node->nodeType==TALK_PAGE)
								{
									*editIt = TRUE;
									editSNR = this_node;
									ReallyDeselectAllButThisOne(this_node);
								}
							}
							scriptDoc.modified=TRUE;
							dragMove = FALSE;
						}
					}
					CED.extraClass = 0;
				}
				else if ( !blockScript &&
									(CED.MouseX >= pos) && (CED.MouseX < (pos+21)) ) // effect icon clicked
				{
					DoTransitions(2);
					scriptDoc.modified=TRUE;
					dragMove = FALSE;
					CED.extraClass = 0;
				}
				else if (	!blockScript &&
									(CED.MouseX > (pos+21)) && CED.MouseX < 418 )	// duration clicked
				{
					DoDuration(2);
					dragMove = FALSE;
					CED.extraClass = 0;
				}
				else if ( CED.MouseX < pos )
				{
					this_node = (struct ScriptNodeRecord *)WhichObjectWasClicked((int)topEntry1, &dummy);
					if (this_node != NULL)
					{
						if (	!blockScript &&
									(CED.Qualifier & IEQUALIFIER_LSHIFT) &&
									(CED.Qualifier & IEQUALIFIER_LALT) &&
									(CED.Qualifier & IEQUALIFIER_CONTROL) )	// DEBUGGING OBJECT INSPECTOR
						{
							ShowSNR(this_node, ObjectRecord.firstObject);
							SetByteBit(&this_node->miscFlags, OBJ_SELECTED);
							SetByteBit(&this_node->miscFlags, OBJ_NEEDS_REFRESH);
							DrawObjectList(-1, FALSE, TRUE);
							scriptDoc.modified=TRUE;
							SetScriptMenus();
						}
						else
						{
							FindSelectedIcon((int)topEntry1);
							objHit=TRUE;
						}
					}
				}
				break;

			case HIT_XAPP_LIST:
				dragMove = FALSE;
				if ( CED.Qualifier&IEQUALIFIER_LSHIFT || CED.Qualifier&IEQUALIFIER_RSHIFT )
				{
					if (numEntries1>numDisplay1 && topEntry1!=(numEntries1-numDisplay1))
					{
						topEntry1 = numEntries1-numDisplay1;
						UA_SetPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);
						DrawObjectList(topEntry1, TRUE, TRUE);
					}
					// first -1 means calc which icon was hit in xapp list
					PasteDraggedIcon(-1,(int)topEntry1,-1,-1);
				}
				else
					StartIconDragging((int)topEntry1, (int)topEntry2, 1);	// 1 means drag object
				if (ObjectRecord.numObjects==0)
					DisableMenu(script_MR[EDIT_MENU], EDIT_SELECTALL);
				else
				{
					if ( ObjectRecord.objList != ObjectRecord.scriptSIR.allLists[0] )
					{
						EnableMenu(script_MR[EDIT_MENU], EDIT_CUT);
						EnableMenu(script_MR[EDIT_MENU], EDIT_COPY);
						EnableMenu(script_MR[EDIT_MENU], EDIT_CLEAR);
					}
					EnableMenu(script_MR[EDIT_MENU], EDIT_SELECTALL);
				}
				scriptDoc.modified=TRUE;
				CED.extraClass = 0;
				break;

			case HIT_PLAY:
				dragMove = FALSE;
				DoPlay(FALSE);
				CED.extraClass = 0;
				break;

			case HIT_PARENT:
				dragMove = FALSE;
				UA_HiliteButton(scriptWindow, &Script_GR[ID]);
				processScriptParent();
				CED.extraClass = 0;
				break;

			case HIT_EDIT:
				UA_HiliteButton(scriptWindow, &Script_GR[ID]);
				this_node = CountNumSelected(ObjectRecord.firstObject, &numSel);
				if ( numSel != 1 )
				{
					if (numEntries1>numDisplay1 && topEntry1!=(numEntries1-numDisplay1))
					{
						topEntry1 = numEntries1-numDisplay1;
						UA_SetPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);
						DrawObjectList(topEntry1, TRUE, TRUE);
					}
					PasteDraggedIcon(0,(int)topEntry1,-1,-1);
					this_node = CountNumSelected(ObjectRecord.firstObject, &numSel);
				}
				if ( this_node && this_node->nodeType==TALK_PAGE )
				{
					*editIt = TRUE;
					editSNR = this_node;
					ReallyDeselectAllButThisOne(this_node);
				}
				break;

			case HIT_SHOW:
				UA_HiliteButton(scriptWindow, &Script_GR[ID]);
				this_node = CountNumSelected(ObjectRecord.firstObject, &numSel);
				if ( this_node && numSel == 1 )
					do_small_play(this_node);
				break;
		}

		/**** drag move clicked and (de)selected) object ****/

		if (	dragMove && CED.MouseY!=0 && CED.MouseX<418 &&
					ObjectRecord.numObjects>1 &&
					ObjectRecord.objList != ObjectRecord.scriptSIR.allLists[0])
		{
			if (DragMoveObjects((int)topEntry1))
			{
				objHit=TRUE;
				scriptDoc.modified=TRUE;
				ObjectRecord.firstObject = (struct ScriptNodeRecord *)ObjectRecord.objList->lh_Head;
				UA_SetPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);
				DrawObjectList(topEntry1, TRUE, TRUE);
				FindSelectedIcon(-1);	// reset Alt selecting
				CED.extraClass = 0;
			}
		}
	}

	if ( !blockScript && CED.extraClass == DBLCLICKED )
	{
		if ( CED.MouseX >= 47 && CED.MouseX < pos )	// dbl-clicked on icon or name
			DoComment(2);
	}

	return(objHit);
}

/******** DoPlay() ********/

void DoPlay(BOOL playFrom)
{
BOOL record=FALSE;
UBYTE scriptTiming;
TEXT scriptPath[SIZE_FULLPATH];

	UA_MakeFullPath(scriptDoc.path,scriptDoc.title,scriptPath);

	UA_InvertButton(scriptWindow, &Script_GR[HIT_PLAY]);

	if ( !ValidateSER(&(ObjectRecord.scriptSIR),TRUE,FALSE) )
	{
		UA_InvertButton(scriptWindow, &Script_GR[HIT_PLAY]);
		return;
	}

	if ( CED.Qualifier & IEQUALIFIER_LSHIFT || CED.Qualifier & IEQUALIFIER_RSHIFT )
	{
		UA_ClearButton(scriptWindow, &Script_GR[HIT_PLAY], AREA_PEN);
		if (CPrefs.ScriptScreenModes & LACE)
			SetFont(scriptWindow->RPort, largeFont);
		UA_DrawText(scriptWindow, &Script_GR[HIT_PLAY], msgs[Msg_Record-1]);
		SetFont(scriptWindow->RPort, smallFont);
		UA_InvertButton(scriptWindow, &Script_GR[HIT_PLAY]);
		Delay(5L);
		record=TRUE;
	}

	if ( do_pre_play_things(FALSE) )
	{
		// be sure our play screen is active

		ActivateWindow(playWindow);

		if ( playFrom )
			fromSNR = GetFirstEffObj();
		else
			fromSNR = (struct ScriptNodeRecord *)ObjectRecord.scriptSIR.allLists[1]->lh_Head;

		if ( !fromSNR )
			fromSNR = (struct ScriptNodeRecord *)ObjectRecord.scriptSIR.allLists[1]->lh_Head;

		scriptTiming = CPrefs.scriptTiming;

		if ( record )
		{
			CPrefs.scriptTiming = 1;	// precise
			playScript( &(ObjectRecord.scriptSIR), 2, TRUE, TRUE, TRUE );		// RECORD
		}
		else
		{
			PlayTheScript();
		}

		CPrefs.scriptTiming = scriptTiming;

		do_post_play_things(FALSE);
	}

	if ( !(allocFlags & SCRIPTINMEM_FLAG) )
	{
		do_Open(&scriptDoc,TRUE,scriptPath);
	}

	/**** if recorded, show new frame codes ****/

	if ( record )
		DrawObjectList(-1, TRUE, TRUE);

	/**** re-render play button ****/

	UA_ClearButton(scriptWindow, &Script_GR[HIT_PLAY], AREA_PEN);
	if (CPrefs.ScriptScreenModes & LACE)
		SetFont(scriptWindow->RPort, largeFont);
	UA_DrawText(scriptWindow, &Script_GR[HIT_PLAY], msgs[Msg_Play-1]);
	SetFont(scriptWindow->RPort, smallFont);

	SetScriptMenus();
}

/******** DoScrolling1() ********/

void DoScrolling1(struct Window *window, struct Gadget *g,
									ULONG numEntries, ULONG numDisplay, LONG *topEntry)
{
	UA_GetPropSlider(window, g, numEntries, numDisplay, (int *)topEntry);
	DrawObjectList((int)*topEntry, TRUE, FALSE);
}

/******** DoScrolling2() ********/

void DoScrolling2(struct Window *window, struct Gadget *g,
									ULONG numEntries, ULONG numDisplay, LONG *topEntry)
{
	UA_GetPropSlider(window, g, numEntries, numDisplay, (int *)topEntry);
	ShowToolIcons(window, *topEntry);
}

/******** doScriptEdSlider() ********/

void doScriptEdSlider(void)
{
ULONG signals;
BOOL loop=TRUE;
struct IntuiMessage *message;
BOOL mouseMoved=FALSE;
int ID;
struct Gadget *g;
LONG f;

	g = (struct Gadget *)CED.IAddress;
	ID = g->GadgetID;

	if ( CED.Qualifier & IEQUALIFIER_LSHIFT || CED.Qualifier & IEQUALIFIER_RSHIFT )
	{
		if ( ID==1 )
		{
			f = ((CED.MouseY - ScriptSlider1.TopEdge) * numEntries1) / ScriptSlider1.Height;
			if ( f < 0 )
				f=0;
			topEntry1 = f;
			if ( (topEntry1+numDisplay1) > numEntries1 )
				topEntry1 = numEntries1-numDisplay1;
			UA_SetPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);
			DrawObjectList(topEntry1, TRUE, FALSE);
		}
		else if ( ID==2 )
		{
			f = ((CED.MouseY - ScriptSlider2.TopEdge) * numEntries2) / ScriptSlider2.Height;
			if ( f < 0 )
				f=0;
			topEntry2 = f;
			if ( (topEntry2+numDisplay2) > numEntries2 )
				topEntry2 = numEntries2-numDisplay2;
			UA_SetPropSlider(scriptWindow, &ScriptSlider2, numEntries2, numDisplay2, topEntry2);
			ShowToolIcons(scriptWindow,topEntry2);
		}
		return;
	}

	/**** always scroll slider (in case of clicking above or beneath knob ****/

	switch(ID)
	{
		case 1:
			DoScrolling1(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, &topEntry1);
			break;
		case 2:
			DoScrolling2(scriptWindow, &ScriptSlider2, numEntries2, numDisplay2, &topEntry2);
			break;
	}

	UA_SwitchMouseMoveOn(scriptWindow);

	while(loop)
	{
		signals = Wait(SIGNALMASK);
		if (signals & SIGNALMASK)
		{
			mouseMoved=FALSE;
			while(message = (struct IntuiMessage *)GetMsg(capsPort))
			{
				CED.Class	= message->Class;
				ReplyMsg((struct Message *)message);
				if ( CED.Class == IDCMP_MOUSEMOVE )
					mouseMoved=TRUE;
				else
					loop=FALSE;
			}
			if (mouseMoved)
			{
				if (ScriptSlider1.Flags & GFLG_SELECTED)
				{
					DoScrolling1(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, &topEntry1);
					loop=TRUE;
				}
				else if (ScriptSlider2.Flags & GFLG_SELECTED)
				{
					DoScrolling2(scriptWindow, &ScriptSlider2, numEntries2, numDisplay2, &topEntry2);
					loop=TRUE;
				}
				else
					loop=FALSE;
			}
		}
	}

	UA_SwitchMouseMoveOff(scriptWindow);
}

/******** CheckScriptMenu() ********/

int CheckScriptMenu(void)
{
int mode = 0, numSel;
struct ScriptNodeRecord *this_node;
BOOL saved, go_on;

	switch(CED.menuNum)
	{
		case DA_MENU:
			switch(CED.itemNum)
			{
				case DA_ABOUT:
					ShowAbout();
					ActivateWindow(scriptWindow);
					break;

				case 1: 
				case 2:
				case 3:
				case 4:
				case 5:
					if ( !ExecuteDA(scriptWindow, CED.itemNum-1) )
					{
						ScreenToFront(scriptScreen);
						ActivateWindow(scriptWindow);
					}
					break;

				case 6:
				case 7:
				case 8:
				case 9:
				case 10:
					if ( MRO_Script[ (CED.itemNum-6)*SIZE_FULLPATH ] )
					{
						if ( do_Open(&scriptDoc,TRUE,&MRO_Script[ (CED.itemNum-6)*SIZE_FULLPATH ]) )
							gotDate = GetScriptTime(&scriptDoc,&fib1);
					}
					break;
			}
			break;

		case FILE_MENU:
			switch(CED.itemNum)
			{
				case FILE_NEW:
					do_New(&scriptDoc);
					gotDate = GetScriptTime(&scriptDoc,&fib1);
					break;

				case FILE_OPEN:
					if ( do_Open(&scriptDoc,FALSE,NULL) )
						gotDate = GetScriptTime(&scriptDoc,&fib1);
					break;

				case FILE_CLOSE:
					do_Close(&scriptDoc,TRUE);
					break;

				case FILE_SAVE:
					if ( !gotDate || (gotDate && CheckScriptTime(&scriptDoc, &fib1)) )
					{
						do_Save(&scriptDoc);
						this_node = FindParentNode(&(ObjectRecord.scriptSIR),ObjectRecord.objList);
						if ( this_node)
							PrintSubBranchName(this_node->objectName);
						else
							PrintSubBranchName(NULL);
						gotDate = GetScriptTime(&scriptDoc,&fib1);
					}
					break;

				case FILE_SAVEAS:
					if ( !gotDate || (gotDate && CheckScriptTime(&scriptDoc, &fib1)) )
					{
						saved = do_SaveAs(&scriptDoc,TRUE);
						this_node = FindParentNode(&(ObjectRecord.scriptSIR),ObjectRecord.objList);
						if ( this_node)
							PrintSubBranchName(this_node->objectName);
						else
							PrintSubBranchName(NULL);
						if ( saved )
							gotDate = GetScriptTime(&scriptDoc,&fib1);
					}
					break;

				case FILE_PAGESETUP:
					do_PageSetUp(&scriptDoc);
					break;

				case FILE_PRINT:
					do_Print(&scriptDoc);
					break;

				case FILE_QUIT:
					if ( do_Close(&scriptDoc,FALSE) )
						mode=DO_QUIT;
					break;
			}
			break;

		case EDIT_MENU:
			switch(CED.itemNum)
			{
				case EDIT_UNDO:
					UndoClear(&(ObjectRecord.scriptSIR));
					if ( ObjectRecord.objList == undoListPtr )
					{
						ObjectRecord.firstObject = (struct ScriptNodeRecord *)ObjectRecord.objList->lh_Head;
						numEntries1=ObjectRecord.numObjects;
						if ((topEntry1+numDisplay1)>numEntries1)
							topEntry1 = numEntries1-numDisplay1;
						if (topEntry1<0)
							topEntry1=0;
						UA_SetPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);
						DrawObjectList(topEntry1, TRUE, TRUE);
						FindSelectedIcon(-1);	/* reset Alt selecting */
						SetScriptMenus();
					}
					break;

				case EDIT_CUT:
				case EDIT_CLEAR:
					DisableMenu(script_MR[EDIT_MENU], EDIT_UNDO);
					if ( CED.itemNum == EDIT_CUT )
					{
						CutScriptObjects(&(ObjectRecord.scriptSIR), ObjectRecord.firstObject);
						EnableMenu(script_MR[EDIT_MENU], EDIT_PASTE);
						if (ObjectRecord.numObjects==0)
							DisableMenu(script_MR[EDIT_MENU], EDIT_SELECTALL);
					}
					else if ( CED.itemNum == EDIT_CLEAR )
						ClearScriptObjects(&(ObjectRecord.scriptSIR), ObjectRecord.firstObject);
					scriptDoc.modified=TRUE;
					numEntries1=ObjectRecord.numObjects;
					if ((topEntry1+numDisplay1)>numEntries1)
						topEntry1 = numEntries1-numDisplay1;
					if (topEntry1<0)
						topEntry1=0;
					UA_SetPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);
					ClearBetweenLines();
					DrawObjectList(topEntry1, TRUE, TRUE);
					FindSelectedIcon(-1);	/* reset Alt selecting */
					SetScriptMenus();
					break;

				case EDIT_COPY:
					DisableMenu(script_MR[EDIT_MENU], EDIT_UNDO);
					CopyScriptObjects(&(ObjectRecord.scriptSIR), ObjectRecord.firstObject);
					scriptDoc.modified=TRUE;
					SetScriptMenus();
					break;

				case EDIT_PASTE:
					DisableMenu(script_MR[EDIT_MENU], EDIT_UNDO);
					StartIconDragging((int)topEntry1, (int)topEntry2, 2); /* 2 means paste */
					scriptDoc.modified=TRUE;
					ObjectRecord.firstObject = (struct ScriptNodeRecord *)ObjectRecord.objList->lh_Head;
					if (topEntry1 == (numEntries1-numDisplay1))
						topEntry1 += (ObjectRecord.numObjects-numEntries1);
					numEntries1=ObjectRecord.numObjects;
					if ((topEntry1+numDisplay1)>numEntries1)
						topEntry1 = numEntries1-numDisplay1;
					if (topEntry1<0)
						topEntry1=0;
					UA_SetPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);
					ClearBetweenLines();
					DrawObjectList(topEntry1, TRUE, TRUE);
					FindSelectedIcon(-1);	/* reset Alt selecting */
					SetScriptMenus();
					break;

				case EDIT_SELECTALL:
					SelectAllObjects();
					DrawObjectList(-1, TRUE, TRUE);
					if ( ObjectRecord.objList != ObjectRecord.scriptSIR.allLists[0] ) /* root encountered */
					{
						EnableMenu(script_MR[EDIT_MENU], EDIT_CUT);
						EnableMenu(script_MR[EDIT_MENU], EDIT_COPY);
					}
					EnableMenu(script_MR[EDIT_MENU], EDIT_CLEAR);
					SetScriptMenus();
					break;
			}
			break;

		case XFER_MENU:
			switch(CED.itemNum)
			{
				case XFER_UPLOAD:
					if (scriptDoc.opened)
					{
						go_on=FALSE;
						if ( scriptDoc.modified )
						{
							if ( UA_OpenGenericWindow(scriptWindow, TRUE, TRUE, msgs[Msg_Yes-1], msgs[Msg_No-1],
																				EXCLAMATION_ICON, msgs[Msg_SaveItNow-1], TRUE, NULL) )
							{
								SetSpriteOfActWdw(SPRITE_BUSY);
								go_on=WriteScript(scriptDoc.path, scriptDoc.title, &(ObjectRecord.scriptSIR), scriptCommands);
								if ( go_on )
								{
									gotDate = GetScriptTime(&scriptDoc,&fib1);
									scriptDoc.modified = FALSE;
								}
								SetSpriteOfActWdw(SPRITE_NORMAL);
							}
						}
						else
							go_on=TRUE;
						if ( go_on )
							ShowRA(scriptDoc.path,scriptDoc.title,XFER_UPLOAD);
					}
					break;

				case XFER_DOWNLOAD:
					if (scriptDoc.opened)
						ShowRA(scriptDoc.path,scriptDoc.title,XFER_DOWNLOAD);
					break;
			}
			break;

		case SMISC_MENU:
			switch(CED.itemNum)
			{
				case SMISC_SHOWPROG:
					ToggleChooseMenuItem(script_MR[SMISC_MENU], SMISC_SHOWPROG);
					if ( CPrefs.showDays )
						CPrefs.showDays = FALSE;
					else
						CPrefs.showDays = TRUE;
					DeselectAllObjects();
					ClearBetweenLines();
					DrawObjectList(-1, TRUE, TRUE);
					doShowAndProgMenus();
					break;

				case SMISC_LOCALEVENTS:
					MonitorLocalEvents(&(ObjectRecord.scriptSIR));
					break;

				case SMISC_TWEAKER:
					if (	( ObjectRecord.scriptSIR.timeCodeFormat!=TIMEFORMAT_HHMMSS ||
									ObjectRecord.scriptSIR.listType==TALK_STARTPAR ) &&
							ObjectRecord.objList != ObjectRecord.scriptSIR.allLists[0] )
					{
						TimeCodeTweaker();
						DrawObjectList(-1, TRUE, TRUE);	// only redraw object name part
					}
					break;

				case SMISC_SCRIPTMANAGER:
					if (scriptDoc.opened)
					{
						if ( ShowSM(scriptDoc.path,scriptDoc.title) )
							gotDate = GetScriptTime(&scriptDoc,&fib1);
					}
					break;

				case SMISC_VARPATH:
					this_node = CountNumSelected(ObjectRecord.firstObject, &numSel);
					if ( this_node && numSel>=1 )
						FixedVarPath();
					break;
			}
			break;

		case SCREEN_MENU:
			switch(CED.itemNum)
			{
				case SCREEN_PAGE:
					mode=DO_OTHER;
					SetSpriteOfActWdw(SPRITE_BUSY);
					break;

				case SCREEN_PREFS:
					ShowPrefs();
					ActivateWindow(scriptWindow);
					break;

				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
					Fill_DA_Menu(TRUE);
					if ( DA_Screens[CED.itemNum-3] != NULL )
					{
						if ( DA_Screens[CED.itemNum-3]->FirstWindow != NULL )
							ActivateWindow( DA_Screens[CED.itemNum-3]->FirstWindow );
						ScreenToFront( DA_Screens[CED.itemNum-3] );
					}
					break;
			}
			break;
	}

	SetScriptMenus();

	return(mode);
}

/******** dokeyScrolling() ********/

void dokeyScrolling(void)
{
	switch(CED.Code)
	{
		case RAW_CURSORLEFT:
		case RAW_KEYPADLEFT:
			if ( !UA_IsGadgetDisabled(&Script_GR[5]) && !blockScript )
				processScriptParent();
			break;

		case RAW_HOME:
#ifndef USED_FOR_DEMO
			if ( topEntry1==0 && !UA_IsGadgetDisabled(&Script_GR[5]) && !blockScript )
				processScriptParent();
			else if (numEntries1>numDisplay1 && topEntry1!=0)
			{
				topEntry1=0L;
				UA_SetPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);
				DrawObjectList(topEntry1, TRUE, TRUE);
			}
#endif
			break;

		case RAW_END:
#ifndef USED_FOR_DEMO
			if (numEntries1>numDisplay1 && topEntry1!=(numEntries1-numDisplay1))
			{
				topEntry1 = numEntries1-numDisplay1;
				UA_SetPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);
				DrawObjectList(topEntry1, TRUE, TRUE);
			}
#endif
			break;

		case RAW_PAGEUP:
#ifndef USED_FOR_DEMO
			if (numEntries1>numDisplay1 && topEntry1!=0L)
			{
				if (topEntry1 < numDisplay1)
					topEntry1=0L;
				else
					topEntry1-=(numDisplay1-1);
				UA_SetPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);
				DrawObjectList(topEntry1, TRUE, TRUE);
			}
#endif
			break;

		case RAW_PAGEDOWN:
#ifndef USED_FOR_DEMO
			if (numEntries1>numDisplay1 && topEntry1!=(numEntries1-numDisplay1))
			{
				topEntry1+=(numDisplay1-1);
				if ((topEntry1+numDisplay1)>numEntries1)
					topEntry1=numEntries1-numDisplay1;
				UA_SetPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);
				DrawObjectList(topEntry1, TRUE, TRUE);
			}
#endif
			break;

		case RAW_CURSORUP:
		case RAW_KEYPADUP:
#ifndef USED_FOR_DEMO
			if (topEntry1>0)
			{
				topEntry1--;
				UA_SetPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);
				DrawObjectList(topEntry1, TRUE, TRUE);
			}
#endif
			break;

		case RAW_CURSORDOWN:
		case RAW_KEYPADDOWN:
#ifndef USED_FOR_DEMO
			if ((topEntry1+numDisplay1)<numEntries1)
			{
				topEntry1++;
				UA_SetPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);
				DrawObjectList(topEntry1, TRUE, TRUE);
			}
#endif
			break;
	}
}

/******** do_pre_play_things() ********/

BOOL do_pre_play_things(BOOL forPreview)
{
#ifdef USED_FOR_DEMO
	CPrefs.playOptions = 2;	// manual
#endif

	if ( !CheckScriptProblems(&ObjectRecord.scriptSIR) )
		return(FALSE);

	//oldview = GfxBase->ActiView;

	CloseWorkBench();

	if ( !MLMMU_OpenMsgQueue() )
		return(FALSE);

	if ( !forPreview )
	{
		FreeClipUndoLists(clipLists);
		FreeClipUndoLists(undoLists);
		DisableMenu(script_MR[EDIT_MENU], EDIT_UNDO);
		DisableMenu(script_MR[EDIT_MENU], EDIT_PASTE);
		FreeEasyBitmaps();
		stopClockTask();
	}

	OpenPlayScreen(CPrefs.playerMonitorID);
	PlayScreenToFront();

	if ( !forPreview )
		TempCloseScriptScreen();

	rvrec.pagescreen = playScreen;

	return(TRUE);
}

/******** do_post_play_things() ********/

void do_post_play_things(BOOL forPreview)
{
struct IntuiMessage *message;

	Forbid();
	BlockAllInput=FALSE;
	Permit();

	PlayScreenToFront();
	Delay(5L);

	if ( !forPreview )
		ReopenScriptScreen();
	ScreenToFront(scriptScreen);

	ClosePlayScreen();

	if ( !forPreview )
		startClockTask();

	if ( !forPreview )
	{
		if ( !AllocEasyBitmaps() )
			UA_WarnUser(-1);
	}

	/**** show accumulated error messages ****/

	ProcessMsgQueue();
	MLMMU_CloseMsgQueue();

	/**** drain all messages ****/
	/***** DO THIS LAST *********/

	while(message = (struct IntuiMessage *)GetMsg(capsPort))
		ReplyMsg((struct Message *)message);

	//GfxBase->ActiView = oldview;
}

/******** GenlockOff() ********/

void GenlockOff(struct Screen *screen)
{
struct TagItem ti[4];

	ti[0].ti_Tag	= VTAG_BORDERNOTRANS_SET;	//BORDERBLANK_CLR; // was SET but border became black
	ti[0].ti_Data	= NULL;

	ti[1].ti_Tag	= VTAG_CHROMAKEY_SET;
	ti[1].ti_Data	= NULL;

	ti[2].ti_Tag	= VTAG_CHROMA_PEN_CLR;
	ti[2].ti_Data = 0;

	ti[3].ti_Tag	= VTAG_END_CM;
	ti[3].ti_Data = NULL;

	if ( VideoControl(screen->ViewPort.ColorMap, ti)!=NULL )
		UA_WarnUser(228);

	MakeScreen(screen);
	RethinkDisplay();
}

/******** GenlockOn() ********/

void GenlockOn(struct Screen *screen)
{
struct TagItem ti[4];

	ti[0].ti_Tag	= VTAG_BORDERNOTRANS_CLR;	//BLANK_CLR;
	ti[0].ti_Data	= NULL;

	ti[1].ti_Tag	= VTAG_CHROMAKEY_SET;	//CLR;
	ti[1].ti_Data	= NULL;

	ti[2].ti_Tag	= VTAG_CHROMA_PEN_SET;
	ti[2].ti_Data = 0;

	ti[3].ti_Tag	= VTAG_END_CM;
	ti[3].ti_Data = NULL;

	if ( VideoControl(screen->ViewPort.ColorMap, ti)!=NULL )
		UA_WarnUser(228);

	MakeScreen(screen);
	RethinkDisplay();
}

/******** GetFirstEffObj() ********/

struct ScriptNodeRecord *GetFirstEffObj(void)
{
struct ScriptNodeRecord *this_node=NULL;

	/**** first look for first selected node ****/

	for(	this_node=(struct ScriptNodeRecord *)ObjectRecord.firstObject;
				this_node->node.ln_Succ;
				this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ	)
		if (	(this_node->nodeType == TALK_ANIM || this_node->nodeType == TALK_PAGE) &&
					(this_node->miscFlags & OBJ_SELECTED) )
			return(this_node);

	for(	this_node=(struct ScriptNodeRecord *)ObjectRecord.firstObject;
				this_node->node.ln_Succ;
				this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ	)
		if ( this_node->nodeType == TALK_ANIM || this_node->nodeType == TALK_PAGE )
			return(this_node);

	return(this_node);
}

/******** GetFirstSelObj() ********/

struct ScriptNodeRecord *GetFirstSelObj(void)
{
struct ScriptNodeRecord *this_node=NULL;

	for(	this_node=(struct ScriptNodeRecord *)ObjectRecord.firstObject;
				this_node->node.ln_Succ;
				this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ	)
		if ( this_node->miscFlags & OBJ_SELECTED )
			return(this_node);

	return(NULL);
}

/******** GhostPlay() ********/

BOOL GhostPlay(void)	// returns TRUE if script is playable
{
	if ( scriptDoc.opened )
	{
		if (	ObjectRecord.scriptSIR.allLists[1]->lh_TailPred ==
					(struct Node *)ObjectRecord.scriptSIR.allLists[1] )
		{
			// script is empty
			if ( !UA_IsGadgetDisabled(&Script_GR[4]) )
				UA_DisableButton(scriptWindow, &Script_GR[4], gui_pattern);
			return(FALSE);
		}
		else
		{
			// script is NOT empty
			if ( UA_IsGadgetDisabled(&Script_GR[4]) )
			{
				if (CPrefs.ScriptScreenModes & LACE)
					SetFont(scriptWindow->RPort, largeFont);
				UA_EnableButton(scriptWindow, &Script_GR[4]);
				SetFont(scriptWindow->RPort, smallFont);
			}
			return(TRUE);
		}
	}
	return(FALSE);
}

/******** do_small_play() ********/

void do_small_play(struct ScriptNodeRecord *this_node)
{
int old_PreLoad, old_StandBy, old_PlayOptions;
BOOL old_showDays;
UBYTE old_mousePointer;
struct ScriptNodeRecord snr;

	CopyMem(this_node, &snr, sizeof(struct ScriptNodeRecord));

	old_showDays = CPrefs.showDays;
	CPrefs.showDays	= FALSE;

	old_PreLoad = CPrefs.objectPreLoading;
	CPrefs.objectPreLoading = 20;	// none

	old_StandBy = CPrefs.standBy;
	CPrefs.standBy = FALSE;

	old_PlayOptions = CPrefs.playOptions;
	CPrefs.playOptions = 0;

	old_mousePointer = CPrefs.mousePointer;

	this_node->duration = 863999L;

	if ( ObjectRecord.scriptSIR.timeCodeFormat!=TIMEFORMAT_HHMMSS )
	{
		this_node->Start.TimeCode.HH	= 0;
		this_node->Start.TimeCode.MM	= 0;
		this_node->Start.TimeCode.SS	= 0;
		this_node->Start.TimeCode.FF	= 0;

		this_node->End.TimeCode.HH		= 23;
		this_node->End.TimeCode.MM		= 0;
		this_node->End.TimeCode.SS		= 0;
		this_node->End.TimeCode.FF		= 0;
	}

	if ( do_pre_play_things(TRUE) )
	{
		ActivateWindow(playWindow);
		fromSNR = this_node;
		PlayTheScript_Small(this_node);
		do_post_play_things(TRUE);
	}

	CPrefs.showDays					= old_showDays;
	CPrefs.objectPreLoading	= old_PreLoad;
	CPrefs.standBy					= old_StandBy;
	CPrefs.playOptions			= old_PlayOptions;
	CPrefs.mousePointer			= old_mousePointer;

	CopyMem(&snr, this_node, sizeof(struct ScriptNodeRecord));
}

/******** DoTransitions() ********/
/*
 * mode = 1	-> 't' pressed
 * mode = 2 -> click on effect icon
 *
 */ 

void DoTransitions(int mode)
{
struct ScriptNodeRecord *this_node;
int dummy;
WORD dummy2;
struct IntuiMessage *message;

	if ( mode==1 )	// 't' pressed
	{
		if ( ObjectRecord.numObjects==0 )
			return;
		this_node = (struct ScriptNodeRecord *)GetFirstEffObj();
	}
	else
		this_node = (struct ScriptNodeRecord *)WhichObjectWasClicked((int)topEntry1, &dummy);

	if ( this_node && (this_node->nodeType==TALK_ANIM || this_node->nodeType==TALK_PAGE) )
	{
		if ( !(CED.Qualifier&IEQUALIFIER_LSHIFT) && !(CED.Qualifier&IEQUALIFIER_RSHIFT) &&
				 mode==2 && !(this_node->miscFlags & OBJ_SELECTED) )
			ReallyDeselectAllButThisOne(this_node);	

		SetByteBit(&this_node->miscFlags, OBJ_SELECTED);
		SetByteBit(&this_node->miscFlags, OBJ_NEEDS_REFRESH);
		DrawObjectList(-1, FALSE, TRUE);	// only redraw object name part

		if ( Monitor_Effect(&dummy2,&dummy2,&dummy2,1) )
			scriptDoc.modified=TRUE;

		if ( ObjectRecord.scriptSIR.listType == -1 )
		{
			ObjectRecord.scriptSIR.listType = 
											FindParentType( &(ObjectRecord.scriptSIR), ObjectRecord.objList);
			if ( ObjectRecord.scriptSIR.listType == -1 )
				ObjectRecord.scriptSIR.listType = TALK_STARTSER;	// root is serial
		}
		if ( ObjectRecord.scriptSIR.listType==TALK_STARTSER )
			ShowSerialEventIcons();
		else
			ShowParallelEventIcons();
	}

	while(message = (struct IntuiMessage *)GetMsg(capsPort))
		ReplyMsg((struct Message *)message);

	CED.Class=0;
	CED.Code=0;

	SetScriptMenus();
}

/******** DoDuration() ********/
/*
 * mode = 1	-> 'd' pressed
 * mode = 2 -> click on duration
 *
 */ 

void DoDuration(int mode)
{
struct IntuiMessage *message;
struct ScriptNodeRecord *this_node;
int dummy, numSel=0;

	if ( mode==1 )	// 'd' pressed
	{
		this_node = CountNumSelected(ObjectRecord.firstObject, &numSel);
		if ( numSel == 0 )
			return;
		this_node = GetFirstSelObj();
	}
	else
		this_node = (struct ScriptNodeRecord *)WhichObjectWasClicked((int)topEntry1, &dummy);

	if ( this_node )
	{
		if ( 	CPrefs.showDays &&
					((CED.Qualifier&IEQUALIFIER_LALT) || (CED.Qualifier&IEQUALIFIER_RALT)) &&
					ObjectRecord.scriptSIR.timeCodeFormat==TIMEFORMAT_HHMMSS &&
					ObjectRecord.scriptSIR.listType != TALK_STARTPAR )
		{
			if ( Build_program_Requester(scriptWindow, (int)topEntry1, NULL) )
			{
				DrawObjectList(-1, TRUE, TRUE);	// only redraw object name part
				scriptDoc.modified=TRUE;
			}
		}
		else
		{
			if ( !(CED.Qualifier&IEQUALIFIER_LSHIFT) && !(CED.Qualifier&IEQUALIFIER_RSHIFT) &&
					 mode==2 && !(this_node->miscFlags & OBJ_SELECTED) )
				ReallyDeselectAllButThisOne(this_node);	

			SetByteBit(&this_node->miscFlags, OBJ_SELECTED);
			SetByteBit(&this_node->miscFlags, OBJ_NEEDS_REFRESH);
			DrawObjectList(-1, FALSE, TRUE);	// only redraw object name part

			SetTime();

			if ( ObjectRecord.scriptSIR.listType == -1 )
			{
				ObjectRecord.scriptSIR.listType = 
												FindParentType( &(ObjectRecord.scriptSIR), ObjectRecord.objList);
				if ( ObjectRecord.scriptSIR.listType == -1 )
					ObjectRecord.scriptSIR.listType = TALK_STARTSER;	// root is serial
			}
			if ( ObjectRecord.scriptSIR.listType==TALK_STARTSER )
				ShowSerialEventIcons();
			else
				ShowParallelEventIcons();
		}
	}

	while(message = (struct IntuiMessage *)GetMsg(capsPort))
		ReplyMsg((struct Message *)message);

	CED.Class=0;
	CED.Code=0;

	SetScriptMenus();
}

/******** DoComment() ********/
/*
 * mode = 1	-> 'c' pressed
 * mode = 2 -> click on object name
 *
 */ 

void DoComment(int mode)
{
struct IntuiMessage *message;
struct ScriptNodeRecord *this_node;
int dummy, numSel=0;

	if ( mode==1 )	// 'c' pressed
	{
		this_node = CountNumSelected(ObjectRecord.firstObject, &numSel);
		if ( numSel != 1 )
			return;
	}
	else
		this_node = (struct ScriptNodeRecord *)WhichObjectWasClicked((int)topEntry1, &dummy);

	if ( this_node )
	{
		if ( 	this_node->nodeType==TALK_AREXX ||
					this_node->nodeType==TALK_DOS ||
					this_node->nodeType==TALK_LABEL ||
					this_node->nodeType==TALK_STARTPAR ||
					this_node->nodeType==TALK_STARTSER ||
					this_node->nodeType==TALK_USERAPPLIC )
		{
			ReallyDeselectAllButThisOne(this_node);

			if (this_node->nodeType==TALK_AREXX || this_node->nodeType==TALK_DOS)
				DoObjectName(this_node->extraData, msgs[Msg_ObjectComment-1], this_node->nodeType);	/* enter comment */
			else if ( this_node->nodeType==TALK_LABEL )
				DoObjectName(this_node->objectName, msgs[Msg_UniqueLabel-1], this_node->nodeType); /* enter unique label name */
			else if ( this_node->nodeType==TALK_STARTPAR )
				DoObjectName(this_node->objectName, msgs[Msg_NameParallel-1], this_node->nodeType); /* enter par name */
			else if ( this_node->nodeType==TALK_STARTSER )
				DoObjectName(this_node->objectName, msgs[Msg_NameSerial-1], this_node->nodeType); /* enter ser name */
			else if ( this_node->nodeType==TALK_USERAPPLIC )
				DoObjectName(this_node->objectName, msgs[Msg_XappComment-1], this_node->nodeType); /* enter XaPP comment */

			SetByteBit(&this_node->miscFlags, OBJ_NEEDS_REFRESH);

			DrawObjectList(-1, FALSE, TRUE);

			scriptDoc.modified=TRUE;
		}
	}

	while(message = (struct IntuiMessage *)GetMsg(capsPort))
		ReplyMsg((struct Message *)message);

	CED.Class=0;
	CED.Code=0;
}

/******** SetScriptMenus() ********/

void SetScriptMenus(void)
{
struct ScriptNodeRecord *last_node;
int numSelected;

	if ( !scriptDoc.opened || !ObjectRecord.objList )
		return;

	last_node = (struct ScriptNodeRecord *)CountNumSelected(
																		ObjectRecord.firstObject, &numSelected);
	if (numSelected>=1)
	{
		if ( ObjectRecord.objList != ObjectRecord.scriptSIR.allLists[0] )	// not root
		{
			EnableMenu(script_MR[EDIT_MENU], EDIT_CUT);
			EnableMenu(script_MR[EDIT_MENU], EDIT_COPY);

			EnableMenu(script_MR[SMISC_MENU], SMISC_VARPATH);	// NEW
		}
		if (	ObjectRecord.objList == ObjectRecord.scriptSIR.allLists[0] &&
					last_node->nodeType==TALK_STARTSER )
			;
		else
			EnableMenu(script_MR[EDIT_MENU], EDIT_CLEAR);
	}
	else
	{
		DisableMenu(script_MR[EDIT_MENU], EDIT_CUT);
		DisableMenu(script_MR[EDIT_MENU], EDIT_COPY);
		DisableMenu(script_MR[EDIT_MENU], EDIT_CLEAR);

		DisableMenu(script_MR[SMISC_MENU], SMISC_VARPATH);	// NEW
	}

	if (	( ObjectRecord.scriptSIR.timeCodeFormat!=TIMEFORMAT_HHMMSS ||
					ObjectRecord.scriptSIR.listType==TALK_STARTPAR ) &&
					ObjectRecord.objList != ObjectRecord.scriptSIR.allLists[0] )
		EnableMenu(script_MR[SMISC_MENU], SMISC_TWEAKER);
	else
		DisableMenu(script_MR[SMISC_MENU], SMISC_TWEAKER);

	// dis/en-able edit button

	if (	numSelected<=1 && UA_IsGadgetDisabled(&Script_GR[7]) &&
				ObjectRecord.objList != ObjectRecord.scriptSIR.allLists[0] )	
	{
		if (CPrefs.ScriptScreenModes & LACE)
			SetFont(scriptWindow->RPort, largeFont);
		UA_EnableButton(scriptWindow, &Script_GR[7]);
		SetFont(scriptWindow->RPort, smallFont);
	}
	else if ( (numSelected>1 ||
						ObjectRecord.objList == ObjectRecord.scriptSIR.allLists[0]) &&
						!UA_IsGadgetDisabled(&Script_GR[7]) )
		UA_DisableButton(scriptWindow, &Script_GR[7], gui_pattern);

	// dis/en-able show button

	if (	numSelected==1 && UA_IsGadgetDisabled(&Script_GR[8]) &&
				ObjectRecord.objList != ObjectRecord.scriptSIR.allLists[0] )	
	{
		if (CPrefs.ScriptScreenModes & LACE)
			SetFont(scriptWindow->RPort, largeFont);
		UA_EnableButton(scriptWindow, &Script_GR[8]);
		SetFont(scriptWindow->RPort, smallFont);
	}
	else if ( (numSelected!=1 ||
						ObjectRecord.objList == ObjectRecord.scriptSIR.allLists[0]) &&
						!UA_IsGadgetDisabled(&Script_GR[8]) )
		UA_DisableButton(scriptWindow, &Script_GR[8], gui_pattern);
}

/******** PlayTheScript() ********/

void PlayTheScript(void)
{
TEXT path[SIZE_FULLPATH];
TEXT filename[SIZE_FILENAME];
UWORD loadedScripts=0;
BOOL deInit=FALSE;

	if ( !SpecialAllocMLSystem() )
		return;

	newScript[0] = '\0';

	if ( pre_player() )
	{
		playScript( &(ObjectRecord.scriptSIR),1,TRUE,FALSE,FALSE );
		post_player();
		if ( newScript[0] != '\0' ) 
			freeScript();
		deInit=TRUE;
	}

	// As long as ARexx fills newScript[] keep on loading, playing and unloading scripts!

	while( newScript[0] != '\0' ) 
	{
		UA_SplitFullPath(newScript, path, filename);
		newScript[0] = '\0';	// arexx may fill it again.

		if ( ReadScript(path, filename, scriptCommands) )
		{
			loadedScripts++;
			fromSNR = (struct ScriptNodeRecord *)ObjectRecord.scriptSIR.allLists[1]->lh_Head;
			if ( pre_player() )
			{
				if ( ValidateSER(&(ObjectRecord.scriptSIR),TRUE,TRUE) )
				{
					playScript( &(ObjectRecord.scriptSIR),1,FALSE,FALSE,FALSE );
				}
				post_player();
			}
			freeScript();
			UnSetBit(&allocFlags, SCRIPTINMEM_FLAG);
		}
	}

	// Read old script back again

	if ( loadedScripts )
	{
		scriptDoc.modified = FALSE;
		scriptDoc.opened = FALSE;
		scriptDoc.title[0] = '\0';
		ObjectRecord.objList = NULL;
		ObjectRecord.firstObject = NULL;
		ObjectRecord.maxObjects	= 0;
		FindSelectedIcon(-1);	// reset Alt selecting
	}

	PlayScreenToFront();

	if ( deInit )
		ProcessDeInitializer();

	SpecialFreeMLSystem();
}

/******** PlayTheScript_Small() ********/

void PlayTheScript_Small(struct ScriptNodeRecord *this_node)
{
BOOL deInit=FALSE;

	if ( !SpecialAllocMLSystem() )
		return;

	newScript[0] = '\0';

	if ( pre_player() )
	{
		CPrefs.mousePointer = 0;
		playScript( &(ObjectRecord.scriptSIR),1,TRUE,FALSE,FALSE );
		post_player();
		deInit=TRUE;
	}

	PlayScreenToFront();

	if ( deInit )
		ProcessDeInitializer();

	SpecialFreeMLSystem();
}

/******** pre_player() ********/

BOOL pre_player(void)
{
ULONG numObjects;
int numButtons;

	kept_mousePointer = CPrefs.mousePointer;

	numObjects = CreateUsedXappList(&(ObjectRecord.scriptSIR));
	if ( numObjects==1 )
	{
		CPrefs.mousePointer = CPrefs.mousePointer & ~(1+2+4+8);
		CPrefs.mousePointer |= 8;	// put input to none
	}

	numButtons = Validate_All_LE(&(ObjectRecord.scriptSIR),TRUE);
	if ( numButtons>0 )
	{
		if ( !(CPrefs.mousePointer & 8) )	// keep none
		{
			CPrefs.mousePointer = CPrefs.mousePointer & ~(1+2+4+8);
			CPrefs.mousePointer |= 1;	// cursor
		}
	}

	// ONLY ENABLE SWITCHING TO WB WHEN INPUT IS CURSOR+MOUSE OR CURSOR

	if ( (CPrefs.mousePointer & 2) || (CPrefs.mousePointer & 8) )
	{
		Forbid();
		BlockAllInput=TRUE;
		Permit();
	}

	return(TRUE);
}

/******** post_player() ********/

void post_player(void)
{
	FreeUsedXappList();
	Free_All_LE( &(ObjectRecord.scriptSIR) );
	CPrefs.mousePointer = kept_mousePointer;
}

/******** GetScriptTime() ********/

BOOL GetScriptTime(struct Document *scriptDoc, struct FileInfoBlock *startFib)
{
BPTR lock;
BOOL retval=FALSE;
TEXT fpath[SIZE_FULLPATH];

	if (	!strcmpi(scriptDoc->title,msgs[Msg_Untitled-1]) ||
				!strcmpi(scriptDoc->title,"untitled") )
		return(FALSE);

	UA_MakeFullPath(scriptDoc->path, scriptDoc->title, fpath);

	if ( lock=Lock(fpath,SHARED_LOCK) )
	{
		if( Examine(lock,startFib) )
			retval = TRUE;
		UnLock(lock);
	}

	return(retval);
}

/******** CheckScriptTime() ********/

BOOL CheckScriptTime(struct Document *scriptDoc, struct FileInfoBlock *startFib)
{
BPTR lock;
BOOL retval=FALSE;
struct FileInfoBlock __aligned endFib;
TEXT fpath[SIZE_FULLPATH];

	if (	!strcmpi(scriptDoc->title,msgs[Msg_Untitled-1]) ||
				!strcmpi(scriptDoc->title,"untitled") )
		return(TRUE);

	UA_MakeFullPath(scriptDoc->path, scriptDoc->title, fpath);

	if ( lock=Lock(fpath,SHARED_LOCK) )
	{
		if ( Examine(lock,&endFib) )
		{
			if ( CompareDates(&startFib->fib_Date, &endFib.fib_Date)==0 )
			{
				retval=TRUE;
			}
		}
		UnLock(lock);
	}

	if ( !retval )	// dates not equal
	{
		// next func returns TRUE if 'Continue' is clicked

		retval = UA_OpenGenericWindow(scriptWindow, TRUE, TRUE, msgs[Msg_Yes-1], msgs[Msg_No-1],
																	EXCLAMATION_ICON, msgs[Msg_ScriptModi-1], TRUE, NULL);
	}

	return(retval);
}

/******** E O F ********/
