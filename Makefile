.PHONY: all clean install

# works for Lua 5.2 on Debian based Linuxes:
LUA_INCDIR = /usr/include/lua5.2
DLL_INSTALL_DIR = /usr/local/lib/lua/5.2
EXTRAFLAGS = -pthread
EXTRALIBS = -lrt

CFLAGS = -Wall -fpic -O2 -g
INCLUDES = -I${LUA_INCDIR}
LIBFLAG = -shared
LIB_EXTENSION = so

DLLNAME = ipc.${LIB_EXTENSION}
SOURCES = ipc.c memfile.c strfile.c shm.c mmap.c sem.c flock.c proc.c
OBJECTS = ${SOURCES:%.c=%.o}

all: ${DLLNAME}

${DLLNAME}: ${OBJECTS}
	${CC} ${EXTRAFLAGS} ${LIBFLAG} -o $@ ${OBJECTS} ${EXTRALIBS}

.c.o:
	${CC} ${CFLAGS} ${EXTRAFLAGS} ${INCLUDES} -c -o $@ $<

install: ${DLLNAME}
	mkdir -p ${DLL_INSTALL_DIR}
	cp ${DLLNAME} ${DLL_INSTALL_DIR}

clean:
	rm -f ${DLLNAME} ${OBJECTS}

# dependencies
ipc.o: ipc.c ipc.h
memfile.o: memfile.c memfile.h ipc.h Makefile
strfile.o: strfile.c memfile.h ipc.h Makefile
shm.o: shm.c memfile.h ipc.h shm_posix.h shm_win.h Makefile
mmap.o: mmap.c memfile.h ipc.h mmap_posix.h mmap_win.h Makefile
sem.o: sem.c ipc.h sem_posix.h sem_win.h Makefile
flock.o: flock.c ipc.h flock_posix.h flock_win.h Makefile
proc.o: proc.c ipc.h proc_posix.h proc_win.h Makefile

