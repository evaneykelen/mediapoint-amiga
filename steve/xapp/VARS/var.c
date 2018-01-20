#include "nb:pre.h"

#include <workbench/startup.h>
#include <exec/types.h>
#include <libraries/dosextens.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include <rexx/storage.h>
#include <rexx/rxslib.h>
#include <rexx/errors.h>
#include "to:rexx3/simplerexx.h"

#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "external.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

#include <stdlib.h>
#include <stdio.h>

VARCONTENTS *VCCopyOf(VIR *, struct List *);
void Add(VARCONTENTS *, VARCONTENTS *, VARCONTENTS *);
void Subtract(VARCONTENTS *, VARCONTENTS *, VARCONTENTS *);
void Multiply(VARCONTENTS *, VARCONTENTS *, VARCONTENTS *);
void Divide(VARCONTENTS *, VARCONTENTS *, VARCONTENTS *);
void CalculateOn(VARCONTENTS *, VARCONTENTS *, VARCONTENTS *, UWORD);
BOOL CheckCondition(VARCONTENTS *, VARCONTENTS *, UWORD);
VCR *CheckConditionsOn(struct List *, VARCONTENTS *, struct List *, VAR *);
void PrintVC(VARCONTENTS *);
VCR *CalculateVariablesOfSNR(struct List *, struct List *);
void DoVars(PROCESSINFO *ThisPI);
void DoPostMakeCalc(VAR *CurVar, struct List *VIList);

/**********************************************
*Func : Answer the VARCONTENTS area of a variable
*		in to copied VIList list.
*in   : aVir -> ptr to a vir
*out  : VARCONTENTS -> ptr to a var
*/

VARCONTENTS *VCCopyOf(aVir, VIList)
VIR *aVir;
struct List	*VIList;
{
  VIR *VI;

	for(VI = (VIR *)VIList->lh_Head; 
		(VIR *)VI->vir_Node.ln_Succ;	
		VI = (VIR *)VI->vir_Node.ln_Succ)
	{
		if(!strnicmp(VI->vir_Name,aVir->vir_Name,20))
			return(&VI->vir_Contents);
	}
	return(NULL);
}

/***************************************************
*Func : add to vars
*in   : Src1, Src2, Dest
*out  : -
*/
void Add( Src1, Src2, Dest)
VARCONTENTS *Src1,
			*Src2,
			*Dest;
{
  char Str[101];

	if(Src1->vc_Type == VCT_STRING)
	{
		// string + string
		if(Src2->vc_Type == VCT_STRING)
		{
			strcpy(Str,Src1->vc_Value.vv_String);
			strcat(Str,Src2->vc_Value.vv_String);
			stccpy(Dest->vc_Value.vv_String,Str,49);
			Dest->vc_Type = VCT_STRING;
		}
		else if(Src2->vc_Type == VCT_INTEGER)	// string + integer
		{
			sprintf(Str,"%s%d",Src1->vc_Value.vv_String,Src2->vc_Value.vv_Integer);			
			stccpy(Dest->vc_Value.vv_String,Str,49);
			Dest->vc_Type = VCT_STRING;
		}
		else	// VCT_EMPTY
		{
			stccpy(Dest->vc_Value.vv_String,Src1->vc_Value.vv_String,49);
			Dest->vc_Type = VCT_STRING;
		}
	}
	else
	{
		// integer + string
		if(Src2->vc_Type == VCT_STRING)
		{
			sprintf(Str,"%d%s",Src1->vc_Value.vv_Integer,Src2->vc_Value.vv_String);			
			stccpy(Dest->vc_Value.vv_String,Str,49);
			Dest->vc_Type = VCT_STRING;
		}
		else if(Src2->vc_Type == VCT_INTEGER)	// integer + integer
		{
			Dest->vc_Type = VCT_INTEGER;
			Dest->vc_Value.vv_Integer = Src1->vc_Value.vv_Integer + Src2->vc_Value.vv_Integer; 
		}
		else	// VCT_EMPTY
		{
			Dest->vc_Type = VCT_INTEGER;
			Dest->vc_Value.vv_Integer = Src1->vc_Value.vv_Integer; 
		}
	}
}

