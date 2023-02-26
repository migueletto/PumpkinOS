#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include "sys.h"
#include "pit_io.h"
#include "ptr.h"
#include "thread.h"
#include "timeutc.h"
#include "filter.h"
#include "debug.h"
#include "xalloc.h"

// dgram server    : thread_server_read(peer,buf) -> sendto(peer, buf)
// stream server   : thread_server_read(buf) -> ignore buf
// connected stream: thread_server_read(buf) -> if IO_CMD == 0, write(peer, buf)
// accepted stream : thread_server_read(buf) -> if IO_CMD == 0, write(peer, buf)

#define MAX_BUFFER  65536

typedef struct {
  char *tag;
  bt_provider_t *bt;
  io_addr_t addr;
  int fd, connected, timer;
  unsigned char buffer[MAX_BUFFER];
  void *data;
  io_callback_f callback;
} io_server_t;

typedef struct {
  char *tag;
  bt_provider_t *bt;
  io_addr_t addr;
  io_loop_f loop;
  int ptr;
  int fd;
} io_simple_t;

typedef struct {
  char *tag;
  bt_provider_t *bt;
  io_addr_t src;
  io_addr_t addr;
  int fd, connect, reading, timeout, timer, end, has_src;
  int line, pos, remain_start, remain_len;
  unsigned char buffer[MAX_BUFFER];
  unsigned char reply[MAX_BUFFER];
  time_t next_try, lastdata;
  conn_filter_t *filter;
  void *data;
  io_callback_f callback;
} io_connection_t;

typedef struct {
  io_addr_t addr;
  uint16_t len;
  unsigned char buf;
} io_write_arg_t;

static int io_callback(io_connection_t *con, int event, int handle, uint8_t *buf, int len, void **param) {
  io_arg_t arg;
  int r;

  arg.event = event;
  arg.addr = &con->addr;
  arg.handle = handle;
  arg.fd = con->fd;
  arg.data = con->data;
  arg.buf = buf;
  arg.len = len;
  arg.param = param ? *param : NULL;

  r = con->callback(&arg);

  con->data = arg.data;
  if (param) *param = arg.param;

  return r;
}

static int io_callback_server(io_server_t *server, int event, io_addr_t *addr, int handle, uint8_t *buf, int len, void **param) {
  io_arg_t arg;
  int r;

  arg.event = event;
  arg.addr = addr;
  arg.handle = handle;
  arg.fd = server->fd;
  arg.data = server->data;
  arg.buf = buf;
  arg.len = len;
  arg.param = param ? *param : NULL;

  r = server->callback(&arg);

  server->data = arg.data;
  if (param) *param = arg.param;

  return r;
}

static int io_callback_bind(io_server_t *server, int handle) {
  return io_callback_server(server, IO_BIND, &server->addr, handle, NULL, 0, NULL);
}

static int io_callback_bind_error(io_server_t *server, int handle) {
  return io_callback_server(server, IO_BIND_ERROR, &server->addr, handle, NULL, 0, NULL);
}

static int io_callback_server_cmd(io_server_t *server, int handle, uint8_t *buf, int len) {
  return io_callback_server(server, IO_CMD, &server->addr, handle, buf, len, NULL);
}

static int io_callback_server_data(io_server_t *server, io_addr_t *addr, int handle, int len) {
  return io_callback_server(server, IO_DATA, addr, handle, server->buffer, len, NULL);
}

static int io_callback_close(io_server_t *server, int handle) {
  return io_callback_server(server, IO_CLOSE, &server->addr, handle, NULL, 0, NULL);
}

static int io_callback_accept(io_connection_t *con, int handle) {
  return io_callback(con, IO_ACCEPT, handle, NULL, 0, NULL);
}

static int io_callback_connect(io_connection_t *con, int handle) {
  return io_callback(con, IO_CONNECT, handle, NULL, 0, NULL);
}

static int io_callback_disconnect(io_connection_t *con, int handle) {
  return io_callback(con, IO_DISCONNECT, handle, NULL, 0, NULL);
}

static int io_callback_connect_error(io_connection_t *con, int handle) {
  return io_callback(con, IO_CONNECT_ERROR, handle, NULL, 0, NULL);
}

static int io_callback_cmd(io_connection_t *con, int handle, uint8_t *buf, int len) {
  return io_callback(con, IO_CMD, handle, buf, len, NULL);
}

