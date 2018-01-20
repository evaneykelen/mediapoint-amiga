#include "nb:pre.h"
#include "protos.h"

/**** externals ****/

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *medialinkLibBase;
extern struct RendezVousRecord *rvrec;
extern char *scriptCommands[];
extern struct EventData CED;
extern struct Window *printWindow;
extern UBYTE **msgs;

/**** globals ****/

union printerIO
{
	struct IOStdReq ios;
	struct IODRPReq iodrp;
	struct IOPrtCmdReq iopc;
};

struct MsgPort *PrinterMP;
union printerIO *PIO;

ULONG printerAllocs=0L;
#define DEVICE_OPENED	0x00000001L
#define EXTIO_OPENED	0x00000002L
#define PORT_OPENED		0x00000004L

struct PStat
{
	UBYTE LSB;
	UBYTE MSB;
};

UWORD PaperLength;

/**** gadgets ****/

extern struct GadgetRecord PrintWindow_GR[];

/**** functions ****/

/******** GetWBPrinterPrefs() ********/

UBYTE GetWBPrinterPrefs(void)
{
struct Preferences *PrefBuffer;
UBYTE retval=2;

	PrefBuffer = (struct Preferences *)AllocMem(sizeof(struct Preferences), (ULONG)MEMF_CLEAR);
	if (PrefBuffer == NULL)
	{
		UA_WarnUser(212);
		return(2);
	}
	GetPrefs(PrefBuffer, sizeof(struct Preferences));
	retval=PrefBuffer->PrinterPort;
	PaperLength=PrefBuffer->PaperLength;
//printser("PaperLength=%d\n",PaperLength);
	FreeMem(PrefBuffer, sizeof(struct Preferences));

	return(retval);
}

/******** SetWBPrinterPrefs() ********/

void SetWBPrinterPrefs(void)
{
struct Preferences *PrefBuffer;

	PrefBuffer = (struct Preferences *)AllocMem(sizeof(struct Preferences), (ULONG)MEMF_CLEAR);
	if (PrefBuffer == NULL)
	{
		UA_WarnUser(213);
		return;
	}

	GetPrefs(PrefBuffer, sizeof(struct Preferences));

	if (rvrec->capsprefs->printerScaleAndOri == PRINTER_ORI_PORTRAIT ||
			rvrec->capsprefs->printerScaleAndOri == PRINTER_ORI_PORTRAIT4X4 )
		PrefBuffer->PrintAspect = ASPECT_HORIZ; //ASPECT_VERT;
	else if ( rvrec->capsprefs->printerScaleAndOri == PRINTER_ORI_LANDSCAPE ||
						rvrec->capsprefs->printerScaleAndOri == PRINTER_ORI_LANDSCAPE4X4 )
		PrefBuffer->PrintAspect = ASPECT_VERT; //ASPECT_HORIZ;

	if ( rvrec->capsprefs->printerQuality == PRINTER_QUALITY_DRAFT )
		PrefBuffer->PrintQuality = DRAFT;
	else if ( rvrec->capsprefs->printerQuality == PRINTER_QUALITY_LETTER )
		PrefBuffer->PrintQuality = LETTER;

	SetPrefs(PrefBuffer, sizeof(struct Preferences), FALSE);

	FreeMem(PrefBuffer, sizeof(struct Preferences));
}

/******** OpenPrinter() ********/

BOOL OpenPrinter(void)
{
	printerAllocs=0L;

	if ( PrinterMP = (struct MsgPort *)CreateMsgPort() )	//CreatePort(0,0) )
	{
		printerAllocs |= PORT_OPENED;
		if (PIO = (union printerIO *)CreateExtIO(PrinterMP, sizeof(union printerIO)))
		{
//printser("PIO 1 = %x\n",PIO);
			printerAllocs |= EXTIO_OPENED;
			if (!(OpenDevice("printer.device",0,(struct IORequest *)PIO,0)))
			{
				printerAllocs |= DEVICE_OPENED;
				return(TRUE);
			}
			else
				UA_WarnUser(214);
		}
		else
			UA_WarnUser(215);
	}
	else
		UA_WarnUser(216);

	return(FALSE);
}

/******** ClosePrinter() ********/

void ClosePrinter(void)
{
	if ( printerAllocs & DEVICE_OPENED )
		CloseDevice( (struct IORequest *)PIO );	

	if ( printerAllocs & EXTIO_OPENED )
		DeleteExtIO( (struct IORequest *)PIO );	

	if ( printerAllocs & PORT_OPENED )
		DeletePort( (struct MsgPort *)PrinterMP );	
}

