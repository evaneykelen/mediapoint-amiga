#include "nb:pre.h"

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
extern struct FileListInfo FLI;
extern UBYTE **msgs;   
extern struct ColorMap *undoCM;
extern int lastUndoableAction;
extern int tileWidth, tileHeight;
extern BOOL do_the_tile;
extern struct EditSupport backES;

/**** static globals ****/

STATIC struct EditWindow *ew_list[MAXEDITWINDOWS];
STATIC struct EditSupport *es_list[MAXEDITWINDOWS];

/**** functions ****/

void RemoveImportWindows(int num);

/******** ImportAPicture() ********/

void ImportAPicture(BOOL resizeIt, int remapIt)
{
int i, numPics, numWdw, slot, listpos, act, fps, numFrames;
TEXT filename[SIZE_FILENAME], fullPath[SIZE_FULLPATH];
struct EditWindow *ew;
struct EditSupport *es;
BOOL allow_pos;
WORD mx,my;
BOOL palettesEqual,setPalette;
TEXT splitPath[SIZE_FULLPATH];
TEXT splitName[SIZE_FILENAME];

	/**** show file requester ****/

	if ( !OpenAFile(	CPrefs.import_picture_Path, filename,
										msgs[Msg_SelectPics-1], pageWindow,
										DIR_OPT_ILBM | DIR_OPT_ANIM | DIR_OPT_NOINFO, TRUE) )
		return;

	numPics=0;
	for(i=0; i<(int)MAX_FILE_LIST_ENTRIES; i++)
		if ( *(FLI.selectionList+i) == 1 )
			numPics++;

	if ( numPics > MAXEDITWINDOWS )
		numWdw = MAXEDITWINDOWS;
	else
		numWdw = numPics;

	RemoveImportWindows(numWdw);	// remove (at least) 'numWdw' windows

	ActivateWindow(pageWindow);

	listpos=0;
	for(i=0; i<(int)MAX_FILE_LIST_ENTRIES; i++)
	{
		if ( *(FLI.selectionList+i) == 1 )
		{
			slot = SearchEmptyWindow();
			if ( slot!=-1 )
			{
				// First try to get an second-hand window...

				if ( ew_list[listpos] && es_list[listpos] )
				{
					EditWindowList[slot] = ew_list[listpos];
					EditSupportList[slot] = es_list[listpos];
					EditWindowList[slot]->DrawSeqNum = GetNewDrawSeqNum_2( EditWindowList[slot] );
					listpos++;
					allow_pos = FALSE;
				}
				else	// ...if all out then make a new one
				{
					slot = OpenEditWindow(slot,0,0,50,50);
					allow_pos = TRUE;
				}

				if ( slot != -1 )
				{
					ew = EditWindowList[slot];				
					es = EditSupportList[slot];				

					// make pic path

					UA_MakeFullPath(CPrefs.import_picture_Path,	FLI.fileList+1+SIZE_FILENAME*i, fullPath);

					// fill es

					es->Active = TRUE;
					if (resizeIt)
						es->photoOpts = SIZE_PHOTO;
					else
						es->photoOpts = MOVE_PHOTO;
					stccpy(es->picPath, fullPath, SIZE_FULLPATH);
					if (remapIt)
						es->photoOpts |= REMAP_PHOTO;

					// fill ew

					ew->PhotoOffsetX = 0;
					ew->PhotoOffsetY = 0;
					//if (resizeIt)
					//	ew->BackFillType = 0; // SOLID
					//else
					ew->BackFillType = 2; // TRANSP

					// open photo

					if ( !doActualILBMLoading(fullPath,
																		FLI.fileList+1+SIZE_FILENAME*i, es, ew, TRUE) )
					{
						// remove info from wdw that makes it a photo-carrying window
						es->photoOpts		= 0;
						es->picPath[0] = '\0';
						ew->BackFillType	= 0; // SOLID
					}

	/* START NEW */
	if ( es->photoOpts != 0 )
	{
		setPalette=FALSE;
		act = FirstActiveEditWindow();
		if ( numPics==1 && act!=-1 && es->cm )
		{
			/**** first check if loaded palette is equal to current palette ****/
			if ( CompareCM(es->cm, CPrefs.PageCM) )
				palettesEqual=TRUE;
			else
				palettesEqual=FALSE;
			/**** if not equal, ask if palette must be used ****/
			if ( !palettesEqual && !remapIt )
			{
				if (UA_OpenGenericWindow(	pageWindow, TRUE, TRUE, msgs[Msg_OK-1], msgs[Msg_Cancel-1],
																	QUESTION_ICON, msgs[Msg_SpecialsUsePalette-1], TRUE, NULL))
					setPalette=TRUE;
			}
		}
		if ( !setPalette )
			es->photoOpts |= REMAP_PHOTO;
	}
	/* END NEW */

					/**** Check if this is an anim ****/

					if ( checkFileType(fullPath,NULL)==ANIM )
						ew->animIsAnim = TRUE;
					else
						ew->animIsAnim = FALSE;

					if ( ew->animIsAnim )
					{
						// REMOVE FLAGS THAT WE DON'T WANT FOR ANIMS
						es->photoOpts &= ~REMAP_PHOTO;	// don't allow remapping with anims
						es->photoOpts &= ~SIZE_PHOTO;		// don't allow scaling with anims
						es->photoOpts |= MOVE_PHOTO;

						// INIT DEFAULT VALUES
						UA_SplitFullPath(fullPath, splitPath, splitName);
						GetAnimSpeed(splitPath,splitName,&fps,&numFrames);
						ew->animLoops = 1;
						ew->animSpeed = fps;
						ew->animFromDisk = 0;
						ew->animAddFrames = FALSE;
					}

					// draw and drag window

					if ( allow_pos )
					{
						if (	resizeIt || ( es->iff->BMH.w > pageWindow->Width  ) ||
									es->iff->BMH.h > pageWindow->Height )
						{
							if ( es->iff->BMH.w > pageWindow->Width )
								ew->Width = pageWindow->Width / 3; 
							else
								ew->Width = es->iff->BMH.w;
	
							if ( es->iff->BMH.h > pageWindow->Height )
								ew->Height = pageWindow->Height / 3; 
							else
								ew->Height = es->iff->BMH.h;
						}
						else
						{
							ew->Width = es->iff->BMH.w;
							ew->Height = es->iff->BMH.h;
						}

						mx = pageWindow->MouseX;
						my = pageWindow->MouseY;

						ew->X = mx - (ew->Width/2);
						if ( ew->X < 0 )
							ew->X = 0;
						if ( ew->X+ew->Width > pageWindow->Width )
							ew->X = pageWindow->Width - ew->Width;

						ew->Y = my - (ew->Height/2);
						if ( ew->Y < 0 )
							ew->Y = 0;
						if ( ew->Y+ew->Height > pageWindow->Height )
							ew->Y = pageWindow->Height - ew->Height;

						if ( ew->Width == pageWindow->Width && ew->Height == pageWindow->Height )
						{
							ew->Width /= 3;
							ew->Height /= 3;
						}
					}

					SetSpriteOfActWdw(SPRITE_BUSY);
					CorrectEW(ew);
					DrawEditWindow(ew,es);
					SetSpriteOfActWdw(SPRITE_NORMAL);

					if ( allow_pos )
					{
						Forbid();
						pageWindow->MouseX = mx;
						pageWindow->MouseY = my;
						Permit();
						DragEditWindow(DO_EW_DRAG,slot);
					}
				}
			}
		}
	}

	/**** free dir memory ****/

	CloseDir();

	SetSpriteOfActWdw(SPRITE_NORMAL);

	/**** process colors ****/

	if ( setPalette && es->cm )
	{
		SetScreenToCM(pageScreen, es->cm);
		lastUndoableAction=PAGE_UNDO_IMPORT;
		SyncAllColors(TRUE);
	}

/*
	slot = FirstActiveEditWindow();
	if ( numPics==1 && slot!=-1 )
		ProcessLoadedColors(EditSupportList[slot],remapIt);
*/
}

