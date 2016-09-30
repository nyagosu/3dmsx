#include <3DS.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DSGraph.h"
#include "font5x6.h"
//#include "icon.h"

char * fontbin = NULL;
char * fontbin_zen = NULL;

u32 clsColor[2] = { 0x00000000 ,0x00000000 };

extern u8 *MemMap[4][4][8];   /* Memory maps [PPage][SPage][Addr] */

extern void LOG( char * str , ... );

void drawAllKeyboard(void);

void InitGraph(void)
{
	//グラフィック初期化
	gfxInitDefault();

//	PrintConsole console =
//	{
//		//Font:
//		{
//			(u8*)default_font_bin, //font gfx
//			0, //first ascii character in the set
//			128, //number of characters in the font set
//		},
//		0,0,  //cursorX cursorY
//		0,0,  //prevcursorX prevcursorY
//		40,   //console width
//		30,   //console height
//		0,    //window x
//		0,    //window y
//		32,   //window width
//		24,   //window height
//		3,    //tab size
//		0,    //font character offset
//		0,    //print callback
//		false //console initialized
//	};

	//コンソール初期化
	consoleInit(GFX_BOTTOM, NULL );
	printf( "Initialized Console\r\n" );

	//3D表示設定
//	gfxSet3D(true);
//	printf( "Use 3D view.\r\n" );

	//ダブルバッファ設定
//	printf( "Use double buffering.\r\n" );
}

void ExitGraph(void)
{
	gfxExit();
}

void memcpy8( u8 * dst,u8 * src, int cnt ){
	int c;
	for( c=0;c<cnt;c++ ) *dst++ = *src++;
}

void memcpy16( u16 * dst,u16 * src, int cnt ){
	int c;
	for( c=0;c<cnt;c++ ) *dst++ = *src++;
}

void pset( gfxScreen_t screen, gfx3dSide_t side, u16 x, u16 y, u32 col )
{
	u16  sx, sy;
	u8 * gfx = gfxGetFramebuffer(screen, side, &sx, &sy );
	gfx = gfx + (y*sy+x) * 3;
	*(gfx  ) = (col>>16)&0xFF;
	*(gfx+1) = (col>> 8)&0xFF;
	*(gfx+2) = (col    )&0xFF;
}

void line( gfxScreen_t screen, gfx3dSide_t side,u16 x, u16 y, u16 w, u16 h, u32 col )
{
	int E;
	int dx,dy,sx,sy,i;

	sx = ( w>0 ) ? 1 : -1;
	dx = ( w>0 ) ? w-1 : (-w)-1 ;
	sy = ( h>0 ) ? 1 : -1;
	dy = ( h>0 ) ? h-1 : (-h)-1 ;

	if( dx >= dy ) {
		E = -dx;
		for( i = 0; i <= dx; i++ ) {
			pset(screen, side , x, y, col );
			x += sx;
			E += 2 * dy;
			if( E >= 0 ) {
				y += sy;
				E -= 2 * dx;
			}
		}
	} else {
		E = -dy;
		for( i = 0; i <= dy; i++ ) {
			pset(screen, side , x, y, col );
			y += sy;
			E += 2 * dx;
			if( E >= 0 ) {
				x += sx;
				E -= 2 * dy;
			}
		}
	}
}

void box(  gfxScreen_t screen, gfx3dSide_t side, u16 x,u16 y,u16 w,u16 h, u32 col  )
{
	line( screen, side,x    ,y,w,0,col );
	line( screen, side,x,y+h-1,w,0,col );
	line( screen, side,x,y    ,0,h,col );
	line( screen, side,x+w-1,y,0,h,col );
}

void fill( gfxScreen_t screen, gfx3dSide_t side, u16 x,u16 y,u16 w,u16 h, u32 col  )
{
	int i;
	for( i=y;i<y+h;i++ ){
		line( screen, side,x,i,w,0,col );
	}
}

void cls(int g){
	if(g&1)fill( GFX_TOP   ,GFX_LEFT, 0,0,256,256, clsColor[0] );
	if(g&2)fill( GFX_BOTTOM,GFX_LEFT, 0,0,256,256, clsColor[1] );
}

void drawWindow( gfxScreen_t screen, gfx3dSide_t side, u16 x, u16 y , u16 w, u16 h )
{
	line( screen, side,x    ,y    ,w,0 ,0xcfcfcf );
	line( screen, side,x    ,y+h-1,w,0 ,0x3f3f3f );
	line( screen, side,x    ,y    ,0,h ,0xcfcfcf );
	line( screen, side,x+w-1,y    ,0,h ,0x3f3f3f );

	line( screen, side,x+1  ,y+1  ,w-2,0  ,0xffffff );
	line( screen, side,x+1  ,y+h-2,w-2,0  ,0x7f7f7f );
	line( screen, side,x+1  ,y+1  ,0  ,h-2,0xffffff );
	line( screen, side,x+w-2,y+1  ,0  ,h-2,0x7f7f7f );

	fill( screen, side, x+2,y+2,w-4,h-4, 0xcfcfcf );
}

