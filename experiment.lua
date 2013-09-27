local tcp_connect = {}
tcp_connect.urls = {"74.125.140.113:80"}

local http_request = {}
http_request.urls = {"http://www.google.com"}

local experiments = {}
experiments.http_request = http_request
experiments.tcp_connect = tcp_connect

for name, experiment in pairs(experiments) do
   experiment.exec = require("experiments."..name)
   local results = experiment.exec(experiment.urls)

   for _, result in pairs(results) do
      for key, value in pairs(result) do
         print(key, value)
      end
   end
end
