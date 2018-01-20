#include "nb:pre.h"

/**** defines ****/

#define FAST_MENU_WIDTH		27	// see also initmenus.c
#define FAST_MENU_HEIGHT	74	// see also initmenus.c
#define MENUWIDTH 180					// see also initmenus.c  and  globalallocs.c

/**** externals ****/

extern struct CapsPrefs CPrefs;
extern struct EventData CED;
extern struct eventHandlerInfo EHI;
extern struct MenuRecord **page_MR;
extern struct MenuRecord **script_MR;
extern struct MsgPort *capsPort;
extern struct Window *pageWindow;
extern struct Window *scriptWindow;
extern struct Screen *pageScreen;
extern struct Screen *scriptScreen;
extern struct BitMap mbarBM;
extern struct RastPort mbarRP;
extern TEXT pageScreenTitle[255];
extern TEXT scriptScreenTitle[255];
extern UWORD palettes[];
extern UWORD chip gui_pattern[];
extern struct Library *medialinkLibBase;
extern UBYTE **msgs;   
extern BOOL blockScript;
extern struct MenuRecord fast_script_MR;
extern struct MenuRecord fast_page_MR;

/**** statics ****/

static int oldmenu=-1;
static int olditem=-1;
static int oldfast=-1;
static WORD rx=-1,ry,rw,rh;

/**** functions ****/

/******** Monitor_Menu() ********/

