//	File		: colormap.c
//	Uses		:
//	Date		: 11-12-1993
//	Author	: M. Luse Bitmapped graphics programming in C++ pag. 20,
//				: Updated and optimized C. Lieshout
// Desc		: Palette modification and color mapping
//

#include <exec/types.h>
#include <exec/memory.h>
#include	<clib/exec_protos.h>
#include <stdio.h>
#include <stdlib.h>
#include	<time.h>

typedef struct rgb
{
	UBYTE	red;
	UBYTE	green;
	UBYTE	blue;
	int dist;
} RGB;

int iscale( int i, int m, int d );
long dist3( int r1, int r2, int g1, int g2, int b1, int b2 );
/*long __saveds __asm dist3(	register __d0 int r1,
						register __d1 int r2,
						register __d2 int g1,
						register __d3 int g2,
						register __d4 int b1,
						register __d5 int b2 );
*/
/*void __asm xchg_rgb( register __a0 RGB *pal,
							register __d0 int m,
							register __d1 int n );
__asm int closest_rgb(	register __a0 RGB *x,
								register __a1 RGB *pal,
								register __d0 int npal );

*/
__asm void reduce_palette( register __a0 RGB *pal,
									register __d0 int npal,
									register __d1 int nsub );

__asm void smooth_colormap( register __a0 RGB *img,
											register __d0 int nimg,
											register __a1 RGB *dev,
											register __d1 int ndev );

/*void color_map( RGB *img, int nimg, RGB *dev, int ndev,
                int *cmap, int smooth_wt );
*/
void sort_pal( RGB *pal, int entries );
void random_pal( RGB *p, int entries );
void print_pal( RGB *p, int entries );
//void reduce_palette( RGB *pal, int npal, int nsub );
void copy_pal( RGB *sp, RGB *dp, int entries );

#define MAXCOL 2000
RGB palet[MAXCOL];
RGB devpal[MAXCOL];
int cmap[MAXCOL];

void main( int argc, char *argv[] )
{
	clock_t start,stop,mid;

	int scols = 16;
	int dcols = 8;
	int print = 0;
	srand( 100 );
	if( argc >= 2 )
		sscanf(argv[1],"%d",&scols );
	if( argc >= 3 )
		sscanf(argv[2],"%d",&dcols );
	if( argc >= 4 )
		print = 1;

	if( dcols > MAXCOL )
		dcols = MAXCOL -1;
	if( scols > MAXCOL )
		scols = MAXCOL -1;
	if( dcols < scols )
	{
		printf("Color reduction test\n");
		printf("Generating %d random colors\n",scols );
		random_pal( devpal, scols );
		copy_pal( devpal, palet, scols );
		printf("Reducing palette and sorting to %d colors\n",dcols );
		start = clock();
			reduce_palette( palet, scols, dcols );
			sort_pal( palet, dcols );
		mid = clock();
//		color_map( devpal, scols, palet, dcols, cmap, 5 );
		smooth_colormap( devpal, scols, palet, dcols );
		stop = clock();
		printf("Time = %d, after reduce %d\n",(100*( stop-start))/ CLOCKS_PER_SEC,(100*( mid-start))/ CLOCKS_PER_SEC );

		if( print )
			print_pal( palet, dcols );
	}
	else
		printf("No reduction\n");
}

int compare( const void *p, const void *q )
{
	return ((RGB *)p)->dist < ((RGB *)q)->dist ? -1 :
				((RGB *)p)->dist > ((RGB *)q)->dist ? 1 : 0;
}

//
// Sort palette with the distance to zero 
//
void sort_pal( RGB *pal, int entries )
{
	int i;
	for( i = 0; i < entries; i++ )
		pal[i].dist = dist3( 0,0,0,pal[i].red,pal[i].green,pal[i].blue );

	qsort( pal, entries, sizeof( RGB ), compare );

}

void copy_pal( RGB *sp, RGB *dp, int entries )
{
	int i;
	for( i = 0; i < entries; i++ )
	{
		dp[i].red = sp[i].red;
		dp[i].green = sp[i].green;
		dp[i].blue = sp[i].blue;
	}
}
void print_pal( RGB *p, int entries )
{
	int i;
	for( i = 0; i < entries; i++ )
		printf("c[%2d] = %d,%d,%d\n",i,p[i].red,p[i].green,p[i].blue);
}

void random_pal( RGB *p, int entries )
{
	int i,r;
	r = RAND_MAX / 255;

	for( i = 0; i < entries; i++ )
	{
		p[i].red =  rand() / r;
		p[i].green = rand() / r;
		p[i].blue = rand() / r;
	}
}

int iscale( int i, int m, int d )
{
	return ( i * m) / d;
}

long dist3( int r1, int r2, int g1, int g2, int b1, int b2 )
{
	int r,g,b;
	r = r1 - r2;
	g = g1 - g2;
	b = b1 - b2;
	return( r * r + g * g + b * b );
}

//..................swap two palette entries

/*void xchg_rgb( RGB *pal, int m, int n )
{
	RGB tmp;

   tmp.red = pal[m].red;
   tmp.green = pal[m].green;
   tmp.blue = pal[m].blue;
	tmp.dist = pal[m].dist;

   pal[m].red = pal[n].red;
   pal[m].green = pal[n].green;
   pal[m].blue = pal[n].blue;
	pal[m].dist = pal[n].dist;
   pal[n].red = tmp.red;
   pal[n].green = tmp.green;
   pal[n].blue = tmp.blue;
   pal[n].dist = tmp.dist;
}
*/

