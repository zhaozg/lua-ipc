#!/usr/bin/lua

local dirsep = assert( package.config:match( "^([^\n]*)" ) )
local socket_ok, socket = pcall( require, "socket" )

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

lua_spawn( "shmread.lua" )
sleep( 2 )
dofile( "shmwrite.lua" )
sleep( 2 )


