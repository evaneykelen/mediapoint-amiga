/**** errors ****/

void __saveds __asm LIBUA_WarnUser(	register __d0 int EC );
BOOL __saveds __asm LIBUA_OpenGenericWindow(register __a0	struct Window *window,
																						register __d0 BOOL ok,
																						register __d1 BOOL cancel,
																						register __a1 STRPTR okText,
																						register __a2 STRPTR cancelText,
																						register __d2 int icon,
																						register __a3 STRPTR messageText,
																						register __d3 BOOL key,
																						register __a5 struct GadgetRecord *GR);
struct Window * __saveds __asm LIBUA_OpenMessagePanel(register __a0	struct Window *window,
																											register __a1 STRPTR messageText);
void __saveds __asm LIBUA_CloseMessagePanel(register __a0 struct Window *window);
void __saveds __asm LIBUA_PutImageInRastPort(	register __a0 struct BitMap *gfxBitMap,
																							register __d0 WORD srcX,
																							register __d1 WORD srcY,
																							register __d2 WORD srcW,
																							register __a1 struct RastPort *dstRp,
																							register __d3 WORD dstX,
																							register __d4 WORD dstY,
																							register __d5 WORD dstW,
																							register __d6 WORD dstH );

/**** events ****/

void __saveds __asm LIBUA_doStandardWait(	register __a0 struct Window *waitWindow,
																					register __a1 struct EventData *CED);
ULONG __saveds __asm LIBUA_doStandardWaitExtra(	register __a0 struct Window *waitWindow,
																								register __a1 struct EventData *CED,
																								register __d0 ULONG extraFlags );
BOOL __saveds __asm LIBUA_CheckIfDragged(	register __a0 struct Window *window,
																					register __a1 struct EventData *CED );
void HandleIDCMP(struct EventData *CED, struct MsgPort *port);
BOOL MyDoubleClick(	ULONG prev_Seconds, ULONG prev_Micros,
										struct EventData *CED );
int AbsWORD(int a, int b);
void DrawMarqueeBox(struct RastPort *rp, WORD x1, WORD y1, WORD x2, WORD y2);
void CheckIfDepthWasClicked(void);

/**** fonts ****/

BOOL __saveds __asm LIBUA_ScanFontsDir(register __a0 struct FER *FER);
void __saveds __asm LIBUA_freeScannedFonts(void);
struct TextFont * __saveds __asm LIBUA_Monitor_FontSelection(
																			register __a0 struct Window *window,
																			register __a1 struct FER *FER,
																			register __a2 struct GadgetRecord *GR,
																			register __a3 UWORD *mypattern1);
struct Window * __saveds __asm LIBUA_OpenFontListWindow(register __a0 struct Window *onWindow,
																												register __a1 struct FER *FER,
																												register __a2 struct GadgetRecord *GR,
																												register __a3 UWORD *mypattern1);
void __saveds __asm LIBUA_CloseFontListWindow(register __a0 struct Window *window);
void doFontSliders(	struct Window *window, struct EventData *CED,
										struct GadgetRecord *GR, struct FER *FER );
void DoFontScrolling(	struct Window *window, int numEntries,
											int numDisplay, int *top, struct GadgetRecord *GR,
											struct Gadget *g, BOOL drawAlways, int which,
											struct FER *FER);
void HiliteFontLine(struct Window *window, struct GadgetRecord *GR, int line);
BOOL SelectFontLine(struct Window *window, int mode, struct FER *FER,
										UWORD *mypattern1, struct GadgetRecord *GR,
										struct EventData *CED);
struct TextFont *fontSample(struct Window *window, struct GadgetRecord *GR,
														struct FER *FER, struct TextFont *TheFont,
														BOOL showSample );

/**** fr ****/

BOOL __saveds __asm LIBUA_OpenAFile(register __a0 struct Window *window,
																		register __a1 struct FileReqRecord *FRR,
																		register __a2 UWORD *mypattern1);
BOOL __saveds __asm LIBUA_SaveAFile(register __a0 struct Window *window,
																		register __a1 struct FileReqRecord *FRR,
																		register __a2 UWORD *mypattern1);
void InitPropInfo(struct PropInfo *PI, struct Image *IM);

/**** gadgsliders ****/

