#include "nb:pre.h"

#define ICON_WIDTH			80	// see also globalallocs.c and filethumbs.c !!!!!!!!!!!!!!!!!!!
#define ICON_HEIGHT			80
#define ICON_DEPTH			4

/**** externals ****/

extern struct CapsPrefs CPrefs;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct Library *medialinkLibBase;
extern struct Screen *pageScreen;
extern UWORD IconColorMap[];
extern struct BitMap iconBM;
extern struct RastPort iconRP;
extern UBYTE **msgs;

/**** globals ****/

static struct Image iconImage = { 0,0, ICON_WIDTH,ICON_HEIGHT,ICON_DEPTH, NULL, 0x00f,0x000, NULL };

static struct DiskObject projIcon =
{
WB_DISKMAGIC, WB_DISKVERSION,
	{
	NULL,
	0,0,ICON_WIDTH,ICON_HEIGHT,
	GFLG_GADGIMAGE | GFLG_GADGHBOX,
	GACT_IMMEDIATE | GACT_RELVERIFY,
	GTYP_BOOLGADGET,
	(APTR)&iconImage,
	NULL,NULL,NULL,NULL,0,NULL
	},
WBPROJECT, ICON_PATH, NULL, NO_ICON_POSITION, NO_ICON_POSITION,	
NULL, NULL, 40000
};

/*----- bitmap : w = 32, h = 16 ------ */

UWORD chip script_icon_data[] =
{ 
	0x1fff,0xfff8,	/* ...**************************... */
	0x3800,0x001c,	/* ..***......................***.. */
	0x31ff,0xff8c,	/* ..**...******************...**.. */
	0x33f8,0x1fcc,	/* ..**..*******......*******..**.. */
	0x33f0,0x0fcc,	/* ..**..******........******..**.. */
	0x33f8,0x1fcc,	/* ..**..*******......*******..**.. */
	0x33ff,0xffcc,	/* ..**..********************..**.. */
	0x33c0,0x0fcc,	/* ..**..****..........******..**.. */
	0x33f0,0x0fcc,	/* ..**..******........******..**.. */
	0x33f0,0x0fcc,	/* ..**..******........******..**.. */
	0x33f0,0x0fcc,	/* ..**..******........******..**.. */
	0x33f0,0x0fcc,	/* ..**..******........******..**.. */
	0x33c0,0x03cc,	/* ..**..****............****..**.. */
	0x31ff,0xff8c,	/* ..**...******************...**.. */
	0x3800,0x001c,	/* ..***......................***.. */
	0x1fff,0xfff8,	/* ...**************************... */
 
	0x0000,0x0000,	/* ................................ */
	0x0000,0x0000,	/* ................................ */
	0x0000,0x0000,	/* ................................ */
	0x0007,0xe000,	/* .............******............. */
	0x000f,0xf000,	/* ............********............ */
	0x0007,0xe000,	/* .............******............. */
	0x0000,0x0000,	/* ................................ */
	0x003f,0xf000,	/* ..........**********............ */
	0x000f,0xf000,	/* ............********............ */
	0x000f,0xf000,	/* ............********............ */
	0x000f,0xf000,	/* ............********............ */
	0x000f,0xf000,	/* ............********............ */
	0x003f,0xfc00,	/* ..........************.......... */
	0x0000,0x0000,	/* ................................ */
	0x0000,0x0000,	/* ................................ */
	0x0000,0x0000,	/* ................................ */
};

static struct Image iconImage2 = { 0,0, 32,16,2, &script_icon_data[0], 0x00f,0x000, NULL };

static struct DiskObject scriptIcon =
{
WB_DISKMAGIC, WB_DISKVERSION,
	{
	NULL,
	0,0,32,16,
	GFLG_GADGIMAGE | GFLG_GADGHBOX,
	GACT_IMMEDIATE | GACT_RELVERIFY,
	GTYP_BOOLGADGET,
	(APTR)&iconImage2,
	NULL,NULL,NULL,NULL,0,NULL
	},
WBPROJECT, ICON_PATH2, NULL, NO_ICON_POSITION, NO_ICON_POSITION,	
NULL, NULL, 40000
};

/**** functions ****/

/******** SaveIcon() ********/

void SaveIcon(STRPTR fullPath)
{
int i,size;
struct ColorMap *cm;
struct ScaleRemapInfo SRI;
struct BitMap bm;

	if ( CPrefs.fastIcons )
	{
		SaveScriptIcon(fullPath);
		return;
	}

	InvalidateTable();

	iconImage.ImageData = (UWORD *)iconBM.Planes[0];
	size = RASSIZE(ICON_WIDTH,ICON_HEIGHT)*ICON_DEPTH;
	// clear 'planes'
	for(i=0; i<size; i++)
		*(iconBM.Planes[0]+i) = 0;

	// create cm

	cm = GetColorMap(16);
	if ( cm==NULL )
		return;

	for(i=0; i<16; i++)
		SetColorCM4(cm, IconColorMap[i], i);

	// create bitmap

	InitBitMap(&bm, ICON_DEPTH, ICON_WIDTH, ICON_HEIGHT);
	for(i=0; i<ICON_DEPTH; i++)
		bm.Planes[i] = iconBM.Planes[i];

	SRI.SrcBitMap					= (struct BitMap24 *)pageScreen->RastPort.BitMap;
	SRI.DstBitMap					= (struct BitMap24 *)&bm;
	SRI.SrcViewModes  		= CPrefs.PageScreenModes;
	SRI.DstViewModes  		= HIRES;
	SRI.SrcColorMap				= CPrefs.PageCM;
	SRI.DstColorMap				= cm;
	SRI.SrcX							= 0;
	SRI.SrcY							= 0;
	SRI.SrcWidth					= pageScreen->Width;
	SRI.SrcHeight					= pageScreen->Height;
	SRI.XSrcFactor				= pageScreen->Width;
	SRI.YSrcFactor				= pageScreen->Height;
	SRI.DestX							= 0;
	SRI.DestY							= 0;
	SRI.DestWidth					= ICON_WIDTH;
	SRI.DestHeight				= ICON_HEIGHT;
	SRI.XDestFactor				= ICON_WIDTH;
	SRI.YDestFactor				= ICON_HEIGHT;
	SRI.Flags							= 0;	//SCAREMF_OPAQUE;
	SRI.DitherMode				= DITHER_OFF;	//FLOYD;
	SRI.DstMaskPlane			= NULL;
	SRI.TransparentColor	= 0;

	if ( !ScaleRemap(&SRI) )
		Message("ScaleRemap Failed\n");

	FreeColorMap(cm);

	if ( PutDiskObject(fullPath, &projIcon) == 0 )
		Message(msgs[Msg_UnableToWriteIcon-1]);

	InvalidateTable();
}

/******** SaveScriptIcon() ********/

void SaveScriptIcon(STRPTR fullPath)
{
	if ( PutDiskObject(fullPath, &scriptIcon) == 0 )
		Message(msgs[Msg_UnableToWriteIcon-1]);
}

/******** E O F ********/
