/*
    SAS/C Background Example
    version 6.0   1992

    This is a program which demonstrates how to write a simple background
    process.  This program opens a window and writes a message.  You may close
    the CLI it was run from before closing the window.

    You should compile this program with the following options:

               STRUCTUREEQUIVALENCE
               LINK
               STARTUP=cback
               NOSTANDARDIO


    STRUCTUREEQUIVALENCE is not required, but it does circumvent several
    warnings that would otherwise be produced.

    STARTUP=cback tells slink to link with the background startup code.  This
    is required to produce a background process.  If you link in a separate step,
    you must link with cback.o instead of c.o.

    NOSTANDARDIO is required to keep the startup code from opening stderr, stdin,
    and stdout.  If any of these are opened, then the you cannot close the
    CLI window from which the background process was opened.  
    
    *** IMPORTANT **  You must remember when writing a backgorund process not to 
    call printf() or other standard IO functions.  If you do, you will probably
    crash the machine.

    You may notice that this program does not open any AmigaDOS libraries.  Instead
    it takes advantage of the fact that the version 6.0 compiler now opens
    needed AmigaDOS libraries for you.
     
*/

/**************************  INCLUDES  ************************/
#include <exec/types.h>
#include <exec/execbase.h>
#include <intuition/intuition.h>
#include <proto/intuition.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>
#include <string.h>
#include <stdlib.h>


/**************************  CONSTANTS	***********************/
#define BANNER "\x9B""0;33mSAS/C Background Example\x9B""0m \nCopyright \xA9 1992 SAS Institute, Inc.\n"


/**************************  PROTOTYPES  **********************/
void clean_exit(struct Window *, struct IOStdReq *, int);
void action(void);


/**********************  CBACK DECLARATIONS  ******************/
long __stack = 4000;			/* Amount of stack space our task needs */
char *__procname = "SAS/C Background";  /* The name of the task to create       */
long __priority = 0;			/* The priority to run the task at	*/
long __BackGroundIO = 1;	  /* Flag indicating we want to send I/O to the
				     original shell.  We will print a banner.
				     NOTE:  This variable may also be called
				     _BackGroundIO.  Notice the single
				     underscore.			     */
extern BPTR _Backstdout;	  /* File handle pointing to originating shell
				   (standard output when run in background) */

/********************************************************************/

extern struct ExecBase *SysBase;

void install_watch( void );
void remove_watch( void );

void main (void)
{
   /*  Write a copyright banner */
   if (_Backstdout)
      {
      Write(_Backstdout, BANNER, sizeof(BANNER));
      Close(_Backstdout);
      }

   /*  Call a function performing some action.	In this case the function will
       open a window and wait for the user to close it.  */
	install_watch();
   action();
   clean_exit(NULL, NULL, NULL);
}

/********************************************************************/

/*  Clean up and exit */
void clean_exit(struct Window * w, struct IOStdReq *writereq, int closedev)
{

	remove_watch();
   if (closedev)
      CloseDevice(writereq);

   if (w)
       CloseWindow(w);

   exit(0);
}


/********************************************************************/


/*  This function will open an Intuition window, attach a console device to it,
    and then write to the window.  The function will then wait until the user
    clicks on the close gadget of the window before closing the window and
    exiting the program.    */

void action(void)
{
    struct Window *w;

    struct NewWindow nw = {
		100,100,				  /* Starting corner */
		292,100,			      /* Width, height */
		2,1,				      /* Detail, Block pens */
	    CLOSEWINDOW | NEWSIZE,    /* IDCMP flags */
	    WINDOWDEPTH | WINDOWDRAG | WINDOWCLOSE | WINDOWSIZING,
							  /* Window flags */
		NULL,				      /* Pointer to first gadget */
		NULL,				      /* Pointer to checkmark */
		"SAS/C Background Example",       /* Title */
		NULL,				      /* Screen pointer */
		NULL,				      /* Bitmap pointer */
		100,100,(USHORT)~0,(USHORT)~0,  /* Allow window to open to
					   maximum screen size	*/
		WBENCHSCREEN			  /* Type of screen */
		};

    struct IntuiMessage * msg = NULL;

    struct IOStdReq *writereq;

    int type;
    WORD Class;
    int done = 0;
    BYTE error;
    char *string = "You may close the CLI window now and this window will remain.";

    /*  Set type of window opened based on AmigaDOS version number */
    if(SysBase->LibNode.lib_Version < 37)
       {
       type = 0;
       nw.Flags |= SMART_REFRESH;
       }
    else 
       {
       type = 3;   
       nw.Flags |= SIMPLE_REFRESH;
       }

    /*	Open the window.   */
    w = OpenWindow(&nw);
    if(w  == NULL)
       clean_exit(w, NULL, 0);

    writereq = calloc(1, sizeof(struct IOStdReq));

    /*	First open the console device, then write a message to the window.  */
    writereq->io_Data = (APTR) w;
    writereq->io_Length = sizeof(struct Window);
    error = OpenDevice("console.device", type, writereq, 0);
    if (error != NULL)
       clean_exit(w, writereq, 0);

    writereq->io_Command = CMD_WRITE;
    writereq->io_Data = (APTR) string;
    writereq->io_Length = strlen(string);
    DoIO(writereq);

    /*	Wait for a signal.  */
    while ( !done )
    {
       WaitPort(w->UserPort);
       while(msg = (struct IntuiMessage *) GetMsg(w->UserPort))
       {
          /*  When a signal is received, reply to it and evaluate it  */
          Class = msg->Class;
          ReplyMsg ( (struct Message *) msg);
          if (Class == CLOSEWINDOW)
            done = 1;
       }
    }

    clean_exit(w, writereq, 1);
}

