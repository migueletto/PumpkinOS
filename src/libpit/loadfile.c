#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "loadfile.h"
#include "sys.h"
#include "debug.h"
#include "xalloc.h"

char *load_fd(int fd, unsigned int *len) {
  char *buf;
  int nread;

  if ((*len = sys_seek(fd, 0, SYS_SEEK_END)) == -1) {
    return NULL;
  }

  if (sys_seek(fd, 0, SYS_SEEK_SET) == -1) {
    return NULL;
  }

  if ((buf = xmalloc(*len + 1)) == NULL) {
    return NULL;
  }

  if ((sys_read_timeout(fd, (uint8_t *)buf, *len, &nread, -1)) == -1) {
    xfree(buf);
    return NULL;
  }
  buf[nread] = 0;

  return buf;
}

char *load_stream(FILE *f, unsigned int *len) {
  int fd;

  if ((fd = fileno(f)) == -1) {
    debug_errno("LOAD", "fileno");
    return NULL;
  }

  return load_fd(fd, len);
}

char *load_file(char *filename, unsigned int *len) {
  int fd;
  char *s;

  if ((fd = sys_open(filename, SYS_READ)) == -1) {
    return NULL;
  }

  s = load_fd(fd, len);
  sys_close(fd);

  return s;
}
