#include <exec/exec.h>
#include <exec/memory.h>
#include <exec/types.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <stdio.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <ctype.h>
#include "nb:gui_texts.h"

void MakeLanExt(UBYTE,STRPTR);

/******** main() ********/

void main(int argc, char **argv)
{
FILE *fp;
int lanCode, len, i, j, k, l, val;
char *strPtr, tmp[1024], part[1024];
ULONG *mem1, *mem2, total, offset;
BOOL isComment;
struct FileHandle *FH;
TEXT filename[150], ext[30];
char **msgs[10], *testje[1000];

	if ( (argc==2 && argv[1][0]=='?') || argc!=3)
	{
		printf("File Translator © 1993 by 1001 Software Development.\n");
		printf("\nUsage: \"%s FileName LanguageCode\"\n", argv[0]);
		printf("where LanguageCode is: 1 = English\n");
		printf("                       2 = Nederlands\n");
		printf("                       3 = Deutsch\n");
		printf("                       4 = Français\n");
		printf("                       5 = Español\n");
		printf("                       6 = Italiano\n");
		printf("                       7 = Português\n");
		printf("                       8 = Dansk\n");
		printf("                       9 = Svenska\n");
		printf("                      10 = Norsk\n");
		printf("\nE.g. \"%s alltexts 1\" will create a file \"alltexts.English\"\n\n", argv[0]);
		exit(0);
	}

	sscanf(argv[2],"%d",&lanCode);
	if (lanCode<1 || lanCode>10)
	{
		printf("Illegal LanguageCode\n");
		exit(0);
	}

	fp = fopen(argv[1], "r");
	if (!fp)
	{
		printf("Unable to open file \'%s\'\n", argv[1]);
		exit(0);
	}

	MakeLanExt(lanCode,ext);
	sprintf(filename, "texts%s",ext);

	FH = (struct FileHandle *)Open((STRPTR)filename, (LONG)MODE_NEWFILE);
	if (FH==NULL)
	{
		printf("Unable to write file \'%s\'\n", filename);
		exit(0);
	}

	mem1 = (ULONG *)AllocMem(100000L, MEMF_CLEAR);
	if (mem1==NULL)
	{
		printf("Not enough memory\n");
		Close((BPTR)FH);
		fclose(fp);
		exit(0);
	}
	mem2 = mem1;

	Write((BPTR)FH, mem1, 4L);	// write first 4 bytes, they are filled later with
															// offset to offset table.

	k=0;
	total=0;
	while(1)
	{
		if (feof(fp)!=0)
			break;
		tmp[0]='\0';
		fgets(tmp, 1024, fp);
		if (tmp[0]!='\0')
		{
			strPtr = stpblk(tmp);	// skip leading white spaces
			len = strlen(strPtr);	// length of trimmed string, with trailing cr
			if (len>1)						// this line is more than only a cr
			{
				len -= 1;	// forget trailing CR
				*(strPtr+len) = '\0';
				isComment=FALSE;
				for(i=0; i<len; i++)
				{
					if ( *(strPtr+i)=='*' )
					{
						isComment=TRUE;			 			
						break;
					}
/*
					else if ( *(strPtr+i)=='|' )
						*(strPtr+i)='\0';
*/
				}
				if ( !isComment )
				{
					sscanf(strPtr, "%d", &val);
					if (val==0 || val==lanCode)
					{
						i=0;
						while( *(strPtr+i) != '\"' )
							i++;
						i++;
						j=0;

						for(l=0; l<1024; l++)
							part[l] = '\0';

						while( *(strPtr+i) != '\"' )
						{
							part[j] = *(strPtr+i);
							part[j+1] = '\0';
							i++;
							j++;
						}

						*mem1++ = total+4;
						len = strlen(part)+1;
						len += (len%2);
						total += len;

						for(l=0; l<1024; l++)
							if ( part[l] == '|' )
								part[l] = '\0';

						Write((BPTR)FH, part, len);

						k++;
					}
				}
			}
		}
	}

	total += 4;
	offset = total + (total%4);

	for(i=0; i<8; i++)
		tmp[i]=0;
	Write((BPTR)FH, tmp, total%4);	// pad texts to longword boundary

	/**** write out offsets ****/

	Write((BPTR)FH, mem2, k*4);

	/**** go to start of file ****/

	Seek((BPTR)FH, 0, OFFSET_BEGINNING);

	Write((BPTR)FH, &offset, 4L);

	FreeMem(mem2, 100000L);

	Close((BPTR)FH);

	fclose(fp);

	printf("Written translation to \'%s\'\n", filename);

#if 0
	/**** read back ****/

	mem1 = (ULONG *)AllocMem(20000L, MEMF_CLEAR);
	mem2 = mem1;
	if (mem1!=NULL)
	{
		FH = (struct FileHandle *)Open((STRPTR)filename, (LONG)MODE_OLDFILE);
		if (FH==NULL)
		{
			printf("Unable to read file \'%s\'\n", filename);
			FreeMem(mem2, 20000L);
			exit(0);
		}
		Read((BPTR)FH,mem1,20000L);

		mem2 += (*mem1/4);		// mem2 now points to 0x00000004 (first offset)

		i=0;
		while ( *mem2 != 0 )
		{
			testje[i] = (UBYTE *)mem1+*mem2;
			mem2++;
			printf("%lx %s\n", testje[i], testje[i]);
			i++;
		}

#if 0
		mem2 = mem1 + (*mem1/4);

printf("offset=%lx\n",mem2);

		i=0;
		while( *(mem2+i) != NULL && i<20)
		{
			stccpy(tmp, (mem2+i), 20);
			printf("%lx %s\n", *mem2, tmp);
			i++;
		}
#endif

		Close((BPTR)FH);
		FreeMem(mem1, 20000L);
	}
#endif

	exit(0);
}

/******** MakeLanExt() ********/
/*
 * This code is also used in config.c
 *
 */

void MakeLanExt(UBYTE lanCode, STRPTR lanExt)
{
	switch(lanCode)
	{
		case  1:	strcpy(lanExt, ".English"); break;
		case  2:	strcpy(lanExt, ".Nederlands"); break;
		case  3:	strcpy(lanExt, ".Deutsch"); break;
		case  4:	strcpy(lanExt, ".Français"); break;
		case  5:	strcpy(lanExt, ".Español"); break;
		case  6:	strcpy(lanExt, ".Italiano"); break;
		case  7:	strcpy(lanExt, ".Português"); break;
		case  8:	strcpy(lanExt, ".Dansk"); break;
		case  9:	strcpy(lanExt, ".Svenska"); break;
		case 10:	strcpy(lanExt, ".Norsk"); break;
		default:	strcpy(lanExt, ".English"); break;
	}
}

/******** E O F ********/
