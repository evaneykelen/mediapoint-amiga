/*******************************************************************/
/*
 *
 *  T H I S   I S   A L S O   A   P L A Y E R   M O D U L E !
 *
 *
 */

#include "nb:pre.h"

/**** externals ****/

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *medialinkLibBase;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct TextFont *smallFont;
extern struct TextFont *largeFont;
extern struct TextFont *systemFont;
extern struct TextFont *tiny_smallFont;
extern struct TextFont *tiny_largeFont;
extern struct MsgPort *capsPort;
extern UWORD palettes[];
extern struct Library *ConsoleDevice;
extern struct ObjectInfo ObjectRecord;
extern struct BitMap gfxBitMap;
extern struct MsgPort *ML_Port;
extern struct Node *ML_Node;
extern struct RendezVousRecord rvrec;
extern struct FER FontEntryRecord;

/**** functions ****/

#ifdef USED_FOR_PLAYER
BOOL OpenAFile(STRPTR a,STRPTR b,STRPTR c,struct Window *d,int e,BOOL f) { } 
BOOL SaveAFile(STRPTR a,STRPTR b,STRPTR c,struct Window *d,int e) { }
#endif

/******** setupRendezVous() ********/

BOOL setupRendezVous(void)
{
	ML_Port = (struct MsgPort *)CreatePort(ML_RENDEZ_VOUS, 0);
	if (ML_Port == NULL)
	{
		UA_WarnUser(63);
		return(FALSE);
	}

	rvrec.intuition 			= (struct Library *)IntuitionBase;
	rvrec.graphics				= (struct Library *)GfxBase;
	rvrec.medialink				= (struct Library *)medialinkLibBase;
	rvrec.capsprefs				=	&CPrefs;
	rvrec.ehi							= &EHI;
	rvrec.smallfont				= smallFont;
	rvrec.largefont				= largeFont;
	rvrec.systemfont			= systemFont;
	rvrec.pagescreen			= NULL;				/* unknown at this time */
	rvrec.scriptscreen		= NULL;				/* unknown at this time */
	rvrec.capsport				= capsPort;
	rvrec.paletteList			= (UWORD *)&palettes;
	rvrec.ObjRec					= &ObjectRecord;
	rvrec.gfxBM						= &gfxBitMap;
	rvrec.aPtr						= NULL;
	rvrec.aLong						= 0L;
	rvrec.console					= (struct Library *)ConsoleDevice;
	rvrec.msgs						= NULL;				/* unknown at this time */   
	rvrec.openfunc				= OpenAFile;
	rvrec.savefunc				= SaveAFile;
	rvrec.aPtrTwo					= NULL;
	rvrec.tiny_smallfont	= tiny_smallFont;
	rvrec.tiny_largefont	= tiny_largeFont;
	rvrec.FontEntryRecord	= &FontEntryRecord;

	UA_PutCapsPort(capsPort);	// set global in mediapoint library

	ML_Node = (struct Node *)AllocMem(sizeof(struct Node), MEMF_PUBLIC|MEMF_CLEAR);
	if (ML_Node!=NULL)
	{
		ML_Node->ln_Name	= (char *)&rvrec;
		ML_Node->ln_Type	= 100;	/* arbitrary type identifier */
		ML_Node->ln_Pri		= 0;
		AddTail((struct List *)&(ML_Port->mp_MsgList), (struct Node *)ML_Node);
		return(TRUE);
	}
	else
		UA_WarnUser(64);

	return(FALSE);
}

/******** removeRendezVous() ********/

void removeRendezVous(void)
{
	if ( ML_Node!=NULL )
	{
		Remove(ML_Node);
		FreeMem(ML_Node, sizeof(struct Node));
	}

	if ( ML_Port != NULL )
		DeletePort((struct MsgPort *)ML_Port);
}

/******** E O F ********/
