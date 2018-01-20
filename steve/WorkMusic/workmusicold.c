//	File		:	workmusic.c
//	Uses		:	external.h mlmmu.h
//	Date		:	-92 ( 01-05-93 )
//	Author	:	S. Vanderhorst
//	Desc.		:	Play a music module
//
#include <stdio.h>
#include <string.h>
#include <workbench/startup.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>
#include <proto/exec.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>

#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "external.h"
#include	"gen:general.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

#include <stdlib.h>
#include <stdio.h>

#define _PRINTF FALSE

#define XAPPPORTNAME "Port_MLMusic"

#define MAKE_ID(a,b,c,d) ((a)<<24|(b)<<16|(c)<<8|(d))

#define MRK2 MAKE_ID('M', 'R', 'K', '2')
#define DSS  MAKE_ID('D', 'S', 'S', '!')
#define ST15 MAKE_ID('S', 'T', '1', '5')
#define TRAK MAKE_ID('T', 'R', 'A', 'K')
#define FC13 MAKE_ID('F', 'C', '1', '3')
#define FC14 MAKE_ID('F', 'C', '1', '4')
#define JAMC MAKE_ID('J', 'A', 'M', 'C')
#define SM20 MAKE_ID('S', 'M', '2', '0')

/*************************************************
*Func : 
*in   : Argv -> Ptr to PROCESSINFO.pi_Startup
*       More info in geninit.c
*out  : -
*/
void main( int argc, char *argv[] )
{
	MEMTAG			*memtag;
	MLSYSTEM			*MLSystem;	
	PROCDIALOGUE	*Msg_XappDial,		// Dialogue to be send to an already active Xapp
						*Msg_RXappDial,	// Dialogue received by another Xapp
						*Msg_MusicDial,	// Our dialogue 
						*Msg_RMusicDial;	// Dialogue when our guide replies
												// This is actually a copy of the ptr we sent.  
												// Also: dialogue from our parent
	PROCESSINFO		*ThisPI;				// ptr to this processinfo blk 
												// (as used in our parent's list)
	struct MsgPort	*Port_Xapp,			// Inter-Xapp communication receive port
						*Port_OtherXapp,	// Port of older Xapp
						*RepP_Xapp,			// Inter-Xapp communication reply port
						*RepP_WorkMusic;	// Reply port for our parent when 
												// replying to our messages

	ULONG				Sig_PtoC,			// A parent to child signal
						SigR_CtoP,			// A reply to a msg we send to our parent

					/* Following two Signals are used to get and reply to messages
						send by this Xapp when started twice. */

						Sig_XtoX,			// A signal from an equal Xapp
						SigR_XtoX,			// Reply from an equal Xapp
						SigRecvd;			// Signals received
	int 				i;
	BOOL 				B_ReInit,			// if TRUE, re-initialise data
						B_Term,				// If TRUE, we are free to terminate
						B_Run,			
						B_Remove,			// If True, our guide wants us to clean up
						B_Setup,				// if True, SetupPlayer succeeded
						B_Stop,				// if True, another xapp wants us to stop
						B_Playing;			// If True, PlayTune Succeeded
	char 				FilePath[300];
	APTR				MusicModule;
	ULONG 			type;
	BYTE				modType=-1;
	UBYTE 			*mod;
	struct Library	*MLMMULibBase = NULL;

	MusicModule = NULL;

	// Get our PROCESSINFO base ptr
	if( (ThisPI = (PROCESSINFO *)ml_FindBaseAddr(argc, argv)) == NULL)
		return;

	/******************************************************************/
	if(ThisPI->pi_Arguments.ar_Worker.aw_Origin == ORG_SCRIPTEDITOR)
	{
		B_Setup = SetupPlayer();
		if (B_Setup)
			return;

		MakeFullPath(ThisPI->pi_Arguments.ar_Worker.aw_Path,ThisPI->pi_Arguments.ar_Worker.aw_Name, FilePath);

//KPrintF("ReadModule [%s]\n",FilePath);

		mod = (BYTE *)ReadModule(FilePath);
		if (mod==NULL)
		{
//KPrintF("ReadModules failed\n");
			KillPlayer();	// ReadModule() failed --> unable to find/alloc/recognize file
			return;
		}

		type = *(mod+4)<<24;
		type |= *(mod+5)<<16;
		type |= *(mod+6)<<8;
		type |= *(mod+7);
		switch(type)
		{
			case MRK2: modType=1; break;
			case DSS : modType=2; break;
			case ST15: modType=3; break;
			case TRAK: modType=4; break;
			case FC13: modType=5; break;
			case FC14: modType=6; break;
			case JAMC: modType=7; break;
			case SM20: modType=8; break;
			default:   modType=-1; break;
		}
		ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[5] = modType;

//KPrintF("modtype = %d\n", modType);

		KillModule(mod);
		KillPlayer();
		return; // return file type in [5]
	}

	/******************************************************/

	{
	char stringetje[100];
	sprintf(	stringetje,"%d %d\n",
				ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[0],
				ThisPI->pi_Arguments.ar_Worker.aw_NumArgs[1] );
	KPrintF(stringetje);
	}

	MLSystem = ThisPI->pi_Arguments.ar_Worker.aw_MLSystem;

	ThisPI->pi_Arguments.ar_RetErr = ERR_WORKER;

	if( ( MLMMULibBase = (struct Library *)OpenLibrary("mpmmu.library",0)) == NULL)
		return;

	// Create a reply port for your guide, needs not to be public cause its base
	// ptr is passed on to the guide when sending a message.

	if( (RepP_WorkMusic = (struct MsgPort *)CreatePort(0,0)) == NULL)
	{
		CloseLibrary(MLMMULibBase);
		return;
	}
	// Make a Dialogue 
	if((Msg_MusicDial = (PROCDIALOGUE *)MLMMU_AllocMem(sizeof(PROCDIALOGUE), 
								MEMF_PUBLIC|MEMF_CLEAR,NULL ) ) == NULL)
	{
		CloseLibrary(MLMMULibBase);
		DeletePort(RepP_WorkMusic);
		return;
	}

#if 0
	Forbid();
	memtag = MLMMU_FindMem( ThisPI->pi_Name );
	if( memtag->mt_AppCnt >= 2 )				// Am I excisting somewhere else ?
	{
		Permit();
//		KPrintF("Music Already excist\n");
		MLMMU_FreeMem(Msg_MusicDial);
		CloseLibrary(MLMMULibBase);
		DeletePort(RepP_WorkMusic);
		return;
	}
	Permit();
#endif

	// Set up the Dialogue message
	Msg_MusicDial->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
	Msg_MusicDial->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);
	Msg_MusicDial->pd_Msg.mn_ReplyPort = RepP_WorkMusic;
	Msg_MusicDial->pd_ChildPI = ThisPI;
	Msg_MusicDial->pd_InUse = TRUE;
	Msg_MusicDial->pd_Cmd = DCI_CHILDREADY;

	SigR_CtoP  		=   1 << RepP_WorkMusic->mp_SigBit;
	Sig_PtoC     	=   1 << ThisPI->pi_Port_PtoC->mp_SigBit;
	PutMsg(ThisPI->pi_Port_CtoP,(struct Message *)Msg_MusicDial);

	// Make A dialogue to send to other Xapps  
	if( (Msg_XappDial = (PROCDIALOGUE *)MLMMU_AllocMem( sizeof(PROCDIALOGUE), 
								MEMF_PUBLIC|MEMF_CLEAR,NULL )) == NULL  )
	{
		MLMMU_FreeMem( Msg_MusicDial );
		CloseLibrary(MLMMULibBase);
		DeletePort(RepP_WorkMusic);
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

	// Load the module file

	MakeFullPath(ThisPI->pi_Arguments.ar_Worker.aw_Path,ThisPI->pi_Arguments.ar_Worker.aw_Name, FilePath);
	MusicModule = NULL;

	ObtainSemaphore(&MLSystem->ms_Sema_Music);
	MusicModule = ReadModule(FilePath);
	ReleaseSemaphore(&MLSystem->ms_Sema_Music);

	B_Stop = FALSE;
	B_Setup = FALSE;
	B_Playing = FALSE;
	B_ReInit = FALSE;
	B_Run = FALSE;
	B_Term = FALSE;
	B_Remove = FALSE;
	while(!B_Term)
	{
		SigRecvd = Wait(Sig_XtoX | SigR_XtoX | Sig_PtoC | SigR_CtoP | SIGF_ABORT);

		if(SigRecvd & SIGF_ABORT)
			break;
        
		if(SigRecvd & Sig_PtoC)
		{
			// Our parent has something to say to us
			while( (Msg_RMusicDial = (PROCDIALOGUE *)GetMsg(ThisPI->pi_Port_PtoC)) != NULL)
			{
				Msg_RMusicDial->pd_ChildPI = ThisPI;	
				switch(Msg_RMusicDial->pd_Cmd)
				{
					case DCC_DOPREPARE:
						Msg_RMusicDial->pd_Cmd = DCI_CHILDPREPARES;                 
						B_ReInit = TRUE;
						break;                          
					case DCC_DORUN:
						// Either start or re-run from pause
						Forbid();
						if( (Port_OtherXapp = (struct MsgPort *)FindPort(XAPPPORTNAME)) != NULL)
						{
							if(Port_OtherXapp != Port_Xapp)
							{
								Msg_XappDial->pd_Cmd = DCC_DOTERM;
								PutMsg(Port_OtherXapp,(struct Message *)Msg_XappDial);
							}
							else
								B_Run = TRUE;
						}
						else
						{
							Port_Xapp = (struct MsgPort *)CreatePort(XAPPPORTNAME,0);
							Sig_XtoX = 1 << Port_Xapp->mp_SigBit;
							B_Run = TRUE;
						}
						Permit();
						Msg_RMusicDial->pd_Cmd = DCI_CHILDRUNS;                 
						break;                          
					case DCC_DOHOLD:
						Msg_RMusicDial->pd_Cmd = DCI_CHILDHOLDS;                 
						break;
					case DCC_DOTERM:
						Msg_RMusicDial->pd_Cmd = DCI_CHILDTERM;
						B_Remove = TRUE;
						B_Stop = TRUE;
						B_Run = FALSE;
						B_ReInit = FALSE;
						break;
					case DCC_DOEASYTERM:
						Msg_RMusicDial->pd_Cmd = DCI_CHILDEASYTERM;
						B_Remove = TRUE;
						B_Stop = TRUE;
						B_Run = FALSE;
						B_ReInit = FALSE;
						break;
					case DCC_DOSTOP:
						Msg_RMusicDial->pd_Cmd = DCI_CHILDREADY;
						B_Stop = TRUE;
						B_Run = FALSE;
						B_ReInit = FALSE;
						break;
					case DCC_DOEASYSTOP:
						Msg_RMusicDial->pd_Cmd = DCI_CHILDEASYSTOP;
						B_Stop = TRUE;
						B_Run = FALSE;
						B_ReInit = FALSE;
						break;
					default:
						// simply ignore what we don't understand
						Msg_RMusicDial->pd_Cmd = DCI_IGNORE;
						break;
				}
				ReplyMsg((struct Message *)Msg_RMusicDial);
			}
		}

		if(SigRecvd & SigR_CtoP)		// get a reply from our guide
			while( (Msg_RMusicDial = (PROCDIALOGUE *)GetMsg(RepP_WorkMusic)) != NULL)
			{
				Msg_RMusicDial->pd_InUse = FALSE;
				if(Msg_RMusicDial->pd_Cmd == DCC_DOTERM)
					B_Remove = TRUE;
			}

		// see if there is a reply to a message we sent to a Xapp
		if(SigRecvd & SigR_XtoX)
		{
			// Get all messages for this port
			while( (Msg_RMusicDial = (PROCDIALOGUE *)GetMsg(RepP_Xapp)) != NULL);
				if( (Port_Xapp = (struct MsgPort *)CreatePort(XAPPPORTNAME,0)) != NULL)
				{
					Sig_XtoX = 1 << Port_Xapp->mp_SigBit;
					B_Run = TRUE;
				}
		}

		if(B_Stop)								// Our guide wants us to stop playing
		{	
			if(B_Playing)
			{
				if(MusicModule != NULL)
					StopTune(MusicModule);				// stop playing the tune
				B_Playing = FALSE;
				KillPlayer();								// Remove interrupts etc.
				if(MusicModule != NULL)
					KillModule(MusicModule);
				MusicModule = NULL;
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
			B_Stop = FALSE;
			// Get all messages for this port
			while( (Port_Xapp != NULL )&&( Msg_RXappDial = (PROCDIALOGUE *)GetMsg(Port_Xapp)) != NULL )
			{
				Msg_RXappDial->pd_ChildPI = ThisPI;	
				if(Msg_RXappDial->pd_Cmd == DCC_DOTERM)	// Lets see what we've got
				{
					Msg_RXappDial->pd_Cmd = DCI_CHILDTERM;       

					if(B_Playing)		// Another musicxapp wants up to stop playing
					{
						if(MusicModule != NULL)				// stop playing the tune
							StopTune(MusicModule);
						B_Playing = FALSE;
						KillPlayer();							// Remove interrupts etc.
						B_Setup = FALSE;
						if(!ThisPI->pi_Preload)
						{
							if(!Msg_MusicDial->pd_InUse)
							{
								Msg_MusicDial->pd_ChildPI = ThisPI;
								Msg_MusicDial->pd_InUse = TRUE;
								Msg_MusicDial->pd_Cmd = DCI_CHILDTERM;
								PutMsg(ThisPI->pi_Port_CtoP,(struct Message *)Msg_MusicDial);
							}
						}
					}

					if(MusicModule != NULL)
						KillModule(MusicModule);
					MusicModule = NULL;
					B_Stop = FALSE;

					if(Port_Xapp != NULL)		
					{
						DeletePort(Port_Xapp);
						Port_Xapp = NULL;
						Sig_XtoX = 0;	
					}
				}
				ReplyMsg((struct Message *)Msg_RXappDial);
			}
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
			if(Msg_MusicDial->pd_InUse)
				B_Term = FALSE;
		}
		else
		{
			if(B_Run)
			{
				// last attempt to read the file, if not possible then don't play
				if(MusicModule == NULL)
				{
					ObtainSemaphore(&MLSystem->ms_Sema_Music);
					MusicModule = ReadModule(FilePath);
					ReleaseSemaphore(&MLSystem->ms_Sema_Music);
				}

				if(MusicModule != NULL)
				{
					if(B_Playing)
					{	// We're already playing a tune, let's start again

						if(MusicModule != NULL)
							StopTune(MusicModule);
						B_Playing = FALSE;
						if(!(B_Playing = !PlayTune(MusicModule)))
						{	// couldn't play tune, remove player and module from memory
							KillModule(MusicModule);
							KillPlayer();
							MusicModule = NULL;
							B_Setup = FALSE;
						}
					}
					else
					{
						if((B_Setup = !SetupPlayer()))
						{
							if(!(B_Playing = !PlayTune(MusicModule)))
							{	// couldn't play tune, remove player and module from memory
								KillModule(MusicModule);
								KillPlayer();
								MusicModule = NULL;
								B_Setup = FALSE;
							}
						}
						else
						{	// couldn't setup the player, remove all
							KillModule(MusicModule);
							MusicModule = NULL;
						}
					}
				}	
				B_Run = FALSE;
			}

			if(B_ReInit)
			{
				if(MusicModule == NULL)
				{
					ObtainSemaphore(&MLSystem->ms_Sema_Music);
					MusicModule = ReadModule(FilePath);
					ReleaseSemaphore(&MLSystem->ms_Sema_Music);
				}	

				if(!Msg_MusicDial->pd_InUse)
				{
					B_ReInit = FALSE;
					Msg_MusicDial->pd_ChildPI = ThisPI;
					Msg_MusicDial->pd_InUse = TRUE;
					Msg_MusicDial->pd_Cmd = DCI_CHILDREADY;
					PutMsg(ThisPI->pi_Port_CtoP,(struct Message *)Msg_MusicDial);
				}
			}
		}
	}

	if(MusicModule != NULL)
		KillModule(MusicModule);

	MLMMU_FreeMem( Msg_XappDial );
	MLMMU_FreeMem( Msg_MusicDial );
	CloseLibrary(MLMMULibBase);
	DeletePort(RepP_WorkMusic);
	if(Port_Xapp != NULL)
		DeletePort(Port_Xapp);
	DeletePort(RepP_Xapp);
	ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;
	return;
}

/******** E O F ********/
