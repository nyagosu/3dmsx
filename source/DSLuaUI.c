#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msx.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "DSGraph.h"
#include "DSDriver.h"
#include "DSFileList.h"

extern LLIST * LOGData;

void StartSound(void);
void StopSound(void);
void waitForVBlank(int cnt);

static int ui_pset(lua_State *L) {
	int g = (int)luaL_checknumber(L, 1);
	int x = (int)luaL_checknumber(L, 2);
	int y = (int)luaL_checknumber(L, 3);
	int c = (int)luaL_checknumber(L, 4);
	pset( g==1?GFX_TOP:GFX_BOTTOM,GFX_LEFT,x,y,c );
	return 0;
}

static int ui_box(lua_State *L) {
	int g = (int)luaL_checknumber(L, 1);
	int x = (int)luaL_checknumber(L, 2);
	int y = (int)luaL_checknumber(L, 3);
	int w = (int)luaL_checknumber(L, 4);
	int h = (int)luaL_checknumber(L, 5);
	int c = (int)luaL_checknumber(L, 6);
	box( g==1?GFX_TOP:GFX_BOTTOM,GFX_LEFT, x, y, w, h, c );
	return 0;
}
static int ui_line(lua_State *L) {
	int g = (int)luaL_checknumber(L, 1);
	int x = (int)luaL_checknumber(L, 2);
	int y = (int)luaL_checknumber(L, 3);
	int w = (int)luaL_checknumber(L, 4);
	int h = (int)luaL_checknumber(L, 5);
	int c = (int)luaL_checknumber(L, 6);
	line( g==1?GFX_TOP:GFX_BOTTOM,GFX_LEFT, x, y, w, h, c );
	return 0;
}
static int ui_fill(lua_State *L) {
	int g = (int)luaL_checknumber(L, 1);
	int x = (int)luaL_checknumber(L, 2);
	int y = (int)luaL_checknumber(L, 3);
	int w = (int)luaL_checknumber(L, 4);
	int h = (int)luaL_checknumber(L, 5);
	int c = (int)luaL_checknumber(L, 6);
	fill( g==1?GFX_TOP:GFX_BOTTOM,GFX_LEFT, x, y, w, h, c );
	return 0;
}
static int ui_drawFont(lua_State *L) {
	int g  = (int)luaL_checknumber(L, 1);
	int x  = (int)luaL_checknumber(L, 2);
	int y  = (int)luaL_checknumber(L, 3);
	int chr= (int)luaL_checknumber(L, 4);
	int fc = (int)luaL_checknumber(L, 5);
	int bc = (int)luaL_checknumber(L, 6);
	drawFont( g==1?GFX_TOP:GFX_BOTTOM,GFX_LEFT, x, y, chr, fc, bc );
	return 0;
}