static int io_callback_data(io_connection_t *con, int handle, int len) {
  return io_callback(con, IO_DATA, handle, con->buffer, len, NULL);
}

static int io_callback_line(io_connection_t *con, int handle) {
  return io_callback(con, IO_LINE, handle, con->reply, con->pos, NULL);
}

static int io_callback_timeout(io_connection_t *con, int handle) {
  return io_callback(con, IO_TIMEOUT, handle, NULL, 0, NULL);
}

static int io_callback_timer(io_connection_t *con, int handle) {
  return io_callback(con, IO_TIMER, handle, NULL, 0, NULL);
}

static int io_callback_filter(io_connection_t *con, int handle) {
  return io_callback(con, IO_FILTER, handle, NULL, 0, (void **)&con->filter);
}

static int line(io_connection_t *con, int n, int handle) {
  int i;

  for (i = 0; i < n; i++) {
    if (con->buffer[i] == 10) {
      con->reply[con->pos] = 0;
      if (con->pos > 0 && con->reply[con->pos - 1] == 13) {
        con->pos--;
        con->reply[con->pos] = 0;
      }
      //if (con->callback(IO_LINE, &con->addr, con->reply, con->pos, con->fd, handle, &con->data)) {
      if (io_callback_line(con, handle)) {
        return 1;
      }
      con->pos = 0;

    } else if (con->pos < MAX_BUFFER-1) {
      con->reply[con->pos++] = con->buffer[i];
    }
  }

  return 0;
}

static int io_connect_addr(io_addr_t *src, io_addr_t *addr, bt_provider_t *bt) {
  int fd = -1;

  if (addr) {
    switch (addr->addr_type) {
      case IO_DEVICE_ADDR:
        fd = sys_serial_open(addr->addr.dev.device, addr->addr.dev.word, addr->addr.dev.baud);
        break;
      case IO_IP_ADDR:
        if (src) {
          fd = sys_socket_bind_connect(src->addr.ip.host, src->addr.ip.port, addr->addr.ip.host, addr->addr.ip.port, IP_STREAM);
        } else {
          fd = sys_socket_open_connect(addr->addr.ip.host, addr->addr.ip.port, IP_STREAM);
        }
        break;
      case IO_BT_ADDR:
        if (bt) {
          fd = bt->connect(addr->addr.bt.addr, addr->addr.bt.port, addr->addr.bt.type);
        } else {
          debug(DEBUG_ERROR, "IO", "no bluetooth provider to connect to %s %d", addr->addr.bt.addr, addr->addr.bt.port);
          fd = -1;
        }
        break;
      case IO_FD_ADDR:
        fd = addr->addr.fd.fd;
        break;
      default:
        debug(DEBUG_ERROR, "IO", "invalid address type %d", addr->addr_type);
        break;
    }
  }

  return fd;
}

int io_fill_addr(char *host, int port, io_addr_t *addr) {
  int i, nsep, sep2, r = -1;

  if (host && host[0] && addr) {
    memset(addr, 0, sizeof(io_addr_t));

    if (!strcmp(host, "fd")) {
      addr->addr_type = IO_FD_ADDR;
      addr->addr.fd.fd = port;

    } else if (host[0] == '/') {
      addr->addr_type = IO_DEVICE_ADDR;
      strncpy(addr->addr.dev.device, host, MAX_DEVICE-1);
      strncpy(addr->addr.dev.word, "N81", MAX_WORD);
      addr->addr.dev.baud = port;

    } else if (host[0] == '\\' && host[1] == '\\') {
      addr->addr_type = IO_DEVICE_ADDR;
      strncpy(addr->addr.dev.device, host, MAX_DEVICE-1);
      strncpy(addr->addr.dev.word, "N81", MAX_WORD);
      addr->addr.dev.baud = port;

    } else {
      if (strchr(host, ':') != NULL) {
        for (i = 0, nsep = 0, sep2 = 0; host[i]; i++) {
          if (host[i] == ':') {
            if (i && host[i-1] == ':') sep2 = 1;
            nsep++;
          }
        }

        if (!sep2 && nsep == 5) {
          addr->addr_type = IO_BT_ADDR;
          strncpy(addr->addr.bt.addr, host, MAX_ADDR-1);
          addr->addr.bt.port = port;
          addr->addr.bt.type = port > 0x1000 ? BT_L2CAP : BT_RFCOMM;

        } else {
          addr->addr_type = IO_IP_ADDR;
          strncpy(addr->addr.ip.host, host, MAX_HOST-1);
          addr->addr.ip.port = port;
        }
      } else {
        addr->addr_type = IO_IP_ADDR;
        strncpy(addr->addr.ip.host, host, MAX_HOST-1);
        addr->addr.ip.port = port;
      }
    }

    r = 0;
  }

  return r;
}

