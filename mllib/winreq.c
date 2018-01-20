#include "mllib_includes.h"

/**** static globals ****/

static int eightColors[3*8];
static int std_count=0;

/**** functions ****/

/*******************************************************************/
/*
 *   PUBLIC FUNCTIONS
 *
 *******************************************************************/

/**** functions ****/

/******** OpenRequesterWindow() ********/

struct Window * __saveds __asm LIBUA_OpenRequesterWindow(	register __a0	struct Window *onWindow,
																													register __a1 struct GadgetRecord *GR,
																													register __d0 BYTE palette)
{
struct NewWindow NewWindowStructure;
struct Window *window;
struct Screen *screen;
struct MsgPort *port;
struct List *list;
struct Node *node;
struct RendezVousRecord *rvrec;
int i,x,y;

	port = (struct MsgPort *)FindPort(ML_RENDEZ_VOUS);
	if (port == NULL)
		return(NULL);
	list = &(port->mp_MsgList);
	node = list->lh_Head;
	rvrec = (struct RendezVousRecord *)node->ln_Name;

	screen = onWindow->WScreen;

	ScaleGadgetList(screen,GR);

	/**** open the window ****/

	x = (screen->Width-GR->x2)/2;
	y = (screen->Height-GR->y2)/2;

	i=0;
	while( GR[i].x1 != -1 )
	{
		if ( GR[i].type==POSPREFS )
		{
			if ( GR[i].x2!=0 )
				x = GR[i].x2;
			if ( GR[i].y2!=0 )
				y = GR[i].y2;

			if ( GR[i].x2==1 || GR[i].x2==2 )	// position over mouse pointer
			{
				x = screen->MouseX - ( GR[0].x2 / 2 );
				if ( x < 0 )
					x=0;
				if ( x > ( screen->Width-GR[0].x2) )
					x = screen->Width-GR[0].x2;
			}

			if ( GR[i].y2==1 || GR[i].y2==2 )	// position over mouse pointer
			{
				y = screen->MouseY - ( GR[0].y2 / 2 );
				if ( y < 0 )
					y=0;
				if ( y > ( screen->Height-GR[0].y2) )
					y = screen->Height-GR[0].y2;
			}

			break;
		}
		i++;
	}

	if ( onWindow->TopEdge>=0 && y<=30 )
		y+=onWindow->TopEdge;

	NewWindowStructure.LeftEdge 		= x;	//(screen->Width-GR->x2)/2;
	NewWindowStructure.TopEdge 			= y;	//(screen->Height-GR->y2)/2;
	NewWindowStructure.Width 				= GR->x2;
	NewWindowStructure.Height 			= GR->y2;

	if ( screen->RastPort.BitMap->Depth==1 )
		NewWindowStructure.DetailPen	= 0;
	else
		NewWindowStructure.DetailPen	= 7;

	NewWindowStructure.BlockPen			= 1;
	NewWindowStructure.IDCMPFlags		= 0L;	/* else a msgport is created */
	NewWindowStructure.Flags 				= WFLG_ACTIVATE | WFLG_NOCAREREFRESH | WFLG_RMBTRAP |
																		WFLG_SMART_REFRESH | WFLG_BORDERLESS;
	NewWindowStructure.FirstGadget	= NULL;
	NewWindowStructure.CheckMark		= NULL;
	NewWindowStructure.Title				= NULL;
	NewWindowStructure.Screen 			= screen;
	NewWindowStructure.BitMap				= NULL;
	NewWindowStructure.MinWidth			= 0;
	NewWindowStructure.MinHeight		= 0;
	NewWindowStructure.MaxWidth			= 0;
	NewWindowStructure.MaxHeight		= 0;
	NewWindowStructure.Type					= CUSTOMSCREEN;

	window = (struct Window *)OpenWindow(&NewWindowStructure);
	if (window == NULL)
		return(NULL);

	/**** set the communications port up ****/

	window->UserPort = rvrec->capsport;
	ModifyIDCMP(window, IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY |
											IDCMP_GADGETDOWN | IDCMP_GADGETUP);	// | IDCMP_NEWSIZE);

	/**** modify the screen's palette ****/

	if ( palette == STDCOLORS )
		LIBUA_SetMenuColors(rvrec, window);

	if ( window->RPort->BitMap->Depth==1 )
		SetAPen(window->RPort, 0L);
	else
		SetAPen(window->RPort, AREA_PEN);

	/**** paint the walls ****/

	SetDrMd(window->RPort, JAM1);

	RectFill(	window->RPort,
						window->BorderLeft, window->BorderTop,
						window->Width  - window->BorderRight-1,
						window->Height - window->BorderBottom-1 );

	/**** set font ****/

	if ( screen->ViewPort.Modes & LACE )
		SetFont(window->RPort, rvrec->largefont);
	else
		SetFont(window->RPort, rvrec->smallfont);

	return(window);
}

