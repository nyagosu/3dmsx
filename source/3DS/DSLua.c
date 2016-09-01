#include "msx.h"
#include <3ds.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "DSDriver.h"
#include "DSLuaMSX.h"
#include "DSLuaUI.h"
#include "DSLuaDBG.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

lua_State *lua = NULL;

void StartLua(char * luaname )
{
	char * luapath[4] = { "fat:/", "fat:/fmsxDS/", "fat:/emu/fmsxDS/", "fat:/emulator/fmsxDS/" };
	char * pkgpath    = "package.path=\"fat:/?.lua;fat:/fmsxDS/?.lua;fat:/emu/fmsxDS/LUA/?.lua;fat:/emulator/fmsxDS/LUA/?.lua\"";
	int ret;
	int cnt = 0;
	char nm[64];
	lua = lua_open();
	if( lua == NULL ) return;
	luaL_openlibs(lua);	// internal lib open
	luaopen_msx(lua);	// MSX Lib open
	luaopen_ui(lua);	// ui Lib open

	/* package ÉpÉXê›íË */
	luaL_loadstring( lua, pkgpath );
	ret = lua_pcall(lua, 0, 0, 0);
	switch( ret ){
	case LUA_ERRRUN:LOG( "[RUNERR]%s",(char*)luaL_checkstring(lua, -1) ); break;
	case LUA_ERRMEM:LOG( "[MEMERR]%s",(char*)luaL_checkstring(lua, -1) ); break;
	case LUA_ERRERR:LOG( "[ERRERR]%s",(char*)luaL_checkstring(lua, -1) ); break;
	case 0 : LOG( "package path setting ok." ); break;
	default: LOG( "other error? code=%d msg=%s.",ret,(char*)luaL_checkstring(lua, -1)  );
	}

	while( cnt<4 ){
		strcpy( nm, luapath[cnt++] );
		strcat( nm, luaname );
		ret = luaL_loadfile(lua, nm);
		if( ret != LUA_ERRFILE ) break;
	}
	switch( ret ){
	case LUA_ERRSYNTAX :LOG( "[ERRSYNTAX]%s",(char*)luaL_checkstring(lua, -1) );break;
	case LUA_ERRMEM    :LOG( "[ERRMEM]%s"   ,(char*)luaL_checkstring(lua, -1) );break;
	case LUA_ERRFILE   :LOG( "[ERRFILE]%s is not found in '%s', '%s' or '%s'", luaname, luapath[0], luapath[1], luapath[2] ); break;
	default: LOG( "%s load ok.",luaname );
	}
	if( ret == 0 ){
		sprintf( nm, "BASEDIR=\"%s\"", luapath[cnt-1] );
		luaL_loadstring( lua, nm );
		ret = lua_pcall(lua, 0, 0, 0);
		switch( ret ){
		case LUA_ERRRUN:LOG( "[RUNERR]%s",(char*)luaL_checkstring(lua, -1) ); break;
		case LUA_ERRMEM:LOG( "[MEMERR]%s",(char*)luaL_checkstring(lua, -1) ); break;
		case LUA_ERRERR:LOG( "[ERRERR]%s",(char*)luaL_checkstring(lua, -1) ); break;
		case 0 : LOG( "BaseDir setting ok." ); break;
		default: LOG( "other error? code=%d msg=%s.",ret,(char*)luaL_checkstring(lua, -1)  );
		}
		
		ret = lua_pcall(lua, 0, 0, 0);
		switch( ret ){
		case LUA_ERRRUN:LOG( "[RUNERR]%s",(char*)luaL_checkstring(lua, -1) ); break;
		case LUA_ERRMEM:LOG( "[MEMERR]%s",(char*)luaL_checkstring(lua, -1) ); break;
		case LUA_ERRERR:LOG( "[ERRERR]%s",(char*)luaL_checkstring(lua, -1) ); break;
		case 0: LOG( "%s normal end",luaname ); break;
		default: LOG( "other error? code=%d.",ret );
		}
	}
	lua_close(lua);
}

int lua_LuaFunc(int flg)
{
	int ret;
	char buf[32];
	sprintf( buf, "LuaFunc(%d);", flg );
	ret = luaL_loadstring( lua, buf );
	switch( ret ){
	case LUA_ERRSYNTAX :LOG( "[ERRSYNTAX]%s",(char*)luaL_checkstring(lua, -1) ); break;
	case LUA_ERRMEM    :LOG( "[ERRMEM]%s"   ,(char*)luaL_checkstring(lua, -1) ); break;
	case LUA_ERRFILE   :LOG( "[ERRFILE]%s"  ,(char*)luaL_checkstring(lua, -1) ); break;
	}
	if(!ret) lua_pcall(lua,0,0,0);
	return 1;
}

int lua_vsync()
{
	if( lua ){
		int	ret = luaL_loadstring( lua, "vsync()" );
		if( !ret ) lua_pcall(lua,0,0,0);
		
	}
	return 1;
}

int lua_setKeybind()
{
	int ret = 0;
	char buf[32];
	if( lua ){
		sprintf( buf, "Keybind(%d);", KeyboardType );
		ret = luaL_loadstring( lua, buf );
		switch( ret ){
		case LUA_ERRSYNTAX :LOG( "[ERRSYNTAX]%s",(char*)luaL_checkstring(lua, -1) ); break;
		case LUA_ERRMEM    :LOG( "[ERRMEM]%s"   ,(char*)luaL_checkstring(lua, -1) ); break;
		}
		if( !ret ) lua_pcall(lua,0,0,0);
	}
	return 1;
}

int lua_DebugMenu()
{
//	LOG( "DebugMenu" );
	int ret = 0;
	if( lua ){
		ret = luaL_loadstring( lua, "DebugMenu()" );
//		LOG( "DebugMenu 2" );
		switch( ret ){
		case LUA_ERRSYNTAX :LOG( "[ERRSYNTAX]%s",(char*)luaL_checkstring(lua, -1) ); break;
		case LUA_ERRMEM    :LOG( "[ERRMEM]%s"   ,(char*)luaL_checkstring(lua, -1) ); break;
		}
//		LOG( "DebugMenu 3" );
		if( !ret ) lua_pcall(lua,0,0,0);
/*
		switch( ret ){
		case LUA_ERRRUN:LOG( "[RUNERR]%s",(char*)luaL_checkstring(lua, -1) ); break;
		case LUA_ERRMEM:LOG( "[MEMERR]%s",(char*)luaL_checkstring(lua, -1) ); break;
		case LUA_ERRERR:LOG( "[ERRERR]%s",(char*)luaL_checkstring(lua, -1) ); break;
		default: LOG( "DebugMenu normal end" );
		}
*/
//		ret = luaL_checknumber(lua, 1);
	}
//	LOG( "DebugMenu 4" );
	return 1;
}
