#!/usr/bin/lua

package.cpath = "../?.so;../?.dll;"..package.cpath
local proc = require( "ipc.proc" )

local lua_command, i = arg[ -1 ], -2
while arg[ i ] ~= nil do
  lua_command, i = arg[ i ], i - 1
end

local function ___()
  io.stdout:write( ("="):rep( 70 ), "\n" )
  io.stdout:flush()
end

local options = {
  stdin = true,
  stdout = true,
  stderr = true,
  callback = function( stream, data, msg, errno )
    if data then
      io.write( stream.."\t", data:upper() )
    else
      io.stderr( "ERROR", tostring( stream ), tostring( data ),
                 tostring( msg ), tostring( errno ) )
    end
  end,
}

___''
local h = assert( proc.spawn( lua_command..' procwork.lua 1', options ) )
print( h:wait() )

___''
h = assert( proc.spawn( lua_command..' procwork.lua 2', options ) )
print( h:wait() )

___''
h = assert( proc.spawn( lua_command..' procwork.lua 3', options ) )
print( h:wait() )

___''
h = assert( proc.spawn( lua_command..' procwork.lua 4', options ) )
print( h:wait() )

___''
h = assert( proc.spawn( lua_command..' procwork.lua 5', options ) )
print( h:wait() )

___''
h = assert( proc.spawn( lua_command..' procwork.lua 6', options ) )
assert( h:write( "sending input\n", proc.EOF ) )
print( h:wait() )

___''
h = assert( proc.spawn( lua_command..' procwork.lua 7', options ) )
assert( h:write( "hallo welt,\n", "ich bin da.\n", "hallo welt,\n",
                 "alles klar?\n", proc.EOF ) )
print( h:wait() )

___''
local options2 = {
  stdin = assert( io.open( "procwork.lua", "r" ) ),
  stdout = assert( io.open( "out.txt", "w" ) ),
}
h = assert( proc.spawn( lua_command..' procwork.lua 7', options2 ) )
options2.stdin:close()
options2.stdout:close()
print( h:wait() )

___''

