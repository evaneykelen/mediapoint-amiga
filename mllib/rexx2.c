#include "mllib_includes.h"
#include "rexx.h"

/*******************************************************************/
/*
 *   PUBLIC FUNCTIONS
 *
 *******************************************************************/

BOOL __saveds __asm LIBUA_IssueRexxCmd_V2(register __a0 STRPTR appName,
																					register __a1 STRPTR port_name,
																					register __a2 STRPTR command,
																					register __a3 STRPTR resultStr,
																					register __a5 ULONG *RC)
{
AREXXCONTEXT *RexxContext;
BOOL result, retVal=TRUE;
ULONG flags;
struct RexxMsg *msg;

	if ( !FindPort(port_name) )
		return(FALSE);

	/**** Init vars ****/

	result = TRUE;
	flags = RXFF_RESULT | RXFF_STRING;

	/**** init ARexx ****/

	RexxContext = InitARexx(appName);
	if ( !RexxContext )
		return(FALSE);

	/**** Send command ****/

	if ( SendARexxMsg_V2(RexxContext,port_name,command,flags) )
	{
		msg = WaitForReply_V2(RexxContext);
		if (RC)
			*RC = msg->rm_Result1;
		if ( resultStr && result && msg->rm_Result1==0 && msg->rm_Result2!=NULL )
			strcpy(resultStr, (char *)msg->rm_Result2);
		FreeARexxMsg_V2(RexxContext,msg);
		retVal = TRUE;
	}
	else
		retVal = FALSE;

	/*** Free ARexx ****/

	FreeARexx_V2( RexxContext );

	return( retVal );
}

/*******************************************************************/
/*
 *   PRIVATE FUNCTIONS
 *
 *******************************************************************/

struct RexxMsg *GetARexxMsg_V2(AREXXCONTEXT *RexxContext)
{
struct RexxMsg *msg;

	if ( !RexxContext )
		return( NULL );

	msg = (struct RexxMsg *)GetMsg( RexxContext->ARexxPort );

	if ( !msg )
		return( NULL );

	if ( msg->rm_Node.mn_Node.ln_Type == NT_REPLYMSG )
		--RexxContext->Outstanding;

	return( msg );
}

BOOL ReplyARexxMsg_V2(AREXXCONTEXT *RexxContext, struct RexxMsg *msg,
											LONG return_code, char *result_string,
											LONG error_code )
{
	if ( !RexxContext || !msg )
		return( FALSE );

	if ( msg->rm_Node.mn_Node.ln_Type == NT_REPLYMSG )
		return( FALSE );

	msg->rm_Result1 = return_code;
	msg->rm_Result2 = (LONG)NULL;

	if ( result_string!=NULL && return_code==RC_OK && (msg->rm_Action & RXFF_RESULT)!=0 )
	{
		msg->rm_Result2 = (LONG)CreateArgstring(result_string,strlen(result_string));
	}
	else if ( return_code != RC_OK )
	{
		msg->rm_Result2 = error_code;
	}

	ReplyMsg((struct Message *)msg);

	return(TRUE);
}

struct RexxMsg *SendARexxMsg_V2(AREXXCONTEXT *RexxContext, char *port_name,
																char *command, ULONG flags)
{
struct MsgPort *port;
struct RexxMsg *msg;

	if ( !RexxContext || !command || !port_name )
		return( NULL );

	msg = CreateRexxMsg(RexxContext->ARexxPort, RexxContext->Extension,
											RexxContext->PortName);
	if ( !msg )
		return( NULL );

	flags &= ( RXFF_NOIO | RXFF_RESULT | RXFF_STRING | RXFF_TOKEN );

	msg->rm_Action = RXCOMM | flags;
	msg->rm_Args[0] = (STRPTR)CreateArgstring(command,strlen(command));

	msg->rm_Result1 = 0;
	msg->rm_Result2 = (LONG)NULL;

	if ( !msg->rm_Args[0] )
	{
		DeleteRexxMsg(msg);
		return( NULL );
	}

	Forbid();

	if ( (port = FindPort((UBYTE *)port_name)) != NULL )
	{
		PutMsg(port,(struct Message *)msg);
		++RexxContext->Outstanding;
	}
	else
	{
		DeleteArgstring((char *)msg->rm_Args[0]);
		DeleteRexxMsg(msg);
		msg = NULL;
	}

	Permit();

	return( msg );
}

