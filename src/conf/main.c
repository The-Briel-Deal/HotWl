#include <lua5.1/lauxlib.h>
#include <lua5.1/lua.h>
#include <lua5.1/lualib.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  char buff[256];
  int error;
  lua_State *L = lua_open();
  // I may have to allocate memory in the above declaration.
  luaL_openlibs(L);
  while (fgets(buff, sizeof(buff), stdin) != NULL) {
    error =
        luaL_loadbuffer(L, buff, strlen(buff), "line") || lua_pcall(L, 0, 0, 0);
    if (error) {
      fprintf(stderr, "%s", lua_tostring(L, -1));
      lua_pop(L, 1); /* pop error message from the stack */
    }
  }
}
