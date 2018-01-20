#include "nb:pre.h"
#include "demo:gen/wait50hz.h"
#include "protos.h"
#include "all.h"

/**** functions ****/

/******** Monitor_SerialPort() ********/

BOOL Monitor_SerialPort(struct Window *window, struct all_record *all_rec,
												struct CDTV_record *CDTV_rec,
												struct CD_record *CD_rec, struct MPEG_record *MPEG_rec)
{
ULONG mask, signal;
TEXT recBuff[30], temp[10];
int i;
BOOL loop, parseIt;

	if( CheckIO((struct IORequest *)all_rec->serialIO) )
	{
		AbortIO((struct IORequest *)all_rec->serialIO);
		WaitIO((struct IORequest *)all_rec->serialIO);
	}

	for(i=0; i<30; i++)
		recBuff[i] = '\0';
	temp[1]='\0';
	parseIt = FALSE;

	while(1)	// only and IDCMP can break this loop
	{
		/**** receive bytes from sender ****/

		all_rec->serialIO->IOSer.io_Length 	= 1;
		all_rec->serialIO->IOSer.io_Data		= (APTR)temp;	//recBuff;
		all_rec->serialIO->IOSer.io_Command	= CMD_READ;
		SendIO((struct IORequest *)all_rec->serialIO);

		loop=TRUE;
		mask =	(1L << window->UserPort->mp_SigBit) |
						(1L << all_rec->serialMP->mp_SigBit);

		while(loop)
		{
			signal = Wait(mask);

			/**** window event ****/

			if ( signal & (1L << window->UserPort->mp_SigBit) )
			{
				AbortIO((struct IORequest *)all_rec->serialIO);
				WaitIO((struct IORequest *)all_rec->serialIO);
				return(TRUE);
			}

			/**** serial event ****/

			if( loop && CheckIO((struct IORequest *)all_rec->serialIO) )
			{
				WaitIO((struct IORequest *)all_rec->serialIO);
				loop=FALSE;
				if ( strlen(recBuff)<25 && temp[0]!='!' )
					strcat(recBuff,temp);
				else
					recBuff[0]='\0';	// start again
				if ( temp[0]==0x0a || temp[0]==0x0d )
					parseIt = TRUE;
				else
					parseIt = FALSE;
			}
		}

		AbortIO((struct IORequest *)all_rec->serialIO);
		WaitIO((struct IORequest *)all_rec->serialIO);

		if ( parseIt )
		{
			ParseTheString(window, recBuff, all_rec, CDTV_rec, CD_rec, MPEG_rec);
		}
	}
}

/******** ParseTheString() ********/

void ParseTheString(struct Window *window, STRPTR recBuff,
										struct all_record *all_rec,
										struct CDTV_record *CDTV_rec,
										struct CD_record *CD_rec, struct MPEG_record *MPEG_rec)
{
TEXT cmd[32], s1[32], s2[32], s3[32];	// string arguments
TEXT str[50];
TEXT info[100];
int i1=0, i2=0, i3=0;	// integer arguments

	if ( FindStringMax(recBuff, "P1", 2) != -1 )
	{
		/*** P1 track 1/0 ***/
		sscanf(recBuff,"%s %d %d", cmd, &i1, &i2);
		/*** Inform user ***/
		sprintf(str, "Play track %d", i1);
		if (i2)
			strcat(str," - fade");
		PrintString(window, str);
		/*** Execute command ***/
		if ( CDTV_rec && CDTV_DiskIsValid(CDTV_rec) )
		{
			if ( CDTV_DiskHasAudio(CDTV_rec) )
				CDTV_PlayTrack(CDTV_rec, i1,i2,i3, s1,s2,s3);
		}
		else if ( CD_rec && MPEG_rec && CD_DiskIsValid(CD_rec) )
		{
			MPEG_PlayTrack(CD_rec, MPEG_rec, i1,i2,i3, s1,s2,s3);
		}
		else if ( CD_rec && CD_DiskIsValid(CD_rec) && CD_DiskHasAudio(CD_rec) )
		{
			CD_PlayTrack(CD_rec, i1,i2,i3, s1,s2,s3);
		}
	}

