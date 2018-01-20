/**** gen: protos ****/

/**** external function (Cees, gen:asm/) ****/

extern void __asm __saveds BltBitMapFM(	register __a0 struct BitMap *,
																				register __d0 WORD,
																				register __d1 WORD,
																				register __a1 struct BitMap *,
																				register __d2 WORD,
																				register __d3 WORD,
																				register __d4 WORD,
																				register __d5 WORD,
																				register __d6 UBYTE,
																				register __d7 UBYTE,
																				register __a2 UWORD *);

/**** PH protos ****/

/**** mltool ****/

GLOBAL void GetToolTypes(struct WBArg *wbarg, STRPTR scriptName);
GLOBAL void GetWorkbenchTools(int argc, char **argv, STRPTR scriptName);

/**** procinit ****/

int GetRefreshRate(ULONG ID);

/***** NB protos ****/

/**** about ****/

void ShowAbout(void);

/**** backwin ****/

void ClearBackWindow(void);
void ImportABackground(	BOOL resizeIt, int remapIt, STRPTR picpath,
												STRPTR picname );
void ShowBackground(void);
void ProcessLoadedColors(struct EditSupport *es, BOOL remapIt);

/**** bgm ****/

void ShowBGM(void);

/**** clipanim ****/

BOOL ClipAnimSettings(struct Window *onWindow);

/**** clipboard ****/

struct IOClipReq *CBOpen(ULONG unit);
void CBClose(struct IOClipReq *ior);
int CBWriteFTXT(struct IOClipReq *ior, char *string);
BOOL WriteLong(struct IOClipReq *ior, LONG *ldata);
int CBQueryClipboard(struct IOClipReq *ior);
struct cbbuf *CBReadCHRS(struct IOClipReq *ior);
BOOL ReadLong(struct IOClipReq *ior, ULONG *ldata);
struct cbbuf *FillCBData(struct IOClipReq *ior, ULONG size);
void CBReadDone(struct IOClipReq *ior);
void CBFreeBuf(struct cbbuf *buf);
void CBCutCopy(void);
void CBPaste(void);

/**** clock ****/

BOOL startClockTask(void);
void stopClockTask(void);
VOID __saveds ClockServer(void);

/**** colad2 ****/

void InitPaletteControls(void);
void SelectNewWell(int skip, int newWell);
void InitSampleAndSliders(int well);
void SetSliders(void);
void PrintPaintColor(int r, int g, int b);
void Harmonize(void);
void ColorRamp(void);

/**** coloradjust ****/

int Monitor_ColorAdjust(void);
void SetSliderColor(int r, int g, int b, int well, int numColors, int how);
void PickAColor(void);
void FlipperColors( int well );
void InitPickWell(void);

/**** colorfuncs ****/

void SpreadCols(int start, int end);
void SwapCols(int start, int end);
void CopyCols(int start, int end);
void UndoCols(void);
void HarmonizeCols(int min, int max, int bright, BOOL onlyPage);
void ColorRampCols(int r, int g, int b, BOOL onlyPage);

/**** colormisc ****/

int GetNumberOfModeColors(ULONG viewmodes, int depth);
BOOL OpenPalette(BOOL initUndo);
void ClosePalette(void);
void TogglePaletteOnOff(void);
void PaletteToBack(void);
void PaletteToFront(void);
void RefreshPalette(void);
int DrawWells(struct Window *window);
void CreateColorsInPalette(int how, int mode);
BOOL MakeCopperList(int how, BOOL Quick);
void doCMoves(struct ColorMap *cm, int wellStart, int colorOffset, int num);
void doCMovesHalfB(struct ColorMap *cm, int wellStart, int colorOffset, int num);
void GetHalfBrite(UWORD *rgb);
void GetHalfBrite32(ULONG *rgb);
void RebuildCopperList(void);
void SyncAllColors(BOOL refreshPalette);
void SelectAWell(int well);
int FindSelectedWell(int mouseX, int mouseY);
int HowToFillTheWells(int numColors);
//BOOL MakeCopperListFast(int how);
BOOL SmallScreenCorrect(struct Screen *screen);

/**** cols32 ****/

void SetColorCM4(struct ColorMap *cm, UWORD rgb, int well);
void SetColorCM32(struct ColorMap *cm, ULONG rgb, int well);
void SetScreenToCM(struct Screen *screen, struct ColorMap *cm);
void SetScreenToColorTable4(struct Screen *screen, UWORD *colorTable, int max);
void SetScreenToPartialCM(struct Screen *screen, struct ColorMap *cm,
													int start, int end);
UWORD GetColorCM4(struct ColorMap *cm, int well);
ULONG GetColorCM32(struct ColorMap *cm, int well);
void CopyCMtoCM(struct ColorMap *from_cm, struct ColorMap *to_cm);
void CopyCMPartial(	struct ColorMap *from_cm, struct ColorMap *to_cm, 
										int from, int to, int num );
void TransferCMtoCM(struct ColorMap *from_cm, struct ColorMap *to_cm,
										int from, int to, int num);
BOOL CompareCM(struct ColorMap *cm1, struct ColorMap *cm2);
BOOL CompareCMPartial(struct ColorMap *cm1, struct ColorMap *cm2,
											int start, int end);
BOOL CompareCMandColorTable4(struct ColorMap *cm, UWORD *colorTable,
														int start, int end);
void GetColorComponents(struct ColorMap *cm, int well, int *r, int *g, int *b);
void SetColorComponents(struct ColorMap *cm, int well, int r, int g, int b);
void SetVPToComponents(struct ViewPort *vp, int well, int r, int g, int b);
void Turn4into32(UWORD rgb, ULONG *r, ULONG *g, ULONG *b);
void TurnSmallIntoLarge(ULONG *r, ULONG *g, ULONG *b);

/**** commod ****/

BOOL OpenMagicBroker(void);
void CloseMagicBroker(void);
void CxFunction(register CxMsg *cxm, CxObj *co);

/**** config ****/

BOOL GetConfigFile(void);
void SetConfigDefaults(void);
void MakeLanExt(UBYTE lanCode, STRPTR lanExt);
void WriteConfigFile(void);
void updateCAPSconfig(int which);
BOOL cleanUpFile(STRPTR path, STRPTR pattern, char **cmdList);
BOOL GetPlayerDeviceInfo(void);
void InitMRO(void);
void AddMRO( STRPTR MRO, STRPTR path );
void SaveMRO(void);
void LoadMRO(void);

#if 0
BOOL GetConfigFile(void);
void SetConfigDefaults(void);
void MakeLanExt(UBYTE lanCode, STRPTR lanExt);
void GetScreenDimensions(WORD *args, int mode);
void GetModesFromID(int mode1, int mode2, ULONG *modes, ULONG *extramodes);
void CalculateWidthAndHeight(	int mode1, int oscan, int scanmode,
															int *width, int *height);
void WriteConfigFile(void);
void GetScreenParams(int ID, int *mode1, int *mode2, int *oscan);
void ValidateScreenSize(int *w, int *h, ULONG *modes);
void updateCAPSconfig(int which);
BOOL cleanUpFile(STRPTR path, STRPTR pattern, char **cmdList);
BOOL GetPlayerDeviceInfo(void);
int ChangeMonitor(int mon, ULONG *monitorID, UBYTE *PalNtsc);
void SetGlobalMonitorDimensions(void);
void InitMRO(void);
void AddMRO( STRPTR MRO, STRPTR path );
void SaveMRO(void);
void LoadMRO(void);
int ChangeMonitorV2(int mon, ULONG *monitorID, UBYTE PalNtsc);
#endif

/**** crawl ****/

BOOL MonitorCrawl(void);
void DrawStrGad(struct Window *window, struct GadgetRecord *GR, UBYTE *txt,
								UBYTE *undo, int max, struct StringInfo *SI, struct Gadget *G,
								struct StringExtend *SE);
BOOL SimpleGetFile(STRPTR fullpath, UBYTE *txt, ULONG max);

/**** da ****/

BOOL Alloc_DA(void);
void Free_DA(void);
BOOL Process_DA_Menu(int daLine);
BOOL ExecuteDA(struct Window *window, int daLine);

/**** dbase ****/

BOOL ImportDBaseFile(STRPTR path, UBYTE *buffer);
BOOL InspectDBaseFile(struct DBaseRecord *dbase_rec, STRPTR path);

/**** dbase_funcs ****/

BOOL OpenDBaseFile(struct DBaseRecord *dbase_rec, char *filename, BOOL readFile);
void CloseDBaseFile(struct DBaseRecord *dbase_rec);
void ReadNBytesMax(FILE *fp, char *buffer, int num);
void KillWhiteSpaces(UBYTE *srcPtr, UBYTE *dstPtr, int len);

