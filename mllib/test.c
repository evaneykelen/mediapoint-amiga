#include <exec/exec.h>
#include <exec/types.h>
#include "mllib:MediaLinkLib_proto.h"
#include "mllib:MediaLinkLib_pragma.h"

struct Library *medialinkLibBase;

void main(void)
{
TEXT command[256];
ULONG RC;

	medialinkLibBase = (struct Library *)OpenLibrary("nb:system/mediapoint.library",0L);
	if ( medialinkLibBase && UA_Open_ML_Lib() )
	{
		sprintf(command,"options results\naddress AIR Get_Gadget_Names");
		if ( UA_IssueRexxCmd_V1("TEST", command, FALSE) )
			printf("v1 ok\n");

		sprintf(command,"options results\naddress AIR Get_Gadget_Names\nsay result");
		if ( UA_IssueRexxCmd_V1("TEST", command, TRUE) )
			printf("v1 ok\n");

		if ( UA_IssueRexxCmd_V2("TEST", "AIR", "Get_Gadget_Names", command, &RC) )
			printf("v2 ok [%s] %ld\n",command,RC);

		CloseLibrary((struct Library *)medialinkLibBase);
	}
}
