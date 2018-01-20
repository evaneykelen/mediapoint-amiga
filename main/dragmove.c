#include "nb:pre.h"

/**** externals ****/

extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct Window *scriptWindow;
extern struct MsgPort *capsPort;
extern struct Library *medialinkLibBase;
extern struct ObjectInfo ObjectRecord;
extern ULONG numEntries1, numDisplay1;
extern LONG topEntry1;
extern struct Gadget ScriptSlider1;
extern struct RastPort dragRP;
extern struct RastPort dragRP2;

/**** gadgets ****/

extern struct GadgetRecord Script_GR[];

/******** DragMoveObjects() ********/

BOOL DragMoveObjects(int top)
{
ULONG signals;
BOOL loop, mouseMoved, redraw=FALSE;
struct IntuiMessage *message;
int row, diffY, add, oldtopY, oldbottomY, old, threshold=0;
struct ScriptNodeRecord *this_node;
UBYTE hasMoved=0;
LONG old_topEntry1;

	old_topEntry1 = topEntry1;

	this_node = (struct ScriptNodeRecord *)WhichObjectWasClicked(top, &row);
	if (this_node==NULL)
		return(FALSE);

	UA_SwitchMouseMoveOn(scriptWindow);

	if (CPrefs.ScriptScreenModes & LACE)
		add=2;
	else
		add=1;

	row = ( (CED.MouseY - Script_GR[0].y1) / 20 );
	oldtopY = row*20;
	oldtopY += Script_GR[0].y1+add;
	oldbottomY = oldtopY+ICONHEIGHT+1;
	diffY = oldtopY-CED.MouseY;
	loop=TRUE;

	/**** copy object bar to bit map ****/

	WaitTOF();
	ClipBlit(	scriptWindow->RPort,
						Script_GR[0].x1+3, oldtopY+1,
						&dragRP,
						0, 0, Script_GR[0].x2-Script_GR[0].x1-6, ICONHEIGHT+1,
						0xc0L );
	old=oldtopY+1;

	SetAPen(&dragRP2, AREA_PEN);
	RectFill(&dragRP2, 0, 0, Script_GR[0].x2-Script_GR[0].x1-6, ICONHEIGHT+1);

	SetByteBit(&this_node->miscFlags, OBJ_BEINGDRAGGED);

	while(loop)
	{
		signals = Wait(SIGNALMASK);
		if (signals & SIGNALMASK)
		{
			mouseMoved=FALSE;
			while(message = (struct IntuiMessage *)GetMsg(capsPort))
			{
				CED.Class	= message->Class;
				CED.Code = message->Code;
				CED.MouseY = message->MouseY+diffY;
				ReplyMsg((struct Message *)message);

				switch(CED.Class)
				{
					case IDCMP_MOUSEBUTTONS:
						if (CED.Code == SELECTUP)
							loop=FALSE;
						break;

					case IDCMP_MOUSEMOVE:
						if ( threshold++ > 3 )
							mouseMoved=TRUE;
						break;

					default:
						loop=FALSE;
						break;
				}
			}

			if (mouseMoved)
			{
				DrawScriptObject(CED.MouseY, &oldtopY, &oldbottomY);
				hasMoved=1;
			}
		}
	}

	if (hasMoved==1)
	{
		redraw = MoveDragMovedObject(this_node);
//		if ( old == oldtopY+1 )

		ClipBlit(	&dragRP2,
							0, 0,
							scriptWindow->RPort,
							Script_GR[0].x1+3, oldtopY+1,
							Script_GR[0].x2-Script_GR[0].x1-6, ICONHEIGHT+1, 0xc0);

		if ( old_topEntry1 == topEntry1 )
		{
			ClipBlit(	&dragRP,
								0, 0,
								scriptWindow->RPort,
								Script_GR[0].x1+3, old,
								Script_GR[0].x2-Script_GR[0].x1-6, ICONHEIGHT+1, 0xc0);
		}
		else
			redraw=TRUE;
	}

	UA_SwitchMouseMoveOff(scriptWindow);

	UnSetByteBit(&this_node->miscFlags, OBJ_BEINGDRAGGED);

	return(redraw);
}

/******** DrawScriptObject() ********/