/******** IsPrinterTurnedOn() ********/

BOOL IsPrinterTurnedOn(void)
{
UBYTE retval;
struct PStat status;

	retval = GetWBPrinterPrefs();
	if (retval==2)
		return(FALSE);

	PIO->ios.io_Data = &status;
	PIO->ios.io_Command = PRD_QUERY;
	DoIO((struct IORequest *)PIO);

	if (retval==PARALLEL_PRINTER)
	{
		if (status.LSB==4)
			return(TRUE);
		else
			return(FALSE);
	}
	else if (retval==SERIAL_PRINTER)
		return(TRUE);	/* not yet implemented */
}

/******** PrintScript() ********/

void PrintScript(void)
{
TEXT str[256];
TEXT str2[256];
int i,j,line,page;
struct ScriptNodeRecord *this_node;
struct ScriptInfoRecord *SIR;
BOOL aborted=FALSE;
BOOL formFeed=FALSE;
BOOL PrintingWindow=FALSE;
int pages;

	SIR = &(rvrec->ObjRec->scriptSIR);

	SetWBPrinterPrefs();

	if ( OpenPrinter() )
	{
//printser("printer opened\n");
		if ( !IsPrinterTurnedOn() )
		{
//printser("printer off\n");
			Message(msgs[Msg_PrinterClosed-1]);
			ClosePrinter();
			return;
		}
		else
		{
//printser("printer on\n");
			PrintingWindow = OpenPrintingWindow( msgs[Msg_Script-1] );
			if (!PrintingWindow)
			{
				ClosePrinter();
				return;
			}

			for(pages=0; pages<rvrec->capsprefs->printerCopies; pages++)
			{
//printser("printing page %d\n",pages);

				/**** init vars ****/

				aborted=FALSE;
				formFeed=FALSE;

				/**** print header ****/

				page=1;	/* page number */
				line=0;	/* printed lines counter, max = PaperLength */

				/**** pages, and header/footer when page starts and ends ****/

				for(i=0; i<rvrec->capsprefs->MaxNumLists; i++)
				{
//printser("i=%d\n",i);

					if (SIR->allLists[i] != NULL)
					{
//printser("list %d has data\n",i);

						/**** eject page, but not the first time ****/

						if (formFeed)
						{
							aborted = PrintFormFeed();
							if (aborted)
								goto stop_printing;
						}
						else
							formFeed=TRUE;

						/**** print header ****/

//printser("START print header\n");

						line=0;
						aborted = PrintHeader();
						if (aborted)
							goto stop_printing;
						line += 2;

//printser("END print header\n");

						/**** print event list ****/

						for(this_node=(struct ScriptNodeRecord *)SIR->allLists[i]->lh_Head;
								this_node->node.ln_Succ;
								this_node=(struct ScriptNodeRecord *)this_node->node.ln_Succ)
						{
//printser("node [%s] [%s]\n", this_node->objectName, this_node->objectPath);
							/**** create string to be printed ****/

							if ( CreatePrintString(SIR, this_node, str, str2, PIO, &line, &aborted, i, &page) )
							{
								aborted = PrintString(PIO, str);
								if (aborted)
									goto stop_printing;
								line++;

								if ( SIR->timeCodeFormat == TIMEFORMAT_HHMMSS && str2[0]!='\0' )
								{
									aborted = PrintString(PIO, str2);
									if (aborted)
										goto stop_printing;
									line++;
								}
							}
							if (aborted)
								goto stop_printing;

							/**** check if form feed in needed ****/

							PerformFormFeed(PIO,&line,&aborted,&page,i);
							if ( aborted )
								goto stop_printing;
						}

						/**** roll paper up until last line ****/

						for(j=line; j<(PaperLength-2); j++)
						{
							aborted = PrintString(PIO, "\n");
							if (aborted)
								goto stop_printing;
						}

						/**** print footer ****/

						aborted = PrintFooter(page, i);
						if (aborted)
							goto stop_printing;
						page++;
					}
				}

				/**** go for next copy ****/

				if (rvrec->capsprefs->printerCopies>1)
				{
					aborted = PrintFormFeed();
					if (aborted)
						goto stop_printing;
				}
			}
		}
	}
	else
		Message(msgs[Msg_PrinterClosed-1]);

stop_printing:
	//if (!aborted)
	ClosePrinter();

	if (PrintingWindow)
		ClosePrintingWindow();
}

