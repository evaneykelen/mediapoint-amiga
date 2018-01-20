/*******************************************************************/
/*
 *
 *  T H I S   I S   A L S O   A   P L A Y E R   M O D U L E !
 *
 *
 */

#include "nb:pre.h"

/**** externals ****/

extern struct CapsPrefs CPrefs;
extern struct ObjectInfo ObjectRecord;
extern struct Window *scriptWindow;
extern UBYTE **msgs;   
extern struct Library *medialinkLibBase;
extern TEXT mainName[];
extern struct Document scriptDoc;

/**** functions ****/

/******** InitScriptList() ********/
/* input:    -
 * output:   pointer to a List structure or NULL on error.
 * function: allocates memory for a list header and initializes the header.
 */

struct List *InitScriptList(void)
{
struct List *list;

	list = (struct List *)AllocMem(sizeof(struct List), MEMF_ANY | MEMF_CLEAR);
	if (list==NULL)
	{
		UA_WarnUser(93);
		return(NULL);
	}

	NewList(list);

	return(list);
}

/******** FreeScriptList() ********/
/* input:    pointer to a List structure.
 * output:   -
 * function: Deallocates all nodes attached to the list and deallocates
 *           the list itself.
 */

void FreeScriptList(struct List *list)
{
struct ScriptNodeRecord *work_node;
struct ScriptNodeRecord *next_node;

	work_node = (struct ScriptNodeRecord *)(list->lh_Head);	/* first node */
	while(next_node = (struct ScriptNodeRecord *)(work_node->node.ln_Succ))
	{
		FreeANode(work_node);
		work_node = next_node;
	}
	if (list!=NULL)
		FreeMem(list, sizeof(struct List));	/* free list header */
	list=NULL;
}

/******** FreeANode() ********/
/* input:    pointer to a node structure.
 * output:   -
 * function: deallocates node and attached memory (if available)
 */

void FreeANode(struct ScriptNodeRecord *node)
{
int i;
struct ScriptEventRecord **serlist;
struct PageEventRecord *work_node;
struct PageEventRecord *next_node;

	/**** FREE LOCAL EVENTS ****/

	if (	node->nodeType == TALK_PAGE && node->list != NULL &&
				node->numericalArgs[15]==1 )
	{
		work_node = (struct PageEventRecord *)(node->list->lh_Head);

		if ( work_node != NULL )
		{
			serlist = (struct ScriptEventRecord **)work_node->er_LocalEvents;
			for(i=0; i<MAX_LOCAL_EVENTS; i++)
				if ( serlist[i] != NULL )
					FreeMem(serlist[i],sizeof(struct ScriptEventRecord));
		
			while(next_node = (struct PageEventRecord *)(work_node->er_Header.ph_Node.ln_Succ))
			{
				FreeMem(work_node, sizeof(struct PageEventRecord));
				work_node = next_node;
			}
		}

		if (node->list!=NULL)
			FreeMem(node->list, sizeof(struct List));

		//FreeMem(serlist, MAXEDITWINDOWS*sizeof(struct ScriptEventRecord *));
	}

	/**** FREE VARS ****/

	if ( node->nodeType==TALK_VARS )
	{
		if ( node->list )
		{
			RemoveAllVARS( node->list );
			FreeMem(node->list, sizeof(struct List));
		}
	}

	/**** FREE EXTRADATA ****/

	if (node->extraData!=NULL && node->extraDataSize>0)
		FreeMem(node->extraData, node->extraDataSize);

	/**** FREE NODE ****/

	FreeMem(node, sizeof(struct ScriptNodeRecord));
}

/******** AddNode() ********/
/* input:    pointer to a List structure.
 * output:   pointer to the added node or NULL on error.
 * function: allocates memory for a node, adds it to the list and cleans
 *           the node before anyone's using it.
 */

struct ScriptNodeRecord *AddNode(struct List *list)
{
struct ScriptNodeRecord *this_node;
int i;

