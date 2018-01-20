//	File		:	worklight.c
//	Uses		:	external.h mlmmu.h
//	Date		:	-92 ( 01-05-93 ) (10-07-93)
//	Author	:	S. Vanderhorst / C. Lieshout
//	Desc.		:	Use lightpen
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

#include "nb:capsdefines.h"
#include "nb:newdefines.h"

#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"

#include "gen:general.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

#include "lightpen.h"

#include <stdlib.h>
#include <stdio.h>

#define _PRINTF FALSE

#define XAPPPORTNAME1 "Port_Light"

void main( int argc, char *argv[] )
{

	MLSYSTEM			*MLSystem;	
	PROCDIALOGUE	*Msg_XappDial,		// Dialogue to be send to an already active Xapp
						*Msg_RXappDial,	// Dialogue received by another Xapp
						*Msg_LightDial,	// Our dialogue 
						*Msg_RLightDial;	// Dialogue when our guide replies
												// This is actually a copy of the ptr we sent.  
												// Also: dialogue from our parent
	PROCESSINFO		*ThisPI;				// ptr to this processinfo blk 
												// (as used in our parent's list)
	struct MsgPort	*Port_Xapp,			// Inter-Xapp communication receive port
						*Port_OtherXapp,	// Port of older Xapp
						*RepP_Xapp,			// Inter-Xapp communication reply port
						*RepP_WorkLight;	// Reply port for our parent when 
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

	// Get our PROCESSINFO base ptr
	if( (ThisPI = (PROCESSINFO *)ml_FindBaseAddr(argc, argv)) == NULL)
		return;

	//KPrintF("Starting SAMPLE [%s]\n",ThisPI->pi_Arguments.ar_Worker.aw_Name);

	if(ThisPI->pi_Arguments.ar_Worker.aw_Origin == ORG_SCRIPTEDITOR)
	{
		XappSetup(ThisPI);
		ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;
		return;
	}
//	else
//		// Place the data from the PI in the local sample structure
//		Init_Sample_rec( ThisPI, &sample_rec );

	portname = XAPPPORTNAME1;

	/******************************************************/

	MLSystem = ThisPI->pi_Arguments.ar_Worker.aw_MLSystem;

	ThisPI->pi_Arguments.ar_RetErr = ERR_WORKER;

	if( ( MLMMULibBase = (struct Library *)OpenLibrary("mpmmu.library",0)) == NULL)
		return;

	if( (RepP_WorkLight = (struct MsgPort *)CreatePort(0,0)) == NULL)
	{
		CloseLibrary(MLMMULibBase);
		return;
	}
	if((Msg_LightDial = (PROCDIALOGUE *)MLMMU_AllocMem(sizeof(PROCDIALOGUE), 
								MEMF_PUBLIC|MEMF_CLEAR,NULL ) ) == NULL)
	{
		CloseLibrary(MLMMULibBase);
		DeletePort(RepP_WorkLight);
		return;
	}

	// Set up the Dialogue message
	Msg_LightDial->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
	Msg_LightDial->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);
	Msg_LightDial->pd_Msg.mn_ReplyPort = RepP_WorkLight;
	Msg_LightDial->pd_ChildPI = ThisPI;
	Msg_LightDial->pd_InUse = TRUE;
	Msg_LightDial->pd_Cmd = DCI_CHILDREADY;

	SigR_CtoP  		=   1 << RepP_WorkLight->mp_SigBit;
	Sig_PtoC     	=   1 << ThisPI->pi_Port_PtoC->mp_SigBit;
	PutMsg(ThisPI->pi_Port_CtoP,(struct Message *)Msg_LightDial);

	// Make A dialogue to send to other Xapps  
		if( (Msg_XappDial = (PROCDIALOGUE *)MLMMU_AllocMem( sizeof(PROCDIALOGUE), 
								MEMF_PUBLIC|MEMF_CLEAR,NULL )) == NULL  )
	{
		MLMMU_FreeMem( Msg_LightDial );
		CloseLibrary(MLMMULibBase);
		DeletePort(RepP_WorkLight);
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

	task = FindTask( 0 );
	while( !B_Term )
	{
		//char tt[200];

		SigRecvd = Wait( sigs );
		if( SigRecvd & lp.signal )
		{

		}

		if(SigRecvd & SIGF_ABORT)
		{
			break;
		}
        
		if(SigRecvd & Sig_PtoC)
		{
#if _PRINTF
KPrintF("sig to ptoc\n");
#endif
			// Our parent has something to say to us
			while( (Msg_RLightDial = (PROCDIALOGUE *)GetMsg(ThisPI->pi_Port_PtoC)) != NULL)
			{
				Msg_RLightDial->pd_ChildPI = ThisPI;	
				switch(Msg_RLightDial->pd_Cmd)
				{
					case DCC_DOPREPARE:
						Msg_RLightDial->pd_Cmd = DCI_CHILDPREPARES;                 
						B_ReInit = TRUE;
						break;                          
					case DCC_DORUN:
//	KPrintF("Do run sample %s\n",sample_rec.filename);
						// Either start or re-run from pause
						Forbid();
						if( (Port_OtherXapp = (struct MsgPort *)FindPort( portname )) != NULL)
						{
							if(Port_OtherXapp != Port_Xapp)
								B_Run = FALSE;
							else
								B_Run = TRUE;
						}
						else
						{
							Port_Xapp = (struct MsgPort *)CreatePort( portname, 0 );
							Sig_XtoX = 1 << Port_Xapp->mp_SigBit;
							sigs |= Sig_XtoX;
							B_Run = TRUE;
						}
						Permit();
						Msg_RLightDial->pd_Cmd = DCI_CHILDRUNS;                 
						break;                          
					case DCC_DOHOLD:
						Msg_RLightDial->pd_Cmd = DCI_CHILDHOLDS;                 
						break;
					case DCC_DOTERM:
#if _PRINTF
	KPrintF("Got doterm\n");
#endif
						Msg_RLightDial->pd_Cmd = DCI_CHILDTERM;
						B_Remove = TRUE;
						B_Stop = TRUE;
						B_Run = FALSE;
						B_ReInit = FALSE;
						break;
					case DCC_DOEASYTERM:
						Msg_RLightDial->pd_Cmd = DCI_CHILDEASYTERM;
//						Msg_RLightDial->pd_Cmd = DCI_IGNORE;
						B_Remove = TRUE;
						B_Stop = TRUE;
						B_Run = FALSE;
						B_ReInit = FALSE;
#if _PRINTF
		KPrintF("got easy term\n");
#endif
						break;
					case DCC_DOSTOP:
//						Msg_RLightDial->pd_Cmd = DCI_CHILDREADY;
						Msg_RLightDial->pd_Cmd = DCI_IGNORE;
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
						Msg_RLightDial->pd_Cmd = DCI_CHILDEASYSTOP;
						B_Stop = TRUE;
						B_Run = FALSE;
						B_ReInit = FALSE;
						break;
					default:
						// simply ignore what we don't understand
						Msg_RLightDial->pd_Cmd = DCI_IGNORE;
						break;
				}
				ReplyMsg((struct Message *)Msg_RLightDial);
			}
		}

		if(SigRecvd & SigR_CtoP)		// get a reply from our guide
			while( (Msg_RLightDial = (PROCDIALOGUE *)GetMsg(RepP_WorkLight)) != NULL)
			{
				Msg_RLightDial->pd_InUse = FALSE;
				if(Msg_RLightDial->pd_Cmd == DCC_DOTERM)
				{
#if _PRINTF
	KPrintF("Remove set by reply ctop\n");
#endif
					B_Remove = TRUE;
				}
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

			if( Msg_LightDial->pd_InUse )
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

// start interrupts and set signals

				B_Run = FALSE;
			}
			else
			{
				if(B_ReInit)
				{
					if( lerr == 0 )
						lerr = loadsoundfile( &si, sample_rec.filename, TRUE, 0L );
	
					if( !Msg_LightDial->pd_InUse )
					{
						B_ReInit = FALSE;
						Msg_LightDial->pd_ChildPI = ThisPI;
						Msg_LightDial->pd_InUse = TRUE;
						Msg_LightDial->pd_Cmd = DCI_CHILDREADY;
						PutMsg(ThisPI->pi_Port_CtoP,(struct Message *)Msg_LightDial);
					}
				}
			}
		}
	}

//#if _PRINTF

	//KPrintF("Ending SAMPLE [%s]\n",ThisPI->pi_Arguments.ar_Worker.aw_Name);

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
	MLMMU_FreeMem( Msg_LightDial );
	CloseLibrary(MLMMULibBase);
	DeletePort(RepP_WorkLight);
	if(Port_Xapp != NULL)
		DeletePort(Port_Xapp);
	DeletePort(RepP_Xapp);
	ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;
	return;
}

/******** E O F ********/
