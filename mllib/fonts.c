#include "mllib_includes.h"

STATIC void unclipWindow(struct Window *win);
STATIC struct Region *clipWindow(	struct Window *win, LONG minX, LONG minY,
																	LONG maxX, LONG maxY);

extern struct MsgPort *capsport;

/**** static globals ****/

static struct FontEntry **FontEntryList;
static ULONG FEL_size;
static struct FontEntry **temp_FontEntryList;
static ULONG temp_FEL_size;
static BOOL laced;

static struct PropInfo SliderInfo1 =
{ AUTOKNOB | FREEVERT | PROPBORDERLESS, 0,0,0,0, };

static struct PropInfo SliderInfo2 =
{ AUTOKNOB | FREEVERT | PROPBORDERLESS, 0,0,0,0, };

static struct Image Image1 = { 0,0,0,0,0,NULL,0x0000,0x0000,NULL };
static struct Image Image2 = { 0,0,0,0,0,NULL,0x0000,0x0000,NULL };

static struct Gadget FontSlider1 =
{	NULL, 0,0,0,0, NULL, GACT_RELVERIFY | GACT_IMMEDIATE, GTYP_PROPGADGET,
	&Image1, NULL, NULL, NULL, (struct PropInfo *)&SliderInfo1, 1,NULL };

static struct Gadget FontSlider2 =
{	NULL, 0,0,0,0, NULL, GACT_RELVERIFY | GACT_IMMEDIATE, GTYP_PROPGADGET,
	&Image2, NULL, NULL, NULL, (struct PropInfo *)&SliderInfo2, 2,NULL };

/**** functions ****/

/*******************************************************************/
/*
 *   PUBLIC FUNCTIONS
 *
 *******************************************************************/

/******** ScanFontsDir() ********/

