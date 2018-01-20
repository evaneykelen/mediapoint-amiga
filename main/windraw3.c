#include "nb:pre.h"

/**** externals ****/

extern struct CapsPrefs CPrefs;
extern struct Window *pageWindow;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct BitMap sizeBM;
extern struct RastPort sizeRP;

/**** functions ****/

/******** RedrawAllTranspWindows() ********/

void RedrawAllTranspWindows(struct EditWindow *ew, struct EditSupport *es,
														int x, int y, int w, int h, int notWdw )
{
int i, pos;
BOOL marked[MAXEDITWINDOWS];
BOOL done[MAXEDITWINDOWS];
int saveX, saveY, saveW, saveH;
struct EditWindow *oriEW;
BOOL old,new;

	pos=0;
	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i]!=NULL && EditWindowList[i]->BackFillType>=1 )
		{
			pos++;
			break;
		}
	}
	if (pos==0)
		return; // exit fast: no transparent windows around!

	oriEW = ew;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		marked[i] = FALSE;
		done[i] = FALSE;
	}

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i] != NULL && EditWindowList[i] == ew )
		{
			pos = i;
			break;
		}
	}

	/**** save current co-ordinates ****/

	saveX = EditWindowList[pos]->X;
	saveY = EditWindowList[pos]->Y;
	saveW = EditWindowList[pos]->Width;
	saveH = EditWindowList[pos]->Height;

	/**** first check situation in old situation ****/

	EditWindowList[pos]->X			= x;
	EditWindowList[pos]->Y			= y;
	EditWindowList[pos]->Width	= w;
	EditWindowList[pos]->Height	= h;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if (	EditWindowList[i]!=NULL && EditWindowList[i]->BackFillType>=1 &&
					EditWindowList[i]->DrawSeqNum > oriEW->DrawSeqNum )
		{
			if ( BoxInsideBox(ew, EditWindowList[i]) )
				marked[i] = TRUE;
		}
	}

	/**** then check situation in new situation ****/

	EditWindowList[pos]->X			= saveX;
	EditWindowList[pos]->Y			= saveY;
	EditWindowList[pos]->Width	= saveW;
	EditWindowList[pos]->Height	= saveH;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if (	EditWindowList[i]!=NULL && EditWindowList[i]->BackFillType>=1 &&
					EditWindowList[i]->DrawSeqNum > oriEW->DrawSeqNum )
		{
			if ( BoxInsideBox(ew, EditWindowList[i]) )
				marked[i] = TRUE;
		}
	}

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i] != NULL && EditWindowList[i] == oriEW )
		{
			marked[i] = TRUE;
			break;
		}
	}	

	pos=0;
	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if (marked[i] && ( EditWindowList[i]->DrawSeqNum >= oriEW->DrawSeqNum ) &&
				EditWindowList[i]->BackFillType>=1 )
			pos++;
		else
			marked[i] = FALSE;
	}

	/**** if only one window is involved and it does not overlap in the old AND new ****/
	/**** situation AND w and h have not changed then don't redraw at all.					****/
	/**** The transparent window has only been moved from none overlapping spot			****/
	/**** to the other (great for demo's!)																					****/

	if (pos==1 && saveW==w && saveH==h)
	{
		old=FALSE;
		new=FALSE;
		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if ( marked[i] )
			{
				saveX = EditWindowList[i]->X;
				saveY = EditWindowList[i]->Y;
				saveW = EditWindowList[i]->Width;
				saveH = EditWindowList[i]->Height;

				/**** first check situation in old situation ****/

				EditWindowList[i]->X			= x;
				EditWindowList[i]->Y			= y;
				EditWindowList[i]->Width	= w;
				EditWindowList[i]->Height	= h;

				old = CheckIfWdwOverlaps(i);

				/**** then check situation in new situation ****/

				EditWindowList[i]->X			= saveX;
				EditWindowList[i]->Y			= saveY;
				EditWindowList[i]->Width	= saveW;
				EditWindowList[i]->Height	= saveH;

				new = CheckIfWdwOverlaps(i);

				if ( !old && !new )
					return;
				else
					break;
			}
		}
	}

	if ( notWdw != -1 )
		marked[ notWdw ] = FALSE;
									
	for(i=0; i<MAXEDITWINDOWS; i++)
		if (marked[i])
			DrawEditWindow(EditWindowList[i], EditSupportList[i]);
}

/******** RedrawAllTranspWindows_V2() ********/

void RedrawAllTranspWindows_V2(int x, int y, int w, int h, int DSN)
{
int i, current=-1, pos;
BOOL marked[MAXEDITWINDOWS];
BOOL done[MAXEDITWINDOWS];
struct EditWindow *ew, dummyEW;

	pos=0;
	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i]!=NULL && EditWindowList[i]->BackFillType>=1 )
		{
			pos++;
			break;
		}
	}
	if (pos==0)
		return; // exit fast: no transparent windows around!

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		marked[i] = FALSE;
		done[i] = FALSE;
	}

	dummyEW.X						= x;
	dummyEW.Y						= y;
	dummyEW.Width				= w;
	dummyEW.Height			= h;

	current = 0;
	ew = &dummyEW;

	/**** mark all windows that overlap ew ****/

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if (	EditWindowList[i]!=NULL && EditWindowList[i]->BackFillType>=1 &&
					EditWindowList[i]->DrawSeqNum >= DSN )
		{
			if ( BoxInsideBox(ew, EditWindowList[i]) )
				marked[i] = TRUE;
		}
	}
									
	for(i=0; i<MAXEDITWINDOWS; i++)
		if (	marked[i] && EditWindowList[i]->BackFillType>=1 &&
					EditWindowList[i]->DrawSeqNum >= DSN )
			DrawEditWindow(EditWindowList[i], EditSupportList[i]);
}

/******** GetAllTranspWindows_V3() ********/

void GetAllTranspWindows_V3(int x, int y, int w, int h, int DSN,
														struct EditWindow **ewlist)
{
int i, pos;
struct EditWindow dummyEW;

	pos=0;
	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i]!=NULL && EditWindowList[i]->BackFillType>=1 )
		{
			pos++;
			break;
		}
	}
	if (pos==0)
		return; // exit fast: no transparent windows around!

	dummyEW.X						= x;
	dummyEW.Y						= y;
	dummyEW.Width				= w;
	dummyEW.Height			= h;

	/**** mark all windows that overlap ew ****/

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if (	EditWindowList[i]!=NULL && EditWindowList[i]->BackFillType>=1 &&
					EditWindowList[i]->DrawSeqNum >= DSN )
		{
			if ( BoxInsideBox(&dummyEW, EditWindowList[i]) )
				ewlist[i] = EditWindowList[i];
		}
	}
}

/******** RedrawAllTranspWindows_V3() ********/

void RedrawAllTranspWindows_V3(struct EditWindow **ewlist)
{
int i,j;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		for(j=0; j<MAXEDITWINDOWS; j++)
			if ( ewlist[i]!=NULL && ewlist[i] == EditWindowList[j] )
				DrawEditWindow(EditWindowList[j], EditSupportList[j]);
	}
}

/******** E O F ********/
