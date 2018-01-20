#include <exec/types.h>
#include <exec/semaphores.h>

#include <exec/types.h>
#include <graphics/gfx.h>
#include <graphics/clip.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <graphics/rastport.h>
#include <graphics/text.h>
#include <graphics/view.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <intuition/preferences.h>

extern struct SignalSemaphore MyDataSemaphore;

#define MEDIALINKLIBNAME "mediapoint.library"

#include "pascal:include/toolslib.h"

#include "nb:capsdefines.h"
#include "nb:newdefines.h"
#include "nb:parser.h"
#include "nb:capsstructs.h"

/******** E O F ********/
