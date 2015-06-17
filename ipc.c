#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "ipc.h"


IPC_LOCAL FILE* ipc_checkfile( lua_State* L, int idx ) {
#if LUA_VERSION_NUM == 501 /* Lua 5.1 / LuaJIT */
  FILE** fp = luaL_checkudata( L, idx, LUA_FILEHANDLE );
  if( *fp == NULL )
    luaL_error( L, "attempt to use closed file" );
  return *fp;
#elif LUA_VERSION_NUM >= 502 && LUA_VERSION_NUM <= 503
  luaL_Stream* s = luaL_checkudata( L, idx, LUA_FILEHANDLE );
  if( s->closef == 0 || s->f == NULL )
    luaL_error( L, "attempt to use closed file" );
  return s->f;
#else
#error unsupported Lua version
#endif
}


/* implementation of compatibility functions */
#if LUA_VERSION_NUM == 501
IPC_LOCAL int ipc_absindex( lua_State* L, int idx ) {
  if( idx < 0 && idx > LUA_REGISTRYINDEX )
    idx += lua_gettop( L );
  return idx;
}
#endif /* LUA_VERSION_NUM == 501 */

