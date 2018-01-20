#include "nb:pre.h"
#include <devices/audio.h>

/**** code lifted from Term ****/

/**** Local sound info. ****/

#define LEFT0F  1
#define RIGHT0F  2
#define RIGHT1F  4
#define LEFT1F  8

STATIC BYTE SoundPlayed	= FALSE;
STATIC struct IOAudio *AudioBlock = NULL; 

UBYTE AnyChannel[4] =
{
	LEFT1F,
	LEFT0F,
	RIGHT1F,
	RIGHT0F,
};

BYTE *SawTooth = NULL;
BYTE *SawTooth2 = NULL;

BYTE MySawTooth[28] =
{
	0, 18, 36, 54, 73, 91, 109, 127,
	109, 91, 73, 54, 36, 18, 0,
	-18, -36, -54, -73, -91, -109, -127,
	-109,	-91, -73, -54, -36, -18,
};

BYTE MySawTooth2[12] =
{
 40, 41, 40, 41, 40, 41, 40, 41,
};

#define WAVESIZE 28
#define WAVESIZE2 8

#define SIG_AUDIO	(1 << AudioBlock->ioa_Request.io_Message.mn_ReplyPort->mp_SigBit)

/******** CreateBeep() ********/
/*
 *	Set up the audio.device for a decent beep sound.
 */

BYTE CreateBeep(void)
{
	/* Do we already have the resources we need? */

	SawTooth = (BYTE *)AllocMem(WAVESIZE,MEMF_CHIP);
	if ( SawTooth )
		CopyMem(MySawTooth,SawTooth,WAVESIZE);

	SawTooth2 = (BYTE *)AllocMem(WAVESIZE2,MEMF_CHIP);
	if ( SawTooth2 )
		CopyMem(MySawTooth2,SawTooth2,WAVESIZE2);

	if(!AudioBlock)
	{
		struct MsgPort *AudioPort;

		/* No sound so far. */

		SoundPlayed = FALSE;

		/* Create the IO reply port. */

		if(AudioPort = (struct MsgPort *)CreateMsgPort())
		{
			/* Create the audio IO info. */

			if(AudioBlock = (struct IOAudio *)CreateIORequest(AudioPort,sizeof(struct IOAudio)))
			{
				/* Open audio.device */

				if(!OpenDevice(AUDIONAME,0,(struct IORequest *)AudioBlock,0))
					return(TRUE);

				DeleteIORequest(AudioBlock);
			}

			DeleteMsgPort(AudioPort);
		}

		AudioBlock = NULL;

		return(FALSE);
	}
	else
		return(TRUE);
}

/******** DeleteBeep() ********/
/*
 *	Remove the data allocated for the beep sound.
 */

VOID DeleteBeep(void)
{
	if(AudioBlock)
	{
		if(AudioBlock->ioa_Request.io_Device)
			CloseDevice((struct IORequest *)AudioBlock);

		if(AudioBlock->ioa_Request.io_Message.mn_ReplyPort)
			DeleteMsgPort(AudioBlock->ioa_Request.io_Message.mn_ReplyPort);

		DeleteIORequest(AudioBlock);

		AudioBlock = NULL;

		if ( SawTooth )
			FreeMem(SawTooth, WAVESIZE);

		if ( SawTooth2 )
			FreeMem(SawTooth2, WAVESIZE2);
	}
}

/******** ClearAudio() ********/
/*
 *	Clear the audio control block for reuse.
 */

VOID ClearAudio(void)
{
	/* Remove the request. */

	WaitIO((struct IORequest *)AudioBlock);

	/* Clear the signal bit. */

	SetSignal(0,SIG_AUDIO);

	/* Free the channels we had allocated. */

	AudioBlock->ioa_Request.io_Command = ADCMD_FREE;

	DoIO((struct IORequest *)AudioBlock);

	/* No sound running. */

	SoundPlayed = FALSE;
}

/******** Beep() ********/
/*
 *	Produce a decent beep sound.
 *
 * freq = 29000 is Beep, 20000 is True, 7000 is false
 *
 */

VOID Beep(int freq, int which)
{
	if(AudioBlock)
	{
		BYTE PlayItAgainSam;	/* Note: `Sam' is Sam Dicker, the author of
													 *       the original audio.device implementation.
													 */

		/* AudioRequest has returned. */

		if(SetSignal(0,0) & SIG_AUDIO)
			ClearAudio();

		/* Check whether we are to play the sound or not. */

		if(!SoundPlayed)
			PlayItAgainSam = TRUE;
		else
		{
			if(CheckIO((struct IORequest *)AudioBlock))
				PlayItAgainSam = TRUE;
			else
				PlayItAgainSam = FALSE;
		}

		/* May we play the sound? */

		if(PlayItAgainSam)
		{
			/* Allocate a sound channel, we don't want to
			 * wait for it, the `beep' sound is to played
			 * right now.
			 */

			AudioBlock->ioa_Request.io_Command	= ADCMD_ALLOCATE;
			AudioBlock->ioa_Request.io_Flags		= ADIOF_NOWAIT | IOF_QUICK;
			AudioBlock->ioa_Request.io_Message.mn_Node.ln_Pri	= 80;
			AudioBlock->ioa_Data								= AnyChannel;
			AudioBlock->ioa_Length							= 4;

			/* Try the allocation. */

			BeginIO((struct IORequest *)AudioBlock);

			/* If still in progress, no channel is available yet. */

			if(!CheckIO((struct IORequest *)AudioBlock))
			{
				/* Abort the allocation. */

				AbortIO((struct IORequest *)AudioBlock);

				/* Wait for request to be returned. */

				WaitIO((struct IORequest *)AudioBlock);
			}
			else
			{
				/* Wait for request to be returned. */

				if(!WaitIO((struct IORequest *)AudioBlock))
				{
					/* Set up the sound IO data. */

					if ( freq <= 0 ) freq=30000;
					AudioBlock->ioa_Period	= 3563220/freq;
					if ( which==1 )
					{
						AudioBlock->ioa_Volume	= 64;
						AudioBlock->ioa_Cycles	= freq/(WAVESIZE*8);
						AudioBlock->ioa_Data		= &SawTooth[0];
						AudioBlock->ioa_Length	= WAVESIZE;
					}
					else if ( which==2 )
					{
						AudioBlock->ioa_Volume	= 64;	//60;
						AudioBlock->ioa_Cycles	= freq/(WAVESIZE2*16);
						AudioBlock->ioa_Data		= &SawTooth2[0];
						AudioBlock->ioa_Length	= WAVESIZE2;
					}
					AudioBlock->ioa_Request.io_Command	= CMD_WRITE;
					AudioBlock->ioa_Request.io_Flags	= ADIOF_PERVOL;

					/* Start the sound. */

					BeginIO((struct IORequest *)AudioBlock);

					SoundPlayed = TRUE;
				}
			}
		}
	}
}

/******** E O F ********/
