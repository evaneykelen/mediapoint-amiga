/**** edit.c ****/

BOOL MonitorUser(struct Window *window, struct EditRecord *edit_rec);
BOOL GetSerialPrefs(STRPTR path, struct EditRecord *edit_rec);
BOOL SetSerialPrefs(STRPTR path, struct EditRecord *edit_rec);
void RemoveQuotes(STRPTR str);

/**** gui.c ****/

struct Window *OpenMPWindow(struct GadgetRecord *GR);
void CloseMPWindow(struct Window *window);
BOOL OpenInput(void);
void CloseInput(void);
BOOL MPOpenLibs(void);
void MPCloseLibs(void);

/******** E O F ********/

