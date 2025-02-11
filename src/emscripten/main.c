#include <stdio.h>
#include <emscripten.h>

#include "main.h"

void cmain(void) {
  char *args[] = {
    "pumpkin",
    "-d", "1",
    "-f", "stdout",
    "-s", "none",
    "vfs/pumpkin.lua",
    NULL
  };

  pit_main(8, args, NULL, NULL);
  // not reached: under Emscripten, pit_main never returns.
  // cleanup is handled by libos.
}

int main(int argc, char *argv[]) {
  EM_ASM(
    FS.mkdir('/vfs/app_registry');
    FS.mkdir('/vfs/app_storage');
    FS.mkdir('/vfs/app_card');
    FS.mount(IDBFS, {}, '/vfs/app_registry');
    FS.mount(IDBFS, {}, '/vfs/app_storage');
    FS.mount(IDBFS, {}, '/vfs/app_card');
    console.log('load IndexedDB');
    FS.syncfs(true, function (err) {
      console.log('call main');
      ccall('cmain', 'v');
    });
  );

  // main will exit here, but cmain will be called asynchronously
  // after IndexedDB is loaded, so leave the runtime running.
  emscripten_exit_with_live_runtime();

  return 0;
}
