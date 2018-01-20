#include "nb:pre.h"

/**** externals ****/

extern struct CapsPrefs CPrefs;
extern struct Library *medialinkLibBase;
extern UBYTE **msgs;   
extern struct Window *pageWindow;
extern struct Screen *pageScreen;
extern struct Document pageDoc;

/**** functions ****/

/******** ReadColorPalette() ********/

BOOL ReadColorPalette(void)
{
TEXT filename[SIZE_FILENAME], fullPath[SIZE_FULLPATH];
BOOL retval;
struct IFF_FRAME iff;
ULONG IFF_ID;

	//SetStandardColors(pageWindow);

	retval = OpenAFile(	CPrefs.import_picture_Path, filename,
											msgs[Msg_OpenColormap-1], pageWindow,
											DIR_OPT_ANIM | DIR_OPT_ILBM | DIR_OPT_NOINFO, FALSE);
	if (!retval)
	{
		//ResetStandardColors(pageWindow);
		PaletteToFront();
		return(FALSE);
	}

	UA_MakeFullPath(CPrefs.import_picture_Path, filename, fullPath);

	FastInitIFFFrame(&iff);

	IFF_ID = FastOpenIFF(&iff, fullPath);

	if ( iff.Error || (IFF_ID!=ILBM && IFF_ID!=ANIM) )
	{
		FastCloseIFF(&iff);
		Message(msgs[Msg_NoColorMap-1], filename);
		//ResetStandardColors(pageWindow);
		PaletteToFront();
		return(FALSE);
	}

	FastParseChunk(&iff, CMAP);
	if (iff.Error != NULL)
	{
		FastCloseIFF(&iff);
		Message(msgs[Msg_NoColorMap-1], filename);
		//ResetStandardColors(pageWindow);
		PaletteToFront();
		return(FALSE);
	}

	CopyCMtoCM(iff.colorMap, pageScreen->ViewPort.ColorMap);

	FastCloseIFF(&iff);

	RefreshPalette();
	SetScreenToCM(pageScreen, pageScreen->ViewPort.ColorMap);
	PaletteToFront();

	return(TRUE);
}

/******** WriteColorPalette() ********/

BOOL WriteColorPalette(void)
{
TEXT filename[SIZE_FILENAME], path[SIZE_FULLPATH], fullPath[SIZE_FULLPATH];
int numCols,i;
struct FileHandle *FH;
ULONG len, rgb, gun;
UBYTE myByte;

	//SetStandardColors(pageWindow);

	strcpy(filename, pageDoc.title);
	strcat(filename, ".cols");
	strcpy(path, CPrefs.import_picture_Path);

	if (SaveAFile(path, filename, msgs[Msg_SaveColormap-1],
								pageWindow,
								DIR_OPT_ILBM | DIR_OPT_ANIM | DIR_OPT_NOINFO))
	{
		//ResetStandardColors(pageWindow);

		UA_MakeFullPath(path, filename, fullPath);

		FH = (struct FileHandle *)Open((STRPTR)fullPath, (LONG)MODE_NEWFILE);
		if (FH!=NULL)
		{
			numCols = GetNumberOfModeColors(CPrefs.PageScreenModes, CPrefs.PageScreenDepth);
			if ( CPrefs.PageScreenModes & EXTRA_HALFBRITE )
				numCols=32;

			/**** write out a small IFF file containing only FORM, ILBM and CMAP ****/

			Write((BPTR)FH, "FORM", 4L);
			len = 12 + 3*numCols;		// <--- here
			Write((BPTR)FH, &len, 4L);

			Write((BPTR)FH, "ILBM", 4L);	// 4 bytes

			Write((BPTR)FH, "CMAP", 4L);	// 4 bytes
			len = 3*numCols;
			Write((BPTR)FH, &len, 4L);		// 4 bytes	-> total 3*4 = 12 --> see equation above ^

			for(i=0; i<numCols; i++)
			{
				/**** returned rgb is formatted as 0x00rrggbb ****/

				rgb = GetColorCM32(pageScreen->ViewPort.ColorMap, i);

				gun = rgb >> 16;		// gun is now 0x000000rr
				gun &= 0x000000ff;	// mask out shit
				myByte = (UBYTE)gun;
				Write((BPTR)FH, &myByte, 1L);

				gun = rgb >> 8;			// gun is now 0x000000gg
				gun &= 0x000000ff;
				myByte = (UBYTE)gun;
				Write((BPTR)FH, &myByte, 1L);

				gun = rgb;
				gun &= 0x000000ff;	// gun is now 0x000000bb
				myByte = (UBYTE)gun;
				Write((BPTR)FH, &myByte, 1L);
			}

			Close((BPTR)FH);
		}
	}
	//else
	//	ResetStandardColors(pageWindow);

	PaletteToFront();

	return(TRUE);
}

/******** E O F ********/
