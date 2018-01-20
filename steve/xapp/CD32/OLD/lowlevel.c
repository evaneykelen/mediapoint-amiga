#include "nb:pre.h"
#include <devices/cdtv.h>
#include "cdtv.h"
#include "protos.h"

/**** externals ****/

extern UBYTE **msgs;

/**** functions ****/

/******** OpenCDTV() ********/

BOOL OpenCDTV(struct CDTV_record *CDTV_rec)
{
#if 0
	if ( CDTV_rec->control == CONTROL_VIA_SERIAL )
	{
		SendSerCmd(CDTV_rec);
		return(TRUE);
	}
#endif

	CDTV_rec->IOReq1	= NULL;
	CDTV_rec->IOReq2	= NULL;
	CDTV_rec->IOPort	= NULL;

	CDTV_rec->IOPort = CreatePort(0,0);
	if (CDTV_rec->IOPort==NULL)
		return(FALSE);

	CDTV_rec->IOReq1 = CreateStdIO(CDTV_rec->IOPort);
	if (CDTV_rec->IOReq1==NULL)
	{
		CloseCDTV(CDTV_rec);
		return(FALSE);
	}

	CDTV_rec->IOReq2 = CreateStdIO(CDTV_rec->IOPort);
	if (CDTV_rec->IOReq2==NULL)
	{
		CloseCDTV(CDTV_rec);
		return(FALSE);
	}

  if(OpenDevice("cdtv.device", 0, (struct IORequest *)CDTV_rec->IOReq1, 0))
	{
		CloseCDTV(CDTV_rec);
    return(FALSE);
	}

	/**** copy Req1 (which now holds device) to Req2 ****/

	CopyMem(CDTV_rec->IOReq1, CDTV_rec->IOReq2, sizeof(struct IOStdReq));

	/**** Read CD table of contents ****/

	DoIOR(CDTV_rec->IOReq1, CD_TOCMSF, 0, 100, CDTV_rec->Toc);

  DoIOR(CDTV_rec->IOReq1, CD_MUTE, 0x7fff, 2, 0);	// Set to full volume

	return(TRUE);
}

/******** CloseCDTV() ********/

void CloseCDTV(struct CDTV_record *CDTV_rec)
{
  if(CDTV_rec->IOReq1->io_Device)
		CloseDevice((struct IORequest *)CDTV_rec->IOReq1);

  if(CDTV_rec->IOReq1)
		DeleteStdIO(CDTV_rec->IOReq1);

  if(CDTV_rec->IOReq2)
		DeleteStdIO(CDTV_rec->IOReq2);

  if(CDTV_rec->IOPort)
		DeletePort(CDTV_rec->IOPort);
}

/******** DoIOR() ********/

void DoIOR(struct IOStdReq *req, int cmd, long off, long len, APTR data)
{
  req->io_Command	= cmd;
  req->io_Offset	= off;
  req->io_Length	= len;
  req->io_Data		= data;

  if(DoIO((struct IORequest *)req))
		; //UA_WarnUser(3000, NULL, NULL);

	WaitIO((struct IORequest *)req);
}

/******** SendIOR() ********/

VOID SendIOR(struct IOStdReq *req, int cmd, long off, long len, APTR data)
{
	req->io_Command	= cmd;
	req->io_Offset	= off;
	req->io_Length	= len;
	req->io_Data		= data;

	SendIO((struct IORequest *)req);
}

/******** CDTV_PlayTrack() ********/

BOOL CDTV_PlayTrack(struct CDTV_record *CDTV_rec)
{
	if ( CDTV_rec->control == CONTROL_VIA_SERIAL )
	{
		SendSerCmd(CDTV_rec, DO_PLAYTRACK, -1, -1);
		return(TRUE);
	}
	else if ( CDTV_rec->control == CONTROL_VIA_CDROM )
	{
		if ( CDTV_IsPlaying(CDTV_rec, FALSE, NULL, NULL) )
		{
		  AbortIO((struct IORequest *)CDTV_rec->IOReq1);
	  	WaitIO((struct IORequest *)CDTV_rec->IOReq1);
		}

		if (CDTV_rec->fadeIn)
		  DoIOR(CDTV_rec->IOReq1, CD_MUTE, 0, 2, 0);						// volume off
		else
		  DoIOR(CDTV_rec->IOReq1, CD_MUTE, 0x7fff, 2, 0);				// full volume

		if ( CDTV_rec->song < 1 )
			CDTV_rec->song = 1;

		if ( CDTV_rec->song > CDTV_rec->Toc[0].LastTrack )
			CDTV_rec->song = CDTV_rec->Toc[0].LastTrack;

		SendIOR(CDTV_rec->IOReq1, CD_PLAYTRACK, CDTV_rec->song, 0, 0);

		if (CDTV_rec->fadeIn)
		  DoIOR(CDTV_rec->IOReq2, CD_FADE, 0x7fff, 75*5, 0);		// fade to loud

		Delay(25L);	// wait a while

		return(TRUE);
	}

	return(FALSE);
}