BOOL __saveds __asm LIBUA_ScanFontsDir(register __a0 struct FER *FER)
{
int afShortage, i, j, fontCount, sizeCount, afSize, temp_fontCount;
struct AvailFontsHeader *afh;
struct AvailFonts *aFonts;
TEXT lastName[60];
struct FontEntry *FE, *FE2;
BOOL topazFound;
char **ptrArr, ch;
TEXT tmp[60];

	/**** scan all font entries in fonts: ****/

	afSize = 400;
	do
	{
		afh = (struct AvailFontsHeader *)AllocMem(afSize, MEMF_CLEAR);
		if (afh != NULL)
		{
			afShortage = AvailFonts((STRPTR)afh, afSize, AFF_MEMORY | AFF_DISK);
			if (afShortage)
			{
				FreeMem(afh, afSize);
				afSize += afShortage;
			}
		}
		else
		{
			//WarnUser(198);
			return(FALSE);
		}
	} while (afShortage);

	/**** skip over the header to the first of the AvailFonts structures ****/

	aFonts = (struct AvailFonts *)&afh[1];

	/**** allocate memory to hold all font names ****/

	FEL_size = sizeof(struct FontEntry)*afh->afh_NumEntries;

	FontEntryList = (struct FontEntry **)AllocMem(FEL_size, MEMF_PUBLIC | MEMF_CLEAR);
	if (FontEntryList==NULL)
	{
		//WarnUser(199);
		return(FALSE);
	}

	/**** store all font names and their associated sizes ****/

	lastName[0] = '\0';

	fontCount = 0;
	sizeCount = 0;

	for(j=afh->afh_NumEntries; j; j--, aFonts++)
	{
		if ( !(aFonts->af_Attr.ta_Flags & FPF_REMOVED) &&
         !(aFonts->af_Attr.ta_Flags & FPF_ROMFONT) &&
          (aFonts->af_Attr.ta_Flags & FPF_DISKFONT) )
		{
			if (strcmpi(lastName, aFonts->af_Attr.ta_Name) != 0) /* new font entry */
			{
				FontEntryList[fontCount] = (struct FontEntry *)AllocMem(sizeof(struct FontEntry), MEMF_PUBLIC | MEMF_CLEAR);
				if ( FontEntryList[fontCount]==NULL )
				{
					//WarnUser(200);
					return(FALSE);
				}

				FE = (struct FontEntry *)FontEntryList[fontCount];

				sizeCount = 0;

				ch = aFonts->af_Attr.ta_Name[0];
				aFonts->af_Attr.ta_Name[0] = toupper(ch);

				stccpy(FE->fontName, aFonts->af_Attr.ta_Name, 49);
				stccpy(lastName, FE->fontName, 50);

				if (aFonts->af_Attr.ta_Flags & FPF_DESIGNED)
					strcat(FE->fontName, ""); /* 0x8f --> bitmapped font */
				else
					strcat(FE->fontName, ""); /* 0x90 --> outlined font */

				FE->fontSize[sizeCount] = aFonts->af_Attr.ta_YSize;
				FE->fontSize[sizeCount+1] = 0;
				sizeCount++;

				fontCount++;
			}
			else
			{
				if ( fontCount>0 && sizeCount>0 )
				{
					FE->fontSize[sizeCount] = aFonts->af_Attr.ta_YSize;
					FE->fontSize[sizeCount+1] = 0;
					if (sizeCount<23)
						sizeCount++;
				}
			}
		}
	}

	FreeMem(afh, afSize);

	/**** search topaz, if found add the sizes 8 and 9 (ROM) ****/

	topazFound = FALSE;

	for(i=0; i<fontCount; i++)
	{
		FE = (struct FontEntry *)FontEntryList[i];
		if (FE!=NULL)
		{
			if ( (strcmpi("Topaz.font", FE->fontName) == 0) ||
					 (strcmpi("Topaz.font", FE->fontName) == 0) )	/* topaz found */
			{
				j=0;
				while( FE->fontSize[j] != 0 )
					j++;
				FE->fontSize[j] = 8;
				FE->fontSize[j+1] = 9;
				topazFound=TRUE;
			}
		}
	}

	if (!topazFound)
	{
		FontEntryList[fontCount] = (struct FontEntry *)AllocMem(sizeof(struct FontEntry), MEMF_PUBLIC | MEMF_CLEAR);
		if ( FontEntryList[fontCount]==NULL )
		{
			//WarnUser(201);
			return(FALSE);
		}
		FE = (struct FontEntry *)FontEntryList[fontCount];
		stccpy(FE->fontName, "Topaz.font", 50);	/* 0x8f --> bitmapped font */
		FE->fontSize[0] = 8;
		FE->fontSize[1] = 9;
		fontCount++;
	}

	/**** allocate memory for pointer list ****/

	ptrArr = (char **)AllocMem(sizeof(char *)*fontCount, MEMF_CLEAR | MEMF_PUBLIC);
	if (ptrArr==NULL)
	{
		//WarnUser(202);
		return(FALSE);
	}

	/**** set up an array of pointers to all font names ****/

	for(i=0; i<fontCount; i++)
	{
		FE = (struct FontEntry *)FontEntryList[i];
		if ( FE != NULL )
			ptrArr[i] = &FE->fontName[0];
	}

	/**** sort alphabetically all font names ****/

	tqsort(ptrArr, fontCount);

	/**** Build new list. Previous list was too large anyway, because	****/
	/**** afh_numentries counts e.g. topaz 3 times (8,9 and 11)				****/

	temp_FEL_size = sizeof(struct FontEntry)*fontCount;

	temp_FontEntryList = (struct FontEntry **)AllocMem(temp_FEL_size, MEMF_PUBLIC | MEMF_CLEAR);
	if (temp_FontEntryList==NULL)
	{
		//WarnUser(203);
		return(FALSE);
	}

	/**** copy sorted names to new list ****/

	lastName[0] = '\0';

	j=0;
	for(i=0; i<fontCount; i++)
	{
		if (ptrArr[i] != NULL && strcmpi(lastName, ptrArr[i]) != 0) /* new font entry */
		{
			temp_FontEntryList[j] = (struct FontEntry *)AllocMem(sizeof(struct FontEntry), MEMF_PUBLIC | MEMF_CLEAR);
			if ( temp_FontEntryList[j]==NULL )
			{
				//WarnUser(204);
				return(FALSE);
			}
			FE = (struct FontEntry *)temp_FontEntryList[j];
			stccpy(FE->fontName, ptrArr[i], 49);
			stccpy(lastName, FE->fontName, 50);
			j++;
		}
	}

	temp_fontCount = j;	/* take over new number of fonts */

	/**** search font name in old list, retrieve font size(s) and copy ****/
	/**** them to the new list ****/

	for(i=0; i<fontCount; i++)
	{
		FE = (struct FontEntry *)temp_FontEntryList[i];
		if (FE != NULL)
		{
			for(j=0; j<fontCount; j++)
			{
				FE2 = (struct FontEntry *)FontEntryList[j];
				if (strcmpi(FE->fontName, FE2->fontName) == 0)
				{
					CopyMem(&FE2->fontSize[0], &FE->fontSize[0], 24);
					/**** sort font sizes ****/
					sizeCount=0;
					while( FE->fontSize[sizeCount] != 0 )
						sizeCount++;
					if (sizeCount>1)
						sqsort(&FE->fontSize[0], sizeCount);				
				}
			}
		}
	}

	FreeMem(ptrArr, sizeof(char *)*fontCount);

	/**** free first (too large) list ****/

	i=0;
	while( FontEntryList[i] != NULL )
	{
		FreeMem( FontEntryList[i], sizeof(struct FontEntry) );
		i++;
	}
	FreeMem(FontEntryList, FEL_size);

	/**** do some more massaging on fontnames ****/

	for(j=0; j<temp_fontCount; j++)
	{
		FE = (struct FontEntry *)temp_FontEntryList[j];

		if (FE != NULL)
		{
			/**** font type indicator is at the end of the string ****/
			/**** put it at the start and remove the trailing one ****/
			ch = FE->fontName[ strlen(FE->fontName)-1 ];
			sprintf(tmp, "%c%s", ch, FE->fontName);
			stccpy(FE->fontName, tmp, strlen(tmp)-5 );
		}
	}

	/**** fill int FER ****/

	FER->FEList				= (struct FontEntry **)temp_FontEntryList;
	FER->numEntries1 	= temp_fontCount;
	FER->numEntries2 	= 0;
	FER->top1					= 0;
	FER->top2					= 0;
	FER->selected1		= -1;
	FER->selected2		= -1;
	FER->fontSize			= -1;

	return(TRUE);
}

