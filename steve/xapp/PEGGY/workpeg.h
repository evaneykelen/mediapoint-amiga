//	File		:	workpeg.h
//	Uses		:
//	Date		:	10-11-93
//	Author	:	ing. C. Lieshout
//	Desc.		:	Proto's structures and defines for MPEG_record
//

struct MPEG_record *AllocMPEG(void);
void FreeMPEG(struct MPEG_record *MPEG_rec);
void play_mpeg( struct MPEG_record *mp );

struct MPEG_record
{
	struct IOMPEGReq *IOReq;
	struct IOMPEGReq *IOReq2;
	struct MsgPort  *IOPort;
	struct Device *MPEG_Device;
	int fade;
	int Xsize,Ysize;
	int Xoff,Yoff;
	int loops;
	char filename[256];

	LONG	mainsignal;
	LONG	sig_xtox;
	LONG	sig_ptoc;
	LONG	quitsig;
	struct MsgPort  *mport_ptoc;
	WORD	action;
	LONG restoresignal;

};

/******** E O F ********/
