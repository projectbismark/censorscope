local transfer = {}

local remote_hostname = "fill_in_hostname"

function transfer.sync_sandbox()
  local remote_path = remote_hostname .. ":censorscope-server/sandbox"
  local local_path = "sandbox"

  if os.execute("rsync --delete -avz " .. remote_path .. "/ " .. local_path) ~= 0 then
    return "error downloading experiments using rsync"
  end
  return nil
end

function transfer.upload_results()
  local remote_path = remote_hostname .. ":censorscope-server/results"
  local local_path = "results"

  if os.execute("rsync -avz " .. local_path .. "/ " .. remote_path) ~= 0 then
    return "error downloading experiments using rsync"
  end
  return nil
end

return transfer
