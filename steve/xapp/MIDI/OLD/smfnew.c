//	File		:	smfnew.c
//	Uses		:	camd_errs.h
//	Date		:	25-04-93
//	Author	:	Dan Baker, Commodore Business Machines
//					( adapted for MP ing. C. Lieshout )
//	Desc.		:	Read and play a MIDI file with the use of the realtime
//					and camd.library. Further, try to implement this in a MP-XAPP
//

/* System Include Files */
#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>
#include <workbench/startup.h>

/* CAMD MIDI Library Includes */
#include <clib/camd_protos.h>
#include <midi/camd.h>
#include <midi/camdbase.h>
#include <pragmas/camd_pragmas.h>

/* CAMD Real Time Library Includes */
#include <clib/realtime_protos.h>
#include <libraries/realtime.h>
//#include <midi/realtimebase.h>
#include <pragmas/realtime_pragmas.h>

/* System function prototypes */
#include	<proto/dos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

/* new XAPP etc includes */
#include "camd_err.h"

#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"

/* Lattice Standard I/O */
#include <stdlib.h>
#include <stdio.h>

#ifdef LATTICE
int CXBRK(void) { return(0); }     /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
#endif

#include "camd.h"

#define _PRINT FALSE

struct note
{
	UBYTE off;
	UBYTE num;
	UBYTE vel;
};

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
	struct Library *CamdBase = smfi->CamdBase;

	for( i = 0; i < 16; i++ )
	{
		notes[i].off = 0x80 + i;
		notes[i].num = 0;
		notes[i].vel = 127;
	}
	ParseMidi(	smfi->pMidiLink, (UBYTE *)notes, 48 );
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
	smfi->pMidiLink	= NULL;
	smfi->pMidiNode	= NULL;
	smfi->pPlayerInfo	= NULL;
	smfi->pData			= NULL;
	smfi->midiSignal	= -1;
	smfi->donecount	= 0L;
	smfi->lastRSchan	= 0xf1; /* Status of $F1 is undefined in Standard MIDI file Spec */
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

	smfi->CamdBase=OpenLibrary("camd.library",0L);
	if( !smfi->CamdBase )
		return( NO_CAMD_LIB_ERR );
  smfi->RealTimeBase=OpenLibrary("realtime.library",0L);
	if( !smfi->RealTimeBase )
		return( NO_REAL_LIB_ERR );

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

	if(pSMFHeader->Ntrks > MAXTRAX )
		return( TOO_MANY_TRAX_ERR );

	smfi->Ntrks = pSMFHeader->Ntrks;

	/*--------------------*/
	/* Evaluate time base */
	/*--------------------*/
	if (pSMFHeader->Division < 0)
		return( WRONG_TIME_BASE_ERR );
	else
	{
		smfi->Division=(ULONG)pSMFHeader->Division;

		/* According to "Standrd MIDI Files 1.0", July 1988, page 5 */
		/* para. 4: "...time signature is assumed to be 4/4 and the */
		/*           tempo 120 beats per minute."                   */
		smfi->tfactor=changetempo( 500000L, smfi ); /* One quarter note every half second */
	}

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
	return( FILE_OKE );
}

