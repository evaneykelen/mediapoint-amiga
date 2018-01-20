#include "nb:pre.h"
#include <intuition/sghooks.h>

#define CRAWLSIZE 2048L

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Library *medialinkLibBase;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct TextFont *largeFont;
extern struct TextFont *textFont;
extern UBYTE **msgs;   
extern struct RendezVousRecord rvrec;

/**** gadgets ****/

extern struct GadgetRecord Crawl_GR[];

/**** functions ****/

/******** MonitorCrawl() ********/

BOOL MonitorCrawl(void)
{
struct Window *window;
BOOL loop, retVal;
int ID, active, fontSize, my_int, col, i;
struct StringInfo SI;
struct Gadget G;
struct StringExtend SE;
UBYTE *txt, *undotxt;
struct EditWindow undoEW, *ew;
TEXT info[100], fontName[51], fileName[SIZE_FILENAME], fullPath[SIZE_FULLPATH];
UWORD literal,x,y,w,h;
TEXT insstr[50];
struct TextFont *oldFont, *myFont;

	/**** make working copy of active window ****/

	active = FirstActiveEditWindow();
	if (active==-1)
		return(FALSE);
	ew = EditWindowList[active];
	CopyMem(ew, &undoEW, sizeof(struct EditWindow));

	/**** alloc string gadget mem ****/

	txt = (UBYTE *)AllocMem(CRAWLSIZE, MEMF_CLEAR | MEMF_ANY);
	if (txt==NULL)
		return(FALSE);

	undotxt = (UBYTE *)AllocMem(CRAWLSIZE, MEMF_CLEAR | MEMF_ANY);
	if (undotxt==NULL)
	{
		FreeMem(txt, CRAWLSIZE);
		return(FALSE);
	}

	/**** open a window ****/

	window = UA_OpenRequesterWindow(pageWindow,Crawl_GR,STDCOLORS);
	if (!window)
	{
		FreeMem(txt, CRAWLSIZE);
		FreeMem(undotxt, CRAWLSIZE);
		UA_WarnUser(-1);
		return(FALSE);
	}

	/**** render gadgets ****/

	UA_DrawGadgetList(window, Crawl_GR);

	DrawStrGad(window, &Crawl_GR[4], txt, undotxt, CRAWLSIZE, &SI, &G, &SE);

	UA_PrintInBox(	window, &Crawl_GR[10], Crawl_GR[10].x1+1,Crawl_GR[10].y1,Crawl_GR[10].x2,Crawl_GR[10].y2,
									"\0", PRINT_CENTERED);	// disk (0x16)

	/**** init vars ****/

	retVal = FALSE;
	loop   = TRUE;

	if ( ew->crawl_fontName[0]=='\0' )	// this window carries no crawl data yet
	{
		strcpy(ew->crawl_fontName,"MediaPoint");
		ew->crawl_fontSize	= 20;
		ew->crawl_speed			= 12;
		ew->crawl_flags			= 0;
		ew->crawl_text			= NULL;
		ew->crawl_length		= 0;
 
		if ( ew->charStyle != 0 || ew->charColor != 2 || ew->charFont	!= largeFont )
		{
			if (findFont(ew->charFont, fontName, &fontSize))
			{
				strcpy(ew->crawl_fontName, fontName);
				ew->crawl_fontSize = fontSize;
			}
		}

		/**** steal window text ****/

		if ( ew->TEI )
		{
			i=0;
			while(i<ew->TEI->textLength && i<CRAWLSIZE)
			{
				txt[i] = ew->TEI->text[i].charCode;
				i++;
			}
			SI.BufferPos = 0;
			RemoveGList(window, &G, 1);
			AddGList(window, &G, ~0, 1, NULL);
			RefreshGList(&G, window, NULL, 1);
		} 
	}
	else
	{
		strcpy(txt,ew->crawl_text);

		SI.BufferPos = 0;
		RemoveGList(window, &G, 1);
		AddGList(window, &G, ~0, 1, NULL);
		RefreshGList(&G, window, NULL, 1);

		if ( ew->crawl_flags & 1 )	// file
		{
			OffGadget(&G,window,NULL);
			UA_InvertButton(window, &Crawl_GR[10]);
		}
	}

	sprintf(info, "%s, %d points", ew->crawl_fontName, (int)ew->crawl_fontSize);
	UA_PrintInBox(window, &Crawl_GR[5],
								Crawl_GR[5].x1, Crawl_GR[5].y1,
								Crawl_GR[5].x2, Crawl_GR[5].y2, info, PRINT_RIGHTPART );

	UA_SetCycleGadgetToVal(window, &Crawl_GR[6], ew->crawl_speed-1);

	if ( !(ew->crawl_flags & 1) )	// file
		ActivateGadget(&G, window, NULL);

	/**** monitor user ****/

	Crawl_GR[5].type=BUTTON_GADGET;
	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if (CED.Class==IDCMP_MOUSEBUTTONS && CED.extraClass==IDCMP_GADGETUP &&
				CED.Code==RAW_HELP )
		{
			literal = InsertLiteral();
			if ( literal!=0 )
			{
				insstr[0]='\0';
				if ( literal<1001 )				// special char
				{
					insstr[0] = (char)literal;
					insstr[1] = '\0';
				}
				else if ( literal>=1001 )	// special @code
					ConvertDatePageToPlayer(insstr, literal-1000);	// start with 1,2,3...
				if (insstr[0]!='\0')
				{
					if ( ( strlen(txt)+strlen(insstr) ) < CRAWLSIZE-5 )
					{
						if ( SI.BufferPos>=0 && SI.BufferPos<CRAWLSIZE )
						{
							strins(&txt[SI.BufferPos],insstr);
							SI.BufferPos += strlen(insstr);
							RemoveGList(window, &G, 1);
							AddGList(window, &G, ~0, 1, NULL);
							RefreshGList(&G, window, NULL, 1);
							ActivateGadget(&G, window, NULL);
						}
					}
				}
			}
		}
		else if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, Crawl_GR, &CED);
			switch(ID)
			{
				case 5:		// fontname/size
					UA_HiliteButton(window, &Crawl_GR[ID]);
					if ( Monitor_FontSelection(ew) )
					{
						if ( findFont(textFont,fontName,&fontSize) )
						{
							strcpy(ew->crawl_fontName, fontName); //, strlen(fontName)-4);
							ew->crawl_fontSize = fontSize;
							sprintf(info, "%s, %d points", ew->crawl_fontName, (int)ew->crawl_fontSize);
							UA_PrintInBox(window, &Crawl_GR[5],
														Crawl_GR[5].x1, Crawl_GR[5].y1,
														Crawl_GR[5].x2, Crawl_GR[5].y2, info, PRINT_RIGHTPART );
						}
					}
					break;

				case 6:		// speed
					UA_ProcessCycleGadget(window, &Crawl_GR[ID], &CED);
					UA_SetValToCycleGadgetVal(&Crawl_GR[ID], &my_int);
					ew->crawl_speed = my_int+1;
					break;

				case 7:		// OK
do_ok:
					UA_HiliteButton(window, &Crawl_GR[7]);
					loop=FALSE;
					retVal=TRUE;
					break;

				case 8:		// Cancel
do_cancel:
					UA_HiliteButton(window, &Crawl_GR[8]);
					loop=FALSE;
					retVal=FALSE;
					break;

				case 9:		// color well selection
					UA_HiliteButton(window, &Crawl_GR[ID]);
					UA_ResetMenuColors(&rvrec,pageWindow);
					col = OpenSmallPalette((int)ew->crawl_color,2,FALSE);
					if ( col != -1 )
						ew->crawl_color = (UBYTE)col;
					UA_SetMenuColors(&rvrec,pageWindow);
					break;

				case 10:	// file req
					UA_InvertButton(window, &Crawl_GR[ID]);

					if ( ew->crawl_flags & 1 )	// file, go to normal mode
					{
						ew->crawl_flags = ew->crawl_flags & ~1;
						SimpleGetFile(txt,txt,CRAWLSIZE);
					}
					else												// normal, go to file mode
					{
						if (	OpenAFile(	CPrefs.import_text_Path, fileName,
															msgs[Msg_SelectATextFile-1], window,
															DIR_OPT_ALL | DIR_OPT_NOINFO, FALSE) )
						{
							ew->crawl_flags |= 1;
							UA_MakeFullPath(CPrefs.import_text_Path, fileName, fullPath);
							strcpy(txt, fullPath);
						}
						else
							UA_InvertButton(window, &Crawl_GR[ID]);	// back to normal mode
					}

					SI.BufferPos = 0;
					RemoveGList(window, &G, 1);
					AddGList(window, &G, ~0, 1, NULL);
					RefreshGList(&G, window, NULL, 1);

					if ( ew->crawl_flags & 1 )	// file
						OffGadget(&G,window,NULL);
					else
						OnGadget(&G,window,NULL);
					break;		
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE)	// cancel
				goto do_cancel;
			else if (CED.Code==RAW_RETURN && !UA_IsGadgetDisabled(&Crawl_GR[7]))	// OK
				goto do_ok;
		}
	}
	Crawl_GR[5].type=HIBOX_REGION;

	UA_CloseRequesterWindow(window,STDCOLORS);

	if ( retVal )
	{
		if ( strlen(txt)==0 )
		{
			if ( ew->crawl_text!=NULL )
				FreeMem(ew->crawl_text, ew->crawl_length);
			ew->crawl_fontName[0]	= '\0';
			ew->crawl_fontSize		= 0;
			ew->crawl_speed				= 0;
			ew->crawl_flags				= 0;
			ew->crawl_text				= NULL;
			ew->crawl_length			= 0;

			DrawAllHandles(LEAVE_ACTIVE);
			RedrawAllOverlapWdw(ew->X,ew->Y,ew->Width,ew->Height,
													ew->X,ew->Y,ew->Width,ew->Height,
													active,TRUE,TRUE);
			DrawAllHandles(LEAVE_ACTIVE);
		}
		else if ( strlen(txt)>0 )
		{
			/**** first free old memory ****/

			if ( ew->crawl_text!=NULL )
				FreeMem(ew->crawl_text, ew->crawl_length);

			/**** allocate new memory ****/

			ew->crawl_length = strlen(txt)+10;
			ew->crawl_text = (UBYTE *)AllocMem(ew->crawl_length, MEMF_CLEAR | MEMF_ANY);

			if (ew->crawl_text!=NULL)
				strcpy(ew->crawl_text,txt);

			/**** remove window text ****/

			if ( ew->TEI )
			{
				ew->TEI->text[0].charCode = 0;
				ew->TEI->textLength = 0;
				TESetSelect(ew->TEI, 0, 0);		// herstel cursor op linksboven
				TESetUpdateRange( ew, LEVEL_FULL );
			}

			/**** repos/resize window ****/

			sprintf(fontName, "%s.font", ew->crawl_fontName); 

			oldFont = textFont;
			if ( OpenTypeFace(fontName, ew->crawl_fontSize, 0, TRUE) )
				myFont = textFont;
			else
				myFont = oldFont;
			textFont = oldFont;

			DrawAllHandles(LEAVE_ACTIVE);

			x = ew->X;
			y = ew->Y;
			w = ew->Width;
			h = ew->Height;

			ew->X = 0;
			ew->Width = pageWindow->Width;
			ew->Height = myFont->tf_YSize + 8;	// 8 for Cees

			if ( (ew->Y + ew->Height) > CPrefs.PageScreenHeight )
				ew->Y = CPrefs.PageScreenHeight - ew->Height;

			ew->BackFillType = 2;	// transp.

			ew->TEI->newText.charFont = myFont;

			CorrectEW(ew);
			//ValidateBoundingBox(&ew->X,&ew->Y,&ew->Width,&ew->Height);

			RedrawAllOverlapWdw(x,y,w,h, ew->X,ew->Y,ew->Width,ew->Height,
													active,TRUE,TRUE);

			DrawAllHandles(LEAVE_ACTIVE);
		}
	}
	else
		CopyMem(&undoEW, ew, sizeof(struct EditWindow));

	FreeMem(txt, CRAWLSIZE);
	FreeMem(undotxt, CRAWLSIZE);

	return(retVal);
}