static int ui_drawText(lua_State *L) {
	int g  = (int)luaL_checknumber(L, 1);
	int x  = (int)luaL_checknumber(L, 2);
	int y  = (int)luaL_checknumber(L, 3);
	u8* b  = (u8*)luaL_checkstring(L, 4);
	int fc = (int)luaL_checknumber(L, 5);
	int bc = (int)luaL_checknumber(L, 6);
	drawText( g==1?GFX_TOP:GFX_BOTTOM,GFX_LEFT, x, y, b, fc, bc );
	return 0;
}
static int ui_loadFont(lua_State *L) {
	int ret;
	char * fn   = (char*)luaL_checkstring(L, 1);
	int h  = (int)luaL_checknumber(L, 2);
	int wh = (int)luaL_checknumber(L, 3);
	int wz = (int)luaL_checknumber(L, 4);
	ret = loadFont( fn, h, wh, wz );
	lua_pushboolean( L,ret );
	return 1;
}
static int ui_drawFont2(lua_State *L) {
	int g  = (int)luaL_checknumber(L, 1);
	int x  = (int)luaL_checknumber(L, 2);
	int y  = (int)luaL_checknumber(L, 3);
	int chr= (int)luaL_checknumber(L, 4);
	int fc = (int)luaL_checknumber(L, 5);
	int bc = (int)luaL_checknumber(L, 6);
	drawFont2( g==1?GFX_TOP:GFX_BOTTOM,GFX_LEFT, x, y, chr, fc, bc );
	return 0;
}
static int ui_drawText2(lua_State *L) {
	int g  = (int)luaL_checknumber(L, 1);
	int x  = (int)luaL_checknumber(L, 2);
	int y  = (int)luaL_checknumber(L, 3);
	u8* b  = (u8*)luaL_checkstring(L, 4);
	int fc = (int)luaL_checknumber(L, 5);
	int bc = (int)luaL_checknumber(L, 6);
	drawText2( g==1?GFX_TOP:GFX_BOTTOM,GFX_LEFT, x, y, b, fc, bc );
	return 0;
}
static int ui_drawFontMSX(lua_State *L) {
	int g  = (int)luaL_checknumber(L, 1);
	int x  = (int)luaL_checknumber(L, 2);
	int y  = (int)luaL_checknumber(L, 3);
	int chr= (int)luaL_checknumber(L, 4);
	int fc = (int)luaL_checknumber(L, 5);
	int bc = (int)luaL_checknumber(L, 6);
	drawFontMSX( g==1?GFX_TOP:GFX_BOTTOM,GFX_LEFT, x, y, chr, fc, bc );
	return 0;
}
static int ui_drawTextMSX(lua_State *L) {
	int g  = (int)luaL_checknumber(L, 1);
	int x  = (int)luaL_checknumber(L, 2);
	int y  = (int)luaL_checknumber(L, 3);
	u8* b  = (u8*)luaL_checkstring(L, 4);
	int fc = (int)luaL_checknumber(L, 5);
	int bc = (int)luaL_checknumber(L, 6);
	drawTextMSX( g==1?GFX_TOP:GFX_BOTTOM,GFX_LEFT, x, y, b, fc, bc );
	return 0;
}
static int ui_drawDesktop(lua_State *L) {
	int g  = (int)luaL_checknumber(L, 1);
	drawDesktop( g==1?GFX_TOP:GFX_BOTTOM,GFX_LEFT );
	return 0;
}
static int ui_copyGraph(lua_State *L) {
	int dg = (int)luaL_checknumber(L, 1);
	int dx = (int)luaL_checknumber(L, 2);
	int dy = (int)luaL_checknumber(L, 3);
	int sg = (int)luaL_checknumber(L, 1);
	int sx = (int)luaL_checknumber(L, 4);
	int sy = (int)luaL_checknumber(L, 5);
	int w  = (int)luaL_checknumber(L, 6);
	int h  = (int)luaL_checknumber(L, 7);
	copyGraph( dg==1?GFX_TOP:GFX_BOTTOM,GFX_LEFT, dx, dy, sg==1?GFX_TOP:GFX_BOTTOM,GFX_LEFT, sx, sy, w, h );
	return 0;
}
/*
static int ui_drawIcon(lua_State *L) {
	int g = (int)luaL_checknumber(L, 1);
	int x = (int)luaL_checknumber(L, 2);
	int y = (int)luaL_checknumber(L, 3);
	int n = (int)luaL_checknumber(L, 4);
	drawIcon( g==1?GFX_TOP:GFX_BOTTOM,GFX_LEFT, x, y, n );
	return 0;
}
*/
static int ui_drawIcon2(lua_State *L) {
	int g = (int)luaL_checknumber(L, 1);
	int x = (int)luaL_checknumber(L, 2);
	int y = (int)luaL_checknumber(L, 3);
	int w = (int)luaL_checknumber(L, 4);
	int h = (int)luaL_checknumber(L, 5);
	u16* b = (u16*)lua_touserdata(L, 6);
	if(b==NULL) return luaL_error(L, "drawIcon2 :not userdata param." );
	drawIcon2( g==1?GFX_TOP:GFX_BOTTOM,GFX_LEFT, x, y, w, h, b );
	return 0;
}

static int ui_loadIcon(lua_State *L) {
	FILE * fp;
	int cnt;
	char * fn   = (char*)luaL_checkstring(L, 1);
	int siz  = (int)luaL_checknumber(L, 2);
	char * buf = (char*)lua_newuserdata(L, siz);

	fp = fopen( fn, "rb" );
	if( fp == NULL ) return luaL_error(L, "loadIcon :%s file not found.", fn );
	cnt = fread(buf, 1, siz, fp );
	if( cnt != siz ) return luaL_error(L, "loadIcon :File size invaild. %d", siz );
	fclose( fp );
	return 1;
}

