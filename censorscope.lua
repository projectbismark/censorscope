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
  success, experiment.results = coroutine.resume(experiment.exec, experiment.urls)

  if not success then
    -- throw exception
    utils.pprint("Error in "..name)
  end

  for _, result in pairs(experiment.results) do
    write_result(result, experiment.output)
  end
end

function schedule(name, experiment)
  utils.pprint("Scheduling "..name)

  if experiment.rerun > 0 then
    experiment.rerun = experiment.rerun-1
  end

  if coroutine.status(experiment.exec) == "dead" then
    -- TODO: this is repeated, maybe optimize it?
    local exec = require("experiments."..name)
    experiment.exec = coroutine.create(exec)
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
        -- we run the experiment at least once, is that a good
        -- assumption to make? for eg., if the user sets rerun = 0,
        -- the experiment will still be run once
        start_experiment(name, experiment)

        -- check if we have to run this experiment again
        if experiment.rerun == 0 then
          experiments.remove(name)
        else
          schedule(name, experiment)
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
    experiment.exec = coroutine.create(exec)
  end
end

function director()
  local experiments = deck.experiments
  bootstrap(experiments)
  engine(experiments)
end

director()
