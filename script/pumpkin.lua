function cleanup_callback()
  if pumpkin then
    pumpkin.finish()
  end
end

pit.cleanup(cleanup_callback)

hdepth = 16

if not pit.getpointer("window_provider") and pit.getenv("WAYLAND_DISPLAY") then
  pit.loadlib("libwwayland")
  hdepth = 32
end

if not pit.getpointer("window_provider") then
  pit.loadlib("liblsdl2")
  hdepth = 16
end

if not pit.getpointer("window_provider") and pit.getenv("DISPLAY") then
  pit.loadlib("libwxcb")
  hdepth = 32
end

if not pit.getpointer("window_provider") then
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
  hdepth   = hdepth,
  depth    = 16
}
