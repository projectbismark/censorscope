local bismark = {}

function bismark.upload_results(results_path)
  local mv_command = "mv " .. results_path .. "/* /tmp/bismark-uploads/censorscope"
  assert(os.execute(mv_command) == 0,
         "error uploading results using bismark-data-transmit")
end

function bismark.sync_sandbox(sandbox_path)
  local remote_path = "bismark-downloads.noise.gatech.edu::sandbox"
  local ssh_command = "ssh -i /etc/bismark/bismark_key -l censorscope"
  local rsync_command = string.format("rsync --delete -avz -e '%s' %s %s",
                                      ssh_command,
                                      remote_path,
                                      sandbox_path)

  local exit_code = assert(os.execute(rsync_command) == 0,
                           "error downloading experiments using rsync")
  return exit_code
end

return bismark
