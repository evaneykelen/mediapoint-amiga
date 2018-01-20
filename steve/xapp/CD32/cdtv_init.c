#include "nb:pre.h"
#include <devices/cdtv.h>
#include "cdtv.h"
#include "protos.h"

/******** AllocCDTV() ********/

struct CDTV_record *AllocCDTV(void)
{
	return( (struct CDTV_record *)
					AllocMem(sizeof(struct CDTV_record),MEMF_CLEAR|MEMF_ANY) ); 
}

/******** FreeCDTV() ********/

void FreeCDTV(struct CDTV_record *CDTV_rec)
{
	if ( CDTV_rec )
		FreeMem(CDTV_rec, sizeof(struct CDTV_record)); 
}

/******** OpenCDTV() ********/

BOOL OpenCDTV(struct CDTV_record *CDTV_rec)
{
	CDTV_rec->IOReq1	= NULL;
	CDTV_rec->IOReq2	= NULL;
	CDTV_rec->IOPort	= NULL;

	CDTV_rec->IOPort = CreatePort(0,0);
	if (CDTV_rec->IOPort==NULL)
		return(FALSE);

	CDTV_rec->IOReq1 = CreateStdIO(CDTV_rec->IOPort);
	if (CDTV_rec->IOReq1==NULL)
	{
		DeletePort(CDTV_rec->IOPort);
		return(FALSE);
	}

	CDTV_rec->IOReq2 = CreateStdIO(CDTV_rec->IOPort);
	if (CDTV_rec->IOReq2==NULL)
	{
		DeleteStdIO(CDTV_rec->IOReq1);
		DeletePort(CDTV_rec->IOPort);
		return(FALSE);
	}

  if(OpenDevice("cdtv.device", 0, (struct IORequest *)CDTV_rec->IOReq1, 0))
	{
		DeleteStdIO(CDTV_rec->IOReq2);
		DeleteStdIO(CDTV_rec->IOReq1);
		DeletePort(CDTV_rec->IOPort);
    return(FALSE);
	}

	/**** copy Req1 (which now holds device) to Req2 ****/

	CopyMem(CDTV_rec->IOReq1, CDTV_rec->IOReq2, sizeof(struct IOStdReq));

	/**** Read CD table of contents ****/

	CDTV_NewCD(CDTV_rec);

	/**** Set to full volume ****/

  DoIOR(CDTV_rec->IOReq1, CD_MUTE, 0x7fff, 2, 0);	// Set to full volume

	return(TRUE);
}

/******** CloseCDTV() ********/

void CloseCDTV(struct CDTV_record *CDTV_rec)
{
  if(CDTV_rec->IOReq1 && CDTV_rec->IOReq1->io_Device)
		CloseDevice((struct IORequest *)CDTV_rec->IOReq1);

  if(CDTV_rec->IOReq1)
		DeleteStdIO(CDTV_rec->IOReq1);

  if(CDTV_rec->IOReq2)
		DeleteStdIO(CDTV_rec->IOReq2);

  if(CDTV_rec->IOPort)
		DeletePort(CDTV_rec->IOPort);

	CDTV_rec->IOReq1	= NULL;
	CDTV_rec->IOReq2	= NULL;
	CDTV_rec->IOPort	= NULL;
}

/******** CDTV_PlayTrack() ********/

BOOL CDTV_PlayTrack(struct CDTV_record *CDTV_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3)
{
BOOL ret;

	if ( !CheckIO((struct IORequest *)CDTV_rec->IOReq1) )
	{
	  AbortIO((struct IORequest *)CDTV_rec->IOReq1);
  	WaitIO((struct IORequest *)CDTV_rec->IOReq1);
	}

	// SET VOLUME

	if (i2)	// fade
	  ret = DoIOR(CDTV_rec->IOReq1, CD_MUTE, 0, 2, 0);						// volume off
	else
	  ret = DoIOR(CDTV_rec->IOReq1, CD_MUTE, 0x7fff, 2, 0);				// full volume

	// CHECK VALUES

	if ( i1 < 1 )
		i1 = 1;
	if ( i1 > CDTV_rec->Toc[0].LastTrack )
		i1 = CDTV_rec->Toc[0].LastTrack;

	// START TO PLAY

	if ( ret )
	{
		if ( CDTV_DiskWasChanged(CDTV_rec) )
			CDTV_NewCD(CDTV_rec);
		ret = SendIOR(CDTV_rec->IOReq1, CD_PLAYTRACK, i1, 0, 0);
	}

	// FADE IN

	if ( ret )
	{
		if (i2)	// fade
		  DoIOR(CDTV_rec->IOReq2, CD_FADE, 0x7fff, 75*5, 0);		// fade to loud
	}

	return(TRUE);
}

/******** CDTV_PlayTrackFromTo() ********/

