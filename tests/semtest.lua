#!/usr/bin/lua

local dirsep = assert( package.config:match( "^([^\n]*)" ) )
package.cpath = "../?.so;../?.dll;"..package.cpath
local sem = require( "ipc.sem" )
local socket_ok, socket = pcall( require, "socket" )

local lua_command, i = arg[ -1 ], -2
while arg[ i ] ~= nil do
  lua_command, i = arg[ i ], i - 1
end

local function lua_spawn( s )
  if dirsep == "\\" then -- Windows
    return os.execute( 'start "'..s..'" "'..lua_command..'" "'..s..'"' )
  else -- assume some form of POSIX
    return os.execute( "'"..lua_command.."' '"..s.."' &" )
  end
end

if socket_ok then
  function sleep( secs )
    socket.select( nil, nil, secs )
  end
else
  function sleep( secs )
    local start = os.clock()
    while os.clock()-start < secs do
    end
  end
end

local s = assert( sem.open( "luasem", 3 ) );

for i = 1, 10 do
  lua_spawn( "semwork.lua" )
end
sleep( 10 )

s:close()

