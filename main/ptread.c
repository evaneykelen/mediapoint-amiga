#include "nb:pre.h"

/**** defines ****/

#define RAWBUFSIZE 2000	// see also parsetext.c and dbase.c

/**** externals ****/

extern struct EditWindow **EditWindowList;
extern struct EditSupport	**EditSupportList;
extern struct CapsPrefs CPrefs;
extern struct Window *pageWindow;
extern struct Screen *pageScreen;
extern struct PageFuncs pageFuncs[];
extern struct TextFont *largeFont;
extern UBYTE **msgs;   
extern struct Library *medialinkLibBase;
extern struct EditWindow backEW;
extern struct EditSupport backES;

/**** globals ****/

UBYTE *crawlBuff;

/**** static globals ****/

static struct PageInfoRecord PageInfoRec;

/**** functions ****/

/******** ReadPage() ********/
/*
 * This functions:
 *
 * - loads an ILBM or a document,
 * - changes from screen mode (if needed),
 * - sets the right page colors,
 * - opens windows (if any).
 *
 */

BOOL ReadPage(STRPTR path, STRPTR fileName, char **pageCommands, BOOL ask)
{
TEXT fullPath[SIZE_FULLPATH];
BOOL retval=FALSE;
ULONG type;

	UA_MakeFullPath(path, fileName, fullPath);
	crawlBuff = NULL;
	SetSpriteOfActWdw(SPRITE_BUSY);

	type = checkFileType(fullPath, NULL);
	if (type==ILBM || type==ANIM)
	{
		/**** selected file is an IFF picture ****/
		retval = CreatePageFromILBM(path, fileName, fullPath, pageCommands, ask);
		if ( retval )
			DrawAllWindows();
	}
	else
	{
		/**** selected file is a document ****/
		retval = CreatePageFromDocument(path, fileName, fullPath, pageCommands, ask);
	}

	if ( crawlBuff != NULL )
		FreeMem(crawlBuff,2048L);
	SetSpriteOfActWdw(SPRITE_NORMAL);

	return(retval);
}

/******** CreatePageFromILBM() ********/

BOOL CreatePageFromILBM(STRPTR path, STRPTR fileName, STRPTR fullPath,
												char **pageCommands, BOOL ask)
{
struct IFF_FRAME iff;
BOOL noCAMG=FALSE;
ULONG IFF_ID;

	/**** INIT THE IFF FRAME ****/

	FastInitIFFFrame(&iff);

	/**** OPEN THE PICTURE ****/

	IFF_ID = FastOpenIFF(&iff, fullPath);
	if ( iff.Error || (IFF_ID!=ILBM && IFF_ID!=ANIM) )
	{
		FastCloseIFF(&iff);
		Message(msgs[Msg_UnableToReadPic-1], fileName);
		return(FALSE);
	}

	/**** SCAN THE BITMAP HEADER ****/

	FastParseChunk(&iff, BMHD);
	if (iff.Error != NULL)
	{
		FastCloseIFF(&iff);
		Message(msgs[Msg_ProblemsWithIFFPic-1], fileName);
		return(FALSE);
	}

	FastParseChunk(&iff, CAMG);
	if (iff.Error != NULL)
	{
		noCAMG=TRUE;
		Message(msgs[Msg_IFFChunkMisses-1], "CAMG");
		iff.viewModes=0L;
	}

	FastCloseIFF(&iff);

	/**** ask if page must be re-opened ****/

	if (	!noCAMG && (
				(iff.viewModes != CPrefs.PageScreenModes) ||
				(iff.BMH.w != CPrefs.PageScreenWidth) ||
				(iff.BMH.h != CPrefs.PageScreenHeight) ||
				(iff.BMH.nPlanes != CPrefs.PageScreenDepth) ))
	{
		if ( !ask || UA_OpenGenericWindow(	pageWindow, TRUE, TRUE, msgs[Msg_OK-1], msgs[Msg_Cancel-1],
																				QUESTION_ICON, msgs[Msg_ChangeScreenSize-1], TRUE, NULL ) )
		{
			if ( ChangePageIntoIFFSize(&iff) )
			{
				OpenNewPageScreen(FALSE,TRUE,TRUE, FALSE);
				SetSpriteOfActWdw(SPRITE_BUSY);
			}
		}
	}

	/**** open window ****/

	ImportABackground(FALSE,FALSE,fullPath,fileName);
	if ( backES.cm )
		SetScreenToCM(pageScreen, backES.cm);
	SyncAllColors(TRUE);

	return(TRUE);
}

