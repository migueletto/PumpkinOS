function cleanup_callback()
  if pumpkin then
    pumpkin.finish()
  end
end

pit.cleanup(cleanup_callback)

pit.loadlib("liblsdl2")

pit.mount("./vfs/", "/")

pumpkin = pit.loadlib("libos")
pumpkin.init()

pumpkin.start {
  single      = true,
  fullrefresh = true,
  density     = 144,
  width       = 320,
  height      = 320,
  hdepth      = 16,
  depth       = 16,
  launcher    = "Launcher"
}
