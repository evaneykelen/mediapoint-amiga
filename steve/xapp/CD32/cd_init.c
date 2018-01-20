#include "nb:pre.h"
#include <devices/cd.h>
#include "cd.h"
#include "protos.h"

//#define	TOMSF(m,s,f) (((ULONG)(m)<<16)+((s)<<8)+(f))
ULONG ToLSN(ULONG LSN, char which);

/******** AllocCD() ********/

struct CD_record *AllocCD(void)
{
	return( (struct CD_record *)
					AllocMem(sizeof(struct CD_record),MEMF_CLEAR|MEMF_ANY) ); 
}

/******** FreeCD() ********/

void FreeCD(struct CD_record *CD_rec)
{
	if ( CD_rec )
		FreeMem(CD_rec, sizeof(struct CD_record)); 
}

/******** OpenCD() ********/

BOOL OpenCD(struct CD_record *CD_rec)
{
	CD_rec->IOReq1 = NULL;
	CD_rec->IOReq2 = NULL;
	CD_rec->IOPort = NULL;
	CD_rec->CD_Device = NULL;

	CD_rec->IOPort = CreatePort(0,0);
	if (CD_rec->IOPort==NULL)
		return(FALSE);

	CD_rec->IOReq1 = CreateStdIO(CD_rec->IOPort);
	if (CD_rec->IOReq1==NULL)
	{
		DeletePort(CD_rec->IOPort);
		return(FALSE);
	}

	CD_rec->IOReq2 = CreateStdIO(CD_rec->IOPort);
	if (CD_rec->IOReq2==NULL)
	{
		DeleteStdIO(CD_rec->IOReq1);
		DeletePort(CD_rec->IOPort);
		return(FALSE);
	}

  if(OpenDevice("cd.device", 0, (struct IORequest *)CD_rec->IOReq1, 0))
	{
		DeleteStdIO(CD_rec->IOReq2);
		DeleteStdIO(CD_rec->IOReq1);
		DeletePort(CD_rec->IOPort);
    return(FALSE);
	}

	CD_rec->CD_Device = CD_rec->IOReq1->io_Device;

	/**** copy Req1 (which now holds device) to Req2 ****/

	CopyMem(CD_rec->IOReq1, CD_rec->IOReq2, sizeof(struct IOStdReq));

	/**** Read CD table of contents ****/

	CD_NewCD(CD_rec);
 
	/**** Set to full volume ****/

  DoIOR(CD_rec->IOReq1, CD_ATTENUATE, 0x7fff, 0, 0);

	return(TRUE);
}

/******** CloseCD() ********/

void CloseCD(struct CD_record *CD_rec)
{
	if ( CD_rec->IOReq1 && !CheckIO((struct IORequest *)CD_rec->IOReq1) )
	{
	  AbortIO((struct IORequest *)CD_rec->IOReq1);
  	WaitIO((struct IORequest *)CD_rec->IOReq1);
	}

	if ( CD_rec->IOReq2 && !CheckIO((struct IORequest *)CD_rec->IOReq2) )
	{
	  AbortIO((struct IORequest *)CD_rec->IOReq2);
  	WaitIO((struct IORequest *)CD_rec->IOReq2);
	}

  if(CD_rec->IOReq1 && CD_rec->CD_Device)
		CloseDevice((struct IORequest *)CD_rec->IOReq1);

	if(CD_rec->IOPort)
		while(GetMsg(CD_rec->IOPort));

  if(CD_rec->IOReq1)
		DeleteStdIO(CD_rec->IOReq1);

  if(CD_rec->IOReq2)
		DeleteStdIO(CD_rec->IOReq2);

  if(CD_rec->IOPort)
		DeletePort(CD_rec->IOPort);

	CD_rec->IOReq1 = NULL;
	CD_rec->IOReq2 = NULL;
	CD_rec->IOPort = NULL;
	CD_rec->CD_Device = NULL;
}

/******** CD_PlayTrack() ********/

