#include "msx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "DSGraph.h"
#include "DSDriver.h"
#include "DSFileList.h"
#include "keybind.h"

extern int psgON;
extern int sccON;
extern int NewMode     ;
extern int NewRAMPages ;              /* Number of RAM pages    */
extern int NewVRAMPages;              /* Number of VRAM pages   */
extern int autoFitScreen;

extern byte CartMap[4][4];

static void strcpya(char ** s, char * nm )
{
//	LOG( "strcpya 1" );
	if( *s!=NULL ) free((byte*)*s);
	*s = NULL;
//	LOG( "strcpya 2" );
	if( nm!=NULL ){
		*s = (char*) malloc( strlen(nm)+1 );
		strcpy( *s, nm );
//		LOG( "strcpya 3 s[%s] nm[%s]",*s,nm );
	}
}

static int msx_DskChange(lua_State *L) {
	int slot = (int)luaL_checknumber(L, 1);
	const char * nm = luaL_checkstring(L, 2);
	byte ret;
	ret = ChangeDisk(slot,nm);
	lua_pushnumber(L, ret);
	return 1;
}

static int msx_CasChange(lua_State *L) {
	const char * nm = luaL_checkstring(L, 1);
	int ret;
	ret = ChangeCas( nm );
	lua_pushnumber(L, ret);
	return 1;
}

static int msx_CasTop(lua_State *L) {
	CasTop();
	return 0;
}
static int msx_CasEnd(lua_State *L) {
	CasEnd();
	return 0;
}
static int msx_CasNext(lua_State *L) {
	CasNext();
	return 0;
}
static int msx_CasPrev(lua_State *L) {
	CasPrev();
	return 0;
}

static int msx_setBIOS(lua_State *L) {
	int ps = (int)luaL_checknumber(L, 1);
	int ss = (int)luaL_checknumber(L, 2);
	int pg = (int)luaL_checknumber(L, 3);
	char * fn = (char *)luaL_checkstring(L, 4);
	int siz = (int)luaL_checknumber(L, 5);
	int i;
	
	// replace?
	for(i=0;i<MAXBIOS;i++){
		if( (bios[i].PS==ps)&&(bios[i].SS==ss)&&(bios[i].PG==pg) ){
			break;
		}
	}
	// free bios search
	if( i>=MAXBIOS ){
		for(i=0;(i<MAXBIOS)&&(bios[i].name!=NULL);i++);
	}
	// bios set
	if( i<MAXBIOS ){
		bios[i].PS  = ps;
		bios[i].SS  = ss;
		bios[i].PG  = pg;
		bios[i].siz = siz;
		strcpya( &(bios[i].name), fn );
		LOG( "set BIOS PS[%d] SS[%d] PG[%d] NM[%s]", ps,ss,pg,bios[i].name );
	}
	return 0;
}

static void setCartType_sub(int slot, char * typ ){
		if     ( strcmp("GUESS",typ)==0 )ROMType[slot] = MAP_GUESS   ;
		else if( strcmp("GEN_8",typ)==0 )ROMType[slot] = MAP_GEN8    ;
		else if( strcmp("GEN16",typ)==0 )ROMType[slot] = MAP_GEN16   ;
		else if( strcmp("KONA5",typ)==0 )ROMType[slot] = MAP_KONAMI5 ;
		else if( strcmp("KONA4",typ)==0 )ROMType[slot] = MAP_KONAMI4 ;
		else if( strcmp("ASC_8",typ)==0 )ROMType[slot] = MAP_ASCII8  ;
		else if( strcmp("ASC16",typ)==0 )ROMType[slot] = MAP_ASCII16 ;
		else if( strcmp("GMSTR",typ)==0 )ROMType[slot] = MAP_GMASTER2;
		else if( strcmp("FMPAC",typ)==0 )ROMType[slot] = MAP_FMPAC   ;
//		else if( strcmp("SCC_P",typ)==0 )ROMType[slot] = MAP_SCC     ;
}

