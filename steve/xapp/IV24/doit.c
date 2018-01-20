#include "nb:pre.h"
#include "fyedefs.h"
#include "minc:types.h"
#include "minc:Errors.h"
#include "minc:process.h"
#include "protos.h"
#include <fye/fye.h>
#include <fye/fyebase.h>
#include <clib/fye_protos.h>
#include <pragmas/fye.h>

/**** functions ****/

/******** XappDoIt() ********/

BOOL XappDoIt(PROCESSINFO *ThisPI, struct IV24_actions *IV24)
{
ULONG	error;
struct PipData Pip;
UBYTE data[10],lo,hi;
BOOL closePip, retval;
int speed,x,y,sizeX,sizeY,scaleX,scaleY,mode,pipMode,i;
UBYTE byteVal1, byteVal2;
struct FyeBase *FyeBase;
BOOL obtained=TRUE;
BOOL libopen=TRUE;

	if ((FyeBase=(struct FyeBase *)OpenLibrary("fye.library",0))==NULL)
		return(FALSE);

	if (error=ObtainFyeBoard())
	{
		CloseLibrary((struct Library *)FyeBase);
		return(FALSE);
	}

	if ( ThisPI->pi_Arguments.ar_Worker.aw_ExtraData[0] != '\0' )
		GetExtraData(ThisPI,IV24);
	else
	{
		ReleaseFyeBoard();
		CloseLibrary((struct Library *)FyeBase);
		return(FALSE);
	}

	/**** take actions ****/

	if ( IV24->action == ACTION_PIP )
	{
		if (IV24->pip_size==0)				// small
		{
			sizeX		= 375;
			sizeY		= 291;
			scaleX	= 2 << 4;
			scaleY	= 2 << 4;
		}
		else if (IV24->pip_size==1)	// smaller
		{
			sizeX		= 249;
			sizeY		= 193;
			scaleX	= 3 << 4;
			scaleY	= 3 << 4;
		}
		else if (IV24->pip_size==2)	// smallest
		{
			sizeX		= 187;
			sizeY		= 142;
			scaleX	= 4 << 4;
			scaleY	= 4 << 4;
		}
		else if (IV24->pip_size==3)	// tiny
		{
			sizeX		= 125;
			sizeY		= 95;
			scaleX	= 6 << 4;
			scaleY	= 6 << 4;
		}
		pipMode		= IV24->pip_display_mode;
		x					= IV24->pip_x;
		y					= IV24->pip_y;
		speed			= (IV24->pip_speed+1) * 10;
		closePip	= IV24->pip_close;
		mode			= IV24->pip_action;

		Pip.PositionX	= x;
		Pip.PositionY	= y;
		Pip.SizeX			= sizeX;
		Pip.SizeY			= sizeY;
		Pip.OffsetX		= 0;
		Pip.OffsetY		= 0;
		Pip.ScaleX		= scaleX;
		Pip.ScaleY		= scaleY;

		/**** read current PIP size (6 if not on screen), answer in data[0] ****/

		error = ReadFyeData(FYE_READ_PIPLENGTH_LOW_X, data, NULL);
		if (error!=0)
		{
			ReleaseFyeBoard();
			CloseLibrary((struct Library *)FyeBase);
			return(FALSE);
		}

		if ( mode==PIP_ACTION_PLACE )
		{
			Pip.PositionX	= x;
			Pip.PositionY	= y;
			if ( !IV_OpenPip(FyeBase, &Pip, pipMode) )
			{
				ReleaseFyeBoard();
				CloseLibrary((struct Library *)FyeBase);
				return(FALSE);
			}
		}
		else if ( mode==PIP_ACTION_MOVEDOWN )
		{
			/** This can mean two things: move EXISTING PIP down or move down		**/
			/** from underneath border, along x, to y (with y is top of window)	**/
	
			if (data[0]==6)	// PIP not on screen, fly from underneath top border
			{
				if ( !IV_FlyPipFromTop(FyeBase, &Pip, x, y, speed, sizeY, pipMode) )
				{
					ReleaseFyeBoard();
					CloseLibrary((struct Library *)FyeBase);
					return(FALSE);
				}
			}
			else	// PIP on screen, get y position
			{
				error = ReadFyeData(FYE_READ_PIPSTART_LOW_Y, &lo, NULL);
				if (error!=0)
				{
					ReleaseFyeBoard();
					CloseLibrary((struct Library *)FyeBase);
					return(FALSE);
				}
	
				error = ReadFyeData(FYE_READ_PIPSTART_HIGH_Y, &hi, NULL);
				if (error!=0)
				{
					ReleaseFyeBoard();
					CloseLibrary((struct Library *)FyeBase);
					return(FALSE);
				}
	
				Pip.PositionX	= x;
				Pip.PositionY	= lo+(hi<<8);
	
				if ( !IV_FlyPipDown(FyeBase, &Pip, x, 600, speed) )
				{
					ReleaseFyeBoard();
					CloseLibrary ((struct Library *) FyeBase);
					return(FALSE);
				}
			}
		}
		else if ( mode==PIP_ACTION_MOVEUP )
		{
			/** This can mean two things: move EXISTING PIP up or move up					**/
			/** to underneath border, along x, to y (with y is bottom of window)	**/

			if (data[0]==6)	// PIP not on screen, fly from underneath bottom border
			{
				if ( !IV_FlyPipFromBottom(FyeBase, &Pip, x, y, speed, sizeY, pipMode) )
				{
					ReleaseFyeBoard();
					CloseLibrary((struct Library *)FyeBase);
					return(FALSE);
				}
			}
			else	// PIP on screen, get y position
			{
				error = ReadFyeData(FYE_READ_PIPSTART_LOW_Y, &lo, NULL);
				if (error!=0)
				{
					ReleaseFyeBoard();
					CloseLibrary((struct Library *)FyeBase);
					return(FALSE);
				}

				error = ReadFyeData(FYE_READ_PIPSTART_HIGH_Y, &hi, NULL);
				if (error!=0)
				{
					ReleaseFyeBoard();
					CloseLibrary((struct Library *)FyeBase);
					return(FALSE);
				}

				Pip.PositionX	= x;
				Pip.PositionY	= lo+(hi<<8);

				if ( !IV_FlyPipUp(FyeBase, &Pip, x, 0, speed) )	// 0 was 28
				{
					ReleaseFyeBoard();
					CloseLibrary((struct Library *)FyeBase);
					return(FALSE);
				}
			}
		}

		if ( closePip )
		{
			/**** remove PIP out of sight ****/

			if ( !IV_ClosePip(FyeBase) )
			{
				ReleaseFyeBoard();
				CloseLibrary((struct Library *)FyeBase);
				return(FALSE);
			}

			if ( !IV_MovePip(FyeBase, &Pip, 8, 28) )
			{
				ReleaseFyeBoard();
				CloseLibrary((struct Library *)FyeBase);
				return(FALSE);
			}

			Pip.PositionX	= 8;
			Pip.PositionY	= 28;
			Pip.SizeX = 6;
			Pip.SizeY = 2;
			if ( !IV_OpenPip(FyeBase, &Pip, pipMode) )
			{
				ReleaseFyeBoard();
				CloseLibrary((struct Library *)FyeBase);
				return(FALSE);
			}
		}
	}
	else if ( IV24->action == ACTION_FSV )
	{
		if ( IV24->fsv_display_mode == 0 )
			byteVal2 = 0x00;
		else if ( IV24->fsv_display_mode == 1 )
			byteVal2 = 0x02;
		else if ( IV24->fsv_display_mode == 2 )
			byteVal2 = 0x04;
		else if ( IV24->fsv_display_mode == 3 )
			byteVal2 = 0x05;

		error = ReadFyeData(FYE_READ_PIXELSWITCH, &byteVal1, 0);
		if (error!=0)
		{
			ReleaseFyeBoard();
			CloseLibrary((struct Library *)FyeBase);
			return(FALSE);
		}

		/**** reset to default ****/

		if (SendFyeCmd(FYE_RECALL_USER))
		{
			ReleaseFyeBoard();
			CloseLibrary((struct Library *)FyeBase);
			return(FALSE);
		}

		error = SendFyeData(FYE_WRITE_PIXELSWITCH, (byteVal1 & 0xf0) | byteVal2, 0);
		if (error!=0)
		{
			ReleaseFyeBoard();
			CloseLibrary((struct Library *)FyeBase);
			return(FALSE);
		}
	}
	else if ( IV24->action == ACTION_FB )
	{
		ReleaseFyeBoard();
		CloseLibrary((struct Library *)FyeBase);
		obtained=FALSE;
		libopen=FALSE;
		DisplayFye(IV24->fileName,IV24->delay);
	}
	else if ( IV24->action == ACTION_CP )
	{
		/**** reset to default ****/

		if (SendFyeCmd(FYE_RECALL_USER))
		{
			ReleaseFyeBoard();
			CloseLibrary((struct Library *)FyeBase);
			return(FALSE);
		}

		/**** hiscan/videoscan ****/

		if ( IV24->misc_output[0]==SCAN_HISCAN )
			byteVal1 = FYE_HISCAN;
		else
			byteVal1 = FYE_VIDEOSCAN;

		if (SendFyeCmd(byteVal1))
		{
			ReleaseFyeBoard();
			CloseLibrary((struct Library *)FyeBase);
			return(FALSE);
		}

		/**** composite ****/

		if ( IV24->comp_output[0]==MODE_AMIGA )
			byteVal1 = 0x0;
		else if ( IV24->comp_output[0]==MODE_KEYER )
		{
			if ( IV24->comp_output[1]==1 && IV24->comp_output[2]==1 )
				byteVal1 = 0x7;		// doesn't exist in fact!
			else if ( IV24->comp_output[1]==1 && IV24->comp_output[2]==2 )
				byteVal1 = 0x5;		// doesn't exist in fact!
			else if ( IV24->comp_output[1]==2 && IV24->comp_output[2]==1 )
				byteVal1 = 0x7;
			else if ( IV24->comp_output[1]==2 && IV24->comp_output[2]==2 )
				byteVal1 = 0x5;
		}
		else if ( IV24->comp_output[0]==MODE_EXTERN )
			byteVal1 = 0x1;

		byteVal1 <<= 4;

		/**** RGB ****/

		if ( IV24->rgb_output[0]==MODE_AMIGA )
			byteVal2 = 0x0;
		else if ( IV24->rgb_output[0]==MODE_KEYER )
		{
			if ( IV24->rgb_output[1]==1 && IV24->rgb_output[2]==1 )
				byteVal2 = 0xb;
			else if ( IV24->rgb_output[1]==1 && IV24->rgb_output[2]==2 )
				byteVal2 = 0xa;		// doesn't exist in fact!
			else if ( IV24->rgb_output[1]==2 && IV24->rgb_output[2]==1 )
				byteVal2 = 0x9;
			else if ( IV24->rgb_output[1]==2 && IV24->rgb_output[2]==2 )
				byteVal2 = 0x8;

			if ( IV24->misc_output[1]==COLORS_4096 )
				byteVal2 += 4;
		}
		else if ( IV24->rgb_output[0]==MODE_EXTERN )
			byteVal2 = 0x3;

		error = SendFyeData(FYE_WRITE_PIXELSWITCH, byteVal1 | byteVal2, 0);
		if (error!=0)
		{
			ReleaseFyeBoard();
			CloseLibrary((struct Library *)FyeBase);
			return(FALSE);
		}

		/**** bypass ****/

		if ( IV24->misc_output[2] )
			byteVal1 = FYE_BYPASS_ON;
		else
			byteVal1 = FYE_BYPASS_OFF;

		if (SendFyeCmd(byteVal1))
		{
			ReleaseFyeBoard();
			CloseLibrary((struct Library *)FyeBase);
			return(FALSE);
		}
	}
	else if ( IV24->action == ACTION_CK )
	{
		if ( IV24->comp_from < IV24->comp_to )
		{
			if ( IV24->comp_fade==COMPFADE_AMIGA )
			{
				for(i=IV24->comp_from; i<=IV24->comp_to; i++)
				{
					retval = IV_SetAmigaCompKeyer(FyeBase, (UBYTE)i);
					if ( !retval )
						break;
				}
			}
			else if ( IV24->comp_fade==COMPFADE_EXT )
			{
				for(i=IV24->comp_from; i<=IV24->comp_to; i++)
				{
					retval = IV_SetExtCompKeyer(FyeBase, (UBYTE)i);
					if ( !retval )
						break;
				}
			}
			else if ( IV24->comp_fade==COMPFADE_CROSS )
			{
				for(i=IV24->comp_from; i<=IV24->comp_to; i+=2)
				{
					retval = IV_SetAmigaCompKeyer(FyeBase, IV24->comp_to-(UBYTE)i);
					if ( !retval )
						break;
				}
				for(i=IV24->comp_from; i<=IV24->comp_to; i+=2)
				{
					retval = IV_SetExtCompKeyer(FyeBase, (UBYTE)i);
					if ( !retval )
						break;
				}
			}
		}
		else if ( IV24->comp_from > IV24->comp_to )
		{
			if ( x == IV24->comp_fade==COMPFADE_AMIGA )
			{
				for(i=IV24->comp_from; i>=IV24->comp_to; i--)
				{
					retval = IV_SetAmigaCompKeyer(FyeBase, (UBYTE)i);
					if ( !retval )
						break;
				}
			}
			else if ( x == IV24->comp_fade==COMPFADE_EXT )
			{
				for(i=IV24->comp_from; i>=IV24->comp_to; i--)
				{
					retval = IV_SetExtCompKeyer(FyeBase, (UBYTE)i);
					if ( !retval )
						break;
				}
			}
			else if ( x == IV24->comp_fade==COMPFADE_CROSS )
			{
				for(i=IV24->comp_from; i>=IV24->comp_to; i-=2)
				{
					retval = IV_SetAmigaCompKeyer(FyeBase, IV24->comp_to-(UBYTE)i);
					if ( !retval )
						break;
				}
				for(i=IV24->comp_from; i>=IV24->comp_to; i-=2)
				{
					retval = IV_SetExtCompKeyer(FyeBase, (UBYTE)i);
					if ( !retval )
						break;
				}
			}
		}
		else if ( IV24->comp_from == IV24->comp_to )
		{
			if ( x == IV24->comp_fade==COMPFADE_AMIGA )
				IV_SetAmigaCompKeyer(FyeBase, (UBYTE)IV24->comp_from);
			else if ( x == IV24->comp_fade==COMPFADE_EXT )
				IV_SetExtCompKeyer(FyeBase, (UBYTE)IV24->comp_from);
			else if ( x == IV24->comp_fade==COMPFADE_CROSS )
			{
				IV_SetAmigaCompKeyer(FyeBase, (UBYTE)IV24->comp_from);
				IV_SetExtCompKeyer(FyeBase, (UBYTE)IV24->comp_from);
			}
		}
	}
	else if ( IV24->action == ACTION_IPF )
	{
		byteVal1 = FYE_INTERNAL_PIP_FREEZE;
		if (SendFyeCmd(byteVal1))
		{
			ReleaseFyeBoard();
			CloseLibrary((struct Library *)FyeBase);
			return(FALSE);
		}
	}
	else if ( IV24->action == ACTION_EPF )
	{
		byteVal1 = FYE_EXTERNAL_PIP_FREEZE;
		if (SendFyeCmd(byteVal1))
		{
			ReleaseFyeBoard();
			CloseLibrary((struct Library *)FyeBase);
			return(FALSE);
		}
	}
	else if ( IV24->action == ACTION_FA )
	{
		byteVal1 = FYE_FREEZE;
		if (SendFyeCmd(byteVal1))
		{
			ReleaseFyeBoard();
			CloseLibrary((struct Library *)FyeBase);
			return(FALSE);
		}
	}
	else if ( IV24->action == ACTION_UA )
	{
		byteVal1 = FYE_UNFREEZE;
		if (SendFyeCmd(byteVal1))
		{
			ReleaseFyeBoard();
			CloseLibrary((struct Library *)FyeBase);
			return(FALSE);
		}
	}

	if ( obtained )
		ReleaseFyeBoard();

	if ( libopen )
		CloseLibrary ((struct Library *)FyeBase);

	return(TRUE);
}

