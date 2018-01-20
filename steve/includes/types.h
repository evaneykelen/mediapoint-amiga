#ifndef MINC_TYPES_H
#define  MINC_TYPES_H

// Use for Process controller modules
// All typedefs are stored in this file

#define OTT_PRELOAD 	1
#define OTT_AFTERLOAD 	2

/****************************************************
*Desc : Custom node.ln_Type definitions
*/

// Node type for ProcessInfo PI->pi_Node.ln_Type
#define NT_SERGUIDEPROC	255		// UBYTE value, lets start at 255 and count downwards
#define NT_PARGUIDEPROC	249
#define NT_WORKERPROC 	254		// list of loaded workers
#define NT_MODULE		248		// Resident moules like synchronizer, geventprocessor

#define NT_SYNCPUNCH	253		// list of syncpunches	

#define NT_SYNCGUIDE	251		// list of guides that filed a syncpunch and are still
								// alive
#define NT_TIMEREQUEST	250		// list of timerequests send to timer device

#define NT_MEMTAG		247		// Node type for MEMTAG 

typedef struct ScriptNodeRecord SNR;

#define NT_RESSEG			252		// resident modules segment list
#define NT_RESSEG_PRELOAD	246		
#define NT_RESSEG_AFTERLOAD	245		

/****************************************************
*Desc : The processcontroller has a list with all
*		resident memory segments. This list contains
*		the various SerGuide/ AddGuide and all the
*		NaPPs. They are all preloaded at startup
*		and stay in memory until break-down.
*		The segment may be re-used a number of times
*		which means that all the modules MUST be
*		re-entrant, like all NaPPs and Guidesegs.
*		In future this list could be used to store
*		XaPPs as well so they won't have to be reloaded
*		but can stay resident as well. Ofcourse these
*		XaPP will also have to be re-entrant.
*/
typedef struct
{
  struct Node 	rs_Node;			// Linked list of resident modules
  UWORD			rs_Pad;				// LONG word alignment
  BPTR			rs_Segment;			// SegmentList of this resident segment
  int			rs_Count;			// Open count for segments
									// Needs to be 0 for Unloadsegments(); 
  char 			rs_Name[108];		// rs_Node.ln_Name pts to this static string
									// This string is a copy of FIB->fib_FileName
  ULONG			rs_StackSize;		// Stacksize of this segment when run
  BOOL			rs_Quiet;			// taken from .info file's : "STOPQUIET"
									// If set then the Xapp always receives a
									// STOP/TERM/EASYSTOP or EASYTERM.
									// Normal Xapps should have it set to YES.
									// System XaPPs do not define the tooltype
									// and are therefore set to NO (FALSE)
} RESSEGMENT;

/***********************************************************
*Desc : After a gosub snr is found the next logical snr
*		will be placed onto the stack. 
*/
#define JT_SINGLE 	0		// show 1 object from the next list, used when
							// gosubbing to a label snr.
#define JT_FULLLIST	1		// show all objects after the label until a 
							// end of list is encountered.									
#define JT_SUBLIST	2		// show all objects in the serial level to which 
							// the pc will jump.
typedef struct
{
  SNR 		*si_SNR;
  ULONG 	si_JumpType;
  SNR		*si_HitSNR;		// used only for JT_FULLLIST.
							// First, when the pc finds a TG_GOSUB to a label
							// the parentsnr of TALK_GOTO snr is stored into this field.
							// Then, each time the scriptfollower runs into an end of
							// list it will check the the parentsnr of the lower list
							// and, when that snr equals the HitSNR of the ontop
							// stackitem, it will return to the main level.		
} SNRSTACKITEM;

typedef struct
{
  SNRSTACKITEM 		*sn_Lower,
					*sn_Upper,
					*sn_Ptr;
} SNRSTACK;

/***************************************************
*Desc : Each command issued from the Arexx port
*		is parsed and the fields are put into an
*		array of identifiers
*/
#define MAXIDENTIFIERS 5
#define IDENTIFIERLENGTH 150

typedef struct
{
   char id_Name[IDENTIFIERLENGTH];

} IDENTIFIER;

typedef struct
{
  char rc_Name[20];
  void (*rc_Func)(IDENTIFIER *, struct ScriptInfoRecord *, char **, char **, int *);

} REXXCMD;

typedef struct
{
  char *re_Name;
  LONG re_ReturnCode;

} REXXERROR;

#endif // MINC_TYPES_H

