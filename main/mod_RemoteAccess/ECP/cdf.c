#include "nb:pre.h"
#include <fcntl.h>
#include "msm:structs.h"
#include "mra:ECP/structs.h"

/**** externals ****/

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *medialinkLibBase;
extern struct SMFuncs funcs[];
extern char *commands[];
extern UBYTE **msgs;
extern TEXT reportStr[256];

/**** functions ****/

char *CDF_Keywords[] =
{
"CONNECTIONCLASS", "SERIALDEVICE", "UNITNUMBER", "BAUDRATE", "HANDSHAKING",
"BUFFERSIZE", "DIALPREFIX", "PHONENR", "PASSWORD", "DESTINATIONPATH",
"REPLYSTRING",
NULL /* ALWAYS END WITH NULL! */
};

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
	strcpy(CDF_rec->HandShaking, "NONE");
	strcpy(CDF_rec->BufferSize, "512");
	strcpy(CDF_rec->DialPrefix, "ATDT");
	strcpy(CDF_rec->ReplyString, "CONNECT");

	CDF_rec->PhoneNr[0] = '\0';
	strcpy(CDF_rec->PassWord, "password");
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

/******** Write_CDF_File() ********/

BOOL Write_CDF_File(struct CDF_Record *CDF_rec, STRPTR path)
{
FILE *fp;
int cc;

	// CC_SERIALCABLE=1, CC_PARALLELCABLE, CC_SCSI, CC_MODEM, CC_NETWORK, CC_DATABROADCAST,
	// CC_INFRARED, CC_EMAIL, CC_ISDN, CC_LEASED_LINE

	// CDF_CONNECTIONCLASS, CDF_SERIALDEVICE, CDF_UNITNUMBER, CDF_BAUDRATE, CDF_HANDSHAKING,
	// CDF_BUFFERSIZE, CDF_DIALPREFIX, CDF_PHONENR, CDF_PASSWORD, CDF_DESTINATIONPATH,
	// CDF_REPLYSTRING,

	fp = fopen(path,"w");
	if ( !fp )
		return(FALSE);

	sscanf(CDF_rec->ConnectionClass,"%d",&cc);

	if ( cc == CC_SERIALCABLE )
	{
		fprintf(fp, "%s \"%s\"\n", CDF_Keywords[CDF_CONNECTIONCLASS], CDF_rec->ConnectionClass);
		fprintf(fp, "%s \"%s\"\n", CDF_Keywords[CDF_SERIALDEVICE], CDF_rec->SerialDevice);
		fprintf(fp, "%s \"%s\"\n", CDF_Keywords[CDF_UNITNUMBER], CDF_rec->UnitNumber);
		fprintf(fp, "%s \"%s\"\n", CDF_Keywords[CDF_BAUDRATE], CDF_rec->BaudRate);
		fprintf(fp, "%s \"%s\"\n", CDF_Keywords[CDF_HANDSHAKING], CDF_rec->HandShaking);
		fprintf(fp, "%s \"%s\"\n", CDF_Keywords[CDF_BUFFERSIZE], CDF_rec->BufferSize);
		fprintf(fp, "%s \"%s\"\n", CDF_Keywords[CDF_PASSWORD], CDF_rec->PassWord);
	}
	else if ( cc == CC_MODEM || cc == CC_LEASED_LINE )
	{
		fprintf(fp, "%s \"%s\"\n", CDF_Keywords[CDF_CONNECTIONCLASS], CDF_rec->ConnectionClass);
		fprintf(fp, "%s \"%s\"\n", CDF_Keywords[CDF_SERIALDEVICE], CDF_rec->SerialDevice);
		fprintf(fp, "%s \"%s\"\n", CDF_Keywords[CDF_UNITNUMBER], CDF_rec->UnitNumber);
		fprintf(fp, "%s \"%s\"\n", CDF_Keywords[CDF_BAUDRATE], CDF_rec->BaudRate);
		fprintf(fp, "%s \"%s\"\n", CDF_Keywords[CDF_HANDSHAKING], CDF_rec->HandShaking);
		fprintf(fp, "%s \"%s\"\n", CDF_Keywords[CDF_BUFFERSIZE], CDF_rec->BufferSize);
		fprintf(fp, "%s \"%s\"\n", CDF_Keywords[CDF_PASSWORD], CDF_rec->PassWord);
		fprintf(fp, "%s \"%s\"\n", CDF_Keywords[CDF_REPLYSTRING], CDF_rec->ReplyString);
		if ( cc == CC_MODEM )
		{
			fprintf(fp, "%s \"%s\"\n", CDF_Keywords[CDF_DIALPREFIX], CDF_rec->DialPrefix);
			fprintf(fp, "%s \"%s\"\n", CDF_Keywords[CDF_PHONENR], CDF_rec->PhoneNr);
		}
	}
	else if ( cc == CC_NETWORK )
	{
		fprintf(fp, "%s \"%s\"\n", CDF_Keywords[CDF_CONNECTIONCLASS], CDF_rec->ConnectionClass);
		fprintf(fp, "%s \"%s\"\n", CDF_Keywords[CDF_DESTINATIONPATH], CDF_rec->DestinationPath);
	}

	fclose(fp);
	return(TRUE);
}

/******** E O F ********/
