#include "nb:pre.h"

/**** defines ****/

#define NONE_COLS 			0
#define COPYING_COLS		1
#define SWAPPING_COLS		2
#define SPREADING_COLS	3

/**** externals ****/

extern ULONG allocFlags;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct MsgPort *capsPort;
extern struct eventHandlerInfo EHI;
extern struct Library *medialinkLibBase;
extern struct Window *pageWindow;
extern struct Screen *pageScreen;
extern struct Window *paletteWindow;
extern struct Screen *paletteScreen;
extern struct IntuitionBase *IntuitionBase;
extern int selectedWell;
extern struct ColorMap *undoCM;
extern struct Library *medialinkLibBase;

/**** gadgets ****/

extern struct GadgetRecord ColorAdjust_GR[];

/**** functions ****/

/******** Monitor_ColorAdjust() ********/
/*
 * Make sure that there's no way that the USER can modify the palette
 * without CPrefs.PageCM knowing about it!
 *
 */

int Monitor_ColorAdjust(void)
{
ULONG signals;
int numCols, ID, r, g, b,steps_factor,how;
struct ViewPort *vp;
struct ColorMap *cm;
BOOL loop=TRUE, takeAction=TRUE;
int actionStartedAt, actionEndedAt, colorchange_mode = NONE_COLS;

	if ( !paletteWindow )
		return(NOTHING_TO_EXAMINE);

	/**** init vars ****/

	colorchange_mode = NONE_COLS;
	numCols = GetNumberOfModeColors(CPrefs.PageScreenModes, CPrefs.PageScreenDepth);
	if ( CPrefs.PageScreenModes & EXTRA_HALFBRITE )
		numCols=32;

	how = HowToFillTheWells(numCols);
	vp = &(pageScreen->ViewPort);
	cm = vp->ColorMap;

	if (CPrefs.AA_available) //GfxBase->LibNode.lib_Version >= 39)
		steps_factor = 1;
	else
		steps_factor = 16;

	{
	struct RasInfo *RI;
		RI = pageScreen->ViewPort.RasInfo;
		CED.MouseY -= RI->RyOffset;
	}

	/**** handle events ****/

	do
	{
		if (takeAction)
		{
			if ( CED.Class == IDCMP_MOUSEBUTTONS && CED.Code == SELECTDOWN )
			{
				/**** color adjust gadgets ****/

				ID=-1;

				ID = UA_CheckGadgetList(paletteWindow, ColorAdjust_GR, &CED);
pretend_mouse:
				switch(ID)
				{
					case 2:	// Hide
doclose:
						UA_HiliteButton(paletteWindow, &ColorAdjust_GR[2]);
						PaletteToBack();
						return(NOTHING_TO_EXAMINE);
						break;

					case 14:	/* SPREAD */
					case 15:	/* COPY */
					case 16:	/* SWAP */
						UA_HiliteButton(paletteWindow, &ColorAdjust_GR[ID]);
						if (colorchange_mode!=NONE_COLS)
						{
							colorchange_mode=NONE_COLS;
							SetSpriteOfActWdw(SPRITE_NORMAL); //UA_SetSprite(paletteWindow,SPRITE_NORMAL);
						}
						else
						{
							if (ID==14)
								colorchange_mode=SPREADING_COLS;
							else if (ID==15)
								colorchange_mode=COPYING_COLS;
							else if (ID==16)
								colorchange_mode=SWAPPING_COLS;
							SetSpriteOfActWdw(SPRITE_TOSPRITE); //UA_SetSprite(paletteWindow,SPRITE_TOSPRITE);
							actionStartedAt = selectedWell;
						}
						break;

					case 17:	/* Undo */
						UA_InvertButton(paletteWindow, &ColorAdjust_GR[ID]);
						UndoCols();
						GetColorComponents(cm, selectedWell, &r, &g, &b);
						SetSliderColor(r, g, b, selectedWell, numCols, how);
						InitSampleAndSliders(selectedWell);
						UA_InvertButton(paletteWindow, &ColorAdjust_GR[ID]);
						break;

					case 18:	// load palette
						UA_HiliteButton(paletteWindow, &ColorAdjust_GR[ID]);
						CopyCMtoCM(pageScreen->ViewPort.ColorMap, undoCM);
						ReadColorPalette();
						break;

					case 19:	// save palette
						UA_HiliteButton(paletteWindow, &ColorAdjust_GR[ID]);
						CopyCMtoCM(pageScreen->ViewPort.ColorMap, undoCM);
						WriteColorPalette();
						break;

					case 20:	// harmonize palette / color ramp palette
						UA_HiliteButton(paletteWindow, &ColorAdjust_GR[ID]);
						if ( CED.Qualifier&IEQUALIFIER_LSHIFT || CED.Qualifier&IEQUALIFIER_RSHIFT )
							ColorRamp();
						else
							Harmonize();
						SelectNewWell(0, selectedWell);
						break;

					case 21:	// best colors
						UA_HiliteButton(paletteWindow, &ColorAdjust_GR[ID]);
						CopyCMtoCM(pageScreen->ViewPort.ColorMap, undoCM);
						if ( !OptimizePalette(TRUE) )
							Message("optimizing failed");
						else
						{
							// START SMARTSTEP
							SyncAllColors(FALSE);
							RemapAllPicsAndBackground();
							// END SMARTSTEP
						}
						break;

					case 9:	/* sample area and pick start */
						PickAColor();
						break;

					case 6:	/* r */
					case 7:	/* g */
					case 8:	/* b */
						CopyCMtoCM(pageScreen->ViewPort.ColorMap, undoCM);
						GetColorComponents(cm, selectedWell, &r, &g, &b);
						if (ID==6)
							ProcessPaletteSlider(	paletteWindow, &ColorAdjust_GR[6],
																		&r, steps_factor, 1, selectedWell);
						else if (ID==7)
							ProcessPaletteSlider(	paletteWindow, &ColorAdjust_GR[7],
																		&g, steps_factor, 2, selectedWell);
						else if (ID==8)
							ProcessPaletteSlider(	paletteWindow, &ColorAdjust_GR[8],
																		&b, steps_factor, 3, selectedWell);
						SetSliderColor(r, g, b, selectedWell, numCols, how);
						break;
				}

				/**** palette gadgets ****/

				if ( ID==-1 )	// no button was selected above
					ID = FindSelectedWell(CED.MouseX, CED.MouseY);
				else
					ID = -1;	// a button was selected above --> don't do things below

				if (ID!=-1 && ID<numCols)
				{
					SelectNewWell(0, ID);
					if (colorchange_mode != NONE_COLS)
					{
						actionEndedAt=selectedWell;

						if (colorchange_mode==COPYING_COLS)
							CopyCols(actionStartedAt, actionEndedAt);
						else if (colorchange_mode==SWAPPING_COLS)
							SwapCols(actionStartedAt, actionEndedAt);
						else if (colorchange_mode==SPREADING_COLS)
							SpreadCols(actionStartedAt, actionEndedAt);

						SelectNewWell(0, actionEndedAt);

						colorchange_mode=NONE_COLS;
						SetSpriteOfActWdw(SPRITE_NORMAL); //UA_SetSprite(paletteWindow,SPRITE_NORMAL);
					}

					SyncAllColors(FALSE);

					FlipperColors( selectedWell );

					return(NOTHING_TO_EXAMINE);
				}
			}
			else if (CED.Class==IDCMP_MENUPICK) // || CED.Class==IDCMP_CLOSEWINDOW)
			{
				SyncAllColors(FALSE);
				return(IDCMP_TO_EXAMINE);
			}
			else if (CED.Class == IDCMP_RAWKEY)
			{
#if 0
				if (CED.Code==0x21)	/* raw s, do a spread */
				{ ID=14; CED.MouseX=-1; goto pretend_mouse; }
				else if (CED.Code==0x33)	/* raw c, do a copy */
				{ ID=15; CED.MouseX=-1; goto pretend_mouse; }
				else if (CED.Code==0x32)	/* raw x, do a swap */
				{ ID=16; CED.MouseX=-1; goto pretend_mouse; }
				else if (CED.Code==0x16)	/* raw u, do an undo */
				{ ID=17; CED.MouseX=-1; goto pretend_mouse; }
				else if (CED.Code==0x38)	/* raw comma, do a color pick */
				{ ID=9; CED.MouseX=-1; goto pretend_mouse; }
				else if (	CED.Code==RAW_ESCAPE || CED.Code==RAW_RETURN ||
									CED.Code==RAW_SPACE )
					goto doclose;
				else
#endif
				{
					SyncAllColors(FALSE);
					return(IDCMP_TO_EXAMINE);
				}
			}
		}

		signals = Wait(SIGNALMASK);
		if (signals & SIGNALMASK)
		{
			HandleIDCMP(paletteWindow,FALSE,NULL);
			takeAction=TRUE;

			if ( !(paletteWindow->Flags & WFLG_WINDOWACTIVE) )
			{
				SyncAllColors(FALSE);
				return(IDCMP_TO_EXAMINE);
			}
		}
		else
			takeAction=FALSE;
	}
	while(loop);

	SyncAllColors(FALSE);

	return(NOTHING_TO_EXAMINE);
}

