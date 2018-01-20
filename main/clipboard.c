#include "nb:pre.h"

#define ID_FORM MAKE_ID('F','O','R','M')
#define ID_FTXT MAKE_ID('F','T','X','T')
#define ID_CHRS MAKE_ID('C','H','R','S')
#define ID_ILBM MAKE_ID('I','L','B','M')
#define ID_ANIM MAKE_ID('A','N','I','M')

/**** externals ****/

extern struct IOClipReq *clipboard;
extern struct Library *medialinkLibBase;

/**** functions ****/

/******** CBOpen() ********/

struct IOClipReq *CBOpen(ULONG unit)
{
struct MsgPort *mp;
struct IOStdReq *ior;

	if ( mp=CreatePort(0L,0L) )
	{
		if ( ior=(struct IOStdReq *)CreateExtIO(mp, sizeof(struct IOClipReq)) )
		{
			if ( !(OpenDevice("clipboard.device",unit,(struct IORequest *)ior,0L)))
				return( (struct IOClipReq *)ior );
			DeleteExtIO((struct IORequest *)ior);
		}
		DeletePort(mp);
	}
	return(NULL);			
}

/******** CBClose() ********/

void CBClose(struct IOClipReq *ior)
{
struct MsgPort *mp;

	mp = ior->io_Message.mn_ReplyPort;
	CloseDevice((struct IORequest *)ior);
	DeleteExtIO((struct IORequest *)ior);
	DeletePort(mp);
}

/******** CBWriteFTXT() ********/

int CBWriteFTXT(struct IOClipReq *ior, char *string)
{
ULONG length, slen;
BOOL odd;

	slen = strlen(string);
	odd = (slen & 1);	// pad byte flag

	length = (odd) ? slen+1 : slen;

	ior->io_Offset = 0;
	ior->io_Error = 0;
	ior->io_ClipID = 0;

	WriteLong(ior, (long *)"FORM");
	length += 12L;

	WriteLong(ior, &length);
	WriteLong(ior, (long *)"FTXT");
	WriteLong(ior, (long *)"CHRS");
	WriteLong(ior, &slen);	

	ior->io_Data = (STRPTR)string;
	ior->io_Length = slen;
	ior->io_Command = CMD_WRITE;
	DoIO((struct IORequest *)ior);

	if (odd)
	{
		ior->io_Data = (STRPTR)"";
		ior->io_Length = 1L;
		ior->io_Command = CMD_WRITE;
		DoIO((struct IORequest *)ior);
	}
 	
	ior->io_Command = CMD_UPDATE;
	DoIO((struct IORequest *)ior);

	return( ior->io_Error ? FALSE : TRUE );
}

/******** WriteLong() ********/

BOOL WriteLong(struct IOClipReq *ior, LONG *ldata)
{
	ior->io_Data = (STRPTR)ldata;
	ior->io_Length = 4L;
	ior->io_Command = CMD_WRITE;
	DoIO((struct IORequest *)ior);

	if ( ior->io_Actual==4 )	
		return( (BOOL)(ior->io_Error ? FALSE : TRUE) );

	return( FALSE );
}

/******** CBQueryClipboard() ********/
/*
 * returns: 0 --> unknown type
 *    			1 --> FTXT
 *					2 --> ILBM
 *					3 --> ANIM (does anybody do this?)
 *
 */

int CBQueryClipboard(struct IOClipReq *ior)
{
ULONG cbuff[4];

	ior->io_Offset = 0;
	ior->io_Error = 0;
	ior->io_ClipID = 0;

	ior->io_Command = CMD_READ;
	ior->io_Data = (STRPTR)cbuff;
	ior->io_Length = 12;

	DoIO((struct IORequest *)ior);

	if ( ior->io_Actual == 12L )
	{
		if ( cbuff[0] == ID_FORM )
		{
			if ( cbuff[2] == ID_FTXT )
				return(1);
			if ( cbuff[2] == ID_ILBM )
				return(2);
			if ( cbuff[2] == ID_ANIM )
				return(3);
		}
	}

	CBReadDone(ior);

	return(0);
}

/******** CBReadCHRS() ********/