/***************************************************
*Func : subtract to vars
*in   : Src1, Src2, Dest
*out  : -
*/
void Subtract( Src1, Src2, Dest)
VARCONTENTS *Src1,
			*Src2,
			*Dest;
{
	if(Src1->vc_Type == VCT_STRING)
	{
		// string - string
		if(Src2->vc_Type == VCT_STRING)
		{
		}
		else	// string - integer
		{
		}
	}
	else
	{
		// integer - string
		if(Src2->vc_Type == VCT_STRING)
		{
		}
		else	// integer - integer
		{
			Dest->vc_Type = VCT_INTEGER;
			Dest->vc_Value.vv_Integer = Src1->vc_Value.vv_Integer - Src2->vc_Value.vv_Integer; 
		}
	}
}

/***************************************************
*Func : multiply to vars
*in   : Src1, Src2, Dest
*out  : -
*/
void Multiply( Src1, Src2, Dest)
VARCONTENTS *Src1,
			*Src2,
			*Dest;
{
	if(Src1->vc_Type == VCT_STRING)
	{
		// string * string
		if(Src2->vc_Type == VCT_STRING)
		{
		}
		else	// string * integer
		{
		}
	}
	else
	{
		// integer * string
		if(Src2->vc_Type == VCT_STRING)
		{
		}
		else	// integer * integer
		{
			Dest->vc_Type = VCT_INTEGER;
			Dest->vc_Value.vv_Integer = Src1->vc_Value.vv_Integer * Src2->vc_Value.vv_Integer; 
		}
	}
}

/***************************************************
*Func : divide to vars
*in   : Src1, Src2, Dest
*out  : -
*/
void Divide( Src1, Src2, Dest)
VARCONTENTS *Src1,
			*Src2,
			*Dest;
{
	if(Src1->vc_Type == VCT_STRING)
	{
		// string / string
		if(Src2->vc_Type == VCT_STRING)
		{
		}
		else	// string / integer
		{
		}
	}
	else
	{
		// integer / string
		if(Src2->vc_Type == VCT_STRING)
		{
		}
		else	// integer / integer
		{
			Dest->vc_Type = VCT_INTEGER;
			if(Src2->vc_Value.vv_Integer)
				Dest->vc_Value.vv_Integer = Src1->vc_Value.vv_Integer / Src2->vc_Value.vv_Integer;
			else
				Dest->vc_Value.vv_Integer = 0; 
		}
	}
}

/******************************************************
*Func : perform a calculation operand on a destination
*		C = A (oper) B
*in   : Src1 -> ptr to VARCONTENTS of A
*       Src2 -> ptr to VARCONTENTS of B
*		Dest -> ptr to VARCONTENTS of C
*
*		about complex var types:
*		C = A + B.	A = string: "plaatje"; B = integer: 3 => C = "plaatje3"
*		C = A + B.  A = integer: 3; B = string: "plaatje" => C = "3plaatje"
*
*		integer to integer just follows normal calc rules
*		"plaatje" + "test" => "plaatjetest"
*out  : -
*/ 
void CalculateOn(Src1,Src2,Dest,Oper)
VARCONTENTS *Src1,
			*Src2,
			*Dest;
UWORD		Oper;
{
	switch(Oper)
	{
		case OPER_ADD:
				//PrintSer(" OPER ADD ");
				Add(Src1,Src2,Dest);
				break;
		case OPER_SUBTRACT:
				//PrintSer(" OPER SUB ");
				Subtract(Src1,Src2,Dest);
				break;
		case OPER_MULTIPLY:
				//PrintSer(" OPER MULT ");
				Multiply(Src1,Src2,Dest);
				break;
		case OPER_DIVIDE:
				//PrintSer(" OPER DIVIDE ");
				Divide(Src1,Src2,Dest);
				break;
		default:
				//PrintSer(" OPER ILLEGAL ");
				break;
	}
} 

/*****************************************************
*Func : Compare two VARCONTENTS struct
*		and return TRUE if the condition matches
*in   : Vc1
*		Vc2
*		Condition
*out  : True -> the condition was true
*		False -> the condition was false
*/
BOOL CheckCondition(Vc1, Vc2, Cond)
VARCONTENTS *Vc1,
			*Vc2;
