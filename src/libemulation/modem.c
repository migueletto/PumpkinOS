#include "sys.h"
#include "filter.h"
#include "modem.h"
#include "xalloc.h"

#define MAX_BUF  256
#define MAX_CMD  64

typedef struct {
  char cmd[MAX_CMD];
  int n;
  int echo;
  int commandMode;
  int plus;
  uint64_t lastChar, lastPlus;
  uint8_t inbuf[MAX_BUF];
  int iptr, optr, len;
  void *p;
  int (*dial)(void *p, char *number);
} modem_t;

static void modem_put(modem_t *modem, uint8_t c) {
  if (modem->len < MAX_BUF) {
    modem->len++;
    modem->inbuf[modem->iptr++] = c;
    if (modem->iptr == MAX_BUF) {
      modem->iptr = 0;
    }
  }
}

static uint8_t modem_get(modem_t *modem) {
  uint8_t c = 0;

  if (modem->len) {
    c = modem->inbuf[modem->optr++];
    if (modem->optr == MAX_BUF) {
      modem->optr = 0;
    }
    modem->len--;
  }

  return c;
}

static void modem_local(modem_t *modem, uint8_t c) {
  modem_put(modem, c);
}

static void modem_reply(modem_t *modem, char *s) {
  int i;

  for (i = 0; s[i]; i++) {
    modem_local(modem, s[i]);
  }
  modem_local(modem, '\r');
  modem_local(modem, '\n');
}

static void modem_ok(modem_t *modem) {
  modem_reply(modem, "OK");
}

static void modem_error(modem_t *modem) {
  modem_reply(modem, "ERROR");
}

static void modem_cmd_reply(modem_t *modem, int r) {
  if (r == 0) {
    modem_ok(modem);
  } else {
    modem_error(modem);
  } 
}

static void modem_cmd01(modem_t *modem, char c, int (*cmd)(modem_t *modem, int r)) {
  int r = -1;
  
  switch (c) {
    case 0:
      r = cmd(modem, 0);
      break;
    case '0':
    case '1':
      r = cmd(modem, c - '0');
      break;
  }

  modem_cmd_reply(modem, r);
}

static int modem_echo(modem_t *modem, int r) {
  modem->echo = r;
  return 0;
}

static int modem_hook_status(modem_t *modem, int s) {
  int r = -1;
  
  if (s == 0) {
    modem->dial(modem->p, NULL);
    r = 0;
  }

  return r;
}

static int modem_data_mode(modem_t *modem, int r) {
  modem->commandMode = 0;
  return 0;
}

static void modem_list_commands(modem_t *modem) {
  modem_reply(modem, "Available commands:");
  modem_reply(modem, "  \"[n]\" means optional argument, \"|\" means alternate arguments");
  modem_reply(modem, "ATE[0]|1 : 0=disables echo, 1=enables echo");
  modem_reply(modem, "ATI0 : prints version information");
  modem_reply(modem, "ATI1 : prints WiFi status");
  modem_reply(modem, "ATI2 : prints currently assigned IP address");
  modem_reply(modem, "ATDhostname:port : connects to server/port");
  modem_reply(modem, "ATH[0] : disconnects from server");
  modem_reply(modem, "ATO[0]|1 : returns to data mode (if already connected)");
  modem_reply(modem, "+++ : enters command mode (when in data mode)");
}

static void modem_process(modem_t *modem) {
  int i;
  
  if (modem->n >= 2) {
    for (i = 0; i < modem->n; i++) {
      if (modem->cmd[i] >= 'a' && modem->cmd[i] <= 'z') {
        modem->cmd[i] -= 32;
      }
    }
    if (modem->cmd[0] == 'A' && modem->cmd[1] == 'T') {
      switch (modem->cmd[2]) {
        case 0:
          modem_ok(modem);
          break;
        case '?':
          if (modem->cmd[3] == 0) {
            modem_list_commands(modem);
          } else {
            modem_error(modem);
          }
          break;
        case 'I':  // info
          switch (modem->cmd[3]) {
            case 0:
            case '0':
              modem_reply(modem, "Modem Ready");
              modem_ok(modem);
              break;
            default:
              modem_error(modem);
              break;
          }
          break;
        case 'E':  // echo
          modem_cmd01(modem, modem->cmd[3], modem_echo);
          break;
        case 'D':  // dial
          if (modem->dial(modem->p, &modem->cmd[3]) == 0) {
            modem_reply(modem, "CONNECT");
            modem->commandMode = 0;
          } else {
            modem_error(modem);
          }
          break;
        case 'H':  // hook status
          modem_cmd01(modem, modem->cmd[3], modem_hook_status);
          break;
        case 'O':
          modem_data_mode(modem, 0);
          break;
        default:
          modem_error(modem);
          break;
      }
    } else {
      modem_error(modem);
    }
  } else {
    modem_error(modem);
  }
}

static void modem_flush_plus(conn_filter_t *filter) {
  modem_t *modem = (modem_t *)filter->data;
  uint8_t b;
  int i;

  if (filter->next) {
    for (i = 0, b = '+'; i < modem->plus; i++) {
      filter->next->write(filter->next, &b, 1);
    }
  }
  modem->plus = 0;
}

