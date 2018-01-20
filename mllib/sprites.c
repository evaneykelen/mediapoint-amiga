#include "mllib_includes.h"

/**** globals ****/

UWORD *GwaitPointer, *GcolorPicker, *GtoSprite, *Ghand;

/**** functions ****/

/******** SetSpritePtrs() ********/

void __saveds __asm LIBUA_SetSpritePtrs(	register __a0 UWORD *waitPointer,
																					register __a1 UWORD *colorPicker,
																					register __a2 UWORD *toSprite,
																					register __a3 UWORD *hand	)
{
	GwaitPointer	= waitPointer;
	GcolorPicker	= colorPicker;
	GtoSprite			= toSprite;
	Ghand					= hand;
}

/******** SetSprite() ********/
/* 
 * These sprite flavours are available: SPRITE_NORMAL
 *																			SPRITE_BUSY
 *																			SPRITE_COLORPICKER:
 *																			SPRITE_TOSPRITE
 *																			SPRITE_HAND
 */

void __saveds __asm LIBUA_SetSprite(	register __a0 struct Window *window,
																			register __d0 BYTE which	)
{
struct TagItem taglist[5];

	if (window==NULL)
		return;

	switch(which)
	{
		case SPRITE_NORMAL:
			ClearPointer(window);
			break;

		case SPRITE_BUSY:
			if ( GfxBase->LibNode.lib_Version >= 39 )
			{
				taglist[0].ti_Tag		= WA_BusyPointer;
				taglist[0].ti_Data	= TRUE;
				taglist[1].ti_Tag		= WA_PointerDelay;
				taglist[1].ti_Data	= TRUE;
				taglist[2].ti_Tag		= TAG_DONE;
				SetWindowPointerA(window, taglist);
			}
			else
				SetPointer(window, GwaitPointer, 16, 16, -8, -8);
			break;

		case SPRITE_COLORPICKER:
			SetPointer(window, GcolorPicker, 16, 16, -2, -10);
			break;

		case SPRITE_TOSPRITE:
			SetPointer(window, GtoSprite, 16, 16, -8, -5);
			break;

		case SPRITE_HAND:
			SetPointer(window, Ghand, 16, 16, -7, -8);
			break;
	}
}

/******** E O F ********/
