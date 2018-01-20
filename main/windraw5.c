#include "nb:pre.h"

/**** externals ****/

extern struct CapsPrefs CPrefs;
extern struct Window *pageWindow;
extern struct Screen *pageScreen;
extern struct EditWindow **EditWindowList;
extern struct EditSupport **EditSupportList;
extern struct Library *medialinkLibBase;
extern struct BitMap sharedBM;

/**** functions ****/

/******** RedrawAllOverlapWdw() ********/

void RedrawAllOverlapWdw(	WORD prevX, WORD prevY, WORD prevW, WORD prevH,
													WORD newX, WORD newY, WORD newW, WORD newH,
													int wdw, BOOL redrawWdw, BOOL removeWdw )
{
int i, j, dsn;
BOOL marked[MAXEDITWINDOWS];
struct EditWindow ew2;

	SetSpriteOfActWdw(SPRITE_BUSY);

	for(i=0; i<MAXEDITWINDOWS; i++)
		marked[i] = FALSE;

	ew2.X			 = prevX;
	ew2.Y			 = prevY;
	ew2.Width	 = prevW;
	ew2.Height = prevH;

	dsn = EditWindowList[wdw]->DrawSeqNum;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i] )
		{
			if (	EditWindowList[i]->DrawSeqNum >= dsn &&
						BoxInsideBox( EditWindowList[i], &ew2 ) )
				marked[i] = TRUE;
		}
	}

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( marked[i] )
		{
			for(j=0; j<MAXEDITWINDOWS; j++)
			{
				if (	EditWindowList[j] &&
							EditWindowList[j]->DrawSeqNum > EditWindowList[i]->DrawSeqNum &&
							BoxInsideBox( EditWindowList[j], EditWindowList[i] ) )
					marked[j] = TRUE;
			}
		}
	}

	ew2.X			 = newX;
	ew2.Y			 = newY;
	ew2.Width	 = newW;
	ew2.Height = newH;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( EditWindowList[i] )
		{
			if (	EditWindowList[i]->DrawSeqNum >= dsn &&
						BoxInsideBox( EditWindowList[i], &ew2 ) )
				marked[i] = TRUE;
		}
	}

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if ( marked[i] )
		{
			for(j=0; j<MAXEDITWINDOWS; j++)
			{
				if (	EditWindowList[j] &&
							EditWindowList[j]->DrawSeqNum > EditWindowList[i]->DrawSeqNum &&
							BoxInsideBox( EditWindowList[j], EditWindowList[i] ) )
					marked[j] = TRUE;
			}
		}
	}

	for(i=MAXEDITWINDOWS-1; i>=0; i--)
		if ( marked[i] && EditSupportList[i]->restore_bm.Planes[0] && i!=wdw )
			RestoreBack( EditWindowList[i], EditSupportList[i] );

	EditWindowList[wdw]->X			= prevX;
	EditWindowList[wdw]->Y			= prevY;
	EditWindowList[wdw]->Width	= prevW;
	EditWindowList[wdw]->Height	= prevH;

	if ( removeWdw )
		RestoreBack(EditWindowList[wdw], EditSupportList[wdw]);

	EditWindowList[wdw]->X			= newX;
	EditWindowList[wdw]->Y			= newY;
	EditWindowList[wdw]->Width	= newW;
	EditWindowList[wdw]->Height	= newH;

	if ( !redrawWdw )
		marked[wdw] = FALSE;

	for(i=0; i<MAXEDITWINDOWS; i++)
		if ( marked[i] )
			DrawEditWindow(EditWindowList[i], EditSupportList[i]);

	SetSpriteOfActWdw(SPRITE_NORMAL);
}

/******** RedrawAllOverlapWdwEasy() ********/

void RedrawAllOverlapWdwEasy( int wdw, BOOL redrawWdw, BOOL removeWdw )
{
	RedrawAllOverlapWdw(	EditWindowList[wdw]->X,
												EditWindowList[wdw]->Y,
												EditWindowList[wdw]->Width,
												EditWindowList[wdw]->Height,
												EditWindowList[wdw]->X,
												EditWindowList[wdw]->Y,
												EditWindowList[wdw]->Width,
												EditWindowList[wdw]->Height, wdw,
												redrawWdw, removeWdw );
}

/******** RedrawAllOverlapWdwList() ********/

void RedrawAllOverlapWdwList(	BOOL *list )
{
int i, j, l;
BOOL marked[MAXEDITWINDOWS];

	SetSpriteOfActWdw(SPRITE_BUSY);

	for(i=0; i<MAXEDITWINDOWS; i++)
		marked[i] = FALSE;

	for(l=0; l<MAXEDITWINDOWS; l++)
	{
		if ( list[l] )
		{
			for(i=0; i<MAXEDITWINDOWS; i++)
			{
				if ( EditWindowList[i] )
				{
					if (	EditWindowList[i]->DrawSeqNum >= EditWindowList[l]->DrawSeqNum &&
								BoxInsideBox( EditWindowList[i], EditWindowList[l] ) )
						marked[i] = TRUE;
				}
			}

			for(i=0; i<MAXEDITWINDOWS; i++)
			{
				if ( marked[i] )
				{
					for(j=0; j<MAXEDITWINDOWS; j++)
					{
						if (	EditWindowList[j] &&
									EditWindowList[j]->DrawSeqNum > EditWindowList[i]->DrawSeqNum &&
									BoxInsideBox( EditWindowList[j], EditWindowList[i] ) )
							marked[j] = TRUE;
					}
				}
			}
		}
	}

	for(i=MAXEDITWINDOWS-1; i>=0; i--)
		if ( marked[i] && EditSupportList[i]->restore_bm.Planes[0] )
			RestoreBack( EditWindowList[i], EditSupportList[i] );

	for(i=0; i<MAXEDITWINDOWS; i++)
		if ( marked[i] )
			DrawEditWindow(EditWindowList[i], EditSupportList[i]);

	SetSpriteOfActWdw(SPRITE_NORMAL);
}

