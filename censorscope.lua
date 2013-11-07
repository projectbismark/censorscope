-- Copyright (c) 2013, Sathyanarayanan Gunasekaran, Georgia Tech Research Corporation
-- See LICENSE for licensing information
DEBUG = true

local api = require("api")
local sandbox = require("sandbox")
local transfer = require("transfer")
local utils = require("utils")

-- Constants
local NEVER_RUN = -1
local RUN_NOW = 0
local SECONDS_PER_MINUTE = 60

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

function is_time_to_run(experiment)
  if experiment.next_run == NEVER_RUN then
    return false
  end
  local current_time = os.time()
  if current_time < experiment.next_run then
    return false
  end
  return true
end

function run_experiment(experiment)
  -- Schedule the next run before we run the current one so that long-running
  -- experiments still run approximiately on time for the next run.
  if experiment.num_runs > 0 then
    local interval = experiment.interval * SECONDS_PER_MINUTE
    experiment.next_run = os.time() + interval
    experiment.num_runs = experiment.num_runs - 1
  else
    experiment.next_run = NEVER_RUN
  end

  -- Now run the experiment.
  -- TODO(sburnett): Implement scheduling.
  sandboxed_api = api.new_instance(experiment)
  local result, err = sandbox.run_file(experiment.path, sandboxed_api)
  if err then
    return "Error running experiment '" .. experiment.name .. "': " .. err
  end
  return nil
end

function engine(configuration)
  while next(configuration.experiments) ~= nil do  -- "While experiments isn't empty"
    for name, experiment in pairs(configuration.experiments) do
      if is_time_to_run(experiment) then
        err = run_experiment(experiment)
        if err then
          utils.pprint(err)
        end
      end

      if experiment.next_run == NEVER_RUN then
        configuration.experiments[name] = nil
      end
    end

    -- Sleep for a bit so we don't spin too much.
    -- TODO(sburnett): Use a proper scheduling library.
    utils.sleep(1)
  end
end

function load_configuration()
  -- Load the list of experiments. This lua file should just return a table with
  -- an "experiments" field, which itself is a table of names.
  --
  -- Because this runs outside an OS sandbox, it may be best to replace this
  -- with something that just reads the names out of a file directly instead of
  -- evaluating them as Lua code.
  local result, err = sandbox.run_file("sandbox/main.lua", {})
  if err then
    utils.pprint("Err reading experiments list")
    os.exit(1)
  end

  for name, experiment in pairs(result.experiments) do
    if not sandbox.is_valid_module_name(name) then
      utils.pprint("Invalid module name: " .. name)
    else
      experiment.name = name
      experiment.path = "sandbox/" .. name .. ".lua"
      if experiment.num_runs > 0 then
        experiment.next_run = RUN_NOW
        experiment.num_runs = experiment.num_runs - 1
      else
        experiment.next_run = NEVER_RUN
      end
    end
  end

  return result
end

function main()
  transfer.sync_sandbox()
  local configuration = load_configuration()
  engine(configuration)
  transfer.upload_results()
end

main()
