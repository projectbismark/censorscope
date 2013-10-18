-- Copyright (c) 2013, Sathyanarayanan Gunasekaran, Georgia Tech Research Corporation
-- See LICENSE for licensing information

local experiments = {
  http_request = {
    input      = "http_request",
    output     = "http_request",
    interval   = 60,
    timeout    = 5,
    rerun      = -1
  },
  tcp_connect = {
    input      = "tcp_connect",
    output     = "tcp_connect",
    interval   = 60,
    timeout    = 5,
    rerun      = -1
  }
}

local metadata = {
  bootstrap = 60
}

local deck = {
  experiments = experiments,
  metadata    = metadata
}

return deck
