#include <PalmOS.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <pty.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>

#include "os_shell.h"
#include "debug.h"

static int login_tty(int t) {
  if (setsid() == -1) {
    debug_errno("OSSHELL", "setsid");
    return -1;
  }

  if (ioctl(t, TIOCSCTTY, NULL) == -1) {
    debug_errno("OSSHELL", "ioctl(TIOCSCTTY)");
    return -1;
  }

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
  if (ioctl(slavefd, TIOCSWINSZ, &ws) == -1) {
    debug_errno("OSSHELL", "ioctl(TIOCSWINSZ)");
  }

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

int os_shell(int argc, char *argv[]) {
  struct timeval tv;
  int masterfd, slavefd;
  char ch, *line, *arg[3];
  char buf[256];
  fd_set rfds;
  pid_t pid;
  int cols, rows;
  int n, m, r;

  rows = 25;
  cols = 80;
  debug(DEBUG_INFO, "OSSHELL", "osshell start");
  debug(DEBUG_INFO, "OSSHELL", "window size is %dx%d", cols, rows);

  if (openpty(&masterfd, &slavefd, NULL, NULL, NULL) == -1) {
    debug_errno("OSSHELL", "openpty");
    return -1;
  }

  line = ttyname(slavefd);
  if (line) {
    debug(DEBUG_INFO, "OSSHELL", "tty \"%s\"", line);
  }

  pid = fork();
  if (pid < 0) {
    debug_errno("OSSHELL", "fork");
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
    if ((n = pumpkin_haschar()) == -1) break;
    if (n == 1) {
      if ((ch = pumpkin_getchar()) == -1) break;
    } else {
      ch = 0;
    }

    if (ch > 0) {
      if ((m = sys_write(masterfd, (uint8_t *)&ch, 1)) != 1) {
        if (m == -1) {
          debug_errno("OSSHELL", "write pty");
        } else {
          debug(DEBUG_INFO, "OSSHELL", "wrote fewer bytes to pty");
        }
        break;
      }
    }

    FD_ZERO(&rfds);
    FD_SET(masterfd, &rfds);

    tv.tv_sec = 0;
    tv.tv_usec = 10000;

    if ((r = select(masterfd+1, &rfds, NULL, NULL, &tv)) == -1) {
      debug_errno("OSSHELL", "select");
      break;
    }

    if (r > 0) {
      if (FD_ISSET(masterfd, &rfds)) {
        if ((n = read(masterfd, buf, sizeof(buf))) == -1) {
          debug_errno("OSSHELL", "read pty");
          break;
        }
        if (n > 0) {
          pumpkin_write(buf, n);
        }
      }
    }
  }

  close(masterfd);
  debug(DEBUG_INFO, "OSSHELL", "osshell end");

  return 0;
}
