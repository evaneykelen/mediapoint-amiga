//	File		:	Mpeg_init.c
//	Uses		:
//	Date		:	24 sept 1994
//	Author	:	E. van Eykelen, adapted Peggy XaPP C.Lieshout
//	Desc.		:	MPEG device protocols
//

#define PRINT 0
#include "nb:pre.h"
#include <devices/cd.h>
#include <devices/peggympeg.h>
#include <proto/peggympeg.h>
#include <pragmas/peggympeg_pragmas.h>


#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"

#include "workpeg.h"
//#include "cd.h"
//#include "protos.h"


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

BOOL OpenMPEG( struct MPEG_record *MPEG_rec)
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

  if(OpenDevice("peggympeg.device", 0, (struct IORequest *)MPEG_rec->IOReq, 0))
	{
		DeleteIORequest(MPEG_rec->IOReq2);
		DeleteIORequest(MPEG_rec->IOReq);
		DeletePort(MPEG_rec->IOPort);
		printf("Couldn't open device\n");
    return(FALSE);
	}


	MPEG_rec->MPEG_Device = MPEG_rec->IOReq->iomr_Req.io_Device;

	/**** copy Req1 (which now holds device) to Req2 ****/

	CopyMem(MPEG_rec->IOReq, MPEG_rec->IOReq2, sizeof(struct IOMPEGReq));

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


/*
 *  SendIOR -- asynchronously execute a device command
 */
BOOL SendIOR( struct IOStdReq * req, LONG cmd, ULONG off, ULONG len, APTR data)
{
	req->io_Command = cmd;
	req->io_Offset = off;
	req->io_Length = len;
	req->io_Data   = data;

	SendIO( (struct IORequest *)req);

	if ( req->io_Error )
	{
		printf("SendIOR() ERROR!!! io_Error= %ld\n",req->io_Error);
		return( FALSE );
	}
	else
	{
		return( TRUE );
	}

} // SendIOR()

BOOL MPEG_InitBoard( struct MPEG_record *MPEG_rec )
{
	int ret;

	struct MPEGVideoParamsSet mvp;
	struct MPEGWindowParams wp;
	struct MPEGBorderParams bp;

	mvp.mvp_Fade = 65535;
	mvp.mvp_Fade = MPEG_rec->fade;
	mvp.mvp_DisplayType = 3;
	mvp.mvp_GenlockMode = 1;
	mvp.mvp_ColorMode = 1;

	ret = SendIOR((struct IOStdReq *)MPEG_rec->IOReq, MPEGCMD_SETVIDEOPARAMS, NULL,
								sizeof(struct MPEGVideoParamsSet), &mvp);
	if( ret )
		WaitIO( (struct IORequest *)MPEG_rec->IOReq );

	wp.mwp_XOffset = 0;
	wp.mwp_YOffset = 0;
	wp.mwp_Width = MPEG_rec->Xsize;
	wp.mwp_Height = MPEG_rec->Ysize;

	ret = SendIOR((struct IOStdReq *)MPEG_rec->IOReq, MPEGCMD_SETWINDOW, NULL,
								sizeof(struct MPEGWindowParams), &wp);
	if( ret )
		WaitIO( (struct IORequest *)MPEG_rec->IOReq );

	bp.mbp_BorderLeft = MPEG_rec->Xoff;
	bp.mbp_BorderTop = MPEG_rec->Yoff;
	bp.mbp_BorderRed = 0;
	bp.mbp_BorderGreen = 0;
	bp.mbp_BorderBlue = 0;

	ret = SendIOR((struct IOStdReq *)MPEG_rec->IOReq, MPEGCMD_SETBORDER, NULL,
								sizeof(struct MPEGBorderParams ), &bp);
	if( ret )
		WaitIO( (struct IORequest *)MPEG_rec->IOReq );

	return( TRUE );
}

struct Library *PeggyMPEGBase;

BOOL MPEG_PlayFile( struct MPEG_record *MPEG_rec )
{
	int ret;
	struct MPEGStreamInfo mi;

	PeggyMPEGBase = (struct Library *)MPEG_rec->IOReq2->iomr_Req.io_Device;

	if( !GetStreamInfo( MPEG_rec->filename, &mi,sizeof(struct MPEGStreamInfo)) )
	{
#if PRINT
		printf("Type %d\n",mi.StreamType );
		printf("Width, height is %d, %d type is %s\n",mi.Width,mi.Height,(mi.StreamType==MPEGSTREAM_VIDEO)?"Video":
																		(mi.StreamType==MPEGSTREAM_AUDIO?"Audio":"System" ) );
#endif

		MPEG_rec->IOReq->iomr_MPEGFlags = 0; //MPEGF_PAUSEATEND;
		MPEG_rec->IOReq->iomr_PTSHigh = 0;
		MPEG_rec->IOReq->iomr_PTSMid = 0;
		MPEG_rec->IOReq->iomr_PTSLow = 0;

		MPEG_rec->IOReq->iomr_StreamType = MPEGSTREAM_VIDEO;
		MPEG_rec->IOReq->iomr_Arg1 = 0;
		MPEG_rec->IOReq->iomr_Arg2 = 0;

		MPEG_rec->IOReq->iomr_Arg2 = 0;

		ret = SendIOR( (struct IOStdReq *)MPEG_rec->IOReq, MPEGCMD_PLAYFILE, NULL,strlen(MPEG_rec->filename),MPEG_rec->filename );
		if( !ret )
			return( FALSE );
	}
	else
	{
		printf("Error in getstreaminfo\n");
		return( FALSE );
	}

	return( TRUE );
}

