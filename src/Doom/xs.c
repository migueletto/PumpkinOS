#include "xs.h"

int xstrncasecmp(char *s1, char *s2, int n) {
  char c1, c2;
  int i;

  for (i = 0; i < n && s1[i] && s2[i]; i++) {
    c1 = s1[i];
    c2 = s2[i];
    if (c1 >= 'A' && c1 <= 'Z') c1 += 32;
    if (c2 >= 'A' && c2 <= 'Z') c2 += 32;
    if (c1 < c2) return -1;
    if (c1 > c2) return 1;
  }

  return 0;
}
