#include "nb:pre.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"
#include "structs.h"

#include <rexx/storage.h>
#include <rexx/rxslib.h>
#include <rexx/errors.h>
#include <libraries/dosextens.h>

#include "AREXX/rexx_pragma.h"
#include "AREXX/rexx_proto.h"
#include "AREXX/rexx.h"

/**** externals ****/

extern struct Library *medialinkLibBase;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;

/**** extern functions ****/

extern int GetARexxMsg(AREXXCONTEXT *RexxContext, ULONG *err, STRPTR arg);
extern void ReplyARexxMsg(AREXXCONTEXT *, struct RexxMsg *, char *, LONG);
extern struct RexxMsg *SendARexxMsg(AREXXCONTEXT *, char *, BOOL);
extern void FreeARexx(AREXXCONTEXT *);
extern AREXXCONTEXT *InitARexx(char *);
extern AREXXCONTEXT *PerformRexxCmd(char *);
extern BOOL ProcessArexxError(LONG RC, LONG errorCode, STRPTR errorStr);

/**** function declaration ****/

BOOL IssueArexxCmd(STRPTR cmdStr, STRPTR errorStr);

/**** functions ****/

/******** PerformActions() ********/

BOOL PerformActions(struct Toaster_record *trec, STRPTR errorStr)
{
BOOL retval=TRUE;
char Command[45]; //Will hold Rexx command
char Cmd1[] = "ADDRESS TOASTER_COMMAND_HOST ";
char bank;
char Cmd2[10];
char FrameNum1[4];
char FrameNum2[4];
int row,column,length,fs;
char fsStr[4];

  strcpy(Command,Cmd1); //Set first part of command string
     
	switch( trec->cmd )
	{
		case 0:	// Perform transition
			/**************************************************************
			 *
			 * trec->previewSource: 	0...6 (1,2,3,4,DV1,DV2,DV3)
			 * trec->transitionBank:	0 (use 1 of the 8 standard transitions) or
			 *												1...9 (Bank A,B,C,D,E,F,G,H,I)
			 * trec->transitionSpeed:	0...2 (Slow, Med, Fast)
			 * trec->transitionCol:		0...7
			 * trec->transitionRow:		0...3
			 * trec->transitionOwn:		0...7	(1 of the 8 standard transitions)
			 *												Use this when transitionBank==0
			 *
			 **************************************************************/

               //First, bring Toaster Screen to Front (if not already)
               retval = IssueArexxCmd("ADDRESS TOASTER_COMMAND_HOST TOSW", errorStr);
               
               switch( trec->previewSource )
               {
                    case 0: //Set Preview to Video Input 1
                         strcat(Command,"P001");
	                    retval = IssueArexxCmd(Command, errorStr);
                         break;
                    case 1: //Set Preview to Video Input 2
                         strcat(Command,"P002");
	                    retval = IssueArexxCmd(Command, errorStr);
                         break;
                    case 2: //Set Preview to Video Input 3
                         strcat(Command,"P003");
	                    retval = IssueArexxCmd(Command, errorStr);
                         break;
                    case 3: //Set Preview to Video Input 4
                         strcat(Command,"P004");
	                    retval = IssueArexxCmd(Command, errorStr);
                         break;
                    case 4: //Set Preview to DV1
                         strcat(Command,"PDV1");
	                    retval = IssueArexxCmd(Command, errorStr);
                         break;
                    case 5: //Set Preview to DV2
                         strcat(Command,"PDV2");
	                    retval = IssueArexxCmd(Command, errorStr);
                         break;
                    case 6: //Set Preview to DV3
                         strcat(Command,"PDV3");
	                    retval = IssueArexxCmd(Command, errorStr);
                         break;
               }
              
               Command[0] = '\0';       // Clear out last command
               strcpy(Command,Cmd1);    // and set 1st part again
               if(trec->transitionBank == 0) { //Graphic transitions

     //******* Replace the following row & columns with proper ones                      
                    if(trec->transitionOwn == 0) {
                         strcat(Command,"GRID A21");
	                    retval = IssueArexxCmd(Command, errorStr);
                    }
                    else if(trec->transitionOwn == 1) {
                         strcat(Command,"GRID A22");
                         retval = IssueArexxCmd(Command, errorStr); 
                    }
                    else if(trec->transitionOwn == 2) {
                         strcat(Command,"GRID A23");
                         retval = IssueArexxCmd(Command, errorStr); 
                    }
                    else if(trec->transitionOwn == 3) {
                         strcat(Command,"GRID A24");
                         retval = IssueArexxCmd(Command, errorStr); 
                    }
                    else if(trec->transitionOwn == 4) {
                         strcat(Command,"GRID A25");
                         retval = IssueArexxCmd(Command, errorStr); 
                    }
                    else if(trec->transitionOwn == 5) {
                         strcat(Command,"GRID A26");
                         retval = IssueArexxCmd(Command, errorStr); 
                    }
                    else if(trec->transitionOwn == 6) {
                         strcat(Command,"GRID A27");
                         retval = IssueArexxCmd(Command, errorStr); 
                    }
                    else if(trec->transitionOwn == 7) {
                         strcat(Command,"GRID A28");
                         retval = IssueArexxCmd(Command, errorStr); 
                    }

               }
               else { // Non-Graphic Bank/Row/Column Transition
                    switch( trec->transitionBank )
                    {
                         case 1:bank = 'A';
                              break;
                         case 2:bank = 'B';
                              break;
                         case 3:bank = 'C';
                              break;
                         case 4:bank = 'D';
                              break;
                         case 5:bank = 'E';
                              break;
                         case 6:bank = 'F';
                              break;                              
                         case 7:bank = 'G';
                              break;
                         case 8:bank = 'H';
                              break;
                         case 9:bank = 'I';
                              break;                              
                    }
                    row    = trec->transitionRow +1;                     
                    column = trec->transitionCol +1;               
                    Command[0] = '\0';       // Clear out last command
                    strcpy(Cmd2,"GRID    ");
                    Cmd2[5]=bank;
                    length = stci_d(&Cmd2[6],row);
                    length = stci_d(&Cmd2[7],column);                    
                    strcpy(Command,Cmd1);
                    strcat(Command,Cmd2);
                    retval = IssueArexxCmd(Command, errorStr); 
               }     


#if 0
{	// O N L Y   F O R   T E S T I N G
char temp[100];

	if ( trec->transitionBank==0 )
		sprintf(temp, "Source=%d, Bank=%d, Speed=%d, Transition=%d\n",
						trec->previewSource, trec->transitionBank,
						trec->transitionSpeed, trec->transitionOwn);
	else
		sprintf(temp, "Source=%d, Bank=%d, Speed=%d, Col=%d, Row=%d\n",
						trec->previewSource, trec->transitionBank,
						trec->transitionSpeed, trec->transitionCol,
						trec->transitionRow);
	KPrintF(temp);
}
#endif

	          retval = IssueArexxCmd("ADDRESS TOASTER_COMMAND_HOST AUTO", errorStr);

	Delay(30);

						if (errorStr)	// errorStr is NULL if run from player
		          retval = IssueArexxCmd("ADDRESS TOASTER_COMMAND_HOST TOWB", errorStr);

	Delay(30);

			break;

		case 1:	// Load framestore
			/**************************************************************
			 *
			 * trec->loadFrameStore:	if [0] != '\0' then this contains a filename
			 *
			 **************************************************************/

        //First, bring Toaster Screen to Front (if not already)
        retval = IssueArexxCmd("ADDRESS TOASTER_COMMAND_HOST TOSW", errorStr);

				// Set FrameStore Device

				sprintf(Command, "%sFSDV %s", Cmd1, trec->frameStorePath);
				if ( Command[ strlen(Command)-1 ] == '/' )
					Command[ strlen(Command)-1 ] == '\0';	// strip trailing slash
				retval = IssueArexxCmd(Command, errorStr);

				// Load the frame store by number from the earlier set fs path

				if ( trec->loadFrameStore[0] != '\0' )
				{
//KPrintF("%s\n",trec->loadFrameStore);
					// get trailing frame store number from file name
					stccpy(fsStr, trec->loadFrameStore, 4);	// create a null terminated 3 digit string
					// convert e.g. "010" to integer 10
					sscanf(fsStr,"%d",&fs);

					if ( fs>= 0 && fs<=999 )
					{
						sprintf(Command, "%sFMLD %03d", Cmd1, fs);
						retval = IssueArexxCmd(Command, errorStr);
					}
				}

Delay(50);

				if (errorStr)	// errorStr is NULL if run from player
	        retval = IssueArexxCmd("ADDRESS TOASTER_COMMAND_HOST TOWB", errorStr);

Delay(50);

#if 0
				length = stci_d(&FrameNum2[0],trec->Into_Number);               
				if(trec->Into_Number < 100)
				{
				  if(trec->Into_Number < 10)
					{
				     strcpy(FrameNum1,"00");
	  			   strcat(Command,FrameNum1);
				     strcat(Command,FrameNum2);                         
				   }
				   else {
			        strcpy(FrameNum1,"0");
             strcat(Command,FrameNum1);
             strcat(Command,FrameNum2);
       		 }
   				}
				   else strcat(Command,FrameNum2);
#endif

#if 0
{	// O N L Y   F O R   T E S T I N G
char temp[100];
	sprintf(temp, "Load frame [%s] into framestore %d\n",
					trec->loadFrameStore, trec->Into_Number);
	KPrintF(temp);
}
#endif

			break;

		case 2:	// Save framestore
			/**************************************************************
			 *
			 * trec->from:						0...1 (DV1, DV2)
			 * trec->FS_Number:				integer (001...999)
			 * trec->saveFrameStore:	if [0] != '\0' then this contains a filename
			 *
			 **************************************************************/

               
#if 0
{	// O N L Y   F O R   T E S T I N G
char temp[100];
	sprintf(temp, "Save from %d frame [%s]\n", trec->from, trec->saveFrameStore);
	KPrintF(temp);
}
#endif

			break;
	}

	return( retval );
}

/******** IssueArexxCmd() ********/

BOOL IssueArexxCmd(STRPTR cmdStr, STRPTR errorStr)
{
ULONG rexxSignal, SigRecvd;
AREXXCONTEXT *RexxContext;
LONG rexxError, RC;
TEXT Arg[256];
BOOL retval=TRUE;

//KPrintF("IssueArexxCmd [%s]\n",cmdStr);

	RexxContext = PerformRexxCmd(cmdStr);
	if ( RexxContext )
	{
		rexxSignal = 1<<RexxContext->ARexxPort->mp_SigBit;
		SigRecvd = Wait( rexxSignal );
		if( SigRecvd & rexxSignal )
		{
			RC = GetARexxMsg(RexxContext, &rexxError, Arg);
			if ( errorStr )
				retval = ProcessArexxError(RC, rexxError, errorStr);
			FreeARexx(RexxContext);
		}
	}

	return( retval );
}

/******** E O F ********/
