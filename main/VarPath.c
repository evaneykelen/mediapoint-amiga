#include "nb:pre.h"

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *scriptWindow;
extern struct Screen *scriptScreen;
extern struct Library *medialinkLibBase;
extern struct ObjectInfo ObjectRecord;
extern UBYTE **msgs;
extern struct TextFont *smallFont;
extern struct TextFont *largeFont;
extern struct TextFont *tiny_smallFont;
extern struct TextFont *tiny_largeFont;
extern int objectXPosList[];
extern int objectYPosList[];
extern int standardXPosList[];
extern int standardYPosList[];
extern struct RastPort xappRP_2;
extern TEXT *dir_xapps;
extern TEXT *dir_system;

STATIC UBYTE *CreateDeclList2(struct ScriptInfoRecord *SIR, int *num, int *act, STRPTR varName);
STATIC void ReplaceString(UBYTE *data, STRPTR str, int repl);
#define VARWIDTH 75

/**** gadgets ****/

extern struct GadgetRecord VarPath_GR[];
extern struct GadgetRecord VarPath_PopUp_GR[];

/**** functions ****/

/******** FixedVarPath() ********/

void FixedVarPath(void)
{
struct Window *window;
BOOL loop=TRUE, retval;
int ID,y;
struct PopUpRecord PUR;
TEXT varName[100];
UBYTE *ptr;
int num,act,val,i;
struct ScriptNodeRecord *work_node;
TEXT arg1[75],arg2[75];
BOOL fixed=FALSE;

	if ( CountVIRS(&(ObjectRecord.scriptSIR.VIList)) == 0 )
	{
		Message( msgs[Msg_NoDeclas-1] );
		return;
	}

	num=0;
	work_node = CountNumSelected(ObjectRecord.firstObject,&num);

	if ( num==1 )
		strcpy(varName,work_node->objectName);
	else
		varName[0] = '\0';

	act = 0;
	if ( varName[0]=='@' )
	{
		i=2;
		while( varName[i] != ')' && i<70 )		
		{
			arg1[i-2] = varName[i];	// @(x)	-> x is on 2
			arg1[i-1] = '\0';
			i++;
		}
	}
	else
		arg1[0] = '\0';
	ptr = CreateDeclList2(&ObjectRecord.scriptSIR,&num,&act,arg1);
	if ( !ptr )
	{
		UA_WarnUser(-1);
		return;
	}

	// OPEN WINDOW

	window = UA_OpenRequesterWindow(scriptWindow, VarPath_GR, USECOLORS);
	if (!window)
	{
		UA_WarnUser(-1);
		return;
	}

	// DRAW GADGETS

	UA_DrawGadgetList(window, VarPath_GR);

	if ( scriptWindow->WScreen->ViewPort.Modes & LACE )
		SetFont(window->RPort, tiny_largeFont);
	else
		SetFont(window->RPort, tiny_smallFont);

	if ( scriptWindow->WScreen->ViewPort.Modes & LACE )
		y=69*2;
	else
		y=69;

	if ( scriptWindow->WScreen->ViewPort.Modes & LACE )
		PutImageInRastPort(	GFX_SM_EXCL_X_L, GFX_SM_EXCL_Y_L,
												window->RPort,
												8, y, GFX_SM_EXCL_W_L, GFX_SM_EXCL_H_L );
	else
		PutImageInRastPort(	GFX_SM_EXCL_X_NL, GFX_SM_EXCL_Y_NL,
												window->RPort,
												8, y, GFX_SM_EXCL_W_NL, GFX_SM_EXCL_H_NL );

	if ( scriptWindow->WScreen->ViewPort.Modes & LACE )
		y=71*2;
	else
		y=71;

	SetAPen(window->RPort,LO_PEN);
	SetDrMd(window->RPort,JAM1);
	Move(window->RPort, 34, y+window->RPort->TxBaseline);
	Text(window->RPort, msgs[Msg_VarPath_4-1], strlen(msgs[Msg_VarPath_4-1]));

	if ( scriptWindow->WScreen->ViewPort.Modes & LACE )
		SetFont(window->RPort, largeFont);
	else
		SetFont(window->RPort, smallFont);

	VarPath_GR[7].type = BUTTON_GADGET;

	// POPUP RECORD

	PUR.window = window;
	PUR.GR = VarPath_PopUp_GR;
	PUR.ptr = ptr;
	PUR.active = act;
	PUR.number = num;
	PUR.width = VARWIDTH;
	PUR.fit = 0;
	PUR.top = 0;

	UA_PrintPopUpChoice(window, &VarPath_GR[7], &PUR);

	stccpy(varName,PUR.ptr+PUR.active*PUR.width,99);

	// EVENT LOOP

	while(loop)
	{
		UA_doStandardWait(window,&CED);
		if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
		{
			ID = UA_CheckGadgetList(window, VarPath_GR, &CED);
			switch(ID)
			{
				case 3:	// OK
do_ok:
					UA_HiliteButton(window, &VarPath_GR[3]);
					loop=FALSE;
					retval=TRUE;
					break;

				case 4:	// Cancel
do_cancel:
					UA_HiliteButton(window, &VarPath_GR[4]);
					loop=FALSE;
					retval=FALSE;
					break;

				case 7:	// var path
					UA_InvertButton(window, &VarPath_GR[ID]);
					if ( UA_OpenPopUpWindow(window, &VarPath_GR[ID], &PUR) )
					{
						UA_Monitor_PopUp(&PUR);
						UA_ClosePopUpWindow(&PUR);
						stccpy(varName,PUR.ptr+PUR.active*PUR.width,99);
						UA_PrintPopUpChoice(window, &VarPath_GR[ID], &PUR);
					}
					else
						UA_InvertButton(window, &VarPath_GR[ID]);
					break;			

				case 8:	// fixed
					UA_HiliteButton(window, &VarPath_GR[ID]);
					fixed=TRUE;
					loop=FALSE;
					retval=TRUE;
					break;			
			}
		}
		else if ( CED.Class==IDCMP_RAWKEY && CED.Code==RAW_RETURN )
			goto do_ok;
		else if ( CED.Class==IDCMP_RAWKEY && CED.Code==RAW_ESCAPE )
			goto do_cancel;
	}

	VarPath_GR[7].type = HIBOX_REGION;

	FreeMem(ptr,num*VARWIDTH);

	if ( retval )
	{
		sscanf(varName,"%s = %s",arg1,arg2);

		for(work_node=(struct ScriptNodeRecord *)ObjectRecord.firstObject;
				work_node->node.ln_Succ;
				work_node=(struct ScriptNodeRecord *)work_node->node.ln_Succ)
		{
			if ( work_node->miscFlags & OBJ_SELECTED )
			{
				if (	work_node->nodeType == TALK_ANIM || work_node->nodeType == TALK_PAGE ||
							work_node->nodeType == TALK_SOUND )
				{
					if ( fixed )
					{
						work_node->objectName[0] = '\0';
						if ( work_node->nodeType == TALK_PAGE )
							work_node->numericalArgs[15] = 2;	// IFF
					}
					else
					{
						sprintf(work_node->objectName,"@(%s)",arg1);					
						if ( work_node->nodeType == TALK_PAGE )
							work_node->numericalArgs[15] = 1;	// document
					}
				}
				else if (	(work_node->nodeType == TALK_AREXX || work_node->nodeType == TALK_DOS) &&
									work_node->numericalArgs[1] == ARGUMENT_SCRIPT )
				{
					if ( fixed )
					{
						work_node->objectName[0] = '\0';
						work_node->extraData[0] = '\0';
					}
					else
					{
						sprintf(work_node->objectName,"@(%s)",arg1);					
						sprintf(work_node->extraData,"@(%s)",arg1);					
					}
				}
				else if ( work_node->nodeType == TALK_USERAPPLIC )
				{
					if ( !strcmpi(work_node->objectPath,"SAMPLE") )
					{
						if ( work_node->extraData[0] )
						{
							sscanf(work_node->extraData,"%d", &val);
							if ( val==1 )	// play
							{
								if ( fixed )
									work_node->objectName[0] = '\0';
								else
									sprintf(work_node->objectName,"@(%s)",arg1);

								if ( fixed )
									strcpy(arg2,"\\\"\\\"");
								else
									sprintf(arg2,"@(%s)",arg1);
								ReplaceString(work_node->extraData, arg2, 1);
							}
						}
					}
					else if ( !strcmpi(work_node->objectPath,"NEWSCRIPT") )
					{
						//if ( work_node->extraData[0] )
						{
							if ( fixed )
								work_node->objectName[0] = '\0';
							else
								sprintf(work_node->objectName,"@(%s)",arg1);

							if ( fixed )
								strcpy(arg2,"\\\"\\\"");
							else
								sprintf(arg2,"@(%s)",arg1);

							strcpy(work_node->extraData, arg2);
							//ReplaceString(work_node->extraData, arg2, 0);
						}
					}
				}
			}
		}

		DrawObjectList(-1, TRUE, TRUE);	// only redraw object name part
	}

	UA_CloseRequesterWindow(window,USECOLORS);
}

