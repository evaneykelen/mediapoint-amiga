#ifndef MINC_SNR_H
#define  MINC_SNR_H

#define MAX_PARSER_ARGS				20		/* max number of arguments */
#define MAX_PARSER_CHARS			256		/* max length of one line of source */
#define MAX_OBJECTNAME_CHARS		32
#define MAX_GLOBALLOCAL_EVENTS		256		/* max no. of global and local events */

/******************************************************************
 * Desc : Information on a certain punch, start/stop/offset info.
 */

typedef	union
{
  struct 
  { 
	LONG Days, 
		 Minutes, 
		 Ticks; 
  } HHMMSS;

  struct 
  { 
	BYTE HH, 
	   	 MM, 
		 SS, 
		 FF; 
  } TimeCode;

  ULONG ParHMSTOffset;		// Start/Stop  punch for objects within parallel events

} PUNCHDATA;

struct ScriptInfoRecord
{
	/*************************/
	struct List *currentList;								/* P R I V A T E */
	char listType;													/* P R I V A T E */
	struct List **lists;										/* P R I V A T E */
	UWORD *scrollPos;												/* P R I V A T E */
	struct ScriptNodeRecord *currentNode;		/* P R I V A T E */
	/*************************/
	BYTE version, revision;
	struct List **allLists;
	struct ScriptEventRecord *globallocalEvents[MAX_GLOBALLOCAL_EVENTS];	/* P U B L I C */
	char timeCodeSource;
	char timeCodeFormat;
	char timeCodeRate;
	PUNCHDATA Offset;
	BOOL timeCodeOut;
	/*************************/
};

/***** struct ScriptNodeRecord is now 482 bytes large *****/

struct ScriptNodeRecord
{
  struct Node 	node;			// Linked list of SNRs 
  struct List 	*list;			// Ptr to sub-list. Used by SERIAL and 
								// PARALLEL objects, else NULL.
  BYTE 		  	nodeType;		// Object type
  WORD 		  	numericalArgs[MAX_PARSER_ARGS];
  TEXT 		  	objectPath[MAX_PARSER_CHARS];
  TEXT 		  	objectName[MAX_OBJECTNAME_CHARS];
  UBYTE 		*extraData;		// e.g. palette data 
  ULONG 		extraDataSize;	// sizeof(*ExtraData)
  LONG 			duration;		// Duration of event, HHMMSS mode only
  BYTE 			dayBits;		// Days on which the object must be shown, HHMMSS only
  PUNCHDATA 	Start;			// Start punch data
  PUNCHDATA 	End;			// Stop punch data
  BYTE 			startendMode;
  BYTE 			miscFlags;
  int			PageNr;
  ULONG 		*ProcInfo;		// Ptr to processinfo structure or NULL if 
								// not in PIList. More information on this
								// field can be found in pc_process.h, 
								// structure PROCESSINFO, field pi_SNR; 
  struct ScriptNodeRecord 
				*ParentSNR;		// ptr to the SNR of the owner of this object
								// This ptr is the same for all objects in a list
};
#endif
