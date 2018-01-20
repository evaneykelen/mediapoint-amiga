/******************** Player.h **************************
 *							*
 *    Advanced Player Device : General-purpose header	*
 *							*
 *    Last Modified: 17-Oct-89				*
 *							*
 ********************************************************/

#ifndef PLAYER_H
#define PLAYER_H

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/libraries.h>
#include <exec/io.h>
#include <devices/serial.h>

   /* macro definition of device/Macro identity long */

#define MakeID(a,b,c,d) ((a)<<24|(b)<<16|(c)<<8|(d))

#define APD_ID MakeID('A','A','P','D')
#define AMF_ID MakeID('A','P','D','M')

   /* player device name */

#define	PLAYER_DEVNAME	"player.device"

   /* definition of player device IO request block */

struct IOPlayer { 
   struct Message  io_Message;   /* IO standard */
   struct Device  *io_Device;    /* IO standard */
   struct Unit    *io_Unit;      /* IO standard */
   UWORD           io_Command;   /* AC_Code value */
   UBYTE           io_Flags;     /* for AVPD use ONLY! */
   BYTE            io_Error;     /* error return */
   ULONG           io_Actual;    /* internal / system calls only */
   ULONG           io_Length;    /* internal / system calls only */
   APTR            io_Data;      /* internal / system calls only */
   UWORD           iop_Format;   /* FORMAT_ value (below) */
   APTR            iop_Argin;    /* pointer to input */
   APTR            iop_Argout;   /* pointer to output */
   LONG            iop_Argx;     /* general parameter in/out */
   LONG            iop_Argy;     /* general parameter in/out */
   UWORD           iop_Macro;    /* Macro parameter in/out */
};

   /* supported parameter formats */

#define FORMAT_ARGS     0x00
#define FORMAT_STRING   0x01
#define FORMAT_ASCARGS  0x02     /* reserved and unused */
#define FORMAT_SPECIAL  0x04     /* reserved and unused */

/* device configuration data structure */

struct DeviceConfig {
   LONG  FRAMEerr;            /* 0 = do error on FRAME   contention */
   LONG  TIMEerr;             /* 0 = do error on TIME    contention */
   LONG  CHAPTERerr;          /* 0 = do error on CHAPTER contention */
   LONG  DevLock;             /* boolean - device locked flag       */
};

/* defined constants */

#define BUFLEN       32
#define MEMLIM       64
#define WRTBUF_SIZE  20
#define CFGLEN       64

/* Macro file header structure */

/* DEFINED CONSTANTS : Use NOMACRO to indicate NULL ID number */

#define NEWMACRO  TRUE
#define OLDMACRO  FALSE

#define DEADMACRO -2
#define NOMACRO   -1

struct MacroTag {
   UBYTE       Macrosetname[BUFLEN];   /* name of Macro set */
   struct List MacroList;              /* Macro List header */
   LONG        Macnum;                 /* Entry count */
   LONG        Ident;                  /* Identity byte */
};

struct MacroEntry {
   struct Node MacNode;                /* Macro Node */
   UBYTE       Macroname[BUFLEN];      /* Macro Name */
   LONG        MacID;                  /* Macro ID number */
   LONG        Macnum;                 /* Atom count */
   struct List DefList;                /* Definition List header */
};

struct MacroAtom {
   struct Node AtomNode;
   UBYTE       Argin[BUFLEN];
   LONG        Cmd;
   LONG        Format;
   LONG        Argx;
   LONG        Argy;
   LONG        Macro;
};

/***** player.device Unit structure *****/

struct PlayerUnit {
   struct MinNode Node;          /* node */

   LONG  UnitNumber;             /* number of this unit */
   UWORD OpenCnt;                /* open count for this unit */

   UBYTE Flags;                  /* unit-specific flags */
   UBYTE DoingExtIO;             /* doing ExtIO bodge */

   ULONG DriverSeg;              /* seglist for loaded driver */
   struct PlayerTag *DriverBase; /* addr of driver for this unit */

