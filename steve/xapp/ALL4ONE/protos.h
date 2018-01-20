#include "gen:support_protos.h"

/**** control ****/

BOOL sendVPcode(struct VuPort_record *vp_rec, STRPTR str);
BOOL PerformActions(struct VuPort_record *vp_rec);
BOOL DoAsyncIO(struct VuPort_record *vp_rec, UBYTE *buffer);


/**** serial ****/

BOOL Open_SerialPort(	struct standard_record *std_rec, int readBits,
											int writeBits, int stopBits);
void Close_SerialPort(struct standard_record *std_rec);

/******** E O F ********/
