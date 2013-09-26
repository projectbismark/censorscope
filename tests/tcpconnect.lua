local socket = require("socket")
local utils = require("utils")

function TcpConnectExperiment(urls)
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

      result.url = url
      result.msg = msg

      -- Add this table to our list of results
      table.insert(results, result)
   end
end

function parse_url(url)
   -- This function assumes the url is of
   -- the form "<host>:<port>", if not raise
   -- an error.
   
   -- Check if there is a ":" separator
   found = string.find(url, ":")

   if found ~= nil then
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

urls = {"74.125.140.113:80"}
TcpConnectExperiment(urls)
