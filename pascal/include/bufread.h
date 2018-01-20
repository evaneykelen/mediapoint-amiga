#ifndef MEDIAPOINT_BUFREAD_H
#define MEDIAPOINT_BUFREAD_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef DOS_DOS_H
#include <dos/dos.h>
#endif

struct BufFileInfo {
	ULONG	BufStart;
	UBYTE	BufValid;
	UBYTE	Pad[3];
	BPTR	FileHandle;
	ULONG	CurPos;
	ULONG	LastSeek;
	UBYTE *Buffer;
	ULONG	BufSize;
	};

GLOBAL struct BufFileInfo *BOpen( char *name, LONG accessMode, ULONG bufSize );
GLOBAL LONG BRead( struct BufFileInfo *BFI, BYTE *buffer, LONG length );
GLOBAL LONG BSeek( struct BufFileInfo *BFI, LONG position, LONG mode );
GLOBAL VOID BClose( struct BufFileInfo *BFI );

#endif /* MEDIAPOINT_BUFREAD_H */