/******** IV_OpenPip() ********/
/*
 * SizeX and SizeY:   375,291  249,193,  187,142,  125,95
 * ScaleX and ScaleY: 2        3         4         6
 *
 * Mode: 0 full screen video, amiga in pip
 *       1 full screen amiga, video in pip
 *       2 fsv, fsa, amiga in pip
 *       3 fsv tru color 0, fsa, amiga in pip
 *       4 fsa, video in pip, amiga in pip
 *       5 fsa, amiga in pip, video tru pip color 0
 *
 */

BOOL IV_OpenPip(struct FyeBase *FyeBase, struct PipData *pip, int mode)
{
ULONG ModeTable[6]= { MODEPIPAMIGA, MODEPIPEXTERN, MODEPIPAMIGA+MODEPIPKEYERPOSITIVE,
								      MODEPIPAMIGA+MODEPIPKEYERNEGATIVE,
		      						MODEPIPEXTERN+MODEPIPKEYERPOSITIVE,
		      						MODEPIPEXTERN+MODEPIPKEYERNEGATIVE };

	if ( FyePip(pip, ENABLEPIP | ModeTable[mode], NULL, NULL) )
		return(FALSE);
	else
		return(TRUE);
}

/******** IV_ClosePip() ********/

BOOL IV_ClosePip(struct FyeBase *FyeBase)
{
	if ( FyePip(NULL, DISABLEPIP, NULL, NULL) )
		return(FALSE);
	else
		return(TRUE);
}