//..................determine closest rgb in a list

int closest_rgb( RGB *x, RGB *pal, int npal )
{
	long d,dmin;
	int i,closest = 0;

	dmin = dist3(	x->red, pal[0].red,
						x->green, pal[0].green,
						x->blue, pal[0].blue );

	for( i=1; i<npal; i++ )
	{
		d = dist3(	x->red, pal[i].red,
						x->green, pal[i].green,
						x->blue, pal[i].blue );

		if( d < dmin )
		{
			dmin = d;
			closest = i;
		}
	}
	return closest;
}


//..................find rgb in mid..max maximal to 0..mid-1

int maximal_rgb( RGB *pal, int mid, int max )
{
	long n = 1L << 30;  // a large number
	long dmin = n;
	int  i,j,maximal = mid;
	for( i=mid; i<max; i++ )
	{
		long dsum = 0;
		for( j=0; j<mid; j++ )
		{
			long d = dist3(	pal[i].red, pal[j].red,
									pal[i].green, pal[j].green,
									pal[i].blue, pal[j].blue );
			if( d == 0 )
			{
				dsum = n;
				break;
			}
			dsum += n / d;
		}
		pal[i].dist = dsum;

		if( dsum < dmin )
		{
			dmin = dsum;
			maximal = i;
		}
   }
   return maximal;
}

//
// There is one color added to the minimized colormap
// adjust the distances so that this new color is added
//
int maximal_rgb_opt( RGB *pal, int mid, int max )
{
	long lastcol = mid - 1;				// the just added color
	int red,green,blue;

	long dsum,d;
	long n = 1L << 30;  // a large number
	long dmin = n;
	int  i,maximal = mid;
	red = pal[lastcol].red;
	green = pal[lastcol].green;
	blue = pal[lastcol].blue;

	for( i=mid; i<max; i++ )
	{
		dsum = pal[i].dist;
		d = dist3(	pal[i].red, red,
						pal[i].green, green,
						pal[i].blue, blue );

		if( d == 0 )
			dsum = n;
		else
			dsum += n / d;

		pal[i].dist = dsum;

		if( dsum < dmin )
		{
			dmin = dsum;
			maximal = i;
		}
   }
   return maximal;
}

//..................compute maximally apart reduced palette
/*
void reduce_palette( RGB *pal, int npal, int nsub )
{
	RGB black = {0,0,0};
	int imin = 0;
	int  i;

	// find "blackest" entry as a starting point

	i = closest_rgb( &black, pal, npal );
	xchg_rgb( pal, i, imin++ );
	i = maximal_rgb( pal, imin, npal );		// get second color and init the distances

	// reorder palette so that first nsub entries
	// are maximally far apart

	while( imin < nsub )
	{
		xchg_rgb( pal, i, imin++ );
		i = maximal_rgb_opt( pal, imin, npal );
	}
}
*/

//..................compute color map, optionally smooth

void color_map( RGB *img, int nimg, RGB *dev, int ndev,
                int *cmap, int smooth_wt )
{
	int	i,*ccnt = 0;
	long	*rsum = 0,
			*gsum = 0,
			*bsum = 0;

	// smooth_wt > 0 enables averaging filter
	ccnt = (int *)AllocMem( sizeof( long ) * ndev , MEMF_PUBLIC );
	rsum = ( long *)AllocMem( sizeof( long ) * ndev * 3 , MEMF_PUBLIC );

	if( (ccnt == 0) || (rsum == 0) )
	{
		smooth_wt = 0;											// switch off avaraging
		if( ccnt ) FreeMem( ccnt, sizeof( long ) * ndev );
		if( rsum ) FreeMem( rsum, sizeof( long ) * ndev * 3 );
	}
	else
	{
		int n = smooth_wt - 1;

		gsum = rsum + ndev;
		bsum = gsum + ndev;

		// initialize weighted sums
		for( i = 0; i < ndev; i++ )
		{
			rsum[i] = dev[i].red; rsum[i] *= n;
			gsum[i] = dev[i].green;  gsum[i] *= n;
			bsum[i] = dev[i].blue;  bsum[i] *= n;
			ccnt[i] = n;
		}
	}
	
	// compute the color map

	for( i = 0; i < nimg; i++ )
	{
		int j = closest_rgb( &img[i], dev, ndev );
		cmap[i] = j;
		if( smooth_wt > 0 )
		{
			ccnt[j] += 1;
			rsum[j] += img[i].red;
			gsum[j] += img[i].green;
			bsum[j] += img[i].blue;
		}
	}

	// smooth the device palette
	if( smooth_wt > 0 )
	{
		for( i=0; i<ndev; i++ )
		{
			rsum[i] /= ccnt[i];
			dev[i].red = rsum[i];
			gsum[i] /= ccnt[i];
			dev[i].green = gsum[i];
			bsum[i] /= ccnt[i];
			dev[i].blue = bsum[i];
		}
	}
	if( ccnt ) FreeMem( ccnt, sizeof( long ) * ndev );
	if( rsum ) FreeMem( rsum, sizeof( long ) * ndev * 3 );
}

