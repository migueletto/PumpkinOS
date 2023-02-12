#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>

#include "script.h"
#include "pit_io.h"
#include "sock.h"
#include "debug.h"
#include "xalloc.h"

static int sock_server(char *tag, int pe, int dgram, io_callback_f cb, int index, void *data, bt_provider_t *bt) {
  char *host = NULL;
  script_int_t port;
  io_addr_t addr;
  int handle, r = -1;

  if (script_get_string(pe, index, &host) == 0 &&
      script_get_integer(pe, index+1, &port) == 0 &&
      io_fill_addr(host, port, &addr) == 0) {

    if (dgram) {
      handle = io_dgram_server(tag, &addr, cb, data, bt);
    } else {
      handle = io_stream_server(tag, &addr, cb, data, 0, bt);
    }
  
    if (handle != -1) {
      r = script_push_integer(pe, handle);
    }
  }

  if (host) xfree(host);

  return r;
}

int sock_stream_server(char *tag, int pe, io_callback_f cb, int index, void *data, bt_provider_t *bt) {
  return sock_server(tag, pe, 0, cb, index, data, bt);
}

int sock_dgram_server(char *tag, int pe, io_callback_f cb, int index, void *data) {
  return sock_server(tag, pe, 1, cb, index, data, NULL);
}

int sock_dgram_close(char *tag, int pe) {
  script_int_t handle;
  int r;

  if (script_get_integer(pe, 0, &handle) == -1) return -1;
  r = io_dgram_close(tag, handle);

  return script_push_boolean(pe, r == 0);
}

int sock_stream_close(char *tag, int pe) {
  script_int_t handle;
  int r;

  if (script_get_integer(pe, 0, &handle) == -1) return -1;
  r = io_stream_close(tag, handle);

  return script_push_boolean(pe, r == 0);
}

int sock_client(char *tag, int pe, io_callback_f cb, int index, void *data, bt_provider_t *bt) {
  script_int_t port, timeout;
  char *peer = NULL;
  io_addr_t addr;
  int handle, r = -1;

  memset(&addr, 0, sizeof(addr));

  if (script_get_string(pe, index, &peer) == 0 &&
      script_get_integer(pe, index+1, &port) == 0 &&
      script_opt_integer(pe, index+2, &timeout) == 0 &&
      io_fill_addr(peer, port, &addr) == 0) {

    if ((handle = io_stream_client(tag, &addr, cb, data, timeout, bt)) != -1) {
      r = script_push_integer(pe, handle);
    }
  }

  if (peer) xfree(peer);

  return r;
}

int sock_sendto(char *tag, int pe) {
  script_int_t handle, port;
  char *peer = NULL, *buf = NULL;
  io_addr_t addr;
  int len, r = -1;

  if (script_get_integer(pe, 0, &handle) == 0 &&
      script_get_string(pe, 1, &peer) == 0 &&
      script_get_integer(pe, 2, &port) == 0 &&
      script_get_lstring(pe, 3, &buf, &len) == 0 &&
      io_fill_addr(peer, port, &addr) == 0) {

    r = script_push_boolean(pe, io_sendto_handle(tag, handle, &addr, (unsigned char *)buf, len) == 0);
  }

  if (peer) xfree(peer);
  if (buf) xfree(buf);

  return r;
}

int sock_write(char *tag, int pe) {
  script_int_t handle;
  char *buf;
  int len, r = -1;

  if (script_get_integer(pe, 0, &handle) == 0 &&
      script_get_lstring(pe, 1, &buf, &len) == 0) {

    r = script_push_boolean(pe, io_write_handle(tag, handle, (unsigned char *)buf, len) == 0);
  }

  return r;
}
