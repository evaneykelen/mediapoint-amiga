#include "nb:pre.h"

/**** externals ****/

extern struct List **clipLists;
extern struct List **undoLists;
extern struct ObjectInfo ObjectRecord;
extern struct MenuRecord **script_MR;
extern int *arrayPos;
extern struct List **newPtrs;
extern struct CapsPrefs CPrefs;
extern ULONG allocFlags;
extern BOOL EditMenuStates[];
extern int *SNRlist;
extern ULONG SNRlistSize;
extern struct List *undoListPtr;
extern struct Window *scriptWindow;
extern struct Library *medialinkLibBase;
extern UBYTE **msgs;   

/**** gadgets ****/

extern struct GadgetRecord Script_GR[];

/**** functions ****/

/******** CopyScriptObjects() ********/

void CopyScriptObjects(	struct ScriptInfoRecord *SIR,
												struct ScriptNodeRecord *first_node)
{
int numSelected;

	/**** mark all selected objects and mark objects in selected branches ****/

	numSelected = SelectCurrentAndChildren(SIR, first_node);
	if (numSelected==0)
		return;

	/**** free current objects in clipboard ****/

	FreeClipUndoLists(undoLists);

	FreeClipUndoLists(clipLists);

	/**** copy all objects marked with OBJ_TO_CLIP to the clipboard ****/

	if (!(CopyToClipboard(SIR, clipLists)))
	{
		UA_WarnUser(163);
		return;
	}

	/**** update pointers ****/

	UpdatePointers(SIR, clipLists);

	/**** clean original lists from OBJ_TO_CLIP ****/

	CleanScriptBits(SIR);

	/**** set menu item(s) ****/

	EnableMenu(script_MR[EDIT_MENU], EDIT_PASTE);
}

/******** CutScriptObjects() ********/

void CutScriptObjects(struct ScriptInfoRecord *SIR,
											struct ScriptNodeRecord *first_node)
{
struct ScriptNodeRecord *work_node;
struct ScriptNodeRecord *next_node;
int i, numSelected;

	/**** mark all selected objects and mark objects in selected branches ****/

	numSelected = SelectCurrentAndChildren(SIR, first_node);
	if (numSelected==0)
		return;

	/**** free current objects in clipboard ****/

	FreeClipUndoLists(undoLists);

	FreeClipUndoLists(clipLists);

	/**** copy all objects marked with OBJ_TO_CLIP to the clipboard ****/

	if (!(CopyToClipboard(SIR, clipLists)))
	{
		UA_WarnUser(164);
		return;
	}

	/**** update pointers ****/

	UpdatePointers(SIR, clipLists);

	/**** remove marked nodes from current list ****/

	work_node = (struct ScriptNodeRecord *)first_node;	/* first node */
	while(next_node = (struct ScriptNodeRecord *)(work_node->node.ln_Succ))
	{
		if (work_node->miscFlags & OBJ_TO_CLIP)
		{
			if (work_node == (struct ScriptNodeRecord *)ObjectRecord.objList->lh_Head) /* head node */
			{
				RemHead(ObjectRecord.objList);
				ObjectRecord.firstObject = (struct ScriptNodeRecord *)ObjectRecord.objList->lh_Head;
			}
			else if (work_node == (struct ScriptNodeRecord *)ObjectRecord.objList->lh_TailPred) /* tail node */
				RemTail(ObjectRecord.objList);
			else
				Remove((struct Node *)work_node);

			//RemoveNodeFromGE(work_node);

			FreeANode(work_node);
		}
		work_node = next_node;
	}

	/**** delete all marked sub branches ****/

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] != NULL &&
				SIR->allLists[i]->lh_Type & OBJ_TO_CLIP)
		{
			FreeScriptList(SIR->allLists[i]);
			SIR->allLists[i] = NULL;
		}
	}

	/**** set menu item(s) ****/

	GetNumObjects();
}

/******** ClearScriptObjects() ********/

void ClearScriptObjects(struct ScriptInfoRecord *SIR,
												struct ScriptNodeRecord *first_node)
{
struct ScriptNodeRecord *work_node;
struct ScriptNodeRecord *next_node;
int i,j,pos;
ULONG preSize;
int numUndoObjects;
int numSelected;

	if ( ObjectRecord.objList == ObjectRecord.scriptSIR.allLists[0] )
	{
		if (ObjectRecord.numObjects<2)
			return;

		for(work_node=(struct ScriptNodeRecord *)first_node;
				work_node->node.ln_Succ;
				work_node=(struct ScriptNodeRecord *)work_node->node.ln_Succ)
		{
			if ( (work_node->miscFlags & OBJ_SELECTED) &&
						work_node->nodeType==TALK_STARTSER)
				work_node->miscFlags = 0;

			if ( work_node->nodeType==TALK_VARS && work_node->miscFlags & OBJ_SELECTED )
			{
				if ( !UA_OpenGenericWindow(	scriptWindow, TRUE,TRUE, msgs[Msg_OK-1],
																		msgs[Msg_Cancel-1], EXCLAMATION_ICON,
																		msgs[Msg_KillDeclas-1], TRUE, NULL) )
				{
					work_node->miscFlags = 0;
				}
			}
		}
	}

