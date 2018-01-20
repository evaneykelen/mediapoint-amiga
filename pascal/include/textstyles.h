#ifndef	MEDIAPOINT_TEXTSTYLES_H
#define	MEDIAPOINT_TEXTSTYLES_H


// MediaPoint Font Style definitions per character

#define	MFSB_UNDERLINED	0
#define	MFSB_BOLD				1
#define	MFSB_ITALIC			2

#define	MFSF_UNDERLINED	0x0001
#define	MFSF_BOLD				0x0002
#define	MFSF_ITALIC			0x0004


// 'Global style' definitions, for total text in window

#define	SHADOWTYPE_NORMAL				0
#define	SHADOWTYPE_CAST					1
#define	SHADOWTYPE_SOLID				2
#define	SHADOWTYPE_OUTLINE			3
#define	SHADOWTYPE_TRANSPARENT	4

#define	LIGHTSOURCE_SE	0
#define	LIGHTSOURCE_S		1
#define	LIGHTSOURCE_SW	2
#define	LIGHTSOURCE_W		3
#define	LIGHTSOURCE_NW	4
#define	LIGHTSOURCE_N		5
#define	LIGHTSOURCE_NE	6
#define	LIGHTSOURCE_E		7

#define	ANTIALIAS_NONE		0
#define	ANTIALIAS_LOW			1
#define	ANTIALIAS_MEDIUM	2
#define	ANTIALIAS_HIGH		3

#define	JUSTIFICATION_LEFT		0
#define	JUSTIFICATION_CENTER	1
#define	JUSTIFICATION_RIGHT		2
#define	JUSTIFICATION_FLUSH		3		// FOR FUTURE EXPANSION


#endif // MEDIAPOINT_TEXTSTYLES_H
