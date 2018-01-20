#include "nb:pre.h"
#include <devices/cd.h>
#include <devices/mpeg.h>
#include "cd.h"
#include "mpeg.h"
#include "protos.h"

/******** AllocMPEG() ********/

struct MPEG_record *AllocMPEG(void)
{
	return( (struct MPEG_record *)
					AllocMem(sizeof(struct MPEG_record),MEMF_CLEAR|MEMF_ANY) ); 
}

/******** FreeMPEG() ********/

void FreeMPEG(struct MPEG_record *MPEG_rec)
{
	if ( MPEG_rec )
		FreeMem(MPEG_rec, sizeof(struct MPEG_record)); 
}

/******** OpenMPEG() ********/

BOOL OpenMPEG(struct CD_record *CD_rec, struct MPEG_record *MPEG_rec)
{
	MPEG_rec->IOReq  = NULL;
	MPEG_rec->IOReq2 = NULL;
	MPEG_rec->IOPort = NULL;
	MPEG_rec->MPEG_Device = NULL;

	MPEG_rec->IOPort = CreatePort(0,0);
	if (MPEG_rec->IOPort==NULL)
		return(FALSE);

	MPEG_rec->IOReq = CreateIORequest(MPEG_rec->IOPort,sizeof(struct IOMPEGReq));
	if (MPEG_rec->IOReq==NULL)
	{
		DeletePort(MPEG_rec->IOPort);
		return(FALSE);
	}

	MPEG_rec->IOReq2 = CreateIORequest(MPEG_rec->IOPort,sizeof(struct IOMPEGReq));
	if (MPEG_rec->IOReq2==NULL)
	{
		DeleteIORequest(MPEG_rec->IOReq);
		DeletePort(MPEG_rec->IOPort);
		return(FALSE);
	}

  if(OpenDevice("cd32mpeg.device", 0, (struct IORequest *)MPEG_rec->IOReq, 0))
	{
		DeleteIORequest(MPEG_rec->IOReq2);
		DeleteIORequest(MPEG_rec->IOReq);
		DeletePort(MPEG_rec->IOPort);
    return(FALSE);
	}

	MPEG_rec->MPEG_Device = MPEG_rec->IOReq->iomr_Req.io_Device;

	/**** copy Req1 (which now holds device) to Req2 ****/

	CopyMem(MPEG_rec->IOReq, MPEG_rec->IOReq2, sizeof(struct IOMPEGReq));

	/**** Read CD table of contents ****/

	CD_NewCD(CD_rec);

	return(TRUE);
}

/******** CloseMPEG() ********/

void CloseMPEG(struct MPEG_record *MPEG_rec)
{
  if(MPEG_rec->IOReq && MPEG_rec->MPEG_Device)
		CloseDevice((struct IORequest *)MPEG_rec->IOReq);

	if(MPEG_rec->IOPort)
		while(GetMsg(MPEG_rec->IOPort));

  if(MPEG_rec->IOReq)
		DeleteIORequest(MPEG_rec->IOReq);

  if(MPEG_rec->IOReq2)
		DeleteIORequest(MPEG_rec->IOReq2);

  if(MPEG_rec->IOPort)
		DeletePort(MPEG_rec->IOPort);

	MPEG_rec->IOReq  = NULL;
	MPEG_rec->IOReq2 = NULL;
	MPEG_rec->IOPort = NULL;
	MPEG_rec->MPEG_Device = NULL;
}

/******** MPEG_PlayTrack() ********/