static int msx_setCart( lua_State *L ){
	int   slot = (int   )luaL_checknumber(L, 1);
	char* fn   = (char *)luaL_checkstring(L, 2);
	if( (slot>=0) && (slot<MAXSLOTS) ){
		strcpya( &(ROMName[slot]), fn );
		if( lua_gettop(L) == 3 ){
			char* typ  = (char *)luaL_checkstring(L, 3);
			setCartType_sub( slot, typ );
		}
	}
	return 0;
}

static int msx_setCartType( lua_State *L ){
	int   slot = (int   )luaL_checknumber(L, 1);
	char* typ  = (char *)luaL_checkstring(L, 2);
	if( (slot>=0) && (slot<MAXSLOTS) ){
		setCartType_sub( slot, typ );
	}
	return 0;
}

static int msx_bootMSX(lua_State *L){
	cls(1);
	bootMSX();
	return 0;
}

static int msx_exitMSX(lua_State *L){
	Power = 0;
	return 0;
}

static int msx_patchBDOS(lua_State *L){
	int flg= (int)lua_toboolean(L, 1);
	int ps = (int)luaL_checknumber(L, 2);
	int ss = (int)luaL_checknumber(L, 3);
	int pg = (int)luaL_checknumber(L, 4);

	if( flg ){
		PatchSlot[0]=ps;
		PatchSlot[1]=ss;
		PatchSlot[2]=pg;
	}
	Mode = ( (Mode&(~MSX_PATCHBDOS))|(flg?MSX_PATCHBDOS:0) );
	return 0;
}

static int msx_saveState(lua_State *L){
	char* fn = (char *)luaL_checkstring(L, 1);
	SaveState( fn );
	return 0;
}
static int msx_loadState(lua_State *L){
	char* fn = (char *)luaL_checkstring(L, 1);
	LoadState( fn );
	return 0;
}

