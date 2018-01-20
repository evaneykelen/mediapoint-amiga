/************************************************************************
*File : Rexxcmd.c 
*Desc : Arexx commands
*/

#include <workbench/startup.h>
#include <exec/types.h>
#include <rexx/storage.h>
#include <rexx/rxslib.h>
#include <rexx/errors.h>

#include <libraries/dosextens.h>

#include "nb:pre.h"
#include "minc:defs.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "minc:sync.h"
#include "minc:ge.h"
#include "minc:external.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"
#include "rexx_pragma.h"
#include "rexx_proto.h"
#include "rexx.h"
#include "external.h"

extern TEXT newScript[100];
extern struct List *VIList;			// ptr of variables area of medialink
//extern struct List *DelayedVIList;	// ptr of variables area of medialink

REXXCMD RexxCmds[] =
{
  {"GOTO",		HandleGoto},
  {"TCSET", 	SetTimeCode},
  {"TCGET", 	GetTimeCode},
  {"QUIT",		QuitScript},
  {"ESCAPE",	SetEscape},
  {"MOUSE",		SetMouse},
  {"CURSOR",	SetCursor},
  {"GETPAGENR",	GetPageNr},
  {"GETNRPAGES",GetNrPages},
  {"GETSTATE",  GetState},
  {"GETVAR",  	GetVAR},
  {"SETVAR",  	SetVAR},
  {"TCGET_ASC",	GetTimeCodeAsc},
  {"NEWSCRIPT",	GetNewScript},
  {"",			NULL}
};

REXXERROR RexxError[] =
{
  {"100: Unknown command",			RC_FATAL},			// 0
  {"1: Invalid label",				RC_ERROR},			// 1
  {"2: Unknown parameter",			RC_ERROR},			// 2
  {"3: Illegal parameter size",		RC_ERROR},			// 3
  {"4: Multiple command collision", RC_WARN},			// 4
  {"5: Port closed",				RC_FATAL}			// 5
};

char ReturnCode[100];

STATIC VIR *FindArexxVIR(struct List *list, STRPTR varName);

/***************************************************
*Func : Send a new object to the processcontroller
*in   : -
*out  : -
*/
void HandleGoto( ID, SIR, Error, Result, ErrLevel)
IDENTIFIER *ID;
struct ScriptInfoRecord *SIR;
char **Error;
char **Result;
int  *ErrLevel;
{
  GEVENTDIALOGUE Msg_RexxDial;
  struct ScriptNodeRecord *snr;

	if(!stricmp(ID[0].id_Name,"NEXT"))
	{	
		Msg_RexxDial.gd_Cmd = RCMDNEXTOBJ;
		AnalyseGlobalMsg(&Msg_RexxDial, TRUE);
	}
	else
	{
		if(!stricmp(ID[0].id_Name,"PREV"))
		{
			Msg_RexxDial.gd_Cmd = RCMDPREVOBJ;
			AnalyseGlobalMsg(&Msg_RexxDial, TRUE);
		}
		else
		{
			if ( ID[0].id_Name[0]=='0' && ID[0].id_Name[1]=='x' )	// 0xaddress ID !
			{
				sscanf(&(ID[0].id_Name[2]),"%d",&snr);
//PrintSer("received snr is %x\n", snr);
				Msg_RexxDial.gd_Label.gl_SNR = snr;
				Msg_RexxDial.gd_Cmd = RCMDSTOPnJUMP;
				AnalyseGlobalMsg(&Msg_RexxDial, TRUE);
			}
			else
			{
				// argument is a label		
				if( (Msg_RexxDial.gd_Label.gl_SNR = FindLabel(SIR,ID[0].id_Name)) != NULL)
				{
					Msg_RexxDial.gd_Cmd = RCMDSTOPnJUMP;
					AnalyseGlobalMsg(&Msg_RexxDial, TRUE);
				}
				else
				{
					*Error = RexxError[1].re_Name;
					*ErrLevel = RexxError[1].re_ReturnCode;
				}
			}
		}
	}
}

/****************************************************
*Func : Set the current timecode of the synchronizer
*in   : -
*out  : -
*/
void SetTimeCode( ID, SIR, Error, Result, ErrLevel)
IDENTIFIER *ID;
struct ScriptInfoRecord *SIR;
char **Error;
char **Result;
int  *ErrLevel;
{
  SYNCDIALOGUE *SD;
  ULONG FrameCode, FCFast;
  int HH,MM,SS,FF;

	FrameCode = atoi(ID[0].id_Name);
	FF = FrameCode % 100;
	SS = (FrameCode/100) % 100;
	MM = (FrameCode/10000) % 100;
	HH = (FrameCode/1000000) % 100;

	if( (FF > 29) |
		(SS > 59) |
		(MM > 59) |
		(HH > 23)
	  )
	{
		*Error = RexxError[3].re_Name;
		*ErrLevel = RexxError[3].re_ReturnCode;
		return;
	}

	FCFast = (HH<<24)|(MM<<16)|(SS<<8)|FF;

	if( (SD = GetFreeSyncDial()) != NULL)
	{
		SD->sd_Cmd = SDC_SETTIMECODE;
		//This is illegal but it leaves out the use of an extra field
		SD->sd_Punch = (SYNCPUNCH *)FCFast;
		PutMsg(Port_PCtoS,(struct Message *)SD);
	}
}


