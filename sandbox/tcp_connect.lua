local ips = require("tcp_connect_ips")

for ip, port in pairs(ips) do
  log(string.format("%s %s:%d", "Connecting to", ip, port))
  result, err = tcp_connect(ip, port)
  if err then
    log("Error connecting to " .. ip .. ": " .. err)
    write_result(string.format("%s:%d %s", ip, port, err))
  else
    -- the result is a boolean, we need to convert it to a string
    result = tostring(result)
    write_result(string.format("%s:%d %s", ip, port, result))
  end
end
