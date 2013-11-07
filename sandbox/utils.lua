local utils = {}

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

return utils
