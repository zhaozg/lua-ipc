#ifndef IPC_H_
#define IPC_H_

#include <stddef.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>


#ifndef IPC_LOCAL
/* emulate CLANG feature checking on other compilers */
#  ifndef __has_attribute
#    define __has_attribute( _x ) 0
#  endif
#  if !defined( _WIN32 ) && !defined( __CYGWIN__ ) && \
      ((defined( __GNUC__ ) && __GNUC__ >= 4 ) || \
       __has_attribute( __visibility__ ))
#    define IPC_LOCAL __attribute__((__visibility__("hidden")))
#  else
#    define IPC_LOCAL
#  endif
#endif

#ifndef IPC_API
#  ifdef _WIN32
#    define IPC_API __declspec(dllexport)
#  else
#    define IPC_API extern
#  endif
#endif


/* maximum expected length of error messages */
#define IPC_MAXERRMSG 200


#define IPC_NOTIMPLEMENTED( L ) \
  luaL_error( L, "module '%s' not implemented on this platform", \
              lua_tostring( L, 1 ) )


IPC_LOCAL FILE* ipc_checkfile( lua_State* L, int idx );
IPC_LOCAL FILE* ipc_testfile( lua_State* L, int idx );


/* compatibility functions for older Lua versions */
#if LUA_VERSION_NUM == 501
IPC_LOCAL int ipc_absindex( lua_State* L, int idx );
#define lua_absindex( L, i ) ipc_absindex( L, i )

IPC_LOCAL void* ipc_testudata( lua_State* L, int idx,
                               char const* name );
#define luaL_testudata( L, i, n ) ipc_testudata( L, i, n )

#define lua_setuservalue( L, i ) lua_setfenv( L, i )
#define lua_getuservalue( L, i ) lua_getfenv( L, i )

#define luaL_newlib( L, r ) \
  (lua_newtable( L ), luaL_register( L, NULL, r ))
#endif /* LUA_VERSION_NUM == 501 */


#endif /* IPC_H_ */

