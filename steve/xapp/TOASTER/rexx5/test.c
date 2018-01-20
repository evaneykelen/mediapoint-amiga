#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <workbench/startup.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <rexx/errors.h>
#include <rexx/storage.h>
#include <rexx/rxslib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include "rexx.h"

int main(void)
{
AREXXCONTEXT *RexxStuff;
char RString[256];
struct FileHandle *FH;
ULONG RC;

	FH = (struct FileHandle *)Open("T:RC",MODE_NEWFILE);
	RexxStuff=InitARexx("Example");
	if (RexxStuff)
	{
		strcpy(RString,"options results\naddress DDR fps\nsay result");
		SendARexxMsg_V1(RexxStuff,RString,FH);

		FreeARexx_V1(RexxStuff);
	}
	Close((BPTR)FH);

	RexxStuff=InitARexx("Example");
	if (RexxStuff)
	{
		strcpy(RString,"options results\naddress DDR STOP");
		SendARexxMsg_V1(RexxStuff,RString,NULL);
		FreeARexx_V1(RexxStuff);
	}

	if ( IssueRexxCmd_V2("Example","AIR","Get_gadget_names",RString,&RC) )
		printf("[%s] %ld\n", RString, RC);

	return(0);
}
