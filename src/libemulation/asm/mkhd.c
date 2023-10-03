#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "disk.h"

static uint8_t buf[HD_TRACKS * HD_SECTORS * SECTOR_LEN];

int main(int argc, char *argv[]) {
  memset(buf, 0xE5, sizeof(buf));
  write(1, buf, sizeof(buf));
  exit(0);
}
