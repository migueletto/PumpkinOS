#include <PalmOS.h>

#include "heap.h"
#include "tos.h"
#include "plibc.h"
#include "debug.h"

int CommandMain(int argc, char *argv[]) {
  UInt32 width, height;
  int r = -1;

  if (argc >= 4) {
    WinScreenGetAttribute(winScreenWidth, &width);
    WinScreenGetAttribute(winScreenHeight, &height);

    if (width == 640 && height == 400) {
      debug(DEBUG_INFO, "TOS", "running TOS command \"%s\" on drive %c directory \"%s\" with %d argument(s)", argv[3], argv[1][0], argv[2], argc - 4);
      r = tos_main(argv[1][0], argv[2], argv[3], argc - 1, &argv[1]);
      debug(DEBUG_INFO, "TOS", "TOS command \"%s\" ended", argv[1]);
      plibc_puts("\x1b[2J\x1b[H");
    } else {
      plibc_puts("TOS apps can only run with the 8x16 font.\r\n");
    }
  }

  return r;
}
