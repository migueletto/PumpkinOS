function cleanup_callback()
  if pumpkin then
    pumpkin.finish()
  end
end

pit.cleanup(cleanup_callback)

pit.mount("./vfs/", "/")

pumpkin = pit.loadlib("libos")
pumpkin.init()

pumpkin.start {
  density  = 144,
  width    = 1024,
  height   = 768,
  hdepth   = 16,
  depth    = 16,
  launcher = "Launcher"
}
