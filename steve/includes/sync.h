#ifndef MINC_SYNC_H
#define  MINC_SYNC_H

#ifndef MINC_PROCESS_H
#include "minc:process.h"
#endif


#define DIAL_MAXSYNC 4

/************************************************
*Desc : Synchronizer collision data
* <!> : This structure must be allocated by guide.
*		Freeing is performed by the guide
*		after a collision has taken place and the
*		guide has received its punch from the synchonizer. 
*
*		HOWEVER when Syncpunches get lost for whatever reason
*		they will be freed by the synchronizer upon Synccleanup.
* 
*		PUNCHDATA is defined in "parser.h"
*
*		About sp_MinNode:
*		When a guide passes a list of SyncPunches these punches
*		have to be chained together. 
*		The first SyncPunc.sp_MinNode.mln_Pred = not used;
*		The last SyncPunc.sp_MinNode.mln_Succ = NULL;		
*		The SyncDialogue.sd_Punch holds a ptr to the first SyncPunch structure
*		This is not a normal double linked list.
*		The synchronizer will simply take the first one and downlinks through
*		mln_Succ until it is NULL;
* <!>	The synchronizer will clear the MinNode.mln_Succ, therefore the guide
*		may not rely on the linked list when freeing the received syncpunch. 
*
*/
typedef struct	// 22 byte
{
  struct MinNode	sp_MinNode;			// linked list of synchropunches, see Desc above
  int				sp_PunchType;		// Start/ stop / Hold punch
										// set by the guide when sending new
										// punches to the synchronizer. When it later
										// receives this punch again it may easily 
										// find out if the new object must be started/	
										// continued, stopped or hold.
  SNR				*sp_SNR;			// Unique code used by the guide to find
										// out to what SNR this Punch belongs to
  UWORD				sp_FKey;			// In case of a Parallel event punch, this
										// field holds the Fkey for the punch.
  union
  {
    ULONG			pi_HMSTDuration;	// GMT. When this time has passed a punch
										// will be returned
    ULONG			pi_FCFastCheck;		// the pi_FrameCode will form a 4 byte longword
										// which can also be accessed by this var.
										// b 0-b 7 -> Framenr
										// b 8-b15 -> Seconds
										// b16-b23 -> Minutes
										// b24-b31 -> Hours
  } sp_PunchInfo;						// different modes take different punch sources
} SYNCPUNCH;

#define PT_NOPUNCH	0					// punches removed from the punch list
#define PT_DORUN	1
#define PT_DOHOLD	2
#define PT_DOSTOP	3

/********************************************************
*Desc : The synchronizer will keep a list of messages send
*		to the timer device. Each time a message has to be
*		send a TIMEREQUEST structure is allocated and linked
*		into the TRList. When a reply from the timer device
*		is received, the TIMEREQUEST structure will be removed
*		from the list. When a SIGF_ABORT is received, all 
*		outstanding TIMEREQUEST will be aborted and freed.
* <!> : This structure is different from the standard 
*		timerequest structure as supplied by devices/timer.h
*/
typedef struct // 68 bytes
{
  struct Node 		tr_Node;		// for TRList linking only, not used by timer device
  UWORD				tr_Pad;			// align longword
  struct IORequest 	tr_IOReq;		// for use by the timer device, the SendIO will pass
									// a pointer to this field in stead of passing a pointer
									// to the structure base
  struct timeval	tr_Time;		// MUST be located directly after IOReq
									// time values, secs and microsecs
  PROCESSINFO		*tr_ProcInfo;	// Ptr to processinfo of guide, used for fast access
  int				tr_ColState;	// if TRUE, a collision on this request was made
  SYNCPUNCH			*tr_Punch;		// ptr to a SYNCPUNCH
} TIMEREQUEST;

#define CS_WAIT 		0
#define CS_FRAMECOL 	1
#define CS_TRANSMITTED 	2


/************************************************
*Desc : Dialogue message between a guide and the
*		synchronizer.
*		When a guide passes his new syncpunches
*		on to the synchronizer the sd_Cols is
*		filled with ptr to a minlist of SYNCPUNCH
*		structs. These struct are inserted in the
*		private synchronizer List. When a collision
*		on a syncpunch has occured the synchronizer
*		sends a SYNCDIALOGUE to the sc_Port_StoG msgport
*		The sd_NumPunch is set to 1 and the sd_Punch
*		field is set to the Punch that caused the collision
*
*/
typedef struct // 38 bytes
{
  struct Message	sd_Msg;				// standard Message
  UWORD				sd_InUse;			// If TRUE, this dialogue is in use
  int 				sd_Cmd;				// Command from guide or info from synchronizer
  PROCESSINFO		*sd_ProcInfo;		// Ptr to processinfo of guide, NULL if msg
										// comes from the ProcessController 
										// or the synchronizer. 
  SYNCPUNCH			*sd_Punch;			// ptr to SYNCPUNCH structs list
										// This ptr points to the first SYNCPUNCH
										// more information on how the SYNCPUNCHES
										// must be chained in SYNCPUNCH structure
  TIMEREQUEST		*sd_TimeReq;		// Needed after a reply from the PC has been
										// received. This structure will be removed
										// and freed at that time.
} SYNCDIALOGUE;