/******** freeScannedFonts() ********/

void __saveds __asm LIBUA_freeScannedFonts(void)
{
int i;

	i=0;
	while( temp_FontEntryList[i] != NULL )
	{
		FreeMem( temp_FontEntryList[i], sizeof(struct FontEntry) );
		i++;
	}
	FreeMem(temp_FontEntryList, temp_FEL_size);
}

/******** Monitor_FontSelection() ********/

struct TextFont * __saveds __asm LIBUA_Monitor_FontSelection(
																			register __a0 struct Window *window,
																			register __a1 struct FER *FER,
																			register __a2 struct GadgetRecord *GR,
																			register __a3 UWORD *mypattern1)
{
BOOL loop=TRUE, retval=FALSE, dblClicked, notListed=TRUE;
int ID, val, i, sizeCount;
struct EventData CED;
struct TextFont *TheFont = NULL;
struct FontEntry *FE;

#if 0
	if ( LIBUA_IsWindowOnLacedScreen(window) )
		laced = TRUE;
	else
		laced = FALSE;
#endif

/*
	if ( FER->selected1 != -1 && ( FER->selected2 != -1 || FER->fontSize != -1 ) )	// font selected
		TheFont = fontSample(window, &GR[7], FER, TheFont);
*/

	/**** handle events ****/

	while(loop)
	{
		LIBUA_doStandardWait(window, &CED);

		if (CED.extraClass == DBLCLICKED)
			dblClicked=TRUE;
		else
			dblClicked=FALSE;

		if ( CED.Class == IDCMP_RAWKEY )
		{
			if (CED.Code==RAW_ESCAPE)	// cancel
			{
				LIBUA_HiliteButton(window, &GR[9]);
				loop=FALSE;
				retval=FALSE;
				break;
			}
			else if (CED.Code==RAW_RETURN) // OK
			{
do_ok:
				if ( !LIBUA_IsGadgetDisabled(&GR[8]) )
				{
					LIBUA_HiliteButton(window, &GR[8]);
					loop=FALSE;
					retval=TRUE;
				}
			}
		}
		else if ( CED.Class == IDCMP_MOUSEBUTTONS )
		{
			if ( CED.extraClass==IDCMP_GADGETDOWN || CED.extraClass==IDCMP_GADGETUP )
			{
				doFontSliders(window, &CED, GR, FER);
			}
			else if ( CED.Code==SELECTDOWN )
			{
				ID = LIBUA_CheckGadgetList(window, GR, &CED);
				switch(ID)
				{
					case 2:	// font name list
						SelectFontLine(window, 1, FER, mypattern1, GR, &CED);
						if ( FER->selected1 != -1 )	// NEW PER Saturday 24-Jul-93 15:02:38      
								fontSample(window, &GR[7], NULL, TheFont, FALSE);
						break;

					case 4:	// size list
						if ( FER->selected1 != -1 )
						{
							if ( SelectFontLine(window, 2, FER, mypattern1, GR, &CED) )
								fontSample(window, &GR[7], NULL, TheFont, FALSE);
							if ( dblClicked )
								goto do_ok;
						}
						break;

					case 6:	// size string
						LIBUA_ProcessStringGadget(window, GR, &GR[ID], &CED);
						LIBUA_SetValToStringGadgetVal(&GR[ID], &val);
						FER->fontSize = val;
						FER->selected2 = -1;
						DoFontScrolling(window, FER->numEntries2, 4,
														&(FER->top2), &GR[4],
														&FontSlider2, TRUE, 2, FER);
						fontSample(window, &GR[7], NULL, TheFont, FALSE);
						LIBUA_EnableButton(window, &GR[8]);	// OK
						break;

					case 7:	// Show
						//LIBUA_HiliteButton(window, &GR[ID]);
						TheFont = fontSample(window, &GR[7], FER, TheFont, TRUE);
						break;

					case 8:	// OK
						LIBUA_HiliteButton(window, &GR[ID]);
						loop=FALSE;
						retval=TRUE;
						break;

					case 9:	// Cancel
						LIBUA_HiliteButton(window, &GR[ID]);
						loop=FALSE;
						retval=FALSE;
						break;
				}
			}
		}
	}

	if ( TheFont )
	{
		if ( !retval )
		{
			CloseFont(TheFont);
			TheFont = NULL;
		}
		else
			TheFont = fontSample(window, &GR[7], FER, TheFont, FALSE);
	}
	else if ( retval )
	{
		TheFont = fontSample(window, &GR[7], FER, TheFont, FALSE);
	}

	/**** add user entered size to font size list ****/

	if ( FER->selected1 != -1 && FER->fontSize != -1 )
	{
		FE = (struct FontEntry *)FER->FEList[ FER->selected1 ];

		for(i=0; i<30; i++)
		{
			if ( FE->fontSize[i] == FER->fontSize )
				notListed=FALSE;
		}

		if ( notListed )
		{
			for(i=0; i<30; i++)
			{
				if ( FE->fontSize[i] == 0 )
				{
					FE->fontSize[i] = FER->fontSize;
					i=30;
				}
			}

			/**** sort font sizes ****/
			sizeCount=0;
			while( FE->fontSize[sizeCount] != 0 )
				sizeCount++;
			if (sizeCount>1)
				sqsort(&FE->fontSize[0], sizeCount);				

			// make new entry also the selected one

			for(i=0; i<30; i++)
			{
				if ( FE->fontSize[i] == FER->fontSize )
				{
					FER->selected2 = i;
					i=30;
				}
			}

			// adjust top

			FER->top2 = FER->selected2;
			if ( (FER->top2+4) > sizeCount )
				FER->top2 = sizeCount-4;
			if ( FER->top2 < 0 )
				FER->top2 = 0;			
		}
	}

#if 0
	if ( !retval && TheFont!=NULL )
	{
		CloseFont(TheFont);
		TheFont = NULL;
	}
	else
		TheFont = fontSample(window, &GR[7], FER, TheFont, FALSE);
#endif

	LIBUA_EnableButton(window, &GR[6]);	// size may have been disabled
	LIBUA_EnableButton(window, &GR[8]);	// OK may have been disabled

	return(TheFont);
}