void Monitor_Menu(struct Window *window, int *menu, int *item, struct MenuRecord **MR)
{
//struct Screen *screen;
ULONG signals;
BOOL loop=TRUE, mouseMoved;	//, colorsSet=FALSE;
struct Window *activeWindow=NULL;
struct IntuiMessage *message;
int prev_menuID, menuID, itemID, i, prev_fastID, fastID;
struct RastPort *rp;
struct MenuRecord *fastMR=NULL;

	/**** init vars ****/

	oldmenu = -1;
	olditem = -1;
	oldfast = -1;
	prev_menuID = -1;
	prev_fastID = -1;
	menuID = -1;
	itemID = -1;
	fastID = -1;
	rp = window->RPort;
	SetDrMd(rp,JAM1);

	if ( window==scriptWindow )
		fastMR = &fast_script_MR;
	else
		fastMR = &fast_page_MR;

	/**** get currently active window and activate page or script window ****/

	Forbid();
	activeWindow = IntuitionBase->ActiveWindow;
	Permit();
	ActivateWindow(window);

	/**** remove parts of a foreign window which can fuck up our screen under some	****/
	/**** buggy OS versions.																												****/

	if ( window==scriptWindow )
	{
		if ( scriptWindow->TopEdge >= 1 )
		{
			SetAPen(&(scriptScreen->RastPort),0L);
			RectFill(&(scriptScreen->RastPort),0,0,scriptScreen->Width-1,scriptWindow->TopEdge-1);
		}
	}
	else if ( window==pageWindow )
	{
		if ( pageWindow->TopEdge >= 1 )
		{
			SetAPen(&(pageScreen->RastPort),0L);
			RectFill(&(pageScreen->RastPort),0,0,pageScreen->Width-1,pageWindow->TopEdge-1);
		}
	}

	/**** render the menu bar (bar or fast menu) ****/

	RenderMenuBar(rp,window,MR,CPrefs.fastMenus);

	/**** check which menu the user wants to see ****/

	if ( CPrefs.fastMenus )
		menuID = CheckFastMenu(window, fastMR, MR, rp);
	else
	{
		menuID = CheckWhichMenu(window, MR, NUMMENUS, rp);
		if (menuID != -1)
		{
			if (menuID==DA_MENU)
				Fill_MRO_Menu(EHI.activeScreen);
			if (menuID==SCREEN_MENU)
				Fill_DA_Menu(TRUE);

			if ( CPrefs.fastMenus )
				RenderMenu(window, MR[menuID], rx+FAST_MENU_WIDTH, ry);
			else
				RenderMenu(window, MR[menuID], MR[menuID]->x, MR[menuID]->y);
		}
		prev_menuID = menuID;
	}

	/**** switch MOUSEMOVE on ****/

	UA_SwitchMouseMoveOn(window);

	/**** handle events ****/

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
				CED.Qualifier = message->Qualifier;
				ReplyMsg((struct Message *)message);

				switch(CED.Class)
				{
					case IDCMP_MOUSEBUTTONS:
						loop=FALSE;
						break;
					case IDCMP_MOUSEMOVE:
						mouseMoved=TRUE;
						break;
				}
			}

			if (mouseMoved)
			{
				if ( CPrefs.fastMenus )
					menuID = CheckFastMenu(window, fastMR, MR, rp);
				else
					menuID = CheckWhichMenu(window, MR, NUMMENUS, rp);

				if (menuID != prev_menuID)	// new menu chosen
				{
					if (olditem!=-1 && prev_menuID!=-1)	// deselect previous menu item
					{
						SafeSetWriteMask(rp, 0x3);
						SetDrMd(rp, JAM2 | COMPLEMENT);
						if ( CPrefs.fastMenus )
							RectFill(	rp,
												(LONG)rx+FAST_MENU_WIDTH+4,
												(LONG)ry+(olditem*MHEIGHT)+2,
												(LONG)rx+FAST_MENU_WIDTH+MR[prev_menuID]->width-5,
												(LONG)ry+(olditem*MHEIGHT)+MHEIGHT-1);
						else
							RectFill(	rp,
												(LONG)MR[prev_menuID]->x+4,
												(LONG)MR[prev_menuID]->y+(olditem*MHEIGHT)+2,
												(LONG)MR[prev_menuID]->x+MR[prev_menuID]->width-5,
												(LONG)MR[prev_menuID]->y+(olditem*MHEIGHT)+MHEIGHT-1);
						SetDrMd(rp, JAM1);
						SafeSetWriteMask(rp, 0xff);
						WaitBlit();
					}

					if (prev_menuID != -1)
					{
						if ( CPrefs.fastMenus )
							RemoveMenu(window, MR[prev_menuID], rx+FAST_MENU_WIDTH, ry);
						else
							RemoveMenu(window, MR[prev_menuID], MR[prev_menuID]->x, MR[prev_menuID]->y);
					}

					if (menuID != -1)
					{
						if (menuID==DA_MENU)
							Fill_MRO_Menu(EHI.activeScreen);
						if (menuID==SCREEN_MENU)
							Fill_DA_Menu(TRUE);
						if ( CPrefs.fastMenus )
							RenderMenu(window, MR[menuID], rx+FAST_MENU_WIDTH, ry);
						else
							RenderMenu(window, MR[menuID], MR[menuID]->x, MR[menuID]->y);
						olditem=-1;
					}

					prev_menuID = menuID;
				}
				if (menuID != -1)
				{
					if ( CPrefs.fastMenus )
						itemID = CheckWhichItem(window, MR[menuID], rx+FAST_MENU_WIDTH, ry);
					else
						itemID = CheckWhichItem(window, MR[menuID], MR[menuID]->x, MR[menuID]->y);
				}
			}
		}
	}

	/**** switch MOUSEMOVE off again ****/

	UA_SwitchMouseMoveOff(window);

	/**** de-hilite last chosen menu item ****/

	if (olditem!=-1 && prev_menuID!=-1)
	{
		SafeSetWriteMask(rp, 0x3);
		SetDrMd(rp, JAM2 | COMPLEMENT);

		for(i=0; i<3; i++ )
		{
			if ( CPrefs.fastMenus )
				RectFill(	rp,
									(LONG)rx+FAST_MENU_WIDTH+4,
									(LONG)ry+(olditem*MHEIGHT)+2,
									(LONG)rx+FAST_MENU_WIDTH+MR[prev_menuID]->width-5,
									(LONG)ry+(olditem*MHEIGHT)+MHEIGHT-1);
			else
				RectFill(	rp,
									(LONG)MR[prev_menuID]->x+4,
									(LONG)MR[prev_menuID]->y+(olditem*MHEIGHT)+2,
									(LONG)MR[prev_menuID]->x+MR[prev_menuID]->width-5,
									(LONG)MR[prev_menuID]->y+(olditem*MHEIGHT)+MHEIGHT-1);
			WaitBlit();
			Delay(2L);
		}

		menuID = prev_menuID;
		itemID = olditem;

		SetDrMd(rp, JAM1);
		SafeSetWriteMask(rp, 0xff);
	}

	/**** remove menu being shown ****/

	if (prev_menuID != -1)
	{
		if ( CPrefs.fastMenus )
			RemoveMenu(window, MR[prev_menuID], rx+FAST_MENU_WIDTH, ry);
		else
			RemoveMenu(window, MR[prev_menuID], MR[prev_menuID]->x, MR[prev_menuID]->y);
	}

	/**** restore screen under menubar (NEW: and remove fast menu) ****/

	restoreMBAR(rp);

	/**** activate previously active window ****/

	if ( activeWindow )
		ActivateWindow(activeWindow);

	/**** leave with selected menu item ****/

	*menu = menuID;
	*item = itemID;
}

