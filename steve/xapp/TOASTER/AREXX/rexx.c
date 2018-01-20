#include <workbench/startup.h>
#include <exec/types.h>
#include <rexx/storage.h>
#include <rexx/rxslib.h>
#include <rexx/errors.h>
#include <libraries/dosextens.h>

#include "nb:pre.h"
#include "ph:rexx_pragma.h"
#include "ph:rexx_proto.h"
#include "ph:rexx.h"

/**** function declarations ****/

int GetARexxMsg(AREXXCONTEXT *RexxContext, ULONG *err, STRPTR arg);
void ReplyARexxMsg(AREXXCONTEXT *, struct RexxMsg *, char *, LONG);
struct RexxMsg *SendARexxMsg(AREXXCONTEXT *, char *, BOOL);
void FreeARexx(AREXXCONTEXT *);
AREXXCONTEXT *InitARexx(char *);
AREXXCONTEXT *PerformRexxCmd(char *RexxCmd, LONG *RC);
BOOL ProcessArexxError(LONG RC, LONG errorCode, STRPTR errorStr);

/**** functions ****/

#if 0

void main(void)
{
ULONG rexxSignal, SigRecvd;
AREXXCONTEXT *RexxContext;
LONG rexxError, RC;
TEXT Arg[256], errorStr[100];

	// This is a small test which accesses BaudBandit and sets the
	// baudrate to 2400.

	RexxContext = PerformRexxCmd("ADDRESS BAUD baud 2400");
	if ( RexxContext )
	{
		rexxSignal = 1<<RexxContext->ARexxPort->mp_SigBit;

		SigRecvd = Wait( rexxSignal );

		if( SigRecvd & rexxSignal )
		{
			printf("Received Arexx reply\n");

			RC = GetARexxMsg(RexxContext, &rexxError, Arg);

			if ( !ProcessArexxError(RC, rexxError, errorStr) )
				printf("[%s]\n",errorStr);
			else
				printf("No error\n");

			FreeARexx(RexxContext);
		}
	}
}

#endif

/*********************************************************
*Func : This function returns a structure that contains the
*				commands sent from ARexx. It will be parsed and
*				returned so it can be freed later
*in   : RexxContext -> Ptr to context struct
*out  : RC
*/	

int GetARexxMsg(AREXXCONTEXT *RexxContext, ULONG *err, STRPTR arg)
{
struct RexxMsg *Msg_RArexx;
int RC;

	RC = RC_OK;

	if(RexxContext == NULL)
		return(NULL);

	while( (Msg_RArexx = (struct RexxMsg *)GetMsg(RexxContext->ARexxPort)) != NULL)
 	{
		if(RC == RC_OK)
		{	
	    if(Msg_RArexx->rm_Node.mn_Node.ln_Type == NT_REPLYMSG)
	    {
				*err = Msg_RArexx->rm_Result2;
				stccpy(arg,Msg_RArexx->rm_Args[0],255);
				RC = Msg_RArexx->rm_Result1;

{
int i;
for(i=0; i<16; i++)
if (Msg_RArexx->rm_Args[i]) printf("%d [%s]\n",i,Msg_RArexx->rm_Args[i]);
}

				DeleteArgstring((char *)Msg_RArexx->rm_Args[0]);
				DeleteRexxMsg(Msg_RArexx);
				// We have 1 message less
				RexxContext->Outstanding -= 1;
	    }
			else
			{
				ReplyARexxMsg(RexxContext, Msg_RArexx, NULL, 100);
			}
		}
	}
	return(RC);
}

/*********************************************************
*Func : Return an AREXX message to the inquirer
*in   : RexxContext
*				RString -> string with the answer
*				Error -> if something is wrong
*out  : as inputs
*/

void ReplyARexxMsg(	AREXXCONTEXT *RexxContext, struct RexxMsg *Msg_Arexx,
										char *RString, LONG Error )
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
*Func : Send a string into a world called ... AREXX
*in   : StringFile -> if TRUE then the STRINGFILE-bit
*				for that message will be set. (the caller will
*				then know that data comes in as a string) 
*out  : NULL->Error
*				else ptr to rexxmsg just sent
*/

