
//#if 0

#include	"nb:pre.h"
#include	<clib/rexxsyslib_protos.h>
#include	<rexx/errors.h>
#include	<rexx/storage.h>
#include	<rexx/rxslib.h>

//#endif

#if 0

#include	<exec/types.h>
#include	<exec/nodes.h>
#include	<exec/lists.h>
#include	<exec/ports.h>
#include	<exec/memory.h>
#include	<clib/exec_protos.h>
#include	<clib/rexxsyslib_protos.h>
#include	<clib/alib_protos.h>
#include	<rexx/errors.h>
#include	<rexx/storage.h>
#include	<rexx/rxslib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<stdarg.h>
#include	<ctype.h>

#endif

#pragma libcall RexxContext->RexxSysBase CreateRexxMsg 90 09803
#pragma libcall RexxContext->RexxSysBase CreateArgstring 7E 0802
#pragma libcall RexxContext->RexxSysBase DeleteRexxMsg 96 801
#pragma libcall RexxContext->RexxSysBase DeleteArgstring 84 801
#pragma libcall RexxContext->RexxSysBase IsRexxMsg A8 801
#pragma libcall RexxContext->RexxSysBase ClearRexxMsg 9C 0802

struct ARexxContext
{
  struct MsgPort *ARexxPort;		/* The port messages come in at... */
  struct Library *RexxSysBase;	/* We will hide the library pointer here... */
  long Outstanding;							/* The count of outstanding ARexx messages... */
  char PortName[24];						/* The port name goes here... */
  char ErrorName[28];						/* The name of the <base>.LASTERROR... */
  char Extension[8];						/* Default file name extension... */
};

#define	AREXXCONTEXT struct ARexxContext *

#include "SimpleRexx.h"

ULONG ARexxSignal(AREXXCONTEXT RexxContext)
{
	if ( RexxContext )
		return( (ULONG)(1L << RexxContext->ARexxPort->mp_SigBit) );
	else
		return( 0L );
}

BOOL IsARexxReply(struct RexxMsg *msg)
{
	if ( msg )
		return( (BOOL)(msg->rm_Node.mn_Node.ln_Type == NT_REPLYMSG) );
	else
		return( FALSE );
}

struct RexxMsg *GetARexxMsg(AREXXCONTEXT RexxContext)
{
struct RexxMsg *msg;

	if ( !RexxContext )
		return( NULL );

	msg = (struct RexxMsg *)GetMsg( RexxContext->ARexxPort );

	if ( !msg )
		return( NULL );

	if ( IsARexxReply( msg ) )
		--RexxContext->Outstanding;

	return( msg );
}

