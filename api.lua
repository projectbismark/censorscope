local api = {}

dns = require('dns')

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

-- Write a message to the message log.
--
-- Depending on the runtime environment, this function may or may not actually
-- do anything. For example, on headless embedded devices it doesn't make any
-- sense to print log messages because nobody will see them.
--
-- Arguments:
-- - message is the message to log.
-- Returns: none
function api.log(message)
  print("[" .. os.date() .. "] " .. message)
end

return api