int io_bind(char *host, int port, int *addr_type, bt_provider_t *bt) {
  io_addr_t addr;
  int fd = -1;

  if (io_fill_addr(host, port, &addr) == 0) {
    *addr_type = addr.addr_type;

    switch (addr.addr_type) {
      case IO_IP_ADDR:
        fd = sys_socket_bind(addr.addr.ip.host, &addr.addr.ip.port, IP_STREAM);
        break;
      case IO_BT_ADDR:
        if (bt) {
          fd = bt->bind(addr.addr.bt.addr, addr.addr.bt.port, addr.addr.bt.type);
        } else {
          debug(DEBUG_ERROR, "IO", "no bluetooth provider to bind to %s %d", addr.addr.bt.addr, addr.addr.bt.port);
        }
        break;
    }
  }

  return fd;
}

int io_accept(int fd, uint32_t us, int addr_type, bt_provider_t *bt) {
  io_addr_t addr;
  sys_timeval_t tv;
  int newfd = -1;

  tv.tv_sec = 0;
  tv.tv_usec = us;

  switch (addr_type) {
    case IO_IP_ADDR:
      addr.addr_type = IO_IP_ADDR;
      newfd = sys_socket_accept(fd, addr.addr.ip.host, MAX_HOST-1, &addr.addr.ip.port, &tv);
      break;
    case IO_BT_ADDR:
      if (bt) {
        addr.addr_type = IO_BT_ADDR;
        newfd = bt->accept(fd, addr.addr.bt.addr, &addr.addr.bt.port, addr_type, &tv);
      } else {
        debug(DEBUG_ERROR, "IO", "no bluetooth provider to accept connections");
      }
      break;
  }

  return newfd;
}

int io_connect(char *peer, int port, bt_provider_t *bt) {
  io_addr_t addr;
  int fd = -1;

  if (io_fill_addr(peer, port, &addr) == 0) {
    fd = io_connect_addr(NULL, &addr, bt);
  }

  return fd;
}

