#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <3ds.h>

typedef uint8_t  uint8;  ///<  8-bit unsigned integer
typedef uint16_t uint16; ///< 16-bit unsigned integer

#define ICON_POWER_OFF	0
#define ICON_POWER_ON	1
#define ICON_RESET		2
#define ICON_CLOSE		3
#define ICON_ROM1		4
#define ICON_ROM2		5
#define ICON_DISK1		6
#define ICON_DISK2		7
#define ICON_CONFIG		8
#define ICON_STICK1		9
#define ICON_STICK2		10
#define ICON_CURSOR		11
#define ICON_CLIPTOP	12
#define ICON_CLIPBTM	13
#define ICON_CLIPFIT	14
#define ICON_STATLOAD	15
#define ICON_STATSAVE	16
#define ICON_CAS		17

#define CURSOR			0
#define STICK1			1
#define STICK2			2

//#define RGB16(a,r,g,b)  RGB15(r,g,b)|((a)<<15)
//#define pset( GFX, X,Y, COL )	(*((uint16*)(GFX) + ((Y)*256+(X)) ) = (COL) )

extern u32 clsColor[2];

void pset( gfxScreen_t screen, gfx3dSide_t side, u16 x,u16 y,             u32 col );
void box ( gfxScreen_t screen, gfx3dSide_t side, u16 x,u16 y,u16 w,u16 h, u32 col );
void line( gfxScreen_t screen, gfx3dSide_t side, u16 x,u16 y,u16 w,u16 h, u32 col );
void fill( gfxScreen_t screen, gfx3dSide_t side, u16 x,u16 y,u16 w,u16 h, u32 col );
void drawFont   ( gfxScreen_t screen, gfx3dSide_t side, u16 fx, u16 fy, u8   chr, u32 col, u32 bcol );
void drawText   ( gfxScreen_t screen, gfx3dSide_t side, u16 fx, u16 fy, u8 * buf, u32 col, u32 bcol );
int  loadFont( char * fn , int h, int wh, int wz );
void drawFont2  ( gfxScreen_t screen, gfx3dSide_t side, u16 fx, u16 fy, u16  chr, u32 col, u32 bcol );
void drawText2  ( gfxScreen_t screen, gfx3dSide_t side, u16 fx, u16 fy, u8 * buf, u32 col, u32 bcol );
void drawFontMSX( gfxScreen_t screen, gfx3dSide_t side, u16 fx, u16 fy, u8   chr, u32 col, u32 bcol );
void drawTextMSX( gfxScreen_t screen, gfx3dSide_t side, u16 fx, u16 fy, u8 * buf, u32 col, u32 bcol );
void drawDesktop( gfxScreen_t screen, gfx3dSide_t side );
void copyGraph  ( gfxScreen_t dscreen, gfx3dSide_t dside, u16 dx, u16 dy, gfxScreen_t sscreen, gfx3dSide_t sside, u16 sx, u16 sy, u16 width, u16 height );
//void drawIcon( gfxScreen_t screen, gfx3dSide_t side, int x, int y, int num );
void drawIcon2( gfxScreen_t screen, gfx3dSide_t side, u16 x, u16 y, u16 w, u16 h, u16 * buf );
void cls( int g );
void drawWindow( gfxScreen_t screen, gfx3dSide_t side, u16 x, u16 y , u16 w, u16 h );
void drawButton( gfxScreen_t screen, gfx3dSide_t side, u16 x, u16 y , u16 w, u16 h ,u8 * buf);

#endif