/**** dblclicked ****/

BOOL processDblClick(int top, SNRPTR this_node);
void processScriptSerPar(SNRPTR this_node);
void processScriptParent(void);
void PrintSubBranchName(STRPTR name);

/**** debug ****/

void ShowSNR(SNRPTR this_node, SNRPTR first_node);

/**** demomsg ****/

void ShowDemoMsg(void);
void ShowPersonalized(void);
void ShowBeta(void);

/**** distributor ****/

void Distributor(void);
void CheckBoundingBox(WORD *x, WORD *y, WORD w, WORD h);
void DrawHoriBars(void);
void DrawVertBars(void);

/**** dofont ****/

BOOL Monitor_FontSelection(struct EditWindow *ew);
void FillInFER(struct TextFont *font);

/**** dol ****/

void DrawObjectList(int start, BOOL allParts, BOOL force);
void printDuration_1(struct ScriptNodeRecord *this_node, int y);
void printDuration_2(struct ScriptNodeRecord *this_node, int y);
void printDuration_3(struct ScriptNodeRecord *this_node, int y);
void printDuration_4(struct ScriptNodeRecord *this_node, int y);
void DrawTimeCodes(int start);
void DrawTransitionBlocks(int start);
void DrawParBars(int start);
int DoObjectDateCheckV2(struct DateStamp *CurDate, struct ScriptNodeRecord *this_node);

/**** dos ****/

void findFullPath(struct FileLock *oriFL, STRPTR str);
BOOL LoadModule(STRPTR moduleName, BOOL *returnCode);

/**** dosmisc ****/

BOOL OpenDir(STRPTR path, int opts);
void CloseDir(void);
void QuickSortList(int l, int r, UBYTE *ptr, int lineSize);
int StringCmp(STRPTR p, STRPTR q);
void btoc(STRPTR bstring);
struct Node *GetNode(STRPTR name, UBYTE type, UBYTE pri);
void FreeNode(struct Node *mynode);
void getdisks(struct List *dlist, int mode);
void freedisks(struct List *dlist);
BOOL GetDevicesAndAssigns(void);
void FreeDevicesAndAssigns(void);
ULONG checkFileType(STRPTR path, STRPTR fileName);
void GetParentOf(STRPTR path);
BOOL ExecuteTextEditor(struct ScriptNodeRecord *this_node);
BOOL GetInfoOnForm(	STRPTR path, ULONG typeID, UBYTE *storage, int storageSize,
										struct FileLock *lock );
void UpdateDirCache(STRPTR path);

/**** dostyle1 ****/

void PutImageInCycleGadget(	struct Window *window, struct GadgetRecord *GR,
														int x, int y, int w, int h );
void PutImageInRastPort(WORD srcX, WORD srcY, struct RastPort *dstRp,
												WORD dstX, WORD dstY, WORD dstW, WORD dstH);

/**** dostyle2 ****/

BOOL ToggleStyle(void);
int MonitorStyle(void);
void CloseStyleScreen(void);
void DoStyleSettings(void);
void EnableDisableStyleList(struct Window *window, int wdw);
void RenderStyleButtons(struct Window *window, int wdw);
void InterpretStyleSettings(struct EditWindow *localEW, int button);

/**** drag ****/

void StartIconDragging(int top1, int top2, int mode);
void DrawScriptIcon(int icon, int mouseX, int mouseY, int oldX, int oldY);
void PasteDraggedIcon(int icon, int top, int start_x, int start_y);
struct ScriptNodeRecord *AddObjectToList(int where, int icon, struct ScriptNodeRecord *this_node, STRPTR path, STRPTR filename);
void PrepareAddedNode(struct ScriptNodeRecord *this_node, STRPTR path,
											STRPTR filename, int icon);
void GetAnimSpeed(STRPTR path, STRPTR filename, int *fps, int *numFrames);

/**** dragmove ****/

BOOL DragMoveObjects(int top);
void DrawScriptObject(int mouseY, int *oldtopY, int *oldbottomY);
//void DrawObjectContourBox(int x1, int y1, int x2, int y2);
//void DrawObjectContourBox(int x1, int y1, int x2, int y2, int oldy1, int oldy2);
//void DrawObjectContourBox(int x1, int y1, int x2, int oldy1);
void DrawObjectContourBox(int y1, int oldy1);
BOOL MoveDragMovedObject(struct ScriptNodeRecord *this_node);

/**** duplicator ****/

void Duplicator(void);

/**** edit ****/

void CopyScriptObjects(	struct ScriptInfoRecord *SIR,
												struct ScriptNodeRecord *first_node);
void CutScriptObjects(struct ScriptInfoRecord *SIR,
											struct ScriptNodeRecord *first_node);
void ClearScriptObjects(struct ScriptInfoRecord *SIR,
												struct ScriptNodeRecord *first_node);
int SelectCurrentAndChildren(	struct ScriptInfoRecord *SIR,
															struct ScriptNodeRecord *first_node);
void SelectBranch(struct ScriptInfoRecord *SIR, struct List *list);
void FreeClipUndoLists(struct List **list);
BOOL CopyToClipboard(struct ScriptInfoRecord *SIR, struct List **list);
void UpdatePointers(struct ScriptInfoRecord *SIR, struct List **list);
void CleanScriptBits(struct ScriptInfoRecord *SIR);
void PasteClipboard(struct ScriptInfoRecord *SIR, int top, int start_x, int start_y);
void CheckNameUniqueness(	struct ScriptInfoRecord *SIR,
													struct ScriptNodeRecord *this_node);
void UpdateClipboardPointers(	struct ScriptInfoRecord *SIR,
															struct List **list,
															struct List **listPtrs);
void UpdateClipboardBranch(	struct ScriptInfoRecord *SIR,
														struct List *list, int *j,
														struct List **listPtrs);
void UndoClear(struct ScriptInfoRecord *SIR);
void UpdateAllListsPointers(struct ScriptInfoRecord *SIR,
														struct List **list, struct List **undoclipList);
void KillAllVars(struct ScriptInfoRecord *SIR);

/**** effect ****/

BOOL Monitor_Effect(WORD *p1, WORD *p2, WORD *p3, int type);
void PrintIconList(	struct GadgetRecord *GR, struct Window *window,
										int top, int displayLines);
void ScrollEffectList(	struct GadgetRecord *GR, struct Window *window,
												UBYTE *list, UBYTE *selectionList,
												int entryWidth, int numEntries, int *top,
												int numDisplay, struct Gadget *g, TEXT *ptrlist[],
												struct ScrollRecord *SR);
void GetEffectPos(int effNr, WORD *x, WORD *y);
void GetThickMax(int effNr, int *max);
int GetSpeed(int effNr);
int GetThick(int effNr);
void ConvertThickToChunck(int inThick, int *outThick, int *thickMax);
void ConvertChunckToThick(int inChunck, int *outThick, int thickMax);
BOOL GetInfoFromPageXaPP(void);
void FreeInfoFromPageXaPP(void);
struct Window *OpenEffectWindow(struct Window *onWindow, struct GadgetRecord *GR,
																int numEntries, int topEntry, int numDisp);
void SetAllObj(WORD effect, WORD speed, WORD thick);
void DoSelAll(void);
void DoubleEffBM(BOOL makeDouble);

/**** effect2 ****/

BOOL Monitor_PLS_Effect(int wdw, int numWdw);
void Show_PLS_Eff(struct Window *window, struct GadgetRecord *GR, int wdw);

/**** errors ****/

void PrintSer(char *fmt, ...);
void Early_WarnUser(STRPTR str);
void Message(char *fmt, ...);
void Formatter(char *str, char *s, va_list Arg);
char *ParseType(char *q);
BOOL PostPlayMessage(char *fmt, ...);
void ProcessMsgQueue(void);

/**** eventhandler ****/

void HandleEvents(struct Window *spriteWindow);
LONG GetVolumeSize(STRPTR path);

/**** events ****/

void HandleIDCMP(struct Window *handleWindow, BOOL dragBar, WORD *ascii);
void GetMenuEquivalent(int *menu, int *item, struct IntuiMessage *msg, struct MenuRecord **MR);
void DoKeys(struct IntuiMessage *msg, USHORT *key);
LONG EventsDeadKeyConvert(struct IntuiMessage *msg,
													UBYTE *kbuffer, LONG kbsize, struct KeyMap *kmap);
BOOL MyDoubleClick(	ULONG prev_Seconds, ULONG prev_Micros,
										ULONG Seconds, ULONG Micros );
