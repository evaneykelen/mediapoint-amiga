/************************************************************************
*File : Rexx.c 
*Desc : Arexx implentation
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
#include "ph:rexx_pragma.h"
#include "ph:rexx_proto.h"
#include "ph:rexx.h"
#include "external.h"

#define _PRINTF FALSE

/**************************************************
*Func : Build a string with a Rexxcmd or Rexxcmd list
*in   : SizeOfRexxCmd -> Size of list
*		ThisPI 
*		FileName -> Name of file (if cmd is a rexx file)
*out  : Ptr to RexxCmd area
*/
char *MakeRexxCmd(SizeOfRexxCmd,ThisPI,FileName)
int SizeOfRexxCmd;
PROCESSINFO *ThisPI;
char *FileName;
{
  char *RexxCmd;
  struct FileHandle *RexxFileHandle;

	if( (RexxCmd = (char *)AllocMem(SizeOfRexxCmd, MEMF_PUBLIC|MEMF_CLEAR)) != NULL)
	{
		switch(ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[1])
		{	
			case ARGUMENT_COMMAND:
						strcpy(RexxCmd,"ADDRESS ");
						if(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[50] != '"')
							strcat(RexxCmd,"\"");
						strcat(RexxCmd,&ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[50]);
						if(ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[strlen(&ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[50])-1] != '"')
							strcat(RexxCmd,"\"");
						strcat(RexxCmd," ");
						strcat(RexxCmd,ThisPI->pi_Arguments.ar_Worker.aw_Path);
						break;
			case ARGUMENT_SCRIPT:
					    if(!( 
							((RexxFileHandle = (struct FileHandle *)Open(FileName,MODE_OLDFILE)) != NULL) &&
    						(Read((BPTR)RexxFileHandle,RexxCmd,SizeOfRexxCmd) != -1)
						  ))
							{	
								if(RexxFileHandle != NULL)
									Close((BPTR)RexxFileHandle);
								FreeMem(RexxCmd,SizeOfRexxCmd);
								return(NULL);
							}
						Close((BPTR)RexxFileHandle);
						break;
		}
	}
	return(RexxCmd);
}

/*********************************************************
*Func : This function returns the port name of the arexxport
*in   : RexxContext -> ptr to context structure
*out  : NULL -> No Arexx port available
*/
char *ARexxName( RexxContext)
AREXXCONTEXT *RexxContext;
{
  register char *tmp = NULL;

    if(RexxContext)
	tmp = RexxContext->PortName;

    return(tmp);
}

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

/*********************************************************
*Func : This function returns a structure that contains the
*		commands sent from ARexx. It will be parsed and
*		returned so it can be freed later
*in   : RexxContext -> Ptr to context struct
*out  : RC
*/	
int GetARexxMsg( RexxContext)
AREXXCONTEXT *RexxContext;
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
#if _PRINTF
				// This is the reply
				printf("Reply: RC =     %d\n",Msg_RArexx->rm_Result1);
				printf("     : Err =    %d\n",Msg_RArexx->rm_Result2);
				printf("     : Arg[0] = %s\n",Msg_RArexx->rm_Args[0]);		
#endif
				RC = Msg_RArexx->rm_Result1;

				DeleteArgstring((char *)Msg_RArexx->rm_Args[0]);
				DeleteRexxMsg(Msg_RArexx);

				// We have 1 message less
				RexxContext->Outstanding -= 1;
		    }
			else
				ReplyARexxMsg(RexxContext, Msg_RArexx, NULL, 100);
		}
   	}
    return(RC);
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
*Func : Send a string into a world called ... AREXX
*in   : StringFile -> if TRUE then the STRINGFILE-bit
*		for that message will be set. (the caller will
*		then know that data comes in as a string) 
*out  : NULL->Error
*		else ptr to rexxmsg just sent
*/
struct RexxMsg *SendARexxMsg( RexxContext, RString, StringFile)
AREXXCONTEXT *RexxContext;
char *RString;
BOOL StringFile;
{
  struct MsgPort *Port_PCtoRX;
  struct RexxMsg *Msg_Arexx;

    if(RexxContext && RString)
    {
		if(Msg_Arexx = CreateRexxMsg(RexxContext->ARexxPort, RexxContext->Extension, RexxContext->PortName))
		{
	    	Msg_Arexx->rm_Action = RXCOMM | (StringFile ? RXFF_STRING : 0);

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
void FreeARexx( RexxContext)
AREXXCONTEXT *RexxContext;
{
//  register struct RexxMsg *Msg_Arexx;

    if(RexxContext)
    {
		RexxContext->PortName[0] = '\0';
		while(RexxContext->Outstanding)
		{	
		    WaitPort(RexxContext->ARexxPort);
	    	GetARexxMsg(RexxContext);
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
AREXXCONTEXT *InitARexx( RexxName)
char *RexxName;
{
  register AREXXCONTEXT *RexxContext = NULL;

    if(RexxContext = (AREXXCONTEXT *)AllocMem(sizeof(AREXXCONTEXT), MEMF_PUBLIC|MEMF_CLEAR|MEMF_FAST))
    {
		if( (RexxContext->RexxSysBase = (struct Library *)OpenLibrary("rexxsyslib.library",NULL)) != NULL)
		{
			strcpy(RexxContext->Extension,"rexx");

		    // set up the last error RVI name
		    strcpy(RexxContext->ErrorName,RexxName);
		    strcat(RexxContext->ErrorName,".LASTERROR");
	    	// the portname has to be unique

			strcpy(RexxContext->PortName,RexxName);
		    RexxContext->ARexxPort = (struct MsgPort *)CreatePort(0, NULL);
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

/*************************************************
*Func : Take a script or a cmd line and send it
*		to the Arexx handler
*in   : RexxCmd -> ptr to script
*out  : NULL-> error
*		else ptr to RexxContext structure
*/
AREXXCONTEXT *PerformRexxCmd( RexxCmd)
char *RexxCmd;
{
  AREXXCONTEXT 		*RexxContext;

	if( (RexxContext = (AREXXCONTEXT *)InitARexx("REXX_XAPP")) != NULL)
	    SendARexxMsg(RexxContext,RexxCmd,1);

	return(RexxContext);
}

