-- Copyright (c) 2013, Sathyanarayanan Gunasekaran, Georgia Tech Research Corporation
-- See LICENSE for licensing information
DEBUG = true

local utils = require("experiments.utils")
local deck = require("deck")

local experiments = deck.experiments

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

  utils.pprint("Writing results to "..file_name)
  io.output("results/"..file_name)

  -- Convert tables to string and write to file
  utils.serialize(result)
end

function start_experiment(name, experiment)
  utils.pprint("Starting "..name)
  --  this is a synchronous call, should we do it async?
  experiment.results = experiment.exec(experiment.urls)

  for _, result in pairs(experiment.results) do
    write_result(result, experiment.output)
  end
end

function engine()
  -- iterate through each experiment
  for name, experiment in pairs(experiments) do
    -- convert start_time to seconds
    local start_time = experiment.interval * 60
    -- elapsed_time is slightly misleading
    local elapsed_time = experiment.bootstrap_time + start_time

    if elapsed_time > os.time() then
      -- TODO: check if experiment has already finished, if so,
      -- restart it
      start_experiment(name, experiment)
      -- TODO: reschedule experiment
    end
  end
end

function bootstrap()
  for name, experiment in pairs(experiments) do
    experiment.bootstrap_time = os.time()
    experiment.urls = get_input(experiment.input)

    utils.pprint("Bootstrapping "..name)

    local exec = require("experiments."..name)
    experiment.exec = coroutine.wrap(exec)
  end
end

function director()
  bootstrap()
  engine()
end

director()