static int io_stream_connection_loop(io_connection_t *con, int handle) {
  int r, nread;
  unsigned char *buf;
  unsigned int n;
  time_t t;

  t = sys_time();

  if (con->fd == -1 && con->connect && t > con->next_try) {
    if ((con->fd = io_connect_addr(con->has_src ? &con->src : NULL, &con->addr, con->bt)) != -1) {
      if (io_callback_filter(con, handle)) {
        debug(DEBUG_INFO, "IO", "exiting on filter callback");
        return -1;
      }

      if (io_callback_connect(con, handle)) {
        debug(DEBUG_INFO, "IO", "exiting on connect callback");
        return -1;
      }

    } else {
      if (io_callback_connect_error(con, handle)) {
        debug(DEBUG_INFO, "IO", "exiting on connection error callback");
        return -1;
      }
      con->next_try = t + 10;
      sys_usleep(10000);
    }

    return 0;
  }

  if ((r = thread_server_read(&buf, &n)) == -1) {
    return -1;
  }

  if (r == 1) {
    if (buf) {
      if (io_callback_cmd(con, handle, buf, n) == 0) {
        // if IO_CMD returns == 0, send data directly to device
        if (con->fd != -1) {
          if (con->filter) {
            r = con->filter->write(con->filter, buf, n);
          } else {
            r = sys_write(con->fd, buf, n);
          }
          if (r != n) {
            xfree(buf);
            debug(DEBUG_ERROR, "IO", "write error");
            return -1;
          }
        }
      }
      xfree(buf);
    }
  }

  if (con->fd == -1) {
    sys_usleep(10000);
    return 0;
  }

  if (con->filter) {
    if ((r = con->filter->peek(con->filter, con->timer ? con->timer*1000 : 20000)) > 0) {
      for (nread = 0; nread < MAX_BUFFER; nread++) {
        if ((r = con->filter->peek(con->filter, 0)) <= 0) break;
        if ((r = con->filter->read(con->filter, &con->buffer[nread])) < 0) break;
        if (r == 0) {
          r = 1;
          break;
        }
      }
    }
  } else {
    r = sys_read_timeout(con->fd, con->buffer, MAX_BUFFER, &nread, con->timer ? con->timer*1000 : 20000);
  }

  if (r == -1) {
    // select or read error
    debug(DEBUG_ERROR, "IO", "read error");
    return -1;
  } 

  if (r > 0) {
    if (nread == 0) {
      // peer disconnected
      debug(DEBUG_INFO, "IO", "peer disconnected");
      return -1;
    }

    // read ok
    if (con->line) {
      if (line(con, nread, handle)) {
        debug(DEBUG_INFO, "IO", "exiting on line callback");
        return -1;
      }
    }
    con->reading = 1;
    con->lastdata = t;
    if (io_callback_data(con, handle, nread)) {
      debug(DEBUG_INFO, "IO", "exiting on data callback");
      return -1;
    }

  } else {
    // nothing to read

    if (con->timer) {
      if (io_callback_timer(con, handle)) {
        return 1;
      }
    }

    if (con->reading) {
      con->reading = 0;
/* XXX do not send empty string
      if (con->callback(IO_DATA, &con->addr, NULL, 0, con->fd, handle, &con->data)) {
        debug(DEBUG_INFO, "IO", "exiting on data callback");
        return -1;
      }
*/
    }

    if (con->timeout && con->lastdata && (t - con->lastdata) > con->timeout) {
      debug(DEBUG_INFO, "IO", "timeout (%d - %d) > %d", t, con->lastdata, con->timeout);
      if (io_callback_timeout(con, handle)) {
        debug(DEBUG_INFO, "IO", "exiting on timeout callback");
        return -1;
      }
    }
  }

  return 0;
}

static int io_stream_connection_action(void *arg) {
  io_connection_t *con;
  int handle;

  con = (io_connection_t *)arg;
  handle = thread_get_handle();

  if (!con->connect) {
    if (io_callback_accept(con, handle)) {
      debug(DEBUG_INFO, "IO", "exiting on accept callback");
      if (con->fd != -1) sys_close(con->fd);
      xfree(con);
      return 0;
    }
    io_callback_filter(con, handle);
  }

  for (; !thread_must_end();) {
    if (io_stream_connection_loop(con, handle)) break;
  }

  io_callback_disconnect(con, handle);
  if (con->end) {
    thread_end(con->tag, handle);
  }
  debug(DEBUG_INFO, "IO", "closing fd %d", con->fd);
  sys_close(con->fd);
  xfree(con);

  return 0;
}

static int io_new_stream(char *tag, int fd, int line, int timeout, int timer, int end, io_addr_t *src, io_addr_t *addr, io_callback_f callback, void *data, bt_provider_t *bt) {
  io_connection_t *con;
  int handle;

  if ((con = xcalloc(1, sizeof(io_connection_t))) == NULL) {
    return -1;
  }

  if (src) {
    memcpy(&con->src, src, sizeof(io_addr_t));
    con->has_src = 1;
  }
  memcpy(&con->addr, addr, sizeof(io_addr_t));
  con->tag = tag;
  con->bt = bt;
  con->fd = fd;
  con->line = line;
  con->timeout = timeout;
  con->timer = timer;
  con->end = end;
  con->data = data;
  con->callback = callback;
  con->connect = fd == -1;

  if ((handle = thread_begin(tag, io_stream_connection_action, con)) == -1) {
    xfree(con);
  }

  return handle;
}