/******** CreatePrintString() ********/

BOOL CreatePrintString(	struct ScriptInfoRecord *SIR,
												struct ScriptNodeRecord *this_node,
												STRPTR str, STRPTR str2,
												union printerIO *request, int *line, BOOL *aborted,
												int listLevel, int *page )
{
TEXT timeStr[80];
int i,j;

	str[0] = '\0';
	str2[0] = '\0';

	/**** get object type name ****/

	sprintf(str, "%s: ", scriptCommands[this_node->nodeType]);

	/**** skip spaces ****/

	j=strlen(str);
	for (i=0; i<(15-j); i++)
		strcat(str, " ");

	/**** get object name ****/

	if ( this_node->nodeType == TALK_GLOBALEVENT )
	{
		DoGlobalEventInfo(SIR,this_node,request,line,aborted,listLevel,page);
		return(FALSE);	// nothing to print hereafter, already did that
	}
	else if ( this_node->nodeType == TALK_TIMECODE )
	{
		DoTimeCodeInfo(SIR,this_node,request,line,aborted,listLevel,page);
		return(FALSE);	// nothing to print hereafter, already did that
	}
	else if ( this_node->nodeType == TALK_VARS )
	{
		DoVarsInfo(SIR,this_node,request,line,aborted,listLevel,page);
		return(FALSE);	// nothing to print hereafter, already did that
	}
	
	GetObjInfo(this_node, timeStr);
	strcat(str, timeStr);

	if (SIR->timeCodeFormat == TIMEFORMAT_HHMMSS)
	{
		j=strlen(str);
		for (i=0; i<(40-j); i++)
			strcat(str, " ");

		if (this_node->duration != -1)
		{
			secondsToDuration(this_node->duration, timeStr);
			strcat(str, timeStr);
		}

		if (this_node->dayBits != -1)
		{
			strcat(str, "  ");

			if (this_node->dayBits & 1)
				strcat(str, "SU|");
			if (this_node->dayBits & 2)
				strcat(str, "MO|");
			if (this_node->dayBits & 4)
				strcat(str, "TU|");
			if (this_node->dayBits & 8)
				strcat(str, "WE|");
			if (this_node->dayBits & 16)
				strcat(str, "TH|");
			if (this_node->dayBits & 32)
				strcat(str, "FR|");
			if (this_node->dayBits & 64)
				strcat(str, "SA");

			if ( str[strlen(str)-1] == '|' )
				str[strlen(str)-1] = '\0';

			if (this_node->Start.HHMMSS.Days != -1)
			{
				datestampToDate(this_node->Start.HHMMSS.Days, timeStr);
				strcat(str2, timeStr);
				datestampToDate(this_node->End.HHMMSS.Days, timeStr);
				strcat(str2, "  ");
				strcat(str2, timeStr);
				datestampToTime(this_node->Start.HHMMSS.Minutes, this_node->Start.HHMMSS.Ticks, timeStr);
				strcat(str2, "  ");
				strcat(str2, timeStr);
				datestampToTime(this_node->End.HHMMSS.Minutes, this_node->End.HHMMSS.Ticks, timeStr);
				strcat(str2, "  ");
				strcat(str2, timeStr);

				if (this_node->startendMode==ARGUMENT_CYCLICAL)
					strcat(str2, "  CYCLICAL");
				else if (this_node->startendMode==ARGUMENT_CONTINUOUS)
					strcat(str2, " CONTINUOUS");
			}
		}
	}
	else
	{
		if (this_node->Start.TimeCode.HH != -1)
		{
			j=strlen(str);
			for (i=0; i<(40-j); i++)
				strcat(str, " ");

			sprintf(timeStr, ",%02d:%02d:%02d:%02d, %02d:%02d:%02d:%02d",
							this_node->Start.TimeCode.HH, this_node->Start.TimeCode.MM, this_node->Start.TimeCode.SS, this_node->Start.TimeCode.FF,
							this_node->End.TimeCode.HH, this_node->End.TimeCode.MM, this_node->End.TimeCode.SS, this_node->End.TimeCode.FF);
			strcat(str, timeStr);
		}
	}

	if (str[0] != '\0')
		strcat(str, "\n");

	if (str2[0] != '\0')
		strcat(str2, "\n");

	return(TRUE);
}

/******** PrintHeader() ********/

