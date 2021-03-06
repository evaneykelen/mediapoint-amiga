# Source code of MediaPoint for the Commodore Amiga

![MediaPoint](https://pbs.twimg.com/media/Cfnrb7DVIAAQpwp.jpg)

MediaPoint was developed between 1991 and 1994 by a Dutch company called 1001 Software Developments. The company was founded by Joost Honig, Stefan "Steve" van der Horst and myself, Erik van Eykelen. Both Joost and Erik were still attending college when the company was launched on January 1st 1989.

Steve left after several years, while Erik and Joost continued to work on MediaPoint until Commodore's demise in 1994.

Our customers used MediaPoint for many different purposes: TV text channels, digital signage systems (lounges, hotels, airports), touch screen kiosk applications and as a CGI tool for television and video productions. Ultimately MediaPoint was sold in 21 countries by distributors in France, UK, US and the Netherlands.

Our company survived Commodore's bankruptcy by changing its business from developing multimedia software for the Amiga to creating CD-ROM productions for large enterprises such as KLM, Ericsson, Nokia, AVIS, and General Electric. While our remaining Amigas were still used to create graphics, the productions we created were designed to run on Apple Macs and PCs which were just getting decent graphics cards and cd-roms.

In later years the company became a well-established web development agency in Amsterdam (named Honig & Van Eykelen), it spun off a content management company called BackStream and was acquired in two stages by Cambridge Technology Partners and Ordina in 2004 and 2005.

Most of the source code in this repository was created by Steve and me. Joost was responsible for the product's artwork, boxing, demos, owners' manual, finance, sales and marketing. Major contributions to the code base were made by Cees Lieshout (player effects & animations) and Pascal Eeftinck (text antialiasing).

## About the code

- The C compiler we used was the SAS/C Amiga Compiler. Some of the code is in 68xxx assembly.
- I don't have access to an Amiga or a VM so I'm unable to check whether the code still compiles.
- Most of the files lack attribution, for instance I don't seem to have placed my name in the stuff I wrote. It's probably safe to assume that I wrote most of the non-attributed code.
- The software was protected by a hardware dongle (yes, yikes but software piracy was rampant on the Amiga). If you try to compile the code base you'll have to set a flag to disable the hardware check.
- Images are in IFF/ILBM (https://en.wikipedia.org/wiki/ILBM) format and don't have a file extension. You can view the images using tools like Irfanview (Windows) or Xee (macOS).

## Interesting tidbits

- A modular approach --called Xapps-- was used to support hardware devices such as the CDTV, Genlock and Studio 16 sound card. Inter-process communication was used to shuttle events and data between Mediapoint and its Xapps.

- A nested file format is used to store scripts and pages. For example:

```
SCRIPTTALK 4, 0

STARTSER

 STARTSER "Root"

  START
   PAGE "Work:MediaPoint/Graphics/Pictograms/Accommodation", OFF, 0, 20, 0, 0, 2
   DURATION 00:00:05:0
   PROGRAM SU|MO|TU|WE|TH|FR|SA
  END

  ...
```

and

```
PAGETALK 1,0

OBJECTSTART
 SCREEN 640,256,4,0,0,3
 PALETTE 1, "000000 999999 888888 777777 555555 444444 333333 222222 "
 PALETTE 2, "dd0000 dd8800 dddd00 008800 0000dd 5555ff 666666 dddddd "
 WINDOW 0,133,640,41, 8,11,11,8, 7,9,3, 0, 0,0,0,0, 8, 1,4,10
 FORMAT 0,0,0,0,2,1,1,0,0,3,0,0
 STYLE topaz,8,0,13,13
 CRAWL "topaz", 8, 12, 1
 CRAWL "MediaPoint"
 CRAWL " "
OBJECTEND
```

- MediaPoint supported Arexx (https://en.wikipedia.org/wiki/ARexx):

```
/* MediaPoint                                                  */
/* Simple example of getting a variable SCORE from MediaPoint, */
/* adapting it and sending it back.                            */

ADDRESS 'MEDIAPOINT'
options results

GETVAR SCORE            /* Get the value from MP */ 
TEMP=RESULT             /* Store it into RESULT  */

TEMP=TEMP+1             /* Do what you like here */

RESULT=TEMP
SETVAR SCORE RESULT     /* Send it back          */
```

- MediaPoint consisted of a visual script and page editor to create interactive playlists of media, a stand-alone player and communications software to transmit scripts, pages and media by modem.

- As mentioned above, some of the code is in 68xxx assembly. Yes, really!

```
free_memory_view:
  move.l db_graphbase(a3),a6
  move.l vb_viewportw(a5),a0
  jsr    _LVOGfxLookUp(a6)
  move.l d0,vb_vpextra(a5)
  move.l vb_vieww(a5),a0
  jsr    _LVOGfxLookUp(a6)
  move.l d0,vb_vextra(a5)
  ...
```

## Copyright?

I honestly don't know who owns the rights of the code. If I recall correctly we sold or licensed the code to some company in the late 1990s but I don't know for sure. Contact me if you have concerns about the legitimacy of the code base being available in this repository.

![MediaPoint](https://pbs.twimg.com/media/DUAm1DnX4AAECKC.png:large)
