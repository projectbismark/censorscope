local run_id = os.date("%Y%m%d-%H%M%S")

local api = {}

sandbox = require("sandbox")
utils = require("utils")

-- Write a message to the debug log.
--
-- Depending on the runtime environment, this function may or may not actually
-- do anything. For example, on headless embedded devices it doesn't make any
-- sense to store log messages because nobody will see them.
--
-- Arguments:
-- - message is the message to log.
-- Returns: none
function api.log(message)
  utils.pprint(message)
end

-- Load a module from the sandboxed module directory.
-- Arguments:
-- - name is the name of the module to load, without a path or .lua extension.
-- Returns:
-- - the result(s) of importing the module.
-- - an error, or nil if no error occurred.
function api.import(name)
  if not sandbox.is_valid_module_name(name) then
    return nil, "invalid module name"
  end
  local filename = "sandbox/" .. name .. ".lua"
  return sandbox.run_file(filename, api)
end

-- Perform a DNS lookup.
--
-- Currently this just queries 8.8.8.8 and returns the first address.
--
-- Arguments:
-- - domain is the domain name to look up.
-- - resolver is the nameserver to use. If omitted or an empty string, then
-- query the system default nameserver.
-- Returns:
-- - return first IPv4 address in the result, or nil on error.
-- - an error message, or nil if no errors occurred.
function api.dns_lookup(domain, resolver)
  if resolver == nil then
    resolver = ""
  end
  return dns_lookup(domain, resolver)
end

-- Write a result to the current results file.
--
-- Each run on an experiment has one result file. This function appends to it.
--
-- Arguments:
-- - result is a Lua object to write. You may pass strings, numbers, booleans,
-- and tables containing those types. Tables may be nested.
-- Returns:
-- - an error message, or nil if no error occurred.
function api.write_result(output)
  -- TODO: Check if results/ is present, if not
  --       create it
  local experiment_name = string.match(SCRIPT_FILENAME, "([^/]+).lua$")
  local filename = string.format("results/%s-%s.txt", experiment_name, run_id)
  utils.pprint("writing results to " .. filename)

  local handle, err = io.open(filename, "a")
  if err then
    return err
  end
  handle:write(output)
  handle:write("\n")
  handle:close()
  return nil
end

-- Useful functions from the Lua standard library. These are safe accoring to
-- http://lua-users.org/wiki/SandBoxes.
api.ipairs = ipairs
api.pairs = pairs
api.string = {}
api.string.format = string.format
api.tostring = tostring
api.type = type

return api
