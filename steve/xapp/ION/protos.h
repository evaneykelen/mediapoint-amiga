#include "demo:gen/support_protos.h"

/**** control ****/

BOOL sendIV321code(struct Ion_record *Ion_rec, STRPTR str);
BOOL GetCurrentFrame(struct Ion_record *Ion_rec, int *frame);
BOOL PerformActions(struct Ion_record *Ion_rec);
BOOL DoAsyncIO(	struct Ion_record *Ion_rec,
								ULONG io_Length, APTR io_Data, UWORD io_Command );
void SwitchTicksOn(struct Window *window);
void SwitchTicksOff(struct Window *window);

/**** controler ****/

struct Window *OpenControler(	struct UserApplicInfo *UAI, STRPTR dragBarTitle,
															struct GadgetRecord *GR, struct Image *image);
void CloseControler(struct UserApplicInfo *UAI, struct GadgetRecord *GR);
BOOL MonitorControler(struct UserApplicInfo *UAI, struct Window *window,
											struct Ion_record *Ion_rec,
											struct GadgetRecord *Controler_GR, UWORD *mypattern1,
											STRPTR unitStr);

/**** serial ****/

BOOL Open_SerialPort(	struct standard_record *std_rec, int readBits,
											int writeBits, int stopBits);
void Close_SerialPort(struct standard_record *std_rec);

/******** E O F ********/