/******** CheckWhichMenu() ********/
/*
 * rp is the rastport of the menubar layer
 *
 */

int CheckWhichMenu(	struct Window *window, struct MenuRecord **MR, int num,
										struct RastPort *rp)
{
int i;

	if (MR==NULL || window==NULL)
		return(-1);

	for(i=0; i<num; i++)
	{
		if (window->MouseX > MR[i]->titleX1 &&
				window->MouseX < MR[i]->titleX2 &&
				window->MouseY >= 0 &&
				window->MouseY <= MHEIGHT && MR[i]->height!=0)
		{
			if (oldmenu==(int)i)
				return((int)i);

			if (oldmenu!=-1)
			{
				SafeSetWriteMask(rp, 0x3);
				SetDrMd(rp, JAM2 | COMPLEMENT);
				RectFill(	rp, (LONG)MR[oldmenu]->titleX1+1, 1L,
									(LONG)MR[oldmenu]->titleX2-1, (LONG)MHEIGHT-1);
				SetDrMd(rp, JAM1);
				SafeSetWriteMask(rp, 0xff);
			}

			SafeSetWriteMask(rp, 0x3);
			SetDrMd(rp, JAM2 | COMPLEMENT);
			RectFill(	rp, (LONG)MR[i]->titleX1+1, 1L,
								(LONG)MR[i]->titleX2-1, (LONG)MHEIGHT-1);
			SetDrMd(rp, JAM1);
			SafeSetWriteMask(rp, 0xff);

			oldmenu=i;

			return(i);
		}
	}
	return(oldmenu);
}

/******** CheckWhichItem() ********/

int CheckWhichItem(struct Window *window, struct MenuRecord *MR, WORD offX, WORD offY)
{
float i, num;
struct RastPort *rp;
	
	if (window==NULL || MR->height==0)
		return(-1);

	rp = window->RPort;	//&screen->RastPort;

	if (window->MouseX > offX &&
			window->MouseX < (offX+MR->width) &&
			window->MouseY > offY &&
			window->MouseY < (offY+MR->height))
	{
		num = (float)MR->height / (float)MHEIGHT;
		if (MR->height>0)
		{
			//i = (float)(screen->MouseY-offY) / (float)MR->height;
			i = (float)(window->MouseY-offY) / (float)MR->height;
			i = i*num;

			if ( (int)i >= (int)num )
				goto not_ok;

			if (olditem==(int)i)
				return((int)i);
		}

		if (olditem!=-1)
		{
			SafeSetWriteMask(rp, 0x3);
			SetDrMd(rp, JAM2 | COMPLEMENT);
			RectFill(	rp,
								(LONG)offX+4,
								(LONG)offY+(olditem*MHEIGHT)+2,
								(LONG)offX+MR->width-5,
								(LONG)offY+(olditem*MHEIGHT)+MHEIGHT-1);
			SetDrMd(rp, JAM1);
			SafeSetWriteMask(rp, 0xff);
		}

		if (i<num)
		{
			if ( !(MR->disabled[(int)i]) )
			{
				SafeSetWriteMask(rp, 0x3);
				SetDrMd(rp, JAM2 | COMPLEMENT);
				RectFill(	rp,
									(LONG)offX+4,
									(LONG)offY+((int)i*MHEIGHT)+2,
									(LONG)offX+MR->width-5,
									(LONG)offY+((int)i*MHEIGHT)+MHEIGHT-1);
				SetDrMd(rp, JAM1);
				SafeSetWriteMask(rp, 0xff);
				olditem=(int)i;
				return((int)i);
			}

			olditem=-1;
			return(-1);
		}
	}

not_ok:

	if (olditem!=-1)
	{
		SafeSetWriteMask(rp, 0x3);
		SetDrMd(rp, JAM2 | COMPLEMENT);
		RectFill(	rp,
							(LONG)offX+4,
							(LONG)offY+(olditem*MHEIGHT)+2,
							(LONG)offX+MR->width-5,
							(LONG)offY+(olditem*MHEIGHT)+MHEIGHT-1);
		SetDrMd(rp, JAM1);
		SafeSetWriteMask(rp, 0xff);
	}

	olditem=-1;

	return(-1);
}

