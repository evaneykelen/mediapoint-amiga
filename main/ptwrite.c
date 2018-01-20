#include "nb:pre.h"

#define SAVE_DISABLED FALSE

/**** DEFINES ****/

#define START_TXT	"OBJECTSTART\n"
#define END_TXT		"OBJECTEND\n"
#define PT_VER		1
#define PT_REV		0

/**** EXTERNALS ****/

extern struct EditWindow **EditWindowList;
extern struct EditSupport	**EditSupportList;
extern struct CapsPrefs CPrefs;
extern struct Window *pageWindow;
extern struct Screen *pageScreen;
extern struct List *fontList;
extern struct TextFont *largeFont;
extern UBYTE **msgs;   
extern struct Library *medialinkLibBase;
extern struct EditWindow backEW;
extern struct EditSupport backES;
extern BOOL do_the_tile;
extern BOOL save_as_iff;
extern struct BitMap sharedBM;
extern struct RastPort sharedRP;

/**** FUNCTIONS ****/

/******** WritePage() ********/

BOOL WritePage(STRPTR path, STRPTR fileName, char **pageCommands)
{
int i,j,p1,p2,p3,p4;
FILE *fp;
long drawArray[MAXEDITWINDOWS];
int maxSort, fontsize, flags;
struct EditWindow *ew;
struct EditSupport *es;
ULONG bitValue;
TEXT fullPath[SIZE_FULLPATH], fontname[50];

	if ( !CheckWriteProtect(path,fileName) )
	{
		Message(msgs[Msg_Overwriting-1],fileName);
		return(FALSE);
	}

#if SAVE_DISABLED

	Message("Sorry, no saving in this version...");
	return(TRUE);

#else

	/**** open the pagetalk file ****/

	UA_MakeFullPath(path, fileName, fullPath);
	UpdateDirCache(path);

	if ( save_as_iff )	// IFF
	{
		return( SavePageAsScreen(fullPath) );
	}
	else		// PAGETALK
	{
		/**** create the order in which the windows must be written ****/

		for(i=0; i<MAXEDITWINDOWS; i++)
			drawArray[ i ] = -1;

		maxSort=0;
		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if (EditWindowList[i] != NULL)
			{
				drawArray[maxSort] = (long)EditWindowList[i]->DrawSeqNum;
				maxSort++;
			}
		}

		if (maxSort>1)
			lqsort(drawArray, maxSort);

		fp = (FILE *)fopen(fullPath, "w");
		if (fp == NULL)
		{
			Message(msgs[Msg_UnableToSaveDoc-1], fileName);
			return(FALSE);
		}

		/**** TALK_PAGETALK ****/

		fprintf(fp, "PAGETALK %d,%d\n\n", PT_VER, PT_REV);

		/**** TALK_OBJECTSTART ****/
	
		fprintf(fp, START_TXT);

		/**** TALK_SCREEN ****/

		p1 = 0;
		if ( CPrefs.PageScreenModes & HAM_KEY )
			p1 = 1;
		else if ( CPrefs.PageScreenModes & EXTRAHALFBRITE_KEY )
			p1 = 2;
		GuessOldStyleMode(&p2);
		doIndent(fp, 0);
		fprintf(fp, "%s %d,%d,%d,%d,%d,%d\n",
						pageCommands[TALK_SCREEN],
						CPrefs.PageScreenWidth, CPrefs.PageScreenHeight, CPrefs.PageScreenDepth,
						p1, CPrefs.overScan, p2);

		/**** TALK_PALETTE ****/

		p4 = UA_GetNumberOfColorsInScreen(CPrefs.PageScreenModes, CPrefs.PageScreenDepth, CPrefs.AA_available);

		if (p4 < 8)
		{
			doIndent(fp, 0);
			fprintf(fp, "%s 1, \"", pageCommands[TALK_PALETTE]);
			for(i=0; i<p4; i++)
				fprintf(fp, "%06x ", GetColorCM32(CPrefs.PageCM, i) );
			fprintf(fp,"\"\n");
		}
		else
		{
			for(j=0; j<(p4/8); j++)		// prints 'p4/8' lines
			{
				doIndent(fp, 0);
				fprintf(fp, "%s %d, \"", pageCommands[TALK_PALETTE], j+1);
				for(i=0; i<8; i++)
					fprintf(fp, "%06x ", GetColorCM32(CPrefs.PageCM, j*8+i) );
				fprintf(fp,"\"\n");
			}
		}

		/**** TALK_BACKGROUND ****/

		if ( backES.ori_bm.Planes[0] )
		{
			ew = &backEW;
			es = &backES;

			flags = 0;
			if ( do_the_tile )
				flags |= EW_IS_TILED;
		
			doIndent(fp, 0);
			fprintf(fp, "%s %d,%d,%d,%d, %d,%d,%d,%d, %d,%d,%d, %d, %d,%d,%d,%d, %d,%d,%d,%d\n",
									pageCommands[TALK_BACKGROUND],
									ew->X, ew->Y, ew->Width, ew->Height,
									-1,-1,-1,-1,	/* border colors */
								  0,0,3,				/* borderwidth, interiorcolor, interiortype */
									flags,				/* flags */
									0,0,0,0,			/* margins */
									0,						/* pattern */
									0,0,0					/* shadowDepth, shadowDirection, shadowPen */
							);
	
			WritePageClip(fp, ew, es, path, fileName, pageCommands);
		}

		/**** TALK_WINDOW ****/

		for(i=0; i<maxSort; i++)
		{
			for(j=0; j<MAXEDITWINDOWS; j++)
			{
				if (	(EditWindowList[j]!=NULL &&
							EditWindowList[j]->DrawSeqNum==drawArray[i]) )
				{
					/**** RENDER WINDOW ****/

					ew = (struct EditWindow *)EditWindowList[j];
					es = (struct EditSupport *)EditSupportList[j];

					/**** SAVE WINDOW ****/

					doIndent(fp, 0);

					p1=-1;
					p2=-1;
					p3=-1;
					p4=-1;

					bitValue = (ULONG)ew->Border;
	
					if (TestBit(bitValue, BORDER_TOP))
						p1 = ew->BorderColor[0];

					if (TestBit(bitValue, BORDER_RIGHT))
						p2 = ew->BorderColor[1];
	
					if (TestBit(bitValue, BORDER_BOTTOM))
						p3 = ew->BorderColor[2];

					if (TestBit(bitValue, BORDER_LEFT))
						p4 = ew->BorderColor[3];

					/**** WINDOW ****/

					fprintf(fp, "%s %d,%d,%d,%d, %d,%d,%d,%d, %d,%d,%d, %d, %d,%d,%d,%d, %d, %d,%d,%d\n",
									pageCommands[TALK_WINDOW],
									ew->X, ew->Y, ew->Width, ew->Height,
									p1, p2, p3, p4,
									ew->BorderWidth, ew->BackFillColor, ew->BackFillType+1,
									ew->flags,
									ew->TopMargin, ew->RightMargin, ew->BottomMargin, ew->LeftMargin,
									ew->patternNum,
									ew->wdw_shadowDepth, ew->wdw_shadowDirection, ew->wdw_shadowPen);

					/**** EFFECT (WINDOW) ****/

					if ( ew->in1[0] != -1 || ew->out1[0] != -1 )
					{
						doIndent(fp, 0);
						fprintf(fp, "%s %d,%d,%d,%d,%d,%d,%d,%d,%d\n",
										pageCommands[TALK_EFFECT], 0,
										ew->in1[0], ew->in2[0], ew->in3[0],
										ew->out1[0], ew->out2[0], ew->out3[0],
										ew->inDelay[0], ew->outDelay[0] );
					}

					/*** CLIP ****/

					if ( es->ori_bm.Planes[0] )
						WritePageClip(fp, ew, es, path, fileName, pageCommands);

					/**** EFFECT (CLIP) ****/

					if ( ew->in1[1] != -1 || ew->out1[1] != -1 )
					{
						doIndent(fp, 0);
						fprintf(fp, "%s %d,%d,%d,%d,%d,%d,%d,%d,%d\n",
										pageCommands[TALK_EFFECT], 1,
										ew->in1[1], ew->in2[1], ew->in3[1],
										ew->out1[1], ew->out2[1], ew->out3[1],
										ew->inDelay[1], ew->outDelay[1] );
					}

					/**** FORMAT ****/

#if 0	
					if (	ew->antiAliasLevel		!= 0 ||
								ew->justification			!= 0 ||
								ew->xSpacing					!= 0 ||
								ew->ySpacing					!= 0 ||
								ew->slantAmount				!= 2 ||
								ew->slantValue				!= 1 ||
								ew->underLineHeight		!= 1 ||
								ew->underLineOffset		!= 0 ||
								ew->shadowDepth				!= 0 ||
								ew->shadow_Pen				!= 0 ||
								ew->shadowType				!= 0 ||
								ew->shadowDirection		!= 0
							)
#endif
					{
						doIndent(fp, 0);
						fprintf(fp, "%s %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
										pageCommands[TALK_FORMAT],
										ew->antiAliasLevel,
										ew->justification,
										ew->xSpacing,
										ew->ySpacing,
										ew->slantAmount,
										ew->slantValue,
										ew->underLineHeight,
										ew->underLineOffset,
										ew->shadowDepth,
										ew->shadow_Pen,
										ew->shadowType,
										ew->shadowDirection );
					}

					/**** STYLE ****/
#if 0
					if (	ew->charStyle	!= 0 ||
								ew->charColor	!= 2 ||
								ew->charFont	!= largeFont ||
								ew->underlineColor != 2 )
#endif
					{
						if (!findFont(ew->charFont, fontname, &fontsize))
						{
							strcpy(fontname, SHORTAPPFONT);
							fontsize=20;
						}
						doIndent(fp, 0);
						fprintf(fp, "%s %s,%d,%d,%d,%d\n",
										pageCommands[TALK_STYLE],
										fontname,
										fontsize,
										ew->charStyle,
										ew->charColor,
										ew->underlineColor);
					}

					/**** TEXT ****/

					if ( ew->TEI && ew->TEI->textLength > 0)
						WriteTextLines(fp, ew, es, pageCommands);
	
					/**** EFFECT (TEXT) ****/

					if ( ew->in1[2] != -1 || ew->out1[2] != -1 )
					{
						doIndent(fp, 0);
						fprintf(fp, "%s %d,%d,%d,%d,%d,%d,%d,%d,%d\n",
										pageCommands[TALK_EFFECT], 2,
										ew->in1[2], ew->in2[2], ew->in3[2],
										ew->out1[2], ew->out2[2], ew->out3[2],
										ew->inDelay[2], ew->outDelay[2] );
					}

					/**** CRAWL ****/

					WriteCrawlLines(fp, ew, es, pageCommands);	

					/**** BUTTON ****/

					WriteButtonInfo(fp, ew, es, pageCommands);	
				}
			}
		}

		/**** TALK_OBJECTEND ****/

		fprintf(fp, END_TXT);

		fclose(fp);

		if ( !save_as_iff )	// PAGETALK
		{
			SaveIcon(fullPath);
		
			UA_MakeFullPath(path, "Clips", fullPath);
			DeleteOldClips(fullPath, fileName);
		}
	}

	return(TRUE);
