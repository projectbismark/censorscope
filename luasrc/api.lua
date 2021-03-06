local run_id = os.date("%Y%m%d-%H%M%S")

local api = {}

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
function api.log(formatstring, ...)
  log_info(string.format(formatstring, unpack(arg)))
end
function api.log_error(formatstring, ...)
  log_error(string.format(formatstring, unpack(arg)))
end
function api.log_debug(formatstring, ...)
  log_debug(string.format(formatstring, unpack(arg)))
end

-- Load a module from the sandboxed module directory.
--
-- The global environment of the loaded module will be that of a new sandbox,
-- not the requiring module. This is different from how requiring modules
-- normally works in in Lua.
--
-- Arguments:
-- - name is the name of the module to load, without a path or .lua extension.
-- Returns:
-- - the result(s) of requiring the module.
-- - an error, or nil if no error occurred.
function api.require(name)
  return run_in_sandbox(name)
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

-- Perform a HTTP GET request.
--
--
-- Arguments:
-- - a url to connect to
-- Returns:
-- - return first IPv4 address in the result, or nil on error.
-- - an error message, or nil if no errors occurred.
function api.http_get(url)
  return http_get(url)
end

-- Perform a TCP connect test.
--
--
-- Arguments:
-- - an IP address and port to connect to
-- Returns:
-- - true if sucessful else raise an error
function api.tcp_connect(ip, port)
  return tcp_connect(ip, port)
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
  local filename = string.format("%s/%s-%s.txt", CENSORSCOPE_OPTIONS.results_dir, SANDBOX_NAME, run_id)
  api.log("writing results to %s", filename)

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
api.select = select
api.tonumber = tonumber
api.tostring = tostring
api.type = type
api.math = {}
api.math.floor = math.floor
api.math.huge = math.huge
api.os = {}
api.os.clock = os.clock
api.os.difftime = os.difftime
api.os.time = os.time
api.string = {}
api.string.byte = string.byte
api.string.char = string.char
api.string.find = string.find
api.string.format = string.format
api.string.gsub = string.gsub
api.string.len = string.len
api.string.match = string.match
api.string.sub = string.sub
api.table = {}
api.table.concat = table.concat
api.table.insert = table.insert
api.table.remove = table.remove
api.table.sort = table.sort

return api