/******** doActualILBMLoading() ********/

BOOL doActualILBMLoading(	STRPTR path, STRPTR fileName,
													struct EditSupport *es, struct EditWindow *ew, BOOL ask)
{
LONG needed, screen_size;
ULONG IFF_ID;
BOOL hasCMAP;
int col,row;

	/**** free window bitmaps ****/

	RemovePic24FromWindow(es,&es->ori_bm);
	RemovePicFromWindow(es,&es->scaled_bm);
	RemovePicFromWindow(es,&es->remapped_bm);
	RemovePicFromWindow(es,&es->mask_bm);
	RemovePicFromWindow(es,&es->ori_mask_bm);

	ClearBitMap24(&es->ori_bm);
	es->ori_w = 0;
	es->ori_h = 0;

	ClearBitMap(&es->scaled_bm);
	es->scaled_w = 0;
	es->scaled_h = 0;

	ClearBitMap(&es->remapped_bm);
	es->remapped_w = 0;
	es->remapped_h = 0;

	ClearBitMap(&es->mask_bm);
	es->mask_w = 0;
	es->mask_h = 0;

	ClearBitMap(&es->ori_mask_bm);
	es->ori_mask_w = 0;
	es->ori_mask_h = 0;

	/**** free colormap ****/

	if ( es->cm!=NULL )
		FreeColorMap(es->cm);
	es->cm = NULL;

	/**** alloc iff struct (if not yet done) ****/

