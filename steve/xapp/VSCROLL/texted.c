#include "nb:pre.h"
#include "demo:gen/support_protos.h"
#include "structs.h"

#define NULLCHAR '\0'
#define APEN 1L
#define BPEN 3L
#define CPEN 2L

#define LINELEN 80
#define NUMLINES 100
#define NUMDISP 7

STATIC int XX=0, YY=0, MAXWW=0;

#define RAW_CURSOR_UP			0x4c
#define RAW_CURSOR_DOWN		0x4d
#define RAW_CURSOR_RIGHT	0x4e
#define RAW_CURSOR_LEFT		0x4f

#define WHERE_Y_IS (TER->textData+TER->os+TER->y*LINELEN)

#define JUST_LEFT 	0
#define JUST_CENTER 1
#define JUST_RIGHT	2

BOOL OpenTextEd(struct Window *window);
void CloseTextEd(void);
void DoTextEd(struct Window *window, struct EventData *CED);
BOOL TE_Alloc(struct TextEditRecord *TER, struct Window *textWindow, UWORD textLen);
void TE_Free(struct TextEditRecord *TER);
void TE_InsertChar(struct TextEditRecord *TER, USHORT code);
void TE_CursorToLeft(struct TextEditRecord *TER, USHORT qual);
void TE_CursorToRight(struct TextEditRecord *TER, USHORT qual);
void TE_CursorUp(struct TextEditRecord *TER);
void TE_CursorDown(struct TextEditRecord *TER);
void TE_Return(struct TextEditRecord *TER);
void TE_DeleteChar(struct TextEditRecord *TER);
void TE_BackspaceChar(struct TextEditRecord *TER);
void TE_RenderText(struct TextEditRecord *TER);
void TE_DeleteLine(struct TextEditRecord *TER);
BOOL TE_LoadCrawl(struct CreditRoll *CR_rec);
BOOL TE_SaveCrawl(struct CreditRoll *CR_rec);
void DisplayLoadedText(void);
void VSC_FetchString(STRPTR buffer, STRPTR dest, int max, int *count);
void VSC_FetchInteger(STRPTR buffer, int *dest, int *count);
void GetRest(FILE *fp, STRPTR buf, int max);
void InsertHelpText(void);
BOOL TextIsEmpty(void);
void ClearText(void);

struct TextEditRecord
{
	WORD x,y;
	WORD os;
	UBYTE *textData;
	UWORD textLen;
	struct Window *textWindow;
	struct BitMap bm;
	struct RastPort rp;
};

struct TextEditRecord TER;

extern struct GadgetRecord VSC_GR[];
extern struct MsgPort *capsport;
extern struct RendezVousRecord *rvrec;
extern UBYTE **msgs;

#define NUMCOLORS 16	// was 9

TEXT *colors[] = {	"000","fff","f00","0f0","00f","ff0","0ff","f0f","888",
										"900","090","009","990","099","909","555" };

/******** OpenTextEd() ********/

BOOL OpenTextEd(struct Window *window)
{
	XX = VSC_GR[10].x1+5;
	if ( window->WScreen->ViewPort.Modes & LACE )
		YY = VSC_GR[10].y1+20;
	else
		YY = VSC_GR[10].y1+10;
	MAXWW = VSC_GR[10].x2 - VSC_GR[10].x1;

	if ( !TE_Alloc(&TER, window, LINELEN*NUMLINES) )
		return(FALSE);

	if ( window->WScreen->ViewPort.Modes & LACE )
		SetFont(&TER.rp, rvrec->tiny_largefont);
	else
		SetFont(&TER.rp, rvrec->tiny_smallfont);

	TE_RenderText(&TER);
	
	return(TRUE);
}

/******** CloseTextEd() ********/

void CloseTextEd(void)
{
	TE_Free(&TER);
}

/******** DoTextEd() ********/