BOOL PrintHeader(void)
{
TEXT str[25];
int a,b,c;
BOOL aborted=FALSE;

//printser("PIO 2 = %x\n",PIO);

	aborted = PrintCommand(PIO, aSGR0, 0, 0, 0, 0);	/* NORMAL */
	if (aborted)
		return(aborted);

	aborted = PrintCommand(PIO, aSGR1, 0, 0, 0, 0);	/* BOLD ON */
	if (aborted)
		return(aborted);

	SystemDate(&a,&b,&c);
	DayMonthYearToString(str, a,b,c);			

	aborted = PrintString(PIO, str);
	if (aborted)
		return(aborted);
	aborted = PrintString(PIO, "\t");	/* tab */
	if (aborted)
		return(aborted);

	SystemTime(&a,&b,&c);
	HoursMinsSecsTenthsToString(str, a,b,c,0);			
	str[8]='\0';	/* cut of tenths of second */
	strcat(str, "\n\n");

	aborted = PrintString(PIO, str);
	if (aborted)
		return(aborted);

	aborted = PrintCommand(PIO, aSGR22, 0, 0, 0, 0);	/* BOLD OFF */
	if (aborted)
		return(aborted);

	return(FALSE);	/* NOT aborted */
}

/******** PrintFooter() ********/

BOOL PrintFooter(int page, int list)
{
TEXT str[80];
BOOL aborted=FALSE;

	aborted = PrintCommand(PIO, aSGR0, 0, 0, 0, 0);	/* NORMAL */
	if (aborted)
		return(aborted);

	aborted = PrintCommand(PIO, aSGR1, 0, 0, 0, 0);	/* BOLD ON */
	if (aborted)
		return(aborted);

	sprintf(str, "%s: %d  ---  %s: %d\n", msgs[Msg_Footer1-1], page, msgs[Msg_Footer2-1], list+1);
	aborted = PrintString(PIO, str);
	if (aborted)
		return(aborted);

	aborted = PrintCommand(PIO, aSGR22, 0, 0, 0, 0);	/* BOLD OFF */
	if (aborted)
		return(aborted);
}

/******** PrintCommand() ********/

BOOL PrintCommand(union printerIO *request, int command,
									int p0, int p1, int p2, int p3)
{
	/**** queue a printer command ****/
	request->iopc.io_Command		= PRD_PRTCOMMAND;
	request->iopc.io_PrtCommand	= command;
	request->iopc.io_Parm0			= p0;
	request->iopc.io_Parm1			= p1;
	request->iopc.io_Parm2		 	= p2;
	request->iopc.io_Parm3 			= p3;

	return(sendPrinterIO(request));
}

/******** PrintString() ********/

BOOL PrintString(union printerIO *request, STRPTR string)
{
	request->ios.io_Command	= CMD_WRITE;
	request->ios.io_Data		= (char *)string;
	request->ios.io_Length	= -1L;

	/* if -1, the printer assumes it has been given
	 * a null terminated string.
	 */

	//printser("[%s]\n",string);

	return(sendPrinterIO(request));
}

/******** sendPrinterIO() ********/