static int ui_cls(lua_State *L) {	cls((int)luaL_checknumber(L, 1));	return 0; }
static int ui_drawWindow(lua_State *L) {
	int g = (int)luaL_checknumber(L, 1);
	int x = (int)luaL_checknumber(L, 2);
	int y = (int)luaL_checknumber(L, 3);
	int w = (int)luaL_checknumber(L, 4);
	int h = (int)luaL_checknumber(L, 5);
	drawWindow( g==1?GFX_TOP:GFX_BOTTOM,GFX_LEFT, x, y , w, h );
	return 0;
}
static int ui_drawButton(lua_State *L) {
	int g = (int)luaL_checknumber(L, 1);
	int x = (int)luaL_checknumber(L, 2);
	int y = (int)luaL_checknumber(L, 3);
	int w = (int)luaL_checknumber(L, 4);
	int h = (int)luaL_checknumber(L, 5);
	u8* b = (u8*)luaL_checkstring(L, 6);
	drawButton( g==1?GFX_TOP:GFX_BOTTOM,GFX_LEFT, x, y, w, h , b);
	return 0;
}
static int ui_LOG(lua_State *L) {	LOG( (char*)luaL_checkstring(L, 1) );	return 0; }

static int ui_waitForVBlank(lua_State *L){	waitForVBlank((int)luaL_checknumber(L, 1));	return 0; }

static int ui_scanKeys(lua_State *L){	scanKeys();	return 0;}

static void pushkeydata(lua_State *L, u32 k )
{
	lua_createtable(L, 0, 0);
	lua_pushstring(L, "A"      ); lua_pushboolean( L, k&BIT( 0) ); lua_settable (L,1);
	lua_pushstring(L, "B"      ); lua_pushboolean( L, k&BIT( 1) ); lua_settable (L,1);
	lua_pushstring(L, "SELECT" ); lua_pushboolean( L, k&BIT( 2) ); lua_settable (L,1);
	lua_pushstring(L, "START"  ); lua_pushboolean( L, k&BIT( 3) ); lua_settable (L,1);
	lua_pushstring(L, "RIGHT"  ); lua_pushboolean( L, k&BIT( 4) ); lua_settable (L,1);
	lua_pushstring(L, "LEFT"   ); lua_pushboolean( L, k&BIT( 5) ); lua_settable (L,1);
	lua_pushstring(L, "UP"     ); lua_pushboolean( L, k&BIT( 6) ); lua_settable (L,1);
	lua_pushstring(L, "DOWN"   ); lua_pushboolean( L, k&BIT( 7) ); lua_settable (L,1);
	lua_pushstring(L, "R"      ); lua_pushboolean( L, k&BIT( 8) ); lua_settable (L,1);
	lua_pushstring(L, "L"      ); lua_pushboolean( L, k&BIT( 9) ); lua_settable (L,1);
	lua_pushstring(L, "X"      ); lua_pushboolean( L, k&BIT(10) ); lua_settable (L,1);
	lua_pushstring(L, "Y"      ); lua_pushboolean( L, k&BIT(11) ); lua_settable (L,1);
	lua_pushstring(L, "TOUCH"  ); lua_pushboolean( L, k&BIT(12) ); lua_settable (L,1);
	lua_pushstring(L, "LID"    ); lua_pushboolean( L, k&BIT(13) ); lua_settable (L,1);
}
static int ui_keysDown(lua_State *L){
	u32 k = keysDown();
	pushkeydata(L,k);
	return 1;
}
static int ui_keysDownRepeat(lua_State *L){
	u32 k = keysDownRepeat();
	pushkeydata(L,k);
	return 1;
}

static int ui_keysHeld(lua_State *L){
	u32 k = keysHeld();
	pushkeydata(L,k);
	return 1;
}

static int ui_keysUp(lua_State *L){
	u32 k = keysUp();
	pushkeydata(L,k);
	return 1;
}

