#include	<exec/types.h>
#include	<libraries/dos.h>
#include	<libraries/dosextens.h>
#include	<intuition/intuition.h>
#include	<proto/exec.h>
#include	<proto/intuition.h>
#include	<rexx/storage.h>
#include	<rexx/rxslib.h>
#include	<stdio.h>
#include	<string.h>
#include	<dos/dos.h>
#include	<dos/dosextens.h>
#include	"SimpleRexx.h"

/*
 * Lattice control-c stop...
 */
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */

int main(void)
{
AREXXCONTEXT RexxStuff;
char RString[256];
struct FileLock *FL;

	FL = (struct FileLock *)Open("T:RC",MODE_NEWFILE);

	RexxStuff=InitARexx("Example","test");
	if (RexxStuff)
	{
		strcpy(RString,"options results\naddress EXAMPLE.1 readtitle\nsay result");
		SendARexxMsg(RexxStuff,RString,TRUE,FL);
		FreeARexx(RexxStuff);
	}

	Close((BPTR)FL);

	return(0);
}
