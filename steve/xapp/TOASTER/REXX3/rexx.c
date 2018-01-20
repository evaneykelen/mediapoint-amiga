#include	<exec/types.h>
#include	<libraries/dos.h>
#include	<libraries/dosextens.h>
#include	<intuition/intuition.h>
#include	<proto/exec.h>
#include	<proto/intuition.h>
#include	<rexx/storage.h>
#include	<rexx/errors.h>
#include	<rexx/rxslib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdarg.h>
#include	<stdlib.h>
#include	"SimpleRexx.h"

int main(int argc, char **argv)
{
AREXXCONTEXT RexxContext;
BOOL result;
ULONG flags;
char port_name[20];
char command[256];
struct RexxMsg *msg;

	/**** Init vars ****/

	result = FALSE;
	flags = 0;
	strcpy(port_name,"DDR");
	strcpy(command,"tofront");

	/**** Set extras ****/

	flags |= RXFF_RESULT;
	result = TRUE;

	flags |= RXFF_STRING;

	/**** init ARexx ****/

	RexxContext = ARexxInit("testje","REXX",TRUE);
	if ( !RexxContext )
	{
		printf("RexxContext failed\n");
		exit(0);
	}

	/**** Send command ****/

	if ( SendARexxMsg(RexxContext,port_name,command,flags) )
	{
		msg = WaitForReply(RexxContext);

		printf("Reply Code %d\n",msg->rm_Result1);

		if ( result && msg->rm_Result1==0 && msg->rm_Result2!=NULL )
			printf("result string [%s]\n", (char *)msg->rm_Result2);
		else
			printf("error code %d\n", (int)msg->rm_Result2);

		FreeARexxMsg(RexxContext,msg);
	}
	else
		printf("command send failed\n");

	/*** Free ARexx ****/

	ARexxFree( RexxContext );
}

