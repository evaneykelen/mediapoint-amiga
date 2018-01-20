/**** errors ****/

void UA_WarnUser(	int EC );
BOOL UA_OpenGenericWindow(struct Window *window,
													BOOL ok,
													BOOL cancel,
													STRPTR okText,
													STRPTR cancelText,
													int icon,
													STRPTR messageText,
													BOOL key,
													struct GadgetRecord *GR);
struct Window *UA_OpenMessagePanel(struct Window *window, STRPTR messageText);
void UA_CloseMessagePanel(struct Window *window);
void UA_PutImageInRastPort(	struct BitMap *gfxBitMap,
														WORD srcX, WORD srcY, WORD srcW, struct RastPort *dstRp,
														WORD dstX, WORD dstY, WORD dstW, WORD dstH );

/**** events ****/

void UA_doStandardWait(	struct Window *waitWindow,
												struct EventData *CED);
ULONG UA_doStandardWaitExtra(	struct Window *waitWindow,
															struct EventData *CED,
															ULONG extraFlags );
BOOL UA_CheckIfDragged(struct Window *window, struct EventData *CED);

/**** fonts ****/

BOOL UA_ScanFontsDir(struct FER *FER);
void UA_freeScannedFonts(void);
struct TextFont *UA_Monitor_FontSelection(struct Window *window,
																					struct FER *FER,
																					struct GadgetRecord *GR,
																					UWORD *mypattern1);
struct Window *UA_OpenFontListWindow(	struct Window *onWindow,
																			struct FER *FER,
																			struct GadgetRecord *GR,
																			UWORD *mypattern1);
void UA_CloseFontListWindow(struct Window *window);

/**** fr ****/

BOOL UA_OpenAFile(struct Window *window,
									struct FileReqRecord *FRR,
									UWORD *mypattern1);
BOOL UA_SaveAFile(struct Window *window,
									struct FileReqRecord *FRR,
									UWORD *mypattern1);

/**** gadgsliders ****/

void UA_SetSliderGadg(struct Window *window,
											struct GadgetRecord *GR,
											int pos,
											int units,
											struct GadgetRecord *showGR,
											int addedVal);
void UA_ProcessSliderGadg(	struct Window *window,
														struct GadgetRecord *GR,
														int *pos,
														int units,
														struct GadgetRecord *showGR,
														struct EventData *CED,
														void (*hookfunc)(int *),
														int *data,
														int which,
														int addedVal );

/**** init ****/

BOOL UA_Open_ML_Lib(void);
void UA_Close_ML_Lib(void);

/**** medialinklib ****/

void UA_InitStruct(struct UserApplicInfo *UAI);
void UA_OpenScreen(struct UserApplicInfo *UAI);
void UA_OpenWindow(struct UserApplicInfo *UAI);
void UA_CloseScreen(struct UserApplicInfo *UAI);
void UA_CloseWindow(struct UserApplicInfo *UAI);
BOOL UA_HostScreenPresent(struct UserApplicInfo *UAI);
void UA_DrawGadget(	struct Window *window,
										struct GadgetRecord *GR);
void UA_DrawGadgetList(	struct Window *window,
												struct GadgetRecord *GR);
int UA_CheckGadget(	struct Window *window,
										struct GadgetRecord *FirstGR,
										struct GadgetRecord *GR,
										struct EventData *ED);
int UA_CheckGadgetList(	struct Window *window,
												struct GadgetRecord *FirstGR,
												struct EventData *ED);
int UA_ProcessCycleGadget(struct Window *window,
													struct GadgetRecord *GR,
													struct EventData *ED);
void UA_HiliteButton(	struct Window *window,
											struct GadgetRecord *GR);
void UA_InvertButton(	struct Window *window,
											struct GadgetRecord *GR);
void UA_HiliteRadioButton(struct Window *window,
													struct GadgetRecord *GR);
void UA_InvertRadioButton(struct Window *window,
													struct GadgetRecord *GR);
void UA_InvertCheckButton(struct Window *window,
													struct GadgetRecord *GR);
void UA_ClearButton(struct Window *window,
										struct GadgetRecord *GR,
										int pen);
void UA_ClearCycleButton(	struct Window *window,
													struct GadgetRecord *GR,
													int pen);
void UA_SetValToCycleGadgetVal(	struct GadgetRecord *GR,
																int *value);
