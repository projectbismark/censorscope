utils = require("experiments.utils")

local tcpconnect = {}
tcpconnect.urls = {"74.125.140.113:80"}

local http = {}
http.urls = {"http://www.google.com"}

local experiments = {}
experiments.http = http
experiments.tcpconnect = tcpconnect

for name, experiment in pairs(experiments) do
   experiment.exec = require("experiments."..name)
   local results = experiment.exec(experiment.urls)

   for _, result in pairs(results) do
      for key, value in pairs(result) do
         print(key, value)
      end
   end
end