#endif
}

/******** doIndent() ********/

void doIndent(FILE *fp, int level)
{
register int i;

	fprintf(fp, "	");	/* TAB */
	for(i=0; i<level; i++)
		fprintf(fp, "	");	/* TAB */
}

/******** WriteTextLines() ********/

void WriteTextLines(FILE *fp, struct EditWindow *ew, struct EditSupport *es,
										char **pageCommands)
{
int i,j,fontsize;
struct TextFont *charFont;
UBYTE	charStyle;
int charColor;
int underlineColor;
TEXT fontname[50];
BOOL quitEOL=FALSE, wroteLine=FALSE;

	charFont = NULL;
	charStyle = 0;
	charColor = -1;
	underlineColor = -1;

	i=0;
	j=0;
	for(i=0; i<ew->TEI->textLength; i++)
	{
		if ( ew->TEI->lineStarts[j]==ew->TEI->lineStarts[j+1] )
			quitEOL=TRUE;

		if ( i==ew->TEI->lineStarts[j] )
		{
			if (!quitEOL)
			{
				j++;
				if(i>0)
					fprintf(fp, "\"\n");
				doIndent(fp, 0);
				fprintf(fp, "%s \"", pageCommands[TALK_TEXT]);
				wroteLine = TRUE;
			}
			else
				break;
		}

		if ( ew->TEI->text[ i ].charCode != '\0' )
		{
			if ( charFont	!= ew->TEI->text[i].charFont )
			{
				charFont = ew->TEI->text[i].charFont;
				if (findFont(charFont, fontname, &fontsize))
					fprintf(fp, "^f%s^^s%d^", fontname, fontsize);
			}

			if ( charColor !=	ew->TEI->text[i].charColor )
			{
				charColor = ew->TEI->text[i].charColor;
				fprintf(fp, "^c%d^", charColor);
			}

			if ( underlineColor != ew->TEI->text[i].underlineColor )
			{
				underlineColor = ew->TEI->text[i].underlineColor;
				fprintf(fp, "^u%d^", underlineColor);
			}

			if ( charStyle !=	ew->TEI->text[i].charStyle )
			{
				charStyle = ew->TEI->text[i].charStyle;
				fprintf(fp, "^a%d^", charStyle);
			}

			if ( ew->TEI->text[i].charCode == 0x0a )			// CR
				fprintf(fp, "^lf^");
			else if ( ew->TEI->text[i].charCode == '\"' )	// double quote
				fprintf(fp, "\\\"");
			else
				fprintf(fp, "%c", ew->TEI->text[i].charCode);
		}
		else
			break;
	}

	if ( wroteLine )
		fprintf(fp, "\"\n");
}