void doStandardWait(struct Window *waitWindow);
void CheckIfDepthWasClicked(struct Window *handleWindow);
WORD RawKeyToASCII(USHORT);

/**** fasteffect ****/

BOOL ChooseEffectFast(void);

/**** filethumbs ****/

int ShowThumbNails(	STRPTR path, UBYTE *selectionList, int Mode,
										UBYTE *listPtr, int numEntries);
int Monitor_Thumbs(	UBYTE *listPtr, int offset, int stepX, int stepY,
										int max, int Mode, int factor, int in_row, int in_col,
										int numEntries, int numThumbs, int dxs, int dys);
void DrawThumb(	int x, int y, int destXSize, int destYSize, STRPTR fullPath,
								STRPTR fileName);
BOOL ReadAndRenderIcon(	STRPTR filename, STRPTR path, int destXSize,
												int destYSize, int x, int y, struct RastPort *rp);
int QuickInterfere(	UBYTE *listPtr, int offset, int stepX, int stepY,
										int max, int Mode, int factor, int in_row, int in_col,
										int numEntries, int numThumbs, int dxs, int dys);
void SelectThumbNail(	UBYTE *listPtr, int offset, int stepX, int stepY,
											int max, int Mode, int factor, int in_row, int in_col,
											int numEntries, struct EventData *ED, int numThumbs,
											int dxs, int dys);
void ScaleAndRenderThumbnail(	struct BitMap24 *source_bm, struct IFF_FRAME *iff,
															int destXSize, int destYSize, int x, int y );

/**** fonts ****/

BOOL OpenAppFonts(STRPTR path);
BOOL OpenTypeFace(STRPTR name, int size, UBYTE flags, BOOL messageAllowed);
BOOL InitFontList(void);
void FreeFontList(void);
BOOL AddFontToList(	STRPTR fontName, int fontSize, struct TextFont *ptr,
										struct List *fontList);

/**** formatterm ****/

int SPrintf(char *, char *,...);

/**** fr ****/

struct Window *CreateFileRequester(int type, struct Window *onWindow, STRPTR title);
void CloseFileRequester(struct Window *window);
void InitPropInfo(struct PropInfo *PI, struct Image *IM);
BOOL OpenAFile(	STRPTR path, STRPTR fileName, STRPTR title, 
								struct Window *onWindow, int opts, BOOL multiple);
BOOL SaveAFile(	STRPTR path, STRPTR fileName, STRPTR title, 
								struct Window *onWindow, int opts);
BOOL MonitorFileRequester(int type, STRPTR path, STRPTR fileName, 
													struct GadgetRecord *GR, int opts, BOOL multiple,
													struct Window *window);
BOOL processClickOnFileOrDir(	struct Window *window, int *top, int line,
															STRPTR path, int opts,
															struct GadgetRecord *GR, struct ScrollRecord *SR);
void processClickOnAssign(struct Window *window, int *top, int line,
													STRPTR path, int opts, struct GadgetRecord *GR,
													struct ScrollRecord *SR);
void processClickOnDevice(struct Window *window, int *top, int line,
													STRPTR path, int opts, struct GadgetRecord *GR,
													struct ScrollRecord *SR);
void processClickOnHome(	struct Window *window, int *top, int line,
													STRPTR path, int opts, struct GadgetRecord *GR,
													struct ScrollRecord *SR );
void processClickOnParent(struct Window *window, int *top, int line,
													STRPTR path, int opts, struct GadgetRecord *GR,
													struct ScrollRecord *SR);
void processClickOnSelectAll(struct Window *window, int top, struct ScrollRecord *SR);
void LoadTheNewDir(	STRPTR oldPath, STRPTR newPath, int opts, int *top,
										struct GadgetRecord *GR, struct Window *window,
										struct ScrollRecord *SR);

/**** getxapps ****/

BOOL OpenToolIcons(struct Window *window, int *tHeight);
BOOL SniffXapps(int numFiles, UBYTE mode);
BOOL PlaceIconsInBitMap(void);
int GetIconIndex(int type);
void PutIconAt(int type, UBYTE mode, int pos);
void ShowToolIcons(struct Window *window, int line);
void CloseToolIcons(void);
void DrawToolDots(int type, BOOL flag);
void DisableTool(int type);
void EnableTool(int type);

/**** globalallocs ****/

BOOL GlobalAllocs(BOOL fromPlayer);
void FreeGlobalAllocs(BOOL fromPlayer);
BOOL AllocEasyBitmaps(void);
void FreeEasyBitmaps(void);

/**** globlab ****/

BOOL Monitor_GlobalLabels(struct Window *window);
BOOL MonitorPickLabel(struct Window *window,STRPTR labelName);
void AddGlobalEvent(int line, STRPTR labelName);
void standAlonePickLabel(struct Window *onWindow, SNRPTR this_node);
void DisplayGlobalList(	struct Window *window, struct GadgetRecord *GR,
												int topEntry, int lines, int displayFactor,
												int numEntries);
void doGlobalListSlider(struct Window *window,
												int numEntries, int numDisplay, int *topEntry,
												struct GadgetRecord *GR, struct Gadget *g, int displayFactor);
int SelectLine(	struct GadgetRecord *GR, int displayFactor,
								int numEntries, int numDisplay);
void HighLightLabelLine(struct Window *window,
												struct GadgetRecord *GR, int displayFactor, int line);
BOOL MonitorKeyList(struct Window *window, int *key, int *raw);

/**** helpwdw ****/

void HelpWindow(void);

/**** icon ****/

void SaveIcon(STRPTR fullPath);
void SaveScriptIcon(STRPTR fullPath);

/**** import ****/

void ProcessImport(void);
int DoImport(BOOL *resizeIt, int *remapIt);

/**** import2 ****/

void ImportADataType(BOOL resizeIt, int remapIt);
BOOL ShowDataType(STRPTR name, struct Screen *tempScreen);
void CloseDataTypeWindow(void);
int CheckDataType(STRPTR name);

/**** import_pic ****/

void ImportAPicture(BOOL resizeIt, int remapIt);
BOOL PutPictureIntoWdw(	STRPTR fullPath, STRPTR filename, int slot,
												BOOL resizeIt, int remapIt );
BOOL doActualILBMLoading(	STRPTR path, STRPTR fileName,
													struct EditSupport *es, struct EditWindow *ew, BOOL ask );
void ClearBitMap(struct BitMap *bm);
BOOL InitAndAllocBitMap(struct BitMap *bm, WORD d, WORD w, WORD h, LONG memType);
void FreeFastBitMap(struct BitMap *bm);
void ClearBitMap24(struct BitMap24 *bm);
BOOL InitAndAllocFastBitMap24(struct BitMap24 *bm, WORD d, WORD w, WORD h);
void FreeFastBitMap24(struct BitMap24 *bm);

/**** import_scr ****/

void ImportAScreen(BOOL resizeIt, int remapIt, struct Screen *inScreen);
BOOL PutScreenIntoWdw(struct Screen *screen, int slot, BOOL resizeIt, int remapIt);

/**** initglobals ****/

BOOL StartUpFuncs(BOOL fromPlayer);
BOOL OpenLibraries(void);
void CloseLibraries(void);
void SetGlobalVars(void);
void SetGlobalArrays(void);
BOOL CheckCoreFiles(void);
void MemWatcher(void);

/**** initmenus ****/

void OpenMenus(void);
void SetMenuTitles(void);
void CalculateMenuParams(void);
void CloseMenus(void);
void Fill_DA_Menu(BOOL skipUntitled);
void Fill_MRO_Menu(BYTE where);
void InitPageEditMenus(void);
void InitScriptEditMenus(void);
void ReRenderMenus(void);

/**** inputsettings ****/

void MonitorInputSettings(struct ScriptNodeRecord *this_node);

/**** inslit ****/

UWORD InsertLiteral(void);

/**** interact ****/

BOOL MakeInteractiveButton(void);
void KeyToKeyName(int key, int raw, STRPTR keyName);
void DrawIAButtons(int wdw, struct Window *window, struct EditWindow *ew);
void DoAllForInteractive(int wdw);
BOOL GetLEInfo( struct ScriptNodeRecord *this_node, STRPTR fullPath,
								struct ScriptInfoRecord *SIR );
BOOL ModifyLEInfo(	struct ScriptNodeRecord *this_node, STRPTR fullPath,
										struct ScriptInfoRecord *SIR );
BOOL UpdateLEInfo(	struct ScriptNodeRecord *this_node, STRPTR fullPath,
										struct ScriptInfoRecord *SIR );
