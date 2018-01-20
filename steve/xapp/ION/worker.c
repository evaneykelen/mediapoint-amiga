/******************************************************
*Desc : General Worker skeleton coding
*       (c) 1992-1001 Software Development
*       Coding is submitted to changes 
*
*       Performance description:
*       -MediaLink variable set up
*       -XaPP variable set up
*
*       -MediaLink init
*         -ProcessInfo retrieve
*         -Set up replyport
*         -Set up a dialogue    
*       -XaPP init
*         -Do your own stuff
*
*       -Send a message to our guide indicating we're ready to run
*       -Wait for reply
*         -The reply will be DCC_IGNORE, wait for a Dialogue from your guide
*
*       -Wait till a PROCDIALOGUE message comes in 
*         -If the command is DCC_DORUN then start your job
*         -If the command is DCC_DOTERM then clean up and return
*
*       -Do your job but keep scanning the Signals
*         -If needed, reply on a command from your guide and perform the order
*         -If the reply is DCC_IGNORE then go on with what you are doing
*         -If the command is DCC_DOHOLD then pause your job
*         -If the command is DCC_DORUN then restart your job
*         -If the command is DCC_DOTERM then clean up and return
*
*       -Processing done
*         -Send a request to your guide to commit suicide
*         -Wait for approval
*
*       -Clean up XaPP vars
*       -Clean up MediaLink vars
*
*       -return with return() command
*
*
* <!> : Never ever do anything on your own.
*       -Only use AllocSignal(-1) to allocate a signal
*       -Never perform any actions after you filed a DCI_CHILDTERM request.
*       -Never leave your program with the dos functions Exit()
*        but use the return() function.
*       -Put your error into the ThisPI->pi_Arguments.ar_RetErr field.
*        A zero (0) is used to indicate No Error.
*       -Always reply on a message from your guide.
*       -Wait for a reply from your guide when you send a infocommand to it.
*       -Never touch the mainlevel fields of your ProcessInfo structure!
*       -Don't change your task/process name!
*        Your nameptr is available in ThisPI->pi_Node.ln_Name.
*        Your Node type is always set to NT_WORKERPROC, don't touch it!
*       -Through ThisPI->pi_SNR you have direct access to the system SNR list
*        You may disrupt the entire system by changing the SNR list. 
*        Again, don't even look at it!
*
*
*   RE-ENTRANT/RESIDENT coding
*   When you want your XaPP to be a fast accessible Application you must
*   make it resident and re-entrant. This means: 
*   -No global references, allocate a structure with your vars in it!
*       struct VarMem
*       {
*           int Var1,
*               Var2;   etc.
*       }
*
*       /\Easy accessible vars, as if the were global 
*       #define Var1 MyMem->Var1
*       #define Var2 MyMem->Var2
*
*       void MyFunc( MyMem)
*       struct VarMem *MyMem;    /\Local var, preferably a register for fast access
*       {
*           Var1 += 30;          /\through MyMem we can reach Var1 again
*       }
*
*       int main( argc, argv)
*       int argc;
*       char **argv;
*       {
*         struct VarMem *MyMem;  /\Local var, preferably a register for fast access
*           /\ AllocMem directly after entering main()
*           MyMem = (struct VarMem *)AllocMem(sizeof(struct VarMem),MEMF_PUBLIC|MEMF_CLEAR)
*
*           /\ Do your own stuff
*           Var1 = 100;
*           MyFunc(MyMem);
*           Var2 = Var1 *3;
*
*           /\ FreeMem MUST be the final call
*           FreeMem(MyMem, sizeof(struct VarMem));
*       }
*   -No calls to the amiga.lib functions, use #pragmas instead!
*   -Using LatticeC 5.1 don't use the -b0(DATA=FAR) option but use -b1(DATA=NEAR)
*   -Using LatticeC 5.1 don't use the -y(SAVEDS)
*   -Compile your umain.c (path: latticeC/sources/) with lc -b1 umain
*   -Don't link with lib:c.o but link with lib:cres.o 
*   If all goes well, after linking your code file will have its Pure bit set.
*   No you copy your XaPP into the MediaLink:Tools directory were it will be
*   added automatically to the resident MediaLink segment list.
*
*	
*
*/

#include "nb:pre.h"

#include <workbench/startup.h>
#include <exec/types.h>
#include <libraries/dosextens.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
//#include "external.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"

#include <stdlib.h>
#include <stdio.h>

#define _PRINTF FALSE

extern int XappDoIt(PROCESSINFO *ThisPI);
extern void XappSetup(PROCESSINFO *ThisPI);
extern PROCESSINFO *ml_FindBaseAddr(int , char **);

