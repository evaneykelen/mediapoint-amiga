//	File		:	camd.h
//	Uses		:
//	Date		:	27-04-93
//	Author	:	Dan Baker, Commodore Business Machines
//					( adapted for MP ing. C. Lieshout )
//	Desc.		:	structures and defines for smfnew
//

#define MakeID(a,b,c,d)  ( (LONG)(a)<<24L | (LONG)(b)<<16L | (c)<<8 | (d) )
#define MThd MakeID('M','T','h','d')
#define MTrk MakeID('M','T','r','k')

#define MAXTRAX 24L
#define TWOxMAXTRAX 32L
#define MIDIBUFSIZE 512L

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
	ULONG	donecount;
	ULONG	sizeDTrack;
	LONG midiSignal;
	struct Player *pPlayerInfo;
	struct MidiLink   *pMidiLink;
	struct MidiNode   *pMidiNode;
	UBYTE *pData;
	UBYTE	*pfillbuf[2];
	UBYTE	lastRSchan;
	struct Library *CamdBase;
	struct Library *RealTimeBase;
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
