#include "nb:pre.h"
#include <devices/cdtv.h>
#include "cdtv.h"
#include "protos.h"

/**** externals ****/

extern struct Library *medialinkLibBase;

/**** gadgets ****/

extern struct GadgetRecord Host_GR[];

/**** functions ****/

/******** Monitor_SerialPort() ********/

BOOL Monitor_SerialPort(struct Window *window, struct CDTV_record *CDTV_rec)
{
ULONG mask, signal;
TEXT recBuff[256];
int i;
BOOL loop;

	mask =	(1L << window->UserPort->mp_SigBit) |
					(1L << CDTV_rec->serialMP->mp_SigBit);

	/**** the only way to leave this loop is by generating an IDCMP event ****/

	if( CheckIO((struct IORequest *)CDTV_rec->serialIO) )
	{
		AbortIO((struct IORequest *)CDTV_rec->serialIO);
		WaitIO((struct IORequest *)CDTV_rec->serialIO);
	}

	while(1)
	{
		/**** clear receive buffer ****/

		for(i=0; i<256; i++)
			recBuff[i] = '\0';

		/**** receive bytes from sender ****/

		CDTV_rec->serialIO->IOSer.io_Length 	= 64;
		CDTV_rec->serialIO->IOSer.io_Data			= (APTR)recBuff;
		CDTV_rec->serialIO->IOSer.io_Command	= CMD_READ;
		SendIO((struct IORequest *)CDTV_rec->serialIO);

		loop=TRUE;
		while(loop)
		{
			signal = Wait(mask);

			/**** window event ****/

			if ( signal & (1L << window->UserPort->mp_SigBit) )
				return(TRUE);

			/**** serial event ****/

			if( CheckIO((struct IORequest *)CDTV_rec->serialIO) )
			{
				WaitIO((struct IORequest *)CDTV_rec->serialIO);
				loop=FALSE;
				break;
			}
		}

		AbortIO((struct IORequest *)CDTV_rec->serialIO);
		WaitIO((struct IORequest *)CDTV_rec->serialIO);

		ParseTheString(window, recBuff, CDTV_rec);
	}
}

/******** PrintString() ********/

void PrintString(struct Window *window, STRPTR str)
{
	SetAPen(window->RPort, LO_PEN);
	SetBPen(window->RPort, AREA_PEN);

	ScrollRaster(	window->RPort, 0, window->RPort->TxBaseline+5,
								Host_GR[2].x1+2, Host_GR[2].y1+2,
								Host_GR[2].x2-2, Host_GR[2].y2-2); 

	Move(window->RPort, Host_GR[2].x1+5, Host_GR[2].y2-window->RPort->TxBaseline);
	Text(window->RPort, str, strlen(str));
}

/******** ParseTheString() ********/

void ParseTheString(struct Window *window, STRPTR recBuff,
										struct CDTV_record *CDTV_rec)
{
int i1,i2, pos;
TEXT s1[30], s2[30], s3[30], s4[30], s5[30], s6[30];
BOOL printOut=FALSE;

	pos = FindStringMax(recBuff, "PLAY TRACK", 10);
	if (pos!=-1)	// PLAY TRACK n FADE
	{
		sscanf(recBuff, "%s %s %d %s", s1,s2,&i1,s3);
		CDTV_rec->song = i1;
		if (strncmp(s3,"FADE",4)==0)
			CDTV_rec->fadeIn = TRUE;
		else
			CDTV_rec->fadeIn = FALSE;
		CDTV_PlayTrack(CDTV_rec);
		printOut=TRUE;
	}

	pos = FindStringMax(recBuff, "PLAY TRACKS", 11);
	if (pos!=-1)	// PLAY TRACKS n TO m FADE
	{
		sscanf(recBuff, "%s %s %d %s %d %s", s1,s2,&i1,s3,&i2,s4);
		CDTV_rec->from = i1;
		CDTV_rec->to = i2;
		if (strncmp(s4,"FADE",4)==0)
			CDTV_rec->fadeIn = TRUE;
		else
			CDTV_rec->fadeIn = FALSE;
		CDTV_PlayTrackFromTo(CDTV_rec);
		printOut=TRUE;
	}

	pos = FindStringMax(recBuff, "PLAY FROM", 9);
	if (pos!=-1)	// PLAY FROM mm:ss:ff TO mm:ss:ff FADE
	{
		sscanf(recBuff, "%s %s %s %s %s %s", s1,s2,s3,s4,s5,s6);
		strcpy(CDTV_rec->start, s3);
		strcpy(CDTV_rec->end, s5);
		if (strncmp(s6,"FADE",4)==0)
			CDTV_rec->fadeIn = TRUE;
		else
			CDTV_rec->fadeIn = FALSE;
		CDTV_PlayTrackStartEnd(CDTV_rec);
		printOut=TRUE;
	}