struct RexxMsg *SendARexxMsg(	AREXXCONTEXT *RexxContext,
															char *RString, BOOL StringFile )
{
struct MsgPort *Port_PCtoRX;
struct RexxMsg *Msg_Arexx;

	if(RexxContext && RString)
	{
		if(Msg_Arexx = CreateRexxMsg(RexxContext->ARexxPort, RexxContext->Extension, RexxContext->PortName))
		{
    	Msg_Arexx->rm_Action = RXCOMM | (StringFile ? RXFF_STRING : 0) | RXFF_RESULT;

    	if(Msg_Arexx->rm_Args[0] = (void *)CreateArgstring(RString, (LONG)strlen(RString)))
    	{
				Forbid();
				if(Port_PCtoRX = (struct MsgPort *)FindPort((char *)RXSDIR))
				{
	    		PutMsg(Port_PCtoRX, (struct Message *)Msg_Arexx);
					RexxContext->Outstanding += 1;
				}
				else
				{
	    		DeleteArgstring((char *)Msg_Arexx->rm_Args[0]);
	    		DeleteRexxMsg(Msg_Arexx);
					Permit();
					return(NULL);
				}
				Permit();
	   	}
	   	else
			{
				DeleteRexxMsg(Msg_Arexx);
				return(NULL);
			}
		}
	}
	return(Msg_Arexx);
} 

/*********************************************************
*Func : Shut down the ARExx context  
*in   : RexxContext
*out  : -
*/

void FreeARexx(AREXXCONTEXT *RexxContext)
{
LONG rexxError;
TEXT Arg[256];

	if(RexxContext)
	{
		RexxContext->PortName[0] = '\0';
		while(RexxContext->Outstanding)
		{
			WaitPort(RexxContext->ARexxPort);
	    GetARexxMsg(RexxContext,&rexxError,Arg);
		}
	
		// clean up the port and delete it
		if(RexxContext->ARexxPort)
    	DeletePort(RexxContext->ARexxPort);	

		// lets close the library
		if(RexxContext->RexxSysBase)
    	CloseLibrary(RexxContext->RexxSysBase);

		FreeMem(RexxContext, sizeof(AREXXCONTEXT));
	}
}

/*********************************************************
*Func : Init an AREXX port for processing
*in   : -
*out  : ptr to the Context structure
*/

AREXXCONTEXT *InitARexx(char *RexxName)
{
register AREXXCONTEXT *RexxContext = NULL;
int i;

	if(RexxContext = (AREXXCONTEXT *)AllocMem(sizeof(AREXXCONTEXT), MEMF_PUBLIC|MEMF_CLEAR|MEMF_FAST))
	{
		if( (RexxContext->RexxSysBase = (struct Library *)OpenLibrary("rexxsyslib.library",NULL)) != NULL)
		{
			strcpy(RexxContext->Extension,"rexx");

	    // set up the last error RVI name
	    strcpy(RexxContext->ErrorName,RexxName);
	    strcat(RexxContext->ErrorName,".LASTERROR");
    	// the portname has to be unique

			Forbid();
			for(i=1, RexxContext->ARexxPort=(VOID *)1; RexxContext->ARexxPort; i++)
			{
				sprintf(RexxContext->PortName,"%s%d",RexxName,i);
				RexxContext->ARexxPort = FindPort(RexxContext->PortName);
			}
			Permit();

	    RexxContext->ARexxPort = (struct MsgPort *)CreatePort(RexxContext->PortName, NULL);
		}

		if (	(!(RexxContext->RexxSysBase)) || (!(RexxContext->ARexxPort)) )
		{
	   	FreeARexx(RexxContext);
  	 	RexxContext = NULL;
		}
	}
	return(RexxContext);
}  

/*************************************************
*Func : Take a script or a cmd line and send it
*				to the Arexx handler
*in   : RexxCmd -> ptr to script
*out  : NULL-> error
*				else ptr to RexxContext structure
*/

AREXXCONTEXT *PerformRexxCmd(char *RexxCmd, LONG *RC)
{
AREXXCONTEXT *RexxContext;
struct RexxMsg *Msg_RArexx = NULL;

	*RC=0L;

	if( (RexxContext = (AREXXCONTEXT *)InitARexx("REXX_XAPP")) != NULL)
    Msg_RArexx = SendARexxMsg(RexxContext,RexxCmd,1);

	return(RexxContext);
}

/******** ProcessArexxError() ********/