static int io_stream_server_loop(io_server_t *server, int handle) {
  sys_timeval_t tv;
  io_addr_t addr;
  int fd, r;
  unsigned char *buf;
  unsigned int n;

  if ((r = thread_server_read(&buf, &n)) == -1) {
    return -1;
  }

  if (r == 1 && buf) {
    io_callback_server_cmd(server, handle, buf, n);
    xfree(buf);
  }

  if (server->fd == -1) {
    // not bound yet
    switch (server->addr.addr_type) {
      case IO_IP_ADDR:
        server->fd = sys_socket_bind(server->addr.addr.ip.host, &server->addr.addr.ip.port, IP_STREAM);
        break;
      case IO_BT_ADDR:
        if (server->bt) {
          server->fd = server->bt->bind(server->addr.addr.bt.addr, server->addr.addr.bt.port, server->addr.addr.bt.type);
        } else {
          debug(DEBUG_ERROR, "IO", "no bluetooth provider to bind to %s %d", server->addr.addr.bt.addr, server->addr.addr.bt.port);
          server->fd = -1;
        }
        break;
    }

    if (server->fd != -1) {
      io_callback_bind(server, handle);
    } else {
      if (io_callback_bind_error(server, handle)) {
        return 1;
      }
      sys_usleep(500000);
    }
    return 0;
  }

  tv.tv_sec = 0;
  tv.tv_usec = 100000;

  switch (server->addr.addr_type) {
    case IO_IP_ADDR:
      addr.addr_type = IO_IP_ADDR;
      fd = sys_socket_accept(server->fd, addr.addr.ip.host, MAX_HOST-1, &addr.addr.ip.port, &tv);
      break;
    case IO_BT_ADDR:
      if (server->bt) {
        addr.addr_type = IO_BT_ADDR;
        fd = server->bt->accept(server->fd, addr.addr.bt.addr, &addr.addr.bt.port, server->addr.addr.bt.type, &tv);
      } else {
        debug(DEBUG_ERROR, "IO", "no bluetooth provider to accept connections");
        fd = -1;
      }
      break;
    default:
      fd = -1;
  }

  if (fd <= 0) {
    return 0;
  }

  if (io_new_stream(server->tag, fd, 1, 0, server->timer, 1, NULL, &addr, server->callback, server->data, server->bt) == -1) {
    sys_close(fd);
  }

  return 0;
}

static int io_stream_server_action(void *arg) {
  io_server_t *server;
  int handle;

  server = (io_server_t *)arg;
  handle = thread_get_handle();

  for (; !thread_must_end();) {
    if (io_stream_server_loop(server, handle)) break;
  }

  //server->callback(IO_CLOSE, &server->addr, NULL, 0, -1, handle, &server->data);
  io_callback_close(server, handle);

  if (server->fd != -1) {
    sys_close(server->fd);
  }
  //thread_end(server->tag, handle);
  xfree(server);

  return 0;
}

static int io_simple_conn_action(void *arg) {
  io_simple_t *conn;

  conn = (io_simple_t *)arg;
  conn->loop(conn->fd, conn->ptr);

  sys_close(conn->fd);
  thread_end(conn->tag, thread_get_handle());
  xfree(conn);

  return 0;
}

static int io_simple_server_action(void *arg) {
  io_simple_t *server, *conn;
  io_addr_t addr;
  sys_timeval_t tv;
  int fd;

  server = (io_simple_t *)arg;

  switch (server->addr.addr_type) {
    case IO_IP_ADDR:
      server->fd = sys_socket_bind(server->addr.addr.ip.host, &server->addr.addr.ip.port, IP_STREAM);
      break;
    case IO_BT_ADDR:
      if (server->bt) {
        server->fd = server->bt->bind(server->addr.addr.bt.addr, server->addr.addr.bt.port, server->addr.addr.bt.type);
      } else {
        debug(DEBUG_ERROR, "IO", "no bluetooth provider to bind to %s %d", server->addr.addr.bt.addr, server->addr.addr.bt.port);
        server->fd = -1;
      }
      break;
    default:
      server->fd = -1;
      break;
  }

  if (server->fd == -1) {
    thread_end(server->tag, thread_get_handle());
    ptr_free(server->ptr, server->tag);
    xfree(server);
    return -1;
  }

  for (; !thread_must_end();) {
    xmemset(&addr, 0, sizeof(io_addr_t));
    tv.tv_sec = 0;
    tv.tv_usec = 100000;

    switch (server->addr.addr_type) {
      case IO_IP_ADDR:
        addr.addr_type = IO_IP_ADDR;
        fd = sys_socket_accept(server->fd, addr.addr.ip.host, MAX_HOST-1, &addr.addr.ip.port, &tv);
        break;
      case IO_BT_ADDR:
        if (server->bt) {
          addr.addr_type = IO_BT_ADDR;
          fd = server->bt->accept(server->fd, addr.addr.bt.addr, &addr.addr.bt.port, server->addr.addr.bt.type, &tv);
        } else {
          debug(DEBUG_ERROR, "IO", "no bluetooth provider to accept connections");
          fd = -1;
        }
        break;
      default:
        fd = -1;
        break;
    }

    if (fd == 0) continue;
    if (fd == -1) break;
    if ((conn = xcalloc(1, sizeof(io_simple_t))) == NULL) break;

    xmemcpy(&conn->addr, &addr, sizeof(io_addr_t));
    conn->tag = server->tag;
    conn->bt = server->bt;
    conn->loop = server->loop;
    conn->ptr = server->ptr;
    conn->fd = fd;

    if (thread_begin(conn->tag, io_simple_conn_action, conn) == -1) break;
  }

  sys_close(server->fd);
  //thread_end(server->tag, thread_get_handle());
  ptr_free(server->ptr, server->tag);
  xfree(server);

  return 0;
}

