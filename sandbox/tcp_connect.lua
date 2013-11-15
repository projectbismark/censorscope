local ips = import("tcp_connect_ips")

for ip, port in pairs(ips) do
  log(string.format("%s %s:%d", "Connecting to", ip, port))
  result, err = tcp_connect(ip, port)
  if err then
    log("Error connecting to " .. ip .. ": " .. err)
    write_result(string.format("%s:%d %s", ip, port, err))
  else
    write_result(string.format("%s:%d %s", ip, port, result))
  end
end