//プロパティ( C -> Lua )
static int msx_getprop(lua_State *L){
	int idx,idx2;
	int i,j;
	char* nm = (char *)luaL_checkstring(L, 2);
	if     ( !strcmp(nm,"RAMPages"  ) ){ lua_pushinteger(L, RAMPages   ); }
	else if( !strcmp(nm,"VRAMPages" ) ){ lua_pushinteger(L, VRAMPages  ); }
#if 0
	else if( !strcmp(nm,"scr_x"     ) ){ lua_pushinteger(L, BACKGROUND.scroll[3].x /* SCR_X */ ); }
	else if( !strcmp(nm,"scr_y"     ) ){ lua_pushinteger(L, BACKGROUND.scroll[3].y /* SCR_Y */ ); }
	else if( !strcmp(nm,"scale_x"   ) ){ lua_pushinteger(L, BACKGROUND.bg3_rotation.hdx    ); }
	else if( !strcmp(nm,"scale_y"   ) ){ lua_pushinteger(L, BACKGROUND.bg3_rotation.vdy    ); }
#endif
	else if( !strcmp(nm,"Version"   ) ){ lua_pushinteger(L, (Mode&MSX_MODEL)  ); }
	else if( !strcmp(nm,"Video"     ) ){ lua_pushinteger(L, VIDEO(MSX_NTSC)?1:2 ); }
	else if( !strcmp(nm,"Verbose"   ) ){ lua_pushinteger(L, Verbose    ); }
	else if( !strcmp(nm,"UPeriod"   ) ){ lua_pushinteger(L, UPeriod    ); }
	else if( !strcmp(nm,"JoyTypeA"  ) ){ lua_pushinteger(L, (Mode&MSX_SOCKET1)>>4 ); }
	else if( !strcmp(nm,"JoyTypeB"  ) ){ lua_pushinteger(L, (Mode&MSX_SOCKET2)>>6 ); }
	else if( !strcmp(nm,"Trace"     ) ){ lua_pushinteger(L, CPU.Trace  ); }
	else if( !strcmp(nm,"Trap"      ) ){ lua_pushinteger(L, CPU.Trap   ); }
	else if( !strcmp(nm,"PSG"       ) ){ lua_pushinteger(L, psgON); }
	else if( !strcmp(nm,"SCC"       ) ){ lua_pushinteger(L, sccON); }
	else if( !strcmp(nm,"SCCPSlot"  ) ){ lua_pushinteger(L, SCCPSlot); }
	else if( !strcmp(nm,"SampleRate") ){ lua_pushinteger(L, SampleRate); }
	else if( !strcmp(nm,"AutoFit"   ) ){ lua_pushinteger(L, autoFitScreen); }
	else if( !strcmp(nm,"DSKName0"  ) ){ lua_pushstring(L, (DSKName[0]!=NULL?DSKName[0]:"") ); }
	else if( !strcmp(nm,"DSKName1"  ) ){ lua_pushstring(L, (DSKName[1]!=NULL?DSKName[1]:"") ); }
	else if( !strcmp(nm,"CASName"   ) ){ lua_pushstring(L, (CasName   !=NULL?CasName   :"") ); }
	else if( !strcmp(nm,"CMOSName"  ) ){ lua_pushstring(L, (CMOSName  !=NULL?CMOSName  :"") ); }
	else if( !strcmp(nm,"KanjiName" ) ){ lua_pushstring(L, (KanjiName !=NULL?KanjiName :"") ); }
	else if( !strcmp(nm,"Power"     ) ){ lua_pushboolean(L, Power ); }
	else if( !strcmp(nm,"ROMName"   ) ){ 
		lua_newtable(L);	idx = lua_gettop(L);
		for(i=0;i<MAXSLOTS;i++ ){
			lua_pushinteger(L, i);
			lua_pushstring (L,(ROMName[i]!=NULL?ROMName[i]:""));
			lua_settable(L,idx);
		}
	}
	else if( !strcmp(nm,"ROMType"   ) ){ 
		lua_newtable(L);	idx = lua_gettop(L);
		for(i=0;i<MAXSLOTS;i++ ){
			lua_pushinteger (L,ROMType[i]);
			lua_rawseti(L,idx,i);
		}
	}
	else if( !strcmp(nm,"CartMap"  ) ){
		lua_newtable(L);	idx = lua_gettop(L);
		for(i=0;i<4;i++ ){
			lua_newtable(L);	idx2 = lua_gettop(L);
			for(j=0;j<4;j++ ){
				lua_pushinteger(L, CartMap[i][j]);
				lua_rawseti(L,idx2,j+1);
			}
			lua_rawseti(L,idx,i+1);
		}
	}
	else return luaL_error(L, "MSX table does not have [%s] key.", nm );
	return 1;
}

