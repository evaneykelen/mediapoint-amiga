// File		: Serrexx.c
// Uses		: simplerexx
//	Date		:
// Author	:
// Desc.		: Send an Arexx command
//

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

int SendArexx( char *port, char *com )
{
	int ret = 0;

	AREXXCONTEXT RexxContext;
	BOOL result;
	ULONG flags;
	char port_name[20];
	char command[256];
	struct RexxMsg *msg;

	/**** Init vars ****/

	result = FALSE;
	flags = 0;
	strcpy(port_name, port );
	strcpy(command, com );

	/**** Set extras ****/

	flags |= RXFF_RESULT;
	result = TRUE;

	flags |= RXFF_STRING;

	/**** init ARexx ****/

	RexxContext = ARexxInit("testje","REXX",TRUE);
	if ( RexxContext )
	{
		/**** Send command ****/

		if ( SendARexxMsg(RexxContext,port_name,command,flags) )
		{
			msg = RXWaitForReply(RexxContext);

//			KPrintF("Reply Code %d\n",msg->rm_Result1);

			if ( result && msg->rm_Result1==0 && msg->rm_Result2!=NULL );
/*
				KPrintF("result string [%s]\n", (char *)msg->rm_Result2);
			else
				KPrintF("error code %d\n", (int)msg->rm_Result2);
*/
			FreeARexxMsg(RexxContext,msg);
		}
		else
			ret = 2;
		ARexxFree( RexxContext );
	}
	else
		ret = 1;

	/*** Free ARexx ****/

	return( ret );
}

/*
	Error 1 - printf("RexxContext failed\n");
	Error 2 - printf("command send failed\n");

*/