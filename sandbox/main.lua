local M = {}

M.experiments = {
  lookup_alexa_domains = {
    interval = 60,
    num_runs = 1,
  },
  lookup_alexa_domains_again = {
    interval = 600,
    num_runs = 4,
  },
}

return M
