#include <PalmOS.h>

#include "heap.h"
#include "tos.h"
#include "debug.h"

int CommandMain(int argc, char *argv[]) {
  int r = -1;

  if (argc >= 2) {
    debug(DEBUG_INFO, "TOS", "running TOS command \"%s\" with %d argument(s)", argv[1], argc - 2);
    r = tos_main_vfs(argv[1], argc - 1, &argv[1]);
  }

  return r;
}
