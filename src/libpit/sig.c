#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <signal.h>

#include "thread.h"
#include "sys.h"
#include "sig.h"
#include "debug.h"

static void sig_set_finish(char *sig, int status) {
  debug(DEBUG_INFO, "SIGNAL", "received signal %s", sig);
  sys_set_finish(status);
}

static void sig_set_fault(char *sig) {
  debug(DEBUG_ERROR, "SIGNAL", "received signal %s", sig);

  sys_install_handler(SIGSEGV, SIG_DFL);
#ifdef SIGBUS
  sys_install_handler(SIGBUS,  SIG_DFL);
#endif
  sys_install_handler(SIGILL,  SIG_DFL);
  sys_install_handler(SIGFPE,  SIG_DFL);
}

static void signal_handler(int s) {
  switch (s) {
    case SIGINT:
      sig_set_finish("SIGINT", STATUS_SUCCESS);
      break;
    case SIGTERM:
      sig_set_finish("SIGTERM", STATUS_SUCCESS);
      break;
#ifdef SIGQUIT
    case SIGQUIT:
      sig_set_finish("SIGQUIT", STATUS_FAULT);
      break;
#endif
#ifdef SIGTTIN
    case SIGTTIN:
      sig_set_finish("SIGTTIN", STATUS_FAULT);
      break;
#endif
#ifdef SIGTTOU
    case SIGTTOU:
      sig_set_finish("SIGTTOU", STATUS_FAULT);
      break;
#endif
#ifdef SIGCHLD
    case SIGCHLD: {
        int status;
        sys_wait(&status);
        debug(DEBUG_INFO, "SIGNAL", "received signal SIGCHLD");
      }
      break;
#endif
    case SIGSEGV:
      sig_set_fault("SIGSEGV");
      break;
#ifdef SIGCBUS
    case SIGBUS:
      sig_set_fault("SIGBUS");
      break;
#endif
    case SIGILL:
      sig_set_fault("SIGILL");
      break;
    case SIGFPE:
      sig_set_fault("SIGFPE");
      break;
    default:
      debug(DEBUG_ERROR, "SIGNAL", "received signal %d", s);
      break;
  }
}

void signal_install_handlers(void) {
  sys_install_handler(SIGINT,  signal_handler);
  sys_install_handler(SIGTERM, signal_handler);
#ifdef SIGQUIT
  sys_install_handler(SIGQUIT, signal_handler);
#endif
#ifdef SIGTTIN
  sys_install_handler(SIGTTIN, signal_handler);
#endif
#ifdef SIGTTOU
  sys_install_handler(SIGTTOU, signal_handler);
#endif
#ifdef SIGCHLD
  sys_install_handler(SIGCHLD, signal_handler);
#endif
  sys_install_handler(SIGSEGV, signal_handler);
#ifdef SIGBUS
  sys_install_handler(SIGBUS,  signal_handler);
#endif
  sys_install_handler(SIGILL,  signal_handler);
  sys_install_handler(SIGFPE,  signal_handler);
#ifdef SIGPIPE
  sys_install_handler(SIGPIPE, SIG_IGN);
#endif
}