/****************************************************
*Func : Get the current timecode of the synchronizer
*in   : -
*out  : -
*/
void GetTimeCode( ID, SIR, Error, Result, ErrLevel)
IDENTIFIER *ID;
struct ScriptInfoRecord *SIR;
char **Error;
char **Result;
int  *ErrLevel;
{
  SYNCDIALOGUE *SD;

	if( (SD = GetFreeSyncDial()) != NULL)
	{
		SD->sd_Cmd = SDC_GETTIMECODE;
		PutMsg(Port_PCtoS,(struct Message *)SD);
	}
	*ErrLevel = -1;
}

/****************************************************
*Func : Get the current timecode of the synchronizer
*in   : -
*out  : -
*/
void GetTimeCodeAsc( ID, SIR, Error, Result, ErrLevel)
IDENTIFIER *ID;
struct ScriptInfoRecord *SIR;
char **Error;
char **Result;
int  *ErrLevel;
{
  SYNCDIALOGUE *SD;

	if( (SD = GetFreeSyncDial()) != NULL)
	{
		SD->sd_Cmd = SDC_GETTIMECODE_ASC;
		PutMsg(Port_PCtoS,(struct Message *)SD);
	}
	*ErrLevel = -1;
}

/****************************************************
*Func : Terminate the current show
*in   : -
*out  : -
*/
void QuitScript( ID, SIR, Error, Result, ErrLevel)
IDENTIFIER *ID;
struct ScriptInfoRecord *SIR;
char **Error;
char **Result;
int  *ErrLevel;
{
  GEVENTDIALOGUE Msg_RexxDial;

	Msg_RexxDial.gd_Cmd = RCMDESCAPE;
	AnalyseGlobalMsg(&Msg_RexxDial, TRUE);
}

/****************************************************
*Func : Set the Escape key listen ON/OFF
*in   : -
*out  : -
*/
void SetEscape( ID, SIR, Error, Result, ErrLevel)
IDENTIFIER *ID;
struct ScriptInfoRecord *SIR;
char **Error;
char **Result;
int  *ErrLevel;
{
  GEDIALOGUE *GD;

	if(!stricmp(ID[0].id_Name,"ON"))
	{	
		if( (GD = GetFreeGEDial()) != NULL)
		{
			GD->gd_Cmd = CBB_SETCLR|CBB_ESCAPE;
			PutMsg(Port_PCtoGE,(struct Message *)GD);
		}
	}
	else
		if(!stricmp(ID[0].id_Name,"OFF"))
		{	
			if( (GD = GetFreeGEDial()) != NULL)
			{
				GD->gd_Cmd = CBB_ESCAPE;
				PutMsg(Port_PCtoGE,(struct Message *)GD);
			}
		}
		else
		{
			*Error = RexxError[2].re_Name;
			*ErrLevel = RexxError[2].re_ReturnCode;
		}
}

/****************************************************
*Func : Set the Cursor keys listen ON/OFF
*in   : -
*out  : -
*/
void SetCursor( ID, SIR, Error, Result, ErrLevel)
IDENTIFIER *ID;
struct ScriptInfoRecord *SIR;
char **Error;
char **Result;
int  *ErrLevel;
{
  GEDIALOGUE *GD;

	if(!stricmp(ID[0].id_Name,"ON"))
	{	
		if( (GD = GetFreeGEDial()) != NULL)
		{
			GD->gd_Cmd = CBB_SETCLR|CBB_CURSOR;
			PutMsg(Port_PCtoGE,(struct Message *)GD);
		}
	}
	else
		if(!stricmp(ID[0].id_Name,"OFF"))
		{	
			if( (GD = GetFreeGEDial()) != NULL)
			{
				GD->gd_Cmd = CBB_CURSOR;
				PutMsg(Port_PCtoGE,(struct Message *)GD);
			}			
		}
		else
		{
			*Error = RexxError[2].re_Name;
			*ErrLevel = RexxError[2].re_ReturnCode;
		}
}