void __saveds __asm LIBUA_SetSliderGadg(	register __a0 struct Window *window,
																					register __a1 struct GadgetRecord *GR,
																					register __d0 int pos,
																					register __d1 int units,
																					register __a3 struct GadgetRecord *showGR,
																					register __d2 int addedVal);
void __saveds __asm LIBUA_ProcessSliderGadg(	register __a0 struct Window *window,
																							register __a1 struct GadgetRecord *GR,
																							register __a2 int *pos,
																							register __d0 int units,
																							register __a3 struct GadgetRecord *showGR,
																							register __d1 struct EventData *CED,
																							register __a5 void (*hookfunc)(int *),
																							register __d2 int *data,
																							register __d3 int which,
																							register __d4 int addedVal );
void DragSliderGadg(struct Window *window, struct GadgetRecord *GR,
										int knobWidth, int *pos, int units, struct GadgetRecord *showGR,
										BOOL new,
										void (*hookfunc)(int *), int *data, int which, int addedVal);

/**** init ****/

BOOL __saveds __asm LIBUA_Open_ML_Lib(void);
void __saveds __asm LIBUA_Close_ML_Lib(void);

/**** medialinklib ****/

void __saveds __asm LIBUA_InitStruct(register __a0 struct UserApplicInfo *UAI);
void __saveds __asm LIBUA_OpenScreen(register __a0 struct UserApplicInfo *UAI);
void __saveds __asm LIBUA_OpenWindow(register __a0 struct UserApplicInfo *UAI);
void __saveds __asm LIBUA_CloseScreen(register __a0 struct UserApplicInfo *UAI);
void __saveds __asm LIBUA_CloseWindow(register __a0 struct UserApplicInfo *UAI);
BOOL __saveds __asm LIBUA_HostScreenPresent(register __a0 struct UserApplicInfo *UAI);
void __saveds __asm LIBUA_DrawGadget(	register __a0 struct Window *window,
																			register __a1 struct GadgetRecord *GR);
void __saveds __asm LIBUA_DrawGadgetList(	register __a0 struct Window *window,
																					register __a1 struct GadgetRecord *GR);
int __saveds __asm LIBUA_CheckGadget(	register __a0 struct Window *window,
																			register __a1 struct GadgetRecord *FirstGR,
																			register __a2 struct GadgetRecord *GR,
																			register __a3 struct EventData *ED);
int __saveds __asm LIBUA_CheckGadgetList(	register __a0 struct Window *window,
																					register __a1 struct GadgetRecord *FirstGR,
																					register __a2 struct EventData *ED);
int __saveds __asm LIBUA_ProcessCycleGadget(	register __a0 struct Window *window,
																							register __a1 struct GadgetRecord *GR,
																							register __a2 struct EventData *ED);
void __saveds __asm LIBUA_HiliteButton(	register __a0 struct Window *window,
																				register __a1 struct GadgetRecord *GR);
void __saveds __asm LIBUA_InvertButton(	register __a0 struct Window *window,
																				register __a1 struct GadgetRecord *GR);
void __saveds __asm LIBUA_HiliteRadioButton(register __a0 struct Window *window,
																						register __a1 struct GadgetRecord *GR);
void __saveds __asm LIBUA_InvertRadioButton(register __a0 struct Window *window,
																						register __a1 struct GadgetRecord *GR);
void __saveds __asm LIBUA_InvertCheckButton(register __a0 struct Window *window,
																						register __a1 struct GadgetRecord *GR);
void __saveds __asm LIBUA_ClearButton(register __a0 struct Window *window,
																			register __a1 struct GadgetRecord *GR,
																			register __d0 int pen);
void __saveds __asm LIBUA_ClearCycleButton(	register __a0 struct Window *window,
																						register __a1 struct GadgetRecord *GR,
																						register __d0 int pen);
void __saveds __asm LIBUA_SetValToCycleGadgetVal(	register __a0 struct GadgetRecord *GR,
																									register __a1 int *value);
void __saveds __asm LIBUA_SetCycleGadgetToVal(register __a0 struct Window *window,
																							register __a1 struct GadgetRecord *GR,
																							register __d0 int value);
int __saveds __asm LIBUA_GetRadioGadgetVal(register __a0 struct GadgetRecord *GR);
void __saveds __asm LIBUA_DisableButton(register __a0 struct Window *window,
																				register __a1 struct GadgetRecord *GR,
																				register __a2 UWORD *pattern);