/******** IV_MovePip() ********/

BOOL IV_MovePip(struct FyeBase *FyeBase, struct PipData *pip, int x, int y)
{
	pip->PositionX = x;
	pip->PositionY = y;
	if ( FyePip(pip, UPDATEPIPPOSITION, NULL, NULL) )
		return(FALSE);
	else
		return(TRUE);
}

/******** IV_FlyPipUp() ********/

BOOL IV_FlyPipUp(struct FyeBase *FyeBase, struct PipData *pip, int x, int y, int addY)
{
	while( pip->PositionY >= 28 )
	{
		if ( FyePip(pip, UPDATEPIPPOSITION | UPDATEPIPOFFSET | UPDATEPIPSIZE, NULL, NULL) )
			return(FALSE);

		pip->PositionY -= addY;

		if ( pip->PositionY <= y )
			return(TRUE);
	}

	pip->PositionY = 28;
	pip->SizeY -= addY;
	pip->OffsetY += addY;

	while( pip->SizeY >= 2 )
	{
		if ( FyePip(pip, UPDATEPIPPOSITION | UPDATEPIPOFFSET | UPDATEPIPSIZE, NULL, NULL) )
			return(FALSE);

		pip->SizeY -= addY;

		pip->OffsetY += addY;
	}

	pip->SizeY = 2;
	pip->OffsetY = 0;

	if ( FyePip(pip, UPDATEPIPSIZE | UPDATEPIPOFFSET, NULL, NULL) )
		return(FALSE);

	return(TRUE);
}

