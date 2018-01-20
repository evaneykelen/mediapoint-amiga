/******************************************************
*Desc : Perform a ScriptTalk "Resource" command
*		This routine will either wait for the command
*		to be finished or  
* <!> : This module is resident and re-entrant
*		Compile without -b1 and without -y options
*		Link with cres.o in stead of c.o
*		Also compile umain.c : lc -b1 umain 
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <workbench/startup.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>
#include <proto/dos.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "nb:parser.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
//#include "external.h"
#include "demo:gen/general_protos.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"
#include "iff_fsound.h"
#include "sample.h"
#include "loadsamp.h"
#include "protos.h"
#include "nb:newdefines.h"
#include "structs.h"
#include <dos/dostags.h>

#define _PRINTF FALSE
#define _PRINTDEB FALSE

void XappSetup(PROCESSINFO *ThisPI);
extern void GetExtraData(PROCESSINFO *ThisPI, char *path, int *mode);
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
			sinfo->loops = sr->loops-2;
			sinfo->type |= SI_LOOPING;
		}
		else
		{
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

/*************************************************
*Func : Display the par: input
*in   : Argv -> Ptr to PROCESSINFO.pi_Startup
*out  : -
*/
void main( argc, argv)
int argc;
char **argv;
{

	char	Command[300];
	int	ErrLoadVSC;	
	char	diskplay,channel,filter;

	PROCDIALOGUE	*Msg_VSCDial,	// Our dialogue 
						*Msg_RVSCDial;	// Our dialogue when our guide replies

	MLSYSTEM			*MLSystem;	
	PROCESSINFO		*ThisPI;		// ptr to this processinfo blk (as used in our parent's list)

	struct MsgPort	*RepP_WorkVSC;// Reply port for our parent when replying to our messages

	ULONG		Sig_PtoC,		// A parent to child signal
				SigR_CtoP,		// A reply to a msg we send to our parent
				SigRecvd;		// Signals received
	int		i;
	BOOL		B_ReInit,		// if TRUE, re-initialise data
				B_Term,			// If TRUE, we are free to terminate
				B_Run,			
				B_Stop,
				B_Remove;		// If True, our guide wants us to clean up

	struct Library *MLMMULibBase;
	SoundInfo si;
	struct Sample_record sample_rec;
	struct TagItem TL[] =
	{
		{ SYS_Input, NULL	},
		{ SYS_Output, NULL },
		{ TAG_DONE, NULL }
	};
	

	// ADDED BY ERIK Sunday 13-Mar-94 12:08:19 because I saw that when play was
	// hit an enforcer hit was reported on line 332 (current line). I might have
	// come from an uninialized structure with garbage, resulting in a signal
	// that didn't exist.
	setmem(&si, sizeof(SoundInfo), 0);


	if( (ThisPI = (PROCESSINFO *)ml_FindBaseAddr(argc, argv)) == NULL)
		return;

	/**** this is called when showing the GUI ****/

	if(ThisPI->pi_Arguments.ar_Worker.aw_Origin == ORG_SCRIPTEDITOR)
	{
		XappSetup(ThisPI);
		ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;
		return;
	}
	Init_Sample_rec( ThisPI, &sample_rec );

	MLSystem = ThisPI->pi_Arguments.ar_Worker.aw_MLSystem;

//	GetExtraData( ThisPI, FileName, &action );

	MLMMULibBase = NULL;
	Msg_VSCDial = NULL;
	RepP_WorkVSC = NULL;
	if(
		((MLMMULibBase = (struct Library *)OpenLibrary("mpmmu.library",0)) == NULL) ||
		((RepP_WorkVSC = (struct MsgPort *)CreatePort(0,0)) == NULL) ||
		((Msg_VSCDial = (PROCDIALOGUE *)MLMMU_AllocMem(sizeof(PROCDIALOGUE), MEMF_PUBLIC|MEMF_CLEAR,NULL)) == NULL)
	  )
	{
		MLMMU_FreeMem(Msg_VSCDial);
		if(RepP_WorkVSC)
			DeletePort(RepP_WorkVSC);
		if(MLMMULibBase)
			CloseLibrary(MLMMULibBase);
		ThisPI->pi_Arguments.ar_RetErr = ERR_WORKER;
		return;
	}

	ErrLoadVSC = TRUE;

	// Set up the Dialogue message
	Msg_VSCDial->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
	Msg_VSCDial->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);
	Msg_VSCDial->pd_Msg.mn_ReplyPort = RepP_WorkVSC;

	// Our guide will reply to us when we must start
	SigR_CtoP = 1 << RepP_WorkVSC->mp_SigBit;
	Sig_PtoC = 1 << ThisPI->pi_Port_PtoC->mp_SigBit;

	// Send a message to the guide to indicate we are ready to start
	SendDialogue(Msg_VSCDial,ThisPI,DCI_CHILDREADY);

	// main 	
	B_ReInit = FALSE;
	B_Run = FALSE;
	B_Term = FALSE;
	B_Remove = FALSE;
	B_Stop = FALSE;
	while(!B_Term)
	{
		SigRecvd = Wait(Sig_PtoC | SigR_CtoP | SIGF_ABORT);

		if(SigRecvd & Sig_PtoC)
		{
			if( (Msg_RVSCDial = (PROCDIALOGUE *)GetMsg(ThisPI->pi_Port_PtoC)) != NULL)
			{
				Msg_RVSCDial->pd_ChildPI = ThisPI;
				switch(Msg_RVSCDial->pd_Cmd)
				{
					case DCC_DOPREPARE:
							Msg_RVSCDial->pd_Cmd = DCI_CHILDPREPARES;	
							if(!B_Remove && !B_Term)
								B_ReInit = TRUE;
							break;
					case DCC_DORUN:
							if(!B_Remove && !B_Term)
								B_Run = TRUE;
							Msg_RVSCDial->pd_Cmd = DCI_CHILDRUNS;
							break;
					case DCC_DOTERM:
						Msg_RVSCDial->pd_Cmd = DCI_CHILDTERM;
							B_Remove = TRUE;
							B_Run = FALSE;
							break;
					case DCC_DOSTOP:
							Msg_RVSCDial->pd_Cmd = DCI_CHILDREADY;	
							B_Stop = TRUE;
							B_Run = FALSE;
							break;
					case DCC_DOEASYTERM:
							Msg_RVSCDial->pd_Cmd = DCI_CHILDEASYTERM;
							B_Remove = TRUE;
							B_Run = FALSE;
							break;
					case DCC_DOEASYSTOP:
							Msg_RVSCDial->pd_Cmd = DCI_CHILDEASYSTOP;	
							B_Stop = TRUE;
							B_Run = FALSE;
							break;
					default:
							// simply ignore what we don't understand
							Msg_RVSCDial->pd_Cmd = DCI_IGNORE;	
							break;
				}
				ReplyMsg((struct Message *)Msg_RVSCDial);
			}
		}

		// get a reply from our guide or the TRANSITION module
		if(SigRecvd & SigR_CtoP)
			while( (Msg_RVSCDial = (PROCDIALOGUE *)GetMsg(RepP_WorkVSC)) != NULL)
				Msg_RVSCDial->pd_InUse = FALSE;

		if(B_Stop)
		{
			B_Stop = FALSE;
		}

		if( (!B_Remove && !B_Term ) )
		{
			if(B_Run)
			{
				diskplay = 'm';

				if( sample_rec.playFromDisk == 1 )
					diskplay = 'd';
				
				channel = '1';
				if( sample_rec.track == 1 )
					channel = '2';

				filter = '0';
				if( sample_rec.filter == 1 )
					filter = '1';

				sprintf(Command,"run >nil: psamp %s %c %c %c %d",sample_rec.filename,
																diskplay,
																channel,
																filter,
																sample_rec.freq );

//				KPrintF("Execute [%s]\n",Command );
//				Execute( Command,0,0 );
			SystemTagList( Command,TL );
//				KPrintF("Ready Execute [%s]\n",Command );
				B_Run = FALSE;
			}

			if(B_ReInit)
			{
				B_ReInit = SendDialogue(Msg_VSCDial,ThisPI,DCI_CHILDREADY);
			}
		}

		if(B_Remove)
		{
			// wait till all dialogues used to send commands to us have been freed
			B_Term = TRUE;

			for(i = 0; i < DIAL_MAXPTOC; i++)
				if(((PROCDIALOGUE *)ThisPI->pi_PtoCDial[i])->pd_InUse)
					B_Term = FALSE;

		if(Msg_VSCDial->pd_InUse && (Msg_VSCDial->pd_Cmd == DCI_CHILDREADY))
				B_Term = FALSE;
		}

		// Check if there are still messages in the portlist
		// if so then signal ourself

		if( (struct List *)ThisPI->pi_Port_PtoC->mp_MsgList.lh_TailPred != &ThisPI->pi_Port_PtoC->mp_MsgList )
			Signal(&ThisPI->pi_Process->pr_Task, Sig_PtoC);
	}

	MLMMU_FreeMem(Msg_VSCDial);
	DeletePort(RepP_WorkVSC);
	CloseLibrary(MLMMULibBase);

	ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;
}