BOOL CDTV_PlayTrackFromTo(struct CDTV_record *CDTV_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3)
{
ULONG start, end;
int swap;
BOOL ret;

	if ( !CheckIO((struct IORequest *)CDTV_rec->IOReq1) )
	{
	  AbortIO((struct IORequest *)CDTV_rec->IOReq1);
  	WaitIO((struct IORequest *)CDTV_rec->IOReq1);
	}

	// SET VOLUME

	if (i3)	// fade
	  ret = DoIOR(CDTV_rec->IOReq1, CD_MUTE, 0, 2, 0);						// volume off
	else
	  ret = DoIOR(CDTV_rec->IOReq1, CD_MUTE, 0x7fff, 2, 0);				// full volume

	// CHECK VALUES

	if ( i1 > i2 )	// can't play backwards
	{
		swap = i1;
		i1 = i2;
		i2 = swap;
	}

	if ( i1 < 1 )
		i1 = 1;
	if ( i1 > CDTV_rec->Toc[0].LastTrack )
		i1 = CDTV_rec->Toc[0].LastTrack;

	if ( i2 < 1 )
		i2 = 1;
	if ( i2 > CDTV_rec->Toc[0].LastTrack )
		i2 = CDTV_rec->Toc[0].LastTrack;

	// CALCULATE START AND END

	start = TOMSF(CDTV_rec->Toc[i1].Position.MSF.Minute,
					 		  CDTV_rec->Toc[i1].Position.MSF.Second,
					   		CDTV_rec->Toc[i1].Position.MSF.Frame);

	if ( i2 == CDTV_rec->Toc[0].LastTrack ) 
		end = TOMSF(CDTV_rec->Toc[0].Position.MSF.Minute,
				   			CDTV_rec->Toc[0].Position.MSF.Second,
				   			CDTV_rec->Toc[0].Position.MSF.Frame);
	else
		end =	TOMSF(CDTV_rec->Toc[i2+1].Position.MSF.Minute,
				   			CDTV_rec->Toc[i2+1].Position.MSF.Second,
				   			CDTV_rec->Toc[i2+1].Position.MSF.Frame);

	// START TO PLAY

	if ( ret )
	{
		if ( CDTV_DiskWasChanged(CDTV_rec) )
			CDTV_NewCD(CDTV_rec);
		ret = SendIOR(CDTV_rec->IOReq1, CD_PLAYMSF, start, end, 0);
	}

	// FADE IN

	if ( ret )
	{
		if (i3)
  		DoIOR(CDTV_rec->IOReq2, CD_FADE, 0x7fff, 75*5, 0);	// fade to loud
	}

	return(TRUE);
}

/******** CDTV_PlayTrackStartEnd() ********/

BOOL CDTV_PlayTrackStartEnd(struct CDTV_record *CDTV_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3)
{
ULONG start, end, cd_start, cd_end, swap;
int mm, ss, ff;
BOOL ret;

	if ( !CheckIO((struct IORequest *)CDTV_rec->IOReq1) )
	{
	  AbortIO((struct IORequest *)CDTV_rec->IOReq1);
  	WaitIO((struct IORequest *)CDTV_rec->IOReq1);
	}

	// SET VOLUME

	if (i1)	// fade
	  ret = DoIOR(CDTV_rec->IOReq1, CD_MUTE, 0, 2, 0);						// volume off
	else
	  ret = DoIOR(CDTV_rec->IOReq1, CD_MUTE, 0x7fff, 2, 0);				// full volume

	// CALCULATE START AND END

	cd_start =  TOMSF(CDTV_rec->Toc[1].Position.MSF.Minute,
					   			  CDTV_rec->Toc[1].Position.MSF.Second,
					      		CDTV_rec->Toc[1].Position.MSF.Frame);

	cd_end =  	TOMSF(CDTV_rec->Toc[0].Position.MSF.Minute,
					   			  CDTV_rec->Toc[0].Position.MSF.Second,
					      		CDTV_rec->Toc[0].Position.MSF.Frame);

	sscanf(s1, "%02d:%02d:%02d", &mm, &ss, &ff);
	start = TOMSF(mm, ss, ff);

	sscanf(s2, "%02d:%02d:%02d", &mm, &ss, &ff);
	end = TOMSF(mm, ss, ff);

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
			if ( CDTV_DiskWasChanged(CDTV_rec) )
				CDTV_NewCD(CDTV_rec);
			ret = SendIOR(CDTV_rec->IOReq1, CD_PLAYMSF, start, end, 0);
		}
	}

	// FADE IN

	if ( ret )
	{
		if (i1)
		  DoIOR(CDTV_rec->IOReq2, CD_FADE, 0x7fff, 75*5, 0);	// fade to loud
	}

	return(TRUE);
}

/******** CDTV_Pause() ********/

