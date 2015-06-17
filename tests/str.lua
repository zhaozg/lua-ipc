#!/usr/bin/lua

package.cpath = "../?.so;../?.dll;"..package.cpath
local ms = require( "ipc.strfile" )
local s = [[
Hallo Welt,
ich bin da!
Hallo Welt,
alles klar?!
Ich bin hier
wie bestellt,
was sagst Du nun,
hallo Welt?!
]]
local f = assert( ms.open( s ) )

print( f:write( "xxx" ) )
print( f:addr(), f:size(), #s )
for l in f:lines() do
  print( ">"..l.."<" )
end
print( f:seek( "end" ) )
print( f:seek( "set" ) )
local s2 = f:read( "*a" )
print( s == s2 )