//プロパティ( C <- Lua )
static int msx_setprop(lua_State *L){
	int i,j,idx;
	char* nm = (char *)luaL_checkstring(L, 2);
	LOG( "SetProp:[%s]", nm );
	if     ( !strcmp(nm,"RAMPages"  ) ){ if( !Power ){ RAMPages  = luaL_checkint(L, 3); } }
	else if( !strcmp(nm,"VRAMPages" ) ){ if( !Power ){ VRAMPages = luaL_checkint(L, 3); } }
/*
	else if( !strcmp(nm,"scr_x"     ) ){ BACKGROUND.scroll[3].x      = luaL_checkint(L, 3); }
	else if( !strcmp(nm,"scr_y"     ) ){ BACKGROUND.scroll[3].y      = luaL_checkint(L, 3); }
	else if( !strcmp(nm,"scale_x"   ) ){ BACKGROUND.bg3_rotation.hdx = luaL_checkint(L, 3) + (1 << 8); }
	else if( !strcmp(nm,"scale_y"   ) ){ BACKGROUND.bg3_rotation.vdy = luaL_checkint(L, 3) + (1 << 8); }
*/
	else if( !strcmp(nm,"UPeriod"   ) ){ UPeriod    = luaL_checkint(L, 3); }
	else if( !strcmp(nm,"Verbose"   ) ){ Verbose    = luaL_checkint(L, 3); }
//	else if( !strcmp(nm,"Version"   ) ){ NewMode    = ((NewMode&(~MSX_MODEL))|(luaL_checkint(L, 3)&MSX_MODEL)); }
//	else if( !strcmp(nm,"Video"     ) ){ Mode       = ((Mode&(~MSX_VIDEO  ))|((luaL_checkint(L, 3)==1)?MSX_NTSC:MSX_PAL)); }
	else if( !strcmp(nm,"JoyTypeA"  ) ){ Mode       = ((Mode&(~MSX_SOCKET1))|((luaL_checkint(L, 3)<<4)&MSX_SOCKET1)); LOG( "Set JoyTypeA: Mode = [%X]",Mode); }
	else if( !strcmp(nm,"JoyTypeB"  ) ){ Mode       = ((Mode&(~MSX_SOCKET2))|((luaL_checkint(L, 3)<<6)&MSX_SOCKET2)); LOG( "Set JoyTypeB: Mode = [%X]",Mode); }
	else if( !strcmp(nm,"Trace"     ) ){ CPU.Trace  = luaL_checkint(L, 3); }
	else if( !strcmp(nm,"Trap"      ) ){ CPU.Trap   = luaL_checkint(L, 3); }
	else if( !strcmp(nm,"PSG"       ) ){ psgON      = luaL_checkint(L, 3); }
	else if( !strcmp(nm,"SCC"       ) ){ sccON      = luaL_checkint(L, 3); }
	else if( !strcmp(nm,"SCCPSlot"  ) ){ if( !Power ){ SCCPSlot   = luaL_checkint(L, 3); } }
	else if( !strcmp(nm,"SampleRate") ){ if( !Power ){ SampleRate = luaL_checkint(L, 3); } }
	else if( !strcmp(nm,"DSKName0"  ) ){ strcpya( &DSKName[0], (char *)luaL_checkstring(L, 3)); ChangeDisk(0,DSKName[0]); }
	else if( !strcmp(nm,"DSKName1"  ) ){ strcpya( &DSKName[1], (char *)luaL_checkstring(L, 3)); ChangeDisk(1,DSKName[1]); }
	else if( !strcmp(nm,"CASName"   ) ){ strcpya( &CasName   , (char *)luaL_checkstring(L, 3)); ChangeCas(CasName);       }
	else if( !strcmp(nm,"CMOSName"  ) ){ if( !Power ){ strcpya( &CMOSName,  (char *)luaL_checkstring(L, 3) ); } }
	else if( !strcmp(nm,"KanjiName" ) ){ if( !Power ){ strcpya( &KanjiName, (char *)luaL_checkstring(L, 3) ); } }
	else if( !strcmp(nm,"Power"     ) ){ Power      = lua_toboolean(L, 3)?0:1; }
/*
	else if( !strcmp(nm,"ROMName"   ) ){
		LOG( "ROMName? Power[%d] table?[%d]",Power, lua_istable( L, 3 ) );
		if( (!Power) && lua_istable( L, 3 ) ){							// 電源ONの間は変更不可
			for(i=0;i<MAXSLOTS;i++ ){
//				LOG( "SLOT i=%d", i );
				lua_rawgeti( L, 3, i );										// 配列からデータを取り出す
				strcpya( &ROMName[i], (char*)lua_tostring(L, lua_gettop(L) ) );	// データをCの配列にセット
				lua_pop(L,1);													// データを削除
				LOG( "ROMName[%d]:[%s]",i, ROMName[i] );
			}
		}
	}
	else if( !strcmp(nm,"ROMType"   ) ){
		if( (!Power) && lua_istable( L, 3 ) ){				// 電源ONの間は変更不可
			for(i=0;i<MAXSLOTS;i++ ){
//				LOG( "SLOT i=%d", i );
				lua_rawgeti( L, 3, i );							// 配列からデータを取り出す
				ROMType[i] = lua_tointeger(L, lua_gettop(L) );	// データをCの配列にセット
				lua_pop(L,1);									// データを削除
			}
		}
	}
*/
	else if( !strcmp(nm,"AutoFit"   ) ){
		autoFitScreen = luaL_checkint(L, 3);
		if( autoFitScreen ){
//		    BACKGROUND.bg3_rotation.vdy = (1 << 8) + ((VDP[9]&0x80)?28:0);
		}
	}
	else if( !strcmp(nm,"CartMap"  ) ){
		if( (!Power) && lua_istable( L, 3 ) ){
			for(i=0;i<4;i++ ){
//				LOG( "CartMap i = %d", i );
				lua_rawgeti( L, 3, i+1 ); idx = lua_gettop(L);		// 配列から配列を取り出す
				for(j=0;j<4;j++ ){
//					LOG( "CartMap j=%d", j );
					lua_rawgeti( L, idx, j+1 )	;					// 配列からデータを取り出す
					CartMap[i][j] = lua_tonumber(L, lua_gettop(L) );	// データをCの配列にセット
					lua_pop(L,1);										// データを削除
				}
				lua_pop(L,1);
			}
		}
/*		LOG("MAP[%d,%d,%d,%d][%d,%d,%d,%d][%d,%d,%d,%d][%d,%d,%d,%d]",
				CartMap[0][0],CartMap[0][1],CartMap[0][2],CartMap[0][3],
				CartMap[1][0],CartMap[1][1],CartMap[1][2],CartMap[1][3],
				CartMap[2][0],CartMap[2][1],CartMap[2][2],CartMap[2][3],
				CartMap[3][0],CartMap[3][1],CartMap[3][2],CartMap[3][3] );
*/
	}
	else return luaL_error(L, "MSX table does not have [%s] key.", nm );
	return 0;
}

