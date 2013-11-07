local sandbox = {}

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
-- - sandboxed_function is the function object to run in the sandbox.
-- - environment in which to run the function. Be very careful.
-- Returns:
-- - the return value(s) of evaluation of the code, or nil if execution was unsuccessful.
-- - an error, or nil if execution was successful.
function sandbox.run_function(sandboxed_function, environment)
  setfenv(sandboxed_function, environment)
  local is_successful, return_value = pcall(sandboxed_function)
  if not is_successful then
    return nil, return_value
  end
  return return_value, nil
end

-- Run an experiment in a sandbox.
--
-- See http://lua-users.org/wiki/SandBoxes
--
-- Arguments:
-- - filename is the filename of the Lua source to execute in the sandbox.
-- - environment in which to run the function. Be very careful.
-- Returns:
-- - the return value(s) of evaluation of the code, or nil if execution was unsuccessful.
-- - an error, or nil if execution was successful.
function sandbox.run_file(filename, environment)
  local sandboxed_function, err = load_sandboxed_code(filename)
  if err then
    return nil, err
  end
  return sandbox.run_function(sandboxed_function, environment)
end

-- Determine whether a module name is valid. This is conservative (i.e., it may
-- exclude module names that Lua normally considers valid.) The primary purpose
-- of this function is to prevent us from accidentally loading a module outside
-- the sandbox.
--
-- Arguments:
-- - name is the name of the module to validate.
-- Returns:
-- - true if the name is valid, false otherwise.
function sandbox.is_valid_module_name(name)
  if not name:match("^[%w_-]+$") then
    return false
  end
  return true
end

return sandbox