	if ( es->iff == NULL )
	{
		es->iff = (struct IFF_FRAME *)AllocMem(	(LONG)sizeof(struct IFF_FRAME),
																						MEMF_ANY | MEMF_CLEAR);
		if (es->iff==NULL)
		{
			UA_WarnUser(144);
			return(FALSE);
		}
	}

	/**** INIT THE IFF FRAME ****/

	FastInitIFFFrame(es->iff);

	/**** OPEN THE PICTURE ****/

	IFF_ID = FastOpenIFF(es->iff, path);

	if ( es->iff->Error || (IFF_ID!=ILBM && IFF_ID!=ANIM) )
	{
		FastCloseIFF(es->iff);
		Message(msgs[Msg_UnableToReadPic-1], fileName);
		return(FALSE);
	}

	/**** SCAN THE BITMAP HEADER ****/

	FastParseChunk(es->iff, BMHD);
	if (es->iff->Error != NULL)
	{
		FastCloseIFF(es->iff);
		Message(msgs[Msg_UnableToReadPic-1], fileName);
		return(FALSE);
	}

	/**** CHECK THE DIMENSIONS ****/

 	if ( es->iff->BMH.w <= 0 || es->iff->BMH.h <= 0 )
	{
		FastCloseIFF(es->iff);
		Message(msgs[Msg_PictureTooSmall-1]);
		return(FALSE);
	}

 	if ( es->iff->BMH.nPlanes < 1 || es->iff->BMH.nPlanes > 24 )
	{
		FastCloseIFF(es->iff);
		Message(msgs[Msg_TooManyColors-1]);
		return(FALSE);
	}

	/***** IF THE WINDOW IS THE BACKDROP WINDOW, DO TILING STUFF ****/

	col = CPrefs.PageScreenWidth / es->iff->BMH.w;
	row = CPrefs.PageScreenHeight / es->iff->BMH.h;

	// There should be at least 4 tiles else 'shall i tile' question is not asked

	if ( es == &backES )
	{
		tileWidth		= es->iff->BMH.w;
		tileHeight	= es->iff->BMH.h;
		do_the_tile = FALSE;

		if ( (col*row) >= 4 || col>=4 || row>=4 )
		{
			if ( !ask || UA_OpenGenericWindow(pageWindow, TRUE, TRUE, msgs[Msg_OK-1], msgs[Msg_Cancel-1],
																				QUESTION_ICON, msgs[Msg_TileIt-1], TRUE, NULL ))
			{		
				do_the_tile = TRUE;
				es->photoOpts &= ~SIZE_PHOTO;	// do not scale of you also tile...
				es->photoOpts |= MOVE_PHOTO;	// ...but switch on this instead
			}		
		}
	}

	/**** CHECK IF THIS PICTURE'S GONNA FIT ****/

	needed = RASSIZE(es->iff->BMH.w,es->iff->BMH.h)*es->iff->BMH.nPlanes;
	screen_size = RASSIZE(CPrefs.PageScreenWidth,CPrefs.PageScreenHeight)*
								CPrefs.PageScreenDepth;
	screen_size *= 2;

	if ( (AvailMem(MEMF_LARGEST)-needed) < screen_size )
	{
		CloseAllUndoWindows();
		CloseAllClipboardWindows();

		// second attempt
	
		if ( (AvailMem(MEMF_LARGEST)-needed) < screen_size )
		{
			FastCloseIFF(es->iff);
			Message(msgs[Msg_GraphicsMemError-1]);
			return(FALSE);
		}
	}

	/**** SCAN THE VIEWMODES ****/

	FastParseChunk(es->iff, CAMG);

	/**** SCAN THE COLORMAP ****/

	FastParseChunk(es->iff, CMAP);
	if (es->iff->Error != NULL)
		hasCMAP=FALSE;
	else
		hasCMAP=TRUE;

	/**** Let es->cm point to the colorMap allocated by Pascal's	 ****/
	/**** iff routines. So: don't let him free this colorMap!!!!!  ****/

	if ( hasCMAP )
	{
		es->cm = es->iff->colorMap;
		es->iff->colorMap = NULL;
	}

	/**** create ori_bm ****/

	if ( !InitAndAllocFastBitMap24(&es->ori_bm, es->iff->BMH.nPlanes, es->iff->BMH.w, es->iff->BMH.h) )
	{
		UA_WarnUser(146);
		FastCloseIFF(es->iff);
		return(FALSE);
	}
	es->ori_w = es->iff->BMH.w;
	es->ori_h = es->iff->BMH.h;

	if ( es->iff->BMH.masking == mskHasMask )
	{	
		if ( !InitAndAllocBitMap(&es->ori_mask_bm, 1, es->ori_w, es->ori_h, MEMF_ANY) )
		{
			UA_WarnUser(-1);
			FastCloseIFF(es->iff);
			return(FALSE);
		}
		es->ori_mask_w = es->ori_w;
		es->ori_mask_h = es->ori_h;

		es->photoOpts	|= HAS_A_MASK;
		es->photoOpts	|= REMAP_PHOTO;
	}