BOOL MPEG_PlayTrack(struct CD_record *CD_rec, struct MPEG_record *MPEG_rec,
										int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3)
{
BOOL ret;
struct MPEGVideoParamsSet mvp;
ULONG begin;

	if ( !CheckIO((struct IORequest *)MPEG_rec->IOReq) )
	{
	  AbortIO((struct IORequest *)MPEG_rec->IOReq);
  	WaitIO((struct IORequest *)MPEG_rec->IOReq);
	}

	// CHECK VALUES

	if ( i1 < CD_rec->Toc[0].Summary.FirstTrack )
		i1 = CD_rec->Toc[0].Summary.FirstTrack;
	if ( i1 > CD_rec->Toc[0].Summary.LastTrack )
		i1 = CD_rec->Toc[0].Summary.LastTrack;

	// CHECK FOR AUDIO OR MPEG

	if ( !(CD_rec->Toc[ i1 ].Entry.CtlAdr & CTL_DATA) )
	{
		ret = CD_PlayTrack(CD_rec, i1, i2, i3, s1, s2, s3);	// DO AUDIO, NO MPEG!
		return( ret );
	}

	ret = CD_Stop(CD_rec,i1,i2,i3,s1,s2,s3);
	ret = MPEG_Stop(CD_rec,MPEG_rec,i1,i2,i3,s1,s2,s3);

	if ( ret )
	{
		// SET VIDEO PARAMS

		mvp.mvp_Fade = 65535;
		mvp.mvp_DisplayType = 0;

		ret = SendIOR((struct IOStdReq *)MPEG_rec->IOReq, MPEGCMD_SETVIDEOPARAMS, NULL,
									sizeof(struct MPEGVideoParamsSet), &mvp);
		if ( ret )
		{
			WaitIO( (struct IORequest *)MPEG_rec->IOReq );

			if ( CD_DiskWasChanged(CD_rec) )
				CD_NewCD(CD_rec);

			// SET STREAM TYPE

			begin = CD_rec->Toc[ i1 ].Entry.Position.LSN;
			if ( begin < 5325 )	// 01:11:00 magic!
				begin = 5325;

			MPEG_rec->IOReq->iomr_MPEGFlags = 0;
			MPEG_rec->IOReq->iomr_StreamType = MPEGSTREAM_SYSTEM;
			MPEG_rec->IOReq->iomr_Arg1 = 2328;
			MPEG_rec->IOReq->iomr_Arg2 = begin;

			// PLAY MPEG

			ret = SendIOR((struct IOStdReq *)MPEG_rec->IOReq, MPEGCMD_PLAYLSN,
										begin,
										CD_rec->Toc[ i1+1 ].Entry.Position.LSN - begin,
										NULL);
		}
	}

	return(TRUE);
}

/******** MPEG_PlayTrackFromTo() ********/

BOOL MPEG_PlayTrackFromTo(struct CD_record *CD_rec, struct MPEG_record *MPEG_rec,
													int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3)
{
ULONG start, end;
int swap;
BOOL ret;
struct MPEGVideoParamsSet mvp;
ULONG begin;

	if ( !CheckIO((struct IORequest *)MPEG_rec->IOReq) )
	{
	  AbortIO((struct IORequest *)MPEG_rec->IOReq);
  	WaitIO((struct IORequest *)MPEG_rec->IOReq);
	}

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

	// CHECK FOR AUDIO OR MPEG

	if ( !(CD_rec->Toc[ i1 ].Entry.CtlAdr & CTL_DATA) )
	{
		ret = CD_PlayTrackFromTo(CD_rec, i1, i2, i3, s1, s2, s3);	// DO AUDIO, NO MPEG!
		return( ret );
	}

	ret = CD_Stop(CD_rec,i1,i2,i3,s1,s2,s3);
	ret = MPEG_Stop(CD_rec,MPEG_rec,i1,i2,i3,s1,s2,s3);

	if ( ret )
	{
		// SET VIDEO PARAMS

		mvp.mvp_Fade = 65535;
		mvp.mvp_DisplayType = 0;

		ret = SendIOR((struct IOStdReq *)MPEG_rec->IOReq, MPEGCMD_SETVIDEOPARAMS, NULL,
									sizeof(struct MPEGVideoParamsSet), &mvp);
		if ( ret )
		{
			WaitIO( (struct IORequest *)MPEG_rec->IOReq );

			if ( CD_DiskWasChanged(CD_rec) )
				CD_NewCD(CD_rec);

			// SET STREAM TYPE

			begin = start;
			if ( begin < 5325 )	// 01:11:00 magic!
				begin = 5325;

			MPEG_rec->IOReq->iomr_MPEGFlags = 0;
			MPEG_rec->IOReq->iomr_StreamType = MPEGSTREAM_SYSTEM;
			MPEG_rec->IOReq->iomr_Arg1 = 2328;
			MPEG_rec->IOReq->iomr_Arg2 = begin;

			// PLAY MPEG
	
			ret = SendIOR((struct IOStdReq *)MPEG_rec->IOReq, MPEGCMD_PLAYLSN,
										begin, end-begin, NULL);
		}
	}

	return(TRUE);
}

/******** MPEG_PlayTrackStartEnd() ********/