/********************************************************
*Desc : This Bool is used for the following purpose:
*		Some Xapps are not allowed to be run twice in the
*		system mainly because of malfunctioning of the
*		output generated by such a Xapp. A Music player for
*		example should not be run twice or else the user
*		will hear two songs at the same time. For this reason
*		we introduced a little inter-Xapp communication channel.
*		A Xapp may set up such a channel if it doesn't exist already
*		and use it to send commands to equal named Xapps. 
*
*		example: A music player Xapp is setup and then waits for
*		a DCC_DORUN command from its guide. After it has received
*		then DORUN it will start playing until it has finished its
*		job OR until it receives a DCC_DOTERM from its guide.
*		Now a second music player Xapp is started (a new call to
*		the reentrant Xapp coding is performed) and the new Xapp will
*		preload all its data needed.
*		Then the new player gets a DCC_DORUN cmd while the old one
*		is still playing.
*		The old player is still performing its job but since
*		the new player must start the old one needs to be terminated.
*		 
*		Well: The coding associated with this Boolean terminates an
*		older player before starting its own job. This is an easy
*		way of avoiding player collisions.
*/
 
#define KILLOLD_XAPP FALSE

#define XAPPPORTNAME "CanonXaPP"

/*************************************************
*Func : 
*in   : Argv -> Ptr to PROCESSINFO.pi_Startup
*       More info in geninit.c
*out  : -
*/