/****************************************************
*Func : Set the Mouse buttons listen ON/OFF
*in   : -
*out  : -
*/
void SetMouse( ID, SIR, Error, Result, ErrLevel)
IDENTIFIER *ID;
struct ScriptInfoRecord *SIR;
char **Error;
char **Result;
int  *ErrLevel;
{
  GEDIALOGUE *GD;

	if(!stricmp(ID[0].id_Name,"ON"))
	{	
		if((GD = GetFreeGEDial()) != NULL)
		{
			GD->gd_Cmd = CBB_SETCLR|CBB_MOUSE;
			PutMsg(Port_PCtoGE,(struct Message *)GD);
		}
	}
	else
		if(!stricmp(ID[0].id_Name,"OFF"))
		{	
			if((GD = GetFreeGEDial()) != NULL)
			{
				GD->gd_Cmd = CBB_MOUSE;
				PutMsg(Port_PCtoGE,(struct Message *)GD);
			}
		}
		else
		{
			*Error = RexxError[2].re_Name;
			*ErrLevel = RexxError[2].re_ReturnCode;
		}
}

/****************************************************
*Func : return the current active pagenumber 
*in   : -
*out  : -
*/
void GetPageNr( ID, SIR, Error, Result, ErrLevel)
IDENTIFIER *ID;
struct ScriptInfoRecord *SIR;
char **Error;
char **Result;
int  *ErrLevel;
{
	if(SNR_TrackPlay)
		sprintf(ReturnCode,"%d",SNR_TrackPlay->PageNr);
	else
		strcpy(ReturnCode,"0");

	*Result = ReturnCode;
}

/****************************************************
*Func : return the number of pages in the script
*in   : -
*out  : -
*/
void GetNrPages( ID, SIR, Error, Result, ErrLevel)
IDENTIFIER *ID;
struct ScriptInfoRecord *SIR;
char **Error;
char **Result;
int  *ErrLevel;
{
	sprintf(ReturnCode,"%d",PageCntr);

	*Result = ReturnCode;
}

/****************************************************
*Func : return the current State of the process controller
*in   : -
*out  : -
*/
void GetState( ID, SIR, Error, Result, ErrLevel)
IDENTIFIER *ID;
struct ScriptInfoRecord *SIR;
char **Error;
char **Result;
int  *ErrLevel;
{
	switch(RunState)
	{
		case RS_INIT:
				*Result = "INIT";
				break;	
		case RS_CONTROL:
				*Result = "CONTROL";
				break;	
		case RS_REMOVE:
				*Result = "REMOVE";
				break;	
	}
}

/****************************************************
*Func : Return the value of a variable
*in   : -
*out  : -
*/
void GetVAR( ID, SIR, Error, Result, ErrLevel)
IDENTIFIER *ID;
struct ScriptInfoRecord *SIR;
char **Error;
char **Result;
int  *ErrLevel;
{
  VIR *CurVIR;

	if ((CurVIR = (VIR *)FindArexxVIR(VIList,ID[0].id_Name)))
	{
		switch(CurVIR->vir_Type)
		{
			case VCT_STRING:
					strncpy(ReturnCode,CurVIR->vir_String,99);
					break;
			case VCT_INTEGER:
					sprintf(ReturnCode,"%d",CurVIR->vir_Integer);
					break;
		}	
	}
	else
	{
		ReturnCode[0] = '\0';
		*ErrLevel = RC_WARN;
	}

	*Result = ReturnCode;
}

/****************************************************
*Func : Set the value of a variable
*in   : -
*out  : -
*/
void SetVAR( ID, SIR, Error, Result, ErrLevel)
IDENTIFIER *ID;
struct ScriptInfoRecord *SIR;
char **Error;
char **Result;
int  *ErrLevel;
{
  VIR *CurVIR;

	if ((CurVIR = (VIR *)FindArexxVIR(VIList,ID[0].id_Name)))
	{
		switch(CurVIR->vir_Type)
		{
			case VCT_STRING:
					strcpy(CurVIR->vir_String,ID[1].id_Name);
					break;
			case VCT_INTEGER:
					if ( isdigit( ID[1].id_Name[0] ) )
						CurVIR->vir_Integer = atoi(ID[1].id_Name);
					else
					{
						strcpy(CurVIR->vir_String,ID[1].id_Name);
						CurVIR->vir_Type = VCT_STRING;
					}
					break;
		}	
	}
	else
	{
		ReturnCode[0] = '\0';
		*ErrLevel = RC_WARN;
	}

	*Result = ReturnCode;
}

/****************************************************
*Func : 
*in   : -
*out  : -
*/
void GetNewScript( ID, SIR, Error, Result, ErrLevel)
IDENTIFIER *ID;
struct ScriptInfoRecord *SIR;
char **Error;
char **Result;
int  *ErrLevel;
{
	strcpy(newScript, ID[0].id_Name);
}

/******** FindArexxVIR() ********/

STATIC VIR *FindArexxVIR(struct List *list, STRPTR varName)
{
VIR *this_vir;

	for(this_vir = (VIR *)list->lh_Head; 
			(VIR *)this_vir->vir_Node.ln_Succ;	
			this_vir = (VIR *)this_vir->vir_Node.ln_Succ)
		if ( !stricmp(varName, this_vir->vir_Name) )
			return( this_vir );

	return( NULL );
}
