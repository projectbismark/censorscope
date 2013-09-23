local socket = require("socket")

TcpConnectExperiment = {}
TcpConnectExperiment.__index = TcpConnectExperiment

function TcpConnectExperiment.create(uris)
   local experiment = {}
   setmetatable(experiment, TcpConnectExperiment)
   experiment.uris = uris
   return experiment
end

function TcpConnectExperiment:run()
   for host, port in pairs(self.uris) do
      client, msg = socket.connect(host, port)
      if client then
         print("Success")
      else
         print("Failure", msg)
      end
   end
end

uris = {["74.125.140.113"] = "80"}
experiment = TcpConnectExperiment.create(uris)
experiment:run()