static int io_simple_client_action(void *arg) {
  io_simple_t *client;

  client = (io_simple_t *)arg;
  client->fd = io_connect_addr(NULL, &client->addr, client->bt);

  if (client->fd != -1) {
    client->loop(client->fd, client->ptr);
  }

  sys_close(client->fd);
  //thread_end(client->tag, thread_get_handle());
  ptr_free(client->ptr, client->tag);
  xfree(client);

  return 0;
}

static int io_dgram_loop(io_server_t *server, int handle) {
  sys_timeval_t tv;
  io_addr_t addr;
  io_write_arg_t *arg;
  unsigned int n;
  int r, nread;

  if (server->fd == -1) {
    // not bound yet
    switch (server->addr.addr_type) {
      case IO_IP_ADDR:
        server->fd = sys_socket_bind(server->addr.addr.ip.host, &server->addr.addr.ip.port, IP_DGRAM);
        break;
    }
    if (server->fd != -1) {
      //server->callback(IO_BIND, &server->addr, NULL, 0, -1, handle, &server->data);
      io_callback_bind(server, handle);
    } else {
      //if (server->callback(IO_BIND_ERROR, &server->addr, NULL, 0, -1, handle, &server->data)) {
      if (io_callback_bind_error(server, handle)) {
        return 1;
      }
      sys_usleep(500000);
    }
    return 0;
  }

  if ((r = thread_server_read((unsigned char **)&arg, &n)) == -1) {
    return -1;
  }

  if (r == 1 && arg) {
    switch (server->addr.addr_type) {
      case IO_IP_ADDR:
        r = sys_socket_sendto(server->fd, arg->addr.addr.ip.host, arg->addr.addr.ip.port, &arg->buf, arg->len);
        break;
    }
    xfree(arg);
  }

  tv.tv_sec = 0;
  tv.tv_usec = 100000;

  switch (server->addr.addr_type) {
    case IO_IP_ADDR:
      addr.addr_type = IO_IP_ADDR;
      nread = sys_socket_recvfrom(server->fd, addr.addr.ip.host, MAX_HOST-1, &addr.addr.ip.port, server->buffer, MAX_BUFFER, &tv);
      break;
    default:
      nread = -1;
  }

  if (nread > 0) {
    //server->callback(IO_DATA, &addr, server->buffer, nread, -1, handle, &server->data);
    io_callback_server_data(server, &addr, handle, nread);
  }

  return 0;
}

static int io_dgram_action(void *arg) {
  io_server_t *server;
  int handle;

  server = (io_server_t *)arg;
  handle = thread_get_handle();

  for (; !thread_must_end();) {
    if (io_dgram_loop(server, handle)) break;
  }

  //server->callback(IO_CLOSE, &server->addr, NULL, 0, -1, handle, &server->data);
  io_callback_close(server, handle);

  if (server->fd != -1) {
    sys_close(server->fd);
  }
  xfree(server);

  return 0;
}

int io_stream_server(char *tag, io_addr_t *addr, io_callback_f callback, void *data, int timer, bt_provider_t *bt) {
  io_server_t *server;
  int handle;

  if (addr == NULL || data == NULL) {
    return -1;
  }

  switch (addr->addr_type) {
    case IO_BT_ADDR:
    case IO_IP_ADDR:
      break;
    default:
      return -1;
  }

  if ((server = xcalloc(1, sizeof(io_server_t))) == NULL) {
    return -1;
  }

  memcpy(&server->addr, addr, sizeof(io_addr_t));
  server->tag = tag;
  server->bt = bt;
  server->callback = callback;
  server->data = data;
  server->timer = timer;
  server->fd = -1;

  if ((handle = thread_begin(tag, io_stream_server_action, server)) == -1) {
    xfree(server);
  }

  return handle;
}

