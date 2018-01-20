#include "nb:pre.h"
#include "demo:gen/support_protos.h"
#include "setup.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "structs.h"

#define VERSI0N "\0$VER: 1.0"
static UBYTE *vers = VERSI0N;

/**** internal function declarations ****/

void XappSetup(PROCESSINFO *ThisPI);
BOOL MonitorUser(struct Window *window, PROCESSINFO *ThisPI);
void GetExtraData(PROCESSINFO *ThisPI, struct CreditRoll *CR_rec);
void PutExtraData(PROCESSINFO *ThisPI, struct CreditRoll *CR_rec);
void SetButtons(struct Window *window, struct CreditRoll *CR_rec);
BOOL VSC_MonitorFontSelection(struct Window *window, struct CreditRoll *CR_rec,
															UWORD *mypattern1 );
void VSC_FillInFER(STRPTR fontname, int fontsize);

/**** external function declarations ****/

extern BOOL DoTextEd(struct Window *window, struct EventData *CED);
extern BOOL OpenTextEd(struct Window *window);
extern void CloseTextEd(void);
extern BOOL TE_LoadCrawl(struct CreditRoll *CR_rec);
extern BOOL TE_SaveCrawl(struct CreditRoll *CR_rec);
extern void DisplayLoadedText(void);
extern void InsertHelpText(void);
extern BOOL TextIsEmpty(void);
extern void ClearText(void);
extern void Preview_roll( LONG MonID, char *filename );

/**** globals ****/

struct Library *medialinkLibBase		= NULL;
struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase *GfxBase							= NULL;

UBYTE **msgs;
struct RendezVousRecord *rvrec;
struct MsgPort *capsport;

/**** disable CTRL-C break ****/

void CXBRK(void) { }
void chkabort(void) { }

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
		UA_DoubleGadgetDimensions(VSC_GR);

	UA_TranslateGR(VSC_GR, msgs);
	UA_TranslateGR(FontSelect_GR, msgs);

	capsport = (struct MsgPort *)FindPort(MEDIALINKPORT);
	oldTask = capsport->mp_SigTask;
	capsport->mp_SigTask = FindTask(NULL);

	UAI.windowX				= -1;	/* -1 means center on screen */
	UAI.windowY				= -1;	/* -1 means center on screen */
	UAI.windowWidth		= VSC_GR[0].x2;
	UAI.windowHeight	= VSC_GR[0].y2;
	UAI.wflg					= WFLG_ACTIVATE|WFLG_BORDERLESS|WFLG_RMBTRAP|WFLG_NOCAREREFRESH;
	UA_OpenWindow(&UAI);

	/**** render all gadgets ****/

	UA_DrawGadgetList(UAI.userWindow, VSC_GR);

	/**** monitor events ****/

	MonitorUser(UAI.userWindow, ThisPI);

	/**** close the window ****/

	UA_CloseWindow(&UAI);

	capsport->mp_SigTask = oldTask;
}

/******** MonitorUser() ********/