BOOL CDTV_Pause(struct CDTV_record *CDTV_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3)
{
struct CDSubQ ReqSubQ;
UBYTE AddrInfo;
BOOL playing=FALSE;

	if ( !CheckIO((struct IORequest *)CDTV_rec->IOReq1) )
	{
	  AbortIO((struct IORequest *)CDTV_rec->IOReq1);
  	WaitIO((struct IORequest *)CDTV_rec->IOReq1);
	}

	CDTV_rec->IOReq1->io_Command = CDTV_SUBQMSF;
	CDTV_rec->IOReq1->io_Offset  = 0;
	CDTV_rec->IOReq1->io_Length  = 0;
	CDTV_rec->IOReq1->io_Data    = (APTR)&ReqSubQ;
	DoIO( (struct IORequest *)CDTV_rec->IOReq1 );
	if ( CDTV_rec->IOReq1->io_Error )
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

	return(TRUE);
}

/******** CDTV_Stop() ********/

BOOL CDTV_Stop(struct CDTV_record *CDTV_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3)
{
	if ( !CheckIO((struct IORequest *)CDTV_rec->IOReq1) )
	{
	  AbortIO((struct IORequest *)CDTV_rec->IOReq1);
  	WaitIO((struct IORequest *)CDTV_rec->IOReq1);
	}

  DoIOR(CDTV_rec->IOReq1, CD_STOPPLAY, 0, 0, 0);

	return(TRUE);
}

/******** CDTV_Fade() ********/

BOOL CDTV_Fade(struct CDTV_record *CDTV_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3)
{
int fadeSpeed=75*5;

	if (i1==1 || i1==3)	// fade in slow and fade out slow
		fadeSpeed *= 2;

	if (i1==1 || i1==2)	// fade in (slow or fast)
	  DoIOR(CDTV_rec->IOReq1, CD_FADE, 0x7fff, fadeSpeed, NULL);
	else
	  DoIOR(CDTV_rec->IOReq1, CD_FADE, 0, fadeSpeed, NULL);

	return(TRUE);
}

/******** CDTV_Mute() ********/

BOOL CDTV_Mute(struct CDTV_record *CDTV_rec, int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3)
{
	if (i1==1)	// mute on
	  DoIOR(CDTV_rec->IOReq1, CDTV_MUTE, 0, 2, NULL);
	else				// mute off
	  DoIOR(CDTV_rec->IOReq1, CDTV_MUTE, 0x7fff, 2, NULL);

	return(TRUE);
}

/******** CDTV_NewCD() ********/

BOOL CDTV_NewCD(struct CDTV_record *CDTV_rec)
{
BOOL ret;

	ret = DoIOR(CDTV_rec->IOReq1, CD_TOCMSF, 0, 100, CDTV_rec->Toc);
	if ( !ret )
		return(FALSE);

	//printf("nt %d\n", CDTV_rec->Toc[0].LastTrack);

  ret = DoIOR(CDTV_rec->IOReq1, CD_ISROM, 0, 0, 0);
	if ( !ret )
		return(FALSE);

#if 0
	if ( CDTV_rec->IOReq1->io_Actual )
		printf("No audio on CD\n");
	else
		printf("%02d:%02d:%02d\n",
						CDTV_rec->Toc[0].Position.MSF.Minute,
					  CDTV_rec->Toc[0].Position.MSF.Second,
				    CDTV_rec->Toc[0].Position.MSF.Frame);

	if(CDTV_rec->IOReq1->io_Actual && CDTV_rec->Toc[0].LastTrack <= 1)
		printf("No audio on CD\n");

  if(CDTV_rec->Toc[0].LastTrack <= 2)
		printf("No audio on CD\n");
#endif

	ret = DoIOR(CDTV_rec->IOReq2, CDTV_CHANGENUM, 0, 0, NULL);
	if ( !ret )
		return( FALSE );

	if ( CDTV_rec->IOReq2->io_Error==0 )
		CDTV_rec->diskChange = CDTV_rec->IOReq2->io_Actual;

	//printf("NewCD says diskChange = %d\n",CDTV_rec->diskChange);

	return(TRUE);
}

/******** CDTV_GetInfo() ********/