void main(int argc, char **argv)
{

/*****************************************************************************/
// The vars below are used for interprocess communication.
// There definitions can be found pc_process.h

  PROCDIALOGUE  *Msg_XappDial,	// Dialogue to be send to an already active Xapp
  				*Msg_RXappDial,	// Dialogue received by another Xapp
				*Msg_WorkDial,  // Our dialogue 
                *Msg_RWorkDial; // Dialogue when our guide replies
                                // This is actually a copy of the ptr we sent.  
                                // Also: dialogue from our parent
  PROCESSINFO   *ThisPI;        // ptr to this processinfo blk 
                                // (as used in our parent's list)
  struct MsgPort *Port_Xapp,	// Inter-Xapp communication receive port
				 *Port_OtherXapp,	// Port of older Xapp
				 *RepP_Xapp,	// Inter-Xapp communication reply port
				 *RepP_Worker; 	// Reply port for our parent when 
                                // replying to our messages

  ULONG         Sig_PtoC,       // A parent to child signal
                SigR_CtoP,     	// A reply to a msg we send to our parent

				/* Following two Signals are used to get and reply to messages
				   send by this Xapp when started twice. */

/*				Sig_XtoX,		// A signal from an equal Xapp
				SigR_XtoX,		// Reply from an equal Xapp */


                SigRecvd;       // Signals received
/****************************************************************************/

  int 	Error;                
  BOOL 	B_ReInit,				// if TRUE, re-initialise data
		B_Term,					// If TRUE, we are free to terminate
		B_Run,			
		B_Remove;				// If True, our guide wants us to clean up

	Msg_XappDial	= NULL;
	Msg_RXappDial	= NULL;
	Msg_WorkDial	= NULL;
    Msg_RWorkDial	= NULL;
	ThisPI			= NULL;
	Port_Xapp		= NULL;
	Port_OtherXapp	= NULL;
	RepP_Xapp		= NULL;
	RepP_Worker		= NULL;

/****************************************************************************/
// Following is MediaLink set up coding, needed for communication with our
// guide or other Xapps

    // Get our PROCESSINFO base ptr
    if( (ThisPI = (PROCESSINFO *)ml_FindBaseAddr(argc, argv)) == NULL)
        return;

	if(ThisPI->pi_Arguments.ar_Worker.aw_Origin == ORG_SCRIPTEDITOR)
	{
		//printf("loaded by editor, go to setup screen\n");
		XappSetup(ThisPI);

    ThisPI->pi_Arguments.ar_RetErr = NO_ERROR;

		return;
	}

    // Create a reply port for your guide, needs not to be public cause its base
    // ptr is passed on to the guide when sending a message
    if( (RepP_Worker = (struct MsgPort *)CreatePort(0,0)) == NULL)
    {
        // Return a general error 
        ThisPI->pi_Arguments.ar_RetErr = ERR_WORKER;
        return;
    }

    // Make a Dialogue 
    if( (Msg_WorkDial = (PROCDIALOGUE *)AllocMem(sizeof(PROCDIALOGUE), 
                                            MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
    {
		if ( RepP_Worker != NULL )
	        DeletePort(RepP_Worker);
        // Return a general error 
        ThisPI->pi_Arguments.ar_RetErr = ERR_WORKER;
        return;
    }

#if KILLOLD_XAPP

	// Make A dialogue to send to other Xapps  
    if( (Msg_XappDial = (PROCDIALOGUE *)AllocMem(sizeof(PROCDIALOGUE), 
                                            MEMF_PUBLIC|MEMF_CLEAR)) == NULL)
    {
		if ( Msg_WorkDial != NULL )
	        FreeMem(Msg_WorkDial,sizeof(PROCDIALOGUE));
		if ( RepP_Worker != NULL )
	        DeletePort(RepP_Worker);
        // Return a general error 
        ThisPI->pi_Arguments.ar_RetErr = ERR_WORKER;
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

#endif

/*****************************************************************************/ 
// Init coding for your XaPP

    // You can do anything you want, set up interrupts, tasks or processes.
    // As long as you can reach them and let them perform your commands. Your
    // only reponsibility is not to loose you own children.

    // How to get your applications arguments ?
    /*
        tool name from            ThisPI->pi_Arguments.ar_Worker.aw_Path
        comment from              ThisPI->pi_Arguments.ar_Worker.aw_Name
        argument string from      ThisPI->pi_Arguments.ar_Worker.aw_ExtraData
        argument string size from ThisPI->pi_Arguments.ar_Worker.aw_ExtraDataSize
        run-mode from             ThisPI->pi_Arguments.ar_Worker.aw_NumArgs
                                  Either DEFER or CONTINUE
    */

    // Use the following example to initialise your coding.
    /*
    if( XaPPInit() == FALSE)
    {
        // clean up MediaLink structs

#if KILLOLD_XAPP
		if ( Msg_XappDial != NULL )
	        FreeMem(Msg_XappDial,sizeof(PROCDIALOGUE));
#endif

		if ( Msg_WorkDial != NULL )
	        FreeMem(Msg_WorkDial,sizeof(PROCDIALOGUE));

		if ( RepP_Worker != NULL )
	        DeletePort(RepP_Worker);

#if KILLOLD_XAPP
		if ( RepP_Xapp != NULL )
	        DeletePort(RepP_Xapp);
#endif

        ThisPI->pi_Arguments.ar_RetErr = ERR_WORKER;
        return;
    }
    */
/*****************************************************************************/
// Send init-ready message to your guide

    // Set up the Dialogue message
    Msg_WorkDial->pd_Msg.mn_Node.ln_Type = NT_MESSAGE;
    Msg_WorkDial->pd_Msg.mn_Length = sizeof(PROCDIALOGUE);
    // Attach the replyport to the message dialogue
    Msg_WorkDial->pd_Msg.mn_ReplyPort = RepP_Worker;
    Msg_WorkDial->pd_ChildPI = ThisPI;
    // Send a message to your guide to indicate you are ready to start
    Msg_WorkDial->pd_InUse = TRUE;
    Msg_WorkDial->pd_Cmd = DCI_CHILDREADY;

    // Reply signal from your guide
    SigR_CtoP  =   1 << RepP_Worker->mp_SigBit;
    // Command signal from your guide
    Sig_PtoC     =   1 << ThisPI->pi_Port_PtoC->mp_SigBit;
    PutMsg(ThisPI->pi_Port_CtoP,(struct Message *)Msg_WorkDial);


/****************************************************************************/
/* 
* Main program
* <!> This Main is for DEFER mode only
* If you want to do a CONTINUE, 
*   -Wait for DCC_DORUN
*   -Start your task
*   -File a DCI_CHILDTERM
*   -Wait for DCC_TERM reply
*   -Quit
* Your task will then be running on the background without having
* to do anything with MediaLink anymore. Considder this before
* you make your application support CONTINUE-mode. 
* See WorkDos.c for an example CONTINUE program
*/
    Error = NO_ERROR;

	B_ReInit = FALSE;
	B_Run = FALSE;
    B_Term = FALSE;
    B_Remove = FALSE;
	while(!B_Term)
    {
        // Add your own signals to this list 

#if KILLOLD_XAPP
        SigRecvd = Wait(Sig_XtoX | SigR_XtoX | Sig_PtoC | SigR_CtoP | SIGF_ABORT);
#else
        SigRecvd = Wait(Sig_PtoC | SigR_CtoP | SIGF_ABORT);
#endif

		if(SigRecvd & SIGF_ABORT)
			break;
        
        if(SigRecvd & Sig_PtoC)
        {
            // Our parent has something to say to us
            while( (Msg_RWorkDial = (PROCDIALOGUE *)GetMsg(ThisPI->pi_Port_PtoC)) != NULL)
            {
				Msg_RWorkDial->pd_ChildPI = ThisPI;	
                switch(Msg_RWorkDial->pd_Cmd)
                {
					case DCC_DOPREPARE:
                                Msg_RWorkDial->pd_Cmd = DCI_CHILDPREPARES;                 
								B_ReInit = TRUE;
                                break;                          
                    case DCC_DORUN:
                                // Either start or re-run from pause
#if KILLOLD_XAPP
								Forbid();
								if( (Port_OtherXapp = (struct MsgPort *)FindPort(XAPPPORTNAME)) != NULL)
								{
									// in case of a double DORUN receive, don't terminate
									// ourself.
									if(Port_OtherXapp != Port_Xapp)
									{
										// Terminate up older Xapp
										Msg_XappDial->pd_Cmd = DCC_DOTERM;
									    PutMsg(Port_OtherXapp,(struct Message *)Msg_XappDial);
									}										
									else
										B_Run = TRUE;
								}
								else
								{
									// We're the only one right now.
									// Make ourself known.
								    Port_Xapp = (struct MsgPort *)CreatePort(XAPPPORTNAME,0);
								    Sig_XtoX = 1 << Port_Xapp->mp_SigBit;
									B_Run = TRUE;
								}
								Permit();
#else
								B_Run = TRUE;
#endif
                                Msg_RWorkDial->pd_Cmd = DCI_CHILDRUNS;                 
								B_Run = TRUE;
                                break;                          
                    case DCC_DOHOLD:
                                // Hold your horses and wait till a 
                                // a DCC_DORUN arrives
                                Msg_RWorkDial->pd_Cmd = DCI_CHILDHOLDS;                 
                                break;
                    case DCC_DOTERM:
                                Msg_RWorkDial->pd_Cmd = DCI_CHILDTERM;                  
								B_Remove = TRUE;
                                break;
					case DCC_DOSTOP:
								Msg_RWorkDial->pd_Cmd = DCI_CHILDREADY;	
								break;
					default:
								// simply ignore what we don't understand
								Msg_RWorkDial->pd_Cmd = DCI_IGNORE;	
								break;
                }
                ReplyMsg((struct Message *)Msg_RWorkDial);
            }
        }

		// get a reply from our guide
		if(SigRecvd & SigR_CtoP)
		{
			while( (Msg_RWorkDial = (PROCDIALOGUE *)GetMsg(RepP_Worker)) != NULL)
			{
				Msg_RWorkDial->pd_InUse = FALSE;
				// Lets see what we've got.
				// Right now we don't need to check the reply cmds
				// since we don't use them.
				switch(Msg_RWorkDial->pd_Cmd)
				{
					case DCC_IGNORE:
							break;
					default:
							break;
				}
			}
		}

		if(B_Run)
		{
			XappDoIt(ThisPI);
			B_Run = FALSE;
		}

		if(B_ReInit)
		{
			// Since this worker is always ready we will
			// send a DCI_CHILDREADY to our guide to indicate we
			// are prepared to run. 
			// Other Xapps may first want to re-init their data before
			// sending a new DCI_CHILDREADY request to the guide.
			if(!Msg_WorkDial->pd_InUse)
			{
				B_ReInit = FALSE;
				Msg_WorkDial->pd_ChildPI = ThisPI;
				Msg_WorkDial->pd_InUse = TRUE;
				Msg_WorkDial->pd_Cmd = DCI_CHILDREADY;
				PutMsg(ThisPI->pi_Port_CtoP,(struct Message *)Msg_WorkDial);
			}
		}

    	// This signal is not used by the system during normal operation but
    	// when something horrible happens, like a corrupted system list, MediaLink
    	// will send SIGF_ABORT signals to tasks which name starts with either
    	// "guide" or "worker".
    	// EMERGENCY EXIT

    	if(SigRecvd & SIGF_ABORT)
            break;
    }

/***************************************************************************/
// Free you own data

    /*
    XaPPFree();
    */
/***************************************************************************/
// Free all MediaLink structures

#if KILLOLD_XAPP
	if ( Msg_XappDial != NULL )
	    FreeMem(Msg_XappDial,sizeof(PROCDIALOGUE));
#endif
	if ( Msg_WorkDial != NULL )
	    FreeMem(Msg_WorkDial,sizeof(PROCDIALOGUE));

	if ( RepP_Worker != NULL )
	    DeletePort(RepP_Worker);

	if(Port_Xapp != NULL)
	    DeletePort(Port_Xapp);

#if KILLOLD_XAPP
	if ( RepP_Xapp != NULL )
	    DeletePort(RepP_Xapp);
#endif

/***************************************************************************/
// Set your error code, overwrites your ar_Worker argument fields
    ThisPI->pi_Arguments.ar_RetErr = Error;

/***************************************************************************/
// Save return, no Exit()!

    return;
}