/******** SetSliderColor() ********/

void SetSliderColor(int r, int g, int b, int well, int numColors, int how)
{
int offset;
BOOL copperList=TRUE;

	if ( !paletteWindow)
		return;

	/**** change screen colors ****/

	SetVPToComponents(&(pageScreen->ViewPort), well, r, g, b);

	/**** change sample well colors ****/

	SetVPToComponents(&(paletteScreen->ViewPort), 4, r, g, b);

	/**** print screen colors ****/

	PrintPaintColor(r,g,b);

	/**** rebuild copper list OR directly change well colors ****/

	if ( TestBit(allocFlags, COPPERLIST_FLAG) )
	{
		if ( CPrefs.AA_available )
		{
			if ( numColors == 256 && well < 128)
			{
				copperList=FALSE;
				SetVPToComponents(&(paletteScreen->ViewPort), 64+well, r, g, b);
			}
		}
		else
		{
			if ( numColors == 64 && well < 16)
			{
				copperList=FALSE;
				SetVPToComponents(&(paletteScreen->ViewPort), well, r, g, b);
			}
			else if ( numColors < 64 && well < 8 )
			{
				copperList=FALSE;
				SetVPToComponents(&(paletteScreen->ViewPort), 8+well, r, g, b);
			}
		}

		if (copperList)
		{	
			MakeCopperList(how, TRUE);
			RebuildCopperList();
		}
	}
	else
	{
		if ( CPrefs.AA_available )
		{
			if ( numColors == 128 )
				offset = 32;
			else if ( numColors == 64 )
				offset = 16;
			else
				offset = 8;
		}
		else
		{
			if ( numColors == 64 )
				offset = 0;
			else
				offset = 8;
		}

		SetVPToComponents(&(paletteScreen->ViewPort), offset+well, r, g, b);
	}

	/**** re-adjust color 0 of palette screen ****/

	GetColorComponents(pageScreen->ViewPort.ColorMap, 0, &r, &g, &b);
	SetVPToComponents(&(paletteScreen->ViewPort), 0, r, g, b);
}

