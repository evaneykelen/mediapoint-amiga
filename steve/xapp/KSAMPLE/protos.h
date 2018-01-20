#include "demo:gen/support_protos.h"

/**** setup ****/

void XappSetup(PROCESSINFO *ThisPI);
BOOL MonitorUser(struct Window *window, PROCESSINFO *ThisPI);
void DrawPage(BYTE page, struct Window *window, struct Sample_record *sample_rec,
							UWORD *mypattern1);
void SetButtons(BYTE page, struct Window *window, struct Sample_record *sample_rec,
								UWORD *mypattern1);
void CheckOtherButtons(	struct Window *window, BYTE page, struct EventData *CED,
												struct Sample_record *sample_rec,
												struct FileReqRecord *FRR, UWORD *mypattern1 );

/**** doit ****/

void GetVarsFromPI(struct Sample_record *sample_rec, PROCESSINFO *ThisPI);
void PutVarsToPI(struct Sample_record *sample_rec, PROCESSINFO *ThisPI);

/******** E O F ********/