BOOL MonitorUser(struct Window *window, PROCESSINFO *ThisPI)
{
BOOL loop=TRUE, retVal, saved;
struct EventData CED;
int ID;
struct FileReqRecord FRR;
TEXT path[SIZE_PATH], filename[SIZE_FILENAME], fullpath[SIZE_FULLPATH], info[100];
UWORD *mypattern1;
struct CreditRoll CR_rec;

	saved = FALSE;

	mypattern1 = (UWORD *)AllocMem(4L, MEMF_CHIP);
	if (mypattern1==NULL)
		return(FALSE);
	mypattern1[0] = 0x5555;
	mypattern1[1] = 0xaaaa;

	/**** set up FRR ****/

	strcpy(path, rvrec->capsprefs->import_text_Path);
	strcpy(filename,"CrawlText");

	FRR.path			= path;
	FRR.fileName	= filename;
	FRR.opts			= DIR_OPT_ALL;
	FRR.multiple	= FALSE;

	/**** get arguments from extraData ****/

	CR_rec.file[0]					= '\0';
	strcpy(CR_rec.fontName,"MediaPoint");
	CR_rec.fontSize					= 20;
	CR_rec.backgroundColor	= 4;	// blue
	CR_rec.textColor				= 1;	// white
	CR_rec.shadowColor			= 0;	// black
	CR_rec.speed						= 3;	// -> speed=10 -> values 0...29 -> text 1...30
	CR_rec.sweight					= 2;	// -> weight=2 -> values 0...15 -> text 0...15
	CR_rec.stype						= 0;	// -> type=solid (0=solid, 1=cast)
	CR_rec.lspc							= 0;

	if ( !OpenTextEd(window) )
		return(FALSE);

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
	{
		GetExtraData(ThisPI, &CR_rec);
		if ( !TE_LoadCrawl(&CR_rec) )
		{
			sprintf(info, msgs[Msg_FileGone-1], filename);
			GiveMessage(window, info);
		}
		else
		{
			UA_SplitFullPath(CR_rec.file, path, filename);
			DisplayLoadedText();
			SetButtons(window,&CR_rec);
			saved=TRUE;
		}
	}

	SetButtons(window,&CR_rec);

	VSC_GR[11].type = BUTTON_GADGET;

	UA_SwitchFlagsOn(window,IDCMP_VANILLAKEY);

	/**** event handler ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);
		if (CED.Class==MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, VSC_GR, &CED);
			switch(ID)
			{
				case 2:		// OK
do_ok:
					UA_HiliteButton(window, &VSC_GR[2]);
					if ( !saved )
						GiveMessage(window, msgs[Msg_VSC_FistSave-1]);
					else
					{
						loop=FALSE;
						retVal=TRUE; 
					}
					break;

				case 3:		// Cancel
do_cancel:
					UA_HiliteButton(window, &VSC_GR[3]);
					loop=FALSE;
					retVal=FALSE; 
					break;

				case 4: 	// Play
					UA_InvertButton(window, &VSC_GR[ID]);
					{
					struct CreditRoll TMP_CR_rec;
						CopyMem(&CR_rec,&TMP_CR_rec,sizeof(struct CreditRoll));
						strcpy(TMP_CR_rec.file,"ram:creditroll.tmp");
						if ( TE_SaveCrawl( &TMP_CR_rec ) )
							Preview_roll( rvrec->capsprefs->playerMonitorID, TMP_CR_rec.file );
						DeleteFile("ram:creditroll.tmp");
					}
					UA_InvertButton(window, &VSC_GR[ID]);
					break;

				case 8:		// load
				case 9:		// save
					UA_InvertButton(window, &VSC_GR[ID]);
					if ( ID==8 )	// load
					{
						FRR.title	= msgs[Msg_LoadCredit-1];
						if ( UA_OpenAFile(window, &FRR, mypattern1) )
						{
							UA_MakeFullPath(path, filename, fullpath);
							strcpy(CR_rec.file,fullpath);
							if ( !TE_LoadCrawl(&CR_rec) )
							{
								sprintf(info, msgs[Msg_FileGone-1], filename);
								GiveMessage(window, info);
							}
							else
							{
								saved=FALSE;
								DisplayLoadedText();
								SetButtons(window,&CR_rec);
							}
						}
					}
					else if ( ID==9 )	// save
					{
						FRR.title	= msgs[Msg_SaveCredit-1];
						if ( UA_SaveAFile(window, &FRR, mypattern1) )
						{
							UA_MakeFullPath(path, filename, fullpath);
							strcpy(CR_rec.file,fullpath);
							if ( !TE_SaveCrawl(&CR_rec) )
								GiveMessage(window, msgs[Msg_FileNotSaved-1]);
							else
								saved=TRUE;
							UA_OpenAFile(NULL,&FRR,NULL);	// special trick to reset dir caching
						}
					}
					UA_InvertButton(window, &VSC_GR[ID]);
					break;

				case 10:	// text editor
					DoTextEd(window,&CED);
					break;

				case 11:	// Font 
					UA_InvertButton(window, &VSC_GR[ID]);
					if ( VSC_MonitorFontSelection(window,&CR_rec,mypattern1) )
					{
						sprintf(info, "%s, %d points", CR_rec.fontName, (int)CR_rec.fontSize);
						UA_PrintInBox(window, &VSC_GR[11],
													VSC_GR[11].x1, VSC_GR[11].y1,
													VSC_GR[11].x2, VSC_GR[11].y2, info, PRINT_RIGHTPART);
					}
					else
						UA_InvertButton(window, &VSC_GR[ID]);
					break;

				case 12:	// background color
				case 13:	// text color
				case 14:	// shadow color
				case 15:	// speed
				case 16:	// shadow weight
				case 17:	// shadow type	
					UA_ProcessCycleGadget(window, &VSC_GR[ID], &CED);
					if ( ID==12 )
						UA_SetValToCycleGadgetVal(&VSC_GR[ID],&CR_rec.backgroundColor);
					else if ( ID==13 )
						UA_SetValToCycleGadgetVal(&VSC_GR[ID],&CR_rec.textColor);
					else if ( ID==14 )
 						UA_SetValToCycleGadgetVal(&VSC_GR[ID],&CR_rec.shadowColor);
					else if ( ID==15 )
						UA_SetValToCycleGadgetVal(&VSC_GR[ID],&CR_rec.speed);
					else if ( ID==16 )
						UA_SetValToCycleGadgetVal(&VSC_GR[ID],&CR_rec.sweight);
					else if ( ID==17 )
						UA_SetValToCycleGadgetVal(&VSC_GR[ID],&CR_rec.stype);
					break;

				case 18:	// Help
do_help:
					UA_HiliteButton(window, &VSC_GR[18]);
					if (	TextIsEmpty() ||
								UA_OpenGenericWindow(	window, TRUE, TRUE,
																			msgs[Msg_OK-1], msgs[Msg_Cancel-1],
																			EXCLAMATION_ICON,
																			msgs[Msg_VSC_HelpWarn-1], TRUE, NULL) )
					{
						InsertHelpText();
						DisplayLoadedText();
					}
					break;

				case 19:
					UA_HiliteButton(window, &VSC_GR[ID]);
					if ( !TextIsEmpty() )
					{
						if ( UA_OpenGenericWindow(window, TRUE, TRUE,
																			msgs[Msg_Yes-1], msgs[Msg_No-1],
																			EXCLAMATION_ICON,
																			msgs[Msg_VSC_Clear-1], TRUE, NULL) )
							ClearText();
					}
					break;

				case 20:	// line spc
					UA_ProcessCycleGadget(window, &VSC_GR[ID], &CED);
					UA_SetValToCycleGadgetVal(&VSC_GR[ID],&CR_rec.lspc);
					break;
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			DoTextEd(window,&CED);
			if (CED.Code==RAW_ESCAPE)
				goto do_cancel;
			else if (CED.Code==RAW_HELP)
				goto do_help;
		}
		else if (CED.Class==IDCMP_VANILLAKEY)
		{
			DoTextEd(window,&CED);
		}
	}

	UA_SwitchFlagsOff(window,IDCMP_VANILLAKEY);

	FreeMem(mypattern1, 4L);

	if ( retVal )
	{
		if ( saved )	// saved is true when save buttons was clicked or
		{							// when xapp already carried data.
			if ( !TE_SaveCrawl(&CR_rec) )
				GiveMessage(window, msgs[Msg_FileNotSaved-1]);
		}

		if ( CR_rec.file[0] )
			PutExtraData(ThisPI,&CR_rec);
	}

	CloseTextEd();

	return(retVal);
}

/******** GetExtraData() ********/

void GetExtraData(PROCESSINFO *ThisPI, struct CreditRoll *CR_rec)
{
int numChars, argNum, len, numSeen;
char *strPtr;
char tmp[USERAPPLIC_MEMSIZE], scrStr[USERAPPLIC_MEMSIZE];

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] == '\0' )
		return;

	ScriptToStr(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData, scrStr);

	strPtr = scrStr;
	len = strlen(strPtr);
	argNum=0;
	numSeen=0;
	while(1)
	{
		if (*strPtr==0)
			break;
		numChars = stcarg(strPtr, " 	,");	// space, tab, comma
		if ( numChars>=1 && numChars<SIZE_FULLPATH )
		{
			stccpy(tmp, strPtr, numChars+1);
			switch(argNum)
			{
				case 0:
					RemoveQuotes(tmp);
					strcpy(CR_rec->file,tmp);
					break;
				default:
					return;	// get the hell out, s'thing is terribly wrong!
			}
			argNum++;
		}
		strPtr += numChars+1;
		numSeen += numChars+1;
		if (numSeen>len)
			break;		
	}
}

