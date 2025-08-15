#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

/**
 * Boilerplate to create an in-memory shared file.
 *
 * Link with `-lrt`.
 */

static void randname(char *buf) {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  long r = ts.tv_nsec;
  for (int i = 0; i < 6; ++i) {
    buf[i] = 'A'+(r&15)+(r&16)*2;
    r >>= 5;
  }
}

static int anonymous_shm_open(void) {
  char name[] = "/hello-wayland-XXXXXX";
  int retries = 100;

  do {
    randname(name + strlen(name) - 6);

    --retries;
    // shm_open guarantees that O_CLOEXEC is set
    int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (fd >= 0) {
      shm_unlink(name);
      return fd;
    }
  } while (retries > 0 && errno == EEXIST);

  return -1;
}

int create_shm_file(off_t size) {
  int fd = anonymous_shm_open();
  if (fd < 0) {
    return fd;
  }

  if (ftruncate(fd, size) < 0) {
    close(fd);
    return -1;
  }

  return fd;
}