/******** OpenFontListWindow() ********/

struct Window * __saveds __asm LIBUA_OpenFontListWindow(register __a0 struct Window *onWindow,
																												register __a1 struct FER *FER,
																												register __a2 struct GadgetRecord *GR,
																												register __a3 UWORD *mypattern1)
{
int i;
struct FontEntry *FE;
struct Window *window;

	window = LIBUA_OpenRequesterWindow(onWindow, GR, STDCOLORS);
	if ( window==NULL)
		return(NULL);

	if ( LIBUA_IsWindowOnLacedScreen(window) )
		laced = TRUE;
	else
		laced = FALSE;

	FontSlider1.LeftEdge	= GR[3].x1+4;
	FontSlider1.TopEdge		= GR[3].y1+2;
	FontSlider1.Width			= GR[3].x2-GR[3].x1-7;
	FontSlider1.Height		= GR[3].y2-GR[3].y1-3;

	if ( LIBUA_IsWindowOnLacedScreen(window) )
	{
		FontSlider1.TopEdge	+= 2;
		FontSlider1.Height	-= 4;
	}

	FontSlider2.LeftEdge	= GR[5].x1+4;
	FontSlider2.TopEdge		= GR[5].y1+2;
	FontSlider2.Width			= GR[5].x2-GR[5].x1-7;
	FontSlider2.Height		= GR[5].y2-GR[5].y1-3;

	if ( LIBUA_IsWindowOnLacedScreen(window) )
	{
		FontSlider2.TopEdge	+= 2;
		FontSlider2.Height	-= 4;
	}

	InitPropInfo((struct PropInfo *)FontSlider1.SpecialInfo, (struct Image *)FontSlider1.GadgetRender);
	InitPropInfo((struct PropInfo *)FontSlider2.SpecialInfo, (struct Image *)FontSlider2.GadgetRender);

	AddGadget(window, &FontSlider1, -1L);
	AddGadget(window, &FontSlider2, -1L);

	LIBUA_DrawGadgetList(window, GR);

	LIBUA_SetPropSlider(window, &FontSlider1, FER->numEntries1, 6, FER->top1);

	if ( FER->selected1 != -1 )	// a font is currently selected
	{
		FE = (struct FontEntry *)FER->FEList[ FER->selected1 ];
		i=0;	// get the number of sizes this font has...
		while( FE->fontSize[i] != 0 )
			i++;
		FER->numEntries2 = i;
		LIBUA_SetPropSlider(window, &FontSlider2, FER->numEntries2, 4, FER->top2);
		LIBUA_SetStringGadgetToVal(window, &GR[6], FE->fontSize[0]);	// size
	}
	else
		LIBUA_SetPropSlider(window, &FontSlider2, 0, 4, 0);	// get a nice drag bar

	DoFontScrolling(window, FER->numEntries1, 6, &(FER->top1), &GR[2],
									&FontSlider1, TRUE, 1, FER);
	DoFontScrolling(window, FER->numEntries2, 4, &(FER->top2), &GR[4],
									&FontSlider2, TRUE, 2, FER);

	if ( FER->selected1 == -1 )	/* no font selected */
	{
		LIBUA_DisableButton(window, &GR[6], mypattern1);	// size
		LIBUA_DisableButton(window, &GR[8], mypattern1);	// OK
	}

	if ( FER->selected1 != -1 && FER->fontSize != -1 )	/* size string used */
	{
		LIBUA_EnableButton(window, &GR[6]);	// size
		LIBUA_SetStringGadgetToVal(window, &GR[6], FER->fontSize);
		LIBUA_EnableButton(window, &GR[8]);	// OK
	}

	return(window);
}

