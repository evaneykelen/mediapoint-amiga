#include "math.h"

main()
{
int i;
double in,db,fac;

	for(i=100; i>=1; i--)
	{
		db = log10((float)i);

		db = db * -1;

		db = db + 2.0;

		db = db * 32.0;

		printf("%d %d\n",i,(int)(db+0.5));
	}
}
