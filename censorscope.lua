-- Copyright (c) 2013, Sathyanarayanan Gunasekaran, Georgia Tech Research Corporation
-- See LICENSE for licensing information
DEBUG = true

local utils = require("experiments.utils")
local deck = require("deck")

function get_input(file_name)
  -- Load input table
  local inputs = require("inputs."..file_name)

  return inputs
end

function write_result(result, file_name)
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

function reschedule(name, experiment)
  if experiment.rerun > 0 then
    experiment.rerun = experiment.rerun-1
  end

  experiment.last_run = os.time()
end

function engine(experiments)
  -- these are just speed optimizations, binding to local vars makes
  -- them more efficient
  local experiments = experiments
  local next = next

  while next(experiments) ~= nil do
    -- iterate through each experiment
    for name, experiment in pairs(experiments) do
      -- convert interval to seconds
      local interval = experiment.interval * 60

      local start_time = experiment.last_run + interval

      if start_time <= os.time() then
        -- TODO: check if experiment has already finished, if so,
        -- restart it
        start_experiment(name, experiment)

        -- check if we have to run this experiment again
        if experiment.rerun == 0 then
          experiments.remove(name)
        else
          reschedule(name, experiment)
        end

      end
    end
  end
end

function bootstrap(experiments)
  for name, experiment in pairs(experiments) do
    experiment.last_run = os.time()
    experiment.urls = get_input(experiment.input)

    utils.pprint("Bootstrapping "..name)

    local exec = require("experiments."..name)
    experiment.exec = coroutine.wrap(exec)
  end
end

function director()
  local experiments = deck.experiments
  bootstrap(experiments)
  engine(experiments)
end

director()