/******** CDTV_PlayTrackFromTo() ********/

BOOL CDTV_PlayTrackFromTo(struct CDTV_record *CDTV_rec)
{
ULONG start, end;
int swap;

	if ( CDTV_rec->control == CONTROL_VIA_SERIAL )
	{
		SendSerCmd(CDTV_rec, DO_PLAYFROMTO, -1, -1);
		return(TRUE);
	}
	else if ( CDTV_rec->control == CONTROL_VIA_CDROM )
	{
		if ( CDTV_IsPlaying(CDTV_rec, FALSE, NULL, NULL) )
		{
		  AbortIO((struct IORequest *)CDTV_rec->IOReq1);
	  	WaitIO((struct IORequest *)CDTV_rec->IOReq1);
		}

		if (CDTV_rec->fadeIn)
		  DoIOR(CDTV_rec->IOReq1, CD_MUTE, 0, 2, 0);				// volume off
		else
	  	DoIOR(CDTV_rec->IOReq1, CD_MUTE, 0x7fff, 2, 0);		// full volume

		if ( CDTV_rec->from > CDTV_rec->to )
		{
			swap = CDTV_rec->from;
			CDTV_rec->from = CDTV_rec->to;
			CDTV_rec->to = swap;
		}

		if ( CDTV_rec->from < 1 )
			CDTV_rec->from = 1;

		if ( CDTV_rec->to > CDTV_rec->Toc[0].LastTrack )
			CDTV_rec->to = CDTV_rec->Toc[0].LastTrack;

		start = TOMSF(CDTV_rec->Toc[CDTV_rec->from].Position.MSF.Minute,
						 		  CDTV_rec->Toc[CDTV_rec->from].Position.MSF.Second,
						   		CDTV_rec->Toc[CDTV_rec->from].Position.MSF.Frame);

		if ( CDTV_rec->to == CDTV_rec->Toc[0].LastTrack ) 
			end = TOMSF(CDTV_rec->Toc[0].Position.MSF.Minute,
					   			CDTV_rec->Toc[0].Position.MSF.Second,
					   			CDTV_rec->Toc[0].Position.MSF.Frame);
		else
			end =	TOMSF(CDTV_rec->Toc[CDTV_rec->to+1].Position.MSF.Minute,
					   			CDTV_rec->Toc[CDTV_rec->to+1].Position.MSF.Second,
					   			CDTV_rec->Toc[CDTV_rec->to+1].Position.MSF.Frame);

		SendIOR(CDTV_rec->IOReq1, CD_PLAYMSF, start, end, 0);

		if (CDTV_rec->fadeIn)
	  	DoIOR(CDTV_rec->IOReq2, CD_FADE, 0x7fff, 75*5, 0);	// fade to loud

		Delay(25L);	// wait a while

		return(TRUE);
	}

	return(FALSE);
}

/******** CDTV_PlayTrackStartEnd() ********/

