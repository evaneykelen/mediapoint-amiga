/*******************************************************************/
/*
 *
 *  T H I S   I S   A L S O   A   P L A Y E R   M O D U L E !
 *
 *
 */

#include "nb:pre.h"

/**** externals ****/

extern struct CapsPrefs CPrefs;
extern struct TextFont *largeFont;
extern struct TextFont *textFont;
extern UBYTE **msgs;   
extern UBYTE *crawlBuff;
extern struct Library *medialinkLibBase;
extern BOOL do_the_tile;

/**** GLOBALS ****/

char *pageCommands[] = {
"SCREEN", "PALETTE", "WINDOW", "CLIP", "TEXT", "PAGETALK", "EFFECT",
"FORMAT", "STYLE", "CRAWL", "BUTTON", "OBJECTSTART", "OBJECTEND",
"BACKGROUND", "CLIPANIM",
NULL }; /* ALWAYS END WITH NULL! */

struct PageFuncs pageFuncs[] =
{ 
	{ ScreenFunc			},
	{ PaletteFunc			},
	{ WindowFunc			},
	{ ClipFunc				},
	{ TextFunc				},
	{ PageTalkFunc		},
	{ EffectFunc			},
	{ FormatFunc			},
	{ StyleFunc				},
	{ CrawlFunc				},
  { ButtonFunc      },
	{ ObjectStartFunc	},
	{ ObjectEndFunc		},
	{ BackgroundFunc	},
	{ ClipAnimFunc		},
};

/**** FUNCTIONS ****/

/******** ScreenFunc() ********/

void ScreenFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR)
{
#ifndef USED_FOR_PLAYER
WORD args[MAX_PARSER_ARGS];
WORD w,h,d,d2;
ULONG modes, monID;
int overScan;
struct DisplayInfo dispinfo;

	/*  SCREEN mode1, mode2,  depth, mode3        				*/
	/*  SCREEN width, height, depth, mode2, oscan, mode1	*/
	/*	where mode2	= 0
	 *								1 = HAM
	 *								2 = EHB
	 * BTW mode1 is not used by the page editor in new style only by the player
	 */

	if (PR->numArgs==5 || PR->numArgs==7)
	{
		GetNumericalArgs(PR, args);

		monID = CPrefs.pageMonitorID;
		d = *(args+2);	// depth	

		if ( PR->numArgs==5 )	// old-style
		{
			modes = GetModesFromMode1And2(0,*(args+1));	// HAM or EHB.
			overScan = *(args+3);	// overscan 
			if ( !GetDimsFromMode1And2And3(*(args+0),*(args+1),*(args+3),&w,&h) )
			{
				w = CPrefs.PageScreenWidth;
				h = CPrefs.PageScreenHeight;
				d = CPrefs.PageScreenDepth;
				modes = CPrefs.PageScreenModes;
				overScan = CPrefs.overScan;
			}
			else
			{
				// recalc overscan modes
				if ( overScan == 1 )
					overScan = 3;
				else if ( overScan == 2 )
					overScan = 4;
			}
		}
		else if ( PR->numArgs==7 )	// new-style
		{
			modes = GetModesFromMode1And2(0,*(args+3));	// HAM or EHB.
			overScan = *(args+4);	// overscan 
			w = *(args+0);
			h = *(args+1);
		}

		if ( !GetIDWithMonName(CPrefs.pageMonName, &monID, w, h, overScan) )
		{
			if ( !GetIDWithMonName(CPrefs.pageMonName, &monID, w, -1, overScan) )
			{
				if ( !GetIDWithMonName(CPrefs.pageMonName, &monID, -1, -1, 0) )
				{
					if ( CPrefs.PagePalNtsc==PAL_MODE )
						monID = PAL_MONITOR_ID;
					else
						monID = NTSC_MONITOR_ID;
				}
			}
		}

		if ( !GetInfoOnModeID(monID,&w,&h,&d2,overScan) )
		{
			w = CPrefs.PageScreenWidth;
			h = CPrefs.PageScreenHeight;
			d = CPrefs.PageScreenDepth;
			modes = CPrefs.PageScreenModes;
			monID = CPrefs.pageMonitorID;
			overScan = CPrefs.overScan;
		}
		else
		{
			if ( !(modes & HAM_KEY) && !(modes & EXTRAHALFBRITE_KEY) )
			{
				if ( d > d2 )
					d = d2;	// maxDepth limit was exceeded
			}
		}

		if ( GetDisplayInfoData(NULL,(UBYTE *)&dispinfo,sizeof(struct DisplayInfo),DTAG_DISP,monID) )
		{
			if ( dispinfo.PropertyFlags & DIPF_IS_LACE )
				modes |= LACE;
		}
		if ( h >= 400 )
			modes |= LACE;				

		PIR->LoadedScreenWidth	= w;
		PIR->LoadedScreenHeight	= h;
		PIR->LoadedScreenDepth	= d;
		PIR->LoadedScreenModes	= modes;
		PIR->LoadedMonitorID		= monID;
		PIR->LoadedOverScan			= overScan;
	}
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** PaletteFunc() ********/

void PaletteFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR)
{
#ifndef USED_FOR_PLAYER
int i, value, len, max;
WORD args[MAX_PARSER_ARGS];
ULONG r,g,b;

	/*  PALETTE 0xnnn, ..., 0xmmm  */
	/*  PALETTE "rgb rgb rgb rgb"  */
	/*  PALETTE n "rrggbb rrggbb rrggbb" */

	if (GfxBase->LibNode.lib_Version >= 39)
		max = 256;
	else
		max = 32;

	if (PR->numArgs==3)
	{
		GetNumericalArgs(PR, args);

		if ( (((*args)-1)*8) >= max )
			return;

		len = strlen( PR->argString[2] ) - 2;	// forget 2 double quotes

		for(i=0; i<(len/7); i++)
		{
			sscanf(&PR->argString[2][1+(i*7)], "%06x ", &value);

			if ( !CPrefs.AA_available )
			{
				// Palette entries are stored like rrggbb, rrggbb etc.
				// When reading an AA document, the SetColorCM32 functions chops off
				// The high-byte. Here I try to save the high-byte in order to read AA
				// docs properly on ECS machines.

				r = (value & 0x00ff0000) >> 16;
				r = r / 16;
				r = r << 16;

				g = (value & 0x0000ff00) >> 8;
				g = g / 16;
				g = g << 8;

				b = value & 0x000000ff;
				b = b / 16;

				value = r|g|b;
			}

			SetColorCM32(PIR->LoadedCM, value, (((*args)-1)*8+i) );
		}
	}
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** WindowFunc() ********/

void WindowFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR)
{
#ifndef USED_FOR_PLAYER
WORD args[MAX_PARSER_ARGS];

	/* WINDOW x,y,w,h,tbc,rbc,bbc,lbc,bw,ic,it,flags[,pn] - 13/14												*/
	/* WINDOW x,y,w,h,tbc,rbc,bbc,lbc,bw,ic,it,flags,tm,rm,bm,lm[,pn] - 17/18						*/
	/* WINDOW x,y,w,h,tbc,rbc,bbc,lbc,bw,ic,it,flags,tm,rm,bm,lm,pn,sdep,sdir,spen - 21	*/

	if (	PR->numArgs==13 || PR->numArgs==14 || PR->numArgs==17 || PR->numArgs==18 ||
				PR->numArgs==21 )
	{
		GetNumericalArgs(PR, args);

		PIR->ew->X 			= *(args+0);
		PIR->ew->Y			= *(args+1);
		PIR->ew->Width	= *(args+2);
		PIR->ew->Height	= *(args+3);

		if ( *(args+4) != -1 ) 	// top border color
		{
			PIR->ew->BorderColor[0]	= *(args+4);
			PIR->ew->Border	|= BORDER_TOP;
		}

		if ( *(args+5) != -1 )	// right border color
		{
			PIR->ew->BorderColor[1]	= *(args+5);
			PIR->ew->Border	|= BORDER_RIGHT;
		}

		if ( *(args+6) != -1 )	// bottom border color
		{
			PIR->ew->BorderColor[2]	= *(args+6);
			PIR->ew->Border	|= BORDER_BOTTOM;
		}

		if ( *(args+7) != -1 )	// left border color
		{
			PIR->ew->BorderColor[3]	= *(args+7);
			PIR->ew->Border	|= BORDER_LEFT;
		}

		PIR->ew->BorderWidth		 	= *(args+8);
		PIR->ew->BackFillColor		= *(args+9);
		PIR->ew->BackFillType			= *(args+10)-1;

		PIR->ew->flags						= *(args+11);

		if (PR->numArgs>=17) // be backwards compatible
		{
			PIR->ew->TopMargin				= *(args+12);
			PIR->ew->RightMargin			= *(args+13);
			PIR->ew->BottomMargin			= *(args+14);
			PIR->ew->LeftMargin				= *(args+15);
		}

		if (PR->numArgs==14)
			PIR->ew->patternNum				= *(args+12);
		else if (PR->numArgs==18 || PR->numArgs==21)
			PIR->ew->patternNum				= *(args+16);
		else
			PIR->ew->patternNum				= DEFAULT_PATTERN;

		if ( PR->numArgs==21 )
		{
			PIR->ew->wdw_shadowDepth	= *(args+17);
			PIR->ew->wdw_shadowDirection = *(args+18);
			PIR->ew->wdw_shadowPen		= *(args+19);
		}
	}
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** ClipFunc() ********/

void ClipFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR)
{
#ifndef USED_FOR_PLAYER
WORD args[MAX_PARSER_ARGS];
TEXT path[256], fileName[100], scrStr[256];
BOOL retval;

	/*  CLIP "path", x, y, w, h, scale  */

	if ( PIR->ew==NULL )
	{
		printError(PR, msgs[Msg_MissesObjectStart-1]);
		return;
	}

	if (PR->numArgs==7)
	{
		GetNumericalArgs(PR, args);

		PIR->ew->PhotoOffsetX	= *(args+1);
		PIR->ew->PhotoOffsetY	= *(args+2);

		/**** load ilbm ****/

		ScriptToStr(&PR->argString[1][0], scrStr);
		UA_SplitFullPath(scrStr, path, fileName);
		stccpy(PIR->es->picPath, scrStr, SIZE_FULLPATH);

		if (strcmpi(&PR->argString[6][0], "ON")==0)
			PIR->es->photoOpts = SIZE_PHOTO;
		else
			PIR->es->photoOpts = MOVE_PHOTO;

		retval = doActualILBMLoading(PIR->es->picPath, fileName, PIR->es, PIR->ew, FALSE);
		if (!retval)
		{
			PIR->es->photoOpts = 0;
			PIR->es->picPath[0] = '\0';
			PIR->ew->PhotoOffsetX	= 0;
			PIR->ew->PhotoOffsetY	= 0;
		}

		if ( PIR->ew->flags & EW_IS_TILED )
		{
			do_the_tile = TRUE;
			UnSetByteBit(&PIR->ew->flags, EW_IS_TILED);
		}
	}
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** TextFunc() ********/

void TextFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR)
{
}

