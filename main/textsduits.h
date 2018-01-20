/******** TEXTS.H ********/

#define FONT_WINDOW_TITLE					"Font & Größe"
#define STYLE_WINDOW_TITLE				"Style"
#define COLORADJUST_WINDOW_TITLE	"Color adjust"
#define PALETTE_WINDOW_TITLE			"Palette"
#define SAMPLE_WINDOW_TITLE				"Font Beispiel"

#define FORMAT_FORMAT_TEXT				"Format:"
#define FORMAT_OVERSCAN_TEXT			"Overscan:"
#define FORMAT_COLORS_TEXT				"Farben:"
#define FORMAT_SPECIAL_TEXT				"Spezial:"

#define DONTSAVE_TEXT1						"Dokument wurde verändert. Trotzdem speichern?"
#define DONTSAVE_TEXT2						"??? schließen?"
#define DONTSAVE_TEXT3						"Save changes to the script"

#define THUMBTEXT1								"Erste"
#define THUMBTEXT2								"Vorherige"

#define THUMBTEXT3								"Abbrechen"
#define THUMBTEXT4								"Grab"

#define THUMBTEXT5a								"Bild zu"
#define THUMBTEXT5b								"Groß."

#define ABOUT_TEXT1								"Personalized for:"
#define ABOUT_TEXT2								"Amiga version 1.0"
#define ABOUT_TEXT3								"Copyright © 1991-92 1001 Software Development."
#define ABOUT_TEXT4								"All right reserved. MediaLink› and the MediaLink›"
#define ABOUT_TEXT5								"logo are trademarks of 1001 Software Development."

#define UNTITLED_TEXT							"Namenlos"
#define COMMENT_TEXT							"Kommentar"

#define DANAMES_TEXT							"Kein programm ausgewählt"

#define CONFIG_TEXT								"s:MediaLink.config"

#define APPFONT 									"MediaLink.font"
#define APPNAME 									"MediaLink"
#define VERSIONTAG								"\0$VER: MediaLink 1.0"
#define VBLANKNAME								"MediaLink_vBlank"
#define MEDIALINKPORT							"MediaLinkPort"
#define MEDIALINKLIBNAME					"medialink.library"

#define	NO_DISK_TEXT							"Keine Diskette vorhanden"

#define TL_FillTypes							"Solid\0  Pattern\0Transp\0 "
																 /*---------+++++++++---------*/

#define TL_FormatTypes						"low\0       low lace\0  high\0      high lace\0 super\0     super lace\0prod\0      prod lace\0 "
																 /*------------++++++++++++------------++++++++++++------------++++++++++++------------++++++++++++*/

#define TL_OverscanTypes					"Off\0Std\0Max\0"
																 /*-----+++++-----*/

#define TL_ColorTypes							"2\0   4\0   8\0   16\0  32\0  EHB\0 HAM\0  "
																 /*------++++++------++++++------++++++------*/

#define TL_NewColorTypes					"2\0   4\0   8\0   16\0  32\0  EHB\0 64\0  128\0 256\0 HAM\0 HAM8\0"
																 /*------++++++------++++++------++++++------++++++------++++++------*/

#define TL_ModeTypes							"PAL\0 NTSC\0"
																 /*------++++++*/

#define TL_SpecialTypes						"NONE\0  AVIDEO\0DCTV\0  IV-24\0 FIRE\0  A2410\0 "
																 /*--------++++++++--------++++++++--------++++++++*/

#define TL_WINDEFS								"Window definitions"
#define TL_COORDS									"Coordinates:"

/*#define TL_Top										"Top:"

#define TL_Bottom									"Bottom:"

#define TL_Left										"Left:"

#define TL_Right									"Right:"*/

#define TL_BorderWidth						"Border width:"
#define TL_FillType								"Fill type:"

#define TL_X											"X:"

#define TL_Y											"Y:"

#define TL_Width									"Width:"

#define TL_Height									"Height:"

/*#define TL_Letter_spacing					"Letter spacing:"

#define TL_Line_spacing						"Line spacing:"*/

