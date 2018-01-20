//	File		:	smfnew.c
//	Uses		:	midi_err.h
//	Date		:	25-04-93
//	Author	:	Dan Baker, Commodore Business Machines
//					( adapted for MP ing. C. Lieshout )
//	Desc.		:	Read and play a MIDI file with its own timing and serials
//

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>
#include <workbench/startup.h>
#include	<proto/dos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include <devices/serial.h>
#include <devices/timer.h>

#include <stdlib.h>
#include <stdio.h>

#define _PRINT FALSE
#define _XAPP TRUE

#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "midi_err.h"
#include "midi_play.h"

#define MakeID(a,b,c,d)  ( (LONG)(a)<<24L | (LONG)(b)<<16L | (c)<<8 | (d) )
#define MThd MakeID('M','T','h','d')

void PlayMidi( SMFInfo *smfi, UBYTE *buffer, LONG length );

struct note
{
	UBYTE off;
	UBYTE num;
	UBYTE vel;
};

#if _XAPP!=TRUE
int CXBRK(void) { return(0); }     /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */

//
// Remember to clear to smfi struct before using it
// Only test a midi file here
//
void main(int argc, char **argv)
{
	SMFInfo		smfi;
	int 			read_err;

	if( argc > 1 )
	{

		init_smfi( &smfi );
		if( ( read_err = read_and_evaluate( argv[1], &smfi ) ) == 0 )
		{
			play_midi( &smfi );
			printf("exiting\n");
			release( &smfi ,0);
			printf("released\n");
		}
		else
		{
			printf("Error : %s\n",errors[ read_err ] );
		}

	}
	printf("exit\n");
}
#endif

//===============================================
//	Name		:	switch_off_notes
//	Function	:	Send a all notes off to Midi
//	Inputs	:	pointer to smfi struct
//	Result	:
//	Updated	:	28-07-1993
//
void switch_off_notes( SMFInfo *smfi )
{
	struct note notes[ 16 ];
	int i;

#if _PRINT
	printf("Switch off notes\n");
#endif
	for( i = 0; i < 16; i++ )
	{
		notes[i].off = 0xb0 + i;
		notes[i].num = 0x7b;
		notes[i].vel = 0;
	}

	PlayMidi( smfi,(UBYTE *)notes, 48 );

}

//===============================================
//	Name		:	init_smfi
//	Function	:	Clear the smfi struct
//	Inputs	:	pointer to smfi struct
//	Result	:	a zero smfi struct
//	Updated	:	08-05-93
//
void init_smfi( SMFInfo *smfi )
{
	smfi->release = 0;
	smfi->pData			= NULL;
	smfi->donecount	= 0L;
	smfi->lastRSchan	= 0xf1; /* Status of $F1 is undefined in Standard MIDI file Spec */
	smfi->fillbuf1 = NULL;
	smfi->fillbuf2 = NULL;
	smfi->ser_reply_port = NULL;
	smfi->ser_request = NULL;
	smfi->timer_reply_port = NULL;
	smfi->timer_request = NULL;
	smfi->serdev = 0;
	smfi->timdev = 0;
	smfi->tempo = 500000;			// default 120 bpm
	smfi->quitsig = 0;
}