/******** PrepareRedrawAll() ********/

void PrepareRedrawAll(	WORD *prevX, WORD *prevY, WORD *prevW, WORD *prevH,
												WORD *newX, WORD *newY, WORD *newW, WORD *newH,
												BOOL *list )
{
int i;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		*(list+i) = NULL;

		if ( EditWindowList[i] )
		{
			*(prevX+i) = EditWindowList[i]->X;		
			*(prevY+i) = EditWindowList[i]->Y;		
			*(prevW+i) = EditWindowList[i]->Width;		
			*(prevH+i) = EditWindowList[i]->Height;		

			*(newX+i) = EditWindowList[i]->X;		
			*(newY+i) = EditWindowList[i]->Y;		
			*(newW+i) = EditWindowList[i]->Width;		
			*(newH+i) = EditWindowList[i]->Height;		
		}
	}
}

/******** RedrawAllOverlapWdwListPrev() ********/

void RedrawAllOverlapWdwListPrev(	WORD *prevX, WORD *prevY, WORD *prevW, WORD *prevH,
																	WORD *newX, WORD *newY, WORD *newW, WORD *newH,
																	BOOL *list )
{
int i, j, dsn, l;
BOOL marked[MAXEDITWINDOWS];
struct EditWindow ew2;

	SetSpriteOfActWdw(SPRITE_BUSY);

	for(i=0; i<MAXEDITWINDOWS; i++)
		marked[i] = FALSE;

	for(l=0; l<MAXEDITWINDOWS; l++)
	{
		if ( list[l] )
		{
			ew2.X			 = *(prevX+l);
			ew2.Y			 = *(prevY+l);
			ew2.Width	 = *(prevW+l);
			ew2.Height = *(prevH+l);

			dsn = EditWindowList[l]->DrawSeqNum;

			for(i=0; i<MAXEDITWINDOWS; i++)
			{
				if ( EditWindowList[i] )
				{
					if (	EditWindowList[i]->DrawSeqNum >= dsn &&
								BoxInsideBox( EditWindowList[i], &ew2 ) )
						marked[i] = TRUE;
				}
			}

			for(i=0; i<MAXEDITWINDOWS; i++)
			{
				if ( marked[i] )
				{
					for(j=0; j<MAXEDITWINDOWS; j++)
					{
						if (	EditWindowList[j] &&
									EditWindowList[j]->DrawSeqNum > EditWindowList[i]->DrawSeqNum &&
									BoxInsideBox( EditWindowList[j], EditWindowList[i] ) )
							marked[j] = TRUE;
					}
				}
			}

			ew2.X			 = *(newX+l);
			ew2.Y			 = *(newY+l);
			ew2.Width	 = *(newW+l);
			ew2.Height = *(newH+l);

			for(i=0; i<MAXEDITWINDOWS; i++)
			{
				if ( EditWindowList[i] )
				{
					if (	EditWindowList[i]->DrawSeqNum >= dsn &&
								BoxInsideBox( EditWindowList[i], &ew2 ) )
						marked[i] = TRUE;
				}
			}

			for(i=0; i<MAXEDITWINDOWS; i++)
			{
				if ( marked[i] )
				{
					for(j=0; j<MAXEDITWINDOWS; j++)
					{
						if (	EditWindowList[j] &&
									EditWindowList[j]->DrawSeqNum > EditWindowList[i]->DrawSeqNum &&
									BoxInsideBox( EditWindowList[j], EditWindowList[i] ) )
							marked[j] = TRUE;
					}
				}
			}
		}
	}

	for(i=MAXEDITWINDOWS-1; i>=0; i--)
	{
		if ( EditSupportList[i] && EditSupportList[i]->restore_bm.Planes[0] )
		{
			if ( list[i] )
			{
				EditWindowList[i]->X			= *(prevX+i);
				EditWindowList[i]->Y			= *(prevY+i);
				EditWindowList[i]->Width	= *(prevW+i);
				EditWindowList[i]->Height	= *(prevH+i);

				RestoreBack(EditWindowList[i], EditSupportList[i]);

				EditWindowList[i]->X			= *(newX+i);
				EditWindowList[i]->Y			= *(newY+i);
				EditWindowList[i]->Width	= *(newW+i);
				EditWindowList[i]->Height	= *(newH+i);
			}
			else if (	marked[i] )
				RestoreBack( EditWindowList[i], EditSupportList[i] );
		}
	}

	for(i=0; i<MAXEDITWINDOWS; i++)
		if ( marked[i] )
			DrawEditWindow(EditWindowList[i], EditSupportList[i]);

	SetSpriteOfActWdw(SPRITE_NORMAL);
}

/******** RestoreBack() ********/

void RestoreBack(	struct EditWindow *ew, struct EditSupport *es )
{
	if ( es->restore_bm.Planes[0] )
	{
		BltBitMapFM((struct BitMap *)&es->restore_bm,
								0, 0,
								&sharedBM,
								0, 0, es->restore_w, es->restore_h,
								0xc0, 0xff, NULL);

		BltBitMapRastPort((struct BitMap *)&sharedBM,
											ew->X % 16, 0,
											pageWindow->RPort,
											ew->X, ew->Y, ew->Width, ew->Height,
											0xc0);
		WaitBlit();
	}
}

/******** E O F ********/
