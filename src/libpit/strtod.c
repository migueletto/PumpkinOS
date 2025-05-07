#include "sys.h"
#include "floatscan.h"

static long double strtox(const char *s, char **p, int prec) {
  floatscan_t f;
  long double y;

  f.buffer = (char *)s;
  f.pos = 0;
  f.size = sys_strlen(s);

  y = floatscan(&f, prec, 1);
  if (p) *p = (char *)s + f.pos;

  return y;
}

float sys_strtof(const char *s, char **p) {
  return strtox(s, p, 0);
}

double sys_strtod(const char *s, char **p) {
  return strtox(s, p, 1);
}

long double sys_strtold(const char *s, char **p) {
  return strtox(s, p, 2);
}
