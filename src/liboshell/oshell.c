#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#ifdef DARWIN
//#include <util.h>
int openpty(int *, int *, char *, struct termios *, struct winsize *);
#else
#include <pty.h>
#endif
#include <time.h>

#include "sys.h"
#include "script.h"
#include "vfs.h"
#include "filter.h"
#include "shell.h"
#include "oshell.h"
#include "debug.h"

static int login_tty(int t) {
  if (setsid() == -1) {
    debug_errno("OSHELL", "setsid");
    return -1;
  }

#ifndef DARWIN
  if (ioctl(t, TIOCSCTTY, NULL) == -1) {
    debug_errno("OSHELL", "ioctl(TIOCSCTTY)");
    return -1;
  }
#endif

  if (t != 0) dup2(t, 0);
  if (t != 1) dup2(t, 1);
  if (t != 2) dup2(t, 2);
  if (t > 2) close(t);

  return 0;
}

static int getptyslave(int masterfd, int slavefd, int cols, int rows) {
  struct termios termbuf, termbuf2;
  struct winsize ws;

  tcgetattr(masterfd, &termbuf);
  memcpy(&termbuf2, &termbuf, sizeof(struct termios));

  memset(&ws, 0, sizeof(ws));
  ws.ws_col = cols;
  ws.ws_row = rows;
#ifndef DARWIN
  if (ioctl(slavefd, TIOCSWINSZ, &ws) == -1) {
    debug_errno("OSHELL", "ioctl(TIOCSWINSZ)");
  }
#endif

  termbuf.c_lflag |= ECHO;
  termbuf.c_oflag |= OPOST|ONLCR;
  termbuf.c_iflag |= ICRNL;
  termbuf.c_iflag &= ~IXOFF;

  cfsetispeed(&termbuf, 9600);
  cfsetospeed(&termbuf, 9600);

  if (memcmp(&termbuf, &termbuf2, sizeof(struct termios))) {
    tcsetattr(masterfd, TCSANOW, &termbuf);
  }

  if (login_tty(slavefd) == -1) {
    return -1;
  }

  return 0;
}

int cmd_oshell(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data) {
  shell_provider_t *p;
  struct timeval tv;
  int masterfd, slavefd;
  char *line, *arg[3];
  char buf[256];
  fd_set rfds;
  pid_t pid;
  int cols, rows;
  int n, m, r;

  debug(DEBUG_INFO, "OSHELL", "oshell start");

  p = script_get_pointer(pe, SHELL_PROVIDER);
  p->window(shell, &cols, &rows);
  debug(DEBUG_INFO, "OSHELL", "window size is %dx%d", cols, rows);

  if (openpty(&masterfd, &slavefd, NULL, NULL, NULL) == -1) {
    debug_errno("OSHELL", "openpty");
    return -1;
  }

  line = ttyname(slavefd);
  if (line) {
    debug(DEBUG_INFO, "OSHELL", "tty \"%s\"", line);
  }

  pid = fork();
  if (pid < 0) {
    debug_errno("OSHELL", "fork");
    return -1;
  }

  if (pid) {
    // parent
    close(slavefd);

  } else {
    // child
    getptyslave(masterfd, slavefd, cols, rows);
    if (masterfd > 2) close(masterfd);
    arg[0] = "bash";
    arg[1] = "-l";  // login shell
    arg[2] = NULL;
    execv("/bin/sh", arg);
    exit(1);
  }

  for (;;) {
    if ((n = p->peek(shell, 0)) == -1) {
      break;
    }

    if (n > 0) {
      if ((n = p->read(shell, buf, sizeof(buf))) == -1) {
        break;
      }

      if (n > 0) {
        if ((m = sys_write(masterfd, (uint8_t *)buf, n)) != n) {
          if (m == -1) {
            debug_errno("OSHELL", "write pty");
          } else {
            debug(DEBUG_INFO, "OSHELL", "wrote fewer bytes to pty");
          }
          break;
        }
      }
    }

    FD_ZERO(&rfds);
    FD_SET(masterfd, &rfds);

    tv.tv_sec = 0;
    tv.tv_usec = 10000;

    if ((r = select(masterfd+1, &rfds, NULL, NULL, &tv)) == -1) {
      debug_errno("OSHELL", "select");
      break;
    }

    if (r > 0) {
      if (FD_ISSET(masterfd, &rfds)) {
        if ((n = read(masterfd, buf, sizeof(buf))) == -1) {
          debug_errno("OSHELL", "read pty");
          break;
        }
        if (n > 0 && (m = p->write(shell, buf, n)) != n) {
          break;
        }
      }
    }
  }

  close(masterfd);
  debug(DEBUG_INFO, "OSHELL", "oshell end");

  return 0;
}
