local bismark = {}

function bismark.upload_results(results_path)
  local mv_command = "mv " .. results_path .. "/* /tmp/bismark-uploads/censorscope"
  assert(os.execute(mv_command) == 0,
         "error uploading results using bismark-data-transmit")
end

return bismark