//===============================================
//	Name		:	read_and_evaluate
//	Function	:	read the midi file and interpret the data
//	Inputs	:	filename and pointer to the SMFInfo struct
//	Result	:	Zero or error code
//	Updated	:	25-04-93
//
int read_and_evaluate( char *smfname, SMFInfo *smfi )
{
	int		rdcount;
	char		iobuffer[14];
	struct	SMFHeader *pSMFHeader;
	long		y,z;

	smfi->release = 1;
	smfi->restoresignal = 0;

	smfi->smfhandle= Open( (UBYTE *)smfname , MODE_OLDFILE );
	if( smfi->smfhandle == 0 )
		return( CANNOT_OPEN_ERR );

   /*-------------------------------*/
   /* Read the SMF Header ID/Length */
   /*-------------------------------*/

	rdcount = Read( smfi->smfhandle, iobuffer, 14 );
	if( rdcount == -1 || rdcount < 14 )
		return( OBAD_FILE_ERR );

   /*-----------------*/
   /* Evaluate Header */
   /*-----------------*/

	pSMFHeader = (struct SMFHeader *)iobuffer;
	if( pSMFHeader->ChunkID != MThd || pSMFHeader->VarLeng != 6 )
		return( UNKNOWN_HEADER_ERR );

	if ( pSMFHeader->Format != 0 && pSMFHeader->Format != 1 )
		return( CANT_PARSE_ERR );
#if _PRINT
	printf("Format is %d\n", pSMFHeader->Format );
#endif

//	if(pSMFHeader->Ntrks > MAXTRAX )
//		return( TOO_MANY_TRAX_ERR );

#if _PRINT
	printf("Nr. of tracks is %d\n", pSMFHeader->Ntrks );
#endif

	smfi->Ntrks = pSMFHeader->Ntrks;

	/*--------------------*/
	/* Evaluate time base */
	/*--------------------*/

#if _PRINT
	printf("Division is %d\n",pSMFHeader->Division );
#endif
	if (pSMFHeader->Division < 0)
		return( WRONG_TIME_BASE_ERR );
	else
	{
		smfi->Division = (ULONG)pSMFHeader->Division;

		/* According to "Standrd MIDI Files 1.0", July 1988, page 5 */
		/* para. 4: "...time signature is assumed to be 4/4 and the */
		/*           tempo 120 beats per minute."                   */

		smfi->tfactor = smfi->tempo / smfi->Division;
	}

#if _PRINT
	printf("Tfactor is %d\n",smfi->tfactor );
#endif

	/*--------------------------------------*/
	/* Calculate size and Read rest of file */
	/*--------------------------------------*/
	y = Seek( smfi->smfhandle, 0L, OFFSET_END );
	if(y==-1)
		return( BAD_FILE_ERR );
	z = Seek( smfi->smfhandle, y, OFFSET_BEGINNING );
	if(z==-1)
		return( BAD_FILE_ERR );

	smfi->smfdatasize = z - y;

	smfi->smfdata = AllocMem( smfi->smfdatasize, MEMF_PUBLIC | MEMF_CLEAR );
	if( smfi->smfdata == NULL )
		return( NO_MEM_ERR );

	rdcount = Read( smfi->smfhandle, smfi->smfdata, smfi->smfdatasize );
	if( rdcount == -1 || rdcount < smfi->smfdatasize )
		return( BAD_FILE_ERR );
	Close( smfi->smfhandle );
	smfi->smfhandle = NULL;
	smfi->fillbuf1 = (UBYTE *)AllocMem( MIDIBUFSIZE, MEMF_FAST|MEMF_CLEAR );
	smfi->fillbuf2 = (UBYTE *)AllocMem( MIDIBUFSIZE, MEMF_FAST|MEMF_CLEAR );

	if( smfi->fillbuf1 == NULL || smfi->fillbuf2 == NULL )
		return( NO_MEM_ERR );

	if( open_req_ports( smfi ) != 0 )
		return( NO_PORTS_ERR );

	return( FILE_OKE );
}

//===============================================
//	Name		:	open_req_ports
//	Function	:	open the needed ports and request structs
//	Inputs	:	pointer to smfi struct
//	Result	:	zero or error
//	Updated	:	30-04-1994
//
int open_req_ports( SMFInfo *smfi )
{
	if( (smfi->ser_reply_port = CreatePort(SERIALNAME,0) ) == NULL )
		return 1;
	if( ( smfi->ser_request = (struct IOExtSer *)
						CreateExtIO(smfi->ser_reply_port,sizeof(struct IOExtSer)))==NULL)
		return 1;

	smfi->ser_request->io_SerFlags =SERF_XDISABLED|SERF_SHARED;

	if((OpenDevice(SERIALNAME,0,
		(struct IORequest *)smfi->ser_request,0)) != NULL )
		return 1;

	smfi->serdev = 1;	

	set_baud( smfi );

	if( (smfi->timer_reply_port = CreatePort(TIMERNAME,0)) == NULL )
		return 1;

	if( ( smfi->timer_request=(struct timerequest *) 
        CreateExtIO(smfi->timer_reply_port,sizeof(struct timerequest)))==NULL)
		return 1;

	if((OpenDevice(TIMERNAME,UNIT_MICROHZ,
		(struct IORequest *)smfi->timer_request,0))!=NULL)
		return 1;

	smfi->timdev = 1;

	smfi->timer_request->tr_node.io_Command=TR_ADDREQUEST;

	return 0;
}

