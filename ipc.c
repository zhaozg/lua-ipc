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
#elif LUA_VERSION_NUM >= 502
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
#elif LUA_VERSION_NUM >= 502
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


IPC_LOCAL int ipc_getuservaluefield( lua_State* L, int idx,
                                     char const* name ) {
  luaL_checkstack( L, 2, "not enough stack space" );
  lua_getuservalue( L, idx );
  if( lua_type( L, -1 ) == LUA_TTABLE )
    lua_getfield( L, -1, name );
  else
    lua_pushnil( L );
  lua_replace( L, -2 );
  return lua_type( L, -1 );
}


IPC_LOCAL void ipc_setuservaluefield( lua_State* L, int idx,
                                      char const* name ) {
  luaL_checkstack( L, 2, "not enough stack space" );
  lua_getuservalue( L, idx );
  if( lua_type( L, -1 ) != LUA_TTABLE )
    luaL_error( L, "attempt to set field of non-table uservalue" );
  lua_pushvalue( L, -2 );
  lua_setfield( L, -2, name );
  lua_pop( L, 2 );
}


IPC_LOCAL int ipc_err( char const* file, int line, char const* func,
                       int code ) {
  if( code != 0 ) {
    if( func != NULL )
      fprintf( stderr, "[%s:%d] error return (%d) in function '%s'\n",
               file, line, code, func );
    else
      fprintf( stderr, "[%s:%d]: error return (%d)\n",
               file, line, code );
    fflush( stderr );
  }
  return code;
}

/* LuaRocks with MSVC can't really handle multiple modules in a single
 * DLL, so we have to export the luaopen_ functions ourself, and let
 * LuaRocks think that the ipc.dll contains the ipc module: */
IPC_API int luaopen_ipc( lua_State* L ) {
  lua_newtable(L);
  luaopen_ipc_filelock(L);
  lua_setfield(L, -2, "filelock");
  luaopen_ipc_mmap(L);
  lua_setfield(L, -2, "mmap");
  luaopen_ipc_proc(L);
  lua_setfield(L, -2, "proc");
  luaopen_ipc_shm(L);
  lua_setfield(L, -2, "shm");
  luaopen_ipc_strfile(L);
  lua_setfield(L, -2, "strfile");
  return 1;
}
