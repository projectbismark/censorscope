-- Copyright (c) 2013, Sathyanarayanan Gunasekaran, Georgia Tech Research Corporation
-- See LICENSE for licensing information

local socket = require("socket")
local utils = require("experiments.utils")

function tcp_connect_experiment(urls)
  -- Store results of all the TCP Requests
  local results = {}

  for _, url in ipairs(urls) do
    -- Parse the url to get host, port
    -- TODO: Wrap this around pcall()
    host, port = parse_url(url)

    -- Do a TCP Connect
    client, msg = socket.connect(host, port)

    -- Store the result in a table.
    -- This "result" table has only
    -- information about a single TCP Request.
    local result = {}

    if client then
      result.success = true
    else
      result.success = false
    end

    result.url = url
    result.msg = msg

    -- Add this table to our list of results
    table.insert(results, result)
  end

  return results
end

function parse_url(url)
  -- This function assumes the url is of
  -- the form "<host>:<port>", if not raise
  -- an error.

  -- Check if there is a ":" separator
  separator = string.find(url, ":")

  if separator ~= nil then
    -- Split the url into host and port
    uri = utils.split(url, ":")

    -- Unpack the uri table and return the values
    return uri[1], uri[2]
  else
    -- Raise an error if URL is not formatted
    -- correctly.
    error("Require URL of the format '<host>:<ip'")
  end
end

return tcp_connect_experiment
