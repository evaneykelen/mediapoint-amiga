//	File		:	midi_play.h
//	Uses		:
//	Date		:	10-11-93
//	Author	:	ing. C. Lieshout
//	Desc.		:	structures and defines for smfinv
//

#define MakeID(a,b,c,d)  ( (LONG)(a)<<24L | (LONG)(b)<<16L | (c)<<8 | (d) )
#define MThd MakeID('M','T','h','d')
#define MTrk MakeID('M','T','r','k')

#define MAXTRAX 48L
#define TWOxMAXTRAX 96L
#define MIDIBUFSIZE 22512L

struct SMFHeader
{
	LONG     ChunkID;  /* 4 ASCII characters */
	LONG     VarLeng;
	WORD     Format;
	UWORD    Ntrks;
	WORD     Division;
};

struct DecTrack
{
	ULONG absdelta;   /* 32-bit delta */
	ULONG nexclock;   /* storage */
	UBYTE status;     /* status from file */
	UBYTE rstatus;    /* running status from track */
	UBYTE d1;         /* data byte 1 */
	UBYTE d2;         /* data byte 2 */
	ULONG absmlength; /* 32-bit absolute metalength */
	UBYTE *endmarker;
	UBYTE metatype;   /* meta event type */
	BOOL playable;
	UBYTE pad3;
};

typedef struct
{
	WORD	release;
	LONG	mainsignal;
	LONG	sig_xtox;
	LONG	sig_ptoc;
	struct MsgPort  *mport_ptoc;
	WORD	action;
	LONG 	restoresignal;
	BPTR	smfhandle;
	UBYTE	*smfdata;
	long	smfdatasize;
	UWORD	Ntrks;
	ULONG	Division;
	ULONG tfactor;
	ULONG tempo;
	ULONG	donecount;
	ULONG	sizeDTrack;
	LONG deltatime;
	UBYTE *pData;
	UBYTE	*pfillbuf[2];
	UBYTE	lastRSchan;
	UBYTE *fillbuf1;
	UBYTE *fillbuf2;

	UBYTE	serdev,timdev;

	struct MsgPort 	*ser_reply_port;
	struct IOExtSer	*ser_request;
	struct MsgPort		*timer_reply_port;
	struct timerequest *timer_request;

}SMFInfo;

/*-------------------*/
/*     Prototypes    */
/*-------------------*/

ULONG ComVarLen (UBYTE *value);
UBYTE *DecodeEvent(UBYTE *ptdata,struct DecTrack *pDTdata, ULONG deswitch, SMFInfo *smfi);
LONG transfer( struct DecTrack *pDT,ULONG mswitch,LONG ylen, SMFInfo *smfi );
ULONG changetempo( ULONG ctbpm, SMFInfo *smfi );
void init_smfi( SMFInfo *smfi );
void release( SMFInfo *smfi, int err );
int play_midi( SMFInfo *smfi );
int read_and_evaluate( char *smfname, SMFInfo *smfi );
int open_req_ports( SMFInfo *smfi );
void set_baud( SMFInfo *smfi );
void set_timer( SMFInfo *smfi, ULONG time );
