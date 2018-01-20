/*******************************************************************/
/*
 *
 *  T H I S   I S   A L S O   A   P L A Y E R   M O D U L E !
 *
 *
 */

#include "nb:pre.h"

/**** externals ****/

extern ULONG allocFlags;
extern struct Window *pageWindow;
extern struct Library *medialinkLibBase;
extern struct Window *paletteWindow;
extern struct CapsPrefs CPrefs;
extern UBYTE **msgs;   
extern struct List usedxapplist;
extern struct IntuitionBase *IntuitionBase;
extern struct Window *smallWindow;

/**** functions ****/

/******** SetBit() ********/

void SetBit(ULONG *allocs, ULONG flags)
{
	*allocs = *allocs | flags;
}

/******** UnSetBit() ********/

void UnSetBit(ULONG *allocs, ULONG flags)
{
	*allocs = *allocs & ~flags;
}

/******** TestBit() ********/

BOOL TestBit(ULONG allocs, ULONG flags)
{
	if (allocs & flags)
		return(TRUE);
	return(FALSE);
}

/******** SetByteBit() ********/

void SetByteBit(UBYTE *allocs, UBYTE flags)
{
	*allocs = *allocs | flags;
}

/******** UnSetByteBit() ********/

void UnSetByteBit(UBYTE *allocs, UBYTE flags)
{
	*allocs = *allocs & ~flags;
}

/******** TestByteBit() ********/

BOOL TestByteBit(UBYTE allocs, UBYTE flags)
{
	if (allocs & flags)
		return(TRUE);
	return(FALSE);
}

/******** InvertByteBit() ********/

void InvertByteBit(UBYTE *allocs, UBYTE flags)
{
	if ( TestByteBit(*allocs, flags) )
		UnSetByteBit(allocs, flags);
	else
		SetByteBit(allocs, flags);
}

#ifndef USED_FOR_PLAYER

/******** GetActiveWindow() ********/

struct Window *GetActiveWindow(void)
{
	Forbid();

	if (	TestBit(allocFlags, PAGEWINDOW_FLAG) &&
				(pageWindow->Flags & WFLG_WINDOWACTIVE) )
	{
		Permit();
		return(pageWindow);
	}

	if (	TestBit(allocFlags, PALETTEWINDOW_FLAG) &&
				(paletteWindow->Flags & WFLG_WINDOWACTIVE) )
	{
		Permit();
		return(paletteWindow);
	}

	if ( smallWindow && (smallWindow->Flags & WFLG_WINDOWACTIVE) )
	{
		Permit();
		return(smallWindow);
	}

	Permit();

	return(NULL);
}

#endif

/******** AbsInt() ********/

int AbsInt(int a, int b)
{
int c;

	c = a - b;
	if (c<0)
		c*=-1;
	return(c);
}

/******** AbsWORD() ********/

int AbsWORD(int a, int b)
{
WORD c;

	c = a - b;
	if (c<0)
		c*=-1;
	return(c);
}

#ifndef USED_FOR_PLAYER

#if 0
/******** my_SetAPen() ********/

void my_SetAPen(struct Window *window, long pen)
{
#if 0

#define HI_PEN 		2L	/* pen color to draw top and left lines of buttons */
#define LO_PEN		1L	/* pen color to draw bottom and right lines of buttons */
#define TEXT_PEN	1L	/* pen color to draw text inside buttons */
#define AREA_PEN	3L	/* pen color to draw back of requesters, gadgets etc. */
#define BGND_PEN	0L

#endif

	if ( window->UserData!=NULL )
	{
		/**** UserData[]  0        1      2      3          ****/
		/****             AREA_PEN LO_PEN HI_PEN TEXT_PEN   ****/

		if (pen==HI_PEN)
			pen = window->UserData[2];
		else if (pen==LO_PEN)
			pen = window->UserData[1];
		else if (pen==TEXT_PEN)
			pen = window->UserData[3];
		else if (pen==AREA_PEN)
			pen = window->UserData[0];
		else
			pen = window->UserData[0];

		SetAPen(window->RPort, pen);
	}
	else
	{
		if (window->WScreen->BitMap.Depth==1)
		{
			if (pen==HI_PEN)
				pen = 1L;
			else if (pen==LO_PEN)
				pen = 1L;
			else if (pen==TEXT_PEN)
				pen = 1L; //0L;
			else if (pen==AREA_PEN)
				pen = 0L;
			else
				pen = 0L;
			SetAPen(window->RPort, pen);
		}
		else
			SetAPen(window->RPort, pen);
	}
}
#endif

