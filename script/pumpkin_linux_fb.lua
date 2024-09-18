function cleanup_callback()
  if pumpkin then
    pumpkin.finish()
  end
end

pit.cleanup(cleanup_callback)

wp = pit.loadlib("libfb")

if not wp then
  print("window provider not found")
  pit.finish(0)
  return
end

wp.setup(0, 1, 0)
wp.cursor(true)
pit.mount("./vfs/", "/")

pumpkin = pit.loadlib("libos")
pumpkin.init()

pumpkin.start {
  width  = 1024,
  height = 768,
  depth  = 16,
  launcher = "Launcher"
}
