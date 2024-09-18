function cleanup_callback()
  if pumpkin then
    pumpkin.finish()
  end
end

pit.cleanup(cleanup_callback)

pit.loadlib("libgpio")
pit.loadlib("libspi")
wp = pit.loadlib("libwaveshare")

if not wp then
  print("window provider not found")
  pit.finish(0)
  return
end

if wp.setup then
  wp.setup(320, 480, 0, 1, 24, 25, 20000000)
end

pit.mount("./vfs/", "/")

pumpkin = pit.loadlib("libos")
pumpkin.init()

pumpkin.start {
  depth  = 16,
  dia = true,
  launcher = "Launcher"
}
