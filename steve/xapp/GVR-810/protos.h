/**** control ****/

BOOL PerformActions(struct GVR_record *GVR_rec);
BOOL DoAsyncIO(	struct GVR_record *GVR_rec,
								ULONG io_Length, APTR io_Data, UWORD io_Command );
void GetFrameCode(struct GVR_record *GVR_rec, STRPTR retStr);

/**** controller ****/

struct Window *OpenController(struct UserApplicInfo *UAI, STRPTR dragBarTitle,
															struct GadgetRecord *GR, struct Image *image);
void CloseController(struct UserApplicInfo *UAI, struct GadgetRecord *GR);
BOOL MonitorController(	struct UserApplicInfo *UAI, struct Window *window,
												struct GVR_record *GVR_rec,
												struct GadgetRecord *Controler_GR, UWORD *mypattern1);

/**** serial ****/

BOOL Open_SerialPort(	struct standard_record *std_rec, int readBits,
											int writeBits, int stopBits);
void Close_SerialPort(struct standard_record *std_rec);

/**** support ****/

BOOL OpenAllLibs(void);
void CloseAllLibs(void);
void OpenUserApplicationFonts(struct UserApplicInfo *UAI, STRPTR fontName,
															int size1, int size2);
void CloseUserApplicationFonts(struct UserApplicInfo *UAI);
void SetSprite(struct Window *window, int number, UWORD *ptr);
void GiveMessage(struct Window *window, char *fmt, ...);
void MyFormatter(char *str, char *s, va_list Arg);
char *MyParseType(char *q);
BOOL IsThisAPALMachine(void);

/******** E O F ********/