BOOL ProcessArexxError(LONG RC, LONG errorCode, STRPTR errorStr)
{
	if ( RC == RC_OK )	
		return(TRUE);

	if ( RC == RC_WARN )
		strcpy(errorStr, "Warning - ");
	else if ( RC == RC_ERROR )
		strcpy(errorStr, "Error - ");
	else if ( RC == RC_FATAL )
		strcpy(errorStr, "Fatal error - ");

	switch( errorCode )
	{
		case ERR10_001: strcat(errorStr, "Program not found."); break;
		case ERR10_002: strcat(errorStr, "Execution halted."); break;
		case ERR10_003: strcat(errorStr, "No memory available."); break;
		case ERR10_004: strcat(errorStr, "Invalid character in program."); break;
		case ERR10_005: strcat(errorStr, "Unmatched quote."); break;
		case ERR10_006: strcat(errorStr, "Unterminated comment."); break;
		case ERR10_007: strcat(errorStr, "Clause too long."); break;
		case ERR10_008: strcat(errorStr, "Unrecognized token."); break;
		case ERR10_009: strcat(errorStr, "Symbol or string too long."); break;
		
		case ERR10_010: strcat(errorStr, "Invalid message packet."); break;
		case ERR10_011: strcat(errorStr, "Command string error."); break;
		case ERR10_012: strcat(errorStr, "Error return from function."); break;
		case ERR10_013: strcat(errorStr, "Host environment not found."); break;
		case ERR10_014: strcat(errorStr, "Required library not found."); break;
		case ERR10_015: strcat(errorStr, "Function not found."); break;
		case ERR10_016: strcat(errorStr, "No return value."); break;
		case ERR10_017: strcat(errorStr, "Wrong number of arguments."); break;
		case ERR10_018: strcat(errorStr, "Invalid argument to function."); break;
		case ERR10_019: strcat(errorStr, "Invalid PROCEDURE."); break;
		
		case ERR10_020: strcat(errorStr, "Unexpected THEN/ELSE."); break;
		case ERR10_021: strcat(errorStr, "Unexpected WHEN/OTHERWISE."); break;
		case ERR10_022: strcat(errorStr, "Unexpected LEAVE or ITERATE."); break;
		case ERR10_023: strcat(errorStr, "Invalid statement in SELECT."); break;
		case ERR10_024: strcat(errorStr, "Missing THEN clauses."); break;
		case ERR10_025: strcat(errorStr, "Missing OTHERWISE."); break;
		case ERR10_026: strcat(errorStr, "Missing or unexpected END."); break;
		case ERR10_027: strcat(errorStr, "Symbol mismatch on END."); break;
		case ERR10_028: strcat(errorStr, "Invalid DO syntax."); break;
		case ERR10_029: strcat(errorStr, "Incomplete DO/IF/SELECT."); break;
		
		case ERR10_030: strcat(errorStr, "Label not found."); break;
		case ERR10_031: strcat(errorStr, "Symbol expected."); break;
		case ERR10_032: strcat(errorStr, "String or symbol expected."); break;
		case ERR10_033: strcat(errorStr, "Invalid sub-keyword."); break;
		case ERR10_034: strcat(errorStr, "Required keyword missing."); break;
		case ERR10_035: strcat(errorStr, "Extraneous characters."); break;
		case ERR10_036: strcat(errorStr, "Sub-keyword conflict."); break;
		case ERR10_037: strcat(errorStr, "Invalid template."); break;
		case ERR10_038: strcat(errorStr, "Invalid TRACE request."); break;
		case ERR10_039: strcat(errorStr, "Uninitialized variable."); break;
		
		case ERR10_040: strcat(errorStr, "Invalid variable name."); break;
		case ERR10_041: strcat(errorStr, "Invalid expression."); break;
		case ERR10_042: strcat(errorStr, "Unbalanced parentheses."); break;
		case ERR10_043: strcat(errorStr, "Nesting level exceeded."); break;
		case ERR10_044: strcat(errorStr, "Invalid expression result."); break;
		case ERR10_045: strcat(errorStr, "Expression required."); break;
		case ERR10_046: strcat(errorStr, "Boolean value not 0 or 1."); break;
		case ERR10_047: strcat(errorStr, "Arithmetic conversion error."); break;
		case ERR10_048: strcat(errorStr, "Invalid operand."); break;

		default: strcat(errorStr, "undefined error."); break;
	}

	return(FALSE);
}
		
/******** E O F ********/
