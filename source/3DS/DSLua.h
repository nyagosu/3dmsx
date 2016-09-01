#ifndef DSLUA_H
#define DSLUA_H

void StartLua();
int lua_LuaFunc(int flg);
int lua_DebugMenu(void);
int lua_getParam(void);
int lua_vsync(void);
int lua_setKeybind(void);
#endif