/******** findFont() ********/

BOOL findFont(struct TextFont *font, STRPTR fontname, int *fontsize)
{
struct FontListRecord *FLR;

	for(FLR=(struct FontListRecord *)fontList->lh_Head;
			FLR->node.ln_Succ;
			FLR=(struct FontListRecord *)FLR->node.ln_Succ)
	{
		if ( FLR->ptr == font )
		{
			stccpy(fontname, FLR->fontName, 50);
			*(fontname+strlen(FLR->fontName)-5) = 0;
			*fontsize = FLR->fontSize;
			return(TRUE);
		}
	}
	return(FALSE);
}

/******** WriteCrawlLines() ********/

void WriteCrawlLines(	FILE *fp, struct EditWindow *ew, struct EditSupport *es,
											char **pageCommands)
{
int i,j,k;
TEXT oneLine[100], scrStr[100];
BOOL loop=TRUE;

	if ( ew->crawl_text==NULL || ew->crawl_length==0 )
		return;	// nothing to save

	doIndent(fp, 0);

	// CRAWL "fontname", size, speed, pen_color

	if ( ew->crawl_flags & 1 )	// from file
	{
		StrToScript(ew->crawl_text, scrStr);
		fprintf(	fp, "%s \"%s\", %d, %d, %d, \"%s\"\n", pageCommands[TALK_CRAWL],
							ew->crawl_fontName, ew->crawl_fontSize, ew->crawl_speed, ew->crawl_color,
							scrStr);
	}
	else
		fprintf(	fp, "%s \"%s\", %d, %d, %d\n", pageCommands[TALK_CRAWL],
							ew->crawl_fontName, ew->crawl_fontSize, ew->crawl_speed, ew->crawl_color);

	if ( !(ew->crawl_flags & 1) )	// *NOT* from file
	{
		// CRAWL "The quick "
		// CRAWL "brown fox."

		i=0;
		while( loop )
		{
			j=0;
			while ( 1 )
			{
				oneLine[j+0] = *(ew->crawl_text+i);		
				oneLine[j+1] = '\0';
				i++;

				if ( *(ew->crawl_text+i) == '\0' )
					loop=FALSE;

				j++;
				if (j==70)
					break;
			}	

			StrToScript(oneLine, scrStr);

			for(k=0; k<100; k++)
				if ( oneLine[k]<0x20 )
					oneLine[k] = 0x20;

			doIndent(fp, 0);
			fprintf(fp, "%s \"%s\"\n", pageCommands[TALK_CRAWL], scrStr);
		}

		doIndent(fp, 0);
		fprintf(fp, "%s \" \"\n", pageCommands[TALK_CRAWL]);
	}
}

