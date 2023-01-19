local _, fs = pcall(require, 'lfs')
if not _ then
  print("can't load lfs(luafilesystem)\n", fs)
  os.exit(-1)
end

local tests = {
  "str.lua",
  "mmapcat.lua",
  "mmaprev.lua",
  "proctest.lua",
  "semtest.lua",
  "shmtest.lua"
}

local function prefix()
  if arg[0] then
    local init, pos = 0, 0
    repeat
      init = pos
      pos = string.find(arg[0], '/', pos + 1, true)
    until pos == nil
    local _ = string.sub(arg[0], 1, init)
    return _
  end
  return ''
end

local PREFIX = prefix()
fs.chdir(PREFIX)
local function test(unit)
  assert(pcall(dofile, unit))
end

for i = 1, #tests do
  test(tests[i])
end

os.remove('shm.lck')
os.remove('out.txt')
os.remove('data.txt')
