#include "nb:pre.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Window *scriptWindow;
extern struct Library *medialinkLibBase;
extern UWORD chip gui_pattern[];

/**** gadgets ****/

extern __far struct GadgetRecord Debug_GR[];

/**** functions ****/

/******** ShowSNR() ********/

void ShowSNR(SNRPTR this_node, SNRPTR first_node)
{
struct Window *window;
BOOL loop, retVal;
int ID,val;
SNRPTR node;

	retVal = FALSE;
	loop   = TRUE;

	/**** open a window ****/

	window = UA_OpenRequesterWindow(scriptWindow,Debug_GR,STDCOLORS);
	if (!window)
	{
		UA_WarnUser(209);
		return;
	}

	/**** render gadget ****/

	UA_DrawGadgetList(window, Debug_GR);

	for(ID=3; ID<19; ID++)
	{
		val = (int)this_node->numericalArgs[ID-3];
		UA_SetStringGadgetToVal(window, &Debug_GR[ID], val);
	}

	UA_SetStringGadgetToVal(window, &Debug_GR[19], (int)this_node->nodeType);
	UA_SetStringGadgetToVal(window, &Debug_GR[20], (int)this_node->startendMode);

	UA_SetStringGadgetToVal(window, &Debug_GR[22], (int)this_node->miscFlags);
	UA_SetStringGadgetToVal(window, &Debug_GR[23], (int)this_node->dayBits);
	UA_SetStringGadgetToVal(window, &Debug_GR[24], (int)this_node->duration);

	if ( this_node->extraData!=NULL && this_node->extraDataSize>0)
		UA_SetStringGadgetToString(window, &Debug_GR[25], this_node->extraData);
	else
		UA_DisableButton(window, &Debug_GR[25], gui_pattern);

	if ( this_node->extraData!=NULL && this_node->extraDataSize>50)
		UA_SetStringGadgetToString(window, &Debug_GR[26], this_node->extraData+50);
	else
		UA_DisableButton(window, &Debug_GR[26], gui_pattern);

	UA_SetStringGadgetToString(window, &Debug_GR[27], this_node->objectPath);
	UA_SetStringGadgetToString(window, &Debug_GR[28], this_node->objectName);

	UA_SetStringGadgetToVal(window, &Debug_GR[29], (int)this_node->Start.TimeCode.HH);
	UA_SetStringGadgetToVal(window, &Debug_GR[30], (int)this_node->Start.TimeCode.MM);
	UA_SetStringGadgetToVal(window, &Debug_GR[31], (int)this_node->Start.TimeCode.SS);
	UA_SetStringGadgetToVal(window, &Debug_GR[32], (int)this_node->Start.TimeCode.FF);

	UA_SetStringGadgetToVal(window, &Debug_GR[33], (int)this_node->End.TimeCode.HH);
	UA_SetStringGadgetToVal(window, &Debug_GR[34], (int)this_node->End.TimeCode.MM);
	UA_SetStringGadgetToVal(window, &Debug_GR[35], (int)this_node->End.TimeCode.SS);
	UA_SetStringGadgetToVal(window, &Debug_GR[36], (int)this_node->End.TimeCode.FF);

	UA_DisableButton(window, &Debug_GR[21], gui_pattern);	// former effect

	/**** monitor user ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, Debug_GR, &CED);
			if (ID==2)
			{
				UA_HiliteButton(window, &Debug_GR[2]);
				loop=FALSE;
				retVal=FALSE;
			}
			else if (ID>=3 && ID<=36)
				UA_ProcessStringGadget(window, Debug_GR, &Debug_GR[ID], &CED);
		}
	}

	UA_EnableButton(window, &Debug_GR[21]);	// former effect

	for(ID=3; ID<19; ID++)
	{
		UA_SetValToStringGadgetVal(	&Debug_GR[ID], &val );
		this_node->numericalArgs[ID-3] = (WORD)val;
	}

	UA_SetValToStringGadgetVal(&Debug_GR[19], &ID);
	this_node->nodeType=ID;
	UA_SetValToStringGadgetVal(&Debug_GR[20], &ID);
	this_node->startendMode=ID;

	UA_SetValToStringGadgetVal(&Debug_GR[22], &ID);
	this_node->miscFlags=ID;
	UA_SetValToStringGadgetVal(&Debug_GR[23], &ID);
	this_node->dayBits=ID;
	UA_SetValToStringGadgetVal(&Debug_GR[24], &ID);
	this_node->duration=ID;

	if (this_node->extraData!=NULL && this_node->extraDataSize>0)
		UA_SetStringToGadgetString(&Debug_GR[25], this_node->extraData);
	else
		UA_EnableButton(window, &Debug_GR[25]);

	if ( this_node->extraData!=NULL && this_node->extraDataSize>50)
		UA_SetStringToGadgetString(&Debug_GR[26], this_node->extraData+50);
	else
		UA_EnableButton(window, &Debug_GR[26]);

	UA_SetStringToGadgetString(&Debug_GR[27], this_node->objectPath);
	UA_SetStringToGadgetString(&Debug_GR[28], this_node->objectName);

	UA_SetValToStringGadgetVal(&Debug_GR[29], &ID);
	this_node->Start.TimeCode.HH=ID;
	UA_SetValToStringGadgetVal(&Debug_GR[30], &ID);
	this_node->Start.TimeCode.MM=ID;
	UA_SetValToStringGadgetVal(&Debug_GR[31], &ID);
	this_node->Start.TimeCode.SS=ID;
	UA_SetValToStringGadgetVal(&Debug_GR[32], &ID);
	this_node->Start.TimeCode.FF=ID;

	UA_SetValToStringGadgetVal(&Debug_GR[33], &ID);
	this_node->End.TimeCode.HH=ID;
	UA_SetValToStringGadgetVal(&Debug_GR[34], &ID);
	this_node->End.TimeCode.MM=ID;
	UA_SetValToStringGadgetVal(&Debug_GR[35], &ID);
	this_node->End.TimeCode.SS=ID;
	UA_SetValToStringGadgetVal(&Debug_GR[36], &ID);
	this_node->End.TimeCode.FF=ID;

	for(node=(struct ScriptNodeRecord *)first_node;
			node->node.ln_Succ;
			node=(struct ScriptNodeRecord *)node->node.ln_Succ)
	{
		if ( node!=this_node && node->miscFlags & OBJ_SELECTED )
		{
			for(ID=0; ID<MAX_PARSER_ARGS; ID++)
				node->numericalArgs[ID] = this_node->numericalArgs[ID];
		}
	}

	UA_CloseRequesterWindow(window,STDCOLORS);
}

/******** E O F ********/