   struct MsgPort InternalRPort; /* port for replies to internal commands */
   struct IOPlayer InternalReq;  /* IORequest for internal cmds to process */
   struct MsgPort InternalPort;  /* process port for internal commands */
   struct MsgPort SerDevPort;    /* process port for serial-ish.device */
   struct IOExtSer SerReq;       /* IORequest to send to serial-ish.device */
   struct MsgPort IORPort;       /* process port for IORequests */

   struct IOPlayer *CurrentReq;  /* IOReq currently being processed by process*/
   struct IOPlayer *AbortReq;    /* IORequest to be aborted by process */

   UBYTE WrtBuf[WRTBUF_SIZE];    /* write buffer */

   UBYTE TermChar;               /* Terminating character */
   UBYTE TermFlag;               /* Termination flag */
   ULONG DeadTicks;              /* number of ticks for player dead time */

   struct DeviceConfig DevConfig; /* various flags used by drivers */

   struct MacroTag MacroTag;     /* tag structure for macro list, etc */
   struct MacroEntry *DefMacro;  /* pointer to entry for macro being defined */
   UBYTE MacEditFlags;           /* bit flags for Edit,Load,Merge allowed */
   UBYTE MacroFlags;             /* bit flags associated with macros */
   LONG  MacroLevel;             /* macro nesting level when executing macro */
   LONG  MacroError;             /* error number for nested macro handling */

   UBYTE MacroDir[CFGLEN];       /* directory for macro load/save */
   UBYTE DVectBuffer[CFGLEN];    /* buffer for AC_DVECT use only!! */
};

/***** Bit definitions to do with macros *****/

/* MacEditFlags (user settable via AC_MEDIT command) */

#define MACF_EDIT       0x01
#define MACF_LOAD       0x02
#define MACF_MERGE      0x04

#define MACEDIT   MACF_EDIT      /* for downward compatibility */
#define MACLOAD   MACF_LOAD
#define MACMERGE  MACF_MERGE

/* MacroFlags (for internal use) */

#define MACROF_DEFINE   0x01
#define MACROF_STOPPED  0x02

/***** Command Tag extender structure *****/

struct TAGExtend {   /* driver internal use only */
   UBYTE *Insert; 
   UBYTE *Term;
   ULONG Command;
   ULONG Flag;
};     

/***** Command Tag structure *****/

struct TAGCommand {  /* driver internal use only */
   LONG              (*Function)();
   UBYTE             *String; 
   struct TAGExtend  *NextCmd;
   UWORD             Flag;
};     

/* driver header structure - appears at start of all driver files */

struct PlayerTag {
   UBYTE    *playername;      /* player/driver name */
   LONG     (*Init)();        /* initialisation routine */
   LONG     (*Expunge)();     /* expunge/abort routine (if used) */
   LONG     (*Service)();     /* poLONGer to driver main */
   LONG     (*ExtCmd)();      /* poLONGer to device IO patch */
   LONG     VertBValue;       /* default dead time count */
   LONG     TermChar;         /* char to terminate write */
   LONG     playerID;         /* driver specific identity code */

   LONG     TAG_IOBUFLEN;     /* length of internal IO buffers */
   LONG     TAG_IOBAUD;	      /* baud rate rs232 */
   LONG     TAG_IOBRKTIME;    /* breaktime rs232 */
   LONG     TAG_IOTERM0;      /* terminators */
   LONG     TAG_IOTERM1;

   BYTE     TAG_IOREADLEN;    /* read length bits */
   BYTE     TAG_IOWRITELEN;   /* write length bits */
   BYTE     TAG_IOSTOPBITS;   /* # stop bits */
   BYTE     TAG_IOEOFFLAG;    /* parsing eofs on cmd flag */
   BYTE     TAG_IOSERF7;      /* 7/3 wire flag */
   BYTE     TAG_TERMFLAG;     /* include termchar on write flag */

   LONG     Ident;            /* driver ID marker */
};

   /* driver operational code follows this structure ..... */

#endif

/***** End of Player.h *****/