static int ui_keysSetRepeat(lua_State *L){
	u8 d = (u8)luaL_checknumber(L, 1);
	u8 r = (u8)luaL_checknumber(L, 2);
	keysSetRepeat( d, r );
	return 0;
}
static int ui_touchReadXY(lua_State *L){
	touchPosition t;
	touchRead(&t);
	lua_createtable(L, 0, 0);
//	lua_pushstring(L, "rawx" ); lua_pushinteger( L, t.rawx ); lua_settable (L,1);
//	lua_pushstring(L, "rawy" ); lua_pushinteger( L, t.rawy ); lua_settable (L,1);
	lua_pushstring(L, "px"); lua_pushinteger( L, t.px); lua_settable (L,1);
	lua_pushstring(L, "py"); lua_pushinteger( L, t.py); lua_settable (L,1);
//	lua_pushstring(L, "z1"); lua_pushinteger( L, t.z1); lua_settable (L,1);
//	lua_pushstring(L, "z2"); lua_pushinteger( L, t.z2); lua_settable (L,1);
	return 1;
}

static int ui_fileselect(lua_State *L){
	int ret;
	char * p   = (char *)luaL_checkstring(L, 1);
	char * ext = (char *)luaL_checkstring(L, 2);
	char fn[256];
	*fn = '\0';
	if( strlen(ext)==0 ) ext = NULL;
	ret = fileselect( p, ext, fn );
	lua_pushinteger(L, ret );
	lua_pushstring( L,fn );
	return 2;
}

static int ui_soundStart(lua_State *L){	LOG( "soundStart" ); StartSound();	return 0; }
static int ui_soundStop(lua_State *L) {	StopSound();	return 0; }
static int ui_drawAllKeyboard(lua_State *L){ drawAllKeyboard();	return 0; }

static int ui_LogData(lua_State *L){
	int cnt = 1;
	int idx;
	int ret = LLIST_top(LOGData);
	lua_newtable(L);	idx = lua_gettop(L);
	while(ret != LLIST_EOL ){
		lua_pushstring (L,(char*)LOGData->cur->object);
		lua_rawseti(L,idx,cnt++);
		ret = LLIST_next(LOGData);
	}
	return 1;
}

static int ui_clsColor(lua_State *L){
	int bg1 = luaL_checkinteger(L, 1);
	int bg2 = luaL_checkinteger(L, 2);
	clsColor[0] = bg1;
	clsColor[1] = bg2;
	return 0;
}

static int ui_swapScreen(lua_State *L){
	gfxSwapBuffers();
	return 0;
}

static int ui_touchMode(lua_State *L){
	int typ = luaL_checkinteger(L, 1);
	touchMode = typ;
	return 0;
}

static const luaL_Reg uilib[] = {
	{"pset"         , ui_pset },
	{"box"          , ui_box },
	{"line"         , ui_line },
	{"fill"         , ui_fill },
	{"drawFont"     , ui_drawFont },
	{"drawText"     , ui_drawText },
	{"drawText2"    , ui_drawText2 },
	{"loadFont"     , ui_loadFont },
	{"drawFont2"    , ui_drawFont2 },
	{"drawFontMSX"  , ui_drawFontMSX },
	{"drawTextMSX"  , ui_drawTextMSX },
	{"drawDesktop"  , ui_drawDesktop },
	{"copyGraph"    , ui_copyGraph },
//	{"drawIcon"     , ui_drawIcon },
	{"drawIcon2"    , ui_drawIcon2 },
	{"loadIcon"     , ui_loadIcon },
	{"cls"          , ui_cls },
	{"clsColor"     , ui_clsColor },
	{"drawWindow"   , ui_drawWindow },
	{"drawButton"   , ui_drawButton },
	{"LOG"          , ui_LOG },
	{"waitForVBlank", ui_waitForVBlank },
	{"scanKeys"     , ui_scanKeys},
	{"keysDown"     , ui_keysDown},
	{"keysDownRepeat",ui_keysDownRepeat},
	{"keysHeld"     , ui_keysHeld},
	{"keysUp"       , ui_keysUp},
	{"keysSetRepeat", ui_keysSetRepeat},
	{"touchReadXY"  , ui_touchReadXY},
	{"fileselect"   , ui_fileselect},
	{"soundStart"   , ui_soundStart},
	{"soundStop"    , ui_soundStop},
	{"drawAllKeyboard",ui_drawAllKeyboard},
	{"LogData"        ,ui_LogData},
	{"swapScreen"   , ui_swapScreen},
	{"touchMode"    , ui_touchMode},
	{NULL, NULL}
};

LUALIB_API int luaopen_ui (lua_State *L) {
	luaL_register(L, "ui", uilib);
	return 1;
}
