-- Copyright (c) 2013, Sathyanarayanan Gunasekaran, Georgia Tech Research Corporation
-- See LICENSE for licensing information

local utils = require("experiments.utils")
local experiments = require("config")

function get_input(file_name)
  -- Load input table
  local inputs = require("inputs."..file_name)

  return inputs
end

function write_result(result, file_name)
  -- TODO: Check if file already exists
  -- TODO: Use timestamps in file_name?
  -- TODO: Check if results/ is present, if not
  --       create it

  local file_name = string.format("%s-%s.txt", file_name,
                                  os.date("%Y%m%d-%H%M%S"))
  io.output("results/"..file_name)

  -- Convert tables to string and write to file
  utils.serialize(result)
end

for name, experiment in pairs(experiments) do
  experiment.exec = require("experiments."..name)
  experiment.urls = get_input(experiment.input)

  local results = experiment.exec(experiment.urls)

  for _, result in pairs(results) do
    write_result(result, experiment.output)
  end
end