	/**** mark all selected objects and mark objects in selected branches ****/

	numSelected = SelectCurrentAndChildren(SIR, first_node);
	if (numSelected==0)
		return;

	/**** free current objects in UNDO clipboard ****/

	FreeClipUndoLists(undoLists);

	/**** copy all objects marked with OBJ_TO_CLIP to the UNDO clipboard ****/

	if (!(CopyToClipboard(SIR, undoLists)))
	{
		UA_WarnUser(165);
		return;
	}

	/**** update pointers ****/

	UpdatePointers(SIR, undoLists);

	/**** count no. of nodes in root list of undo lists ****/

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if ( undoLists[i] != NULL )
		{
			pos=i;
			break;
		}
	}

	numUndoObjects=0;
	for(work_node=(struct ScriptNodeRecord *)undoLists[pos]->lh_Head;
			work_node->node.ln_Succ;
			work_node=(struct ScriptNodeRecord *)work_node->node.ln_Succ)
		numUndoObjects++;

	/**** only alloc new list if new list is larger ****/

	preSize = numUndoObjects*sizeof(int);

	if ( TestBit(allocFlags, SMALLNODES_FLAG) && preSize > SNRlistSize )
	{
		FreeMem(SNRlist, SNRlistSize);
		UnSetBit(&allocFlags, SMALLNODES_FLAG);

		SNRlistSize = preSize;
		SNRlist = (int *)AllocMem(SNRlistSize, MEMF_PUBLIC | MEMF_CLEAR);
		if (SNRlist==NULL)
		{
			UA_WarnUser(166);
			return;
		}
		SetBit(&allocFlags, SMALLNODES_FLAG);
	}
	else
	{
		SNRlistSize = preSize;
		SNRlist = (int *)AllocMem(SNRlistSize, MEMF_PUBLIC | MEMF_CLEAR);
		if (SNRlist==NULL)
		{
			UA_WarnUser(167);
			return;
		}
		SetBit(&allocFlags, SMALLNODES_FLAG);
	}

	undoListPtr = (struct List *)ObjectRecord.objList;

	/**** remove marked nodes from current list ****/

	i=0;
	j=0;
	work_node = (struct ScriptNodeRecord *)first_node;	/* first node */
	while(next_node = (struct ScriptNodeRecord *)(work_node->node.ln_Succ))
	{
		if (work_node->miscFlags & OBJ_TO_CLIP)
		{
			SNRlist[i] = j;
			i++;

			/**** delete nodes ****/

			if (work_node == (struct ScriptNodeRecord *)ObjectRecord.objList->lh_Head) /* head node */
			{
				RemHead(ObjectRecord.objList);
				ObjectRecord.firstObject = (struct ScriptNodeRecord *)ObjectRecord.objList->lh_Head;
			}
			else if (work_node == (struct ScriptNodeRecord *)ObjectRecord.objList->lh_TailPred) /* tail node */
				RemTail(ObjectRecord.objList);
			else
				Remove((struct Node *)work_node);

			/**** remove all global key assignments ****/

			if ( work_node->nodeType == TALK_GLOBALEVENT )
			{
				for(i=0; i<MAX_GLOBAL_EVENTS; i++)
				{
					if (SIR->globallocalEvents[i] != NULL)
					{
						FreeMem(SIR->globallocalEvents[i], sizeof(struct ScriptEventRecord) );
						SIR->globallocalEvents[i]=NULL;
					}
				}
			}

			/**** re-enable tools that may only appear once in the root ****/

			if (work_node->nodeType == TALK_GLOBALEVENT ||
					work_node->nodeType == TALK_TIMECODE ||
					work_node->nodeType == TALK_VARS ||
					work_node->nodeType == TALK_INPUTSETTINGS )
			{
				EnableTool(work_node->nodeType);
				ShowToolIcons(scriptWindow,-2);
			}

			if (	(ObjectRecord.objList == ObjectRecord.scriptSIR.allLists[0]) &&
						work_node->nodeType == TALK_VARS )
				KillAllVars(SIR);

			FreeANode(work_node);
		}
		work_node = next_node;
		j++;
	}

	/**** delete all marked sub branches ****/

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] != NULL &&
				SIR->allLists[i]->lh_Type & OBJ_TO_CLIP)
		{
			FreeScriptList(SIR->allLists[i]);
			SIR->allLists[i] = NULL;
		}
	}

	/**** NEW: skip first object if in root ****/

	if ( ObjectRecord.objList == ObjectRecord.scriptSIR.allLists[0] ) /* root encountered */
	{
		work_node = (struct ScriptNodeRecord *)ObjectRecord.objList->lh_Head;
		work_node = (struct ScriptNodeRecord *)work_node->node.ln_Succ;
		ObjectRecord.firstObject = (struct ScriptNodeRecord *)work_node;
	}
	else
		EnableMenu(script_MR[EDIT_MENU], EDIT_UNDO);

	/**** set menu item(s) ****/

	GetNumObjects();

	if (ObjectRecord.numObjects==0)
		DisableMenu(script_MR[EDIT_MENU], EDIT_SELECTALL);
}

