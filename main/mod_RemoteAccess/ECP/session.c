#include "nb:pre.h"
#include "msm:structs.h"
#include <fcntl.h>
#include "protos.h"
#include "structs.h"

/**** externals ****/

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *medialinkLibBase;
extern struct SMFuncs funcs[];
extern char *commands[];
extern UBYTE **msgs;
extern FILE *logfile;
extern struct RendezVousRecord *rvrec;
extern char *CDF_Keywords[];

/**** functions ****/

/***********************************************************************************
 * Step 1 - Read CDF, check class, set defaults, read settings   ValidateSession()
 * Step 2 -   Initialize connection                              InitConnection()
 * Step 3 -     Open connection                                  OpenConnection()
 * Step 4 -       Log on                                         LogOn()
 * Step 5 -         Do transaction                               DoTransaction()
 * Step 6 -       Log off                                        LogOff()
 * Step 7 -     Close connection                                 CloseConnection()
 * Step 8 -   Deinitialize connection                            DeInitConnection()
 ***********************************************************************************/

/******** Session() ********/

BOOL Session(STRPTR CDF, STRPTR TempScript, STRPTR BigFile, STRPTR Type, STRPTR LogName )
{
BOOL retVal=FALSE;
struct CDF_Record CDF_rec;
//TEXT path[SIZE_FULLPATH];
long oldpri;
struct Task *task;

	logfile = NULL;

	// Init CDF record

	Init_CDF_Record(&CDF_rec);

	if( Type )
		sscanf(Type,"%d",&CDF_rec.ChangeScriptType );

	if( LogName )
		logfile = fopen(LogName,"a" );

	if (rvrec)
	{
		//UA_MakeFullPath(rvrec->aPtrThree, "CDF_Drawer", path);
		//UA_MakeFullPath(path, CDF, CDF_rec.CDF_Path);
		strcpy(CDF_rec.CDF_Path, CDF);
	}
	else	// if started in CLI, then CDF is already a complete path
	{
		strcpy(CDF_rec.CDF_Path, CDF);
	}
	CDF_rec.TempScript_Path = TempScript;
	CDF_rec.BigFile_Path = BigFile;

	// Parse CDF file

	if ( !Parse_CDF_File(&CDF_rec) )
	{
		//printser("Error during CDF file parsing.\n");
		return(FALSE);
	}

	// DEBUG
#if 0
	printser("ConnectionClass = [%s]\n", CDF_rec.ConnectionClass); 
	printser("SerialDevice    = [%s]\n", CDF_rec.SerialDevice);
	printser("UnitNumber      = [%s]\n", CDF_rec.UnitNumber);
	printser("BaudRate        = [%s]\n", CDF_rec.BaudRate);
	printser("HandShaking     = [%s]\n", CDF_rec.HandShaking);
	printser("BufferSize      = [%s]\n", CDF_rec.BufferSize);
	printser("DialPrefix      = [%s]\n", CDF_rec.DialPrefix);
	printser("PhoneNr         = [%s]\n", CDF_rec.PhoneNr);
	printser("PassWord        = [%s]\n", CDF_rec.PassWord);
	printser("DestinationPath = [%s]\n", CDF_rec.DestinationPath);
	printser("ReplyString     = [%s]\n", CDF_rec.ReplyString);
#endif

	// DEBUG

	task = FindTask( 0 );
	oldpri = SetTaskPri(task, 127 );

	// Do Job

	while( CDF_rec.SendError != 0 && (--CDF_rec.RetryCount) > 0 && !IsTheButtonHit() )
	{

		CDF_rec.SendError = 0;												// By start reset error

		if ( ValidateSession(&CDF_rec) )
		{
			if ( InitConnection(&CDF_rec) )
			{
				if ( OpenConnection(&CDF_rec) )
				{
					if ( LogOn(&CDF_rec) )
					{
						retVal = ProcessBigFile(&CDF_rec); 		// cals DoTransaction()
						if ( retVal )													// for each file in BigFile
							LogOff(&CDF_rec);
					}
					CloseConnection(&CDF_rec);
				}
				DeInitConnection(&CDF_rec);
			}
		}
	}

	if( logfile )
		fclose( logfile );

	return(retVal);
}

#if 0

/******** Init_CDF_Record() ********/

