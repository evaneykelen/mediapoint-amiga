/**** print_seg.c ****/

void main(int argc, char **argv);
BOOL doYourThing(void);
BOOL MonitorUser(struct Window *window);
void GetObjInfo(struct ScriptNodeRecord *this_node, STRPTR objectName);
BOOL OpenPrintingWindow(STRPTR printWhat);
void ClosePrintingWindow(void);
void TranslateGR(struct GadgetRecord *GR);

/**** printsupport.c ****/

UBYTE GetWBPrinterPrefs(void);
void SetWBPrinterPrefs(void);
BOOL OpenPrinter(void);
void ClosePrinter(void);
BOOL IsPrinterTurnedOn(void);
void PrintScript(void);
BOOL CreatePrintString(	struct ScriptInfoRecord *SIR,
												struct ScriptNodeRecord *this_node,
												STRPTR str, STRPTR str2,
												union printerIO *request, int *line, BOOL *aborted,
												int listLevel, int *page );
BOOL PrintHeader(void);
BOOL PrintFooter(int page, int list);
BOOL PrintCommand(union printerIO *request, int command,
									int p0, int p1, int p2, int p3);
BOOL PrintString(union printerIO *request, STRPTR string);
BOOL sendPrinterIO(union printerIO *request);
BOOL PrintFormFeed(void);
void DoGlobalEventInfo(	struct ScriptInfoRecord *SIR,
												struct ScriptNodeRecord *this_node,
												union printerIO *request, int *line, BOOL *aborted,
												int listLevel, int *page );
void KeyToKeyName(int key, int raw, STRPTR keyName);
void DoTimeCodeInfo(	struct ScriptInfoRecord *SIR,
											struct ScriptNodeRecord *this_node,
											union printerIO *request, int *line, BOOL *aborted,
											int listLevel, int *page );
void DoVarsInfo(	struct ScriptInfoRecord *SIR,
									struct ScriptNodeRecord *this_node,
									union printerIO *request, int *line, BOOL *aborted,
									int listLevel, int *page );
void VIRToDecl(VIR *this_vir, STRPTR declStr);
BOOL VARToExpr(struct ScriptInfoRecord *SIR, STRPTR declStr, VAR *this_var);
void PerformFormFeed(	union printerIO *request, int *line, BOOL *aborted, int *page,
											int list );

/**** printsup2.c ****/

void PrintPage(void);
void DumpPage(int vergroting);
BOOL DumpRPort(	union printerIO *request,
								struct RastPort *rastPort, struct ColorMap *colorMap,
								ULONG modes, UWORD sx, UWORD sy, UWORD sw, UWORD sh,
								LONG dc, LONG dr, UWORD s);
void DumpTexts(void);
BOOL CheckFF(int *line, int *page, BOOL doFF);
BOOL PrintPageFooter(int page);

/**** times.c ****/

void dateStringtoDays(char *strPtr, int *totalDays);
void timeStringtoMinutesAndTicks(char *strPtr, int *minutes, int *ticks);
void timeStringtoDuration(char *strPtr, int *hh, int *mm, int *ss, int *tt);
void timeStringtoTimeCode(char *strPtr, int *hh, int *mm, int *ss, int *ff);
void secondsToDuration(int seconds, STRPTR str);
void datestampToTime(ULONG minute, ULONG tick, STRPTR str);
void datestampToDate(ULONG days, STRPTR str);
void SystemDate(int *day, int *month, int *year);
void SystemTime(int *hours, int *mins, int *secs);
int DayOfWeek(int month, int year);
int MonthLength(int month, int year);
void createStartEndDay(LONG days, STRPTR str, int startend);
void createStartEndTime(LONG minutes, LONG ticks, STRPTR str);
void datestampToDMY(ULONG days, int *day, int *month, int *year);
void datestampToHMST(	ULONG minutes, ULONG ticks,
											int *hours, int *mins, int *secs, int *tenths);
void DayMonthYearToString(STRPTR str, int day, int month, int year);
void HoursMinsSecsTenthsToString(STRPTR str, int hours, int mins, int secs, int tenths);
void DurationStringToSeconds(STRPTR str, ULONG *seconds);
void CheckEnteredDate(struct Window *window, struct GadgetRecord *GR, int ID);
void CheckEnteredTime(struct GadgetRecord *GR);
void DDMMYY_2_DDMMMYYYY(STRPTR in, STRPTR out);

/******** E O F ********/