static int msx_keybind(lua_State *L){
	int num, i;
	int idx,idx2, idx3;
	LOG("msx_keybind");
	for( num=0;num<128;num++ ){
		lua_rawgeti( L, 1, num ); idx = lua_gettop(L);
		if( !lua_isnil(L,idx) ){
			lua_rawgeti( L, idx, 1 ); KeyBinds[num].type  =(u8 )luaL_checknumber(L, lua_gettop(L));
			lua_rawgeti( L, idx, 2 ); KeyBinds[num].code  =(u8 )luaL_checknumber(L, lua_gettop(L));
			lua_rawgeti( L, idx, 3 ); KeyBinds[num].mask  =(u8 )luaL_checknumber(L, lua_gettop(L));
			lua_rawgeti( L, idx, 4 ); KeyBinds[num].Hold  =(u8 )luaL_checknumber(L, lua_gettop(L));
			lua_rawgeti( L, idx, 5 ); KeyBinds[num].x     =(u16)luaL_checknumber(L, lua_gettop(L));
			lua_rawgeti( L, idx, 6 ); KeyBinds[num].y     =(u16)luaL_checknumber(L, lua_gettop(L));
			lua_rawgeti( L, idx, 7 ); KeyBinds[num].width =(u16)luaL_checknumber(L, lua_gettop(L));
			lua_rawgeti( L, idx, 8 ); KeyBinds[num].height=(u16)luaL_checknumber(L, lua_gettop(L));
			lua_pop(L,8);
//			LOG( "KB[%d] %d %d %d %d %d %d %d %d",num,KeyBinds[num].type, KeyBinds[num].code, KeyBinds[num].mask,KeyBinds[num].Hold,
//                                                   KeyBinds[num].x   ,KeyBinds[num].y     ,KeyBinds[num].width ,KeyBinds[num].height );
			lua_getfield( L, idx, "kt" ); idx2 = lua_gettop(L);
			for( i=0;i<10;i++ ){
				lua_rawgeti( L, idx2, i+1 ); idx3 = lua_gettop(L);
				lua_rawgeti( L, idx3, 1 ); KeyBinds[num].keytop[i].fnttype =(u8 )luaL_checknumber(L,lua_gettop(L));
				lua_rawgeti( L, idx3, 2 ); KeyBinds[num].keytop[i].x_mgn   =(u16)luaL_checknumber(L,lua_gettop(L));
				lua_rawgeti( L, idx3, 3 ); KeyBinds[num].keytop[i].y_mgn   =(u16)luaL_checknumber(L,lua_gettop(L));
				lua_rawgeti( L, idx3, 4 ); strncpy( KeyBinds[num].keytop[i].text, (char *)luaL_checkstring(L,lua_gettop(L)),8);
				lua_pop(L,5);
			}
			lua_pop(L,1);
		}else{
			// 終端
			KeyBinds[num].type  =0;
			KeyBinds[num].code  =0;
			KeyBinds[num].mask  =0;
			KeyBinds[num].Hold  =0;
			break;
		}
		lua_pop(L,1);
	}
	createTouchMap();
	return 0;
}