/******** CreatePageFromDocument() ********/

BOOL CreatePageFromDocument(STRPTR path, STRPTR fileName, STRPTR fullPath,
														char **pageCommands, BOOL ask)
{
struct ParseRecord *PR;
TEXT buffer[MAXSCANDEPTH];
int instruc, line, wdwNr;
BOOL validPage=TRUE;
BOOL headerPassed=FALSE;
BOOL winsopen=FALSE;
int i;
ULONG modes;

	/**** init vars ****/

	instruc = -1;
	line=0;
	wdwNr = 0;

	/**** open document file ****/

	PR = (struct ParseRecord *)OpenParseFile(pageCommands, fullPath);
	if (PR==NULL)
	{
		Message(msgs[Msg_UnableToReadDoc-1], fileName);
		return(FALSE);
	}

	InitPageInfoRecord(&PageInfoRec);

	/**** parse all the lines ****/

	while(instruc != PARSE_STOP)
	{
		/**** get one line of source ****/

		instruc = GetParserLine((struct ParseRecord *)PR, buffer);

		if (instruc == PARSE_INTERPRET)
		{
			passOneParser((struct ParseRecord *)PR, buffer);
			if (passTwoParser((struct ParseRecord *)PR))
			{
				if (line>=1000)	/* check for too large page description file */
				{
					Message(msgs[Msg_DocTooLarge-1]);
					validPage=FALSE;
					instruc=PARSE_STOP;
				}
				else if (line==0 && PR->commandCode!=TALK_PAGETALK)	/* check for page description file validity */
				{
					Message(msgs[Msg_DocUnreadable-1]);
					validPage=FALSE;
					instruc=PARSE_STOP;
				}
				else
				{
					if ( PR->commandCode == TALK_PAGETALK )
						headerPassed = TRUE;

					if ( PR->commandCode == TALK_WINDOW )
					{
						if (InitNewWindow( &PageInfoRec, wdwNr ))
							wdwNr++;
					}
					else if ( PR->commandCode == TALK_BACKGROUND )
						InitNewWindow( &PageInfoRec, -1 );
	
					PR->sourceLine = line+1;
					PerfFunc((struct GenericFuncs *)pageFuncs, PR, (struct ScriptInfoRecord *)&PageInfoRec);
					if (PR->commandCode == PRINTERROR_CODE)	/* an error was reported so stop parsing */
					{
						validPage=FALSE;
						instruc=PARSE_STOP;
					}
				}
			}
			else
			{
				/**** catch weird command after line 0, line 0 is treated above ****/
				if (line>0 && PR->commandCode==-1)	/* command not valid */
				{
					sprintf(buffer, msgs[Msg_IllegalCommandInLine-1], line+1);
					printError(PR, buffer);
					validPage=FALSE;
					instruc=PARSE_STOP;
				}
			}
		}
		line++;	/* keeps track of number of parsed lines */
	}

	CloseParseFile(PR);

	if (!headerPassed)
		validPage=FALSE;

	if (!validPage)
	{
		FreeColorMap(PageInfoRec.LoadedCM);
		return(FALSE);
	}

	/**** ask if page must be re-opened ****/

	modes = CPrefs.PageScreenModes & ( LACE | HAM_KEY | EXTRAHALFBRITE_KEY );

	if (	(PageInfoRec.LoadedScreenWidth != CPrefs.PageScreenWidth) ||
				(PageInfoRec.LoadedScreenHeight != CPrefs.PageScreenHeight) ||
				(PageInfoRec.LoadedScreenDepth != CPrefs.PageScreenDepth) ||
				(PageInfoRec.LoadedScreenModes != modes) ||
				(PageInfoRec.LoadedMonitorID != CPrefs.pageMonitorID) ||
				(PageInfoRec.LoadedOverScan != CPrefs.overScan) )
	{
		if ( !ask || UA_OpenGenericWindow(	pageWindow, TRUE, TRUE,
																				msgs[Msg_OK-1], msgs[Msg_Cancel-1],
																				QUESTION_ICON,
																				msgs[Msg_ChangeScreenSize-1],
																				TRUE, NULL) )
		{
			/**** open a screen ****/

			CPrefs.PageScreenWidth	= PageInfoRec.LoadedScreenWidth;
			CPrefs.PageScreenHeight	= PageInfoRec.LoadedScreenHeight;
			CPrefs.PageScreenDepth	= PageInfoRec.LoadedScreenDepth;
			CPrefs.PageScreenModes	=	PageInfoRec.LoadedScreenModes;
			CPrefs.pageMonitorID		=	PageInfoRec.LoadedMonitorID;
			CPrefs.overScan					=	PageInfoRec.LoadedOverScan;

			OpenNewPageScreen(FALSE,TRUE,TRUE, FALSE);	// also re-opens Edit Windows
			SetSpriteOfActWdw(SPRITE_BUSY);
			winsopen=TRUE;
		}
	}

	/**** set page colors to picture colors ****/

	SetScreenToCM(pageScreen, PageInfoRec.LoadedCM);
	SyncAllColors(TRUE);

	if ( !winsopen )
	{
		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if ( EditWindowList[i] != NULL )
			{
/*
				ValidateBoundingBox(&EditWindowList[i]->X, &EditWindowList[i]->Y,
														&EditWindowList[i]->Width,
														&EditWindowList[i]->Height);
*/
				CorrectEW(EditWindowList[i]);
				OpenEditWindow(i,0,0,0,0);
			}
		}
	}

	FreeColorMap(PageInfoRec.LoadedCM);

	GetAllTexts(fullPath, pageCommands);

	if ( backES.ori_bm.Planes[0] )
		ShowBackground();

	for(i=0; i<MAXEDITWINDOWS; i++)
		if ( EditWindowList[i] != NULL )
			DrawEditWindow(EditWindowList[i], EditSupportList[i]);

	return(TRUE);
}

