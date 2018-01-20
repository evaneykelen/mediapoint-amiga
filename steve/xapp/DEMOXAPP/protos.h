#include "Gen/MINC/process.h"

// Setup.c

void XappSetup(PROCESSINFO *);
void MonitorUser(struct Window *, PROCESSINFO *);

// Doit.c

void GetVarsFromPI(struct DemoRecord *, PROCESSINFO *);
void PutVarsToPI(struct DemoRecord *, PROCESSINFO *);
BOOL Preview(struct DemoRecord *demo_rec, PROCESSINFO *ThisPI);