void DoTextEd(struct Window *window, struct EventData *CED)
{
	switch( CED->Class )
	{
		case IDCMP_RAWKEY:
			if ( CED->Code==RAW_CURSOR_RIGHT )
				TE_CursorToRight(&TER,CED->Qualifier);
			else if ( CED->Code==RAW_CURSOR_LEFT )
				TE_CursorToLeft(&TER,CED->Qualifier);
			else if ( CED->Code==RAW_CURSOR_UP )
				TE_CursorUp(&TER);
			else if ( CED->Code==RAW_CURSOR_DOWN )
				TE_CursorDown(&TER);
			break;

		case IDCMP_VANILLAKEY:
			if ( CED->Code>=32 && CED->Code<=126 )	// space tru tilde(~)
			{
				TE_InsertChar(&TER,CED->Code);
				TE_RenderText(&TER);
			}
			else if ( CED->Code==0x0d )							// carriage return
			{
				TE_Return(&TER);
			}
			else if ( CED->Code==0x7f )							// delete
			{
				TE_DeleteChar(&TER);
			}
			else if ( CED->Code==0x08 )							// backspace
			{
				TE_BackspaceChar(&TER);
			}
			break;
	}
}

/******** TE_Alloc() ********/

BOOL TE_Alloc(struct TextEditRecord *TER, struct Window *textWindow, UWORD textLen)
{
	TER->x					= 0;
	TER->y					= 0;
	TER->os					= 0;
	TER->textData		= (UBYTE *)AllocMem(textLen,MEMF_ANY|MEMF_CLEAR);
	TER->textLen		= textLen;
	TER->textWindow	= textWindow;
	InitBitMap(&TER->bm,2,textWindow->Width+16,50L);
	TER->bm.Planes[0] = (PLANEPTR)AllocRaster(textWindow->Width+16,50L);
	TER->bm.Planes[1] = (PLANEPTR)AllocRaster(textWindow->Width+16,50L);
	InitRastPort(&TER->rp);
	TER->rp.BitMap = &TER->bm;

	if ( !TER->textData )
		return(FALSE);

	if ( !TER->bm.Planes[0] )
	{
		FreeMem(TER->textData,TER->textLen);
		return(FALSE);
	}

	if ( !TER->bm.Planes[1] )
	{
		FreeMem(TER->textData,TER->textLen);
		FreeRaster(TER->bm.Planes[0],textWindow->Width+16,50L);
		return(FALSE);
	}

	return(TRUE);
}

/******** TE_Free() ********/

void TE_Free(struct TextEditRecord *TER)
{
	FreeMem(TER->textData,TER->textLen);
	FreeRaster(TER->bm.Planes[0],TER->textWindow->Width+16,50L);
	FreeRaster(TER->bm.Planes[1],TER->textWindow->Width+16,50L);
}

/******** TE_InsertChar() ********/

void TE_InsertChar(struct TextEditRecord *TER, USHORT code)
{
WORD i;

	if ( TextLength(&TER->rp, WHERE_Y_IS, strlen(WHERE_Y_IS)) > MAXWW-XX )
	{
		DisplayBeep(NULL);
		return;
	}

	if ( TER->x < LINELEN )
	{
		for(i=LINELEN-2; i>TER->x; i--)
			*(WHERE_Y_IS+i) = *(WHERE_Y_IS+i-1);
		*(WHERE_Y_IS+TER->x) = code;
		TER->x++;
	}
	else
		DisplayBeep(NULL);
}

/******** TE_CursorToLeft() ********/

void TE_CursorToLeft(struct TextEditRecord *TER, USHORT qual)
{
	if ( TER->x==0 )
		DisplayBeep(NULL);
	else
	{
		if ( (qual & IEQUALIFIER_LSHIFT) || (qual & IEQUALIFIER_RSHIFT) )
			TER->x=0;
		else
			TER->x--;
		TE_RenderText(TER);
	}
}

/******** TE_CursorToRight() ********/

void TE_CursorToRight(struct TextEditRecord *TER, USHORT qual)
{
	if ( *(WHERE_Y_IS+TER->x) == NULLCHAR )
		DisplayBeep(NULL);
	else
	{
		if ( (qual & IEQUALIFIER_LSHIFT) || (qual & IEQUALIFIER_RSHIFT) )
			TER->x=strlen(WHERE_Y_IS);
		else
			TER->x++;
		TE_RenderText(TER);
	}
}

/******** TE_CursorUp() ********/