/******** AdjustGadgetCoordsRange() ********/

void AdjustGadgetCoordsRange(	struct GadgetRecord *GR,
															int xoffset, int yoffset,
															int start, int end)
{
int i;

	for(i=start; i<(end+1); i++)
	{
		GR[i].x1 += xoffset; 
		GR[i].x2 += xoffset; 
		GR[i].y1 += yoffset; 
		GR[i].y2 += yoffset;
	}
}

#endif

/******** RemoveQuotes() ********/

void RemoveQuotes(STRPTR str)
{
int i,len;

	/* remove trailing and ending ' or " */
	len = strlen(str)-2;
	if (len>0)
	{
		for(i=0; i<len; i++)
			str[i] = str[i+1];
		str[len] = '\0';
	}
	else
		str[0] = '\0';
}

/******** swapInts() ********/

void swapInts(int *a, int *b)
{
int swap;

	swap = *a;
	*a = *b;
	*b = swap;
}

/******** swapWORDS() ********/

void swapWORDS(WORD *a, WORD *b)
{
WORD swap;

	swap = *a;
	*a = *b;
	*b = swap;
}

/******** CreateUsedXappList() ********/

ULONG CreateUsedXappList(struct ScriptInfoRecord *SIR)
{
int i;
SNRPTR this_node;
struct List *list;
struct Node *node;
BOOL addxapp;
ULONG numObjects=0L;

	NewList(&usedxapplist);

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] != NULL)
		{
			list = SIR->allLists[i];
			if (list->lh_TailPred != (struct Node *)list)
			{
				for(this_node=(SNRPTR)list->lh_Head;
						this_node->node.ln_Succ;
						this_node=(SNRPTR)this_node->node.ln_Succ)
				{
					if (i>0)
						numObjects++;
					if ( this_node->nodeType==TALK_USERAPPLIC )
					{
						addxapp=TRUE;
						if ( IsXappUsed(this_node->objectPath) )
							addxapp=FALSE;	// xapp already in list
						if ( addxapp )
						{
							node = (struct Node *)AllocMem(sizeof(struct Node),MEMF_CLEAR|MEMF_ANY);
							if (node)
							{
								AddTail(&usedxapplist, (struct Node *)node);
								node->ln_Name = (char *)this_node->objectPath;
							}
						}
					}
				}
			}
		}
	}

	return( numObjects );
}

/******** FreeUsedXappList() ********/

void FreeUsedXappList(void)
{
struct Node *work_node, *next_node;

	work_node = usedxapplist.lh_Head;	// first node
	while(next_node = (work_node->ln_Succ))
	{
		FreeMem(work_node,sizeof(struct Node));
		work_node = next_node;
	}
}

#ifndef USED_FOR_PLAYER

/******** SetSpriteOfActWdw() ********/

void SetSpriteOfActWdw(BYTE which)
{
struct Window *window;
	
	Forbid();
	window = IntuitionBase->ActiveWindow;
	Permit();
	if ( window )
		UA_SetSprite(window,which);
}

#endif

/******** PrintAt() ********/

void PrintAt(int pen, struct RastPort *rp, int x, int y, STRPTR str)
{
	SetAPen(rp,pen);
	SetDrMd(rp,JAM1);
	Move(rp,x,y);
	Text(rp,str,strlen(str));
}

/******** E O F ********/
