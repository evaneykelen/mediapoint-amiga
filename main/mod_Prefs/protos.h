/**** prefs_seg ****/

int main(int argc, char **argv);
BOOL doYourThing(void);
BOOL MonitorUser(struct Window *window);
void RenderPrefsWindow(int screen, struct Window *window);
void MonitorPrefsScreen(int screen, int ID, struct Window *window);
void ValidateTimeCode(struct GadgetRecord *GR, struct Window *window);
void TakeNextColor(struct Window *window);
BOOL GetPlayerDeviceInfo(void);
BOOL PutPlayerDeviceInfo(void);
int LanToCycle(int lanCode);
int CycleToLan(int cycle);
void InitPropInfo(struct PropInfo *PI, struct Image *IM);

/**** monsfuncs ****/

int MakeMonitorList(UBYTE *monitorList, ULONG *IDS, UBYTE maxMonitors, BOOL laced,
										WORD width, WORD height, ULONG monitorID, STRPTR monName, int *listPos);

/**** selectmon ****/

BOOL SelectMonitor(	struct Window *onWindow, ULONG *monitorID, BOOL laced,
										WORD width, WORD height, STRPTR monName );

/******** E O F ********/
