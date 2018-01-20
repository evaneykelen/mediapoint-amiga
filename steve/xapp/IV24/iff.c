#include <string.h>
#include <stdlib.h>
#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <dos.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>
#include <fye/fye.h>
#include <clib/fye_protos.h>
#include <pragmas/fye.h>
#include <fye/fyebase.h>
#include "iff.h"
#include "fyeview_protos.h"
#include "pascal:include/fiff.h"

/**** defines ****/

#define	ILBMBUFFER_SIZE	1000
#define UGetByte()	(*source++)
#define	UPutByte(c)	(*dest++=(c))

ULONG FyeReadIffPictHeader(ULONG file, struct PictHeader **phptr, struct Library *DOSBase)
{
int	length, offset, lenread;
unsigned int *pointer;
char buffer[BUFFERLEN];
BOOL bmhdflag = FALSE, ilbmmode;
struct PictHeader	*ph;

	*phptr= NULL;

	ph=(struct PictHeader *) AllocMem( sizeof (struct PictHeader), MEMF_CLEAR);
	if(!ph)	return (FYE_ERRINSUFFICIENTMEMORY);

	ph->file=file;

	if (HEADERLEN!=(lenread=Read (file, buffer, HEADERLEN)))	/* Read the file header */
		return (riph_cleanup (FYE_ERRREADINGFILE, ph));

	pointer = (unsigned int *) buffer;			/* Pointer to the start of buffer */

	if ( *(pointer++) != FORM)					/* Is it a IFF file ? */
		return (riph_cleanup (FYE_ERRDATAISBAD, ph));	/* no, error */

	length = *(pointer++) -12;			/* length of Data not read */

	switch (*(pointer++))				/* read the type */
	{
		case ILBM: ilbmmode = TRUE ; break;
		case ACBM: ilbmmode = FALSE; break;
		default: return (riph_cleanup (FYE_ERRNOTFYEORILBMFORMAT, ph));
	}

	offset = lenread;				/* Start of Data of next chunk */

	while (lenread && (length > NULL))
	{
		switch (*(pointer++))
		{
			case BMHD:
			if ((*pointer)!=IFF_BMHD)
				return (riph_cleanup(FYE_ERRBMHDBADLEN, ph));

			if (IFF_BMHD>(lenread=Read (file, buffer,IFF_BMHD+CHUNKHEADER)))
				return (riph_cleanup (FYE_ERRREADINGFILE, ph));

			pointer = (unsigned int *) buffer;

			ph->bmhd	 = *((FyeBitMapHeader *) pointer);
			pointer		+= IFF_BMHD>>2;		/* increase of 20 bytes (5 words) */
			length		-= lenread;		/* size of bmhd */
			offset		+= lenread;
	 		bmhdflag	 = TRUE;
			lenread		-= IFF_BMHD;
			break;

			case ABIT:
			if (!(ph->bodyoffset) && !ilbmmode)
			{
				ph->bodyoffset	= offset; 	/* offset of the bitmap */
				ph->bodysize	= *pointer;
			}

			case BODY:
			if (!(ph->bodyoffset) && ilbmmode)
			{
				ph->bodyoffset	= offset; 	/* offset of the bitmap */
				ph->bodysize	= *pointer;
			}

			default :
			lenread = (((*pointer)+1)>>1)<<1;
			if ((length-=lenread) > NULL)
			{
				if (-1==Seek (file,lenread, OFFSET_CURRENT))
					return (riph_cleanup (FYE_ERRREADINGFILE, ph));
				length -= CHUNKHEADER;
				offset += lenread+CHUNKHEADER;
				if (CHUNKHEADER != (lenread = Read (file, buffer, CHUNKHEADER)))
					return (riph_cleanup (FYE_ERRREADINGFILE, ph));
				pointer = (unsigned int *) buffer;
			}
		}
	}

	if (!ph->bodyoffset) return (riph_cleanup (FYE_ERRNOBODYINFILE,ph));

	if (!bmhdflag) return (riph_cleanup (FYE_ERRNOBMHDINFILE,ph));

	ph->extra=ilbmmode;
	ph->filebuffersize=FILEBUFFERLENGTH;
	ph->bitplanesize= (((ph->bmhd.w+15)>>4)<<1)*ph->bmhd.h;

	*phptr	= ph;
	return (NULL);
}

ULONG riph_cleanup(ULONG error, struct PictHeader *ph)
{
	if (ph)
	{
		if (ph->filebuffer)
			FreeMem (ph->filebuffer,ph->filebuffersize);
		FreeMem (ph, sizeof(struct PictHeader));
	}
	return (error);
}