enum
{
  SDC_NEWPUNCH,			// A guide has new punches for the synchronizer
  SDI_PUNCHACCEPTED,	// The punches send to the synchronizer have been accepted
  SDI_PUNCHCOL,			// A punchcollision has occured, this info is send to the guide
  SDI_PUNCHERROR,		// A received punch caused some problems, guide must stop
						// processing and file a general termination request
  SDC_GUIDETERM,		// A guide is about to terminate itself and needs to be removed
						// from the SyncGuide list.
  SDI_GUIDEREMOVED,		// A guide has been removed from the SyncGuide list after
						// the synchronizer received a SDC_GUIDETERM request  
  SDC_ABORTPUNCHES,		// The Processcontroller wants all punches to become invalid
  SDI_PUNCHESABORTED,	// All active punches have been removed
  SDC_SETTIMECODE,		// Set the current timecode to a new value
  SDI_TIMECODESET,
  SDC_GETTIMECODE,		// get the current timecode from the synchronizer
  SDI_TIMECODEGET,

  SDC_EXTERNALEXIT,		
  SDI_EXTERNALEXIT,		// Send by the synchronizer to the proccesscontroller
						// when external timing is used

  SDC_GETTIMECODE_ASC,	// get the current timecode from the synchronizer,
  SDI_TIMECODEGET_ASC	// present it to the user in HH:MM:SS:FF instead of a value.

  	
};

/**********************************************************
*Desc : The synchronizer will build a list of guides that
*		have currently a number of punches active. When
*		a guide sends a SDC_GUIDETERM its ProcInfo is removed
*		from the SGList. When it first files a punch, it is
*		inserted into the list.
*		When the synchronizer encounters a punch collision the
*		first thing it will do is find out if the guide to 
*		which the punch belongs to, still exists. If not the
*		punch will not be send to the guide. If the guide is
*		still alive, it pi_State is checked for ST_TERM, if
*		not then the punchcollision will be send to the guide.
*/
typedef struct	// 24 bytes
{
  struct Node 	sg_Node;			// linked list of guides which have filed a punch
									// type : NT_SYNCGUIDE
  UWORD			sg_Pad;				// Align to longword				
  PROCESSINFO	*sg_ProcInfo;		// pointer to the processinfo structure of a guide
  int			*sg_NrPunches;		// number of punches the guide has filed 	
} SYNCGUIDE;


/**********************************************
*Desc : data for syncproctask
*		Signalmask is used by higher level
*		interrupts for SMPTE or other timecode
*		devices. In that case, all common datafields
*		used be stored in this structure and this
*		structure should be passed on to the interrupt
*		server as being the Datafield
*/
typedef struct	// 44 bytes
{
  struct Task 		*sd_Task;			// ptr to task struct of synchronizer
  ULONG 			sd_Sig_TCItoS;		// Signal send to synctask
  
  UBYTE				sd_TimeRate;		// 25, 30 fps, Double function
										// ITC -> max frames per second
										// ETC -> Max bytes per frame = 4  
  BOOL				sd_B_TimeCodeOut;	// if true, TimeCode output on RS232 
  ULONG				sd_FrameNr;			// nr of the frame since the start of this
										// synchronizer. 
  ULONG				sd_FCFastCheck;		// Actual FrameCounter
										// For description see SYNCPUNC.pi_FCFastCheck
  BOOL				sd_B_Field;			// Odd/Even field -> divides 50/60 into 25/30 Hz
  BOOL				sd_B_Count;			// If true, the interrupt will start counting
										// used only by ITC, not by ETC
  struct List		*sd_TRList;			// ptr to list of TIMEREQUESTs
  struct Custom		*sd_Custom;			// ptr to custom chips base
  int				sd_SpecCmd;			// Special commands from the interrupt handler
  BOOL				sd_B_DoCheck;		// Tempoeditor sets this bool to false
										// to prevent the itc from returning punches
  BOOL				sd_GenerateInternally;	// see sync:itc.c - inthandler  for description
} SYNCDATA;

enum 
{
  SCC_NOCMD,
  SCC_EXIT			// Frame Cntr has received an external quit command   
};
#endif
