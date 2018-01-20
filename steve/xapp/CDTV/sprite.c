#include "nb:pre.h"
#include "spritedata.h"

/**** globals ****/

static struct Window *spriteWindow=NULL;

/**** functions ****/

/******** ChangeSpriteImage() ********/

void ChangeSpriteImage(int number)
{
	if (spriteWindow!=NULL)
		ClearSpriteImage();

	if (number==SPRITE_NORMAL)
	{
		ClearSpriteImage();
		return;
	}

	Forbid();
	spriteWindow = IntuitionBase->ActiveWindow;
	Permit();

	if (spriteWindow==NULL)
		return;

	switch(number)
	{
		case SPRITE_BUSY:
			SetPointer(spriteWindow, WaitPointer, 16, 16, -8, -8);
			break;

		case SPRITE_COLORPICKER:
			SetPointer(spriteWindow, colorPicker, 16, 16, -2, -10);
			break;

		case SPRITE_TOSPRITE:
			SetPointer(spriteWindow, toSprite, 16, 16, -8, -5);
			break;

		case SPRITE_HAND:
			SetPointer(spriteWindow, hand, 16, 16, -7, -8);
			break;
	}
}

/******** ChangeSpriteImage() ********/

void ClearSpriteImage(void)
{
struct Window *wdw;

	/**** when this routine is called and no windows are available	****/
	/**** (i.e. no window is active) then don't change the pointer	****/

	Forbid();
	wdw = IntuitionBase->ActiveWindow;
	Permit();

	if (wdw!=NULL && spriteWindow != NULL)
		ClearPointer(spriteWindow);

	spriteWindow=NULL;
}

/******** E O F ********/
