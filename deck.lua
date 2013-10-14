-- Copyright (c) 2013, Sathyanarayanan Gunasekaran, Georgia Tech Research Corporation
-- See LICENSE for licensing information

local experiments = {
  http_request = {
    input      = "http_request",
    output     = "http_request",
    start_time = 60,
    timeout    = 5
  },
  tcp_connect = {
    input      = "tcp_connect",
    output     = "tcp_connect",
    start_time = 60,
    timeout    = 5
  }
}

local metadata = {
  poll_server = 60
}

local deck = {
  experiments = experiments,
  metadata    = metadata
}

return deck