/******** CheckFastMenu() ********/
/*
 * rp is the rastport of the menubar layer
 *
 */

int CheckFastMenu(struct Window *window, struct MenuRecord *fastMR,
									struct MenuRecord **MR, struct RastPort *rp)
{
int line;

	if (fastMR==NULL || window==NULL)
		return(-1);

	if (	window->MouseX >= fastMR->x &&
				window->MouseX < (fastMR->x+fastMR->width-(fastMR->width/4)) &&
				window->MouseY >= fastMR->y &&
				window->MouseY < (fastMR->y+fastMR->height) )
	{
		//line = (screen->MouseY-fastMR->y) / (fastMR->height/NUMMENUS);
		line = (window->MouseY-fastMR->y) / (fastMR->height/NUMMENUS);

		if (line<0)
			return(oldmenu);
		if (line>=NUMMENUS)
			return(oldmenu);

		if ( MR[line]->height==0 )
			return(oldmenu);

		if (oldmenu==line)
			return(line);

		if (oldmenu!=-1)
		{
			SafeSetWriteMask(rp, 0x3);
			SetDrMd(rp, JAM2 | COMPLEMENT);
			RectFill(	rp,
								fastMR->x+4, fastMR->y+oldmenu*MHEIGHT+2,
								fastMR->x+fastMR->width-5, fastMR->y+oldmenu*MHEIGHT+MHEIGHT-1);
			SetDrMd(rp, JAM1);
			SafeSetWriteMask(rp, 0xff);
		}

		SafeSetWriteMask(rp, 0x3);
		SetDrMd(rp, JAM2 | COMPLEMENT);
		RectFill(	rp,
							fastMR->x+4, fastMR->y+line*MHEIGHT+2,
							fastMR->x+fastMR->width-5, fastMR->y+line*MHEIGHT+MHEIGHT-1 );
		SetDrMd(rp, JAM1);
		SafeSetWriteMask(rp, 0xff);

		oldmenu=line;

		return(line);
	}

	return(oldmenu);
}

/******** saveMBAR() ********/

void saveMBAR(struct RastPort *rp, WORD x, WORD y, WORD width, WORD height)
{
	ClipBlit(rp,x,y,&mbarRP,0,0,width,height,0xc0);
	WaitBlit();
	rx = x;
	ry = y;
	rw = width;
	rh = height;
}

/******** restoreMBAR() ********/

void restoreMBAR(struct RastPort *rp)
{
	WaitTOF();
	ClipBlit(&mbarRP,0,0,rp,rx,ry,rw,rh,0xc0);
	WaitBlit();
	rx=-1;
}

/******** RenderMenuBar() ********/

void RenderMenuBar(	struct RastPort *rp, struct Window *window,
										struct MenuRecord **MR, BOOL fastMenus )
{
int i;
WORD x,y,tw,th;
struct MenuRecord *fastMR;

