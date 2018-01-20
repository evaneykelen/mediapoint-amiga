/**** structs.h ****/

enum
{
CDF_CONNECTIONCLASS, CDF_SERIALDEVICE, CDF_UNITNUMBER, CDF_BAUDRATE, CDF_HANDSHAKING,
CDF_BUFFERSIZE, CDF_DIALPREFIX, CDF_PHONENR, CDF_PASSWORD, CDF_DESTINATIONPATH,
CDF_REPLYSTRING,
};

struct CDF_Record
{
// Part 1

TEXT CDF_Path[SIZE_FULLPATH];
STRPTR TempScript_Path;
STRPTR BigFile_Path;

// Part 2

TEXT ConnectionClass[8];					// 1=serial cable, 2=parallel cable, 3=SCSI, 4=modem
																	// 5=network, 6=data broadcast, 7=infrared 8=email
																	// 9=ISDN, 10=leased line
TEXT SerialDevice[SIZE_FILENAME];
TEXT UnitNumber[8];
TEXT BaudRate[32];
TEXT HandShaking[32];							// "NONE" "XON/XOFF" "RTS/CTS"
TEXT BufferSize[32];
TEXT DialPrefix[32];							// "ATDT" "ATDP"
TEXT PhoneNr[64];
TEXT PassWord[32];
TEXT DestinationPath[SIZE_FULLPATH];
TEXT ReplyString[64];
int  ChangeScriptType;				// 0 change direct, 1 change with swapfile
int  SendError;
int  RetryCount;

// Part 3

VOID *misc_ptr1;
VOID *misc_ptr2;
VOID *misc_ptr3;
VOID *misc_ptr4;
};

enum {
CC_SERIALCABLE=1, CC_PARALLELCABLE, CC_SCSI, CC_MODEM, CC_NETWORK, CC_DATABROADCAST,
CC_INFRARED, CC_EMAIL, CC_ISDN, CC_LEASED_LINE
};

// Define Added KC

#define SERP ((SERDAT *)CDF_rec->misc_ptr1)


/******** E O F ********/