void TE_CursorUp(struct TextEditRecord *TER)
{
int i, len, oldx;

	if ( TER->y==0 && TER->os==0 )
	{
		DisplayBeep(NULL);
		return;
	}

	oldx=TER->x;
	TER->x=-1;
	TE_RenderText(TER);	// get rid of cursor

	if ( TER->y==0 )
	{
		TER->os -= LINELEN;
		for(i=0; i<=NUMDISP; i++)
		{
			TER->y=i;
			TE_RenderText(TER);
		}
		TER->y=0;
	}
	else
		TER->y--;

	len = strlen( WHERE_Y_IS ); 
	if ( oldx > len )
		TER->x = strlen( WHERE_Y_IS ); 
	else
		TER->x = oldx;
	TE_RenderText(TER);
}

/******** TE_CursorDown() ********/

void TE_CursorDown(struct TextEditRecord *TER)
{
int i, len, oldx;

	if ( (TER->os/LINELEN)+TER->y == NUMLINES-1 )
	{
		DisplayBeep(NULL);
		return;
	}

	oldx=TER->x;
	TER->x=-1;
	TE_RenderText(TER);	// get rid of cursor

	if ( TER->y == NUMDISP )
	{
		TER->os += LINELEN;
		for(i=0; i<=NUMDISP; i++)
		{
			TER->y=i;
			TE_RenderText(TER);
		}
		TER->y=NUMDISP;
	}
	else
		TER->y++;

	len = strlen( WHERE_Y_IS ); 
	if ( oldx > len )
		TER->x = strlen( WHERE_Y_IS ); 
	else
		TER->x = oldx;
	TE_RenderText(TER);
}

/******** TE_Return() ********/

void TE_Return(struct TextEditRecord *TER)
{
int i,numLines,line,oldy;

	if ( (TER->os/LINELEN)+TER->y == NUMLINES-1 )
	{
		DisplayBeep(NULL);
		return;
	}

	numLines=NUMLINES-1;
	while( *(TER->textData+numLines*LINELEN) == NULLCHAR )
	{
		if (numLines==0)
			break;
		numLines--;
	}
	numLines++;

	if ( numLines == NUMLINES )
	{
		TER->x = 0;
		TE_CursorDown(TER);
		return;
	}

	if ( TER->x != 0 )	// only insert blank line when cursor is completely left
	{
		TER->x = 0;
		TE_CursorDown(TER);
		return;
	}

	TER->x=-1;
	TE_RenderText(TER);	// get rid of cursor

	line=(TER->os/LINELEN)+TER->y;	// current line
	for(i=NUMLINES-2; i>=line; i--)
		strcpy(TER->textData+(i+1)*LINELEN, TER->textData+i*LINELEN);
	*(TER->textData+line*LINELEN) = NULLCHAR;

	oldy = TER->y;
	for(i=0; i<=NUMDISP; i++)
	{
		TER->y=i;
		TE_RenderText(TER);
	}
	TER->y=oldy;
	TER->x=0;
	TE_RenderText(TER);

	TE_CursorDown(TER);
}

/******** TE_DeleteChar() ********/

void TE_DeleteChar(struct TextEditRecord *TER)
{
WORD i;

	if ( *(WHERE_Y_IS+TER->x) == NULLCHAR )
	{
		if ( TER->x==0 )
			TE_DeleteLine(TER);
		else
			DisplayBeep(NULL);
	}
	else
	{
		i=TER->x+1;
		while( *(WHERE_Y_IS+i) != NULLCHAR )
		{
			*(WHERE_Y_IS+i-1) = *(WHERE_Y_IS+i);
			i++;
		}
		*(WHERE_Y_IS+i-1) = NULLCHAR;
		TE_RenderText(TER);
	}
}

/******** TE_BackspaceChar() ********/

void TE_BackspaceChar(struct TextEditRecord *TER)
{
WORD i,line;

	if ( TER->x==0 )
	{
		line=(TER->os/LINELEN)+TER->y;	// current line
		if ( line>0 && *(TER->textData+(line-1)*LINELEN) == NULLCHAR )
		{
			TE_CursorUp(TER);
			TE_DeleteLine(TER);
		}
		else
			DisplayBeep(NULL);
	}
	else
	{
		i=TER->x;
		while( *(WHERE_Y_IS+i) != NULLCHAR )
		{
			*(WHERE_Y_IS+i-1) = *(WHERE_Y_IS+i);
			i++;
		}
		*(WHERE_Y_IS+i-1) = NULLCHAR;
		TER->x--;
		TE_RenderText(TER);
	}
}

