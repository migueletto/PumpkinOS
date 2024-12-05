#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>

#include "pdb.h"

#define MAX_RSRC 8192

static int prcbuild(char *filename, char *type, char *creator, char *name, char *rsrc[], int num) {
  pdb_t *pdb;
  uint16_t id;
  int64_t size;
  uint8_t *data;
  char *s, stype[16];
  int i, aux, f, fr, len, r = -1;

  if ((f = creat(filename, 0644)) != -1) {
    if ((pdb = pdb_new(name, type, creator)) != NULL) {
      for (i = 0; i < num; i++) {
        len = strlen(rsrc[i]);
        if (len >= 12) {
          s = &rsrc[i][len-12];
          if (sscanf(s, "%4s%04x.bin", stype, &aux) == 2 || sscanf(s, "%4s%04x.dat", stype, &aux) == 2) {
            id = aux;
            if ((fr = open(rsrc[i], O_RDONLY)) != -1) {
              if ((size = lseek(fr, 0, SEEK_END)) > 0) {
                if (lseek(fr, 0, SEEK_SET) == 0) {
                  if ((data = calloc(1, size)) != NULL) {
                    if (read(fr, data, size) == size) {
                      if (pdb_add_res(pdb, stype, id, size, data) == 0) {
                        fprintf(stderr, "Adding resource %s %5d (%d bytes)\n", stype, id, (int)size);
                      } else {
                        free(data);
                      }
                    } else {
                      free(data);
                    }
                  }
                }
              }
              close(fr);
            }
          }
        }
      }
      r = pdb_save(pdb, f);
      pdb_destroy(pdb);
    }
    close(f);
  }

  return r;
}

int main(int argc, char *argv[]) {
  char *filename = NULL, *type = NULL, *creator = NULL, *name = NULL;
  char *rsrc[MAX_RSRC];
  int i, num;

  for (i = 1, num = 0; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
        case 'f':
          filename = argv[++i];
          break;
        case 't':
          type = argv[++i];
          break;
        case 'c':
          creator = argv[++i];
          break;
        case 'n':
          name = argv[++i];
          break;
      }
    } else {
      if (num < MAX_RSRC) {
        rsrc[num++] = argv[i];
      }
    }
  }

  if (filename && type && creator && name && num > 0 && strlen(type) == 4 && strlen(creator) == 4) {
    prcbuild(filename, type, creator, name, rsrc, num);
  } else {
    fprintf(stderr, "usage: %s -f <filename> -t <type> -c <creator> -n <name> rsrc.bin ...\n", argv[0]);
  }

  exit(0);
}