/******** CloseFontListWindow() ********/

void __saveds __asm LIBUA_CloseFontListWindow(register __a0 struct Window *window)
{
	LIBUA_CloseRequesterWindow(window, STDCOLORS);
}

/*******************************************************************/
/*
 *   PUBLIC FUNCTIONS
 *
 *******************************************************************/

/******** doFontSliders() ********/

void doFontSliders(	struct Window *window, struct EventData *CED,
										struct GadgetRecord *GR, struct FER *FER )
{
ULONG signals, signalMask;
BOOL loop=TRUE, mouseMoved=FALSE;
struct IntuiMessage *message;
int ID;
struct Gadget *g;
int toppie;
LONG f;

	signalMask = 1L << capsport->mp_SigBit;

	g = (struct Gadget *)CED->IAddress;
	ID = g->GadgetID;

	if ( CED->Qualifier & IEQUALIFIER_LSHIFT || CED->Qualifier & IEQUALIFIER_RSHIFT )
	{
		if ( ID==1 )
		{
			f = ( (CED->MouseY - g->TopEdge) * FER->numEntries1) / g->Height;
			if ( f < 0 )
				f = 0;
			FER->top1 = f;
			if ( (FER->top1+6) > FER->numEntries1 )
				FER->top1 = FER->numEntries1-6;
			LIBUA_SetPropSlider(window, g, FER->numEntries1, 6, FER->top1);
			DoFontScrolling(window, FER->numEntries1, 6, &(FER->top1), &GR[2],
											&FontSlider1, FALSE, 1, FER);
			return;
		}
		else if ( ID==2 )
		{
			f = ( (CED->MouseY - g->TopEdge) * FER->numEntries2) / g->Height;
			if ( f < 0 )
				f = 0;
			FER->top2 = f;
			if ( (FER->top2+4) > FER->numEntries2 )
				FER->top2 = FER->numEntries2-4;
			LIBUA_SetPropSlider(window, g, FER->numEntries2, 4, FER->top2);
			DoFontScrolling(window, FER->numEntries2, 4, &(FER->top2), &GR[4],
											&FontSlider2, FALSE, 2, FER);
			return;
		}
	}

