//	File		:	worksamp.c
//	Uses		:	external.h mlmmu.h
//	Date		:	-92 ( 01-05-93 ) (10-07-93)
//	Author	:	S. Vanderhorst / C. Lieshout
//	Desc.		:	Play a sample
//
#include <stdio.h>
#include <string.h>
#include <workbench/startup.h>
#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <libraries/dosextens.h>
#include <proto/exec.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>

//#include "pascal:include/textedit.h"
//#include "pascal:include/textstyles.h"
#include "nb:capsdefines.h"
#include "nb:newdefines.h"
//#include "nb:parser.h"
//#include "pascal:include/misctools.h"
//#include "nb:capsstructs.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"

#include "gen:general.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

#include "iff_fsound.h"
#include "sample.h"
#include "loadsamp.h"
#include "protos.h"
#include "structs.h"

/**** diskplay.o ****/

void fade_in( SoundInfo *sinfo );
void remove_fade( SoundInfo *sinfo );
void exit_sound( SoundInfo *sinfo );
void play_sound( SoundInfo *sinfo );
void change_sound( SoundInfo *sinfo );

#include <stdlib.h>
#include <stdio.h>

#define _PRINTF FALSE

#define XAPPPORTNAME1 "Port_MLSample1"
#define XAPPPORTNAME2 "Port_MLSample2"

void set_sound_vars( SoundInfo *sinfo, struct Sample_record *sr )
{
	sinfo->filter = sr->filter;
	sinfo->channel = 1;
	if( sr->track == 1 )								// check which channel
		sinfo->channel = 2;

	if( sr->action == SAMPLE_PLAY )
	{

		if( sr->playFromDisk )
			sinfo->type |= SI_DISK;
		else
		{ 												// if both on than forced diskplay
			if( ( sinfo->type & ( SI_DISK | SI_NORMAL ) ) == ( SI_DISK | SI_NORMAL   ) )
			{
//				KPrintF("Forced from vars\n");
				sinfo->type &= ~SI_NORMAL;								// clear the normal bit
				sinfo->type |= SI_DISK;
				sr->playFromDisk = 1;
			}
			else
				sinfo->type |= SI_NORMAL;
		}

		sinfo->sigtest = 1;								// you should signal me
		sinfo->end = 0;
		if( sr->loops != 2 )	// 2 means 1 loop, 1 means infinite.
		{
//	KPrintF("Set loops\n");
			sinfo->loops = sr->loops-2;
			sinfo->type |= SI_LOOPING;
		}
		else
		{
//			KPrintF("Loop is loops -1\n");
			sinfo->loops = sr->loops-1;
		}

		sinfo->loop = 0;
		sinfo->period = (WORD)( 3579546 / sr->freq );
		set_volume( sinfo, sr->volume, sr->balance );
	}
}

int Init_Sample_rec( PROCESSINFO *ThisPI, struct Sample_record *sr )
{
	sr->action			= SAMPLE_PLAY;
	sr->filename[0]	= '\0';
	sr->loops			= DEFAULT_LOOPS;
	sr->volume			= DEFAULT_VOL;
	sr->freq				= DEFAULT_FREQ;
	sr->playFadeIn		= DEFAULT_FADE;
	sr->balance			= DEFAULT_BALANCE;

	sr->fadeOut			= DEFAULT_FADEINOUT;
	sr->fadeIn			= DEFAULT_FADEINOUT;
	sr->setVolume		= DEFAULT_SETVOL;

	GetVarsFromPI( sr, ThisPI );

	if ( sr->filename[ 0 ] != '\0' )
		return 1;
	else
		return 0;
}

