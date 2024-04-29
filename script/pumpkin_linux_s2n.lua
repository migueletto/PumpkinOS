function cleanup_callback()
  if pumpkin then
    pumpkin.finish()
  end
end

function readfile(name)
  local f = io.open(name, "r")
  local content = f:read("*all")
  f:close()
  return content
end

pit.cleanup(cleanup_callback)

s = pit.loadlib("libls2n")
key = readfile("key.pem")
s.key(key)
cert = readfile("certificate.pem")
s.cert(cert)

wp = pit.loadlib("liblsdl2")

if not wp then
  print("window provider not found")
  pit.finish(0)
  return
end

pit.mount("./vfs/", "/")

pumpkin = pit.loadlib("libos")
pumpkin.init()
pumpkin.start(1024, 680, 16, false, false, false, "Launcher")