BOOL MPEG_PlayTrackStartEnd(struct CD_record *CD_rec, struct MPEG_record *MPEG_rec,
														int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3)
{
ULONG start, end, cd_start, cd_end, swap;
int mm, ss, ff, track;
BOOL ret;
struct MPEGVideoParamsSet mvp;
ULONG begin;

	if ( !CheckIO((struct IORequest *)MPEG_rec->IOReq) )
	{
	  AbortIO((struct IORequest *)MPEG_rec->IOReq);
  	WaitIO((struct IORequest *)MPEG_rec->IOReq);
	}

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

	// CHECK FOR AUDIO OR MPEG

	track = LSN2Track(CD_rec,start);

	if ( !(CD_rec->Toc[ track ].Entry.CtlAdr & CTL_DATA) )
	{
		ret = CD_PlayTrack(CD_rec, i1, i2, i3, s1, s2, s3);	// DO AUDIO, NO MPEG!
		return( ret );
	}

	ret = CD_Stop(CD_rec,i1,i2,i3,s1,s2,s3);
	ret = MPEG_Stop(CD_rec,MPEG_rec,i1,i2,i3,s1,s2,s3);

	if ( ret )
	{
		// SET VIDEO PARAMS

		mvp.mvp_Fade = 65535;
		mvp.mvp_DisplayType = 0;

		ret = SendIOR((struct IOStdReq *)MPEG_rec->IOReq, MPEGCMD_SETVIDEOPARAMS, NULL,
									sizeof(struct MPEGVideoParamsSet), &mvp);
		if ( ret )
		{
			WaitIO( (struct IORequest *)MPEG_rec->IOReq );

			if ( CD_DiskWasChanged(CD_rec) )
				CD_NewCD(CD_rec);

			// SET STREAM TYPE

			begin = start;
			if ( begin < 5325 )	// 01:11:00 magic!
				begin = 5325;

			MPEG_rec->IOReq->iomr_MPEGFlags = 0;
			MPEG_rec->IOReq->iomr_StreamType = MPEGSTREAM_SYSTEM;
			MPEG_rec->IOReq->iomr_Arg1 = 2328;
			MPEG_rec->IOReq->iomr_Arg2 = begin;

			// PLAY MPEG
	
			ret = SendIOR((struct IOStdReq *)MPEG_rec->IOReq, MPEGCMD_PLAYLSN,
										begin, end-begin, NULL);
		}
	}

	return(TRUE);
}

/******** MPEG_Pause() ********/

BOOL MPEG_Pause(struct CD_record *CD_rec, struct MPEG_record *MPEG_rec,
								int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3)
{
STATIC int paused = 0;

	paused = !paused;
	MPEG_rec->IOReq2->iomr_Arg1 = paused;
	MPEG_rec->IOReq2->iomr_Arg2 = NULL;

	if ( SendIOR((struct IOStdReq *)MPEG_rec->IOReq2, MPEGCMD_PAUSE, NULL, NULL, NULL) )
		WaitIO((struct IORequest *)MPEG_rec->IOReq2);

	if ( SendIOR((struct IOStdReq *)CD_rec->IOReq2, CD_PAUSE, 0, paused, NULL) )
		WaitIO((struct IORequest *)CD_rec->IOReq2);

	return(TRUE);
}

/******** MPEG_Stop() ********/

BOOL MPEG_Stop(	struct CD_record *CD_rec, struct MPEG_record *MPEG_rec,
								int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3)
{
	if ( !CheckIO((struct IORequest *)MPEG_rec->IOReq) )
	{
	  AbortIO((struct IORequest *)MPEG_rec->IOReq);
  	WaitIO((struct IORequest *)MPEG_rec->IOReq);
	}

	return(TRUE);
}

/******** MPEG_SingleStep() ********/

BOOL MPEG_SingleStep(	struct CD_record *CD_rec, struct MPEG_record *MPEG_rec,
											int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3)
{
	if ( SendIOR((struct IOStdReq *)MPEG_rec->IOReq2, MPEGCMD_SINGLESTEP, NULL, NULL, NULL) )
		WaitIO((struct IORequest *)MPEG_rec->IOReq2);

	return(TRUE);
}

/******** MPEG_SlowMotion() ********/

BOOL MPEG_SlowMotion(	struct CD_record *CD_rec, struct MPEG_record *MPEG_rec,
											int i1,int i2,int i3, STRPTR s1,STRPTR s2,STRPTR s3)
{
	MPEG_rec->IOReq2->iomr_Arg1 = i1;

	if ( SendIOR((struct IOStdReq *)MPEG_rec->IOReq2, MPEGCMD_SLOWMOTION, NULL, NULL, NULL) )
		WaitIO((struct IORequest *)MPEG_rec->IOReq2);

	return(TRUE);
}

/******** LSN2Track() ********/

int LSN2Track(struct CD_record *CD_rec, ULONG start)
{
int i;

	for(i=1; i<=CD_rec->Toc[0].Summary.LastTrack; i++)
		if ( start >= CD_rec->Toc[i].Entry.Position.LSN )
			return(i);

	return(0);
}

/******** E O F ********/
