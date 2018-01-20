#include <workbench/startup.h>
// CLIB
#include <clib/exec_protos.h>
// PRAGMAS
#include <pragmas/exec_pragmas.h>
// ROOT
#include <string.h>
#include <stdio.h>
// USER
#include "demo:Gen/minc/types.h"
#include "demo:Gen/minc/process.h"

PROCESSINFO *ml_FindBaseAddr( int argc, char **argv)
{
	if( argc == 0 )
		if(((struct WBStartup *)argv)->sm_Message.mn_Length >=
			( sizeof(PROCESSINFO)-sizeof(struct Node)-sizeof(UWORD)))
			return((PROCESSINFO *)((ULONG)argv - (ULONG)sizeof(struct Node) - sizeof(UWORD)) );
	return( NULL );
}

void MakeFullPath( STRPTR Path, STRPTR Name, STRPTR Dest ) 
{
  int PathLength;

	PathLength = strlen(Path);
	if(Path[PathLength-1] == ':' || Path[PathLength-1] == '/')
	{
		strcpy(Dest,Path);
		strcat(Dest,Name);
	}
	else 
	{
		if(Path != NULL && Path[0] != '\0')
		{
			strcpy(Dest,Path);
			strcat(Dest,"/");
			strcat(Dest,Name);
		}
		else
			strcpy(Dest, Name);
	}	
}

BOOL SendDialogue( PROCDIALOGUE *Msg_Dial, PROCESSINFO *PI, int Cmd)
{
	if(Msg_Dial->pd_InUse)
		return(FALSE);

	Msg_Dial->pd_ChildPI = PI;
	Msg_Dial->pd_InUse = TRUE;
	Msg_Dial->pd_Cmd = Cmd;
	PutMsg(PI->pi_Port_CtoP,(struct Message *)Msg_Dial);
	return(TRUE);
}
