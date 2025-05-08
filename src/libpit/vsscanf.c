#include "sys.h"

// barebones implementation of sscanf() that supports only
// the funcionality required by PumpkinOS

#define test() \
  c = str[j++]; \
  if (p != c) { \
    return -1; \
  }

#define next() \
  c = str[j++];

int my_vsscanf(const char *str, const char *fmt, sys_va_list ap) {
  int i, j, k, len, *ip, s = 0, n = 0;
  unsigned int *up;
  char width[16], buf[128], p, c, *cp;

  for (i = 0, j = 0; fmt[i]; i++) {
    p = fmt[i];
    switch (s) {
      case 0:
        if (p == '%') {
          k = 0;
          s = 1;
        } else {
          test();
        }
        break;
      case 1:
        if (p == '%') {
          test();
          s = 0;
        } else if (p >= '0' && p <= '9') {
          if (k < 15) {
            width[k++] = p;
          }
          s = 2;
        } else {
format:
          width[k] = 0;
          switch (p) {
            case 'c':
              if (k > 0) return -1;
              cp = (char *)sys_va_arg(ap, char *);
              next();
              if (c == 0) return -1;
              *cp = c;
              n++;
              s = 0;
              break;
            case 's':
            case 'd':
            case 'u':
            case 'X':
              len = k > 0 ? sys_atoi(width) : -1;
              cp = (p == 's') ? (char *)sys_va_arg(ap, char *) : buf;
              for (k = 0; k < 126; k++) {
                next();
                if (c) {
                  if (p == 'd' || p == 'u') {
                    if (c < '0' || c > '9') {
                      if (len > 0) return -1;
                      j--;
                      break;
                    }
                  }
                  if (p == 'X') {
                    if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) {
                      if (len > 0) return -1;
                      j--;
                      break;
                    }
                  }
                }
                cp[k] = c;
                if (len > 0 && k == len-1) {
                  k++;
                  break;
                }
                if (c == 0) {
                  if (len > 0) return -1;
                  break;
                }
              }
              cp[k] = 0;
              if (p == 'd' || p == 'X') {
                ip = (int *)sys_va_arg(ap, int *);
                *ip = sys_strtol(buf, NULL, p == 'd' ? 10 : 16);
              } else if (p == 'u') {
                up = (unsigned int *)sys_va_arg(ap, unsigned int *);
                *up = sys_strtoul(buf, NULL, 10);
              }
              n++;
              s = 0;
              break;
            default:
              return -1;
          }
        }
        break;
      case 2:
        if (p >= '0' && p <= '9') {
          if (k < 15) {
            width[k++] = p;
          }
        } else {
          s = 2;
          goto format;
        }
        break;
    }
  }

  return n;
}
