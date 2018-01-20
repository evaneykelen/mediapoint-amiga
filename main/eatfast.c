#include <exec/exec.h>
#include <exec/types.h>

int main(int argc, char **argv)
{
int val;
long size;
char *ptr;
char ch[10];

	if ( argc==2 )
	{
		sscanf(argv[1],"%d",&val);
		size = AvailMem(MEMF_FAST);
		size -= val;
		ptr = AllocMem(size,MEMF_FAST);
		if ( ptr )
		{
			printf("FAST total now %ld -- press <RETURN> to free memory.\n", AvailMem(MEMF_FAST));
			gets(ch);
			FreeMem(ptr,size);
		}
		else
			printf("alloc failed\n");
	}

	exit(0);
}
