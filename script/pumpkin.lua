function cleanup_callback()
  if pumpkin then
    pumpkin.finish()
  end
end

pit.cleanup(cleanup_callback)

fullrefresh = false
abgr = false

lib = pit.loadlib("liblsdl3")

if not lib then
  lib = pit.loadlib("liblsdl2")
end

if lib and pit.osname() == "Windows" then
  fullrefresh = true
end

if not lib and pit.getenv("WAYLAND_DISPLAY") then
  lib = pit.loadlib("libwwayland")
  if lib then pit.loadlib("libaalsa") end
end

if not lib and pit.getenv("DISPLAY") then
  lib = pit.loadlib("libwxcb")
  if lib then pit.loadlib("libaalsa") end
end

if not lib and custom_load then
  lib = custom_load()
end

if not lib then
  print("could not load a display lib")
  pit.finish(0)
  return
end

if pit.osname() == "Haiku" then
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
