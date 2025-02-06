#include <stdio.h>

#include "main.h"

int main(int argc, char *argv[]) {
  char *args[] = {
    "pumpkin",
    "-d", "1",
    "-s", "none",
    "vfs/pumpkin.lua",
    NULL
  };

  return pit_main(6, args, NULL, NULL);
}
