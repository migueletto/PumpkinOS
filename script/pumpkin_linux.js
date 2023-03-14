function cleanup_callback() {
  if (pumpkin) {
    pumpkin.finish();
  }
}

pit.cleanup(cleanup_callback);

if (pit.loadlib("liblsdl2")) {
  pit.mount("./vfs/", "/");
  pumpkin = pit.loadlib("libos");
  pumpkin.init();
  pumpkin.start(1024, 680, 16, false, false, false, "Launcher");
} else {
  print("liblsdl2 not found");
  pit.finish(0);
}
