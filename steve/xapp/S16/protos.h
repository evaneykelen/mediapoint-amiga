#include "demo:gen/support_protos.h"

/**** control ****/

BOOL PerformActions(struct Studio_Record *studio_rec);	//, STRPTR errorStr);
void SilenceInTheStudio(void);

/**** doit ****/

int XappDoIt(PROCESSINFO *ThisPI);

/**** setup ****/

void XappSetup(PROCESSINFO *ThisPI);
BOOL MonitorUser(struct Window *window, PROCESSINFO *ThisPI);
void DrawTocPage(	struct Window *window, struct Studio_Record *studio_rec,
									UWORD *mypattern1 );
void CheckOtherButtons(	struct Window *window, struct Studio_Record *studio_rec,
												struct FileReqRecord *FRR, UWORD *mypattern1,
												struct EventData *CED );
void GetExtraData(PROCESSINFO *ThisPI, struct Studio_Record *studio_rec);
void PutExtraData(PROCESSINFO *ThisPI, struct Studio_Record *studio_rec);
void DoPreview(	struct Window *window, struct Studio_Record *studio_rec,
								struct EventData *CED, UWORD *mypattern1 );

/******** E O F ********/
