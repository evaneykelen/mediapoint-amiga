/*******************************************************************/
/*
 *
 *  T H I S   I S   A L S O   A   P L A Y E R   M O D U L E !
 *
 *
 */

#include "nb:pre.h"

#define EXPRESSION_WIDTH 75	// see also varparse.c
#define NUMDECL1	8		// left scroll area
#define NUMDECL2	8		// right scroll area

/**** externals ****/

extern struct MsgPort *capsPort;
extern struct EventData CED;
extern struct CapsPrefs CPrefs;
extern struct eventHandlerInfo EHI;
extern struct Window *pageWindow;
extern struct Window *scriptWindow;
extern struct Library *medialinkLibBase;
extern struct ObjectInfo ObjectRecord;
extern UBYTE **msgs;   
extern struct Gadget PropSlider1;
extern UWORD chip gui_pattern[];

/**** static globals ****/

static struct PropInfo PI2 = { AUTOKNOB | FREEVERT | PROPBORDERLESS, 0,0,0,0, };
static struct Image Im2 = { 0,0,0,0,0,NULL,0x0000,0x0000,NULL };
static struct Gadget PropSlider2 =
{
	NULL, 0,0,0,0, NULL, GACT_RELVERIFY | GACT_IMMEDIATE, GTYP_PROPGADGET,
	&Im2, NULL, NULL, NULL, (struct PropInfo *)&PI2, 2, NULL
};

/**** gadgets ****/

extern struct GadgetRecord VarDec_GR[];
extern struct GadgetRecord ExpDec_GR[];

/**** functions ****/

#ifndef USED_FOR_PLAYER

/******** MonitorVariables() ********/

