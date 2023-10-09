function cleanup_callback()
  if pumpkin then
    pumpkin.finish()
  end
end

pit.cleanup(cleanup_callback)

wp = pit.loadlib("liblsdl2")

if not wp then
  print("window provider not found")
  pit.finish(0)
  return
end

pit.mount("./vfs/", "/")

pumpkin = pit.loadlib("libos")
pumpkin.init()
pumpkin.start(1024, 680, 16, false, false, false, "Launcher")