#define TL_Cancel									"Cancel"

#define TL_OK											"OK"

#define TL_R											"R"
#define TL_G											"G"
#define TL_B											"B"
#define TL_Spread									"Spread"
#define TL_Copy										"Copy"
#define TL_Swap										"Swap"
#define TL_Undo										"Undo"
#define TL_Extra_color						"Extra color"
#define TL_Load_palette						"Load palette"
#define TL_Save_palette						"Save palette"
#define TL_Harmonize							"Harmonize"

#define TL_Menu_name							"Menu name:"

#define TL_Select_program					"Select program"

#define TL_Remove									"Remove"

#define TL_Select_path						"Select path"

#define TL_Show										"Show"

#define TL_Plain									"Plain"

#define TL_Bold										"Bold"

#define TL_Italic									"Italic"

#define TL_Underline							"Underline"

#define TL_Shadow									"Shadow"

#define TL_Outline								"Outline"

#define TL_Depth									"Depth:"

#define TL_Cast										"Cast"

#define TL_Solid									"Solid"

#define TL_Parent									"Parent"

#define TL_Disks									"Disks"

#define TL_Assigns								"Assigns"

#define TL_Open										"Open"

#define TL_Save										"Save"

#define TL_DontSave								"Don't save"

#define TL_More										"More"

#define TL_NextPage								"Next"

#define TL_SelectImportType				"Select import type"

#define TL_Picture								"Picture"

#define TL_Screen									"Screen"

#define TL_Text										"Text"

#define TL_ResizeToFit						"Resize to fit"
#define TL_Remap									"Remap"

#define TL_DayTypes								"Sunday\0        Monday\0        Tuesday\0       Wednesday\0     Thursday\0      Friday\0        Saturday\0      "
																 /*----------------++++++++++++++++----------------++++++++++++++++----------------++++++++++++++++----------------*/

#define TL_MonthTypes							"January\0       February\0      March\0         April\0         May\0           June\0          July\0          August\0        September\0     October\0       November\0      December\0      "
																 /*----------------++++++++++++++++----------------++++++++++++++++----------------++++++++++++++++----------------++++++++++++++++----------------++++++++++++++++----------------++++++++++++++++*/

#if 0
#define TL_SoundTypes							"File Type\0      Raw\0           8SVX\0            Audio IFF\0      Soundtracker\0   Noisetracker\0   Protracker\0     Pikketrekker\0   "
																 /*-----------------+++++++++++++++++-----------------+++++++++++++++++-----------------+++++++++++++++++-----------------+++++++++++++++++*/
#endif

#define TL_StartDay								"Start day"

#define TL_EndDay									"End day"

#define TL_StartTime							"Start time"

#define TL_EndTime								"End time"

#define TL_DateTimeRelation				"Date/Time relation"

#define TL_Su											"Su"

#define TL_Mo											"Mo"

#define TL_Tu											"Tu"

#define TL_We											"We"

#define TL_Th											"Th"

#define TL_Fr											"Fr"

#define TL_Sa											"Sa"

#define TL_MoreChoices						"More choices"

#define TL_Play										"Play"

/*efine TL_Defer									"Defer"
#define TL_Continue								"Continue"*/
#define TL_Wait										"Wait"

#define TL_View										"View"

#define TL_Edit										"Edit"

#define TL_Command								"Command"

#define TL_Script									"Script"

#define TL_FramesPerSecond				"Frames per second:"

#define TL_Loops									"Loops:"

#define TL_ColorCycle							"Color cycle"

#define TL_Frequency							"Frequency:"

#define TL_Volume									"Volume:"

#define TEXT_SELECT_DA						"Select an application:"

#define TEXT_OPEN_PAGE						"Select a page:"
#define TEXT_SAVE_PAGE						"Save this page as:"

#define TEXT_OPEN_SCRIPT					"Select a script:"
#define TEXT_SAVE_SCRIPT					"Save this script as:"

#define TEXT_OPEN_PICTURE					"Select a picture:"
#define TEXT_OPEN_TEXT						"Select a text file:"