BOOL MPEG_Stop( struct MPEG_record *MPEG_rec )
{
	int ret;
	struct MPEGVideoParamsSet mvp;

	if ( !CheckIO((struct IORequest *)MPEG_rec->IOReq ) )
	{
		AbortIO((struct IORequest *)MPEG_rec->IOReq);
		WaitIO((struct IORequest *)MPEG_rec->IOReq);
	}

	mvp.mvp_Fade = 0;
	mvp.mvp_DisplayType = 3;
	mvp.mvp_GenlockMode = 0;

	ret = SendIOR((struct IOStdReq *)MPEG_rec->IOReq2, MPEGCMD_SETVIDEOPARAMS, NULL,
								sizeof(struct MPEGVideoParamsSet), &mvp);
	if( ret )
		WaitIO( (struct IORequest *)MPEG_rec->IOReq2 );
	return( TRUE );
}

BOOL MPEG_Pause( struct MPEG_record *MPEG_rec )
{
	int ret;

	MPEG_rec->IOReq2->iomr_Arg1 = 1;

	ret = SendIOR((struct IOStdReq *)MPEG_rec->IOReq2, MPEGCMD_PAUSE, NULL,
								0, 0 );
	if( ret )
		WaitIO( (struct IORequest *)MPEG_rec->IOReq2 );
	return( TRUE );
}

BOOL MPEG_UnPause( struct MPEG_record *MPEG_rec )
{
	int ret;

	MPEG_rec->IOReq2->iomr_Arg1 = 0;

	ret = SendIOR((struct IOStdReq *)MPEG_rec->IOReq2, MPEGCMD_PAUSE, NULL,
								0, 0 );
	if( ret )
		WaitIO( (struct IORequest *)MPEG_rec->IOReq2 );
	return( TRUE );
}

void play_mpeg( struct MPEG_record *MPEG_rec )
{
	long isig,wsig,psig;
	int stop = 0;
	struct	Task	*mt;

	mt = FindTask( NULL );
	MPEG_rec->restoresignal = 0;

	if( OpenMPEG( MPEG_rec ) )
	{
		MPEG_rec->Xoff = 0;
		MPEG_rec->Yoff = 0;
		MPEG_rec->fade = 65000;
		MPEG_rec->Xsize = 352*2;
		MPEG_rec->Ysize = 240;
		MPEG_InitBoard( MPEG_rec );
		if( MPEG_PlayFile( MPEG_rec ) )
		{
			psig = 1L << MPEG_rec->IOPort->mp_SigBit;

			wsig = MPEG_rec->mainsignal | MPEG_rec->quitsig | psig;

			while( !stop )
			{
				isig = Wait( wsig );
				if( isig & MPEG_rec->quitsig )
				{
					stop = 1;
					break;
				}
				else
				{

					if( isig & MPEG_rec->sig_ptoc )
					{
						struct Node *td;
						struct Node *node;
						Forbid();
						node = ( struct Node * )&MPEG_rec->mport_ptoc->mp_MsgList;
						while( node->ln_Pred ) node = node->ln_Pred;

						for( td = node; td->ln_Succ; td=td->ln_Succ)
						{
							if (	((PROCDIALOGUE * )td)->pd_Cmd == DCC_DOTERM ||
										((PROCDIALOGUE * )td)->pd_Cmd == DCC_DORUN )		// re-run
								break;
				
							if( MPEG_rec->action == 0 )
								if ( ((PROCDIALOGUE * )td)->pd_Cmd == DCC_DOSTOP )
									break;
						}
						Permit();

						if (	((PROCDIALOGUE * )td)->pd_Cmd == DCC_DOTERM  ||
									((PROCDIALOGUE * )td)->pd_Cmd == DCC_DORUN ||
									( (MPEG_rec->action == 0 ) && ((PROCDIALOGUE * )td)->pd_Cmd == DCC_DOSTOP ) )
						{
							Signal( mt, isig );			// restore signals
						}
						else
							MPEG_rec->restoresignal = MPEG_rec->sig_ptoc;
					}

					if( isig & psig )				// MPEG movie finished ?
					{
						if ( CheckIO((struct IORequest *)MPEG_rec->IOReq ) )
						{
//							WaitIO((struct IORequest *)MPEG_rec->IOReq);
//							MPEG_PlayFile( MPEG_rec );
							stop = 1;
							break;
						}
					}
				}
			} 
		}

		if( MPEG_rec->restoresignal )
			Signal( mt, MPEG_rec->restoresignal );

		MPEG_Stop( MPEG_rec );
		CloseMPEG( MPEG_rec );
	}
}

/******** E O F ********/