static int modem_filter_peek(conn_filter_t *filter, uint32_t us) {
  modem_t *modem = (modem_t *)filter->data;
  uint64_t now;
  uint8_t b;
  int r;

  now = sys_get_clock();
  if (modem->plus && (now - modem->lastPlus) >= 1000000) {
    modem_flush_plus(filter);
  }

  for (;;) {
    if (modem->len) {
      return 1;
    }
    if (filter->next == NULL) return 0;
    r = filter->next->peek(filter->next, us);
    if (r == 0) return 0;
    if (r == -1) return -1;

    r = filter->next->read(filter->next, &b);
    if (r <= 0) return -1;
    if (modem->commandMode) return 0;
    if (b) modem_put(modem, b);
  }

  return 0;
}

static int modem_filter_read(conn_filter_t *filter, uint8_t *b) {
  modem_t *modem = (modem_t *)filter->data;
  uint64_t now;
  uint8_t c;
  int r;

  now = sys_get_clock();
  if (modem->plus && (now - modem->lastPlus) >= 1000000) {
    modem_flush_plus(filter);
  }

  for (;;) {
    if (modem->len) {
      *b = modem_get(modem);
      return 1;
    }
    if (filter->next == NULL) return 0;
    r = filter->next->read(filter->next, &c);
    if (r <= 0) return -1;
    if (modem->commandMode) return 0;
    if (c) modem_put(modem, c);
  }

  return 0;
}

static void modem_write(conn_filter_t *filter, uint8_t c) {
  modem_t *modem = (modem_t *)filter->data;
  uint64_t now;
  
  if (modem->commandMode) {
    switch (c) {
      case '\b':
        if (modem->n) {
          modem_local(modem, '\b');
          modem_local(modem, ' ');
          modem_local(modem, '\b');
          modem->n--;
        }
        break;
      case '\r':
      case '\n':
        modem_local(modem, '\r');
        modem_local(modem, '\n');
        modem->cmd[modem->n] = 0;
        if (modem->n) modem_process(modem);
        modem->n = 0;
        break;   
      default:
        if (modem->echo) modem_local(modem, c);
        if (modem->n < MAX_CMD-1) modem->cmd[modem->n++] = c;
        break;
    }
  } else {
    now = sys_get_clock();
    if (c == '+') {
      if ((now - modem->lastChar) >= 1000000) {
        modem->lastPlus = now;
        modem->plus++;
        if (modem->plus == 3) {
          // transfers the modem from data mode to command mode.
          // Must be preceded by at least 1 second of no characters and followed by one second of no characters.
          // ATO0 or ATO returns the modem to data mode.
          modem_ok(modem);
          modem->commandMode = 1;
          modem->plus = 0;
        }
      } else {
        modem_flush_plus(filter);
        modem->lastChar = now;
        if (filter->next) filter->next->write(filter->next, &c, 1);
      }
    } else {
      modem_flush_plus(filter);
      modem->lastChar = now;
      if (filter->next) filter->next->write(filter->next, &c, 1);
    }
  }
}

static int modem_filter_write(conn_filter_t *filter, uint8_t *buf, int n) {
  int i;

  for (i = 0; i < n; i++) {
    modem_write(filter, buf[i]);
  }

  return n;
}

static modem_t *modem_init(void *p, int (*dial)(void *p, char *number)) {
  modem_t *modem;

  if ((modem = xcalloc(1, sizeof(modem_t))) != NULL) {
    modem->n = 0;
    modem->echo = 1;
    modem->plus = 0;
    modem->commandMode = 1;
    modem->lastChar = 0;
    modem->lastPlus = 0;
    modem->iptr = modem->optr = modem->len = 0;
    modem->p = p;
    modem->dial = dial;
  }

  return modem;
}

conn_filter_t *modem_filter(conn_filter_t *next, void *p, int (*dial)(void *p, char *number)) {
  conn_filter_t *filter;

  if ((filter = xcalloc(1, sizeof(conn_filter_t))) != NULL) {
    filter->peek = modem_filter_peek;
    filter->read = modem_filter_read;
    filter->write = modem_filter_write;
    filter->next = next;
    filter->data = modem_init(p, dial);
  }

  return filter;
}

void modem_close(conn_filter_t *filter) {
  modem_t *modem;

  if (filter) {
    modem = (modem_t *)filter->data;
    if (modem) xfree(modem);
    xfree(filter);
  }
}

void modem_set(conn_filter_t *filter, conn_filter_t *next) {
  modem_t *modem = (modem_t *)filter->data;

  conn_set(filter, next);

  if (!next) {
    modem->commandMode = 1;
    modem->iptr = modem->optr = modem->len = 0;
    modem->plus = 0;
  }
}

char modem_getstate(conn_filter_t *filter) {
  modem_t *modem;
  char c = '?';

  if (filter) {
    modem = (modem_t *)filter->data;
    c = modem->commandMode ? 'C' : 'D';
  }

  return c;
}
