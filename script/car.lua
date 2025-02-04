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
  single   = true,
  density  = 144,
  width    = 800,
  height   = 600,
  hdepth   = 32,
  depth    = 32,
  launcher = "Car"
}