void main( int argc, char *argv[] )
{

	MLSYSTEM			*MLSystem;	
	PROCDIALOGUE	*Msg_XappDial,		// Dialogue to be send to an already active Xapp
						*Msg_RXappDial,	// Dialogue received by another Xapp
						*Msg_SampleDial,	// Our dialogue 
						*Msg_RSampleDial;	// Dialogue when our guide replies
												// This is actually a copy of the ptr we sent.  
												// Also: dialogue from our parent
	PROCESSINFO		*ThisPI;				// ptr to this processinfo blk 
												// (as used in our parent's list)
	struct MsgPort	*Port_Xapp,			// Inter-Xapp communication receive port
						*Port_OtherXapp,	// Port of older Xapp
						*RepP_Xapp,			// Inter-Xapp communication reply port
						*RepP_WorkSample;	// Reply port for our parent when 
												// replying to our messages

	ULONG				Sig_PtoC,			// A parent to child signal
						SigR_CtoP,			// A reply to a msg we send to our parent

						Sig_XtoX,			// A signal from an equal Xapp
						SigR_XtoX,			// Reply from an equal Xapp
						SigRecvd;			// Signals received

	BOOL 				B_ReInit,			// if TRUE, re-initialise data
						B_Term,				// If TRUE, we are free to terminate
						B_Run,			
						B_Remove,			// If True, our guide wants us to clean up
						B_Setup,				// if True, SetupPlayer succeeded
						B_Stop,				// if True, another xapp wants us to stop
						B_Playing,			// If True, PlayTune Succeeded
						B_Last,
						B_Ex,
						B_Fading;

	int i;
	struct Library	*MLMMULibBase = NULL;
	struct Task *task;
	long oldpri = 0;
	char *portname;
	ULONG sigs = 0;

	int lerr = 0;

	SoundInfo si;
	struct Sample_record sample_rec;
	struct Sample_record ex_sample_rec;
	struct Sample_record *ex_sr;

	// ADDED BY ERIK Sunday 13-Mar-94 12:08:19 because I saw that when play was
	// hit an enforcer hit was reported on line 332 (current line). I might have
	// come from an uninialized structure with garbage, resulting in a signal
	// that didn't exist.
	setmem(&si, sizeof(SoundInfo), 0);

	// Get our PROCESSINFO base ptr
	if( (ThisPI = (PROCESSINFO *)ml_FindBaseAddr(argc, argv)) == NULL)
		return;

	KPrintF("Starting SAMPLE [%s]\n",ThisPI->pi_Arguments.ar_Worker.aw_Name);

	if(ThisPI->pi_Arguments.ar_Worker.aw_Origin == ORG_SCRIPTEDITOR)
	{
		XappSetup(ThisPI);
		ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;
		return;
	}
	else
		// Place the data from the PI in the local sample structure
		Init_Sample_rec( ThisPI, &sample_rec );

	si.type = 0;								// clear type 
	set_sound_vars( &si, &sample_rec );

	if( si.channel == 1 )
		portname = XAPPPORTNAME1;
	else
		portname = XAPPPORTNAME2;

	/******************************************************/

	MLSystem = ThisPI->pi_Arguments.ar_Worker.aw_MLSystem;

	ThisPI->pi_Arguments.ar_RetErr = ERR_WORKER;

	if( ( MLMMULibBase = (struct Library *)OpenLibrary("mpmmu.library",0)) == NULL)
		return;

	if( (RepP_WorkSample = (struct MsgPort *)CreatePort(0,0)) == NULL)
	{
		CloseLibrary(MLMMULibBase);
		return;
	}
	if((Msg_SampleDial = (PROCDIALOGUE *)MLMMU_AllocMem(sizeof(PROCDIALOGUE), 
								MEMF_PUBLIC|MEMF_CLEAR,NULL ) ) == NULL)
	{
		CloseLibrary(MLMMULibBase);
		DeletePort(RepP_WorkSample);
		return;
	}

	// Set up the Dialogue message
	Msg_SampleDial->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
	Msg_SampleDial->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);
	Msg_SampleDial->pd_Msg.mn_ReplyPort = RepP_WorkSample;
	Msg_SampleDial->pd_ChildPI = ThisPI;
	Msg_SampleDial->pd_InUse = TRUE;
	Msg_SampleDial->pd_Cmd = DCI_CHILDREADY;

	SigR_CtoP  		=   1 << RepP_WorkSample->mp_SigBit;
	Sig_PtoC     	=   1 << ThisPI->pi_Port_PtoC->mp_SigBit;
	PutMsg(ThisPI->pi_Port_CtoP,(struct Message *)Msg_SampleDial);

	// Make A dialogue to send to other Xapps  
		if( (Msg_XappDial = (PROCDIALOGUE *)MLMMU_AllocMem( sizeof(PROCDIALOGUE), 
								MEMF_PUBLIC|MEMF_CLEAR,NULL )) == NULL  )
	{
		MLMMU_FreeMem( Msg_SampleDial );
		CloseLibrary(MLMMULibBase);
		DeletePort(RepP_WorkSample);
		return;
	}

	Port_Xapp = NULL;
	Sig_XtoX = 0;

	Port_OtherXapp = NULL;

	// Set up a reply port in case we need a reply from an older Xapp
	RepP_Xapp = (struct MsgPort *)CreatePort(0,0);
	SigR_XtoX  = 1 << RepP_Xapp->mp_SigBit;

	Msg_XappDial->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
	Msg_XappDial->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);
	Msg_XappDial->pd_Msg.mn_ReplyPort = RepP_Xapp;
	Msg_XappDial->pd_ChildPI = ThisPI;

	// Load the sample

	if( sample_rec.action == SAMPLE_PLAY )
	{
//		KPrintF("Loading sample -%s-\n",sample_rec.filename );
		lerr = loadsoundfile( &si, sample_rec.filename, TRUE, 0L );
		set_sound_vars( &si, &sample_rec );
	}
