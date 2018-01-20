#include "nb:pre.h"

/**** externals ****/

extern struct CapsEventData CED;
extern struct CapsPrefs CPrefs;
extern struct MsgPort *capsPort;
extern struct Library *medialinkLibBase;

/******** DrawVector() ********/

void DrawVector(struct Window *window, struct VectorRecord *VR)
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

void DrawVectorList(struct Window *window, struct VectorRecord *VR)
{
register int i=0;

	while(VR[i].x1 != -1)
	{
		DrawVector(window, &VR[i]);
		i++;
	}
}

/******** DrawVectorLine() ********/

void DrawVectorLine(struct Window *window, struct VectorRecord *VR)
{
	SetBPen(window->RPort, AREA_PEN);

	if (VR->type == HI_LINE)
		SetAPen(window->RPort, HI_PEN);
	else if (VR->type == LO_LINE)
		SetAPen(window->RPort, LO_PEN);
	else if (VR->type == HI_PATTERN_LINE)
	{
		SetAPen(window->RPort, HI_PEN);
		SetDrPt(window->RPort, 0xaaaa);
	}
	else if (VR->type == LO_PATTERN_LINE)
	{
		SetAPen(window->RPort, LO_PEN);
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
		SetAPen(window->RPort, HI_PEN);
	else
		SetAPen(window->RPort, LO_PEN);

	RectFill(window->RPort, VR->x1, VR->y1, VR->x2, VR->y2);
}

/******** DoubleVectorDimensions() ********/

void DoubleVectorDimensions(struct VectorRecord *VR)
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

void HalveVectorDimensions(struct VectorRecord *VR)
{
register int i=0;

	while(VR[i].x1 != -1)
	{
		VR[i].y1 /= 2; 
		VR[i].y2 /= 2; 
		i++;
	}
}

/******** E O F ********/
