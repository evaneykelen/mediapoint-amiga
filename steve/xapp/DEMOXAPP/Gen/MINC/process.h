#ifndef MINC_PROCESS_H
#define  MINC_PROCESS_H

#ifndef MINC_SYSTEM_H
#include "demo:gen/MINC/types.h"
#endif

#ifndef MINC_SYSTEM_H
#include "demo:gen/MINC/system.h"
#endif

#define DIAL_MAXPTOC 4
#define DIAL_MAXSER  4

#define ORG_PROCHANDLER 1
#define ORG_SCRIPTEDITOR 2

/****************************************************
*Desc : Process Information structure. Every time a
*		worker or guide is loaded a PI is set up 
*		with arguments and name information on that
*		particulair process. It also contains a few
*		ports which both processes (parent & child)
*		may use for communication.
* <!>   The pi_Pad element is used to align the 
*		succeeding data on a long word alignment
*		
*/
typedef struct
{
  struct Node		pi_Node;		// linked list of guides/workers for the masterguide
									// ln_Name contains ptr to gp_Name 
									// ln_Type is NT_SERGUIDEPROC, NT_PARGUIDEPROC, NT_WORKERPROC
									// or NT_MODULE
  UWORD				pi_Pad;			// Long word alignment
  struct WBStartup	pi_Startup;		// The workbench startup message, returned after termination
									// This is also the address passed in ARGV
  struct Process	*pi_Process;	// fast accessible Process/Task ptr to process
  BPTR				pi_Segment;		// Segment list	
  struct MsgPort	*pi_Port_PtoC;	// port used by the parent to send messages to
									// the child, set up by the parent. The sigbit
									// is however attached to the childprocess
									// This port is made by AddGuide() for the child
  struct MsgPort	*pi_Port_CtoP;	// Port used by the child to send messages to
									// the parent. The sigbit is attached to the
									// parenttask.
									// This port is made by the parent
  int 				pi_Unit;		// Number of this guide/worker in SNRobject list	 	
  int 				pi_SizeOfName;	// Length of string in pi_Name	
  char				*pi_Name;		// ptr to FULL guide/worker name e.g. "Guide.1.2.3"
  int				pi_State;		// State of this guide/worker
  SNR				*pi_SNR;		// SNR from which this process is derived.
									// This field needs some more information :
									// After a SNR has been preloaded by a guide
									// the guide will put a PI ptr into the SNR.ProcInfo
									// field. When the guide terminates a child, it	
									// has to know what SNR was used to built this PI.
									// It will therefore take this ptr and clears 
									// the SNR.ProcInfo field after unloading a child.
									// <!> the SNR.ProcInfo contains a ULONG ptr
									// which has to be typecasted before used.

  ULONG				*pi_PtoCDial[DIAL_MAXPTOC];
									// 4 dialogues which may be used by the parent to reach
									// the child. Allocated and set up by AddGuide/Addworker	
									// These are actually ptrs to 4 PROCDIALOGUEs
  BOOL				pi_Preload;		// If TRUE this object was preloaded and should
									// not file a DCI_DOTERM request.

  BOOL				pi_Quiet;		// Read the file minc:types.h at the RSSEGMENT
									// structure for detailed information about this
									// flag. Copy of rs_Quiet.
  union								// Union because guide args are different from worker
  {									// arguments.	
	struct 
	{ 
	  WORD			*aw_NumArgs;		// Ptr to SNR.numericalArgs[0]
	  char 			*aw_Path;			// Ptr to SNR.objectPath[0]
	  char 			*aw_Name;			// Ptr to SNR.objectName[0]
	  UBYTE 		*aw_ExtraData;		// Copy of SNR.extraData
	  ULONG 		*aw_ExtraDataSize;	// Ptr to SNR.extraDataSize
	  struct List   *aw_List;			// Ptr to SNR.List
	  MLSYSTEM		*aw_MLSystem;		// Ptr to Medialink System structure
	  int			aw_Origin;			// Who called the worker: editor or player 
										// ORG_PROCHANDLER or ORG_SCRIPTEDITOR
	} ar_Worker;

	struct 
	{ 
	  struct List 	*ag_SegList;		// Ptr to public resident-Segments List	
	  struct ScriptInfoRecord *ag_SIR; 	// Ptr to system ScriptInfoRecord	
	  char 		  	*ag_MainDirPath;	// Ptr to string with a full path of main dir		
	  int		  	ag_PLL;				// Preloadlevel
	  MLSYSTEM		*ag_MLSystem;		// Ptr to Medialink System structure
	} ar_Guide;

	struct
	{
	  struct ScriptInfoRecord *am_SIR;	// ScriptInfo record
	  ULONG  		am_TempoType;		// Used by GlobeProc and Synchronizer
	  MLSYSTEM		*am_MLSystem;		// Ptr to Medialink System structure
	} ar_Module;
		
  	int	ar_RetErr;					// Return argument, error of child

  } pi_Arguments;

} PROCESSINFO;

// Guide types, serial or parallel guides
#define GT_SER 1
#define GT_PAR 2

