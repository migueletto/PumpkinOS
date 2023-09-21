#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

static void printb(uint8_t b) {
  uint8_t mask = 0x80;
  for (int i = 0; i < 8; i++) {
    printf("%c", b & mask ? '#' : ' ');
    mask >>= 1;
  }
}

int main(void) {
  uint32_t b1, b2, b3, b4;
  char buf[256];

  for (;;) {
    if (fgets(buf, sizeof(buf), stdin) == NULL) break;
    if (buf[0] == 0) break;
    if (sscanf(buf, "%2X %02X %02X %02X\n", &b1, &b2, &b3, &b4) != 4) break;
    printb(b1);
    printb(b2);
    printb(b3);
    printb(b4);
    printf("\n");
  }

  return 0;
}
