/***********************************************************
*File : defs.h
*	
*Desc : Defs
*/

#define VERSION 1.0


/*
* set this flag to FALSE for use with commercial products
* the donglewrite routines will not be compiled since
* they should not be used within applications
* Set to TRUE only for programming the dongle!!!!!!!!!!
*/
#define _DONGLEWRITE_ON FALSE

/*
* If set to TRUE, the main() function will be compiled as well
* If set to FALSE , the main function will not be compiled for linking for
* other applications
*/
#define _MAIN_ON FALSE

/*
	for programming a dongle, set both flags to TRUE 
	for linking this software dongle.o and dongleio.o set both flags to FALSE
*/

/*********************************************************
*	Defs for Potgo resource	
*/
#define OUTRY 1<<15	/*CS */
#define DATRY 1<<14
#define OUTRX 1<<13	/*DI/DO */
#define DATRX 1<<12

#define CS_OUT WritePotgo(-1,OUTRY)
#define DATA_OUT WritePotgo(-1,OUTRX)
#define DATA_IN  WritePotgo(0,OUTRX)

#define CS_HIGH  WritePotgo(-1,DATRY) 
#define DATA_HIGH WritePotgo(-1,DATRX)

#define CS_LOW WritePotgo(0,DATRY) 
#define DATA_LOW WritePotgo(0,DATRX) 

#define DATA_INPUT GetInput()

#define DELAY {int i;for(i=0;i<500;i++);}

/*********************************************************
*	Defs for CIAA
*	No resource is available for this bit
*	Therefore we have to crawl into the hardware
*	directly. 
*/

#define SK 0x80				/* bit 7 of CIAA_PRA / CIAA_DDRA */

#define SK_HIGH SetOut((UBYTE *)0xbfe001,SK)
#define SK_LOW  ClrOut((UBYTE *)0xbfe001,SK)