BOOL CDTV_PlayTrackStartEnd(struct CDTV_record *CDTV_rec)
{
ULONG start, end, cd_start, cd_end, swap;
int mm, ss, ff;

	if ( CDTV_rec->control == CONTROL_VIA_SERIAL )
	{
		SendSerCmd(CDTV_rec, DO_PLAYSTARTEND, -1, -1);
		return(TRUE);
	}
	else if ( CDTV_rec->control == CONTROL_VIA_CDROM )
	{
		if ( CDTV_IsPlaying(CDTV_rec, FALSE, NULL, NULL) )
		{
		  AbortIO((struct IORequest *)CDTV_rec->IOReq1);
	  	WaitIO((struct IORequest *)CDTV_rec->IOReq1);
		}

		if (CDTV_rec->fadeIn)
		  DoIOR(CDTV_rec->IOReq1, CD_MUTE, 0, 2, 0);				// volume off
		else
		  DoIOR(CDTV_rec->IOReq1, CD_MUTE, 0x7fff, 2, 0);		// full volume

		cd_start =  TOMSF(CDTV_rec->Toc[1].Position.MSF.Minute,
						   			  CDTV_rec->Toc[1].Position.MSF.Second,
						      		CDTV_rec->Toc[1].Position.MSF.Frame);

		cd_end =  	TOMSF(CDTV_rec->Toc[0].Position.MSF.Minute,
						   			  CDTV_rec->Toc[0].Position.MSF.Second,
						      		CDTV_rec->Toc[0].Position.MSF.Frame);

		sscanf(CDTV_rec->start, "%02d:%02d:%02d", &mm, &ss, &ff);
		start = TOMSF(mm, ss, ff);

		sscanf(CDTV_rec->end, "%02d:%02d:%02d", &mm, &ss, &ff);
		end = TOMSF(mm, ss, ff);

		if ( start > end )
		{
			swap = start;
			start = end;
			end = swap;
		}

		if ( start < cd_start )
			start = cd_start;

		if ( end > cd_end )
			end = cd_end;

		if ( start!=end )
			SendIOR(CDTV_rec->IOReq1, CD_PLAYMSF, start, end, 0);

		if (CDTV_rec->fadeIn)
		  DoIOR(CDTV_rec->IOReq2, CD_FADE, 0x7fff, 75*5, 0);	// fade to loud

		Delay(25L);	// wait a while

		return(TRUE);
	}

	return(FALSE);
}

/******** CDTV_Pause() ********/

BOOL CDTV_Pause(struct CDTV_record *CDTV_rec)
{
struct CDSubQ ReqSubQ;
UBYTE AddrInfo;
BOOL playing=FALSE;

	if ( CDTV_rec->control == CONTROL_VIA_SERIAL )
	{
		SendSerCmd(CDTV_rec, DO_PAUSE, -1, -1);
		return(TRUE);
	}
	else if ( CDTV_rec->control == CONTROL_VIA_CDROM )
	{
		if ( CDTV_IsPlaying(CDTV_rec, FALSE, NULL, NULL) )
		{
		  AbortIO((struct IORequest *)CDTV_rec->IOReq1);
	  	WaitIO((struct IORequest *)CDTV_rec->IOReq1);
		}

		CDTV_rec->IOReq1->io_Command = CDTV_SUBQMSF;
		CDTV_rec->IOReq1->io_Offset  = 0;
		CDTV_rec->IOReq1->io_Length  = 0;
		CDTV_rec->IOReq1->io_Data    = (APTR)&ReqSubQ;
		if ( DoIO( (struct IORequest *)CDTV_rec->IOReq1 ) )
			return(FALSE);
	
		// First check that it is valid
		AddrInfo = ( ReqSubQ.AddrCtrl & ADRCTL_MASK );
		if ( AddrInfo == ADRCTL_NOMODE )
			return(FALSE);			// INVALID SUBQ
	
		switch ( ReqSubQ.Status )
		{
			case ( SQSTAT_NOTVALID ):
			case ( SQSTAT_NOSTAT   ):
				return(FALSE);		// INVALID SUBQ

			case ( SQSTAT_PLAYING  ):
				playing=TRUE;
				break;

			case ( SQSTAT_PAUSED   ):
				playing=FALSE;
				break;
		}

		if ( playing )
		  DoIOR(CDTV_rec->IOReq1, CD_PAUSE, 0, TRUE, NULL);
		else
		  DoIOR(CDTV_rec->IOReq1, CD_PAUSE, 0, FALSE, NULL);

		Delay(25L);	// wait a while

		return(TRUE);
	}

	return(FALSE);
}

/******** CDTV_Stop() ********/

BOOL CDTV_Stop(struct CDTV_record *CDTV_rec)
{
	if ( CDTV_rec->control == CONTROL_VIA_SERIAL )
	{
		SendSerCmd(CDTV_rec, DO_STOP, -1, -1);
		return(TRUE);
	}
	else if ( CDTV_rec->control == CONTROL_VIA_CDROM )
	{
		if ( CDTV_IsPlaying(CDTV_rec, FALSE, NULL, NULL) )
		{
		  AbortIO((struct IORequest *)CDTV_rec->IOReq1);
	  	WaitIO((struct IORequest *)CDTV_rec->IOReq1);
		}

	  DoIOR(CDTV_rec->IOReq1, CD_STOPPLAY, 0, 0, 0);			// stop the play

		Delay(25L);			// wait a while

		return(TRUE);
	}

	return(FALSE);
}

