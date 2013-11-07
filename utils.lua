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

-- Check http://www.lua.org/pil/12.1.1.html
-- for more information
function utils.serialize(o)
  -- TODO: We should make an optimistic tostring call
  --       and check if it errors, instead of just giving
  --       an error anyways
  if type(o) == "number" then
    return string.format("%d", o), nil
  elseif type(o) == "string" then
    return string.format("%q", o), nil
  elseif type(o) == "table" then
    s = "{\n"
    for k, v in pairs(o) do
      c, err = utils.serialize(v)
      if err then
        return nil, err
      end
      s = s .. string.format(" %s = %s,\n", k, c)
    end
    s = s .. "}"
    return s, nil
  elseif type(o) == "boolean" then
    return tostring(o), nil
  else
    return nil, "cannot serialize a " .. type(o)
  end
end

function utils.pprint(data)
  if DEBUG == true then
    print(string.format("[%s] %s", os.date(), data))
  end
end

function utils.sleep(seconds)
  assert(os.execute("sleep " .. tonumber(seconds)) == 0)
end

return utils