BOOL CD_PlayTrack(struct CD_record *CD_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3)
{
BOOL ret;

	if ( !CheckIO((struct IORequest *)CD_rec->IOReq1) )
	{
	  AbortIO((struct IORequest *)CD_rec->IOReq1);
  	WaitIO((struct IORequest *)CD_rec->IOReq1);
	}

	// SET VOLUME

	if (i2)	// fade
	  ret = DoIOR(CD_rec->IOReq1, CD_ATTENUATE, 0, 0, 0);				// volume off
	else
	  ret = DoIOR(CD_rec->IOReq1, CD_ATTENUATE, 0x7fff, 0, 0);	// full volume

	// CHECK VALUES

	if ( i1 < CD_rec->Toc[0].Summary.FirstTrack )
		i1 = CD_rec->Toc[0].Summary.FirstTrack;
	if ( i1 > CD_rec->Toc[0].Summary.LastTrack )
		i1 = CD_rec->Toc[0].Summary.LastTrack;

	// START TO PLAY

	if ( ret )
	{
		if ( CD_DiskWasChanged(CD_rec) )
			CD_NewCD(CD_rec);
		ret = SendIOR(CD_rec->IOReq1, CD_PLAYTRACK, i1, 1, 0);
	}

	// FADE IN

	if ( ret )
	{
		if (i2)	// fade
		  DoIOR(CD_rec->IOReq2, CD_ATTENUATE, 0x7fff, 75*5, 0);		// fade to loud
	}

	return(TRUE);
}

/******** CD_PlayTrackFromTo() ********/

BOOL CD_PlayTrackFromTo(struct CD_record *CD_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3)
{
ULONG start, end;
int swap;
BOOL ret;

	if ( !CheckIO((struct IORequest *)CD_rec->IOReq1) )
	{
	  AbortIO((struct IORequest *)CD_rec->IOReq1);
  	WaitIO((struct IORequest *)CD_rec->IOReq1);
	}

	// SET VOLUME

	if (i3)	// fade
	  ret = DoIOR(CD_rec->IOReq1, CD_ATTENUATE, 0, 0, 0);				// volume off
	else
	  ret = DoIOR(CD_rec->IOReq1, CD_ATTENUATE, 0x7fff, 0, 0);	// full volume

	// CHECK VALUES

	if ( i1 > i2 )	// can't play backwards
	{
		swap = i1;
		i1 = i2;
		i2 = swap;
	}

	if ( i1 < CD_rec->Toc[0].Summary.FirstTrack )
		i1 = CD_rec->Toc[0].Summary.FirstTrack;
	if ( i1 > CD_rec->Toc[0].Summary.LastTrack )
		i1 = CD_rec->Toc[0].Summary.LastTrack;

	if ( i2 < CD_rec->Toc[0].Summary.FirstTrack )
		i2 = CD_rec->Toc[0].Summary.FirstTrack;
	if ( i2 > CD_rec->Toc[0].Summary.LastTrack )
		i2 = CD_rec->Toc[0].Summary.LastTrack;

	// CALCULATE START AND END

	start = CD_rec->Toc[i1].Entry.Position.LSN;

	if ( i2 == CD_rec->Toc[0].Summary.LastTrack ) 
		end = CD_rec->Toc[0   ].Entry.Position.LSN;
	else
		end =	CD_rec->Toc[i2+1].Entry.Position.LSN;

	// START TO PLAY

	if ( ret )
	{
		if ( CD_DiskWasChanged(CD_rec) )
			CD_NewCD(CD_rec);
		ret = SendIOR(CD_rec->IOReq1, CD_PLAYLSN, start, end-start, 0);
	}

	// FADE IN

	if ( ret )
	{
		if (i3)
  		DoIOR(CD_rec->IOReq2, CD_ATTENUATE, 0x7fff, 75*5, 0);	// fade to loud
	}

	return(TRUE);
}

/******** CD_PlayTrackStartEnd() ********/