void Init_CDF_Record(struct CDF_Record *CDF_rec)
{
	// Part 1

	CDF_rec->CDF_Path[0] = '\0';
	CDF_rec->TempScript_Path = NULL;
	CDF_rec->BigFile_Path = NULL;

	// Part 2

	strcpy(CDF_rec->ConnectionClass, "-1");
	strcpy(CDF_rec->SerialDevice, "serial.device");
	strcpy(CDF_rec->UnitNumber, "1");
	strcpy(CDF_rec->BaudRate, "9600");
	strcpy(CDF_rec->HandShaking, "XON/XOFF");
	strcpy(CDF_rec->BufferSize, "512");
	strcpy(CDF_rec->DialPrefix, "ATDT");
	strcpy(CDF_rec->ReplyString, "CONNECT");

	CDF_rec->PhoneNr[0] = '\0';
	strcpy(CDF_rec->PassWord, "changeme");
	CDF_rec->DestinationPath[0] = '\0';
	CDF_rec->ChangeScriptType = 0;			// change direct
	CDF_rec->SendError = 1;
	CDF_rec->RetryCount = 4;

	// Part 3

	CDF_rec->misc_ptr1 = NULL;
	CDF_rec->misc_ptr2 = NULL;
	CDF_rec->misc_ptr3 = NULL;
	CDF_rec->misc_ptr4 = NULL;
}

/******** Parse_CDF_File() ********/
BOOL Parse_CDF_File(struct CDF_Record *CDF_rec)
{
struct ParseRecord *PR;
int instruc, line;
UBYTE *buffer;
TEXT whiteSpcs[512];

	/**** open parser and alloc mem ****/

	PR = (struct ParseRecord *)OpenParseFile(CDF_Keywords,CDF_rec->CDF_Path);
	if (PR==NULL)
		return(FALSE);

	buffer = (UBYTE *)AllocMem(SCRIPT_LINESIZE, MEMF_CLEAR);
	if ( buffer==NULL )
	{
		CloseParseFile(PR);
		return(FALSE);
	}

	/**** parse all lines ****/

	instruc = -1;
	line = 0;
	while(instruc!=PARSE_STOP)
	{
		instruc = Special_GetParserLine((struct ParseRecord *)PR, buffer, whiteSpcs);
		if (instruc == PARSE_INTERPRET)
		{
			passOneParser((struct ParseRecord *)PR, buffer);
			if (passTwoParser((struct ParseRecord *)PR))
			{
				PR->sourceLine = line+1;
				RemoveQuotes(PR->argString[1]);
				switch(PR->commandCode)
				{
					case CDF_CONNECTIONCLASS:
						strcpy(CDF_rec->ConnectionClass, PR->argString[1]);
						break;
					case CDF_SERIALDEVICE:
						strcpy(CDF_rec->SerialDevice, PR->argString[1]);
						break;
					case CDF_UNITNUMBER:
						strcpy(CDF_rec->UnitNumber, PR->argString[1]);
						break;
					case CDF_BAUDRATE:
						strcpy(CDF_rec->BaudRate, PR->argString[1]);
						break;
					case CDF_HANDSHAKING:
						strcpy(CDF_rec->HandShaking, PR->argString[1]);
						break;
					case CDF_BUFFERSIZE:
						strcpy(CDF_rec->BufferSize, PR->argString[1]);
						break;
					case CDF_DIALPREFIX:
						strcpy(CDF_rec->DialPrefix, PR->argString[1]);
						break;
					case CDF_PHONENR:
						strcpy(CDF_rec->PhoneNr, PR->argString[1]);
						break;
					case CDF_PASSWORD:
						strcpy(CDF_rec->PassWord, PR->argString[1]);
						break;
					case CDF_DESTINATIONPATH:
						strcpy(CDF_rec->DestinationPath, PR->argString[1]);
						break;
					case CDF_REPLYSTRING:
						strcpy(CDF_rec->ReplyString, PR->argString[1]);
						break;
				}
			}
		}
		line++;
	}

	/**** free memory ****/

	FreeMem(buffer,SCRIPT_LINESIZE);
	CloseParseFile(PR);

	return(TRUE);
}
#endif

/******** RemoveQuotes() ********/

void RemoveQuotes(STRPTR str)
{
int i,len;

	/* remove trailing and ending ' or " */
	len = strlen(str)-2;
	if (len>0)
	{
		for(i=0; i<len; i++)
			str[i] = str[i+1];
		str[len] = '\0';
	}
	else
		str[0] = '\0';
}

/******** E O F ********/
