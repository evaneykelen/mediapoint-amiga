/***************************************************/
/*	File				:Text.c									*/
/*																	*/
/*	Beschrijving	:Diverse procedures om in een		*/
/*						 intuition window te schrijven.	*/
/*																	*/
/* Datum				:13-Jul-92								*/
/***************************************************/

#include <intuition/intuition.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include "text.h"

#define	FONTHOOGTE	8

struct	IntuiText	MyIText;
ULONG		MyAPen,MyBPen;

struct	Border	ShineBorder;
struct	Border	ShadowBorder;

ULONG		MyShinePen;
ULONG		MyShadowPen;

WORD		BorderData[10];


VOID InitPens(VOID)
{
	struct	DrawInfo		*drawinfo;
	struct	Screen		*screen;

	if(screen=LockPubScreen(NULL))
	{
		if(drawinfo=GetScreenDrawInfo(screen))
		{
			MyAPen = drawinfo->dri_Pens[TEXTPEN];
			MyBPen = drawinfo->dri_Pens[BACKGROUNDPEN];
			MyShinePen = drawinfo->dri_Pens[SHINEPEN];
			MyShadowPen = drawinfo->dri_Pens[SHADOWPEN];
			FreeScreenDrawInfo(screen,drawinfo);
		}
	UnlockPubScreen(NULL,screen);
	}
	MyIText.FrontPen	= MyAPen;
	MyIText.BackPen	= MyBPen;
	MyIText.DrawMode	= JAM2;
	MyIText.LeftEdge	= 0;
	MyIText.TopEdge	= 0;
	MyIText.ITextFont	= NULL;
	MyIText.NextText	= NULL;

	ShadowBorder.LeftEdge	= 1;
	ShadowBorder.TopEdge		= 1;
	ShadowBorder.FrontPen	= MyShadowPen;
	ShadowBorder.NextBorder	= &ShineBorder;

	ShineBorder.LeftEdge		= 0;
	ShineBorder.TopEdge		= 0;
	ShineBorder.FrontPen		= MyShinePen;
	ShineBorder.NextBorder	= NULL;

	ShadowBorder.BackPen		= ShineBorder.BackPen	= 0;
	ShadowBorder.DrawMode	= ShineBorder.DrawMode	= JAM1;
	ShadowBorder.Count		= ShineBorder.Count		= 5;
	ShadowBorder.XY			= ShineBorder.XY			= BorderData;

}

VOID PrintText(struct Window *window, char *tekst, int x, int y)
{
	MyIText.IText		= tekst;
	PrintIText(window->RPort,&MyIText,x,y);
}

VOID TekenBorder(struct Window *window,int x, int y, int dx, int dy)
{
	BorderData[0]	= 0;
	BorderData[1]	= 0;
	BorderData[2]	= dx;
	BorderData[3]	= 0;
	BorderData[4]	= dx;
	BorderData[5]	= dy;
	BorderData[6]	= 0;
	BorderData[7]	= dy;
	BorderData[8]	= 0;
	BorderData[9]	= 0;

	DrawBorder(window->RPort, &ShadowBorder, x, y);
}

VOID ClearBox(struct Window *window,int x, int y, int dx, int dy)
{
	SetAPen(window->RPort, 0 );
	RectFill( window->RPort,x,y,x+dx,y+dy );
}

VOID SetBox(struct Window *window,int x, int y, int dx, int dy)
{
	SetAPen(window->RPort, 1 );
	RectFill( window->RPort,x,y,x+dx,y+dy );
}

VOID TextButton(struct Window *window, char *text,int x, int y, int dx, int dy)
{
	PrintText(window, text, x+4, y+(dy/2)-(FONTHOOGTE/2)+2);
	TekenBorder(window,x,y,dx,dy);
}

VOID TextButtons( struct Window *window, char *text[] , int vertical )
{
	int i = 3 * vertical;
	int x,y;

	x = TBS_XOFFSET;
	y = TBS_YOFFSET;

	SetAPen(window->RPort, 0 );
	RectFill( window->RPort,10,10,620,210);

	while( *text != NULL && i > 0 )
	{
		TextButton(window, *text, x, y, TBS_XSIZE, TBS_YSIZE);
		y += TBS_DY;
		if ( y > vertical * TBS_DY )
		{
			y = TBS_YOFFSET;
			x += TBS_DX;
		}
		*text++;
		i--;
	}
}

VOID TextKeyButtons( struct Window *window, char *text[] , int vertical )
{
	int i = 3 * vertical;
	int x,y;

	x = TBS_XOFFSET-25;
	y = TBS_YOFFSET;

	while( *text != NULL && i > 0 )
	{
		TextButton(window, *text, x, y, TBS_KEYXSIZE, TBS_YSIZE);
		y += TBS_DY;
		if ( y > vertical * TBS_DY )
		{
			y = TBS_YOFFSET;
			x += TBS_DX;
		}
		*text++;
		i--;
	}
}
