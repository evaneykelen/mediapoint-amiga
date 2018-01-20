//
// Test splining 2 oct 1993 C. Lieshout

#include <stdio.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <clib/graphics_protos.h>

struct POINT
{
	int x,y;
};

struct FPOINT
{
	float x,y;
};

//
// Draw a polygon
//
void draw_poly( struct RastPort *rp, struct POINT *points )
{
	int i;
	Move( rp, points[0].x, points[0].y );
	i = 1;
	while( points[i].x != -1000 )
	{
		Draw( rp, points[i].x,points[i].y );
		i++;
	}
	Draw( rp, points[0].x,points[0].y );

}

//
// Try to draw a splined polygon
// using procs from interactive 3D computer graphics page 147
//
void draw_spline( struct RastPort *rp, struct POINT *points, int N )
{
	float xa,xb,xc,xd;
	float ya,yb,yc,yd;
	float a0,a1,a2,a3;
	float b0,b1,b2,b3;
	float x,y;
	float t;
	int first;

	struct FPOINT tp[50];			// room for 48 points	
	int i,j,p;

	i = 0;
	while( points[i].x != -1000 )			// end variable
	{
		tp[i].x = points[i].x;				// just copy the points
		tp[i].y = points[i].y;
		i++;
	}

	p = i;										// number off points
	if( p < 2 )									// spline a line ???
		return;

	tp[i].x = points[0].x;					// first two points are the same as the last
	tp[i].y = points[0].y;
	i++;
	tp[i].x = points[1].x;
	tp[i].y = points[1].y;
	i++;
	tp[i].x = points[2].x;
	tp[i].y = points[2].y;

	first = 0;

	for( i = 1; i <= p; i++ )
	{
#if 0
		printf("\n");
		printf("spline -1 %d,%d\n",(int)tp[i-1].x,(int)tp[i-1].y );
		printf("spline    %d,%d\n",(int)tp[i].x,(int)tp[i].y );
		printf("spline +1 %d,%d\n",(int)tp[i+1].x,(int)tp[i+1].y );
		printf("spline +2 %d,%d\n",(int)tp[i+2].x,(int)tp[i+2].y );
#endif
		xa = tp[ i - 1 ].x;
		xb = tp[ i ].x;
		xc = tp[ i + 1 ].x;
		xd = tp[ i + 2 ].x;

		ya = tp[ i - 1 ].y;
		yb = tp[ i ].y;
		yc = tp[ i + 1 ].y;
		yd = tp[ i + 2 ].y;

		a3 = ( -xa + 3 * ( xb - xc ) + xd ) / 6.0;
		a2 = ( xa - 2 * xb + xc )/ 2.0;
		a1 = ( xc - xa )/ 2.0;
		a0 = ( xa + 4 * xb + xc ) / 6.0;

		b3 = ( -ya + 3 * ( yb - yc ) + yd ) / 6.0;
		b2 = ( ya - 2 * yb + yc )/ 2.0;
		b1 = ( yc - ya )/ 2.0;
		b0 = ( ya + 4 *  yb + yc ) / 6.0;

		if( first == 0 )
			Move( rp, (int)a0, (int)b0 );

		for( j = first; j <= N; j++ )
		{
			t = (float)j / (float)N;
			x = ((a3*t+a2)*t+a1)*t+a0;
			y = ((b3*t+b2)*t+b1)*t+b0;
			Draw( rp, (int)x, (int)y );
		}		
		first = 1;
	}
}

//
// Draw the line pointed at by points as a polygon
//
void draw_lines( struct RastPort *rp, struct POINT *points, int N )
{
	SetAPen( rp,1 );
	draw_poly( rp, points );

	SetAPen( rp,2 );
	draw_spline( rp, points, N  );
}