/******** SelectCurrentAndChildren() ********/

int SelectCurrentAndChildren(	struct ScriptInfoRecord *SIR,
															struct ScriptNodeRecord *first_node)
{
struct ScriptNodeRecord *count_node;
int numSelected=0;

	for(count_node=(struct ScriptNodeRecord *)first_node;
			count_node->node.ln_Succ;
			count_node=(struct ScriptNodeRecord *)count_node->node.ln_Succ)
	{
		if (count_node->list != NULL &&
				(count_node->nodeType==TALK_STARTSER || count_node->nodeType==TALK_STARTPAR) &&
				count_node->miscFlags & OBJ_SELECTED )	/* sub branch */
		{
			SetByteBit(&count_node->miscFlags, OBJ_TO_CLIP);
			SetByteBit(&(count_node->list->lh_Type), OBJ_TO_CLIP);
			SelectBranch(SIR, count_node->list);
			numSelected++;
		}
		else if (count_node->miscFlags & OBJ_SELECTED)
		{
			SetByteBit(&count_node->miscFlags, OBJ_TO_CLIP);
			numSelected++;
		}
	}
	return(numSelected);
}

/******** SelectBranch() ********/

void SelectBranch(struct ScriptInfoRecord *SIR, struct List *list)
{
struct ScriptNodeRecord *this_node, *start_node;
int pos;
BOOL newList;
struct List *mylist;

	if (list->lh_TailPred == (struct Node *)list)	/* list is empty */
	{
		SetByteBit(&list->lh_Type, OBJ_TO_CLIP);
		return;
	}

	pos=0;
	start_node = (struct ScriptNodeRecord *)list->lh_Head;
	while(1)
	{
		newList=FALSE;
		for(this_node=(struct ScriptNodeRecord *)start_node;
				this_node->node.ln_Succ;
				this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
		{
			if (this_node->list != NULL &&
					(this_node->nodeType==TALK_STARTSER || this_node->nodeType==TALK_STARTPAR))
			{
				SIR->lists[pos] = (struct List *)this_node->node.ln_Succ;
				pos++;
				start_node = (struct ScriptNodeRecord *)this_node->list->lh_Head;
				newList=TRUE;
				mylist = (struct List *)this_node->list;
				SetByteBit(&mylist->lh_Type, OBJ_TO_CLIP);
				break;
			}
		}
		if (!newList)
		{
			if (pos>0)
			{
				pos--;
				start_node = (struct ScriptNodeRecord *)SIR->lists[pos];
			}
			else
				return;	/* READY */
		}
	}
}

/******** FreeClipUndoLists() ********/

void FreeClipUndoLists(struct List **list)
{
int i;

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (list[i] != NULL)
		{
			FreeScriptList(list[i]);
			list[i] = NULL;
		}
	}
}

/******** CopyToClipboard() ********/