/******** CDTV_GetPrevSong() ********/

void CDTV_GetPrevSong(struct CDTV_record *CDTV_rec, int *song)
{
TEXT track[30], index[30];

	CDTV_IsPlaying(CDTV_rec, TRUE, track, index);
	sscanf(track, "%d", song);

	*song = (*song - 1); 

	if ( *song < 1 )
		*song = 1;

#if 0
	if ( *song > CDTV_rec->Toc[0].LastTrack )
		*song = CDTV_rec->Toc[0].LastTrack;
#endif
}										

/******** CDTV_GetNextSong() ********/

void CDTV_GetNextSong(struct CDTV_record *CDTV_rec, int *song)
{
TEXT track[30], index[30];

	CDTV_IsPlaying(CDTV_rec, TRUE, track, index);
	sscanf(track, "%d", song);

	*song = (*song + 1); 

	if ( *song < 1 )
		*song = 1;

#if 0
	if ( *song > CDTV_rec->Toc[0].LastTrack )
		*song = CDTV_rec->Toc[0].LastTrack;
#endif
}										

/******** CDTV_IsPlaying() ********/

BOOL CDTV_IsPlaying(struct CDTV_record *CDTV_rec, BOOL getPos,
										STRPTR track, STRPTR index)
{
struct CDSubQ ReqSubQ;
UBYTE AddrInfo;

	if ( CDTV_rec->control == CONTROL_VIA_SERIAL )
	{
		SendSerCmd(CDTV_rec, DO_ISPLAYING, -1, -1);
		if ( !GetTwoStringsFromSer(CDTV_rec, track, index) )
		{
			track[0] = '\0';
			index[0] = '\0';
		}
		return(TRUE);
	}
	else if ( CDTV_rec->control == CONTROL_VIA_CDROM )
	{
		CDTV_rec->IOReq2->io_Command = CDTV_SUBQMSF;
		CDTV_rec->IOReq2->io_Offset  = 0;
		CDTV_rec->IOReq2->io_Length  = 0;
		CDTV_rec->IOReq2->io_Data    = (APTR)&ReqSubQ;
		if ( DoIO( (struct IORequest *)CDTV_rec->IOReq2 ) )
			return(FALSE);

		// First check that it is valid
		AddrInfo = ( ReqSubQ.AddrCtrl & ADRCTL_MASK );
		if ( AddrInfo == ADRCTL_NOMODE )
			return(FALSE);			// INVALID SUBQ

		if ( !getPos )
		{
			switch ( ReqSubQ.Status )
			{
				case ( SQSTAT_NOTVALID ):
				case ( SQSTAT_NOSTAT   ):
					return(FALSE);	// INVALID SUBQ

				case ( SQSTAT_PLAYING  ):
					return(TRUE);
	
				case ( SQSTAT_PAUSED   ):
					return(FALSE);
			}
		}

		if ( getPos && (AddrInfo == ADRCTL_POSITION) )
		{
			sprintf(track, "%3d", ReqSubQ.Track);

			sprintf(index, "%02d:%02d:%02d",
							ReqSubQ.DiskPosition.MSF.Minute,
							ReqSubQ.DiskPosition.MSF.Second,
							ReqSubQ.DiskPosition.MSF.Frame);
		}

		return(FALSE);
	}

	return(FALSE);
}

/******** CDTV_GetCDInfo() ********/