	/**** DECRUNCH BITMAP ****/	

	if ( es->iff->BMH.masking == mskHasMask )
		FastDecodeBody(es->iff, (struct BitMap24 *)&es->ori_bm, (APTR)es->ori_mask_bm.Planes[0]);
	else
		FastDecodeBody(es->iff, (struct BitMap24 *)&es->ori_bm, NULL);
	if (es->iff->Error != NULL)
	{
		FastCloseIFF(es->iff);
		Message(msgs[Msg_IFFChunkMisses-1], "BODY");
		return(FALSE);
	}

	/**** CLOSE IFF PARSER ****/

	FastCloseIFF(es->iff);

	return(TRUE);
}

/******** ClearBitMap() ********/

void ClearBitMap(struct BitMap *bm)
{
int i;

	for(i=0; i<8; i++)
		bm->Planes[i] = NULL;
}

/******** InitAndAllocBitMap() ********/

BOOL InitAndAllocBitMap(struct BitMap *bm, WORD d, WORD w, WORD h, LONG memType)
{
	ClearBitMap(bm);	// I'm not sure if pascal does this -- I need it definitely
	return( AllocPlanes24(	(struct BitMap24 *)bm, d,w,h,
													memType | MEMF_CLEAR, BM24F_NONE) );
}

/********	FreeFastBitMap() ********/

void FreeFastBitMap(struct BitMap *bm)
{
	FreePlanes24( (struct BitMap24 *)bm );
	ClearBitMap(bm);	// I'm not sure if pascal does this -- I need it definitely
}

/******** ClearBitMap24() ********/

void ClearBitMap24(struct BitMap24 *bm)
{
int i;

	for(i=0; i<24; i++)
		bm->Planes[i] = NULL;
}

/******** InitAndAllocFastBitMap24() ********/

BOOL InitAndAllocFastBitMap24(struct BitMap24 *bm, WORD d, WORD w, WORD h)
{
	ClearBitMap24(bm);	// I'm not sure if pascal does this -- I need it definitely
	bm->MagicCookie = MAGIC_COOKIE_BM24;
	return( AllocPlanes24(bm, d,w,h, MEMF_ANY | MEMF_CLEAR, BM24F_NONE) );
								//BM24F_CLEAR|BM24F_INTERLEAVED) );
}

/********	FreeFastBitMap24() ********/

void FreeFastBitMap24(struct BitMap24 *bm)
{
	FreePlanes24(bm);
	ClearBitMap24(bm);	// I'm not sure if pascal does this -- I need it definitely
}

/******** RemoveImportWindows() ********/

void RemoveImportWindows(int num)
{
int i,j,dsn,actualRemoved;

	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		ew_list[i] = NULL;
		es_list[i] = NULL;
	}

	if ( NumActiveEditWindows()==0 )
		return;

	/**** remove windows until 'num' *active* windows are removed ****/

	j=0;
	dsn=9999;
	actualRemoved=0;
	for(i=MAXEDITWINDOWS-1; i>=0; i--)
	{
		if ( j<num && EditSupportList[i] )
		{
			// remove window...

			if ( EditSupportList[i]->restore_bm.Planes[0] )
			{
				actualRemoved++;
				RestoreBack(EditWindowList[i], EditSupportList[i]);
				FreeFastBitMap( &EditSupportList[i]->restore_bm );
				EditSupportList[i]->restore_w = 0;
				EditSupportList[i]->restore_h = 0;
			}

			if ( EditWindowList[i]->DrawSeqNum < dsn )
				dsn = EditWindowList[i]->DrawSeqNum;

			// ...but only count active ones

			if ( EditSupportList[i]->Active)
				j++;
		}
	}

	// remove active windows which were 'marked' in previous loop

	j=0;
	for(i=0; i<MAXEDITWINDOWS; i++)
	{
		if (	EditSupportList[i] && EditSupportList[i]->Active &&
					!EditSupportList[i]->restore_bm.Planes[0] )	// this is the 'mark'
		{
			//CloseEditWindow(EditWindowList[i], EditSupportList[i]);
			ew_list[j] = EditWindowList[i];
			es_list[j] = EditSupportList[i];
			j++;
			EditWindowList[i] = NULL;
			EditSupportList[i] = NULL;
		}
	}

	if ( actualRemoved > 0 )
	{	
		for(i=0; i<MAXEDITWINDOWS; i++)
		{
			if ( EditWindowList[i] && EditWindowList[i]->DrawSeqNum >= dsn )
			{
				DrawEditWindow(EditWindowList[i],EditSupportList[i]);
			}
		}
	}

	SortEditWindowLists(0);
}

/******** E O F ********/
