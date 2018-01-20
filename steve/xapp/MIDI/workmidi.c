//	File		:	WorkMidi.c
//	Uses		:	camd.h geninitmidi.c 
//	Date		:	? / 28-04-993
//	Author	:	S. Vanderhorst ( Music XaPP ) adapted Midi XaPP C.Lieshout
//				:	( The Midi play routines ) Dan Baker, Commodore Business Machines
//	Desc.		:	Play a Midi file
//

#include "nb:pre.h"

#include <stdio.h>
#include <string.h>
#include <workbench/startup.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>
#include <proto/exec.h>
#include <pragmas/exec_pragmas.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "demo:gen/general_protos.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

#include <stdlib.h>
#include <stdio.h>
#include "midi_play.h"

void XappSetup(PROCESSINFO *ThisPI);
void GetExtraData(PROCESSINFO *ThisPI, STRPTR fullPath, int *action);

#define _PRINTF FALSE
#define MIDIPORTNAME "Port_MLMidi"

/*************************************************
*Func : 
*in   : Argv -> Ptr to PROCESSINFO.pi_Startup
*       More info in geninit.c
*out  : -
*/
void main( int argc, char *argv[] )
{
	MLSYSTEM			*MLSystem;	
	PROCDIALOGUE	*Msg_XappDial,		// Dialogue to be send to an already active Xapp
						*Msg_RXappDial,	// Dialogue received by another Xapp
						*Msg_MidiDial,		// Our dialogue 
						*Msg_RMidiDial;	// Dialogue when our guide replies
                  	             	// This is actually a copy of the ptr we sent.  
												// Also: dialogue from our parent
	PROCESSINFO		*ThisPI;				// ptr to this processinfo blk 
												// (as used in our parent's list)
	struct MsgPort	*Port_Xapp,			// Inter-Xapp communication receive port
						*Port_OtherXapp,	// Port of older Xapp
						*RepP_Xapp,			// Inter-Xapp communication reply port
						*RepP_WorkMidi;	// Reply port for our parent when 
												// replying to our messages

	ULONG				Sig_PtoC,			// A parent to child signal
						SigR_CtoP,			// A reply to a msg we send to our parent

				/* Following two Signals are used to get and reply to messages
				   send by this Xapp when started twice. */

						Sig_XtoX,			// A signal from an equal Xapp
						SigR_XtoX,			// Reply from an equal Xapp

						SigRecvd;			// Signals received
	int				i;
	BOOL				B_ReInit,			// if TRUE, re-initialise data
						B_Term,				// If TRUE, we are free to terminate
						B_Run,			
						B_Remove,			// If True, our guide wants us to clean up
						B_Stop;				// if True, another xapp wants us to stop
	char 				FileName[300];
	SMFInfo			smfi;
	int				MidiErr = 0, action;
	struct Library *MLMMULibBase = NULL;
	int alrun=0;

	smfi.release = 0;

	// Get our PROCESSINFO base ptr
	if( (ThisPI = (PROCESSINFO *)ml_FindBaseAddr(argc, argv)) == NULL)
		return;


	if(ThisPI->pi_Arguments.ar_Worker.aw_Origin == ORG_SCRIPTEDITOR)
	{
		XappSetup(ThisPI);
		ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;
		return;
	}


	GetExtraData(ThisPI, FileName, &action);	// NEW NEW NEW !!!!
								// action --> 0=Wait, 1=Loop, 2=Stop

	if( action != 2 )
		init_smfi( &smfi );

	MLSystem = ThisPI->pi_Arguments.ar_Worker.aw_MLSystem;

	ThisPI->pi_Arguments.ar_RetErr = ERR_WORKER;		// Return a general error 

	if( ( MLMMULibBase = (struct Library *)OpenLibrary("mpmmu.library",0)) == NULL)
		return;

	// Create a reply port for your guide, needs not to be public cause its base
	// ptr is passed on to the guide when sending a message.
	if( (RepP_WorkMidi = (struct MsgPort *)CreatePort(0,0)) == NULL)
	{
		CloseLibrary(MLMMULibBase);
		return;
	}

	// Make a Dialogue 
	if( (Msg_MidiDial = (PROCDIALOGUE *)MLMMU_AllocMem(sizeof(PROCDIALOGUE), 
														MEMF_PUBLIC|MEMF_CLEAR, NULL )) == NULL)
	{
		CloseLibrary(MLMMULibBase);
		DeletePort(RepP_WorkMidi);
		return;
	}

	// Set up the Dialogue message
	Msg_MidiDial->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
	Msg_MidiDial->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);
	Msg_MidiDial->pd_Msg.mn_ReplyPort = RepP_WorkMidi;
	Msg_MidiDial->pd_ChildPI = ThisPI;
	Msg_MidiDial->pd_InUse = TRUE;
	Msg_MidiDial->pd_Cmd = DCI_CHILDREADY;

	SigR_CtoP  		=   1 << RepP_WorkMidi->mp_SigBit;
	Sig_PtoC     	=   1 << ThisPI->pi_Port_PtoC->mp_SigBit;
	PutMsg(ThisPI->pi_Port_CtoP,(struct Message *)Msg_MidiDial);

	// Make A dialogue to send to other Xapps  
	if( (Msg_XappDial = (PROCDIALOGUE *)MLMMU_AllocMem(sizeof(PROCDIALOGUE), 
														MEMF_PUBLIC|MEMF_CLEAR, NULL )) == NULL)
	{
		MLMMU_FreeMem( Msg_MidiDial );
		CloseLibrary(MLMMULibBase);
		DeletePort(RepP_WorkMidi);
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

	if( action != 2 )
		MidiErr = read_and_evaluate( FileName, &smfi );

	B_Stop = FALSE;
	B_ReInit = FALSE;
	B_Run = FALSE;
	B_Term = FALSE;
	B_Remove = FALSE;
	while(!B_Term)
	{
		SigRecvd = Wait(Sig_XtoX | SigR_XtoX | Sig_PtoC | SigR_CtoP | SIGF_ABORT);

		if(SigRecvd & SIGF_ABORT)
			break;
        
//		printf("Signal %x\n",SigRecvd );
		if(SigRecvd & Sig_PtoC)
		{
			// Our parent has something to say to us
			while( (Msg_RMidiDial = (PROCDIALOGUE *)GetMsg(ThisPI->pi_Port_PtoC)) != NULL)
			{
				Msg_RMidiDial->pd_ChildPI = ThisPI;	
				switch(Msg_RMidiDial->pd_Cmd)
				{
					case DCC_DOPREPARE:
	//printf("DOprep\n");
						Msg_RMidiDial->pd_Cmd = DCI_CHILDPREPARES;                 
						B_ReInit = TRUE;
						break;                          
					case DCC_DORUN:
	//printf("DO_run\n");
						// Either start or re-run from pause
						Forbid();
						if( (Port_OtherXapp = (struct MsgPort *)FindPort(MIDIPORTNAME)) != NULL)
						{
							if(Port_OtherXapp != Port_Xapp)
							{
//								printf("Send stop to other xapp\n");
								Msg_XappDial->pd_Cmd = DCC_DOTERM;
								PutMsg(Port_OtherXapp,(struct Message *)Msg_XappDial);
							}
							else
								B_Run = TRUE;
						}
						else
						{
							if( action != 2 )	// when action is stop other
													// you don't want the port
							{
							   Port_Xapp = (struct MsgPort *)CreatePort(MIDIPORTNAME,0);
							   Sig_XtoX = 1 << Port_Xapp->mp_SigBit;
								B_Run = TRUE;
							}
						}
						Permit();
						Msg_RMidiDial->pd_Cmd = DCI_CHILDRUNS;                 
						break;                          
					case DCC_DOHOLD:
						Msg_RMidiDial->pd_Cmd = DCI_CHILDHOLDS;                 
						break;
					case DCC_DOTERM:
						Msg_RMidiDial->pd_Cmd = DCI_CHILDTERM;
						B_Remove = TRUE;
						B_Stop = TRUE;
						B_Run = FALSE;
						B_ReInit = FALSE;
						break;
					case DCC_DOEASYTERM:
						Msg_RMidiDial->pd_Cmd = DCI_CHILDEASYTERM;
						B_Remove = TRUE;
						B_Stop = TRUE;
						B_Run = FALSE;
						B_ReInit = FALSE;
						break;
					case DCC_DOSTOP:
//						Msg_RMidiDial->pd_Cmd = DCI_CHILDREADY;
						Msg_RMidiDial->pd_Cmd = DCI_IGNORE;
//						B_Stop = TRUE;
//						B_Run = FALSE;
//						B_ReInit = FALSE;
						break;
					case DCC_DOEASYSTOP:
						Msg_RMidiDial->pd_Cmd = DCI_CHILDEASYSTOP;
						B_Stop = TRUE;
						B_Run = FALSE;
						B_ReInit = FALSE;
						break;
					default:
						// simply ignore what we don't understand
						Msg_RMidiDial->pd_Cmd = DCI_IGNORE;
						break;
				}
				ReplyMsg((struct Message *)Msg_RMidiDial);
			}
		}

		// get a reply from our guide
		if(SigRecvd & SigR_CtoP)
			while( (Msg_RMidiDial = (PROCDIALOGUE *)GetMsg(RepP_WorkMidi)) != NULL)
			{
				Msg_RMidiDial->pd_InUse = FALSE;
				if(Msg_RMidiDial->pd_Cmd == DCC_DOTERM)
					B_Remove = TRUE;
			}

		// see if there is a reply to a message we sent to a Xapp
		if(SigRecvd & SigR_XtoX)
		{
			// Get all messages for this port
			while( (Msg_RMidiDial = (PROCDIALOGUE *)GetMsg(RepP_Xapp)) != NULL);
			if( action != 2 )		// not stop other
			{
				if( (Port_Xapp = (struct MsgPort *)CreatePort(MIDIPORTNAME,0)) != NULL)
				{
//					printf("Got go from other xapp\n");
					Sig_XtoX = 1 << Port_Xapp->mp_SigBit;
					B_Run = TRUE;
				}
			}
			else
				B_Run = FALSE;		// in fact you've run already
		}

		// see if there is a message from another Xapp
		if( SigRecvd & Sig_XtoX )
		{
//			printf("Got xtox\n");
			B_Stop = FALSE;
			// Get all messages for this port
			while( (Port_Xapp != NULL ) && (Msg_RXappDial = (PROCDIALOGUE *)GetMsg(Port_Xapp)) != NULL)
			{
				Msg_RXappDial->pd_ChildPI = ThisPI;	
				if(Msg_RXappDial->pd_Cmd == DCC_DOTERM)	// Lets see what we've got
				{
					Msg_RXappDial->pd_Cmd = DCI_CHILDTERM;       
					B_Stop = FALSE;
					if(Port_Xapp != NULL)		
					{
//						printf("Deleting port\n");
						DeletePort(Port_Xapp);
						Port_Xapp = NULL;
						Sig_XtoX = 0;	
					}
				}
				ReplyMsg((struct Message *)Msg_RXappDial);
			}
		}

		if(B_Stop)
		{	
			if(Port_Xapp != NULL)			// Our guide wants us to stop playing
			{
//				printf("Delete port 1\n");
				DeletePort(Port_Xapp);
				Port_Xapp = NULL;
				Sig_XtoX = 0;	
			}
			B_Stop = FALSE;
		}

		if(B_Remove)
		{
			if(Port_Xapp)
			{
				Forbid();
				// reply to outstanding requests from another Xapp
				while( (Msg_RXappDial = (PROCDIALOGUE *)GetMsg(Port_Xapp)) != NULL)
				{
					Msg_RXappDial->pd_Cmd = DCI_CHILDTERM;       
					Msg_RXappDial->pd_ChildPI = ThisPI;	
					ReplyMsg((struct Message *)Msg_RXappDial);
				}	
				DeletePort(Port_Xapp);
				Port_Xapp = NULL;
				Sig_XtoX = 0;	
				Permit();
			}

			// Wait till our guide has processed all our replies
			B_Term = TRUE;
			for(i = 0; i < DIAL_MAXPTOC; i++)
				if(((PROCDIALOGUE *)ThisPI->pi_PtoCDial[i])->pd_InUse)
			B_Term = FALSE;
			if(Msg_MidiDial->pd_InUse)
				B_Term = FALSE;
		}
		else
		{
			if( B_Run ) //  && alrun==0 )
			{
				// last attempt to read the file, if not possible then don't play
				if( MidiErr != 0 )
					MidiErr = read_and_evaluate( FileName, &smfi );

				if( MidiErr == 0 )
				{
					smfi.action = ( WORD )action;
					smfi.mport_ptoc = ThisPI->pi_Port_PtoC;
					smfi.sig_xtox = Sig_XtoX;
					smfi.sig_ptoc = Sig_PtoC;
					smfi.mainsignal = Sig_XtoX | Sig_PtoC;

					play_midi( &smfi );

					alrun = 1;
				}	
				B_Run = FALSE;
			}

			if( B_ReInit )
			{
//				printf("A re init ontvangen\n");

				if( MidiErr != 0 )
					MidiErr = read_and_evaluate( FileName, &smfi );

				if(!Msg_MidiDial->pd_InUse)
				{
					B_ReInit = FALSE;
					Msg_MidiDial->pd_ChildPI = ThisPI;
					Msg_MidiDial->pd_InUse = TRUE;
					Msg_MidiDial->pd_Cmd = DCI_CHILDREADY;
					PutMsg(ThisPI->pi_Port_CtoP,(struct Message *)Msg_MidiDial);
				}
			}
		}
	}

	if( smfi.release )					// if not done release bases etc.
		release( &smfi, 0 );

	MLMMU_FreeMem( Msg_XappDial );
	MLMMU_FreeMem( Msg_MidiDial );
	CloseLibrary(MLMMULibBase);
	DeletePort(RepP_WorkMidi);
	if(Port_Xapp != NULL)
		DeletePort(Port_Xapp);
	DeletePort(RepP_Xapp);
	ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;
	return;
}

/******** E O F ********/
