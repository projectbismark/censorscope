local http = require("socket.http")

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
      if status_code == 200 then
         print("Url:", url)
         print("Status: ", status_code)
      end
   end
end

experiment = HttpRequestExperiment.create({"http://www.google.com"})
experiment:run()