#define TEXT_OPEN_ANIMS						"Select animation file(s):"
#define TEXT_OPEN_AREXXS					"Select ARexx script(s):"
#define TEXT_OPEN_BINARIES				"Select file(s):"
#define TEXT_OPEN_MAILS						"Select mail file(s):"
#define TEXT_OPEN_PAGES						"Select page(s) or picture(s):"
#define TEXT_OPEN_SOUNDS					"Select sound file(s):"

#define TEXT_EDIT_ANIM						"Select animation file:"
#define TEXT_EDIT_AREXX						"Select ARexx script:"
#define TEXT_EDIT_DOS							"Select DOS script:"
#define TEXT_EDIT_BINARY					"Select file:"
#define TEXT_EDIT_MAIL						"Select mail file:"
#define TEXT_EDIT_PAGE						"Select page or picture:"
#define TEXT_EDIT_SOUND						"Select sound file:"
#define TEXT_EDIT_XAPP						"Select external application:"

#define TEXT_LABEL_1							"Select a label:"

#define TEXT_GLOBAL_1							"Available keys:"
#define TEXT_GLOBAL_2							"Assigned labels:"
#define TEXT_GLOBAL_3							"Global Events"

#define TEXT_TIMECODE_1						"Time Code"
#define TEXT_TIMECODE_2						"Timing source:"
#define TEXT_TIMECODE_3						"Timing rate:"
#define TEXT_TIMECODE_4						"Timing type:"
#define TEXT_TIMECODE_5						"Internal"
#define TEXT_TIMECODE_6						"External"
#define TEXT_TIMECODE_7						"HH:MM:SS:T"
#define TEXT_TIMECODE_8						"MIDI time code"
#define TEXT_TIMECODE_9						"SMPTE"
#define TEXT_TIMECODE_10					"24 FPS"
#define TEXT_TIMECODE_11					"25 FPS"
#define TEXT_TIMECODE_12					"30 FPS drop frame"
#define TEXT_TIMECODE_13					"30 FPS"
#define TEXT_TIMECODE_14					"MIDI clock"
#define TEXT_TIMECODE_15					"MIDI time code out"
#define TEXT_TIMECODE_16					"Offset:"

#define TEXT_TIMECODE_17					"Switching from HH:MM:SS:T to MIDI/SMPTE \
timecode will discard all schedule information."

#define TEXT_TIMECODE_18					"Switching from MIDI/SMPTE to HH:MM:SS:T \
timecode will discard all frame codes."

#define TL_DISCARD								"Discard"

#define TEXT_OBJECTNAME_1					"Enter comment for this object:"
#define TEXT_OBJECTNAME_2					"Enter a unique label name:"
#define TEXT_OBJECTNAME_3					"Name this parallel event:"
#define TEXT_OBJECTNAME_4					"Name this serial event:"
#define TEXT_OBJECTNAME_5					"Rename this object:"
#define TEXT_OBJECTNAME_6					"Enter comment for this XaPP:"

#define TL_Launch									"Launch"

#define TL_Preload								"Preload"
#define TL_Unload									"Unload"

#define TEXT_DEFAULT_ED						"c:ed"	/* don't translate */

#define TL_Port										"Port:"

#define TEXT_NO_FILE							"No file selected yet."

#define TEXT_ERROR_THUMB1					"Unable to read picture %s."
#define TEXT_ERROR_THUMB2					"Problems with IFF picture %s."
#define TEXT_ERROR_THUMB3					"Not enough memory."

#define TEXT_CLOCK1								"Unable to create clock task."

#define READ_AND_RENDER_ICON_TEXT	"The .info file of page %s could not be read."

#define TL_STWRITE1								"Unable to save script %s."

#define TL_DOS1										"Unable to execute %s."

#define TL_BASIC1									"The file %s could not be found."

#define TL_BASIC2									"The file fonts:%s could not be found."

#define TL_SCREENS1								"QueryOverscan failed."

#define TL_IMPORT1								"The file %s could not be read."