	this_node = (struct ScriptNodeRecord *)AllocMem(	sizeof(struct ScriptNodeRecord),
																										MEMF_ANY | MEMF_CLEAR);
	if (this_node==NULL)
	{
		UA_WarnUser(94);
		return(NULL);
	}

	this_node->node.ln_Name 				= NULL;
	this_node->node.ln_Type 				= 100;	/* arbitrary type identifier */
	this_node->node.ln_Pri 					= 0;
	AddTail((struct List *)list, (struct Node *)this_node);

	this_node->list									= NULL;
	this_node->nodeType							= -1;

	for(i=0; i<MAX_PARSER_ARGS; i++)
		this_node->numericalArgs[i] = -1;

	for(i=0; i<MAX_PARSER_CHARS; i++)
		this_node->objectPath[i] = '\0';

	for(i=0; i<MAX_OBJECTNAME_CHARS; i++)
		this_node->objectName[i] = '\0';

	this_node->extraData						= NULL;
	this_node->extraDataSize				= NULL;
	this_node->duration							= -1;
	this_node->dayBits							= -1;

/*
	this_node->Start.HHMMSS.Days		= -1;
	this_node->Start.HHMMSS.Minutes	= -1;
	this_node->Start.HHMMSS.Ticks		= -1;
*/

	this_node->Start.TimeCode.HH		= -1;
	this_node->Start.TimeCode.MM		= -1;
	this_node->Start.TimeCode.SS		= -1;
	this_node->Start.TimeCode.FF		= -1;

/*
	this_node->End.HHMMSS.Days			= -1;
	this_node->End.HHMMSS.Minutes		= -1;
	this_node->End.HHMMSS.Ticks			= -1;
*/

	this_node->End.TimeCode.HH			= -1;
	this_node->End.TimeCode.MM			= -1;
	this_node->End.TimeCode.SS			= -1;
	this_node->End.TimeCode.FF			= -1;

	this_node->startendMode					= -1;
	/*this_node->effectNr							= -1;*/
	this_node->miscFlags						= 0;

	this_node->PageNr								= 0;
	this_node->ProcInfo							= NULL;
	this_node->ParentSNR						= NULL;

	return((struct ScriptNodeRecord *)this_node);
}

/******** InitScriptInfoRecord() ********/
/* input:    pointer to a ScriptInfoRecord.
 * output:   -
 * function: empties the complete SIR and makes it ready for use.
 */

void InitScriptInfoRecord(struct ScriptInfoRecord *SIR)
{
int i;

	SIR->currentList		= NULL;

	SIR->listType				= -1;

	SIR->lists = (struct List **)AllocMem((sizeof(struct List **)*CPrefs.MaxNumLists),
																				MEMF_ANY | MEMF_CLEAR);
	if (SIR->lists==NULL)
	{
		UA_WarnUser(95);
		return;
	}

	SIR->allLists = (struct List **)AllocMem(	(sizeof(struct List **)*CPrefs.MaxNumLists),
																						MEMF_ANY | MEMF_CLEAR);
	if (SIR->allLists==NULL)
	{
		UA_WarnUser(96);
		return;
	}

	SIR->scrollPos = (UWORD *)AllocMem(	(sizeof(UWORD)*CPrefs.MaxNumLists),
																			MEMF_ANY | MEMF_CLEAR);
	if (SIR->scrollPos==NULL)
	{
		UA_WarnUser(97);
		return;
	}

	SIR->currentNode		= NULL;

	SIR->version				=	0;
	SIR->revision				=	0;

	for(i=0; i<MAX_GLOBAL_EVENTS; i++)
		SIR->globallocalEvents[i] = NULL;

	SIR->timeCodeSource	= TIMESOURCE_INTERNAL;

	SIR->timeCodeFormat	= TIMEFORMAT_HHMMSS;

	if ( CPrefs.PlayerPalNtsc == NTSC_MODE)
		SIR->timeCodeRate	= TIMERATE_30FPS;
	else
		SIR->timeCodeRate	= TIMERATE_25FPS;

	SIR->Offset.TimeCode.HH = 0;
	SIR->Offset.TimeCode.MM = 0;
	SIR->Offset.TimeCode.SS = 0;
	SIR->Offset.TimeCode.FF = 0;

	SIR->timeCodeOut = FALSE;

	NewList(&SIR->VIList);
}

