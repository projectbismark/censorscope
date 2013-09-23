http = require("socket.http")

HttpRequestExperiment = {}
HttpRequestExperiment.__index = HttpRequestExperiment

function HttpRequestExperiment.create(urls)
   local experiment = {}
   setmetatable(experiment, HttpRequestExperiment)
   experiment.urls = urls
   return experiment
end

function HttpRequestExperiment:run()
   for _, url in pairs(self.urls) do
      body, status_code, header, status_line = http.request(url)
      print("Body:")
      print(body)
      print("Status Code:")
      print(status_code)
      print("Headers")
      for k,v in pairs(header) do print (k,v) end
   end
end

experiment = HttpRequestExperiment.create({"http://www.google.com"})
experiment:run()
