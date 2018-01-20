#include "nb:pre.h"

#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>
#include <datatypes/animationclass.h>
#include <intuition/icclass.h>
#include <libraries/iffparse.h>

#include <clib/datatypes_protos.h>
#include <clib/utility_protos.h>
#include <clib/iffparse_protos.h>

#include <pragmas/datatypes_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

/**** defines ****/

#define DT_PICTURE		1
#define DT_TEXT				2
#define DT_ANIMATION	3
#define DT_MOVIE			4

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Screen *pageScreen;
extern struct Library *medialinkLibBase;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct EditSupport **Clipboard_SL;
extern struct EditSupport **Undo_SL;
extern struct Screen **DA_Screens;
extern struct TextFont *largeFont;
extern struct TextFont *textFont;
extern UBYTE **msgs;   
extern struct TextFont *smallFont;
extern struct TextFont *tiny_smallFont;

/**** globals ****/

struct Library *UtilityBase;
struct Library *DataTypesBase;
struct Library *IFFParseBase;

static Object *dto = NULL;
static struct Window *win = NULL;
static struct dtFrameBox dtf;
static struct FrameInfo fri;
static struct BitMap DT_BM;

#define	IDCMP_FLAGS	( IDCMP_VANILLAKEY | IDCMP_IDCMPUPDATE | IDCMP_MOUSEBUTTONS )

/**** gadgets ****/

extern struct GadgetRecord ImportType_GR[];
extern struct GadgetRecord ScreenThumbs_GR[];

/**** functions ****/

/******** ImportADataType() ********/

void ImportADataType(BOOL resizeIt, int remapIt)
{
int numColors, i;
TEXT filename[SIZE_FILENAME], fullPath[SIZE_FULLPATH];
BOOL retval;
struct Screen tempScreen;
ULONG r,g,b;
struct ColorRegister *cregs;

	dto = NULL;
	win = NULL;

	for(i=0; i<8; i++)
		DT_BM.Planes[i] = 0;

	/**** show file requester ****/

	retval = OpenAFile(	CPrefs.import_picture_Path, filename,
											msgs[Msg_SelectPics-1], pageWindow,
											DIR_OPT_ALL | DIR_OPT_NOINFO, FALSE);
	if (!retval)
		return;

	UA_MakeFullPath(CPrefs.import_picture_Path, filename, fullPath);

	if ( !ShowDataType(fullPath,&tempScreen) )
	{
		CloseDataTypeWindow();
		return;
	}

	/**** steal colormap ****/

	numColors = UA_GetNumberOfColorsInScreen(	tempScreen.ViewPort.Modes,
																						DT_BM.Depth, CPrefs.AA_available );

	tempScreen.ViewPort.ColorMap = GetColorMap(numColors);

	if (tempScreen.ViewPort.ColorMap==NULL)
	{
		CloseDataTypeWindow();
		return;
	}

	if ( GetDTAttrs(dto, PDTA_CRegs, &cregs, TAG_DONE) )
	{
    for (i=0; i<numColors; i++)
    {
			/**** rgb MUST be 0x00rrggbb ****/
			r = (ULONG)cregs[i*4].red;			// 0x000000r0
			r |= (r>>4);										// 0x000000rr
			r = r << 16;										// 0x00rr0000
			r &= 0x00ff0000;

			g = (ULONG)cregs[i*4+1].green;	// 0x000000g0
			g |= (g>>4);										// 0x000000gg
			g = g << 8;											// 0x0000gg00
			g &= 0x0000ff00;

			b = cregs[i*4+2].blue;					// 0x000000b0
			b |= (b>>4);										// 0x000000bb

			SetColorCM32(tempScreen.ViewPort.ColorMap, r|g|b, i);
		}
	}

	/**** copy DT_BM into embedded bitmap structure of Screen ****/

	CopyMem(&DT_BM, &(tempScreen.BitMap), sizeof(struct BitMap));
	tempScreen.RastPort.BitMap = &DT_BM;

	CloseDataTypeWindow();

	/* tempScreen must contain:
			- Width
			- Height
			- BitMap (complete with pointers, Depth etc.)
			- ViewPort.Modes
			- ViewPort.ColorMap
	 */			

/*
PrintSer("numColors=%d\n",numColors);
PrintSer("w=%d\n",tempScreen.Width);
PrintSer("h=%d\n",tempScreen.Height);
PrintSer("m=%d\n",tempScreen.ViewPort.Modes);
*/

	if ( DT_BM.Planes[0] )
	{
		ImportAScreen(resizeIt, remapIt, &tempScreen);
	}

	WaitBlit();

	for(i=0; i<8; i++)
		if ( DT_BM.Planes[i] )
			FreeRaster(DT_BM.Planes[i], DT_BM.BytesPerRow*8, DT_BM.Rows);

	FreeColorMap(tempScreen.ViewPort.ColorMap);
}

