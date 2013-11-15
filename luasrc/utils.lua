-- Copyright (c) 2013, Sathyanarayanan Gunasekaran, Georgia Tech Research Corporation
-- See LICENSE for licensing information

-- Table to store all the util functions
-- we define here.
local utils = {}

-- See http://lua-users.org/wiki/SplitJoin
-- for more information
function utils.split(str, pat)
  local t = {}  -- NOTE: use {n = 0} in Lua-5.0
  local fpat = "(.-)" .. pat
  local last_end = 1
  local s, e, cap = str:find(fpat, 1)
  while s do
    if s ~= 1 or cap ~= "" then
      table.insert(t,cap)
    end
    last_end = e+1
    s, e, cap = str:find(fpat, last_end)
  end
  if last_end <= #str then
    cap = str:sub(last_end)
    table.insert(t, cap)
  end
  return t
end

function utils.pprint(format, ...)
  DEBUG = true
  if DEBUG == true then
    print(string.format("[%s] " .. format, os.date(), unpack(arg)))
  end
end

function utils.sleep(seconds)
  assert(os.execute("sleep " .. tonumber(seconds)) == 0)
end

return utils