BOOL CopyToClipboard(struct ScriptInfoRecord *SIR, struct List **list)
{
int i;
struct ScriptNodeRecord *this_node, *new_node;
struct List *this_list;
BOOL listReady, copyThisList;

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] != NULL)
		{
			this_list = SIR->allLists[i];

			listReady = FALSE;
			copyThisList = FALSE;

			if (this_list->lh_Type & OBJ_TO_CLIP)
			{
				listReady = TRUE;
				copyThisList = TRUE;
				list[i] = (struct List *)InitScriptList();
			}

			if (this_list->lh_TailPred != (struct Node *)this_list)	/* non empty */
			{
				for(this_node=(struct ScriptNodeRecord *)this_list->lh_Head;
						this_node->node.ln_Succ;
						this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
				{
					if (copyThisList || this_node->miscFlags & OBJ_TO_CLIP)
					{
						if (!listReady)
						{
							listReady=TRUE;
							list[i] = (struct List *)InitScriptList();
						}

						new_node = (struct ScriptNodeRecord *)AllocMem(sizeof(struct ScriptNodeRecord), MEMF_PUBLIC|MEMF_CLEAR);
						if (new_node==NULL)
							return(FALSE);

						CopyMem(this_node, new_node, sizeof(struct ScriptNodeRecord));

						/**** if node carries around extraData, copy that too ****/
						/**** look also to extraDataSize 'cause GOTO only uses ptr ****/

						if (this_node->extraData != NULL && this_node->extraDataSize>0)
						{
							new_node->extraData = (UBYTE *)AllocMem(this_node->extraDataSize, MEMF_PUBLIC | MEMF_CLEAR);
							if (new_node->extraData==NULL)
								return(FALSE);
							CopyMem(this_node->extraData,new_node->extraData,this_node->extraDataSize);
						}

						if (	this_node->nodeType==TALK_PAGE && this_node->numericalArgs[15]==1 &&
									this_node->list!=NULL )
							Copy_LE_Info(this_node, new_node);

						if (	this_node->nodeType==TALK_VARS )
							Copy_Vars_Info(this_node, new_node);

						new_node->node.ln_Name = NULL;
						new_node->node.ln_Type = 100;	/* arbitrary type identifier */
						new_node->node.ln_Pri = 0;

						AddTail((struct List *)list[i], (struct Node *)new_node);
					}
				}
			}
		}
	}

	return(TRUE);
}

/******** UpdatePointers() ********/

void UpdatePointers(struct ScriptInfoRecord *SIR, struct List **list)
{
struct ScriptNodeRecord *count_node;
int i, j, where;

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if ( list[i] != NULL )	/* clipLists[] */
		{
			for(count_node=(struct ScriptNodeRecord *)list[i]->lh_Head;
					count_node->node.ln_Succ;
					count_node=(struct ScriptNodeRecord *)count_node->node.ln_Succ)
			{
				if (count_node->list != NULL &&
						(count_node->nodeType==TALK_STARTSER || count_node->nodeType==TALK_STARTPAR) )
				{
					where=-1;
					for(j=0; j<CPrefs.MaxNumLists; j++)
					{
						if ( SIR->allLists[j]	== count_node->list )
						{
							where = j;
							break;
						}
					}
					if (where!=-1)
						count_node->list = list[where];
				}
			}
		}
	}	
}

/******** CleanScriptBits() ********/

void CleanScriptBits(struct ScriptInfoRecord *SIR)
{
int i;
struct ScriptNodeRecord *this_node;
struct List *this_list;

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] != NULL)
		{
			this_list = SIR->allLists[i];
			if (this_list->lh_Type & OBJ_TO_CLIP)
				this_list->lh_Type = 0;
			else if (this_list->lh_TailPred != (struct Node *)this_list)	/* non empty */
			{
				for(this_node=(struct ScriptNodeRecord *)this_list->lh_Head;
						this_node->node.ln_Succ;
						this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
				{
					if (this_node->miscFlags & OBJ_TO_CLIP)
						UnSetByteBit(&this_node->miscFlags, OBJ_TO_CLIP);
				}
			}
		}
	}
}

/******** PasteClipboard() ********/

