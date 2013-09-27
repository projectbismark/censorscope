local utils = require("experiments.utils")

local tcp_connect = {}
local http_request = {}

local experiments = {}
experiments.http_request = http_request
experiments.tcp_connect = tcp_connect

function get_input(file_name)
   -- Local table to store list of inputs
   local inputs = {}

   -- Open file to read from
   -- TODO: Check if file exists
   io.input("inputs/"..file_name)

   -- Iterate through the file and store
   -- it in "inputs" table
   for line in io.lines() do
      table.insert(inputs, line)
   end

   -- Return the list of inputs
   return inputs
end

function write_result(result, file_name)
   -- TODO: Check if file already exists
   -- TODO: Use timestamps in file_name?
   -- TODO: Check if results/ is present, if not
   --       create it
   io.output("results/"..file_name)

   -- Convert tables to string and write to file
   utils.serialize(result)
end

for name, experiment in pairs(experiments) do
   experiment.exec = require("experiments."..name)
   experiment.urls = get_input(name..".txt")

   local results = experiment.exec(experiment.urls)

   for _, result in pairs(results) do
      write_result(result, name..".txt")
   end
end