/******** PutExtraData() ********/

void PutExtraData(PROCESSINFO *ThisPI, struct CreditRoll *CR_rec)
{
UBYTE scrStr[512];

	StrToScript(CR_rec->file, scrStr);
	sprintf(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData, "\\\"%s\\\"", scrStr);
}

/******** SetButtons() ********/

void SetButtons(struct Window *window, struct CreditRoll *CR_rec)
{
TEXT info[100];

	sprintf(info, "%s, %d points", CR_rec->fontName, (int)CR_rec->fontSize);
	UA_PrintInBox(window, &VSC_GR[11],
								VSC_GR[11].x1, VSC_GR[11].y1,
								VSC_GR[11].x2, VSC_GR[11].y2, info, PRINT_RIGHTPART);

	UA_SetCycleGadgetToVal(window, &VSC_GR[12], CR_rec->backgroundColor);
	UA_SetCycleGadgetToVal(window, &VSC_GR[13], CR_rec->textColor);
	UA_SetCycleGadgetToVal(window, &VSC_GR[14], CR_rec->shadowColor);

	UA_SetCycleGadgetToVal(window, &VSC_GR[15], CR_rec->speed);
	UA_SetCycleGadgetToVal(window, &VSC_GR[16], CR_rec->sweight);
	UA_SetCycleGadgetToVal(window, &VSC_GR[17], CR_rec->stype);

	UA_SetCycleGadgetToVal(window, &VSC_GR[20], CR_rec->lspc);
}