void UA_SetCycleGadgetToVal(struct Window *window,
														struct GadgetRecord *GR,
														int value);
int UA_GetRadioGadgetVal(struct GadgetRecord *GR);
void UA_DisableButton(struct Window *window,
											struct GadgetRecord *GR,
											UWORD *pattern);
void UA_EnableButton(	struct Window *window,
											struct GadgetRecord *GR);
void UA_EnableButtonQuiet(struct GadgetRecord *GR);
void UA_DisableButtonQuiet(struct GadgetRecord *GR);
void UA_AdjustGadgetCoords(	struct GadgetRecord *GR,
														int xoffset,
														int yoffset);
void UA_DisableRangeOfButtons(struct Window *window,
															struct GadgetRecord *GR,
															int num,
															UWORD *pattern);
void UA_EnableRangeOfButtons(	struct Window *window,
															struct GadgetRecord *GR,
															int num);
void UA_SetStringGadgetToString(struct Window *window,
																struct GadgetRecord *GR,
																STRPTR str);
void UA_SetStringToGadgetString(struct GadgetRecord *GR,
																STRPTR str);
void UA_WipeStringGadget(struct GadgetRecord *GR);
BOOL UA_IsGadgetDisabled(struct GadgetRecord *GR);
void UA_DrawTwoColorBox(struct Window *window,
												struct GadgetRecord *GR);
void UA_DrawText(	struct Window *window,
									struct GadgetRecord *GR,
									STRPTR str);
void UA_GetKeys(USHORT *raw,
								char *key,
								int *numkeys,
								struct EventData *ED);
void UA_SetStringGadgetToVal(	struct Window *window,
															struct GadgetRecord *GR,
															int value);
void UA_SetValToStringGadgetVal(struct GadgetRecord *GR,
																int *value);
BOOL UA_ProcessStringGadget(struct Window *window,
														struct GadgetRecord *FirstGR,
														struct GadgetRecord *GR,
														struct EventData *ED);
void UA_DrawStringText(	struct Window *window,
												struct GadgetRecord *GR,
												STRPTR str);
void UA_DrawSpecialGadgetText(struct Window *window,
															struct GadgetRecord *GR,
															STRPTR str,
															int mode);
int UA_GetDepthOfWindowScreen(struct Window *window);
void UA_DoubleGadgetDimensions(struct GadgetRecord *GR);
void UA_HalveGadgetDimensions(struct GadgetRecord *GR);
BOOL UA_IsWindowOnLacedScreen(struct Window *window);
BOOL UA_IsUAScreenLaced(struct UserApplicInfo *UAI);
int UA_FindString(STRPTR OrgString,
									STRPTR CheckString);
void UA_ValidatePath(STRPTR path);
void UA_TurnAssignIntoDir(STRPTR ass);
void UA_MakeFullPath(	STRPTR path,
											STRPTR name,
											STRPTR answer);
void UA_SplitFullPath(STRPTR fullPath, STRPTR path, STRPTR filename);
void UA_DrawSliderNotches(	struct Window *window,
														struct GadgetRecord *GR,
														int type,
														int num,
														int pen );
void UA_DrawDefaultButton( struct Window *window, struct GadgetRecord *GR );
void UA_TranslateGR(struct GadgetRecord *GR, UBYTE **msgs);
void UA_PutCapsPort(struct MsgPort *port);
void UA_CloseWindowSafely(struct Window *window);
int UA_GetRightPen(struct Window *window, struct GadgetRecord *GR, long pen);
void UA_DrawGadgetListRange(struct Window *window,
														struct GadgetRecord *GR,
														int first,
														int last );
void UA_DrawComboBox(struct Window *window, struct GadgetRecord *GR);

/**** newlist ****/

void UA_PrintNewList(struct ScrollRecord *SR, int top, TEXT *ptrlist[], BOOL updateButtons);
void UA_ScrollNewList(struct ScrollRecord *SR, int *top, struct Gadget *g,
											TEXT *ptrlist[], struct EventData *oldCED);
int UA_SelectNewListLine(struct ScrollRecord *SR, int top, struct EventData *CED);

/**** popup ****/