BOOL ReplyARexxMsg(	AREXXCONTEXT RexxContext, struct RexxMsg *msg,
										LONG return_code, char *result_string,
										LONG error_code )
{
	if ( !RexxContext || !msg )
		return( FALSE );

	if ( IsARexxReply(msg) )
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

struct RexxMsg *SendARexxMsg(	AREXXCONTEXT RexxContext, char *port_name,
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

void ARexxFree(AREXXCONTEXT RexxContext)
{
struct RexxMsg *msg;

	if ( !RexxContext )
		return;

	RexxContext->PortName[0]='\0';

	while ( RexxContext->Outstanding>0 && RexxContext->ARexxPort )
	{
		WaitPort(RexxContext->ARexxPort);
		while( (msg=GetARexxMsg(RexxContext))!=NULL )
		{
			if ( !FreeARexxMsg(RexxContext,msg) )
        ReplyARexxMsg(RexxContext,msg,100,NULL,0);
		}
	}

	if (RexxContext->ARexxPort)
	{
		Forbid();

		while( (msg=GetARexxMsg(RexxContext))!=NULL )
			ReplyARexxMsg(RexxContext,msg,100,NULL,0);

		DeletePort(RexxContext->ARexxPort);

		Permit();
	}

	CloseLibrary(RexxContext->RexxSysBase);

	FreeMem(RexxContext,sizeof(struct ARexxContext));
}

AREXXCONTEXT ARexxInit(char *name, char *extension, BOOL multiple)
{
struct MsgPort *port;
AREXXCONTEXT RexxContext;
short count;
char *ptr;

	RexxContext = AllocMem(sizeof(struct ARexxContext),MEMF_PUBLIC|MEMF_CLEAR);
	if ( !RexxContext )
		return( NULL );

	RexxContext->RexxSysBase = OpenLibrary("rexxsyslib.library",NULL);
	if ( RexxContext->RexxSysBase )
	{
		if (!extension)
			extension="rexx";

		StrNUpper(RexxContext->Extension, extension, 8);
		ptr = StrNUpper(RexxContext->PortName, name, 24);

		Forbid();

		if ( multiple )
		{
			*ptr++ = '.';

			for(count=1; count<100; ++count)
			{
				UiToStr(count,ptr);
				port = FindPort((UBYTE *)RexxContext->PortName);
				if ( !port )
					break;
			}
		}
		else
		{
			port = FindPort((UBYTE *)RexxContext->PortName);
		}

		if ( !port )
			RexxContext->ARexxPort = CreatePort((UBYTE *)RexxContext->PortName,0);

		Permit();
	}

	if ( !RexxContext->RexxSysBase || !RexxContext->ARexxPort )
	{
		ARexxFree( RexxContext );
		RexxContext = NULL;
	}

	return( RexxContext );
}

BOOL FreeARexxMsg(AREXXCONTEXT RexxContext, struct RexxMsg *msg)
{
	if ( !RexxContext || !msg )
		return( FALSE );

	if ( !IsARexxReply(msg) )
		return( FALSE );

	ClearRexxMsg(msg,16);

	if ( msg->rm_Result1==0 && msg->rm_Result2!=NULL )
		DeleteArgstring((char *)msg->rm_Result2 );

	DeleteRexxMsg(msg);

	return(TRUE);
}

char *StrNUpper(char *dest, char *src, unsigned int len)
{
	for( ; *src != '\0' && len-- > 0; ++src )
	{
		if ( islower(*src) )
			*dest++ = toupper( *src );
		else
			*dest++ = *src;
	}
	*dest = '\0';

	return( dest );
}

char *UiToStr(unsigned int num, char *buf)
{
	num %= 100;
	buf[0] = ( num / 10 ) + '\0';
	buf[1] = ( num % 10 ) + '\0';
	buf[2] = '\0';

	return(buf);
}

struct RexxMsg *WaitForReply(AREXXCONTEXT RexxContext)
{
struct RexxMsg *msg, *reply;

	reply=NULL;

	while(reply==NULL)
	{
		Wait(ARexxSignal(RexxContext));
		while( !reply && (msg=GetARexxMsg(RexxContext))!=NULL )
		{
			if ( !IsARexxReply(msg) )
			{
				KPrintF("** Got a message: [%s]\n", msg->rm_Args[0]);
				KPrintF("** Action code %x\n", msg->rm_Action);
				ReplyARexxMsg(RexxContext,msg,RC_OK,"A reply string",0);
			}
			else
				reply = msg;
		}
	}

	return( reply );
}

BOOL SendFunction(STRPTR appName, STRPTR port_name, STRPTR resultStr, ULONG *RC,
									char *args[], int nargs)
{
AREXXCONTEXT RexxContext;
BOOL result, retVal=TRUE;
ULONG flags;
struct RexxMsg *msg;

	/**** Init vars ****/

	result = FALSE;
	flags = RXFF_STRING;

	if ( RC )
	{
		result = TRUE;
		flags |= RXFF_RESULT;
	}

	/**** init ARexx ****/

	RexxContext = ARexxInit(appName,"REXX",TRUE);
	if ( !RexxContext )
		return(FALSE);

	/**** Send command ****/

	if ( CallARexxFunc(	RexxContext,port_name,args[0],flags,nargs,
											args[ 1],args[ 2],args[ 3],args[ 4],args[ 5],args[ 6],args[ 7],args[ 8],
											args[ 9],args[10],args[11],args[12],args[13],args[14],args[15],args[16] ) )
	{
KPrintF("Call ok\n");
		msg = WaitForReply(RexxContext);
		if (RC)
			*RC = msg->rm_Result1;
		if ( resultStr && result && msg->rm_Result1==0 && msg->rm_Result2!=NULL )
			strcpy(resultStr, (char *)msg->rm_Result2);
		FreeARexxMsg(RexxContext,msg);
		retVal = TRUE;
	}
	else
		retVal = FALSE;

	/*** Free ARexx ****/

	ARexxFree( RexxContext );

	return( retVal );
}

struct RexxMsg *CallARexxFunc(AREXXCONTEXT RexxContext, STRPTR port_name,
															char *function, ULONG flags, int num_args, ...)
{
va_list argptr;
struct MsgPort *port;
struct RexxMsg *msg;
BOOL quit;
int i;
char *arg;

	if ( !RexxContext || !function || num_args<0 )
		return( NULL );

	msg = CreateRexxMsg(RexxContext->ARexxPort, RexxContext->Extension,
											RexxContext->PortName);
	if ( !msg )
		return( NULL );

	flags &= ( RXFF_NOIO | RXFF_RESULT | RXFF_STRING );

	msg->rm_Action = RXFUNC | flags;
	msg->rm_Action |= ( num_args & RXARGMASK );

	msg->rm_Args[0] = (STRPTR)CreateArgstring(function,strlen(function));
KPrintF("func=[%s]\n",function);
	msg->rm_Result1 = 0;
	msg->rm_Result2 = (LONG)NULL;

	if ( !msg->rm_Args[0] )
	{
		DeleteRexxMsg( msg );
		return(NULL);
	}

	va_start(argptr, num_args);

	for(i=1,quit=FALSE; i<=num_args && !quit; ++i)
	{
		arg = (char *)va_arg(argptr,char *);
KPrintF("arg [%s]\n", arg);
		if (!arg)
			arg="";
		msg->rm_Args[i] = (STRPTR)CreateArgstring(arg,strlen(arg));
		if ( !msg->rm_Args[i] )
			quit = TRUE;
	}

	va_end(argptr);

	if (quit)
	{
		ClearRexxMsg(msg,16);
		DeleteRexxMsg(msg);
		return(NULL);
	}

	Forbid();

	if ( (port = FindPort((UBYTE *)port_name)) != NULL )
	{
KPrintF("hier\n");
		PutMsg(port,(struct Message *)msg);
		++RexxContext->Outstanding;
	}
	else
	{
		ClearRexxMsg(msg,16);
		DeleteRexxMsg(msg);
		msg = NULL;
	}

	Permit();

	return( msg );
}

BOOL IssueRexxCmd(STRPTR appName, STRPTR port_name, STRPTR command, STRPTR resultStr,
									ULONG *RC)
{
AREXXCONTEXT RexxContext;
BOOL result, retVal=TRUE;
ULONG flags;
struct RexxMsg *msg;

KPrintF("1 [%s]\n", appName);
KPrintF("2 [%s]\n", port_name);
KPrintF("3 [%s]\n", command);

	/**** Init vars ****/

	result = TRUE;
	flags = RXFF_RESULT | RXFF_STRING;

	/**** init ARexx ****/

	RexxContext = ARexxInit(appName,"REXX",TRUE);
	if ( !RexxContext )
		return(FALSE);

	/**** Send command ****/

	if ( SendARexxMsg(RexxContext,port_name,command,flags) )
	{
		msg = WaitForReply(RexxContext);
		if (RC)
			*RC = msg->rm_Result1;
		if ( resultStr && result && msg->rm_Result1==0 && msg->rm_Result2!=NULL )
			strcpy(resultStr, (char *)msg->rm_Result2);
		FreeARexxMsg(RexxContext,msg);
		retVal = TRUE;
	}
	else
		retVal = FALSE;

	/*** Free ARexx ****/

	ARexxFree( RexxContext );

	return( retVal );
}

/******** E O F ********/