	/**** always scroll slider (in case of clicking above or beneath knob ****/

	switch(ID)
	{
		case 1:
			DoFontScrolling(window, FER->numEntries1, 6, &(FER->top1), &GR[2],
											&FontSlider1, FALSE, 1, FER);
			break;
		case 2:
			DoFontScrolling(window, FER->numEntries2, 4, &(FER->top2), &GR[4],
											&FontSlider2, FALSE, 2, FER);
			break;
	}

	LIBUA_SwitchMouseMoveOn(window);

	while(loop)
	{
		signals = Wait(signalMask);
		if (signals & signalMask)
		{
			mouseMoved=FALSE;
			//while(message = (struct IntuiMessage *)GetMsg(window->UserPort))
			while(message = (struct IntuiMessage *)GetMsg(capsport))
			{
				CED->Class = message->Class;
				ReplyMsg((struct Message *)message);
				if ( CED->Class == IDCMP_MOUSEMOVE )
					mouseMoved=TRUE;
				else
					loop=FALSE;
			}
			if (mouseMoved)
			{
				if (FontSlider1.Flags & GFLG_SELECTED)
				{
					toppie=FER->top1;
					LIBUA_GetPropSlider(window, &FontSlider1, FER->numEntries1, 6, &toppie);
					if ( toppie!=FER->top1 )
						DoFontScrolling(window, FER->numEntries1, 6, &(FER->top1), &GR[2],
														&FontSlider1, FALSE, 1, FER);
					loop=TRUE;
				}
				else if (FontSlider2.Flags & GFLG_SELECTED)
				{
					toppie=FER->top2;
					LIBUA_GetPropSlider(window, &FontSlider2, FER->numEntries2, 4, &toppie);
					if ( toppie!=FER->top2 )
						DoFontScrolling(window, FER->numEntries2, 4, &(FER->top2), &GR[4],
														&FontSlider2, FALSE, 2, FER);
					loop=TRUE;
				}
				else
					loop=FALSE;
			}
		}
	}

	LIBUA_SwitchMouseMoveOff(window);
}

/******** DoFontScrolling() ********/

void DoFontScrolling(	struct Window *window, int numEntries,
											int numDisplay, int *top, struct GadgetRecord *GR,
											struct Gadget *g, BOOL drawAlways, int which,
											struct FER *FER)
{
int i, lines, displayFactor, offset;
TEXT printStr[50];
struct FontEntry *FE;

	LIBUA_GetPropSlider(window, g, numEntries, numDisplay, top);

	if (laced)
		displayFactor=18;
	else
		displayFactor=9;

	lines=numDisplay;
	if (numEntries<lines)
		lines=numEntries;

	SetDrMd(window->RPort, JAM1);

