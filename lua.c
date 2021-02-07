#include <u.h>
#include <libc.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

extern int luaopen_lpeg(lua_State *L);

void
main(int argc, char *argv[])
{
	lua_State *L;
	
	if((L = luaL_newstate()) == NULL)
		sysfatal("out of memory");
	luaL_openlibs(L);
	luaL_requiref(L, "lpeg", luaopen_lpeg, 1);
	lua_pop(L, 1);

	lua_createtable(L, argc, 0);
	for(int i = 0; i < argc; i++){
		lua_pushstring(L, argv[i]);
		lua_rawseti(L, -2, i);
	}
	lua_setglobal(L, "arg");

	if(luaL_loadfile(L, argv[1]) != LUA_OK)
		sysfatal("%s", lua_tostring(L, -1));
	for(int i = 1; i < argc; i++)
		lua_pushstring(L, argv[i]);
	if(lua_pcall(L, argc-1, LUA_MULTRET, 0) != LUA_OK)
		sysfatal("%s", lua_tostring(L, -1));
	lua_close(L);
	
	exits(nil);
}
