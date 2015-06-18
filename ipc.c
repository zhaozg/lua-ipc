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


IPC_LOCAL FILE* ipc_testfile( lua_State* L, int idx ) {
#if LUA_VERSION_NUM == 501 /* Lua 5.1 / LuaJIT */
  FILE** fp = luaL_testudata( L, idx, LUA_FILEHANDLE );
  if( fp == NULL )
    return NULL;
  else if( *fp == NULL )
    luaL_error( L, "attempt to use closed filed" );
  return *fp;
#elif LUA_VERSION_NUM >= 502 && LUA_VERSION_NUM <= 503
  luaL_Stream* s = luaL_testudata( L, idx, LUA_FILEHANDLE );
  if( s == NULL )
    return NULL;
  else if( s->closef == 0 || s->f == NULL )
    luaL_error( L, "attempt to use closed filed" );
  return s->f;
#else
#error unsupported Lua version
#endif
}


IPC_LOCAL int ipc_err( char const* file, int line, int code ) {
  if( code != 0 ) {
    fprintf( stderr, "[%s:%d]: error return (%d)\n",
             file, line, code );
    fflush( stderr );
  }
  return code;
}


/* implementation of compatibility functions */
#if LUA_VERSION_NUM == 501
IPC_LOCAL int ipc_absindex( lua_State* L, int idx ) {
  if( idx < 0 && idx > LUA_REGISTRYINDEX )
    idx += lua_gettop( L );
  return idx;
}


IPC_LOCAL void* ipc_testudata( lua_State* L, int idx,
                               char const* name ) {
  void* p = lua_touserdata( L, idx );
  if( p == NULL || !lua_getmetatable( L, i ) )
    return NULL;
  else {
    int res = 0;
    luaL_getmetatable( L, name );
    res = lua_rawequal( L, -1, -2 );
    lua_pop( L, 2 );
    if( !res )
      p = NULL;
  }
  return p;
}
#endif /* LUA_VERSION_NUM == 501 */

