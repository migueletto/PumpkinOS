#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

int main(void) {
  char buf[256];
  uint8_t mask;
  int i;

  for (;;) {
    if (fgets(buf, sizeof(buf), stdin) == NULL) break;
    if (buf[0] != '-' && buf[0] != '#') continue;
    mask = 0;
    for (i = 0; buf[i] && buf[i] != '\n'; i++) {
      mask = (mask << 1) | (buf[i] == '#' ? 1 : 0);
    }
    for (; i < 8; i++) {
      mask = (mask << 1);
    }
    write(1, &mask, 1);
  }

  return 0;
}