/******** FreeScriptInfoRecord() ********/
/* input:    pointer to a SIR structure.
 * output:   -
 * function: frees all memory consumed by global events structures,
 *           frees all lists by calling FreeScriptList which deallocates in
 *           its turn all attached nodes and associated memory.
 */

void FreeScriptInfoRecord(struct ScriptInfoRecord *SIR)
{
int i;

	/**** FREE VIRS ****/

	RemoveAllVIRS( &SIR->VIList );

	/**** FREE GLOBAL EVENTS ****/

	for(i=0; i<MAX_GLOBAL_EVENTS; i++)
	{
		if (SIR->globallocalEvents[i] != NULL)
		{
			FreeMem(SIR->globallocalEvents[i], sizeof(struct ScriptEventRecord) );
			SIR->globallocalEvents[i]=NULL;
		}
	}

	/**** FREE LISTS ****/

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] != NULL)
		{
			FreeScriptList(SIR->allLists[i]);
			SIR->allLists[i] = NULL;
		}
	}

	/**** FREE SIR ****/

	FreeMem(SIR->lists, (sizeof(struct List **)*CPrefs.MaxNumLists));
	FreeMem(SIR->allLists, (sizeof(struct List **)*CPrefs.MaxNumLists));
	FreeMem(SIR->scrollPos, (sizeof(UWORD)*CPrefs.MaxNumLists));
}

/******** CreateNewList() ********/
/* input:    pointer to a SIR structure.
 * output:   boolean
 * function: allocates and initializes a list structure, adds the list
 *           to the array of lists which is used for easy clean up at
 *           exit time.
 */

BOOL CreateNewList(struct ScriptInfoRecord *SIR)
{
int i;

	SIR->currentList = (struct List *)InitScriptList();
	if (SIR->currentList==NULL)
		return(FALSE);

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] == NULL)
		{
			SIR->allLists[i] = SIR->currentList;
			break;
		}
	}

	return(TRUE);
}

/******** ProcessGlobalEvents() ********/
/* input:    pointer to a SIR structure.
 * output:   -
 * function: links every global event with its respective node.
 */

void ProcessGlobalEvents(struct ScriptInfoRecord *SIR)
{
struct ScriptEventRecord *SER;
struct ScriptNodeRecord *node;
int i;

	for(i=0; i<MAX_GLOBAL_EVENTS; i++)
	{
		SER = SIR->globallocalEvents[i];
		if (SER != NULL)
		{
			node = (struct ScriptNodeRecord *)FindLabel(SIR, SER->labelName);
			if (node!=NULL)
			{
				SER->labelSNR = (struct ScriptNodeRecord *)node;
			}
		}
	}
}

/******** FindLabel() ********/
/* input:		pointer to a SIR structure, label name string
 * output:	pointer to a (label) node with matching label name.    
 */

struct ScriptNodeRecord *FindLabel(struct ScriptInfoRecord *SIR, STRPTR labelName)
{
int i;
struct ScriptNodeRecord *this_node;
struct List *list;

	if (labelName==NULL || labelName[0]=='\0')
		return(NULL);

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] != NULL)
		{
			list = SIR->allLists[i];

			if (list->lh_TailPred != (struct Node *)list)
			{
				for(this_node=(struct ScriptNodeRecord *)list->lh_Head;
						this_node->node.ln_Succ;
						this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
				{
					if (this_node->nodeType==TALK_LABEL ||
							this_node->nodeType==TALK_STARTSER ||
							this_node->nodeType==TALK_STARTPAR)
					{
						if ( stricmp(this_node->objectName, labelName)==0 )
							return( (struct ScriptNodeRecord *)this_node );
					}
				}
			}
		}
	}
	return(NULL);
}

