/************************************************************************
*File : Rexx.c 
*Desc : Arexx implentation
*/

#include <workbench/startup.h>
#include <exec/types.h>
#include <rexx/storage.h>
#include <rexx/rxslib.h>
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

IDENTIFIER Identifier[MAXIDENTIFIERS];

AREXXCONTEXT *PublicRexxContext;
struct RexxMsg *PublicMsg_Arexx;

/********************************************************
*Func : parse a command line
*in   : CL -> ptr to null terminated cmdline
*		ID -> Ptr to Identifier[n] arry
*out  : -
*/		

void ParseCmdLine( CL, ID)
char *CL;
IDENTIFIER *ID;
{
  int i;
  char *Next;
	
	Next = CL;
	for(i = 0; i < MAXIDENTIFIERS; i++)
	{
		Next = stptok(Next,ID[i].id_Name,IDENTIFIERLENGTH," ,");

		if(*Next)
			Next++;
	}
}

#if 0
void ParseCmdLine(char *CL, IDENTIFIER *ID)
{
int numChars, argNum, len, numSeen;
char *strPtr;

	strPtr = CL;
	len = strlen(strPtr);
	argNum = 0;
	numSeen =0;

	while(1)
	{
		if (argNum>=MAXIDENTIFIERS)
			break;
		if (*strPtr==0)
			break;
		numChars = stcarg(strPtr, " 	,");	// space, tab, comma
		if (numChars>=1)
		{
			stccpy(ID[argNum].id_Name, strPtr, numChars+1);
{ char strstr[100]; sprintf(strstr,"argNum=%d [%s]\n",argNum,ID[argNum].id_Name); KPrintF(strstr); }
			argNum++;
		}
		strPtr += numChars+1;
		numSeen += numChars+1;
		if (numSeen>len)
			break;		
	}
}
#endif

/*********************************************************
*Func : This function returns a structure that contains the
*		commands sent from ARexx. It will be parsed and
*		returned so it can be freed later
*in   : RexxContext -> Ptr to context struct
*out  : NULL -> no message
*/	
struct RexxMsg *GetARexxMsg( RexxContext)
AREXXCONTEXT *RexxContext;
{
  struct RexxMsg *Msg_RArexx = NULL;
  register short flag;

    if(RexxContext)
	{
		if(Msg_RArexx = (struct RexxMsg *)GetMsg(RexxContext->ARexxPort))
    	{
		    if(Msg_RArexx->rm_Node.mn_Node.ln_Type == NT_REPLYMSG)
		    {
				flag = FALSE;
				if(Msg_RArexx->rm_Result1)
		    		flag = TRUE;

				DeleteArgstring((char *)Msg_RArexx->rm_Args[0]);
				DeleteRexxMsg(Msg_RArexx);
				RexxContext->Outstanding -= 1;

				Msg_RArexx = (flag ? REXX_RETURN_ERROR : 0);
		    }
    	}
	}
    return(Msg_RArexx);
}

/*********************************************************
*Func : Return an AREXX message to the inquirer
*in   : RexxContext
*		RString -> string with the answer
*		Error -> if something is wrong
*out  : as inputs
*/
void ReplyARexxMsg( RexxContext, Msg_Arexx, RString, Error)
AREXXCONTEXT *RexxContext;
struct RexxMsg *Msg_Arexx;
char *RString;
LONG Error;
{
    if(RexxContext && Msg_Arexx && (Msg_Arexx != REXX_RETURN_ERROR))
    {
		Msg_Arexx->rm_Result2 = 0;
		if(!(Msg_Arexx->rm_Result1 = Error))
		{
	    	if(Msg_Arexx->rm_Action & (1<< RXFB_RESULT))
				if(RString)
				    Msg_Arexx->rm_Result2 = (long)CreateArgstring(RString, (LONG)strlen(RString));
		}
		ReplyMsg((struct Message *)Msg_Arexx);
    }
}