/******** DeleteOldClips() ********/

void DeleteOldClips(STRPTR fullPath, STRPTR fileName)
{
int i,j;
TEXT clipPath[SIZE_FULLPATH], clipFileName[SIZE_FULLPATH];
BOOL found;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		//if ( EditWindowList[i] )
		{
			sprintf(clipFileName, "%s-%d", fileName, i+1);
			UA_MakeFullPath(fullPath, clipFileName, clipPath);

			found=FALSE;
			for(j=0; j<MAXEDITWINDOWS; j++)
			{
				if (	EditSupportList[j] &&
							!strcmpi( EditSupportList[j]->picPath, clipPath ) )
				{
					found=TRUE;
					break;
				}
			}

			if ( !found )
			{
				DeleteFile( clipPath );
			}
		}
	}
}

/******** WriteButtonInfo() ********/

void WriteButtonInfo(	FILE *fp, struct EditWindow *ew, struct EditSupport *es,
											char **pageCommands)
{
TEXT tmp[50], tmp2[20], scrStr[100];
WORD x,y,w,h;

	if ( ew->jumpType==0 )
		return;	// nothing to save

	doIndent(fp, 0);

	// BUTTON x,y,w,h, jumpType, renderType, audioCue, "buttonName", "key"
	// BUTTON x,y,w,h, jumpType, renderType, audioCue, "buttonName", "rawkey"
	// BUTTON x,y,w,h, jumpType, renderType, audioCue, "buttonName",

	// NEW FOR 128...

	// BUTTON x,y,w,h, jumpType, renderType, audioCue, "buttonName", "key", "assignment"
	// BUTTON x,y,w,h, jumpType, renderType, audioCue, "buttonName", "rawkey", "assignment"
	// BUTTON x,y,w,h, jumpType, renderType, audioCue, "buttonName", "", "assignment"

	if ( ew->jumpType==JUMPTYPE_GOTO )
		strcpy(tmp2, JUMPTYPE_GOTO_TEXT);
	else if ( ew->jumpType==JUMPTYPE_GOSUB )
		strcpy(tmp2, JUMPTYPE_GOSUB_TEXT);
	else if ( ew->jumpType==JUMPTYPE_PREV )
		strcpy(tmp2, JUMPTYPE_PREV_TEXT);
	else if ( ew->jumpType==JUMPTYPE_NEXT )
		strcpy(tmp2, JUMPTYPE_NEXT_TEXT);
	else if ( ew->jumpType==JUMPTYPE_PREVPAGE )
		strcpy(tmp2, JUMPTYPE_PREVPAGE_TEXT);
	else if ( ew->jumpType==JUMPTYPE_NEXTPAGE )
		strcpy(tmp2, JUMPTYPE_NEXTPAGE_TEXT);

	x = ew->X;			// ew->bx
	y = ew->Y;			// ew->by
	w = ew->Width;	// ew->bwidth
	h = ew->Height;	// ew->bheight

	GetWindowVarsShadow(ew, &x, &y, &w, &h);

	if (ew->keyCode != -1)
		sprintf(tmp, "\"%c\"", ew->keyCode);
	else if (ew->rawkeyCode != -1)
		KeyToKeyName(ew->keyCode, ew->rawkeyCode, tmp);
	else
		strcpy(tmp, "\"\"");

	StrToScript(ew->assignment, scrStr);

	fprintf(fp, "%s %d, %d, %d, %d, %s, %d, %d, \"%s\", %s, \"%s\"\n",
					pageCommands[TALK_BUTTON],
					x,y,w,h,
					tmp2,
					ew->renderType,
					ew->audioCue,
					ew->buttonName,
					tmp,
					scrStr);
}

