#include "nb:pre.h"
#include "protos.h"
#include "structs.h"
#include "gen:wait50hz.h"

/**** externals ****/

extern struct Library *medialinkLibBase;
extern struct Library *DiskfontBase;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *IconBase;

/**** functions ****/

/******** PerformActions() ********/

BOOL PerformActions(struct VuPort_record *vp_rec)
{
UBYTE buffer[32];
BOOL retval;

	switch(vp_rec->command)
	{
		case 0:
			vp_rec->serialIO->IOSer.io_Command	= SDCMD_BREAK;		// DTR high
			DoIO((struct IORequest *)vp_rec->serialIO);						// BRK

			buffer[0] = 0x1b;
			retval = DoAsyncIO(vp_rec, buffer);

			buffer[0] = 0xbc;
			retval = DoAsyncIO(vp_rec, buffer);
/*
			buffer[0] = 0x17;
			retval = DoAsyncIO(vp_rec, buffer);

			buffer[0] = 0x30;
			retval = DoAsyncIO(vp_rec, buffer);

			vp_rec->serialIO->IOSer.io_Command	= SDCMD_BREAK;		// DTR high
			DoIO((struct IORequest *)vp_rec->serialIO);						// BRK
*/
			break;
	}

	return(TRUE);
}

/******** DoAsyncIO() ********/

BOOL DoAsyncIO(struct VuPort_record *vp_rec, UBYTE *buffer)
{
ULONG signal, mask, hz_signal=0;
BOOL loop, retval=TRUE;
struct wjif WJIF;
UBYTE receiveBuffer[256];

	/***********************************************/
	/********************* SEND ********************/
	/***********************************************/

printf("1\n");
	if( CheckIO((struct IORequest *)vp_rec->serialIO) )
	{
		AbortIO((struct IORequest *)vp_rec->serialIO);
		WaitIO((struct IORequest *)vp_rec->serialIO);
	}
printf("2\n");

	vp_rec->serialIO->IOSer.io_Length 	= 1;
	vp_rec->serialIO->IOSer.io_Data			= buffer;
	vp_rec->serialIO->IOSer.io_Command	= (UWORD)CMD_WRITE;
	SendIO((struct IORequest *)vp_rec->serialIO);

	/**** wait for asynchronous IO to complete ****/

	WJIF.signum=0;
	hz_signal = set50hz(&WJIF, 500);
	mask = (1L << vp_rec->serialMP->mp_SigBit) | hz_signal;

printf("3\n");

	loop=TRUE;
	while(loop)
	{
		signal = Wait(mask);
		if ( signal & hz_signal )
		{
			loop=FALSE;
			retval=FALSE;
printf("4\n");
		}
		if( CheckIO((struct IORequest *)vp_rec->serialIO) )
		{
			WaitIO((struct IORequest *)vp_rec->serialIO);
			loop=FALSE;
printf("5\n");
		}
	}
	//AbortIO((struct IORequest *)vp_rec->serialIO);
	//WaitIO((struct IORequest *)vp_rec->serialIO);

printf("6\n");

	/**** end wait for IO ****/

	if ( hz_signal != 0 )
		remove50hz( &WJIF );

printf("7\n");

	/***********************************************/
	/******************* RECEIVE *******************/
	/***********************************************/

strcpy(receiveBuffer,"123456789");

	vp_rec->serialIO->IOSer.io_Length 	= 1;
	vp_rec->serialIO->IOSer.io_Data			= receiveBuffer;
	vp_rec->serialIO->IOSer.io_Command	= (UWORD)CMD_READ;
	SendIO((struct IORequest *)vp_rec->serialIO);

printf("8\n");

	/**** wait for asynchronous IO to complete ****/

	WJIF.signum=0;
	hz_signal = set50hz(&WJIF, 500);
	mask = (1L << vp_rec->serialMP->mp_SigBit) | hz_signal;

printf("9\n");

	loop=TRUE;
	while(loop)
	{
		signal = Wait(mask);
		if ( signal & hz_signal )
		{
			loop=FALSE;
			retval=FALSE;
printf("10\n");
		}
		if( CheckIO((struct IORequest *)vp_rec->serialIO) )
		{
			WaitIO((struct IORequest *)vp_rec->serialIO);
			loop=FALSE;
printf("11 - %d bytes received\n", vp_rec->serialIO->IOSer.io_Actual);
		}
	}

	AbortIO((struct IORequest *)vp_rec->serialIO);
	WaitIO((struct IORequest *)vp_rec->serialIO);

printf("12\n");

	/**** end wait for IO ****/

	if ( hz_signal != 0 )
		remove50hz( &WJIF );

printf("13 [%s] %x\n",receiveBuffer,receiveBuffer[0]);

	return(retval);
}

/******** E O F ********/
