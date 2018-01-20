//	File		: colormap.c
//	Uses		:
//	Date		: 11-12-1993
//	Author	: M. Luse Bitmapped graphics programming in C++ pag. 20,
//				: UPdated for C C. Lieshout
// Desc		: Palette modification and color mapping
//

#include <exec/types.h>
#include <exec/memory.h>
#include	<clib/exec_protos.h>
#include <stdio.h>

typedef struct rgb
{
	UBYTE	red;
	UBYTE	green;
	UBYTE	blue;
} RGB;

RGB pre[]={ {0,0,0},
				{1,1,1},
				{2,2,2},
				{3,3,3},
				{4,4,4},
				{5,5,5},
				{6,6,6},
				{7,7,7},
				{8,8,8},
				{9,9,9},
				{10,10,10},
				{11,11,11},
				{12,12,12},
				{13,13,13},
				{14,14,14},
				{15,15,15},
				{16,16,16}
			};


int iscale( int i, int m, int d );
long dist3( int r1, int r2, int g1, int g2, int b1, int b2 );
void fill_pal( RGB *p, int entries );
void print_pal( RGB *p, int entries );
void reduce_palette( RGB *pal, int npal, int nsub );
void copy_pal( RGB *sp, RGB *dp, int entries );
void color_map( RGB *img, int nimg, RGB *dev, int ndev,
                int *cmap, int smooth_wt );

void main( void )
{
	RGB palet[255];
	RGB devpal[255];
	int cm[255];

	printf("Color reduction test\n");
	fill_pal( devpal, 16 );
	print_pal( devpal, 16 );
	copy_pal( devpal, palet, 16 );


	reduce_palette( palet, 16, 8 );


	print_pal( palet, 8 );
	color_map( devpal, 16, palet, 8, cm, 5 );
	print_pal( palet, 8 );
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

void fill_pal( RGB *p, int entries )
{
	int i;
	for( i = 0; i < entries; i++ )
	{
		p[i].red = pre[i].red << 4 | pre[i].red ;
		p[i].green = pre[i].green << 4 | pre[i].green;
		p[i].blue = pre[i].blue << 4 | pre[i].blue;
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

void xchg_rgb( RGB *pal, int m, int n )
{
	RGB tmp;

   tmp.red = pal[m].red;
   tmp.green = pal[m].green;
   tmp.blue = pal[m].blue;
   pal[m].red = pal[n].red;
   pal[m].green = pal[n].green;
   pal[m].blue = pal[n].blue;
   pal[n].red = tmp.red;
   pal[n].green = tmp.green;
   pal[n].blue = tmp.blue;
}

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
		if( dsum < dmin )
		{
			dmin = dsum;
			maximal = i;
		}
   }
   return maximal;
}

//..................compute maximally apart reduced palette

void reduce_palette( RGB *pal, int npal, int nsub )
{
	RGB black = {0,0,0};
	int imin = 0;
	int  i;

	// find "blackest" entry as a starting point

	i = closest_rgb( &black, pal, npal );

	// reorder palette so that first nsub entries
	// are maximally far apart
	while( imin < nsub )
	{
		xchg_rgb( pal, i, imin++ );
		i = maximal_rgb( pal, imin, npal );
	}
}


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