void drawButton( gfxScreen_t screen, gfx3dSide_t side, u16 x, u16 y, u16 w, u16 h, u8 * buf )
{
	box( screen, side, x, y, w, h, 0x00 );
	drawWindow(screen, side, x+1,y+1, w-2,h-2 );
	int len = strlen( (char*)buf );
	drawText( screen, side, x + w/2 - len*5/2, y + h/2 - 3, buf, 0x00 , 0 );
}

void drawFontMSX( gfxScreen_t screen, gfx3dSide_t side, u16 fx, u16 fy, u8 chr, u32 col, u32 bcol )
{
	int i;
	u8 * f=(u8*)MemMap[0][0][0] + 7103 + (int)chr*8;
	for( i=0;i<8;i++ ){
		pset( screen, side, fx  , fy+i, *f&0x80?col:bcol );
		pset( screen, side, fx+1, fy+i, *f&0x40?col:bcol );
		pset( screen, side, fx+2, fy+i, *f&0x20?col:bcol );
		pset( screen, side, fx+3, fy+i, *f&0x10?col:bcol );
		pset( screen, side, fx+4, fy+i, *f&0x08?col:bcol );
		pset( screen, side, fx+5, fy+i, *f&0x04?col:bcol );
		pset( screen, side, fx+6, fy+i, *f&0x02?col:bcol );
		pset( screen, side, fx+7, fy+i, *f&0x01?col:bcol );
		f++;
	}
}
void drawTextMSX( gfxScreen_t screen, gfx3dSide_t side, u16 fx, u16 fy, u8 * buf, u32 col, u32 bcol )
{
	int x = fx;
	int i=0;
	while( buf[i] )
	{
		drawFontMSX( screen, side, x, fy, buf[i], col, bcol );
		x +=8;
		i++;
	}
}

void drawFont( gfxScreen_t screen, gfx3dSide_t side, u16 fx, u16 fy, u8 chr, u32 col, u32 bcol )
{
	int i;
	if( (chr>=0x20) && (chr<=0x82) ){
		u8 * f=(u8*)font5x6 +(int)(chr-0x20)*6;
		for( i=0;i<6;i++ ){
			pset( screen, side, fx  , fy+i, *f&0x80?col:bcol );
			pset( screen, side, fx+1, fy+i, *f&0x40?col:bcol );
			pset( screen, side, fx+2, fy+i, *f&0x20?col:bcol );
			pset( screen, side, fx+3, fy+i, *f&0x10?col:bcol );
			pset( screen, side, fx+4, fy+i, *f&0x08?col:bcol );
			f++;
		}
	}
}
int font_height = 8;
int font_width_han = 4;
int font_width_zen = 8;
int font_width_han_byte = 1;
int font_width_zen_byte = 1;
int font_zen_line = 188 * ( 1 * 8 );

int loadFont(char * fn, int h, int wh, int wz )
{
	int siz;
	FILE * fp;
	fp = fopen(fn, "rb");
	if( fp != NULL ){
		fseek(fp,0,SEEK_END);
		siz = ftell(fp);
		fseek(fp,0,SEEK_SET);
		LOG( "loadfont siz:%d", siz );
		if( siz == 0 ){
			LOG( "File size zero?" );
			return false;
		}
		if( fontbin != NULL ) free(fontbin);
		fontbin = (char *)malloc( siz );
		if( fread(fontbin, 1, siz, fp ) != siz ){
			LOG( "font load error" );
			fclose( fp );
			free( fontbin );
			fontbin = NULL;
			return false;
		}
		fclose( fp );
	}else{
		LOG( "wrong font filename" );
		return false;
	}

	font_height = h;
	font_width_han = wh;
	font_width_zen = wz;
	font_width_han_byte = (wh%8)?wh/8+1:wh/8;
	font_width_zen_byte = (wz%8)?wz/8+1:wz/8;
	font_zen_line = 188 * ( font_width_zen_byte * font_height );	   /* 188文字 */
	fontbin_zen = fontbin + 256 * font_width_han_byte * font_height;   /* 半角256文字 */
	return true;
}