void __saveds __asm LIBUA_EnableButton(	register __a0 struct Window *window,
																				register __a1 struct GadgetRecord *GR);
void __saveds __asm LIBUA_EnableButtonQuiet(register __a0 struct GadgetRecord *GR);
void __saveds __asm LIBUA_DisableButtonQuiet(register __a0 struct GadgetRecord *GR);
void __saveds __asm LIBUA_AdjustGadgetCoords(	register __a0 struct GadgetRecord *GR,
																							register __d0 int xoffset,
																							register __d1 int yoffset);
void __saveds __asm LIBUA_DisableRangeOfButtons(register __a0 struct Window *window,
																								register __a1 struct GadgetRecord *GR,
																								register __d0 int num,
																								register __a2 UWORD *pattern);
void __saveds __asm LIBUA_EnableRangeOfButtons(	register __a0 struct Window *window,
																								register __a1 struct GadgetRecord *GR,
																								register __d0 int num);
void __saveds __asm LIBUA_SetStringGadgetToString(register __a0 struct Window *window,
																									register __a1 struct GadgetRecord *GR,
																									register __a2 STRPTR str);
void __saveds __asm LIBUA_SetStringToGadgetString(register __a0 struct GadgetRecord *GR,
																									register __a1 STRPTR str);
void __saveds __asm LIBUA_WipeStringGadget(register __a0 struct GadgetRecord *GR);
BOOL __saveds __asm LIBUA_IsGadgetDisabled(register __a0 struct GadgetRecord *GR);
void __saveds __asm LIBUA_DrawTwoColorBox(register __a0 struct Window *window,
																					register __a1 struct GadgetRecord *GR);
void __saveds __asm LIBUA_DrawText(	register __a0 struct Window *window,
																		register __a1 struct GadgetRecord *GR,
																		register __a2 STRPTR str);
void __saveds __asm LIBUA_GetKeys(register __a0 USHORT *raw,
																	register __a1 char *key,
																	register __a2 int *numkeys,
																	register __a3 struct EventData *ED);
void __saveds __asm LIBUA_SetStringGadgetToVal(	register __a0 struct Window *window,
																								register __a1 struct GadgetRecord *GR,
																								register __d0 int value);
void __saveds __asm LIBUA_SetValToStringGadgetVal(register __a0 struct GadgetRecord *GR,
																									register __a1 int *value);
BOOL __saveds __asm LIBUA_ProcessStringGadget(register __a0 struct Window *window,
																							register __a1 struct GadgetRecord *FirstGR,
																							register __a2 struct GadgetRecord *GR,
																							register __a3 struct EventData *ED);
void __saveds __asm LIBUA_DrawStringText(	register __a0 struct Window *window,
																					register __a1 struct GadgetRecord *GR,
																					register __a2 STRPTR str);
void __saveds __asm LIBUA_DrawSpecialGadgetText(register __a0 struct Window *window,
																								register __a1 struct GadgetRecord *GR,
																								register __a2 STRPTR str,
																								register __d0 int mode);
int __saveds __asm LIBUA_GetDepthOfWindowScreen(register __a0 struct Window *window);
void __saveds __asm LIBUA_DoubleGadgetDimensions(register __a0 struct GadgetRecord *GR);
void __saveds __asm LIBUA_HalveGadgetDimensions(register __a0 struct GadgetRecord *GR);
BOOL __saveds __asm LIBUA_IsWindowOnLacedScreen(register __a0 struct Window *window);
BOOL __saveds __asm LIBUA_IsUAScreenLaced(register __a0 struct UserApplicInfo *UAI);
int __saveds __asm LIBUA_FindString(register __a0 STRPTR OrgString,
																		register __a1 STRPTR CheckString);
void __saveds __asm LIBUA_ValidatePath(register __a0 STRPTR path);
void __saveds __asm LIBUA_TurnAssignIntoDir(register __a0 STRPTR ass);
void __saveds __asm LIBUA_MakeFullPath(	register __a0 STRPTR path,
																				register __a1 STRPTR name,
																				register __a2 STRPTR answer);
void __saveds __asm LIBUA_SplitFullPath(	register __a0 STRPTR fullPath,
																					register __a1 STRPTR path,
																					register __a2 STRPTR filename	);