/******** CloseRequesterWindow() ********/

void __saveds __asm LIBUA_CloseRequesterWindow(	register __a0 struct Window *window,
																								register __d0 BYTE palette )
{
struct MsgPort *port;
struct List *list;
struct Node *node;
struct RendezVousRecord *rvrec;

	if ( palette==STDCOLORS )
	{
		port = (struct MsgPort *)FindPort(ML_RENDEZ_VOUS);
		if (port == NULL)
			return;
		list = &(port->mp_MsgList);
		node = list->lh_Head;
		rvrec = (struct RendezVousRecord *)node->ln_Name;
		LIBUA_ResetMenuColors(rvrec,window);
	}

	LIBUA_CloseWindowSafely(window);
}

/******** GetNumberOfColorsInScreen() ********/

int __saveds __asm LIBUA_GetNumberOfColorsInScreen(	register __d0 ULONG viewmodes,
																										register __d1 int depth,
																										register __d2 BOOL AA_available )
{
int depthList[] = { 0, 2, 4, 8, 16, 32, 16, 16, 16 };
int depthList2[] = { 0, 2, 4, 8, 16, 32, 64, 128, 256 };

	if (depth>8)
		return(256);

	if ( AA_available )
	{
		if (depth==8)	// HAM8 mode
		{
			if (viewmodes & HAM)
				return(64);	// colormap size
			else
				return(256);
		}

		if (depth==6)
		{
			if (viewmodes & HAM)
				return(16);	// colormap size
			else if (viewmodes & EXTRA_HALFBRITE)
				return(32);	// colormap size
			else
				return(64);
		}

		return((int)depthList2[depth]);
	}
	else
	{
		if (viewmodes & HAM)
			return(16);	// colormap size

		if (viewmodes & EXTRA_HALFBRITE)
			return(32);	// colormap size

		return((int)depthList[depth]);
	}
}

/******** MyFindColor() ********/
/*
 * maxpen can be -1 --> search all
 *
 *
 */

LONG __saveds __asm LIBUA_MyFindColor(register __a0	struct ColorMap *cm,
																			register __d0 ULONG r,
																			register __d1 ULONG g,
																			register __d2 ULONG b,
																			register __d3 LONG maxpen,
																			register __d4	int count,
																			register __d5 BOOL colorZeroAlso )
{
int i,max;
LONG pen, rgb;
ULONG r1,g1,b1;
LONG rr,gg,bb, r2,g2,b2;
int r0,g0,b0,start;
struct MsgPort *port;
struct List *list;
struct Node *node;
struct RendezVousRecord *rvrec;

	port = (struct MsgPort *)FindPort(ML_RENDEZ_VOUS);
	if (port == NULL)
		return(0);
	list = &(port->mp_MsgList);
	node = list->lh_Head;
	rvrec = (struct RendezVousRecord *)node->ln_Name;

	if ( !colorZeroAlso )
	{
		start=1;
		GetColorComponents(cm, 0, &r0, &g0, &b0, rvrec);
		SetColorComponents(cm, 0, ~r, ~g, ~b, rvrec);
	}
	else
		start=0;

	if ( maxpen > count )	// kan niet
		max = count;
	else
		max = maxpen;

	if ( GfxBase->LibNode.lib_Version >= 39 )
	{
		pen = FindColor(cm,r,g,b,max);
		if ( !colorZeroAlso )
			SetColorComponents(cm, 0, r0, g0, b0, rvrec);
		return( pen );
	}
	else
	{
		if ( max==-1 )
			max = count;

		/**** one day we'll remove this, when everybody uses >39 ****/

		r &= 0x000000ff;
		g &= 0x000000ff;
		b &= 0x000000ff;

		pen = 0;
		r2 = 0xffffff;
		g2 = 0xffffff;
		b2 = 0xffffff;

		for(i=start; i<(max+1); i++)
		{
			rgb = GetColorCM32(cm,i);
			r1 = rgb & 0x00ff0000;
			r1 >>= 16;
			g1 = rgb & 0x0000ff00;
			g1 >>= 8;
			b1 = rgb & 0x000000ff;

			r1++;
			g1++;
			b1++;
 
			rr = r1 - r;
			gg = g1 - g;
			bb = b1 - b;

			if ( rr<0 ) rr *= -1;
			if ( gg<0 ) gg *= -1;
			if ( bb<0 ) bb *= -1;

			if ( rr==r && gg==g && bb==b )
			{
				pen = i;
				break;
			}

			if ( rr<=r2 && gg<=g2 && bb<=b2 )
			{
				pen = i;
				r2 = rr;
				g2 = gg;
				b2 = bb;
			}
		}

		if ( !colorZeroAlso )
			SetColorComponents(cm, 0, r0, g0, b0, rvrec);

		return( pen );
	}
}