/*********************************************************
*func : Set an error string for the ARExx application 
*		in the variable defined as <appname>.LASTERROR
*in   : the usual stuff
*out  : TRUE -> all ok
*		FALSE -> some sort of error occured
*/
BOOL SetARexxLastError( RexxContext, rmsg, ErrorString)
AREXXCONTEXT *RexxContext;
struct RexxMsg *rmsg;
char *ErrorString;
{
  BOOL OKFlag = FALSE;

    if(RexxContext && rmsg && ErrorString && CheckRexxMsg((struct Message *)rmsg))
    {
		if(!SetRexxVar((struct Message *)rmsg, RexxContext->ErrorName, ErrorString, (long)strlen(ErrorString)))
	    	OKFlag = TRUE;
    }
    return(OKFlag);
}

/*********************************************************
*Func : Send a string into a world called ... AREXX
*in   : StringFile -> if TRUE then the STRINGFILE-bit
*		for that message will be set. (the caller will
*		then know that data comes in as a string) 
*out  : TRUE -> Message sent
*		FALSE -> none sent
*/
BOOL SendARexxMsg( RexxContext, RString, StringFile)
AREXXCONTEXT *RexxContext;
char *RString;
BOOL StringFile;
{
  struct MsgPort *Port_PCtoRX;
  struct RexxMsg *Msg_Arexx;
  BOOL Flag = FALSE;

    if(RexxContext && RString)
    {
		if(Msg_Arexx = CreateRexxMsg(RexxContext->ARexxPort, RexxContext->Extension, RexxContext->PortName))
		{
	    	Msg_Arexx->rm_Action = RXCOMM | (StringFile ? (1 << RXFB_STRING) : 0);

	    	if(Msg_Arexx->rm_Args[0] = (void *)CreateArgstring(RString, (LONG)strlen(RString)))
	    	{
				Forbid();
				if(Port_PCtoRX = (struct MsgPort *)FindPort((char *)RXSDIR))
				{
		    		PutMsg(Port_PCtoRX, (struct Message *)Msg_Arexx);
					RexxContext->Outstanding += 1;
		    		Flag = TRUE;
				}
				else
				{
		    		DeleteArgstring((char *)Msg_Arexx->rm_Args[0]);
		    		DeleteRexxMsg(Msg_Arexx);
				}
				Permit();
	    	}
	    	else
				DeleteRexxMsg(Msg_Arexx);
		}
    }
    return(Flag);
} 

/*********************************************************
*Func : Shut down the ARExx context  
*in   : RexxContext
*out  : -
*/
void FreeARexx( RexxContext)
AREXXCONTEXT *RexxContext;
{
  register struct RexxMsg *Msg_Arexx;

	if(PublicRexxContext != NULL)
	{
		SetARexxLastError(PublicRexxContext, PublicMsg_Arexx,RexxError[5].re_Name);
		ReplyARexxMsg(PublicRexxContext,PublicMsg_Arexx, NULL, RexxError[5].re_ReturnCode);
	}

    if(RexxContext)
    {
		RexxContext->PortName[0] = '\0';
		while(RexxContext->Outstanding)
		{	
		    WaitPort(RexxContext->ARexxPort);
	    	while(Msg_Arexx = (struct RexxMsg *)GetARexxMsg(RexxContext))
	    	{
				if(Msg_Arexx != REXX_RETURN_ERROR)
				{
				    SetARexxLastError(RexxContext, Msg_Arexx, RexxError[5].re_Name);
				    ReplyARexxMsg(RexxContext, Msg_Arexx, NULL, RexxError[5].re_ReturnCode);
				}
	    	}	
		}
	
		// clean up the port and delete it
		if(RexxContext->ARexxPort)
		{
	    	while(Msg_Arexx = GetARexxMsg(RexxContext))
  	    	{
				SetARexxLastError(RexxContext, Msg_Arexx, "99: Port Closed");
				ReplyARexxMsg(RexxContext, Msg_Arexx, NULL, 100);
	    	}
	    	DeletePort(RexxContext->ARexxPort);	
		}

		// lets close the library
		if(RexxContext->RexxSysBase)
	    	CloseLibrary(RexxContext->RexxSysBase);

		MLMMU_FreeMem(RexxContext);
    }
}