void FreeARexx_V2(AREXXCONTEXT *RexxContext)
{
struct RexxMsg *msg;

	if ( !RexxContext )
		return;

	RexxContext->PortName[0]='\0';

	while ( RexxContext->Outstanding>0 && RexxContext->ARexxPort )
	{
		WaitPort(RexxContext->ARexxPort);
		while( (msg=GetARexxMsg_V2(RexxContext))!=NULL )
		{
			if ( !FreeARexxMsg_V2(RexxContext,msg) )
        ReplyARexxMsg_V2(RexxContext,msg,100,NULL,0);
		}
	}

	if (RexxContext->ARexxPort)
	{
		Forbid();

		while( (msg=GetARexxMsg_V2(RexxContext))!=NULL )
			ReplyARexxMsg_V2(RexxContext,msg,100,NULL,0);

		DeletePort(RexxContext->ARexxPort);

		Permit();
	}

	//CloseLibrary(RexxContext->RexxSysBase);

	FreeMem(RexxContext,sizeof(AREXXCONTEXT));
}

AREXXCONTEXT *InitARexx(char *RexxName)
{
AREXXCONTEXT *RexxContext;

	RexxContext = AllocMem(sizeof(AREXXCONTEXT),MEMF_PUBLIC|MEMF_CLEAR);
	if ( !RexxContext )
		return( NULL );

	//RexxContext->RexxSysBase = OpenLibrary("rexxsyslib.library",NULL);
	//if ( RexxContext->RexxSysBase )

	{
		strcpy(RexxContext->Extension,"rexx");

    // set up the last error RVI name
    strcpy(RexxContext->ErrorName,RexxName);
    strcat(RexxContext->ErrorName,".LASTERROR");
   	// the portname has to be unique

		RexxContext->ARexxPort = CreatePort((UBYTE *)RexxContext->PortName,0);

		//Permit();
	}

	//if ( !RexxContext->RexxSysBase || !RexxContext->ARexxPort )

	if ( !RexxContext->ARexxPort )
	{
		FreeARexx_V2( RexxContext );
		RexxContext = NULL;
	}

	return( RexxContext );
}

BOOL FreeARexxMsg_V2(AREXXCONTEXT *RexxContext, struct RexxMsg *msg)
{
	if ( !RexxContext || !msg )
		return( FALSE );

	if ( !(msg->rm_Node.mn_Node.ln_Type == NT_REPLYMSG) )
		return( FALSE );

	ClearRexxMsg(msg,16);

	if ( msg->rm_Result1==0 && msg->rm_Result2!=NULL )
		DeleteArgstring((char *)msg->rm_Result2 );

	DeleteRexxMsg(msg);

	return(TRUE);
}

struct RexxMsg *WaitForReply_V2(AREXXCONTEXT *RexxContext)
{
struct RexxMsg *msg, *reply;

	reply=NULL;

	while(reply==NULL)
	{
		Wait(1L << RexxContext->ARexxPort->mp_SigBit);
		while( !reply && (msg=GetARexxMsg_V2(RexxContext))!=NULL )
		{
			if ( !(msg->rm_Node.mn_Node.ln_Type == NT_REPLYMSG) )
			{
				ReplyARexxMsg_V2(RexxContext,msg,RC_OK,"A reply string",0);
			}
			else
				reply = msg;
		}
	}

	return( reply );
}

/******** E O F ********/