void MonitorVariables(struct ScriptNodeRecord *this_node)
{
struct Window *window;
BOOL loop, retVal;//, doStr=FALSE;
int i,ID,listType,numLines,line,topEntry1,topEntry2,selLine=-1,numDecl;
struct GadgetRecord *GR;
struct ScrollRecord SR1, SR2;
UBYTE **list,*selList,**sublist;
struct Gadget *g;
TEXT str[EXPRESSION_WIDTH], labelStr[EXPRESSION_WIDTH];
VIR *vir;
VAR *var;
struct List tempList;

	SetByteBit(&this_node->miscFlags, OBJ_NEEDS_REFRESH);

	listType = FindParentType(&(ObjectRecord.scriptSIR),ObjectRecord.objList);
	if ( listType == -1 )	// root
		GR = VarDec_GR;
	else
		GR = ExpDec_GR;

	if ( GR==ExpDec_GR )
	{
		if ( CountVIRS( &(ObjectRecord.scriptSIR.VIList) ) == 0 ) 
		{
			Message( msgs[Msg_NoDeclas-1] );
			return;
		}
	}

	list = (UBYTE **)AllocMem(MAX_LOCAL_EVENTS*sizeof(UBYTE *), MEMF_CLEAR | MEMF_ANY);
 	if ( !list )
		return;

	sublist = (UBYTE **)AllocMem(MAX_LOCAL_EVENTS*sizeof(UBYTE *), MEMF_CLEAR | MEMF_ANY);
 	if ( !sublist )
		return;

	selList = (UBYTE *)AllocMem(MAX_LOCAL_EVENTS, MEMF_CLEAR | MEMF_ANY);
	if ( !selList )
		return;

	/**** open a window ****/

	window = UA_OpenRequesterWindow(scriptWindow, GR, STDCOLORS);
	if (!window)
	{
		UA_WarnUser(-1);
		for(i=0; i<MAX_LOCAL_EVENTS; i++)
		{
			if ( list[i] )
				FreeMem(list[i], EXPRESSION_WIDTH);
			if ( sublist[i] )
				FreeMem(sublist[i], EXPRESSION_WIDTH);
		}
		FreeMem(list, MAX_LOCAL_EVENTS*sizeof(UBYTE *));
		FreeMem(sublist, MAX_LOCAL_EVENTS*sizeof(UBYTE *));
		FreeMem(selList, MAX_LOCAL_EVENTS);
		return;
	}

	/**** render gadgets ****/

	UA_DrawGadgetList(window,GR);

	ClearTheStrGad(window,&GR[8]);

	/**** add left prop gadget ****/

	PropSlider1.LeftEdge	= GR[7].x1+4;
	PropSlider1.TopEdge		= GR[7].y1+2;
	PropSlider1.Width			= GR[7].x2-GR[7].x1-7;
	PropSlider1.Height		= GR[7].y2-GR[7].y1-3;

	if ( UA_IsWindowOnLacedScreen(window) )
	{
		PropSlider1.TopEdge	+= 2;
		PropSlider1.Height	-= 4;
	}
	InitPropInfo(	(struct PropInfo *)PropSlider1.SpecialInfo,
								(struct Image *)PropSlider1.GadgetRender);
	AddGadget(window, &PropSlider1, -1L);

	if ( GR == VarDec_GR )
	{
		UA_DrawSpecialGadgetText(window, &GR[6], msgs[Msg_Decla-1], SPECIAL_TEXT_TOP);
	}
	else
	{
		UA_DrawSpecialGadgetText(window, &GR[6], msgs[Msg_Expre-1], SPECIAL_TEXT_TOP);
		UA_DrawSpecialGadgetText(window, &GR[12], msgs[Msg_Decla-1], SPECIAL_TEXT_TOP);

		/**** add right prop gadget ****/

		PropSlider2.LeftEdge	= GR[13].x1+4;
		PropSlider2.TopEdge		= GR[13].y1+2;
		PropSlider2.Width			= GR[13].x2-GR[13].x1-7;
		PropSlider2.Height		= GR[13].y2-GR[13].y1-3;
		if ( UA_IsWindowOnLacedScreen(window) )
		{
			PropSlider2.TopEdge	+= 2;
			PropSlider2.Height	-= 4;
		}
		InitPropInfo(	(struct PropInfo *)PropSlider2.SpecialInfo,
									(struct Image *)PropSlider2.GadgetRender);
		AddGadget(window, &PropSlider2, -1L);
	}

	/**** init scroll record ****/

	if ( GR == VarDec_GR )
		CreateDeclList( &(ObjectRecord.scriptSIR), list, &numLines, TRUE );
	else
	{
		CreateDeclList( &(ObjectRecord.scriptSIR), sublist, &numDecl, FALSE );
		CreateExprList( &(ObjectRecord.scriptSIR), this_node, list, &numLines );
	}

	SR1.GR						= &GR[6];
	SR1.window				= window;
	SR1.list					= NULL;
	SR1.sublist				= NULL;
	SR1.selectionList	= selList;
	SR1.entryWidth		= -1;
	SR1.numDisplay		= NUMDECL1;
	SR1.numEntries		= numLines;

	topEntry1 = 0;
	topEntry2 = 0;
	selList[0] = 1;

	UA_PrintStandardList(NULL,-1,NULL);	// init static
	UA_PrintStandardList(&SR1,0,list);
	UA_SetPropSlider(window, &PropSlider1, numLines, NUMDECL1, 0);

	if ( GR == ExpDec_GR )
	{
		SR2.GR						= &GR[12];
		SR2.window				= window;
		SR2.list					= NULL;
		SR2.sublist				= NULL;
		SR2.selectionList	= NULL;
		SR2.entryWidth		= -1;
		SR2.numDisplay		= NUMDECL2;
		SR2.numEntries		= numDecl;

		UA_PrintStandardList(NULL,-1,NULL);	// init static
		UA_PrintStandardList(&SR2,0,sublist);
		UA_SetPropSlider(window, &PropSlider2, numDecl, NUMDECL2, 0);
	}

	/**** monitor user ****/

	retVal = FALSE;
	loop   = TRUE;

	/**** start with processing first line ****/

	if ( numLines!=0 )
		UA_SetStringGadgetToString(window,&GR[8],list[0]);
	else
	{
		UA_DisableButton(window, &GR[ 9], gui_pattern);
		UA_DisableButton(window, &GR[10], gui_pattern);
	}

	if ( GR == ExpDec_GR )
		UA_PrintInBox(	window, &GR[11], GR[11].x1, GR[11].y1, GR[11].x2, GR[11].y2,
										msgs[Msg_InsLabel-1], PRINT_CENTERED);

	OffGadget(&PropSlider1,window,NULL); OffGadget(&PropSlider2,window,NULL);

	while ( UA_ProcessStringGadget(window, GR, &GR[8], &CED) )
		InsertLabelName(window,&GR[8]);

	OnGadget(&PropSlider1,window,NULL); OnGadget(&PropSlider2,window,NULL);

	if ( GR == ExpDec_GR )
		UA_ClearButton(window, &GR[11], AREA_PEN);

	UA_SetStringToGadgetString(&GR[8],str);
	if ( GR == VarDec_GR )
		CheckDeclStr(&(ObjectRecord.scriptSIR), str);
	else
		CheckExprStr(&(ObjectRecord.scriptSIR), str);

	strcpy(list[ 0 ], str);	// [0] allocated by CreateDeclList

	if ( strlen(str)>=3 && !list[ 1 ] )	// check if next line is empty
	{
		numLines++;
		SR1.numEntries = numLines;

		list[ 1 ] = (UBYTE *)AllocMem(EXPRESSION_WIDTH, MEMF_ANY | MEMF_CLEAR);
		CheckDeclStr( &(ObjectRecord.scriptSIR), list[1] );

		numLines++;
		SR1.numEntries = numLines;
		selList[0] = 0;
		selList[1] = 1;
		selLine=1;
		ClearTheStrGad(window,&GR[8]);
	}
	else
	{
		selLine=0;
	}

	UA_PrintStandardList(NULL,-1,NULL);	// init static
	UA_PrintStandardList(&SR1,topEntry1,list);

	if ( selLine!=(numLines-1) )
	{
		UA_EnableButton(window, &GR[ 9]);
		UA_EnableButton(window, &GR[10]);
	}

	/**** do rest of event handling ****/

	while(loop)
	{
		UA_doStandardWait(window,&CED);

		if ( CED.Class==IDCMP_MOUSEBUTTONS )
		{
			if ( CED.extraClass==IDCMP_GADGETDOWN || CED.extraClass==IDCMP_GADGETUP )
			{
				g = (struct Gadget *)CED.IAddress;
				if (g)
				{
					ID = g->GadgetID;
					if ( GR == VarDec_GR )
					{
						UA_ScrollStandardList(&SR1, &topEntry1, &PropSlider1, list, &CED);
					}
					else
					{
						if ( ID==1 )
							UA_ScrollStandardList(&SR1, &topEntry1, &PropSlider1, list, &CED);
						else if ( ID==2 )
							UA_ScrollStandardList(&SR2, &topEntry2, &PropSlider2, sublist, &CED);
					}
				}
			}
			else if ( CED.Code==SELECTDOWN )
			{
				ID = UA_CheckGadgetList(window, GR, &CED);
				switch(ID)
				{
					case 2:	// OK
do_ok:
						UA_HiliteButton(window, &GR[2]);
						loop=FALSE;
						retVal=TRUE;
						break;

					case 3:	// Cancel
do_cancel:
						UA_HiliteButton(window, &GR[3]);
						loop=FALSE;
						retVal=FALSE;
						break;

					case 6:		// Decl/Expr list 
						selLine=-1;
						line = UA_SelectStandardListLine(&SR1,topEntry1,FALSE,&CED,TRUE,TRUE);
						if ( (topEntry1+line)>(numLines-1) )	// last line
							line=-1;
						if ( line != -1 )
						{
							selLine=topEntry1+line;
							if ( selList[ selLine ] )
							{
								UA_EnableButton(window, &GR[ 8]);
								UA_SetStringGadgetToString(window,&GR[8],list[ selLine ]);
								UA_EnableButton(window, &GR[ 9]);
								UA_EnableButton(window, &GR[10]);
							}
							else
							{
								UA_DisableButton(window, &GR[ 8], gui_pattern);
								UA_DisableButton(window, &GR[ 9], gui_pattern);
								UA_DisableButton(window, &GR[10], gui_pattern);
							}
						}
						if ( line==-1 || selLine==(numLines-1) )	// last line
						{
							UA_DisableButton(window, &GR[ 9], gui_pattern);
							UA_DisableButton(window, &GR[10], gui_pattern);
						}
						if ( line==-1 )
						{
							UA_DisableButton(window, &GR[ 8], gui_pattern);
							for(i=0; i<MAX_LOCAL_EVENTS; i++)
								selList[i] = 0;
							UA_PrintStandardList(NULL,-1,NULL);	// init static
							UA_PrintStandardList(&SR1,topEntry1,list);
						}
						break;

					case 9:		// New
						UA_HiliteButton(window, &GR[ID]);
						if ( selLine != -1 )
						{
							for(i=numLines; i>selLine; i--)
								list[ i ] = list[ i-1	];
							numLines++;
							SR1.numEntries = numLines;
							list[ selLine ] = (UBYTE *)AllocMem(EXPRESSION_WIDTH, MEMF_ANY | MEMF_CLEAR);
							CheckDeclStr( &(ObjectRecord.scriptSIR), list[ selLine ] );
							if (	(selLine >= topEntry1) &&
										(selLine < (topEntry1+SR1.numDisplay)) &&
										((selLine-topEntry1)==(SR1.numDisplay-1)) )
							{	// last line of scroll list
								topEntry1++;
							}
							UA_SetPropSlider(	window, &PropSlider1, SR1.numEntries,
																SR1.numDisplay, topEntry1);
							UA_ClearButton(window, &GR[6], AREA_PEN);
							UA_PrintStandardList(NULL,-1,NULL);	// init static
							UA_PrintStandardList(&SR1,topEntry1,list);
							ClearTheStrGad(window,&GR[8]);
						}
						break;

					case 10:	// Del
						UA_HiliteButton(window, &GR[ID]);
						if ( selLine != -1 )
						{
							if ( list[ selLine ] )
							{
								FreeMem( list[ selLine ], EXPRESSION_WIDTH );
								list[ selLine ] = NULL;
							}
							for(i=selLine; i<(numLines-1); i++)
							{
								list[ i ] = list[ i+1	];
								list[ i+1 ] = NULL;
							}
							numLines--;
							SR1.numEntries = numLines;
							if (	(selLine >= topEntry1) &&
										(selLine < (topEntry1+SR1.numDisplay)) )
							{
								if (topEntry1>0)
									topEntry1--;
							}
							UA_SetPropSlider(	window, &PropSlider1, SR1.numEntries,
																SR1.numDisplay, topEntry1);
							UA_EnableButton(window, &GR[ 8]);
							UA_SetStringGadgetToString(window,&GR[8],list[ selLine ]);
							UA_ClearButton(window, &GR[6], AREA_PEN);
							UA_PrintStandardList(NULL,-1,NULL);	// init static
							UA_PrintStandardList(&SR1,topEntry1,list);
						}
						if ( numLines==1 || selLine==-1 || selLine==(numLines-1) )
						{
							UA_DisableButton(window, &GR[ 9], gui_pattern);
							UA_DisableButton(window, &GR[10], gui_pattern);
						}
						break;

					case 8:		// String
do_str:
						if ( GR == ExpDec_GR )
							UA_PrintInBox(	window, &GR[11], GR[11].x1, GR[11].y1, GR[11].x2, GR[11].y2,
															msgs[Msg_InsLabel-1], PRINT_CENTERED );

						OffGadget(&PropSlider1,window,NULL); OffGadget(&PropSlider2,window,NULL);

						while ( UA_ProcessStringGadget(window, GR, &GR[8], &CED) )
							InsertLabelName(window,&GR[8]);

						OnGadget(&PropSlider1,window,NULL); OnGadget(&PropSlider2,window,NULL);

						if ( GR == ExpDec_GR )
							UA_ClearButton(window, &GR[11], AREA_PEN);

						UA_SetStringToGadgetString(&GR[8],str);

						if ( GR == VarDec_GR )
							CheckDeclStr(&(ObjectRecord.scriptSIR),str);
						else
							CheckExprStr(&(ObjectRecord.scriptSIR),str);
						
						UA_ClearButton(window, &GR[6], AREA_PEN);

						if ( strlen(str)>=3 && selLine!=-1 && list[ selLine ] )
						{
							strcpy( list[ selLine ], str );
							if ( selLine==(numLines-1) )	// last line, add line automatically
							{
								selLine++;
								list[ selLine ] = (UBYTE *)AllocMem(EXPRESSION_WIDTH, MEMF_ANY | MEMF_CLEAR);
								CheckDeclStr( &(ObjectRecord.scriptSIR),list[ selLine ] );
								numLines++;
								SR1.numEntries = numLines;
								selList[selLine-1] = 0;
								selList[selLine  ] = 1;
								ClearTheStrGad(window,&GR[8]);
								if ( selLine >= SR1.numDisplay )
								{
									topEntry1++;
								}
								UA_SetPropSlider(	window, &PropSlider1, SR1.numEntries,
																	SR1.numDisplay, topEntry1);
							}
						}

						UA_PrintStandardList(NULL,-1,NULL);	// init static
						UA_PrintStandardList(&SR1,topEntry1,list);
						if ( selLine==-1 || selLine==(numLines-1) )
						{
							UA_DisableButton(window, &GR[ 9], gui_pattern);
							UA_DisableButton(window, &GR[10], gui_pattern);
						}
						break;
				}
			}
		}
		else if (CED.Class==IDCMP_RAWKEY)
		{
			if (CED.Code==RAW_ESCAPE)				// cancel
				goto do_cancel;
			else if (CED.Code==RAW_RETURN)	// OK
				goto do_ok;
		}
	}

	UA_CloseRequesterWindow(window,STDCOLORS);

	UA_EnableButtonQuiet(&GR[ 8]);
	UA_EnableButtonQuiet(&GR[ 9]);
	UA_EnableButtonQuiet(&GR[10]);

	if ( retVal )
	{
		if ( GR == VarDec_GR )
		{
			CopyMem( &(ObjectRecord.scriptSIR.VIList), &tempList, sizeof(struct List) );
			NewList( &(ObjectRecord.scriptSIR.VIList) );

			for(i=0; i<MAX_LOCAL_EVENTS; i++)
			{
				if ( list[i] )
				{
					strcpy(str,list[i]);
					if ( str[0] != ' ' )
					{
						vir = AllocVIR();
						if ( vir )
						{
							if ( DeclToVIR(&(ObjectRecord.scriptSIR), list[i], vir) )
								AddVIR(&(ObjectRecord.scriptSIR.VIList),vir);
							else
							{
								//Message("error during DeclToVIR");
								FreeMem(vir,sizeof(VIR));
							}
						}
					}
				}
			}

			UpdateAllForeignVIRS( &(ObjectRecord.scriptSIR) );
			RemoveAllVIRS( &tempList );
		}
		else	// expressions
		{
			RemoveAllVARS( this_node->list );
			for(i=0; i<MAX_LOCAL_EVENTS; i++)
			{
				if ( list[i] )
				{
					strcpy(str,list[i]);
					if ( str[0] != ' ' )
					{
						var = AllocVAR();
						if ( var )
						{
							if ( ExprToVAR(&(ObjectRecord.scriptSIR), list[i], var, labelStr) )
							{
								AddTail(this_node->list, (struct Node *)var);
							}
							else
							{
								//Message("error during ExprToVAR");
								FreeMem(var,sizeof(VAR));
							}
						}
					}
				}
			}
		}
	}

	for(i=0; i<MAX_LOCAL_EVENTS; i++)
	{
		if ( list[i] )
			FreeMem(list[i], EXPRESSION_WIDTH);
		if ( sublist[i] )
			FreeMem(sublist[i], EXPRESSION_WIDTH);
	}
	FreeMem(list, MAX_LOCAL_EVENTS*sizeof(UBYTE *));
	FreeMem(sublist, MAX_LOCAL_EVENTS*sizeof(UBYTE *));
	FreeMem(selList, MAX_LOCAL_EVENTS);
}

