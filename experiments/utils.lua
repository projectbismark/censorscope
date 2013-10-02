# Copyright (c) 2013, Sathyanarayanan Gunasekaran, Georgia Tech Research Corporation
# See LICENSE for licensing information

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
      io.write(o)
   elseif type(o) == "string" then
      io.write(string.format("%q", o))
   elseif type(o) == "table" then
      io.write("{\n")
      for k,v in pairs(o) do
         io.write("  ", k, " = ")
         utils.serialize(v)
         io.write(",\n")
      end
      io.write("}\n")
   elseif type(o) == "boolean" then
      io.write(tostring(o))
   else
      error("cannot serialize a " .. type(o))
   end
end

return utils
