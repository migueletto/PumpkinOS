function cleanup_callback()
  if pumpkin then
    pumpkin.finish()
  end
end

pit.cleanup(cleanup_callback)

wp = pit.loadlib("libwwayland")

if not wp then
  print("window provider not found")
  pit.finish(0)
  return
end

pit.mount("./vfs/", "/")

pumpkin = pit.loadlib("libos")
pumpkin.init()

pumpkin.start {
  density  = 144,
  width    = 1024,
  height   = 768,
  hdepth   = 32,
  depth    = 16
}