BOOL sendPrinterIO(union printerIO *request)
{
ULONG signal;
int j;
BOOL aborted=FALSE;
struct IntuiMessage *message;
//USHORT key;
struct Window *window;
//USHORT raw;
//char key;
//int numkeys;

	window = printWindow;

	/**** send command to printer ****/
//printser("start sendIO\n");
	SendIO((struct IORequest *)request);
//printser("end sendIO\n");
	/**** wait for printer or user clicking on cancel ****/

opnieuw:
//printser("start Wait() %x %x\n",(1L << PrinterMP->mp_SigBit),(1L << window->UserPort->mp_SigBit));
	signal = Wait( 	(1L << PrinterMP->mp_SigBit) |
									(1L << window->UserPort->mp_SigBit) );
//printser("end Wait()\n");
	/**** process clicking on cancel ****/

	if (signal & (1L << window->UserPort->mp_SigBit))
	{
		while(message = (struct IntuiMessage *)GetMsg(window->UserPort))
		{	
			CED.Class		 = message->Class;
			CED.Code 		 = message->Code;
			CED.Qualifier= message->Qualifier;
			CED.MouseX 	 = message->MouseX;
			CED.MouseY 	 = message->MouseY;
//			DoKeys(message, &key);
//			UA_GetKeys(&raw, &key, &numkeys, &CED);
			ReplyMsg((struct Message *)message);

			if (CED.Class==IDCMP_MOUSEBUTTONS && CED.Code==SELECTDOWN)
			{
				if( UA_CheckGadgetList(window, PrintWindow_GR, &CED)==2)
				{
					UA_HiliteButton(window, &PrintWindow_GR[2]);
					AbortIO((struct IORequest *)PIO);
					WaitIO((struct IORequest *)PIO);
					aborted=TRUE;
				}
			}
/*
			else if (	CED.Class==IDCMP_RAWKEY &&
								(CED.Qualifier&IEQUALIFIER_RCOMMAND) &&
								key=='.' )
			{
				UA_HiliteButton(window, &PrintWindow_GR[2]);
				AbortIO((struct IORequest *)PIO);
				WaitIO((struct IORequest *)PIO);
				aborted=TRUE;
			}
*/
		}
		if (aborted)
			return(aborted);				
		else
			goto opnieuw;
	}
//printser("after loop\n");
	/**** process printer ****/

	if (signal & (1L << PrinterMP->mp_SigBit))
	{
		/* printer is either ready or an error has occured */
		/* remove any messages */

		while(GetMsg(PrinterMP))
			;
	}

	if (PIO->iodrp.io_Error != 0)
	{
		j = PIO->iodrp.io_Error;
		if (j<0)
			j = j * -1 + 7;
//Message("Printer error %d occured", j);
		aborted=TRUE;
	}	

	return(aborted);
}

/******** PrintFormFeed() ********/

BOOL PrintFormFeed(void)
{
	PIO->ios.io_Command	= CMD_WRITE;
	PIO->ios.io_Data		= "\014";
	PIO->ios.io_Length	= -1L;

	return(sendPrinterIO(PIO));
}

/******** DoGlobalEventInfo() ********/

void DoGlobalEventInfo(	struct ScriptInfoRecord *SIR,
												struct ScriptNodeRecord *this_node,
												union printerIO *request, int *line, BOOL *aborted,
												int listLevel, int *page )
{
int i,j,k;
struct ScriptEventRecord *SER;
TEXT str[256], buf[256];

	for(i=0; i<MAX_GLOBAL_EVENTS; i++)
	{
		SER = SIR->globallocalEvents[i];
		if (SER != NULL)
		{
			sprintf(str, "%s: ", scriptCommands[this_node->nodeType]);
			j=strlen(str);
			for (k=0; k<(15-j); k++)
				strcat(str, " ");
			if (SER->keyCode != -1)
			{
				sprintf(&str[15], "GLOBALEVENT \"%c\", \"%s\"\n", SER->keyCode, SER->labelName);	//scrStr_1);
				*aborted = PrintString(PIO, str);
				if ( *aborted )
					return;
				*line = *line + 1;
				PerformFormFeed(PIO,line,aborted,page,listLevel);
				if ( *aborted )
					return;
			}
			else if (SER->rawkeyCode != -1)
			{
				KeyToKeyName(SER->keyCode,SER->rawkeyCode,buf);
				sprintf(&str[15], "GLOBALEVENT %s, \"%s\"\n", buf, SER->labelName);	//scrStr_1);
				*aborted = PrintString(PIO, str);
				if ( *aborted )
					return;
				*line = *line + 1;
				PerformFormFeed(PIO,line,aborted,page,listLevel);
				if ( *aborted )
					return;
			}
		}
	}
}

/******** KeyToKeyName() ********/

