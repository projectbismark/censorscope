local rsync = {}

local remote_hostname = "fill_in_hostname"

function rsync.sync_sandbox()
  local remote_path = remote_hostname .. ":censorscope-server/sandbox"
  local local_path = "sandbox"
  local rsync_command = "rsync --delete -avz " .. remote_path .. "/ " .. local_path

  local exit_code = assert(os.execute(rsync_command) == 0,
                           "error downloading experiments using rsync")
  return exit_code
end

function rsync.upload_results()
  local remote_path = remote_hostname .. ":censorscope-server/results"
  local local_path = "results"
  local rsync_command = "rsync -avz " .. local_path .. "/ " .. remote_path

  local exit_code = assert(os.execute(rsync_command) == 0,
                           "error downloading experiments using rsync")
  return exit_code
end

return rsync
