#include "nb:pre.h"
#include "pascal:include/bufread.h"

#include <stdio.h>

extern struct LibBase * __asm __saveds GetGfxBase( VOID )
	{
	return ( (struct LibBase *) GfxBase );
	}

extern struct LibBase * __asm __saveds GetDOSBase( VOID )
	{
	return ( (struct LibBase *) DOSBase );
	}

// #define	BUFFERED_FILES

struct BufFileInfo *BOpen( char *name, LONG accessMode, ULONG bufSize )
	{
	struct BufFileInfo *BFI;

#ifdef BUFFERED_FILES

	if ( BFI = (struct BufFileInfo *) AllocMem(
			(ULONG) sizeof( struct BufFileInfo ) + bufSize, MEMF_PUBLIC ) )
		{
		if ( BFI->FileHandle = Open( name, accessMode ) )
			{
			BFI->BufValid = FALSE;
			BFI->BufStart = 0;
			BFI->CurPos = 0;
			BFI->LastSeek = 0;
			BFI->BufSize = bufSize;
			BFI->Buffer = (UBYTE *)( BFI + sizeof( struct BufFileInfo ) );
			return( BFI );
			}
		else
			{
			FreeMem( BFI, (ULONG) sizeof( struct BufFileInfo ) + bufSize );
			return( NULL );
			}
		}
	return( BFI );

#else

	if ( BFI = (struct BufFileInfo *) AllocMem( (ULONG) sizeof( struct BufFileInfo ),
		MEMF_PUBLIC | MEMF_CLEAR ) )
		{
		if ( BFI->FileHandle = Open( name, accessMode ) )
			{
			return( BFI );
			}
		else
			{
			FreeMem( BFI, (ULONG) sizeof( struct BufFileInfo ) );
			return ( NULL );
			}
		}
	return( BFI );

#endif
	}

LONG BRead( struct BufFileInfo *BFI, BYTE *buffer, LONG length )
	{
#ifdef BUFFERED_FILES

	if ( 0 )	//BFI->BufValid )
		{
		REGISTER LONG remainder, offset;
		LONG returncode, totallength;
		BYTE *Ptr;

		remainder = (BFI->BufStart + BFI->BufSize) - BFI->CurPos;
		offset = BFI->CurPos - BFI->BufStart;

		if ( length <= remainder )
			{
			if ( length == 4 )
				{
				Ptr = &BFI->Buffer[offset];
				*buffer++ = *Ptr++;
				*buffer++ = *Ptr++;
				*buffer++ = *Ptr++;
				*buffer++ = *Ptr++;
				BFI->CurPos += 4;
				return( 4 );
				}
			else
				{
				CopyMem( &BFI->Buffer[offset], buffer, length);
				BFI->CurPos += length;
				return( length );
				}
			}
		else
			{
			totallength = length;

			PrintSer( "BufRead %d Copying %d Start %d Offset %d \n", length, remainder,
				BFI->BufStart, BFI->CurPos );

			if ( BFI->CurPos != BFI->LastSeek )
				{
				if ( Seek( BFI->FileHandle, BFI->CurPos - BFI->LastSeek, OFFSET_CURRENT ) == -1 )
					return ( -1 );
				}

			CopyMem( &BFI->Buffer[offset], buffer, remainder);
			length -= remainder;
			buffer = &(buffer[remainder]);

			returncode = Read( BFI->FileHandle, buffer, length );

			BFI->BufValid = FALSE;
			BFI->CurPos += totallength;
			BFI->LastSeek = BFI->CurPos;
			return( returncode );
			}
		}
	else // buffer invalid
		{
		LONG returncode, totallength;

		totallength = length;

		PrintSer( "BRead %d bytes ... ", length );

		if ( BFI->CurPos != BFI->LastSeek )
			{
			if ( Seek( BFI->FileHandle, BFI->CurPos - BFI->LastSeek, OFFSET_CURRENT ) == -1 )
				return ( -1 );
			BFI->LastSeek = BFI->CurPos;
			}

		if ( length < BFI->BufSize )
			{
			returncode = Read( BFI->FileHandle, BFI->Buffer, BFI->BufSize );
			if ( returncode == -1 )
				{
				PrintSer( "\n" );
				return( -1 );
				}
			BFI->BufValid = TRUE;
			BFI->BufStart = BFI->CurPos;
			BFI->CurPos += length;
			BFI->LastSeek = Seek( BFI->FileHandle, 0, OFFSET_CURRENT );
			PrintSer( "%d bytes read\n", returncode );
			CopyMem( BFI->Buffer, buffer, length );
			return( (returncode < length) ? -1 : length );
			}
		else
			{
			returncode = Read( BFI->FileHandle, buffer, length );
			BFI->CurPos += length;
			BFI->LastSeek = Seek( BFI->FileHandle, 0, OFFSET_CURRENT );
			return( returncode );
			}
		}

#else

	return ( Read( BFI->FileHandle, buffer, length ) );

#endif

	}

LONG BSeek( struct BufFileInfo *BFI, LONG position, LONG mode )
	{

#ifdef BUFFERED_FILES

	LONG returncode;

	returncode = BFI->LastSeek;
	switch( mode )
		{
		case OFFSET_CURRENT:
			BFI->CurPos += position;
			break;
		case OFFSET_BEGINNING:
			BFI->CurPos = position;
			break;
		case OFFSET_END:
			returncode = Seek( BFI->FileHandle, position, OFFSET_END );
			if ( returncode == -1 )
				return( -1 );
			BFI->CurPos = Seek( BFI->FileHandle, 0, OFFSET_CURRENT );
			BFI->LastSeek = BFI->CurPos;
			break;
		}
	if ( (BFI->CurPos < BFI->BufStart) || (BFI->CurPos >= (BFI->BufStart+BFI->BufSize)) )
		BFI->BufValid = FALSE;
	return( returncode );

#else

	return( Seek( BFI->FileHandle, position, mode ) );

#endif
	}

VOID BClose( struct BufFileInfo *BFI )
	{
#ifdef BUFFERED_FILES

	if ( BFI )
		{
		Close( BFI->FileHandle );
		FreeMem( BFI, (ULONG) sizeof( struct BufFileInfo ) + BFI->BufSize );
		}

#else

	if ( BFI )
		{
		Close( BFI->FileHandle );
		FreeMem( BFI, (ULONG) sizeof( struct BufFileInfo ) );
		}

#endif
	}
