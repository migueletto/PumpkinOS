#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

static uint8_t buffer[65536];

int main(int argc, char *argv[]) {
  int fd0, fd1, len, n, i;

  if (argc != 4) {
    fprintf(stderr, "usage: %s <basic.rom> <len> <basic.bin>\n", argv[0]);
    exit(1);
  }

  if ((fd0 = open(argv[1], O_RDONLY)) == -1) {
    perror("open");
    exit(1);
  }

  len = atoi(argv[2]);
  if (len > sizeof(buffer)) len = sizeof(buffer);
  memset(buffer, 0, sizeof(buffer));

  if ((n = read(fd0, buffer, len)) == -1) {
    perror("read");
    close(fd0);
    exit(1);
  }
  close(fd0);

  if ((fd1 = open(argv[3], O_CREAT | O_WRONLY, 0644)) == -1) {
    perror("open");
    close(fd0);
    exit(1);
  }

  write(fd1, buffer, len);
  close(fd1);

  printf("const int ROM_SIZE = %d;\n\n", n);
  printf("const uint8_t rom[ROM_SIZE] PROGMEM = {\n  ");
  for (i = 0; i < n; i++) {
    if (i && (i % 16) == 0) printf("\n  ");
    printf("0x%02X, ", buffer[i]);
  }
  printf("\n};\n");

  exit(0);
}
