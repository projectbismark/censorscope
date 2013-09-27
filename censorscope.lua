local tcp_connect = {}

local http_request = {}

local experiments = {}
experiments.http_request = http_request
experiments.tcp_connect = tcp_connect

function get_input(file_name)
   -- Local table to store list of inputs
   local inputs = {}

   -- Open file to read from
   io.input("inputs/"..file_name)

   -- Iterate through the file and
   -- store it in "inputs" table
   for line in io.lines() do
      table.insert(inputs, line)
   end

   -- Return the list of inputs
   return inputs
end

for name, experiment in pairs(experiments) do
   experiment.exec = require("experiments."..name)
   experiment.urls = get_input(name..".txt")
   local results = experiment.exec(experiment.urls)

   for _, result in pairs(results) do
      for key, value in pairs(result) do
         print(key, value)
      end
   end
end
