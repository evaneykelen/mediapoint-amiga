#include "mllib_includes.h"

/**** functions ****/

/*******************************************************************/
/*
 *   PUBLIC FUNCTIONS
 *
 *******************************************************************/

/******** DrawVector() ********/

void __saveds __asm LIBUA_DrawVector(	register __a0 struct Window *window,
																			register __a1 struct VectorRecord *VR)
{
	switch(VR->type)
	{
		case HI_LINE:
		case LO_LINE:
		case HI_PATTERN_LINE:
		case LO_PATTERN_LINE:
			DrawVectorLine(window, VR);
			break;
		case HI_AREA:
		case LO_AREA:
			DrawVectorArea(window, VR);
			break;
	}
}

/******** DrawVectorList() ********/

void __saveds __asm LIBUA_DrawVectorList(	register __a0 struct Window *window,
																					register __a1 struct VectorRecord *VR)
{
register int i=0;

	while(VR[i].x1 != -1)
	{
		LIBUA_DrawVector(window, &VR[i]);
		i++;
	}
}

/******** DoubleVectorDimensions() ********/

void __saveds __asm LIBUA_DoubleVectorDimensions(register __a0 struct VectorRecord *VR)
{
register int i=0;

	while(VR[i].x1 != -1)
	{
		VR[i].y1 *= 2; 
		VR[i].y2 *= 2; 
		i++;
	}
}

/******** HalveVectorDimensions() ********/

void __saveds __asm LIBUA_HalveVectorDimensions(register __a0 struct VectorRecord *VR)
{
register int i=0;

	while(VR[i].x1 != -1)
	{
		VR[i].y1 /= 2; 
		VR[i].y2 /= 2; 
		i++;
	}
}

/*******************************************************************/
/*
 *   PRIVATE FUNCTIONS
 *
 *******************************************************************/

/******** DrawVectorLine() ********/

void DrawVectorLine(struct Window *window, struct VectorRecord *VR)
{
	SetBPen(window->RPort, AREA_PEN);

	if (VR->type == HI_LINE)
		my_SetAPen(window, HI_PEN);
	else if (VR->type == LO_LINE)
		my_SetAPen(window, LO_PEN);
	else if (VR->type == HI_PATTERN_LINE)
	{
		my_SetAPen(window, HI_PEN);
		SetDrPt(window->RPort, 0xaaaa);
	}
	else if (VR->type == LO_PATTERN_LINE)
	{
		my_SetAPen(window, LO_PEN);
		SetDrPt(window->RPort, 0xaaaa);
	}

	/* draw box */

	Move(window->RPort, VR->x1, VR->y1);
	Draw(window->RPort, VR->x2, VR->y1);
	Draw(window->RPort, VR->x2, VR->y2);
	Draw(window->RPort, VR->x1, VR->y2);
	Draw(window->RPort, VR->x1, VR->y1);

	Move(window->RPort, VR->x1+1, VR->y1);
	Draw(window->RPort, VR->x1+1, VR->y2);

	Move(window->RPort, VR->x2-1, VR->y1);
	Draw(window->RPort, VR->x2-1, VR->y2);

	/* thicken top and bottom border on lace screens */

	if (window->WScreen->ViewPort.Modes & LACE)
	{
		Move(window->RPort, VR->x1, VR->y1+1);
		Draw(window->RPort, VR->x2, VR->y1+1);

		Move(window->RPort, VR->x1, VR->y2-1);
		Draw(window->RPort, VR->x2, VR->y2-1);
	}

	SetDrPt(window->RPort, 0xffff);
}

/******** DrawVectorArea() ********/

void DrawVectorArea(struct Window *window, struct VectorRecord *VR)
{
	if (VR->type == HI_AREA)
		my_SetAPen(window, HI_PEN);
	else
		my_SetAPen(window, LO_PEN);

	RectFill(window->RPort, VR->x1, VR->y1, VR->x2, VR->y2);
}

/******** E O F ********/