/******** ShowDataType() ********/

BOOL ShowDataType(STRPTR name, struct Screen *tempScreen)
{
ULONG modeid = INVALID_ID;
ULONG nomwidth, nomheight;
BOOL useScreen = FALSE;
BOOL going = TRUE, retval=TRUE;
struct IntuiMessage *imsg;
ULONG sigr;
struct TagItem *tstate, *tag;
struct TagItem *tags;
ULONG tidata,errnum;
struct BitMapHeader *BMH, tempBMH;
int ID, i, gadg;
ULONG ww,hh,dd;
struct BitMap *stealBM;
LONG result = 0;
struct RastPort tempRP;
//WORD bmw, bmh;

	// Open libraries

	DataTypesBase = OpenLibrary ("datatypes.library", 39);
	if ( DataTypesBase==NULL )
		return(FALSE);

  UtilityBase = OpenLibrary ("utility.library", 39);
	if ( UtilityBase==NULL )
		return(FALSE);

	IFFParseBase = OpenLibrary ("iffparse.library", 39);
	if ( IFFParseBase==NULL )
		return(FALSE);

	// Check if this is loadable data

	ID = CheckDataType(name);	// see function belows
	if ( ID==-1 )
	{
    CloseLibrary(IFFParseBase);
    CloseLibrary(UtilityBase);
    CloseLibrary(DataTypesBase);
		return(FALSE);
	}

	if ( dto = NewDTObject(name,TAG_DONE) )
	{
		if ( ID==DT_PICTURE || ID==DT_ANIMATION || ID==DT_MOVIE )
		{
			if (!GetDTAttrs (	dto,
												/* Get the desired size */
												DTA_NominalHoriz,	&nomwidth, DTA_NominalVert,	&nomheight,
												/* Get the mode ID for graphical objects */
												PDTA_ModeID, &modeid,
												TAG_DONE))
			{
		    CloseLibrary(IFFParseBase);
		    CloseLibrary(UtilityBase);
		    CloseLibrary(DataTypesBase);
				return(FALSE);
			}

			// Store mode ID of screen

			tempScreen->ViewPort.Modes = modeid & 0x0000ffff;

			memset(&dtf, 0, sizeof(struct dtFrameBox));
			memset(&fri, 0, sizeof(struct FrameInfo));

			dtf.MethodID					= DTM_FRAMEBOX;
			dtf.dtf_FrameInfo			= &fri;
			dtf.dtf_ContentsInfo	= &fri;
			dtf.dtf_SizeFrameInfo	= sizeof(struct FrameInfo);

			if (DoDTMethodA (dto, NULL, NULL, (Msg)&dtf) && fri.fri_Dimensions.Depth)
			{
		    if ((fri.fri_PropertyFlags == 0) && (modeid & 0x800) && (modeid != INVALID_ID))
					useScreen = TRUE;
			}

//PrintSer("depth %d\n",fri.fri_Dimensions.Depth);
//PrintSer("use screen %d\n",useScreen);

			nomwidth  = ((nomwidth) ? nomwidth : CPrefs.PageScreenWidth-20);
			nomheight = ((nomheight) ? nomheight : CPrefs.PageScreenHeight-20);
			if ( nomwidth < 200 )
				nomwidth = 200;

	    // Open the window

	    if (win = OpenWindowTags(	NULL,
												      	WA_InnerWidth,		nomwidth,
													      WA_InnerHeight,		nomheight,
													      WA_IDCMP,					IDCMP_FLAGS,
													      WA_DragBar,				FALSE,
													      WA_CloseGadget,		TRUE,
													      WA_SimpleRefresh,	TRUE,
													      WA_Activate,			TRUE,
																WA_CustomScreen,	pageScreen,
													      WA_BusyPointer,		TRUE,
													      TAG_DONE))
	    {
				// Set the dimensions of the DataType object
				SetDTAttrs (dto, NULL, NULL,
								    GA_Left,		win->BorderLeft,
								    GA_Top,			win->BorderTop,
								    GA_Width,		win->Width - win->BorderLeft - win->BorderRight,
								    GA_Height,	win->Height - win->BorderTop - win->BorderBottom,
								    ICA_TARGET,	ICTARGET_IDCMP,
										PDTA_Remap, FALSE,
							  	  TAG_DONE);

				// Add the object to the window
				AddDTObject (win, NULL, dto, -1);

				// Refresh the DataType object
				RefreshDTObjects (dto, win, NULL, NULL);

				// Render gadgets
				SetFont(win->RPort, tiny_smallFont);
				ScreenThumbs_GR[0].y1 = 0;
				ScreenThumbs_GR[0].y2 = 13;
				ScreenThumbs_GR[1].y1 = 0;
				ScreenThumbs_GR[1].y2 = 13;
				ScreenThumbs_GR[0].x1 -= 5;
				ScreenThumbs_GR[0].x2 -= 5;
				UA_DrawGadgetList(win, ScreenThumbs_GR);

				// Keep going until we're told to stop

				while (going)
				{
	    		sigr = Wait((1L << win->UserPort->mp_SigBit));
			    while( imsg = (struct IntuiMessage *)GetMsg (win->UserPort) )
			    {
						switch (imsg->Class)
						{
							case IDCMP_MOUSEBUTTONS:
								CED.MouseX = imsg->MouseX;
								CED.MouseY = imsg->MouseY;
								UA_DrawGadgetList(win, ScreenThumbs_GR);
								gadg = UA_CheckGadgetList(win, ScreenThumbs_GR, &CED);
								if (gadg==0||gadg==1)
									UA_HiliteButton(win, &ScreenThumbs_GR[gadg]);
								if (gadg==0)	// cancel
								{
									going=FALSE;
									retval=FALSE;
								}
								if (gadg==1)	// Grab
								{
									going=FALSE;
									retval=TRUE;
								}
								break;

/*
					    case IDCMP_CLOSEWINDOW:
								going = FALSE;
								retval=TRUE;
							break;
*/

					    case IDCMP_IDCMPUPDATE:
								tstate = tags = (struct TagItem *) imsg->IAddress;
								while (tag = NextTagItem (&tstate))
								{
							    tidata = tag->ti_Data;
							    switch (tag->ti_Tag)
							    {
										/* Change in busy state */
										case DTA_Busy:
									    if (tidata)
											{
												SetWindowPointer (win, WA_BusyPointer, TRUE, TAG_DONE);
												UA_DrawGadgetList(win, ScreenThumbs_GR);
											}
									    else
												SetWindowPointer (win, WA_Pointer, NULL, TAG_DONE);
									    break;

										/* Error message */
										case DTA_ErrorLevel:
									    if (tidata)
												errnum = GetTagData (DTA_ErrorNumber, NULL, tags);
									    break;

										/* Time to refresh */
										case DTA_Sync:
									    /* Refresh the DataType object */
									    RefreshDTObjects (dto, win, NULL, NULL);
											UA_DrawGadgetList(win, ScreenThumbs_GR);
								  	  break;
							    }
								}
								break;
						}
						/* Done with the message, so reply to it */
						ReplyMsg ((struct Message *)imsg);
		    	}
				}

		    RefreshDTObjects (dto, win, NULL, NULL);

				// Selection made

//PrintSer("nomwidth=%d nomheight=%d\n",nomwidth,nomheight);
//PrintSer("%d %d %d %d\n", win->BorderLeft, win->BorderTop, win->BorderRight, win->BorderBottom);

				if ( ID==DT_PICTURE )
				{
					result = GetDTAttrs(dto, PDTA_BitMap, &stealBM, PDTA_BitMapHeader, &BMH, TAG_DONE);
					CopyMem(BMH,&tempBMH,sizeof(struct BitMapHeader));
				}

				if ( ID==DT_ANIMATION || ID==DT_MOVIE )
				{
					result = GetDTAttrs(dto, ADTA_Width, &ww, ADTA_Height, &hh, ADTA_Depth, &dd, TAG_DONE);
					tempBMH.bmh_Width		= ww;
					tempBMH.bmh_Height	= hh;
					tempBMH.bmh_Depth		= dd;
				}

				if ( result>0 )
				{
//PrintSer("%d %d %d\n",tempBMH.bmh_Width,tempBMH.bmh_Height,tempBMH.bmh_Depth);

					tempScreen->Width  = (win->Width - win->BorderLeft - win->BorderRight);	//tempBMH.bmh_Width;
					tempScreen->Height = (win->Height - win->BorderBottom - win->BorderTop);// - 12)-1;	//tempBMH.bmh_Height;

					if ( tempBMH.bmh_Width < tempScreen->Width )
						tempScreen->Width = tempBMH.bmh_Width;

					if ( tempBMH.bmh_Height < tempScreen->Height )
						tempScreen->Height = tempBMH.bmh_Height;

					InitBitMap(&DT_BM, tempBMH.bmh_Depth, tempScreen->Width, tempScreen->Height);

					for(i=0; i<tempBMH.bmh_Depth; i++)
					{
						DT_BM.Planes[i] = AllocRaster(tempScreen->Width, tempScreen->Height);
						if ( DT_BM.Planes[i] == NULL )
						{
							for(i=0; i<8; i++)
								if ( DT_BM.Planes[i] != NULL )
									FreeRaster(DT_BM.Planes[i], tempScreen->Width, tempScreen->Height);
							CloseLibrary(IFFParseBase);
							CloseLibrary(UtilityBase);
							CloseLibrary(DataTypesBase);
							ScreenThumbs_GR[0].x1 += 5;
							ScreenThumbs_GR[0].x2 += 5;
							return(FALSE);
						}
					}

					if ( ID==DT_PICTURE )
					{
						BltBitMap(stealBM,0,0,&DT_BM,0,0,tempScreen->Width,tempScreen->Height,0xc0,0xff,NULL);
						WaitBlit();
					}
					else if ( ID==DT_ANIMATION || ID==DT_MOVIE )
					{
						InitRastPort( &tempRP );
						tempRP.BitMap = &DT_BM;
						Move(&tempRP,0,0); SetRast(&tempRP,0); WaitBlit();
						ClipBlit(	win->RPort,
											win->BorderLeft, win->BorderTop,&tempRP,
											0, 0, tempScreen->Width, tempScreen->Height-16,
											0xc0);
						WaitBlit();
					}
				}
			}
    }
	}
	else
		retval = FALSE;

	// Close the libraries

	CloseLibrary(IFFParseBase);
	CloseLibrary(UtilityBase);
	CloseLibrary(DataTypesBase);

	ScreenThumbs_GR[0].x1 += 5;
	ScreenThumbs_GR[0].x2 += 5;

	return( retval );
}