/******** TE_RenderText() ********/

void TE_RenderText(struct TextEditRecord *TER)
{
int i,x,y,len,j,axis,max,x2,y2;
UBYTE *ptr;

	SetBPen(&TER->rp, BPEN);
	SetDrMd(&TER->rp, JAM2);

	//x = XX;
	y = YY + TER->y*TER->rp.TxHeight;
	ptr = WHERE_Y_IS;
	i = 0;
	max = MAXWW-TER->rp.TxWidth;

	SetAPen(&TER->rp, BPEN);
	RectFill(&TER->rp,0,0,TER->textWindow->Width-1,TER->rp.TxHeight+1);
	SetAPen(&TER->rp, APEN);

	x=-1;

	if ( *(ptr+i) == '>' )			// right aligned
	{
		x = max-TextLength(&TER->rp, ptr, strlen(ptr));
		if ( x < XX )
			x = XX;
	}
	else if ( *(ptr+i) == '<' )	// left aligned
	{
		x = XX;
	}
	else	// look for '|'
	{
		axis=-1;
		for(j=0; j<strlen(ptr); j++)
		{
			if ( *(ptr+j)=='|' )
			{
				axis=j;
				break;
			}
		}
		if ( axis!=-1 )
		{
			x = TextLength(&TER->rp, ptr, axis);
			x = (max/2)-x;
			if ( x < XX )
				x = XX;
		}
	}
	if (x==-1)	// center by default
	{
		x = (max-TextLength(&TER->rp, ptr, strlen(ptr)))/2;
		if ( x < XX )
			x = XX;
	}

	while( *(ptr+i) )
	{
		if ( i==TER->x )	// cursor pos
			SetBPen(&TER->rp, CPEN);
		len = TextLength(&TER->rp, ptr+i, 1L);

		Move(&TER->rp, x-XX, TER->rp.TxBaseline);
		Text(&TER->rp, ptr+i, 1L);

		if ( i==TER->x )	// cursor pos
			SetBPen(&TER->rp, BPEN);
		x+=len;
		if ( x>=MAXWW )
			break;
		i++;
	}

	x2 = XX;
	y2 = YY + TER->y*TER->rp.TxHeight;

	if ( i==TER->x )	// cursor is at end of line
	{
		SetBPen(&TER->rp, CPEN);
		Move(&TER->rp, x-XX, TER->rp.TxBaseline);
		Text(&TER->rp, " ", 1L);
		SetBPen(&TER->rp, BPEN);
	}

	ClipBlit(	&TER->rp, 0, 0,
						TER->textWindow->RPort,
						x2,
						y2 - TER->rp.TxBaseline,
						MAXWW-x2,
						TER->rp.TxHeight,
						0xc0 );
}

/******** TE_DeleteLine() ********/

void TE_DeleteLine(struct TextEditRecord *TER)
{
int i,line,oldy;

	TER->x=-1;
	TE_RenderText(TER);	// get rid of cursor

	line=(TER->os/LINELEN)+TER->y;	// current line

	for(i=line; i<(NUMLINES-1); i++)
		strcpy(TER->textData+i*LINELEN, TER->textData+(i+1)*LINELEN);
	*(TER->textData+(NUMLINES-1)*LINELEN) = NULLCHAR;

	oldy = TER->y;
	for(i=0; i<=NUMDISP; i++)
	{
		TER->y=i;
		TE_RenderText(TER);
	}
	TER->y=oldy;
	TER->x=0;
	TE_RenderText(TER);
}

/******** TE_LoadCrawl() ********/