BOOL CD_PlayTrackStartEnd(struct CD_record *CD_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3)
{
ULONG start, end, cd_start, cd_end, swap;
int mm, ss, ff;
BOOL ret;

	if ( !CheckIO((struct IORequest *)CD_rec->IOReq1) )
	{
	  AbortIO((struct IORequest *)CD_rec->IOReq1);
  	WaitIO((struct IORequest *)CD_rec->IOReq1);
	}

	// SET VOLUME

	if (i1)	// fade
	  ret = DoIOR(CD_rec->IOReq1, CD_ATTENUATE, 0, 0, 0);				// volume off
	else
	  ret = DoIOR(CD_rec->IOReq1, CD_ATTENUATE, 0x7fff, 0, 0);	// full volume

	// CALCULATE START AND END

	cd_start = CD_rec->Toc[1].Entry.Position.LSN;
	cd_end = CD_rec->Toc[0].Entry.Position.LSN;

	sscanf(s1, "%02d:%02d:%02d", &mm, &ss, &ff);
	start = mm*(60*75) + ss*75 + ff;

	sscanf(s2, "%02d:%02d:%02d", &mm, &ss, &ff);
	end = mm*(60*75) + ss*75 + ff;

	// CHECK VALUES

	if ( start > end )
	{
		swap = start;
		start = end;
		end = swap;
	}

	if ( start < cd_start )	// cd_start is start of first track
		start = cd_start;

	if ( end > cd_end )	// cd_end is total length of CD
		end = cd_end;

	// START TO PLAY

	if ( ret )
	{
		if ( start!=end )
		{
			if ( CD_DiskWasChanged(CD_rec) )
				CD_NewCD(CD_rec);
			ret = SendIOR(CD_rec->IOReq1, CD_PLAYLSN, start, end-start, 0);
		}
	}

	// FADE IN

	if ( ret )
	{
		if (i1)
		  DoIOR(CD_rec->IOReq2, CD_ATTENUATE, 0x7fff, 75*5, 0);	// fade to loud
	}

	return(TRUE);
}

/******** CD_Pause() ********/

BOOL CD_Pause(struct CD_record *CD_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3)
{
struct CDInfo Info;
int paused;
BOOL ret;

	ret = DoIOR(CD_rec->IOReq2, CD_INFO, 0, sizeof(struct CDInfo), &Info);
	if ( ret )
	{
		if (Info.Status & CDSTSF_PAUSED)
			paused = 0;
		else
			paused = 1;

	  DoIOR(CD_rec->IOReq2, CD_PAUSE, 0, paused, NULL);
	}

	return(TRUE);
}

/******** CD_Stop() ********/

BOOL CD_Stop(struct CD_record *CD_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3)
{
	if ( !CheckIO((struct IORequest *)CD_rec->IOReq1) )
	{
	  AbortIO((struct IORequest *)CD_rec->IOReq1);
  	WaitIO((struct IORequest *)CD_rec->IOReq1);
	}

	return(TRUE);
}

/******** CD_Fade() ********/

BOOL CD_Fade(struct CD_record *CD_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3)
{
int fadeSpeed=75*5;

	if (i1==1 || i1==3)	// fade in slow and fade out slow
		fadeSpeed *= 2;

	if (i1==1 || i1==2)	// fade in (slow or fast)
	  DoIOR(CD_rec->IOReq2, CD_ATTENUATE, 0x7fff, fadeSpeed, NULL);
	else
	  DoIOR(CD_rec->IOReq2, CD_ATTENUATE, 0, fadeSpeed, NULL);

	return(TRUE);
}

/******** CD_Mute() ********/

BOOL CD_Mute(struct CD_record *CD_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3)
{
	if (i1==1)	// mute on
	  DoIOR(CD_rec->IOReq2, CD_ATTENUATE, 0, 0, NULL);
	else				// mute off
	  DoIOR(CD_rec->IOReq2, CD_ATTENUATE, 0x7fff, 0, NULL);

	return(TRUE);
}

/******** CD_NewCD() ********/

