#include "sys.h"
#include "util.h"

char *getext(char *file) {
  char *ext;
  int i;

  ext = NULL;

  for (i = sys_strlen(file)-1; i > 0; i--) {
    if (file[i] == '.') {
      ext = &file[i+1];
      break;
    }
  }

  return ext;
}