	for(i=0; i<lines; i++)
	{
		my_SetAPen(window, ChoosePen(window,GR,AREA_PEN));

		RectFill(	window->RPort,
							GR->x1+2, GR->y1+2+displayFactor*i,
							GR->x2-2, GR->y1+window->RPort->TxBaseline+4+displayFactor*i);

		//my_SetAPen(window, ChoosePen(window,GR,LO_PEN));
		my_SetAPen(window, ChooseTextPen(window,GR));

		if (which==1)	/* font names */
		{
			FE = (struct FontEntry *)FER->FEList[*top+i];

			stccpy(printStr, FE->fontName, 50);
			LIBUA_ShortenString(window->RPort, printStr, (GR->x2-GR->x1)-16);

			Move(window->RPort, GR->x1+3, GR->y1+window->RPort->TxBaseline+2+displayFactor*i);
			Text(window->RPort, printStr, strlen(printStr));
	
			if ( FER->selected1 == (*(top)+i) )
				HiliteFontLine(window, GR, i);
		}
		else	// sizes
		{
			if ( FER->selected1 != -1 )
			{
				FE = (struct FontEntry *)FER->FEList[ FER->selected1 ];
				sprintf(printStr, "%d", FE->fontSize[*top+i]);
				offset = TextLength(window->RPort, printStr, strlen(printStr));
				Move(window->RPort, GR->x2-3-offset, GR->y1+window->RPort->TxBaseline+2+displayFactor*i);
				Text(window->RPort, printStr, strlen(printStr));
				if ( FER->selected2 == (*(top)+i) )
					HiliteFontLine(window, GR, i);
			}
		}
	}
}

/******** HiliteFontLine() ********/

void HiliteFontLine(struct Window *window, struct GadgetRecord *GR, int line)
{
int displayFactor;

	if (laced)
		displayFactor=18;
	else
		displayFactor=9;

	SetDrMd(window->RPort, JAM2 | COMPLEMENT);
	SafeSetWriteMask(window->RPort, 0x7);

	RectFill(	window->RPort, GR->x1+2, GR->y1+2+displayFactor*line,
						GR->x2-2, GR->y1+window->RPort->TxBaseline+3+displayFactor*line);

	SetDrMd(window->RPort, JAM1);
	SafeSetWriteMask(window->RPort, 0xff);
}

/******** SelectFontLine() ********/

BOOL SelectFontLine(struct Window *window, int mode, struct FER *FER,
										UWORD *mypattern1, struct GadgetRecord *GR,
										struct EventData *CED)
{
int line, displayFactor, numEntries, y1, i, max;
struct FontEntry *FE;

	if (mode==1)
	{
		max = 6;
		numEntries = FER->numEntries1;
		y1 = GR[2].y1;
	}
	else
	{
		max = 4;
		numEntries = FER->numEntries2;
		y1 = GR[4].y1;
	}

	if (laced)
		displayFactor=18;
	else
		displayFactor=9;

	line = (CED->MouseY-y1) / displayFactor;
	if (numEntries<max)
	{
		if (line-1>=(numEntries-1))
			line=-1;
	}
	else if (line<0 || line>=max)
		line=-1;

	if (line != -1)
	{
		if (mode==1)	/* font chosen */
		{
			FER->selected1 = FER->top1+line;
			DoFontScrolling(window, FER->numEntries1, 6, &(FER->top1), &GR[2],
											&FontSlider1, TRUE, 1, FER);

			if ( FER->selected1 != -1 )
			{
				FE = (struct FontEntry *)FER->FEList[ FER->top1+line ];
				i=0;
				while( FE->fontSize[i] != 0 )
					i++;
				FER->numEntries2 = i;
//				FER->selected2 = -1;
				FER->selected2 = 0;	// NEW PER Saturday 24-Jul-93 15:02:38      
				FER->top2 = 0;

FER->fontSize=-1;		// NEW PER Friday 01-Jul-94 10:00:31

				LIBUA_ClearButton(window, &GR[4], AREA_PEN);
				DoFontScrolling(window, FER->numEntries2, 4,
												&(FER->top2), &GR[4],
												&FontSlider2, TRUE, 2, FER);
				LIBUA_SetPropSlider(window, &FontSlider2,
														FER->numEntries2, 4, FER->top2);

				//LIBUA_DisableButton(window, &GR[7], mypattern1);	// show

				LIBUA_EnableButton(window, &GR[6]);	// size string
				LIBUA_SetStringGadgetToVal(window, &GR[6], FE->fontSize[0]);
			}
		}
		else	/* size chosen */
		{
			FER->selected2 = FER->top2+line;
			DoFontScrolling(window, FER->numEntries2, 4,
											&(FER->top2), &GR[4],
											&FontSlider2, TRUE, 2, FER);
			LIBUA_EnableButton(window, &GR[8]);	// OK
		}

		return(TRUE);
	}

	return(FALSE);	/* clicked outside box */
}

/******** fontSample() ********/

