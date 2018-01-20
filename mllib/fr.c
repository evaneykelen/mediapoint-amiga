#include "mllib_includes.h"

/**** functions ****/

/*******************************************************************/
/*
 *   PUBLIC FUNCTIONS
 *
 *******************************************************************/

BOOL __saveds __asm LIBUA_OpenAFile(register __a0 struct Window *window,
																		register __a1 struct FileReqRecord *FRR,
																		register __a2 UWORD *mypattern1)
{
BOOL (*func)(APTR,APTR,APTR,APTR,int,BOOL);
struct MsgPort *port;
struct List *list;
struct Node *node;
struct RendezVousRecord *rvrec;

	port = (struct MsgPort *)FindPort(ML_RENDEZ_VOUS);
	if (port == NULL)
		return(FALSE);

	list = &(port->mp_MsgList);
	node = list->lh_Head;
	rvrec = (struct RendezVousRecord *)node->ln_Name;

	func = rvrec->openfunc;

	if ( !window && !mypattern1 )
		return( (*(func))(FRR->path,NULL,NULL,NULL,-1,0) );
	else
		return( (*(func))(FRR->path,FRR->fileName,FRR->title,window,FRR->opts,FRR->multiple) );
}

/******** SaveAFile() ********/

BOOL __saveds __asm LIBUA_SaveAFile(register __a0 struct Window *window,
																		register __a1 struct FileReqRecord *FRR,
																		register __a2 UWORD *mypattern1)
{
BOOL (*func)(APTR,APTR,APTR,APTR,int);
struct MsgPort *port;
struct List *list;
struct Node *node;
struct RendezVousRecord *rvrec;

	port = (struct MsgPort *)FindPort(ML_RENDEZ_VOUS);
	if (port == NULL)
		return(FALSE);

	list = &(port->mp_MsgList);
	node = list->lh_Head;
	rvrec = (struct RendezVousRecord *)node->ln_Name;

	func = rvrec->savefunc;
	return( (*(func))(FRR->path,FRR->fileName,FRR->title,window,FRR->opts) );
}

/*******************************************************************/
/*
 *	PRIVATE FUNCTIONS
 *
 *******************************************************************/

/******** InitPropInfo() ********/
/*
 * This functions should eliminate the problems with prop gadgets
 * which are used in non-lace and lace environments and write over
 * the container in which they live.
 *
 */

void InitPropInfo(struct PropInfo *PI, struct Image *IM)
{
	PI->VertPot			= 0;
	PI->VertBody		= 0;
	PI->CHeight			= 0;
	PI->VPotRes			= 0;

	IM->Height			= 0;
	IM->Depth				= 0;
	IM->ImageData		= NULL;
	IM->PlanePick		= 0x0000;
	IM->PlaneOnOff	= 0x0000;
	IM->NextImage 	= NULL;
}

/******** E O F ********/