/******** FindParent() ********/
/* input:		pointer to a SIR structure, pointer to old List
 * output:	pointer to parent list or NULL if not found
 */

struct List *FindParent(struct ScriptInfoRecord *SIR, struct List *oldList)
{
int i;
struct ScriptNodeRecord *this_node;
struct List *list;

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] != NULL)
		{
			list = SIR->allLists[i];

			if (list->lh_TailPred != (struct Node *)list)
			{
				for(this_node=(struct ScriptNodeRecord *)list->lh_Head;
						this_node->node.ln_Succ;
						this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
				{
					if ( this_node->list == oldList)
						return( list );
				}
			}
		}
	}
	return(NULL);
}

/******** FindParentType() ********/
/* input:		pointer to a SIR structure, pointer to old List
 * output:	int meaning STARTSER or STARTPAR
 */

int FindParentType(struct ScriptInfoRecord *SIR, struct List *oldList)
{
struct ScriptNodeRecord *this_node;

	this_node = FindParentNode(SIR,oldList);
	if ( this_node )
		return( (int)this_node->nodeType );

	return(-1);
}

/******** FindParentNode() ********/
/* input:		pointer to a SIR structure, pointer to old List
 * output:	int meaning STARTSER or STARTPAR
 */

struct ScriptNodeRecord *FindParentNode(struct ScriptInfoRecord *SIR, struct List *oldList)
{
int i;
struct ScriptNodeRecord *this_node;
struct List *list;

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] != NULL)
		{
			list = SIR->allLists[i];

			if (list->lh_TailPred != (struct Node *)list)
			{
				for(this_node=(struct ScriptNodeRecord *)list->lh_Head;
						this_node->node.ln_Succ;
						this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
				{
					if ( this_node->list == oldList)
						return( this_node );
				}
			}
		}
	}
	return(NULL);
}

/******** AllocateNode() ********/
/* input:    
 * output:   pointer to the added node or NULL on error.
 * function: allocates memory for a node and cleans
 *           the node before anyone's using it.
 */

struct ScriptNodeRecord *AllocateNode(void)
{
struct ScriptNodeRecord *this_node;
int i;

	this_node = (struct ScriptNodeRecord *)AllocMem(sizeof(struct ScriptNodeRecord),
																									MEMF_ANY | MEMF_CLEAR);
	if (this_node==NULL)
	{
		UA_WarnUser(98);
		return(NULL);
	}

	this_node->node.ln_Name 					= NULL;
	this_node->node.ln_Type 					= 100;	/* arbitrary type identifier */
	this_node->node.ln_Pri 						= 0;

	this_node->list										= NULL;
	this_node->nodeType								= -1;

	for(i=0; i<MAX_PARSER_ARGS; i++)
		this_node->numericalArgs[i] = -1;

	for(i=0; i<MAX_PARSER_CHARS; i++)
		this_node->objectPath[i] = '\0';

	for(i=0; i<MAX_OBJECTNAME_CHARS; i++)
		this_node->objectName[i] = '\0';

	this_node->extraData							= NULL;
	this_node->extraDataSize					= NULL;
	this_node->duration								= -1;
	this_node->dayBits								= -1;

/*
	this_node->Start.HHMMSS.Days			= -1;
	this_node->Start.HHMMSS.Minutes		= -1;
	this_node->Start.HHMMSS.Ticks			= -1;
*/

	this_node->Start.TimeCode.HH			= -1;
	this_node->Start.TimeCode.MM			= -1;
	this_node->Start.TimeCode.SS			= -1;
	this_node->Start.TimeCode.FF			= -1;

/*
	this_node->End.HHMMSS.Days				= -1;
	this_node->End.HHMMSS.Minutes			= -1;
	this_node->End.HHMMSS.Ticks				= -1;
*/

	this_node->End.TimeCode.HH				= -1;
	this_node->End.TimeCode.MM				= -1;
	this_node->End.TimeCode.SS				= -1;
	this_node->End.TimeCode.FF				= -1;