char UnpackRLL(BYTE **pSource,BYTE **pDest,int *srcBytes0,int dstBytes0)
{
int srcBytes,dstBytes;
BYTE *source, *dest, c;
WORD n,minus128=-128;

   source=*pSource;
   dest=*pDest;
   srcBytes=*srcBytes0;
   dstBytes=dstBytes0;

   while(dstBytes>0)
    {
     if((srcBytes-=1)<0) return(1);
     n=UGetByte();

     if (n >= 0)
      {
       n += 1;
       if((srcBytes-=n)<0) return(2);
       if((dstBytes-=n)<0) return(3);
       do
        {
         UPutByte(UGetByte());
        }
       while(--n>0);
      }
     else if(n!=minus128)
      {
       n = -n + 1;
       if((srcBytes-=1)<0) return(4);
       if((dstBytes-=n)<0) return(5);
       c = UGetByte();
       do
        {
         UPutByte(c);
        }
       while(--n>0);
      }
    }

	*srcBytes0=srcBytes;
   *pSource=source;  *pDest=dest;
   return(0);
  }

char SkipRLL(BYTE **pSource,int *srcBytes0,int dstBytes0)
{
int srcBytes,dstBytes;
BYTE *source;
WORD n,minus128=-128;

   source=*pSource;
   srcBytes=*srcBytes0;
   dstBytes=dstBytes0;

   while(dstBytes>0)
    {
     if((srcBytes-=1)<0) return(1);
     n=UGetByte();

     if (n >= 0)
      {
       n += 1;
       if((srcBytes-=n)<0) return(2);
       if((dstBytes-=n)<0) return(3);
       source+=n;
      }
     else if(n!=minus128)
      {
       n = -n + 1;
       if((srcBytes-=1)<0) return(4);
       if((dstBytes-=n)<0) return(5);
       source++;
      }
    }
	*srcBytes0=srcBytes;
   *pSource=source;
   return(0);
}

void FyeCleanupReadIff (struct PictHeader * ph)
{
	(void) riph_cleanup (0,ph);
}

ULONG FyeReadIffPictBody ( 	struct PictHeader * ph,
														PLANEPTR * bitmap,
														ULONG bitplanestart,
														ULONG nbrofbitplanes,
														ULONG mode, struct Library *DOSBase)
{
int	i,line, plane, width, size;
BYTE *source, *dest;

	if ( (ph->extra & IFFILBM) && !(ph->filebuffer) )
	{
		ph->filebuffersize=ph->bodysize;
		ph->filebuffer= (UBYTE *) AllocMem (ph->bodysize, MEMF_ANY);
		if (!(ph->filebuffer))
			return(FYE_ERRINSUFFICIENTMEMORY);
		ph->bufferptr=NULL;
		ph->offsetstartbuffer=ph->bodyoffset;
		if (-1==Seek (ph->file,ph->bodyoffset, OFFSET_BEGINNING))
			return (FYE_ERRREADINGFILE);
		if (ph->bodysize!=Read (ph->file, ph->filebuffer, ph->bodysize))
			return (FYE_ERRREADINGFILE);
	}

	if (ph->extra & IFFILBM)
	{
		/* Iff ILBM decoding */

		width=((ph->bmhd.w+15) & 0xFFF0) >> 3;
		plane=bitplanestart;
		source=ph->filebuffer;
		size=ph->bodysize;

		if (ph->bmhd.compression==cmpNone)
			for (line=0;line<ph->bmhd.h;line++)
			{
				source+=width*plane; size-=width*plane;
				if (size<0)	return (FYE_ERRUNPACKINGIFFFILE);

				for (plane=bitplanestart; plane<bitplanestart+nbrofbitplanes; plane++)
				{
					memcpy(bitmap[plane]+line*width, source, width);
					source+=width; size-=width;
					if (size<0)	return (FYE_ERRUNPACKINGIFFFILE);
				}
				plane= ph->bmhd.nplanes-nbrofbitplanes;
			}
		else
			for (line=0;line<ph->bmhd.h;line++)
			{
				while (plane--)
					if (SkipRLL(&source, &size, width))
						return (FYE_ERRUNPACKINGIFFFILE);

				for (plane=bitplanestart; plane<bitplanestart+nbrofbitplanes; plane++)
				{
					dest=bitmap[plane]+line*width;
					if(UnpackRLL (&source, &dest, &size, width))
						return (FYE_ERRUNPACKINGIFFFILE);
				}
				plane= ph->bmhd.nplanes-nbrofbitplanes;
			}
	}

	else
	{
		/* ACBM decoding */
		/* check if the mode value is correct */
		if( ph->bitplanesize * (bitplanestart + nbrofbitplanes) > ph->bodysize)
			return (FYE_ERRINVALIDARGUMENT);

		if (-1==Seek (ph->file,ph->bodyoffset + ph->bitplanesize*bitplanestart, OFFSET_BEGINNING))
			return (FYE_ERRREADINGFILE);

		if (mode | CONTINOUS_BITPLANES)
		{
			if (nbrofbitplanes * ph->bitplanesize!=Read (ph->file, bitmap[bitplanestart],
				 nbrofbitplanes * ph->bitplanesize))
				return (FYE_ERRREADINGFILE);
		}
		else
		{
			for (i=0; i<nbrofbitplanes;i++)
				if (ph->bitplanesize!=Read (ph->file, bitmap[i+bitplanestart], ph->bitplanesize))
					return (FYE_ERRREADINGFILE);
		}
	}
	return (NULL);
}