void GetAllLEInfos(struct ScriptInfoRecord *SIR);
int Validate_All_LE(struct ScriptInfoRecord *SIR, BOOL makeGOTOS);
void Free_All_LE( struct ScriptInfoRecord *SIR );
void Copy_LE_Info(struct ScriptNodeRecord *oldSNR, struct ScriptNodeRecord *newSNR);
BOOL IsXappUsed(STRPTR xappname);
void RelinkButtons(struct ScriptNodeRecord *SNR);

/**** localedate ****/

void GetDateString(STRPTR str, BYTE format);
void ConvertDatePageToPlayer(STRPTR str, int format);

/**** main ****/

void CheckStack(void);
void FreeAndExit(void);
BOOL DonglePresent(void);

/**** memory ****/

//BOOL CheckMemStart(void);
//BOOL CheckMemRunning(struct Window *window);
void FlushTheSucker(void);

/**** menubar ****/

void Monitor_Menu(struct Window *window, int *menu, int *item, struct MenuRecord **MR);
int CheckWhichMenu(	struct Window *window, struct MenuRecord **MR, int num,
										struct RastPort *rp);
int CheckWhichItem(struct Window *window, struct MenuRecord *MR, WORD offX, WORD offY);
int CheckFastMenu(struct Window *window, struct MenuRecord *fastMR,
									struct MenuRecord **MR, struct RastPort *rp);
void saveMBAR(struct RastPort *rp, WORD x, WORD y, WORD width, WORD height);
void restoreMBAR(struct RastPort *rp);
void RenderMenuBar(	struct RastPort *rp, struct Window *window,
										struct MenuRecord **MR, BOOL fastMenus );

/**** menus ****/

void InitializeMenu(struct MenuRecord *MR, struct TextFont *menuFont);
void RenderMenuInterior(struct MenuRecord *MR);
void WriteIntoMenu(struct MenuRecord *MR, int line);
void DrawMenuLine(struct MenuRecord *MR, int line);
void RenderMenu(struct Window *window, struct MenuRecord *MR, WORD offX, WORD offY);
void RemoveMenu(struct Window *window, struct MenuRecord *MR, WORD offX, WORD offY);
void FreeMenu(struct MenuRecord *MR);
void DisableMenu(struct MenuRecord *MR, int line);
void EnableMenu(struct MenuRecord *MR, int line);
void ClearMenuLine(struct MenuRecord *MR, int line);
void ToggleChooseMenuItem(struct MenuRecord *MR, int line);
void SetChooseMenuItem(struct MenuRecord *MR, int line);
void UnsetChooseMenuItem(struct MenuRecord *MR, int line);

/**** misc ****/

void SetBit(ULONG *allocs, ULONG flags);
void UnSetBit(ULONG *allocs, ULONG flags);
BOOL TestBit(ULONG allocs, ULONG flags);
void SetByteBit(UBYTE *allocs, UBYTE flags);
void UnSetByteBit(UBYTE *allocs, UBYTE flags);
BOOL TestByteBit(UBYTE allocs, UBYTE flags);
void InvertByteBit(UBYTE *allocs, UBYTE flags);
struct Window *GetActiveWindow(void);
int AbsInt(int a, int b);
int AbsWORD(int a, int b);
void AdjustGadgetCoordsRange(	struct GadgetRecord *GR,
															int xoffset, int yoffset,
															int start, int end);
void RemoveQuotes(STRPTR str);
void swapInts(int *a, int *b);
void swapWORDS(WORD *a, WORD *b);
void ValidateOffsets(void);
void EnableButtonQuiet( struct GadgetRecord *GR );
ULONG CreateUsedXappList(struct ScriptInfoRecord *SIR);
void FreeUsedXappList(void);
void SetSpriteOfActWdw(BYTE which);
void PrintAt(int pen, struct RastPort *rp, int x, int y, STRPTR str);

/**** monitors1 ****/

BOOL SetMonitorDefaults(void);
BOOL SetMonitorFromConfig(void);
BOOL GetMonitorValues(ULONG monitorID, WORD *w, WORD *h, WORD *d);
BOOL CheckPAL(STRPTR screenname);
BOOL GetDimsFromMode1And2And3(int mode1, int mode2, int mode3, WORD *w, WORD *h);
ULONG GetModesFromMode1And2(int mode1, int mode2);
BOOL GetInfoOnModeID(ULONG modeID, WORD *w, WORD *h, WORD *d, WORD overscan);
void DefaultMonName(STRPTR name, ULONG ID, ULONG *monID);
BOOL GetIDWithMonName(STRPTR name, ULONG *monID, WORD w, WORD h, int oscan);
BOOL GetDimsFromIFF(struct IFF_FRAME *iff, WORD *w, WORD *h, int *oscan, WORD *maxDepth,
										ULONG *monID, ULONG *modes, BOOL dontCareH);
void GetOverscanValues(struct DimensionInfo *DimInfo, int oscan, WORD *w, WORD *h);

/**** monloc ****/

BOOL MonitorLocalEvents(struct ScriptInfoRecord *SIR);
UBYTE **CreateJumpList(int line, int *numJumps, struct ScriptInfoRecord *SIR);
void ModifyJumpList(struct ScriptInfoRecord *SIR,
										struct ScriptNodeRecord *temp_node, int numPages,
										int selectedPage, int selectedJump);

/**** nodes ****/

struct List *InitScriptList(void);
void FreeScriptList(struct List *list);
void FreeANode(struct ScriptNodeRecord *node);
struct ScriptNodeRecord *AddNode(struct List *list);
void InitScriptInfoRecord(struct ScriptInfoRecord *SIR);
void FreeScriptInfoRecord(struct ScriptInfoRecord *SIR);
BOOL CreateNewList(struct ScriptInfoRecord *SIR);
void ProcessGlobalEvents(struct ScriptInfoRecord *SIR);
struct ScriptNodeRecord *FindLabel(struct ScriptInfoRecord *SIR, STRPTR labelName);
struct List *FindParent(struct ScriptInfoRecord *SIR, struct List *oldList);
int FindParentType(struct ScriptInfoRecord *SIR, struct List *oldList);
struct ScriptNodeRecord *FindParentNode(struct ScriptInfoRecord *SIR, struct List *oldList);
struct ScriptNodeRecord *AllocateNode(void);
int InWhichListAreWe(struct ScriptInfoRecord *SIR, struct List *list);
void ChangeGotosAndGlobals(struct ScriptInfoRecord *SIR, STRPTR objectName, STRPTR tempName);
struct ScriptNodeRecord *CountNumSelected(struct ScriptNodeRecord *first_node, int *numSelected);
BOOL ValidateSER(struct ScriptInfoRecord *SIR, BOOL convert, BOOL quiet);
void UpdateDeferCont(struct ScriptInfoRecord *SIR);
void RemoveNodeFromGE(struct ScriptNodeRecord *this_node);
void FillInLabelPointers(struct ScriptInfoRecord *SIR);
BOOL CheckScriptProblems(struct ScriptInfoRecord *SIR);
struct ScriptNodeRecord *IsNodeTypePresent(struct ScriptInfoRecord *SIR, int type);

/**** objedit ****/

void DoObjectName(STRPTR objectName, STRPTR title, int nodeType);
void ValidateLabelName(STRPTR str);
struct Window *Open_A_Request_Window(struct Window *onWindow, struct GadgetRecord *GR);
void Close_A_Request_Window(struct Window *window);
BOOL Build_SmallScriptWindow(	struct GadgetRecord *GR,
															struct ScriptNodeRecord *this_node);
void editobject_AREXX_and_DOS(struct Window *window, struct ScriptNodeRecord *this_node);
void editobject_ANIM(struct Window *window, struct ScriptNodeRecord *this_node);
BOOL editobject_PAGE(struct Window *window, struct ScriptNodeRecord *this_node);
BOOL checkOKandCancel(struct Window *window);
BOOL selectAFile(struct Window *window, struct ScriptNodeRecord *this_node);
void editobject_SOUND(struct Window *window, struct ScriptNodeRecord *this_node);
void PrintSoundType(struct Window *window, SNRPTR this_node);

/**** opticols ****/

BOOL OptimizePalette(BOOL smooth);
void RemapWindows(int emptyFromHere);

/**** page ****/

