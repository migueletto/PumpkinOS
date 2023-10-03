#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

int main(void) {
  char line[256];
  uint8_t b;
  int i, t;

  t = 0;
  b = 0;
  for (i = 0; i < 32*8; i++) {
    write(1, &b, 1);
    t++;
  }

  for (;;) {
    if (fgets(line, sizeof(line), stdin) == NULL) break;
    if (line[0] == 'G' || line[0] == '\n' || line[0] == 0) {
      line[strlen(line)-1] = 0;
      fprintf(stderr, "continue: [%s]\n", line);
      continue;
    }
    b = 0;
    // -#-#-
    for (i = 0; line[i] != '\n'; i++) {
      b |= line[i] == '#' ? 1 : 0;
      b <<= 1;
    }
    b <<= 2;
    fprintf(stderr, "write\n");
    write(1, &b, 1);
    t++;
  }

  for (i = 0; i < 128*8; i++) {
    write(1, &b, 1);
    t++;
  }

  fprintf(stderr, "t = %d\n", t);

  return 0;
}