	if ( fastMenus )
	{
		if ( window==scriptWindow )
			fastMR = &fast_script_MR;
		else
			fastMR = &fast_page_MR;

		tw = FAST_MENU_WIDTH;	//FAST_MENU_WIDTH+MENUWIDTH;
		th = FAST_MENU_HEIGHT;	//(11*MHEIGHT)+2;	// tallest menu

		x = window->MouseX - ( tw / 2 );
		if ( x < 0 )
			x = 0;
		if ( x > (window->Width-(tw+MENUWIDTH)) )
			x = window->Width-(tw+MENUWIDTH);

		y = window->MouseY - ( th / 2 );
		if ( y < 0 )
			y = 0;
		if ( y > (window->Height-((11*MHEIGHT)+2)) )
			y = window->Height-((11*MHEIGHT)+2);

		fastMR->x = x;
		fastMR->y = y;
	}
	else
	{
		x		= 0;
		y		= 0;
		tw	= window->Width;
		th	= MHEIGHT;
	}

	for(i=0; i<mbarBM.Depth; i++)
		mbarBM.Planes[i] = mbarBM.Planes[0] + i*RASSIZE(tw,th);
	InitBitMap(&mbarBM, mbarBM.Depth, tw, th);

	saveMBAR(rp,x,y,tw,th);

	if ( fastMenus )
	{
		SetAPen(rp, BGND_PEN);
		SetDrMd(rp, JAM1);
		RectFill(rp, x, y, x+fastMR->width-1, y+fastMR->height-1);

		/**** blit menu ****/

		WaitTOF();
/*
		BltBitMap(&fastMR->menuBM,
							0,0,
							rp->BitMap,
							fastMR->x, fastMR->y,
							fastMR->width, fastMR->height, 0xc0, 0xff, NULL);
*/
		BltBitMapRastPort(&fastMR->menuBM,0,0,rp,fastMR->x,fastMR->y,fastMR->width,fastMR->height,0xc0);
		WaitBlit();

		/**** disable 0 height menus ****/

		SetAPen(rp, HI_PEN);
		SetAfPt(rp, gui_pattern, 1);
		SetDrMd(rp, JAM1);
		for(i=0; i<NUMMENUS; i++)
		{
			if ( MR[i]->height==0 )
				RectFill(	rp, x+2L, (LONG)y+i*MHEIGHT+2,
									(LONG)x+fastMR->width-4, (LONG)y+(i*MHEIGHT)+MHEIGHT-1);
		}
		SetAfPt(rp, NULL, 0);
	}
	else
	{
		/**** draw white bar ****/

		SetAPen(rp, HI_PEN);
		RectFill(rp, 0,0,tw-1,th-1);

		/**** print the menu bar text ****/

		SetAPen(rp, LO_PEN);
		SetDrMd(rp, JAM1);

		if (window->WScreen->ViewPort.Modes & HIRES)
			Move(rp, 5, rp->TxBaseline+1);
		else
			Move(rp, 2, rp->TxBaseline+1);

		if (window==pageWindow)
		{
			Text(rp, pageScreenTitle, strlen(pageScreenTitle));
			if ( window->Width >= 640 )
			{
				Move(rp, window->Width-300, 8);
				Text(rp, msgs[Msg_AppTitle-1], strlen(msgs[Msg_AppTitle-1]));
			}
		}
		else
		{
			Text(rp, scriptScreenTitle, strlen(scriptScreenTitle));
			Move(rp, 298, 8);
			Text(rp, msgs[Msg_AppTitle-1], strlen(msgs[Msg_AppTitle-1]));
		}

		/**** disable 0 height menus ****/

		SetAPen(rp, HI_PEN);
		SetAfPt(rp, gui_pattern, 1);
		SetDrMd(rp, JAM1);
		for(i=0; i<NUMMENUS; i++)
			if ( MR[i]->height==0 )
				RectFill(rp, MR[i]->titleX1, 0, MR[i]->titleX2, MHEIGHT-1);
		SetAfPt(rp, NULL, 0);
	}
}

/******** E O F ********/