int HandlePageEvents(void);
int CheckPageMenu(void);
void doEditWindows(void);
void SetPageEditMenuItems(void);
void DrawAllMarquees(BYTE mode);
void DrawSimpleBox(struct RastPort *rp, UWORD x1, UWORD y1, UWORD x2, UWORD y2);
void SetMargins(int hitWdw);
void FastCloseEW(int myWdw);
void FastDepthEW(int myWdw);
BOOL CheckSmallFastButtons(void);
void ChangeWindowStacking(int wdw);
void DrawTrack(void);
void CycleTruScreens(void);
void ScreenAtNormalPos(void);
int MassageY(int y);
void MyScreenToBack(struct Screen *screen);

/**** pagesetup ****/

void ShowPageSetUp(void);

/**** pagetalk ****/

void ScreenFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR);
void PaletteFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR);
void WindowFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR);
void ClipFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR);
void TextFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR);
void PageTalkFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR);
void EffectFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR);
void FormatFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR);
void StyleFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR);
BOOL DoPageILBM(STRPTR path,STRPTR fileName,struct EditWindow *ew,struct EditSupport *es);
void CrawlFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR);
void ButtonFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR);
void ObjectStartFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR);
void ObjectEndFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR);
void BackgroundFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR);
void ClipAnimFunc(struct ParseRecord *PR, struct PageInfoRecord *PIR);

/**** parser ****/

struct ParseRecord *OpenParseFile(char **cmdList, STRPTR path);
void CloseParseFile(struct ParseRecord *PR);
struct ParseRecord *OpenParser(void);
void CloseParser(struct ParseRecord *PR);
int GetParserLine(struct ParseRecord *PR, STRPTR buffer);
void passOneParser(struct ParseRecord *PR, char *buffer);
BOOL passTwoParser(struct ParseRecord *PR);
int GetCommandID(struct ParseRecord *PR);
void GetNumericalArgs(struct ParseRecord *PR, WORD *args);
void printError(struct ParseRecord *PR, STRPTR str);
void StrToScript(char *oldStr, char *newStr);
void ScriptToStr(char *oldStr, char *newStr);
int Special_GetParserLine(struct ParseRecord *PR, STRPTR buffer, STRPTR whiteSpcs);

/**** parsetext ****/

BOOL ParseText(	STRPTR path, STRPTR filename, struct EditWindow *ew,
								struct EditSupport *es, int *num );
void FetchString(STRPTR buffer, STRPTR dest, int max, int *count);
void FetchInteger(STRPTR buffer, int *dest, int *count);

/**** pedit ****/

void do_New(struct Document *doc);
BOOL do_Open(struct Document *doc, BOOL MRO, STRPTR MRO_path);
BOOL do_Close(struct Document *doc, BOOL setColors);
BOOL do_Close_no_Close(struct Document *doc);
void do_Save(struct Document *doc);
BOOL do_SaveAs(struct Document *doc, BOOL setColors);
void do_PageSetUp(struct Document *doc);
void do_Print(struct Document *doc);
void OpenDocumentProc(struct Document *doc);
void CloseDocumentProc(struct Document *doc);
BOOL SaveDocumentProc(struct Document *doc, BOOL setColors);
void DrawOpenedPage(void);
void DrawOpenedScript(void);
void DrawClosedPage(void);
void DrawClosedScript(void);
int AskDontSaveCancelSave(struct Document *doc, STRPTR askStr, BOOL cancel);
void SetClosedStatePageMenuItems(void);
void SetOpenedStatePageMenuItems(void);
void SetClosedStateScriptMenuItems(void);
void SetOpenedStateScriptMenuItems(void);

/**** prefs ****/

void ShowPrefs(void);
void SetScriptUserLevel(void);
void SetPageUserLevel(void);
void ChangeUserLevel(void);
void CheckLanguage(int newLan, int oldLan);
void HintUserLocale(void);

/**** print ****/

void ShowPrint(int which);
//BOOL OpenCopyOfBitmap(struct BitMap *bm, struct RastPort *rp);
//void CloseCopyOfBitmap(struct BitMap *bm);

/**** prog ****/

BOOL Build_program_Requester(	struct Window *onWindow, int top,
															struct ScriptNodeRecord *from_node);
BOOL Monitor_DateProgramming(	struct Window *window, 
															struct ScriptNodeRecord *selected_node);

/**** ptread ****/

BOOL ReadPage(STRPTR path, STRPTR fileName, char **pageCommands, BOOL ask);
BOOL CreatePageFromILBM(STRPTR path, STRPTR fileName, STRPTR fullPath,
												char **pageCommands, BOOL ask);
BOOL CreatePageFromDocument(STRPTR path, STRPTR fileName, STRPTR fullPath,
														char **pageCommands, BOOL ask);
void InitPageInfoRecord(struct PageInfoRecord *PIR);
BOOL InitNewWindow(struct PageInfoRecord *PIR, int wdwNr);
int GetAllTexts(STRPTR fullPath, char **pageCommands);
void ParseTextBuffer(UBYTE *buffer, int wdwNr);
BOOL ChangePageIntoIFFSize(struct IFF_FRAME *iff);

/**** ptwrite ****/

BOOL WritePage(STRPTR path, STRPTR fileName, char **pageCommands);
void doIndent(FILE *fp, int level);
void WriteTextLines(FILE *fp, struct EditWindow *ew, struct EditSupport *es,
										char **pageCommands);
BOOL findFont(struct TextFont *font, STRPTR fontname, int *fontsize);
void WriteCrawlLines(	FILE *fp, struct EditWindow *ew, struct EditSupport *es,
											char **pageCommands);
void DeleteOldClips(STRPTR fullPath, STRPTR fileName);
void WriteButtonInfo(	FILE *fp, struct EditWindow *ew, struct EditSupport *es,
											char **pageCommands);
BOOL SavePageAsScreen(STRPTR path);
void GuessOldStyleMode(int *mode);
BOOL CheckWriteProtect(STRPTR path, STRPTR fileName);

/**** ra ****/

void ShowRA(STRPTR scriptPath, STRPTR scriptName, int mode);

/**** rdwrcols.c ****/

BOOL ReadColorPalette(void);
BOOL WriteColorPalette(void);

/**** releasetwo ****/

void DoReleaseTwoFirstPart(void);
BOOL DoReleaseTwo(void);
void DoReleaseTwoSecondPart(void);
//BOOL CheckPAL(STRPTR screenname);
void FillLocaleStrings(void);
void WriteSmallLocaleStrings(void);
int GetCenterX(struct RastPort *rp, STRPTR str, int numChars, int numPixels);

/**** saveclips ****/

void WritePageClip(	FILE *fp, struct EditWindow *ew, struct EditSupport *es,
										STRPTR path, STRPTR fileName, char **pageCommands);
void WriteTheClip(STRPTR path, STRPTR fileName, struct BitMap *destBM,
									WORD clipW, WORD clipH, struct EditSupport *es);

/**** screens ****/

BOOL OpenPageScreen(void);
BOOL OpenPageWindow(void);
BOOL OpenScriptScreen(void);
BOOL OpenScriptWindow(void);
void OpenNewPageScreen(BOOL drawWins, BOOL screenToFront, BOOL openPlay, BOOL optimize);
void TempClosePageScreen(void);
BOOL ReopenPageScreen(void);
void TempCloseScriptScreen(void);
void ReopenScriptScreen(void);
BOOL OpenPlayScreen(ULONG monitorID);
void ClosePlayScreen(void);
void PlayScreenToFront(void);
void WaitInPlayScreen(void);
BOOL IsWBThere(void);
void GetOScanValues(struct Rectangle *spos, WORD w, WORD h, ULONG modes);
struct Window *OpenSmallScreen(int height);
void CloseSmallScreen(struct Window *window);
void CloseSmallScrWdwStuff(void);
int GetMaxDepth(ULONG ID);
BOOL ModeHasThisWidth(ULONG ID, WORD w);
void DeleteAllPicsAndBackground(void);

/**** screenthumbs ****/

int ShowThumbScreens(void);
struct Screen *Monitor_ScreenThumbs(int count, int destXSize, int destYSize, int height);
void DrawThumbBox(int x, int y, int w, int h);

/**** script ****/

int HandleScriptEvents(void);
BOOL doScriptMouseButtons(int *editIt);
void DoPlay(BOOL playFrom);
void DoScrolling1(struct Window *window, struct Gadget *g,
									ULONG numEntries, ULONG numDisplay, LONG *topEntry);
void DoScrolling2(struct Window *window, struct Gadget *g,
									ULONG numEntries, ULONG numDisplay, LONG *topEntry);