	else if ( FindStringMax(recBuff, "P2", 2) != -1 )
	{
		/*** P2 startTrack endTrack 1/0 ***/
		sscanf(recBuff,"%s %d %d %d", cmd, &i1, &i2, &i3);
		/*** Inform user ***/
		sprintf(str, "Play tracks %d to %d", i1, i2);
		if (i3)
			strcat(str," - fade");
		PrintString(window, str);
		/*** Execute command ***/
		if ( CDTV_rec && CDTV_DiskIsValid(CDTV_rec) )
		{
			CDTV_PlayTrackFromTo(CDTV_rec, i1,i2,i3, s1,s2,s3);
		}
		else if ( CD_rec && MPEG_rec && CD_DiskIsValid(CD_rec) )
		{
			MPEG_PlayTrackFromTo(CD_rec, MPEG_rec, i1,i2,i3, s1,s2,s3);
		}
		else if ( CD_rec && CD_DiskIsValid(CD_rec) )
		{
			if ( CD_DiskHasAudio(CD_rec) )
				CD_PlayTrackFromTo(CD_rec, i1,i2,i3, s1,s2,s3);
		}
	}

	else if ( FindStringMax(recBuff, "P3", 2) != -1 )
	{
		/*** P3 startTime endTime 1/0 ***/
		sscanf(recBuff,"%s %s %s %d", cmd, s1, s2, &i1);
		/*** Inform user ***/
		sprintf(str, "Play from %s to %s", s1, s2);
		if (i1)
			strcat(str," - fade");
		PrintString(window, str);
		/*** Execute command ***/
		if ( CDTV_rec && CDTV_DiskIsValid(CDTV_rec) )
		{
			CDTV_PlayTrackStartEnd(CDTV_rec, i1,i2,i3, s1,s2,s3);
		}
		else if ( CD_rec && MPEG_rec && CD_DiskIsValid(CD_rec) )
		{
			MPEG_PlayTrackStartEnd(CD_rec, MPEG_rec, i1,i2,i3, s1,s2,s3);
		}
		else if ( CD_rec && CD_DiskIsValid(CD_rec) )
		{
			if ( CD_DiskHasAudio(CD_rec) )
				CD_PlayTrackStartEnd(CD_rec, i1,i2,i3, s1,s2,s3);
		}
	}

	else if ( FindStringMax(recBuff, "PA", 2) != -1 )
	{
		/*** Inform user ***/
		PrintString(window, "Pause");
		/*** Execute command ***/
		if ( CDTV_rec && CDTV_DiskIsValid(CDTV_rec) )
		{
			CDTV_Pause(CDTV_rec, i1,i2,i3, s1,s2,s3);
		}
		else if ( CD_rec && MPEG_rec && CD_DiskIsValid(CD_rec) )
		{
			MPEG_Pause(CD_rec, MPEG_rec, i1,i2,i3, s1,s2,s3);
		}
		else if ( CD_rec && CD_DiskIsValid(CD_rec) )
		{
			if ( CD_DiskHasAudio(CD_rec) )
				CD_Pause(CD_rec, i1,i2,i3, s1,s2,s3);
		}
	}

	else if ( FindStringMax(recBuff, "ST", 2) != -1 )
	{
		/*** Inform user ***/
		PrintString(window, "Stop");
		/*** Execute command ***/
		if ( CDTV_rec && CDTV_DiskIsValid(CDTV_rec) )
		{
			CDTV_Stop(CDTV_rec, i1,i2,i3, s1,s2,s3);
		}
		if ( CD_rec && MPEG_rec && CD_DiskIsValid(CD_rec) )
		{
			MPEG_Stop(CD_rec, MPEG_rec, i1,i2,i3, s1,s2,s3);
		}
		if ( CD_rec && CD_DiskIsValid(CD_rec) )
		{
			CD_Stop(CD_rec, i1,i2,i3, s1,s2,s3);
		}
	}

	else if ( FindStringMax(recBuff, "F1", 2) != -1 )
	{
		/*** Inform user ***/
		PrintString(window, "Fade in slow");
		/*** Execute command ***/
		i1=1;
		if ( CDTV_rec && CDTV_DiskIsValid(CDTV_rec) )
		{
			CDTV_Fade(CDTV_rec, i1,i2,i3, s1,s2,s3);
		}
		else if ( CD_rec && MPEG_rec && CD_DiskIsValid(CD_rec) )
		{
			if ( CD_DiskHasAudio(CD_rec) )
				CD_Fade(CD_rec, i1,i2,i3, s1,s2,s3);
		}
		else if ( CD_rec && CD_DiskIsValid(CD_rec) )
		{
			if ( CD_DiskHasAudio(CD_rec) )
				CD_Fade(CD_rec, i1,i2,i3, s1,s2,s3);
		}
	}

