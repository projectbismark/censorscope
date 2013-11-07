local api_creater = {}

dns = require('dns')
sandbox = require("sandbox")
utils = require("utils")

function api_creater.new_instance(experiment)
  local run_id = os.date("%Y%m%d-%H%M%S")

  local api = {}

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
  -- Returns:
  -- - return first IPv4 address in the result, or nil on error.
  -- - an error message, or nil if no errors occurred.
  function api.dns_lookup(domain)
    return dns.dns_lookup(domain)
  end

  function api.write_result(result)
    -- TODO: Check if results/ is present, if not
    --       create it
    local filename = string.format("results/%s-%s.txt", experiment.name, run_id)
    utils.pprint("writing results to " .. filename)

    local handle, err = io.open(filename, "a")
    if err then
      return err
    end
    local output, err = utils.serialize(result)
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
  api.pairs = pairs
  api.ipairs = ipairs

  return api
end

return api_creater