/******** CreateDeclList() ********/
/*
 * Scan list of VIRs, alloc mem for each entry. If none available,
 * at least alloc mem for 1 entry
 *
 */

void CreateDeclList(struct ScriptInfoRecord *SIR, UBYTE **list, int *numDecl,
										BOOL doAlloc)
{
VIR *this_vir;

	*numDecl = 0;
	for(this_vir = (VIR *)SIR->VIList.lh_Head; 
			(VIR *)this_vir->vir_Node.ln_Succ;	
			this_vir = (VIR *)this_vir->vir_Node.ln_Succ)
	{
		list[ *numDecl ] = (UBYTE *)AllocMem(EXPRESSION_WIDTH, MEMF_ANY | MEMF_CLEAR);
		if ( list[ *numDecl ] )
		{
			VIRToDecl(this_vir,list[ *numDecl ]);
			*numDecl = *numDecl + 1;
		}
	}

	if ( doAlloc )
	{
		list[ *numDecl ] = (UBYTE *)AllocMem(EXPRESSION_WIDTH, MEMF_ANY | MEMF_CLEAR);
		CheckDeclStr( SIR, list[ *numDecl ] );
		if ( *numDecl>0 )
			*numDecl = *numDecl + 1;
	}
}

/******** CreateExprList() ********/
/*
 * Scan list of VARs, alloc mem for each entry. If none available,
 * at least alloc mem for 1 entry
 *
 */