/******** PickAColor() ********/

void PickAColor(void)
{
struct IntuiMessage *message;
UWORD val;
ULONG signals;
BOOL loop, mouseMoved=FALSE, mouseDown=FALSE;
int max, prev_val=-1;

	if ( !paletteWindow)
		return;

	loop=TRUE;
	if ( CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTUP )
		loop=FALSE;
	while(loop)	// you came here on a SELECT DOWN -- Now first wait for SELECTUP
	{
		signals = Wait(SIGNALMASK);
		if (signals & SIGNALMASK)
		{
			while(message = (struct IntuiMessage *)GetMsg(capsPort))
			{
				CED.Class	= message->Class;
				CED.Code = message->Code;
				ReplyMsg((struct Message *)message);
				if ( CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTUP )
					loop=FALSE;
			}
		}
	}

	ActivateWindow(pageWindow);
	SetSpriteOfActWdw(SPRITE_COLORPICKER);
	UA_SwitchMouseMoveOn(pageWindow);

	loop=TRUE;
	while(loop)
	{
		signals = Wait(SIGNALMASK);
		if (signals & SIGNALMASK)
		{
			mouseMoved=FALSE;
			while(message = (struct IntuiMessage *)GetMsg(capsPort))
			{
				CED.Class	= message->Class;
				CED.Code = message->Code;
				CED.MouseX = message->MouseX;
				CED.MouseY = MassageY(message->MouseY);
				ReplyMsg((struct Message *)message);
				if ( CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN )
					mouseDown=TRUE;
				else if ( CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTUP )
					loop=FALSE;
				else if ( CED.Class==IDCMP_MOUSEMOVE )
					mouseMoved=TRUE;
			}
			if ( mouseMoved && mouseDown )
			{
				val=ReadPixel(pageWindow->RPort, CED.MouseX, CED.MouseY);
				max = UA_GetNumberOfColorsInScreen(	CPrefs.PageScreenModes, CPrefs.PageScreenDepth, CPrefs.AA_available );
				if (val>max)	// e.g. val=15 and max=16
					val=max-1;
				if ( val != prev_val )
				{
					SelectNewWell(0, val);
					prev_val = val;
				}
			}
		}
	}

	val=ReadPixel(pageWindow->RPort, CED.MouseX, CED.MouseY);
	max = UA_GetNumberOfColorsInScreen(	CPrefs.PageScreenModes, CPrefs.PageScreenDepth, CPrefs.AA_available );
	if (val>max)	// e.g. val=15 and max=16
		val=max-1;
	SelectNewWell(0, val);

	UA_SwitchMouseMoveOff(pageWindow);
	SetSpriteOfActWdw(SPRITE_NORMAL);
	ActivateWindow(paletteWindow);
}

