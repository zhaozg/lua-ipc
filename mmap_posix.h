#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


typedef struct {
  void* addr;
  size_t len;
} ipc_mmap_handle;

static void* ipc_mmap_addr( ipc_mmap_handle* h ) {
  return h->addr;
}

static size_t ipc_mmap_size( ipc_mmap_handle* h ) {
  return h->len;
}


static void ipc_mmap_error( char* buf, size_t len, int code ) {
  if( len > 0 && strerror_r( code, buf, len ) != (int)0 ) {
    strncpy( buf, "unknown error", len-1 );
    buf[ len-1 ] = '\0';
  }
}


static int ipc_mmap_open( ipc_mmap_handle* h, char const* name,
                          int mode ) {
  int fd, oflags = 0, mmflags = 0;
  struct stat buf;
  if( (mode & MEMFILE_RW) == MEMFILE_RW ) {
    oflags = O_RDWR;
    mmflags = PROT_READ | PROT_WRITE;
  } else if( mode & MEMFILE_R ) {
    oflags = O_RDONLY;
    mmflags = PROT_READ;
  } else if( mode & MEMFILE_W ) {
    oflags = O_RDWR;
    mmflags = PROT_WRITE;
  }
#ifdef O_CLOEXEC
  flags |= O_CLOEXEC;
#endif
  fd = open( name, oflags );
  if( fd < 0 )
    return IPC_ERR( errno );
  /* figure out its size */
  if( fstat( fd, &buf ) < 0 ) {
    int saved_errno = errno;
    close( fd );
    return IPC_ERR( saved_errno );
  }
  if( buf.st_size > ~((size_t)0) ) {
    close( fd );
    return IPC_ERR( EFBIG );
  }
  h->len = buf.st_size;
  /* create mmap */
  h->addr = mmap( NULL, h->len, mmflags, MAP_SHARED, fd, 0 );
  if( h->addr == MAP_FAILED ) {
    int saved_errno = errno;
    close( fd );
    return IPC_ERR( saved_errno );
  }
  close( fd ); /* we don't need it anymore! */
  return 0;
}


static int ipc_mmap_close( ipc_mmap_handle* h ) {
  int rv = munmap( h->addr, h->len );
  if( rv < 0 )
    return IPC_ERR( errno );
  return 0;
}


#if defined( _POSIX_SYNCHRONIZED_IO ) && _POSIX_SYNCHRONIZED_IO > 0
#  define IPC_MMAP_HAVE_FLUSH
static int ipc_mmap_flush( ipc_mmap_handle* h, size_t pos ) {
  int rv = msync( h->addr, pos, MS_ASYNC|MS_INVALIDATE );
  if( rv < 0 )
    return IPC_ERR( errno );
  return 0;
}
#endif

