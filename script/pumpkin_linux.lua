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

pumpkin.start {
  density  = 144,
  width    = 1024,
  height   = 768,
  hdepth   = 16,
  depth    = 16,
  launcher = "Launcher"
}

--[[
pumpkin.start {
  density  = 72,
  xfactor  = 2,
  yfactor  = 2,
  width    = 512,
  height   = 384,
  hdepth   = 16,
  depth    = 8,
  launcher = "Launcher"
}
]]--