BOOL CD_NewCD(struct CD_record *CD_rec)
{
struct CDInfo Info;
BOOL ret;

	// Get TOC

	ret = DoIOR(CD_rec->IOReq2, CD_TOCLSN, 0, 100, CD_rec->Toc);

#if 0
	{
	int i;
		for(i=1; i<=CD_rec->Toc[0].Summary.LastTrack; i++)
			printf("%02d %02d:%02d:%02d %d\n", i,
										ToLSN(CD_rec->Toc[i].Entry.Position.LSN,0),
										ToLSN(CD_rec->Toc[i].Entry.Position.LSN,1),
										ToLSN(CD_rec->Toc[i].Entry.Position.LSN,2),
										CD_rec->Toc[i].Entry.CtlAdr );
		printf("end %02d:%02d:%02d %d\n",
										ToLSN(CD_rec->Toc[0].Entry.Position.LSN,0),
										ToLSN(CD_rec->Toc[0].Entry.Position.LSN,1),
										ToLSN(CD_rec->Toc[0].Entry.Position.LSN,2),
										CD_rec->Toc[0].Entry.CtlAdr );
	}
#endif

	// Get Info

	if ( ret )
	{
		ret = DoIOR(CD_rec->IOReq2, CD_INFO, 0, sizeof(struct CDInfo), &Info);
		if ( !ret )
			return(FALSE);

#if 0
		if (Info.Status & CDSTSF_CDROM)
			printf("No audio on CD\n");
		else
			printf("%02d:%02d:%02d\n",
							CD_rec->Toc[0].Entry.Position.MSF.Minute,
						  CD_rec->Toc[0].Entry.Position.MSF.Second,
					    CD_rec->Toc[0].Entry.Position.MSF.Frame);

		printf("max speed = %d\n", Info.MaxSpeed);
		printf("audio precision = %d\n", Info.AudioPrecision);
		printf("status = %d\n", Info.Status);
		printf("play speed = %d\n", Info.PlaySpeed);
		printf("read speed = %d\n", Info.ReadSpeed);
		printf("read xl speed = %d\n", Info.ReadXLSpeed);
		printf("sector size = %d\n", Info.SectorSize);

		if (Info.Status & CDSTSF_CLOSED)
			printf("Drive door is closed\n");                        
		if (Info.Status & CDSTSF_DISK)
			printf("A disk has been detected\n");
		if (Info.Status & CDSTSF_SPIN)
			printf("Disk is spinning (motor is on)\n");
		if (Info.Status & CDSTSF_TOC)
			printf("Table of contents read.  Disk is valid.\n");
		if (Info.Status & CDSTSF_CDROM)
			printf("Track 1 contains CD-ROM data\n");
		if (Info.Status & CDSTSF_PLAYING)
			printf("Audio is playing\n");
		if (Info.Status & CDSTSF_PAUSED)
			printf("Pause mode (pauses on play command)\n");
		if (Info.Status & CDSTSF_SEARCH)
			printf("Search mode (Fast Forward/Fast Reverse)\n");
		if (Info.Status & CDSTSF_DIRECTION)
			printf("Search direction (0 = Forward, 1 = Reverse)\n");
#endif

		// Get disk changes

		ret = DoIOR(CD_rec->IOReq2, CD_CHANGENUM, 0, 0, NULL);
		if ( ret )
		{
			if ( CD_rec->IOReq2->io_Error==0 )
				CD_rec->diskChange = CD_rec->IOReq2->io_Actual;

			//printf("NewCD says diskChange = %d\n",CD_rec->diskChange);
		}
	}

	return(TRUE);
}

/******** CD_GetInfo() ********/

BOOL CD_GetInfo(struct CD_record *CD_rec, STRPTR str)
{
struct QCode qcode;

	strcpy(str, "[00] [00] [--:--:--] [--:--:--]");

	CD_rec->IOReq2->io_Command = CD_QCODELSN;
	CD_rec->IOReq2->io_Offset  = 0;
	CD_rec->IOReq2->io_Length  = 0;
	CD_rec->IOReq2->io_Data    = (APTR)&qcode;
	DoIO( (struct IORequest *)CD_rec->IOReq2 );
	if ( !CD_rec->IOReq2->io_Error )
	{
		sprintf(str, "[%02d] [%02d] [%02d:%02d:%02d] [%02d:%02d:%02d]",
						qcode.Track,

						CD_rec->Toc[0].Summary.LastTrack,

						ToLSN(qcode.DiskPosition.LSN,0),
						ToLSN(qcode.DiskPosition.LSN,1),
						ToLSN(qcode.DiskPosition.LSN,2),

						ToLSN(CD_rec->Toc[0].Entry.Position.LSN,0),
						ToLSN(CD_rec->Toc[0].Entry.Position.LSN,1),
						ToLSN(CD_rec->Toc[0].Entry.Position.LSN,2) );
	}
	else
	{
		sprintf(str, "[00] [%02d] [--:--:--] [%02d:%02d:%02d]",
						CD_rec->Toc[0].Summary.LastTrack,						
						ToLSN(CD_rec->Toc[0].Entry.Position.LSN,0),
						ToLSN(CD_rec->Toc[0].Entry.Position.LSN,1),
						ToLSN(CD_rec->Toc[0].Entry.Position.LSN,2) );
	}

	//printf("track=%s  index=%s\n",track,index);

	return(TRUE);
}

