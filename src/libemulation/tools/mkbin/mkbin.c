#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

static uint8_t buffer[65536];

int main(int argc, char *argv[]) {
  int fd0, fd1, start, len;

  if (argc != 5) {
    fprintf(stderr, "usage: %s <file1.bin> <start> <len> <file2.bin>\n", argv[0]);
    exit(1);
  }

  if ((fd0 = open(argv[1], O_RDONLY | O_BINARY)) == -1) {
    perror("open");
    exit(1);
  }

  start = strtoul(argv[2], NULL, 0);
  len = strtoul(argv[3], NULL, 0);

  if (lseek(fd0, start, SEEK_SET) != start) {
    perror("seek");
    close(fd0);
    exit(1);
  }

  if (len > sizeof(buffer)) len = sizeof(buffer);
  memset(buffer, 0, sizeof(buffer));

  if ((len = read(fd0, buffer, len)) == -1) {
    perror("read");
    close(fd0);
    exit(1);
  }
  close(fd0);

  if ((fd1 = open(argv[4], O_CREAT | O_WRONLY | O_BINARY, 0644)) == -1) {
    perror("open");
    close(fd0);
    exit(1);
  }

  write(fd1, buffer, len);
  close(fd1);

  exit(0);
}