// Process State PI->pi_State
enum
{
  ST_INIT,		// Process is initialising 
				// Set by AddGuide/Addworker during process setup
  ST_READY,		// Child process is ready to start
				// Set by Guide after DCI_CHILDREADY has been received
  ST_RUNNING,	// Child process is running 
				// Set by guide after a DCC_DORUN has been send
  ST_HOLD,		// Child process is pausing after command from parent	
				// Set by guide after a DCC_DOHOLD has been send
  ST_TERM		// A child has send a termination request and will
				// terminate itself after it receives a 	
};

/**********************************************************
*Desc : Dialogue structure for use between a parent and
*		its children.
*		The receiver of a Message may change the pd_Cmd
*		field to directly send back a command to the sender
*		but has to change the pd_Msg->ReplyPort to its
*		own reply port. This way, replying to each other, 
*		a dialogue can be built between the two processes.
* <!>   The pd_Pad element is used to align the 
*		succeeding data on a long word alignment
*/
typedef struct
{
  struct Message 	pd_Msg;				// Standard message structure for GetMsg
  UWORD			 	pd_InUse;			// If TRUE -> current dialogue is being used 
  PROCESSINFO	 	*pd_ChildPI;		// ProcessInfo struct ptr of child
										// If message comes from parent then NULL
  int			 	pd_Cmd;				// Command send by either parent or child
										// DCI -> information, DCC -> Command
  struct
  {
	SNR				*lu_SNR;			// Object which the command relates to 
	struct Message 	*lu_Dial;			// this field has a second use when
										// a xapp sends dialogues to the 
										// TRANSITION module.
										// It then holds a pointer to the picturesegment		
  } pd_Luggage;

} PROCDIALOGUE;

// Dialogue command -> PROCDIALOGUE.pd_Cmd
enum
{
  DCC_IGNORE,			// Ignore the dialogue received
  DCI_IGNORE,			// A child just replies
  DCI_CHILDREADY,		// A child is ready to start its job

  // Commands used between the process controller and its guides

  DCC_DORUNOBJ,			// A guide must start a worker, object in pd_SNR
  DCI_OBJRUNS,			// A guide will start a worker, object in pd_SNR

  DCC_DOHOLDOBJ,		// A guide must old its worker, object in pd_SNR
  DCI_OBJHOLDS,			// A guide will hold its worker, object in pd_SNR
	
  DCC_DOTERMOBJ,		// A guide must terminate a worker, object in pd_SNR
						// A guide will not terminate a continues object
  DCI_OBJTERMS,			// A guide will terminate a worker, object in pd_SNR
						// A guide will terminate a worker if non-continues

  DCC_DOLOADOBJ,		// A guide must load a worker, object in pd_SNR
  DCI_OBJLOAD,			// A guide will load a worker, object in pd_SNR

  DCC_DOTERMALLOBJS,	// A guide must terminate all non-continues children
  DCI_TERMALLOBJS,		// A guide will terminate all non-continues children

  // Commands used between PC and guide OR guide and worker
  
  DCC_DORUN,			// A child (usually a worker) must start
  DCI_CHILDRUNS,		// A child will start 
  
  DCC_DOHOLD,			// A child (usually a worker) must hold
  DCI_CHILDHOLDS,		// A child will hold	

  DCC_DOTERM,			// A child must terminate or gets approval to commit suicide
						// if send to a guide, it will terminate all objects	
  DCI_CHILDTERM,		// Request from child or reply to child
  DCC_DOSTOP,			// Preloaded objects are not terminated but stopped. They
						// should get ready for the next DORUN.
  DCC_DOPREPARE,		// a child must prepare itself for the next run
						// this command is send before a dcc_dorun is send and
						// the child must have been preloaded earlier.
  DCI_CHILDPREPARES,	// a child will prepare itself for the next run
	
  // additional commands used between the PC and a guide 	
  DCC_DOPREPOBJ,		// A guide must signal a child to prepare itself
  DCI_OBJPREPS,			// a guide has signalled a child to prepare 
  DCI_SEVEREPROBLEM,	// A guide has run into a problem it can't fix.
						// it signals the PC to quit the playsequence

  // Commands used between PC and guide OR guide and worker
  DCC_DOEASYTERM,
  DCI_CHILDEASYTERM,
  DCC_DOEASYSTOP,
  DCI_CHILDEASYSTOP,	

  // commands used between a serial guide and a parallel guide	
  DCC_DOEASYSTOPOBJ,	// a object of the parallel guid must stop but may finish it job
  DCI_CHILDEASYSTOPOBJ,
  DCC_DOEASYTERMOBJ,
  DCI_CHILDEASYTERMOBJ,
  DCC_DOSTOPOBJ,
  DCI_OBJSTOPS,

  DCI_CHILDREADY_MEMPROBLEM = 100,	// a guide is ready to start couldn't allocate enough
									// memory for its workers at preload

  DCC_DOCLEANUPEASY,				// send to the main guide on memory problems
									// normal guides should terminate all workers
									// but the main guide only terminates up to
									// a certain amount of memory is available	
  DCC_DOCLEANUP,					// a guide is requested to terminate as much
									// workers as possible since we have a memory 
  DCI_CHILDCLEANUP,

  // command used between xapps and the Transition module
  // This way, the xapps may use the transitions from the standard transitionxapp	 
  DCC_DOTRANSITION,					// perform a transition 
  DCI_TRANSITION					// transition will be performed

};

#endif
