#ifndef IPC_H_
#define IPC_H_

#include <stddef.h>
#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <assert.h>

#if LUA_VERSION_NUM < 502
/* lua_rawlen: Not entirely correct, but should work anyway */
# ifndef lua_rawlen
#	define lua_rawlen lua_objlen
# endif
/* lua_...uservalue: Something very different, but it should get the job done */
# ifndef lua_getuservalue
#	define lua_getuservalue lua_getfenv
# endif
# ifndef lua_setuservalue
#	define lua_setuservalue lua_setfenv
# endif
# ifndef luaL_newlib
#	define luaL_newlib(L,l) (lua_newtable(L), luaL_register(L,NULL,l))
# endif
#define lua_callk(L, na, nr, ctx, cont) \
  ((void)(ctx), (void)(cont), lua_call((L), (na), (nr)))
#define lua_pcallk(L, na, nr, err, ctx, cont) \
  ((void)(ctx), (void)(cont), lua_pcall((L), (na), (nr), (err)))

#define lua_resume(L, from, nargs) \
  ((void)(from), lua_resume((L), (nargs)))

inline static int lua_absindex (lua_State *L, int i) {
  if (i < 0 && i > LUA_REGISTRYINDEX)
    i += lua_gettop(L) + 1;
  return i;
}
#endif

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


#ifndef NDEBUG
#  if (defined( __STDC_VERSION__ ) && __STDC_VERSION__  >= 199901L) || \
      defined( __GNUC__ ) || defined( __clang__ )
#    define IPC_ERR( code ) (ipc_err( __FILE__, __LINE__, __func__, (int)(code) ))
#  elif defined( _MSC_VER ) && _MSC_VER >= 1100L
#    define IPC_ERR( code ) (ipc_err( __FILE__, __LINE__, __FUNCTION__, (int)(code) ))
#  else
#    define IPC_ERR( code ) (ipc_err( __FILE__, __LINE__, NULL, (int)(code) ))
#  endif
#else
#  define IPC_ERR( code ) ((int)(code))
#endif


#define IPC_NOTIMPLEMENTED( L ) \
  luaL_error( L, "module '%s' not implemented on this platform", \
              lua_tostring( L, 1 ) )


#define IPC_EINTR( _rv, _call ) \
  do { \
    _rv = _call; \
  } while( _rv < 0 && errno == EINTR )


#define IPC_OPTBIGINT( _t, _l, _i, _d ) \
  ((sizeof( _t ) > sizeof( lua_Integer ) && \
    sizeof( lua_Number ) > sizeof( lua_Integer )) \
    ? (_t)luaL_optnumber( _l, _i, _d ) \
    : (_t)luaL_optinteger( _l, _i, _d ))


IPC_LOCAL FILE* ipc_checkfile( lua_State* L, int idx );
IPC_LOCAL FILE* ipc_testfile( lua_State* L, int idx );
IPC_LOCAL int ipc_getuservaluefield( lua_State* L, int idx,
                                     char const* name );
IPC_LOCAL void ipc_setuservaluefield( lua_State* L, int idx,
                                      char const* name );
IPC_LOCAL int ipc_err( char const* file, int line, char const* func,
                       int code );

IPC_API int luaopen_ipc_filelock(lua_State* L);
IPC_API int luaopen_ipc_mmap(lua_State* L);
IPC_API int luaopen_ipc_proc(lua_State* L);
IPC_API int luaopen_ipc_shm(lua_State* L);
IPC_API int luaopen_ipc_strfile(lua_State* L);

#endif /* IPC_H_ */

