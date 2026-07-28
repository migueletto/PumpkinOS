function cleanup_callback()
  if pumpkin then
    pumpkin.finish()
  end
end

pit.cleanup(cleanup_callback)

abgr = false

if custom_load then
  lib = custom_load()
end

if not lib and pit.getenv("WAYLAND_DISPLAY") then
  lib = pit.loadlib("libwwayland")
  if lib then pit.loadlib("libaalsa") end
end

if not lib then
  lib = pit.loadlib("libray")
  if lib then
    pit.loadlib("libaalsa")
    abgr = true
  end
end

if not lib and pit.getenv("DISPLAY") then
  lib = pit.loadlib("libwxcb")
  if lib then pit.loadlib("libaalsa") end
end

if not lib then
  lib = pit.loadlib("liblsdl2")
end

if not lib then
  lib = pit.loadlib("libfb")
  if lib then
    lib.setup(0, 1, 0)
    lib.cursor(true)
  end
end

if not lib then
  lib = pit.loadlib("libwnull")
  lib.setup("127.0.0.1")
end

if not lib then
  print("could not load a display lib")
  pit.finish(0)
  return
end

fullrefresh = false
if pit.getenv("BE_HOST_CPU") then
  fullrefresh = true
end

pit.mount("./vfs/", "/")

pumpkin = pit.loadlib("libos")
pumpkin.init()

pumpkin.start {
  density  = 144,
  width    = 1024,
  height   = 768,
  abgr     = abgr,
  fullrefresh = fullrefresh,
  depth    = 16,
  hdepth   = lib.hdepth
}