struct TextFont *fontSample(struct Window *window, struct GadgetRecord *GR,
														struct FER *FER, struct TextFont *TheFont,
														BOOL showSample )
{
struct FontEntry *FE;
int size, y, max;
TEXT fontName[50];
UBYTE flags;
struct TextAttr textAttr;
struct TextFont *textFont, *currentFont;
struct Region *region;

	if ( !FER )
	{
		LIBUA_ClearButton(window, GR, AREA_PEN);
		return(NULL);
	}

	max = 80;
	if (laced)
		max *= 2;

	if ( TheFont != NULL )
		CloseFont(TheFont);
	TheFont = NULL;

	LIBUA_ClearButton(window, GR, AREA_PEN);

	currentFont = (struct TextFont *)window->RPort->Font;

	if ( FER->selected1 != -1 )
	{
		FE = (struct FontEntry *)FER->FEList[ FER->selected1 ];
		if ( FER->selected2 != -1 )
			size = FE->fontSize[ FER->selected2 ];
		else if ( FER->fontSize != -1 )
			size = FER->fontSize;

		sprintf(fontName, "%s.font", &FE->fontName[1]);	// skip ID

		if ( FE->fontName[0] == '' )	/* 0x90 --> outlined font */
			flags = NULL;
		else
			flags = FPF_DESIGNED;				/* "" (0x8f) --> bitmapped font */

		textAttr.ta_Name	= (UBYTE *)fontName;
		textAttr.ta_YSize	= size;
		textAttr.ta_Style	= FS_NORMAL;
		textAttr.ta_Flags	= NULL;

		if ( strcmp(fontName, "topaz.font")==0 && (size==8 || size==9) )
			textAttr.ta_Flags	|= FPF_ROMFONT;

		LIBUA_SetSprite(window, SPRITE_BUSY);

		textFont = (struct TextFont *)OpenDiskFont(&textAttr);
		if (textFont==NULL)
			return(NULL);

		LIBUA_SetSprite(window, SPRITE_NORMAL);

		if ( showSample )
		{
			SetFont(window->RPort, textFont);
			//my_SetAPen(window, ChoosePen(window,GR,LO_PEN));
			my_SetAPen(window, ChooseTextPen(window,GR));
			SetDrMd(window->RPort, JAM1);

			region = (struct Region *)clipWindow(	window, GR->x1, GR->y1,
																						GR->x2, GR->y2);
			strcpy(fontName, "AaBbCcDdEe0123456789");
			if ( window->RPort->TxHeight < max ) //86 )
				y = window->RPort->TxBaseline + 1;
			else
				y = max;
			Move(window->RPort, GR->x1+4, GR->y1+4+y);
			Text(window->RPort, fontName, strlen(fontName));
			unclipWindow(window);
		}

		if (currentFont!=NULL)
			SetFont(window->RPort, currentFont);

		TheFont = textFont;
	}

	return(TheFont);
}

/******** unclipWindow() ********/
/*
 * code lifted from RKM page 723
 *
 * Used to remove a clipping region installed by clipWindow(),
 * disposing of the installed region and reinstalling the region removed.
 *
 */

STATIC void unclipWindow(struct Window *win)
{
struct Region *old_region;

	/* Remove any old region by installing a NULL region,
	** then dispose of the old region if one was installed.
	*/

	if (NULL != (old_region = InstallClipRegion(win->WLayer,NULL)))
		DisposeRegion(old_region);
}

/******** clipWindow() ********/
/*
 * code lifted from RKM page 723
 *
 * Clip a window to a specified rectangle (given by upper left and lower
 * right corner). The removed region is returned so that it may be re-
 * installed later.
 *
 */

STATIC struct Region *clipWindow(	struct Window *win, LONG minX, LONG minY,
																	LONG maxX, LONG maxY)
{
struct Region *new_region;
struct Rectangle my_rectangle;

	my_rectangle.MinX = minX+2; 
	my_rectangle.MinY = minY+1;
	my_rectangle.MaxX = maxX-2;
	my_rectangle.MaxY = maxY-1;

	if (NULL != (new_region = NewRegion()))
	{
		if (FALSE == OrRectRegion(new_region, &my_rectangle))
		{
			DisposeRegion(new_region);
			new_region = NULL;
		}
	}

	/* Install the new region, and return any existing region.
	** If the above allocation and region processing failed, then
	** new_region will be NUL and no clip will be installed.
	*/

	return( InstallClipRegion(win->WLayer, new_region) );
}

/******** E O F ********/