/******** CloseDataTypeWindow() ********/

void CloseDataTypeWindow(void)
{
	/* Remove the object from the window */
	if (win && dto)
		RemoveDTObject(win, dto);

	/* Close the window now */
	if (win)
		CloseWindow (win);

	/* Dispose of the DataType object */
	if (dto)
		DisposeDTObject(dto);
}

/******** CheckDataType() ********/

int CheckDataType(STRPTR name)
{
struct FileLock *lock;
struct DataType *dtn;
struct DataTypeHeader *dth;
TEXT group[40];
int ID=-1;

	if (lock = (struct FileLock *)Lock(name, ACCESS_READ))
	{
		if (dtn = ObtainDataTypeA (DTST_FILE, (APTR)lock, NULL))
		{
			dth = dtn->dtn_Header;
/***********
			PrintSer("information on: %s\n", name);
			PrintSer("   Description: %s\n", dth->dth_Name);
			PrintSer("     Base Name: %s\n", dth->dth_BaseName);
			PrintSer("          Type: %s\n", GetDTString ((dth->dth_Flags & DTF_TYPE_MASK) + DTMSG_TYPE_OFFSET));
			PrintSer("         Group: %s\n", GetDTString (dth->dth_GroupID));
			//PrintSer("            ID: %s\n\n", IDtoStr (dth->dth_ID, buffer));
************/
			stccpy(group, GetDTString(dth->dth_GroupID), 39);
			ReleaseDataType(dtn);
			if ( !stricmp(group,"Picture") )
				ID = DT_PICTURE;
			else if ( !stricmp(group,"Text") )
				ID = DT_TEXT;
			else if ( !stricmp(group,"Animation") )
				ID = DT_ANIMATION;
			else if ( !stricmp(group,"Movie") )
				ID = DT_MOVIE;
		}
		UnLock((BPTR)lock);
	}

	return(ID);
}

#if 0
{
    UWORD	 bmh_Width;		/* Width in pixels */
    UWORD	 bmh_Height;		/* Height in pixels */
    WORD	 bmh_Left;		/* Left position */
    WORD	 bmh_Top;		/* Top position */
    UBYTE	 bmh_Depth;		/* Number of planes */
    UBYTE	 bmh_Masking;		/* Masking type */
    UBYTE	 bmh_Compression;	/* Compression type */
    UBYTE	 bmh_Pad;
    UWORD	 bmh_Transparent;	/* Transparent color */
    UBYTE	 bmh_XAspect;
    UBYTE	 bmh_YAspect;
    WORD	 bmh_PageWidth;
    WORD	 bmh_PageHeight;
};
#endif

/******** E O F ********/