/******** InitPageInfoRecord() ********/

void InitPageInfoRecord(struct PageInfoRecord *PIR)
{
	PIR->ew												= NULL;
	PIR->es												= NULL;
	PIR->version									= 0;
	PIR->revision									= 0;
	PIR->LoadedScreenWidth				= 0;
	PIR->LoadedScreenHeight				= 0;
	PIR->LoadedScreenDepth				= 0;
	PIR->LoadedScreenModes				= 0;

	if (GfxBase->LibNode.lib_Version >= 39)
		PIR->LoadedCM								= GetColorMap(256);
	else
		PIR->LoadedCM								= GetColorMap(32);

	PIR->LoadedMonitorID					= 0L;
	PIR->LoadedOverScan						= 0;
}

/******** InitNewWindow() ********/

BOOL InitNewWindow(struct PageInfoRecord *PIR, int wdwNr)
{
struct EditWindow *ew;
struct EditSupport *es;
int i;

	PIR->ew = NULL;	/* maybe an error occurs */
	PIR->es = NULL;

	if ( wdwNr == -1 )	// for backEW
	{
		ClearBackWindow();
		ew = &backEW;
		es = &backES;
	}
	else
	{
		ew = (struct EditWindow *)AllocMem(sizeof(struct EditWindow), MEMF_ANY | MEMF_CLEAR);
		if (ew == NULL)
		{
			UA_WarnUser(129);
			return(FALSE);
		}
		EditWindowList[wdwNr] = ew;

		es = (struct EditSupport *)AllocMem(sizeof(struct EditSupport), MEMF_ANY | MEMF_CLEAR);
		if (es == NULL)
		{
			UA_WarnUser(130);
			return(FALSE);
		}
		EditSupportList[wdwNr] = es;

		ew->X 								= 0;
		ew->Y 								= 0;
		ew->Width 						= 50;
		ew->Height 						= 50;
		ew->TopMargin					= DEFAULT_TM;
		ew->BottomMargin			= DEFAULT_BM;
		ew->LeftMargin				= DEFAULT_LM;
		ew->RightMargin				= DEFAULT_RM;
		ew->Border						= DEFAULT_BORDER;
		ew->BorderColor[0]		= DEFAULT_BCOLOR;
		ew->BorderColor[1]		= DEFAULT_BCOLOR;
		ew->BorderColor[2]		= DEFAULT_BCOLOR;
		ew->BorderColor[3]		= DEFAULT_BCOLOR;
		ew->BorderWidth				= DEFAULT_BWIDTH;
		ew->BackFillType			= DEFAULT_BFTYPE;
		ew->BackFillColor			= DEFAULT_BFCOLOR;
		ew->PhotoOffsetX			= 0;
		ew->PhotoOffsetY			= 0;
		ew->patternNum				= DEFAULT_PATTERN;
		ew->DrawSeqNum				= wdwNr+1;

		ew->TEI								= NULL;

		ew->charFont					= largeFont;
		ew->underlineColor		= 2;
		ew->charStyle					= 0;
		ew->charColor					= 2;

		ew->flags							= 0;

		for(i=0; i<3; i++)
		{
			ew->in1[i]					= -1;
			ew->in2[i]					= -1;
			ew->in3[i]					= -1;
			ew->out1[i]					= -1;
			ew->out2[i]					= -1;
			ew->out3[i]					= -1;
			ew->inDelay[i]			= 0;
			ew->outDelay[i]			= 0;
		}

		ew->antiAliasLevel		= 0;
		ew->justification			= 0;
		ew->xSpacing					= 0;
		ew->ySpacing					= 0;
		ew->slantAmount				= 2;
		ew->slantValue				= 1;
		ew->underLineHeight		= 1;
		ew->underLineOffset		= 0;
		ew->shadowDepth				= 0;
		ew->shadow_Pen				= 0;
		ew->shadowType				= 0;
		ew->shadowDirection		= 0;

		ew->wdw_shadowDepth		= 4;
		ew->wdw_shadowDirection = 0;
		ew->wdw_shadowPen			= 1;

		ew->crawl_fontName[0]	= '\0';
		ew->crawl_fontSize		= 0;
		ew->crawl_speed				= 0;
		ew->crawl_flags				= 0;
		ew->crawl_text				= NULL;
		ew->crawl_length			= 0;
		ew->crawl_color				= 1;

		ew->bx								= -1;
		ew->by								= -1;
		ew->bwidth						= -1;
		ew->bheight						= -1;
		ew->jumpType					= 0;
		ew->renderType				= RENDERTYPE_INVERT;
		ew->audioCue					= 0;
		ew->keyCode						= -1;	
		ew->rawkeyCode				= -1;
		ew->buttonName[0]			= '\0';
		ew->assignment[0]			= '\0';

		ew->animIsAnim				= FALSE;
		ew->animLoops					= 0;
		ew->animSpeed					= 0;
		ew->animFromDisk			= 0;
		ew->animAddFrames			= 0;

		es->Active						= FALSE;
		es->iff								= NULL;
		es->photoOpts					= 0;
		es->picPath[0]				= '\0';
		es->cm								= NULL;

		ClearBitMap24(&es->ori_bm);
		es->ori_w							= 0;
		es->ori_h							= 0;

		ClearBitMap(&es->scaled_bm);
		es->scaled_w					= 0;
		es->scaled_h					= 0;

		ClearBitMap(&es->remapped_bm);
		es->remapped_w				= 0;
		es->remapped_h				= 0;

		ClearBitMap(&es->restore_bm);
		es->restore_w					= 0;
		es->restore_h					= 0;

		ClearBitMap(&es->mask_bm);
		es->mask_w						= 0;
		es->mask_h						= 0;

		ClearBitMap(&es->ori_mask_bm);
		es->ori_mask_w				= 0;
		es->ori_mask_h				= 0;

		es->ditherMode				= DITHER_FLOYD;
	}

	PIR->ew = ew;
	PIR->es = es;

	return(TRUE);
}

