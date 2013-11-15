local urls = import("tcp_connect_ips")

for _, url in pairs(urls) do
  log("Connecting to " .. url)
  result, err = tcp_connect(url)
  if err then
    log("Error connecting to " .. url .. ": " .. err)
    write_result(url .. ",, " .. err)
  else
    write_result(url .. ", " .. result .. ",")
  end
end