BOOL CDTV_GetInfo(struct CDTV_record *CDTV_rec, STRPTR str)
{
struct CDSubQ ReqSubQ;
//UBYTE AddrInfo;

	strcpy(str, "[00] [00] [--:--:--] [--:--:--]");

	CDTV_rec->IOReq2->io_Command = CDTV_SUBQMSF;
	CDTV_rec->IOReq2->io_Offset  = 0;
	CDTV_rec->IOReq2->io_Length  = 0;
	CDTV_rec->IOReq2->io_Data    = (APTR)&ReqSubQ;
	DoIO( (struct IORequest *)CDTV_rec->IOReq2 );
	if ( CDTV_rec->IOReq2->io_Error )
	{
		return(FALSE);
	}
#if 0
	// First check that it is valid
	AddrInfo = ( ReqSubQ.AddrCtrl & ADRCTL_MASK );
	if ( AddrInfo == ADRCTL_NOMODE )
	{
		printf("GetInfo failure 2\n");
		return(FALSE);	// INVALID SUBQ
	}

	if ( AddrInfo == ADRCTL_POSITION )
#endif
	{
		sprintf(str, "[%02d] [%02d] [%02d:%02d:%02d] [%02d:%02d:%02d]",
						ReqSubQ.Track,

						CDTV_rec->Toc[0].LastTrack,

						ReqSubQ.DiskPosition.MSF.Minute,
						ReqSubQ.DiskPosition.MSF.Second,
						ReqSubQ.DiskPosition.MSF.Frame,

						CDTV_rec->Toc[0].Position.MSF.Minute,
		   			CDTV_rec->Toc[0].Position.MSF.Second,
		   			CDTV_rec->Toc[0].Position.MSF.Frame );
	}
#if 0
	else
	{
		sprintf(str, "[00] [%02d] [--:--:--] [%02d:%02d:%02d]",
						CDTV_rec->Toc[0].LastTrack,
						CDTV_rec->Toc[0].Position.MSF.Minute,
		   			CDTV_rec->Toc[0].Position.MSF.Second,
		   			CDTV_rec->Toc[0].Position.MSF.Frame );
	}
#endif

	return(TRUE);
}

/******** CDTV_GetTrack() ********/

BOOL CDTV_GetTrack(struct CDTV_record *CDTV_rec, STRPTR str, int track)
{
	if ( track == CDTV_rec->Toc[0].LastTrack )
	{
		sprintf(str, "[%02d:%02d:%02d] [%02d:%02d:%02d]",
						CDTV_rec->Toc[track].Position.MSF.Minute,
						CDTV_rec->Toc[track].Position.MSF.Second,
						CDTV_rec->Toc[track].Position.MSF.Frame,
						CDTV_rec->Toc[0].Position.MSF.Minute,
				   	CDTV_rec->Toc[0].Position.MSF.Second,
				   	CDTV_rec->Toc[0].Position.MSF.Frame);
	}
	else
	{
		sprintf(str, "[%02d:%02d:%02d] [%02d:%02d:%02d]",
						CDTV_rec->Toc[track].Position.MSF.Minute,
						CDTV_rec->Toc[track].Position.MSF.Second,
						CDTV_rec->Toc[track].Position.MSF.Frame,
						CDTV_rec->Toc[track+1].Position.MSF.Minute,
				   	CDTV_rec->Toc[track+1].Position.MSF.Second,
				   	CDTV_rec->Toc[track+1].Position.MSF.Frame);
	}

	return(TRUE);
}

/******** CDTV_DiskWasChanged() ********/

BOOL CDTV_DiskWasChanged(struct CDTV_record *CDTV_rec)
{
BOOL ret;

	ret = DoIOR(CDTV_rec->IOReq2, CDTV_CHANGENUM, 0, 0, NULL);
	if ( !ret )
		return(FALSE);

	if ( CDTV_rec->IOReq2->io_Error==0 )
	{
		//printf("%d %d\n",CDTV_rec->diskChange,CDTV_rec->IOReq2->io_Actual);
		if ( CDTV_rec->diskChange != CDTV_rec->IOReq2->io_Actual )
			return(TRUE);	// changed
	}
	return(FALSE);
}

/******** CDTV_DiskValid() ********/

BOOL CDTV_DiskIsValid(struct CDTV_record *CDTV_rec)
{
BOOL ret;

	ret = DoIOR(CDTV_rec->IOReq2, CDTV_QUICKSTATUS, NULL, NULL, NULL);
	if ( !ret )
		return(FALSE);

	if ( !(CDTV_rec->IOReq2->io_Actual & QSF_SPIN) )
		return(FALSE);

	if ( !(CDTV_rec->IOReq2->io_Actual & QSF_DISK) )
		return(FALSE);

	//if ( CDTV_rec->IOReq2->io_Actual & QSF_ERROR )
	//	return(FALSE);

	return(TRUE);
}

/******** CDTV_DiskHasAudio() ********/

BOOL CDTV_DiskHasAudio(struct CDTV_record *CDTV_rec)
{
BOOL ret;

  ret = DoIOR(CDTV_rec->IOReq2, CD_ISROM, 0, 0, 0);
	if ( !ret )
		return(FALSE);

	if ( CDTV_rec->IOReq2->io_Actual )
		return(FALSE);

	if( CDTV_rec->Toc[0].LastTrack <= 1 )
		return(FALSE);

  if( CDTV_rec->Toc[0].LastTrack <= 2 )
		return(FALSE);

	return(TRUE);
}

/******** E O F ********/
