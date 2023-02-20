function cleanup_callback()
  if pumpkin then
    pumpkin.finish()
  end
end

pit.cleanup(cleanup_callback)

pit.mount("./vfs/", "/")

pumpkin = pit.loadlib("libos")
pumpkin.init()
pumpkin.start(1024, 768, 16, false, false, false, "Launcher")