//===============================================
//	Name		:	set_baud
//	Function	:	set the baud rate to midi rate
//	Inputs	:	pointer to smfi struct
//	Result	:	none
//	Updated	:	30-04-1994
//
void set_baud( SMFInfo *smfi )
{
	smfi->ser_request->io_RBufLen = 64;
	smfi->ser_request->io_Baud = 31250;
	smfi->ser_request->io_ReadLen = 8;
	smfi->ser_request->io_WriteLen = 8;
	smfi->ser_request->io_StopBits = 1;
	smfi->ser_request->io_SerFlags |= SERF_RAD_BOOGIE;
	smfi->ser_request->io_SerFlags |= SERF_XDISABLED;
	smfi->ser_request->io_SerFlags &= ~SERF_7WIRE;
	smfi->ser_request->io_SerFlags &= ~SERF_PARTY_ON;	   
	smfi->ser_request->IOSer.io_Command = SDCMD_SETPARAMS; 
	DoIO((struct IORequest *)smfi->ser_request);
}

//===============================================
//	Name		:	set_timer
//	Function	:	set the timer to wait time secs
//	Inputs	:	pointer to smfi struct and delta
//	Result	:	none
//	Updated	:	30-04-1994
//
void set_timer( SMFInfo *smfi, ULONG time )
{
#if _PRINT
	printf("Set on %ld\n",time );
#endif
	smfi->timer_request->tr_time.tv_secs = time/1000000;
	smfi->timer_request->tr_time.tv_micro = time%1000000;

//	printf("Secs, mics %ld,%ld\n",smfi->timer_request->tr_time.tv_secs,
//										  smfi->timer_request->tr_time.tv_micro);

	SendIO( (struct IORequest *)smfi->timer_request );
}


void release( SMFInfo *smfi, int err )
{

	if( !CheckIO( (struct IORequest *)smfi->ser_request ) )
	{
		WaitIO( (struct IORequest *)smfi->ser_request );
	}

	if( !CheckIO( (struct IORequest *)smfi->timer_request ) )
	{
		WaitIO( (struct IORequest *)smfi->timer_request );
	}

	if( smfi->timdev == 1)
		CloseDevice((struct IORequest *)smfi->timer_request);

	if( smfi->serdev == 1)
		CloseDevice((struct IORequest *)smfi->ser_request);

	if( smfi->timer_request )
		DeleteExtIO((struct IORequest *)smfi->timer_request);

	if( smfi->timer_reply_port )
		DeletePort( smfi->timer_reply_port );

	if( smfi->ser_request )
		DeleteExtIO((struct IORequest *)smfi->ser_request);

	if( smfi->ser_reply_port )
		DeletePort( smfi->ser_reply_port );

	smfi->release = 0;
	if( smfi->fillbuf1 != NULL )
		FreeMem( smfi->fillbuf1, MIDIBUFSIZE );

	if( smfi->fillbuf2 != NULL )
		FreeMem( smfi->fillbuf2, MIDIBUFSIZE );

	if( smfi->smfhandle != NULL )
		Close( smfi->smfhandle );
	smfi->smfhandle = NULL;
	if( smfi->smfdata != NULL )
		FreeMem( smfi->smfdata, smfi->smfdatasize );
	if( smfi->pData )
		FreeMem( smfi->pData, smfi->sizeDTrack );
}

