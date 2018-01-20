#include "nb:pre.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "mlmmu:mlmmu.h"
#include "mlmmu:mlmmu_pragma.h"
#include "mlmmu:mlmmu_proto.h"
#include "structs.h"

/**** externals ****/

extern struct Library *medialinkLibBase;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;

/******** PerformActions() ********/

BOOL PerformActions(struct Studio_Record *studio_rec)	//, STRPTR errorStr)
{
BOOL retval=TRUE;
TEXT *file;
TEXT cmdStr[256];
int i,w;
UBYTE perc_to_db[] =
{
0 , 0 , 0 , 1 , 1 , 1 , 1 , 2 , 2 , 2,
2 , 3 , 3 , 3 , 3 , 4 , 4 , 4 , 4 , 5,
5 , 5 , 5 , 6 , 6 , 6 , 7 , 7 , 7 , 7,
8 , 8 , 8 , 9 , 9 , 9 , 10, 10, 10, 11,
11, 11, 12, 12, 13, 13, 13, 14, 14, 15,
15, 15, 16, 16, 17, 17, 18, 18, 19, 19,
20, 20, 21, 22, 22, 23, 23, 24, 25, 25,
26, 27, 28, 28, 29, 30, 31, 32, 33, 34,
35, 36, 37, 38, 40, 41, 43, 44, 46, 48,
50, 52, 55, 58, 61, 65, 70, 76, 85, 100, 100,
};
struct GfxBase *GfxBase;

	file = (TEXT *)AllocMem(1024L,MEMF_ANY|MEMF_CLEAR);
	if ( !file )
		return(FALSE);
	file[0] = '\0';

	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0L);
	if ( !GfxBase )
	{
		CloseLibrary((struct Library *)GfxBase);
	}

	if ( studio_rec->mode==MODE_PLAY )
	{
		if ( studio_rec->playPath1[0] )
		{
			sprintf(cmdStr,"S16OpenWithAutoClose %s\n",studio_rec->playPath1);
			strcat(file,cmdStr);
			strcat(file,"a=result\n");
		}		

		if ( studio_rec->playPath2[0] )
		{
			sprintf(cmdStr,"S16OpenWithAutoClose %s\n",studio_rec->playPath2);
			strcat(file,cmdStr);
			strcat(file,"b=result\n");
		}		

		if ( studio_rec->playPath3[0] )
		{
			sprintf(cmdStr,"S16OpenWithAutoClose %s\n",studio_rec->playPath3);
			strcat(file,cmdStr);
			strcat(file,"c=result\n");
		}		

		if ( studio_rec->playPath4[0] )
		{
			sprintf(cmdStr,"S16OpenWithAutoClose %s\n",studio_rec->playPath4);
			strcat(file,cmdStr);
			strcat(file,"d=result\n");
		}		

		strcat(file, "S16Trigger");

		if ( studio_rec->playPath1[0] )
			strcat(file," a");
		if ( studio_rec->playPath2[0] )
			strcat(file," b");
		if ( studio_rec->playPath3[0] )
			strcat(file," c");
		if ( studio_rec->playPath4[0] )
			strcat(file," d");
		strcat(file,"\n");

		if ( studio_rec->playFadeIn != 0 )
			strcat(file,"S16ChanVol 3 1 0\n");	// 3=CHAN_OUTPUT 1=Channel# 0=Off
		else
		{
			sprintf(cmdStr,"S16ChanVol 3 1 %d\n", 100-perc_to_db[100-studio_rec->playVolume]);
			strcat(file,cmdStr);
		}

		strcat(file,"S16Exit\n");

		retval = UA_IssueRexxCmd_V2("MP_S16","Studio16.1",file,NULL,NULL);

		if ( retval )
		{
			if ( studio_rec->playFadeIn != 0 )
			{
				for(i=0; i<studio_rec->playVolume; i=i+(studio_rec->playVolume/20))	//i=i+(16-studio_rec->playFadeIn))
				{
					sprintf(cmdStr,"address 'Studio16.1' S16ChanVol 3 1 %d\n", 100-perc_to_db[100-i]);
					retval = UA_IssueRexxCmd_V2("MP_S16","Studio16.1",cmdStr,NULL,NULL);
					//retval = UA_IssueRexxCmd_V2NoRet(cmdStr);
					for(w=0; w<studio_rec->playFadeIn*2; w++)
						WaitTOF();
				}
				sprintf(cmdStr,"address 'Studio16.1' S16ChanVol 3 1 %d\n", 100-perc_to_db[100-studio_rec->playVolume]);
				retval = UA_IssueRexxCmd_V2("MP_S16","Studio16.1",cmdStr,NULL,NULL);
				//retval = UA_IssueRexxCmd_V2NoRet(cmdStr);
			}	
		}
	}
	else if ( studio_rec->mode==MODE_MISC )
	{
		if ( studio_rec->misc==MISC_STOP)
		{
			if ( studio_rec->stopFadeOut )
			{
				for(i=studio_rec->playVolume; i>=0; i=i-(studio_rec->playVolume/20))
				{
					sprintf(cmdStr,"address 'Studio16.1' S16ChanVol 3 1 %d\n", 100-perc_to_db[100-i]);
					retval = UA_IssueRexxCmd_V2("MP_S16","Studio16.1",cmdStr,NULL,NULL);
					//retval = UA_IssueRexxCmd_V2NoRet(cmdStr);
					for(w=0; w<studio_rec->stopFadeOut*2; w++)
						WaitTOF();
				}
			}
			retval = UA_IssueRexxCmd_V2("MP_S16","Studio16.1","S16StopAllPlayback",NULL,NULL);
			//retval = UA_IssueRexxCmd_V2NoRet("address 'Studio16.1' S16StopAllPlayback");
		}
		else if ( studio_rec->misc==MISC_FADEIN )
		{
			if ( studio_rec->fadeInSecs )
			{
				for(i=0; i<studio_rec->playVolume; i=i+(studio_rec->playVolume/20))
				{
					sprintf(cmdStr,"address 'Studio16.1' S16ChanVol 3 1 %d\n", 100-perc_to_db[100-i]);
					retval = UA_IssueRexxCmd_V2("MP_S16","Studio16.1",cmdStr,NULL,NULL);
					//retval = UA_IssueRexxCmd_V2NoRet(cmdStr);
					for(w=0; w<studio_rec->fadeInSecs*2; w++)
						WaitTOF();
				}
			}
			sprintf(cmdStr,"address 'Studio16.1' S16ChanVol 3 1 %d\n", 100-perc_to_db[100-studio_rec->playVolume]);
			retval = UA_IssueRexxCmd_V2("MP_S16","Studio16.1",cmdStr,NULL,NULL);
			//retval = UA_IssueRexxCmd_V2NoRet(cmdStr);
		}
		else if ( studio_rec->misc==MISC_FADEOUT )
		{
			if ( studio_rec->fadeOutSecs )
			{
				for(i=studio_rec->playVolume; i>=0; i=i-(studio_rec->playVolume/20))
				{
					sprintf(cmdStr,"address 'Studio16.1' S16ChanVol 3 1 %d\n", 100-perc_to_db[100-i]);
					retval = UA_IssueRexxCmd_V2("MP_S16","Studio16.1",cmdStr,NULL,NULL);
					//retval = UA_IssueRexxCmd_V2NoRet(cmdStr);
					for(w=0; w<studio_rec->fadeOutSecs*2; w++)
						WaitTOF();
				}
			}
			retval = UA_IssueRexxCmd_V2("MP_S16","Studio16.1","S16ChanVol 3 1 0",NULL,NULL);
			//retval = UA_IssueRexxCmd_V2NoRet("address 'Studio16.1' S16ChanVol 3 1 0\n");
		}
		else if ( studio_rec->misc==MISC_SETVOL )
		{
			if ( studio_rec->setVolSecs )
			{
				if ( studio_rec->setVolFrom > studio_rec->setVolTo )
				{
					for(i=studio_rec->setVolFrom; i>=studio_rec->setVolTo; i=i-((studio_rec->setVolFrom-studio_rec->setVolTo)/20))
					{
						sprintf(cmdStr,"address 'Studio16.1' S16ChanVol 3 1 %d\n", 100-perc_to_db[100-i]);
						retval = UA_IssueRexxCmd_V2("MP_S16","Studio16.1",cmdStr,NULL,NULL);
						//retval = UA_IssueRexxCmd_V2NoRet(cmdStr);
						for(w=0; w<studio_rec->setVolSecs*2; w++)
							WaitTOF();
					}
				}
				else if ( studio_rec->setVolFrom < studio_rec->setVolTo )
				{
					for(i=studio_rec->setVolFrom; i<studio_rec->setVolTo; i=i+((studio_rec->setVolTo-studio_rec->setVolFrom)/20))
					{
						sprintf(cmdStr,"address 'Studio16.1' S16ChanVol 3 1 %d\n", 100-perc_to_db[100-i]);
						retval = UA_IssueRexxCmd_V2("MP_S16","Studio16.1",cmdStr,NULL,NULL);
						//retval = UA_IssueRexxCmd_V2NoRet(cmdStr);
						for(w=0; w<studio_rec->setVolSecs*2; w++)
							WaitTOF();
					}
				}
			}
			sprintf(cmdStr,"address 'Studio16.1' S16ChanVol 3 1 %d\n", 100-perc_to_db[100-studio_rec->setVolTo]);
			retval = UA_IssueRexxCmd_V2("MP_S16","Studio16.1",cmdStr,NULL,NULL);
			//retval = UA_IssueRexxCmd_V2NoRet(cmdStr);
		}
	}

	CloseLibrary((struct Library *)GfxBase);

	FreeMem(file,1024L);

	return(retval);              
}

/******** SilenceInTheStudio() ********/

void SilenceInTheStudio(void)
{
	UA_IssueRexxCmd_V2("MP_S16","Studio16.1","S16StopAllPlayback",NULL,NULL);
}

/******** E O F ********/