void __saveds __asm LIBUA_DrawSliderNotches(	register __a0	struct Window *window,
																							register __a1 struct GadgetRecord *GR,
																							register __d0 int type,
																							register __d1 int num,
																							register __d2 int pen  );
void __saveds __asm LIBUA_DrawDefaultButton(	register __a0	struct Window *window,
																							register __a1 struct GadgetRecord *GR );
void __saveds __asm LIBUA_TranslateGR(	register __a0 struct GadgetRecord *GR,
																				register __a1 UBYTE **msgs );
void __saveds __asm LIBUA_PutCapsPort(register __a0 struct MsgPort *port);
void __saveds __asm LIBUA_CloseWindowSafely(register __a0 struct Window *window);
int __saveds __asm LIBUA_GetRightPen(	register __a0 struct Window *window,
																			register __a1 struct GadgetRecord *GR,
																			register __d0 long pen );
void __saveds __asm LIBUA_DrawGadgetListRange(register __a0 struct Window *window,
																							register __a1 struct GadgetRecord *GR,
																							register __d0 int first,
																							register __d1 int last );
void __saveds __asm LIBUA_DrawComboBox(	register __a0 struct Window *window,
																				register __a1 struct GadgetRecord *GR );

void StripIntuiMessages(struct MsgPort *mp, struct Window *window);
void UA_DrawButton(struct Window *window, struct GadgetRecord *GR);
void UA_DrawCycle(struct Window *window, struct GadgetRecord *GR);
void UA_DrawRadio(struct Window *window, struct GadgetRecord *GR);
void UA_DrawCheck(struct Window *window, struct GadgetRecord *GR);
void UA_DrawSimpleBox(struct Window *window, struct GadgetRecord *GR);
void UA_DrawABorder(struct Window *window, struct GadgetRecord *GR);
void UA_DrawABox(struct Window *window, int pen, int x1, int y1, int x2, int y2);
void UA_DrawCycleText(struct Window *window, struct GadgetRecord *GR, STRPTR str);
int UA_GetGadgetID(	struct Window *window, struct GadgetRecord *FirstGR,
										struct GadgetRecord *GR, STRPTR str);
void UA_ClearStringGadget(struct Window *window,
													struct GadgetRecord *GR, int pen);
LONG UA_DeadKeyConvert(	UBYTE *kbuffer, LONG kbsize, struct KeyMap *kmap,
												struct EventData *ED);
void UA_ZeroString(STRPTR str, int len);
int UA_CalcCursorPosFromMousePos(	struct Window *window,
																	struct GadgetRecord *FirstGR,
																	struct GadgetRecord *GR,
																	STRPTR str, int mode,
																	struct EventData *ED);
void UA_RenderCursor(	struct Window *window, struct GadgetRecord *GR,
											STRPTR str, int pos);
void UA_PrtStrTxt(struct Window *window, struct GadgetRecord *GR,
									STRPTR str, int pos);
void UA_DrawString(struct Window *window, struct GadgetRecord *GR);
void UA_DrawALine(struct Window *window, struct GadgetRecord *GR);
void my_SetAPen(struct Window *window, long pen);
int my_getpath(BPTR lock, char *path);
int ChoosePen(struct Window *window, struct GadgetRecord *GR, int pen);
int ChooseTextPen(struct Window *window, struct GadgetRecord *GR);
void MyRectFill(struct RastPort *rp, WORD x1, WORD y1, WORD x2, WORD y2);

/**** newlist ****/

void __saveds __asm LIBUA_PrintNewList(	register __a0 struct ScrollRecord *SR,
																				register __d0 int top,
																				register __a1 TEXT *ptrlist[],
																				register __d1 BOOL updateButtons );
void __saveds __asm LIBUA_ScrollNewList(	register __a0 struct ScrollRecord *SR,
																					register __a1 int *top,
																					register __a2 struct Gadget *g,
																					register __a3 TEXT *ptrlist[],
																					register __a5 struct EventData *oldCED );
int __saveds __asm LIBUA_SelectNewListLine(	register __a0 struct ScrollRecord *SR,
																						register __d0 int top,
																						register __a1 struct EventData *CED );
void PrintNewListLine(struct Window *window, struct GadgetRecord *GR,
											int x1, int y1, int x2, int y2,
											STRPTR oristr, BOOL updateButtons);

/**** popup ****/