/******** SavePageAsScreen() ********/

BOOL SavePageAsScreen(STRPTR path)
{
struct IFF_FRAME iff;
struct ViewPort *vp;
//struct ColorMap *cm;
struct BitMap bm;
UWORD w,h,d,i;
struct Window *panel;

	FastInitIFFFrame(&iff);

	vp = &pageScreen->ViewPort;
	//cm = vp->ColorMap;
	d = CPrefs.PageScreenDepth;
	w = pageWindow->Width;
	h = pageWindow->Height;

	// create a fake bitmap - use sharedBM planes -- save 'cause fakebm <= than sharedBM

	InitBitMap(&bm,d,w,h);
	for(i=0; i<d; i++)
		bm.Planes[i] = sharedBM.Planes[i];

	BltBitMap(pageScreen->RastPort.BitMap,
						pageWindow->LeftEdge, pageWindow->TopEdge,
						&bm,
						0,0,w,h,
						0xc0,0xff,NULL);
	WaitBlit();

	panel = UA_OpenMessagePanel(pageWindow,msgs[Msg_SavingAsIFF-1]);
	if (panel)
	{
		ActivateWindow(panel);
		SetSpriteOfActWdw(SPRITE_BUSY);
	}

	iff.BMH.w									= w;
	iff.BMH.h									= h;
	iff.BMH.x									= 0;
	iff.BMH.y									= 0;
	iff.BMH.nPlanes						= d;
	iff.BMH.masking						= 0;
	iff.BMH.compression				= 1;
	iff.BMH.pad1							= 0;
	iff.BMH.transparentColor	= 0;
	iff.BMH.xAspect						= 0;
	iff.BMH.yAspect						= 0;
	iff.BMH.pageWidth					= w;
	iff.BMH.pageHeight				= h;
	iff.colorMap							= CPrefs.PageCM;	//cm;	// screens colorMap -- DON'T FREE IT!!!!
	iff.viewModes							= vp->Modes & 0x0000ffff;

	FastWriteIFF(&iff, path, (struct BitMap24 *)&bm, NULL);

	iff.colorMap = NULL;	// else FastCloseIFF frees cm !!!!!!!

	FastCloseIFF(&iff);

	if ( panel )
		UA_CloseMessagePanel(panel);

	if ( iff.Error != IFF_ERROR_OK )
		return(FALSE);

	return(TRUE);
}

/******** GuessOldStyleMode() ********/

void GuessOldStyleMode(int *mode)
{
	*mode = 0;

	if ( CPrefs.PageScreenWidth >= 1280 )
		*mode	= 5;
	else if ( CPrefs.PageScreenWidth >= 640 )
		*mode	= 3;
	else if ( CPrefs.PageScreenWidth >= 320 )
		*mode	= 1;

	if ( (CPrefs.PageScreenModes & LACE) || (CPrefs.PageScreenHeight>=400) )
		*mode = *mode + 1;
}

/******** CheckWriteProtect() ********/

BOOL CheckWriteProtect(STRPTR path, STRPTR fileName)
{
TEXT fpath[SIZE_FULLPATH];
struct FileInfoBlock __aligned fib;
BPTR lock;
BOOL retval=TRUE;

	UA_MakeFullPath(path,fileName,fpath);

	if ( lock=Lock(fpath,SHARED_LOCK) )
	{
		if ( Examine(lock,&fib) )
		{
			if ( fib.fib_Protection & FIBF_DELETE )
				retval=FALSE;
		}
		UnLock(lock);
	}
	return(retval);
}

/******** E O F ********/
