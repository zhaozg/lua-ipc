#!/usr/bin/lua

local FILE = "data.txt"
local N = 3

package.cpath = "../?.so;../?.dll;"..package.cpath
local mmap = require( "ipc.mmap" )
print( "pagesize is", mmap.pagesize )

local function cat( path )
  io.write( "Contents of '", path, "':\n" )
  local f = assert( io.open( path, "rb" ) )
  for l in f:lines() do
    io.write( l, "\n" )
  end
  f:close()
  io.flush()
end

-- create a new file
local f = assert( io.open( FILE, "wb" ) )
for i = 1, N do
  f:write( "0123456789" )
end
f:write( "\n" )
f:close()

-- make sure that it has the contents above
cat( FILE )

-- open the file to mmap
local mem = assert( mmap.open( FILE, "w" ) )
-- modify the contents
for i = 1, N do
  mem:write( "9876543210" )
end
mem:close()

-- make sure that is has the new contents
io.write( "Data should be reversed now!\n" )
cat( FILE )