void UA_Monitor_PopUp(struct PopUpRecord *PUR);
BOOL UA_OpenPopUpWindow(struct Window *onWindow,struct GadgetRecord *onGR,struct PopUpRecord *PUR);
void UA_ClosePopUpWindow(struct PopUpRecord *PUR);
void UA_PrintPopUpChoice(struct Window *onWindow, struct GadgetRecord *onGR, struct PopUpRecord *PUR);
void UA_PrintPopUpChoice2(struct Window *onWindow, struct GadgetRecord *onGR, char *txt);

/**** rexx1 ****/

BOOL UA_IssueRexxCmd_V1(STRPTR appName, STRPTR command, STRPTR result, BOOL resultWanted, int max);

/**** rexx2 ****/

BOOL UA_IssueRexxCmd_V2(STRPTR appName, STRPTR port_name, STRPTR command, STRPTR resultStr, ULONG *RC);

/**** sprites ****/

void UA_SetSpritePtrs(UWORD *waitPointer, UWORD *colorPicker, UWORD *toSprite, UWORD *hand);
void UA_SetSprite(struct Window *window, BYTE which);

/**** stdlist ****/

void UA_PrintStandardList(struct ScrollRecord *SR,
													int top,
													TEXT *ptrlist[]);
void UA_ScrollStandardList(	struct ScrollRecord *SR,
														int *top,
														struct Gadget *g,
														TEXT *ptrlist[],
														struct EventData *oldCED );
int UA_SelectStandardListLine(	struct ScrollRecord *SR,
																int top,
																BOOL multiple,
																struct EventData *CED,
																BOOL deselect,
																BOOL select );
void UA_ShortenString(struct RastPort *rp,
											STRPTR str,
											int numPixels);
void UA_ShortenStringFront(	struct RastPort *rp,
														STRPTR str,
														int numPixels);
void UA_PrintInBox(	struct Window *window,
										struct GadgetRecord *GR,
										int x1,
										int y1,
										int x2,
										int y2,
										STRPTR oristr,
										int mode);
void UA_printSeveralLines(struct Window *window,
													struct GadgetRecord *GR,
													int x,
													int y,
													int width,
													int height,
													STRPTR str);
void UA_SwitchMouseMoveOn(struct Window *window);
void UA_SwitchMouseMoveOff(struct Window *window);
void UA_GetPropSlider(struct Window *window,
											struct Gadget *g,
											int numEntries,
											int numDisplay,
											int *top);
void UA_GetHorizPropSlider(	struct Window *window,
														struct Gadget *g,
														int numEntries,
														int numDisplay,
														int *top);
void UA_InitPropSlider(	struct Window *window,
												struct Gadget *g,
												ULONG numEntries,
												ULONG numDisplay,
												LONG topEntry);
void UA_InitHorizPropSlider(struct Window *window,
														struct Gadget *g,
														ULONG numEntries,
														ULONG numDisplay,
														LONG topEntry);
void UA_SetPropSlider(struct Window *window,
											struct Gadget *g,
											ULONG numEntries,
											ULONG numDisplay,
											LONG topEntry);
void UA_SetHorizPropSlider(	struct Window *window,
														struct Gadget *g,
														ULONG numEntries,
														ULONG numDisplay,
														LONG topEntry);
void UA_SwitchFlagsOn(struct Window *window, ULONG flags);
void UA_SwitchFlagsOff(struct Window *window, ULONG flags);

/**** vectors ****/

void UA_DrawVector(	struct Window *window,
										struct VectorRecord *VR);
void UA_DrawVectorList(	struct Window *window,
												struct VectorRecord *VR);
void UA_DoubleVectorDimensions(struct VectorRecord *VR);
void UA_HalveVectorDimensions(struct VectorRecord *VR);

/**** winreq ****/

struct Window * UA_OpenRequesterWindow(struct Window *onWindow,struct GadgetRecord *GR,BYTE palette);
void UA_CloseRequesterWindow(struct Window *window, BYTE palette);
int UA_GetNumberOfColorsInScreen(ULONG viewmodes,int depth,BOOL AA_available);
LONG UA_MyFindColor(struct ColorMap *cm,
										ULONG r,
										ULONG g,
										ULONG b,
										LONG maxpen,
										int count,
										BOOL colorZeroAlso );
void UA_SetMenuColors(struct RendezVousRecord *rvrec, struct Window *window);
void UA_ResetMenuColors(struct RendezVousRecord *rvrec, struct Window *window);

/******** E O F ********/
