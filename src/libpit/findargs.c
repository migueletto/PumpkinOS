#include "sys.h"
#include "findargs.h"
#include "xalloc.h"

#define MAX_LINE  256

static char escape_char(char c) {
  switch (c) {
    case '\\': c = '\\'; break;
    case '"':  c = '"'; break;
    case 'b':  c = 8; break;
    case 't':  c = 9; break;
    case 'n':  c = 10; break;
    case 'r':  c = 13; break;
    default:   c = 0;
  }

  return c;
}

int pit_findargs(char *buf, char *argv[], int nargs, char *(eval)(void *data, char *expr), void *data) {
  char aux[MAX_LINE], expr[MAX_LINE], *reply;
  int s, i, j, k, e, prev, argc;

  for (s = 0, i = 0, j = 0, argc = 0; buf[i]; i++) {
    switch (s) {
      case 0:
        switch (buf[i]) {
          case ' ':
            break;
          case '"':
            s = 2;
            break;
          case '`':
            if (eval) {
              e = 0;
              s = 4;
            } else {
              aux[j++] = buf[i];
              s = 1;
            }
            break;
          case '\\':
            prev = 1;
            s = 3;
            break;
          default:
            aux[j++] = buf[i];
            s = 1;
            break;
        }
        break;

      case 1:
        switch (buf[i]) {
          case ' ':
            aux[j] = 0;
            j = 0;
            if (argc < nargs) {
              argv[argc++] = xstrdup(aux);
            }
            s = 0;
            break;
          case '`':
            e = 0;
            s = 4;
            break;
          case '\\':
            prev = 1;
            s = 3;
            break;
          default:
            if (j < MAX_LINE-1) {
              aux[j++] = buf[i];
            }
            break;
        }
        break;

      case 2:
        switch (buf[i]) {
          case '"':
            aux[j] = 0;
            j = 0;
            if (argc < nargs) {
              argv[argc++] = xstrdup(aux);
            }
            s = 0;
            break;
          case '\\':
            prev = 2;
            s = 3;
            break;
          default:
            if (j < MAX_LINE-1) {
              aux[j++] = buf[i];
            }
            break;
        }
        break;

      case 3:
        if (j < MAX_LINE-1) {
          aux[j] = escape_char(buf[i]);
          if (aux[j]) j++;
        }
        s = prev;
        break;

      case 4:
        switch (buf[i]) {
          case '`':
            expr[e] = 0;
            reply = eval(data, expr);
            if (reply) {
              for (k = 0; reply[k] && j < MAX_LINE-1; k++) {
                aux[j++] = reply[k];
              }
              xfree(reply);
            }
            s = 1;
            break;
          case '\\':
            s = 5;
            break;
          default:
            if (e < MAX_LINE-1) {
              expr[e++] = buf[i];
            }
            break;
        }
        break;

      case 5:
        if (e < MAX_LINE-1) {
          expr[e] = escape_char(buf[i]);
          if (expr[e]) e++;
        }
        s = 4;
        break;
    }
  }

  if (j) {
    if (s == 1) {
      aux[j] = 0;
      if (argc < nargs) {
        argv[argc++] = xstrdup(aux);
      }
    } else {
      for (i = 0; i < argc; i++) {
        if (argv[i]) xfree(argv[i]);
      }
      argc = 0;
    }
  }

  return argc;
}
