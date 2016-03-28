#include "msx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
//#include "DSGraph.h"
#include "DSDriver.h"

extern int DAsm(char *S,word A);

static int dbg_setReg(lua_State *L){
	char* nm = (char *)luaL_checkstring(L, 1);
	if     ( !strcmp(nm,"AF"  )){  CPU.AF.W  = luaL_checkint(L, 2); }
	else if( !strcmp(nm,"BC"  )){  CPU.BC.W  = luaL_checkint(L, 2); }
	else if( !strcmp(nm,"DE"  )){  CPU.DE.W  = luaL_checkint(L, 2); }
	else if( !strcmp(nm,"HL"  )){  CPU.HL.W  = luaL_checkint(L, 2); }
	else if( !strcmp(nm,"IX"  )){  CPU.IX.W  = luaL_checkint(L, 2); }
	else if( !strcmp(nm,"IY"  )){  CPU.IY.W  = luaL_checkint(L, 2); }
	else if( !strcmp(nm,"PC"  )){  CPU.PC.W  = luaL_checkint(L, 2); }
	else if( !strcmp(nm,"SP"  )){  CPU.SP.W  = luaL_checkint(L, 2); }
	else if( !strcmp(nm,"AF1" )){  CPU.AF1.W = luaL_checkint(L, 2); }
	else if( !strcmp(nm,"BC1" )){  CPU.BC1.W = luaL_checkint(L, 2); }
	else if( !strcmp(nm,"DE1" )){  CPU.DE1.W = luaL_checkint(L, 2); }
	else if( !strcmp(nm,"HL1" )){  CPU.HL1.W = luaL_checkint(L, 2); }
	else if( !strcmp(nm,"IFF" )){  CPU.IFF   = luaL_checkint(L, 2); }
	else if( !strcmp(nm,"I"   )){  CPU.I     = luaL_checkint(L, 2); }
	else if( !strcmp(nm,"R"   )){  CPU.R     = luaL_checkint(L, 2); }
	return 0;
}

static int dbg_getReg(lua_State *L){
	lua_createtable(L, 0, 15);
	lua_pushstring(L, "AF"  ); lua_pushinteger( L, CPU.AF.W  ); lua_settable (L,1);
	lua_pushstring(L, "BC"  ); lua_pushinteger( L, CPU.BC.W  ); lua_settable (L,1);
	lua_pushstring(L, "DE"  ); lua_pushinteger( L, CPU.DE.W  ); lua_settable (L,1);
	lua_pushstring(L, "HL"  ); lua_pushinteger( L, CPU.HL.W  ); lua_settable (L,1);
	lua_pushstring(L, "IX"  ); lua_pushinteger( L, CPU.IX.W  ); lua_settable (L,1);
	lua_pushstring(L, "IY"  ); lua_pushinteger( L, CPU.IY.W  ); lua_settable (L,1);
	lua_pushstring(L, "PC"  ); lua_pushinteger( L, CPU.PC.W  ); lua_settable (L,1);
	lua_pushstring(L, "SP"  ); lua_pushinteger( L, CPU.SP.W  ); lua_settable (L,1);
	lua_pushstring(L, "AF1" ); lua_pushinteger( L, CPU.AF1.W ); lua_settable (L,1);
	lua_pushstring(L, "BC1" ); lua_pushinteger( L, CPU.BC1.W ); lua_settable (L,1);
	lua_pushstring(L, "DE1" ); lua_pushinteger( L, CPU.DE1.W ); lua_settable (L,1);
	lua_pushstring(L, "HL1" ); lua_pushinteger( L, CPU.HL1.W ); lua_settable (L,1);
	lua_pushstring(L, "IFF" ); lua_pushinteger( L, CPU.IFF   ); lua_settable (L,1);
	lua_pushstring(L, "I"   ); lua_pushinteger( L, CPU.I     ); lua_settable (L,1);
	lua_pushstring(L, "R"   ); lua_pushinteger( L, CPU.R     ); lua_settable (L,1);
	return 1;
}

static int dbg_setMem(lua_State *L){
	int adr = luaL_checknumber(L, 1);
	WrZ80(adr, luaL_checknumber(L, 2));
	return 0;
}

static int dbg_getMem(lua_State *L){
	int adr = luaL_checknumber(L, 1);
	int dat = RdZ80(adr);
	lua_pushinteger( L, dat );
	return 1;
}

static int dbg_setVDP(lua_State *L){
	int adr = luaL_checknumber(L, 1 );
	VDP[adr] = luaL_checknumber( L, 2 );
	return 0;
}

static int dbg_getVDP(lua_State *L){
	int adr = luaL_checknumber(L, 1 );
	lua_pushinteger( L, VDP[adr]  );
	return 1;
}

static int dbg_dasm(lua_State *L){
	char buf[256];
	int adr = luaL_checknumber(L, 1 );
	int ret = DAsm( buf, adr );
	lua_pushinteger( L, ret  );
	lua_pushstring( L, buf  );
	return 2;
}

static const luaL_Reg dbglib[] = {
	{"setReg"      , dbg_setReg		 },
	{"getReg"      , dbg_getReg		 },
	{"setMem"      , dbg_setMem      },
	{"getMem"      , dbg_getMem      },
	{"setVDP"      , dbg_setVDP      },
	{"getVDP"      , dbg_getVDP      },
	{"dasm"        , dbg_dasm        },
	{NULL, NULL}
};

LUALIB_API int luaopen_dbg(lua_State *L) {
//	int idx, idx2;
	// 関数定義
	luaL_register(L, "DEBUG", dbglib);

/*
	// テーブルにメタテーブルを設定(プロパティ的操作を実現)
	lua_getglobal(L,"DEBUG"); idx  = lua_gettop(L);
	lua_newtable(L);		  idx2 = lua_gettop(L);
	lua_pushcfunction(L, dbg_getprop );
	lua_setfield(L,idx2,"__index");
	lua_pushcfunction(L, dbg_setprop );
	lua_setfield(L,idx2,"__newindex");
	lua_setmetatable(L, idx );
	lua_pop(L, idx);
*/
	return 1;
}