/******** IV_FlyPipDown() ********/

BOOL IV_FlyPipDown(struct FyeBase *FyeBase, struct PipData *pip, int x, int y, int addY)
{
	while( pip->PositionY < (600-pip->SizeY-1) )
	{
		if ( FyePip(pip, UPDATEPIPPOSITION | UPDATEPIPOFFSET | UPDATEPIPSIZE, NULL, NULL) )
			return(FALSE);

		pip->PositionY += addY;

		if ( pip->PositionY>=y )
			return(TRUE);
	}

	pip->SizeY -= addY;

	while( pip->PositionY < y )
	{
		if ( FyePip(pip, UPDATEPIPPOSITION | UPDATEPIPOFFSET | UPDATEPIPSIZE, NULL, NULL) )
			return(FALSE);

		pip->SizeY -= addY;
		if ( pip->SizeY < 2 )
			pip->SizeY = 2;

		pip->PositionY += addY;

		if ( pip->PositionY>=y )
			return(TRUE);
	}

	pip->SizeY = 2;
	pip->PositionY = 600;

	if ( FyePip(pip, UPDATEPIPSIZE | UPDATEPIPPOSITION, NULL, NULL) )
		return(FALSE);

	return(TRUE);
}

/******** IV_FlyPipFromTop() ********/