void PasteClipboard(struct ScriptInfoRecord *SIR, int top, int start_x, int start_y)
{
struct ScriptNodeRecord *this_node;
struct ScriptNodeRecord *new_node;
struct ScriptNodeRecord *after_node;
int i, j, row, where, numClipLists, pos;

	if (ObjectRecord.objList == NULL)
		return;

	FreeClipUndoLists(undoLists);

	/**** calculate which object POS we clicked on *****/

	this_node=NULL;
	new_node=NULL;
	where=-1;

	if ( (start_y < Script_GR[0].y1) || ObjectRecord.numObjects==0)
	{
		this_node = NULL;
		where = ADD_TO_HEAD;
	}
	else
	{
		start_y += ( (ICONHEIGHT/2)-11 );
		row = ( ( (start_y - Script_GR[0].y1) / 20 ) + top );

		if (row<ObjectRecord.numObjects)
		{
			/**** with the object POS, find the object NODE ****/

			if (ObjectRecord.objList->lh_TailPred == (struct Node *)ObjectRecord.objList)
				return;

			this_node=(struct ScriptNodeRecord *)ObjectRecord.firstObject;
	
			if (row>0)
			{
				for(i=0; i<row; i++)
				{
					if (this_node!=NULL)
						this_node = (struct ScriptNodeRecord *)(this_node->node.ln_Succ);
				}
			}

			if (this_node==NULL ||
					this_node==(struct ScriptNodeRecord *)ObjectRecord.objList->lh_TailPred)
				where = ADD_TO_TAIL;
			else
				where = ADD_TO_MIDDLE;
		}
		else
			where = ADD_TO_TAIL;
	}

	if (where==-1)
		UA_WarnUser(168);

	after_node = this_node;

	/**** do pre-processing on clipboard ****/

	/**** count number of lists in clipboard ****/

	numClipLists=0;
	for(i=0; i<CPrefs.MaxNumLists; i++)
		if ( clipLists[i] != NULL )
			numClipLists++;

	/**** store in arrayPos the empty allLists positions ****/

	if (numClipLists > 1)
	{
		j=0;
		for(i=0; i<CPrefs.MaxNumLists; i++)
		{
			if (SIR->allLists[i]==NULL)
			{
				*(arrayPos+j) = i;
				j++;
				if (j==numClipLists)
					break;
			}
		}

		/**** prepare new allLists entries ****/

		i=0;
		do
		{
			SIR->allLists[ *(arrayPos+i) ] = InitScriptList();
			newPtrs[i] = SIR->allLists[ *(arrayPos+i) ];
			i++;
		} while(i< (numClipLists-1) );
	}

	/**** insert (add) COPIES of all clipLists[0] nodes ****/

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if ( clipLists[i] != NULL )
		{
			pos=i;
			break;
		}
	}

	for(this_node=(struct ScriptNodeRecord *)clipLists[pos]->lh_Head;
			this_node->node.ln_Succ;
			this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
	{
		new_node = (struct ScriptNodeRecord *)AllocMem(sizeof(struct ScriptNodeRecord), MEMF_PUBLIC|MEMF_CLEAR);
		if (new_node==NULL)
		{
			UA_WarnUser(169);
			return;
		}	

		CopyMem(this_node, new_node, sizeof(struct ScriptNodeRecord));

		/**** if node carries around extraData, copy that too ****/

		if (this_node->extraData != NULL && this_node->extraDataSize>0)
		{
			new_node->extraData = (UBYTE *)AllocMem(this_node->extraDataSize, MEMF_PUBLIC | MEMF_CLEAR);
			if (new_node->extraData==NULL)
			{
				UA_WarnUser(170);
				return;
			}	
			CopyMem(this_node->extraData,new_node->extraData,this_node->extraDataSize);
		}

		if (	this_node->nodeType==TALK_PAGE && this_node->numericalArgs[15]==1 &&
					this_node->list!=NULL )
			Copy_LE_Info(this_node, new_node);

		if (	this_node->nodeType==TALK_VARS )
			Copy_Vars_Info(this_node, new_node);

		new_node->miscFlags = 0;

		if (new_node->nodeType == TALK_LABEL ||
				new_node->nodeType == TALK_STARTSER ||
				new_node->nodeType == TALK_STARTPAR)
			CheckNameUniqueness(SIR, new_node);

		if ( where == ADD_TO_HEAD )
		{
			AddHead((struct List *)ObjectRecord.objList, (struct Node *)new_node);
			where = ADD_TO_MIDDLE;	/* else all objects are placed at head */
			after_node = new_node;

			/**** keep list scroller happy ****/
			ObjectRecord.firstObject = new_node;
		}
		else if ( where == ADD_TO_MIDDLE )
		{
			Insert(	(struct List *)ObjectRecord.objList, (struct Node *)new_node,
							(struct Node *)after_node);
			after_node = new_node;
		}
		else if ( where == ADD_TO_TAIL )
		{
			AddTail((struct List *)ObjectRecord.objList, (struct Node *)new_node);
		}
	}

	/**** add COPIES of clipLists to free allLists[] ****/

	if (numClipLists > 1)
	{
		for(i=pos+1, j=0; i<CPrefs.MaxNumLists; i++)
		{
			if (clipLists[i] != NULL &&
					clipLists[i]->lh_TailPred != (struct Node *)clipLists[i]) /* non empty */
			{
				/**** add all nodes ****/
	
				for(this_node=(struct ScriptNodeRecord *)clipLists[i]->lh_Head;
						this_node->node.ln_Succ;
						this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
				{
					new_node = (struct ScriptNodeRecord *)AllocMem(sizeof(struct ScriptNodeRecord), MEMF_PUBLIC|MEMF_CLEAR);
					if (new_node==NULL)
					{
						UA_WarnUser(171);
						return;
					}	
	
					CopyMem(this_node, new_node, sizeof(struct ScriptNodeRecord));
	
					/**** if node carries around extraData, copy that too ****/
	
					if (this_node->extraData != NULL && this_node->extraDataSize>0)
					{
						new_node->extraData = (UBYTE *)AllocMem(this_node->extraDataSize, MEMF_PUBLIC | MEMF_CLEAR);
						if (new_node->extraData==NULL)
						{
							UA_WarnUser(172);
							return;
						}	
						CopyMem(this_node->extraData,new_node->extraData,this_node->extraDataSize);
					}

					if (	this_node->nodeType==TALK_PAGE && this_node->numericalArgs[15]==1 &&
								this_node->list!=NULL )
						Copy_LE_Info(this_node, new_node);

#if 0
					if (	this_node->nodeType==TALK_PAGE && this_node->numericalArgs[15]==1 &&
								this_node->list!=NULL )
						RelinkButtons(new_node);
#endif
	
					if (	this_node->nodeType==TALK_VARS )
						Copy_Vars_Info(this_node, new_node);

					new_node->miscFlags = 0;
	
					if (new_node->nodeType == TALK_LABEL ||
							new_node->nodeType == TALK_STARTSER ||
							new_node->nodeType == TALK_STARTPAR)
						CheckNameUniqueness(SIR, new_node);
	
					AddTail((struct List *)SIR->allLists[ *(arrayPos+j) ], (struct Node *)new_node);
				}
	
				j++;
			}
		}
	
		UpdateAllListsPointers(SIR, newPtrs, clipLists);
	}

	GetNumObjects();
}

/******** CheckNameUniqueness() ********/

void CheckNameUniqueness(	struct ScriptInfoRecord *SIR,
													struct ScriptNodeRecord *this_node)
{
struct ScriptNodeRecord *found_node=NULL;
TEXT newName[41];

	while(1)
	{
		found_node = FindLabel(SIR, this_node->objectName);
		if (found_node!=NULL)
		{
			BumpRevision(newName, this_node->objectName);
			stccpy(this_node->objectName, newName, MAX_OBJECTNAME_CHARS);
		}
		else
			break;
	}
}

/******** UpdateClipboardPointers() ********/

void UpdateClipboardPointers(	struct ScriptInfoRecord *SIR,
															struct List **list,
															struct List **listPtrs)
{
struct ScriptNodeRecord *count_node;
struct List *this_list;
int j;

	j=0;
	for(count_node=(struct ScriptNodeRecord *)list[0]->lh_Head;
			count_node->node.ln_Succ;
			count_node=(struct ScriptNodeRecord *)count_node->node.ln_Succ)
	{
		if (count_node->list != NULL &&
				(count_node->nodeType==TALK_STARTSER || count_node->nodeType==TALK_STARTPAR))
		{
			this_list = count_node->list;
			count_node->list = listPtrs[j];
			j++;
			UpdateClipboardBranch(SIR, this_list, &j, listPtrs);
		}
	}
}

/******** UpdateClipboardBranch() ********/

void UpdateClipboardBranch(	struct ScriptInfoRecord *SIR,
														struct List *list, int *j,
														struct List **listPtrs)
{
BOOL newList;
int pos;
struct ScriptNodeRecord *this_node, *start_node;

	if (list->lh_TailPred == (struct Node *)list)	/* list is empty */
		return;

	pos=0;
	start_node = (struct ScriptNodeRecord *)list->lh_Head;

	while(1)
	{
		newList=FALSE;
		for(this_node=(struct ScriptNodeRecord *)start_node;
				this_node->node.ln_Succ;
				this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
		{
			if (this_node->list != NULL &&
					(this_node->nodeType==TALK_STARTSER || this_node->nodeType==TALK_STARTPAR))
			{
				SIR->lists[pos] = (struct List *)this_node->node.ln_Succ;
				pos++;

				start_node = (struct ScriptNodeRecord *)this_node->list->lh_Head;

				newList=TRUE;

				this_node->list = listPtrs[*j];
				*j = *j + 1;

				break;
			}
		}
		if (!newList)
		{
			if (pos>0)
			{
				pos--;
				start_node = (struct ScriptNodeRecord *)SIR->lists[pos];
			}
			else
				return;	/* READY */
		}
	}
}

/******** UndoClear() ********/

void UndoClear(struct ScriptInfoRecord *SIR)
{
struct ScriptNodeRecord *this_node;
struct ScriptNodeRecord *new_node;
struct ScriptNodeRecord *after_node;
struct ScriptNodeRecord *count_node;
int i, j, where, numUndoLists, pos, oldNO;

	if (ObjectRecord.objList == NULL)
		return;

	/**** count number of lists in clipboard ****/

	numUndoLists=0;
	for(i=0; i<CPrefs.MaxNumLists; i++)
		if ( undoLists[i] != NULL )
			numUndoLists++;

	/**** store in arrayPos the empty allLists positions ****/

	if (numUndoLists > 1)
	{
		j=0;
		for(i=0; i<CPrefs.MaxNumLists; i++)
		{
			if (SIR->allLists[i]==NULL)
			{
				*(arrayPos+j) = i;
				j++;
				if (j==numUndoLists)
					break;
			}
		}

		/**** prepare new allLists entries ****/

		i=0;
		do
		{
			SIR->allLists[ *(arrayPos+i) ] = InitScriptList();
			newPtrs[i] = SIR->allLists[ *(arrayPos+i) ];
			i++;
	
		} while(i< (numUndoLists-1) );
	}

	/**** insert (add) COPIES of all undoLists[0] nodes ****/

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if ( undoLists[i] != NULL )
		{
			pos=i;
			break;
		}
	}

	/**** calculate numObjects for undo list (restore this value later) ****/

	oldNO = ObjectRecord.numObjects;

	ObjectRecord.numObjects=0;
	if (undoListPtr->lh_TailPred != (struct Node *)undoListPtr)
		for(this_node=(struct ScriptNodeRecord *)undoListPtr->lh_Head;
				this_node->node.ln_Succ;
				this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
			ObjectRecord.numObjects++;

	j=0;
	for(this_node=(struct ScriptNodeRecord *)undoLists[pos]->lh_Head;
			this_node->node.ln_Succ;
			this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
	{
		new_node = (struct ScriptNodeRecord *)AllocMem(sizeof(struct ScriptNodeRecord), MEMF_PUBLIC|MEMF_CLEAR);
		if (new_node==NULL)
		{
			UA_WarnUser(173);
			ObjectRecord.numObjects = oldNO;
			return;
		}	

		CopyMem(this_node, new_node, sizeof(struct ScriptNodeRecord));

		/**** if node carries around extraData, copy that too ****/

		if (this_node->extraData != NULL && this_node->extraDataSize>0)
		{
			new_node->extraData = (UBYTE *)AllocMem(this_node->extraDataSize, MEMF_PUBLIC | MEMF_CLEAR);
			if (new_node->extraData==NULL)
			{
				UA_WarnUser(174);
				ObjectRecord.numObjects = oldNO;
				return;
			}	
			CopyMem(this_node->extraData,new_node->extraData,this_node->extraDataSize);
		}

		if (	this_node->nodeType==TALK_PAGE && this_node->numericalArgs[15]==1 &&
					this_node->list!=NULL )
			Copy_LE_Info(this_node, new_node);

		if (	this_node->nodeType==TALK_VARS )
			Copy_Vars_Info(this_node, new_node);

		new_node->miscFlags = 0;

/******
		if (new_node->nodeType == TALK_LABEL ||
				new_node->nodeType == TALK_STARTSER ||
				new_node->nodeType == TALK_STARTPAR)
			CheckNameUniqueness(SIR, new_node);
********/

		if (SNRlist[j] == 0)
			where=ADD_TO_HEAD;
		else if (SNRlist[j]==ObjectRecord.numObjects)
			where=ADD_TO_TAIL;
		else
		{
			where=ADD_TO_MIDDLE;
			for(count_node=(struct ScriptNodeRecord *)undoListPtr->lh_Head, i=0;
					count_node->node.ln_Succ;
					count_node=(struct ScriptNodeRecord *)count_node->node.ln_Succ, i++)
			{
				if (i==SNRlist[j])
				{
					after_node = (struct ScriptNodeRecord *)count_node->node.ln_Pred;
					break;
				}
			}
		}

		if ( where == ADD_TO_HEAD )
		{
			AddHead((struct List *)undoListPtr, (struct Node *)new_node);
			where = ADD_TO_MIDDLE;	/* else all objects are placed at head */
			after_node = new_node;

			/**** keep list scroller happy ****/
			if (undoListPtr == ObjectRecord.objList)
				ObjectRecord.firstObject = new_node;

			ObjectRecord.numObjects++;
		}
		else if ( where == ADD_TO_MIDDLE )
		{
			Insert(	(struct List *)undoListPtr, (struct Node *)new_node,
							(struct Node *)after_node);
			after_node = new_node;

			ObjectRecord.numObjects++;
		}
		else if ( where == ADD_TO_TAIL )
		{
			AddTail((struct List *)undoListPtr, (struct Node *)new_node);

			ObjectRecord.numObjects++;
		}

		j++;
	}

	/**** add COPIES of undoLists to free allLists[] ****/

	if (numUndoLists > 1)
	{
		for(i=pos+1, j=0; i<CPrefs.MaxNumLists; i++)
		{
			if (undoLists[i] != NULL &&
					undoLists[i]->lh_TailPred != (struct Node *)undoLists[i]) /* non empty */
			{
				/**** add all nodes ****/
	
				for(this_node=(struct ScriptNodeRecord *)undoLists[i]->lh_Head;
						this_node->node.ln_Succ;
						this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
				{
					new_node = (struct ScriptNodeRecord *)AllocMem(sizeof(struct ScriptNodeRecord), MEMF_PUBLIC|MEMF_CLEAR);
					if (new_node==NULL)
					{
						UA_WarnUser(175);
						ObjectRecord.numObjects = oldNO;
						return;
					}	
	
					CopyMem(this_node, new_node, sizeof(struct ScriptNodeRecord));
	
					/**** if node carries around extraData, copy that too ****/
	
					if (this_node->extraData != NULL && this_node->extraDataSize>0)
					{
						new_node->extraData = (UBYTE *)AllocMem(this_node->extraDataSize, MEMF_PUBLIC | MEMF_CLEAR);
						if (new_node->extraData==NULL)
						{
							UA_WarnUser(176);
							ObjectRecord.numObjects = oldNO;
							return;
						}	
						CopyMem(this_node->extraData,new_node->extraData,this_node->extraDataSize);
					}

					if (	this_node->nodeType==TALK_PAGE && this_node->numericalArgs[15]==1 &&
								this_node->list!=NULL )
						Copy_LE_Info(this_node, new_node);

					if (	this_node->nodeType==TALK_VARS )
						Copy_Vars_Info(this_node, new_node);

					new_node->miscFlags = 0;

/**********	
					if (new_node->nodeType == TALK_LABEL ||
							new_node->nodeType == TALK_STARTSER ||
							new_node->nodeType == TALK_STARTPAR)
						CheckNameUniqueness(SIR, new_node);
********/

					AddTail((struct List *)SIR->allLists[ *(arrayPos+j) ], (struct Node *)new_node);
				}
	
				j++;
			}
		}
	
		UpdateAllListsPointers(SIR, newPtrs, undoLists);
	}

	ObjectRecord.numObjects = oldNO;

	FreeClipUndoLists(undoLists);

	GetNumObjects();

	/**** set menu item(s) ****/

	DisableMenu(script_MR[EDIT_MENU], EDIT_UNDO);

	if (ObjectRecord.numObjects>0)
		EnableMenu(script_MR[EDIT_MENU], EDIT_SELECTALL);

	/**** when clearing in branch 1, and you jump to br. 0 and undo then ****/
	/**** on returning to br. 1 the undo must not be set back! ****/

	EditMenuStates[EDIT_UNDO] = TRUE;				// TRUE IF DISABLED!
	EditMenuStates[EDIT_SELECTALL] = FALSE;
}

/******** UpdateAllListsPointers() ********/

void UpdateAllListsPointers(struct ScriptInfoRecord *SIR,
														struct List **list, struct List **undoclipList)
{
int i, j, k, pos;
struct List *this_list;
struct ScriptNodeRecord *this_node;

	pos=-1;
	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if ( undoclipList[i] != NULL )
		{
			pos=i;
			break;
		}
	}
	if (pos==-1)
		return;

	k=0;
	for(i=pos; i<CPrefs.MaxNumLists; i++)
	{
		if ( undoclipList[i] != NULL )
		{
			for(j=0; j<CPrefs.MaxNumLists; j++)
			{
				if (SIR->allLists[j] != NULL)
				{
					this_list = SIR->allLists[j];

					if (this_list->lh_TailPred != (struct Node *)this_list)	/* non empty */
					{
						for(this_node=(struct ScriptNodeRecord *)this_list->lh_Head;
								this_node->node.ln_Succ;
								this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
						{
							if (	this_node->list != NULL &&
										(this_node->nodeType==TALK_STARTSER || this_node->nodeType==TALK_STARTPAR) )
							{
								if ( this_node->list == undoclipList[i] )
								{	
									this_node->list = list[k];
									k++;
								}
							}
						}
					}
				}
			}
		}
	}						
}

/******** KillAllVars() ********/

void KillAllVars(struct ScriptInfoRecord *SIR)
{
int i;
//SNRPTR this_node;
struct List *list;
struct ScriptNodeRecord *work_node;
struct ScriptNodeRecord *next_node;

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if (SIR->allLists[i] != NULL)
		{
			list = SIR->allLists[i];
			if (list->lh_TailPred != (struct Node *)list)
			{
				work_node = (struct ScriptNodeRecord *)(list->lh_Head);
				while(next_node = (struct ScriptNodeRecord *)(work_node->node.ln_Succ))
				{
					if ( work_node->nodeType == TALK_VARS )
					{
						if (work_node == (struct ScriptNodeRecord *)list->lh_Head)
							RemHead( list );
						else if (work_node == (struct ScriptNodeRecord *)list->lh_TailPred)
							RemTail( list );
						else
							Remove((struct Node *)work_node);

						FreeANode(work_node);
					}
					work_node = next_node;
				}
			}
		}
	}

	ObjectRecord.firstObject = (struct ScriptNodeRecord *)ObjectRecord.objList->lh_Head;
}

/******** E O F ********/
