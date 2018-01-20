#include "nb:pre.h"
#include "protos.h"
#include "structs.h"
#include "demo:gen/wait50hz.h"

#define SERIAL_DELAY 100L

/**** externals ****/

extern struct Library *medialinkLibBase;
extern struct Library *DiskfontBase;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *IconBase;

/**** functions ****/

/******** sendVPcode() ********/
/*
 * Embeddes a command string into a Selectra VuPort-style frame
 *
 */

BOOL sendVPcode(struct VuPort_record *vp_rec, STRPTR str)
{
TEXT buf[32];

//KPrintF("\n\nSendVPCode [%s]\n\n", buf);

	sprintf(buf, "\2%s\3", str);
	return ( DoAsyncIO(vp_rec, (ULONG)strlen(buf), (APTR)buf, (UWORD)CMD_WRITE) );
}

/******** PerformActions() ********/

BOOL PerformActions(struct VuPort_record *vp_rec)
{
TEXT buffer[50];

	// RESET - Perform VCR reset

	if (!sendVPcode(vp_rec, "A@Z"))
		return(FALSE);
	Delay(SERIAL_DELAY);

	// LINK - Establish Daisy Chain Linking

	if (!sendVPcode(vp_rec, "AAH"))
		return(FALSE);
	Delay(SERIAL_DELAY);

	// SEL - Select VCR on Daisy Chain
	
	sprintf(buffer, "AAI%d", vp_rec->unit);
	if (!sendVPcode(vp_rec, buffer))
		return(FALSE);
	Delay(SERIAL_DELAY);

	switch(vp_rec->command)
	{
		case 0:		// Cue to frame
			sprintf(buffer, "A@T %d", vp_rec->arg1);
			break;
		case 1:		// Eject
			sprintf(buffer, "A@A");
			break;
		case 2:		// Fast Forward
			sprintf(buffer, "A@C");
			break;
		case 3:		// Forward shuttle
			sprintf(buffer, "AAF %d", vp_rec->arg1);
			break;
		case 4:		// Play
			sprintf(buffer, "A@J");
			break;
		case 5:		// Play Fast Forward
			sprintf(buffer, "A@E");
			break;
		case 6:		// Play Fast Reverse
			sprintf(buffer, "A@D");
			break;
		case 7:		// Play segment 
			sprintf(buffer, "A@Q %d %d", vp_rec->arg1, vp_rec->arg2);
			break;
		case 8:		// Play To Frame
			sprintf(buffer, "A@U %d", vp_rec->arg1);
			break;
		case 9:		// Power on/off
			sprintf(buffer, "ACM");
			break;
		case 10:	// Record
			sprintf(buffer, "A@H");
			break;
		case 11:	// Reset
			sprintf(buffer, "A@Z");
			break;
		case 12:	// Reverse play
			sprintf(buffer, "A@K");
			break;
		case 13:	// Reverse shuttle
			sprintf(buffer, "AAG %d", vp_rec->arg1);
			break;
		case 14:	// Rewind
			sprintf(buffer, "A@B");
			break;
		case 15:	// Set cue type
			sprintf(buffer, "AAD %d", vp_rec->arg1);
			break;
		case 16:	// Step forward 1 field
			sprintf(buffer, "A@L");
			break;
		case 17:	// Step reverse 1 field
			sprintf(buffer, "A@M");
			break;
		case 18:	// Still frame
			sprintf(buffer, "A@F");
			break;
		case 19:	// Stop
			sprintf(buffer, "A@@");
			break;
	}

	if (!sendVPcode(vp_rec, buffer))
		return(FALSE);
	Delay(SERIAL_DELAY);

	return(TRUE);
}

/******** DoAsyncIO() ********/

BOOL DoAsyncIO(	struct VuPort_record *vp_rec,
								ULONG io_Length, APTR io_Data, UWORD io_Command )
{
ULONG signal, mask, hz_signal=0;
BOOL loop=TRUE, retval=TRUE;
struct wjif WJIF;

	if( CheckIO((struct IORequest *)vp_rec->serialIO) )
	{
		AbortIO((struct IORequest *)vp_rec->serialIO);
		WaitIO((struct IORequest *)vp_rec->serialIO);
	}

	vp_rec->serialIO->IOSer.io_Length 	= io_Length;
	vp_rec->serialIO->IOSer.io_Data			= io_Data;
	vp_rec->serialIO->IOSer.io_Command	= io_Command;
	SendIO((struct IORequest *)vp_rec->serialIO);

	/**** wait for asynchronous IO to complete ****/

	WJIF.signum=0;
	hz_signal = set50hz(&WJIF, 500);

	mask = (1L << vp_rec->serialMP->mp_SigBit) | hz_signal;

	while(loop)
	{
		signal = Wait(mask);

		if ( signal & hz_signal )
		{
			loop=FALSE;
			retval=FALSE;
		}

		if( CheckIO((struct IORequest *)vp_rec->serialIO) )
		{
			WaitIO((struct IORequest *)vp_rec->serialIO);
			loop=FALSE;
		}
	}

	AbortIO((struct IORequest *)vp_rec->serialIO);
	WaitIO((struct IORequest *)vp_rec->serialIO);

	/**** end wait for IO ****/

	if ( hz_signal != 0 )
		remove50hz( &WJIF );

	return(retval);
}

/******** E O F ********/