	else if ( FindStringMax(recBuff, "F2", 2) != -1 )
	{
		/*** Inform user ***/
		PrintString(window, "Fade in fast");
		/*** Execute command ***/
		i1=2;
		if ( CDTV_rec && CDTV_DiskIsValid(CDTV_rec) )
		{
			CDTV_Fade(CDTV_rec, i1,i2,i3, s1,s2,s3);
		}
		else if ( CD_rec && MPEG_rec && CD_DiskIsValid(CD_rec) )
		{
			if ( CD_DiskHasAudio(CD_rec) )
				CD_Fade(CD_rec, i1,i2,i3, s1,s2,s3);
		}
		else if ( CD_rec && CD_DiskIsValid(CD_rec) )
		{
			if ( CD_DiskHasAudio(CD_rec) )
				CD_Fade(CD_rec, i1,i2,i3, s1,s2,s3);
		}
	}

	else if ( FindStringMax(recBuff, "F3", 2) != -1 )
	{
		/*** Inform user ***/
		PrintString(window, "Fade out slow");
		/*** Execute command ***/
		i1=3;
		if ( CDTV_rec && CDTV_DiskIsValid(CDTV_rec) )
		{
			CDTV_Fade(CDTV_rec, i1,i2,i3, s1,s2,s3);
		}
		else if ( CD_rec && MPEG_rec && CD_DiskIsValid(CD_rec) )
		{
			if ( CD_DiskHasAudio(CD_rec) )
				CD_Fade(CD_rec, i1,i2,i3, s1,s2,s3);
		}
		else if ( CD_rec && CD_DiskIsValid(CD_rec) )
		{
			if ( CD_DiskHasAudio(CD_rec) )
				CD_Fade(CD_rec, i1,i2,i3, s1,s2,s3);
		}
	}

	else if ( FindStringMax(recBuff, "F4", 2) != -1 )
	{
		/*** Inform user ***/
		PrintString(window, "Fade out fast");
		/*** Execute command ***/
		i1=4;
		if ( CDTV_rec && CDTV_DiskIsValid(CDTV_rec) )
		{
			CDTV_Fade(CDTV_rec, i1,i2,i3, s1,s2,s3);
		}
		else if ( CD_rec && MPEG_rec && CD_DiskIsValid(CD_rec) )
		{
			if ( CD_DiskHasAudio(CD_rec) )
				CD_Fade(CD_rec, i1,i2,i3, s1,s2,s3);
		}
		else if ( CD_rec && CD_DiskIsValid(CD_rec) )
		{
			if ( CD_DiskHasAudio(CD_rec) )
				CD_Fade(CD_rec, i1,i2,i3, s1,s2,s3);
		}
	}

	else if ( FindStringMax(recBuff, "M1", 2) != -1 )
	{
		/*** Inform user ***/
		PrintString(window, "Mute on");
		/*** Execute command ***/
		i1=1;
		if ( CDTV_rec && CDTV_DiskIsValid(CDTV_rec) )
		{
			CDTV_Mute(CDTV_rec, i1,i2,i3, s1,s2,s3);
		}
		else if ( CD_rec && MPEG_rec && CD_DiskIsValid(CD_rec) )
		{
			if ( CD_DiskHasAudio(CD_rec) )
				CD_Mute(CD_rec, i1,i2,i3, s1,s2,s3);
		}
		else if ( CD_rec && CD_DiskIsValid(CD_rec) )
		{
			if ( CD_DiskHasAudio(CD_rec) )
				CD_Mute(CD_rec, i1,i2,i3, s1,s2,s3);
		}
	}

	else if ( FindStringMax(recBuff, "M2", 2) != -1 )
	{
		/*** Inform user ***/
		PrintString(window, "Mute off");
		/*** Execute command ***/
		i1=2;
		if ( CDTV_rec && CDTV_DiskIsValid(CDTV_rec) )
		{
			CDTV_Mute(CDTV_rec, i1,i2,i3, s1,s2,s3);
		}
		else if ( CD_rec && MPEG_rec && CD_DiskIsValid(CD_rec) )
		{
			if ( CD_DiskHasAudio(CD_rec) )
				CD_Mute(CD_rec, i1,i2,i3, s1,s2,s3);
		}
		else if ( CD_rec && CD_DiskIsValid(CD_rec) )
		{
			if ( CD_DiskHasAudio(CD_rec) )
				CD_Mute(CD_rec, i1,i2,i3, s1,s2,s3);
		}
	}

	else if ( FindStringMax(recBuff, "CD", 2) != -1 )
	{
		/*** Inform user ***/
		PrintString(window, "New CD");
		/*** Execute command ***/
		if ( CDTV_rec && CDTV_DiskIsValid(CDTV_rec) )
		{
			CDTV_NewCD(CDTV_rec);
		}
		else if ( CD_rec && MPEG_rec && CD_DiskIsValid(CD_rec) )
		{
			CD_NewCD(CD_rec);
		}
		else if ( CD_rec && CD_DiskIsValid(CD_rec) )
		{
			if ( CD_DiskHasAudio(CD_rec) )
				CD_NewCD(CD_rec);
		}
	}

