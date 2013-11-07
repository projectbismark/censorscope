local sandbox = {}

local sandboxed_api = require("api")

local BYTECODE_MAGIC_NUMBER = 27

local function load_sandboxed_code(filename)
  local handle, err = io.open(filename, "r")
  if err then
    return nil, err
  end
  local sandboxed_code = handle:read("*a")  -- "*a" means "read the whole file."
  if not sandboxed_code then
    return nil, "error reading contents of file " .. filename
  end
  if sandboxed_code:byte(1) == BYTECODE_MAGIC_NUMBER then
    return nil, "cannot execute binary bytecode in the sandbox"
  end
  local sandboxed_function, err = loadstring(sandboxed_code)
  if err then
    return nil, err
  end
  return sandboxed_function, nil
end

-- Run an experiment in a sandbox.
--
-- See http://lua-users.org/wiki/SandBoxes
--
-- Arguments:
-- - filename is the filename of the Lua source to execute in the sandbox.
-- Returns:
-- - the return value of evaluation of the code, or nil if execution was unsuccessful.
-- - an error, or nil if execution was successful.
function sandbox.run(filename)
  local sandboxed_function, err = load_sandboxed_code(filename)
  if err then
    return nil, err
  end
  setfenv(sandboxed_function, sandboxed_api)
  local is_successful, return_value = pcall(sandboxed_function)
  if not is_successful then
    return nil, "error running sandboxed code: " .. return_value
  end
  return return_value, nil
end

return sandbox
