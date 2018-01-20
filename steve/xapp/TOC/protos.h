#include "demo:gen/support_protos.h"

/**** doit ****/

int XappDoIt(PROCESSINFO *ThisPI);

/**** setup ****/

void XappSetup(PROCESSINFO *ThisPI);
BOOL MonitorUser(struct Window *window, PROCESSINFO *ThisPI);
void DrawTocPage(	struct Window *window, struct Toccata_Record *toc_rec,
									UWORD *mypattern1 );
void GetExtraData(PROCESSINFO *ThisPI, struct Toccata_Record *toc_rec);
void PutExtraData(PROCESSINFO *ThisPI, struct Toccata_Record *toc_rec);
struct ToccataBase *ObtainToccata(struct Window *window);
void ReleaseToccata(struct ToccataBase *lib);
void GetToccataDefaults(struct Toccata_Record *toc_rec);
void SetToccataDefaults(struct Toccata_Record *toc_rec);
void CheckOtherButtons(	struct Window *window, struct Toccata_Record *toc_rec,
												struct FileReqRecord *FRR, UWORD *mypattern1,
												struct EventData *CED );
void DoPreview(	struct Window *window, struct Toccata_Record *toc_rec,
								struct EventData *CED, UWORD *mypattern1, BOOL toccataPresent,
								struct ToccataBase *ToccataBase );
void DoRecord(struct Window *window, struct Toccata_Record *toc_rec,
							struct EventData *CED, UWORD *mypattern1 );
void SetTheVol(struct ToccataBase *ToccataBase, UBYTE *perc_to_db, int val, struct TagItem *tags);

/******** E O F ********/