void PlayMidi( SMFInfo *smfi, UBYTE *buffer, LONG length )
{
//	if( length > 20 )
//		printf("length is %ld\n",length );

	smfi->ser_request->IOSer.io_Data=(APTR)buffer;
	smfi->ser_request->IOSer.io_Length=length;
	smfi->ser_request->IOSer.io_Command=CMD_WRITE; 
	DoIO((struct IORequest *)smfi->ser_request);
}

//===============================================
//	Name		:	play_midi
//	Function	:	play a midi file pointed at by smfi
//	Inputs	:	pointer to smfi structure
//	Result	:	play a midi file
//	Updated	:	29-04-93
//
int play_midi( SMFInfo *smfi )
{
	int return_value = 0;

	UBYTE	*pbyte,x;
	LONG	y,z;
	BOOL	notdone;
	ULONG	masterswitch,lowclock,
			ylength[2],wakeup,wsigs;

	UBYTE	*ptrack[TWOxMAXTRAX],trackct;
	LONG	fillclock[2];
	ULONG	oldclock;

	BYTE		oldpri;			/* Priority to restore */
	struct	Task	*mt;		/* Pointer to this task */

	struct DecTrack *pDTrack[MAXTRAX];

	smfi->release = 1;

	oldclock			= 0L;
	mt					= NULL;
	notdone			= TRUE;
	fillclock[0]	= 0L;
	fillclock[1]	= 0L;

	trackct=0;
	pbyte=smfi->smfdata;
	smfi->pfillbuf[0]	= smfi->fillbuf1;
	smfi->pfillbuf[1]	= smfi->fillbuf2;

// This could be done with an faster search algoritme
// For example with the Boyer Moore with a pre define jump string
//	printf("size is %d\n",smfi->smfdatasize );
//	printf("base is %x\n",smfi->smfdata );

#if _PRINT
	printf("Zoek MTrk headers\n");
#endif
   while((pbyte-smfi->smfdata < smfi->smfdatasize) && (trackct < MAXTRAX))
   {
      if((*pbyte=='M')&&(*(pbyte+1)=='T')&&
         (*(pbyte+2)=='r')&&(*(pbyte+3)=='k'))
      {
         /* Beginning of track */
         ptrack[trackct]=pbyte+8;
         /* End of track marker */
         ptrack[MAXTRAX+trackct-1]=pbyte;
         trackct++;
         pbyte+=4;
      }
      else pbyte++;
   }

   /* End of track marker */   
   ptrack[MAXTRAX+trackct-1]=pbyte;

	return_value = MISSING_TRACKS_ERR;

	if(trackct != smfi->Ntrks)
		goto error;

	/*----------------------------------------*/
	/* Set up DTrack structure for each track */
	/*----------------------------------------*/

	smfi->sizeDTrack = trackct * sizeof( struct DecTrack );
	smfi->pData = AllocMem( smfi->sizeDTrack, MEMF_PUBLIC | MEMF_CLEAR );

	return_value = NO_MEM_WORK_ERR;

	if( !smfi->pData )
		goto error;

   for( x = 0; x < trackct; x++ )
   {
      pDTrack[ x ] = ( struct DecTrack *)
							( x * sizeof( struct DecTrack ) + smfi->pData );
      pDTrack[ x ]->endmarker = ptrack[ MAXTRAX + x ];	/* add end marker */
   }

	/*------------------------------------------------*/  
	/* Get events from track into DecTrack structures */
	/*------------------------------------------------*/
	y = 0;
	masterswitch = 0L;
	lowclock = 0xffffffff;

   /* Initialize DecTrack structures */

   for( x = 0; x < trackct; x++ )
   {  
		/* Takes a pointer to the delta of a raw <MTrk event>, a pointer   */
		/* to a DecTrack decoding structure to store the decoded event and */ 
		/* a switch that tells which of the two buffers to use.  Returns a */
		/* pointer to the next raw <MTrk event> in the track or 0 if the   */
		/* track is exhausted.                                             */

		ptrack[ x ] = DecodeEvent( ptrack[x] , pDTrack[x] , masterswitch, smfi );

		if( pDTrack[ x ]->nexclock < lowclock && ptrack[ x ])
			/* Find the first event */
			lowclock=pDTrack[ x ]->nexclock;
   }

	/*-----------------------------------*/
	/* Transfer first events to A buffer */
	/*-----------------------------------*/

	for( x = 0; x < trackct; x++ )
	{
		if( ( pDTrack[ x ]->nexclock == lowclock ) && ptrack[ x ] )
		{
			// Transfer event to parse buffer and handle successor
			y = transfer( pDTrack[ x ], masterswitch, y, smfi );
			z = 1;
			while( z == 1 )
			{
				ptrack[ x ] = DecodeEvent( ptrack[x],pDTrack[x],masterswitch,smfi);

 	 			// Next delta is zero...

				if( !( pDTrack[ x ]->absdelta ) && ptrack[ x ] )
				{
					y = transfer( pDTrack[x], masterswitch, y, smfi );
				}
				else
				{
					z = 0;
				}
			}
		}
	}

	ylength[ masterswitch ] = y;
//   fillclock[ masterswitch ] = (LONG)( smfi->tfactor * lowclock );
   fillclock[ masterswitch ] = (LONG)( lowclock );

	smfi->deltatime = fillclock[ masterswitch];

	/*------------------------------------*/
	/* Transfer second events to B buffer */
	/*------------------------------------*/
	y=0;
	masterswitch = 1L;
	lowclock = 0xffffffff;

	for( x = 0; x < trackct; x++ )
	{
		if( pDTrack[ x ]->nexclock < lowclock && ptrack[ x ])
			lowclock=pDTrack[ x ]->nexclock;
	}

	for( x = 0;x < trackct; x++ )
	{
		if( pDTrack[ x ]->nexclock == lowclock && ptrack[ x ] )
		{
			// Transfer event to parse buffer and handle successor

			y = transfer( pDTrack[ x ], masterswitch, y, smfi );
			z = 1;
			while( z == 1 )
			{
				ptrack[ x ] = DecodeEvent(ptrack[x],pDTrack[x],masterswitch,smfi);
				// Next delta is zero...

				if( !( pDTrack[ x ]->absdelta ) && ptrack[ x ] )
				{
					y = transfer( pDTrack[ x ], masterswitch, y, smfi );
//					printf("%d, ",y );
				}
				else
				{
					z=0;
				}
			}
		}
	}

	ylength[ masterswitch ] = y;

//	fillclock[ masterswitch ] = (LONG)( smfi->tfactor * lowclock );
	fillclock[ masterswitch ] = (LONG)( lowclock );

//	printf("Init first buffers done\n");
//	printf("Ylength 1 is %d\n",ylength[ 0 ] );
//	printf("Ylength 2 is %d\n",ylength[ 1 ] );

//	printf("fillclock 1 is %d\n",fillclock[ 0 ] );
//	printf("fillclock 2 is %d\n",fillclock[ 1 ] );
//	printf("Would send\n");
/*	{
		int i;
		for( i = 0 ; i < ylength[ 0 ]; i++ )
			printf(" %2x,", *( smfi->pfillbuf[ 0 ] + i ) );
		printf("\n");
		for( i = 0 ; i < ylength[ 1 ]; i++ )
			printf(" %2x,", *( smfi->pfillbuf[ 1 ] + i ) );
		printf("\n");
	}
*/

	/*-----------------------------------------------------*/
	/* Priority Must Be Above Intuition and Graphics Stuff */
	/*-----------------------------------------------------*/ 

	mt = FindTask( NULL );
	oldpri = SetTaskPri( mt, 21 );

	/*-------------------------------------------*/
	/* Play the first batch of notes in Buffer A */
	/*-----------------	--------------------------*/

	if( ylength[ masterswitch^1 ] != 0 )
		PlayMidi ( smfi,smfi->pfillbuf[masterswitch^1],ylength[masterswitch^1] );


	/*-------------------------------------------------------------------*/
	/* and start the clock with clock value in fillclock[ masterswitch ];
	/*-------------------------------------------------------------------*/

	set_timer( smfi, fillclock[ masterswitch ] - smfi->deltatime);

	smfi->deltatime = fillclock[ masterswitch ];

#if _XAPP
	wsigs = 1L<<smfi->timer_reply_port->mp_SigBit | smfi->mainsignal | smfi->quitsig;
#else
	wsigs = SIGBREAKF_CTRL_C | 1L<<smfi->timer_reply_port->mp_SigBit | smfi->mainsignal | smfi->quitsig;
#endif


	/*-----------------*/
	/* MAIN EVENT LOOP */
	/*-----------------*/   
	while( smfi->donecount < trackct )
	{
		masterswitch ^= 1L;
		y = 0;
		lowclock = 0xffffffff;

		/*------------------------------------------------*/  
		/* Get events from track into DecTrack structures */
		/*------------------------------------------------*/
		for( x = 0; x <trackct; x++ )
		{
			if( ( pDTrack[ x ]->nexclock < lowclock ) && ptrack[ x ] )
				lowclock = pDTrack[ x ]->nexclock;
		}

		/*-----------------------------------*/
		/* Transfer events to current buffer */
		/*-----------------------------------*/
		for( x= 0; x < trackct; x++ )
		{
			if( ( pDTrack[ x ]->nexclock == lowclock ) && ptrack[ x ] )
			{
				// Transfer event to parse buffer and handle successor
				y = transfer( pDTrack[x], masterswitch, y, smfi );
				z = 1;
				while( z==1 )
				{
					ptrack[x] = DecodeEvent(ptrack[x],pDTrack[x],masterswitch,smfi);

//					printf("edec,");

					/* Next delta is zero... */
					if( !(pDTrack[x]->absdelta) && ptrack[x] )
					{
						y = transfer(pDTrack[x],masterswitch,y,smfi);
					}
					else
					{
						z = 0;
					}
            }
         }
      }

		ylength[ masterswitch ] = y;
//      fillclock[ masterswitch ] = (LONG)( smfi->tfactor * lowclock );
      fillclock[ masterswitch ] = (LONG)( lowclock );

		/*---------------------------------------------------------------*/
		/* Wait() for the CAMD alarm or a CTRL-C keypress from the user. */
		/*---------------------------------------------------------------*/

		wakeup = Wait( wsigs );

#if _XAPP==FALSE
		if( wakeup & SIGBREAKF_CTRL_C )
			trackct = 0;
#endif

		if( wakeup & smfi->quitsig )		// signal from inthandler to quit
			trackct = 0;

		set_timer( smfi, fillclock[ masterswitch ] - smfi->deltatime );
		smfi->deltatime = fillclock[ masterswitch ];

#if _XAPP

		if( wakeup & smfi->sig_xtox )		// signal from other xapp
		{
			Signal( mt, wakeup );		// restore signals
			trackct = 0;
		}

		if(wakeup & smfi->sig_ptoc )		// got a term or stop check
		{
			//PROCDIALOGUE *pd;
			struct Node *td;
			struct Node *node;
			Forbid();
			node = ( struct Node * )&smfi->mport_ptoc->mp_MsgList;
			while( node->ln_Pred ) node = node->ln_Pred;

			for( td = node; td->ln_Succ; td=td->ln_Succ)
			{
				if (	((PROCDIALOGUE * )td)->pd_Cmd == DCC_DOTERM ||
							((PROCDIALOGUE * )td)->pd_Cmd == DCC_DORUN )		// re-run
					break;
				
				if( smfi->action == 0 )
					if ( ((PROCDIALOGUE * )td)->pd_Cmd == DCC_DOSTOP )
						break;
			}
			Permit();

			if (	((PROCDIALOGUE * )td)->pd_Cmd == DCC_DOTERM  ||
						((PROCDIALOGUE * )td)->pd_Cmd == DCC_DORUN ||
						( (smfi->action == 0 ) && ((PROCDIALOGUE * )td)->pd_Cmd == DCC_DOSTOP ) )
			{
				Signal( mt, wakeup );			// restore signals
				trackct = 0;
			}
			else
				smfi->restoresignal = smfi->sig_ptoc;
		}
#endif

		/*-------------------------------*/
		/* Start the next set of events  */
		/*-------------------------------*/
		if(ylength[masterswitch^1]!=0)
		{
			PlayMidi( smfi,smfi->pfillbuf[ masterswitch^1 ], ylength[ masterswitch^1 ] );
		}

	}

	/*-----------------------------------*/
	/* Finish off the last set of events */
	/*-----------------------------------*/
	masterswitch ^= 1L;

	if( ylength[ masterswitch^1 ] != 0 )
	{
		PlayMidi( smfi,smfi->pfillbuf[ masterswitch^1 ], ylength[ masterswitch^1 ] );
	}

	error_restore:

	if( mt != NULL)
		SetTaskPri(mt,oldpri);

	return_value = 0;

	error:

	switch_off_notes( smfi );

#if _PRINT
	printf("Arrived at error with %s\n",errors[return_value] );
#endif

		if( smfi->restoresignal != NULL )
		{
			Signal( mt, smfi->restoresignal );
//			printf("Restoring signal\n");
		}

#if _PRINT
	printf("exit main loop\n");
#endif
	return(return_value);
}