/******** SetMenuColors() ********/

void __saveds __asm LIBUA_SetMenuColors(register __a0 struct RendezVousRecord *rvrec,
																				register __a1 struct Window *window)
{
struct ViewPort *vp;
struct ColorMap *cm;
UWORD r,g,b,i,c,w;
//TEXT str[50];

	if ( !window )
		return;

	std_count++;
	if ( std_count != 1 )
		return;

	vp = &(window->WScreen->ViewPort);
	cm = vp->ColorMap;

	// Get user colors 

	c=0;
	for(i=0; i<=7; i++)
	{
		GetColorComponents(cm, i, &eightColors[c], &eightColors[c+1], &eightColors[c+2], rvrec);
		c+=3;
	}

	// Set standard colors

	if ( window->RPort->BitMap->Depth==1 )	// color 0
	{
		w = rvrec->paletteList[rvrec->capsprefs->colorSet*8];
		r = (w & 0x0f00) >> 8;
		g = (w & 0x00f0) >> 4;
		b = (w & 0x000f);
		SetRGB4(vp, 0, r, g, b);
//sprintf(str,"s 0 %x %x %x\n",r,g,b);
//KPrintF(str);
	}

	// color 1

	w = rvrec->paletteList[1+rvrec->capsprefs->colorSet*8];
	r = (w & 0x0f00) >> 8;
	g = (w & 0x00f0) >> 4;
	b = (w & 0x000f);
	SetRGB4(vp, 1, r, g, b);

	if ( window->RPort->BitMap->Depth>=2 )	// color 2 and 3
	{
		for(i=2; i<=3; i++)
		{
			w = rvrec->paletteList[i+rvrec->capsprefs->colorSet*8];
			r = (w & 0x0f00) >> 8;
			g = (w & 0x00f0) >> 4;
			b = (w & 0x000f);
			SetRGB4(vp, i, r, g, b);
//sprintf(str,"s i %x %x %x\n",i,r,g,b);
//KPrintF(str);
		}
	}

	if ( window->RPort->BitMap->Depth>=3 )	// color 4,5,6 and 7
	{
		for(i=4; i<=7; i++)
		{
			w = rvrec->paletteList[i+rvrec->capsprefs->colorSet*8];
			r = (w & 0x0f00) >> 8;
			g = (w & 0x00f0) >> 4;
			b = (w & 0x000f);
			SetRGB4(vp, i, r, g, b);
//sprintf(str,"s i %x %x %x\n",i,r,g,b);
//KPrintF(str);
		}
	}
}

/******** ResetMenuColors() ********/

void __saveds __asm LIBUA_ResetMenuColors(register __a0 struct RendezVousRecord *rvrec,
																					register __a1 struct Window *window)
{
int i,c;
//TEXT str[50];

	if ( !window )
		return;

	std_count--;
	if ( std_count!=0 )
		return;

	if ( window->RPort->BitMap->Depth==1 )	// color 0
		SetVPToComponents(&(window->WScreen->ViewPort), 0, eightColors[0], eightColors[1], eightColors[2], rvrec);

//sprintf(str,"r 0 %x %x %x\n",eightColors[0], eightColors[1], eightColors[2]);
//KPrintF(str);

	// color 1

	SetVPToComponents(&(window->WScreen->ViewPort), 1, eightColors[3], eightColors[4], eightColors[5], rvrec);

//sprintf(str,"r 1 %x %x %x\n",eightColors[3], eightColors[4], eightColors[5]);
//KPrintF(str);

	c=6;
	if ( window->RPort->BitMap->Depth>=2 )	// color 2 and 3
	{
		for(i=2; i<=3; i++)
		{
			SetVPToComponents(&(window->WScreen->ViewPort), i, eightColors[c], eightColors[c+1], eightColors[c+2], rvrec);

//sprintf(str,"r %d %x %x %x\n",i,eightColors[c], eightColors[c+1], eightColors[c+2]);
//KPrintF(str);

			c=c+3;
		}
	}	

	if ( window->RPort->BitMap->Depth>=3 )	// color 4,5,6 and 7
	{
		for(i=4; i<=7; i++)
		{
			SetVPToComponents(&(window->WScreen->ViewPort), i, eightColors[c], eightColors[c+1], eightColors[c+2], rvrec);

//sprintf(str,"r %d %x %x %x\n",i,eightColors[c], eightColors[c+1], eightColors[c+2]);
//KPrintF(str);

			c=c+3;
		}
	}	
}

/*******************************************************************/
/*
 *   PRIVATE FUNCTIONS
 *
 *******************************************************************/

