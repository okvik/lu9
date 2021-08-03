#include <u.h>
#include <libc.h>
#include <bio.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

extern int luaopen_p9(lua_State*);
extern int luaopen_p9_note(lua_State*);
extern int luaopen_lpeg(lua_State*);

luaL_Reg preloadlibs[] = {
	{"p9", luaopen_p9},
	{"p9.note", luaopen_p9_note},
	{"lpeg", luaopen_lpeg},
	{nil, nil}
};

char flag[] = {
	['c'] = 0, /* bytecode dump */
	['i'] = 0, /* interactive */
	['v'] = 0, /* print version */
	['w'] = 0, /* enable warnings */
};

void
usage(void)
{
	fprint(2, "usage: %s [-ivw] [script] [arg ...]\n", argv0);
	exits("usage");
}

int
iscons(int fd)
{
	int n, c;
	char buf[64];
	
	if(fd2path(fd, buf, sizeof buf) != 0)
		sysfatal("fd2path: %r");
	n = strlen(buf);
	c = strlen("/dev/cons");
	return n >= c && strcmp(buf + n - c, "/dev/cons") == 0;
}

void
luaerror(lua_State *L)
{
	fprint(2, "%s: %s\n", argv0, lua_tostring(L, -1));
	lua_pop(L, 1);
}

int
pcallerror(lua_State *L)
{
	const char *m;
	
	m = lua_tostring(L, 1);
	if(m == nil){
		if(luaL_callmeta(L, 1, "__tostring") && lua_type(L, -1) == LUA_TSTRING)
			return 1;
		else
			m = lua_pushfstring(L,
				"(error object is a %s value)", luaL_typename(L, 1));
	}
	luaL_traceback(L, L, m, 1);
	return 1;
}

int
pcall(lua_State *L, int narg, int nres)
{
	int r, base;
	
	base = lua_gettop(L) - narg;
	lua_pushcfunction(L, pcallerror);
	lua_insert(L, base);
	r = lua_pcall(L, narg, nres, base);
	lua_remove(L, base);
	return r;
}

int
pushargs(lua_State *L)
{
	int i, n;
	
	/* Push the 'arg' array to stack */
	if(lua_getglobal(L, "arg") != LUA_TTABLE)
		luaL_error(L, "'arg' is not a table");
	n = luaL_len(L, -1);
	luaL_checkstack(L, n + 1, "too many arguments");
	for(i = 1; i <= n; i++)
		lua_rawgeti(L, -i, i);
	lua_remove(L, -i);
	return n;
}

int
runrepl(lua_State *L)
{
	int r, narg;
	char *ln;
	Biobuf bin;
	
	if(Binit(&bin, 0, OREAD) == -1)
		sysfatal("Binit: %r");
	r = LUA_OK;
	while(fprint(2, "> "), (ln = Brdstr(&bin, '\n', 1)) != nil){
		r = luaL_loadstring(L, ln);
		free(ln);
		if(r != LUA_OK){
			luaerror(L);
			continue;
		}
		narg = pushargs(L);
		if((r = pcall(L, narg, LUA_MULTRET)) != LUA_OK)
			luaerror(L);
	}
	Bterm(&bin);
	return r;
}

int
runfile(lua_State *L, char *file)
{
	int narg, r;
	
	if((r = luaL_loadfile(L, file)) == LUA_OK){
		narg = pushargs(L);
		r = pcall(L, narg, LUA_MULTRET);
	}
	if(r != LUA_OK)
		luaerror(L);
	return r;
}

int
dumpwriter(lua_State *, const void *p, size_t sz, void *data)
{
	Biobuf *w;
	
	w = data;
	if(sz != 0 && Bwrite(w, p, sz) != sz)
		return -1;
	return 0;
}

int
dumpfile(lua_State *L, char *file)
{
	int r;
	Biobuf w;
	
	if((r = luaL_loadfile(L, file)) != LUA_OK)
		luaerror(L);
	if(Binit(&w, 1, OWRITE) == -1)
		sysfatal("Binit: %r");
	if(lua_dump(L, dumpwriter, &w, 0) == -1)
		sysfatal("dump: %r");
	Bterm(&w);
	return r;
}

int
luamain(lua_State *L)
{
	int argc, i;
	char **argv, *file;
	
	argc = lua_tointeger(L, 1);
	argv = lua_touserdata(L, 2);
	file = argv[0];
	
	/* GC in generational mode */
	lua_gc(L, LUA_GCGEN, 0, 0);
	
	if(flag['c']){
		dumpfile(L, file);
		lua_pushboolean(L, 1);
		return 1;
	}
	
	if(flag['w'])
		lua_warning(L, "@on", 0);

	/* Signal for libraries to ignore LUA_* env. vars */
	lua_pushboolean(L, 1);
	lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
	
	luaL_openlibs(L);
	/*
	 * Preload additional libraries.
	 * Because dynamic loading of C libraries is not
	 * supported these must be manually loaded by the host,
	 * as in the case of standard libraries above.
	 * An alternative is to register library loaders in the
	 * package.preload table, which makes them run at the
	 * time the program requires the library.
	 */
	luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
	for(luaL_Reg *lib = preloadlibs; lib->func; lib++){
		lua_pushcfunction(L, lib->func);
		lua_setfield(L, -2, lib->name);
	}
	lua_pop(L, 1);
	
	/* Create global 'arg' table */
	lua_createtable(L, argc, 0);
	lua_pushstring(L, argv0);
	lua_rawseti(L, -2, 0);
	for(i = 1; i < argc; i++){
		lua_pushstring(L, argv[i]);
		lua_rawseti(L, -2, i);
	}
	lua_setglobal(L, "arg");
	
	if(file == nil){
		file = "/fd/0";
		if(flag['i'] && iscons(0))
			file = nil;
	}
	if(file != nil)
		runfile(L, file);
	if(flag['i'])
		runrepl(L);

	lua_pushboolean(L, 1);
	return 1;
}

void
main(int argc, char *argv[])
{
	int r, v;
	lua_State *L;
	
	ARGBEGIN{
	case 'c': flag['c'] = 1; break;
	case 'i': flag['i'] = 1; break;
	case 'v': flag['v'] += 1; break;
	case 'w': flag['w'] = 1; break;
	default: usage();
	}ARGEND;
	if(flag['v']){
		if(flag['v'] == 1)
			print("%s\n", LUA_VERSION_MAJOR "." LUA_VERSION_MINOR);
		else
			print("%s\n", LUA_RELEASE);
		exits(nil);
	}

	setfcr(getfcr() & ~(FPZDIV | FPOVFL | FPINVAL));
	if((L = luaL_newstate()) == nil)
		sysfatal("out of memory");
	lua_pushcfunction(L, luamain);
	lua_pushinteger(L, argc);
	lua_pushlightuserdata(L, argv);
	r = lua_pcall(L, 2, 1, 0);
	if(r != LUA_OK)
		luaerror(L);
	v = lua_toboolean(L, -1);
	lua_close(L);
	exits(r == LUA_OK && v == 1 ? nil : "errors");
}
