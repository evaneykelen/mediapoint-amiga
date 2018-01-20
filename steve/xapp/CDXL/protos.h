#include "demo:gen/support_protos.h"

/**** setup ****/

void XappSetup(PROCESSINFO *ThisPI);
BOOL MonitorUser(struct Window *window, PROCESSINFO *ThisPI);
void GetExtraData(PROCESSINFO *ThisPI, STRPTR fullPath);
void PutExtraData(PROCESSINFO *ThisPI, STRPTR fullPath);
void RemoveQuotes(STRPTR str);
BOOL DoWeHaveAtLeastA68020(void);

/******** E O F ********/