/*********************************************************
*Func : Init an AREXX port for processing
*in   : -
*out  : ptr to the Context structure
*/
AREXXCONTEXT *InitARexx( void)
{
  register AREXXCONTEXT *RexxContext = NULL;

	PublicRexxContext = NULL;
	PublicMsg_Arexx = NULL;

    if(RexxContext = (AREXXCONTEXT *)MLMMU_AllocMem(sizeof(AREXXCONTEXT), MEMF_PUBLIC|MEMF_CLEAR|MEMF_FAST,NULL))
    {
		if( (RexxContext->RexxSysBase = (struct Library *)OpenLibrary("rexxsyslib.library",NULL)) != NULL)
		{
			strcpy(RexxContext->Extension,"rexx");

		    // set up the last error RVI name 
		    strcpy(RexxContext->ErrorName,"MEDIAPOINT.LASTERROR"); // "MEDIALINK.LASTERROR");
	    	// the portname has to be unique

			strcpy(RexxContext->PortName,"MEDIAPOINT");	// "MEDIALINK");
		    RexxContext->ARexxPort = (struct MsgPort *)CreatePort((char *)RexxContext->PortName, NULL);
		}

		if(
	    	(!(RexxContext->RexxSysBase)) ||	
	    	(!(RexxContext->ARexxPort))
		  )
		{
	    	FreeARexx(RexxContext);
	    	RexxContext = NULL;
		}
    }
    return(RexxContext);
}  

/******************************************************
*Func : Arexx cmdline interpreter
*in   : RexxContext -> Ptr to context structure
*		SIR
*out  : -
*/
void ArexxTalks( RexxContext, SIR)
AREXXCONTEXT *RexxContext;
struct ScriptInfoRecord *SIR;
{
  struct RexxMsg *Msg_Arexx;
  char *Error, *Result;  
  int ErrLevel,i;

	while( (Msg_Arexx = GetARexxMsg(RexxContext)) != NULL)
	{
		Error = NULL;
		Result = NULL;
		ErrLevel = 0;

		ParseCmdLine(Msg_Arexx->rm_Args[0],Identifier);

		for(i = 0; RexxCmds[i].rc_Func != NULL; i++)
			if(!stricmp(RexxCmds[i].rc_Name,Identifier[0].id_Name))
			{
				(*(RexxCmds[i].rc_Func))(&Identifier[1],SIR, &Error, &Result, &ErrLevel);
				break;
			}

		if(RexxCmds[i].rc_Func == NULL)
		{
			Error = RexxError[0].re_Name;
			ErrLevel = RexxError[0].re_ReturnCode;
		}

		if(Error)
			SetARexxLastError(RexxContext, Msg_Arexx,Error);

		// When a Errlevel of -1 is returned then the function doesn't want
		// us the reply now but later.
		// Usually the reply is given when the processcontroller has obtained
		// the requested information from one of its children, which may
		// be later in time.
		if(ErrLevel != -1)
			ReplyARexxMsg(RexxContext,Msg_Arexx, Result, ErrLevel);
		else
		{
			// Make the context and the msg public for later use
			if(PublicRexxContext == NULL)
			{
				PublicRexxContext = RexxContext;
				PublicMsg_Arexx = Msg_Arexx;
			}
			else
			{
				// we can't reply put this command in the global wait queue
				Error = RexxError[4].re_Name;
				ErrLevel = RexxError[4].re_ReturnCode;
				SetARexxLastError(RexxContext, Msg_Arexx,Error);
				ReplyARexxMsg(RexxContext,Msg_Arexx, Result, ErrLevel);
			}
		}
	}
}

