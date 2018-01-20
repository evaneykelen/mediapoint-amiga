#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <exec/semaphores.h>
#include <proto/exec.h>
#include <string.h>
#include "mlmmu_pragma.h"
#include "mlmmu.h"
#include "mlmmu_proto.h"
#include "mpmmu_ext.h"
#include "mpmmu_app.h"

#define MLMMULibBase (getreg(REG_A6))

STATIC struct List *errorList;

/******** OpenMsgQueue() ********/

BOOL __saveds __asm LIBMLMMU_OpenMsgQueue(void)
{
	ObtainSemaphore(&MMUSemaphore);

	errorList = (struct List *)AllocMem(sizeof(struct List), MEMF_ANY | MEMF_CLEAR);
	if (errorList==NULL)
	{
		ReleaseSemaphore(&MMUSemaphore);
		return(FALSE);
	}
	NewList(errorList);

	ReleaseSemaphore(&MMUSemaphore);
	return(TRUE);
}

/******** CloseMsgQueue() ********/

void __saveds __asm LIBMLMMU_CloseMsgQueue(void)
{
struct MsgNode *work_node, *next_node;

	ObtainSemaphore(&MMUSemaphore);

	work_node = (struct MsgNode *)(errorList->lh_Head);	// first node
	while(next_node = (struct MsgNode *)(work_node->node.ln_Succ))
	{
		if ( work_node->txt )
			FreeMem(work_node->txt, work_node->txtSize);	// free text
		FreeMem(work_node, sizeof(struct MsgNode));			// free node itself
		work_node = next_node;
	}
	if (errorList!=NULL)
		FreeMem(errorList, sizeof(struct List));
	errorList=NULL;

	ReleaseSemaphore(&MMUSemaphore);
}

/******** AddMsgToQueue() ********/

void __saveds __asm LIBMLMMU_AddMsgToQueue(	register __a0 STRPTR txt,
																						register __d0 ULONG flags)
{
struct MsgNode *this_node;

	ObtainSemaphore(&MMUSemaphore);

	for(this_node=(struct MsgNode *)(errorList->lh_Head);
			this_node->node.ln_Succ;
			this_node=(struct MsgNode *)this_node->node.ln_Succ)
	{
		if ( strcmpi(this_node->txt,txt)==0 )	// error already exists
		{
			ReleaseSemaphore(&MMUSemaphore);
			return;
		}
	}

	this_node = (struct MsgNode *)AllocMem(sizeof(struct MsgNode), MEMF_ANY | MEMF_CLEAR);
	if (this_node!=NULL)
	{
		this_node->node.ln_Name = NULL;
		this_node->node.ln_Type = 100;
		this_node->node.ln_Pri 	= 0;
		AddTail((struct List *)errorList,(struct Node *)this_node);

		this_node->txt = (UBYTE *)AllocMem(strlen(txt)+2, MEMF_ANY | MEMF_CLEAR);
		if ( this_node->txt != NULL )
		{
			strcpy(this_node->txt, txt);
			this_node->txtSize = strlen(txt)+2;
			this_node->flags = flags;
		}
	}

	ReleaseSemaphore(&MMUSemaphore);
}

/******** GetQueueList() ********/

struct List * __saveds __asm LIBMLMMU_GetQueueList(void)
{
	return( errorList );
}

/******** E O F ********/
