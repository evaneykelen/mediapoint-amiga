#define INTUI_V36_NAMES_ONLY
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <intuition/intuitionbase.h>
#include <intuition/intuition.h>
#include <intuition/sghooks.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>
#include <dos/stdio.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <graphics/rastport.h>
#include <graphics/text.h>
#include <graphics/view.h>
#include <graphics/clip.h>
#include <rexx/errors.h>
#include <rexx/storage.h>
#include <rexx/rxslib.h>

#include <clib/exec_protos.h>
#include <clib/rexxsyslib_protos.h>
#include <clib/alib_protos.h>

#include "myproto/exec.h"

#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/dos.h>
#include <proto/console.h>
#include <proto/diskfont.h>
#include <proto/layers.h>
#include <proto/rexxsyslib.h>

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "medialinklib.h"
#define medialinkLibBase (getreg(REG_A6))
#include "medialinkLib_proto.h"
#include "medialinkLib_pragma.h"
#include "allprotos.h"
#include "nb:gui_texts.h"