/******** GetAllTexts() ********/

int GetAllTexts(STRPTR fullPath, char **pageCommands)
{
struct ParseRecord *PR;
TEXT buf[MAXSCANDEPTH];
int instruc, line, wdwNr, pos, numTexts;
UBYTE *buffer;

	/**** init vars ****/

	instruc	= -1;
	line		= 0;
	wdwNr		= -1;
	pos = 0;
	numTexts = 0;

	/**** open document file ****/

	PR = (struct ParseRecord *)OpenParseFile(pageCommands, fullPath);
	if (PR==NULL)
		return(0);

	InitPageInfoRecord(&PageInfoRec);

	buffer = (UBYTE *)AllocMem(RAWBUFSIZE, MEMF_ANY | MEMF_CLEAR);
	if (buffer==NULL)
		return(0);

	/**** parse all the lines ****/

	while(instruc != PARSE_STOP)
	{
		/**** get one line of source ****/

		instruc = GetParserLine((struct ParseRecord *)PR, buf);

		if (instruc == PARSE_INTERPRET)
		{
			passOneParser((struct ParseRecord *)PR, buf);
			if (passTwoParser((struct ParseRecord *)PR))
			{
				if (line>=1000)	/* check for too large page description file */
				{
					Message(msgs[Msg_DocTooLarge-1]);
					instruc=PARSE_STOP;
				}
				else if (line==0 && PR->commandCode!=TALK_PAGETALK)
				{
					Message(msgs[Msg_DocUnreadable-1]);
					instruc=PARSE_STOP;
				}
				else if (PR->commandCode==TALK_WINDOW)
				{
					wdwNr++;
					if (wdwNr>0)
					{
						buffer[pos] = 0;
						pos=0;
						ParseTextBuffer(buffer, wdwNr-1);
						buffer[0] = '\0';
					}
				}
				else if ( PR->commandCode==TALK_TEXT && wdwNr!=-1 )
				{
					numTexts++;
					if ((pos+strlen(PR->argString[1])) < RAWBUFSIZE)
					{
						strcpy(buffer+pos, &PR->argString[1][1]);	// survived strcpy
						pos = pos + (strlen(PR->argString[1])-2);
					}
				}
				else if ( PR->commandCode==TALK_OBJECTEND )
				{
					if (pos!=0)
					{
						buffer[pos] = 0;
						ParseTextBuffer(buffer, wdwNr);
						buffer[0] = '\0';
					}
				}
			}
			else
			{
				/**** catch weird command after line 0, line 0 is treated above ****/
				if (line>0 && PR->commandCode==-1)	/* command not valid */
				{
					sprintf(buf, msgs[Msg_IllegalCommandInLine-1], line+1);
					printError(PR, buf);
					instruc=PARSE_STOP;
				}
			}
		}
		line++;	/* keeps track of number of parsed lines */
	}

	CloseParseFile(PR);

	FreeColorMap(PageInfoRec.LoadedCM);

	FreeMem(buffer, RAWBUFSIZE);

	return(numTexts);
}

