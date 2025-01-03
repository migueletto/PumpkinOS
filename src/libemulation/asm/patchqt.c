#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

static uint8_t buf[65536];

int main(void) {
  int fd, n, m;

  fd = open("QTERM.COM", O_RDONLY);
  n = read(fd, buf, 32768);
  close(fd);
  fprintf(stderr, "read %d bytes from QTERM.COM\n", n);

  fd = open("qp.cim", O_RDONLY);
  m = read(fd, buf + 16, 364);
  fprintf(stderr, "read %d bytes from qp.cim\n", m);
  close(fd);

  fd = open("QTERMP.COM", O_CREAT | O_WRONLY, 0644);
  n = write(fd, buf, n);
  close(fd);
  fprintf(stderr, "wrote %d bytes to QTERMP.COM\n", n);

  return 0;
}
