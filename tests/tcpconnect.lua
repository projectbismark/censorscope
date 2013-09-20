local socket = require("socket")
host = "74.125.140.113"
port = 80

if arg then
   host = arg[1] or host
   port = arg[2] or port
end

client, msg = assert(socket.connect(host, port))
print(client, msg)