void CreateExprList(struct ScriptInfoRecord *SIR,
										struct ScriptNodeRecord *SNR, UBYTE **list, int *numDecl)
{
VAR *this_var;

	*numDecl = 0;
	if ( SNR )
	{
		for(this_var = (VAR *)SNR->list->lh_Head; 
				(VAR *)this_var->var_Node.ln_Succ;	
				this_var = (VAR *)this_var->var_Node.ln_Succ)
		{
			list[ *numDecl ] = (UBYTE *)AllocMem(EXPRESSION_WIDTH, MEMF_ANY | MEMF_CLEAR);
			if ( list[ *numDecl ] )
			{
				VARToExpr(SIR, list[ *numDecl ], this_var);
				*numDecl = *numDecl + 1;
			}
		}
	}

	list[ *numDecl ] = (UBYTE *)AllocMem(EXPRESSION_WIDTH, MEMF_ANY | MEMF_CLEAR);
	CheckDeclStr( SIR, list[ *numDecl ] );
	if ( *numDecl>0 )
		*numDecl = *numDecl + 1;
}

/******** CheckDeclStr() ********/

void CheckDeclStr(struct ScriptInfoRecord *SIR, STRPTR str)
{
	if ( str[0]=='\0' )
		strcat(str," ");
	else
		CheckThisVir(SIR, str);
}