BOOL IV_FlyPipFromTop(struct FyeBase *FyeBase, struct PipData *pip, int x, int y,
											int addY,	int height, int Mode)
{
ULONG ModeTable[6]= { MODEPIPAMIGA, MODEPIPEXTERN, MODEPIPAMIGA+MODEPIPKEYERPOSITIVE,
								      MODEPIPAMIGA+MODEPIPKEYERNEGATIVE,
		      						MODEPIPEXTERN+MODEPIPKEYERPOSITIVE,
		      						MODEPIPEXTERN+MODEPIPKEYERNEGATIVE };
int oriY;

	oriY = y;

	pip->PositionX = x;
	pip->PositionY = 28;
	pip->SizeY = 2;

	if ( FyePip(pip, UPDATEPIPPOSITION | ENABLEPIP | ModeTable[Mode], NULL, NULL) )
		return(FALSE);

	pip->OffsetY = height-addY;

	while( pip->SizeY < height )
	{
		if ( FyePip(pip, UPDATEPIPPOSITION | UPDATEPIPSIZE | UPDATEPIPOFFSET, NULL, NULL) )
			return(FALSE);

		pip->SizeY += addY;
		pip->OffsetY -= addY;
		if ( pip->OffsetY < 0 )
			pip->OffsetY = 0;
	}

	pip->SizeY = height;
	pip->OffsetY = 0;

	y = oriY;

	return ( IV_FlyPipDown(FyeBase, pip, x, y, addY) );
}

