#include "nb:pre.h"
#include "protos.h"

/**** externals ****/

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct Library *medialinkLibBase;
extern struct RendezVousRecord *rvrec;
extern struct EventData CED;
extern struct UserApplicInfo req_UAI;
extern UBYTE **msgs;
extern UWORD PaperLength;

union printerIO
{
	struct IOStdReq ios;
	struct IODRPReq iodrp;
	struct IOPrtCmdReq iopc;
};

extern union printerIO *PIO;

/**** gadgets ****/

extern struct GadgetRecord Right_button_GR[];

/**** functions ****/

/******** PrintPage() ********/

void PrintPage(void)
{
//BOOL aborted=FALSE;
BOOL PrintingWindow=FALSE;
int pages;

	SetWBPrinterPrefs();

	if ( OpenPrinter() )
	{
		if ( !IsPrinterTurnedOn() )
		{
			Message(msgs[Msg_PrinterClosed-1]);
			ClosePrinter();
			return;
		}
		else
		{
			PrintingWindow = OpenPrintingWindow( msgs[Msg_Document-1] );
			if (!PrintingWindow)
			{
				ClosePrinter();
				return;
			}

			/**** print ****/

			for(pages=0; pages<rvrec->capsprefs->printerCopies; pages++)
			{
				if ( rvrec->capsprefs->printerTextOnly )
					DumpTexts();
				else
					DumpPage(100);	// 100 is scale (not used currently)
			}
		}
	}
	else
		Message(msgs[Msg_PrinterClosed-1]);

	//if (!aborted)
	ClosePrinter();

	if (PrintingWindow)
		ClosePrintingWindow();
}

/******** DumpPage() ********/

void DumpPage(int vergroting)
{
int width, height, modes;
ULONG dc, dr;
float per, destcols, destrows;
UWORD factor;

	per = (float)vergroting;	// percentage, 100 is max

	if ( rvrec->capsprefs->PageScreenModes & LACE )
		factor=2;
	else
		factor=1;

	width = rvrec->capsprefs->PageScreenWidth;
	height = rvrec->capsprefs->PageScreenHeight;
	modes = rvrec->capsprefs->PageScreenModes;

	destcols = per/100;
	destrows = per/100;

	dc = 6700;	/* 6.7 inch is default width */
	dr = 5300;	/* 5.3 inch is default height */

	dc = dc * destcols;
	dr = dr * destrows;

	DumpRPort(	PIO,								/* pointer to initialized request */
							(struct RastPort *)rvrec->aPtr,
							rvrec->pagescreen->ViewPort.ColorMap,
							modes,  						/* low, high res, etc (display modes)*/
							0, 0,   						/* x and y offsets into rastport */
							width,height,				/* source size */
							dc, dr,							/* dest rows, columns */
							SPECIAL_CENTER | SPECIAL_ASPECT |
							SPECIAL_MILCOLS | SPECIAL_MILROWS );
}

/******** DumpRPort() ********/

BOOL DumpRPort(	union printerIO *request,
								struct RastPort *rastPort, struct ColorMap *colorMap,
								ULONG modes, UWORD sx, UWORD sy, UWORD sw, UWORD sh,
								LONG dc, LONG dr, UWORD s)
{
	request->iodrp.io_Command		= PRD_DUMPRPORT;
	request->iodrp.io_RastPort	= rastPort;
	request->iodrp.io_ColorMap	= colorMap;
	request->iodrp.io_Modes			= modes;
	request->iodrp.io_SrcX			= sx;
	request->iodrp.io_SrcY			= sy;
	request->iodrp.io_SrcWidth	= sw;
	request->iodrp.io_SrcHeight	= sh;
	request->iodrp.io_DestCols	= dc;
	request->iodrp.io_DestRows	= dr;
	request->iodrp.io_Special		= s;

	return ( sendPrinterIO(request) );
}

/******** DumpTexts() ********/

void DumpTexts(void)
{
int i,k,line=0,page=1;
char str[5];

	str[1] = '\0';

	for(k=0; k<MAXEDITWINDOWS; k++)
	{
		if ( rvrec->EW[k] && rvrec->EW[k]->TEI )
		{
			for(i=0; i<rvrec->EW[k]->TEI->textLength; i++)
			{
				if ( rvrec->EW[k]->TEI->text[ i ].charCode == '\n' )
					line++;
				if ( rvrec->EW[k]->TEI->text[ i ].charCode != '\0' )
				{
					str[0] = rvrec->EW[k]->TEI->text[i].charCode;
//printser("[%s] ",str);
					if ( PrintString(PIO,str) )
						return;
					if ( CheckFF(&line,&page,FALSE) )
						return;
				}
			}
//printser("\n\n");
			if ( PrintString(PIO,"\n\n") )
				return;
			line += 2;

			if ( CheckFF(&line,&page,FALSE) )
				return;
		}
	}

	CheckFF(&line,&page,TRUE);
}

/******** CheckFF() ********/
/*
 * returns TRUE on failure ("abort==TRUE")
 *
 */

BOOL CheckFF(int *line, int *page, BOOL doFF)
{
int j;

//printser("%d %d \n", *line, PaperLength);

	if ( doFF || (*line >= (PaperLength-4)) )	// paper 's up!
	{
		for(j=*line; j<(PaperLength-2); j++)
		{
			if ( PrintString(PIO,"\n") )
				return(TRUE);
		}
		*line=0;

		if ( PrintPageFooter(*page) )
			return(TRUE);
		*page = *page + 1;

		if ( PrintFormFeed())
			return(TRUE);
	}
	return(FALSE);
}

/******** PrintPageFooter() ********/

BOOL PrintPageFooter(int page)
{
TEXT str[80];
BOOL aborted=FALSE;

	aborted = PrintCommand(PIO, aSGR0, 0, 0, 0, 0);	/* NORMAL */
	if (aborted)
		return(aborted);

	aborted = PrintCommand(PIO, aSGR1, 0, 0, 0, 0);	/* BOLD ON */
	if (aborted)
		return(aborted);

	sprintf(str, "			-%d-\n", page);
	aborted = PrintString(PIO, str);
	if (aborted)
		return(aborted);

	aborted = PrintCommand(PIO, aSGR22, 0, 0, 0, 0);	/* BOLD OFF */
	if (aborted)
		return(aborted);
}

/******** E O F ********/