/******** CheckExprStr() ********/

void CheckExprStr(struct ScriptInfoRecord *SIR, STRPTR str)
{
	if ( str[0]=='\0' )
		strcat(str," ");
	else
		CheckThisVar(SIR, str);
}

/******** ClearTheStrGad() ********/

void ClearTheStrGad(struct Window *window, struct GadgetRecord *GR)
{
struct StringRecord *SR_ptr;

	SR_ptr = (struct StringRecord *)GR->ptr;
	if ( SR_ptr && SR_ptr->buffer )
		SR_ptr->buffer[0] = '\0';

	UA_ClearButton(window,GR,AREA_PEN);
}

/******** CheckThisVir() ********/

BOOL CheckThisVir(struct ScriptInfoRecord *SIR, STRPTR decl)
{
VIR *vir;
TEXT temp[EXPRESSION_WIDTH];
BOOL retval;

	strcpy(temp, decl);

	vir = (VIR *)AllocMem(sizeof(VIR),MEMF_ANY|MEMF_CLEAR);
	if (!vir)
		return(FALSE);

	retval = DeclToVIR(SIR, temp, vir);
	//VIRToDecl(vir, temp);
	if ( retval )
	{
		VIRToDecl(vir, temp);
		strcpy(decl, temp);
	}

	FreeMem(vir,sizeof(VIR));

	return(retval);
}

