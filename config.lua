-- Copyright (c) 2013, Sathyanarayanan Gunasekaran, Georgia Tech Research Corporation
-- See LICENSE for licensing information

local experiments = {}

experiments["http_request"] = {
   ["input"] = "http_request.txt",
   ["output"] = "http_request.txt",
}

experiments["tcp_connect"] = {
   ["input"] = "tcp_connect.txt",
   ["output"] = "tcp_connect.txt"
}

return experiments
