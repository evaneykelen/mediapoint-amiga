#include "demo:gen/support_protos.h"

/**** doit ****/

int XappDoIt(PROCESSINFO *ThisPI);
void Preview(struct PAR_Record *par_rec);

/**** setup ****/

void XappSetup(PROCESSINFO *ThisPI);
BOOL MonitorUser(struct Window *window, PROCESSINFO *ThisPI);
void GetExtraData(PROCESSINFO *ThisPI, struct PAR_Record *par_rec);
void PutExtraData(PROCESSINFO *ThisPI, struct PAR_Record *par_rec);
BOOL DoArexxCmd(STRPTR cmd, STRPTR ret, int max);
void PrintFrameNr(struct Window *window, int numFrames);
void InitPropInfo(struct PropInfo *PropInfo, struct Image *IM);
void DoSlider(struct Window *window, struct Gadget *g, ULONG body,
							struct PropInfo *PropInfo, UWORD numLevels, struct PAR_Record *par_rec);
void DragArrow(	struct Window *window, WORD *startX, WORD endX, WORD y1, WORD y2,
								BYTE mode, int numFrames, int frameRate, struct PAR_Record *par_rec);
void PAR_CheckEnteredTimeCode(int frameRate, STRPTR str);
void PAR_CheckHMSF(int *h, int *m, int *s, int *f, int frameRate);
void SetBackSlider(	struct Window *window, struct PropInfo *PropInfo,
										struct PAR_Record *par_rec, UWORD numLevels, ULONG body );

/******** E O F ********/