static int msx_joybind(lua_State *L){
	int num = (int)luaL_checknumber(L, 1);
	JoyBinds[num].type  =  (u8)luaL_checknumber(L, 2);
	JoyBinds[num].code  =  (u8)luaL_checknumber(L, 3);
	JoyBinds[num].mask  =  (u8)luaL_checknumber(L, 4);
	return 0;
}

/*
static int msx_getprop_ROMName(lua_State *L){
	int idx = luaL_checkinteger(L, 2);
	LOG( "getprop_ROMName idx[%d]",idx );
	lua_pushstring( L, ROMName[idx] );
	return 1;
}

static int msx_setprop_ROMName(lua_State *L){
	int idx = luaL_checkinteger(L, 2);
	LOG( "setprop_ROMName idx[%d]",idx );
	strcpya( &ROMName[idx], (char *)luaL_checkstring(L, 3) );
	return 0;
}
*/

static const luaL_Reg msxlib[] = {
	{"DskChange"   , msx_DskChange   },
	{"CasChange"   , msx_CasChange   },
	{"CasTop"      , msx_CasTop      },
	{"CasEnd"      , msx_CasEnd      },
	{"CasNext"     , msx_CasNext     },
	{"CasPrev"     , msx_CasPrev     },
	{"setBIOS"     , msx_setBIOS     },
	{"patchBDOS"   , msx_patchBDOS   },
	{"boot"        , msx_bootMSX     },
	{"exit"        , msx_exitMSX     },
	{"saveState"   , msx_saveState   },
	{"loadState"   , msx_loadState   },
	{"setCart"     , msx_setCart     },
	{"setCartType" , msx_setCartType },
	{"Keybind"     , msx_keybind     },
	{"Joybind"     , msx_joybind     },
	{NULL, NULL}
};

LUALIB_API int luaopen_msx(lua_State *L) {
	int idx, idx2;
//	int idx3;

	// 関数定義
	luaL_register(L, "MSX", msxlib);

	// MSX テーブルにメタテーブルを設定(プロパティ的操作を実現)
	lua_getglobal(L,"MSX"); idx  = lua_gettop(L);

	lua_newtable(L);		idx2 = lua_gettop(L);
	lua_pushcfunction(L, msx_getprop );
	lua_setfield(L,idx2,"__index");
	lua_pushcfunction(L, msx_setprop );
	lua_setfield(L,idx2,"__newindex");
	lua_setmetatable(L, idx );
/*
	lua_newtable(L);		idx2 = lua_gettop(L);	// ROMName テーブル
	lua_newtable(L);		idx3 = lua_gettop(L);	// ROMNameテーブルのメタテーブル
	lua_pushcfunction(L, msx_getprop_ROMName );
	lua_setfield(L,idx3,"__index");
	lua_pushcfunction(L, msx_setprop_ROMName );
	lua_setfield(L,idx3,"__newindex");
	lua_setmetatable(L, idx2 );
	lua_setfield(L,idx,"ROMName");					// ROMNameテーブルをMSXテーブルにセット
*/
	lua_pop(L, 1);									// スタックに取り出したMSXテーブルを取り除く。

	return 1;
}