/******** IV_FlyPipFromBottom() ********/

BOOL IV_FlyPipFromBottom(	struct FyeBase *FyeBase, struct PipData *pip, int x, int y,
													int addY,	int height, int Mode)
{
ULONG ModeTable[6]= { MODEPIPAMIGA, MODEPIPEXTERN, MODEPIPAMIGA+MODEPIPKEYERPOSITIVE,
								      MODEPIPAMIGA+MODEPIPKEYERNEGATIVE,
		      						MODEPIPEXTERN+MODEPIPKEYERPOSITIVE,
		      						MODEPIPEXTERN+MODEPIPKEYERNEGATIVE };
int oriY;

	oriY = y;

	pip->PositionX = x;
	pip->PositionY = 600;
	pip->SizeY = 2;

	if ( FyePip(pip, UPDATEPIPPOSITION | ENABLEPIP | ModeTable[Mode], NULL, NULL) )
		return(FALSE);

	while( pip->SizeY < height )
	{
		if ( FyePip(pip, UPDATEPIPPOSITION | UPDATEPIPOFFSET | UPDATEPIPSIZE, NULL, NULL) )
			return(FALSE);

		pip->PositionY -= addY;
		pip->SizeY += addY;
	}

	pip->SizeY = height;

	y = oriY;

	return ( IV_FlyPipUp(FyeBase, pip, x, y, addY) );
}

/******** IV_SetAmigaCompKeyer() ********/

BOOL IV_SetAmigaCompKeyer(struct FyeBase *FyeBase, UBYTE val)
{
ULONG error;

	error = SendFyeData(FYE_WRITE_AMIGACOMPKEYER, val, 0);
	if (error!=0)
		return(FALSE);
	else
		return(TRUE);
}

/******** IV_SetExtCompKeyer() ********/

BOOL IV_SetExtCompKeyer(struct FyeBase *FyeBase, UBYTE val)
{
ULONG error;

	error = SendFyeData(FYE_WRITE_EXTCOMPKEYER, val, 0);
	if (error!=0)
		return(FALSE);
	else
		return(TRUE);
}

/******** E O F ********/