void doScriptEdSlider(void);
int CheckScriptMenu(void);
void dokeyScrolling(void);
BOOL do_pre_play_things(BOOL forPreview);
void do_post_play_things(BOOL forPreview);
void GenlockOff(struct Screen *screen);
void GenlockOn(struct Screen *screen);
struct ScriptNodeRecord *GetFirstEffObj(void);
struct ScriptNodeRecord *GetFirstSelObj(void);
BOOL GhostPlay(void);
void do_small_play(struct ScriptNodeRecord *this_node);
void DoTransitions(int mode);
void DoDuration(int mode);
void DoComment(int mode);
void SetScriptMenus(void);
void PlayTheScript(void);
void PlayTheScript_Small(struct ScriptNodeRecord *this_node);
BOOL pre_player(void);
void post_player(void);
BOOL GetScriptTime(struct Document *scriptDoc, struct FileInfoBlock *startFib);
BOOL CheckScriptTime(struct Document *scriptDoc, struct FileInfoBlock *startFib);

/**** scriptmisc ****/

void DrawScriptScreen(void);
void DrawScriptScreen_2(void);
void doShowAndProgMenus(void);
void ShowMainEventIcons(void);
void ShowSerialEventIcons(void);
void ShowParallelEventIcons(void);
void EnableAllEventIcons(void);
void DisableAllEventIcons(void);
void GetObjectName(	struct ScriptNodeRecord *this_node, STRPTR objectName,
										int lookStart, int width);
void ExpungeScript(void);
void DrawClosedScr(void);
BOOL FindSelectedIcon(int top);
void ClearInfoPanel(void);
struct ScriptNodeRecord *WhichObjectWasClicked(int top, int *row);
void SelectAllObjects(void);
void DeselectAllObjects(void);
void SelectObject(struct ScriptNodeRecord *this_node);
void SelectMoreObjects(int prevRow, int row);
void DeSelectObject(struct ScriptNodeRecord *this_node);
void DeSelectMoreObjects(int prevRow, int row);
void RedrawVisibleObjects(void);
void ShowInfoPanel(struct ScriptNodeRecord *this_node);
void ClearBetweenLines(void);
void ScriptGadgetsOn(void);
void ScriptSlider2On(void);
void ScriptGadgetsOff(void);
BOOL ReloadXapps(void);
void UnMergeStrings(char *sourceStr, char *destStr1, char *destStr2, int maxLen);
void CodeString(char *str, int maxLen);
void DeselectAllButThisOne(struct ScriptNodeRecord *this_node);
void ReallyDeselectAllButThisOne(struct ScriptNodeRecord *this_node);

/**** scripttalk ****/

void AnimFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void ArexxDosFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void BinaryFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void UserApplicFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void LabelFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void NopFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void MailFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void StartSerFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void EndSerFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void StartParFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void EndParFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void SoundFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void StartFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void EndFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void DurationFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void ProgramFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void TalkFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void PageFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void GotoFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void GlobalEventFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void TimeCodeFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void CounterFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void LocalEventFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void VarsFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void InputSettingsFunc(struct ParseRecord *PR, struct ScriptInfoRecord *SIR);

/**** scrman ****/

BOOL ShowSM(STRPTR scriptPath, STRPTR scriptName);

/**** scrsize ****/

BOOL MonitorScreenSize(BOOL *choseOptimize, int colorUsage, BOOL forceReopen);
int MakeDisplayList(UBYTE *monitorList, ULONG *IDS, UBYTE maxMonitors, STRPTR monName,
										int *listPos, ULONG currentID, BOOL doAll);
BOOL GetDisplayInfoText(struct Window *window, ULONG ID, int overScan, STRPTR monName);
void InterpretColors(int val, ULONG ID);

/**** settime ****/

void SetTime(void);
void Check_HMST_SER_Buttons(	struct Window *window, struct GadgetRecord *GR,
															BOOL *loop, BOOL *retVal,
															struct ScriptNodeRecord *this_node );
void Check_HMST_PAR_Buttons(	struct Window *window, struct GadgetRecord *GR,
															BOOL *loop, BOOL *retVal,
															struct ScriptNodeRecord *this_node );
void Check_HMST_PRG_Buttons(	struct Window *window, struct GadgetRecord *GR,
															BOOL *loop, BOOL *retVal,
															struct ScriptNodeRecord *this_node	);
void Check_TC_SER_Buttons(	struct Window *window, struct GadgetRecord *GR,
														BOOL *loop, BOOL *retVal,
														struct ScriptNodeRecord *this_node );
void PrintHorizText(struct Window *window, struct GadgetRecord *GR, int y, STRPTR str);
void SetTiming(STRPTR str1, STRPTR str2, UBYTE dayBits);
void RenderDelayArrows(struct Window *window, struct GadgetRecord *GR, int mode);
void MonitorArrows(struct Window *window, int mode, int ID, struct GadgetRecord *GR);
void FixDurationAndDays(struct ScriptNodeRecord *firstSNR);

/**** sliders ****/

void SetPaletteSlider(	struct Window *window, struct GadgetRecord *GR,
												int pos, int steps_factor);
void ProcessPaletteSlider(struct Window *window, struct GadgetRecord *GR,
													int *pos, int steps_factor, int gun, int well);
void DragPaletteSlider(	struct Window *window, struct GadgetRecord *GR,
												int w, int *pos, int steps_factor, int gun, int well);

/**** smallbuttons ****/

BOOL MonitorSmallButtons(struct ScriptInfoRecord *SIR, int top);
BOOL GetButton(	struct ScriptInfoRecord *SIR,
								struct ScriptNodeRecord *this_node, BOOL fast, int ghosted);
void PrintHHMMSST(int h, int m, int s, int t, struct GadgetRecord *GR,
									struct ScriptNodeRecord *this_node);
void DoPrintHHMMSST(struct GadgetRecord *GR,
										struct ScriptNodeRecord *this_node);
void PrintHHMMSSFF(	int h, int m, int s, int f, struct GadgetRecord *GR,
										struct ScriptNodeRecord *this_node, int startend);
void DoPrintHHMMSSFF(	struct GadgetRecord *GR,
											struct ScriptNodeRecord *this_node, int startend);
void PrintHHMMSST_par(int h, int m, int s, int f, struct GadgetRecord *GR,
											struct ScriptNodeRecord *this_node, int startend);
void DoPrintHHMMSST_par(	struct GadgetRecord *GR,
													struct ScriptNodeRecord *this_node, int startend);
void GetTimeCodeKeys(	USHORT key, struct ScriptInfoRecord *SIR,
											struct ScriptNodeRecord *this_node);

/**** specials ****/

BOOL ToggleSpecials(void);
int MonitorSpecials(void);
void DrawSpecialsPage(struct Window *window, int page, int wdw,
											UBYTE *windowOptsList[], UBYTE *backgroundOptsList[]);
void ProcessExtrasSelection(int page, int wdw,
														UBYTE *windowOptsList[], UBYTE *backgroundOptsList[],
														int line, BOOL noRedraw );
void RefreshAllWindows(	int page,
												WORD *prevX, WORD *prevY, WORD *prevW, WORD *prevH,
												WORD *newX, WORD *newY, WORD *newW, WORD *newH );
void CloseSpecialsScreen(void);
void DoSpecialsSettings(BOOL force);
void EnableDisableSpecialsList(	struct Window *window, int page, int wdw,
																struct ScrollRecord *SR, int sp_topEntry,
																UBYTE *windowOptsList[], UBYTE *backgroundOptsList[] );

/**** sprite ****/

void ChangeSpriteImage(int number);
void ClearSpriteImage(void);

/**** stread ****/

BOOL ReadScript(STRPTR path, STRPTR fileName, char **scriptCommands);
void freeScript(void);
void InitObjectArea(void);
void DrawDottedLines(void);
void GetNumObjects(void);
void GetMemSize(STRPTR appName, int *memSize);
void PerfFunc(struct GenericFuncs *FuncList, struct ParseRecord *PR, struct ScriptInfoRecord *SIR);
void createGlobalEntry(void);
void createInputSettings(void);
void createVarsEntry(void);

/**** stwrite ****/

BOOL WriteScript(	STRPTR path, STRPTR filename, struct ScriptInfoRecord *SIR,
									char **scriptCommands);
void FollowList(struct ScriptInfoRecord *SIR, char **scriptCommands,
								FILE *fp);
void doProgram(struct ScriptInfoRecord *SIR, struct ScriptNodeRecord *node, FILE *fp, int level, BOOL extraLF);
void WriteEmptyScript(struct ScriptInfoRecord *SIR, char **scriptCommands,
								FILE *fp);
BOOL CreateEmptyScript(void);
void doLocalEvents(	struct ScriptInfoRecord *SIR, struct ScriptNodeRecord *node,
										FILE *fp, int level, BOOL extraLF);