#define TL_IMPORT2								"The picture is too small."

#define TL_PARSER1								"Syntax error in script, line %d, in function %s: %s."

#define TL_PARSER2								"Syntax error in script %s."

#define TL_SAVERS1								"The script %s could not be saved."

#define TL_SAVERS2								"The page %s could not be saved."

#define TL_XCMD1									"The Tools directory could not be found. Is it in the same directory as the %s application?"

#define TL_XCMD2									"You have too many tools! A maximum of %d tools is allowed."

#define TL_XCMD3									"The .info file of the %s tool could not be read."

#define TL_XCMD4									"The %s tool has no Tool Type. It will be treated as an external application."

#define TL_XCMD5									"The icon of the tool %s has wrong dimensions."

#define TL_LOADERS1								"The script %s could not be opened."
#define TL_LOADERS2								"The page %s could not be opened."

#define TL_ROOTNAME								"Root"

#define TL_DURATION								"Duration"

#define TL_SETTRANSITION					"Select transition:"

#define TL_Destination						"Destination:"
#define TL_Printer1								"Printer"
#define TL_Printer2								"PostScript® device on SER:"
#define TL_Printer3								"PostScript® device on PAR:"
#define TL_Printer4								"PostScript® file"
#define TL_Options								"Options:"
#define TL_TextOnly								"Text Only"
#define TL_Copies									"Copies:"
#define TL_Print									"Print"
#define TL_TextOnly								"Text Only"
#define TL_PageSetup							"Page Setup"
#define TL_ScaleAndOri						"Scale and Orientation:"
#define TL_Quality								"Quality:"
#define TL_Draft									"Draft"
#define TL_Letter									"Letter"
#define TL_PrintScript						"Print Script"
#define TL_PrintPage							"Print Page"
#define TL_GetWBPrefs							"Get WB prefs"

#define TL_PrinterClosed					"The printer cannot be accessed."

#define TL_FOOTER1								"Page"
#define TL_FOOTER2								"Event List"

#define TL_STOP										"Stop"

#define TL_SCRIPTPRINTING					"The script is now being printed. \
To cancel printing, click Stop or press Œ. (period)."

#define TL_STACK									"The stack size is too small. \
You are strongly advised to increase it to at least 20000 bytes."

#define TL_UNTITLEDMISSES					"The untitled script is missing and \
cannot be created either. Please replace it from the installation disk."

#define TL_UNTITLEDMISSES2				"The untitled script is missing and \
will be created again."

#define TL_REDRAW									"Redraw current page after choosing new screen size?"

#define INTERNAL_ERROR						"Internal error: %d"

#define TL_USECOLORS							"Use color palette?"

#define TL_DISPLAY_NOT_THERE			"Sorry: this machine does not offer the requested display mode."

#define TL_REMAP1									"Remapping failed due to low memory"

#define TL_SPECIALS1							"Set picture offset to 0,0"
#define TL_SPECIALS2							"Delete picture from window"
#define TL_SPECIALS3							"Conform window to picture size"
#define TL_SPECIALS4							"Revert to picture on disk"
#define TL_SPECIALS5							"Forget original picture"
#define TL_SPECIALS6							"Use palette of this picture"
#define TL_SPECIALS7							"Resize to fit"
#define TL_SPECIALS8							"Specials"

#define TL_ERRORILBM1							"The clip %s could not be written."

#define TL_CHANGEMODE							"Change to screen size of loaded page?"

#define TL_OVERWRITE							"Replace existing file "

#define TL_NOICON									"Unable to write icon"

#define TL_UserLevels							"Test Drive\0    Presentation\0  Multimedia\0    Expert\0        "
																 /*----------------++++++++++++++++----------------++++++++++++++++*/

#define TL_PREFS1									"General Preferences"
#define TL_PREFS2									"User Level:"
#define TL_PREFS3									"Startup Screen:"
#define TL_PREFS4									"Page"
#define TL_PREFS5									"Show menubar"
#define TL_PREFS6									"Close WorkBench"
#define TL_PREFS7									"Video format:"
#define TL_PREFS8									"Thumbnails:"
#define TL_PREFS9									"Small"
#define TL_PREFS10								"Large"

