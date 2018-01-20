#include "mllib_includes.h"
#include "rexx.h"

/*******************************************************************/
/*
 *   PUBLIC FUNCTIONS
 *
 *******************************************************************/

BOOL __saveds __asm LIBUA_IssueRexxCmd_V1(register __a0 STRPTR appName,
																					register __a1 STRPTR command,
																					register __a2 STRPTR result,
																					register __d0 BOOL resultWanted,
																					register __d1 int max )
{
AREXXCONTEXT *RexxStuff;
struct FileHandle *FH;
BOOL retval = FALSE;
int read;

	if ( resultWanted )
		FH = (struct FileHandle *)Open("T:RC",MODE_NEWFILE);
	else
		FH = NULL;

	RexxStuff=InitARexx(appName);

	if (RexxStuff)
	{
		retval = SendARexxMsg_V1(RexxStuff,command,FH);
		FreeARexx_V1(RexxStuff);
		if ( resultWanted )
		{
			Seek((BPTR)FH,0,OFFSET_BEGINNING);
			read = Read((BPTR)FH,result,max);
			result[read]='\0';			
		}
	}

	if ( FH )
		Close((BPTR)FH);

	return(retval);
}

/*******************************************************************/
/*
 *   PRIVATE FUNCTIONS
 *
 *******************************************************************/

struct RexxMsg *GetARexxMsg_V1(AREXXCONTEXT *RexxContext)
{
register struct RexxMsg *tmp=NULL;
register short flag;

	if (RexxContext)
	{
		if (tmp=(struct RexxMsg *)GetMsg(RexxContext->ARexxPort))
		{
			if (tmp->rm_Node.mn_Node.ln_Type==NT_REPLYMSG)
			{
        flag=FALSE;
        if (tmp->rm_Result1) flag=TRUE;
        DeleteArgstring(tmp->rm_Args[0]);
        DeleteRexxMsg(tmp);
        RexxContext->Outstanding-=1;
        tmp=flag ? REXX_RETURN_ERROR : NULL;
			}
		}
	}
  return(tmp);
}

void ReplyARexxMsg_V1(AREXXCONTEXT *RexxContext, struct RexxMsg *rmsg, char *RString, LONG Error)
{
	if ((RexxContext) && (rmsg) && (rmsg!=REXX_RETURN_ERROR))
	{
		rmsg->rm_Result2=0;
		if (!(rmsg->rm_Result1=Error))
		{
			if ((rmsg->rm_Action & (1L << RXFB_RESULT)) && (RString))
			{
				rmsg->rm_Result2=(LONG)CreateArgstring(RString,(LONG)strlen(RString));
			}
		}
		ReplyMsg((struct Message *)rmsg);
	}
}

short SendARexxMsg_V1(AREXXCONTEXT *RexxContext, char *RString, struct FileHandle *FH)
{
register struct MsgPort *RexxPort;
register struct RexxMsg *rmsg;
register short flag=FALSE;

	if ((RexxContext) && (RString))
	{
		if (rmsg=CreateRexxMsg(RexxContext->ARexxPort, RexxContext->Extension,
				RexxContext->PortName))
		{
			rmsg->rm_Action=RXCOMM | (1L << RXFB_STRING);
			if (rmsg->rm_Args[0]=CreateArgstring(RString,(LONG)strlen(RString)))
			{
				if ( FH )
				{
					rmsg->rm_Stdout = (LONG)FH;
				}
				Forbid();
				if(RexxPort=FindPort(RXSDIR))
				{
					PutMsg(RexxPort,(struct Message *)rmsg);
					RexxContext->Outstanding+=1;
					flag=TRUE;
				}
				Permit();
				if(!RexxPort)
				{
					DeleteArgstring(rmsg->rm_Args[0]);
					DeleteRexxMsg(rmsg);
				}
			}
			else
			{
				DeleteRexxMsg(rmsg);
			}
		}
	}
  return(flag);
} 

void FreeARexx_V1(AREXXCONTEXT *RexxContext)
{
register struct RexxMsg *rmsg;

	if (RexxContext)
	{
		RexxContext->PortName[0]='\0';
		while (RexxContext->Outstanding)
		{
			WaitPort(RexxContext->ARexxPort);
			while (rmsg=GetARexxMsg_V1(RexxContext))
			{
				if (rmsg!=REXX_RETURN_ERROR)
				{
					SetARexxLastError(RexxContext,rmsg,"99: Port Closed!");
					ReplyARexxMsg_V1(RexxContext,rmsg,NULL,100);
				}
			}
		}

		if (RexxContext->ARexxPort)
		{
			while (rmsg=GetARexxMsg_V1(RexxContext))
			{
				SetARexxLastError(RexxContext,rmsg,"99: Port Closed!");
				ReplyARexxMsg_V1(RexxContext,rmsg,NULL,100);
			}
			DeletePort(RexxContext->ARexxPort);
		}
		//if (RexxContext->RexxSysBase) CloseLibrary(RexxContext->RexxSysBase);
		FreeMem(RexxContext,sizeof(AREXXCONTEXT));
	}
}

short SetARexxLastError(AREXXCONTEXT *RexxContext, struct RexxMsg *rmsg, char *ErrorString)
{
register short OkFlag=FALSE;

	if (RexxContext && rmsg && CheckRexxMsg((struct Message *)rmsg))
	{
		if (!SetRexxVar((struct Message *)rmsg,RexxContext->ErrorName,ErrorString,(long)strlen(ErrorString)))
		{
			OkFlag=TRUE;
		}
	}
	return(OkFlag);
}

/******** E O F ********/