void doVars(	struct ScriptInfoRecord *SIR, struct ScriptNodeRecord *node,
							FILE *fp, int level, char **scriptCommands );

/**** tctweak ****/

void TimeCodeTweaker(void);
ULONG TC2Jiffies(BYTE HH, BYTE MM, BYTE SS, BYTE FF);
void Jiffies2TC(LONG jiffies, BYTE *HH, BYTE *MM, BYTE *SS, BYTE *FF);

/**** text ****/

void ImportAText(void);
BOOL ProcessTextEdit(int hitWdw);
void DrawSafeBox(struct RastPort *rp, WORD x, WORD y, WORD w, WORD h);
void MoreThanEnoughChip(struct EditWindow *ew, BYTE *status);

/**** textedit ****/

UWORD TextEdit(	struct EditWindow *ew );
void RealTimeUpdate(struct TEInfo *tei,
										struct EditWindow *srcEW, struct EditWindow *dstEW,
										BOOL *touchedList);
void UpdateWindowText(struct EditWindow *ew);

/**** timecode ****/

void Monitor_TimeCode(struct Window *window);
void switchTimeCode(int mode);

/**** times ****/

void dateStringtoDays(char *strPtr, int *totalDays);
void timeStringtoMinutesAndTicks(char *strPtr, int *minutes, int *ticks);
void timeStringtoDuration(char *strPtr, int *hh, int *mm, int *ss, int *tt);
void timeStringtoTimeCode(char *strPtr, int *hh, int *mm, int *ss, int *ff);
void secondsToDuration(int seconds, STRPTR str);
void datestampToTime(ULONG minute, ULONG tick, STRPTR str);
void datestampToDate(ULONG days, STRPTR str);
void SystemDate(int *day, int *month, int *year);
int DayOfWeek(int month, int year);
int MonthLength(int month, int year);
void createStartEndDay(LONG days, STRPTR str, int startend);
void createStartEndTime(LONG minutes, LONG ticks, STRPTR str);
void datestampToDMY(ULONG days, int *day, int *month, int *year);
void datestampToHMST(	ULONG minutes, ULONG ticks,
											int *hours, int *mins, int *secs, int *tenths);
void DayMonthYearToString(STRPTR str, int day, int month, int year);
void HoursMinsSecsTenthsToString(STRPTR str, int hours, int mins, int secs, int tenths);
void HoursMinsSecsFramesToString(STRPTR str, int hours, int mins, int secs, int frames);
void setStartEndCalender(struct Window *window, ULONG startDays, ULONG endDays);
void increaseDayName(struct Window *window, int ID1, int ID2);
void decreaseDayName(struct Window *window, int ID1, int ID2);
void increaseDay(struct Window *window, int ID1, int ID2);
void decreaseDay(struct Window *window, int ID1, int ID2);
void changeMonth(struct Window *window, int ID1, int ID2);
void CheckEnteredDate(struct Window *window, struct GadgetRecord *GR, int ID);
void CheckEnteredTime(struct GadgetRecord *GR);
void DurationStringToSeconds(STRPTR str, ULONG *seconds);
void SystemTime(int *hours, int *mins, int *secs);
void CheckEnteredTimeCode(int rate, struct GadgetRecord *GR);
int GetDayNumber(void);
void CheckHMST(int *h, int *m, int *s, int *t);
void CheckHMSF(int *h, int *m, int *s, int *f, int rate);
void CheckHMST_Cycle(int *h, int *m, int *s, int *t);
void CheckHMSF_Cycle(int *h, int *m, int *s, int *f, int rate);
void PutDateIntoIt(struct ScriptNodeRecord *this_node);

/**** translate ****/

BOOL TranslateApp(BOOL firstTime);
BOOL LoadTranslationFile(BOOL firstTime);
void UnLoadTranslationFile(void);

/**** transp ****/

void ClipBlitTransparent(	struct RastPort *drp,
													WORD x, WORD y, WORD w, WORD h, WORD offX,
													struct BitMap *maskBM );
void MakeMovePic(int lastHit);
void DrawMovePic(struct EditWindow *ew, WORD x, WORD y);

/**** rendezvous ****/

BOOL setupRendezVous(void);
void removeRendezVous(void);

void CheckStack(void);
void InitGadgets(void);

/**** varpath ****/

void FixedVarPath(void);

/**** varparse ****/

struct VarInfoRecord *AllocVIR(void);
void AddVIR(struct List *list, struct VarInfoRecord *VIR);
struct VarAssignRecord *AllocVAR(void);
int ParseDeclaration(	struct ScriptInfoRecord *SIR,
											STRPTR parseStr, STRPTR varName1, STRPTR varName2,
											int *value,
											STRPTR varContents, int *type, BOOL VarIsVarAllowed);
int ParseExpression(	struct ScriptInfoRecord *SIR,
											STRPTR parseStr,
											STRPTR varName1, STRPTR varName2, STRPTR varName3,
											int *value1, int *value2, 
											STRPTR varContents1, STRPTR varContents2,
											int *oper, int *type, STRPTR labelName, int *jump,
											STRPTR varName4, STRPTR varName5, STRPTR varName6,
											int *value3, int *value4, 
											STRPTR varContents3, STRPTR varContents4,
											int *oper2, int *type2 );
void ProcessParseError(int error);
void TrimVarName(STRPTR str);
void TrimVarNameV2(STRPTR str);
BOOL DeclToVIR(struct ScriptInfoRecord *SIR, STRPTR declStr, VIR *this_vir);
void VIRToDecl(VIR *this_vir, STRPTR declStr);
VIR *FindVIR(struct ScriptInfoRecord *SIR, STRPTR varName, int *num);
BOOL ExprToVAR(	struct ScriptInfoRecord *SIR, STRPTR declStr, VAR *this_var,
								STRPTR labelStr );
BOOL VARToExpr(struct ScriptInfoRecord *SIR, STRPTR declStr, VAR *this_var);
int DoDecla(STRPTR parseStr, int *oper, int *type,
						STRPTR token1, STRPTR token2, STRPTR token3);
int CheckVarType(	int type,
									STRPTR varName1, STRPTR varName2, STRPTR varName3,
									STRPTR varContents1, STRPTR varContents2,
									STRPTR token1, STRPTR token2, STRPTR token3, STRPTR token4, 
									int *value1, int *value2 );
BOOL FillInThisVarA(	VAR *this_var, int oper, int type, STRPTR varName1, STRPTR varName2, STRPTR varName3,
											STRPTR varContents1, STRPTR varContents2,
											int value1, int value2,
											struct ScriptInfoRecord *SIR );
BOOL FillInThisVarB(	VAR *this_var, int oper, int type, STRPTR varName1, STRPTR varName2, STRPTR varName3,
											STRPTR varContents1, STRPTR varContents2,
											int value1, int value2,
											struct ScriptInfoRecord *SIR );
void CreateDeclStrA(STRPTR declStr, VAR *this_var);
void CreateDeclStrB(STRPTR declStr, VAR *this_var);

/**** vars ****/

void MonitorVariables(struct ScriptNodeRecord *this_node);
void CreateDeclList(struct ScriptInfoRecord *SIR, UBYTE **list, int *numDecl,
										BOOL doAlloc);
void CreateExprList(struct ScriptInfoRecord *SIR,
										struct ScriptNodeRecord *SNR, UBYTE **list, int *numDecl);
void CheckDeclStr(struct ScriptInfoRecord *SIR, STRPTR str);
void CheckExprStr(struct ScriptInfoRecord *SIR, STRPTR str);
void ClearTheStrGad(struct Window *window, struct GadgetRecord *GR);
BOOL CheckThisVir(struct ScriptInfoRecord *SIR, STRPTR decl);
BOOL CheckThisVar(struct ScriptInfoRecord *SIR, STRPTR decl);
void RemoveAllVIRS(struct List *list);
void RemoveAllVARS(struct List *list);
void Copy_Vars_Info(struct ScriptNodeRecord *oldSNR, struct ScriptNodeRecord *newSNR);
int CountVIRS(struct List *list);
void UpdateAllForeignVIRS( struct ScriptInfoRecord *SIR );
void InsertLabelName(struct Window *window, struct GadgetRecord *GR);

/**** wdwtr ****/

void SetWdwTransitions(void);
void PrintWTInfo(struct Window *window, int wdw);
void RenderWdwTr(struct Window *window, BOOL laced, int wdw);
void DrawThickBorder(int wdw);
void RenderEffectIcon(struct RastPort *rp, WORD srcX, WORD srcY,
											WORD dstX, WORD dstY, WORD dstW, WORD dstH);

/**** windef1 ****/

