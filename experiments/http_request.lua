local http = require("socket.http")

function http_request_experiment(urls)
   -- Store results of all the HTTP Requests.
   local results = {}

   for _, url in ipairs(urls) do
      -- Make a HTTP request
      body, status_code, header, status_line = http.request(url)

      -- Store the result in a table.
      -- This "result" table has only
      -- information about a single HTTP Request.
      local result = {}
      result.url = url
      result.body = body
      result.status_code = status_code
      result.header = header
      result.status_line = status_line

      -- Add this table to our list of results
      table.insert(results, result)
   end

   -- Return our list of all results
   return results
end

return http_request_experiment