UWORD Cond;
{
  int Res;

	// types must be the same
	if(Vc1->vc_Type == Vc2->vc_Type)
	{
		if(Vc1->vc_Type == VCT_STRING)
			Res = strcmpi(Vc1->vc_Value.vv_String,Vc2->vc_Value.vv_String);
		else
			Res = Vc1->vc_Value.vv_Integer - Vc2->vc_Value.vv_Integer;

		switch(Cond)
		{
			case COND_LESS:
					//PrintSer(" LESS ");
					if(Res < 0) return(TRUE);
					break;
			case COND_LESSEQUAL:
					//PrintSer(" LESSEQUAL ");
					if(Res <= 0) return(TRUE);
					break;
			case COND_EQUAL:
					//PrintSer(" EQUAL ");
					if(Res == 0) return(TRUE);
					break;
			case COND_GREATEREQUAL:
					//PrintSer(" GRTREQUAL ");
					if(Res >= 0) return(TRUE);
					break;
			case COND_GREATER:
					//PrintSer(" GREATER ");
					if(Res > 0) return(TRUE);
					break;
			case COND_NOTEQUAL:
					//PrintSer(" UNEQUAL ");
					if(Res != 0) return(TRUE);
					break;
			default:
					//PrintSer(" ILL ");
					break;
		}
	}
	return(FALSE);
}

/*******************************************************
*Func : check all conditions for a variable
*		and return a ptr to the VCR that caused the hit
*in   : CondList -> ptr to condition list
*		CheckVar -> Var that must be checked
*/
VCR *CheckConditionsOn(CondList,CheckVar,VIList,CurVar)
struct List *CondList;
VARCONTENTS *CheckVar;
struct List	*VIList;
VAR *CurVar;
{
  VARCONTENTS *Vc;
  VCR *CurVCR;

	for(CurVCR = (VCR *)CondList->lh_Head; 
		(VCR *)CurVCR->vcr_Node.ln_Succ;	
		CurVCR = (VCR *)CurVCR->vcr_Node.ln_Succ)
	{
		if(CurVCR->vcr_CheckBits & VC_CHECKVIR)
			Vc = VCCopyOf(CurVCR->vcr_Check.vc_VIR,VIList);
		else
			Vc = &CurVCR->vcr_Check.vc_Direct;

		if(CheckCondition(CheckVar, Vc, CurVCR->vcr_Condition))
		{
			// START NEW
			if ( CurVCR->vcr_JumpType == TG_MAKE )
			{
				DoPostMakeCalc(CurVar,VIList);
				return(NULL);	// and not CurVCR!
			}
			// END NEW

			return(CurVCR);		
		}
	}
	return(NULL);
}


/******************************************************
*Func : Print the varcontents
*in   : Vc -> ptr to VARCONTENTS
*out  : -
*/					
void PrintVC( VARCONTENTS *Vc)
{
#if _PRINTF
	if(Vc->vc_Type == VCT_STRING)
		PrintSer("Str[%s] ",Vc->vc_Value.vv_String);
	if(Vc->vc_Type == VCT_INTEGER)
		PrintSer("Int[%d] ",Vc->vc_Value.vv_Integer);
#endif
}

/******************************************************
*Func : For all expressions of the SNR, calculate the
*		new varaiable value
*in   : SNRVarList -> ptr to list of expressions
*out  : NULL -> no valid condition found
*		else -> ptr to VCR that caused the hit
*/					
VCR *CalculateVariablesOfSNR(SNRVarList,VIList)
struct List *SNRVarList;
struct List	*VIList;
{
  VAR *CurVar;
  VARCONTENTS *Source1, *Source2;
  VCR *HitVCR;

	if(SNRVarList == NULL)
		return(NULL);