void KeyToKeyName(int key, int raw, STRPTR keyName)
{
	if (key != -1)
		sprintf(keyName, "%c", toupper(key));
	else if (raw != -1)
	{
		switch(raw)
		{
			case TALK_HELP_KC: 				strcpy(keyName, TALK_HELP_KT); 				break;
			case TALK_ESC_KC: 				strcpy(keyName, TALK_ESC_KT); 				break;
			case TALK_F1_KC: 					strcpy(keyName, TALK_F1_KT); 					break;
			case TALK_F2_KC: 					strcpy(keyName, TALK_F2_KT); 					break;
			case TALK_F3_KC: 					strcpy(keyName, TALK_F3_KT); 					break;
			case TALK_F4_KC: 					strcpy(keyName, TALK_F4_KT); 					break;
			case TALK_F5_KC: 					strcpy(keyName, TALK_F5_KT); 					break;
			case TALK_F6_KC: 					strcpy(keyName, TALK_F6_KT); 					break;
			case TALK_F7_KC: 					strcpy(keyName, TALK_F7_KT); 					break;
			case TALK_F8_KC: 					strcpy(keyName, TALK_F8_KT); 					break;
			case TALK_F9_KC: 					strcpy(keyName, TALK_F9_KT); 					break;
			case TALK_F10_KC: 				strcpy(keyName, TALK_F10_KT); 				break;
			case TALK_CURSORUP_KC: 		strcpy(keyName, TALK_CURSORUP_KT); 		break;
			case TALK_CURSORDOWN_KC: 	strcpy(keyName, TALK_CURSORDOWN_KT); 	break;
			case TALK_CURSORLEFT_KC: 	strcpy(keyName, TALK_CURSORLEFT_KT); 	break;
			case TALK_CURSORRIGHT_KC:	strcpy(keyName, TALK_CURSORRIGHT_KT); break;
			case TALK_TAB_KC: 				strcpy(keyName, TALK_TAB_KT); 				break;
			case TALK_DEL_KC: 				strcpy(keyName, TALK_DEL_KT); 				break;
			case TALK_BACKSPACE_KC: 	strcpy(keyName, TALK_BACKSPACE_KT); 	break;
			case TALK_RETURN_KC: 			strcpy(keyName, TALK_RETURN_KT); 			break;
			case TALK_SPACE_KC: 			strcpy(keyName, TALK_SPACE_KT); 			break;

			case TALK_OPEN_BRACKET_KC:	strcpy(keyName, TALK_OPEN_BRACKET_KT);	break;
			case TALK_CLOSE_BRACKET_KC:	strcpy(keyName, TALK_CLOSE_BRACKET_KT);	break;
			case TALK_STAR_KC:				strcpy(keyName, TALK_STAR_KT);				break;
			case TALK_PLUS_KC:				strcpy(keyName, TALK_PLUS_KT);				break;

			default: 									strcpy(keyName, TALK_ESC_KT); 				break;
		}
	}
	else
		keyName[0]='\0';
}

/******** DoTimeCodeInfo() ********/

void DoTimeCodeInfo(	struct ScriptInfoRecord *SIR,
											struct ScriptNodeRecord *this_node,
											union printerIO *request, int *line, BOOL *aborted,
											int listLevel, int *page )
{
int j,k;
TEXT str[256], str2[256];

	sprintf(str, "%s: ", scriptCommands[this_node->nodeType]);
	j=strlen(str);
	for (k=0; k<(15-j); k++)
		strcat(str, " ");

	if ( SIR->timeCodeSource == TIMESOURCE_EXTERNAL )
		strcat(str, "EXTERNAL, ");
	else
		strcat(str, "INTERNAL, ");

	if ( SIR->timeCodeFormat == TIMEFORMAT_HHMMSS)
		strcat(str, "HHMMSS");
	else if ( SIR->timeCodeFormat == TIMEFORMAT_MIDI)
		strcat(str, "MIDI");
	else if ( SIR->timeCodeFormat == TIMEFORMAT_SMPTE)
		strcat(str, "SMPTE");
	else if ( SIR->timeCodeFormat == TIMEFORMAT_MLTC)
		strcat(str, "MLTC");
	else if ( SIR->timeCodeFormat == TIMEFORMAT_CUSTOM)
		strcat(str, "CUSTOM");

	if ( SIR->timeCodeFormat != TIMEFORMAT_HHMMSS)
	{
		if ( SIR->timeCodeRate == TIMERATE_24FPS)
			strcat(str, ", 24FPS");
		else if ( SIR->timeCodeRate == TIMERATE_25FPS)
			strcat(str, ", 25FPS");
		else if ( SIR->timeCodeRate == TIMERATE_30FPS_DF)
			strcat(str, ", 30FPS_DF");
		else if ( SIR->timeCodeRate == TIMERATE_30FPS)
			strcat(str, ", 30FPS");
		else if ( SIR->timeCodeRate == TIMERATE_MIDICLOCK)
			strcat(str, ", MIDICLOCK");

		if ( SIR->timeCodeOut )
			strcat(str, ", TRUE");
		else
			strcat(str, ", FALSE");

		sprintf(str2, ", %02d:%02d:%02d:%02d",
						SIR->Offset.TimeCode.HH,
						SIR->Offset.TimeCode.MM,
						SIR->Offset.TimeCode.SS,
						SIR->Offset.TimeCode.FF);
		strcat(str,str2);
	}

	sprintf(str2, ", %d, %d, %d, %d",
					rvrec->capsprefs->scriptTiming+1,
					rvrec->capsprefs->objectPreLoading,
					rvrec->capsprefs->playOptions,
					rvrec->capsprefs->bufferOptions+1);
	strcat(str,str2);

	if ( SIR->timeCodeFormat == TIMEFORMAT_CUSTOM )
	{
		sprintf(str2, ", %s", rvrec->capsprefs->customTimeCode);
		strcat(str,str2);
	}

	strcat(str, "\n");

	*aborted = PrintString(PIO, str);
	*line = *line + 1;
}

