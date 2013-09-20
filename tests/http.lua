local http = require("socket.http")

body, status_code, header, status_line = http.request("http://www.google.com")

print("Body")
print(body)
print("Status Code:")
print(status_code)
print("Headers")
for k,v in pairs(header) do print (k,v) end
