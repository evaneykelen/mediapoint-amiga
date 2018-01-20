#include "nb:pre.h"

int AllocDongle(void) { return(TRUE); }

UWORD ReadDongle(UWORD code)
{
	switch(code)
	{
		case 0: return(0x4d45);		
		case 1: return(0x4449);		
		case 2: return(0x414c);		
		case 3: return(0x494e);		
		case 4: return(0x4b00);		
		case 5: return(23);		
		case 6: return(11);		
		case 7: return(1992);		
	}
}

void FreeDongle(void) { }