/******** DoVarsInfo() ********/

void DoVarsInfo(	struct ScriptInfoRecord *SIR,
									struct ScriptNodeRecord *this_node,
									union printerIO *request, int *line, BOOL *aborted,
									int listLevel, int *page )
{
VIR *this_vir;
VAR *this_var;
TEXT buf[256];
int j,k;

	if ( listLevel==0 )	// PRINT DECLARATION
	{
		for(this_vir = (VIR *)SIR->VIList.lh_Head; 
				(VIR *)this_vir->vir_Node.ln_Succ;	
				this_vir = (VIR *)this_vir->vir_Node.ln_Succ)
		{
			sprintf(buf, "%s: ", scriptCommands[this_node->nodeType]);
			j=strlen(buf);
			for (k=0; k<(15-j); k++)
				strcat(buf, " ");

			VIRToDecl(this_vir, &buf[15]);
			strcat(buf,"\n");
			*aborted = PrintString(PIO, buf);
			if ( *aborted )
				return;
			*line = *line + 1;
			PerformFormFeed(PIO,line,aborted,page,listLevel);
			if ( *aborted )
				return;
		}
	}
	else	// PRINT EXPRESSION
	{
		if ( this_node->list )
		{
			for(this_var = (VAR *)this_node->list->lh_Head; 
					(VAR *)this_var->var_Node.ln_Succ;	
					this_var = (VAR *)this_var->var_Node.ln_Succ)
			{
				sprintf(buf, "%s: ", scriptCommands[this_node->nodeType]);
				j=strlen(buf);
				for (k=0; k<(15-j); k++)
					strcat(buf, " ");
				VARToExpr(SIR, &buf[15], this_var);
				strcat(buf,"\n");
				*aborted = PrintString(PIO, buf);
				if ( *aborted )
					return;
				*line = *line + 1;
				PerformFormFeed(PIO,line,aborted,page,listLevel);
				if ( *aborted )
					return;
			}
		}
	}
}

/******** VIRToDecl() ********/

void VIRToDecl(VIR *this_vir, STRPTR declStr)
{
	if ( this_vir->vir_Type == VCT_INTEGER )
		sprintf(declStr, "%s = %d", this_vir->vir_Name, this_vir->vir_Integer);
	else if ( this_vir->vir_Type == VCT_STRING )
		sprintf(declStr, "%s = \"%s\"", this_vir->vir_Name, this_vir->vir_String);
}

/******** VARToExpr() ********/

