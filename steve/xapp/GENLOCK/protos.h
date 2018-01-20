#include "demo:gen/support_protos.h"

/**** control ****/

BOOL PerformActions(struct Genlock_record *gl_rec, struct Window *window,
										PROCESSINFO *ThisPI);
void GL_GenlockOn(struct Genlock_record *gl_rec, struct Screen *screen,
									PROCESSINFO *ThisPI);
void GL_GenlockOff(struct Screen *screen);

/******** E O F ********/
