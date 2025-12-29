ExUnit.start(capture_log: true)

windows? = match?({:win32, _}, :os.type())

if not windows? and not File.exists?("/dev/shm") do
  ExUnit.configure(exclude: [:shm_tmpfs, :shm_resizable])
end

if windows?, do: ExUnit.configure(exclude: [:not_windows])