	this_node->startendMode						= -1;
	/*this_node->effectNr								= -1;*/
	this_node->miscFlags							= 0;

	this_node->PageNr									= NULL;
	this_node->ProcInfo								= NULL;
	this_node->ParentSNR							= NULL;

	return((struct ScriptNodeRecord *)this_node);
}

/******** InWhichListAreWe() ********/

int InWhichListAreWe(struct ScriptInfoRecord *SIR, struct List *list)
{
int i;

	for(i=0; i<CPrefs.MaxNumLists; i++)
		if ( SIR->allLists[i]	== list)
			return(i);

	return(-1);
}

/******** ChangeGotosAndGlobals() ********/

void ChangeGotosAndGlobals(struct ScriptInfoRecord *SIR, STRPTR objectName, STRPTR tempName)
{
int i,j;
struct ScriptNodeRecord *this_node;
struct ScriptEventRecord *SER;
struct List *list;
struct ScriptEventRecord *ser, **serlist;
struct PageEventRecord *per;

	/**** first change all objectNames of TALK_GOTO into tempName ****/

	if (tempName==NULL || tempName[0]=='\0')
		return;

	if (objectName==NULL || objectName[0]=='\0')
		return;

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] != NULL)
		{
			list = SIR->allLists[i];

			if (list->lh_TailPred != (struct Node *)list)
			{
				for(this_node=(struct ScriptNodeRecord *)list->lh_Head;
						this_node->node.ln_Succ;
						this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
				{
					/**** change GOTO ****/

					if (this_node->nodeType==TALK_GOTO)
					{
						if ( stricmp(this_node->objectName, objectName)==0 )
						{
							stccpy(this_node->objectName, tempName, MAX_OBJECTNAME_CHARS);
							SetByteBit(&(this_node->miscFlags), OBJ_NEEDS_REFRESH);
						}
					}

					/**** change button names ****/

					if (	this_node->nodeType==TALK_PAGE &&
								this_node->numericalArgs[15]==1 &&
								this_node->list!=NULL )
					{
						j=0;
						per = (struct PageEventRecord *)(this_node->list->lh_Head);
						serlist = (struct ScriptEventRecord **)per->er_LocalEvents;
						while( (serlist[j] != NULL) && (j < MAX_LOCAL_EVENTS) )
						{
							ser = (struct ScriptEventRecord *)serlist[j];						
							if ( stricmp(ser->labelName, objectName)==0 )
								strcpy(ser->labelName, tempName);
							j++;
						}
					}
				}
			}
		}
	}

	/**** change all labelNames in SER into tempName (global events) ****/

	for(i=0; i<MAX_GLOBAL_EVENTS; i++)
	{
		SER = SIR->globallocalEvents[i];
		if ( SER != NULL)
		{
			if ( stricmp(SER->labelName, objectName)==0 )
				stccpy(SER->labelName, tempName, MAX_PARSER_CHARS);
		}
	}
}

/******** CountNumSelected() ********/

struct ScriptNodeRecord *CountNumSelected(struct ScriptNodeRecord *first_node, int *numSelected)
{
struct ScriptNodeRecord *count_node, *last_node=NULL;

	*numSelected=0;
	for(count_node=(struct ScriptNodeRecord *)first_node;
			count_node->node.ln_Succ;
			count_node=(struct ScriptNodeRecord *)count_node->node.ln_Succ)
	{
		if ( count_node->miscFlags & OBJ_SELECTED )
		{
			last_node = count_node;
			*numSelected = *numSelected + 1;
		}
	}

	return(last_node);
}

/******** ValidateSER() ********/
/*
 * returns TRUE if dangling SERs have been removed
 *
 */

