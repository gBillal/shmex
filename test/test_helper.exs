ExUnit.start(capture_log: true)

if not match?({:win32, _}, :os.type()) and not File.exists?("/dev/shm") do
  ExUnit.configure(exclude: [:shm_tmpfs, :shm_resizable])
end
