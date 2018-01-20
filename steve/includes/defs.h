#ifndef MINC_DEFS_H
#define  MINC_DEFS_H


/*******************************************
*Desc : Runmodes for the slideshow
*/
enum
{
  RM_PRESENTATIONMANUAL,	
  RM_PRESENTATIONAUTO,	
  RM_INTERACTIVE,
  RM_TIMECODE
};

/*********************************************
*Desc : State of the processcontroller
*		RS_INIT 	- PC is initializing its guides/modules and their children
*		RS_CONTROL  - PC is controlling the script
*		RS_REMOVE	- PC is removing the guides/modules and their children
*/
enum
{
  RS_INIT,
  RS_CONTROL,
  RS_REMOVE 
};

/***********************************************
*Desc : RECORD lets the user edit the syncpunches
*		roughly by pressing the LEFT/RIGHT keys
*		PLAY is set when the show is running without
*		editing the syncpunches.
*/
enum
{
  TT_RECORD,
  TT_PLAY	
};
#endif
