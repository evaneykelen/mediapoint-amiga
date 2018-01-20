#include "nb:pre.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"
#include "structs.h"

extern struct Library *medialinkLibBase;

/**** functions ****/

/******** XappDoIt() ********/

int XappDoIt(PROCESSINFO *ThisPI)
{
struct PAR_Record par_rec;

	medialinkLibBase = (struct Library *)OpenLibrary("mediapoint.library",0L);
	if ( !medialinkLibBase )
		return(0);

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
	{
		GetExtraData(ThisPI,&par_rec);
		if ( FindPort("DDR") )
			Preview(&par_rec);
	}

	CloseLibrary((struct Library *)medialinkLibBase);

	return(0);
}

/******** Preview() ********/

void Preview(struct PAR_Record *par_rec)
{
TEXT cmd[SIZE_FULLPATH];

	if ( par_rec->path[0] )
	{
		// LOAD FILE

		sprintf(cmd,"address DDR FILE '%s'",par_rec->path);
		UA_IssueRexxCmd_V1("MP_PAR",cmd,NULL,FALSE,0);

		if ( par_rec->startFrame != 0 && par_rec->endFrame != 0 )	// anim not a still
		{
			// STUDIO 16

			if ( par_rec->studio16Cue )
			{
				sprintf(cmd,"address DDR SMPTE %s",par_rec->cue);
				UA_IssueRexxCmd_V1("MP_PAR",cmd,NULL,FALSE,0);
				UA_IssueRexxCmd_V1("MP_PAR","address DDR SMPTE ON",NULL,FALSE,0);
				UA_IssueRexxCmd_V1("MP_PAR","address DDR PAUSE ON",NULL,FALSE,0);
			}

			// SHOW FIRST FRAME

			sprintf(cmd,"address DDR JUMP %d",par_rec->startFrame);
			UA_IssueRexxCmd_V1("MP_PAR",cmd,NULL,FALSE,0);

			// START TO PLAY

			UA_IssueRexxCmd_V1("MP_PAR","address DDR PLAY",NULL,FALSE,0);

			// PLAY UNTIL LAST FRAME

			sprintf(cmd,"address DDR STOP %d",par_rec->endFrame);
			UA_IssueRexxCmd_V1("MP_PAR",cmd,NULL,FALSE,0);
		}
	}
}

/******** E O F ********/
