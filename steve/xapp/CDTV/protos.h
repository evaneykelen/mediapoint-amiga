#include "demo:gen/support_protos.h"

/**** serial ****/

BOOL Open_SerialPort(struct CDTV_record *CDTV_rec);
void Close_SerialPort(struct CDTV_record *CDTV_rec);
void GetInfoFile(STRPTR appName, STRPTR appPath, STRPTR devName, int *portNr, int *baudRate);
BOOL SendSerCmd(struct CDTV_record *CDTV_rec, int cmd);
BOOL SendStringViaSer(struct CDTV_record *CDTV_rec, STRPTR str);
BOOL GetStringFromSer(struct CDTV_record *CDTV_rec, STRPTR str, int length);

/**** setup ****/

void XappSetup(PROCESSINFO *ThisPI);
BOOL MonitorUser(	struct Window *window, PROCESSINFO *ThisPI,
									struct CDTV_record *CDTV_rec);
void SetButtons(struct CDTV_record *CDTV_rec, struct Window *window, UWORD *mypattern1,
								struct PopUpRecord *PUR);
BOOL CheckTimeCode(STRPTR str);
BOOL ControlCDTV(struct CDTV_record *CDTV_rec);

/******** E O F ********/