void CDTV_GetCDInfo(struct CDTV_record *CDTV_rec,
										STRPTR numTracks, STRPTR duration)
{
	if ( CDTV_rec->control == CONTROL_VIA_SERIAL )
	{
		SendSerCmd(CDTV_rec, DO_GETINFO, -1, -1);
		if ( !GetTwoStringsFromSer(CDTV_rec, numTracks, duration) )
		{
			numTracks[0] = '\0';
			duration[0] = '\0';
		}
	}
	else if ( CDTV_rec->control == CONTROL_VIA_CDROM )
	{
		if ( CDTV_IsPlaying(CDTV_rec, FALSE, NULL, NULL) )
		{
		  AbortIO((struct IORequest *)CDTV_rec->IOReq1);
	  	WaitIO((struct IORequest *)CDTV_rec->IOReq1);
		}

		DoIOR(CDTV_rec->IOReq1, CD_TOCMSF, 0, 100, CDTV_rec->Toc);

		sprintf(numTracks, "%3d", CDTV_rec->Toc[0].LastTrack);

	  DoIOR(CDTV_rec->IOReq1, CD_ISROM, 0, 0, 0);

		if ( CDTV_rec->IOReq1->io_Actual )
			sprintf(duration, msgs[Msg_CDTV_13-1]); // "No audio on CD"
		else
			sprintf(duration, "%02d:%02d:%02d",
							CDTV_rec->Toc[0].Position.MSF.Minute,
						  CDTV_rec->Toc[0].Position.MSF.Second,
					    CDTV_rec->Toc[0].Position.MSF.Frame);

		if(CDTV_rec->IOReq1->io_Actual && CDTV_rec->Toc[0].LastTrack <= 1)
			sprintf(duration, msgs[Msg_CDTV_13-1]); // "No audio on CD"

	  if(CDTV_rec->Toc[0].LastTrack <= 2)
			sprintf(duration, msgs[Msg_CDTV_13-1]); // "No audio on CD"
	}
}

/******** CDTV_Fade() ********/
/*
 * speed 1 is slow
 * speed 2 is fast
 *
 * inout 1 is fade in
 * inout 2 is fade out
 *
 */

BOOL CDTV_Fade(struct CDTV_record *CDTV_rec, int speed, int inout)
{
int fadeSpeed=75*5;

	if (speed==1)
		fadeSpeed *= 2;

	if ( CDTV_rec->control == CONTROL_VIA_SERIAL )
	{
		SendSerCmd(CDTV_rec, DO_FADE, speed, inout);
		return(TRUE);
	}
	else if ( CDTV_rec->control == CONTROL_VIA_CDROM )
	{
		if (inout==1)
		  DoIOR(CDTV_rec->IOReq1, CD_FADE, 0x7fff, fadeSpeed, NULL);
		else
		  DoIOR(CDTV_rec->IOReq1, CD_FADE, 0, fadeSpeed, NULL);
		Delay(25L);			// wait a while
		return(TRUE);
	}

	return(FALSE);
}

/******** CDTV_Reset() ********/

BOOL CDTV_Reset(struct CDTV_record *CDTV_rec)
{
	if ( CDTV_rec->control == CONTROL_VIA_SERIAL )
	{
		SendSerCmd(CDTV_rec, DO_RESET, -1, -1);
		return(TRUE);
	}
	else if ( CDTV_rec->control == CONTROL_VIA_CDROM )
	{
	  DoIOR(CDTV_rec->IOReq1, CDTV_RESET, 0, 0, NULL);
		Delay(25L);			// wait a while
		return(TRUE);
	}

	return(FALSE);
}

/******** CDTV_Mute() ********/
/*
 * mode 1 is on
 * mode 2 is off
 *
 */

BOOL CDTV_Mute(struct CDTV_record *CDTV_rec, int mode)
{
	if ( CDTV_rec->control == CONTROL_VIA_SERIAL )
	{
		SendSerCmd(CDTV_rec, DO_MUTE, mode, -1);
		return(TRUE);
	}
	else if ( CDTV_rec->control == CONTROL_VIA_CDROM )
	{
		if (mode==1)
		  DoIOR(CDTV_rec->IOReq1, CDTV_MUTE, 0, 2, NULL);
		else
		  DoIOR(CDTV_rec->IOReq1, CDTV_MUTE, 0x7fff, 2, NULL);
		Delay(25L);			// wait a while
		return(TRUE);
	}

	return(FALSE);
}

/******** CDTV_FrontPanel() ********/
/*
 * mode 1 is on
 * mode 2 is off
 *
 */

BOOL CDTV_FrontPanel(struct CDTV_record *CDTV_rec, int mode)
{
	if ( CDTV_rec->control == CONTROL_VIA_SERIAL )
	{
		SendSerCmd(CDTV_rec, DO_FRONTPANEL, mode, -1);
		return(TRUE);
	}
	else if ( CDTV_rec->control == CONTROL_VIA_CDROM )
	{
		if ( mode==1)
		  DoIOR(CDTV_rec->IOReq1, CDTV_FRONTPANEL, 0, TRUE, NULL);
		else
		  DoIOR(CDTV_rec->IOReq1, CDTV_FRONTPANEL, 0, FALSE, NULL);
		Delay(25L);			// wait a while
		return(TRUE);
	}

	return(FALSE);
}

/******** E O F ********/
