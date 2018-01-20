//	File		: serhost.h
//	Uses		:
//	Date		: 26 june 1994
//	Author	: ing. C. Lieshout
//	Desc.		: proto's and struct for serial communications
//

#include "waitsec.h"

#define _PRINT 0
#define _SERPRINT 0

#define NAK 0x15
#define ACK 0x06
#define EOT 0x04
#define CAN 0x18

#define PROTO_BUFFER_SIZE 2048
#define SUPER_NAME "password"
#define DEFAULT_PATH "ram:"
#define DEFAULT_DIRNAME "MP_RA:"
#define DEVICE_NAME "serial.device"
#define UNIT_NUMBER 0
#define SERIAL_BUFFER_SIZE 10000
#define READ_BUFFER_SIZE 512
#define DEFAULT_BAUDRATE 9600
#define DEFAULT_PRIOR 127
#define BUFFER_REMAIN 100000
#define MAX_PSWDS 2
#define MAX_PSWD_SIZE 8
#define ID_SUPER 1024

#define COPY_BUF_SIZE 10240

typedef struct serpref
{
	long baudrate;
	long controlbits;
	long unit_number;
	long read_buffer_size;
	long priority;
	short int run,notrun;
	short int connectionclass;
	unsigned char lftocr,crtolf;
	char devname[50];
	char login[MAX_PSWD_SIZE];
	char dialpref[128];
	char replystring[128];
	char phonenr[40];
	char currentname[250];
//	char Downname[250];
//	char dirname[250];
} SERPREF;

typedef struct serdat
{
	struct MsgPort	*SerialMP;
	struct MsgPort	*SerialWriteMP;
	struct IOExtSer *SerialIO;
	struct IOExtSer *SerialWriteIO;
	struct Library *DOSBase;
	unsigned char *buf;
	long bufsize;

	struct wjif secsig;
	long ID;
	char superpwd[MAX_PSWD_SIZE];
	char passwords[MAX_PSWDS][MAX_PSWD_SIZE];
	int DevOpen;
	char dirname[250];
	long bytessend,time;
	long totalsend,totaltime;
	SERPREF pref;
	unsigned char	*buffer;
} SERDAT;