/******** CD_GetTrack() ********/

BOOL CD_GetTrack(struct CD_record *CD_rec, STRPTR str, int track)
{
	if ( track == CD_rec->Toc[0].Summary.LastTrack )
	{
		sprintf(str, "[%02d:%02d:%02d] [%02d:%02d:%02d]",
						ToLSN(CD_rec->Toc[track].Entry.Position.LSN,0),
						ToLSN(CD_rec->Toc[track].Entry.Position.LSN,1),
						ToLSN(CD_rec->Toc[track].Entry.Position.LSN,2),
						ToLSN(CD_rec->Toc[0].Entry.Position.LSN,0),
				    ToLSN(CD_rec->Toc[0].Entry.Position.LSN,1),
				   	ToLSN(CD_rec->Toc[0].Entry.Position.LSN,2) );
	}
	else
	{
		sprintf(str, "[%02d:%02d:%02d] [%02d:%02d:%02d]",
						ToLSN(CD_rec->Toc[track].Entry.Position.LSN,0),
						ToLSN(CD_rec->Toc[track].Entry.Position.LSN,1),
						ToLSN(CD_rec->Toc[track].Entry.Position.LSN,2),
						ToLSN(CD_rec->Toc[track+1].Entry.Position.LSN,0),
				    ToLSN(CD_rec->Toc[track+1].Entry.Position.LSN,1),
				   	ToLSN(CD_rec->Toc[track+1].Entry.Position.LSN,2) );
	}

	return(TRUE);
}

/******** CD_DiskWasChanged() ********/

BOOL CD_DiskWasChanged(struct CD_record *CD_rec)
{
	DoIOR(CD_rec->IOReq2, CD_CHANGENUM, 0, 0, NULL);
	if ( CD_rec->IOReq2->io_Error==0 )
	{
		//printf("%d %d\n",CD_rec->diskChange,CD_rec->IOReq2->io_Actual);
		if ( CD_rec->diskChange != CD_rec->IOReq2->io_Actual )
			return(TRUE);	// changed
	}
	return(FALSE);
}

/******** CD_DiskValid() ********/

BOOL CD_DiskIsValid(struct CD_record *CD_rec)
{
struct CDInfo Info;
BOOL ret;

	ret = DoIOR(CD_rec->IOReq2, CD_INFO, 0, sizeof(struct CDInfo), &Info);
	if ( !ret )
		return(FALSE);

	if (!(Info.Status & CDSTSF_CLOSED))
	{
//printf("NOT CLOSED\n");
		return(FALSE);
	}
	if (!(Info.Status & CDSTSF_DISK))
	{
//printf("NO DISK\n");
		return(FALSE);
	}
	if (!(Info.Status & CDSTSF_SPIN))
	{
//printf("NO SPIN\n");
		return(FALSE);
	}
/*
	if (Info.Status & CDSTSF_CDROM)
	{
printf("IS CD-ROM\n");
		return(FALSE);
	}
*/

	//printf("DISK IS VALID\n");
		
	return(TRUE);
}

/******** CD_DiskHasAudio() ********/

BOOL CD_DiskHasAudio(struct CD_record *CD_rec)
{
struct CDInfo Info;
BOOL ret;

	ret = DoIOR(CD_rec->IOReq2, CD_INFO, 0, sizeof(struct CDInfo), &Info);
	if ( !ret )
		return(FALSE);
	if (Info.Status & CDSTSF_TOC)
		return(TRUE);
	
	return(FALSE);
}

/******** ToLSN() ********/

ULONG ToLSN(ULONG LSN, char which)
{
ULONG val;

	if ( which==0 )				// hh
	{
		return( LSN/(60*75) );
	}
	else if ( which==1 )	// mm
	{
		val = LSN/(60*75);
		LSN -= val * (60*75);
		return( LSN/75 );
	}
	else if ( which==2 )	// ff
	{
		val = LSN/(60*75);
		LSN -= val * (60*75);
		val = LSN/75;
		LSN -= val * 75;
		return( LSN );
	}
}

/******** E O F ********/