#endif

/******** CheckThisVar() ********/

BOOL CheckThisVar(struct ScriptInfoRecord *SIR, STRPTR decl)
{
VAR *var;
TEXT temp[EXPRESSION_WIDTH], labelStr[EXPRESSION_WIDTH];
BOOL retval;
VCR *work_vcr, *next_vcr;

	strcpy(temp, decl);

	var = (VAR *)AllocMem(sizeof(VAR),MEMF_ANY|MEMF_CLEAR);
	if (!var)
		return(FALSE);
	NewList( &(var->var_CondList) );

	retval = ExprToVAR(SIR, temp, var, labelStr);
	if ( retval )
	{
		VARToExpr(SIR, temp, var);
		strcpy(decl, temp);

		// STAR NEW
		work_vcr = (VCR *)var->var_CondList.lh_Head;	// first node
		while( next_vcr = (VCR *)(work_vcr->vcr_Node.ln_Succ) )
		{		
			FreeMem(work_vcr,sizeof(VCR));
			work_vcr = next_vcr;
		}
		// END NEW
	}

	FreeMem(var,sizeof(VAR));

	return(retval);
}

#ifndef USED_FOR_PLAYER

/******** CountVIRS() ********/

int CountVIRS(struct List *list)
{
VIR *this_vir;
int num=0;

	for(this_vir = (VIR *)list->lh_Head; 
			(VIR *)this_vir->vir_Node.ln_Succ;	
			this_vir = (VIR *)this_vir->vir_Node.ln_Succ)
		num++;

	return( num );
}

/******** UpdateAllForeignVIRS() ********/