struct cbbuf *CBReadCHRS(struct IOClipReq *ior)
{
ULONG chunk, size;
struct cbbuf *buf;
int looking;

	looking = TRUE;
	buf = NULL;

	while(looking)
	{
		looking = FALSE;

		if ( ReadLong(ior,&chunk) )
		{
			if ( chunk==ID_CHRS )
			{
				if ( ReadLong(ior,&size) )
				{
					if (size)
						buf = FillCBData(ior,size);
				}
			}
			else
			{
				if ( ReadLong(ior,&size) )
				{
					looking = TRUE;
					if (size & 1)
						size++;

					ior->io_Offset += size;
				}
			}
		}
	}

	if (buf==NULL)
		CBReadDone(ior);

	return(buf);
}

/******** ReadLong() ********/

BOOL ReadLong(struct IOClipReq *ior, ULONG *ldata)
{
	ior->io_Command = CMD_READ;
	ior->io_Data = (STRPTR)ldata;
	ior->io_Length = 4L;

	DoIO( (struct IORequest *)ior );

	if ( ior->io_Actual==4 )	
		return( (BOOL)(ior->io_Error ? FALSE : TRUE) );

	return(FALSE);
}		

/******** FillCBData() ********/

struct cbbuf *FillCBData(struct IOClipReq *ior, ULONG size)
{
register UBYTE *to, *from;
register ULONG x,count;
ULONG length;
struct cbbuf *buf, *success;

	success = NULL;

	if ( buf = AllocMem(sizeof(struct cbbuf), MEMF_PUBLIC))
	{
		length = size;
		if ( size & 1 )
			length++;

		if ( buf->mem = AllocMem(length+1L, MEMF_PUBLIC) )
		{
			buf->size = length+1;

			ior->io_Command = CMD_READ;
			ior->io_Data = (STRPTR)buf->mem;
			ior->io_Length = length;

			to = buf->mem;
			count = 0L;

			if ( !(DoIO((struct IORequest *)ior)) )
			{
				if ( ior->io_Actual == length )
				{
					success = buf;

					for(x=0,from=buf->mem; x<size; x++)
					{
						if ( *from )
						{
							*to = *from;
							to++;
							count++;
						}
						from++;
					}
					*to = 0x0;
					buf->count = count;
				}
			}

			if ( !(success) )
				FreeMem(buf->mem, buf->size);
		}

		if ( !(success) )
			FreeMem(buf, sizeof(struct cbbuf));
	}

	return(success);
}

/******** CBReadDone() ********/

void CBReadDone(struct IOClipReq *ior)
{
char buffer[256];

	ior->io_Command = CMD_READ;
	ior->io_Data = (STRPTR)buffer;
	ior->io_Length = 254;

	while( ior->io_Actual )
		if ( DoIO((struct IORequest *)ior ) )
			break;
}

/******** CBFreeBuf() ********/

void CBFreeBuf(struct cbbuf *buf)
{
	FreeMem(buf->mem, buf->size);
	FreeMem(buf, sizeof(struct cbbuf));
}

/******** CBCutCopy() ********/

void CBCutCopy(void)
{
#if 0
	if ( !CBWriteFTXT(clipboard,"A message from MediaPoint") )
		Message("CBWriteFTXT failed");
#endif
}

/******** CBPaste() ********/

void CBPaste(void)
{
#if 0
struct cbbuf *buf;
int type, size;
UBYTE *mem;

	type = CBQueryClipboard( clipboard );
	if ( type==1 )			// FTXT
	{
		mem = (UBYTE *)AllocMem(2000L, MEMF_ANY | MEMF_CLEAR);
		if ( !mem )
			UA_WarnUser(-1);
		else
		{
			size=0;
			while( buf=CBReadCHRS(clipboard) )
			{
				size += strlen(buf->mem);
				if ( size>0 && size<2000 )
					strncat(mem, buf->mem, 1995);
				CBFreeBuf(buf);
			}
			CBReadDone(clipboard);
			printf("[%s]\n",mem);
			FreeMem(mem,2000L);
		}
	}
	else if ( type==2 )	// ILBM
	{
	}	
	else if ( type==3 )	// ANIM
	{
	}	
#endif
}

/******** E O F ********/