BOOL TE_LoadCrawl(struct CreditRoll *CR_rec)
{
int i,j,k;
FILE *fp;
char ch,ch2,ch3,st;
TEXT buf[100];
BOOL doch, special;

	fp = fopen(CR_rec->file,"r");
	if ( fp )
	{
		for(i=0; i<NUMLINES; i++)
			*(TER.textData+i*LINELEN) = NULLCHAR;

		i=j=0;
		special=FALSE;

		while((ch=fgetc(fp))!=EOF)
		{
			if ( ch==0x0a )
				ch=NULLCHAR;

			if ( special )	// start of code sequence
			{
				special=FALSE;

				// BACKGROUND COLOR
				//fgetc(fp);	// read '^'
				fgetc(fp);	// read 'c'
				fgetc(fp);	// read 'b'
				GetRest(fp, buf, 99);
				k=0;
				while(k<NUMCOLORS)
				{
					if ( !strcmp(buf,colors[k]) )
						break;
					k++;
				}
				if ( k<NUMCOLORS )
					CR_rec->backgroundColor=k;
				else
					CR_rec->backgroundColor=4;
/*
{
char str[100];
sprintf(str,"1 %d\n",CR_rec->backgroundColor);
KPrintF(str);
}
*/
				// TEXT COLOR
				fgetc(fp);	// read '^'
				fgetc(fp);	// read 'c'
				fgetc(fp);	// read 'f'
				GetRest(fp, buf, 99);
				k=0;
				while(k<NUMCOLORS)
				{
					if ( !strcmp(buf,colors[k]) )
						break;
					k++;
				}
				if ( k<NUMCOLORS )
					CR_rec->textColor=k;
				else
					CR_rec->textColor=1;
/*
{
char str[100];
sprintf(str,"2 %d\n",CR_rec->textColor);
KPrintF(str);
}
*/
				// SHADOW COLOR
				fgetc(fp);	// read '^'
				fgetc(fp);	// read 'c'
				fgetc(fp);	// read 's'
				GetRest(fp, buf, 99);
				k=0;
				while(k<NUMCOLORS)
				{
					if ( !strcmp(buf,colors[k]) )
						break;
					k++;
				}
				if ( k<NUMCOLORS )
					CR_rec->shadowColor=k;
				else
					CR_rec->shadowColor=4;
/*
{
char str[100];
sprintf(str,"3 %d\n",CR_rec->shadowColor);
KPrintF(str);
}
*/
				// FONT NAME
				fgetc(fp);	// read '^'
				fgetc(fp);	// read 'f'
				GetRest(fp, buf, 99);
				sscanf(buf,"%s",CR_rec->fontName);
/*
{
char str[100];
sprintf(str,"4 %s\n",CR_rec->fontName);
KPrintF(str);
}
*/
				// FONT SIZE
				fgetc(fp);	// read '^'
				fgetc(fp);	// read 's'
				GetRest(fp, buf, 99);
				sscanf(buf,"%d",&CR_rec->fontSize);
/*
{
char str[100];
sprintf(str,"5 %d\n",CR_rec->fontSize);
KPrintF(str);
}
*/
				// SPEED
				fgetc(fp);	// read '^'
				fgetc(fp);	// read 'v'
				GetRest(fp, buf, 99);
				sscanf(buf,"%d",&CR_rec->speed);
/*
{
char str[100];
sprintf(str,"6 %d\n",CR_rec->speed);
KPrintF(str);
}
*/
				// SHADOW WEIGHT
				fgetc(fp);	// read '^'
				fgetc(fp);	// read 'd'
				GetRest(fp, buf, 99);
				sscanf(buf,"%d",&CR_rec->sweight);
/*
{
char str[100];
sprintf(str,"7 %d\n",CR_rec->sweight);
KPrintF(str);
}
*/
				// LINE SPACING
				fgetc(fp);	// read '^'
				fgetc(fp);	// read 'i'
				GetRest(fp, buf, 99);
				sscanf(buf,"%d",&CR_rec->lspc);

				// SHADOW TYPE
				fgetc(fp);	// read '^'
				fgetc(fp);	// read 't'
				GetRest(fp, buf, 99);
				sscanf(buf,"%c",&st);
				if ( st=='f' )
					CR_rec->stype = 0;
				else
					CR_rec->stype = 1;
/*
{
char str[100];
sprintf(str,"8 %c\n",st);
KPrintF(str);
}
*/

				while(1)
				{
					ch=fgetc(fp);
					if ( ch==EOF || ch==0x0a )
						break;
				}
			}
			else
			{
				ch2=NULLCHAR;
				ch3=NULLCHAR;
				doch=TRUE;

				if ( ch=='^' )
				{
					ch=fgetc(fp);
					if ( ch=='a' )	// ^al^ or ^ar^ or ^am^
					{
						ch=fgetc(fp);
						if ( ch=='l' )
							ch3='<';
						else if ( ch=='r' )
							ch3='>';
						fgetc(fp);	// get trailing ^
						doch=FALSE;
					}
					else if ( ch=='!' )	// ^!^
					{
						special=TRUE;
						fgetc(fp);	// get trailing ^
						doch=FALSE;
					}
					else
					{
						ch2=ch;	// pass char after ^ on as normal text
						ch='^';
					}
				}

				if ( ch3 )
				{
					*(TER.textData+i*LINELEN+j) = ch3;
					j++;
					if ( j>=LINELEN )
					{
						i++;
						j=0;
					}
					if ( i>=NUMLINES-1 )
						break;
				}
				else if (doch)	// do ch
				{
					if ( ch>=0x20 || ch==NULLCHAR )
					{
						*(TER.textData+i*LINELEN+j) = ch;
						j++;
						if ( j>=LINELEN || ch==NULLCHAR )
						{
							i++;
							j=0;
						}
						if ( i>=NUMLINES-1 )
							break;
					}
				}

				if ( ch2 )
				{
					*(TER.textData+i*LINELEN+j) = ch2;
					j++;
					if ( j>=LINELEN )
					{
						i++;
						j=0;
					}
					if ( i>=NUMLINES-1 )
						break;
				}
			}
		}

		*(TER.textData+i*LINELEN+j) = NULLCHAR;

		fclose(fp);
	}						
	else
		return(FALSE);

	return(TRUE);
}