void UpdateAllForeignVIRS( struct ScriptInfoRecord *SIR )
{
int i,dummy;
SNRPTR this_node;
struct List *list;
VAR *work_var, *next_var;
VCR *work_vcr, *next_vcr;
VIR *firstVIR;

	if ( SIR->VIList.lh_TailPred == (struct Node *)&SIR->VIList )
		return;

	firstVIR = (VIR *)SIR->VIList.lh_Head;

	for(i=0; i<CPrefs.MaxNumLists; i++)
	{
		if ( SIR->allLists[i] )
		{
			list = SIR->allLists[i];
			if ( list->lh_TailPred != (struct Node *)list )
			{
				for(this_node=(SNRPTR)list->lh_Head; this_node->node.ln_Succ;
						this_node=(SNRPTR)this_node->node.ln_Succ)
				{
					if ( this_node->nodeType == TALK_VARS )
					{
						if ( this_node->list->lh_TailPred != (struct Node *)this_node->list )
						{
							work_var = (VAR *)this_node->list->lh_Head;	// first node
							while( next_var = (VAR *)(work_var->var_Node.ln_Succ) )
							{
								if (	work_var->var_CondList.lh_TailPred !=
											(struct Node *)&(work_var->var_CondList) )
								{
									work_vcr = (VCR *)work_var->var_CondList.lh_Head;	// first node
									while( next_vcr = (VCR *)(work_vcr->vcr_Node.ln_Succ) )
									{
										// look into work_vcr
	
										if ( work_vcr->vcr_CheckBits & VC_CHECKVIR )
										{
											work_vcr->vcr_Check.vc_VIR =
												FindVIR(SIR, work_vcr->vcr_Check.vc_VIR->vir_Name, &dummy);
											if ( !work_vcr->vcr_Check.vc_VIR )
												work_vcr->vcr_Check.vc_VIR = firstVIR;
										}
										work_vcr = next_vcr;
									}
								}
								// look into work_var

								if ( work_var->var_ResultVIR )
								{
									work_var->var_ResultVIR =
													FindVIR(SIR, work_var->var_ResultVIR->vir_Name, &dummy);
								}
								if ( !work_var->var_ResultVIR )
									work_var->var_ResultVIR = firstVIR;

								if ( work_var->var_SourceBits & SB_SOURCE1VIR )
								{
									work_var->var_Source1.vs_VIR =
									FindVIR(SIR, work_var->var_Source1.vs_VIR->vir_Name, &dummy);
									if ( !work_var->var_Source1.vs_VIR )
										work_var->var_Source1.vs_VIR = firstVIR;
								}

								if ( work_var->var_SourceBits & SB_SOURCE2VIR )
								{
									work_var->var_Source2.vs_VIR =
									FindVIR(SIR, work_var->var_Source2.vs_VIR->vir_Name, &dummy);
									if ( !work_var->var_Source2.vs_VIR )
										work_var->var_Source2.vs_VIR = firstVIR;
								}

								if ( work_var->var_ResultVIR_v2 )
								{
									work_var->var_ResultVIR_v2 =
													FindVIR(SIR, work_var->var_ResultVIR_v2->vir_Name, &dummy);
								}
								if ( !work_var->var_ResultVIR_v2 )
									work_var->var_ResultVIR_v2 = firstVIR;

								if ( work_var->var_SourceBits_v2 & SB_SOURCE3VIR )
								{
									work_var->var_Source3.vs_VIR =
									FindVIR(SIR, work_var->var_Source3.vs_VIR->vir_Name, &dummy);
									if ( !work_var->var_Source3.vs_VIR )
										work_var->var_Source3.vs_VIR = firstVIR;
								}

								if ( work_var->var_SourceBits_v2 & SB_SOURCE4VIR )
								{
									work_var->var_Source4.vs_VIR =
									FindVIR(SIR, work_var->var_Source4.vs_VIR->vir_Name, &dummy);
									if ( !work_var->var_Source4.vs_VIR )
										work_var->var_Source4.vs_VIR = firstVIR;
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

/******** Copy_Vars_Info() ********/

void Copy_Vars_Info(struct ScriptNodeRecord *oldSNR, struct ScriptNodeRecord *newSNR)
{
VAR *work_var, *next_var, *var;
VCR *work_vcr, *next_vcr, *vcr;

	newSNR->list = (struct List *)AllocMem(sizeof(struct List), MEMF_ANY | MEMF_CLEAR);
	if ( newSNR->list )
	{
		NewList(newSNR->list);

		if ( oldSNR->list->lh_TailPred != (struct Node *)oldSNR->list )
		{
			work_var = (VAR *)oldSNR->list->lh_Head;	// first node
			while( next_var = (VAR *)(work_var->var_Node.ln_Succ) )
			{
				// allocate new VAR, add it to new list, copy old VAR to new VAR

				var = (struct VarAssignRecord *)AllocVAR();
				if (var)
				{
					AddTail(newSNR->list, (struct Node *)var);

					/**** copy VAR ****/
					var->var_CheckBeforeAssign = work_var->var_CheckBeforeAssign;
					var->var_Operator = work_var->var_Operator;
					var->var_SourceBits = work_var->var_SourceBits;
					var->var_ResultVIR = work_var->var_ResultVIR;
					CopyMem(&work_var->var_Source1, &var->var_Source1, sizeof(VARCONTENTS));
					CopyMem(&work_var->var_Source2, &var->var_Source2, sizeof(VARCONTENTS));

					var->var_Operator_v2 = work_var->var_Operator_v2;
					var->var_SourceBits_v2 = work_var->var_SourceBits_v2;
					var->var_ResultVIR_v2 = work_var->var_ResultVIR_v2;
					CopyMem(&work_var->var_Source3, &var->var_Source3, sizeof(VARCONTENTS));
					CopyMem(&work_var->var_Source4, &var->var_Source4, sizeof(VARCONTENTS));
				}

				// copy list of VCR's attached to old VAR

				if (	work_var->var_CondList.lh_TailPred !=
							(struct Node *)&(work_var->var_CondList) )
				{
					work_vcr = (VCR *)work_var->var_CondList.lh_Head;	// first node
					while( next_vcr = (VCR *)(work_vcr->vcr_Node.ln_Succ) )
					{
						vcr = (VCR *)AllocMem(sizeof(VCR),MEMF_CLEAR | MEMF_ANY);
						if ( vcr )
						{
							AddTail(&var->var_CondList, (struct Node *)vcr);

							/**** copy VAR ****/
							vcr->vcr_Condition = work_vcr->vcr_Condition;
							vcr->vcr_CheckBits = work_vcr->vcr_CheckBits;
							vcr->vcr_JumpType = work_vcr->vcr_JumpType;
							CopyMem(&work_vcr->vcr_Check, &vcr->vcr_Check, sizeof(VARCONTENTS));
							//vcr->vcr_LabelSNR = work_vcr->vcr_LabelSNR;
							vcr->vcr_LabelSNR = FindLabel( &(ObjectRecord.scriptSIR), work_vcr->vcr_LabelName );
							strcpy( vcr->vcr_LabelName, work_vcr->vcr_LabelName );
						}
						work_vcr = next_vcr;
					}
				}

				work_var = next_var;
			}
		}
	}
}

/******** InsertLabelName() ********/

void InsertLabelName(struct Window *window, struct GadgetRecord *GR)
{
TEXT str[100];
struct ScriptNodeRecord tempNode;

	UA_SetStringToGadgetString(GR,str);

	tempNode.objectName[0] = '\0';

	standAlonePickLabel(scriptWindow,&tempNode);

	if (	(strlen(str)+strlen(tempNode.objectName+3)) < EXPRESSION_WIDTH &&
				(tempNode.objectName[0] != '\0') )
	{
		strcat(str, " \"");
		strcat(str, tempNode.objectName);
		strcat(str, "\"");

		UA_SetStringGadgetToString(window,GR,str);
	}
}

#endif

/******** RemoveAllVIRS() ********/

void RemoveAllVIRS(struct List *list)
{
VIR *work_vir, *next_vir;

	if ( list->lh_TailPred != (struct Node *)list )
	{
		work_vir = (VIR *)list->lh_Head;	// first node
		while( next_vir = (VIR *)(work_vir->vir_Node.ln_Succ) )
		{
			FreeMem(work_vir,sizeof(VIR));
			work_vir = next_vir;
		}
	}
	NewList( list );
}

/******** RemoveAllVARS() ********/

void RemoveAllVARS(struct List *list)
{
VAR *work_var, *next_var;
VCR *work_vcr, *next_vcr;

	if ( list->lh_TailPred != (struct Node *)list )
	{
		work_var = (VAR *)list->lh_Head;	// first node
		while( next_var = (VAR *)(work_var->var_Node.ln_Succ) )
		{
			if (	work_var->var_CondList.lh_TailPred !=
						(struct Node *)&(work_var->var_CondList) )
			{
				/**** free list of VCR's inside this VAR ****/
				work_vcr = (VCR *)work_var->var_CondList.lh_Head;	// first node
				while( next_vcr = (VCR *)(work_vcr->vcr_Node.ln_Succ) )
				{		
					FreeMem(work_vcr,sizeof(VCR));
					work_vcr = next_vcr;
				}
			}
			FreeMem(work_var,sizeof(VAR));
			work_var = next_var;
		}
	}
	NewList( list );
}

/******** E O F ********/
