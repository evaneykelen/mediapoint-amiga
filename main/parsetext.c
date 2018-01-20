#include "nb:pre.h"

/**** defines ****/

#define RAWBUFSIZE 2000	// see also ptread.c and dbase.c

/**** externals ****/

extern struct TextFont *largeFont;
extern struct TextFont *textFont;
extern struct Library *medialinkLibBase;

/**** functions ****/

/******** ParseText() ********/
/*
 * if path is NULL then filename points to a buffer containing the text
 * else the path and filename are used to load a file from disk.
 *
 */

BOOL ParseText(	STRPTR path, STRPTR filename, struct EditWindow *ew,
								struct EditSupport *es, int *num )
{
TEXT str[SIZE_FULLPATH], fontname[50];
BPTR file;
LONG actualLen;
UBYTE *buffer;
int i,n,numChars,fontsize,add;
struct TextFont *charFont;
UBYTE	charStyle;
UBYTE	charColor;
UBYTE underlineColor;
BOOL DBASE=FALSE, not_canceled=TRUE;

	*num=0;
	
	/**** set default values ****/

	if ( path==NULL )	// called from ptread, parse string
	{
		charFont = largeFont;
		charStyle = 0;
		charColor = 2;
		underlineColor = 2;
	}
	else
	{
		charFont	= ew->TEI->newText.charFont;	
		charColor = ew->TEI->newText.charColor;	
		charStyle = ew->TEI->newText.charStyle;	
		underlineColor = ew->TEI->newText.underlineColor;	
	}

	/**** allocate memory for raw buffer ****/

	if (path==NULL)
	{
		buffer = (UBYTE *)filename;	// filename points already to a buffer
		actualLen = strlen(buffer);
	}
	else
	{
		buffer = (UBYTE *)AllocMem(RAWBUFSIZE, MEMF_ANY);
		if (buffer==NULL)
			return(FALSE);

		/**** read the file from disk ****/

		UA_MakeFullPath(path, filename, str);

		file = Open(str, MODE_OLDFILE);
		if (file==NULL)
		{
			FreeMem(buffer, RAWBUFSIZE);
			return(FALSE);
		}

		actualLen = Read(file, buffer, RAWBUFSIZE);
		if (actualLen<2)	// -1=error, 0=EOF, 1 and 2 are not enough characters
		{
			FreeMem(buffer, RAWBUFSIZE);
			Close(file);
			return(FALSE);
		}

		Close(file);
	}

	/**** parse the raw buffer and create the text buffer ****/

	if ( path && buffer[0] == 0x03 || buffer[0] == 0x83 )	// dBase header (big header huh!)
	{
		not_canceled = ImportDBaseFile(str,buffer);
		if ( not_canceled )
		{
			DBASE=TRUE;
			actualLen = strlen(buffer);
		}
	}

	if ( not_canceled )	// 'not_canceled' may be set to FALSE by ImportDBaseFile
	{
		numChars=0;	// number of characters actually stored in text buffer

		for(i=0; i<actualLen; i++)
		{
			if ( !DBASE && buffer[i]=='^' )						// start of command sequence
			{
				i++;
				if ( buffer[i]=='f' )					// font name follows
				{
					i++;
					FetchString(&buffer[i], fontname, 50, &i);
					strcat(fontname, ".font");
				}
				else if ( buffer[i]=='s' )		// size follows
				{
					i++;
					FetchInteger(&buffer[i], &fontsize, &i);
					if ( OpenTypeFace(fontname, fontsize, 0, TRUE) )
						charFont = textFont;
					else
						charFont = largeFont;	// font not available, go for second best.
				}
				else if ( buffer[i]=='c' )		// color follows
				{
					i++;
					FetchInteger(&buffer[i], &n, &i);
					charColor = n;
				}
				else if ( buffer[i]=='u' )		// underline color follows
				{
					i++;
					FetchInteger(&buffer[i], &n, &i);
					underlineColor = n;
				}
				else if ( buffer[i]=='l' )		// f from lf follows
				{
					ew->TEI->text[numChars].charFont	= charFont;
					ew->TEI->text[numChars].charStyle	= charStyle;
					ew->TEI->text[numChars].charColor	= charColor;
					ew->TEI->text[numChars].underlineColor	= underlineColor;
					ew->TEI->text[numChars].charCode	= 0x0a;
					numChars++;
					i=i+2;
				}
				else if ( buffer[i]=='a' )		// attribute follows
				{
					i++;
					FetchInteger(&buffer[i], &n, &i);
					charStyle = n;
				}
				else if ( buffer[i]==0x5c )		// backslash, linefeed
				{
					i++;	// LF
				}
			}
			else
			{
				if (	(buffer[i]>=0x20 && buffer[i]!=127) ||
							buffer[i]==0x0d || buffer[i]==0x08 || buffer[i]==0x0c || buffer[i]==0x0a )
				{
					add=0;
					if( buffer[i]==0x0d )
						buffer[i]=0x0a;
					else if ( buffer[i]=='\\' && buffer[i+1]=='\"' )
					{
						buffer[i]='\"';
						add=1;
					}
					ew->TEI->text[numChars].charFont	= charFont;
					ew->TEI->text[numChars].charStyle	= charStyle;
					ew->TEI->text[numChars].charColor	= charColor;
					ew->TEI->text[numChars].underlineColor = underlineColor;
					ew->TEI->text[numChars].charCode	= buffer[i];
					numChars++;
					i=i+add;
				}
			}
		}

		ew->TEI->text[numChars].charCode = 0;
	}

	if (path!=NULL)
		FreeMem(buffer, RAWBUFSIZE);

	*num = numChars;

	return( not_canceled );
}

/******** FetchString() ********/
/*
 * buffer points to start of start of parameter string
 * e.g. <fGaramond, buffer points to the G
 *
 */

void FetchString(STRPTR buffer, STRPTR dest, int max, int *count)
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

/******** FetchInteger() ********/
/*
 * buffer points to start of start of parameter string
 * e.g. <s64, buffer points to the 6
 *
 */

void FetchInteger(STRPTR buffer, int *dest, int *count)
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

/******** E O F ********/
