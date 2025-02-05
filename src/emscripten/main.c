#include <stdio.h>
#include <emscripten.h>
#include <emscripten/html5.h>

#include "main.h"

extern int liblsdl2_load(void);

static void main_callback(int pe, void *data) {
  liblsdl2_load();
}

static void callback_func(void *userData) {
}

static EM_BOOL cb(double time, void *userData) {
fprintf(stderr, "cb\n");
  return 0;
}

int main(int argc, char *argv[]) {
  char *args[] = {
    "pumpkin",
    "-d", "1",
    "-s", "none",
    "vfs/pumpkin.lua",
    NULL
  };

  if (argc == 999) {
fprintf(stderr, "antes\n");
    //emscripten_request_animation_frame_loop(cb, NULL);
    emscripten_set_main_loop_arg(callback_func, NULL, 30, 0);
fprintf(stderr, "depois\n");
  }

  return pit_main(6, args, main_callback, NULL);
}
