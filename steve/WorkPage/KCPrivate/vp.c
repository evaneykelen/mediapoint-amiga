/**** TEST VIEW(PORT) MODE PROMOTION UNDER >=36 ****/

#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <graphics/copper.h>
#include <graphics/view.h>
#include <graphics/displayinfo.h>
#include <graphics/gfxnodes.h>
#include <graphics/videocontrol.h>
#include <libraries/dos.h>
#include <utility/tagitem.h>

#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <stdio.h>
#include <stdlib.h>

#define WW 640
#define HH 512
#define DD 8
#define CC 256

struct POINT
{
	int x,y;
};

void draw_lines( struct RastPort *rp, struct POINT *points, int N );

struct GfxBase *GfxBase = NULL;

struct View view, *oldview = NULL;
struct ViewPort viewPort = { 0 };
struct BitMap bitMap = { 0 };
struct ColorMap *cm = NULL;

struct ViewExtra *vextra = NULL;
struct MonitorSpec *monspec = NULL;
struct ViewPortExtra *vpextra = NULL;
struct DimensionInfo dimquery = { 0 };

void main( int argc, char *argv[] )
{
	int N;
	struct POINT points[50];

struct RasInfo rasInfo;
ULONG modeID;
struct TagItem vcTags[] =
{
	{ VTAG_VIEWPORTEXTRA_SET, NULL	},
	{ VTAG_NORMAL_DISP_SET, NULL		},
	{ VTAG_END_CM, NULL							}
};
int i;
struct RastPort rastport;

	if( argc >= 2 )
		sscanf(argv[1],"%d",&N);
	printf("Splining with value %d\n",N );
	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0L);
	if (GfxBase==NULL)
		exit(0);

	oldview = GfxBase->ActiView;

	InitView(&view);
	view.Modes |= LACE;	// old 1.3 way (only lace counts)

	if ( GfxBase->LibNode.lib_Version >= 36 )
	{
		modeID = 0x89004L; //SUPER72_MONITOR_ID|HIRES|LACE|SUPER_KEY;

		if ( vextra=GfxNew(VIEW_EXTRA_TYPE) )
		{
			view.DxOffset = oldview->DxOffset;
			view.DyOffset = oldview->DyOffset;

			GfxAssociate(&view, vextra);
			view.Modes |= EXTEND_VSTRUCT;

			if ( monspec=OpenMonitor(NULL,modeID) )
				vextra->Monitor = monspec;	
			else
				printf("no monspec");
		}
		else
			printf("no gfxnew");
	}

	InitBitMap(&bitMap,DD,WW,HH);		

	for(i=0; i<8; i++)
		bitMap.Planes[i] = NULL;

	for(i=0; i<DD; i++)
		bitMap.Planes[i] = AllocRaster(WW,HH);

	InitRastPort(&rastport);
	rastport.BitMap = &bitMap;

	rasInfo.BitMap		=	&bitMap;
	rasInfo.RxOffset	= 0;			
	rasInfo.RyOffset	= 0;			
	rasInfo.Next			= NULL;

	InitVPort(&viewPort);
	view.ViewPort			= &viewPort;
	viewPort.RasInfo	= &rasInfo;			
	viewPort.DWidth		= WW;
	viewPort.DHeight	= HH;

	viewPort.Modes		= HIRES | LACE;		// old-fashioned way

	if ( GfxBase->LibNode.lib_Version >= 36 )
	{
		if ( vpextra=GfxNew(VIEWPORT_EXTRA_TYPE) )
		{
			vcTags[0].ti_Data = (ULONG)vpextra;
			if ( !(vcTags[1].ti_Data = (ULONG)FindDisplayInfo(modeID)) )
				printf("no finddisplayinfo");
		}
		else
			printf("no gfxnew");
	}

	cm = GetColorMap(CC);
	if (cm==NULL)
		printf("no cm");

	if ( GfxBase->LibNode.lib_Version >= 36 )
	{
		if ( VideoControl(cm,vcTags))
			printf("videocontrol failed");
	}
	viewPort.ColorMap = cm;

	MakeVPort(&view,&viewPort);
	MrgCop(&view);
	Move(&rastport,0,0);
	ClearScreen( &rastport );
	LoadView(&view);

	points[0].x = 20;
	points[0].y = 20;
	points[1].x = 300;
	points[1].y = 10;
	points[2].x = 300;
	points[2].y = 200;
	points[3].x = -1000;
	points[3].y = -1000;

	for( i = 20; i < 200; i+=10 )
	{
		draw_lines( &rastport, points,N );
		points[ 0 ].y = i;
		Move(&rastport,0,0);
		ClearScreen( &rastport );
	}

	Delay(5*TICKS_PER_SECOND);

	LoadView(oldview);
	WaitTOF();
	FreeCprList(view.LOFCprList);
	if (view.SHFCprList)
		FreeCprList(view.SHFCprList);
	FreeVPortCopLists(&viewPort);
	if (cm)
		FreeColorMap(cm);
	if (vpextra)
		GfxFree(vpextra);

	for(i=0; i<DD; i++)
		if ( bitMap.Planes[i] )
			FreeRaster(bitMap.Planes[i],WW,HH);

	if (monspec)
		CloseMonitor(monspec);
	if (vextra)
		GfxFree(vextra);

	CloseLibrary((struct Library *)GfxBase);

	exit(0);
}
		
/******** E O F ********/
