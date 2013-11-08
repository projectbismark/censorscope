local domains = import("alexa_100")

write_result("domain, ip, error")
for _, domain in pairs(domains) do
  log("Looking up " .. domain)
  address, err = dns_lookup(domain)
  if err then
    log("Error looking up " .. domain .. ": " .. err)
    write_result(domain .. ",, " .. err)
  else
    write_result(domain .. ", " .. address .. ",")
  end
end