void drawFont2( gfxScreen_t screen, gfx3dSide_t side, u16 fx, u16 fy, u16 chr, u32 col, u32 bcol )
{
#if 0
	if( fontbin == NULL ) return;
	int i;
	u8 nochr[32] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

	u8 * f;
	u16 o;
	u16 * g;
	u16 * ng;
	u16 fw;

	if( chr<=0x00FF ){
//		LOG( "HAN:%x",chr );
		f=(u8*)fontbin + (int)chr * font_width_han_byte * font_height;
		g = gfx + fy * 256 + fx;
		fw = font_width_han;
	}else{
//		LOG( "ZEN:%x",chr );
		int c1 = (chr>>8);
		if( c1>0xDF) c1 -= 0x40;
		c1 -= 0x81;
		int c2 = chr&0xFF;
		f=(u8*)fontbin_zen + c1 * font_zen_line;
		if( c2<0x40 || c2==0x7F || c2>0xFC ){ 
			f = nochr;
		}else {
			if( c2>0x7F ) c2--;
			c2 -= 0x40;
			f += c2 * font_width_zen_byte * font_height;
		}
		g = gfx + fy * 256 + fx;
		fw = font_width_zen;
	}

	for( i=0;i<font_height;i++ ){
		u16 w = fw;
		ng = g + 256;
		while( w > 0 ){
			o = *g; *g = *f&0x80?col:(bcol?bcol:o);g++;w--;if(w==0)break;
			o = *g; *g = *f&0x40?col:(bcol?bcol:o);g++;w--;if(w==0)break;
			o = *g; *g = *f&0x20?col:(bcol?bcol:o);g++;w--;if(w==0)break;
			o = *g; *g = *f&0x10?col:(bcol?bcol:o);g++;w--;if(w==0)break;
			o = *g; *g = *f&0x08?col:(bcol?bcol:o);g++;w--;if(w==0)break;
			o = *g; *g = *f&0x04?col:(bcol?bcol:o);g++;w--;if(w==0)break;
			o = *g; *g = *f&0x02?col:(bcol?bcol:o);g++;w--;if(w==0)break;
			o = *g; *g = *f&0x01?col:(bcol?bcol:o);g++;w--;if(w==0)break;
			f++;
		}
		f++;
		g = ng;
	}
#endif
}

int isleadbyte( u8 c )
{
	if((c>0x80 && c<0xa0)|| (c>0xdf && c<0xfd)) return true;
	return false;
}

void drawText2( gfxScreen_t screen, gfx3dSide_t side, u16 fx, u16 fy, u8 * buf, u32 col, u32 bcol )
{
	int x = fx;
	int i=0;
	int c;
	int n;
	
	while( buf[i] )
	{
		if( isleadbyte( buf[i] ) ) {
			c = (int)buf[i]<<8 | buf[i+1];
			n = font_width_zen;
			i+=2;
		}else{
			c = buf[i];
			n = font_width_han;
			i++;
		}
		drawFont2( screen, side, x, fy, c, col, bcol );
		x += n;
	}
}

void drawText( gfxScreen_t screen, gfx3dSide_t side, u16 fx, u16 fy, u8 * buf, u32 col, u32 bcol )
{
	int x = fx;
	int i=0;
	while( buf[i] )
	{
		drawFont( screen, side, x, fy, buf[i], col, bcol );
		x +=5;
		i++;
	}
}

void drawDesktop( gfxScreen_t screen, gfx3dSide_t side )
{
/*
	int i,j;
	int col1 = 0xBFBFBF;
	int col2 = 0x9F9F9F;
	int col;
	int flg=0;
	for(i=0;i<20*256;i++) *gfx++ = 0;
	for(i=20;i<160;i++){
		col = flg?col1:col2;
		for(j=0;j<256;j++){
			*gfx++ = col;
		}
		flg =(flg+1)&1;
	}
*/
}

void copyGraph( gfxScreen_t dscreen, gfx3dSide_t dside, u16 dx, u16 dy, gfxScreen_t sscreen, gfx3dSide_t sside, u16 sx, u16 sy, u16 width, u16 height )
{
	int i;
	u16 dmx,dmy,smx,smy;
	u8 * dgfx = gfxGetFramebuffer(dscreen, dside, &dmx, &dmy );
	u8 * sgfx = gfxGetFramebuffer(sscreen, sside, &smx, &smy );
	for( i=sy;i<sy+height;i++ ){
		memcpy8( dgfx + dmy * dy + dx, sgfx + smy * i + sx, width * 3 );
		dy++;
	}
}
/*
void drawIcon( gfxScreen_t screen, gfx3dSide_t side, int x, int y, int num )
{
	int i,j;
	u16 * f=(u16*)icon + 1024 * num;
	gfx += y*256+x;

	for(i=0;i<32;i++){
		for(j=0;j<32;j++) *gfx++ = *f++ | 0x8000 ;
		gfx += (256-32);
	}
}
*/
void drawIcon2( gfxScreen_t screen, gfx3dSide_t side, u16 x, u16 y, u16 w, u16 h, u16 * buf )
{
/*
	int i,j;
	gfx += y*256+x;

	for(i=0;i<h;i++){
		for(j=0;j<w;j++) *gfx++ = *buf++;
		gfx += (256-w);
	}
*/
}

