//	File		:	general.c
//	Uses		:
//	Date		:	-92
//	Author	:	
//	Desc.		:	Functions used by Xapp programs
//
#include <stdio.h>
#include <workbench/startup.h>
#include <string.h>
#include "minc:types.h"
#include "minc:process.h"
#include <proto/exec.h>
#include "nb:parser.h"
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <pragmas/dos_pragmas.h>

/**************************************************
*Func : Find out the environment in which this process 
*		has been started. If it is not from MediaPoint
*		tell the user about it and return NULL.
*		If a programmer wants to test its software from
*		the CLI then he should setup a PROCESSINFO
*		structure using AllocMem and put his parameters
*		into the sugestive PI->pi_Arguments.ar_Worker fields.
*		Notice that when runnig from the CLI, it is not possible
*		to communicate with other processes since all other fields
*		are not valid. 
*in   : argc -> if != 0 then CLI
*               if == 0 then workbench or ML ( if size msg < procinfo ) ML
*out  : NULL -> CLI or WORKBENCH process
*               else ptr to PROCESSINFO structure 
*/
PROCESSINFO *ml_FindBaseAddr( int argc, char **argv)
{
	if( argc == 0 )
		if(((struct WBStartup *)argv)->sm_Message.mn_Length >=
			( sizeof(PROCESSINFO)-sizeof(struct Node)-sizeof(UWORD)))
			return((PROCESSINFO *)((ULONG)argv - (ULONG)sizeof(struct Node) - sizeof(UWORD)) );
	return( NULL );
}

/****************************
* Func	:	Create a filename with the path before the actual name
*	in		:	Path
*				Name
*				Dest
*	out	:	-
*/
void MakeFullPath( STRPTR Path, STRPTR Name, STRPTR Dest ) 
{
  int PathLength;

	PathLength = strlen(Path);
	if(Path[PathLength-1] == ':' || Path[PathLength-1] == '/')
	{
		strcpy(Dest,Path);
		strcat(Dest,Name);
	}
	else 
	{
		if(Path != NULL && Path[0] != '\0')
		{
			strcpy(Dest,Path);
			strcat(Dest,"/");
			strcat(Dest,Name);
		}
		else
			strcpy(Dest, Name);
	}	
}

/*******************************************
*Func :	Send a dialogue to our guide
*in   :	Msg_Dial -> the dialogue
*			PI 		 -> PI of task owner
*			Cmd 	 -> the command to be send
*out  :	TRUE -> ok
*			FALSE -> error
*/
BOOL SendDialogue( PROCDIALOGUE *Msg_Dial, PROCESSINFO *PI, int Cmd)
{
	if(Msg_Dial->pd_InUse)
		return(FALSE);

	Msg_Dial->pd_ChildPI = PI;
	Msg_Dial->pd_InUse = TRUE;
	Msg_Dial->pd_Cmd = Cmd;
	PutMsg(PI->pi_Port_CtoP,(struct Message *)Msg_Dial);
	return(TRUE);
}

/******** WaitJiffies() ********/

LONG WaitJiffies(struct ScriptNodeRecord *node, MLSYSTEM *MLSystem)
{
LONG start, end, h, m, s, jiffies=0;

	if ( MLSystem->miscFlags & 0x00000001L )	// PAL
	{
		h = 60*60*25;	// 90000 ('frames per hour')
		m = 60*25;		// 1500 fpm ('frames per minute')
		s = 25;			// 25 fps
	}
	else	// NTSC
	{
		h = 60*60*30;	// 10800 fph ('frames per hour')
		m = 60*30;		// 1800 fpm ('frames per minute')
		s = 30;			// 30 fps
	}

	if ( MLSystem->miscFlags & 0x00000002L )	// timecode
	{
		start =		node->Start.TimeCode.HH * h;
		start += (  node->Start.TimeCode.MM * m );
		start += (	node->Start.TimeCode.SS * s );
		start +=	node->Start.TimeCode.FF;

		end   =		node->End.TimeCode.HH * h;
		end   += (  node->End.TimeCode.MM * m );
		end   += (	node->End.TimeCode.SS * s );
		end   +=	node->End.TimeCode.FF;

		jiffies = end - start;		
	}
	else	// hhmmsst timecode (duration is total no. of seconds)
		jiffies = (node->duration/10) * s;	// multiply by fps

	if ( jiffies < 0 )
		return(0);	
	else
		return(jiffies * 2 );
}

#if 0
/******** DoObjectDateCheck() ********/

void GenObjectDateCheck(SNR *CurSNR)
{
struct DateStamp CurDate;
LONG CurMinutesSince1978, ObjMinutesSince1978Start, ObjMinutesSince1978End;
struct Library *DOSBase;

	DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 0L);
	if (!DOSBase)
		return;

	//if(ObjectDateCheck)
	{
		CurSNR->miscFlags &= ~OBJ_OUTDATED;

		DateStamp(&CurDate);

		if( CurSNR->dayBits & (1<<(CurDate.ds_Days % 7)) )
		{
			if(CurSNR->startendMode == ARGUMENT_CYCLICAL)
			{
				if((CurSNR->Start.HHMMSS.Minutes == 0) && (CurSNR->End.HHMMSS.Minutes == 0))
				{
					if(!(
						  (CurSNR->Start.HHMMSS.Days <= CurDate.ds_Days) &&
						  (CurSNR->End.HHMMSS.Days >= CurDate.ds_Days)
					  ) )	
						CurSNR->miscFlags |= OBJ_OUTDATED;
				}
				else
				{
					if(!(
						  (CurSNR->Start.HHMMSS.Days <= CurDate.ds_Days) &&
						  (CurSNR->End.HHMMSS.Days >= CurDate.ds_Days) &&
						  (CurSNR->Start.HHMMSS.Minutes <= CurDate.ds_Minute) &&
						  (CurSNR->End.HHMMSS.Minutes >= CurDate.ds_Minute)
					  ) )	
						CurSNR->miscFlags |= OBJ_OUTDATED;
				}
			}

			if(CurSNR->startendMode == ARGUMENT_CONTINUOUS)
			{
				CurMinutesSince1978 = (CurDate.ds_Days*1440)+CurDate.ds_Minute;
				ObjMinutesSince1978Start = (CurSNR->Start.HHMMSS.Days*1440)+CurSNR->Start.HHMMSS.Minutes;
				ObjMinutesSince1978End = (CurSNR->End.HHMMSS.Days*1440)+CurSNR->End.HHMMSS.Minutes;

				if(!(
					  (ObjMinutesSince1978Start <= CurMinutesSince1978) &&
					  (ObjMinutesSince1978End >= CurMinutesSince1978) 
				  ) )
					CurSNR->miscFlags |= OBJ_OUTDATED;
			}
		}
		else
			CurSNR->miscFlags |= OBJ_OUTDATED;
	}

	CloseLibrary((struct Library *)DOSBase);
}
#endif

/******** E O F ********/
