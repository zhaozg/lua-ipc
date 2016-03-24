[![Build Status](https://travis-ci.org/siffiejoe/lua-luaipc.svg?branch=master)](https://travis-ci.org/siffiejoe/lua-luaipc)

#            LuaIPC -- Inter-Process Communication for Lua           #

##                           Introduction                           ##

Since Lua is mostly written in portable ISO C it comes with the bare
minimum of batteries. There are some extension libraries that provide
a portable (usually lowest common denominator) interface to some parts
of the operating system not covered by ISO C, like sockets
([LuaSocket][1]), or filesystem operations ([LuaFileSystem][2]).
**LuaIPC** is such a library consisting of multiple modules for
inter-process communication in Lua. It allows you to:

*   create/read/write shared memory
*   create/manipulate semaphores
*   memory-map files
*   lock/unlock (parts of) files
*   spawn subprocesses and communicate with them using unnamed pipes

The implementation should be portable to Lua 5.1-5.3 on recent Windows
and POSIX machines (tested on Win 7, Ubuntu Linux, FreeBSD, and OSX).

  [1]: http://w3.impa.br/~diego/software/luasocket/
  [2]: http://keplerproject.github.io/luafilesystem/


##                             Reference                            ##

In general all functions return a true-ish value on success (`true` if
there is nothing better to return). This applies only to functions
that do not aim for compatibility with existing Lua functions (like
e.g.  the file methods). On error, usually `nil`, an error message,
and a numeric error code are returned. All handles created by this
library are finalized during garbage-collection, so manually cleaning
up is optional under most circumstances.

The **LuaIPC** library consists of the following modules:


###                             ipc.shm                            ###

```lua
local shm = require( "ipc.shm" )
```

A shared memory segment is a piece of RAM that can be accessed by
multiple processes at the same time. Since Lua does not have any form
of pointer arithmetic, a file-like interface is provided instead.
The shared memory segment is deleted automatically when all its
handles are closed. It is unspecified whether you can still open a
new handle to an existing shared memory segment once its original
creator has closed the handle. On some OSes this module might allocate
more than the requested number of bytes (usually a multiple of the
page size). To be portable you have to agree on the real size (e.g.
by storing it at the beginning of the shared memory segment) and
adjust the file size that this module uses by calling `h:truncate()`
for all parties that attached to the shared memory (see below).

The module provides the following functions:

*   `shm.create( name, size ) ==> handle`
*   `shm.attach( name ) ==> handle`

`shm.create()` creates a new shared memory segment with the given
`name` and `size` (in bytes), and returns an attached handle to it. If
a memory segment with that `name` already exists, this function fails.
`shm.attach()` on the other hand tries to open an existing shared
memory segment with the given `name`. In both cases `name` is not a
file path: you cannot find it on the filesystem (well, on POSIX you
can), and it must not contain any directory separators!

A shared memory handle has all the methods that a normal Lua file
handle has, and additionally:

*   `h:size() ==> bytes`
*   `h:addr() ==> pointer`
*   `h:truncate( bytes ) ==> true`

`h:size()` returns the size of the shared memory segment in bytes, and
`h:addr()` returns the starting address as a lightuserdata. Unless you
use some form of FFI, this is probably not very useful for you. The
`h:truncate()` method can tell this module to use less than the
current shared memory size for its file-like interface.

The file-like methods behave exactly like their Lua file equivalents
with the following exception: Currently the `"*n"` format specifier of
`read`/`lines` is not supported. Since handles are not actually Lua
file objects, `io.type()` will return `nil` when called with such a
handle.


###                             ipc.sem                            ###

```lua
local sem = require( "ipc.sem" )
```

A semaphore is a shared integer counter that you can atomically
increment or decrement, but its value will never go below zero. If a
decrement operation would cause the value to become negative, the call
blocks (and/or returns `false` for the non-blocking/waiting calls).
The semaphore is deleted automatically when all its handles are
closed. It is unspecified whether you can still open a new handle to
an existing semaphore once its original creator has closed the handle.

The module provides the following function:

*   `sem.open( name [, n] ) ==> handle`

If the initial counter value `n` is given (and not `0`), `sem.open`
tries to create a new semaphore with the given `name`. If such a
semaphore already exists, this function fails. If `n` is zero or
absent, `sem.open` tries to open an existing semaphore with the given
`name`. Anyways, `name` is not a file path: you cannot find it on the
filesystem (well, on POSIX you can), and it must not contain any
directory separators!

A semaphore handle has the following methods:

*   `h:inc() ==> true`
*   `h:dec( [timeout] ) ==> boolean`
*   `h:close() ==> true`

`h:inc()` increments the semaphore value (if successful). `h:dec()`
tries to decrement the semaphore value. If the optional `timeout` (in
seconds, can have a fractional part) is absent or negative, the
function will block until some other process increments the semaphore.
Otherwise `h:dec()` will wait at most `timeout` seconds and return
`true` or `false` depending on the success (errors still result in
`nil`, error message, and error code).


###                            ipc.mmap                            ###

```lua
local mmap = require( "ipc.mmap" )
```

Memory-mapped I/O is usually faster than normal file I/O, because by
making the kernel buffers available at a certain memory address, you
can save the copy operations to user-supplied buffers. Memory-mapping
also has some draw-backs: Most OSes don't allow mapping a zero-length
file or very large files (e.g. larger than the address space). Also,
this module does not allow resizing the file while it is mapped, so
you can't write beyond the initial bounds of the memory-mapped file.

The module provides the following function:

*   `mmap.pagesize`
*   `mmap.open( filepath [, mode [, offset [, size]]] ) ==> handle`

`mmap.open()` opens the given `filepath` and maps the contents into
memory. `mode` can be `"r"` (the default), `"w"`, or `"rw"`. On
success a mmap handle is returned. Offsets usually must be given in
multiples of the current pages size (or equivalent). `mmap.pagesize`
contains this number. Default value for `size` is `0` which uses the
current size of the file.

An mmap handle has all the methods that a normal Lua file handle has,
and additionally:

*   `h:size() ==> bytes`
*   `h:addr() ==> pointer`
*   `h:truncate( bytes ) ==> true`

`h:size()` returns the size of the memory map in bytes, and `h:addr()`
returns the starting address as a lightuserdata. Unless you use some
form of FFI, this is probably not very useful for you. `h:truncate()`
can shrink the size of the memory mapping, but it's mostly useful for
the `ipc.shm` module (see there).

The file-like methods behave exactly like their Lua file equivalents
with the following exception: Currently the `"*n"` format specifier of
`read`/`lines` is not supported. Since handles are not actually Lua
file objects, `io.type()` will return `nil` when called with such a
handle.


###                          ipc.filelock                          ###

```lua
local filelock = require( "ipc.filelock" )
```

This module contains functions for (un-)locking byte ranges of an
opened Lua file. The locks might be advisory (you have to check for
locks yourself) or mandatory (file functions will honor the locks
automatically) depending on the OS. Also, the file locking functions
can't be used to synchonize different threads in the same process.
There is similar functionality in [LuaFileSystem][2], but in LFS all
locking is non-blocking. It is explicitly *not* supported to mix the
locking functions from LFS and from this module.

This module provides the following functions:

*   `filelock.lock( file, mode [, offset [, nbytes]] ) ==> true`
*   `filelock.trylock( file, mode [, offset [, nbytes]] ) ==> boolean`
*   `filelock.unlock( file [, offset [, nbytes]] ) ==> true`

`filelock.lock()` acquires a lock for the given range of `nbytes`
bytes starting at `offset` (default `0`) in the Lua file object. If
`nbytes` is absent (or `0`), the whole file is locked. If the given
range already is locked by another process, `filelock.lock()` will
block until the other process releases that lock. `filelock.trylock()`
is similar, but it will not block. Instead it returns false if another
process already holds a lock on the given byte range.
`filelock.unlock` unlocks the given byte range that has been locked by
the same process before.


###                            ipc.proc                            ###

```lua
local proc = require( "ipc.proc" )
```

On the most common OSes Lua provides the `io.popen()` function to
spawn a subprocess and capture its output *or* provide its input via
unnamed pipes. This module can capture `stdout` *and* `stderr`
simultaneously, while still providing input for the spawned command.
To avoid deadlocks as much as possible and to work around platform
differences the interface is callback-based.

This module provides the following functions/fields:

*   `proc.spawn( cmd, options ) ==> handle`
*   `proc.EOF ==> lightuserdata`

Similar to `os.execute()` and `io.popen()`, `proc.spawn()` takes a
command (`cmd`) as string and runs it using the shell. The `options`
table specifies the callback function (field `callback`), and which
streams to redirect via pipes (fields `stdin`, `stdout`, and `stderr`;
a true value creates a pipe, a false value uses the default streams of
the parent process -- you can also specify Lua file objects).

A process handle has the following methods:

*   `h:write( ... ) ==> true`
*   `h:kill( "term"/"kill" ) ==> true`
*   `h:wait() ==> true/nil, string, number`

`h:write()` accepts strings and enqueues them to be sent to the child
process' `stdin` stream when it is ready. You may also pass `proc.EOF`
to close the `stdin` stream when all enqueued output has been sent.
The `h:kill()` function sends either a `SIGTERM` (requesting graceful
shutdown) or a `SIGKILL` (for immediate shutdown) to the child
process. `h:wait()` starts a small blocking event loop that will send
enqueued data to the child process' `stdin`, read data from the
child's `stdout`/`stderr` streams, and/or wait for the child process
to exit. The callback given to the `proc.spawn()` function is called
with the stream name (`"stdout"` or `"stdin"`) and the received data
(or the usual error values) when output from the child is available.
On Lua 5.2+ you may yield from the callback function. Return values of
`h:wait()` are the same as for `os.execute()` on recent Lua versions.


###                           ipc.strfile                          ###

```lua
local strfile = require( "ipc.strfile" )
```

This module is not about IPC at all. It allows you to create a
file-like object from a Lua string. It shares most of the code for
file handling with the `ipc.shm` and `ipc.mmap` modules, and it is
often useful -- that's why it is here. It has exactly the same methods
as the shared memory or mmap handles (except you are not allowed to
`write`).

The following function is provided:

*   `strfile.open( str ) ==> handle`


##                              Contact                             ##

Philipp Janda, siffiejoe(a)gmx.net

Comments and feedback are always welcome.


##                              License                             ##

**LuaIPC** is *copyrighted free software* distributed under the MIT
license (the same license as Lua 5.1). The full license text follows:

    LuaIPC (c) 2015, 2016 Philipp Janda

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to
    permit persons to whom the Software is furnished to do so, subject to
    the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

The `sem_timedwait()` implementation used on OSX was written by Keith
Shortridge at the Australian Astronomical Observatory. See the
comments in `osx/sem_timedwait.c` for details.

