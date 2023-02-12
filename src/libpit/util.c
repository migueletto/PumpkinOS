#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "util.h"

char *getext(char *file) {
  char *ext;
  int i;

  ext = NULL;

  for (i = strlen(file)-1; i > 0; i--) {
    if (file[i] == '.') {
      ext = &file[i+1];
      break;
    }
  }

  return ext;
}
