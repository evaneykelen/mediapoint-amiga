#include "nb:pre.h"

/**** EXTERNALS ****/

extern struct EditWindow **EditWindowList;
extern struct EditSupport	**EditSupportList;
extern struct CapsPrefs CPrefs;
extern struct Window *pageWindow;
extern struct Screen *pageScreen;
extern struct TextFont *largeFont;
extern UBYTE **msgs;   
extern struct BitMap sharedBM;
extern struct RastPort sharedRP;
extern struct Library *medialinkLibBase;
extern struct EditWindow backEW;
extern struct EditSupport backES;
extern struct BitMap transpBM;
extern struct RastPort transpRP;

/**** FUNCTIONS ****/

/******** WritePageClip() ********/
 
void WritePageClip(	FILE *fp, struct EditWindow *ew, struct EditSupport *es,
										STRPTR path, STRPTR fileName, char **pageCommands)
{
struct BitMap *clipBM=NULL, destBM;
WORD offX, clipX, clipY, clipW, clipH, dummy;
BOOL alloced=FALSE;
TEXT fullPath[SIZE_FULLPATH], clipPath[SIZE_FULLPATH], clipFileName[SIZE_FILENAME];
TEXT scaleonoff[10], scrStr[SIZE_FULLPATH];
struct FileLock *lock, *dirLock;
UBYTE d;
struct RastPort destRP;
ULONG type;

	// Bitmap to save is of right size if:
	// · SIZE_PHOTO en es->scaled_bm / es->remapped_bm
	// · MOVE_PHOTO en es->remapped_bm

	if ( es->photoOpts & SIZE_PHOTO )
	{
		PutSizePicture(ew,es,TRUE,&clipW,&clipH,NULL,0,0);

		offX	= 0;	// offset in sharedBM
		clipX	= 0;	// x offset to save
		clipY	= 0;	// y offset to save

		/**** check if it is necessary to create a new save bitmap ****/

		if ( es->remapped_bm.Planes[0] )
			clipBM = &es->remapped_bm;
		else if ( es->scaled_bm.Planes[0] )
			clipBM = &es->scaled_bm;
		else
			clipBM = &sharedBM;
	}
	else if ( es->photoOpts & MOVE_PHOTO )
	{
		if ( !PutMovePicture(	ew,es,&dummy,&dummy,&dummy,&dummy,
													TRUE,&offX,&clipX,&clipY,&clipW,&clipH,NULL,0,0) )
			return;	// pic moved out of sight

		/**** check if it is necessary to create a new save bitmap ****/

		if ( es->remapped_bm.Planes[0] )
			clipBM = &es->remapped_bm;
		else
			clipBM = &sharedBM;

		if ( !(es->photoOpts & REMAP_PHOTO) )
			clipBM = NULL;

		if ( es->picPath[0]=='\0' )	// problably a screen
			clipBM = &sharedBM;
	}

	/**** create a save bitmap (if necessary) ****/

	if ( clipBM )
	{
		if ( es==&backES )	// background is saved without mask
			type = MEMF_CHIP;	// un'masked bitmap is first created and then saved
		else
			type = MEMF_ANY;
		
		if ( !InitAndAllocBitMap(&destBM, CPrefs.PageScreenDepth, clipW, clipH, type) )
		{
			UA_WarnUser(-1);
			return;
		}

		if ( es==&backES )
		{
			InitRastPort(&destRP);
			destRP.BitMap = &destBM;
			// copy mask from fast mem to chip mem
			d = transpBM.Depth;
			transpBM.Depth = 1;
			BltBitMapFM(&es->mask_bm,offX,0,&transpBM,offX,0,clipW,clipH,0xc0,0xff,NULL);
			transpBM.Depth = d;
			// use mask to blit only what we need to bitmap which is later saved
			BltMaskBitMapRastPort(&sharedBM,offX,0,&destRP,offX,0,clipW,clipH,
														ABC | ANBC | ABNC, transpBM.Planes[0]);
			WaitBlit();
		}
		else
			BltBitMapFM(&sharedBM,offX,0,&destBM,0,0,clipW,clipH,0xc0,0xff,NULL);

		alloced=TRUE;
		clipBM = &destBM;

		//BltBitMapFM(&destBM,0,0,pageScreen->RastPort.BitMap,0,0,clipW+clipW%16,clipH,0xc0,0xff,NULL);
		//BltBitMapFM(&es->mask_bm,0,0,pageScreen->RastPort.BitMap,0,clipH,clipW+clipW%16,clipH,0xc0,0xff,NULL);
	}

	/**** init vars ****/

	if ( es->photoOpts & SIZE_PHOTO )
		stccpy(scaleonoff, "ON", 10);
	else
		stccpy(scaleonoff, "OFF", 10);

	/**** create a path, e.g. 'ram:clips' ****/

	if ( clipBM )	// we are going to save a clip
	{
		UA_MakeFullPath(path, "Clips", fullPath);	// 'path' is where doc is saved

		lock = (struct FileLock *)Lock((STRPTR)fullPath, (LONG)ACCESS_WRITE);
		if (lock==NULL)
		{
			dirLock = (struct FileLock *)CreateDir(fullPath);
			if (dirLock==NULL)
			{
				/**** both locks failed ****/
				//SetStandardColors( pageWindow );
				Message(msgs[Msg_UnableToCreateClipDir-1]);
				if (alloced)
					FreeFastBitMap(&destBM);
				//ResetStandardColors( pageWindow );
				return;
			}
			else
				UnLock((BPTR)dirLock);
		}
		else
			UnLock((BPTR)lock);
	}

	/**** write the line 'CLIP "path", x, y, w, h, scale' ****/

	doIndent(fp, 0);

	if ( clipBM )
	{
		UA_ValidatePath(fullPath);
		sprintf(clipFileName, "%s-%d", fileName, (int)ew->DrawSeqNum);
		UA_MakeFullPath(fullPath, clipFileName, clipPath);

		if ( clipX < 0 )
			clipX = 0;

		if ( clipY < 0 )
			clipY = 0;

		StrToScript(clipPath, scrStr);
		fprintf(fp, "%s \"%s\",%d,%d,%d,%d,%s\n",
						pageCommands[TALK_CLIP], scrStr, clipX, clipY, clipW, clipH, scaleonoff);
		strcpy(es->picPath,clipPath);

		WriteTheClip(clipPath, clipFileName, clipBM, clipW, clipH, es);
	}
	else
	{
		StrToScript(es->picPath, scrStr);
		if ( ew->animIsAnim )
		{
			fprintf(fp, "%s \"%s\",%d,%d,%d,%d,%d,%d,%d\n",
							pageCommands[TALK_CLIPANIM], scrStr, clipX, clipY, clipW, clipH,
							ew->animSpeed, ew->animLoops, ew->animFromDisk);
		}
		else
		{
			fprintf(fp, "%s \"%s\",%d,%d,%d,%d,%s\n",
							pageCommands[TALK_CLIP], scrStr, clipX, clipY, clipW, clipH, scaleonoff);
		}
	}

	if (alloced)
		FreeFastBitMap(&destBM);
}