BOOL VARToExpr(struct ScriptInfoRecord *SIR, STRPTR declStr, VAR *this_var)
{
BOOL allOK=FALSE;
VCR *vcr;
TEXT temp[100];

	if (	this_var->var_CondList.lh_TailPred !=
				(struct Node *)&this_var->var_CondList )	// cond. list is NOT empty
	{
		vcr = (VCR *)this_var->var_CondList.lh_Head;
		if ( vcr )	// THIS IS A CONDITION EXPRESSION
		{
			// CREATE 'IF ...' PART 

			sprintf(declStr, "IF %s ", this_var->var_ResultVIR->vir_Name);

			// CREATE CONDITION PART (<, > etc )

			if ( vcr->vcr_Condition == COND_LESS )
				strcat(declStr, "< ");			
			else if ( vcr->vcr_Condition == COND_LESSEQUAL )
				strcat(declStr, "<= ");			
			else if ( vcr->vcr_Condition == COND_EQUAL )
				strcat(declStr, "= ");			
			else if ( vcr->vcr_Condition == COND_GREATEREQUAL )
				strcat(declStr, ">= ");			
			else if ( vcr->vcr_Condition == COND_GREATER )
				strcat(declStr, "> ");			
			else if ( vcr->vcr_Condition == COND_NOTEQUAL )
				strcat(declStr, "<> ");			

			// CREATE AFTER CONDITION PART (IF ... COND ...)

			if ( vcr->vcr_CheckBits & VC_CHECKVIR )
			{
				sprintf(temp, "%s ", vcr->vcr_Check.vc_VIR->vir_Name);
				strcat(declStr, temp);
			}
			else
			{
				if ( vcr->vcr_Type == VCT_INTEGER )
				{
					sprintf(temp, "%d ", vcr->vcr_Integer);
					strcat(declStr, temp);
				}
				else if ( vcr->vcr_Type == VCT_STRING )
				{
					sprintf(temp, "\"%s\" ", vcr->vcr_String);
					strcat(declStr, temp);
				}
			}

			// CREATE GOTO

			if ( vcr->vcr_JumpType == TG_GOTO )
				strcat(declStr, "GOTO ");
			else if ( vcr->vcr_JumpType == TG_GOSUB )
				strcat(declStr, "GOSUB ");
			else if ( vcr->vcr_JumpType == TG_SPNEXTGOSUB )
				strcat(declStr, "GONEXT ");
			else if ( vcr->vcr_JumpType == TG_SPPREVGOSUB )
				strcat(declStr, "GOPREV ");
			else
				strcat(declStr, "*ERROR* ");

			// CREATE LABEL TO JUMP TO

			strcat(declStr,"\"");
			strcat(declStr, vcr->vcr_LabelSNR->objectName);
			strcat(declStr,"\"");
		}
	}
	else	// THIS IS AN EXPRESSION (NOT A CONDITION)
	{
		// CREATE 'VAR1 = '
		
		sprintf(declStr, "%s = ", this_var->var_ResultVIR->vir_Name);

		// CREATE 'VAR1 = VAR2'

		if ( this_var->var_SourceBits & SB_SOURCE1VIR )
		{
			sprintf(temp, "%s ", this_var->var_Source1.vs_VIR->vir_Name);
			strcat(declStr, temp);
		}
		else
		{
			if ( this_var->var_Type1 == VCT_INTEGER )
			{
				sprintf(temp, "%d ", this_var->var_Integer1);
				strcat(declStr, temp);
			}
			else if ( this_var->var_Type1 == VCT_STRING )
			{
				sprintf(temp, "\"%s\" ", this_var->var_String1);
				strcat(declStr, temp);
			}
		}

		// 'EXPRESSIONS' CAN BE DECLARATIONS OR EXPRESSIONS

		if ( this_var->var_Type2 != VCT_EMPTY )
		{
			// CREATE 'VAR1 = VAR2 + '

			if ( this_var->var_Operator == OPER_ADD )
				strcat(declStr, "+ ");
			else if ( this_var->var_Operator == OPER_SUBTRACT )
				strcat(declStr, "- ");
			else if ( this_var->var_Operator == OPER_MULTIPLY )
				strcat(declStr, "* ");
			else if ( this_var->var_Operator == OPER_DIVIDE )
				strcat(declStr, "/ ");

			// CREATE 'VAR1 = VAR2 + VAR3'

			if ( this_var->var_SourceBits & SB_SOURCE2VIR )
			{
				sprintf(temp, "%s", this_var->var_Source2.vs_VIR->vir_Name);
				strcat(declStr, temp);
			}
			else
			{
				if ( this_var->var_Type2 == VCT_INTEGER )
				{
					sprintf(temp, "%d", this_var->var_Integer2);
					strcat(declStr, temp);
				}
				else if ( this_var->var_Type2 == VCT_STRING )
				{
					sprintf(temp, "\"%s\"", this_var->var_String2);
					strcat(declStr, temp);
				}
			}
		}
	}

	return( allOK );
}

/******** PerformFormFeed() ********/

void PerformFormFeed(	union printerIO *request, int *line, BOOL *aborted, int *page,
											int list )
{
int j;

	if (*line>=(PaperLength-4))
	{
//printser("line = %d  PaperLength = %d\n", *line, PaperLength);

		/**** roll paper up until last line ****/

		for(j=*line; j<(PaperLength-2); j++)
		{
			*aborted = PrintString(request, "\n");
			if (*aborted)
				return;
		}

		/**** print footer ****/

		*aborted = PrintFooter(*page, list);
		if (*aborted)
			return;
		*page = *page + 1;

		/**** eject paper ****/

		*aborted = PrintFormFeed();
		if (*aborted)
			return;

		/**** print header ****/

		*line=0;
		*aborted = PrintHeader();
		if (*aborted)
			return;
		*line = *line + 2;
	}
}

/******** E O F ********/
