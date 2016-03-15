#!/usr/bin/lua

package.cpath = "../?.so;../?.dll;"..package.cpath
local shm = require( "ipc.shm" )
local fl = require( "ipc.filelock" )
local socket_ok, socket = pcall( require, "socket" )

local lockfile = assert( io.open( "shm.lck", "w+" ) )
local function lock()
  local ok, msg, code = fl.trylock( lockfile, "r" )
  print( "trylock", ok, msg, code )
  if not ok then
    ok, msg, code = assert( fl.lock( lockfile, "r" ) )
    print( "lock", ok, msg, code )
  end
end
local function unlock()
  assert( fl.unlock( lockfile ) )
end

if socket_ok then
  function sleep( secs )
    socket.select( nil, nil, secs )
  end
else
  function sleep( secs )
    local start = os.clock()
    while os.clock() - start < secs do end
  end
end

-- create the shared memory segment
local shm = assert( shm.create( "luashm", 200 ) )

local generation = "0"
shm:write( generation )

while generation ~= "3" do
  lock()
  shm:seek( "set", 0 )
  local newgeneration = shm:read( 1 )
  if newgeneration ~= generation then
    generation = newgeneration
    local line = shm:read( "l" )
    print( "Read:", line )
  end
  unlock()
  sleep( 1 )
end
shm:close()
lockfile:close()

print( "Done!" )