	else if ( FindStringMax(recBuff, "GI", 2) != -1 )
	{
		/*** Execute command ***/
		if ( CDTV_rec && CDTV_DiskIsValid(CDTV_rec) )
		{
			CDTV_GetInfo(CDTV_rec,info);
			PutStringToSer(all_rec,info);
		}
		else if ( CD_rec && MPEG_rec && CD_DiskIsValid(CD_rec) )
		{
			CD_GetInfo(CD_rec,info);
			PutStringToSer(all_rec,info);
		}
		else if ( CD_rec && CD_DiskIsValid(CD_rec) )
		{
			if ( CD_DiskHasAudio(CD_rec) )
			{
				CD_GetInfo(CD_rec,info);
				PutStringToSer(all_rec,info);
			}
		}
	}

	else if ( FindStringMax(recBuff, "GT", 2) != -1 )
	{
		/*** GT track ***/
		sscanf(recBuff,"%s %d", cmd, &i1);
		/*** Execute command ***/
		if ( CDTV_rec && CDTV_DiskIsValid(CDTV_rec) )
		{
			CDTV_GetTrack(CDTV_rec,info,i1);
			PutStringToSer(all_rec,info);
		}
		else if ( CD_rec && MPEG_rec && CD_DiskIsValid(CD_rec) )
		{
			CD_GetTrack(CD_rec,info,i1);
			PutStringToSer(all_rec,info);
		}
		else if ( CD_rec && CD_DiskIsValid(CD_rec) )
		{
			if ( CD_DiskHasAudio(CD_rec) )
			{
				CD_GetTrack(CD_rec,info,i1);
				PutStringToSer(all_rec,info);
			}
		}
	}

	else if ( FindStringMax(recBuff, "SS", 2) != -1 )
	{
		/*** Inform user ***/
		PrintString(window, "Single step");
		/*** Execute command ***/
		if ( CD_rec && MPEG_rec && CD_DiskIsValid(CD_rec) )
		{
			MPEG_SingleStep(CD_rec, MPEG_rec, i1,i2,i3, s1,s2,s3);
		}
	}

	else if ( FindStringMax(recBuff, "S0", 2) != -1 )
	{
		/*** Inform user ***/
		PrintString(window, "Slow motion off");
		/*** Execute command ***/
		if ( CD_rec && MPEG_rec && CD_DiskIsValid(CD_rec) )
		{
			i1=0;
			MPEG_SlowMotion(CD_rec, MPEG_rec, i1,i2,i3, s1,s2,s3);
		}
	}

	else if ( FindStringMax(recBuff, "S1", 2) != -1 )
	{
		/*** Inform user ***/
		PrintString(window, "Video but no audio");
		/*** Execute command ***/
		if ( CD_rec && MPEG_rec && CD_DiskIsValid(CD_rec) )
		{
			i1=1;
			MPEG_SlowMotion(CD_rec, MPEG_rec, i1,i2,i3, s1,s2,s3);
		}
	}

	else if ( FindStringMax(recBuff, "S2", 2) != -1 )
	{
		/*** Inform user ***/
		PrintString(window, "Slow motion (half speed)");
		/*** Execute command ***/
		if ( CD_rec && MPEG_rec && CD_DiskIsValid(CD_rec) )
		{
			i1=2;
			MPEG_SlowMotion(CD_rec, MPEG_rec, i1,i2,i3, s1,s2,s3);
		}
	}

	else if ( FindStringMax(recBuff, "S3", 2) != -1 )
	{
		/*** Inform user ***/
		PrintString(window, "Slow motion (quarter speed)");
		/*** Execute command ***/
		if ( CD_rec && MPEG_rec && CD_DiskIsValid(CD_rec) )
		{
			i1=4;
			MPEG_SlowMotion(CD_rec, MPEG_rec, i1,i2,i3, s1,s2,s3);
		}
	}

	recBuff[0]='\0';	// start again
}

/******** FindStringMax() ********/
/*
 * returns -1 on not found, char pos on found
 *
 */

int FindStringMax(STRPTR OrgString, STRPTR CheckString, int MaxLength)
{
int i, Length;

	i = 0;
	Length = strlen(CheckString);
	MaxLength -= Length;
	while(i < (MaxLength+1))
	{
		if(!strncmp(&OrgString[i], CheckString, Length))
			return(i);
		i++;
	}
	return(-1);
}

/******** E O F ********/
