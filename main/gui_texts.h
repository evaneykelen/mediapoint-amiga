/******** GUI_TEXTS.H ********/

/* DO NOT TRANSLATE THESE */

#define APPLICTITLE			"mediapoint"	// used e.g. to find root directory
#define ML_RENDEZ_VOUS	"MP rendez-vous"
#define ML_CLOCK_NAME		"MP clock"
#define ML_CLOCK_TASK		"MP clock task"
//#define EX_ML_STARTUP		"execute s:mediapoint-startup"
#define ICON_PATH				"MP:MediaPoint"	// see icon.c
#define ICON_PATH2			"MP:Player"			// see icon.c
#define ML_LIBRARY_1		"mediapoint.library"
#define ML_LIBRARY_2		"mpmmu.library"
#define APPFONT					"MediaPoint.font"
#define SHORTAPPFONT		"MediaPoint"
#define CONFIG_TEXT			"system/mediapoint.config"
#define TEXT_DEFAULT_ED	"c:ed"
#define MEDIALINKPORT		"MediaPointPort"

/* Msg_... Is an index. E.g. char *msgs[Msg_OK-1] points to the OK string. */

enum {

/* MISC */

Msg_VerStr=1, Msg_VersionNumber, Msg_RevisionNumber, Msg_BetaNr,

Msg_AppTitle, Msg_AppName, Msg_OK, Msg_Cancel, Msg_More, Msg_Untitled,
Msg_Previous, Msg_Next, Msg_Numbers_1_40, Msg_Years_1990_1998, Msg_FPS_Auto_60,
Msg_Infinite_30, Msg_Minus3_15, Msg_Numbers_0_15, Msg_InternalError, Msg_Yes,
Msg_No,
/* Msg_LowOnMem,*/


/* ABOUT */

Msg_About_a1, Msg_About_a2, Msg_About_a3, Msg_About_a4, Msg_About_a5, Msg_About_a6,

Msg_About_b1, Msg_About_b2, Msg_About_b3, Msg_About_b4, Msg_About_b5, Msg_About_b6,
Msg_About_b7, Msg_About_b8, Msg_About_b9,

Msg_About_c1, Msg_About_c2, Msg_About_c3, Msg_About_c4, Msg_About_c5, Msg_About_c6,

Msg_Translation, Msg_Translator,
Msg_AboutSupport, Msg_AboutSup1, Msg_AboutSup2,

/* FA MENU */

Msg_FA_1, Msg_FA_2, Msg_FA_3, Msg_FA_4,


/* OPEN FILE REQUESTER */

Msg_Parent, Msg_Disks, Msg_Assigns, Msg_Open, Msg_Home,


/* SAVE FILE REQUESTER */

Msg_SaveThisAs, Msg_Replace,


/* CLOSE */

Msg_SaveChanges, Msg_Document, Msg_Script, Msg_DontSave, Msg_Save,


/* PAGE SETUP */

Msg_PageSetup, Msg_ScaleAndOri, Msg_ScaleAndOri_List, Msg_Quality,
Msg_Draft, Msg_Letter, Msg_GetWBPrefs,


/* PRINT */

Msg_PrintPage, Msg_PrintScript, Msg_Dest, Msg_Printer1, Msg_Printer2,
Msg_Printer3, Msg_Printer4, Msg_Options, Msg_TextOnly, Msg_MultipleFiles,
Msg_Copies, Msg_Print,


/* DISTRIBUTOR */

Msg_Distributor, Msg_Distribute, Msg_Center, Msg_Distribute_List, Msg_Center_List,


/* DUPLICATOR */
Msg_Duplicator, Msg_Copy, Msg_Times, Msg_Add, Msg_ToX, Msg_ToY,


/* IMPORT */

Msg_SelectImportType,
Msg_Background, Msg_Picture, Msg_Screen, Msg_Text, Msg_DataType,
Msg_RemapIt, Msg_ScaleIt,

Msg_PictureTooSmall, Msg_Delay2,

/* GRAB SCREEN */

Msg_Grab,


/* WINDEF */

Msg_WindowDefinitions, Msg_BorderWidth, Msg_FillType, Msg_FillType_List,
Msg_X, Msg_Y, Msg_Width, Msg_Height, Msg_ApplyThisToAll, Msg_Window,


/* PALETTE */

Msg_R, Msg_G, Msg_B, Msg_Spread, Msg_Swap, Msg_Undo, Msg_LoadPalette,
Msg_SavePalette, Msg_Limits, Msg_BroadcastLimits, Msg_Min, Msg_Max,
Msg_NoColorMap, Msg_OpenColormap, Msg_SaveColormap,


/* SCREEN SIZE */

Msg_Format, Msg_Format_List, Msg_Overscan, Msg_Overscan_List, Msg_Colors,
Msg_OldColors_List, Msg_NewColors_List, Msg_ScanRate, Msg_Mode1, Msg_Mode2,
Msg_Mode3, Msg_Mode4, Msg_Mode5, Msg_Mode6, Msg_UnknownType,


/* CHANGE SCREEN MODE */

Msg_ChangeScreenSize,


/* WINDOW TRANSITIONS */

Msg_WindowTransitions, Msg_Window2,	Msg_In, Msg_Out, Msg_Delay,


/* PREFS1 */

Msg_UserLevel, Msg_UserLevel_List,
Msg_ColorSet,
Msg_Lang, Msg_Lang_List,
Msg_Workbench, Msg_OnOff_List, 
Msg_MenuMonitor, Msg_PlayerMonitor, Msg_Monitor_List,
Msg_ScriptScreen, Msg_Laced_List,


/* PREFS 2 */

Msg_ThumbnailSize, Msg_ThumbnailSize_List,
Msg_ThumbColors,
Msg_ThumbScreen,
Msg_ShowDateProg,
Msg_ShowDates,
Msg_YesNo_List,


/* PREFS 3 */

Msg_PrgTimeCodes,
Msg_F1, Msg_F2, Msg_F3, Msg_F4, Msg_F5, Msg_F6, Msg_F7, Msg_F8, Msg_F9, Msg_F10,


/* PREFS 4 */

Msg_DefaultDirs, Msg_HomeDirs, Msg_DefaultList, Msg_Descrip, Msg_Path,


/* PREFS 5 */

Msg_PlayerDevice, Msg_BaudRate, Msg_BaudRate_List,
Msg_SerialDevice, Msg_Unit, Msg_NoDeviceSelected,


/* PREFS MISC */

Msg_LanNotAvailable, Msg_Script2, Msg_NoMonitor,


/* FILETHUMBS */

Msg_ImageToo, Msg_Large,


/* TRANSITION */

Msg_SelectTransition, Msg_Speed, Msg_ChunkSize, Msg_NoEffect,


/* SCRIPT SCREEN */

Msg_GlobalEvents, Msg_TimeCode, Msg_Root, Msg_Comment, Msg_Vars,

Msg_Play, Msg_Record, Msg_DeleteGlobalEvent, Msg_UserLevelConflict,
Msg_Ignore, Msg_Close, Msg_DepthArr,


/* GLOBAL EVENTS AND LABELS */

Msg_AvailableKeys, Msg_AssignedLabels, Msg_SelectALabel, Msg_UniqueLabel,
Msg_NameParallel, Msg_NameSerial, Msg_ObjectComment, Msg_XappComment,


/* TIME CODE */

Msg_TimingSource, Msg_TimingRate, Msg_TimingType, Msg_Internal, Msg_External,
Msg_HHMMSST, Msg_MIDI, Msg_SMPTE, Msg_25FPS, Msg_30FPS, Msg_Offset,
Msg_SendOut, Msg_SwitchStd2TC, Msg_SwitchTC2Std, Msg_Discard, Msg_ScriptTiming,
Msg_ScriptTiming_List, Msg_Preload, Msg_Preload_List, Msg_PlayOptions,
Msg_PlayOptions_List, 


/* SCRIPT OBJECTS AND XAPPS */

Msg_Wait, Msg_View, Msg_Edit, Msg_Command, Msg_FramesPerSecond,
Msg_PlayFromDisk, Msg_AddLoopFrames, Msg_Loops, Msg_Frequency, Msg_Volume,
Msg_Port, Msg_CDTVSpeed, Msg_Show, Msg_Stop, Msg_NoFileSelectedYet, Msg_SoundObjList,


/* DISPLAY PROGRAMMER */

Msg_LongDayNames, Msg_MonthNames, Msg_StartDay, Msg_EndDay, Msg_StartTime,
Msg_EndTime, Msg_DateTimeRelation, Msg_ShortSunday, Msg_ShortMonday,
Msg_ShortTuesday, Msg_ShortWednesday, Msg_ShortThursday, Msg_ShortFriday,
Msg_ShortSaturday, Msg_Duration,

/* PARSER */

Msg_IllegalCommandInLine, Msg_MissesStart, Msg_MissesObjectStart,
Msg_InvalidNumberOfArgs, Msg_SerialWithinParallel, Msg_ParallelWithinParallel,
Msg_ReadingClipFailed, Msg_SyntaxErrorInLine, Msg_SyntaxErrorInScript,
Msg_DocTooLarge, Msg_DocUnreadable, Msg_ScriptTooLarge, Msg_ScriptUnreadable,


/* SELECT FILES */

Msg_SelectAProgram, Msg_SaveThisDocAs, Msg_SelectDocsOrPics,
Msg_SelectADocOrAPic, Msg_SelectAScript, Msg_SaveThisScriptAs, Msg_SelectATextFile,
Msg_SelectPics, Msg_SelectAnims, Msg_SelectAnAnim, Msg_SelectFiles, Msg_SelectAFile,
Msg_SelectMailFiles, Msg_SelectAMailFile, Msg_SelectSoundFiles, Msg_SelectASoundFile,
Msg_SelectAnArexxScript, Msg_SelectADosScript, Msg_SelectPath, Msg_SelectDevice,


/* DOS ERRORS */

Msg_UnableToExecute, Msg_UnableToLaunchXapp, Msg_UnableToReadPic,
Msg_ProblemsWithIFFPic, Msg_NotEnoughMemory, Msg_NoDotInfoFile, Msg_IFFChunkMisses,
Msg_UnableToWriteIcon, Msg_UnableToReadDoc, Msg_UnableToWriteClip, Msg_ClipDirMisses,
Msg_UnableToCreateClipDir, Msg_UnableToSaveDoc, Msg_UnableToReadScript,
Msg_UnableToSaveScript, Msg_UntitledMisses, Msg_UntitledMissesFatal,


/* STYLE SELECT */

Msg_Plain, Msg_Bold, Msg_Italic, Msg_Underline,
Msg_Justification_List, Msg_ShadowDirection_List,
Msg_ShadowType_List, Msg_Italicize_List,
Msg_Style_Anti_List,
Msg_Style_Just_List,
Msg_Style_CSpc_List,
Msg_Style_LSpc_List,
Msg_Style_Slan_List,
Msg_Style_STyp_List,
Msg_Style_SLen_List,
Msg_Style_SDir_List,
Msg_Style_UWei_List,
Msg_Style_UOff_List,
Msg_LiteralList, 


/* GET XAPPS */

Msg_NoXappsDir, Msg_TooManyXapps, Msg_XappMissesInfo, Msg_NoToolType, Msg_IconTooLarge,


/* MODULE TYPES */

Msg_Mod_MarkII, Msg_Mod_DSS, Msg_Mod_ST, Msg_Mod_SNPro, Msg_Mod_FC13, Msg_Mod_FC14,
Msg_Mod_Jam, Msg_Mod_SM, Msg_Mod_Unknown,


/* SPECIAL CHARS */

Msg_Char_SelectAll, Msg_Char_Thumbnails, Msg_Char_CloseGadget, Msg_Char_Cross,


/* MENU TEXTS */

Msg_Menu_DA,
	Msg_Menu_About,

Msg_Menu_File,
	Msg_Menu_File_New,
	Msg_Menu_File_Open,
	Msg_Menu_File_Close,
	Msg_Menu_File_Save,
	Msg_Menu_File_SaveAs,
	Msg_Menu_File_PageSetUp,
	Msg_Menu_File_Print,
	Msg_Menu_File_Quit,

Msg_Menu_Edit,
	Msg_Menu_Edit_Undo,
	Msg_Menu_Edit_Cut,
	Msg_Menu_Edit_Copy,
	Msg_Menu_Edit_Paste,
	Msg_Menu_Edit_Clear,
	Msg_Menu_Edit_SelectAll,
	Msg_Menu_Edit_Distribute,
	Msg_Menu_Edit_Duplicate,

Msg_Menu_Font,
	Msg_Menu_Font_Type,
	Msg_Menu_Font_Style,

Msg_Menu_PMisc,
	Msg_Menu_PMisc_Import,
	Msg_Menu_PMisc_Define,
	Msg_Menu_PMisc_Palette,
	Msg_Menu_PMisc_ScreenSize,
	Msg_Menu_PMisc_Link,
	Msg_Menu_PMisc_Remap,
	Msg_Menu_PMisc_Specials,
	Msg_Menu_PMisc_Transitions,

Msg_Menu_Screen,
	Msg_Menu_Page,
	Msg_Menu_Prefs,
	Msg_Menu_Script,

Msg_Menu_Xfer,
	Msg_Menu_Xfer_Upload,
	Msg_Menu_Xfer_Download,

Msg_Menu_SMisc,
	Msg_Menu_SMisc_ShowProg,
	Msg_Menu_SMisc_ShowDays,
	Msg_Menu_SMisc_LocalEvents,


/* FONTS */

Msg_FontNotFound,


/* PRINTING */

Msg_ScriptPrinting, Msg_PrinterClosed, Msg_Footer1, Msg_Footer2,


/* PH MESSAGES */

Msg_NoTempoEditor, Msg_OutOfMemory, Msg_CouldNotLoadXapp, Msg_StackOverflow,
Msg_GraphicsMemError, Msg_CouldNotLoadInput, Msg_ScriptIsEmpty, Msg_InvalidTimeCode,
Msg_UnableToPlayScript,


/* Crawl */

Msg_Crawl, Msg_CrawlFont, Msg_CrawlColor,


/* XAPP TEXTS */

Msg_X_1, Msg_X_2, Msg_X_3, Msg_X_4, Msg_X_5,


/* LaserDisc xapp */

Msg_X_LD_1,  Msg_X_LD_2,  Msg_X_LD_3,  Msg_X_LD_4,  Msg_X_LD_5,  Msg_X_LD_6,  
Msg_X_LD_7,  Msg_X_LD_8,  Msg_X_LD_9,  Msg_X_LD_10, Msg_X_LD_11,  


/* Ion xapp */

Msg_X_I_1, Msg_X_I_2, Msg_X_I_3, Msg_X_I_4, Msg_X_I_5,
Msg_X_I_6, Msg_X_I_7, Msg_X_I_8, Msg_X_I_9, Msg_X_I_10, 


/* Studio 16 xapp */

Msg_S16_1,  Msg_S16_2,  Msg_S16_3,  Msg_S16_4,  Msg_S16_5,  Msg_S16_6, 
Msg_S16_7,  Msg_S16_8,  Msg_S16_9,  Msg_S16_10, Msg_S16_11, Msg_S16_12, 


/* CDTV xapp */

Msg_CDTV_1, Msg_CDTV_2, Msg_CDTV_3,  Msg_CDTV_4,  Msg_CDTV_5,  Msg_CDTV_6,  Msg_CDTV_7, 
Msg_CDTV_8, Msg_CDTV_9, Msg_CDTV_10, Msg_CDTV_11, Msg_CDTV_12, Msg_CDTV_13, Msg_CDTV_14, 


/* CDXL xapp */

Msg_CDXL_1, Msg_CDXL_2, Msg_CDXL_3, Msg_CDXL_4, Msg_CDXL_5, Msg_CDXL_6, Msg_CDXL_7, 


/* IV24 page 1 */

Msg_IV_1_1, Msg_IV_1_2, Msg_IV_1_3, Msg_IV_1_4, Msg_IV_1_5, 
Msg_IV_1_6, Msg_IV_1_7, Msg_IV_1_8, Msg_IV_1_9, Msg_IV_1_10, 


/* IV24 page 2 */

Msg_IV_2_1, Msg_IV_2_2,


/* IV24 page 3 */

Msg_IV_3_1, Msg_IV_3_2, Msg_IV_3_3, Msg_IV_3_4, 
Msg_IV_3_5, Msg_IV_3_6, Msg_IV_3_7, Msg_IV_3_8, 


/* IV24 page 4 */

Msg_IV_4_1, Msg_IV_4_2, Msg_IV_4_3,


/* IV24 page mode descriptions */

Msg_IV_PM_1,  Msg_IV_PM_2,  Msg_IV_PM_3,  Msg_IV_PM_4,  Msg_IV_PM_5,  Msg_IV_PM_6, 
Msg_IV_PM_7,  Msg_IV_PM_8,  Msg_IV_PM_9,  Msg_IV_PM_10, Msg_IV_PM_11, Msg_IV_PM_12, 


/* IV24 misc */

Msg_IV_MISC_1, Msg_IV_MISC_2, Msg_IV_MISC_3, Msg_IV_MISC_4,


/* MIDI xapp */

Msg_MIDI_1,


/* rest of Sample xapp */

Msg_Sample_1, Msg_Sample_2, Msg_Sample_3, Msg_Sample_4, Msg_Sample_5, Msg_Sample_6,


/* Interactive */

Msg_Inter1, Msg_Inter2, Msg_Inter3, Msg_Inter4, Msg_Inter5, Msg_Inter6, 
Msg_Inter7, Msg_Inter8, 


/* Variable declaration */

Msg_VarDec1, Msg_VarDec2, Msg_VarDec3, 


/* Expression declaration */

Msg_ExpDec1,


/* local events */

Msg_LocalEvents_1, Msg_LocalEvents_2, Msg_LocalEvents_3, Msg_LocalEvents_4, 
Msg_LocalEvents_5,
Msg_LocalEvents_7, Msg_LocalEvents_8, Msg_LocalEvents_9,

/* rest prefs */

Msg_LocaleHint, Msg_PdevMisses, Msg_PlayBuffer, Msg_PlayBuffer_List, Msg_PressAnyKey,
Msg_Standby, Msg_PlayerInput, Msg_PlayerInputList, Msg_MousePointer,

/* tweaker */

Msg_TC_Title, Msg_TC_Start, Msg_TC_End, Msg_TC_Delta, Msg_TC_Offset,

Msg_Sched2,
Msg_8_16_32,

/* misc page editor */

Msg_TooManyColors,
Msg_GettingFonts,

/* palette */

Msg_Hide,

/* rest menu */

Msg_Menu_Font_Color,

/* specials/extra's */

Msg_SelectPic,

Msg_SpecialsConform,
Msg_SpecialsUsePalette,

/* rest interact */

Msg_InterActButton,

/* variables */

Msg_Variables, Msg_InsLabel,

/* rest script texts */

Msg_Abort,

/* variables */

Msg_Decla, Msg_Expre,

Msg_NoDeclas, Msg_KillDeclas,

Msg_StackTooLow,

Msg_GotoPrevPage, Msg_GotoNextPage, Msg_DestCantBeChanged,

Msg_ColorCycle,

Msg_PatternType,

/* Script Manager */

Msg_SM_1,  Msg_SM_2,  Msg_SM_3,  Msg_SM_4,  Msg_SM_5,  Msg_SM_6,  Msg_SM_7,  Msg_SM_8,
Msg_SM_9,  Msg_SM_10, Msg_SM_11, Msg_SM_12,

Msg_SM_20, Msg_SM_21, Msg_SM_22, Msg_SM_23, Msg_SM_24, 
Msg_SM_25, Msg_SM_26, Msg_SM_27, Msg_SM_28, Msg_SM_29, 
Msg_SM_30, Msg_SM_31,

Msg_SM_32, Msg_SM_33, Msg_SM_34, Msg_SM_35, Msg_SM_36, Msg_SM_37,

Msg_SaveChanges2, Msg_ScriptOn, Msg_OverItself,

Msg_SpriteOnOffAutoList,

Msg_ButtonsSameName,

Msg_OptimizePalette,

Msg_FileGone, Msg_FirstFixTheRun1, Msg_FirstFixTheRun2,

Msg_TileIt,

Msg_WDefShadowList, Msg_WDefShadow,

Msg_BestColors,

Msg_Ex_Extras, Msg_Ex_Cycle, Msg_Ex_Spaces, Msg_Ex_Remap, Msg_Ex_Resize,
Msg_Ex_Delete, Msg_Ex_UseColors, Msg_Ex_Conform, Msg_Ex_Offset, Msg_Ex_Transparent, 
Msg_Ex_Floyd, Msg_Ex_Burkes, Msg_Ex_Ordered, Msg_Ex_Tile, 

Msg_FastMenus, Msg_GamePort, Msg_Ex_NoDither,

Msg_DeletePic,

Msg_Borders,

Msg_Ex_Random,

Msg_AutoDetect,

Msg_IFF, Msg_SavingAsIFF,

Msg_DB_1, Msg_DB_2, Msg_DB_3, Msg_DB_4, Msg_DB_5, Msg_DB_6, Msg_DB_7, Msg_DB_8, 
Msg_DB_Error_1, Msg_DB_Error_2, Msg_DB_Error_3, Msg_DB_Error_4, 

/* Tocatta xapp */

Msg_TOC_1,  Msg_TOC_2,  Msg_TOC_3,  Msg_TOC_4,  Msg_TOC_5, 
Msg_TOC_6,  Msg_TOC_7,  Msg_TOC_8,  Msg_TOC_9,  Msg_TOC_10, 
Msg_TOC_11, Msg_TOC_12, Msg_TOC_13, Msg_TOC_14, Msg_TOC_15, 
Msg_TOC_16, Msg_TOC_17, Msg_TOC_18, Msg_TOC_19, Msg_TOC_20,
Msg_TOC_21, Msg_TOC_22, Msg_TOC_23, Msg_TOC_24, Msg_TOC_25,

/* new prefs texts */

Msg_P_ScriptMon, Msg_P_PageMon, Msg_P_PlayerMon, Msg_P_ScriptLace, Msg_P_ThumbsLace,
Msg_P_SelAMon,

Msg_P_Genlockable, Msg_P_NotGenlockable,

Msg_P_DefaultMon,

/* new for script manager */

Msg_SM_38, Msg_SM_39, Msg_SM_40, Msg_SM_41,

/* Genlock xapp */

Msg_GL_Name, Msg_GL_Mode, Msg_GL_DisplayIs, Msg_GL_AlwaysLaced,

/* new for style req */

Msg_AntiAlias_List,

/* new for tocatta */

Msg_TOC_Title,

/* VuPort */

Msg_VU_Name, Msg_VU_Cmds, Msg_VU_Units, Msg_VU_Cmd, Msg_Unit2,

/* Arexx */

Msg_NoARexx,

/* Added after release 127 */

Msg_Filter,

/* AirLink */

Msg_Air_1, Msg_Air_2, Msg_Air_3,

/* Neptun */

Msg_Nep_Title,
Msg_Nep_Page,
Msg_Nep_Computer,
Msg_Nep_Genlock,
Msg_Nep_Amiga,
Msg_Nep_Overlay,
Msg_Nep_Normal, 
Msg_Nep_Alpha,
Msg_Nep_Video,
Msg_Nep_Invert,
Msg_Nep_FadeIn, 
Msg_Nep_FadeOut, 
Msg_Nep_From, 
Msg_Nep_To, 
Msg_Nep_Get, 

Msg_Nep_Dur, Msg_Nep_Click,

Msg_NoRexxHost,

/* AirLink */

Msg_Air1, Msg_Air2,

/* resource xapp */

Msg_RX_LoadFlush, Msg_RX_FileSize,

/* Rest cd32 */

Msg_CD32_Title,

/* New anim entry in extras list view */

Msg_Ex_AnimSettings, Msg_FirstImportAnim,

Msg_FreeOnDisk,

/* vertical scroll */

Msg_VSC_Title, Msg_VSC_Bgnd, Msg_VSC_Text, Msg_VSC_Weight, Msg_VSC_Type,
Msg_VSC_ColorsList, Msg_VSC_SType, Msg_SaveCredit, Msg_LoadCredit, Msg_FileNotSaved,
Msg_VSC_FistSave, Msg_VSC_HelpWarn, Msg_VSC_Help1, Msg_VSC_Help2, Msg_VSC_Help3,
Msg_VSC_Help4, Msg_VSC_Help5, Msg_VSC_Help6, Msg_VSC_Help7, Msg_VSC_Help8,
Msg_VSC_Help9, Msg_VSC_Help10, Msg_VSC_Help11, Msg_VSC_Help12, Msg_VSC_Help13,
Msg_VSC_Help14, Msg_VSC_Clear,

/* serial xapp */

Msg_Ser1, Msg_Ser2, Msg_Ser3, Msg_Ser4, Msg_Ser5, Msg_Ser6, Msg_Ser7, Msg_Ser8,
Msg_Ser9, Msg_Ser10, Msg_Ser_HS, Msg_Ser_PA, Msg_Ser_BC, Msg_Ser_SB, 
Msg_Ser11, Msg_Ser12, Msg_Ser13, Msg_Ser14, Msg_Device, Msg_BaudList,

Msg_AnimBrNoScaleRemap,

Msg_Cache,

Msg_Transp,

Msg_PictureAndAnim,

Msg_XI_2, Msg_XI_3,

Msg_VarPath_1, Msg_VarPath_2, Msg_VarPath_3, Msg_VarPath_4, Msg_VarPath_5,

Msg_Menu_SMisc_VarPath,

Msg_BroadcastLimits2,

/* rest interactivity */

Msg_Inter9,

Msg_FirstSave,

/* new for DOS xapp */

Msg_Stack,

/* New for 129 */

Msg_InputSettings, Msg_AsyncClicking, Msg_AC_Help,

/* DPS PAR */

Msg_PAR_Title, Msg_PAR_In, Msg_PAR_Out, Msg_PAR_Cue,

Msg_NoARexxPort,

/* REMOTE ACCESS */

Msg_RA_Remote_Access, Msg_RA_Open_Session, Msg_RA_Save_Session, Msg_RA_Options,
Msg_RA_Upload, Msg_RA_Scripts, Msg_RA_Script, Msg_RA_Swap, Msg_RA_Carrier, Msg_RA_Connection,
Msg_RA_Add, Msg_RA_Delete, Msg_RA_Edit, Msg_RA_Swap_immediately, Msg_RA_Abort,
Msg_RA_Upload_Options, Msg_RA_Upload_all_files, Msg_RA_Delayed_upload,
Msg_RA_Skip_system_files, Msg_RA_Upload_multiple_scripts,
Msg_RA_Countdown, Msg_RA_Upload_starts, Msg_RA_from_now, Msg_RA_Select_Script,
Msg_RA_Select_Carrier_and_Connection, Msg_RA_Upload_delay, Msg_RA_Start_to_upload,
Msg_RA_Immediately, Msg_RA_Later, Msg_RA_Date, Msg_RA_Time,
Msg_RA_ECP, Msg_RA_CDF, Msg_RA_Modi1, Msg_RA_Modi2,
Msg_RA_Select_a_session, Msg_RA_Save_this_session_as, Msg_RA_SessionSaveError,
Msg_RA_SelectECP, Msg_RA_SelectCDF, Msg_RA_SessionReadError,
Msg_RA_PrepScript, Msg_RA_Missing1, Msg_RA_Missing2, Msg_RA_LaunchingECP,
Msg_RA_ECP_Error, Msg_RA_DOS_error, Msg_RA_Finishing_upload, Msg_RA_ErrorRep1,
Msg_RA_ErrorRep2, Msg_RA_ECP_CDF_Missing, Msg_RA_Remote_Access2, Msg_RA_NoScripts,

/* New for script editor */

Msg_ScriptModi, Msg_Overwriting, Msg_SaveItNow,

/* Rest Remote Access */

Msg_ECP_1, Msg_ECP_2, Msg_ECP_3, Msg_ECP_4, Msg_ECP_5, Msg_ECP_6, Msg_ECP_7, Msg_ECP_8, 
Msg_ECP_9, Msg_ECP_10, Msg_ECP_11, Msg_ECP_12, Msg_ECP_13,

Msg_RA_Aborted, Msg_ECP_14,

/* CDF Editor */

Msg_CDF_1, Msg_CDF_2, Msg_CDF_3, Msg_CDF_4, Msg_CDF_5, Msg_CDF_6, Msg_CDF_7, Msg_CDF_8,
Msg_CDF_9, Msg_CDF_10, Msg_CDF_11, Msg_CDF_12, Msg_CDF_13, Msg_CDF_14,

Msg_Use,

Msg_CDF_15,

};

/******** E O F ********/