//===============================================
//	Name		:	ComVarLen
//	Function	:	convert variable length value pointed at by value
//	Inputs	:	pointer to variable lengt string
//	Result	:	the value in decimal
//	Updated	:	27-04-93
//
ULONG ComVarLen( UBYTE *value)
{
   register ULONG newval=0;
   register UBYTE x=0;

   while(x<4)
   {
      newval <<= 7;
      newval |=  *(value+x) & 0x7f;
      if(*(value+x) < 0x80)					// x=4;
			return( newval );
      x++;
   }
   return(newval);
}

/*--------------------------------------------------*/  
/* Translate from raw track data to a decoded event */
/*--------------------------------------------------*/
 
UBYTE *DecodeEvent(	UBYTE 	*ptdata,
							struct	DecTrack *pDTdata,
							ULONG		deswitch,
							SMFInfo	*smfi )
{
   LONG status;
   ULONG length;
   BOOL skipit;

   pDTdata->absdelta = 0L;
   pDTdata->playable = TRUE; /* Assume it's playble and not a meta-event */

   skipit=FALSE;
   do
   {
      /* is this track all used up? */             
      if( ptdata >= pDTdata->endmarker )
      {
#if _PRINT
         printf("Track done,...\n");
#endif
         smfi->donecount++;
//			printf("returning\n");
         return(0L);
      }							/* there is more data to handle in the track */
      else
      {
         /* Decode delta */

         pDTdata->absdelta += smfi->tfactor * ComVarLen(ptdata);
         pDTdata->nexclock += pDTdata->absdelta;

         /* Update pointer to event following delta */

			while( *ptdata > 127 )
			{
				ptdata++;
			}
			ptdata++;		// skip the < 127 byte

         if( *ptdata > 127 ) /* Event with status ($80-$FF): decode new status */
         {
            status = *ptdata;
         
            pDTdata->status = status;
            pDTdata->rstatus = 0;    // No running status was used
     
            ptdata++;
            
            if( status < 240) // Handle easy status $8x - $Ex
            {
               skipit = FALSE;
               pDTdata->d1 = *ptdata;
               if( status < 192 || status > 223) // $80-$BF, $E0-$EF: 2 data bytes
               {
                  ptdata++;
                  pDTdata->d2 = *ptdata;
               }
               else pDTdata->d2 = 0;            // $C0-$DF: 1 data byte
            }
            else // Status byte $Fx, system exclusive or meta events
            {
               skipit = TRUE;
               
               if( status == 0xff )            // It's a meta event ($ff)
               {
                  pDTdata->metatype = *ptdata;
              
                  ptdata++; // Now pointing at length byte

                  if( pDTdata->metatype == 81 )
                  {
                     /* Tempo change event.  There are 60 milllion    */
                     /* microseconds in a minute.  The lower 3 bytes  */ 
                     /* pointed to by ptdata give the microseconds    */
                     /* per MIDI quarter note. So, assuming a quarter */
                     /* note gets the beat, this equation             */
                     /*      60000000L /                              */
                     /*          ( *((ULONG *)ptdata) & 0x00ffffff ) )*/
                     /* gives beats per minute.                       */

							smfi->tempo = *((ULONG *)ptdata ) & 0x00ffffff;
//							smfi->tfactor = changetempo( *((ULONG *)ptdata ) & 0x00ffffff, smfi );
							smfi->tfactor = smfi->tempo / smfi->Division;
#if _PRINT
							printf("Factor set to %ld\n",smfi->tfactor );
#endif

                     /* Tempo event is not playable.  This prevents the */
                     /* event from being transferred to the play buffer */

                     pDTdata->playable = FALSE; 

                     /* Even though this event can't be played, it     */
                     /* takes some time and should not be skipped.     */

                     skipit = FALSE;
                  }
                  length = ComVarLen( ptdata );
                  pDTdata->absmlength = length;
                  while( *ptdata > 127 )
							ptdata++;
                  ptdata+=length;
               }
               else
					if( status == 0xf0 || status == 0xf7) // It's a sysex event
               {
                  pDTdata->metatype=0xff;

#if _PRINT
                  printf("Sysex event");
#endif

                  length = ComVarLen( ptdata );
                  pDTdata->absmlength = length;
                  while(*ptdata>127)
							ptdata++;
                  ptdata += length;
               }
               else        // It's an unkown event type ($f1-$f6, $f8-$fe)
               {
                  pDTdata->metatype = 0xff;
#if _PRINT
                  printf("Unknown event"); 
#endif
               }
            }
         }
         else	// Event without status ($00-$7F): use running status
         {
            skipit=FALSE;

            // Running status data bytes

            status = pDTdata->status;
            pDTdata->rstatus = status;

            if( status == 0 )
					release( smfi, BAD_FILE_ERR );

				pDTdata->d1 = *ptdata;

            if( status < 192 || status > 223 ) // $80-$BF, $E0-$EF: 2 data bytes
            {
               ptdata++;
               pDTdata->d2 = *ptdata;
            }
            else pDTdata->d2=0;         // $C0-$DF: 1 data byte
         }
         ptdata++;
      }
   }
   while(skipit);

   return(ptdata);
}


/*------------------------------------------------------------*/
/* Transfer the decoded event to the fill buffer for playback */
/*------------------------------------------------------------*/
LONG transfer( struct DecTrack *pDT,ULONG mswitch,LONG ylen, SMFInfo *smfi )
{
   ULONG y;

   y = ( ULONG )ylen;

   if ( pDT->playable )
   {
      if(pDT->rstatus == smfi->lastRSchan)
      { 
         // Running status so just put the 2 data bytes in buffer

         *( smfi->pfillbuf[ mswitch ] + y ) = pDT->d1;
         y++;
         *( smfi->pfillbuf[ mswitch ] + y ) = pDT->d2;
      }
      else 
      {
         // New status so store status and data bytes
         *( smfi->pfillbuf[ mswitch ] + y ) = pDT->status;
         y++;
         *( smfi->pfillbuf[ mswitch ] + y ) = pDT->d1;
         if( pDT->status < 192 || pDT->status > 223 )
         {
            y++;
            *( smfi->pfillbuf[ mswitch ] + y ) = pDT->d2;
         }
         smfi->lastRSchan = pDT->status;     
      }
      y++;
   }
   return( ( LONG )y );
}