	HitVCR = NULL;
	for(CurVar = (VAR *)SNRVarList->lh_Head; 
		(VAR *)CurVar->var_Node.ln_Succ;	
		CurVar = (VAR *)CurVar->var_Node.ln_Succ)
	{
		if(!(CurVar->var_SourceBits & SB_NOSOURCES))
		{
			if(CurVar->var_SourceBits & SB_SOURCE1VIR)
			{
				//PrintSer("VAR[%s] ",CurVar->var_Source1.vs_VIR->vir_Name);
				Source1 = VCCopyOf(CurVar->var_Source1.vs_VIR,VIList);
			}
			else
			{
				//PrintSer("DIR ");
				Source1 = &CurVar->var_Source1.vs_Direct;
			}

			//PrintVC(Source1);			

			if(CurVar->var_SourceBits & SB_SOURCE2VIR)
			{
				//PrintSer("VAR[%s] ",CurVar->var_Source2.vs_VIR->vir_Name);
				Source2 = VCCopyOf(CurVar->var_Source2.vs_VIR,VIList);
			}
			else
			{
				//PrintSer("DIR ");
				Source2 = &CurVar->var_Source2.vs_Direct;
			}
			//PrintVC(Source2);			
		}

		if(CurVar->var_CheckBeforeAssign)
		{
			HitVCR = CheckConditionsOn(&CurVar->var_CondList,VCCopyOf(CurVar->var_ResultVIR,VIList),VIList,CurVar);
			if(!(CurVar->var_SourceBits & SB_NOSOURCES))
				CalculateOn(Source1,Source2,VCCopyOf(CurVar->var_ResultVIR,VIList),CurVar->var_Operator);
		}
		else
		{
			if(!(CurVar->var_SourceBits & SB_NOSOURCES))
				CalculateOn(Source1,Source2,VCCopyOf(CurVar->var_ResultVIR,VIList),CurVar->var_Operator);

			//PrintSer("RESVAR[%s] ",CurVar->var_ResultVIR->vir_Name);
			//PrintVC(VCCopyOf(CurVar->var_ResultVIR));
			//PrintSer("\n");

			HitVCR = CheckConditionsOn(&CurVar->var_CondList,VCCopyOf(CurVar->var_ResultVIR,VIList),VIList,CurVar);
		}
		//PrintSer("\n");
		if(HitVCR)
			break;
	}

	return(HitVCR);
}

/******** DoVars() ********/

void DoVars(PROCESSINFO *ThisPI)
{
MLSYSTEM *MLSystem;
VCR *vcr;
TEXT cmd[100];

	MLSystem = ThisPI->pi_Arguments.ar_Worker.aw_MLSystem;
	vcr = CalculateVariablesOfSNR(ThisPI->pi_Arguments.ar_Worker.aw_List, MLSystem->VIList);
	if ( vcr )
	{
/*
	char str[100];
		sprintf(str,"sent snr is %x\n", vcr->vcr_LabelSNR);
		KPrintF(str);
*/
		sprintf(cmd,"GOTO 0x%d",vcr->vcr_LabelSNR);
		IssueRexxCmd("MP_VARS","MEDIAPOINT",cmd,NULL,NULL);
	}
/*
	else
		KPrintF("no vcr\n");
*/
}

/******** DoPostMakeCalc() ********/

void DoPostMakeCalc(VAR *CurVar, struct List *VIList)
{
VARCONTENTS *Source3, *Source4;

	if(!(CurVar->var_SourceBits_v2 & SB_NOSOURCES))
	{
		if(CurVar->var_SourceBits_v2 & SB_SOURCE3VIR)
		{
			//PrintSer("VAR[%s] ",CurVar->var_Source3.vs_VIR->vir_Name);
			Source3 = VCCopyOf(CurVar->var_Source3.vs_VIR,VIList);
		}
		else
		{
			//PrintSer("DIR ");
			Source3 = &CurVar->var_Source3.vs_Direct;
		}

		//PrintVC(Source3);			

		if(CurVar->var_SourceBits_v2 & SB_SOURCE4VIR)
		{
			//PrintSer("VAR[%s] ",CurVar->var_Source4.vs_VIR->vir_Name);
			Source4 = VCCopyOf(CurVar->var_Source4.vs_VIR,VIList);
		}
		else
		{
			//PrintSer("DIR ");
			Source4 = &CurVar->var_Source4.vs_Direct;
		}
		//PrintVC(Source4);			
	}

	if( !(CurVar->var_SourceBits_v2 & SB_NOSOURCES) )
		CalculateOn(Source3,Source4,VCCopyOf(CurVar->var_ResultVIR_v2,VIList),CurVar->var_Operator_v2);
}

/******** E O F ********/
