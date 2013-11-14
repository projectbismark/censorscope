local urls = import("http_urls")

for _, url in pairs(urls) do
  log("Connecting to " .. url)
  result, err = http_get(url)
  if err then
    log("Error connecting to " .. url .. ": " .. err)
    write_result(url .. ",, " .. err)
  else
    write_result(url .. ", " .. result .. ",")
  end
end
