#!/usr/bin/lua

package.cpath = "../?.so;../?.dll;"..package.cpath
local shm = require( "ipc.shm" )
local fl = require( "ipc.filelock" )
local socket_ok, socket = pcall( require, "socket" )

local lockfile = assert( io.open( "shm.lck", "w+" ) )
local function lock()
  assert( fl.lock( lockfile, "w" ) )
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

-- attach to the shared memory segment
local shm = assert( shm.attach( "luashm" ) )
shm:truncate( 200 ) -- we "guess" the correct size
print( shm:addr(), shm:size() )

for i = 1, 3 do
  repeat
    sleep( 0.5 )
    lock()
    shm:seek( "set", 0 )
    local generation = assert( shm:read( 1 ) )
    unlock()
    local gennum = tonumber( generation )
  until gennum ~= nil and gennum < i
  lock()
  print( "Writing ..." )
  shm:seek( "set", 0 )
  shm:write( i )
  shm:write( "hello world no. ", i, "\n" )
  sleep( 3 )
  unlock()
end
shm:close()
lockfile:close()

print( "Done!" )