/******** FlipperColors() ********/

void FlipperColors( int well )
{
BOOL loop;
struct ViewPort *vp;
struct ColorMap *cm;
BYTE flip=0;
int oldColors[5];
ULONG signals;
struct IntuiMessage *message;

	if ( !paletteWindow)
		return;

	vp = &(pageScreen->ViewPort);
	cm = vp->ColorMap;

	GetColorComponents(cm, well, &oldColors[0], &oldColors[1], &oldColors[2]);

	if ( CED.Qualifier&IEQUALIFIER_LSHIFT || CED.Qualifier&IEQUALIFIER_RSHIFT )
	{
		UA_SwitchFlagsOn(paletteWindow, IDCMP_INTUITICKS);

		loop=TRUE;
		while(loop)
		{
			signals = Wait(SIGNALMASK);
			if (signals & SIGNALMASK)
			{
				while(message = (struct IntuiMessage *)GetMsg(capsPort))
				{
					CED.Class		= message->Class;
					CED.Code		= message->Code;
					ReplyMsg((struct Message *)message);
					switch(CED.Class)
					{
						case IDCMP_MOUSEBUTTONS:
							if (CED.Code == SELECTUP)
								loop=FALSE;
							break;

						case IDCMP_RAWKEY:
							loop=FALSE;
							break;

						case IDCMP_INTUITICKS:
							if (flip==0)
							{
								flip=1;
								SetRGB4(vp, well, 0xf, 0x0, 0xf); // OS function
							}
							else
							{
								flip=0;
								SetRGB4(vp, well, 0x0, 0x0, 0x0); // OS function
							}
							break;
					}
				}
			}
		}

		UA_SwitchFlagsOff(paletteWindow, IDCMP_INTUITICKS);

		SetVPToComponents(&(pageScreen->ViewPort), well,
											oldColors[0], oldColors[1], oldColors[2]);
	}
}

/******** InitPickWell() ********/

void InitPickWell(void)
{
	//SelectNewWell(-1, selectedWell);
	InitSampleAndSliders(selectedWell);
}

/******** E O F ********/
