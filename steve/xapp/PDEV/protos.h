#include "demo:gen/support_protos.h"

/**** control ****/

BOOL CommandPlayer(	struct Pdev_record *Pdev_rec, int acode, int x, int y,
										ULONG waitFlags );
BOOL OpenPlayerDevice(struct Pdev_record *Pdev_rec);
void ClosePlayerDevice(struct Pdev_record *Pdev_rec);
BOOL PerformActions( struct Pdev_record *Pdev_rec, ULONG waitFlags );

/**** controler ****/

struct Window *OpenController(struct UserApplicInfo *UAI, STRPTR dragBarTitle,
															struct GadgetRecord *GR, struct Image *image);
void CloseController(struct UserApplicInfo *UAI, struct GadgetRecord *GR);
BOOL MonitorController(	struct UserApplicInfo *UAI, struct Window *window,
												struct Pdev_record *Pdev_rec,
												int *frameReturn, struct GadgetRecord *Controler_GR);

/******** E O F ********/