void __saveds __asm LIBUA_Monitor_PopUp(register __a0 struct PopUpRecord *PUR);
BOOL __saveds __asm LIBUA_OpenPopUpWindow(register __a0 struct Window *onWindow,
																					register __a1 struct GadgetRecord *onGR,
																					register __a2 struct PopUpRecord *PUR);
void __saveds __asm LIBUA_ClosePopUpWindow(register __a0 struct PopUpRecord *PUR);
void __saveds __asm LIBUA_PrintPopUpChoice(	register __a0 struct Window *onWindow,
																						register __a1 struct GadgetRecord *onGR,
																						register __a2 struct PopUpRecord *PUR);
void __saveds __asm LIBUA_PrintPopUpChoice2(	register __a0 struct Window *onWindow,
																							register __a1 struct GadgetRecord *onGR,
																							register __a2 char *txt );

/**** rexx1 ****/

BOOL __saveds __asm LIBUA_IssueRexxCmd_V1(register __a0 STRPTR appName,
																					register __a1 STRPTR command,
																					register __a2 STRPTR result,
																					register __d0 BOOL resultWanted,
																					register __d1 int max );

/**** rexx2 ****/

BOOL __saveds __asm LIBUA_IssueRexxCmd_V2(register __a0 STRPTR appName,
																					register __a1 STRPTR port_name,
																					register __a2 STRPTR command,
																					register __a3 STRPTR resultStr,
																					register __a5 ULONG *RC);

/**** sprites ****/

void __saveds __asm LIBUA_SetSpritePtrs(	register __a0 UWORD *waitPointer,
																					register __a1 UWORD *colorPicker,
																					register __a2 UWORD *toSprite,
																					register __a3 UWORD *hand	);
void __saveds __asm LIBUA_SetSprite(	register __a0 struct Window *window,
																			register __d0 BYTE which	);

/**** stdlist ****/

void __saveds __asm LIBUA_PrintStandardList(register __a0 struct ScrollRecord *SR,
																						register __d1 int top,
																						register __a1 TEXT *ptrlist[]);
void __saveds __asm LIBUA_ScrollStandardList(	register __a0 struct ScrollRecord *SR,
																							register __a1 int *top,
																							register __a2 struct Gadget *g,
																							register __a3 TEXT *ptrlist[],
																							register __a5 struct EventData *oldCED );
int __saveds __asm LIBUA_SelectStandardListLine(register __a0 struct ScrollRecord *SR,
																								register __d0 int top,
																								register __d1 BOOL multiple,
																								register __a1 struct EventData *CED,
																								register __d2 BOOL deselect,
																								register __d3 BOOL select );
void __saveds __asm LIBUA_ShortenString(register __a0 struct RastPort *rp,
																				register __a1 STRPTR str,
																				register __d0 int numPixels);
void __saveds __asm LIBUA_ShortenStringFront(	register __a0 struct RastPort *rp,
																							register __a1 STRPTR str,
																							register __d0 int numPixels);
void __saveds __asm LIBUA_PrintInBox(	register __a0 struct Window *window,
																			register __a1 struct GadgetRecord *GR,
																			register __d0 int x1,
																			register __d1 int y1,
																			register __d2 int x2,
																			register __d3 int y2,
																			register __a2 STRPTR oristr,
																			register __d4 int mode);
void __saveds __asm LIBUA_printSeveralLines(register __a0 struct Window *window,
																						register __a1 struct GadgetRecord *GR,
																						register __d0 int x,
																						register __d1 int y,
																						register __d2 int width,
																						register __d3 int height,
																						register __a2 STRPTR str);
void __saveds __asm LIBUA_SwitchMouseMoveOn(register __a0 struct Window *window);
void __saveds __asm LIBUA_SwitchMouseMoveOff(register __a0 struct Window *window);
void __saveds __asm LIBUA_GetPropSlider(register __a0	struct Window *window,
																				register __a1	struct Gadget *g,
																				register __d0	int numEntries,
																				register __d1	int numDisplay,
																				register __a2	int *top);
void __saveds __asm LIBUA_GetHorizPropSlider(	register __a0	struct Window *window,
																							register __a1	struct Gadget *g,
																							register __d0	int numEntries,
																							register __d1	int numDisplay,
																							register __a2	int *top);