/******** PageTalkFunc() ********/

void PageTalkFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR)
{
#ifndef USED_FOR_PLAYER
WORD args[MAX_PARSER_ARGS];

	if (PR->numArgs==3)
	{
		GetNumericalArgs(PR, args);
		PIR->version	= *(args+0);
		PIR->revision	= *(args+1);
	}
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** EffectFunc() ********/

void EffectFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR)
{
#ifndef USED_FOR_PLAYER
WORD args[MAX_PARSER_ARGS];

	if ( PIR->ew==NULL )
		return;

	/* EFFECT num, in1, in2, in3, out1, out2, out3, delay1, delay2 */

	if (PR->numArgs==10)
	{
		GetNumericalArgs(PR, args);

		PIR->ew->in1[				*(args+0) ]	= *(args+1);
		PIR->ew->in2[				*(args+0) ]	= *(args+2);
		PIR->ew->in3[				*(args+0) ]	= *(args+3);
		PIR->ew->out1[			*(args+0) ]	= *(args+4);
		PIR->ew->out2[			*(args+0) ]	= *(args+5);
		PIR->ew->out3[			*(args+0) ]	= *(args+6);
		PIR->ew->inDelay[ 	*(args+0) ]	= *(args+7);
		PIR->ew->outDelay[	*(args+0) ]	= *(args+8);
	}
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** FormatFunc() ********/

void FormatFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR)
{
#ifndef USED_FOR_PLAYER
WORD args[MAX_PARSER_ARGS];

	if ( PIR->ew==NULL )
		return;

	/*	FORMAT antiAL, just, xsp, ysp, slantA, slantV, undH, undO, sDep,
			sPen, sTyp, sDir	*/

	if (PR->numArgs==13)
	{
		GetNumericalArgs(PR, args);
		PIR->ew->antiAliasLevel		= *(args+0);
		PIR->ew->justification		= *(args+1);
		PIR->ew->xSpacing					= *(args+2);
		PIR->ew->ySpacing					= *(args+3);
		PIR->ew->slantAmount			= *(args+4);
		PIR->ew->slantValue				= *(args+5);
		PIR->ew->underLineHeight	= *(args+6);
		PIR->ew->underLineOffset	= *(args+7);
		PIR->ew->shadowDepth			= *(args+8);
		PIR->ew->shadow_Pen				= *(args+9);
		PIR->ew->shadowType				= *(args+10);
		PIR->ew->shadowDirection	= *(args+11);
	}
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** StyleFunc() ********/

void StyleFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR)
{
#ifndef USED_FOR_PLAYER
WORD args[MAX_PARSER_ARGS];
TEXT fontname[50];

	if ( PIR->ew==NULL )
		return;

	/*	STYLE "fontname", fontsize, style, color, [underlineColor]  */

	if (PR->numArgs==5 || PR->numArgs==6)
	{
		GetNumericalArgs(PR, args);

		strcpy(fontname, PR->argString[1]);
		strcat(fontname, ".font");

		if ( OpenTypeFace(fontname, *(args+1), 0, TRUE) )
			PIR->ew->charFont = textFont;
		else
			PIR->ew->charFont = largeFont;

		PIR->ew->charStyle = *(args+2);

		PIR->ew->charColor = *(args+3);

		if (PR->numArgs==6)
			PIR->ew->underlineColor = *(args+4);
	}
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** CrawlFunc() ********/

void CrawlFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR)
{
#ifndef USED_FOR_PLAYER
WORD args[MAX_PARSER_ARGS];
int i;
TEXT scrStr[100];

	if ( PIR->ew==NULL )
		return;

	/*	CRAWL "fontname", size, speed, color  						*/
	/*	CRAWL "text"																			*/
	/*  or																								*/
	/*	CRAWL "fontname", size, speed, color, "filename"  */

	if (PR->numArgs==5 || PR->numArgs==6)
	{
		GetNumericalArgs(PR, args);
		RemoveQuotes(&PR->argString[1][0]);
		strcpy(PIR->ew->crawl_fontName,&PR->argString[1][0]);
		PIR->ew->crawl_fontSize = *(args+1);
		PIR->ew->crawl_speed 		= *(args+2);
		PIR->ew->crawl_color 		= *(args+3);

		if ( crawlBuff==NULL )
		{
			crawlBuff = (UBYTE *)AllocMem(2048L, MEMF_CLEAR | MEMF_ANY);
			if (crawlBuff==NULL)
				UA_WarnUser(-1);
		}
		else
		{
			for(i=0; i<2048; i++)
				*(crawlBuff+i)=(UBYTE)0;
		}

		if ( PR->numArgs==6 )
		{
			ScriptToStr(PR->argString[5],scrStr);	
			RemoveQuotes(scrStr);
			strcpy(crawlBuff,scrStr);
			PIR->ew->crawl_length = strlen(crawlBuff)+10;
			PIR->ew->crawl_text = (UBYTE *)AllocMem(PIR->ew->crawl_length, MEMF_CLEAR | MEMF_ANY);
			if (PIR->ew->crawl_text!=NULL)
				strcpy(PIR->ew->crawl_text,crawlBuff);
			PIR->ew->crawl_flags |= 1;	// from file
		}
	}
	else if (PR->numArgs==2)
	{
		if (crawlBuff)
		{
			ScriptToStr(PR->argString[1],scrStr);
			RemoveQuotes(scrStr);
			strcat(crawlBuff,scrStr);

			if ( strlen(scrStr) == 1 )	// last line, all others are +/- 70
			{
				PIR->ew->crawl_length = strlen(crawlBuff)+10;
				PIR->ew->crawl_text = (UBYTE *)AllocMem(PIR->ew->crawl_length, MEMF_CLEAR | MEMF_ANY);
				if (PIR->ew->crawl_text!=NULL)
				{
					strcpy(PIR->ew->crawl_text,crawlBuff);
					PIR->ew->crawl_text[ strlen(PIR->ew->crawl_text)-1 ] = '\0';
				}
			}
		}
	}
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** ButtonFunc() ********/

void ButtonFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR)
{
WORD args[MAX_PARSER_ARGS];
TEXT scrStr[100];

	if ( PIR->ew==NULL )
		return;

	// BUTTON x,y,w,h, jumpType, renderType, audioCue, "buttonName", "key" (10)
	// BUTTON x,y,w,h, jumpType, renderType, audioCue, "buttonName", "rawkey" (10)
	// BUTTON x,y,w,h, jumpType, renderType, audioCue, "buttonName" (9)

	// NEW FOR 128...

	// BUTTON x,y,w,h, jumpType, renderType, audioCue, "buttonName", "key", "assignment" (11)
	// BUTTON x,y,w,h, jumpType, renderType, audioCue, "buttonName", "rawkey", "assignment" (11)
	// BUTTON x,y,w,h, jumpType, renderType, audioCue, "buttonName", "", "assignment" (11)

	if ( PR->numArgs==9 || PR->numArgs==10 || PR->numArgs==11 )
	{
		GetNumericalArgs(PR, args);

		PIR->ew->bx				= *(args+0);
		PIR->ew->by				= *(args+1);
		PIR->ew->bwidth		= *(args+2);
		PIR->ew->bheight	= *(args+3);

		if (strcmpi(&PR->argString[5][0], JUMPTYPE_GOTO_TEXT)==0)
			PIR->ew->jumpType = JUMPTYPE_GOTO;
		else if (strcmpi(&PR->argString[5][0], JUMPTYPE_GOSUB_TEXT)==0)
			PIR->ew->jumpType = JUMPTYPE_GOSUB;
		else if (strcmpi(&PR->argString[5][0], JUMPTYPE_PREV_TEXT)==0)
			PIR->ew->jumpType = JUMPTYPE_PREV;
		else if (strcmpi(&PR->argString[5][0], JUMPTYPE_NEXT_TEXT)==0)
			PIR->ew->jumpType = JUMPTYPE_NEXT;
		else if (strcmpi(&PR->argString[5][0], JUMPTYPE_PREVPAGE_TEXT)==0)
			PIR->ew->jumpType = JUMPTYPE_PREVPAGE;
		else if (strcmpi(&PR->argString[5][0], JUMPTYPE_NEXTPAGE_TEXT)==0)
			PIR->ew->jumpType = JUMPTYPE_NEXTPAGE;
		else
			PIR->ew->jumpType = 0;	// shouldn't happen...

		PIR->ew->renderType = *(args+5);

		PIR->ew->audioCue = *(args+6);

		stccpy(PIR->ew->buttonName, &PR->argString[8][0], 49);
		RemoveQuotes(PIR->ew->buttonName);

		if ( PR->numArgs==11 )	// assignment
		{
			stccpy(scrStr, &PR->argString[10][0], 74);
			ScriptToStr(scrStr, PIR->ew->assignment);
			RemoveQuotes(PIR->ew->assignment);
		}

		if ( PR->numArgs==9 )	// no key code (old style)
		{
			PIR->ew->keyCode		= -1;
			PIR->ew->rawkeyCode	= -1;
		}
		else	// 10 or 11 args
		{			
			if ( PR->argString[9][0]=='\"' && PR->argString[9][1]=='\"' )	// no key code (new style)
			{
				PIR->ew->keyCode		= -1;
				PIR->ew->rawkeyCode	= -1;
			}
			else if(PR->argString[9][0]=='\"')	// keycode
			{
				PIR->ew->keyCode		= (int)PR->argString[9][1];
				PIR->ew->rawkeyCode	= -1;
			}
			else	// raw key code
			{
				PIR->ew->keyCode = -1;
				if (strcmpi(&PR->argString[9][0], TALK_HELP_KT)==0)
					PIR->ew->rawkeyCode = TALK_HELP_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_ESC_KT)==0)
					PIR->ew->rawkeyCode = TALK_ESC_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_F1_KT)==0)
					PIR->ew->rawkeyCode = TALK_F1_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_F2_KT)==0)
					PIR->ew->rawkeyCode = TALK_F2_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_F3_KT)==0)
					PIR->ew->rawkeyCode = TALK_F3_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_F4_KT)==0)
					PIR->ew->rawkeyCode = TALK_F4_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_F5_KT)==0)
					PIR->ew->rawkeyCode = TALK_F5_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_F6_KT)==0)
					PIR->ew->rawkeyCode = TALK_F6_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_F7_KT)==0)
					PIR->ew->rawkeyCode = TALK_F7_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_F8_KT)==0)
					PIR->ew->rawkeyCode = TALK_F8_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_F9_KT)==0)
					PIR->ew->rawkeyCode = TALK_F9_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_F10_KT)==0)
					PIR->ew->rawkeyCode = TALK_F10_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_CURSORUP_KT)==0)
					PIR->ew->rawkeyCode = TALK_CURSORUP_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_CURSORDOWN_KT)==0)
					PIR->ew->rawkeyCode = TALK_CURSORDOWN_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_CURSORLEFT_KT)==0)
					PIR->ew->rawkeyCode = TALK_CURSORLEFT_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_CURSORRIGHT_KT)==0)
					PIR->ew->rawkeyCode = TALK_CURSORRIGHT_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_TAB_KT)==0)
					PIR->ew->rawkeyCode = TALK_TAB_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_DEL_KT)==0)
					PIR->ew->rawkeyCode = TALK_DEL_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_BACKSPACE_KT)==0)
					PIR->ew->rawkeyCode = TALK_BACKSPACE_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_RETURN_KT)==0)
					PIR->ew->rawkeyCode = TALK_RETURN_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_SPACE_KT)==0)
					PIR->ew->rawkeyCode = TALK_SPACE_KC;

				else if (strcmpi(&PR->argString[9][0], TALK_OPEN_BRACKET_KT)==0)
					PIR->ew->rawkeyCode = TALK_OPEN_BRACKET_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_CLOSE_BRACKET_KT)==0)
					PIR->ew->rawkeyCode = TALK_CLOSE_BRACKET_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_STAR_KT)==0)
					PIR->ew->rawkeyCode = TALK_STAR_KC;
				else if (strcmpi(&PR->argString[9][0], TALK_PLUS_KT)==0)
					PIR->ew->rawkeyCode = TALK_PLUS_KC;

				else
					PIR->ew->rawkeyCode = TALK_ESC_KC;
			}
		}
	}
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
}