/******** VSC_MonitorFontSelection() ********/

BOOL VSC_MonitorFontSelection(struct Window *window, struct CreditRoll *CR_rec,
															UWORD *mypattern1 )
{
struct Window *fontWdw;
struct FontEntry *FE;
struct TextFont *newFont=NULL;
struct FER *FontEntryRecord;
int size;
 
	FontEntryRecord = rvrec->FontEntryRecord;
	VSC_FillInFER(CR_rec->fontName, CR_rec->fontSize);

	fontWdw = UA_OpenFontListWindow(window, FontEntryRecord, FontSelect_GR, mypattern1);
	if (fontWdw==NULL)
		return(FALSE);

	newFont =  UA_Monitor_FontSelection(fontWdw, FontEntryRecord, FontSelect_GR, mypattern1);
	if ( newFont )
	{
		if ( FontEntryRecord->selected1 != -1 )
		{
			FE = (struct FontEntry *)FontEntryRecord->FEList[ FontEntryRecord->selected1 ];
			if ( FontEntryRecord->selected2 != -1 )
				size = FE->fontSize[ FontEntryRecord->selected2 ];
			else if ( FontEntryRecord->fontSize != -1 )
				size = FontEntryRecord->fontSize;
			if (size==-1)
				size=FE->fontSize[0];
			// Store info
			sprintf(CR_rec->fontName,&FE->fontName[1]);
			CR_rec->fontSize = size;
		}
	}

	UA_CloseFontListWindow(fontWdw);

	if (newFont==NULL)
		return(FALSE);
	else
		CloseFont(newFont);

	return(TRUE);
}

/******** VSC_FillInFER() ********/

void VSC_FillInFER(STRPTR fontname, int fontsize)
{
int i,j;
struct FontEntry *FE;
struct FER *FontEntryRecord;

	FontEntryRecord = rvrec->FontEntryRecord;

	FontEntryRecord->selected1 = -1;
	FontEntryRecord->selected2 = -1;
	FontEntryRecord->fontSize = -1;

	FE = (struct FontEntry *)FontEntryRecord->FEList[ 0 ];

	for(i=0; i<FontEntryRecord->numEntries1; i++)
	{
		FE = (struct FontEntry *)FontEntryRecord->FEList[ i ];
		if ( !strcmpi( &FE->fontName[1], fontname ) )	// bijna beet?
		{
			FontEntryRecord->selected1 = i;
			for(j=0; j<30; j++)
			{
				if ( FE->fontSize[j]==fontsize )
				{
					FontEntryRecord->selected2 = j;
					goto go_on;
				}
			}
		}
	}

	if ( FontEntryRecord->selected1 != -1 )
		FontEntryRecord->fontSize = fontsize;

go_on:

	if ( FontEntryRecord->selected1 != -1 && FontEntryRecord->selected2 != -1 )
	{
		FontEntryRecord->top1 = FontEntryRecord->selected1;
		if ( FontEntryRecord->top1+6 >= FontEntryRecord->numEntries1 )
			FontEntryRecord->top1 = FontEntryRecord->numEntries1-6;
		if ( FontEntryRecord->top1 < 0 )
			FontEntryRecord->top1 = 0;

		FontEntryRecord->top2 = FontEntryRecord->selected2;
		if ( FontEntryRecord->top2+4 >= FontEntryRecord->numEntries2 )
			FontEntryRecord->top2 = FontEntryRecord->numEntries2-4;
		if ( FontEntryRecord->top2 < 0 )
			FontEntryRecord->top2 = 0;
	}
}

/******** E O F ********/