	pos = FindStringMax(recBuff, "PAUSE", 5);
	if (pos!=-1)	// PAUSE
	{
		CDTV_Pause(CDTV_rec);
		printOut=TRUE;
	}

	pos = FindStringMax(recBuff, "STOP", 4);
	if (pos!=-1)	// STOP
	{
		CDTV_Stop(CDTV_rec);
		printOut=TRUE;
	}

	pos = FindStringMax(recBuff, "FADE IN", 7);
	if (pos!=-1)	// FADE IN IN 5/10 SECONDS
	{
		sscanf(recBuff, "%s %s %s %d", s1,s2,s3,&i1);
		if (i1==5)	// fast
			CDTV_Fade(CDTV_rec, 2, 1);	// last 1 means fade IN
		else
			CDTV_Fade(CDTV_rec, 1, 1);
		printOut=TRUE;
	}

	pos = FindStringMax(recBuff, "FADE OUT", 8);
	if (pos!=-1)	// FADE OUT IN 5/10 SECONDS
	{
		sscanf(recBuff, "%s %s %s %d", s1,s2,s3,&i1);
		if (i1==5)	// fast
			CDTV_Fade(CDTV_rec, 2, 2);	// last 2 means fade OUT
		else
			CDTV_Fade(CDTV_rec, 1, 2);
		printOut=TRUE;
	}

	pos = FindStringMax(recBuff, "FRONTPANEL", 10);
	if (pos!=-1)	// FRONTPANEL ON/OFF
	{
		sscanf(recBuff, "%s %s", s1,s2);
		if (strncmp(s2,"ON",2)==0)
			CDTV_FrontPanel(CDTV_rec, 1);
		else
			CDTV_FrontPanel(CDTV_rec, 2);
		printOut=TRUE;
	}

	pos = FindStringMax(recBuff, "MUTE", 4);
	if (pos!=-1)	// MUTE ON/OFF
	{
		sscanf(recBuff, "%s %s", s1,s2);
		if (strncmp(s2,"ON",2)==0)
			CDTV_Mute(CDTV_rec, 1);
		else
			CDTV_Mute(CDTV_rec, 2);
		printOut=TRUE;
	}

	pos = FindStringMax(recBuff, "RESET", 5);
	if (pos!=-1)
	{
		CDTV_Reset(CDTV_rec);
		printOut=TRUE;
	}

#if 0
	pos = FindStringMax(recBuff, "FAST FORWARD", 12);
	if (pos!=-1)
	{
		sscanf(recBuff, "%s %s %d", dummy,dummy,&from);
//		CDTV_PlayFastFwd(window, CDTV_rec, from);
		printOut=TRUE;
	}

	pos = FindStringMax(recBuff, "FAST REWIND", 11);
	if (pos!=-1)
	{
		sscanf(recBuff, "%s %s %d", dummy,dummy,&from);
//		CDTV_PlayFastRew(window, CDTV_rec, from);
		printOut=TRUE;
	}
#endif

	/**** special cases ****/

	pos = FindStringMax(recBuff, "ISPLAYING", 9);
	if (pos!=-1)
	{
		SendIsPlayingInfo(CDTV_rec);
		printOut=FALSE;
	}

	pos = FindStringMax(recBuff, "GET", 3);
	if (pos!=-1)
	{
		SendGetCDInfo(CDTV_rec);
		printOut=FALSE;
	}

	if (printOut)
		PrintString(window, recBuff);
}

/******** SendIsPlayingInfo() ********/

void SendIsPlayingInfo(struct CDTV_record *CDTV_rec)
{
TEXT index[30], track[30], str[256];

	CDTV_IsPlaying(CDTV_rec, TRUE, track, index);
	sprintf(str, "%s %s", track, index);
	if ( !SendStringViaSer(CDTV_rec, str) )
		GiveMessage(CDTV_rec->window, "Serial communication failed!");
}

/******** SendGetCDInfo() ********/

void SendGetCDInfo(struct CDTV_record *CDTV_rec)
{
TEXT duration[30], numTracks[30], str[256];

	CDTV_GetCDInfo(CDTV_rec, numTracks, duration);
	sprintf(str, "%s %s", numTracks, duration);
	if ( !SendStringViaSer(CDTV_rec, str) )
		GiveMessage(CDTV_rec->window, "Serial communication failed!");
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