/******** ObjectStartFunc() ********/

void ObjectStartFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR)
{
}

/******** ObjectEndFunc() ********/

void ObjectEndFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR)
{
}

/******** BackgroundFunc() ********/

void BackgroundFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR)
{
#ifndef USED_FOR_PLAYER
WORD args[MAX_PARSER_ARGS];

/* BACKGROUND x,y,w,h,tbc,rbc,bbc,lbc,bw,ic,it,flags[,pn] - 13/14												*/
/* BACKGROUND x,y,w,h,tbc,rbc,bbc,lbc,bw,ic,it,flags,tm,rm,bm,lm[,pn] - 17/18						*/
/* BACKGROUND x,y,w,h,tbc,rbc,bbc,lbc,bw,ic,it,flags,tm,rm,bm,lm,pn,sdep,sdir,spen - 21	*/

	if (	PR->numArgs==13 || PR->numArgs==14 || PR->numArgs==17 || PR->numArgs==18 ||
				PR->numArgs==21 )
	{
		GetNumericalArgs(PR, args);

		PIR->ew->X 			= *(args+0);
		PIR->ew->Y			= *(args+1);
		PIR->ew->Width	= *(args+2);
		PIR->ew->Height	= *(args+3);

		if ( *(args+4) != -1 ) 	// top border color
		{
			PIR->ew->BorderColor[0]	= *(args+4);
			PIR->ew->Border	|= BORDER_TOP;
		}

		if ( *(args+5) != -1 )	// right border color
		{
			PIR->ew->BorderColor[1]	= *(args+5);
			PIR->ew->Border	|= BORDER_RIGHT;
		}

		if ( *(args+6) != -1 )	// bottom border color
		{
			PIR->ew->BorderColor[2]	= *(args+6);
			PIR->ew->Border	|= BORDER_BOTTOM;
		}

		if ( *(args+7) != -1 )	// left border color
		{
			PIR->ew->BorderColor[3]	= *(args+7);
			PIR->ew->Border	|= BORDER_LEFT;
		}

		PIR->ew->BorderWidth		 	= *(args+8);
		PIR->ew->BackFillColor		= *(args+9);
		PIR->ew->BackFillType			= *(args+10)-1;

		PIR->ew->flags						= *(args+11);

		if (PR->numArgs>=17) // be backwards compatible
		{
			PIR->ew->TopMargin				= *(args+12);
			PIR->ew->RightMargin			= *(args+13);
			PIR->ew->BottomMargin			= *(args+14);
			PIR->ew->LeftMargin				= *(args+15);
		}

		if (PR->numArgs==14)
			PIR->ew->patternNum				= *(args+12);
		else if (PR->numArgs==18 || PR->numArgs==20)
			PIR->ew->patternNum				= *(args+16);
		else
			PIR->ew->patternNum				= DEFAULT_PATTERN;

		if ( PR->numArgs==21 )
		{
			PIR->ew->wdw_shadowDepth	= *(args+17);
			PIR->ew->wdw_shadowDirection = *(args+18);
			PIR->ew->wdw_shadowPen		= *(args+19);
		}
	}
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** ClipAnimFunc() ********/

void ClipAnimFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR)
{
#ifndef USED_FOR_PLAYER
WORD args[MAX_PARSER_ARGS];
TEXT path[256], fileName[100], scrStr[256];
BOOL retval;

	/*  CLIPANIM "path", x, y, w, h, fps, loops, disk  */

	if ( PIR->ew==NULL )
	{
		printError(PR, msgs[Msg_MissesObjectStart-1]);
		return;
	}

	if (PR->numArgs==9)
	{
		GetNumericalArgs(PR, args);

		PIR->ew->PhotoOffsetX	= *(args+1);
		PIR->ew->PhotoOffsetY	= *(args+2);

		ScriptToStr(&PR->argString[1][0], scrStr);
		UA_SplitFullPath(scrStr, path, fileName);
		stccpy(PIR->es->picPath, scrStr, SIZE_FULLPATH);

		PIR->es->photoOpts = MOVE_PHOTO;

		PIR->ew->animIsAnim = TRUE;
		PIR->ew->animSpeed = *(args+5);
		PIR->ew->animLoops = *(args+6);
		PIR->ew->animFromDisk = *(args+7);

		retval = doActualILBMLoading(PIR->es->picPath, fileName, PIR->es, PIR->ew, FALSE);
		if (!retval)
		{
			PIR->es->photoOpts = 0;
			PIR->es->picPath[0] = '\0';
			PIR->ew->PhotoOffsetX	= 0;
			PIR->ew->PhotoOffsetY	= 0;
		}
	}
	else
		printError(PR, msgs[Msg_InvalidNumberOfArgs-1]);
#endif
}

/******** E O F ********/