BOOL ValidateSER(struct ScriptInfoRecord *SIR, BOOL convert, BOOL quiet)
{
int i,j;
SNRPTR this_node;
struct ScriptEventRecord *SER;
struct List *list;
BOOL globlabList[MAX_GLOBAL_EVENTS];
BOOL warned=FALSE;

	ProcessGlobalEvents(SIR);	// last attempt to collect cut/pasted/cleared SNR

 /**** create a boolean list with TRUE if a SER exists and FALSE if not ****/
 /**** after searched for matching labels, delete all TRUE s because    ****/
 /**** the SER points to a no longer existing SNR                       ****/

	for(i=0; i<MAX_GLOBAL_EVENTS; i++)
	{
		SER = (struct ScriptEventRecord *)ObjectRecord.scriptSIR.globallocalEvents[i];
		if ( SER != NULL )
			globlabList[i] = TRUE;	// free me
		else
			globlabList[i] = FALSE;
	}

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] != NULL)
		{
			list = SIR->allLists[i];
			if (list->lh_TailPred != (struct Node *)list)
			{
				for(this_node=(SNRPTR)list->lh_Head; this_node->node.ln_Succ;
						this_node=(SNRPTR)this_node->node.ln_Succ)
				{
					if (this_node->nodeType == TALK_LABEL ||
							this_node->nodeType == TALK_STARTSER ||
							this_node->nodeType == TALK_STARTPAR )
					{
						for(j=0; j<MAX_GLOBAL_EVENTS; j++)
						{
							SER = (struct ScriptEventRecord *)ObjectRecord.scriptSIR.globallocalEvents[j];
							if ( SER && SER->labelSNR==this_node )
								globlabList[j]=FALSE;	// don't free this one
						}
					}
					else if (convert)
					{
						if ( this_node->dayBits	== -1 )
							this_node->dayBits = 127;

						if ( this_node->Start.TimeCode.HH == -1 )
							this_node->Start.TimeCode.HH = 0;
						if ( this_node->Start.TimeCode.MM == -1 )
							this_node->Start.TimeCode.MM = 0;
						if ( this_node->Start.TimeCode.SS == -1 )
							this_node->Start.TimeCode.SS = 17;
						if ( this_node->Start.TimeCode.FF == -1 )
							this_node->Start.TimeCode.FF = 31;

						if ( this_node->End.TimeCode.HH == -1 )
							this_node->End.TimeCode.HH = 0;
						if ( this_node->End.TimeCode.MM == -1 )
							this_node->End.TimeCode.MM = 0;
						if ( this_node->End.TimeCode.SS == -1 )
							this_node->End.TimeCode.SS = 29;
						if ( this_node->End.TimeCode.FF == -1 )
							this_node->End.TimeCode.FF = -11;

						if ( this_node->nodeType == TALK_VARS )
							this_node->duration=0;
					}
				}
			}
		}
	}

	/**** now remove 'dangling' SER ptrs ****/

	for(i=0; i<MAX_GLOBAL_EVENTS; i++)
	{
		if ( globlabList[i] )
		{
			SER = (struct ScriptEventRecord *)ObjectRecord.scriptSIR.globallocalEvents[i];
			if (SER)
			{
				if (!warned && !quiet)
				{
					warned=TRUE;
					if ( !UA_OpenGenericWindow(	scriptWindow, TRUE,TRUE, msgs[Msg_OK-1],
																			msgs[Msg_Cancel-1], QUESTION_ICON,
																			msgs[Msg_DeleteGlobalEvent-1], TRUE, NULL) )
						return(FALSE);
				}
				FreeMem(SER, sizeof(struct ScriptEventRecord) );
				ObjectRecord.scriptSIR.globallocalEvents[i] = NULL;
			}
		}
	}	

	return(TRUE);
}

/******** UpdateDeferCont() ********/

void UpdateDeferCont(struct ScriptInfoRecord *SIR)
{
int i;
SNRPTR this_node;
struct List *list;
WORD deferCont;

	if (CPrefs.scriptTiming==0)
		deferCont = ARGUMENT_DEFER;
	else
		deferCont = ARGUMENT_CONTINUE;

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] != NULL)
		{
			list = SIR->allLists[i];
			if (list->lh_TailPred != (struct Node *)list)
			{
				for(this_node=(SNRPTR)list->lh_Head; this_node->node.ln_Succ;
						this_node=(SNRPTR)this_node->node.ln_Succ)
				{
					if (	this_node->nodeType == TALK_ANIM ||
								this_node->nodeType == TALK_PAGE	)
						this_node->numericalArgs[0] = (WORD)deferCont;
				}
			}
		}
	}
}

