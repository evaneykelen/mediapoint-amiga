#include "nb:pre.h"
#include <pascal:include/txedstyles.h>

/**** externals ****/

extern struct EventData CED;
extern struct MsgPort *capsPort;
extern struct Library *medialinkLibBase;
extern struct Window *pageWindow;
extern struct TextFont *largeFont;
extern struct TextFont *smallFont;
extern UBYTE **msgs;
extern struct Gadget PropSlider1;
extern struct BitMap gfxBitMap;
extern struct RendezVousRecord rvrec;	// ADDED ERIK

/**** gadgets ****/

extern struct GadgetRecord Style_GR[];

/**** functions ****/

#if 0
/******** Monitor_StyleSelection() ********/
/*
 * touchedList shows which settings are changed:
 *
 * 0 = antialias
 * 1 = justif.
 * 2 = letter sp.
 * 3 = line sp.
 * 4 = slant
 * 5 = shadow type
 * 6 = shadow depth
 * 7 = shadow dir.
 * 8 = underline height
 * 9 = underline offset
 * <future extension>
 * 20 = plain/bold/italic/underlined (OBSOLETE)
 * 21 = text color (OBSOLETE)
 * 22 = shadow color
 *
 */

BOOL Monitor_StyleSelection(struct EditWindow *ew, BOOL *touchedList,
														struct EditWindow *dstEW, struct TEInfo *tei )
{
struct Window *window;
int ID, col, val;
BOOL retval, loop=TRUE;

	/**** open window ****/

	if ( UA_IsWindowOnLacedScreen(pageWindow) )
		SetFont(pageWindow->RPort, largeFont);

	window = UA_OpenRequesterWindow(pageWindow,Style_GR,STDCOLORS);
	if (!window)
	{
		UA_WarnUser(221);
		return(FALSE);
	}

	SetFont(pageWindow->RPort, smallFont);

	/**** init window and scroll bar ****/

	PropSlider1.LeftEdge	= Style_GR[14].x1+4;
	PropSlider1.TopEdge		= Style_GR[14].y1+2;
	PropSlider1.Width			= Style_GR[14].x2-Style_GR[14].x1-7;
	PropSlider1.Height		= Style_GR[14].y2-Style_GR[14].y1-3;

	if ( UA_IsWindowOnLacedScreen(pageWindow) )
	{
		PropSlider1.TopEdge	+= 2;
		PropSlider1.Height	-= 4;
	}

	UA_DrawGadgetList(window, Style_GR);

	InitPropInfo(	(struct PropInfo *)PropSlider1.SpecialInfo,
								(struct Image *)PropSlider1.GadgetRender);
	AddGadget(window, &PropSlider1, -1L);

	UA_SetPropSlider(window,&PropSlider1,10,10,0);	// filled drag bar

	/**** set cycle gadgets ****/

	UA_SetCycleGadgetToVal(window, &Style_GR[3], (int)ew->antiAliasLevel);
	UA_SetCycleGadgetToVal(window, &Style_GR[4], (int)ew->justification);
	UA_SetCycleGadgetToVal(window, &Style_GR[5], (int)ew->xSpacing+3);
	UA_SetCycleGadgetToVal(window, &Style_GR[6], (int)ew->ySpacing+3);

	val=-1;
	switch( ew->slantValue )
	{
		case -1:
			switch( ew->slantAmount )
			{
				case 1:
					val = 0;		// -45°
					break;
				case 2:
					val = 1;		// -27°
					break;
				case 4:
					val = 2;		// -14°
					break;
			}
			break;
		case 1:
			switch( ew->slantAmount )
			{
				case 4:
					val = 3;		// +14°
						break;
					case 2:
						val = 4;		// +27°
						break;
					case 1:
						val = 5;		// +45°
						break;
				}
				break;
	}
	if (val!=-1)
		UA_SetCycleGadgetToVal(window, &Style_GR[7], val);

	UA_SetCycleGadgetToVal(window, &Style_GR[8], (int)ew->shadowType);
	UA_SetCycleGadgetToVal(window, &Style_GR[9], (int)ew->shadowDepth);
	UA_SetCycleGadgetToVal(window, &Style_GR[10], (int)ew->shadowDirection);
	UA_SetCycleGadgetToVal(window, &Style_GR[11], (int)ew->underLineHeight-1);
	UA_SetCycleGadgetToVal(window, &Style_GR[12], (int)ew->underLineOffset+3);

	SetAttrib(4,ew,window);
	SetAttrib(8,ew,window);

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window, &CED);

		switch(CED.Class)
		{
			case IDCMP_MOUSEBUTTONS:
				if (CED.Code==SELECTDOWN)
				{
					ID = UA_CheckGadgetList(window, Style_GR, &CED);
					switch(ID)
					{
						case 3:
						case 4:
						case 5:
						case 6:
						case 7:
						case 8:
						case 9:
						case 10:
						case 11:
						case 12:
							UA_ProcessCycleGadget(window, &Style_GR[ID], &CED);
							SetAttrib(ID, ew, window);
							touchedList[ID-3] = TRUE;
							RealTimeUpdate(tei,ew,dstEW,touchedList);
							break;
						
						case 13:	// shadow color
							UA_HiliteButton(window, &Style_GR[ID]);
							UA_ResetMenuColors(&rvrec,pageWindow);
							col = OpenSmallPalette((int)ew->shadow_Pen,2,FALSE);
							if ( col != -1 )
								ew->shadow_Pen = (UBYTE)col;
							UA_SetMenuColors(&rvrec,pageWindow);
							if ( col != -1 )
							{
								touchedList[22]=TRUE;
								RealTimeUpdate(tei,ew,dstEW,touchedList);
							}
							break;

						case 15:	// OK
do_ok:
							UA_HiliteButton(window, &Style_GR[15]);
							retval=TRUE;
							loop=FALSE;
							break;

						case 16:	// Cancel
do_cancel:
							UA_HiliteButton(window, &Style_GR[16]);
							retval=FALSE;
							loop=FALSE;
							break;
					}
				}
				break;

			case IDCMP_RAWKEY:
				if (CED.Code==RAW_ESCAPE)	// cancel
					goto do_cancel;
				else if (CED.Code==RAW_RETURN) // OK
					goto do_ok;
				break;
		}
	}

	UA_CloseRequesterWindow(window,STDCOLORS);

	return(retval);
}