//	else
//		KPrintF("Action non play \n");

//	KPrintF("Starting SAMPLE\n");

	B_Stop = FALSE;
	B_Setup = FALSE;
	B_Playing = FALSE;
	B_ReInit = FALSE;
	B_Run = FALSE;
	B_Term = FALSE;
	B_Remove = FALSE;
	B_Fading = FALSE;
	B_Last = FALSE;
	B_Ex = FALSE;

	sigs = Sig_XtoX | SigR_XtoX | Sig_PtoC | SigR_CtoP | SIGF_ABORT;

#if _PRINTF
	{
		char st[500];
		sprintf(st,"xtox %x,rxtox %x,ptoc %x,rctop,%x",Sig_XtoX,SigR_XtoX,Sig_PtoC,SigR_CtoP );
		KPrintF("sigs - %s\n",st );
	}
#endif

	task = FindTask( 0 );
	while( !B_Term )
	{
		//char tt[200];

		SigRecvd = Wait( sigs );
#if _PRINTF
	sprintf(tt,"%x",SigRecvd);
	KPrintF("signal %s\n",tt);
#endif
		if( SigRecvd & si.audiosig )
		{
			if( si.end == 1 )
				if( !B_Last )
				{
#if _PRINTF
KPrintF("False is true\n");
#endif
					B_Last = TRUE;
				}
				else
				{
#if _PRINTF
KPrintF("vol at 0\n");
#endif
					set_volume( &si, 0, 0 );	// sound off ?
					change_sound( &si );
					si.sigtest = 0;						// swith off signalling
				}
			load_sound_frame( &si );
		}

		if( SigRecvd & si.fadesig )
		{
			remove_fade( &si );
			sigs &= ~si.fadesig;
			B_Fading = FALSE;
		}

		if(SigRecvd & SIGF_ABORT)
		{
#if _PRINTF
			KPrintF("Aborting\n");
#endif
			break;
		}
        
		if(SigRecvd & Sig_PtoC)
		{
#if _PRINTF
KPrintF("sig to ptoc\n");
#endif
			// Our parent has something to say to us
			while( (Msg_RSampleDial = (PROCDIALOGUE *)GetMsg(ThisPI->pi_Port_PtoC)) != NULL)
			{
				Msg_RSampleDial->pd_ChildPI = ThisPI;	
				switch(Msg_RSampleDial->pd_Cmd)
				{
					case DCC_DOPREPARE:
						Msg_RSampleDial->pd_Cmd = DCI_CHILDPREPARES;                 
						B_ReInit = TRUE;
						break;                          
					case DCC_DORUN:
//	KPrintF("Do run sample %s\n",sample_rec.filename);
						// Either start or re-run from pause
						Forbid();
						if( (Port_OtherXapp = (struct MsgPort *)FindPort( portname )) != NULL)
						{
							if(Port_OtherXapp != Port_Xapp)
							{
								Msg_XappDial->pd_Cmd = DCC_DOTERM;
								if( sample_rec.action != SAMPLE_PLAY && sample_rec.action != SAMPLE_STOP )
								{
//KPrintF("Send stop to other1\n");
									Msg_XappDial->pd_Cmd = DCC_IGNORE;
									Msg_XappDial->pd_Luggage.lu_SNR = (struct ScriptNodeRecord * )&sample_rec;
								}
//	KPrintF("Send stop to other2 %s\n",sample_rec.filename);
								PutMsg(Port_OtherXapp,(struct Message *)Msg_XappDial);
							}
							else
							{
								set_sound_vars( &si, &sample_rec );
								B_Run = TRUE;
							}
						}
						else
						{
							if( sample_rec.action == SAMPLE_PLAY )	// the other xapp is ready 
								{
									Port_Xapp = (struct MsgPort *)CreatePort( portname, 0 );
									Sig_XtoX = 1 << Port_Xapp->mp_SigBit;
#if _PRINTF
	sprintf(tt,"%x",Sig_XtoX );
	KPrintF("Set xtox %s\n",tt );
#endif
									sigs |= Sig_XtoX;
									B_Run = TRUE;
								}
						}
						Permit();
						Msg_RSampleDial->pd_Cmd = DCI_CHILDRUNS;                 
						break;                          
					case DCC_DOHOLD:
						Msg_RSampleDial->pd_Cmd = DCI_CHILDHOLDS;                 
						break;
					case DCC_DOTERM:
#if _PRINTF
	KPrintF("Got doterm\n");
#endif
						Msg_RSampleDial->pd_Cmd = DCI_CHILDTERM;
						B_Remove = TRUE;
						B_Stop = TRUE;
						B_Run = FALSE;
						B_ReInit = FALSE;
						break;
					case DCC_DOEASYTERM:
						Msg_RSampleDial->pd_Cmd = DCI_CHILDEASYTERM;
//						Msg_RSampleDial->pd_Cmd = DCI_IGNORE;
						B_Remove = TRUE;
						B_Stop = TRUE;
						B_Run = FALSE;
						B_ReInit = FALSE;
#if _PRINTF
		KPrintF("got easy term\n");
#endif
						break;
					case DCC_DOSTOP:
//						Msg_RSampleDial->pd_Cmd = DCI_CHILDREADY;
						Msg_RSampleDial->pd_Cmd = DCI_IGNORE;
//						B_Stop = TRUE;
//						B_Run = FALSE;
//						B_ReInit = FALSE;
#if _PRINTF
KPrintF("got do stop\n");
#endif
						break;
					case DCC_DOEASYSTOP:
#if _PRINTF
	KPrintF("got easy stop\n");
#endif
						Msg_RSampleDial->pd_Cmd = DCI_CHILDEASYSTOP;
						B_Stop = TRUE;
						B_Run = FALSE;
						B_ReInit = FALSE;
						break;
					default:
						// simply ignore what we don't understand
						Msg_RSampleDial->pd_Cmd = DCI_IGNORE;
						break;
				}
				ReplyMsg((struct Message *)Msg_RSampleDial);
			}
		}

		if(SigRecvd & SigR_CtoP)		// get a reply from our guide
			while( (Msg_RSampleDial = (PROCDIALOGUE *)GetMsg(RepP_WorkSample)) != NULL)
			{
				Msg_RSampleDial->pd_InUse = FALSE;
				if(Msg_RSampleDial->pd_Cmd == DCC_DOTERM)
				{
#if _PRINTF
	KPrintF("Remove set by reply ctop\n");
#endif
					B_Remove = TRUE;
				}
			}

		// see if there is a reply to a message we sent to a Xapp
		if(SigRecvd & SigR_XtoX)
		{
#if _PRINTF
			KPrintF("Got signal back %s\n",sample_rec.filename );
#endif
			// Get all messages for this port
			while( (Msg_RSampleDial = (PROCDIALOGUE *)GetMsg(RepP_Xapp)) != NULL);

			if( sample_rec.action == SAMPLE_PLAY )	// the other xapp is ready 
			{
				if( (Port_Xapp = (struct MsgPort *)CreatePort( portname ,0)) != NULL)
				{
					Sig_XtoX = 1 << Port_Xapp->mp_SigBit;
					sigs |= Sig_XtoX;
						set_sound_vars( &si, &sample_rec );
						B_Run = TRUE;				// see what action you need to take
				}
			}
			else
				B_Run = FALSE;
		}

		if( B_Stop )								// Our guide wants us to stop playing
		{	
			if( B_Playing )						// if playing stop the sound
			{
				if( lerr )
				{
					exit_sound( &si );
					free_chipmem( &si );
					SetTaskPri( task,oldpri );
				}

				B_Playing = FALSE;
				B_Setup = FALSE;
			}
			if(Port_Xapp != NULL)		
			{
				DeletePort(Port_Xapp);
				Port_Xapp = NULL;
				Sig_XtoX = 0;	
			}
			B_Stop = FALSE;
		}

		if(SigRecvd & Sig_XtoX)		// see if there is a message from another Xapp
		{

#if _PRINTF
KPrintF("got XtoX signal\n");
#endif
			B_Stop = FALSE;
			// Get all messages for this port
			while( (Port_Xapp != NULL )&&( Msg_RXappDial = (PROCDIALOGUE *)GetMsg(Port_Xapp)) != NULL )
			{
				Msg_RXappDial->pd_ChildPI = ThisPI;	
				if( Msg_RXappDial->pd_Cmd == DCC_DOTERM )	// Lets see what we've got
				{
					Msg_RXappDial->pd_Cmd = DCI_CHILDTERM;       

// KPrintF("Receive stop,%s\n",sample_rec.filename);
					if( B_Playing )	// Another sample xapp wants us to stop playing
					{
						exit_sound( &si );
						free_chipmem( &si );
						SetTaskPri( task,oldpri );
						B_Playing = FALSE;
						B_Setup = FALSE;
						if( !ThisPI->pi_Preload )
						{
							if( !Msg_SampleDial->pd_InUse )
							{
								Msg_SampleDial->pd_ChildPI = ThisPI;
								Msg_SampleDial->pd_InUse = TRUE;
								Msg_SampleDial->pd_Cmd = DCI_CHILDTERM;
								PutMsg(ThisPI->pi_Port_CtoP,(struct Message *)Msg_SampleDial);
							}
						}
					}
					B_Playing = FALSE;
					B_Stop = FALSE;

					if(Port_Xapp != NULL)		
					{
//				KPrintF("Remove port %s\n",sample_rec.filename);
						DeletePort(Port_Xapp);
						Port_Xapp = NULL;
						Sig_XtoX = 0;	
					}
				}
				else			// should be a DCC_IGNORE saying fade, setcol etc.
				{
					if( Msg_RXappDial->pd_Cmd == DCC_IGNORE )	// Lets see what we've got
					{
						ex_sr = ( struct Sample_record *)Msg_RXappDial->pd_Luggage.lu_SNR;
// copy the data to local storage
						ex_sample_rec.action = ex_sr->action;
						ex_sample_rec.fadeOut = ex_sr->fadeOut;
						ex_sample_rec.fadeIn = ex_sr->fadeIn;
						ex_sample_rec.setVolume = ex_sr->setVolume;
						ex_sample_rec.balance = 0;
						B_Ex = TRUE;
						Msg_RXappDial->pd_Cmd = DCI_IGNORE;
					}
				}

				ReplyMsg((struct Message *)Msg_RXappDial);
			}
		}

		if( B_Ex )
		{
			if( B_Fading )		// if you are fading now remove the fade
			{
				remove_fade( &si );
				sigs &= ~si.fadesig;
				B_Fading = FALSE;
			}
			switch( ex_sample_rec.action )
			{
				case SAMPLE_FADEOUT:
//					KPrintF("Received fade out\n");
// first restore original volume
					set_volume( &si, sample_rec.volume, sample_rec.balance );
					fade_it_out( &si, ex_sample_rec.fadeOut );
					sigs |= si.fadesig;
					B_Fading = TRUE;
					break;
				case SAMPLE_FADEIN:
//					KPrintF("Received fade in\n");
// first restore original volume
					set_volume( &si, sample_rec.volume, sample_rec.balance );
					fade_it_in( &si, ex_sample_rec.fadeOut );
					si.vol_right = si.vol_temp_right;
					si.vol_left = si.vol_temp_left;
					si.vol_temp_left = 0;
					si.vol_temp_right = 0;
					sigs |= si.fadesig;
					B_Fading = TRUE;
					break;
				case SAMPLE_SETVOL:
//					KPrintF("Received set volume\n");
					set_volume( &si, ex_sample_rec.setVolume, ex_sample_rec.balance );
					change_sound( &si );
					break;
			}
			B_Ex = FALSE;
		}

		if( B_Remove )
		{
#if _PRINTF
KPrintF("Removing\n");
#endif
			if( Port_Xapp )
			{
				Forbid();
				// reply to outstanding requests from another Xapp
				while( (Msg_RXappDial = (PROCDIALOGUE *)GetMsg(Port_Xapp)) != NULL)
				{
#if _PRINTF
	KPrintF("Reply to rxapdial\n");
#endif
					Msg_RXappDial->pd_Cmd = DCI_CHILDTERM;       
					Msg_RXappDial->pd_ChildPI = ThisPI;	
					ReplyMsg((struct Message *)Msg_RXappDial);
				}	
				DeletePort( Port_Xapp );
				Port_Xapp = NULL;
				Sig_XtoX = 0;	
				Permit();
			}

			// Wait till our guide has processed all our replies
			B_Term = TRUE;
			for( i = 0; i < DIAL_MAXPTOC; i++ )
				if( ( ( PROCDIALOGUE * )ThisPI->pi_PtoCDial[ i ] )->pd_InUse )
				{
					B_Term = FALSE;
#if _PRINTF
					KPrintF("ptocdial in use\n");
#endif
				}

			if( Msg_SampleDial->pd_InUse )
			{
#if _PRINTF
			KPrintF("sampledial in use\n");
#endif
				B_Term = FALSE;
			}

#if _PRINTF
	if( B_Term )
KPrintF("Set bterm to True\n");
	else
KPrintF("Set bterm to False\n");
#endif
		}
		else
		{
			if( B_Run )
			{
				if( lerr && B_Playing)				// remove the sample played now
				{
					exit_sound( &si );
					SetTaskPri( task,oldpri );
					reset_sound( &si );
					set_volume( &si, sample_rec.volume, sample_rec.balance );
				}

				B_Playing = FALSE;

				// last attempt to read the file, if not possible then don't play
				if( lerr == 0 )
					lerr = loadsoundfile( &si, sample_rec.filename, TRUE, 0L );

				if( lerr && get_chipmem( &si ) )
				{
					oldpri = SetTaskPri(task, 21 );
					if( sample_rec.playFadeIn > 0 )
					{	
						fade_it_in( &si, sample_rec.playFadeIn );
						B_Fading = TRUE;

						play_sound( &si );
						si.vol_right = si.vol_temp_right;
						si.vol_left = si.vol_temp_left;
						si.vol_temp_left = 0;
						si.vol_temp_right = 0;
						sigs |= si.fadesig;
					}
					else
					{
//		KPrintF("Oke ik ga draaien\n");
							set_sound_vars( &si, &sample_rec );
							reset_sound( &si );
//						set_volume( &si, sample_rec.volume, sample_rec.balance );
							play_sound( &si );
					}
					B_Playing = TRUE;
					sigs |= si.audiosig;
					B_Last = FALSE;
				}
				else
				{
					B_Stop= TRUE;
					B_Remove = TRUE;
				}
				B_Run = FALSE;
			}
			else
			{
				if(B_ReInit)
				{
					if( lerr == 0 )
						lerr = loadsoundfile( &si, sample_rec.filename, TRUE, 0L );
	
					if( !Msg_SampleDial->pd_InUse )
					{
						B_ReInit = FALSE;
						Msg_SampleDial->pd_ChildPI = ThisPI;
						Msg_SampleDial->pd_InUse = TRUE;
						Msg_SampleDial->pd_Cmd = DCI_CHILDREADY;
						PutMsg(ThisPI->pi_Port_CtoP,(struct Message *)Msg_SampleDial);
					}
				}
			}
		}
	}

//#if _PRINTF

	KPrintF("Ending SAMPLE [%s]\n",ThisPI->pi_Arguments.ar_Worker.aw_Name);

//#endif
	if( B_Fading )
		remove_fade( &si );
	if( B_Playing )
	{
		exit_sound( &si );
		SetTaskPri( task,oldpri );
	}

	if( lerr )
		freesound( &si );

	MLMMU_FreeMem( Msg_XappDial );
	MLMMU_FreeMem( Msg_SampleDial );
	CloseLibrary(MLMMULibBase);
	DeletePort(RepP_WorkSample);
	if(Port_Xapp != NULL)
		DeletePort(Port_Xapp);
	DeletePort(RepP_Xapp);
	ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;
	return;
}

/******** E O F ********/