/******** FillInLabelPointers() ********/

void FillInLabelPointers(struct ScriptInfoRecord *SIR)
{
int i;
SNRPTR this_node;
struct List *list, *list2;
VAR *work_var, *next_var;
VCR *work_vcr, *next_vcr;

	for(i=1; i<CPrefs.MaxNumLists; i++)	// skip root
	{
		if (SIR->allLists[i] != NULL)
		{
			list = SIR->allLists[i];
			if (list->lh_TailPred != (struct Node *)list)
			{
				for(this_node=(SNRPTR)list->lh_Head; this_node->node.ln_Succ;
						this_node=(SNRPTR)this_node->node.ln_Succ)
				{
					if ( this_node->nodeType == TALK_VARS )
					{
						list2 = this_node->list;
						if ( list2->lh_TailPred != (struct Node *)list2 )
						{
							work_var = (VAR *)list2->lh_Head;	// first node
							while( next_var = (VAR *)(work_var->var_Node.ln_Succ) )
							{
								if (	work_var->var_CondList.lh_TailPred !=
											(struct Node *)&(work_var->var_CondList) )
								{
									work_vcr = (VCR *)work_var->var_CondList.lh_Head;	// first node
									while( next_vcr = (VCR *)(work_vcr->vcr_Node.ln_Succ) )
									{		
										work_vcr->vcr_LabelSNR = FindLabel(SIR,work_vcr->vcr_LabelName);
										if ( !work_vcr->vcr_LabelSNR )
											work_vcr->vcr_LabelSNR = FindLabel(SIR,mainName);
										work_vcr = next_vcr;
									}
								}
								work_var = next_var;
							}
						}
					}
				}
			}
		}
	}
}

/******** CheckScriptProblems() ********/

BOOL CheckScriptProblems(struct ScriptInfoRecord *SIR)
{
int i;
SNRPTR this_node;
struct List *list;
BOOL ARexxInScript=FALSE;
BOOL NewScriptInScript=FALSE;

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] != NULL)
		{
			list = SIR->allLists[i];
			if (list->lh_TailPred != (struct Node *)list)
			{
				for(this_node=(SNRPTR)list->lh_Head; this_node->node.ln_Succ;
						this_node=(SNRPTR)this_node->node.ln_Succ)
				{
					if ( this_node->nodeType == TALK_AREXX )
						ARexxInScript=TRUE;

					if ( this_node->nodeType == TALK_USERAPPLIC )
					{
						if ( !strcmpi(this_node->objectPath,"NewScript") )
							NewScriptInScript=TRUE;
					}
				}
			}
		}
	}

	if ( ARexxInScript && !FindPort("AREXX") )
	{
		Message( msgs[Msg_NoARexx-1] );
		return(FALSE);
	}

	if ( NewScriptInScript && scriptDoc.modified )
	{
		Message( msgs[Msg_FirstSave-1] );
		return(FALSE);
	}

	return(TRUE);
}

/******** IsNodeTypePresent() ********/

struct ScriptNodeRecord *IsNodeTypePresent(struct ScriptInfoRecord *SIR, int type)
{
int i;
struct ScriptNodeRecord *this_node;
struct List *list;

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i])
		{
			list = SIR->allLists[i];
			if (list->lh_TailPred != (struct Node *)list)
			{
				for(this_node=(struct ScriptNodeRecord *)list->lh_Head;
						this_node->node.ln_Succ;
						this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
				{
					if ( this_node->nodeType==type )
							return( (struct ScriptNodeRecord *)this_node );
				}
			}
		}
	}
	return(NULL);
}

/******** E O F ********/
