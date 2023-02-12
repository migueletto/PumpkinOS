#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "thread.h"
#include "pit_io.h"
#include "filter.h"
#include "login.h"
#include "debug.h"
#include "xalloc.h"

#define MAX_LOGIN     16
#define MAX_PASSWORD  16

static char *LOGIN = "\r\nlogin: ";
static char *PASSWORD = "\r\npassword: ";

typedef struct {
  char login[MAX_LOGIN];
  char password[MAX_LOGIN];
  char clogin[MAX_LOGIN];
  char cpassword[MAX_LOGIN];
  int s, nlogin, npassword;
} login_t;

int login_loop(conn_filter_t *next, char *login, char *password) {
  login_t data;
  uint8_t b;
  int r = 0;

  debug(DEBUG_INFO, "LOGIN", "login begin");
  xmemset(&data, 0, sizeof(login_t));
  strncpy(data.login, login, MAX_LOGIN-1);
  strncpy(data.password, password, MAX_PASSWORD-1);
  next->write(next, (uint8_t *)LOGIN, strlen(LOGIN));

  for (; !thread_must_end() && data.s != 2;) {
    if ((r = next->read(next, &b)) == -1) {
      debug(DEBUG_ERROR, "LOGIN", "login error");
      break;
    }
    if (r == 0) continue;

    switch (data.s) {
      case 0:
        if (b == 13) {
          data.clogin[data.nlogin] = 0;
          debug(DEBUG_INFO, "LOGIN", "got login [%s]", data.clogin);
          next->write(next, (uint8_t *)PASSWORD, strlen(PASSWORD));
          data.s = 1;
        } else if (b >= 32) {
          if (data.nlogin < MAX_LOGIN-1) {
            next->write(next, &b, 1);
            data.clogin[data.nlogin++] = b;
          }
        }
        break;
      case 1:
        if (b == 13) {
          data.cpassword[data.npassword] = 0;
          debug(DEBUG_INFO, "LOGIN", "got password [%s]", data.cpassword);
          if (!strcmp(data.login, data.clogin) && !strcmp(data.password, data.cpassword)) {
            debug(DEBUG_INFO, "LOGIN", "login accepetd");
            next->write(next, (uint8_t *)"\r\n", 2);
            data.s = 2;
            r = 0;
          } else {
            debug(DEBUG_ERROR, "LOGIN", "login rejected");
            next->write(next, (uint8_t *)LOGIN, strlen(LOGIN));
            data.nlogin = 0;
            data.npassword = 0;
            data.s = 0;
          }
        } else if (b >= 32) {
          if (data.npassword < MAX_PASSWORD-1) {
            data.cpassword[data.npassword++] = b;
          }
        }
        break;
    }
  }

  debug(DEBUG_INFO, "LOGIN", "login end");
  return r;
}
