#!/usr/bin/lua

package.cpath = "../?.so;../?.dll;"..package.cpath
local mmap = require( "ipc.mmap" )
print( "pagesize is", mmap.pagesize )

-- open the file to mmap
local mem = assert( mmap.open( "mmapcat.lua", "r" ) )
-- writing should not be allowed for that memory mapping:
print( mem:write( "xxx" ) )
-- `cat` the file to stdout
for s in mem:lines( 10 ) do
  io.stdout:write( s )
end
mem:close()