/******** CreateDeclList2() ********/

STATIC UBYTE *CreateDeclList2(struct ScriptInfoRecord *SIR, int *num, int *act, STRPTR varName)
{
VIR *this_vir;
UBYTE *ptr,*q;
int line;
TEXT arg1[75],arg2[75];

	*num=0;
	for(this_vir = (VIR *)SIR->VIList.lh_Head; 
			(VIR *)this_vir->vir_Node.ln_Succ;	
			this_vir = (VIR *)this_vir->vir_Node.ln_Succ)
		*num=*num+1;

	ptr = (UBYTE *)AllocMem(*num*VARWIDTH,MEMF_ANY|MEMF_CLEAR);
	if ( !ptr )
		return(NULL);

	q=ptr;
	line=0;
	for(this_vir = (VIR *)SIR->VIList.lh_Head; 
			(VIR *)this_vir->vir_Node.ln_Succ;	
			this_vir = (VIR *)this_vir->vir_Node.ln_Succ)
	{
		VIRToDecl(this_vir, q);
		sscanf(q,"%s = %s",arg1,arg2);

		if ( strlen(arg1)==strlen(varName) && !strcmpi(arg1,varName) )
			*act = line;
		q=q+VARWIDTH;
		line++;
	}

	return(ptr);
}

/******** ReplaceString() ********/

STATIC void ReplaceString(UBYTE *data, STRPTR str, int repl)
{
int numChars, argNum, len, numSeen;
char *strPtr;
char tmp[200];
char answer[800];

	strPtr = data;
	len = strlen(strPtr);
	argNum=0;
	numSeen=0;
	answer[0]='\0';
	while(1)
	{
		if (*strPtr==0)
			break;
		numChars = stcarg(strPtr, " 	,");	// space, tab, comma
		if (numChars>=1)
		{
			if ( argNum>0 )
				strcat(answer," ");
			stccpy(tmp, strPtr, numChars+1);
			if ( argNum==repl )
				strcpy(tmp, str);
			strcat(answer,tmp);
			argNum++;
		}
		strPtr += numChars+1;
		numSeen += numChars+1;
		if (numSeen>len)
			break;		
	}
	if ( argNum > 0 )
		strcpy(data,answer);
}

/******** E O F ********/
