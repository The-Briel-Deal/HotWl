#include <lua5.1/lauxlib.h>
#include <lua5.1/lua.h>
#include <lua5.1/lualib.h>
#include <stdlib.h>
#include <string.h>

// Returns new_term_key
char *load(char *filename) {
  lua_State *L = lua_open();
  luaopen_base(L);
  luaopen_io(L);
  luaopen_string(L);
  luaopen_math(L);

  if (luaL_loadfile(L, filename) || lua_pcall(L, 0, 0, 0))
    luaL_error(L, "cannot run configuration file: %s", lua_tostring(L, -1));

  lua_getglobal(L, "new_term_key");
  if (!lua_isstring(L, -1))
    luaL_error(L, "`new_term_key` should be a string\n");
  const char *new_term_key_global = lua_tostring(L, -1);

  char *new_term_key = calloc(16, sizeof(char));
  strncpy(new_term_key, new_term_key_global, sizeof(char *));

  lua_close(L);
  return new_term_key;
}