void DrawScriptObject(int mouseY, int *oldtopY, int *oldbottomY)
{
BOOL redraw=FALSE;
int oldY1,oldY2;

	/**** remove old box ****/

	oldY1 = *oldtopY;
	oldY2 = *oldbottomY;

	/**** scroll list up or down if the mouse pointer is way up or down ****/

	if ( mouseY<Script_GR[0].y1 && topEntry1>0 )
	{
		topEntry1--;

		ClipBlit(	&dragRP2,
							0, 0,
							scriptWindow->RPort,
							Script_GR[0].x1+3, oldY1+1, Script_GR[0].x2-Script_GR[0].x1-6, ICONHEIGHT+1,
							0xc0L );

		UA_SetPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);
		redraw=TRUE;
	}
	
	if ( (mouseY+ICONHEIGHT) > Script_GR[0].y2 && (topEntry1+numDisplay1)<numEntries1 )
	{
		topEntry1++;

		ClipBlit(	&dragRP2,
							0, 0,
							scriptWindow->RPort,
							Script_GR[0].x1+3, oldY1+1, Script_GR[0].x2-Script_GR[0].x1-6, ICONHEIGHT+1,
							0xc0L );

		UA_SetPropSlider(scriptWindow, &ScriptSlider1, numEntries1, numDisplay1, topEntry1);
		redraw=TRUE;
	}

	/**** if scrolled, redraw list ****/

	if (redraw)
		DrawObjectList(topEntry1, TRUE, TRUE);

	/**** draw new box ****/

	*oldtopY = mouseY;

	if ( *oldtopY < (Script_GR[0].y1+1) )
		*oldtopY = Script_GR[0].y1+1;

	if ( *oldtopY > (Script_GR[0].y2-2-ICONHEIGHT) )
		*oldtopY = Script_GR[0].y2-2-ICONHEIGHT;

	if (redraw)
		DrawObjectContourBox(*oldtopY, -1);
	else
		DrawObjectContourBox(*oldtopY, oldY1);
}

/******** DrawObjectContourBox() ********/

void DrawObjectContourBox(int y1, int oldy1)
{
	/**** copy off-screen area back to previously obscured window part ****/

	if ( oldy1 != -1 )
		ClipBlit(	&dragRP2,
							0, 0,
							scriptWindow->RPort,
							Script_GR[0].x1+3, oldy1+1, Script_GR[0].x2-Script_GR[0].x1-6, ICONHEIGHT+1,
							0xc0L );

	/**** copy area under new mouse pointer to off-screen area ****/

	ClipBlit(	scriptWindow->RPort,
						Script_GR[0].x1+3, y1+1,
						&dragRP2,
						0, 0, Script_GR[0].x2-Script_GR[0].x1-6, ICONHEIGHT+1,
						0xc0L );

	/**** copy object to area under mouse pointer ****/

	ClipBlit(	&dragRP,
						0, 0,
						scriptWindow->RPort,
						Script_GR[0].x1+3, y1+1, Script_GR[0].x2-Script_GR[0].x1-6, ICONHEIGHT+1,
						0xc0L );
}

/******** MoveDragMovedObject() ********/

BOOL MoveDragMovedObject(struct ScriptNodeRecord *this_node)
{
struct ScriptNodeRecord *to_node, *oldNode;
int where, start_y, row, i;

	oldNode = (struct ScriptNodeRecord *)this_node->node.ln_Succ;

	to_node=NULL;
	where=-1;

	start_y=CED.MouseY;

	if (start_y < Script_GR[0].y1)
	{
		to_node = NULL;
		where = ADD_TO_HEAD;
	}
	else
	{
		start_y += ( (ICONHEIGHT/2)-11 );
		row = ( ( (start_y - Script_GR[0].y1) / 20 ) + topEntry1 );

		if (row<ObjectRecord.numObjects)
		{
			/**** with the object POS, find the object NODE ****/

			if (ObjectRecord.objList->lh_TailPred == (struct Node *)ObjectRecord.objList)
				return(FALSE);

			to_node=(struct ScriptNodeRecord *)ObjectRecord.firstObject;

			if (row>0)
			{
				for(i=0; i<row; i++)
				{
					if (to_node!=NULL)
						to_node = (struct ScriptNodeRecord *)(to_node->node.ln_Succ);
				}
			}

			if (to_node==NULL ||
					to_node==(struct ScriptNodeRecord *)ObjectRecord.objList->lh_TailPred)
				where = ADD_TO_TAIL;
			else
				where = ADD_TO_MIDDLE;
		}
		else
			where = ADD_TO_TAIL;
	}

	if (where==-1)
		UA_WarnUser(196);

	if ( to_node==this_node )
		return(FALSE);

	/**** first remove node from list ****/

	if (this_node == (struct ScriptNodeRecord *)ObjectRecord.objList->lh_Head) /* head node */
	{
		RemHead(ObjectRecord.objList);
		ObjectRecord.firstObject = (struct ScriptNodeRecord *)ObjectRecord.objList->lh_Head;
	}
	else if (this_node == (struct ScriptNodeRecord *)ObjectRecord.objList->lh_TailPred) /* tail node */
		RemTail(ObjectRecord.objList);
	else
		Remove((struct Node *)this_node);

	/**** then insert it back at another position ****/

	if ( where == ADD_TO_HEAD )
	{
		AddHead((struct List *)ObjectRecord.objList, (struct Node *)this_node);
		ObjectRecord.firstObject = this_node;
	}
	else if ( where == ADD_TO_MIDDLE )
	{
		Insert(	(struct List *)ObjectRecord.objList, (struct Node *)this_node,
						(struct Node *)to_node);
	}
	else if ( where == ADD_TO_TAIL )
	{
		AddTail((struct List *)ObjectRecord.objList, (struct Node *)this_node);
	}

	if ( oldNode == (struct ScriptNodeRecord *)this_node->node.ln_Succ )
		return(FALSE);

	return(TRUE);
}

/******** E O F ********/
