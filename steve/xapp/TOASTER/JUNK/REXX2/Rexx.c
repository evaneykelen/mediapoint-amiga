#include "nb:pre.h"

#if 0
#include	<exec/types.h>
#include	<exec/nodes.h>
#include	<exec/lists.h>
#include	<exec/ports.h>
#include	<exec/memory.h>
#include  <dos/dos.h>
#include  <dos/dosextens.h>
#include	<clib/exec_protos.h>
#include	<clib/alib_protos.h>
#endif

#include	<clib/rexxsyslib_protos.h>
#include	<rexx/storage.h>
#include	<rexx/rxslib.h>
#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdlib.h>

#define REXX_EXTENSION	"rexx"
#define OK							TRUE
#define NOTOK						FALSE
#define MAKEMAIN				FALSE			// TRUE FOR RUNNING STAND-ALONE (TESTING)

struct MsgPort *setup_rexx_port(struct RexxContext *, char *);
void shutdown_rexx_port(struct RexxContext *, struct MsgPort *);
int send_rexx_command(struct RexxContext *, char *buff, struct MsgPort *rexx_port, char *);
void free_rexx_command(struct RexxContext *, struct RexxMsg *);
BOOL IssueARexxCmd(char *, char *, char *, char *,int);

#pragma libcall rc->RexxSysBase CreateRexxMsg 90 09803
#pragma libcall rc->RexxSysBase CreateArgstring 7E 0802
#pragma libcall rc->RexxSysBase DeleteRexxMsg 96 801
#pragma libcall rc->RexxSysBase DeleteArgstring 84 801

struct RexxContext
{
	struct RxsLib *RexxSysBase;
	char portname[64];
};

/******** setup_rexx_port() ********/

struct MsgPort *setup_rexx_port(struct RexxContext *rc, char *host_portname)
{
struct MsgPort *the_port;
int i=1;

	sprintf(rc->portname,"%s.%d",host_portname,i);	// e.g. MP.1 

	Forbid();
	while(i<32)
	{
		if (FindPort(rc->portname))	// already there
		{
			i++;
			sprintf(rc->portname,"%s.%d",host_portname,i);
		}
		else
			break;
	}
	the_port = CreatePort(rc->portname,0L);
	Permit();	

	return(the_port);
}

/******** shutdown_rexx_port() ********/

void shutdown_rexx_port(struct RexxContext *rc, struct MsgPort *rexx_port)
{
	DeletePort(rexx_port);
}

/******** send_rexx_command() ********/

int send_rexx_command(struct RexxContext *rc, char *buff, struct MsgPort *rexx_port,
											char *hostname)
{
struct MsgPort *rexxport;
struct RexxMsg *rexx_command_message;

	Forbid();
	if ((rexxport=FindPort(hostname))==NULL)
	{
		Permit();
		return(NOTOK);
	}

	if ((rc->RexxSysBase=(struct RxsLib *)OpenLibrary(RXSNAME,0L))==NULL)
	{
		Permit();
		return(NOTOK);
	}

	if ((rexx_command_message=CreateRexxMsg(rexx_port,REXX_EXTENSION,
																					rexx_port->mp_Node.ln_Name))==NULL)
	{
		CloseLibrary((struct Library *)rc->RexxSysBase);
		Permit();
		return(NOTOK);
	}

	if ((rexx_command_message->rm_Args[0]=CreateArgstring(buff,strlen(buff)))==NULL)
	{
		DeleteRexxMsg(rexx_command_message);
		CloseLibrary((struct Library *)rc->RexxSysBase);
		Permit();
		return(NOTOK);
	}

	rexx_command_message->rm_Action = RXCOMM | RXFF_RESULT;
	PutMsg(rexxport,(struct Message *)rexx_command_message);
	Permit();

	return(OK);
}

/******** free_rexx_command() ********/

void free_rexx_command(struct RexxContext *rc, struct RexxMsg *rexxmessage)
{
	DeleteArgstring(rexxmessage->rm_Args[0]);
	DeleteRexxMsg(rexxmessage);
	CloseLibrary((struct Library *)rc->RexxSysBase);
}

/******** IssueARexxCmd() ********/

BOOL IssueARexxCmd(char *ourport, char *theirport, char *cmd, char *result,int reslen)
{
struct MsgPort *rexx_port = NULL;
struct RexxMsg *rexxmessage = NULL;
BOOL retval=TRUE;
struct RexxContext *rc;

	rc = AllocMem(sizeof(struct RexxContext),MEMF_CLEAR);
	if (!rc)
		return(FALSE);

	if ((rexx_port = setup_rexx_port(rc,ourport)))	// OUR NAME
	{
//printf("1\n");
		send_rexx_command(rc,cmd,rexx_port,theirport);	// THEIR NAME
//printf("2\n");
		Wait(1L<<rexx_port->mp_SigBit);
//printf("3\n");
		while(rexxmessage = (struct RexxMsg *)GetMsg(rexx_port))
		{
//printf("4\n");
			if ( rexxmessage->rm_Node.mn_Node.ln_Type == NT_REPLYMSG )
			{
				//printf("REXX: %x %x\n",rexxmessage->rm_Result1,rexxmessage->rm_Result2);
				//printf("REXX: %s\n",rexxmessage->rm_Result2);
/*
				{
				int i;
					for(i=0;i<16;i++)
						if ( rexxmessage->rm_Args[i] )
							printf("REXX: %s\n", rexxmessage->rm_Args[i]);
				}
*/

				if ( rexxmessage->rm_Result1!=0 )	// error
					retval=FALSE;
				if ( rexxmessage->rm_Result1==0 && rexxmessage->rm_Result2 )	// there's a RC
				{
					if (result)	// maybe we don't want/expect a result!
						stccpy(result,(char *)rexxmessage->rm_Result2,reslen);
					DeleteArgstring((UBYTE *)rexxmessage->rm_Result2);	// IMPORTANT!
				}
				free_rexx_command(rc,rexxmessage);
			}
			//else
			//	printf("REXX: NO REPLY\n");
		}
//printf("5\n");
		shutdown_rexx_port(rc,rexx_port);
//printf("6\n");
	}
	else
		retval=FALSE;

	FreeMem(rc,sizeof(struct RexxContext));

//printf("7-end\n");

	return(retval);
}

/******** E O F ********/


#if MAKEMAIN 

main(int argc, char *argv[])
{
struct MsgPort *rexx_port = NULL;
struct RexxMsg *rexxmessage = NULL;

	if ((rexx_port = setup_rexx_port("MP")))	// OUR NAME
	{
		send_rexx_command(argv[1],rexx_port,"Neptun");	// THEIR NAME
		Wait(1L<<rexx_port->mp_SigBit);
		while(rexxmessage = (struct RexxMsg *)GetMsg(rexx_port))
		{
			if ( rexxmessage->rm_Node.mn_Node.ln_Type == NT_REPLYMSG )
			{
				/*************************************/
				if ( rexxmessage->rm_Args[0] )
					printf("[%s]\n",rexxmessage->rm_Args[0]);
				printf("result code = %d\n", rexxmessage->rm_Result1);
				if ( rexxmessage->rm_Result1==0 && rexxmessage->rm_Result2 )
				{
					printf("[%s]\n",rexxmessage->rm_Result2);
					DeleteArgstring((UBYTE *)rexxmessage->rm_Result2);	// IMPORTANT!
				}
				/*************************************/
				free_rexx_command(rexxmessage);
			}
		}
		shutdown_rexx_port(rexx_port);
	}

	exit(0);
}

#endif
