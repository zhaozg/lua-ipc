#!/usr/bin/lua

if arg[ 1 ] == "1" then
  io.stdout:write( "hello world\n" )
elseif arg[ 1 ] == "2" then
  io.stderr:write( "hello world\n" )
elseif arg[ 1 ] == "3" then
  io.stdout:write( "hello world\n" )
  io.stderr:write( "hello world\n" )
elseif arg[ 1 ] == "4" then
  error( "an error message" )
elseif arg[ 1 ] == "5" then
  -- no input/output!
elseif arg[ 1 ] == "6" then
  local l = io.read()
  io.write( l, "\n" )
elseif arg[ 1 ] == "7" then
  for l in io.lines() do
    io.write( l, "\n" )
  end
end

