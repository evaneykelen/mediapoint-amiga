/*********************************************************/
/*	File				:Text.h											*/
/*																			*/
/*	Beschrijving	:Diverse prototypes van procedures om	*/
/*						 in een intuition window te schrijven.	*/
/*																			*/
/* Datum				:13-Jul-92										*/
/*********************************************************/

// enkele maten voor de Tekst ButtonS

#define TBS_XOFFSET 28
#define TBS_YOFFSET 10
#define TBS_XSIZE 170
#define TBS_YSIZE 11
#define TBS_DX 200
#define TBS_DY 14 
#define TBS_KEYXSIZE 20

VOID InitPens(VOID);
VOID PrintText(struct Window *window, char *tekst, int x, int y);
VOID TekenBorder(struct Window *window,int x, int y, int dx, int dy);
VOID TextButton(struct Window *window, char *text,int x, int y, int dx, int dy);
VOID TextButtons( struct Window *window, char *text[] , int vertical );
VOID TextKeyButtons( struct Window *window, char *text[] , int vertical );
VOID ClearBox(struct Window *window,int x, int y, int dx, int dy);
VOID SetBox(struct Window *window,int x, int y, int dx, int dy);
