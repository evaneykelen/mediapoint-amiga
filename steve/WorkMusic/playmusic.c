//	File		:	Music.c
//	Uses		:	MLP.o
//	Date		:	26-04-93
//	Author	:	Ing C. Lieshout ( the C-code ), Mike Kennedy ( the A music coding )
//	Desc.		:	Try to use Mikes coding in a C-program
//

#include <stdio.h>
#include	<exec/types.h>

/* Mikes proto's */

int SetupPlayer(void);
void KillPlayer(void);
APTR ReadModule(char *name);
void KillModule(APTR);
int PlayTune(APTR);
void StopTune(APTR);

void main( int argc, char *argv[] )
{
	APTR mod;
	char *t;
	int i;

	if( argc >=2 )
	{
		printf("Setting up player\n");

		if( !SetupPlayer() )
		{
			printf("Loading module\n");
			mod =	ReadModule( argv[1] );
			if( mod != NULL )
			{
				t = (char *)mod;
				printf("Identfier = [");
				for( i = 4; i < 8; i++ )
					printf("%c",t[i] );
				printf("]\n");
				printf("succesful loaded module\n");
				PlayTune( mod );
				getch();
				StopTune( mod );
				KillModule( mod );
			}
			KillPlayer();
		}
		else
			printf("Can't open player\n");
	}
	else
		printf("Usage %s filename\n",argv[0] );
}