/******** ScaleGadgetList() ********/

void ScaleGadgetList(struct Screen *screen, struct GadgetRecord *GR)
{
	if ( GR->type != DIMENSIONS )
		return;

	if ( screen->ViewPort.Modes & LACE )
	{
		if ( GR->x1==0 )	// not doubled
		{
			LIBUA_DoubleGadgetDimensions(GR);
			GR->x1 = 1;
		}
	}
	else
	{
		if ( GR->x1==1 )	// doubled
		{
			LIBUA_HalveGadgetDimensions(GR);
			GR->x1 = 0;
		}
	}
}

/******** GetColorCM32() ********/

ULONG GetColorCM32(struct ColorMap *cm, int well)
{
ULONG table[10];
ULONG r, g, b, rgb;

	if ( GfxBase->LibNode.lib_Version >= 39 )	// really should be if aa_ ...
	{
		GetRGB32(cm, well, 2, &table[0]);	// OS function
		r = (table[0] & 0x000000ff) << 16;
		g = (table[1] & 0x000000ff) << 8;
	  b = (table[2] & 0x000000ff);
	}
	else
	{
		table[0] = GetRGB4(cm, well);
		r = (table[0] & 0x00000f00);	// r is now 0x00000r00
		r |= ( r << 4 );							// r is now 0x0000rr00
		r <<= 8;											// r is now 0x00rr0000
		g = (table[0] & 0x000000f0);	// g is now 0x000000g0
		g |= ( g << 4 );							// g is now 0x00000gg0
		g <<= 4;											// g is now 0x0000gg00
		b = (table[0] & 0x0000000f);	// b is now 0x0000000b
		b |= ( b << 4 );							// b is now 0x000000bb
	}
	rgb = r | g | b;

	return(rgb);
}

/******** SetVPToComponents() ********/

void SetVPToComponents(	struct ViewPort *vp, int well, int r, int g, int b,
												struct RendezVousRecord *rvrec)
{
ULONG gun;
ULONG rr,gg,bb;

	if ( rvrec->capsprefs->AA_available )
	{
		gun = r;
		rr = gun;									// rr is 0x000000ff
		rr |= (gun << 8);					// rr is 0x0000ffff
		rr |= (gun << 16);				// rr is 0x00ffffff
		rr |= (gun << 24);				// rr is 0xffffffff

		gun = g;
		gg = gun;
		gg |= (gun << 8);
		gg |= (gun << 16);
		gg |= (gun << 24);

		gun = b;
		bb = gun;
		bb |= (gun << 8);
		bb |= (gun << 16);
		bb |= (gun << 24);

		SetRGB32(vp, well, rr, gg, bb);	// OS function
	}
	else
		SetRGB4(vp, well, r, g, b);	// OS function
}

/******** GetColorComponents() ********/

void GetColorComponents(struct ColorMap *cm, int well, int *r, int *g, int *b,
												struct RendezVousRecord *rvrec)
{
ULONG table[10];
ULONG rr,gg,bb;

	if ( rvrec->capsprefs->AA_available )
	{
		GetRGB32(cm, well, 2, &table[0]);	// OS function

		rr = (table[0] & 0x000000ff);
		gg = (table[1] & 0x000000ff);
	  bb = (table[2] & 0x000000ff);
		*r = rr;
		*g = gg;
		*b = bb;
	}
	else
	{
		table[0] = GetRGB4(cm, well);

		/* table[0] is now 0x00000rgb */

		rr = (table[0] & 0x00000f00);
		rr >>= 8;
		gg = (table[0] & 0x000000f0);
		gg >>= 4;
		bb = (table[0] & 0x0000000f);

		*r = rr;
		*g = gg;
		*b = bb;
	}
}

/******** SetColorComponents() ********/

void SetColorComponents(struct ColorMap *cm, int well, int r, int g, int b,
												struct RendezVousRecord *rvrec)
{
ULONG gun;
ULONG rr,gg,bb;

	if ( rvrec->capsprefs->AA_available )
	{
		gun = r;
		rr = gun;									// rr is 0x000000ff
		rr |= (gun << 8);					// rr is 0x0000ffff
		rr |= (gun << 16);				// rr is 0x00ffffff
		rr |= (gun << 24);				// rr is 0xffffffff

		gun = g;
		gg = gun;
		gg |= (gun << 8);
		gg |= (gun << 16);
		gg |= (gun << 24);

		gun = b;
		bb = gun;
		bb |= (gun << 8);
		bb |= (gun << 16);
		bb |= (gun << 24);

		SetRGB32CM(cm, well, rr, gg, bb);	// OS function
	}
	else
		SetRGB4CM(cm, well, r, g, b);			// OS function
}

/******** E O F ********/
