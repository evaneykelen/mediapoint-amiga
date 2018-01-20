#include "nb:pre.h"
#include "protos.h"
#include "structs.h"
#include "demo:gen/wait50hz.h"

/**** externals ****/

extern struct Library *medialinkLibBase;
extern struct Library *DiskfontBase;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *IconBase;

/**** functions ****/

/******** sendIV321code() ********/
/*
 * Embeddes a Canon IV321 command string into a Canon-style frame
 *
 * unitStr = FF
 *
 */

BOOL sendIV321code(struct Ion_record *Ion_rec, STRPTR str)
{
TEXT buf[32];

	/****      1 S2 FF str 4	****/
	/**** byte 0 12 34 5			****/

	sprintf(buf, "\1S2%s%s\4", Ion_rec->unitStr, str);

	return ( DoAsyncIO(Ion_rec, (ULONG)strlen(buf), (APTR)buf, (UWORD)CMD_WRITE) );
}

/******** GetCurrentFrame() ********/

BOOL GetCurrentFrame(struct Ion_record *Ion_rec, int *frame)
{
TEXT buf[32];
BOOL retval;

	if (!sendIV321code(Ion_rec, "IDR"))
		return(FALSE);
	Delay(CMD_FINISHED_DELAY);

	retval = DoAsyncIO(Ion_rec, 27L, (APTR)buf, (UWORD)CMD_READ);
	Delay(CMD_FINISHED_DELAY);

	*frame = (buf[25]-0x30)*10 + buf[26]-0x30;

	return(retval);
}

/******** PerformActions() ********/

BOOL PerformActions(struct Ion_record *Ion_rec)
{
TEXT buffer[10];
int currentFrame, start, end, i;
BOOL retval;

	/**** switch counter on/off ****/

	if ( Ion_rec->counterOn )
		retval = sendIV321code(Ion_rec, "T1");
	else
		retval = sendIV321code(Ion_rec, "T0");
	Delay(CMD_FINISHED_DELAY);

	if (!retval)
		return(FALSE);

	/**** cancel image muting ****/

	if (!sendIV321code(Ion_rec, "M0"))
		return(FALSE);
	Delay(CMD_FINISHED_DELAY);

	/**** do a forward or a reverse if requested frame is near by ****/

	if ( Ion_rec->action==1 )
		start = Ion_rec->frame;
	else
		start = Ion_rec->startFrame;

	GetCurrentFrame(Ion_rec, &currentFrame);
	if ( start == (currentFrame+1) )
	{
		if (!sendIV321code(Ion_rec, "F"))	// forward
			return(FALSE);
		if (Ion_rec->delay == 0)
			Delay(5*CMD_FINISHED_DELAY);
		else
			Delay(50*Ion_rec->delay);
	}
	else if ( start == (currentFrame-1) )
	{
		if (!sendIV321code(Ion_rec, "R"))	// reverse
			return(FALSE);
		if (Ion_rec->delay == 0)
			Delay(5*CMD_FINISHED_DELAY);
		else
			Delay(50*Ion_rec->delay);
	}
	else
	{
		sprintf(buffer, "A%02d", start);
		if (!sendIV321code(Ion_rec, buffer))
			return(FALSE);
		if ( Ion_rec->delay<=1)
			Delay(100L);
		else
			Delay(50*Ion_rec->delay);
	}

	if (Ion_rec->action==2 &&	(Ion_rec->startFrame != Ion_rec->endFrame) )
	{
		start = Ion_rec->startFrame;
		end = Ion_rec->endFrame;
		if (start>end)
		{
			i=start;
			start=end;
			end=i;
			strcpy(buffer, "R");	// go into reverse
		}
		else
			strcpy(buffer, "F");	// go forward

		for(i=start; i<end; i++)
		{
			if (!sendIV321code(Ion_rec, buffer))
				return(FALSE);
			if (Ion_rec->delay == 0)
				Delay(5*CMD_FINISHED_DELAY);
			else
				Delay(50*Ion_rec->delay);
		}
	}

	/**** blank screen if requested ****/

	if ( Ion_rec->blank )
	{
		if (Ion_rec->delay == 0)
			Delay(100);
		if (!sendIV321code(Ion_rec, "M1"))
			return(FALSE);
		Delay(CMD_FINISHED_DELAY);
	}

	return(TRUE);
}

/******** DoAsyncIO() ********/

BOOL DoAsyncIO(	struct Ion_record *Ion_rec,
								ULONG io_Length, APTR io_Data, UWORD io_Command )
{
ULONG signal, mask, hz_signal=0;
BOOL loop=TRUE, retval=TRUE;
struct wjif WJIF;

	if( CheckIO((struct IORequest *)Ion_rec->serialIO) )
	{
		AbortIO((struct IORequest *)Ion_rec->serialIO);
		WaitIO((struct IORequest *)Ion_rec->serialIO);
	}

	Ion_rec->serialIO->IOSer.io_Length 	= io_Length;
	Ion_rec->serialIO->IOSer.io_Data		= io_Data;
	Ion_rec->serialIO->IOSer.io_Command	= io_Command;
	SendIO((struct IORequest *)Ion_rec->serialIO);

	/**** wait for asynchronous IO to complete ****/

	WJIF.signum=0;
	hz_signal = set50hz(&WJIF, 500);

	mask = (1L << Ion_rec->serialMP->mp_SigBit) | hz_signal;

	while(loop)
	{
		signal = Wait(mask);

		if ( signal & hz_signal )
		{
			loop=FALSE;
			retval=FALSE;
		}

		if( CheckIO((struct IORequest *)Ion_rec->serialIO) )
		{
			WaitIO((struct IORequest *)Ion_rec->serialIO);
			loop=FALSE;
		}
	}

	AbortIO((struct IORequest *)Ion_rec->serialIO);
	WaitIO((struct IORequest *)Ion_rec->serialIO);

	/**** end wait for IO ****/

	if ( hz_signal != 0 )
		remove50hz( &WJIF );

	return(retval);
}

/******** E O F ********/