/******** DrawStrGad() ********/

void DrawStrGad(struct Window *window, struct GadgetRecord *GR, UBYTE *txt,
								UBYTE *undo, int max, struct StringInfo *SI, struct Gadget *G,
								struct StringExtend *SE)
{
	SI->Buffer					= txt;
	SI->UndoBuffer			= undo;
	SI->BufferPos				= 0;
	SI->MaxChars				= max;

	SE->Font						= (struct TextFont *)window->RPort->Font;

	if ( window->RPort->BitMap->Depth==1 )
	{
		SE->Pens[0]				= 1;
		SE->Pens[1]				= 0;
		SE->ActivePens[0]	= 1;
		SE->ActivePens[1]	= 0;
	}
	else
	{
		SE->Pens[0]				= LO_PEN;
		SE->Pens[1]				= UA_GetRightPen(window,GR,AREA_PEN);
		SE->ActivePens[0]	= LO_PEN;
		SE->ActivePens[1]	= UA_GetRightPen(window,GR,AREA_PEN);
	}

	SE->InitialModes		= SGM_EXITHELP;
	SE->EditHook				= NULL;
	SE->WorkBuffer			= NULL;

	SI->Extension = (struct StringExtend *)SE;

	G->LeftEdge					= GR->x1 + 4;
	G->TopEdge					= GR->y1 + 2;
	G->Width						= GR->x2 - GR->x1 - 7;
	G->Height						= GR->y2 - GR->y1 - 3;
	G->Flags						= GFLG_GADGHNONE | GFLG_STRINGEXTEND;
	G->Activation				= GACT_STRINGLEFT | GACT_RELVERIFY;
	G->GadgetType				= GTYP_STRGADGET;
	G->GadgetRender			= NULL;
	G->SelectRender			= NULL;
	G->GadgetText				= NULL;
	G->MutualExclude		= 0L;
	G->SpecialInfo			= SI;
	G->GadgetID					= 0;
	G->UserData					= NULL;

	AddGadget(window, G, -1L);
}

/******** SimpleGetFile() ********/

BOOL SimpleGetFile(STRPTR fullpath, UBYTE *txt, ULONG max)
{
FILE *fp;
char c;
int i;

	fp = fopen(fullpath,"r");
	if (fp==NULL)
		return(FALSE);

	i=0;
	while( ((c=getc(fp)) != EOF) && i<CRAWLSIZE-5 )
	{
		if ( c<0x20 )
			c=0x20;
		txt[i] = c;
		i++;
	}

	fclose(fp);

	return(TRUE);
}

/******** E O F ********/
