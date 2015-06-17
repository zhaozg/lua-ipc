#!/usr/bin/lua

package.cpath = "../?.so;../?.dll;"..package.cpath
local shm = require( "ipc.shm" )
local fl = require( "ipc.filelock" )

local lockfile = assert( io.open( "shm.lck", "w+" ) )
local function lock()
  assert( fl.lock( lockfile, "w" ) )
end
local function unlock()
  assert( fl.unlock( lockfile ) )
end

local function sleep( secs )
  local start = os.clock()
  while os.clock() - start < secs do end
end

-- attach to the shared memory segment
local shm = assert( shm.attach( "luashm" ) )
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

