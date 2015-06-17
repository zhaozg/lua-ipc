#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


#define MAGIC "\033luaipc\033"

typedef struct {
  char magic[ sizeof( MAGIC )-1 ];
  DWORD hisize;
  DWORD losize;
} ipc_shm_header;


typedef struct {
  HANDLE h; /* creator handle */
  void* raddr;
  void* addr;
  size_t len;
} ipc_shm_handle;

static void* ipc_shm_addr( ipc_shm_handle* h ) {
  return h->addr;
}

static size_t ipc_shm_size( ipc_shm_handle* h ) {
  return h->len;
}


static void ipc_shm_error( char* buf, size_t len, int code ) {
  if( len > 0 && 0 == FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM |
                                      FORMAT_MESSAGE_IGNORE_INSERTS,
                                      NULL,
                                      code,
                                      0,
                                      buf,
                                      len,
                                      NULL ) ) {
    strncpy( buf, "unknown error", len-1 );
    buf[ len-1 ] = '\0';
  }
}


static int ipc_shm_make_name( char const* s, char** name ) {
  size_t len = strlen( s );
  if( len == 0 || strcspn( s, "/\\" ) != len )
    return ERROR_INVALID_NAME;
  /* FIXME: should we try a "Global\\" name first? */
#define P "Local\\"
  *name = malloc( len + sizeof( P ) );
  if( *name == NULL )
    return ERROR_OUTOFMEMORY; /* or ERROR_NOT_ENOUGH_MEMORY?! */
  memcpy( *name, P, sizeof( P )-1 );
  memcpy( *name+sizeof( P )-1, s, len+1 );
#undef P
  return 0;
}


static int ipc_shm_create( ipc_shm_handle* h, char const* name,
                           size_t req ) {
  char* rname = NULL;
  ipc_shm_header header;
  size_t rreq = req + sizeof( header );
  int rv = ipc_shm_make_name( name, &rname );
  if( rv != 0 )
    return rv;
  if( req == 0 ) {
    free( rname );
    return ERROR_INVALID_PARAMETER;
  }
  memcpy( header.magic, MAGIC, sizeof( header.magic ) );
  header.hisize = sizeof( size_t ) > 4 ? ((req >> 16) >> 16) : 0;
  header.losize = (DWORD)req;
  h->h = CreateFileMappingA( INVALID_HANDLE_VALUE,
                             NULL,
                             PAGE_READWRITE,
                             (DWORD)(sizeof( size_t ) > 4 ?
                               ((rreq >> 16) >> 16) :
                               0),
                             (DWORD)rreq,
                             rname );
  if( h->h == NULL ) {
    int saved_errno = GetLastError();
    free( rname );
    return saved_errno;
  } else if( GetLastError() == ERROR_ALREADY_EXISTS ) {
    /* It seems we have no way of actually removing the shared memory
     * object -- should we fail with EEXIST anyway?! */
    CloseHandle( h->h );
    free( rname );
    return ERROR_ALREADY_EXISTS;
  }
  /* Windows automatically handles the lifetime of the shared memory
   * object, so we don't need to keep track of the name! */
  free( rname );
  h->raddr = MapViewOfFile( h->h,
                            FILE_MAP_ALL_ACCESS,
                            0,
                            0,
                            0 );
  if( h->raddr == NULL ) {
    int saved_errno = GetLastError();
    CloseHandle( h->h );
    return saved_errno;
  }
  *(ipc_shm_header*)(h->raddr) = header;
  h->addr = ((char*)(h->raddr))+sizeof( header );
  h->len = req;
  return 0;
}


static int ipc_shm_attach( ipc_shm_handle* h, char const* name ) {
  HANDLE hmap;
  ipc_shm_header* hptr = NULL;
  char* rname = NULL;
  int rv = ipc_shm_make_name( name, &rname );
  if( rv != 0 )
    return rv;
  hmap = OpenFileMappingA( FILE_MAP_ALL_ACCESS,
                           FALSE,
                           rname );
  if( hmap == NULL ) {
    int saved_errno = GetLastError();
    free( rname );
    return saved_errno;
  }
  free( rname );
  h->raddr = MapViewOfFile( hmap,
                            FILE_MAP_ALL_ACCESS,
                            0,
                            0,
                            0 );
  if( h->raddr == NULL ) {
    int saved_errno = GetLastError();
    CloseHandle( hmap );
    return saved_errno;
  }
  CloseHandle( hmap );
  hptr = h->raddr;
  if( memcmp( hptr->magic, MAGIC, sizeof( hptr->magic ) ) == 0 ) {
    h->len = hptr->losize;
    if( sizeof( size_t ) > 4 ) {
      h->len += (((size_t)hptr->hisize) << 16) << 16;
    } else if( hptr->hisize != 0 ) {
      UnmapViewOfFile( h->raddr );
      return ERROR_ARITHMETIC_OVERFLOW;
    }
    h->addr = ((char*)h->raddr) + sizeof( *hptr );
  } else {
    /* There's no header in the shared memory segment. Maybe some
     * other program/library created it? We can only guess the
     * approximate size of the segment! */
    MEMORY_BASIC_INFORMATION meminfo;
    if( VirtualQuery( h->raddr, &meminfo, sizeof( meminfo ) ) == 0 ) {
      int saved_errno = GetLastError();
      UnmapViewOfFile( h->raddr );
      return saved_errno;
    }
    h->addr = h->raddr;
    h->len = meminfo.RegionSize;
  }
  return 0;
}


static int ipc_shm_detach( ipc_shm_handle* h ) {
  if( !UnmapViewOfFile( h->raddr ) )
    return GetLastError();
  return 0;
}


static int ipc_shm_remove( ipc_shm_handle* h ) {
  if( !UnmapViewOfFile( h->raddr ) ) {
    int saved_errno = GetLastError();
    CloseHandle( h->h );
    return saved_errno;
  }
  CloseHandle( h->h );
  return 0;
}