/******** ParseTextBuffer() ********/

void ParseTextBuffer(UBYTE *buffer, int wdwNr)
{
int textlength;
struct EditWindow *ew;
struct EditSupport *es;

	ew = EditWindowList[wdwNr];
	es = EditSupportList[wdwNr];

	if ( buffer[0]=='\0' )
		return;

	if ( !ParseText(NULL, buffer, ew, es, &textlength) )
		return;

	/**** clear window ****/

	ew->TEI->textLength = textlength;

	TESetSelect(ew->TEI, 0, 0);						// herstel cursor op linksboven

	TESetFont(ew->TEI, largeFont);				// zet het default font
	TESetColor(ew->TEI, 2);								// zet de default kleur
	TESetStyle(ew->TEI, 0, 0xFF);					// zet de default style

	TESetUpdateRange( ew, LEVEL_FULL );
}

/******** ChangePageIntoIFFSize() ********/

BOOL ChangePageIntoIFFSize(struct IFF_FRAME *iff)
{
ULONG monID, modes, psmodes;
WORD w,h,maxDepth;
int oscan;
BOOL ret2;

	// IFF_FRAME tells me:	iff->BMH.w and iff->BMH.h
	//											iff->BMH.nPlanes
	//											iff->viewModes
	//											iff->colorMap

	ret2 = GetDimsFromIFF(iff,&w,&h,&oscan,&maxDepth,&monID,&modes,FALSE);
	if ( !ret2 )
		ret2 = GetDimsFromIFF(iff,&w,&h,&oscan,&maxDepth,&monID,&modes,TRUE);

	if ( ret2 )
	{
		psmodes = modes;
		if ( iff->viewModes & HAM_KEY )
			psmodes |= HAM_KEY;
		else if ( iff->viewModes & EXTRAHALFBRITE_KEY )
			psmodes |= EXTRAHALFBRITE_KEY;

		if ( ModeNotAvailable(monID | psmodes) )
		{
			if ( ModeNotAvailable(monID) )
			{
				return(FALSE);	// forget it -> use current monitor and display modes
			}
		}

		CPrefs.PageScreenWidth = w;
		CPrefs.PageScreenHeight = h;

		CPrefs.overScan = oscan;

		CPrefs.PageScreenDepth = iff->BMH.nPlanes;
		if ( !(psmodes & HAM_KEY) && !(psmodes & EXTRAHALFBRITE_KEY) )
		{
			if ( CPrefs.PageScreenDepth > maxDepth )
				CPrefs.PageScreenDepth = maxDepth;
		}

		CPrefs.pageMonitorID = monID;

		CPrefs.PageScreenModes = psmodes;

		return(TRUE);
	}

	return(FALSE);
}

/******** E O F ********/