int io_simple_server(char *tag, io_addr_t *addr, io_loop_f loop, int ptr, bt_provider_t *bt) {
  io_simple_t *server;
  int handle;

  if (addr == NULL) {
    return -1;
  }

  switch (addr->addr_type) {
    case IO_BT_ADDR:
    case IO_IP_ADDR:
      break;
    default:
      return -1;
  }

  if ((server = xcalloc(1, sizeof(io_simple_t))) == NULL) {
    return -1;
  }

  memcpy(&server->addr, addr, sizeof(io_addr_t));
  server->tag = tag;
  server->bt = bt;
  server->loop = loop;
  server->ptr = ptr;

  if ((handle = thread_begin(tag, io_simple_server_action, server)) == -1) {
    xfree(server);
  }

  return handle;
}

int io_simple_client(char *tag, io_addr_t *addr, io_loop_f loop, int ptr, bt_provider_t *bt) {
  io_simple_t *client;
  int handle;

  if (addr == NULL) {
    return -1;
  }

  switch (addr->addr_type) {
    case IO_DEVICE_ADDR:
    case IO_BT_ADDR:
    case IO_IP_ADDR:
      break;
    default:
      return -1;
  }

  if ((client = xcalloc(1, sizeof(io_simple_t))) == NULL) {
    return -1;
  }

  memcpy(&client->addr, addr, sizeof(io_addr_t));
  client->tag = tag;
  client->bt = bt;
  client->loop = loop;
  client->ptr = ptr;

  if ((handle = thread_begin(tag, io_simple_client_action, client)) == -1) {
    xfree(client);
  }

  return handle;
}

int io_dgram_server(char *tag, io_addr_t *addr, io_callback_f callback, void *data, bt_provider_t *bt) {
  io_server_t *server;
  int handle;

  if (addr == NULL || data == NULL) {
    return -1;
  }

  switch (addr->addr_type) {
    case IO_IP_ADDR:
      break;
    default:
      return -1;
  }

  if ((server = xcalloc(1, sizeof(io_server_t))) == NULL) {
    return -1;
  }

  memcpy(&server->addr, addr, sizeof(io_addr_t));
  server->tag = tag;
  server->bt = bt;
  server->callback = callback;
  server->data = data;
  server->fd = -1;

  if ((handle = thread_begin(tag, io_dgram_action, server)) == -1) {
    xfree(server);
  }

  return handle;
}

int io_dgram_close(char *tag, int handle) {
  return thread_end(tag, handle);
}

int io_stream_close(char *tag, int handle) {
  return thread_end(tag, handle);
}

int io_stream_bound_client(char *tag, io_addr_t *src, io_addr_t *addr, io_callback_f callback, void *data, int timeout, bt_provider_t *bt) {
  int handle = -1;

  if (src && addr && data) {
    handle = io_new_stream(tag, -1, 1, timeout, 0, 1, src, addr, callback, data, bt);
  }

  return handle;
}

int io_stream_client(char *tag, io_addr_t *addr, io_callback_f callback, void *data, int timeout, bt_provider_t *bt) {
  int handle = -1;

  if (addr && data) {
    handle = io_new_stream(tag, -1, 1, timeout, 0, 1, NULL, addr, callback, data, bt);
  }

  return handle;
}

int io_sendto_handle(char *tag, int handle, io_addr_t *addr, unsigned char *buf, unsigned int len) {
  io_write_arg_t *arg;
  unsigned int n;
  int r;

  n = sizeof(io_write_arg_t) + len - 1;

  if ((arg = xcalloc(1, n)) == NULL) {
    return -1;
  }

  memcpy(&arg->addr, addr, sizeof(io_addr_t));
  arg->len = len;
  memcpy(&arg->buf, buf, len);

  r = thread_client_write(handle, (unsigned char *)arg, n);
  xfree(arg);

  return r == -1 ? -1 : 0;
}

int io_write_handle(char *tag, int handle, unsigned char *buf, unsigned int len) {
  return thread_client_write(handle, buf, len) == len ? 0 : -1;
}