#define TL_PREFS11								"Page Preferences"
#define TL_PREFS12								"Import picture path:"
#define TL_PREFS13								"Import text path:"
#define TL_PREFS14								"Open document path:"
#define TL_PREFS15								"Save document path:"
#define TL_PREFS16								"Screen Size:"
#define TL_PREFS17								"Color Set:"
#define TL_PREFS18								"Screen Refresh"
#define TL_PREFS19								"Simple"
#define TL_PREFS20								"Smart"

#define TL_PREFS21								"Script Preferences"
#define TL_PREFS22								"Open script path"
#define TL_PREFS23								"Save script path"
#define TL_PREFS24								"Programmable time codes:"
#define TL_PREFS25								"F1:"
#define TL_PREFS26								"F2:"
#define TL_PREFS27								"F3:"
#define TL_PREFS28								"F4:"
#define TL_PREFS29								"F5:"
#define TL_PREFS30								"Display options:"
#define TL_PREFS31								"Show Days"
#define TL_PREFS32								"Show Prog"

#define TL_PREFS33								"Player Device Preferences"
#define TL_PREFS34								"Baud rate:"
#define TL_PREFS35								"Serial device:"
#define TL_PREFS36								"Unit:"
#define TL_PREFS37								"Player device:"

#define TL_PREFS38								"No device selected yet."

#define TEXT_SELECT_DEVICE				"Select a player device:"

#define TEXT_SELECT_PATH					"Select a path:"

#define UNKNOWN_SCREEN_ID					"Weird screen mode requested"

#define TL_CHUNK_MISSES						"This picture misses its %s header."

#define TL_CLIPDIR								"The clips directory misses so I will create one."
#define TL_NOCLIPDIR							"Unable to create clips directory!"

#define TL_DOC_TOO_LARGE					"The document is too large."

#define TL_DOC_UNREADABLE					"The document is unreadable."

#define TL_SCRIPT_TOO_LARGE				"The script is too large."

#define TL_SCRIPT_UNREADABLE			"The script is unreadable."

#define TL_NO_CM_MEMORY						"Not enough memory to hold color map."

#define TL_SERIOUS_ERROR					"A serious error occurred! Shall I save the script plus page and quit?"
#define TL_SE_1										"Save & Quit"
#define TL_SE_2										"Quit"

#define TL_ADDED_TEXT							" Please wait 10 seconds and fix this problem."

#define TL_LOW_MEM								"Memory shortage! Flush clipboard and undo buffers?"

#define TL_YES										"Yes"
#define TL_NO											"No"

#define LR_MODE										"Low-Res"
#define LRL_MODE									"Low-Res Lace"
#define HR_MODE										"High-Res"
#define HRL_MODE									"High-Res Lace"
#define S_MODE										"Super"
#define SL_MODE										"Super Lace"
#define P_MODE										"Prod."
#define PL_MODE										"Prod. Lace"
#define UNKNOWN_FORMAT						"Unknown Format"

#define TL_Distribute							"Between left and right\0 Between top and bottom\0 "
																 /*-------------------------+++++++++++++++++++++++++*/

#define TL_Center									"Horizontally\0 Vertically\0   "
																 /*---------------+++++++++++++++*/

#define TL_Distri1								"The Object Distributor"
#define TL_Distri2								"Distribute:"
#define TL_Distri3								"Center:"

#define TL_Dupli1									"The Object Duplicator"
#define TL_Dupli2									"Multiply"
#define TL_Dupli3									"times"
#define TL_Dupli4									"Add"
#define TL_Dupli5									"to x"
#define TL_Dupli6									"to y"

#define TEXT_GLOBLABS							"There is a global event which has no \
object. Shall I delete this global event?"

#define TEXT_NO_XAPP							"Unable to launch the %s xapp"

#define NO_TEXT_FILE							"Unable to read %s"

/******** E O F ********/
