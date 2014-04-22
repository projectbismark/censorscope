local rsync = {}

-- Fill in SSH details here
local remote_hostname = "fill-in-hostname-here"
local remote_port = 22
local remote_user = 'fill-in-user-here'

function rsync.sync_sandbox(sandbox_path)
  local remote_path = remote_hostname .. ":censorscope-server/sandbox"
  local rsync_command = "rsync -e 'ssh -p" ..  tostring(remote_port) .. "' --delete -avz " .. remote_user .. "@" .. remote_path .. "/ " .. sandbox_path

  local exit_code = assert(os.execute(rsync_command) == 0,
                           "error downloading experiments using rsync")
  return exit_code
end

function rsync.upload_results(results_path)
  local remote_path = remote_hostname .. ":censorscope-server/results"
  local rsync_command = "rsync -e 'ssh -p " .. tostring(remote_port) .."' -avz " .. results_path .. "/ " .. remote_user .. "@" .. remote_path

  local exit_code = assert(os.execute(rsync_command) == 0,
                           "error uploading results using rsync")
  return exit_code
end

return rsync

