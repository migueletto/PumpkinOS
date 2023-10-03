#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

static uint8_t buffer[65536*10];

int main(int argc, char *argv[]) {
  int fd, n, i, j, last;

  if (argc != 2) {
    fprintf(stderr, "usage: %s <disk0.dsk>\n", argv[0]);
    exit(1);
  }

  if ((fd = open(argv[1], O_RDONLY | O_BINARY)) == -1) {
    perror("open");
    exit(1);
  }

  if ((n = read(fd, buffer, sizeof(buffer))) == -1) {
    perror("read");
    close(fd);
    exit(1);
  }
  close(fd);

  last = -1;
  for (i = 0; i < n; i += 128) {
    for (j = 0; j < 128; j++) {
      if (buffer[i+j]) break;
    }
    if (j < 128) last = i;
  }
  n = last + 128;

  printf("#define DISK0_SIZE %d\n\n", n);
  printf("const uint8_t disk0[DISK0_SIZE] = {\n  ");
  for (i = 0; i < n; i++) {
    if (i && (i % 16) == 0) printf("\n  ");
    printf("0x%02X, ", buffer[i]);
  }
  printf("\n};\n");

  exit(0);
}