/******** TE_SaveCrawl() ********/

BOOL TE_SaveCrawl(struct CreditRoll *CR_rec)
{
int i,j,numLines,just;
FILE *fp;
BOOL replaced;

	just=JUST_CENTER;

	numLines=NUMLINES-1;
	while( *(TER.textData+numLines*LINELEN) == NULLCHAR )
	{
		if (numLines==0)
			break;
		numLines--;
	}
	numLines++;

	fp = fopen(CR_rec->file,"w");
	if ( fp )
	{
		// Write header

		fprintf(fp,"^!^^cb%s^^cf%s^^cs%s^^f%s^^s%d^^v%d^^d%d^^i%d^",
						colors[CR_rec->backgroundColor],
						colors[CR_rec->textColor],
						colors[CR_rec->shadowColor],
						CR_rec->fontName,CR_rec->fontSize,
						CR_rec->speed,CR_rec->sweight,CR_rec->lspc);
		if ( CR_rec->stype==0 )
			fprintf(fp,"^tf^\n");
		else
			fprintf(fp,"^tn^\n");

		// write text

		for(i=0; i<numLines; i++)
		{
			j=0;
			while( *(TER.textData+i*LINELEN+j) != NULLCHAR )
			{
				replaced=FALSE;
				if ( j==0 )	// start of line
				{
					if ( *(TER.textData+i*LINELEN+j) == '<' )
					{
						replaced=TRUE;
						if ( just!=JUST_LEFT )
						{
							just=JUST_LEFT;
							fprintf(fp,"^al^");
						}
					}
					else if ( *(TER.textData+i*LINELEN+j) == '>' )
					{
						replaced=TRUE;
						if ( just!=JUST_RIGHT )
						{
							just=JUST_RIGHT;
							fprintf(fp,"^ar^");
						}
					}
					else if ( just!=JUST_CENTER )
					{
						just=JUST_CENTER;
						fprintf(fp,"^am^");
					}
				}
				if ( !replaced )
					fputc((char)*(TER.textData+i*LINELEN+j),fp);

				j++;
			}
			fputc('\n',fp);
		}
		fclose(fp);
	}
	else
		return(FALSE);

	return(TRUE);
}

/******** DisplayLoadedText() ********/

void DisplayLoadedText(void)
{
int i;

	TER.x=-1;
	TE_RenderText(&TER);	// get rid of cursor
	TER.os=0;

	for(i=0; i<=NUMDISP; i++)
	{
		TER.y=i;
		TE_RenderText(&TER);
	}

	TER.x=0;
	TER.y=0;
	TE_RenderText(&TER);
}