int OpenSmallPalette(int well, int mode, BOOL noColorZero);
void DrawCross(	struct Window *window, int well,
								int w, int h, int numRows, int numCols,
								int offsetX, int offsetY);
int DrawPaletteBox(	struct Window *window, int mouseX, int mouseY,
										int w, int h, int numRows, int numCols,
										int offsetX, int offsetY);
void RenderWindowIandB(struct RastPort *rp, struct EditWindow *ew);
void RenderWindowInterior(struct RastPort *rp, struct EditWindow *ew);
void RenderWindowBorders(struct RastPort *rp, struct EditWindow *ew);
void ToggleScaling(void);
void SetDrawMode(int mode);
void InvertLockedState(void);
void PaintButton(struct Window *window, struct GadgetRecord *GR, int pen);

/**** windef2 ****/

BOOL ToggleWinDef(void);
int MonitorWinDef(void);
void CloseWinDefScreen(void);
void DoWinDefSettings(void);
void EnableDisableWinDefList(struct Window *window, int wdw);
void RenderWinDefButtons(struct Window *window, int wdw);
void InterpretWinDefSettings(	struct EditWindow *localEW, struct EditSupport *localES,
															int button, BOOL redraw );
void DoPaintedButton(int well,int pen);
void DoWinDefGhosting(struct EditWindow *ew);
BOOL FirstTimeCase(struct Window *window, struct EditWindow *localEW);
void SetTheMargins(void);

/**** windows ****/

void InitWindowLists(void);
BOOL CreateEditWindow(void);
int SearchEmptyWindow(void);
void DrawHandles(WORD x1, WORD y1, WORD x2, WORD y2);
void DrawAllHandles(BOOL Flag);
void DrawMarquee(WORD start_x, WORD start_y, WORD *end_x, WORD *end_y);
void DrawMarqueeBox(struct RastPort *rp, WORD x1, WORD y1, WORD x2, WORD y2);
void DrawFatMarqueeBox(struct RastPort *rp, WORD x1, WORD y1, WORD x2, WORD y2);
void DrawEditWindow(struct EditWindow *ew, struct EditSupport *es);
int OpenEditWindow(int slot, WORD start_x, WORD start_y, WORD end_x, WORD end_y);
void CloseEditWindow(	struct EditWindow *ew, struct EditSupport *es);
void CloseAllEditWindows(void);
void CloseAllClipboardWindows(void);
void CloseAllUndoWindows(void);
int GetNewDrawSeqNum(void);
int GetNewDrawSeqNum_2(struct EditWindow *ew);
int DetermineClickEvent(int *hitWdwNr, BOOL drawHandles);
void SizeEditWindow(int action, int lastHit);
void SizeActiveEditWindow(int action, int lastHit, int mouseX, int mouseY);
void DragEditWindow(int action, int lastHit);
void MoveEditWindow(int mouseX, int mouseY, WORD DiffX, WORD DiffY,
										struct EditWindow *ew, struct EditSupport *es);
struct Window *GetActiEditWin(void);
void ResizeEditWindow(int wdwNr,
											WORD prevX, WORD prevY, WORD prevW, WORD prevH,
											WORD newX, WORD newY, WORD newW, WORD newH, BOOL redraw);
void RepositionEditWindow(int wdwNr,
													WORD prevX, WORD prevY, WORD prevW, WORD prevH,
													WORD newX, WORD newY, WORD newW, WORD newH,
													BOOL redraw);
void WaitForChangeCompletion(void);
void RemovePicFromWindow(struct EditSupport *es, struct BitMap *bm);
void RemovePic24FromWindow(struct EditSupport *es, struct BitMap24 *bm);
/*
void RemovePictureFromWindow(struct EditSupport *es, BOOL freeAll);
void RemoveColorMapFromWindow(struct EditSupport *es);
void RemoveTEIFromWindow(struct EditWindow *ew);
*/
void DrawAllWindows(void);
int SortEditWindowLists(int slot);
void DoWdwMove(void);
void DoWdwTab(void);
void ChopMouse(SHORT *MouseX, SHORT *MouseY);

/**** windows2 ****/

int NumActiveEditWindows(void);
int NumEditWindows(void);
int FirstActiveEditWindow(void);
void CutActiveWindows(void);
void CopyActiveWindows(void);
void CopyAWindow(int wdw, int to_wdw, BOOL closeClipboard);
void PasteActiveWindows(WORD x, WORD y);
void SelectAllWindows(void);
void ClearActiveWindows(void);
void UndoEditWindowModification(void);
void PasteUndoWindows(void);
void ValidateBoundingBox(WORD *x, WORD *y, WORD *w, WORD *h);
void GetWindowVars(struct EditWindow *ew, WORD *x1, WORD *y1, WORD *x2, WORD *y2);
void GetWindowVarsShadow(struct EditWindow *ew, WORD *x1, WORD *y1, WORD *x2, WORD *y2);
void AddTEI(struct EditWindow *NEWew, struct EditWindow *OLDew);
void AddCrawl(struct EditWindow *NEWew, struct EditWindow *OLDew);
void CorrectEW(struct EditWindow *ew);
void CheckXandY(ULONG bitValue, struct EditWindow *ew,
								WORD *x1, WORD *y1, WORD *x2, WORD *y2);

/**** windraw2 ****/

void RenderPhotoAndOrDoTransp(struct EditWindow *ew, struct EditSupport *es,
															struct RastPort *destRP);
void CreateTranspWdw(	struct EditWindow *ew, struct EditSupport *es,
											struct RastPort *destRP, WORD toX, WORD toY );
void doTrans(	struct EditWindow *ew, struct EditSupport *es,
							WORD x, WORD y, WORD w, WORD h, WORD offX,
							struct RastPort *destRP, WORD toX, WORD toY );
BOOL BoxInsideBox(struct EditWindow *ew1, struct EditWindow *ew2);
BOOL PointInsideBox(int x, int y, struct EditWindow *ew);
BOOL AllocateSharedBM(WORD w, WORD h, WORD d);
void FreeSharedBM(void);
void PutSizePicture(struct EditWindow *ew, struct EditSupport *es,
										BOOL forClip, WORD *clipW, WORD *clipH,
										struct RastPort *destRP, WORD toX, WORD toY );
BOOL PutMovePicture(struct EditWindow *ew, struct EditSupport *es,
										WORD *destX, WORD *destY, WORD *destW, WORD *destH,
										BOOL forClip,
										WORD *offX, WORD *clipX, WORD *clipY, WORD *clipW, WORD *clipH,
										struct RastPort *destRP, WORD toX, WORD toY);
BOOL EnoughFastMem(WORD d, WORD w, WORD h);
void RemapAllPics(void);
void RemapAllPicsAndBackground(void);
BOOL NoNeedToScaleRemap(struct EditSupport *es, WORD w, WORD h);

/**** windraw4 ****/

void MovePicture(int wdwNr);
void unclipWindow(struct Window *win);
struct Region *clipWindow(struct Window *win, LONG minX, LONG minY, LONG maxX, LONG maxY);

/**** windraw5 ****/

void RedrawAllOverlapWdw(	WORD prevX, WORD prevY, WORD prevW, WORD prevH,
													WORD newX, WORD newY, WORD newW, WORD newH,
													int wdw, BOOL redrawWdw, BOOL removeWdw );
void RedrawAllOverlapWdwEasy( int wdw, BOOL redrawWdw, BOOL removeWdw );
void RedrawAllOverlapWdwList(	BOOL *list );
void PrepareRedrawAll(	WORD *prevX, WORD *prevY, WORD *prevW, WORD *prevH,
												WORD *newX, WORD *newY, WORD *newW, WORD *newH,
												BOOL *list );
void RedrawAllOverlapWdwListPrev(	WORD *prevX, WORD *prevY, WORD *prevW, WORD *prevH,
																	WORD *newX, WORD *newY, WORD *newW, WORD *newH,
																	BOOL *list );
void RestoreBack(	struct EditWindow *ew, struct EditSupport *es );

/**** winreq ****/

void SetStandardColors(struct Window *window);
void ResetStandardColors(struct Window *window);
void SetAllStandardColors(void);
void SetAllColorsToZero(void);
void ScaleGadgetList(struct Screen *screen, struct GadgetRecord *GR);

/**** xappinfo ****/

void ShowXappInfo(struct ScriptNodeRecord *this_node);
BOOL GetToolTypeStr(STRPTR path, STRPTR tag, STRPTR answer);

/**** xload ****/

BOOL InitXaPP(char *, struct ScriptNodeRecord *, BOOL);

/******** E O F ********/