/******** WriteTheClip() ********/

void WriteTheClip(STRPTR path, STRPTR fileName, struct BitMap *destBM,
									WORD clipW, WORD clipH, struct EditSupport *es)
{
struct IFF_FRAME iff;
int oldDepth;

	oldDepth = destBM->Depth;
	if ( destBM->Depth > CPrefs.PageScreenDepth )
		destBM->Depth = CPrefs.PageScreenDepth;

	FastInitIFFFrame(&iff);

	iff.BMH.w									= clipW; //destBM->BytesPerRow*8;
	iff.BMH.h									= clipH; //destBM->Rows;
	iff.BMH.x									= 0;
	iff.BMH.y									= 0;
	iff.BMH.nPlanes						= destBM->Depth;
	iff.BMH.masking						= 0;
	iff.BMH.compression				= 1;
	iff.BMH.pad1							= 0;
	iff.BMH.transparentColor	= 0;
	iff.BMH.xAspect						= 0;
	iff.BMH.yAspect						= 0;
	iff.BMH.pageWidth					= CPrefs.PageScreenWidth;
	iff.BMH.pageHeight				= CPrefs.PageScreenHeight;
	iff.colorMap							= pageScreen->ViewPort.ColorMap;
	iff.viewModes							= CPrefs.PageScreenModes;

	if ( es==&backES )
		FastWriteIFF(&iff, (STRPTR)path, (struct BitMap24 *)destBM, NULL);
	else
		FastWriteIFF(&iff, (STRPTR)path, (struct BitMap24 *)destBM, (APTR)&es->mask_bm);

	if ( iff.Error != IFF_ERROR_OK )
		Message(msgs[Msg_UnableToWriteClip-1], fileName);

	iff.colorMap = NULL;	// else FastCloseIFF tries to deallocate it!!!!!

	FastCloseIFF(&iff);

	destBM->Depth = oldDepth;
}

/******** E O F ********/