/******** VSC_FetchString() ********/
/*
 * buffer points to start of start of parameter string
 * e.g. <fGaramond, buffer points to the G
 *
 */

void VSC_FetchString(STRPTR buffer, STRPTR dest, int max, int *count)
{
int i;

	for(i=0; i<max; i++)
	{
		if ( buffer[i]=='^' )
		{
			dest[i]=0;
			return;
		}
		dest[i] = buffer[i];
		*count = *count + 1;
	}
}

/******** VSC_FetchInteger() ********/
/*
 * buffer points to start of start of parameter string
 * e.g. <s64, buffer points to the 6
 *
 */

void VSC_FetchInteger(STRPTR buffer, int *dest, int *count)
{
int i;
TEXT str[10];

	for(i=0; i<10; i++)
	{
		if ( buffer[i]=='^' )
		{
			str[i]=0;
			sscanf(str, "%d", dest);
			return;
		}
		str[i] = buffer[i];
		*count = *count + 1;
	}
}

/******** GetRest() ********/

void GetRest(FILE *fp, STRPTR buf, int max)
{
char ch;
int i;

	i=0;
	while(1)
	{
		ch=fgetc(fp);
		if ( ch==EOF || ch=='^' )
			break;
		else
			buf[i] = ch;
		if ( i<max )
			i++;
		else
			break;			
	}
	buf[i]=NULLCHAR;
}

/******** InsertHelpText() ********/

void InsertHelpText(void)
{
int i;

	for(i=0; i<NUMLINES; i++)
		*(TER.textData+i*LINELEN) = NULLCHAR;

	strcpy(TER.textData+0*LINELEN, msgs[Msg_VSC_Help1-1]);

	strcpy(TER.textData+2*LINELEN, msgs[Msg_VSC_Help2-1]);
	strcpy(TER.textData+3*LINELEN, msgs[Msg_VSC_Help3-1]);
	strcpy(TER.textData+4*LINELEN, msgs[Msg_VSC_Help4-1]);

	strcpy(TER.textData+6*LINELEN, msgs[Msg_VSC_Help5-1]);
	strcpy(TER.textData+7*LINELEN, msgs[Msg_VSC_Help6-1]);
	strcpy(TER.textData+8*LINELEN, msgs[Msg_VSC_Help7-1]);
	strcpy(TER.textData+9*LINELEN, msgs[Msg_VSC_Help8-1]);

	strcpy(TER.textData+11*LINELEN, msgs[Msg_VSC_Help9-1]);
	strcpy(TER.textData+12*LINELEN, msgs[Msg_VSC_Help10-1]);

	strcpy(TER.textData+14*LINELEN, msgs[Msg_VSC_Help11-1]);

	strcpy(TER.textData+16*LINELEN, msgs[Msg_VSC_Help12-1]);
	for(i=0; i<strlen(TER.textData+16*LINELEN); i++)
		if ( *(TER.textData+16*LINELEN+i) == '/' )
			*(TER.textData+16*LINELEN+i) = '|';

	strcpy(TER.textData+17*LINELEN, msgs[Msg_VSC_Help13-1]);
	for(i=0; i<strlen(TER.textData+17*LINELEN); i++)
		if ( *(TER.textData+17*LINELEN+i) == '/' )
			*(TER.textData+17*LINELEN+i) = '|';

	strcpy(TER.textData+18*LINELEN, msgs[Msg_VSC_Help14-1]);
	for(i=0; i<strlen(TER.textData+18*LINELEN); i++)
		if ( *(TER.textData+18*LINELEN+i) == '/' )
			*(TER.textData+18*LINELEN+i) = '|';
}

/******** TextIsEmpty() ********/

BOOL TextIsEmpty(void)
{
int numLines;

	numLines=NUMLINES-1;
	while( *(TER.textData+numLines*LINELEN) == NULLCHAR )
	{
		if (numLines==0)
			return(TRUE);
		numLines--;
	}
	return(FALSE);
}

/******** ClearText() ********/

void ClearText(void)
{
int i;
 	for(i=0; i<NUMLINES; i++)
		*(TER.textData+i*LINELEN) = NULLCHAR;
	DisplayLoadedText();
}

/******** E O F ********/
