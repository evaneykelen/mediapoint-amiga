#include "nb:pre.h"
#include <exec/types.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/keymap_protos.h>
#include <clib/potgo_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include "minc:types.h"
#include "minc:defs.h"
#include "minc:errors.h"
#include "minc:process.h"
#include "minc:ge.h"
#include "minc:system.h"
#include "external.h"
#include "nb:capsstructs.h"

STATIC VIR *InputFindVIR(struct List *VIList, STRPTR varName);
STATIC BOOL GetTwoArgs(STRPTR declStr, STRPTR arg1, STRPTR arg2);

/******** InterpretAssignment() ********/

void InterpretAssignment(STRPTR assignment, MLSYSTEM *MLSystem)
{
TEXT arg1[50], arg2[50];
VIR *vir1, *vir2;
int val, len;

	if ( !GetTwoArgs(assignment, arg1, arg2) )
		return;

	vir1 = InputFindVIR(MLSystem->VIList,arg1);
	if ( vir1 && arg2[0] )
	{
		if ( isdigit( arg2[0] ) )	// VAR = VALUE
		{
			sscanf(arg2,"%d",&val);
/*
{
char tmp[20];
	sprintf(tmp, "make %s = int %d\n", arg1, (WORD)val);
	KPrintF(tmp);
}
*/
			vir1->vir_Contents.vc_Type = VCT_INTEGER;
			vir1->vir_Contents.vc_Value.vv_Integer = val;
		}
		else if ( arg2[0]=='\"' )	// VAR = "STRING"
		{
//KPrintF("make %s = string %s\n", arg1, arg2);

			vir1->vir_Contents.vc_Type = VCT_STRING;
			len = strlen(arg2);
			if ( len>=3 && len<50 )
				stccpy(vir1->vir_Contents.vc_Value.vv_String,&arg2[1],len-1);
		}
		else						// VAR1 = VAR2
		{
//KPrintF("make %s = var %s\n", arg1, arg2);

			vir2 = InputFindVIR(MLSystem->VIList,arg2);
			if ( vir2 )
				CopyMem(&vir2->vir_Contents,&vir1->vir_Contents,sizeof(VARCONTENTS));
		}
	}
}		

/******** InputFindVIR() ********/

STATIC VIR *InputFindVIR(struct List *VIList, STRPTR varName)
{
VIR *this_vir;

	for(this_vir = (VIR *)VIList->lh_Head; 
			(VIR *)this_vir->vir_Node.ln_Succ;	
			this_vir = (VIR *)this_vir->vir_Node.ln_Succ)
	{
		if ( !stricmp(varName, this_vir->vir_Name) )
			return(this_vir);
	}
	return( NULL );
}

/******** GetTwoArgs() ********/

STATIC BOOL GetTwoArgs(STRPTR declStr, STRPTR arg1, STRPTR arg2)
{
int numChars, argNum, len, numSeen;
char *strPtr;

	strPtr = declStr;
	len = strlen(strPtr);
	argNum=0;
	numSeen=0;
	while(1)
	{
		if (*strPtr==0)
			break;
		numChars = stcarg(strPtr, " 	,");	// space, tab, comma
		if (numChars>=1)
		{
			if ( argNum==0 )
				stccpy(arg1, strPtr, numChars+1);
			else if ( argNum==2 )
				stccpy(arg2, strPtr, numChars+1);
			argNum++;
		}
		strPtr += numChars+1;
		numSeen += numChars+1;
		if (numSeen>len)
			break;		
	}

/*
KPrintF("[%s] [%s]\n",arg1,arg2);
if ( argNum==3 )
KPrintF("3\n");
*/

	return(TRUE);
}

/******** E O F ********/