void __saveds __asm LIBUA_InitPropSlider(	register __a0	struct Window *window,
																					register __a1	struct Gadget *g,
																					register __d0	ULONG numEntries,
																					register __d1	ULONG numDisplay,
																					register __d2	LONG topEntry);
void __saveds __asm LIBUA_InitHorizPropSlider(register __a0	struct Window *window,
																							register __a1	struct Gadget *g,
																							register __d0	ULONG numEntries,
																							register __d1	ULONG numDisplay,
																							register __d2	LONG topEntry);
void __saveds __asm LIBUA_SetPropSlider(register __a0	struct Window *window,
																				register __a1	struct Gadget *g,
																				register __d0	ULONG numEntries,
																				register __d1	ULONG numDisplay,
																				register __d2	LONG topEntry);
void __saveds __asm LIBUA_SetHorizPropSlider(	register __a0 struct Window *window,
																							register __a1 struct Gadget *g,
																							register __d0	ULONG numEntries,
																							register __d1	ULONG numDisplay,
																							register __d2	LONG topEntry);
void __saveds __asm LIBUA_SwitchFlagsOn(	register __a0 struct Window *window,
																					register __d0 ULONG flags );
void __saveds __asm LIBUA_SwitchFlagsOff(	register __a0 struct Window *window,
																					register __d0 ULONG flags );
void PrintStdListLine(struct Window *window, struct GadgetRecord *GR,
											int x1, int y1, int x2, int y2,
											STRPTR oristr);

/**** strgad ****/

void AddStrGad(	struct Window *window, struct GadgetRecord *GR, UBYTE *txt,
								UBYTE *undo, int max, struct StringInfo *SI, struct Gadget *G,
								struct StringExtend *SE);
void RemoveStrGad(struct Window *window, struct Gadget *G);

/**** vectors ****/

void __saveds __asm LIBUA_DrawVector(	register __a0 struct Window *window,
																			register __a1 struct VectorRecord *VR);
void __saveds __asm LIBUA_DrawVectorList(	register __a0 struct Window *window,
																					register __a1 struct VectorRecord *VR);
void __saveds __asm LIBUA_DoubleVectorDimensions(register __a0 struct VectorRecord *VR);
void __saveds __asm LIBUA_HalveVectorDimensions(register __a0 struct VectorRecord *VR);
void DrawVectorLine(struct Window *window, struct VectorRecord *VR);
void DrawVectorArea(struct Window *window, struct VectorRecord *VR);

/**** winreq ****/

struct Window * __saveds __asm LIBUA_OpenRequesterWindow(	register __a0	struct Window *onWindow,
																													register __a1 struct GadgetRecord *GR,
																													register __d0 BYTE palette);
void __saveds __asm LIBUA_CloseRequesterWindow(	register __a0 struct Window *window,
																								register __d0 BYTE palette );
int __saveds __asm LIBUA_GetNumberOfColorsInScreen(	register __d0 ULONG viewmodes,
																										register __d1 int depth,
																										register __d2 BOOL AA_available );
LONG __saveds __asm LIBUA_MyFindColor(register __a0	struct ColorMap *cm,
																			register __d0 ULONG r,
																			register __d1 ULONG g,
																			register __d2 ULONG b,
																			register __d3 LONG maxpen,
																			register __d4	int count,
																			register __d5 BOOL colorZeroAlso );
void __saveds __asm LIBUA_SetMenuColors(register __a0 struct RendezVousRecord *rvrec,
																				register __a1 struct Window *window);
void __saveds __asm LIBUA_ResetMenuColors(register __a0 struct RendezVousRecord *rvrec,
																					register __a1 struct Window *window);
void ScaleGadgetList(struct Screen *screen, struct GadgetRecord *GR);
ULONG GetColorCM32(struct ColorMap *cm, int well);
void SetVPToComponents(	struct ViewPort *vp, int well, int r, int g, int b,
												struct RendezVousRecord *rvrec);
void GetColorComponents(struct ColorMap *cm, int well, int *r, int *g, int *b,
												struct RendezVousRecord *rvrec);
void SetColorComponents(struct ColorMap *cm, int well, int r, int g, int b,
												struct RendezVousRecord *rvrec);
void SearchBestGUIColors(	struct RendezVousRecord *rvrec, UBYTE *SW_pens,
													struct Window *window);
void TurnSmallIntoLarge(ULONG *r, ULONG *g, ULONG *b);

/******** E O F ********/
