#!/usr/bin/lua

package.cpath = "../?.so;../?.dll;"..package.cpath
local sem = require( "ipc.sem" )

local function sleep( secs )
  local start = os.clock()
  while os.clock()-start < secs do
  end
end

local s = assert( sem.open( "luasem" ) )
local ok, msg, errno = s:dec( 0 )
io.stdout:write( "peeking: ", tostring( ok ), ", ", tostring( msg ), ", ", tostring( errno ), "\n" )
if not ok then
  ok, msg, errno = s:dec( 3 )
  io.stdout:write( "waiting: ", tostring( ok ), ", ", tostring( msg ), ", ", tostring( errno ), "\n" )
end
if not ok then
  ok, msg, errno = s:dec()
  io.stdout:write( "blocking: ", tostring( ok ), ", ", tostring( msg ), ", ", tostring( errno ), "\n" )
end
io.stdout:write( "decremented!\n" )
io.stdout:flush()
sleep( 2 )
io.stdout:write( "incrementing!\n" )
io.stdout:flush()
s:inc()
s:close()
-- XXX You may want to uncomment this if you test on Windows:
--io.read()

