#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>

#include "pdb.h"

static int prclist(char *filename) {
  int f, r = -1;

  if ((f = open(filename, O_RDONLY)) != -1) {
    r = pdb_list(f);
    close(f);
  }

  return r;
}

int main(int argc, char *argv[]) {
  if (argc == 2) {
    prclist(argv[1]);
  } else {
    fprintf(stderr, "usage: %s <filename>\n", argv[0]);
  }

  exit(0);
}