/******** SetAttrib() ********/

void SetAttrib(int ID, struct EditWindow *ew, struct Window *window)
{
int val,x,y,w,h;

	UA_SetValToCycleGadgetVal(&Style_GR[ID], &val);

	switch(ID)
	{
		case 3:	// Antialias level
			ew->antiAliasLevel = val;
			break;

		case 4:	// Justification
			ew->justification = val;
			w = GFX_JUST_W;
			if ( window->WScreen->ViewPort.Modes & LACE )
			{
				y = GFX_L_JUST_Y;
				h = GFX_L_JUST_H;
			}
			else
			{
				y = GFX_NL_JUST_Y;
				h = GFX_NL_JUST_H;
			}
			if ( val==0 )	x=GFX_JUST_L_X;
			if ( val==1 )	x=GFX_JUST_C_X;
			if ( val==2 )	x=GFX_JUST_R_X;
			PutImageInCycleGadget(window, &Style_GR[ID],x,y,w,h);
			break;

		case 5:	// Letter spacing
			ew->xSpacing = val-3;	// convert 0 to -3
			break;

		case 6:	// Line spacing
			ew->ySpacing = val-3;	// convert 0 to -3
			break;

		case 7:	// Slanting
			switch( val )
			{
				case 0:
					ew->slantAmount = 1;
					ew->slantValue = -1;
					break;
				case 1:
					ew->slantAmount = 2;
					ew->slantValue = -1;
					break;
				case 2:
					ew->slantAmount = 4;
					ew->slantValue = -1;
					break;
				case 3:
					ew->slantAmount = 4;
					ew->slantValue = 1;
					break;
				case 4:
					ew->slantAmount = 2;
					ew->slantValue = 1;
					break;
				case 5:
					ew->slantAmount = 1;
					ew->slantValue = 1;
					break;
			}
			break;

		case 8:	// Shadow type
			ew->shadowType = val;
			w = GFX_SHAD_W;
			if ( window->WScreen->ViewPort.Modes & LACE )
			{
				y = GFX_L_SHAD_Y;
				h = GFX_L_SHAD_H;
			}
			else
			{
				y = GFX_NL_SHAD_Y;
				h = GFX_NL_SHAD_H;
			}
			if ( val==0 )	x=GFX_SHAD_N_X;
			if ( val==1 )	x=GFX_SHAD_C_X;
			if ( val==2 )	x=GFX_SHAD_S_X;
			if ( val==3 )	x=GFX_SHAD_O_X;
			if ( val==4 )	x=GFX_SHAD_T_X;
			PutImageInCycleGadget(window, &Style_GR[ID],x,y,w,h);
			break;

		case 9:	// Shadow depth
			ew->shadowDepth = val;
			break;

		case 10:	// Shadow direction
			ew->shadowDirection = val;
			break;

		case 11:	// Underline height
			ew->underLineHeight = val+1;
			break;

		case 12:	// Underline offset
			ew->underLineOffset = val-3;	// convert 0 to -3
			break;
	}
}
#endif

/******** PutImageInCycleGadget() ********/

void PutImageInCycleGadget(	struct Window *window, struct GadgetRecord *GR,
														int x, int y, int w, int h )
{
int xx,yy;

	xx = ( GR->x2 - GR->x1 + 20 - w ) / 2;
	yy = 2;
	if ( window->WScreen->ViewPort.Modes & LACE )
		yy+=2;
	xx += GR->x1;
	yy += GR->y1;
	UA_ClearCycleButton(window, GR, AREA_PEN);
	SetDrMd(window->RPort,JAM1);
	PutImageInRastPort(x, y, window->RPort, xx, yy, w, h);
}

/******** PutImageInRastPort() ********/

void PutImageInRastPort(WORD srcX, WORD srcY, struct RastPort *dstRp,
												WORD dstX, WORD dstY, WORD dstW, WORD dstH)
{
	UA_PutImageInRastPort(&gfxBitMap,srcX,srcY,GFX_W,dstRp,dstX,dstY,dstW,dstH);

#if 0
int pen;

	SetAPen(dstRp,1L);	// black
	BltTemplate(	gfxBitMap.Planes[0]+(GFX_W/8)*srcY+(srcX/8), srcX%16, GFX_W/8,
								dstRp, dstX, dstY, dstW, dstH);

	if ( dstRp->BitMap->Depth==1 )
		pen=1;	
	else
		pen=2;	// white

	SetAPen(dstRp,pen);
	BltTemplate(	gfxBitMap.Planes[1]+(GFX_W/8)*srcY+(srcX/8), srcX%16, GFX_W/8,
								dstRp, dstX, dstY, dstW, dstH);
#endif

}

/******** E O F ********/
