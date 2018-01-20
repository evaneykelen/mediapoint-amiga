
/* IFF definition */

struct PictHeader {
	ULONG	file;
	FyeBitMapHeader bmhd;
	ULONG	bitplanesize;	/* memory used by one bitplane */
	ULONG	bodyoffset;
	ULONG	bodysize;
	ULONG	extra;			/* size of body chunk in iff file */
	ULONG	camg;			/* Fye file mode: ilbm or acbm */

/* Extra for ILBM mode */
	UBYTE	* filebuffer;
	ULONG	filebuffersize;
	ULONG	bufferptr;
	ULONG	offsetstartbuffer;

	SHORT	vpdx,vpdy;
};

struct FyeScreen {
	struct View	v;
	struct ViewPort	vp;
	struct BitMap	bmp[6];
	struct RasInfo	ri;
	struct View	* oldview;
	struct cprlist	* lof[6];
	struct cprlist	* shf[6];
};

#define	IFFILBM			1
#define	FILEBUFFERLENGTH	40000

#define between(m,v,n) ((m)<(v)?((n)>(v)?(v):(n)):(m))

#define	HEADERLEN	20
#define	IFF_BMHD		20
#define	BUFFERLEN	30
#define	CHUNKHEADER	8

#define	DEPTH	4

#define DX	0
#define	DY	0

#define	MODE_24BP_CHIPRAM	0x8000	/* the 24 bitplanes are in chip ram. Display the picture in one time */
#define MODE_24BP_FASTRAM	0x4000	/* the 20 bitplanes are in fast (or chip) ram
										and 4 first bitplanes are in chip ram */

#define CONTINOUS_BITPLANES	1	/* if the bitplanes' memory has been allocated in one step,
									you can add this flag in readIffPictBody (It is faster */
