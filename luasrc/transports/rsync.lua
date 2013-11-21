local rsync = {}

local remote_hostname = "fill_in_hostname"

function rsync.sync_sandbox(sandbox_path)
  local remote_path = remote_hostname .. ":censorscope-server/sandbox"
  local rsync_command = "rsync --delete -avz " .. remote_path .. "/ " .. sandbox_path

  local exit_code = assert(os.execute(rsync_command) == 0,
                           "error downloading experiments using rsync")
  return exit_code
end

function rsync.upload_results(results_path)
  local remote_path = remote_hostname .. ":censorscope-server/results"
  local rsync_command = "rsync -avz " .. results_path .. "/ " .. remote_path

  local exit_code = assert(os.execute(rsync_command) == 0,
                           "error downloading experiments using rsync")
  return exit_code
end

return rsync
