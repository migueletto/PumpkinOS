function cleanup_callback()
  if pumpkin then
    pumpkin.finish()
  end
end

pit.cleanup(cleanup_callback)

pit.loadlib("libgpio")
pit.loadlib("libspi")
lib = pit.loadlib("libwaveshare")

if not lib then
  print("could not load a display lib")
  pit.finish(0)
  return
end

if lib.setup then
  lib.setup(320, 480, 0, 1, 24, 25, 20000000)
end

pit.mount("./vfs/", "/")

pumpkin = pit.loadlib("libos")
pumpkin.init()

pumpkin.start {
  density  = 144,
  width    = 480,
  height   = 320,
  depth    = 16,
  mode     = 1,
  dia      = true,
  hdepth   = lib.hdepth
}
