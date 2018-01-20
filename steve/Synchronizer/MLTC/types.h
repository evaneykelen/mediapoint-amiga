typedef struct
{
  SYNCDATA 	*td_SyncData;
  UBYTE		td_Buffer[32];
  int		td_BufIndex,		// keeps track of next byte to be send 
 			td_BufLength;		// nr of bytes to be send
  UBYTE		td_TimeCodeType;	// bit code of time code type, u = unused
								// u00uuuuu	  -> 24 fps (not supported)
								// u01uuuuu	  -> 25 fps
								// u10uuuuu	  -> 30 fps drop (not supported)
								// u11uuuuu	  -> 30 fps non drop
} TBEDATA;

#define DE_NONE 		0
#define DE_QUARTERFRAME 1
#define DE_FULLFRAME	2

typedef struct
{
  SYNCDATA 	*rd_SyncData;
  UBYTE		rd_Buffer[32];
  int		rd_BufLength,		// nr of quarter frames received
			rd_BufIndex;		// current position in receive buffer
  int		rd_DataExpected;	// indicates what kind of data is expected next time
  UBYTE		rd_TimeCodeType;	// bit code of time code type, u = unused
								// u00uuuuu	  -> 24 fps (not supported)
								// u01uuuuu	  -> 25 fps
								// u10uuuuu	  -> 30 fps drop (not supported)
								// u11uuuuu	  -> 30 fps non drop
} RBFDATA;
