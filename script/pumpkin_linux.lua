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

xdg_pumpkin_os = os.getenv("XDG_PUMPKINOS")

if not xdg_pumpkin_os then
  pit.mount("./vfs/", "/")
else
  pit.mount(xdg_pumpkin_os .. "/vfs/", "/")
end

pumpkin = pit.loadlib("libos")
pumpkin.init()
pumpkin.start(1024, 680, 16, false, false, false, "Launcher")