void release( SMFInfo *smfi, int err )
{
	struct Library *CamdBase = smfi->CamdBase;
	struct Library *RealTimeBase = smfi->RealTimeBase;
	smfi->release = 0;

	if( smfi->smfhandle != NULL )
		Close( smfi->smfhandle );
	smfi->smfhandle = NULL;
	if( smfi->smfdata != NULL )
		FreeMem( smfi->smfdata, smfi->smfdatasize );
	if( smfi->pMidiLink )
		RemoveMidiLink( smfi->pMidiLink );
	if( smfi->pMidiNode )
		DeleteMidi( smfi->pMidiNode );
	if( smfi->pPlayerInfo )
		DeletePlayer( smfi->pPlayerInfo );
	if( smfi->pData )
		FreeMem( smfi->pData, smfi->sizeDTrack );
	if( smfi->midiSignal != -1 )
		FreeSignal( smfi->midiSignal );
	if( smfi->RealTimeBase )
		CloseLibrary( smfi->RealTimeBase );
	if( smfi->CamdBase )
		CloseLibrary( smfi->CamdBase );
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
	LONG	y,z,res;
	BOOL	notdone,timerr;
	ULONG	masterswitch,lowclock,
			ylength[2],wakeup;

	UBYTE	*ptrack[TWOxMAXTRAX],trackct;
	LONG	fillclock[2];
	ULONG	oldclock;
	struct Library *CamdBase = smfi->CamdBase;
	struct Library *RealTimeBase = smfi->RealTimeBase;
	struct TagItem taglist[32];
	struct MsgPort *pt;

	BYTE		oldpri;				/* Priority to restore */
	struct	Task	*mt;		/* Pointer to this task */

	UBYTE fillbuf1[MIDIBUFSIZE];	/* These buffers hold the notes translated */
	UBYTE fillbuf2[MIDIBUFSIZE];	/* from the SMF file for playback          */
	struct DecTrack *pDTrack[MAXTRAX];

	smfi->release = 1;

	oldclock			= 0L;
	mt					= NULL;
	notdone			= TRUE;
	fillclock[0]	= 0L;
	fillclock[1]	= 0L;

	trackct=0;
	pbyte=smfi->smfdata;
	smfi->pfillbuf[0]	= fillbuf1;
	smfi->pfillbuf[1]	= fillbuf2;

// This could be done with an faster search algoritme
// For example with the Boyer Moore with a pre define jump string
//	printf("size is %d\n",smfi->smfdatasize );
//	printf("base is %x\n",smfi->smfdata );

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

#if _PRINT
	printf("There are %ld tracks in this SMF file.\n",trackct);
#endif

	/*----------------------------------------------*/
	/* Set up a MidiNode and a MidiLink.  Link the  */
	/* node to the default "out.0" MidiCluster .    */
	/*----------------------------------------------*/

	taglist[0].ti_Tag		= MIDI_Name;
	taglist[0].ti_Data	= (ULONG)"SMF Player";
	taglist[1].ti_Tag		= MIDI_MsgQueue;
	taglist[1].ti_Data	= 0L;
	taglist[2].ti_Tag		= MIDI_SysExSize;
	taglist[2].ti_Data	= 0L;
	taglist[3].ti_Tag		= TAG_END;
	taglist[3].ti_Data	= NULL;

	smfi->pMidiNode=CreateMidiA(taglist);
	return_value = NO_MEM_NODE_ERR;

	if( !smfi->pMidiNode )
		goto error;

	taglist[0].ti_Tag		= MLINK_Comment;
	taglist[0].ti_Data	= (ULONG)"SMF Player Link";
	taglist[1].ti_Tag		= MLINK_Parse;
	taglist[1].ti_Data	= TRUE;
	taglist[2].ti_Tag		= MLINK_Location;
	taglist[2].ti_Data	= (ULONG)"out.0";
	taglist[3].ti_Tag		= TAG_END;
	taglist[3].ti_Data	= NULL;

	smfi->pMidiLink=AddMidiLinkA(smfi->pMidiNode, MLTYPE_Sender, taglist);
	return_value = NO_MEM_LINK_ERR;
   if( !smfi->pMidiLink )
		goto error;

	/*----------------------------------------*/
	/* Set up DTrack structure for each track */
	/*----------------------------------------*/

	smfi->sizeDTrack=trackct*sizeof(struct DecTrack);
	smfi->pData=AllocMem( smfi->sizeDTrack, MEMF_PUBLIC | MEMF_CLEAR );
	return_value = NO_MEM_WORK_ERR;
	if( !smfi->pData )
		goto error;

   for( x = 0; x < trackct; x++ )
   {
      pDTrack[x]=(struct DecTrack *)(x * sizeof(struct DecTrack) + smfi->pData);
      pDTrack[x]->endmarker = ptrack[MAXTRAX+x];      /* add end marker */
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

		ptrack[x] = DecodeEvent( ptrack[x] , pDTrack[x] , masterswitch, smfi );
		if(pDTrack[x]->nexclock < lowclock && ptrack[x])
			/* Find the first event */
			lowclock=pDTrack[x]->nexclock;
   }

	/*-----------------------------------*/
	/* Transfer first events to A buffer */
	/*-----------------------------------*/
	for(x=0;x<trackct;x++)
	{
		if((pDTrack[x]->nexclock==lowclock) && ptrack[x])
		{
			/* Transfer event to parse buffer and handle successor */
			y=transfer(pDTrack[x],masterswitch,y, smfi );
			z=1;
			while(z==1)
			{
				ptrack[x]=DecodeEvent(ptrack[x],pDTrack[x],masterswitch,smfi);
 	 			/* Next delta is zero... */
				if( !(pDTrack[x]->absdelta) && ptrack[x])
				{
					y=transfer(pDTrack[x],masterswitch,y,smfi);
				}
				else
				{
					z=0;
				}
			}
		}
	}
	ylength[masterswitch] = y;
   fillclock[masterswitch] = (LONG)( smfi->tfactor * lowclock );


	/*------------------------------------*/
	/* Transfer second events to B buffer */
	/*------------------------------------*/
	y=0;
	masterswitch=1L;
	lowclock=0xffffffff;
	for(x=0;x<trackct;x++)
	{
		if(pDTrack[x]->nexclock < lowclock && ptrack[x])
			lowclock=pDTrack[x]->nexclock;
	}

	for(x=0;x<trackct;x++)
	{
		if(pDTrack[x]->nexclock==lowclock && ptrack[x])
		{
			/* Transfer event to parse buffer and handle successor */

			y=transfer(pDTrack[x],masterswitch,y,smfi);
			z=1;
			while(z==1)
			{
				ptrack[x]=DecodeEvent(ptrack[x],pDTrack[x],masterswitch,smfi);
				/* Next delta is zero... */
				if( !(pDTrack[x]->absdelta) && ptrack[x])
				{
					y=transfer(pDTrack[x],masterswitch,y,smfi);
				}
				else
				{
					z=0;
				}
			}
		}
	}

	ylength[masterswitch] = y;
	fillclock[masterswitch] = (LONG)(smfi->tfactor*lowclock);

	/*-----------------------------------------------------*/
	/* Priority Must Be Above Intuition and Graphics Stuff */
	/*-----------------------------------------------------*/ 
	mt = FindTask( NULL );
	oldpri = SetTaskPri( mt, 21 );


	/*---------------------------------------------------------------*/ 
	/* Set up a PlayerInfo and a Conductor to get timing information */
	/*---------------------------------------------------------------*/ 
	smfi->midiSignal = AllocSignal( -1L );
	return_value = NO_SIGNAL_ERR;
	if( smfi->midiSignal == -1 ) 
		goto error;

	taglist[0].ti_Tag		= PLAYER_Name;
	taglist[0].ti_Data	= (ULONG)"SMF Player PlayerInfo";
	taglist[1].ti_Tag		= PLAYER_Conductor;
	taglist[1].ti_Data	= (ULONG)"SMF Player's Conductor";
	taglist[2].ti_Tag		= PLAYER_AlarmSigTask;
	taglist[2].ti_Data	= (ULONG)mt;
	taglist[3].ti_Tag		= PLAYER_AlarmSigBit;
	taglist[3].ti_Data	= smfi->midiSignal;
	taglist[4].ti_Tag		= TAG_END;
	taglist[4].ti_Data	= NULL;

  smfi->pPlayerInfo=CreatePlayerA(taglist);
	return_value = NO_PLAYER_ERR;
	if(!smfi->pPlayerInfo)
		goto error;

	/*---------------------------------*/
	/* Make sure the clock is stopped. */
	/*---------------------------------*/
	res = SetConductorState( smfi->pPlayerInfo, CONDSTATE_STOPPED, 0L );
	return_value = NO_STOP_COND_ERR;
	if( res != 0 )
		goto error;

	/*-------------------------------------------*/
	/* Play the first batch of notes in Buffer A */
	/*-------------------------------------------*/
	if( ylength[masterswitch^1] != 0 )
	{
		ParseMidi(	smfi->pMidiLink,smfi->pfillbuf[masterswitch^1],
						ylength[masterswitch^1]);
	}

	/*------------------------------------*/
	/* and start the RealTime alarm clock */
	/*------------------------------------*/
	res = SetConductorState( smfi->pPlayerInfo, CONDSTATE_RUNNING, 0L );
	return_value = NO_START_COND_ERR;
	if( res != 0 )
		goto error;
	/*---------------------*/
	/* and set the alarm.  */
	/*---------------------*/

	taglist[0].ti_Tag		= PLAYER_AlarmTime;
	taglist[0].ti_Data	= fillclock[masterswitch];
	taglist[1].ti_Tag		= PLAYER_Ready;
	taglist[1].ti_Data	= TRUE;
	taglist[2].ti_Tag		= TAG_END;
	taglist[2].ti_Data	= NULL;

	timerr = SetPlayerAttrsA(smfi->pPlayerInfo,taglist);
	return_value = NO_PLAY_ATTR1_ERR;
	if( !timerr )
		goto error;

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
			if((pDTrack[x]->nexclock < lowclock) && ptrack[x])
				lowclock=pDTrack[x]->nexclock;
		}

		/*-----------------------------------*/
		/* Transfer events to current buffer */
		/*-----------------------------------*/
		for( x= 0; x < trackct; x++ )
		{
			if( ( pDTrack[ x ]->nexclock == lowclock ) && ptrack[ x ] )
			{
				/* Transfer event to parse buffer and handle successor */
				y = transfer( pDTrack[x], masterswitch, y, smfi );
				z = 1;
				while( z==1 )
				{
					ptrack[x] = DecodeEvent(ptrack[x],pDTrack[x],masterswitch,smfi);
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
      fillclock[ masterswitch ] = (LONG)( smfi->tfactor * lowclock );


		/*---------------------------------------------------------------*/
		/* Wait() for the CAMD alarm or a CTRL-C keypress from the user. */
		/*---------------------------------------------------------------*/
		if( timerr )
			wakeup=Wait(1L<< smfi->midiSignal | smfi->mainsignal);

		if( wakeup & smfi->sig_xtox )		// signal from other xapp
		{
#if _PRINT
			printf("Got signal from other xapp reseting sig %x\n", wakeup );
#endif
			Signal( mt, wakeup );		// restore signals
			/* Restore Priority */
			if(mt!=NULL) SetTaskPri(mt,oldpri);
			/* And Quit */
			return_value = USER_ABORT;
			goto error;
		}

		if(wakeup & smfi->sig_ptoc )		// got a term or stop check
		{
			PROCDIALOGUE *pd;
			struct Node *td;
			struct Node *node;
#if _PRINT
			printf("Got signal\n");
#endif
			// check the port to see if you need to stop

			Forbid();
			node = ( struct Node * )&smfi->mport_ptoc->mp_MsgList;
			while( node->ln_Pred ) node = node->ln_Pred;

			for( td = node; td->ln_Succ; td=td->ln_Succ)
			{
#if _PRINT
				printf("command = %d\n",((PROCDIALOGUE * )td)->pd_Cmd );
#endif
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
#if _PRINT
			printf("Got Term signal\n");
#endif

				Signal( mt, wakeup );			// restore signals
				/* Restore Priority */
				if(mt!=NULL) SetTaskPri(mt,oldpri);
				/* And Quit */
				return_value = USER_ABORT;
				goto error;

			}
			else
			{
#if _PRINT
			printf("Got other signal\n");
#endif
				// Store the signal for now but continue playing
				smfi->restoresignal = smfi->sig_ptoc;
			}
		}

		/*-------------------------------*/
		/* Start the next set of events  */
		/*-------------------------------*/
		if(ylength[masterswitch^1]!=0)
		{
			ParseMidi(	smfi->pMidiLink,smfi->pfillbuf[masterswitch^1],
							ylength[masterswitch^1] );
		}

		/*---------------------*/
		/* and set the alarm.  */
		/*---------------------*/

		taglist[0].ti_Tag		= PLAYER_AlarmTime;
		taglist[0].ti_Data	= fillclock[masterswitch];
		taglist[1].ti_Tag		= PLAYER_Ready;
		taglist[1].ti_Data	= TRUE;
		taglist[2].ti_Tag		= TAG_END;
		taglist[2].ti_Data	= NULL;

		timerr = SetPlayerAttrsA(smfi->pPlayerInfo, taglist);
		return_value = NO_PLAY_ATTR2_ERR;
      if( !timerr )
			goto error;
	}

	/*-----------------------------------*/
	/* Finish off the last set of events */
	/*-----------------------------------*/
	masterswitch ^= 1L;

	if( timerr )
		wakeup=Wait(1L<< smfi->midiSignal | smfi->mainsignal );

	if(ylength[masterswitch^1]!=0)
	{
		ParseMidi(	smfi->pMidiLink,smfi->pfillbuf[masterswitch^1],
						ylength[masterswitch^1]);
	}

	/* Restore Priority */
	if( mt != NULL)
		SetTaskPri(mt,oldpri);
	return_value = 0;

	error:

//		switch_off_notes( smfi );

#if _PRINT
	printf("Arrived at error with %s\n",errors[return_value] );
#endif

		if( smfi->restoresignal != NULL )
		{
			Signal( mt, smfi->restoresignal );
//			printf("Restoring signal\n");
		}

//		release( smfi, FILE_OKE );

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
 
UBYTE *DecodeEvent(UBYTE *ptdata,struct DecTrack *pDTdata, ULONG deswitch, SMFInfo *smfi)
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
         return(0L);
      }
      else /* there is more data to handle in the track */
      {
         /* Decode delta */
         pDTdata->absdelta += ComVarLen(ptdata);
         pDTdata->nexclock+= pDTdata->absdelta;

         /* Update pointer to event following delta */
         while(*ptdata>127)
            {
            ptdata++;
            }
         ptdata++;

         if(*ptdata>127) /* Event with status ($80-$FF): decode new status */  
         {
            status=*ptdata;
         
            pDTdata->status=status;
            pDTdata->rstatus=0;    /* No running status was used */
     
            ptdata++;
            
            if(status<240) /* Handle easy status $8x - $Ex */
            {
               skipit=FALSE;
               pDTdata->d1 = *ptdata;
               if(status<192 || status>223) /* $80-$BF, $E0-$EF: 2 data bytes */
               {
                  ptdata++;
                  pDTdata->d2=*ptdata;
               }
               else pDTdata->d2=0;            /* $C0-$DF: 1 data byte */
            }
            else /* Status byte $Fx, system exclusive or meta events  */
            {
               skipit=TRUE;
               
               if(status==0xff)            /* It's a meta event ($ff) */
               {
                  pDTdata->metatype=*ptdata;
              
                  ptdata++; /* Now pointing at length byte */

                  if(pDTdata->metatype==81)
                  {
                     /* Tempo change event.  There are 60 milllion    */
                     /* microseconds in a minute.  The lower 3 bytes  */ 
                     /* pointed to by ptdata give the microseconds    */
                     /* per MIDI quarter note. So, assuming a quarter */
                     /* note gets the beat, this equation             */
                     /*      60000000L /                              */
                     /*          ( *((ULONG *)ptdata) & 0x00ffffff ) )*/
                     /* gives beats per minute.                       */

							smfi->tfactor = changetempo( *((ULONG *)ptdata ) & 0x00ffffff, smfi );

                     /* Tempo event is not playable.  This prevents the */
                     /* event from being transferred to the play buffer */
                     pDTdata->playable = FALSE; 

                     /* Even though this event can't be played, it     */
                     /* takes some time and should not be skipped.     */
                     skipit=FALSE;
                  }
                  length=ComVarLen(ptdata);
                  pDTdata->absmlength=length;
                  while(*ptdata>127)ptdata++;

                  ptdata+=length;
               }
               else if(status==0xf0 || status==0xf7) /* It's a sysex event */
               {
                  pDTdata->metatype=0xff;
#if _PRINT
                  printf("Sysex event");
#endif
                  length=ComVarLen(ptdata);
                  pDTdata->absmlength=length;
                  while(*ptdata>127)ptdata++;

                  ptdata+=length;
               }
               else        /* It's an unkown event type ($f1-$f6, $f8-$fe) */
               {
                  pDTdata->metatype=0xff;
#if _PRINT
                  printf("Unknown event"); 
#endif
               }
            }
         }
         else	/* Event without status ($00-$7F): use running status */
         {
            skipit=FALSE;
            /* Running status data bytes */
            status=pDTdata->status;
            pDTdata->rstatus=status;

            if(status==0)
					release( smfi, BAD_FILE_ERR );
				pDTdata->d1=*ptdata;

            if(status<192 || status>223) /* $80-$BF, $E0-$EF: 2 data bytes */
            {
               ptdata++;
               pDTdata->d2=*ptdata;
            }
            else pDTdata->d2=0;         /* $C0-$DF: 1 data byte */
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
   y=(ULONG )ylen;

   if (pDT->playable)
   {
      if(pDT->rstatus == smfi->lastRSchan)
      { 
         /* Running status so just put the 2 data bytes in buffer */
         *(smfi->pfillbuf[mswitch] + y)=pDT->d1;
         y++;
         *(smfi->pfillbuf[mswitch] + y)=pDT->d2;
      }
      else 
      {
         /* New status so store status and data bytes */
         *(smfi->pfillbuf[mswitch] + y)=pDT->status;
         y++;
         *(smfi->pfillbuf[mswitch] + y)=pDT->d1;
         if(pDT->status<192 || pDT->status>223)
         {
            y++;
            *(smfi->pfillbuf[mswitch] + y)=pDT->d2;
         }
         smfi->lastRSchan=pDT->status;     
      }
      y++;
   }
   return((LONG)y);
}


/*-------------------------------------------------------------------------*/
/* Handle the Change Tempo event.   With the realtime.library, the timing  */
/* quantum is fixed at 1.66 milliseconds (600 Hz).  This makes handling    */
/* of SMF tempo pretty rough.  Tempo is controlled through a ULONG integer */
/* named tfactor which is used to multiply the time deltas in the SMF      */
/* file.  We can't multiply the time deltas by a fractional amount and     */
/* that makes tempo handling even rougher.  For correct tempo handling the */
/* clock quantum has to be variable and controlled by this program.        */
/*-------------------------------------------------------------------------*/
ULONG changetempo( ULONG ctbpm, SMFInfo *smfi )
{
   ULONG timefac,timerem,tickfreq;

   tickfreq=(ULONG)((struct RealTimeBase *)smfi->RealTimeBase)->rtb_Reserved1;

   /* CAMD uses 1.66ms quantum for 600 ticks/sec                      */
   /* SMF uses one tick = quarter note/pSMFHeader->Division           */
   /* Hence, microseconds per delta in SMF is given by ctbpm/division */
   /* and BPM is given by 60,000,000/ctbpm.                           */
   timefac=(tickfreq * (ctbpm/smfi->Division)) / 1000000;
   timerem=(tickfreq * (ctbpm/smfi->Division)) % 1000000;
   if ( timerem >= 500000 )
      timefac++;

   return(timefac);
}
